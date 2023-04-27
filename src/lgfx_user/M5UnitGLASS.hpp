#pragma once

#define LGFX_USE_V1
#include <lgfx/v1/panel/Panel_M5UnitGLASS.hpp>
#include <LovyanGFX.hpp>

#if defined ( ARDUINO )
 #include <Arduino.h>
#endif
#if __has_include( <sdkconfig.h> )
 #include <sdkconfig.h>
#endif

#ifndef M5UNITGLASS_SDA
 #if defined ( ARDUINO )
  #define M5UNITGLASS_SDA SDA
 #elif defined (CONFIG_IDF_TARGET_ESP32S3)
  #define M5UNITGLASS_SDA 2
 #elif defined (CONFIG_IDF_TARGET_ESP32C3)
  #define M5UNITGLASS_SDA 1
 #else
  #define M5UNITGLASS_SDA 21
 #endif
#endif

#ifndef M5UNITGLASS_SCL
 #if defined ( ARDUINO )
  #define M5UNITGLASS_SCL SCL
 #elif defined (CONFIG_IDF_TARGET_ESP32S3)
  #define M5UNITGLASS_SCL 1
 #elif defined (CONFIG_IDF_TARGET_ESP32C3)
  #define M5UNITGLASS_SCL 0
 #else
  #define M5UNITGLASS_SCL 22
 #endif
#endif

#ifndef M5UNITGLASS_ADDR
#define M5UNITGLASS_ADDR 0x3D
#endif

#ifndef M5UNITGLASS_FREQ
#define M5UNITGLASS_FREQ 400000
#endif

class M5UnitGLASS : public lgfx::LGFX_Device
{
  lgfx::Bus_I2C _bus_instance;
  lgfx::Panel_M5UnitGlass _panel_instance;

public:

  M5UnitGLASS(uint8_t pin_sda = M5UNITGLASS_SDA, uint8_t pin_scl = M5UNITGLASS_SCL, uint32_t i2c_freq = M5UNITGLASS_FREQ, int8_t i2c_port = -1, uint8_t i2c_addr = M5UNITGLASS_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
  }

  using lgfx::LGFX_Device::init;
  bool init(uint8_t pin_sda, uint8_t pin_scl, uint32_t i2c_freq = M5UNITGLASS_FREQ, int8_t i2c_port = -1, uint8_t i2c_addr = M5UNITGLASS_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
    return init();
  }

  void setup(uint8_t pin_sda = M5UNITGLASS_SDA, uint8_t pin_scl = M5UNITGLASS_SCL, uint32_t i2c_freq = M5UNITGLASS_FREQ, int8_t i2c_port = -1, uint8_t i2c_addr = M5UNITGLASS_ADDR)
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
      cfg.prefix_len = 0;
      _bus_instance.config(cfg);
      _panel_instance.bus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.offset_rotation = 3;
      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
    _board = lgfx::board_t::board_M5UnitGLASS;
  }
};
