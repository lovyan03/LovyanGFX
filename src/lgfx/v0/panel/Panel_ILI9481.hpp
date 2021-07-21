#ifndef LGFX_PANEL_ILI9481_HPP_
#define LGFX_PANEL_ILI9481_HPP_

#include "PanelIlitekCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_ILI9481 : public PanelIlitekCommon
  {
    Panel_ILI9481(void)
    {
      panel_width  = memory_width  = 320;
      panel_height = memory_height = 480;

      freq_write = 16000000;
      freq_read  = 16000000;
      freq_fill  = 16000000;
    }

  protected:

    struct CMD : public CommandCommon
    {
      static constexpr uint8_t PNLDRV = 0xC0;
      static constexpr uint8_t FRMCTR = 0xC5;
      static constexpr uint8_t IFCTR  = 0xC6;
      static constexpr uint8_t PWSET  = 0xD0;
      static constexpr uint8_t VMCTR  = 0xD1;
      static constexpr uint8_t PWSETN = 0xD2;
      static constexpr uint8_t GMCTR  = 0xC8;
    };
    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] = {
          CMD::SLPOUT ,  CMD_INIT_DELAY, 130,    // Exit sleep mode
          CMD::PWSET  , 3, 0x07, 0x41, 0x1D,
          CMD::VMCTR  , 3, 0x00, 0x1C, 0x1F,
          CMD::PWSETN , 2, 0x01, 0x11,
          CMD::PNLDRV , 5, 0x10, 0x3B, 0x00, 0x02, 0x11,
          CMD::FRMCTR , 1, 0x03,
          CMD::IFCTR  , 1, 0x83,
          CMD::GMCTR  ,12, 0x00, 0x26, 0x21, 0x00, 0x00, 0x1F,
                           0x65, 0x23, 0x77, 0x00, 0x0F, 0x00,
          0xB0        , 1, 0x00,  // CommandAccessProtect
          0xE4        , 1, 0xA0,
          0xF0        , 1, 0x01,
          
          CMD::DISPON, 0,     // Set display on
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }

    uint8_t getMadCtl(uint8_t r) const override
    {
      static constexpr uint8_t madctl_table[] = {
               MAD_HF       ,
        MAD_MV              ,
                      MAD_VF,
        MAD_MV|MAD_HF|MAD_VF,
               MAD_HF|MAD_VF,
        MAD_MV|MAD_HF       ,
                           0,
        MAD_MV|       MAD_VF,
      };
      return madctl_table[r];
    }

    color_depth_t getAdjustBpp(color_depth_t) const override { return rgb888_3Byte; }
  };

//----------------------------------------------------------------------------
 }
}

#endif
