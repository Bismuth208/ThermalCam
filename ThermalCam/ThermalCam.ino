//*******************************************************
//  Quick and dirty thermal imager code
//  Runs on ESP32 (LOLIN32), uses 128x128 OLED screen
//  from adafruit:
//  https://learn.adafruit.com/adafruit-1-5-color-oled-breakout-board/downloads-and-links
//  and MLX90640 thermal imager chip.
//
//  CODE IS A MESS!!! Only use for reference
//  MOST MENUS DOES NOTHING!
//  (F) DZL 2016
//*******************************************************

#define TFT_CS   4  // goes to TFT CS
#define TFT_DC   5  // goes to TFT DC

// for soft SPI
//#define TFT_MOSI 13  // goes to TFT MOSI
//#define TFT_CLK  14  // goes to TFT SCK/CLK
//#define TFT_RST  26   // ESP RST to TFT RESET
//#define TFT_MISO 12

// ----------------------------------------------------------------------

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     30

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN            32

// ----------------------------------------------------------------------

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
//#include <Adafruit_ILI9341.h>
#include <Adafruit_ST7735.h>

#include <MLX90640_API.h>
#include <MLX90640_I2C_Driver.h>

#include "FreeRTOS_helper.h"
#include "common.h"

#include "Duck_logo_Iron_Gradient.h"


// ----------------------------------------------------------------------

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640
#define TA_SHIFT 8 //Default shift for MLX90640 in open air

#define IR_CAM_DATA_FRAMES 2

// ----------------------------------------------------------------------

void vAppMainTask(void *pvArg);
void vGetFrameDataTask(void *pvArg);

// ----------------------------------------------------------------------

float mlx90640To[IR_SENSOR_DATA_FRAME_SIZE];
float fMLX90640Oversampling[IR_ADC_OVERSAMPLING_COUNT][IR_SENSOR_DATA_FRAME_SIZE];
uint32_t ulOversamplingPos = 0;

paramsMLX90640 mlx90640;

uint8_t ucBootProgress = 0;

//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, -1 /*, TFT_MOSI, TFT_CLK , TFT_RST, TFT_MISO*/);

// ----------------------------------------------------------------------
Task AppMainTask(vAppMainTask, 2048, 1, 0);
Task GetFrameDataTask(vGetFrameDataTask, 2048, 1, 1); // on WiFi/Network core does not work...

Queue <IR_CAM_DATA_FRAMES, IrCamDataFrame_t*> mlx90640FrameInOutQueue; // from global to decoder
Queue <IR_CAM_DATA_FRAMES, IrCamDataFrame_t*> mlx90640FrameOutInQueue; // from decoder to printer task


// ----------------------------------------------------------------------
// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void vPrintAt(uint32_t ulPosX, uint32_t ulPosY, const char *pucText)
{
  tft.setCursor(ulPosX, ulPosY);
  tft.print(pucText);
}

void vPrintAt(uint32_t ulPosX, uint32_t ulPosY, float fVal)
{
  tft.setCursor(ulPosX, ulPosY);
  tft.printf("%.2f", fVal);
}

// A bit 
void vDrawLogo(void)
{
#if 1
  uint16_t usColor = 0;
  const uint8_t *pucPic = &Duck_logo_Iron_Gradient_data[0];

  tft.startWrite();
  tft.setAddrWindow(105, 35, DUCK_LOGO_W, DUCK_LOGO_H);
  tft.endWrite();
  
  for (uint32_t i = 0; i < DUCK_LOGO_SIZE; i += 3) {
    usColor = tft.color565(*(pucPic + i), *(pucPic + i + 1), *(pucPic + i + 2));
    
    tft.pushColor(usColor);
  }
#else
  tft.drawRGBBitmap(105, 35, &Duck_logo_Iron_Gradient_data[0], DUCK_LOGO_W, DUCK_LOGO_H);
#endif
}


// ----------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);
  btStop();
  
  // ----------------------------------------------------------------------
  // Setup timer and attach timer to a led pin for TFT backlight
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
  ledcAnalogWrite(LEDC_CHANNEL_0, 255);
  
  // ----------------------------------------------------------------------
  // Init TFT chip
//  tft.begin();
  tft.initR(INITR_BLACKTAB);
  
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);

  // ----------------------------------------------------------------------
  // Boot logo...
  tft.drawRect(1, 1, tft.width()-2, tft.height()-2, ST7735_WHITE);
//  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  vPrintAt(20, 50, "Maaagic");
  vPrintAt(46, 68, "Thermal");

  tft.setTextSize(1);

  vPrintAt(100, 90, VERSION_STR); // version build

  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;

  vDrawLogo();
  
  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;
  
  Wire.begin(21, 15, 400000); //SDA, SCL and increase I2C clock speed to 400kHz
  
  /*
    if (isConnected() == false)
    {
      Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
      while (1);
    }
  */
  //Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.printf("Failed to load system parameters %d \n", status);

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.printf("Parameter extraction failed %d \n", status);

  // Once params are extracted
  // we can release eeMLX90640 array and we can increase to 1MHz
  Wire.setClock(1000000);

  MLX90640_SetRefreshRate(MLX90640_address, MLX90640_SPEED_4_HZ);
  MLX90640_SetResolution(MLX90640_address, MLX90640_RESOLUTION_19BIT);
