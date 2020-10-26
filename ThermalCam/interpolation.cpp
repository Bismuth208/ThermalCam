/*
 * Originally this code was based on this project:
 * http://blog.dzl.dk/2019/06/08/compact-gaussian-interpolation-for-small-displays/
 * http://blog.dzl.dk/2019/06/08/cheap-diy-thermal-imager/
 *
 * Quick and dirty thermal imager code
 * (F) DZL 2016
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
// https://forum.arduino.cc/index.php?topic=247210.0
template < typename T, typename U > struct IsSameType{ enum { Value = false }; };
template < typename T > struct             IsSameType< T, T > { enum { Value = true }; };

template< typename T, typename U, typename V, typename X, typename Y > 
auto map_a( const T &x, U &&imin, V &&imax, X &&omin, Y &&omax ) -> T
{
  typedef decltype( IsSameType< T, decltype( ( signed ) T() ) >::Value ? 1LL : 1ULL ) cll;
  return (cll)(x - imin) * (omax - omin) / (imax - imin) + omin;
}


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
  xGrid.buff[500] = 40; // why this is here and what it does !?

  xGrid.fEmissivity = 0.95;
}

// ----------------------------------------------------------------------
void vGridPlace(int px, int py, int w, int h)
{
  xGrid.ulScreenW = w;
  xGrid.ulScreenH = h;
}

// ----------------------------------------------------------------------
void vGridFindMaxMinAvg(float f_high, float f_low)
{
  // calculate avarage temp in center
  float t = 0;
  t += xGrid.pfScreenData[335];
  t += xGrid.pfScreenData[336];

  t += xGrid.pfScreenData[366];
  t += xGrid.pfScreenData[367];
  t += xGrid.pfScreenData[368];
  t += xGrid.pfScreenData[369];

  t += xGrid.pfScreenData[398];
  t += xGrid.pfScreenData[399];
  t += xGrid.pfScreenData[400];
  t += xGrid.pfScreenData[401];

  t += xGrid.pfScreenData[431];
  t += xGrid.pfScreenData[432];
  t /= 12.0;

  xGrid.fAvgCenter = t;

  // I have no idea what this thing is doing here...
#if 0
  xGrid.fHigh = xGrid.high * 0.9 + 0.1 * f_high * 1.2;
  xGrid.fLow = xGrid.low * 0.9 + 0.1 * (1.0/1.2) * f_low;
#else
  xGrid.fHigh = f_high;
  xGrid.fLow = f_low;
#endif
}

void vGridMakeAvg(void)
{
  float f_high = -1000;
  float f_low = 1000;
  float f_tmp_mlx_val = 0;

  if (xGrid.xHiPrecisionModeIsEn == pdFALSE) {
    for (uint32_t j = 0; j < IR_SENSOR_DATA_FRAME_SIZE; j++) {
      for (uint32_t i = 0; i < IR_ADC_OVERSAMPLING_COUNT; i++) {
        f_tmp_mlx_val += fMLX90640Oversampling[i][j];
      }

      f_tmp_mlx_val = (f_tmp_mlx_val / IR_ADC_OVERSAMPLING_COUNT);
  
      if (f_tmp_mlx_val > f_high) {
        f_high = f_tmp_mlx_val;
      }
      if (f_tmp_mlx_val < f_low) {
        f_low = f_tmp_mlx_val;
      }
  
      xGrid.pfScreenData[j] = f_tmp_mlx_val;
      f_tmp_mlx_val = 0;
    }
  } else {
    // smaaal implementation of circular buffer...
    uint32_t ulPos = (IR_ADC_OVERSAMPLING_COUNT + xGrid.ulOversamplingPos - 1) % IR_ADC_OVERSAMPLING_COUNT;
    
    for (uint32_t j = 0; j < IR_SENSOR_DATA_FRAME_SIZE; j++) {
      f_tmp_mlx_val = fMLX90640Oversampling[ulPos][j];
  
      if (f_tmp_mlx_val > f_high) {
        f_high = f_tmp_mlx_val;
      }
      if (f_tmp_mlx_val < f_low) {
        f_low = f_tmp_mlx_val;
      }
  
      xGrid.pfScreenData[j] = f_tmp_mlx_val;
    }
  }

  vGridFindMaxMinAvg(f_high, f_low);
}

void vGridDrawInterpolated(void)
{
  float pix;
  int dx = xGrid.ulScreenW / IR_SENSOR_MATRIX_2W;
  int dy = xGrid.ulScreenH / IR_SENSOR_MATRIX_2H;

  tft.startWrite();

  // down frame clip
  for (int i = 0; i < (IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H - IR_SENSOR_MATRIX_2W); i++) {
    pix = 0;
    
    int sourceAddress = ((i >> 1) & 0x1f) + ((i & 0xffffff80) >> 2);
    int q = (i & 0x00000001) + ((i & 0x00000040) >> 5);
    
    for (int z = 0; z < IR_INTERPOLATION_K_WEIGHTS; z++) {
      pix += fKernelWeights[z] * (float) xGrid.pfScreenData[sourceAddress + offset[q][z]];
    }
    
    int x = i & 0x0000003f;
    int y = i >> 6;
    int v = map_a(pix, xGrid.fLow, xGrid.fHigh, 0, IR_CAM_MAX_COLORS);

    v = constrain(v, 0, (IR_CAM_MAX_COLORS-1));

    // down frame clip    
//    if (i < (IR_SENSOR_MATRIX_W * IR_SENSOR_MATRIX_2H - IR_SENSOR_MATRIX_2W))
    {
      // also make horizontal flip
      //tft.fillRect(xGrid.ulScreenW - (x*2) + xGrid.ulScreenX, (y*2) + xGrid.ulScreenY, dx, dy, usPaletteColors[v]);
      tft.writeFillRectPreclipped(xGrid.ulScreenW - (x*2) + xGrid.ulScreenX, (y*2) + xGrid.ulScreenY, dx, dy, usPaletteColors[v]);
    }

    // origin size; Debug sensor
//    tft.drawPixel(x + xGrid.ulScreenX, y + xGrid.ulScreenY, usPaletteColors[v]);
    ucFrameBuffer[i] = v;
  }

  tft.endWrite();

#if 0
  tft.startWrite();
  tft.setAddrWindow(xGrid.ulScreenX, xGrid.ulScreenY, IR_SENSOR_MATRIX_2W, IR_SENSOR_MATRIX_2H);
  tft.writePixels(&usFrameBuffer[0], IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H);
  tft.endWrite();
#endif
}
