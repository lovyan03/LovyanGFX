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

#if defined ( ARDUINO )

 #if defined ( ARDUINO_SEEED_XIAO_M0 )
    #define LGFX_SEEED_XIAO_EXPANSION
 #endif

 #if defined ( ARDUINO_SAMD_HALLOWING )
    #define LGFX_HALLOWING_M0
 #endif

#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Light_TC : public ILight
  {
    struct config_t
    {
      uint8_t pin = 0;
      uint8_t tc_index = 0;
      uint8_t cc_index = 0;
      bool timer_alt = false;
    };

    config_t config(void) const { return _cfg; }
    void config(const config_t& config) { _cfg = config; }

    bool init(uint8_t brightness) override
    {
      uint8_t gclk_id;
      switch (_cfg.tc_index)
      {
      case 3: _tc = TC3; gclk_id = 0x1B; break;
      case 4: _tc = TC4; gclk_id = 0x1C; break;
      case 5: _tc = TC5; gclk_id = 0x1C; break;
#if defined ( TC6 )
      case 6: _tc = TC6; gclk_id = 0x1D; break;
#endif
#if defined ( TC7 )
      case 7: _tc = TC7; gclk_id = 0x1D; break;
#endif
      default: return false;
      }

      // As written, _tc will source from Generic Clock Generator 0, which
      // is presumed already configured in boot code. For example, Arduino
      // startup sets this to DFLL48M (48 MHz), and almost certainly true
      // of any non-Arduino environment as well. That step is NOT repeated
      // here as GCLK0 is the main system clock and unwise to alter now.

      // Enable TC peripheral clock, sourced from GCLK0 (presumed 48 MHz)
      GCLK->CLKCTRL.reg = (1 << 14) | (0 << 8) | gclk_id; // CLKEN + Generic clock gen 0 + ID
      while (GCLK->STATUS.bit.SYNCBUSY);

      // Configure _tc
      _tc->COUNT8.CTRLA.bit.SWRST = 1;
      while (_tc->COUNT8.CTRLA.bit.SWRST);
      _tc->COUNT8.CTRLA.reg = (0x01 << 2) | (0x02 << 5) | (0x04 << 8) | (0x01 << 12); // MODE=COUNT8, WAVEGEN=NPWM, PRESCALER=DIV16, PRESCSYNC=PRESC
      _tc->COUNT8.PER.reg = 255;      // max brightness;
      _tc->COUNT8.CC[_cfg.cc_index].reg = brightness;
      _tc->COUNT8.DBGCTRL.bit.DBGRUN = 1;
      _tc->COUNT8.INTFLAG.reg = 0x33; // Clear all flags
      while ( _tc->COUNT8.STATUS.bit.SYNCBUSY );
      _tc->COUNT8.CTRLA.bit.ENABLE = 1;
      while ( _tc->COUNT8.STATUS.bit.SYNCBUSY );

      // Configure PORT
      lgfx::pinMode( _cfg.pin, pin_mode_t::output );
      lgfx::pinAssignPeriph( _cfg.pin, _cfg.timer_alt ? 5 : 4 ); // 4, 5 = periph E, F (PIO_TIMER, PIO_TIMER_ALT)

      return true;
    }

    void setBrightness(uint8_t brightness) override
    {
      if (_tc) {
        _tc->COUNT8.CC[_cfg.cc_index].reg = brightness;
        while ( _tc->COUNT8.STATUS.bit.SYNCBUSY );
      }
    }

  protected:
    config_t _cfg;
    Tc* _tc = nullptr;
  };

  struct Light_TCC : public ILight
  {
    struct config_t
    {
      uint8_t pin = 0;
      uint8_t tcc_index = 0;
      uint8_t cc_index = 0;
      bool timer_alt = false;
    };

    config_t config(void) const { return _cfg; }
    void config(const config_t& config) { _cfg = config; }

    bool init(uint8_t brightness) override
    {
      uint8_t gclk_id;
      switch (_cfg.tcc_index)
      {
      case 0: _tcc = TCC0; gclk_id = 0x1A; break;
      case 1: _tcc = TCC1; gclk_id = 0x1A; break;
      case 2: _tcc = TCC2; gclk_id = 0x1B; break;
#if defined ( TCC3 )
      case 3: _tcc = TCC3; gclk_id = 0x25; break;
#endif
      default: return false;
      }

      // As written, _tc will source from Generic Clock Generator 0, which
      // is presumed already configured in boot code. For example, Arduino
      // startup sets this to DFLL48M (48 MHz), and almost certainly true
      // of any non-Arduino environment as well. That step is NOT repeated
      // here as GCLK0 is the main system clock and unwise to alter now.

      // Enable TCC peripheral clock, sourced from GCLK0 (presumed 48 MHz)
      GCLK->CLKCTRL.reg = (1 << 14) | (0 << 8) | gclk_id; // CLKEN + Generic clock gen 0 + ID
      while (GCLK->STATUS.bit.SYNCBUSY);

      // Configure _tcc
      _tcc->CTRLA.bit.SWRST = 1;
      while (_tcc->SYNCBUSY.bit.SWRST);
      _tcc->CTRLA.reg = (0x04 << 8) | (0x01 << 12); // PRESCALER=DIV16, PRESCSYNC=PRESC
      _tcc->WAVE.bit.WAVEGEN = 2;   // Single-slope PWM
      while (_tcc->SYNCBUSY.bit.WAVE);
      _tcc->PER.reg = 255;          // 8-bit range
      while (_tcc->SYNCBUSY.bit.PER);
      _tcc->CC[_cfg.cc_index].reg = brightness;
      while (_tcc->SYNCBUSY.reg & 0xF00);
      _tcc->DBGCTRL.bit.DBGRUN = 1;
      _tcc->INTFLAG.reg = 0x3FCF;   // Clear all flags
      _tcc->CTRLA.bit.ENABLE = 1;
      while (_tcc->SYNCBUSY.bit.ENABLE);

      // Configure PORT
      lgfx::pinMode( _cfg.pin, pin_mode_t::output );
      lgfx::pinAssignPeriph( _cfg.pin, _cfg.timer_alt ? 5 : 4 ); // 4, 5 = periph E, F (PIO_TIMER, PIO_TIMER_ALT)

      return true;
    }

    void setBrightness(uint8_t brightness) override
    {
      if (_tcc)
      {
        _tcc->CC[_cfg.cc_index].reg = brightness;
        while (_tcc->SYNCBUSY.reg & 0xF00);
      }
    }

  protected:
    config_t _cfg;
    Tcc* _tcc = nullptr;
  };

  class LGFX : public LGFX_Device
  {
    lgfx::Panel_Device* _panel_last = nullptr;
    lgfx::ILight* _light_last = nullptr;
//  lgfx::ITouch* _touch_last = nullptr;
    lgfx::Bus_SPI _bus_spi;

    static void _pin_level(int_fast16_t pin, bool level)
    {
      lgfx::pinMode(pin, lgfx::pin_mode_t::output);
      if (level) lgfx::gpio_hi(pin);
      else       lgfx::gpio_lo(pin);
    }

    static void _pin_reset(int_fast16_t pin, bool use_reset)
    {
      _pin_level(pin, true);
      if (!use_reset) return;
      lgfx::gpio_lo(pin);
      auto time = lgfx::millis();
      do
      {
        lgfx::delay(1);
      } while (lgfx::millis() - time < 2);
      lgfx::gpio_hi(pin);
      time = lgfx::millis();
      do
      {
        lgfx::delay(1);
      } while (lgfx::millis() - time < 10);
    }

    static uint32_t _read_panel_id(lgfx::Bus_SPI* bus, int_fast16_t pin_cs, uint32_t cmd = 0x04, uint8_t dummy_read_bit = 1) // 0x04 = RDDID command
    {
      bus->beginTransaction();
      _pin_level(pin_cs, false);
      bus->writeCommand(cmd, 8);
//      if (dummy_read_bit) bus->writeData(0, dummy_read_bit);  // dummy read bit
      bus->beginRead();
      uint32_t res = bus->readData(32);
      bus->endTransaction();
      _pin_level(pin_cs, true);

//      Serial.printf("[Autodetect] read cmd:%02x = %08x\r\n", cmd, res);
      return res;
    }

    bool init_impl(bool use_reset, bool use_clear)
    {
      board_t board = board_t::board_unknown;

#if defined ( ARDUINO_WIO_TERMINAL )
      board = lgfx::board_WioTerminal;
#endif

      int retry = 4;
      do
      {
        if (retry == 1) use_reset = true;
        board = autodetect(use_reset, board);
      } while (board_t::board_unknown == board && --retry >= 0);

      _board = board;

      /// autodetectの際にreset済みなのでここではuse_resetをfalseで呼び出す。
      return LGFX_Device::init_impl(false, use_clear);
    }

  public:

    board_t autodetect(bool use_reset = true, board_t board = board_t::board_unknown)
    {
      auto bus_cfg = _bus_spi.config();

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
//    if (_touch_last)
//    {
//      delete _touch_last;
//      _touch_last = nullptr;
//    }

      bus_cfg.sercom_clkfreq = F_CPU;
      bus_cfg.sercom_clksrc = 0;
      bus_cfg.freq_write = 8000000;
      bus_cfg.freq_read  = 8000000;
      bus_cfg.spi_mode = 0;

      uint32_t id;
      (void)id;  // suppress warning

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_SEEED_XIAO_EXPANSION )

      if (board == 0) // || board == board_t::board_SeeedXIAOExpansion)
      {
        // board = board_t::board_SeeedXIAOExpansion;
/////
      }

#endif

// HalloWing M0 screen is write-only, no LGFX_AUTODETECT
#if defined ( LGFX_HALLOWING_M0 )

      if (board == 0 || board == board_t::board_HalloWingM0)
      {
        _pin_reset(samd21::PORT_A | 27, use_reset);
        bus_cfg.sercom_index = 5;
        bus_cfg.pin_mosi  = samd21::PORT_B | 22;
        bus_cfg.pin_miso  = -1;
        bus_cfg.pin_sclk  = samd21::PORT_B | 23;
        bus_cfg.pin_dc    = samd21::PORT_A | 28;
        bus_cfg.freq_write = 24000000;
        _bus_spi.config(bus_cfg);
        board = board_t::board_HalloWingM0;
        auto p = new lgfx::Panel_ST7735S();
        _panel_last = p;
        {
          auto cfg = p->config();
          cfg.pin_cs  = samd21::PORT_A |  1;
          cfg.pin_rst = samd21::PORT_A | 27;
          cfg.panel_width  = 128;
          cfg.panel_height = 128;
          cfg.memory_width = 128;
          cfg.memory_height = 128;
          cfg.offset_x = 2;
          cfg.offset_y = 1;
          cfg.readable = false;
          cfg.rgb_order = false;
          cfg.invert = false;
          p->config(cfg);
        }
        p->setBus(&_bus_spi);
        {
          auto l = new Light_TCC();
          auto cfg = l->config();
          cfg.pin = samd21::PORT_A | 0;
          cfg.tcc_index = 2;
          cfg.cc_index = 0;
          l->config(cfg);
          p->setLight(l);
          _light_last = l;
        }

        goto init_clear;
      }

#endif // end LGFX_HALLOWING_M0

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
