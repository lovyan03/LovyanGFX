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

 #if defined ( ARDUINO_PYBADGE_M4 ) || defined ( ARDUINO_PYBADGE_AIRLIFT_M4 )
  #define LGFX_PYBADGE
 #endif

 #if defined ( ARDUINO_WIO_TERMINAL )
  #define LGFX_WIO_TERMINAL
 #endif

 #if defined ( ARDUINO_HALLOWING_M4 )
  #define LGFX_HALLOWING_M4
 #endif

 #if defined ( ARDUINO_PYGAMER_M4 )
  #define LGFX_PYGAMER
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
    };

    config_t config(void) const { return _cfg; }
    void config(const config_t& config) { _cfg = config; }

    bool init(uint8_t brightness) override
    {
      uint8_t pchctrl_index = 26;
      switch (_cfg.tc_index)
      {
      case 0: MCLK->APBAMASK.bit.TC0_ = 1; _tc = TC0; pchctrl_index =  9; break;
      case 1: MCLK->APBAMASK.bit.TC1_ = 1; _tc = TC1; pchctrl_index =  9; break;
      case 2: MCLK->APBBMASK.bit.TC2_ = 1; _tc = TC2; pchctrl_index = 26; break;
      case 3: MCLK->APBBMASK.bit.TC3_ = 1; _tc = TC3; pchctrl_index = 26; break;
      case 4: MCLK->APBCMASK.bit.TC4_ = 1; _tc = TC4; pchctrl_index = 30; break;
      case 5: MCLK->APBCMASK.bit.TC5_ = 1; _tc = TC5; pchctrl_index = 30; break;
#if defined ( TC6 )
      case 6: MCLK->APBDMASK.bit.TC6_ = 1; _tc = TC6; pchctrl_index = 39; break;
#endif
#if defined ( TC7 )
      case 7: MCLK->APBDMASK.bit.TC7_ = 1; _tc = TC7; pchctrl_index = 39; break;
#endif
      default: return false;
      }

      /* Enable Peripheral Clocks */
      GCLK->PCHCTRL[pchctrl_index].reg = 0 | (1u << 6);
      while (!GCLK->PCHCTRL[pchctrl_index].bit.CHEN);

      /* Configure _tc */
      _tc->COUNT8.CTRLA.reg = (1u << 0);   // SWRST;
      while ( _tc->COUNT8.SYNCBUSY.bit.SWRST );
      _tc->COUNT8.CTRLA.reg = (0x01 << 2) | (0x01 << 4) | (0x04 << 8);   // MODE=COUNT8, PRESCALER=DIV16, PRESCSYNC=PRESC
      _tc->COUNT8.WAVE.reg  = 0x02; // WAVEGEN=NPWM;
      _tc->COUNT8.CTRLBSET.reg = (1u<<1); // LUPD
      _tc->COUNT8.PER.reg = 255;  // max brightness;
      _tc->COUNT8.CC[_cfg.cc_index].reg = brightness;
      _tc->COUNT8.DBGCTRL.bit.DBGRUN = 1;
      _tc->COUNT8.INTFLAG.reg = 0x33;    // Clear all flags
      while ( _tc->COUNT8.SYNCBUSY.reg );
      _tc->COUNT8.CTRLA.bit.ENABLE = 1;   // ENABLE
      while ( _tc->COUNT8.SYNCBUSY.bit.ENABLE );

      /* Configure PORT */
      lgfx::pinMode( _cfg.pin, pin_mode_t::output );
      lgfx::pinAssignPeriph( _cfg.pin, 4 ); // 4 = periph E ( PIO_TIMER )

      return true;
    }

    void setBrightness(uint8_t brightness) override
    {
      if (_tc) _tc->COUNT8.CC[_cfg.cc_index].reg = brightness;
    }

  protected:
    config_t _cfg;
    Tc* _tc = nullptr;
  };

  struct Light_WioTerminal : public Light_TC
  {
    bool init(uint8_t brightness) override
    {
      _cfg.pin = samd51::PORT_C | 5;
      _cfg.tc_index = 0;
      _cfg.cc_index = 0;
      if (!Light_TC::init(brightness))
      {
        return false;
      }

      /* Enable Peripheral Clocks */
      GCLK->PCHCTRL[11].reg = 0 | (1u<<6);    // EVSYS[0]
      while (!GCLK->PCHCTRL[11].bit.CHEN);
      GCLK->PCHCTRL[33].reg = 0 | (1u<<6);    // CCL
      while (!GCLK->PCHCTRL[33].bit.CHEN);

      /* Enable Peripheral APB Clocks */
      MCLK->APBBMASK.bit.EVSYS_ = 1;
      MCLK->APBCMASK.bit.CCL_ = 1;

      /* Configure EVSYS */
      EVSYS->USER[1].reg = 0x01;  // Channel0 -> PORT_EV0
      EVSYS->Channel[0].CHANNEL.reg = 0x74 | (0x02<<8) | (0x00<<10);  // CCL_LUTOUT0, ASYNCHRONOUS, NO_EVT_OUTPUT
      /* Configure CCL */
      CCL->CTRL.reg = (1<<0); // SWRST
      CCL->SEQCTRL[0].reg = 0x0; // Disable SEQCTRL
      CCL->LUTCTRL[0].reg = (0xaau << 24) | (1u<<22) | (0x6<<8) | (1<<1); // TRUTH=0xaa, LUTEO, INSEL0=0x06(TC), ENABLE
      CCL->CTRL.reg = (1<<1); // ENABLE

      /* Configure PORT */
      lgfx::pinMode( _cfg.pin, pin_mode_t::output );
      PORT->Group[ _cfg.pin >> samd51::PORT_SHIFT ].EVCTRL.reg = 0x80 | ( _cfg.pin & samd51::PIN_MASK ); // PC05, OUT

      return true;
    }
  };

  class LGFX : public LGFX_Device
  {
    lgfx::Panel_Device* _panel_last = nullptr;
    lgfx::ILight* _light_last = nullptr;
    lgfx::ITouch* _touch_last = nullptr;
    lgfx::Bus_SPI _bus_spi;

    static void _pin_level(int_fast16_t pin, bool level)
    {
      lgfx::pinMode(pin, lgfx::pin_mode_t::output);
      if (level) lgfx::gpio_hi(pin);
      else       lgfx::gpio_lo(pin);
    }

    static bool _pin_read(int_fast16_t pin)
    {
      lgfx::pinMode(pin, lgfx::pin_mode_t::input);
      return lgfx::gpio_in(pin);
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
      bus->beginRead(dummy_read_bit);  // dummy read bit
      uint32_t res = bus->readData(32);
      bus->endRead();
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

      /// autodetectの際にreset済みなのでここではuse_resetをfalseで呼び出す。;
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
      if (_touch_last)
      {
        delete _touch_last;
        _touch_last = nullptr;
      }

      bus_cfg.sercom_clkfreq = F_CPU;
      bus_cfg.sercom_clksrc = 0;
      bus_cfg.freq_write = 8000000;
      bus_cfg.freq_read  = 8000000;
      bus_cfg.spi_mode = 0;

      uint32_t id;
      (void)id;  // suppress warning

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_FEATHER_M4_HX8357 )

      if (board == 0 || board == board_t::board_FeatherM4_HX8357)
      {
        _pin_level(samd51::PORT_A | 19, true);
        bus_cfg.sercom_index = 1;
        bus_cfg.pin_mosi  = samd51::PORT_B | 23;
        bus_cfg.pin_miso  = samd51::PORT_B | 22;
        bus_cfg.pin_sclk  = samd51::PORT_A | 17;
        bus_cfg.pin_dc    = samd51::PORT_A | 20;
        _bus_spi.config(bus_cfg);
        if (_bus_spi.init())
        {
          _bus_spi.beginTransaction();
          _pin_level(samd51::PORT_A | 19, false);
          _bus_spi.writeCommand(0xB9, 8);    // SETEXTC
          _bus_spi.writeData(0x005783FF, 24);
          _bus_spi.writeCommand(0xB3, 8);
          _bus_spi.writeData(0x06060080, 32); // SDO Enabled
          _bus_spi.endTransaction();
          _pin_level(samd51::PORT_A | 19, true);

          id = _read_panel_id(&_bus_spi, samd51::PORT_A | 19);
          if ((id & 0xFF) == 0 && _read_panel_id(&_bus_spi, samd51::PORT_A | 19, 0x09) != 0)
          {
            board = board_t::board_FeatherM4_HX8357;
            _bus_spi.release();
            bus_cfg.freq_write = 39000000;
            bus_cfg.freq_read  = 12000000;
            _bus_spi.config(bus_cfg);

            auto p = new lgfx::Panel_HX8357D();
            _panel_last = p;
            {
              auto cfg = p->config();
              cfg.pin_cs  = samd51::PORT_A | 19;
              cfg.pin_rst = -1;
              p->config(cfg);
            }
            p->setRotation(1);
            p->setBus(&_bus_spi);

            auto t = new lgfx::Touch_STMPE610();
            _touch_last = t;
            {
              auto cfg = t->config();
              cfg.spi_host = 1;   // SERCOM 1
              cfg.freq = 2000000;
              cfg.pin_sclk = samd51::PORT_A | 17;
              cfg.pin_mosi = samd51::PORT_B | 23;
              cfg.pin_miso = samd51::PORT_B | 22;
              cfg.pin_cs   = samd51::PORT_A | 18;
              cfg.offset_rotation = 0;
              cfg.bus_shared = true;
              t->config(cfg);
              p->setTouch(t);
            }

            goto init_clear;
          }
          _bus_spi.release();
        }
      }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_WIO_TERMINAL )

      if (board == 0 || board == board_t::board_WioTerminal)
      {
        lgfx::pinMode(samd51::PORT_C | 7, lgfx::pin_mode_t::input_pulldown); // LCD RST
        bus_cfg.sercom_index = 7;
        bus_cfg.pin_mosi  = samd51::PORT_B | 19;
        bus_cfg.pin_miso  = samd51::PORT_B | 18;
        bus_cfg.pin_sclk  = samd51::PORT_B | 20;
        bus_cfg.pin_dc    = samd51::PORT_C |  6;
        if (lgfx::gpio_in(samd51::PORT_C | 7))
        {
          _pin_level(samd51::PORT_B | 21, true);
          _pin_reset(samd51::PORT_C | 7, use_reset); // LCD RST

          _bus_spi.config(bus_cfg);   // 設定を反映する;
          if (_bus_spi.init())
          {
            id = _read_panel_id(&_bus_spi, samd51::PORT_B | 21);
            if ((id & 0xFF) == 0 && _read_panel_id(&_bus_spi, samd51::PORT_B | 21, 0x0C) != 0)
            { // check panel (ILI9341) panelIDが0なのでReadDisplayPixelFormat 0x0Cを併用する;
              board = board_t::board_WioTerminal;
              _bus_spi.release();
              bus_cfg.freq_write = 60000000;
              bus_cfg.freq_read  = 20000000;
              _bus_spi.config(bus_cfg);

              auto p = new lgfx::Panel_ILI9341();
              _panel_last = p;
              {
                auto cfg = p->config();
                cfg.pin_cs  = samd51::PORT_B | 21;
                cfg.pin_rst = samd51::PORT_C |  7;
                p->config(cfg);
              }
              p->setRotation(1);
              p->setBus(&_bus_spi);
              _light_last = new Light_WioTerminal();
              p->setLight(_light_last);
              goto init_clear;
            }
            _bus_spi.release();
          }
        }
      }
