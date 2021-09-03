#ifndef LGFX_PANEL_ILI9341_HPP_
#define LGFX_PANEL_ILI9341_HPP_

#include "PanelIlitekCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_ILI9341 : public PanelIlitekCommon
  {
    Panel_ILI9341(void)
    {
      panel_width  = memory_width  = 240;
      panel_height = memory_height = 320;

      freq_write = 40000000;
      freq_read  = 16000000;
      freq_fill  = 40000000;
    }

  protected:

    struct CMD : public PanelIlitekCommon::CommandCommon
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
      static constexpr uint8_t GMCTRP1 = 0xE0; // Positive Gamma Correction (E0h)
      static constexpr uint8_t GMCTRN1 = 0xE1; // Negative Gamma Correction (E1h)

      static constexpr uint8_t RDINDEX = 0xD9; // ili9341
      static constexpr uint8_t IDXRD   = 0xDD; // ILI9341 only, indexed control register read
    };

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          0xEF       , 3, 0x03,0x80,0x02,
          0xCF       , 3, 0x00,0xC1,0x30,
          0xED       , 4, 0x64,0x03,0x12,0x81,
          0xE8       , 3, 0x85,0x00,0x78,
          0xCB       , 5, 0x39,0x2C,0x00,0x34,0x02,
          0xF7       , 1, 0x20,
          0xEA       , 2, 0x00,0x00,
          CMD::PWCTR1, 1, 0x23,
          CMD::PWCTR2, 1, 0x10,
          CMD::VMCTR1, 2, 0x3e,0x28,
          CMD::VMCTR2, 1, 0x86,
          CMD::FRMCTR1,2, 0x00,0x13,
          0xF2       , 1, 0x00,

          CMD::GAMMASET,1, 0x01,  // Gamma set, curve 1
          CMD::GMCTRP1,15, 0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00,
          CMD::GMCTRN1,15, 0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F,
          0xFF,0xFF, // end
      };
      static constexpr uint8_t list1[] = {
          CMD::DFUNCTR,3, 0x08,0xC2,0x27,
          0xFF,0xFF, // end
      };
      static constexpr uint8_t list2[] = {
          CMD::IDMOFF, 0,
          CMD::SLPOUT, 0,
          CMD::DISPON, 0,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      case 1: return list1;
      case 2: return list2;
      default: return nullptr;
      }
    }
  };

  struct Panel_ILI9342 : public PanelIlitekCommon
  {
    Panel_ILI9342(void) {
      panel_width  = memory_width  = 320;
      panel_height = memory_height = 240;

      freq_write = 40000000;
      freq_read  = 16000000;
      freq_fill  = 40000000;
    }
  protected:

    struct CMD : public PanelIlitekCommon::CommandCommon
    {
      static constexpr uint8_t DFUNCTR = 0xB6;
      static constexpr uint8_t PWCTR1  = 0xC0;
      static constexpr uint8_t PWCTR2  = 0xC1;
      static constexpr uint8_t VMCTR1  = 0xC5;
      static constexpr uint8_t GMCTRP1 = 0xE0; // Positive Gamma Correction (E0h)
      static constexpr uint8_t GMCTRN1 = 0xE1; // Negative Gamma Correction (E1h)
    };

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          0xC8       ,  3, 0xFF,0x93,0x42,   // Turn on the external command
          CMD::PWCTR1,  2, 0x12, 0x12,
          CMD::PWCTR2,  1, 0x03,
          CMD::VMCTR1,  1, 0xF2,
          0xB0       ,  1, 0xE0,
          0xF6       ,  3, 0x01, 0x00, 0x00,
          CMD::GMCTRP1,15, 0x00,0x0C,0x11,0x04,0x11,0x08,0x37,0x89,0x4C,0x06,0x0C,0x0A,0x2E,0x34,0x0F,
          CMD::GMCTRN1,15, 0x00,0x0B,0x11,0x05,0x13,0x09,0x33,0x67,0x48,0x07,0x0E,0x0B,0x2E,0x33,0x0F,
          CMD::DFUNCTR, 4, 0x08,0x82,0x1D,0x04,
          CMD::SLPOUT, 0+CMD_INIT_DELAY, 130,    // Exit sleep mode
          CMD::IDMOFF, 0,
          CMD::DISPON, 0,
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

#endif
