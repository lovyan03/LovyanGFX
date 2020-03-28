/*
MIT License

Copyright (c) 2020 lovyan03

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/*----------------------------------------------------------------------------/
  Lovyan GFX library - ESP32 hardware SPI graphics library .  
  
    for Arduino and ESP-IDF  
  
Original Source  
 https://github.com/lovyan03/LovyanGFX/  

Licence  
 [MIT](https://github.com/lovyan03/LovyanGFX/blob/master/LICENSE)  

Author  
 [lovyan03](https://twitter.com/lovyan03)  
/----------------------------------------------------------------------------*/
#ifndef LOVYANGFX_HPP_
#define LOVYANGFX_HPP_

#ifdef min
#undef min
#endif

#include "lgfx/lgfx_common.hpp"  // common include (always include)

#if !defined LOAD_GFXFF
#include "lgfx/gfxfont.hpp"      // GFX font support (optional)
#endif

#include "lgfx/utility/lgfx_tjpgd.h" // JPEG decode support (optional)
#include "lgfx/utility/lgfx_pngle.h" // PNG decode support (optional)

#include "lgfx/lgfx_sprite.hpp"   // sprite class (optional)

#include "lgfx/panel/panel_ILI9163.hpp"
#include "lgfx/panel/panel_ILI9341.hpp"   // M5Stack / ODROID-GO / ESP-WROVER-KIT4.1
#include "lgfx/panel/panel_SSD1351.hpp"
#include "lgfx/panel/panel_ST7789.hpp"    // LilyGO TTGO T-Watch
#include "lgfx/panel/panel_ST7735.hpp"    // M5StickC


#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)

  #include "lgfx/platforms/lgfx_spi_esp32.hpp"

#elif defined (ESP8266)
// not implemented.
  #include "lgfx/platforms/lgfx_spi_esp8266.hpp"

#elif defined (STM32F7)
// not implemented.
  #include "lgfx/platforms/lgfx_spi_stm32_spi.hpp"

#elif defined (__AVR__)
// not implemented.
  #include "lgfx/platforms/lgfx_spi_avr.hpp"

#endif



namespace lgfx {

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack
  struct LGFX_Config {
    static const Panel_M5Stack panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 19;
    static constexpr int spi_sclk = 18;
  };

#elif defined(ARDUINO_M5Stick_C) // M5Stick C
  struct LGFX_Config {
    static const Panel_M5StickC panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 15;
    static constexpr int spi_miso = 14;
    static constexpr int spi_sclk = 13;
  };

#elif defined(ARDUINO_ODROID_ESP32) // ODROID-GO
  struct LGFX_Config {
    static const Panel_ODROID_GO panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 19;
    static constexpr int spi_sclk = 18;
  };

#elif defined(ARDUINO_T) // TTGO T-Watch
  struct LGFX_Config {
    static const Panel_TTGO_TWatch panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 19;
    static constexpr int spi_miso = -1;
    static constexpr int spi_sclk = 18;
  };

#elif defined ( ARDUINO_ESP32_DEV ) // ESP-WROVER-KIT
  struct LGFX_Config {
    struct Panel_Config {
      static constexpr bool spi_3wire = false;
      static constexpr int spi_cs   = 22;
      static constexpr int spi_dc   = 21;
      static constexpr int gpio_rst = 18;
      static constexpr int gpio_bl  = 5;
      static constexpr int pwm_ch_bl = 7;
      static constexpr int freq_write = 40000000;
      static constexpr int freq_read  = 20000000;
      static constexpr int freq_fill  = 40000000;
    };

    static lgfx::Panel_ILI9341<Panel_Config> panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 25;
    static constexpr int spi_sclk = 19;
  };

#else

  struct LGFX_Config {
//    static const Panel_M5Stack panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
//    static constexpr int dma_channel = 1;
//    static constexpr int spi_mosi = 23;
//    static constexpr int spi_miso = 19;
//    static constexpr int spi_sclk = 18;
  };

#endif
}

typedef lgfx::LGFXSprite LGFXSprite;
typedef lgfx::LGFX_SPI<lgfx::LGFX_Config> LGFX;


#endif
