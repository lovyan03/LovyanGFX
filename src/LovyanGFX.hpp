/*----------------------------------------------------------------------------/
  Lovyan GFX library - ESP32 hardware SPI graphics library .

    for Arduino and ESP-IDF

Original Source
 https://github.com/lovyan03/LovyanGFX/

Licence
 [MIT](https://github.com/lovyan03/LovyanGFX/blob/master/LICENSE)  

Author
 [lovyan03](https://twitter.com/lovyan03)  
/-----------------------------------------------------------------------------/
/----------------------------------------------------------------------------*/


#ifndef LOVYANGFX_HPP_
#define LOVYANGFX_HPP_

#include "lgfx/lgfx_common.hpp"  // common include (always include)

#include "lgfx/gfxfont.hpp"      // GFX font support (optional)

#include "lgfx/utility/tjpgdClass.h" // JPEG decode support (optional)

#include "lgfx/lgfx_sprite.hpp"   // sprite class (optional)

#include "lgfx/panel/panel_ILI9163.hpp"
#include "lgfx/panel/panel_ILI9341.hpp"   // M5Stack / ODROID-GO / ESP-WROVER-KIT4.1
#include "lgfx/panel/panel_SSD1351.hpp"
#include "lgfx/panel/panel_ST7789.hpp"    // TTGO T-Watch
#include "lgfx/panel/panel_ST7735.hpp"    // M5StickC


#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)

  #include "lgfx/platforms/lgfx_spi_esp32.hpp"

#elif defined (ESP8266)

  #include "lgfx/platforms/lgfx_spi_esp8266.hpp"

#elif defined (STM32F7)

  #include "lgfx/platforms/lgfx_spi_stm32_spi.hpp"

#elif defined (__AVR__)

  #include "lgfx/platforms/lgfx_spi_avr.hpp"

#endif


#endif
