// ----------------------------------------------------------------------

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

#include "common.h"
#include "thermal_cam_pins.h"

#include "Duck_logo_Iron_Gradient.h"


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

void vPrintAt(uint32_t ulPosX, uint32_t ulPosY, const char *pucText)
{
  tft.setCursor(ulPosX, ulPosY);
  tft.print(pucText);
}

void vPrintAt(uint32_t ulPosX, uint32_t ulPosY, float fVal)
{
  tft.setCursor(ulPosX, ulPosY);
  tft.printf("%3.2f", fVal);
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
void vMLX90640_EnableHiQualityMode(BaseType_t xEnable)
{
  MLX90640Mutex.lock();
  
  MLX90640_SetRefreshRate(IR_SENSOR_I2C_ADDR, (xEnable == pdTRUE) ? IR_SAMPLING_HI_Q : IR_SAMPLING_LOW_Q);
  MLX90640_SetResolution(IR_SENSOR_I2C_ADDR, (xEnable == pdTRUE) ? IR_SAMPLING_RES_HI_Q : IR_SAMPLING_RES_LOW_Q);

  xGrid.xHiPrecisionModeIsEn = xEnable;

  MLX90640Mutex.unlock();
}

void vMLX90640_GetDataFrame(IrCamDataFrame_t *pxIrCamDataFrame)
{
  //  float vdd = 0;
  float Ta = 0;
  float tr = -8; //Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
  
  MLX90640Mutex.lock();

  for (uint32_t i = 0; i < IR_CAM_DATA_FRAMES; i++) { // read both subpages
    MLX90640_GetFrameData(IR_SENSOR_I2C_ADDR, &pxIrCamDataFrame->mlx90640Frame[0]);

//      vdd = MLX90640_GetVdd(&pxIrCamDataFrame->mlx90640Frame[0], &mlx90640);
    Ta = MLX90640_GetTa(&pxIrCamDataFrame->mlx90640Frame[0], &mlx90640);
    tr = Ta - TA_SHIFT;

//      MLX90640_BadPixelsCorrection(&mlx90640.brokenPixels[0], &fMLX90640Oversampling[xGrid.ulOversamplingPos][0], 1, &mlx90640);
//      MLX90640_BadPixelsCorrection(&mlx90640.outlierPixels[0], &fMLX90640Oversampling[xGrid.ulOversamplingPos][0], 1, &mlx90640);

    MLX90640_CalculateTo(&pxIrCamDataFrame->mlx90640Frame[0], &mlx90640, xGrid.fEmissivity, tr, &fMLX90640Oversampling[xGrid.ulOversamplingPos][0]);
  }

  // now move ONLY here to read subpages propertly
  ++( xGrid.ulOversamplingPos );
  if (xGrid.ulOversamplingPos >= IR_ADC_OVERSAMPLING_COUNT) {
    xGrid.ulOversamplingPos = 0;
  }
  
  MLX90640Mutex.unlock();
}

// ----------------------------------------------------------------------
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

  vPrintAt(140, 116, (xGrid.xHiPrecisionModeIsEn == pdTRUE) ? IR_SAMPLING_HI_Q_TEXT : IR_SAMPLING_LOW_Q_TEXT);
}

// Immitation of serious work
void vDrawProgressBar(uint32_t ulState)
{
  tft.drawRect(20, 100, GUI_PROGRESS_BAR_STEP_SIZE * (ulState + 1), 8, ST7735_WHITE);
}

void vStartColdReadings(IrCamDataFrame_t *pxIrCamDataFrame)
{
  float tr = -8; //Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature

  for (int i = ucBootProgress; i < IR_SENSOR_COLD_READS_NUM; i++) {
    vMLX90640_GetDataFrame(pxIrCamDataFrame);
    vDrawProgressBar(i);
  }

  // FIXME: remove this one by oversamplng read
  vTaskDelay(pdMS_TO_TICKS(GUI_BOOT_LOGO_DELAY)); // yeees...
}
