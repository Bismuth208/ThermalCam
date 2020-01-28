# ThermalCam


Yet another ESP32 Thermal Camera

Based on:
- Melexis API;
- MLX90640;
- Some dude who made Gauss interpolation implementation;
- ESP32;

Enabled :
- 19bit ADC resolution;
- 10x oversampling for a pure smoothie;
- ~Gauss interpolation 2x (up to 64x48)px;~
- ~can save raw data on SD card to process on Desktop;~
- ~HiQ mode via button for less noisy pics (1Hz speed);~
- two color modes: grayscale and normal thermogram;
- min, max and average temp show;
- more or less clean code without magic numbers.

Work pretty awesome on ESP32 and using it’s dual core feature.

FPS? Hmmm... it’s limited by hardware of IR Sensor. So, typical value is 4Hz or 4FPS.


How to:

To make link on linux/nix/bsd systems:

ln -s "/Users/YourUserFilderName/Documents/Repos/Git/ThermalCam/mxl90640_lib" "/Users/YourUserFilderName/Documents/Arduino/libraries/mxl90640_lib"

on windows just copy folder "mxl90640_lib" to "Documents/Arduino/libraries/"