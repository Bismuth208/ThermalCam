#ifndef _COMMON_H
#define _COMMON_H

// ----------------------------------------------------------------------
#define VERSION_STR "v0.5"
#define OTA_STR     "OTA"

// ----------------------------------------------------------------------
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
//#include <Adafruit_ST7735.h>

#include <FreeRTOS_helper.hpp>


#ifdef _ADAFRUIT_ILI9341H_
#define COLOR_BLACK ILI9341_BLACK
#define COLOR_WHITE ILI9341_WHITE

extern Adafruit_ILI9341 tft;
#else
#define COLOR_BLACK ST7735_BLACK
#define COLOR_WHITE ST7735_WHITE

extern Adafruit_ST7735 tft;
#endif


// ----------------------------------------------------------------------
#define GUI_PROGRESS_BAR_STEP_SIZE 14
#define GUI_BOOT_LOGO_DELAY        (1000) // 1 sec.

// ----------------------------------------------------------------------
// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     30

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
extern btn_poller_t xBtns[1];

// ----------------------------------------------------------------------
extern uint8_t ucBootProgress;

// ----------------------------------------------------------------------
extern Task <4096>AppMainTask;
//extern Task <2048>BtnPollerTask;

// ----------------------------------------------------------------------
void vAppMainTask(void *pvArg);
void vBtnPollerTask(void *pvArg);

// ----------------------------------------------------------------------
void vGridInit(void);

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

void vDrawLogo(void);
void vDrawProgressBar(uint32_t ulState);
void vDrawInterpolated(void);
void vDrawMeasurement(void);

void vPrintSDStats(void);

// ----------------------------------------------------------------------
void start_wi_fi(void);
void initWebServer(void);


#endif // _COMMON_H
