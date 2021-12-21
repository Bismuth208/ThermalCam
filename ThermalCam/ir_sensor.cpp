// ----------------------------------------------------------------------
#include <Wire.h>

#include "common.h"
#include "sd_writer.h"
#include "pins_definitions.h"

#include "ir_sensor.h"

// ----------------------------------------------------------------------
Grid_t xGrid;

paramsMLX90640 x_mlx90640;
IrCamDataFrame_t x_mlx90640Frame;
float fMLX90640Oversampling[IR_ADC_OVERSAMPLING_COUNT][IR_SENSOR_DATA_FRAME_SIZE];

Mutex MLX90640Mutex;
Counter <IR_ADC_OVERSAMPLING_COUNT> mlx90640FrameRdyCounter;
Task <2048>GetFrameDataTask(vGetFrameDataTask, OS_MCU_CORE_1);

// ----------------------------------------------------------------------
void vInitSensor(void)
{
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000); //SDA, SCL and increase I2C clock speed to 400kHz
  
  /*
    if (isConnected() == false)
    {
      Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
      while (1);
    }
  */
  //Get device parameters - We only have to do this once
  int status;
  
  status = MLX90640_DumpEE(IR_SENSOR_I2C_ADDR, &x_mlx90640Frame.mlx90640Frame[0]);
  if (status != 0) {
    Serial.printf("Failed to load system parameters %d \n", status);
  }

  status = MLX90640_ExtractParameters(&x_mlx90640Frame.mlx90640Frame[0], &x_mlx90640);
  if (status != 0) {
    Serial.printf("Parameter extraction failed %d \n", status);
  }

  // Once params are extracted
  // we can release eeMLX90640 array and we can increase to 800kHz
  Wire.setClock(800000);

  v_thc_save_cal();

  vMLX90640_EnableHiQualityMode(pdTRUE);
//  Serial.printf("Cur res %d\n", MLX90640_GetCurResolution(IR_SENSOR_I2C_ADDR));
}

// ----------------------------------------------------------------------
void vGetFrameDataTask(void *pvArg)
{
  (void) pvArg;

  IrCamDataFrame_t *pxIrCamDataFrame = &x_mlx90640Frame;

  // Stuck here untill initialisation is done
  GetFrameDataTask.waitSignal();

  // Perform "cold" data readings
  vStartColdReadings(pxIrCamDataFrame);

  // Now safely unlock main thread
  AppMainTask.emitSignal();

  for (;;) {
    MLX90640Mutex.lock();
    vMLX90640_GetDataFrame(pxIrCamDataFrame);
    
    vGridMakeAvg();
    vGridInterpolate();
    MLX90640Mutex.unlock();

     // New frame is ready, so, process it in another task
    mlx90640FrameRdyCounter.give();

    Task<0>::delay(1);
  }
}

// ----------------------------------------------------------------------
// TODO: make this function scallable with more than 2 settings
void vMLX90640_EnableHiQualityMode(BaseType_t xEnable)
{
  if (xEnable == pdFALSE) {
    MLX90640_SetRefreshRate(IR_SENSOR_I2C_ADDR, IR_SAMPLING_HI_Q);
    MLX90640_SetResolution(IR_SENSOR_I2C_ADDR, IR_SAMPLING_RES_HI_Q);
  } else {
    MLX90640_SetRefreshRate(IR_SENSOR_I2C_ADDR, IR_SAMPLING_LOW_Q);
    MLX90640_SetResolution(IR_SENSOR_I2C_ADDR, IR_SAMPLING_RES_LOW_Q);
  }

  xGrid.xHiPrecisionModeIsEn = xEnable;
}

void vMLX90640_GetDataFrame(IrCamDataFrame_t *pxIrCamDataFrame)
{
  //  float vdd = 0;
  float Ta = 0;
  // Reflected temperature based on the sensor ambient temperature
  float tr = -8; //Ta - TA_SHIFT;
  
  for (uint32_t i = 0; i < IR_CAM_DATA_FRAMES; i++) { // read both subpages
    MLX90640_GetFrameData(IR_SENSOR_I2C_ADDR, &pxIrCamDataFrame->mlx90640Frame[0]);

//      vdd = MLX90640_GetVdd(&pxIrCamDataFrame->mlx90640Frame[0], &x_mlx90640);
    Ta = MLX90640_GetTa(&pxIrCamDataFrame->mlx90640Frame[0], &x_mlx90640);
    tr = Ta - TA_SHIFT;

//      MLX90640_BadPixelsCorrection(&mlx90640.brokenPixels[0], &fMLX90640Oversampling[xGrid.ulOversamplingPos][0], 1, &x_mlx90640);
//      MLX90640_BadPixelsCorrection(&mlx90640.outlierPixels[0], &fMLX90640Oversampling[xGrid.ulOversamplingPos][0], 1, &x_mlx90640);

    MLX90640_CalculateTo(&pxIrCamDataFrame->mlx90640Frame[0], &x_mlx90640, xGrid.fEmissivity, tr, &fMLX90640Oversampling[xGrid.ulOversamplingPos][0]);
  }

  // Move ONLY here to read subpages propertly
  ++( xGrid.ulOversamplingPos );
  if (xGrid.ulOversamplingPos >= IR_ADC_OVERSAMPLING_COUNT) {
    xGrid.ulOversamplingPos = 0;
  }
}
