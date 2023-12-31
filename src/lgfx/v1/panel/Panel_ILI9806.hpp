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

    struct Panel_ILI9806 : public Panel_LCD
    {
      Panel_ILI9806(void)
      {
        _cfg.memory_width = _cfg.panel_width = 480;
        _cfg.memory_height = _cfg.panel_height = 854;
      }

    protected:
      static constexpr uint8_t CMD_EXTC   = 0xFF;
      static constexpr uint8_t CMD_SPISET = 0xBA;
      static constexpr uint8_t CMD_GIP1   = 0xBC;
      static constexpr uint8_t CMD_GIP2   = 0xBD;
      static constexpr uint8_t CMD_GIP3   = 0xBE;
      static constexpr uint8_t CMD_VCOM   = 0xC7;
      static constexpr uint8_t CMD_VOLT   = 0xED;
      static constexpr uint8_t CMD_PWCTR1 = 0xC0;
      static constexpr uint8_t CMD_LVGVOT = 0xFC;
      static constexpr uint8_t CMD_ENGSET = 0xDF;
      static constexpr uint8_t CMD_DVDD   = 0xF3;
      static constexpr uint8_t CMD_DSPINV = 0xB4;
      static constexpr uint8_t CMD_RESOL  = 0xF7;
      static constexpr uint8_t CMD_FPSCTL = 0xB1;
      static constexpr uint8_t CMD_TMCTR1 = 0xF1;
      static constexpr uint8_t CMD_TMCTR2 = 0xF2;
      static constexpr uint8_t CMD_PWCTR2 = 0xC1;
      static constexpr uint8_t CMD_GAMMAP = 0xE0;
      static constexpr uint8_t CMD_GAMMAN = 0xE1;
      static constexpr uint8_t CMD_TEAR   = 0x35;
      static constexpr uint8_t CMD_MACTL  = 0x36;
      static constexpr uint8_t CMD_PIXFMT = 0x3A;
      static constexpr uint8_t CMD_SLPOUT = 0x11;
      static constexpr uint8_t CMD_DISPON = 0x29;

      const uint8_t *getInitCommands(uint8_t listno) const override
      {
        static constexpr uint8_t list0[] =
            {
                CMD_EXTC, 3, 0xFF, 0x98, 0x06,
                CMD_SPISET, 1, 0xE0,
                CMD_GIP1, 21, 0x03, 0x0F, 0x63, 0x69,
                              0x01, 0x01, 0x1B, 0x11,
                              0x70, 0x73, 0xFF, 0xFF,
                              0x08, 0x09, 0x05, 0x00,
                              0xEE, 0xE2, 0x01, 0x00,
                              0xC1,
                CMD_GIP2, 8,  0x01, 0x23, 0x45, 0x67,
                              0x01, 0x23, 0x45, 0x67,
                CMD_GIP3, 9,  0x00, 0x22, 0x27, 0x6A,
                              0xBC, 0xD8, 0x92, 0x22, 0x22,
                CMD_VCOM, 1,  0x1E,
                CMD_VOLT, 3,  0x7F, 0x0F, 0x00,
                CMD_PWCTR1,3, 0xE3, 0x0B, 0x00,
                CMD_LVGVOT,1, 0x08,
                CMD_ENGSET,6, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x02,
                CMD_DVDD,1,   0x74,
                CMD_DSPINV,3, 0x00, 0x00, 0x00,
                CMD_RESOL,1,  0x81,
                CMD_FPSCTL,3, 0x00, 0x10, 0x14,
                CMD_TMCTR1,3, 0x29, 0x8A, 0x07,
                CMD_TMCTR2,4, 0x40, 0xD2, 0x50, 0x28,
                CMD_PWCTR2,4, 0x17, 0x85, 0x85, 0x20,
                CMD_GAMMAP,16,0x00, 0x0C, 0x15, 0x0D,
                              0x0F, 0x0C, 0x07, 0x05,
                              0x07, 0x0B, 0x10, 0x10,
                              0x0D, 0x17, 0x0F, 0x00,
                CMD_GAMMAN,16,0x00, 0x0D, 0x15, 0x0E,
                              0x10, 0x0D, 0x08, 0x06,
                              0x07, 0x0C, 0x11, 0x11,
                              0x0E, 0x17, 0x0F, 0x00,
                CMD_TEAR,1,   0x00,
                CMD_MACTL,1,  0x60,
                CMD_PIXFMT,1, 0x55,
                CMD_SLPOUT, 0 + CMD_INIT_DELAY, 120, // Exit sleep mode
                CMD_DISPON, 0 + CMD_INIT_DELAY, 100,
                0xFF, 0xFF, // end
            };
        switch (listno)
        {
        case 0:
          return list0;
        default:
          return nullptr;
        }
      }
    };

    //----------------------------------------------------------------------------
  }
}
