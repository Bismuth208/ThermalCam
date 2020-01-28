/*
 * Originally this code was based on this project:
 * http://blog.dzl.dk/2019/06/08/compact-gaussian-interpolation-for-small-displays/
 * http://blog.dzl.dk/2019/06/08/cheap-diy-thermal-imager/
 *
 */

#include "common.h"
#include "ironbow_palette.h"

// ----------------------------------------------------------------------
uint16_t usPaletteColors[IR_CAM_MAX_COLORS];


// ----------------------------------------------------------------------
//#define USE_SIGMA_05 // Signa = 0.5
#define USE_SIGMA_08 // Signa = 0.8
//#define USE_SIGMA_1  // Signa = 1.0
//#define USE_SIGMA_2  // Signa = 2.0


// ----------------------------------------------------------------------
Grid_t xGrid;

const int offset[4][9] = {
  { -33, -32, -32,
    -1 , 0  , 0,
    -1 , 0  , 0
  },
  { - 32, -32, -31,
    0, 0, 1,
    0, 0, 1
  },
  { - 1, 0, 0,
    -1, 0, 0,
    31, 32, 32
  },
  { 0, 0, 1,
    0, 0, 1,
    32, 32, 33
  },
};

const float fKernelWeights[IR_INTERPOLATION_K_WEIGHTS] = {
#ifdef USE_SIGMA_05
  0.024879,  0.107973,  0.024879,
  0.107973,  0.468592,  0.107973,
  0.024879,  0.107973,  0.024879
#endif
#ifdef USE_SIGMA_08
  0.062920, 0.124998,  0.062920,
  0.124998, 0.248326,  0.124998,
  0.062920, 0.124998,  0.062920
#endif
#ifdef USE_SIGMA_1
  0.077847,  0.123317,  0.077847,
  0.123317,  0.195346,  0.123317,
  0.077847,  0.123317,  0.077847
#endif
#ifdef USE_SIGMA_2
  0.102059,  0.115349,  0.102059,
  0.115349,  0.130371,  0.115349,
  0.102059,  0.115349,  0.102059
#endif
};


// ----------------------------------------------------------------------
void vGridSetPaletteType(uint32_t ulPaletteType)
{
  switch (ulPaletteType)
  {
    case IR_PALETTE_TYPE_GRAYSCALE: {
      for (uint32_t i = 0; i < IR_CAM_MAX_COLORS; i++) {
        usPaletteColors[i] = tft.color565(i, i, i);
      }
    }
    break;

    case IR_PALETTE_TYPE_IRONBOW: {
      for (uint32_t i = 0; i < IR_CAM_MAX_COLORS; i++) {
        usPaletteColors[i] = tft.color565(ucIrIronbowPaletteData[i][0], ucIrIronbowPaletteData[i][1], ucIrIronbowPaletteData[i][2]);
      }
    }
    break;

    default: break;
  }
}

void vGridInit(void)
{
   // meh... magic numbers...
  xGrid.ulScreenX = 16;
  xGrid.ulScreenY = 2;
  // yyyes, they swapped... for some reason.
  xGrid.ulScreenH = tft.width();
  xGrid.ulScreenW = tft.height();
  // ?
  xGrid.pfScreenData = &xGrid.buff[32];
  xGrid.buff[500] = 40;

  xGrid.fEmissivity = 0.93;
}

// ----------------------------------------------------------------------
void vGridPlace(int px, int py, int w, int h)
{
  xGrid.ulScreenW = w;
  xGrid.ulScreenH = h;
}

void vGridMakeAvg(void)
{
  memset(&mlx90640To[0], 0x00, sizeof(mlx90640To));
  
  for (uint32_t j = 0; j < IR_SENSOR_DATA_FRAME_SIZE; j++) {
    for (uint32_t i = 0; i < IR_ADC_OVERSAMPLING_COUNT; i++) {
      mlx90640To[j] += fMLX90640Oversampling[i][j];
    }

    mlx90640To[j] = (mlx90640To[j] / IR_ADC_OVERSAMPLING_COUNT);
  }
}

void vGridFindMinMax(void)
{
  float high = -1000;
  float low = 1000;
  
  for (int x = 0; x < IR_SENSOR_DATA_FRAME_SIZE; x++) {    
    float t = (float) mlx90640To[x];
    if (t > high)
      high = t;
    if (t < low)
      low = t;

    xGrid.pfScreenData[x] =  t;
  }

  // calculate avarage temp in center
  float t = 0;
  t += mlx90640To[335];
  t += mlx90640To[336];

  t += mlx90640To[366];
  t += mlx90640To[367];
  t += mlx90640To[368];
  t += mlx90640To[369];

  t += mlx90640To[398];
  t += mlx90640To[399];
  t += mlx90640To[400];
  t += mlx90640To[401];

  t += mlx90640To[431];
  t += mlx90640To[432];
  t /= 12.0;

  xGrid.fAvgCenter = t;

  // I have no idea what this thing is doing here...
#if 0
  xGrid.fHigh = xGrid.high * 0.9 + 0.1 * high * 1.2;
  xGrid.fLow = xGrid.low * 0.9 + 0.1 * (1.0/1.2) * low;
#else
  xGrid.fHigh = high;
  xGrid.fLow = low;
#endif
}

void vGridDrawInterpolated(void)
{
  float pix;
  int dx = xGrid.ulScreenW / IR_SENSOR_MATRIX_2W;
  int dy = xGrid.ulScreenH / IR_SENSOR_MATRIX_2H;

  for (int i = 0; i < (IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H); i++) {
    pix = 0;
    
    int sourceAddress = ((i >> 1) & 0x1f) + ((i & 0xffffff80) >> 2);
    int q = (i & 0x00000001) + ((i & 0x00000040) >> 5);
    
    for (int z = 0; z < IR_INTERPOLATION_K_WEIGHTS; z++) {
      pix += fKernelWeights[z] * (float) xGrid.pfScreenData[sourceAddress + offset[q][z]];
    }
    int x = i & 0x0000003f;
    int y = i >> 6;
    int v = map(pix, xGrid.fLow, xGrid.fHigh, 0, IR_CAM_MAX_COLORS);

#if 1
    if (v < 0) {
      v = 0;
    }
    if (v > (IR_CAM_MAX_COLORS-1)) {
      v = (IR_CAM_MAX_COLORS-1);
    }
#endif

    // up, down and right frame clip
    if ( (i < (IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H - IR_SENSOR_MATRIX_2W)) 
      && (i > (IR_SENSOR_MATRIX_2W))
      && ( ( i % IR_SENSOR_MATRIX_2W) != 0) )
    {
      // also make horizontal flip
      tft.fillRect(xGrid.ulScreenW - (x*2) + xGrid.ulScreenX, (y*2) + xGrid.ulScreenY, dx, dy, usPaletteColors[v]);
    }

    // origin size; Debug sensor
//    tft.drawPixel(x + xGrid.ulScreenX, y + xGrid.ulScreenY, usPaletteColors[v]);
  }
}
