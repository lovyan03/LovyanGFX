# LovyanGFX

An [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) based LCD/TFT driver to simultaneously drive up to 8 (or more?) SPI displays

Devices support
---------------
  - MCU
    - ESP32
    - ESP8266
    - ~~AVR~~
    - ~~STM32~~
  - Displays
    - ILI9341
    - ILI9163
    - SSD1351
    - ST7735
    - ST7789
  - Boards
    - M5Stack Classic/Go/Fire
    - M5StickC
    - OdroidGo
    - TTGO T-Watch
    - ESP-WROVER-KIT
    - 

Platform support
----------------
  - ESP-IDF
  - Arduino
  - Platformio


Usage
-----

```C
    
    #include <LGFX_TFT_eSPI.hpp>
    #include <driver/ledc.h>
    
    static TFT_eSPI tft;
    static TFT_eSprite sprite( &tft );


    void setup() {
    
     tft.init();
     
     tft.setRotation(1);
     
    }


    void loop() {
      tft.startWrite();
      
      tft.fillRect(0, 0, random(tft.width()), random(tft.height()), random(2^16));
      
      tft.endWrite();
    }

```

See the [Examples](examples) folder for more advanced use.


Motivation behind this library
----------

TFT_eSPI is a great library to work with, but very difficult to contribute to because it is updated very often.

It also comes with a lot of constraints such as config files requirements, hardcoded pin numbers and the need to 
apply any change to all architectures.

Most of those limitations come from structural choices due to maintaining multiple architectures in the same codebase.

As a result many `#ifdef` statements are found in the code, this adds a lot of complexity to any contribution.

The LovyanGFX library aims at solving some of these problems while adding performance optimizations and 
board coverage.

A Template meta programming approach is used to reduce `#ifdef` statements in the code base.

The API is based on TFT_eSPI lexicon.


Upcoming support
--------------------------
- [ ] TFT Backlight
- [ ] SD config/init
- [ ] Buttons config/init/debounce
- [ ] TouchScreen config/init


Credits
-------

  - Inspiration: [Bodmer](https://github.com/Bodmer)
  - Author: [Lovyan03](https://github.com/lovyan03)
  - Contributors:
    - ciniml
    - mongonta0716 
    - tobozo

    


