#ifndef LGFX_PANEL_ILI9163_HPP_
#define LGFX_PANEL_ILI9163_HPP_

#include "panel_lcd_common.hpp"

namespace lgfx
{
  struct Panel_ILI9163_COMMON : public PanelLcdCommon
  {
    Panel_ILI9163_COMMON() : PanelLcdCommon()
    {
      len_command = 8;
      len_read_pixel = 24;
      len_dummy_read_pixel = 8;
      len_dummy_read_rddid = 0;
    }

    enum colmod_t
    { RGB565_2BYTE = 0x55
    , RGB666_3BYTE = 0x66
    };
    uint8_t getColMod(uint8_t bpp) const { return (bpp > 16) ? RGB666_3BYTE : RGB565_2BYTE; }

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
    enum MAD
    { MY  = 0x80
    , MX  = 0x40
    , MV  = 0x20
    , ML  = 0x10
    , BGR = 0x08
    , MH  = 0x04
    , RGB = 0x00
    };

    const uint8_t* getInitCommands(uint8_t listno = 0) const {
      static constexpr uint8_t list0[] = {
          0x01,  0 + CMD_INIT_DELAY, 120,  // Software reset
          0x11,  0 + CMD_INIT_DELAY, 5,    // Exit sleep mode
          0x3A,  1, 0x05, // Set pixel format
          0x26,  1, 0x04, // Set Gamma curve 3
          0xF2,  1, 0x01, // Gamma adjustment enabled
          0xE0, 15, 0x3F, 0x25, 0x1C, 0x1E, 0x20, 0x12, 0x2A, 0x90,
                    0x24, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, // Positive Gamma
          0xE1, 15, 0x20, 0x20, 0x20, 0x20, 0x05, 0x00, 0x15,0xA7,
                    0x3D, 0x18, 0x25, 0x2A, 0x2B, 0x2B, 0x3A, // Negative Gamma
          0xB1,  2, 0x08, 0x08, // Frame rate control 1
          0xB4,  1, 0x07,       // Display inversion
          0xC0,  2, 0x0A, 0x02, // Power control 1
          0xC1,  1, 0x02,       // Power control 2
          0xC5,  2, 0x50, 0x5B, // Vcom control 1
          0xC7,  1, 0x40,       // Vcom offset
          0x2A,  4, 0x00, 0x00, 0x00, 0x7F, // Set column address
          0x2B,  4 + CMD_INIT_DELAY, 0x00, 0x00, 0x00, 0x9F, 250, // Set page address
          0x36,  1, 0xC8,       // Set address mode
          0x29,  0,             // Set display on
          0xFF,0xFF, // end
      };
      static constexpr uint8_t list1[] = {
          CMD::DISPON, 0,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      case 1: return list1;
      default: return nullptr;
      }
    }

    const rotation_data_t* getRotationData(uint8_t r) const {
      static constexpr uint8_t madctl_table[] = {
                MAD::MX|MAD::MY,
        MAD::MV        |MAD::MY,
                              0,
        MAD::MV|MAD::MX        ,
      };
      r = r & 3;
      auto res = const_cast<rotation_data_t*>(PanelLcdCommon::getRotationData(r));
      res->madctl = madctl_table[r];
      return res;
    }
  };

  template<uint16_t RamWidth = 132, uint16_t RamHeight = 162, uint16_t PanelWidth = 132, uint16_t PanelHeight = 162, uint16_t OffsetX = 0, uint16_t OffsetY = 0>
  struct Panel_ILI9163 : public Panel_ILI9163_COMMON
  {
    Panel_ILI9163() : Panel_ILI9163_COMMON()
    {
      ram_width  = RamWidth;
      ram_height = RamHeight;
      panel_width  = PanelWidth;
      panel_height = PanelHeight;
      offset_x = OffsetX;
      offset_y = OffsetY;
    }
  };


  struct Panel_ILI9163_128x160 : public Panel_ILI9163<128, 160, 128, 160, 0, 0> {};

}

#endif
