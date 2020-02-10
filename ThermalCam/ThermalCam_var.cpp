// ----------------------------------------------------------------------

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

#include "common.h"
#include "thermal_cam_pins.h"


// ----------------------------------------------------------------------
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, -1 /*, TFT_MOSI, TFT_CLK , TFT_RST, TFT_MISO*/);

// this is buffer end interpolated picture to draw on screen o lat faset
// and save as BMP to SD with minimum overhead
uint16_t usFrameBuffer[IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H];

// ----------------------------------------------------------------------
Grid_t xGrid;

paramsMLX90640 mlx90640;
IrCamDataFrame_t x_mlx90640Frame;
float fMLX90640Oversampling[IR_ADC_OVERSAMPLING_COUNT][IR_SENSOR_DATA_FRAME_SIZE];

uint8_t ucBootProgress = 0;

// ----------------------------------------------------------------------
Task AppMainTask(vAppMainTask, 2048);
Task GetFrameDataTask(vGetFrameDataTask, 2048);

Mutex MLX90640Mutex;

Counter <IR_ADC_OVERSAMPLING_COUNT> mlx90640FrameRdyCounter;
