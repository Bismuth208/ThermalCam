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

void writeFile(const char *path, uint8_t *pucData, uint32_t ulDataSize)
{
 #if 0
  File file = SD.open(path, FILE_WRITE);
  
  if (file) {
    file.write(pucData, ulDataSize);
    file.close();
  }
#else
  FILE *file = fopen(path, "w");

  if (file != NULL)  {
    size_t err = fwrite(pucData, 1, ulDataSize, file);
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