#endif

// pybadge はLCDからの読出しが出来ないため、無条件設定になる。;
// そのため、LGFX_AUTODETECTでは対応しないようにしておく。;
#if defined ( LGFX_PYBADGE )

      if (board == 0 || board == board_t::board_PyBadge)
      {
        bus_cfg.sercom_index = 4;
        bus_cfg.pin_mosi     = samd51::PORT_B | 15;
        bus_cfg.pin_miso     = -1;
        bus_cfg.pin_sclk     = samd51::PORT_B | 13;
        bus_cfg.pin_dc       = samd51::PORT_B |  5;
        bus_cfg.freq_write   = 27000000;
        _pin_reset(samd51::PORT_A | 0, use_reset); // LCD RST
        board = board_t::board_PyBadge;
        _bus_spi.config(bus_cfg);
        auto p = new lgfx::Panel_ST7735S();
        _panel_last = p;
        {
          auto cfg = p->config();
          cfg.pin_cs  = samd51::PORT_B | 7;
          cfg.pin_rst = samd51::PORT_A | 0;
          cfg.panel_width  = 128;
          cfg.panel_height = 160;
          cfg.memory_width = 128;
          cfg.memory_height = 160;
          cfg.readable = false;
          cfg.rgb_order = true;
          cfg.offset_rotation = 3;
          p->config(cfg);
        }
        p->setBus(&_bus_spi);
        {
          auto l = new Light_TC();
          auto cfg = l->config();
          cfg.pin = samd51::PORT_A | 1;
          cfg.tc_index = 2;
          cfg.cc_index = 1;
          l->config(cfg);
          p->setLight(l);
          _light_last = l;
        }

        goto init_clear;
      }

