#ifndef _COMMON_H
#define _COMMON_H


// ----------------------------------------------------------------------
#define VERSION_STR "v0.1a"


// ----------------------------------------------------------------------
#include <Adafruit_GFX.h>
//#include <Adafruit_ILI9341.h>
#include <Adafruit_ST7735.h>


// ----------------------------------------------------------------------
#define GUI_PROGRESS_BAR_STEP_SIZE 14
#define GUI_BOOT_LOGO_DELAY        (1000) // 1 sec.

// ----------------------------------------------------------------------
#define IR_SENSOR_MATRIX_W 32 // px
#define IR_SENSOR_MATRIX_H 24 // px

// double resolution interpolation
#define IR_SENSOR_MATRIX_2W 64 // px
#define IR_SENSOR_MATRIX_2H 48 // px

#define IR_SENSOR_DATA_FRAME_SIZE (IR_SENSOR_MATRIX_W * IR_SENSOR_MATRIX_H) // 768

#define IR_SENSOR_COLD_READS_NUM  8 // nuber of frames to read on start

#define IR_INTERPOLATION_K_WEIGHTS  9

#define IR_ADC_OVERSAMPLING_COUNT 10

#define IR_SAMPLING_LOW_Q  MLX90640_SPEED_8_HZ
#define IR_SAMPLING_HI_Q   MLX90640_SPEED_4_HZ

#define IR_SAMPLING_RES_LOW_Q MLX90640_RESOLUTION_16BIT
#define IR_SAMPLING_RES_HI_Q  MLX90640_RESOLUTION_19BIT

#define IR_SAMPLING_LOW_Q_TEXT "8x"
#define IR_SAMPLING_HI_Q_TEXT  "4x"


// ----------------------------------------------------------------------
typedef struct {
  uint16_t mlx90640Frame[834];
} IrCamDataFrame_t;


typedef struct {
  float buff[IR_SENSOR_MATRIX_W * IR_SENSOR_MATRIX_H + IR_SENSOR_MATRIX_2W];
  float *pfScreenData;

  uint16_t ulScreenX;
  uint16_t ulScreenY;
  uint16_t ulScreenW;
  uint16_t ulScreenH;
  float fHigh;
  float fLow;
  float fAvgCenter;
  float fEmissivity;

  BaseType_t xHiPrecisionModeIsEn;
} Grid_t;

// A rate of 0.5Hz takes 4Sec per reading because we have to read two frames to get complete picture
typedef enum {
  MLX90640_SPEED_025_HZ = 0x00, // 0.25Hz effective - Works
  MLX90640_SPEED_05_HZ,         // 0.5Hz effective - Works
  MLX90640_SPEED_1_HZ,          // 1Hz effective - Works
  MLX90640_SPEED_2_HZ,          // 2Hz effective - Works
  MLX90640_SPEED_4_HZ,          // 4Hz effective - Works
  MLX90640_SPEED_8_HZ,          // 8Hz effective - Works at 800kHz
  MLX90640_SPEED_16_HZ,         // 16Hz effective - Works at 800kHz
  MLX90640_SPEED_32_HZ,         // 32Hz effective - fails
  MLX90640_SPEED_64_HZ          // 64Hz effective - fails
} MLX90640_SamplingSpeeds_t;

typedef enum {
  MLX90640_RESOLUTION_16BIT = 0x00,
  MLX90640_RESOLUTION_17BIT,
  MLX90640_RESOLUTION_18BIT,
  MLX90640_RESOLUTION_19BIT
} MLX90640_Resolutions_t;

typedef enum {
  IR_PALETTE_TYPE_GRAYSCALE = 0x00,
  IR_PALETTE_TYPE_IRONBOW
} IR_Palettes_t;

extern Grid_t xGrid;


// ----------------------------------------------------------------------
void vGridInit(void);

void vGridSetPaletteType(uint32_t ulPaletteType);
void vGridPlace(int px, int py, int w, int h);
void vGridSetColorMode(int mode);
void vGridMakeAvg(void);
void vGridMakeFast(void);
void vGridDrawInterpolated(void);


// ----------------------------------------------------------------------
extern Adafruit_ST7735 tft;
extern float fMLX90640Oversampling[IR_ADC_OVERSAMPLING_COUNT][IR_SENSOR_DATA_FRAME_SIZE];


#endif // _COMMON_H
