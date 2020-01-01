#ifndef LGFX_PANEL_DEVICE_HPP_
#define LGFX_PANEL_DEVICE_HPP_

#include "lgfx_common.hpp"

#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)
  #include "esp32_common.hpp"
  #include "panel_device_esp32spi.hpp"
#elif defined (STM32F7)
  #include "panel_device_stm32_spi.hpp"
#elif defined (__AVR__)
  #include "panel_device_avr_spi.hpp"
#endif

#endif