#endif

// PyGamer screen is write-only, no LGFX_AUTODETECT
#if defined ( LGFX_PYGAMER )

      if (board == 0 || board == board_t::board_PyGamer)
      {
        bus_cfg.sercom_index = 4;
        bus_cfg.pin_mosi     = samd51::PORT_B | 15;
        bus_cfg.pin_miso     = -1;
        bus_cfg.pin_sclk     = samd51::PORT_B | 13;
        bus_cfg.pin_dc       = samd51::PORT_B |  5;
        bus_cfg.freq_write   = 27000000;
        _pin_reset(samd51::PORT_A | 0, use_reset); // LCD RST
        board = board_t::board_PyGamer;
        _bus_spi.config(bus_cfg);
        auto p = new lgfx::Panel_ST7735S();
        _panel_last = p;
        {
          auto cfg = p->config();
          cfg.pin_cs  = samd51::PORT_B | 12; // The only diff vs PyBadge TFT
          cfg.pin_rst = samd51::PORT_A | 0;
          cfg.panel_width  = 128;
          cfg.panel_height = 160;
          cfg.memory_width = 128;
          cfg.memory_height = 160;
          cfg.readable = false;
          cfg.rgb_order = true;
          cfg.offset_rotation = 3;
          p->config(cfg);
        }
        p->setBus(&_bus_spi);
        {
          auto l = new Light_TC();
          auto cfg = l->config();
          cfg.pin = samd51::PORT_A | 1;
          cfg.tc_index = 2;
          cfg.cc_index = 1;
          l->config(cfg);
          p->setLight(l);
          _light_last = l;
        }

        goto init_clear;
      }

