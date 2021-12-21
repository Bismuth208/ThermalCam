#ifndef _IR_SENSOR_H
#define _IR_SENSOR_H

// ----------------------------------------------------------------------
#include <MLX90640_API.h>
#include <MLX90640_I2C_Driver.h>

#include <FreeRTOS_helper.hpp>

// ----------------------------------------------------------------------
// Default shift for MLX90640 in open air
#define TA_SHIFT 8

// ----------------------------------------------------------------------
// Default 7-bit unshifted address of the MLX90640
#define IR_SENSOR_I2C_ADDR 0x33

// Basic resolution of MLX sensor (in pixels)
#define IR_SENSOR_MATRIX_W 32
#define IR_SENSOR_MATRIX_H 24

// Doubled resolution after interpolation (in pixels)
#define IR_SENSOR_MATRIX_2W (IR_SENSOR_MATRIX_W * 2)
#define IR_SENSOR_MATRIX_2H (IR_SENSOR_MATRIX_H * 2)

// Size of single frame (768 pixels)
#define IR_SENSOR_DATA_FRAME_SIZE (IR_SENSOR_MATRIX_W * IR_SENSOR_MATRIX_H)

// Nuber of "cold" frames to read on start
#define IR_SENSOR_COLD_READS_NUM  8

#define IR_INTERPOLATION_K_WEIGHTS  9

// Number of frame samples
#define IR_ADC_OVERSAMPLING_COUNT 10

#define IR_CAM_DATA_FRAMES 2

#define IR_CAM_MAX_COLORS 250

// Pre-definition of Minumum & Maximum frequency polling
// This is need for image quality swithing
#define IR_SAMPLING_LOW_Q  MLX90640_SPEED_8_HZ
#define IR_SAMPLING_HI_Q   MLX90640_SPEED_4_HZ

// Pre-definition of Minumum & Maximum resolution of MLX ADC
// This is need for image quality swithing
#define IR_SAMPLING_RES_LOW_Q MLX90640_RESOLUTION_18BIT
#define IR_SAMPLING_RES_HI_Q  MLX90640_RESOLUTION_18BIT // or 19 bits ?

#define IR_SAMPLING_LOW_Q_TEXT "8x"
#define IR_SAMPLING_HI_Q_TEXT  "4x"

// ----------------------------------------------------------------------
//#define IR_SENSOR_SD_THERMOGRAM_FILE  "%s/%05u.thc"
//#define IR_SENSOR_SD_THERMOGRAMS_PATH "/mlx"
//#define IR_SENSOR_SD_THERMOGRAM_PATH  "/sdcard/mlx/%04u" 

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

  uint16_t usSreenDx;
  uint16_t usSreenDy;
  
  float fHigh;
  float fLow;
  float fAvgCenter;
  float fEmissivity;

  BaseType_t xHiPrecisionModeIsEn;
  uint32_t ulOversamplingPos;
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


// ----------------------------------------------------------------------
extern uint8_t ucFrameBuffer[IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H];
extern uint16_t usPaletteColors[IR_CAM_MAX_COLORS];

extern Grid_t xGrid;

extern paramsMLX90640 x_mlx90640;
extern IrCamDataFrame_t x_mlx90640Frame;
extern float fMLX90640Oversampling[IR_ADC_OVERSAMPLING_COUNT][IR_SENSOR_DATA_FRAME_SIZE];

extern Mutex MLX90640Mutex;
extern Counter <IR_ADC_OVERSAMPLING_COUNT> mlx90640FrameRdyCounter;
extern Task <2048>GetFrameDataTask;

// ----------------------------------------------------------------------
void vInitSensor(void);

void vStartColdReadings(IrCamDataFrame_t *pxIrCamDataFrame);

void vMLX90640_EnableHiQualityMode(BaseType_t xEnable);
void vMLX90640_GetDataFrame(IrCamDataFrame_t *pxIrCamDataFrame);

void vGetFrameDataTask(void *pvArg);

#endif // _IR_SENSOR_H
