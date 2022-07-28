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

  struct Panel_S6D04K1 : public Panel_LCD
  {
    Panel_S6D04K1(void)
    {
      _cfg.memory_width  = _cfg.panel_width  = 320;
      _cfg.memory_height = _cfg.panel_height = 240;
    }

  protected:

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
          0xF0      ,  2, 0x5A ,0x5A,  // PASSWD1
          0xF1      ,  2, 0x5A ,0x5A,  // PASSWD2
          0xF4      , 14, // PWRCTL
                          0x09, 0x00, 0x00, 0x00, 0x21, 0x47, 0x01,
                          0x02, 0x2A, 0x64, 0x05, 0x2A, 0x00, 0x05, 
          0xF5      , 10, // VCMCTL
                          0x00, 0x4D, 0x5C, 0x00, 0x00, 0x09, 0x00, 0x00, 0x01, 0x01, 
          0xF6      ,  9, // SRCCTL
                          0x01, 0x01, 0x03, 0x00, 0x04, 0x0C, 0x02, 0x00, 0x07, 
          0xF7      ,  4, // IFCTL
                          0x00, 0x00, 0x00, 0x00, 
          0xF2      , 12,  // DISCTL
                          0x1E, 0x95, 0x03, 0x08,
                          0x08, 0x10, 0x00, 0x19,
                          0x48, 0x00, 0x07, 0x01,
          0xF9      ,  1, // GAMMASEL
                          0x04,

          0xFA      , 12, // PGAMMACTL
                          0x0A, 0x10, 0x0B, 0x1B,
                          0x14, 0x28, 0x1F, 0x33,
                          0x2A, 0x25, 0x23, 0x00,

          0xFB      , 12, //NGAMMACTL
                          0x10, 0x0A, 0x23, 0x25,
                          0x27, 0x2C, 0x1F, 0x2E,
                          0x20, 0x1B, 0x0B, 0x00,
          0xF9      ,  1, // GAMMASEL
                   	      0x03,

          0xFA      , 12, // PGAMMACTL
                          0x0A, 0x24, 0x0B, 0x14,
                          0x1A, 0x33, 0x2D, 0x22,
                          0x17, 0x11, 0x0B, 0x00,

          0xFB      , 12, // NGAMMACTL
                          0x24, 0x0A, 0x0B, 0x11,
                          0x14, 0x1B, 0x11, 0x39,
                          0x26, 0x14, 0x0B, 0x00,

          0x35      ,  1, 0x00, // TEON

          CMD_SLPOUT , 0 + CMD_INIT_DELAY, 120,    // Exit sleep mode
        
          0xF0      ,  2, // PASSWD1
                          0xA5, 0xA5,
          CMD_IDMOFF , 0,
          CMD_DISPON , 0,
          CMD_SLPOUT , 0 + CMD_INIT_DELAY, 120,    // Exit sleep mode
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
