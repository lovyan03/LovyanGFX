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

#include "Panel_LCD.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_SSD1963 : public Panel_LCD
  {
    struct timing_params_t
    {
      uint32_t xtal_clock = 10000000;  /// Oscillator frequency on board;
      uint32_t pll_clock = 120000000;
      uint8_t refresh_rate = 60;

      /// HSYNC parameter
      uint16_t h_branking_total = 128;
      uint8_t  hpw = 48;
      uint16_t hps = 46;
      uint16_t lps = 15;
      uint8_t  lpspp = 0;

      /// VSYNC parameter
      uint16_t v_branking_total = 45;
      uint8_t  vpw = 16;
      uint16_t vps = 16;
      uint16_t fps = 8;
    };

    Panel_SSD1963(void)
    {
      _cfg.memory_width  = _cfg.panel_width  = 864;
      _cfg.memory_height = _cfg.panel_height = 480;
      _write_depth = rgb565_2Byte;
      _read_depth  = rgb565_2Byte;
    }

    void setHSync(uint_fast16_t front, uint_fast16_t sync, uint_fast16_t back, uint_fast16_t move = 0, uint_fast16_t lpspp = 0);
    void setVSync(uint_fast16_t front, uint_fast16_t sync, uint_fast16_t back, uint_fast16_t move = 0);

    bool init(bool use_reset) override;
    void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override;

    void setRotation(uint_fast8_t r) override;
    color_depth_t setColorDepth(color_depth_t depth) override;

    const timing_params_t& config_timing_params(void) const { return _timing_params; }
    void config_timing_params(const timing_params_t& timing_params);

  protected:

    timing_params_t _timing_params;

/*
    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
        //PLL multiplier, set PLL clock to 120M
        0xE2, 3, 0x23, 0x02, 0x54,
              //N=0x36 for 6.5M, 0x23 for 10M crystal
        0xE0, 1+CMD_INIT_DELAY, 0x01, 1,  // PLL enable
        0xE0, 1               , 0x03,
        0x01, 0+CMD_INIT_DELAY, 10,  // software reset

        0xE6, 3, 0x03, 0x33, 0x33,   //PLL setting for PCLK
        0xB0, 7, 0x20, 0x00, 0x03, 0x5F, 0x01, 0xDF, 0x2D,   //LCD SPECIFICATION
        0xB4, 8, 0x03, 0xA0, 0x00, 0x2E, 0x30, 0x00, 0x0F, 0x00,    //HSYNC
        0xB6, 7, 0x02, 0x0D, 0x00, 0x10, 0x10, 0x00, 0x08,          //VSYNC
        
        0xBA, 1, 0x0F,    //GPIO[3:0] out 1

        0xB8, 2, 0x07, 0x01,      //GPIO3=input, GPIO[2:0]=output  GPIO0 normal

        0xB8, 2, 0x0F, 0x01,    //GPIO is controlled by host GPIO[3:0]=output   GPIO[0]=1  LCD ON  GPIO[0]=1  LCD OFF GPIO0 normal

        0xBA, 1, 0x01,     //GPIO[0] out 1 --- LCD display on/off control PIN

        0x29, 0,      //display on

        0xBE, 6, 0x06, 0xf0, 0x01, 0xf0, 0x00, 0x00,   //set PWM for B/L
        0xD0, 1, 0x0D,

        0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }
//*/
  };

//----------------------------------------------------------------------------
 }
}
