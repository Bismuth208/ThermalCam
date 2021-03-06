#ifndef _COMMON_H
#define _COMMON_H


// ----------------------------------------------------------------------
#define VERSION_STR "v0.4c"
#define OTA_STR     "OTA"


// ----------------------------------------------------------------------
#include <Adafruit_GFX.h>
//#include <Adafruit_ILI9341.h>
#include <Adafruit_ST7735.h>

#include <MLX90640_API.h>
#include <MLX90640_I2C_Driver.h>

#include <FreeRTOS_helper.hpp>


// ----------------------------------------------------------------------
#define GUI_PROGRESS_BAR_STEP_SIZE 14
#define GUI_BOOT_LOGO_DELAY        (1000) // 1 sec.

// ----------------------------------------------------------------------
#define TA_SHIFT 8 //Default shift for MLX90640 in open air

// ----------------------------------------------------------------------
#define IR_SENSOR_I2C_ADDR 0x33  //Default 7-bit unshifted address of the MLX90640

#define IR_SENSOR_MATRIX_W 32 // px
#define IR_SENSOR_MATRIX_H 24 // px

// double resolution interpolation
#define IR_SENSOR_MATRIX_2W 64 // px
#define IR_SENSOR_MATRIX_2H 48 // px

#define IR_SENSOR_DATA_FRAME_SIZE (IR_SENSOR_MATRIX_W * IR_SENSOR_MATRIX_H) // 768

#define IR_SENSOR_COLD_READS_NUM  8 // nuber of frames to read on start

#define IR_INTERPOLATION_K_WEIGHTS  9

#define IR_ADC_OVERSAMPLING_COUNT 10

#define IR_CAM_DATA_FRAMES 2

#define IR_CAM_MAX_COLORS 250

#define IR_SAMPLING_LOW_Q  MLX90640_SPEED_8_HZ
#define IR_SAMPLING_HI_Q   MLX90640_SPEED_4_HZ

#define IR_SAMPLING_RES_LOW_Q MLX90640_RESOLUTION_18BIT
#define IR_SAMPLING_RES_HI_Q  MLX90640_RESOLUTION_18BIT // or 19 bits ?

#define IR_SAMPLING_LOW_Q_TEXT "8x"
#define IR_SAMPLING_HI_Q_TEXT  "4x"

// ----------------------------------------------------------------------
//#define IR_SENSOR_SD_THERMOGRAM_FILE  "%s/%05u.thc"
//#define IR_SENSOR_SD_THERMOGRAMS_PATH "/mlx"
//#define IR_SENSOR_SD_THERMOGRAM_PATH  "/sdcard/mlx/%04u" 


// ----------------------------------------------------------------------
// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     30


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

#define THC_FRAME_DATA_VER     2

// how much frames should be taken before STILL will be switche to MOV
#define THC_MAX_FRAMES_FOR_MOV 4

enum {
  THC_FRAME_DATA_TYPE_CAL = 1,  // calibration
  THC_FRAME_DATA_TYPE_STILL,    // single frame
  THC_FRAME_DATA_TYPE_MOV       // video (gif)
};

typedef struct {
  union {
    uint32_t ul_marker;
    uint8_t uc_marker[4];
  };
  uint32_t ul_version;
  uint32_t ul_size;
  uint32_t ul_type;
  uint32_t ul_reserved[12];
  //uint32_t ul_num_frames;

  uint32_t ul_cal_data_size;
  uint32_t ul_frame_data_size;
} thc_frame_header_t;

typedef struct {
  thc_frame_header_t x_header;

  uint8_t *puc_cal_data;
  uint8_t *puc_frame_data;
} thc_frame_t;

// ----------------------------------------------------------------------
typedef void (*btn_poller_callabck_t) (BaseType_t xBtnState);

typedef struct {
  uint32_t ulPollTimeout;
  uint32_t ulLastPollTimeout;
  
  int32_t ilBtn;
  //int32_t ilBtnReadState;

  BaseType_t xCurState;
  BaseType_t xPrevState;

  btn_poller_callabck_t pvfxCallback;
} btn_poller_t;


// ----------------------------------------------------------------------
extern Adafruit_ST7735 tft;

extern uint8_t ucFrameBuffer[IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H];

extern uint16_t usPaletteColors[IR_CAM_MAX_COLORS];


extern int16_t sSdBufTest[32 * 24];

extern btn_poller_t xBtns[1];

// ----------------------------------------------------------------------
extern Grid_t xGrid;

extern paramsMLX90640 x_mlx90640;
extern IrCamDataFrame_t x_mlx90640Frame;
extern float fMLX90640Oversampling[IR_ADC_OVERSAMPLING_COUNT][IR_SENSOR_DATA_FRAME_SIZE];

extern uint8_t ucBootProgress;
extern BaseType_t xIsSDCardFail;

// ----------------------------------------------------------------------
extern Task <4096>AppMainTask;
extern Task <2048>GetFrameDataTask;
//extern Task <2048>BtnPollerTask;

//extern Timer TakeScreenShotTimer;

extern Mutex MLX90640Mutex;

extern Counter <IR_ADC_OVERSAMPLING_COUNT> mlx90640FrameRdyCounter;
extern Counter <1> ScreenShootRdyCounter;

// ----------------------------------------------------------------------
void vAppMainTask(void *pvArg);
void vGetFrameDataTask(void *pvArg);
void vBtnPollerTask(void *pvArg);

void vTakeScreenShoot(void *pvArg);

// ----------------------------------------------------------------------
void vGridInit(void);
void vSDInit(void);
void v_init_thc_struct(void);
void v_thc_save_cal(void);
void v_thc_check_frame_type(void);

void vGridSetPaletteType(uint32_t ulPaletteType);
void vGridPlace(int px, int py, int w, int h);
void vGridSetColorMode(int mode);
void vGridMakeAvg(void);
void vGridFindMaxMinAvg(float f_high, float f_low);
void vGridInterpolate(void);

// ----------------------------------------------------------------------
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255);

void vPrintAt(uint32_t ulPosX, uint32_t ulPosY, const char *pucText);
void vPrintAt(uint32_t ulPosX, uint32_t ulPosY, float fVal);

void vStartColdReadings(IrCamDataFrame_t *pxIrCamDataFrame);

void vDrawLogo(void);
void vDrawProgressBar(uint32_t ulState);
void vDrawInterpolated(void);
void vDrawMeasurement(void);

void vPrintSDStats(void);


// ----------------------------------------------------------------------
void vTakeSreenShotFlag(BaseType_t xBtnState);
void vtakeVideoFlag(BaseType_t xBtnState);

// ----------------------------------------------------------------------
void start_wi_fi(void);
void initWebServer(void);

// ----------------------------------------------------------------------
void vMLX90640_EnableHiQualityMode(BaseType_t xEnable);
void vMLX90640_GetDataFrame(IrCamDataFrame_t *pxIrCamDataFrame);


#endif // _COMMON_H
