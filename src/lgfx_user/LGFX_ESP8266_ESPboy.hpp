#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

// LGFX for ESP boy

class LGFX : public lgfx::LGFX_Device
{
  struct Panel_ESPboy : public lgfx::Panel_ST7735S
  {
    static constexpr int i2c_freq = 400000;
    static constexpr int pin_sda = 4;
    static constexpr int pin_scl = 5;
    static constexpr int port = 0;

    // for CS control
    static constexpr int MCP23017_ADDR = 0x20;
    static constexpr int MCP23017_GPIO_B = 0x13;
    static constexpr int MCP23017_IODIR_B = 0x01;

    // for Backlight control
    static constexpr int MCP4725_ADDR = 0x60;

    void init_cs(void) override
    {
      lgfx::i2c::init(port, pin_sda, pin_scl);

      // MCP23017 PortB bit0 = CS. set to output.
      lgfx::i2c::writeRegister8(port, MCP23017_ADDR, MCP23017_IODIR_B, 0x00, 0xFE, i2c_freq);

      cs_control(true);
    }

    void cs_control(bool level) override
    {
      // MCP23017 PortB bit0 = CS. set level
      lgfx::i2c::writeRegister8(port, MCP23017_ADDR, MCP23017_GPIO_B, level, 0xFE, i2c_freq);
    }

    void setBrightness(uint8_t brightness) override
    {
      if (brightness)
      {
        brightness = (128+brightness) >> 2;
      }
      uint8_t buf[2];
      buf[0] = brightness>>4;
      buf[1] = brightness>>4 | brightness<<4;
      lgfx::i2c::transactionWrite(port, MCP4725_ADDR, buf, 2, i2c_freq);
    }
  };

  Panel_ESPboy _panel_instance;
  lgfx::Bus_SPI _bus_instance;

public:

  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      cfg.spi_mode = 0;
      cfg.spi_3wire  = true;
      cfg.freq_write = 30000000;
      cfg.freq_read  =  8000000;
      cfg.pin_sclk = 14;
      cfg.pin_mosi = 13;
      cfg.pin_miso = 12;
      cfg.pin_dc   = 16;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.memory_width     =   132;
      cfg.memory_height    =   132;
      cfg.panel_width      =   128;
      cfg.panel_height     =   128;
      cfg.offset_x         =     2;
      cfg.offset_y         =     1;
      cfg.offset_rotation  =     2;
      cfg.dummy_read_pixel =     9;
      cfg.dummy_read_bits  =     1;
      cfg.readable         =  true;
      cfg.bus_shared       = false;

      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

