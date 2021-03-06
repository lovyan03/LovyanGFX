/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#include "gitTagVersion.h"

#include "platforms/device.hpp"
#include "platforms/common.hpp"
#include "lgfx_filesystem_support.hpp"
#include "LGFXBase.hpp"
#include "LGFX_Sprite.hpp"
#include "panel/Panel_ILI9341.hpp"
#include "panel/Panel_ILI9342.hpp"
#include "panel/Panel_ST7735.hpp"
#include "panel/Panel_ST7789.hpp"
#include "touch/Touch_FT5x06.hpp"
#include "touch/Touch_GT911.hpp"

using LovyanGFX = lgfx::LovyanGFX;

#if !defined ( LGFX_USER_SETTING )

  #if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (ESP_PLATFORM)

  #if defined( ARDUINO_M5Stack_Core_ESP32 ) || defined( ARDUINO_M5STACK_FIRE )
    #define LGFX_M5STACK
    #define LGFX_M5STACK_CORE2
  #elif defined( ARDUINO_M5STACK_Core2 ) // M5Stack Core2
    #define LGFX_M5STACK_CORE2
  #elif defined( ARDUINO_M5Stick_C ) // M5Stick C / CPlus
    #define LGFX_M5STICK_C
  #elif defined( ARDUINO_M5Stack_CoreInk ) // M5Stack CoreInk
    #define LGFX_M5STACK_COREINK
  #elif defined( ARDUINO_M5STACK_Paper ) // M5Paper
    #define LGFX_M5PAPER
  #elif defined( ARDUINO_ODROID_ESP32 ) // ODROID-GO
    #define LGFX_ODROID_GO
  #elif defined( ARDUINO_TTGO_T1 ) // TTGO TS
    #define LGFX_TTGO_TS
  #elif defined( ARDUINO_TWatch ) || defined( ARDUINO_T ) // TTGO T-Watch
    #define LGFX_TTGO_TWATCH
  #elif defined( ARDUINO_D ) || defined( ARDUINO_DDUINO32_XS ) // DSTIKE D-duino-32 XS
    #define LGFX_DDUINO32_XS
  #elif defined( ARDUINO_LOLIN_D32_PRO )
    #define LGFX_LOLIN_D32_PRO
  #elif defined( ARDUINO_ESP32_WROVER_KIT )
    #define LGFX_ESP_WROVER_KIT
  #endif

  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_COREINK )

    #include "panel/Panel_GDEW0154M09.hpp"

  #endif
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5PAPER )

    #include "panel/Panel_IT8951.hpp"

  #endif

  #include "platforms/esp32/LGFX_AutoDetect.hpp"
  // #include "../config/LGFX_Config_AutoDetectESP32.hpp"

  #elif defined (__SAMD51__)

  #if defined( LGFX_WIO_TERMINAL ) || defined (ARDUINO_WIO_TERMINAL) || defined(WIO_TERMINAL)
  //  #include "../config/LGFX_Config_WioTerminal.hpp"

  #include "platforms/samd51/LGFX_AutoDetect.hpp"

  #endif

  #endif

#endif
