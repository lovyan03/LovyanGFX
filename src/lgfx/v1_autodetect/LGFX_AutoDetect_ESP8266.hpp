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

#include "../v1_init.hpp"
#include "common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

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


  class LGFX : public lgfx::LGFX_Device
  {
    lgfx::Panel_Device* _panel_last = nullptr;
    lgfx::ILight* _light_last = nullptr;
    lgfx::ITouch* _touch_last = nullptr;
    lgfx::Bus_SPI _bus_spi;

    bool init_impl(bool use_reset, bool use_clear)
    {
      auto board = board_t::board_unknown;

      int retry = 4;
      do
      {
        if (retry == 1) use_reset = true;
        board = autodetect(use_reset, board);
        //ESP_LOGI(LIBRARY_NAME,"autodetect board:%d", board);
      } while (board_t::board_unknown == board && --retry >= 0);
      _board = board;

      return LGFX_Device::init_impl(false, use_clear);
    }

  public:

    board_t autodetect(bool use_reset = true, board_t board = board_t::board_unknown)
    {
      auto bus_cfg = _bus_spi.config();
//    if (bus_cfg.pin_mosi != -1 && bus_cfg.pin_sclk != -1) return true;

      panel(nullptr);

      if (_panel_last)
      {
        delete _panel_last;
        _panel_last = nullptr;
      }
      if (_light_last)
      {
        delete _light_last;
        _light_last = nullptr;
      }
      if (_touch_last)
      {
        delete _touch_last;
        _touch_last = nullptr;
      }

      bus_cfg.freq_write = 8000000;
      bus_cfg.freq_read  = 8000000;
      bus_cfg.spi_mode = 0;
      bus_cfg.pin_sclk = 14;
      bus_cfg.pin_mosi = 13;
      bus_cfg.pin_miso = 12;

      uint32_t id;
      (void)id;  // suppress warning

#if defined ( LGFX_AUTODETECT ) \
 || defined ( LGFX_ESPBOY )

      if (board == 0
      || board == board_t::board_ESPboy)
      {
        bus_cfg.spi_3wire  = true;
        bus_cfg.pin_dc     = 16;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();

        auto p = new Panel_ESPboy();
        p->bus(&_bus_spi);

        p->init_cs();
        id = p->readCommand(0x04, 0, 4);
// Serial.printf("read:%08x\r\n", id);
        if ((id & 0xFF) == 0x7C)
        { /// ST7735S
// Serial.printf("ESPboy");
          board = board_t::board_ESPboy;
          bus_cfg.freq_write = 30000000;
          bus_cfg.freq_read  =  8000000;
          _bus_spi.config(bus_cfg);

          { // 表示パネル制御の設定を行います。
            auto cfg = p->config();    // 表示パネル設定用の構造体を取得します。

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

            p->config(cfg);
          }

          _panel_last = p;
          goto init_clear;
        }
        delete p;
      }

#endif

      board = board_t::board_unknown;

      goto init_clear;
  init_clear:

      panel(_panel_last);

      return board;
    }
  };

//----------------------------------------------------------------------------
 }
}

using LGFX = lgfx::LGFX;
