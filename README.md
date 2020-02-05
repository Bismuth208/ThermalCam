# ThermalCam
![Logo ](pics/Duck_logo_Iron_Gradient.png "Logo")
***

Yet another ESP32 Thermal Camera

#### Based on:
- [Melexis MLX90640 API ](https://github.com/melexis/mlx90640-library.git "But fixed for this project");
- [Compact Gaussian interpolation for small displays ](http://blog.dzl.dk/2019/06/08/compact-gaussian-interpolation-for-small-displays/ "Dzl TheEvilGenius");
- Espressif ESP32 (In my case WROVER-I bare module);
- ST7735 TFT SPI Display.

#### Enabled:
- 19bit ADC resolution;
- 10x oversampling for a pure smoothie;
- Gauss interpolation 2x (up to 64x48)px;
- ~can save raw data on SD card to process on Desktop;~
- HiQ mode via button for less noisy pics (4Hz speed);
- two color modes: grayscale and normal thermogram;
- min, max and average temp show;
- more or less clean code without magic numbers.

Work pretty awesome on ESP32 and using it’s dual core feature.
FPS? Hmmm... it’s limited by hardware of IR Sensor. So, typical value is 4Hz or 4FPS.

#### HOWTO

To make link on linux/nix/bsd systems:
```
ln -s "/Users/YourUserFolderName/Documents/Repos/Git/ThermalCam/mxl90640_lib" "/Users/YourUserFolderName/Documents/Arduino/libraries/mxl90640_lib"
```
on windows just copy folder "mxl90640_lib" to "Documents/Arduino/libraries/"

And to make my life more simple to work with FreeRTOS download [this repo](https://github.com/Bismuth208/FreeRTOS-Helper.git "My own FreeRTOS-Helper git"),
then link same way:
```
ln -s "/Users/YourUserFolderName/Documents/Repos/Git/FreeRTOS-Helper" "/Users/YourUserFolderName/Documents/Arduino/libraries/FreeRTOS-Helper"
```
on windows just copy folder "FreeRTOS-Helper" to "Documents/Arduino/libraries/"


Also Adafruit dependencies required:
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library.git "Adafruit GFX Library git");
- [Adafruit ST7735 Library](https://github.com/adafruit/Adafruit-ST7735-Library.git "Adafruit ST7735 Library git");

***
![TestSubject1 ](pics/test_showcase1.png "TestSubject1")