//  Serial.printf("Cur res %d\n", MLX90640_GetCurResolution(MLX90640_address));

  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;
  
  vGridInit();
  vGridSetPaletteType(IR_PALETTE_TYPE_IRONBOW);

  // ----------------------------------------------------------------------
  // create memory pool for faster data frames from camera
  IrCamDataFrame_t *pxIrCamDataFrame = NULL;
  for (uint32_t i = 0; i < IR_CAM_DATA_FRAMES; i++) {
    pxIrCamDataFrame = (IrCamDataFrame_t *) malloc(sizeof(IrCamDataFrame_t));
    mlx90640FrameInOutQueue.send(&pxIrCamDataFrame);
  }

  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;
  
  // start everything now safely
  GetFrameDataTask.emitSignal();
}

void vDrawMeasurement(void)
{
  tft.drawCircle(80, 50, 4, ST7735_WHITE);

  vPrintAt(5, 102, "L");
  vPrintAt(12, 102, xGrid.fLow);

  vPrintAt(52, 102, "H");
  vPrintAt(59, 102, xGrid.fHigh);

  vPrintAt(100, 102, "C");
  vPrintAt(107, 102, xGrid.fAvgCenter);

  vPrintAt(5, 116, "E");
  vPrintAt(12, 116, xGrid.fEmissivity);
}

// Immitation of serious work
void vDrawProgressBar(uint32_t ulState)
{
  tft.drawRect(20, 100, GUI_PROGRESS_BAR_STEP_SIZE * (ulState + 1), 8, ST7735_WHITE);
}

void vStartColdReadings(IrCamDataFrame_t *pxIrCamDataFrame)
{
  float tr = -8; //Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
  
    // 40ms + max value for 4Hz (to be sure)
//  vTaskDelay(pdMS_TO_TICKS(40));

  mlx90640FrameInOutQueue.receive(&pxIrCamDataFrame);
  for (int i = ucBootProgress; i < IR_SENSOR_COLD_READS_NUM; i++) {
    MLX90640_GetFrameData(MLX90640_address, &pxIrCamDataFrame->mlx90640Frame[0]);
    MLX90640_CalculateTo(&pxIrCamDataFrame->mlx90640Frame[0], &mlx90640, xGrid.fEmissivity, tr, &fMLX90640Oversampling[ulOversamplingPos][0]);

    ++ulOversamplingPos;
#if 1
    if (ulOversamplingPos > IR_ADC_OVERSAMPLING_COUNT) {
      ulOversamplingPos = 0;
    }
#endif

    vGridMakeAvg();

    vDrawProgressBar(i);
  }
  mlx90640FrameInOutQueue.send(&pxIrCamDataFrame);

  // FIXME: remove this one by oversamplng read
  vTaskDelay(pdMS_TO_TICKS(GUI_BOOT_LOGO_DELAY)); // yeees...
}

void vGetFrameDataTask(void *pvArg)
{
  (void) pvArg;

  IrCamDataFrame_t *pxIrCamDataFrame = NULL;

  // wait for a start
  while(GetFrameDataTask.waitSignal() == 0);

  // prepare data by cold read
  vStartColdReadings(pxIrCamDataFrame);

  // remove progress bar
  tft.fillRect(20, 100, GUI_PROGRESS_BAR_STEP_SIZE * (IR_SENSOR_COLD_READS_NUM + 1), 8, ST7735_BLACK);
  // draw frame for thermogramm
  tft.drawRect(xGrid.ulScreenX-1, xGrid.ulScreenY-1, IR_SENSOR_MATRIX_2W * 2 + 4, IR_SENSOR_MATRIX_2H * 2 + 2, ST7735_WHITE);

  for (;;) {
    while (mlx90640FrameInOutQueue.receive(&pxIrCamDataFrame) == pdFALSE);
    
    MLX90640_GetFrameData(MLX90640_address, &pxIrCamDataFrame->mlx90640Frame[0]);

    while (mlx90640FrameOutInQueue.send(&pxIrCamDataFrame) == pdFALSE);

    vTaskDelay(1);
  }
}

void vAppMainTask(void *pvArg)
{
  (void) pvArg;

  float vdd = 0;
  float Ta = 0;
  float tr = -8; //Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature

  IrCamDataFrame_t *pxIrCamDataFrame = NULL;

  for (;;) {
    if (!mlx90640FrameOutInQueue.isEmpty())
    {
      mlx90640FrameOutInQueue.receive(&pxIrCamDataFrame); // get new frame
 
  #if 0
      vdd = MLX90640_GetVdd(&pxIrCamDataFrame->mlx90640Frame[0], &mlx90640);
      Ta = MLX90640_GetTa(&pxIrCamDataFrame->mlx90640Frame[0], &mlx90640);
      tr = Ta - TA_SHIFT;
  #endif

      MLX90640_CalculateTo(&pxIrCamDataFrame->mlx90640Frame[0], &mlx90640, xGrid.fEmissivity, tr, &fMLX90640Oversampling[ulOversamplingPos][0]);

      while (mlx90640FrameInOutQueue.send(&pxIrCamDataFrame) == pdFALSE); // return memory pointer back to decoder

//      MLX90640_BadPixelsCorrection(&mlx90640.brokenPixels[0], &fMLX90640Oversampling[ulOversamplingPos][0], 1, &mlx90640);
//      MLX90640_BadPixelsCorrection(&mlx90640.outlierPixels[0], &fMLX90640Oversampling[ulOversamplingPos][0], 1, &mlx90640);

      ++ulOversamplingPos;
      if (ulOversamplingPos > IR_ADC_OVERSAMPLING_COUNT) {
        ulOversamplingPos = 0;
      }

      // now, calculate and deaw everything
      vGridMakeAvg();
      vGridFindMinMax();
      vGridDrawInterpolated();
      vDrawMeasurement();
    }

    vTaskDelay(1);
  }
}

// this poor function not used at all
void loop()
{
  taskYIELD();
}
