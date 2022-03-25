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

#if defined (ESP_PLATFORM)

 #include <sdkconfig.h>

 #if defined (CONFIG_IDF_TARGET_ESP32S2)

  #include "LGFX_AutoDetect_ESP32S2.hpp"

 #elif defined (CONFIG_IDF_TARGET_ESP32S3)

  #include "LGFX_AutoDetect_ESP32S3.hpp"

 #elif defined (CONFIG_IDF_TARGET_ESP32C3)

 #else

  #include "LGFX_AutoDetect_ESP32.hpp"

 #endif

#elif defined (ESP8266)

 #include "LGFX_AutoDetect_ESP8266.hpp"

#elif defined (__SAMD21__) || defined(__SAMD21G18A__) || defined(__SAMD21J18A__) || defined(__SAMD21E17A__) || defined(__SAMD21E18A__)

 #include "LGFX_AutoDetect_SAMD21.hpp"

#elif defined (__SAMD51__)

 #include "LGFX_AutoDetect_SAMD51.hpp"

#elif defined (STM32F2xx) || defined (STM32F4xx) || defined (STM32F7xx)

  #include "LGFX_AutoDetect_STM32.hpp"

#elif __has_include(<opencv2/opencv.hpp>)

  #include "LGFX_AutoDetect_OpenCV.hpp"

#elif __has_include(<SDL2/SDL.h>)

  #include "LGFX_AutoDetect_sdl.hpp"

#elif defined (__linux__)

  #include "LGFX_AutoDetect_FrameBuffer.hpp"

#endif
