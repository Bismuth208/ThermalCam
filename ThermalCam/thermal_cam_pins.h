/*
 * ESP32's definitions for GPIOs
 * 
 */

#ifndef _thermal_cam_pins_H
#define _thermal_cam_pins_H


// ----------------------------------------------------------------------
// For most of ESP32
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 15

// ----------------------------------------------------------------------
// TFT Screen 
#define TFT_CS   4  // goes to TFT CS
#define TFT_DC   5  // goes to TFT DC

// hardware VSPI
//#define TFT_MOSI 23  // goes to TFT MOSI
//#define TFT_CLK  18  // goes to TFT SCK/CLK
//#define TFT_RST  EN   // ESP EN/RST to TFT RESET
//#define TFT_MISO 19

// ----------------------------------------------------------------------
// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN            32


#endif /* _thermal_cam_pins_H */
