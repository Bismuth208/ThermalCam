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

paramsMLX90640 mlx90640;
IrCamDataFrame_t x_mlx90640Frame;
float fMLX90640Oversampling[IR_ADC_OVERSAMPLING_COUNT][IR_SENSOR_DATA_FRAME_SIZE];

uint8_t ucBootProgress = 0;
BaseType_t xIsSDCardFail = pdFALSE;

// ----------------------------------------------------------------------
Task <4096>AppMainTask((TaskFunction_t) vAppMainTask);
Task <2048>GetFrameDataTask((TaskFunction_t) vGetFrameDataTask);

//Timer TakeScreenShotTimer((TimerCallbackFunction_t) vTakeScreenShoot);

Mutex MLX90640Mutex;

Counter <IR_ADC_OVERSAMPLING_COUNT> mlx90640FrameRdyCounter;
Counter <1> ScreenShootRdyCounter;
