/*
 * ESP32's definitions for GPIOs
 * 
 */

#ifndef _thermal_cam_pins_H
#define _thermal_cam_pins_H


// ----------------------------------------------------------------------
// For most of ESP32
#define I2C_SDA_PIN           21  // IO21
#define I2C_SCL_PIN           15  // IO15
// ----------------------------------------------------------------------
// TFT Screen 
#define TFT_CS_PIN             4  // IO4
#define TFT_DC_PIN             5  // IO5
#define TFT_SD_CS_PIN         13  // IO13
// ----------------------------------------------------------------------
// hardware VSPI
//#define TFT_MOSI_PIN          23  // IO23
//#define TFT_CLK_PIN           18  // IO18
//#define TFT_RST_PIN           EN  // ESP EN/RST
//#define TFT_MISO_PIN          19  // IO19
// ----------------------------------------------------------------------
#define OPT_KEY1_PIN           25  // IO25
#define OPT_KEY2_PIN           26  // IO26
#define OPT_KEY3_PIN           27  // IO27
#define BAT_SENS_PIN           34  // IO34
// ----------------------------------------------------------------------
// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN               32  // IO32

// ----------------------------------------------------------------------



#endif /* _thermal_cam_pins_H */
