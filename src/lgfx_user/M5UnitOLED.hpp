#pragma once

#if defined ( ESP_PLATFORM )
 #include <sdkconfig.h>
#endif

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#if defined ( ARDUINO )
 #include <Arduino.h>
#elif __has_include( <sdkconfig.h> )
 #include <sdkconfig.h>
#endif

#ifndef M5UNITOLED_SDA
 #if defined ( ARDUINO )
  #define M5UNITOLED_SDA SDA
 #elif defined (CONFIG_IDF_TARGET_ESP32C3)
  #define M5UNITOLED_SDA 1
 #else
  #define M5UNITOLED_SDA 21
 #endif
#endif

#ifndef M5UNITOLED_SCL
 #if defined ( ARDUINO )
  #define M5UNITOLED_SCL SCL
 #elif defined (CONFIG_IDF_TARGET_ESP32C3)
  #define M5UNITOLED_SCL 0
 #else
  #define M5UNITOLED_SCL 22
 #endif
#endif

#ifndef M5UNITOLED_ADDR
#define M5UNITOLED_ADDR 0x3C
#endif

#ifndef M5UNITOLED_FREQ
#define M5UNITOLED_FREQ 400000
#endif

class M5UnitOLED : public lgfx::LGFX_Device
{
  lgfx::Bus_I2C _bus_instance;
  lgfx::Panel_SH110x _panel_instance;

public:

  M5UnitOLED(uint8_t pin_sda = M5UNITOLED_SDA, uint8_t pin_scl = M5UNITOLED_SCL, uint32_t i2c_freq = M5UNITOLED_FREQ, int8_t i2c_port = -1, uint8_t i2c_addr = M5UNITOLED_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
  }

  using lgfx::LGFX_Device::init;
  bool init(uint8_t pin_sda, uint8_t pin_scl, uint32_t i2c_freq = M5UNITOLED_FREQ, int8_t i2c_port = -1, uint8_t i2c_addr = M5UNITOLED_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
    return init();
  }

  void setup(uint8_t pin_sda = M5UNITOLED_SDA, uint8_t pin_scl = M5UNITOLED_SCL, uint32_t i2c_freq = M5UNITOLED_FREQ, int8_t i2c_port = -1, uint8_t i2c_addr = M5UNITOLED_ADDR)
  { 
    if (i2c_port < 0)
    {
      i2c_port = 0;
#ifdef _M5EPD_H_
      if ((pin_sda == 25 && pin_scl == 32)  /// M5Paper
      {
        i2c_port = 1
      }
#endif
    }

    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = i2c_freq;
      cfg.freq_read  = i2c_freq;
      cfg.pin_scl = pin_scl;
      cfg.pin_sda = pin_sda;
      cfg.i2c_port = i2c_port;
      cfg.i2c_addr = i2c_addr;
      cfg.prefix_len = 1;
      cfg.prefix_cmd = 0x00;
      cfg.prefix_data = 0x40;
      _bus_instance.config(cfg);
      _panel_instance.bus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.panel_width = 64;
      cfg.offset_x = 32;
      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
    _board = lgfx::board_t::board_M5UnitOLED;
  }
};
