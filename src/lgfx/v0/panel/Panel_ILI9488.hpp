#ifndef LGFX_PANEL_ILI9488_HPP_
#define LGFX_PANEL_ILI9488_HPP_

#include "PanelIlitekCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_ILI9488 : public PanelIlitekCommon
  {
    Panel_ILI9488(void)
    {
      panel_width  = memory_width  = 320;
      panel_height = memory_height = 480;

      freq_write = 40000000;
      freq_read  = 16000000;
      freq_fill  = 40000000;
      read_depth  = rgb888_3Byte;
      write_depth = rgb888_3Byte;
    }

  protected:

    struct CMD : public CommandCommon
    {
      static constexpr uint8_t FRMCTR1 = 0xB1;
      static constexpr uint8_t FRMCTR2 = 0xB2;
      static constexpr uint8_t FRMCTR3 = 0xB3;
      static constexpr uint8_t INVCTR  = 0xB4;
      static constexpr uint8_t DFUNCTR = 0xB6;
      static constexpr uint8_t ETMOD   = 0xB7;
      static constexpr uint8_t PWCTR1  = 0xC0;
      static constexpr uint8_t PWCTR2  = 0xC1;
      static constexpr uint8_t PWCTR3  = 0xC2;
      static constexpr uint8_t PWCTR4  = 0xC3;
      static constexpr uint8_t PWCTR5  = 0xC4;
      static constexpr uint8_t VMCTR   = 0xC5;
      static constexpr uint8_t GMCTRP1 = 0xE0; // Positive Gamma Correction
      static constexpr uint8_t GMCTRN1 = 0xE1; // Negative Gamma Correction
      static constexpr uint8_t ADJCTL3 = 0xF7;
    };

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {

          CMD::PWCTR1,  2, 0x17,  // VRH1
                           0x15,  // VRH2
          CMD::PWCTR2,  1, 0x41,  // VGH, VGL
          CMD::VMCTR ,  3, 0x00,  // nVM
                           0x12,  // VCM_REG
                           0x80,  // VCM_REG_EN
          CMD::COLMOD,  1, 0x66,  // Interface Pixel Format = 18bit
          CMD::FRMCTR1, 1, 0xA0,  // Frame rate = 60Hz
          CMD::INVCTR,  1, 0x02,  // Display Inversion Control = 2dot
          CMD::DFUNCTR, 2, 0x02,  // Nomal scan
                           0x02,  // 5 frames
          CMD::ETMOD,   1, 0xC6,  // 
          CMD::ADJCTL3, 4, 0xA9,  // Adjust Control 3 
                           0x51,
                           0x2C,
                           0x82,

//          CMD::GMCTRP1,15, 0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78,
//                           0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F, 
//          CMD::GMCTRN1,15, 0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45,
//                           0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F, 

          0xFF,0xFF, // end
      };
      static constexpr uint8_t list1[] = {
          CMD::SLPOUT,  CMD_INIT_DELAY, 120,    // Exit sleep mode
          CMD::DISPON,  CMD_INIT_DELAY,  20,    // Set display on
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      case 1: return list1;
      default: return nullptr;
      }
    }
    uint8_t getMadCtl(uint8_t r) const override {
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

    color_depth_t getAdjustBpp(color_depth_t) const override { return rgb888_3Byte; }
  };

//----------------------------------------------------------------------------
 }
}

#endif