#endif // end LGFX_PYGAMER

// HalloWing M4 screen is write-only, no LGFX_AUTODETECT
#if defined ( LGFX_HALLOWING_M4 )

      if (board == 0 || board == board_t::board_HalloWingM4)
      {
        _pin_reset(samd51::PORT_B | 30, use_reset);
        bus_cfg.sercom_index = 1;
        bus_cfg.pin_mosi  = samd51::PORT_A |  0;
        bus_cfg.pin_miso  = -1;
        bus_cfg.pin_sclk  = samd51::PORT_A |  1;
        bus_cfg.pin_dc    = samd51::PORT_B | 31;
        bus_cfg.freq_write = 50000000;
        _bus_spi.config(bus_cfg);
        board = board_t::board_HalloWingM4;
        auto p = new lgfx::Panel_ST7789();
        _panel_last = p;
        {
          auto cfg = p->config();
          cfg.pin_cs  = samd51::PORT_A | 27;
          cfg.pin_rst = samd51::PORT_B | 30;
          cfg.panel_width  = 240;
          cfg.panel_height = 240;
          cfg.readable = false;
          cfg.rgb_order = false;
          cfg.invert = true;
          cfg.offset_rotation = 2;
          p->config(cfg);
        }
        p->setBus(&_bus_spi);
        {
          auto l = new Light_TC();
          auto cfg = l->config();
          cfg.pin = samd51::PORT_B | 14;
          cfg.tc_index = 5;
          cfg.cc_index = 0;
          l->config(cfg);
          p->setLight(l);
          _light_last = l;
        }

        goto init_clear;
      }

#endif // end LGFX_HALLOWING_M4

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
