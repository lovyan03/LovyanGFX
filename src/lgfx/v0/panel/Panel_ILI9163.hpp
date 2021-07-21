#ifndef LGFX_PANEL_ILI9163_HPP_
#define LGFX_PANEL_ILI9163_HPP_

#include "PanelIlitekCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_ILI9163 : public PanelIlitekCommon
  {
    Panel_ILI9163(void)
    {
// The ILI9163 is available in six resolutions, 132x162 / 128x128 / 120x160 / 128x160 / 130x130 / 132x132, depending on the mode select pin status.
      panel_width  = memory_width  = 132;
      panel_height = memory_height = 162;

      freq_write = 20000000;
      freq_read  = 10000000;
      freq_fill  = 40000000;
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
      static constexpr uint8_t GMCTRP1 = 0xE0; // Positive Gamma Correction (E0h)
      static constexpr uint8_t GMCTRN1 = 0xE1; // Negative Gamma Correction (E1h)

      static constexpr uint8_t RDINDEX = 0xD9; // ili9341
      static constexpr uint8_t IDXRD   = 0xDD; // ILI9163 only, indexed control register read
    };
    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CMD::SWRESET, CMD_INIT_DELAY, 120,  // Software reset
          CMD::SLPOUT,  CMD_INIT_DELAY, 5,    // Exit sleep mode
          CMD::FRMCTR1, 2, 0x08, 0x08, // Frame rate control 1
          CMD::INVCTR , 1, 0x07,       // Display inversion
          CMD::PWCTR1 , 2, 0x0A, 0x02, // Power control 1
          CMD::PWCTR2 , 1, 0x02,       // Power control 2
          CMD::VMCTR1 , 2, 0x50, 0x5B, // Vcom control 1
          CMD::VMCTR2 , 1, 0x40,       // Vcom offset

          0x26        , 1, 0x04, // Set Gamma curve 3
          0xF2        , 1, 0x01, // Gamma adjustment enabled
          CMD::GMCTRP1,15, 0x3F, 0x25, 0x1C, 0x1E, 0x20, 0x12, 0x2A, 0x90,
                           0x24, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, // Positive Gamma
          CMD::GMCTRN1,15, 0x20, 0x20, 0x20, 0x20, 0x05, 0x00, 0x15,0xA7,
                           0x3D, 0x18, 0x25, 0x2A, 0x2B, 0x2B, 0x3A, // Negative Gamma
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
  };

//----------------------------------------------------------------------------
 }
}

#endif
