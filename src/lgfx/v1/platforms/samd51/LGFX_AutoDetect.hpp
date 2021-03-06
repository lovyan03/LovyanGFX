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

#include "../../init.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  class Light_WioTerminal : public ILight
  {
  public:
    void init(void) override
    {
      /* Enable Peripheral Clocks */
      GCLK->PCHCTRL[9].reg = 0 | (1u<<6);         // TC0, TC1
      while(!GCLK->PCHCTRL[9].bit.CHEN);
      GCLK->PCHCTRL[11].reg = 0 | (1u<<6);    // EVSYS[0]
      while(!GCLK->PCHCTRL[11].bit.CHEN);
      GCLK->PCHCTRL[33].reg = 0 | (1u<<6);    // CCL
      while(!GCLK->PCHCTRL[33].bit.CHEN);
      /* Enable Peropheral APB Clocks */
      MCLK->APBAMASK.bit.TC0_ = 1;
      MCLK->APBBMASK.bit.EVSYS_ = 1;
      MCLK->APBCMASK.bit.CCL_ = 1;

      /* Configure PORT */
      PORT->Group[2].DIRSET.reg = (1<<5);
      PORT->Group[2].EVCTRL.reg = 0x85; // PC05, OUT
      /* Configure EVSYS */
      EVSYS->USER[1].reg = 0x01;  // Channel0 -> PORT_EV0
      EVSYS->Channel[0].CHANNEL.reg = 0x74 | (0x02<<8) | (0x00<<10);  // CCL_LUTOUT0, ASYNCHRONOUS, NO_EVT_OUTPUT
      /* Configure CCL */
      CCL->CTRL.reg = (1<<0); // SWRST
      CCL->SEQCTRL[0].reg = 0x0; // Disable SEQCTRL
      CCL->LUTCTRL[0].reg = (0xaau << 24) | (1u<<22) | (0x6<<8) | (1<<1); // TRUTH=0xaa, LUTEO, INSEL0=0x06(TC), ENABLE
      CCL->CTRL.reg = (1<<1); // ENABLE

      /* Configure TC0 */
      TC0->COUNT8.CTRLA.reg = (1u<<0);   // SWRST;
      while( TC0->COUNT8.SYNCBUSY.bit.SWRST );

      TC0->COUNT8.CTRLA.reg = (0x01 << 2) | (0x01 << 4) | (0x04 << 8);   // MODE=COUNT8, PRESCALER=DIV16, PRESCSYNC=PRESC
      TC0->COUNT8.WAVE.reg  = 0x02; // WAVEGEN=NPWM;
      TC0->COUNT8.CTRLBSET.reg = (1u<<1); // LUPD
      TC0->COUNT8.PER.reg = 255;  // this->maxBrightness;
      TC0->COUNT8.CC[0].reg = _brightness;
      TC0->COUNT8.CC[1].reg = 0u;
      TC0->COUNT8.DBGCTRL.bit.DBGRUN = 1;
      TC0->COUNT8.INTFLAG.reg = 0x33;    // Clear all flags
      while( TC0->COUNT8.SYNCBUSY.reg );

      TC0->COUNT8.CTRLA.bit.ENABLE = 1;   // ENABLE
      while( TC0->COUNT8.SYNCBUSY.bit.ENABLE );
    }

    void setBrightness(std::uint8_t brightness) override
    {
      _brightness = brightness;
      TC0->COUNT8.CC[0].reg = brightness;
      while(TC0->COUNT8.SYNCBUSY.bit.CC0);
    }
  };

  struct LGFX : public LGFX_Device
  {
    LGFX(void) // コンストラクタ内で定義を行う
    {
      static lgfx::Bus_SPI _bus;
      static lgfx::Panel_ILI9341 _panel;
      static lgfx::Light_WioTerminal _light;

      _panel.bus(&_bus);   // パネルが使用するバスを指定する
      panel(&_panel);      // 使用するパネルを指定する
      light(&_light);      // 使用するバックライトを指定する

      {
        auto cfg = _bus.config(); // バスの設定値を取得
        cfg.sercom_index = 7;
        cfg.sercom_clksrc = 0;   // -1=notchange / 0=select GCLK0
        cfg.sercom_clkfreq = F_CPU;
        cfg.spi_mode = 0;
        cfg.pin_mosi  = samd51::PORT_B | 19;
        cfg.pin_miso  = samd51::PORT_B | 18;
        cfg.pin_sclk  = samd51::PORT_B | 20;
        cfg.pin_dc    = samd51::PORT_C |  6;
        cfg.freq_write = 60000000;
        cfg.freq_read  = 20000000;
        _bus.config(cfg);   // 設定を反映する
      }
      {
        auto cfg = _panel.config(); // パネルの設定値を取得
        cfg.pin_cs  = samd51::PORT_B | 21;
        cfg.pin_rst = samd51::PORT_C |  7;
        cfg.rotation = 1;
        _panel.config(cfg);  // 設定を反映する
      }
    }
  };

//----------------------------------------------------------------------------
 }
}

using LGFX = lgfx::LGFX;