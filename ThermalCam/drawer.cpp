// ----------------------------------------------------------------------
#include "common.h"
#include "common_pics.h"
#include "ir_sensor.h"
#include "pins_definitions.h"

// ----------------------------------------------------------------------
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
  tft.setAddrWindow(DUCK_LOGO_POS_X, DUCK_LOGO_POS_Y, DUCK_LOGO_W, DUCK_LOGO_H);
  tft.endWrite();
  
  for (uint32_t i = 0; i < DUCK_LOGO_SIZE; i += 3) {
    usColor = tft.color565(*(pucPic + i), *(pucPic + i + 1), *(pucPic + i + 2));
    
    tft.pushColor(usColor);
  }
#else
  tft.drawRGBBitmap(DUCK_LOGO_POS_X, DUCK_LOGO_POS_Y, &Duck_logo_Iron_Gradient_data[0], DUCK_LOGO_W, DUCK_LOGO_H);
#endif
}

// ----------------------------------------------------------------------
void vDrawInterpolated(void)
{
  int x = 0;
  int y = 0;
  uint16_t usColor = 0;

  tft.startWrite();

  // down frame clip
  for (int i = 0; i < (IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H - IR_SENSOR_MATRIX_2W); i++) {
    x = i & 0x0000003f;
    x = xGrid.ulScreenW - (x*2) + xGrid.ulScreenX;
    
    y = i >> 6;
    y = (y*2) + xGrid.ulScreenY;
    
    usColor = usPaletteColors[ucFrameBuffer[i]];

    // down frame clip    
//    if (i < (IR_SENSOR_MATRIX_W * IR_SENSOR_MATRIX_2H - IR_SENSOR_MATRIX_2W))
    {
      // also make horizontal flip
      //tft.fillRect(xGrid.ulScreenW - (x*2) + xGrid.ulScreenX, (y*2) + xGrid.ulScreenY, dx, dy, usPaletteColors[v]);
      tft.writeFillRectPreclipped(x, y, xGrid.usSreenDx, xGrid.usSreenDy, usColor);
    }

    // origin size; Debug sensor
//    tft.drawPixel(x + xGrid.ulScreenX, y + xGrid.ulScreenY, usPaletteColors[v]);
  }

  tft.endWrite();

#if 0
  tft.startWrite();
  tft.setAddrWindow(xGrid.ulScreenX, xGrid.ulScreenY, IR_SENSOR_MATRIX_2W, IR_SENSOR_MATRIX_2H);
  tft.writePixels(&usFrameBuffer[0], IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H);
  tft.endWrite();
#endif
}

void vDrawMeasurement(void)
{
  tft.drawCircle(80, 50, 4, COLOR_WHITE);

  vPrintAt(5, 102, "L");
  vPrintAt(12, 102, xGrid.fLow);

  vPrintAt(52, 102, "H");
  vPrintAt(59, 102, xGrid.fHigh);

  vPrintAt(100, 102, "C");
  vPrintAt(107, 102, xGrid.fAvgCenter);

  vPrintAt(5, 116, "E");
  vPrintAt(12, 116, xGrid.fEmissivity);

  vPrintAt(140, 116, (xGrid.xHiPrecisionModeIsEn == pdFALSE) ? IR_SAMPLING_HI_Q_TEXT : IR_SAMPLING_LOW_Q_TEXT);

  vPrintSDStats();
}

// Immitation of serious work
void vDrawProgressBar(uint32_t ulState)
{
  tft.drawRect(20, 100, GUI_PROGRESS_BAR_STEP_SIZE * (ulState + 1), 8, COLOR_WHITE);
}

// ----------------------------------------------------------------------
void vStartColdReadings(IrCamDataFrame_t *pxIrCamDataFrame)
{
  float tr = -8; //Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature

  for (int i = ucBootProgress; i < IR_SENSOR_COLD_READS_NUM; i++) {
    vMLX90640_GetDataFrame(pxIrCamDataFrame);
    vDrawProgressBar(i);
  }

  // FIXME: remove this one by oversamplng read
  Task<0>::delay(GUI_BOOT_LOGO_DELAY); // yeees...
}
