// ----------------------------------------------------------------------

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

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

  AppMainTask.waitSignal();

    // remove progress bar
  tft.fillRect(20, 100, GUI_PROGRESS_BAR_STEP_SIZE * (IR_SENSOR_COLD_READS_NUM + 1), 8, ST7735_BLACK);
  // draw frame for thermogramm
  tft.drawRect(xGrid.ulScreenX, xGrid.ulScreenY, IR_SENSOR_MATRIX_2W * 2 + 4, IR_SENSOR_MATRIX_2H * 2, ST7735_WHITE);
  
  for (;;) {
    if (mlx90640FrameRdyCounter.take()) {
      MLX90640Mutex.lock();

      // now, calculate and draw everything
      vGridMakeAvg();
      vGridDrawInterpolated();
      vDrawMeasurement();

      MLX90640Mutex.unlock();
    }

    // now check hi-res button
    xHiResBtnState = (BaseType_t) digitalRead(HI_PRECISION_BTN_PIN);

    if (xHiResBtnState != xGrid.xHiPrecisionModeIsEn) {
      vMLX90640_EnableHiQualityMode(xHiResBtnState);
    }

    vTaskDelay(pdMS_TO_TICKS(25));
  }
}
