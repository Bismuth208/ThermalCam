// ----------------------------------------------------------------------

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

#include <FS.h>
#include <SD.h>

#include <SPIFFS.h>


#include "common.h"
#include "thermal_cam_pins.h"

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
  
  // ----------------------------------------------------------------------
  // Init TFT chip
//  tft.begin();
  tft.initR(INITR_BLACKTAB);
  
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);

  // ----------------------------------------------------------------------
  // Boot logo...
  tft.drawRect(1, 1, tft.width()-2, tft.height()-2, ST7735_WHITE);
//  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  vPrintAt(20, 50, "Maaagic");
  vPrintAt(46, 68, "Thermal");

  tft.setTextSize(1);

  vPrintAt(100, 90, VERSION_STR); // version build

  vDrawLogo();

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
  
  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;
  
  // ----------------------------------------------------------------------
  vDrawProgressBar(ucBootProgress);
  ++ucBootProgress;
  
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

  vMLX90640_EnableHiQualityMode(pdTRUE);
//  Serial.printf("Cur res %d\n", MLX90640_GetCurResolution(IR_SENSOR_I2C_ADDR));

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
