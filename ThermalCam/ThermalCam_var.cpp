// ----------------------------------------------------------------------

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

#include "common.h"
#include "thermal_cam_pins.h"


// ----------------------------------------------------------------------
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, -1 /*, TFT_MOSI_PIN, TFT_CLK_PIN, TFT_RST_PIN, TFT_MISO_PIN*/);

// ----------------------------------------------------------------------
Grid_t xGrid;

paramsMLX90640 x_mlx90640;
IrCamDataFrame_t x_mlx90640Frame;
float fMLX90640Oversampling[IR_ADC_OVERSAMPLING_COUNT][IR_SENSOR_DATA_FRAME_SIZE];

uint8_t ucBootProgress = 0;
BaseType_t xIsSDCardFail = pdFALSE;

uint32_t ul_screenshots_taken = 0;

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
Task <2048>GetFrameDataTask(vGetFrameDataTask);
//Task <2048>BtnPollerTask(vBtnPollerTask);

//Timer TakeScreenShotTimer(vTakeScreenShoot);

Mutex MLX90640Mutex;

Counter <IR_ADC_OVERSAMPLING_COUNT> mlx90640FrameRdyCounter;
Counter <1> ScreenShootRdyCounter;
