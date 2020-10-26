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
    vMLX90640_GetDataFrame(pxIrCamDataFrame);
    mlx90640FrameRdyCounter.give();

    vTaskDelay(pdMS_TO_TICKS(1));
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
      MLX90640Mutex.lock();

      // now, calculate and draw everything
      vGridMakeAvg();
      vGridDrawInterpolated();
      vDrawMeasurement();

      MLX90640Mutex.unlock();


      if (xHiResBtnState == pdFALSE) { // low or aka pressed
        if ((millis() - ulLastScreenShot) > 125) { // 8 frames per sec
          vTakeScreenShoot(NULL);
  //        TakeScreenShotTimer.start(5000);
          
          ulLastScreenShot = millis();
  
          xNeedDelay = pdFALSE;
        }
      }
    }

    if (xHiResBtnState != xGrid.xHiPrecisionModeIsEn) {
      //vMLX90640_EnableHiQualityMode(xHiResBtnState);
    }
    
    if (xNeedDelay == pdTRUE) {
     vTaskDelay(pdMS_TO_TICKS(25));
    }
  }
}
