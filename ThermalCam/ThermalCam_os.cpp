// ----------------------------------------------------------------------

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

#include <FS.h>
#include <SD.h>

#include "common.h"
#include "thermal_cam_pins.h"

// ----------------------------------------------------------------------
void vGetFrameDataTask(void *pvArg)
{
  (void) pvArg;

  IrCamDataFrame_t *pxIrCamDataFrame = &x_mlx90640Frame;

  // wait for a start
  GetFrameDataTask.waitSignal();

  // prepare data by cold read
  vStartColdReadings(pxIrCamDataFrame);

  AppMainTask.emitSignal();

  for (;;) {
    // now, calculate safely...
    MLX90640Mutex.lock();
    vMLX90640_GetDataFrame(pxIrCamDataFrame);
    
    vGridMakeAvg();
    vGridInterpolate();
    MLX90640Mutex.unlock();

     // new frame ready, so, process it in another task
    mlx90640FrameRdyCounter.give();

    Task<0>::delay(1);
  }
}

// ----------------------------------------------------------------------
void vAppMainTask(void *pvArg)
{
  (void) pvArg;

  BaseType_t xHiResBtnState = pdFALSE;
  BaseType_t xNeedDelay = pdFALSE;     // compensate SD write time to get next MLX90640 frame faster

  AppMainTask.waitSignal();

  // remove progress bar
  tft.fillRect(20, 100, GUI_PROGRESS_BAR_STEP_SIZE * (IR_SENSOR_COLD_READS_NUM + 1), 8, ST7735_BLACK);
  // draw frame for thermogramm
  tft.drawRect(xGrid.ulScreenX, xGrid.ulScreenY, IR_SENSOR_MATRIX_2W * 2 + 4, IR_SENSOR_MATRIX_2H * 2, ST7735_WHITE);

  uint32_t ulLastScreenShot = 0;
  
  for (;;) {
    xHiResBtnState = (BaseType_t) digitalRead(OPT_KEY1_PIN);
    xNeedDelay = pdTRUE; // by default yes, othervice overwrite
    
    if (mlx90640FrameRdyCounter.take(1)) {
      // now, draw everything
      MLX90640Mutex.lock();
      
      vDrawInterpolated();
      vDrawMeasurement();

      if (xHiResBtnState == pdFALSE) { // low or aka pressed
        if ((millis() - ulLastScreenShot) > 100) { // 8 frames per sec
          vTakeScreenShoot(NULL);
  //        TakeScreenShotTimer.start(5000);
          
          ulLastScreenShot = millis();
  
          xNeedDelay = pdFALSE;
        }
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
