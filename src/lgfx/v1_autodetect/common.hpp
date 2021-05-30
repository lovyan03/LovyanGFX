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

#ifndef LGFX_USE_V1
#define LGFX_USE_V1
#endif

#include "LovyanGFX.hpp"

namespace lgfx
{
 inline namespace v1
 {
  namespace boards
  {
    enum board_t
    { board_unknown
    , board_Non_Panel
    , board_M5Stack
    , board_M5StackCore2
    , board_M5StickC
    , board_M5StickCPlus
    , board_TTGO_TS
    , board_TTGO_TWatch
    , board_TTGO_TWristband
    , board_ODROID_GO
    , board_DDUINO32_XS
    , board_ESP_WROVER_KIT
    , board_LoLinD32
    , board_WioTerminal
    , board_WiFiBoy_Pro
    , board_WiFiBoy_Mini
    , board_Makerfabs_TouchCamera
    , board_Makerfabs_MakePython
    , board_M5StackCoreInk
    , board_M5Stack_CoreInk = board_M5StackCoreInk
    , board_M5Paper
    , board_ESP32_S2_Kaluga_1
    , board_WT32_SC01
    };
  }
  using namespace boards;
 }
}


#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (ESP_PLATFORM)

 #include "LGFX_AutoDetect_ESP32.hpp"

#elif defined (__SAMD51__)

 #if defined( LGFX_WIO_TERMINAL ) || defined (ARDUINO_WIO_TERMINAL) || defined(WIO_TERMINAL)

  #include "LGFX_AutoDetect_SAMD51.hpp"

 #endif

#else

  #include "LGFX_AutoDetect_STM32.hpp"

#endif

