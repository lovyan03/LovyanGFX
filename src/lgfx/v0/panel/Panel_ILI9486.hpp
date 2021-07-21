#ifndef LGFX_PANEL_ILI9486_HPP_
#define LGFX_PANEL_ILI9486_HPP_

#include "PanelIlitekCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_ILI9486 : public PanelIlitekCommon
  {
    Panel_ILI9486(void)
    {
      panel_width  = memory_width  = 320;
      panel_height = memory_height = 480;

      freq_write = 27000000;
      freq_read  = 16000000;
      freq_fill  = 27000000;
    }

  protected:

    struct CMD : public CommandCommon
    {
      static constexpr uint8_t FRMCTR1 = 0xB1;
      static constexpr uint8_t FRMCTR2 = 0xB2;
      static constexpr uint8_t FRMCTR3 = 0xB3;
      static constexpr uint8_t INVCTR  = 0xB4;
      static constexpr uint8_t DFUNCTR = 0xB6;
      static constexpr uint8_t PWCTR1  = 0xC0;
      static constexpr uint8_t PWCTR2  = 0xC1;
      static constexpr uint8_t PWCTR3  = 0xC2;
      static constexpr uint8_t PWCTR4  = 0xC3;
      static constexpr uint8_t PWCTR5  = 0xC4;
      static constexpr uint8_t VMCTR1  = 0xC5;
      static constexpr uint8_t VMCTR2  = 0xC7;
      static constexpr uint8_t GMCTRP1 = 0xE0; // Positive Gamma Correction
      static constexpr uint8_t GMCTRN1 = 0xE1; // Negative Gamma Correction
    };
    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] = {
          CMD::SLPOUT,  CMD_INIT_DELAY, 5,    // Exit sleep mode
          CMD::PWCTR3 , 1, 0x44,       // Power control 3
          CMD::VMCTR1 , 4, 0x00, 0x00, 0x00, 0x00,

          CMD::GMCTRP1,15, 0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98,
                           0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00,

          CMD::GMCTRN1,15, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
                           0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
          0xFF,0xFF, // end
      };
      static constexpr uint8_t list1[] = {
          CMD::DISPON, 0,     // Set display on
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      case 1: return list1;
      default: return nullptr;
      }
    }

    uint8_t getMadCtl(uint8_t r) const override
    {
      static constexpr uint8_t madctl_table[] = {
               MAD_MX|MAD_MH              ,
        MAD_MV                            ,
                             MAD_MY|MAD_ML,
        MAD_MV|MAD_MX|MAD_MY|MAD_MH|MAD_ML,
               MAD_MX|MAD_MH|MAD_MY|MAD_ML,
        MAD_MV|MAD_MX|MAD_MH              ,
                                         0,
        MAD_MV|              MAD_MY|MAD_ML,
      };
      return madctl_table[r];
    }

    const uint8_t* getColorDepthCommands(uint8_t* buf, color_depth_t depth) override
    {
      auto res = PanelIlitekCommon::getColorDepthCommands(buf, depth);
      // ILI9486 読込時のデータは書込と同じ並びになる (パラレル接続ILI9486で確認)
      read_depth = write_depth;
      return res;
    }
  };

  struct Panel_ILI9486L : public Panel_ILI9486
  {
    color_depth_t getAdjustBpp(color_depth_t) const override { return rgb888_3Byte; }
  };

//----------------------------------------------------------------------------
 }
}

#endif
