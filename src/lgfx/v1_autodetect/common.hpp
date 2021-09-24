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
    , board_PyBadge
    , board_M5Tough
    , board_OpenCV
    };
  }
  using namespace boards;
 }
}


#if defined (ESP_PLATFORM)

 #include <sdkconfig.h>

 #if defined (CONFIG_IDF_TARGET_ESP32S2)

  #include "LGFX_AutoDetect_ESP32S2.hpp"

 #elif defined (CONFIG_IDF_TARGET_ESP32C3)

 #else

  #include "LGFX_AutoDetect_ESP32.hpp"

 #endif

#elif defined (__SAMD21__)

 #include "LGFX_AutoDetect_SAMD21.hpp"

#elif defined (__SAMD51__)

 #include "LGFX_AutoDetect_SAMD51.hpp"

#elif defined (STM32F2xx) || defined (STM32F4xx) || defined (STM32F7xx)

  #include "LGFX_AutoDetect_STM32.hpp"

#elif __has_include(<opencv2/opencv.hpp>)

  #include "LGFX_AutoDetect_OpenCV.hpp"

#endif
