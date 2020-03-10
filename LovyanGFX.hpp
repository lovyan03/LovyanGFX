#ifndef LOVYANGFX_HPP_
#define LOVYANGFX_HPP_


#define LOAD_GFXFF               // GFX font support when defined.
#include "LGFX_FontLoad.hpp"

#include "utility/tjpgdClass.h"  // JPEG support when included.


#include "platforms/lgfx_common.hpp"
#include "lgfx_panel/panel_common.hpp"

#include "LGFX_core.hpp"

#include "platforms/lgfx_sprite.hpp"

#include "lgfx_panel/panel_ILI9341.hpp"    // M5Stack / ODROID-GO / ESP-WROVER-KIT4.1
#include "lgfx_panel/panel_ST7735.hpp"     // M5StickC
#include "lgfx_panel/panel_ST7789.hpp"     // TTGO T-Watch
#include "lgfx_panel/panel_ILI9163.hpp"
#include "lgfx_panel/panel_ssd_common.hpp" // for SSD1351



#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)

  #include "platforms/lgfx_spi_esp32.hpp"

#elif defined (ESP8266)

  #include "platforms/lgfx_spi_esp8266.hpp"

#elif defined (STM32F7)

  #include "platforms/lgfx_spi_stm32_spi.hpp"

#elif defined (__AVR__)

  #include "platforms/lgfx_spi_avr.hpp"

#endif


#endif
