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

  struct Panel_R61529  : public Panel_LCD
  {
    Panel_R61529(void)
    {
      _cfg.panel_width = _cfg.memory_width = 320;
      _cfg.panel_height = _cfg.memory_height = 480;
    }

  protected:

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CMD_SLPOUT , CMD_INIT_DELAY, 120,
          /// Manufacturer Command Access Protect 
          0xB0,  1, 0x00,

          /// Frame Memory Access and Interface Setting
          0xB3,  4, 0x02,
                    0x00,
                    0x00,
                    0x00,

          /// Panel Driving Setting
          0xC0,  8, 0x03, // Output polarity is inverted. Left/right interchanging scan. Forward scan. BGR mode (depends on other settings). S960 → S1 (depends)
                    0xDF, // Number of lines for driver to drive - 480.
                    0x40, // Scan start position - Gate1. (depend on other param);
                    0x10, // Dot inversion. Dot inversion in not-lit display area. If 0x13 - both will be set to 'column inversion'.
                    0x00, // settings for non-lit display area...
                    0x01, // 3 frame scan interval in non-display area...
                    0x00, // Source output level in retrace period...
                    0x55, //54 . Internal clock divider = 5 (low and high periods).

          /// Gamma setting
          0xC8, 24, 0x03, 0x12, 0x1A, 0x24,
                    0x32, 0x4B, 0x3B, 0x29,
                    0x1F, 0x18, 0x12, 0x04,
                    0x03, 0x12, 0x1A, 0x24,
                    0x32, 0x4B, 0x3B, 0x29,
                    0x1F, 0x18, 0x12, 0x04,
          0xC9, 24, 0x03, 0x12, 0x1A, 0x24,
                    0x32, 0x4B, 0x3B, 0x29,
                    0x1F, 0x18, 0x12, 0x04,
                    0x03, 0x12, 0x1A, 0x24,
                    0x32, 0x4B, 0x3B, 0x29,
                    0x1F, 0x18, 0x12, 0x04,
          0xCA, 24, 0x03, 0x12, 0x1A, 0x24,
                    0x32, 0x4B, 0x3B, 0x29,
                    0x1F, 0x18, 0x12, 0x04,
                    0x03, 0x12, 0x1A, 0x24,
                    0x32, 0x4B, 0x3B, 0x29,
                    0x1F, 0x18, 0x12, 0x04,

          /// Manufacturer Command Access Protect 
          0xB0,  1, 0x03,

          CMD_IDMOFF, 0,
          CMD_DISPON, 0,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------
 }
}
