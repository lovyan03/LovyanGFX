#pragma once

#if defined ( ESP_PLATFORM )
 #include <sdkconfig.h>
#endif

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#if defined ( ARDUINO ) && defined ( SDA ) && defined ( SCL )
 static constexpr uint8_t M5_UNIT_LCD_SDA = SDA;
 static constexpr uint8_t M5_UNIT_LCD_SCL = SCL;
#else
 static constexpr uint8_t M5_UNIT_LCD_SDA = 21;
 static constexpr uint8_t M5_UNIT_LCD_SCL = 22;
#endif
static constexpr uint8_t M5_UNIT_LCD_ADDR = 0x3E;

class M5UnitLCD : public lgfx::LGFX_Device
{
  lgfx::Bus_I2C _bus_instance;
  lgfx::Panel_M5UnitLCD _panel_instance;

public:

  M5UnitLCD(uint8_t pin_sda = M5_UNIT_LCD_SDA, uint8_t pin_scl = M5_UNIT_LCD_SCL, uint32_t i2c_freq = 400000, int8_t i2c_port = -1, uint8_t i2c_addr = M5_UNIT_LCD_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
  }

  using lgfx::LGFX_Device::init;
  bool init(uint8_t pin_sda, uint8_t pin_scl, uint32_t i2c_freq = 400000, int8_t i2c_port = -1, uint8_t i2c_addr = M5_UNIT_LCD_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
    return init();
  }

  void setup(uint8_t pin_sda = M5_UNIT_LCD_SDA, uint8_t pin_scl = M5_UNIT_LCD_SCL, uint32_t i2c_freq = 400000, int8_t i2c_port = -1, uint8_t i2c_addr = M5_UNIT_LCD_ADDR)
  {
    if (i2c_port < 0)
    {
#if !defined (CONFIG_IDF_TARGET) || defined (CONFIG_IDF_TARGET_ESP32)
      if (pin_sda == 21 && pin_scl == 22)  /// BASIC / FIRE
      {
        i2c_port = 0;
      }
      else
        // if ((pin_sda == 25 && pin_scl == 32)  /// M5Paper
        //  || (pin_sda == 26 && pin_scl == 32)  /// ATOM
        //  || (pin_sda == 32 && pin_scl == 33)  /// Core2 / CoreInk / Tough / StickC / CPlus
        //  || (pin_sda ==  4 && pin_scl == 13)  /// TimerCam
        //    )
      {
        i2c_port = 1;
      }
#else
      i2c_port = 0;
#endif
    }

    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = i2c_freq;
      cfg.freq_read = i2c_freq > 400000 ? 400000 + ((i2c_freq - 400000) >> 1) : i2c_freq;
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
      cfg.memory_width     = 135;
      cfg.memory_height    = 240;
      cfg.panel_width      = 135;
      cfg.panel_height     = 240;
      cfg.offset_x         =   0;
      cfg.offset_rotation  =   0;
      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};
