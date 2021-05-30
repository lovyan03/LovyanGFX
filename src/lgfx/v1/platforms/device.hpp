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

#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (CONFIG_IDF_TARGET_ESP32S2) || defined (ESP_PLATFORM)

#include "esp32/Light_PWM.hpp"
#include "esp32/Bus_SPI.hpp"
#include "esp32/Bus_I2C.hpp"
#include "esp32/Bus_Parallel8.hpp"

#elif defined (__SAMD51__)

#include "samd51/Bus_SPI.hpp"
#include "samd51/Bus_I2C.hpp"

#elif defined (STM32F2xx) || defined (STM32F4xx) || defined (STM32F7xx)

#include "stm32/Bus_SPI.hpp"

#elif defined ( ARDUINO )

#include "arduino_default/Bus_SPI.hpp"

#endif

