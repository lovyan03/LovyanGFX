#pragma once

#if defined ( ESP_PLATFORM )
 #include <sdkconfig.h>
#endif

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#if defined ( ARDUINO ) && defined ( SDA ) && defined ( SCL )
 static constexpr uint8_t M5_UNIT_OLED_SDA = SDA;
 static constexpr uint8_t M5_UNIT_OLED_SCL = SCL;
#elif defined (ESP_PLATFORM)
 #include <sdkconfig.h>
 #if defined (CONFIG_IDF_TARGET_ESP32C3)
  static constexpr uint8_t M5_UNIT_OLED_SDA = 1;
  static constexpr uint8_t M5_UNIT_OLED_SCL = 0;
 #else
  static constexpr uint8_t M5_UNIT_OLED_SDA = 21;
  static constexpr uint8_t M5_UNIT_OLED_SCL = 22;
 #endif
#else
 static constexpr uint8_t M5_UNIT_OLED_SDA = 21;
 static constexpr uint8_t M5_UNIT_OLED_SCL = 22;
#endif
static constexpr uint8_t M5_UNIT_OLED_ADDR = 0x3C;

class M5UnitOLED : public lgfx::LGFX_Device
{
  lgfx::Bus_I2C _bus_instance;
  lgfx::Panel_SH110x _panel_instance;

public:

  M5UnitOLED(uint8_t pin_sda = M5_UNIT_OLED_SDA, uint8_t pin_scl = M5_UNIT_OLED_SCL, uint32_t i2c_freq = 400000, int8_t i2c_port = -1, uint8_t i2c_addr = M5_UNIT_OLED_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
  }

  using lgfx::LGFX_Device::init;
  bool init(uint8_t pin_sda, uint8_t pin_scl, uint32_t i2c_freq = 400000, int8_t i2c_port = -1, uint8_t i2c_addr = M5_UNIT_OLED_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
    return init();
  }

  void setup(uint8_t pin_sda = M5_UNIT_OLED_SDA, uint8_t pin_scl = M5_UNIT_OLED_SCL, uint32_t i2c_freq = 400000, int8_t i2c_port = -1, uint8_t i2c_addr = M5_UNIT_OLED_ADDR)
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
  }
};
