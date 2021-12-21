// ----------------------------------------------------------------------

#include <WiFi.h>
#include <SPI.h>

#include <FS.h>
#include <SD.h>

#include <SPIFFS.h>

#include "common.h"
#include "ir_sensor.h"
#include "sd_writer.h"
#include "pins_definitions.h"

extern uint32_t ul_screenshots_taken;

// ----------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  
  // ----------------------------------------------------------------------
  // Setup timer and attach timer to a led pin for TFT backlight
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
  ledcAnalogWrite(LEDC_CHANNEL_0, 255);

  // ----------------------------------------------------------------------
  pinMode(OPT_KEY1_PIN, INPUT_PULLUP);
//  pinMode(OPT_KEY2_PIN, INPUT_PULLUP);
//  pinMode(OPT_KEY3_PIN, INPUT_PULLUP);
  
  // ----------------------------------------------------------------------
  // Init TFT chip
#ifdef _ADAFRUIT_ILI9341H_
  tft.begin();
#else
  tft.initR(INITR_BLACKTAB);
#endif
  
  tft.fillScreen(COLOR_BLACK);
  tft.setRotation(1);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);

  // ----------------------------------------------------------------------
  // Boot logo...
  tft.drawRect(1, 1, tft.width()-2, tft.height()-2, COLOR_WHITE);

  vDrawLogo();
  
  tft.setTextSize(2);
  vPrintAt(15, 50, "ThermalCam");
  
  tft.setTextSize(1);
  vPrintAt(100, 90, VERSION_STR); // version build

  // ----------------------------------------------------------------------
  // check for OTA
  if (digitalRead(OPT_KEY1_PIN) == LOW) {
    AppMainTask.stop();
    GetFrameDataTask.stop();

    btStop();

    start_wi_fi();
    initWebServer();

    // show to user what ThermalCam in OTA Mode
    vPrintAt(20, 90, OTA_STR);
  } else {
    WiFi.mode(WIFI_OFF);
    btStop();

    //vPrintAt(20, 100, ESP.getSdkVersion());

    normal_init();
  }
}

// this poor function not used at all
void loop()
{  
  taskYIELD();
}

// ----------------------------------------------------------------------
void normal_init(void)
{
  // ----------------------------------------------------------------------
  vSDInit();
  v_init_thc_struct();
  
  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;
  
  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;

  vInitSensor();

  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;
  
  vGridInit();
  vGridSetPaletteType(IR_PALETTE_TYPE_IRONBOW);

  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;
  
  // start everything now safely
  GetFrameDataTask.emitSignal();

  Serial.print("Ok!\n");
}

// ----------------------------------------------------------------------
// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax)
{
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}


// ----------------------------------------------------------------------
void vAppMainTask(void *pvArg)
{
  (void) pvArg;

  BaseType_t xHiResBtnState = pdFALSE;
  // Compensate SD write time to get next MLX90640 frame faster
  // By default pdTRUE
  BaseType_t xNeedDelay = pdTRUE;

  // Wait here untill MLX sensor wiil be initialised
  AppMainTask.waitSignal();

  // Remove progress bar
  tft.fillRect(20, 100, GUI_PROGRESS_BAR_STEP_SIZE * (IR_SENSOR_COLD_READS_NUM + 1), 8, COLOR_BLACK);
  // Draw frame to split thermal image visually from rest of UI
  tft.drawRect(xGrid.ulScreenX, xGrid.ulScreenY, IR_SENSOR_MATRIX_2W * 2 + 4, IR_SENSOR_MATRIX_2H * 2, COLOR_WHITE);

  uint32_t ulLastScreenShot = 0;
  
  for (;;) {
    xHiResBtnState = (BaseType_t) digitalRead(OPT_KEY1_PIN);
    
    if (mlx90640FrameRdyCounter.take(1)) {
      // Draw everything
      MLX90640Mutex.lock();
      
      vDrawInterpolated();
      vDrawMeasurement();

      if (xHiResBtnState == pdFALSE) { // low or aka pressed
        if ((millis() - ulLastScreenShot) > 100) { // 8 frames per sec
          ++ul_screenshots_taken;
          v_thc_check_frame_type();
          
          vTakeScreenShoot(NULL);
  //        TakeScreenShotTimer.start(5000);
          
          ulLastScreenShot = millis();
          xNeedDelay = pdFALSE;
        }
      } else {
        ul_screenshots_taken = 0;
        v_thc_check_frame_type();
      }

      if (xHiResBtnState != xGrid.xHiPrecisionModeIsEn) {
        vMLX90640_EnableHiQualityMode(xHiResBtnState);
      }

      MLX90640Mutex.unlock();
    }
    
    if (xNeedDelay == pdTRUE) {
      Task<0>::delay(25);
    } else {
      Task<0>::delay(1);
    }
  }
}

void vBtnPollerTask(void *pvArg)
{
  (void) pvArg;

  for (;;) {
    for (auto &xBtn : xBtns) {
     if ((millis() - xBtn.ulLastPollTimeout) >= xBtn.ulPollTimeout) {
        xBtn.xCurState = (BaseType_t) digitalRead(xBtn.ilBtn);

        if (xBtn.xCurState != xBtn.xPrevState) {
          xBtn.xPrevState = xBtn.xCurState;
        } else {
          if (xBtn.pvfxCallback != NULL) {
            xBtn.pvfxCallback(xBtn.xCurState);
            xBtn.ulLastPollTimeout = millis();
          }
        }
      } 
    }

    Task<0>::delay(1);
  }
}
