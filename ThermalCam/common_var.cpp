// ----------------------------------------------------------------------
#include "common.h"
#include "ir_sensor.h"
#include "sd_writer.h"
#include "pins_definitions.h"

// ----------------------------------------------------------------------
#ifdef _ADAFRUIT_ILI9341H_
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN, -1);
#else
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, -1 /*, TFT_MOSI_PIN, TFT_CLK_PIN, TFT_RST_PIN, TFT_MISO_PIN*/);
#endif

// ----------------------------------------------------------------------
uint8_t ucBootProgress = 0;
BaseType_t xIsSDCardFail = pdFALSE;

uint32_t ul_screenshots_taken = 0;
uint32_t ul_mov_file_is_open = 0;

btn_poller_t xBtns[1] = {
  { 
    25, // ulPollTimeout
    0,  // ulLastPollTimeout

    OPT_KEY1_PIN,  // ilBtn

    pdFALSE,  // xCurState
    pdFALSE,  // xPrevState

    vTakeSreenShotFlag  // .pvfxCallback
  }  
};

// ----------------------------------------------------------------------
Task <4096>AppMainTask(vAppMainTask, OS_MCU_CORE_0);
//Task <2048>BtnPollerTask(vBtnPollerTask);
