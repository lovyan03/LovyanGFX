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

  struct Panel_ILI9163 : public Panel_LCD
  {
    Panel_ILI9163(void)
    {
// The ILI9163 is available in six resolutions, 132x162 / 128x128 / 120x160 / 128x160 / 130x130 / 132x132, depending on the mode select pin status.
      _cfg.memory_width  = _cfg.panel_width  = 132;
      _cfg.memory_height = _cfg.panel_height = 162;
    }

  protected:

    static constexpr uint8_t CMD_FRMCTR1 = 0xB1;
    static constexpr uint8_t CMD_FRMCTR2 = 0xB2;
    static constexpr uint8_t CMD_FRMCTR3 = 0xB3;
    static constexpr uint8_t CMD_INVCTR  = 0xB4;
    static constexpr uint8_t CMD_DFUNCTR = 0xB6;
    static constexpr uint8_t CMD_PWCTR1  = 0xC0;
    static constexpr uint8_t CMD_PWCTR2  = 0xC1;
    static constexpr uint8_t CMD_PWCTR3  = 0xC2;
    static constexpr uint8_t CMD_PWCTR4  = 0xC3;
    static constexpr uint8_t CMD_PWCTR5  = 0xC4;
    static constexpr uint8_t CMD_VMCTR1  = 0xC5;
    static constexpr uint8_t CMD_VMCTR2  = 0xC7;
    static constexpr uint8_t CMD_GMCTRP1 = 0xE0; // Positive Gamma Correction (E0h)
    static constexpr uint8_t CMD_GMCTRN1 = 0xE1; // Negative Gamma Correction (E1h)

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
        CMD_FRMCTR1, 2, 0x08, 0x08, // Frame rate control 1
        CMD_INVCTR , 1, 0x07,       // Display inversion
        CMD_PWCTR1 , 2, 0x0A, 0x02, // Power control 1
        CMD_PWCTR2 , 1, 0x02,       // Power control 2
        CMD_VMCTR1 , 2, 0x50, 0x5B, // Vcom control 1
        CMD_VMCTR2 , 1, 0x40,       // Vcom offset

        CMD_GAMMASET,1, 0x04, // Set Gamma curve 3
        0xF2       , 1, 0x01, // Gamma adjustment enabled
        CMD_GMCTRP1,15, 0x3F, 0x25, 0x1C, 0x1E, 0x20, 0x12, 0x2A, 0x90,
                        0x24, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, // Positive Gamma
        CMD_GMCTRN1,15, 0x20, 0x20, 0x20, 0x20, 0x05, 0x00, 0x15,0xA7,
                        0x3D, 0x18, 0x25, 0x2A, 0x2B, 0x2B, 0x3A, // Negative Gamma
        CMD_SLPOUT , 0+CMD_INIT_DELAY, 120,    // Exit sleep mode
        CMD_IDMOFF , 0,
        CMD_DISPON , 0+CMD_INIT_DELAY, 100,
        0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------
 }
}
