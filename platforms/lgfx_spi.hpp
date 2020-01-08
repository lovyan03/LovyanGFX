#ifndef LGFX_SPI_HPP_
#define LGFX_SPI_HPP_

#include "lgfx_common.hpp"

#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)
  #include "esp32_common.hpp"
  #include "lgfx_spi_esp32.hpp"
#elif defined (STM32F7)
  #include "lgfx_spi_stm32_spi.hpp"
#elif defined (__AVR__)
  #include "lgfx_spi_avr.hpp"
#endif

#endif
