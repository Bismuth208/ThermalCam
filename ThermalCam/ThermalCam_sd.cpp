// ----------------------------------------------------------------------

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

#include <FS.h>
#include <SD.h>

#include "common.h"
#include "thermal_cam_pins.h"

uint32_t ulTotalPics = 0;
uint32_t ulTotalFolders = 0;

uint8_t ucCurrentFolderPathBuff[64];

// this is buffer end interpolated picture to draw on screen o lat faset
// and save as raw to SD with minimum overhead
uint8_t ucFrameBuffer[IR_SENSOR_MATRIX_2W * IR_SENSOR_MATRIX_2H];

int16_t sSdBufTest[32 * 24];

thc_frame_t x_thc_frame;

// ----------------------------------------------------------------------
void vSDInit(void)
{
  SD.begin(TFT_SD_CS_PIN);
  
  if (!SD.begin(TFT_SD_CS_PIN)) {
    xIsSDCardFail = pdTRUE;
  } else {
    if (!SD.exists("/mlx")) {
      SD.mkdir("/mlx");
    }

    // i'm pretty sure what it never reach over9000 !!!
    sprintf((char *) &ucCurrentFolderPathBuff[0], "/mlx/%04u", ulTotalFolders);

    while (SD.exists((const char *) &ucCurrentFolderPathBuff[0])) {
      ++ulTotalFolders;
      
      sprintf((char *) &ucCurrentFolderPathBuff[0], "/mlx/%04u", ulTotalFolders);
    }

    --ulTotalFolders;
      
    sprintf((char *) &ucCurrentFolderPathBuff[0], "/mlx/%04u", ulTotalFolders);

    // now check if dir is empty
    auto dir = SD.open((const char *) &ucCurrentFolderPathBuff[0]);
    auto entry = dir.openNextFile();

    if (entry) {
      ++ulTotalFolders;
      
      sprintf((char *) &ucCurrentFolderPathBuff[0], "/mlx/%04u", ulTotalFolders);

      dir.close();
    }

    SD.mkdir((char *) &ucCurrentFolderPathBuff[0]);
  }
}

/*
 * @brief Basic init for frame data files
 * @ret none
 */
void v_init_thc_struct(void)
{
  memset(&x_thc_frame, 0xff, sizeof(x_thc_frame));
  memcpy(&x_thc_frame.x_header.uc_marker[0], "THC", sizeof("THC"));

  x_thc_frame.x_header.ul_version = THC_FRAME_DATA_VER;
  x_thc_frame.x_header.ul_type = THC_FRAME_DATA_TYPE_STILL;
  x_thc_frame.x_header.ul_cal_data_size = sizeof(paramsMLX90640);
  x_thc_frame.x_header.ul_frame_data_size = sizeof(ucFrameBuffer);

  x_thc_frame.puc_cal_data = (uint8_t*) &x_mlx90640;
  x_thc_frame.puc_frame_data = (uint8_t *) &ucFrameBuffer[0];
}

/*
 * @brief Save calibration data from MLX90640 sensor
 * @ret none
 */
void v_thc_save_cal(void)
{
  uint8_t ucBuff[64];

  if (xIsSDCardFail == pdFALSE) {
    sprintf((char*) &ucBuff[0],  "/sd%s/cal.thc", (char *) &ucCurrentFolderPathBuff[0]);

    FILE *file = fopen((char *) &ucBuff[0], "wb");

    x_thc_frame.x_header.ul_type = THC_FRAME_DATA_TYPE_CAL;

    fwrite(&x_thc_frame.x_header, 1, sizeof(x_thc_frame.x_header), file);
    fwrite(x_thc_frame.puc_cal_data, 1, x_thc_frame.x_header.ul_cal_data_size, file);
    fclose(file);
  }
}

void writeFile(const char *path, uint8_t *pucData, uint32_t ulDataSize)
{
 #if 0
  File file = SD.open(path, FILE_WRITE);
  
  if (file) {

    file.write(pucData, ulDataSize);
    file.close();
  }
#else
  FILE *file = fopen(path, "wb");

  if (file != NULL)  {
    x_thc_frame.x_header.ul_type = THC_FRAME_DATA_TYPE_STILL;
    
    fwrite(&x_thc_frame.x_header, 1, sizeof(x_thc_frame.x_header), file);
    fwrite(x_thc_frame.puc_frame_data, 1, x_thc_frame.x_header.ul_frame_data_size, file);
  }

  fclose(file);
#endif
}

void vTakeScreenShoot(void *pvArg)
{
  (void) pvArg;

  uint8_t ucBuff[64];  // FIXME: create VLA buf based on file name and path

  if (xIsSDCardFail == pdFALSE) {
    //sprintf((char *) &ucBuff[0], "%s/%05u.thc", (char *) &ucCurrentFolderPathBuff[0], ulTotalPics);
    sprintf((char *) &ucBuff[0], "/sd%s/%05u.thc", (char *) &ucCurrentFolderPathBuff[0], ulTotalPics);
    
    writeFile((const char *) &ucBuff[0], (uint8_t *) &ucFrameBuffer[0], sizeof(ucFrameBuffer));
    //writeFile((const char *) &ucBuff[0], (uint8_t *) &sSdBufTest[0], sizeof(sSdBufTest));

    ++ulTotalPics;

    if (xGrid.xHiPrecisionModeIsEn == pdTRUE) {
      // TODO: add some magic here
    } 
  }
}

void vPrintSDStats(void)
{
  uint8_t ucBuff[32];
  
  if (xIsSDCardFail == pdFALSE) {

    sprintf((char *) &ucBuff[0], "SD %04u", ulTotalPics);
    
    vPrintAt(75, 116, (const char *) &ucBuff[0]); 
  }
}
