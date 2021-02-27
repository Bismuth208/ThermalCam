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

extern uint32_t ul_screenshots_taken;

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

    memset(&ucCurrentFolderPathBuff[0], 0x00, sizeof(ucCurrentFolderPathBuff));

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
  if (xIsSDCardFail == pdFALSE) {
    uint8_t uc_thc_file_name[64];

    memset(&uc_thc_file_name[0], 0x00, sizeof(uc_thc_file_name));
    sprintf((char*) &uc_thc_file_name[0],  "/sd%s/cal.thc", (char *) &ucCurrentFolderPathBuff[0]);

    FILE *file = fopen((char *) &uc_thc_file_name[0], "wb");

    x_thc_frame.x_header.ul_type = THC_FRAME_DATA_TYPE_CAL;

    fwrite(&x_thc_frame.x_header, 1, sizeof(x_thc_frame.x_header), file);
    fwrite(x_thc_frame.puc_cal_data, 1, x_thc_frame.x_header.ul_cal_data_size, file);
    fclose(file);
  }
}

/*
 * @brief Check for how log shutter butten is pressed and select proper mode
 * @note this is temporal in future it will be replaced with vBtnPollerTask
 */
void v_thc_check_frame_type(void)
{
  if (ul_screenshots_taken > THC_MAX_FRAMES_FOR_MOV) {
    x_thc_frame.x_header.ul_type = THC_FRAME_DATA_TYPE_MOV;
  } else {
    x_thc_frame.x_header.ul_type = THC_FRAME_DATA_TYPE_STILL;
  }
}

/*
 * @brief Based on lower and faster level of SD library
 * @param *puc_dest_file - full path and name for file
 */
void v_thc_save_frame(const char *puc_dest_file)
{
  FILE *file = fopen(puc_dest_file, "wb");

  if (file != NULL)  {
    fwrite(&x_thc_frame.x_header, 1, sizeof(x_thc_frame.x_header), file);
    fwrite(x_thc_frame.puc_frame_data, 1, x_thc_frame.x_header.ul_frame_data_size, file);
  }

  fclose(file);
}

void vTakeScreenShoot(void *pvArg)
{
  (void) pvArg;

  if (xIsSDCardFail == pdFALSE) {
    // FIXME: this is too long operation! Make most of it cached !
    uint8_t uc_thc_file_name[64];

    memset(&uc_thc_file_name[0], 0x00, sizeof(uc_thc_file_name));
    sprintf((char *) &uc_thc_file_name[0], "/sd%s/%05u.thc", (char *) &ucCurrentFolderPathBuff[0], ulTotalPics);
    
    v_thc_save_frame((const char *) &uc_thc_file_name[0]);

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
