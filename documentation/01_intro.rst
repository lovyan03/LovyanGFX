********
Overview
********

LovyanGFX is a graphics library that works with either an ESP32 with SPI or 8-bit parallel connection to the display or an ATSAMD51 with SPI connection to the LCD. (See compatibility list below.)

This library offers added functionality and faster speed while being largely compatibile with the `AdafruitGFX <https://github.com/adafruit/Adafruit-GFX-Library>`_ and `TFT_eSPI <https://github.com/Bodmer/TFT_eSPI>`_. The goal is for things to "just work" wherever possible.

This library has the following advantages over pre-existing libraries:

* Arduino ESP32 and ESP-IDF are supported.
* Both 16bit and 24bit color modes are supported. (Actual number of colors depends on LCD specifications)
* Execute another process during communication operation using DMA transfer.
* Fast rotation / expansion of the off-screen buffer (sprite).
* Simultaneous use of multiple LCDs.


Supported environments
======================

* Platforms
   * ESP-IDF
   * Arduino ESP32
   * Arduino ATSAMD51 (Seeed)
   * PlatformIO

* Displays
   * HX8357
   * ILI9163
   * ILI9341 (WioTerminal, ESP-WROVER-KIT, ODROID-GO, LoLin D32 Pro, WiFiBoy Pro)
   * ILI9342 (M5Stack, M5Stack Core2)
   * ILI9486
   * ILI9488 (Makerfabs Touch with Camera)
   * SSD1351
   * ST7735 (M5StickC, TTGO T-Wristband, TTGO TS, LoLin D32 Pro, WiFiBoy mini)
   * ST7789 (M5StickCPlus, TTGO T-Watch, ESP-WROVER-KIT, Makerfabs MakePython, DSTIKE D-duino-32 XS)
   * ST7796
   * IT8951 (M5Paper)
   * GDEW0154M09 (M5Stack CoreInk)

* TouchScreens (only ESP32)
   * I2C FT5x06 / FT6x36
   * I2C GT911
   * SPI XPT2046
   * SPI STMPE610


Please refer to `src/lgfx/panel <https://github.com/lovyan03/LovyanGFX/tree/master/src/lgfx/panel>`_ for compatible models.

The default settings for connecting pins are in `src/LovyanGFX.hpp <https://github.com/lovyan03/LovyanGFX/tree/master/src/LovyanGFX.hpp>`_.

LCD panels with similar command systems to the above compatible models can be supported, but only those that we have obtained and confirmed to work are officially supported.

We will give priority to the models for which we receive a support request.
  
This library is also compatible with the above models and LCD panels with a similar command system, but only those that have been obtained and confirmed to work are officially supported.