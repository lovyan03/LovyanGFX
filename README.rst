#########
LovyanGFX
#########

LovyanGFX is an LCD driver library for ESP32 (SPI or 8bit Parallel) and ATSAMD51 (SPI). Supported devices include M5Stack, M5Core2, M5StickC, TTGO T-Watchm ODROID-GO, ESP-WROVER-KIT, WioTerminal and more...

.. image:: documentation/_static/images/demo1.gif
   :target: https://www.youtube.com/watch?v=SMOHRPqUZcQ
   :width: 49 %
.. image:: documentation/_static/images/demo2.gif
   :target: https://www.youtube.com/watch?v=F5gsp41Elac
   :width: 49 %


************
Introduction
************

..
  intro-start

Microcontrollers and various tinker-friendly devices need display drivers to talk to their LCD display modules. Popular driver libraries include `AdafruitGFX <https://github.com/adafruit/Adafruit-GFX-Library>`__ made by Adafruit Industries, which brought easy-to-use proportional fonts.

Then Bodmer came along and wrote `TFT_eSPI <https://github.com/Bodmer/TFT_eSPI>`__ based on that, with support for many more displays. TFT_eSPI is a great library. However, it is structurally complex because it targets multiple architectures, making it very difficult to add required functions such as support for ESP-IDF and 18-bit color.

On the supported architectures, LovyanGFX is the next evolutionary step. It  offers added functionality and improved performance while being largely compatibile with its predecessors AdafruitGFX and TFT_eSPI. Major improvements in the API mean not everything could stay the same, but the goal has been for things to "just work" wherever possible.

LovyanGFX works with either an ESP32 with SPI or 8-bit parallel connection to the display or an ATSAMD51 with SPI connection to the LCD.

..
  intro-end

Among the advantages of LovyanGFX over existing libraries:

..
  advantages-start

* Arduino ESP32 and ESP-IDF are supported.
* Both 16bit and 24bit color modes are supported. (Actual number of colors depends on LCD specifications)
* Uses DMA transfers so other processes can execute during communication with display.
* Fast rotation / expansion of the off-screen buffer (sprite).
* Simultaneous use of multiple LCDs.

..
  advantages-end


Supported environments
======================

Platforms
---------

..
  platforms-start

* ESP-IDF
* Arduino ESP32
* Arduino ATSAMD51 (Seeed)
* PlatformIO

..
  platforms-end

Displays
--------

..
  displays-start

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

..
  displays-end

TouchScreens (only on ESP32)
----------------------------

..
  touchscreens-start

* I2C FT5x06 / FT6x36
* I2C GT911
* SPI XPT2046
* SPI STMPE610

..
  touchscreens-end


Documentation
=============

To read the documentation, click `here <https://lovyangfx.readthedocs.io/>`__.


Acknowledgements
================

Created by
----------

..
  created-start

* Inspiration: `Bodmer <https://github.com/Bodmer>`__
* Author: `lovyan03 <https://github.com/lovyan03>`__

..
  created-end
  

Contributors
------------

..
  contributors-start

* `ciniml <https://github.com/ciniml>`__
* `mongonta0716 <https://github.com/mongonta0716>`__
* `tobozo <https://github.com/tobozo>`__

..
  contributors-end


Thanks to
---------

..
  thanks-start

* `Bodmer <https://github.com/Bodmer/>`__, author of the `TFT_eSPI <https://github.com/Bodmer/TFT_eSPI>`__ library, for the inspiration to create this library.  
* `Adafruit Industries <https://github.com/adafruit/>`__ for publishing `AdafruitGFX <https://github.com/adafruit/Adafruit-GFX-Library>`__, which is the basis for TFT_eSPI.  
* `ChaN <http://elm-chan.org/>`__, author of `TJpgDec <http://elm-chan.org/fsw/tjpgd/00index.html>`__ (Tiny JPEG Decompressor).  
* `kikuchan <https://github.com/kikuchan/>`__, author of `Pngle <https://github.com/kikuchan/pngle>`__ (PNG Loader for Embedding).  
* `Richard Moore <https://github.com/ricmoo/>`__, author of `QRCode <https://github.com/ricmoo/QRCode/>`__ (QR code generation library).  
* `ciniml <https://github.com/ciniml>`__, for many technical tips and help with validation in the ESP-IDF environment.  
* `mongonta0716 <https://github.com/mongonta0716>`__, for verifying the work from the beginning of the development with many bugs and for his advice.  
* `tobozo <https://github.com/tobozo>`__, for testing it on various boards, translating it into English and giving me a lot of advice.  
* `TANAKA Masayuki <https://github.com/tanakamasayuki>`__, for creating the font data.  
* `YAMANEKO <https://github.com/yamamaya>`__, for creating the `lgfxFontSubsetGenerator <https://github.com/yamamaya/lgfxFontSubsetGenerator>`__.

..
  thanks-end
  

Legal Information
=================

Included Libraries
------------------

..
  included-libs-start


* `TJpgDec <http://elm-chan.org/fsw/tjpgd/00index.html>`__ *Tiny JPEG Decompressor* by `ChaN <http://elm-chan.org/>`__  
* `Pngle <https://github.com/kikuchan/pngle>`__ *stream based portable PNG Loader for Embedding* by `kikuchan <https://github.com/kikuchan/>`__  
* `QRCode <https://github.com/ricmoo/QRCode/>`__ *library for generating QR codes* by `Richard Moore <https://github.com/ricmoo/>`__ and `Nayuki <https://www.nayuki.io/page/qr-code-generator-library>`__

..
  included-libs-end


Licenses
--------

..
  licenses-start

* This library (unless otherwise specified) : `FreeBSD <https://github.com/lovyan03/LovyanGFX/blob/master/license.txt>`__ - lovyan03
* TJpgDec : `original <https://github.com/lovyan03/LovyanGFX/blob/master/src/lgfx/utility/lgfx_tjpgd.c>`__ - ChaN  
* Pngle : `MIT <https://github.com/kikuchan/pngle/blob/master/LICENSE>`__ - kikuchan  
* QRCode : `MIT <https://github.com/ricmoo/QRCode/blob/master/LICENSE.txt>`__ - Richard Moore and Nayuki  
* GFX font and GLCD font : `2-clause BSD <https://github.com/adafruit/Adafruit-GFX-Library/blob/master/license.txt>`__ - Adafruit Industries
* Font 2,4,6,7,8 :  `FreeBSD <https://github.com/Bodmer/TFT_eSPI/blob/master/license.txt>`__ - Bodmer  
* converted IPA font : `IPA Font License <src/Fonts/IPA/IPA_Font_License_Agreement_v1.0.txt>`__  
* efont : `3-Clause BSD License <src/Fonts/efont/COPYRIGHT.txt>`__  

..
  licenses-end





