#ifndef LGFX_PANEL_SSD_COMMON_HPP_
#define LGFX_PANEL_SSD_COMMON_HPP_

#include "panel_common.hpp"

namespace lgfx
{
  class PanelSsdCommon : public PanelCommon
  {
  public:

    PanelSsdCommon() : PanelCommon()
    {
      cmd_caset  = CommandCommon::CASET;
      cmd_raset  = CommandCommon::RASET;
      cmd_ramwr  = CommandCommon::RAMWR;
      cmd_ramrd  = CommandCommon::RAMRD;
      cmd_invon  = CommandCommon::INVON;
      cmd_invoff = CommandCommon::INVOFF;
      read_depth = rgb888_3Byte;
      len_dummy_read_pixel = 8;
      len_dummy_read_rddid = 0;
      len_setwindow = 16;
    }

  protected:
    void setConfig_impl(void) override
    {
      if (!_ram_width   ) _ram_width = 128;
      if (!_ram_height  ) _ram_height = 128;
      if (!_panel_width ) _panel_width = 128;
      if (!_panel_height) _panel_height = 128;
    }

    const uint8_t* getRotationCommands(uint8_t* buf, uint8_t r) override
    {
      PanelCommon::getRotationCommands(buf, r);
      buf[0] = CommandCommon::MADCTL;
      buf[1] = 1;
      buf[2] = getMadCtl(rotation) | getColMod(write_depth);
      buf[3] = CommandCommon::STARTLINE;
      buf[4] = 1;
      buf[5] = (rotation < 2) ? _ram_height : 0;
      buf[6] = buf[7] = 0xFF;
      if (rotation & 1) {
        cmd_caset = CommandCommon::RASET;
        cmd_raset = CommandCommon::CASET;
      } else {
        cmd_caset = CommandCommon::CASET;
        cmd_raset = CommandCommon::RASET;
      }
      return buf;
    }

    const uint8_t* getColorDepthCommands(uint8_t* buf, color_depth_t depth) override
    {
      PanelCommon::getColorDepthCommands(buf, depth);
      buf[0] = CommandCommon::MADCTL;
      buf[1] = 1;
      buf[2] = getMadCtl(rotation) | getColMod(write_depth);
      buf[3] = buf[4] = 0xFF;
      return buf;
    }

    struct CommandCommon {
    static constexpr uint_fast8_t NOP     = 0x00;
    static constexpr uint_fast8_t SWRESET = 0x01;
//  static constexpr uint_fast8_t RDDID   = 0x04;
//  static constexpr uint_fast8_t RDDST   = 0x09;
    static constexpr uint_fast8_t SLPIN   = 0xAE;
    static constexpr uint_fast8_t SLPOUT  = 0xAF;
//  static constexpr uint_fast8_t PTLON   = 0x12;
//  static constexpr uint_fast8_t NORON   = 0x13;
    static constexpr uint_fast8_t INVOFF  = 0xA6;
    static constexpr uint_fast8_t INVON   = 0xA7;
//  static constexpr uint_fast8_t GAMMASET= 0x26;
    static constexpr uint_fast8_t DISPOFF = 0xA4;
    static constexpr uint_fast8_t DISPON  = 0xA5;
    static constexpr uint_fast8_t CASET   = 0x15;
    static constexpr uint_fast8_t RASET   = 0x75; static constexpr uint8_t PASET = 0x75;
    static constexpr uint_fast8_t RAMWR   = 0x5C;
    static constexpr uint_fast8_t RAMRD   = 0x5D;
    static constexpr uint_fast8_t MADCTL  = 0xA0;

    static constexpr uint_fast8_t CMDLOCK = 0xFD;
    static constexpr uint_fast8_t STARTLINE = 0xA1;
    };

    color_depth_t getAdjustBpp(color_depth_t bpp) const override { return (bpp > 16) ? rgb666_3Byte : rgb565_2Byte; }

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CommandCommon::CMDLOCK, 1, 0x12,
          CommandCommon::CMDLOCK, 1, 0xB1,
          CommandCommon::SLPIN  , 0,
          //CommandCommon::DISPOFF, 0,
          0xB3                  , 1, 0xF1,  // CLOCKDIV
          0xCA                  , 1, 0x7F,  // MUXRATIO
          0xA2                  , 1, 0x00,  // DISPLAYOFFSET
          0xB5                  , 1, 0x00,  // SETGPIO
          0xAB                  , 1, 0x01,  // FUNCTIONSELECT
          0xB1                  , 1, 0x32,  // PRECHARGE

          0xBE                  , 1, 0x05,  // VCOMH
          CommandCommon::INVOFF , 0,
          0xC1                  , 3, 0xC8, 0x80, 0xC8, // CONTRASTABC
          0xC7                  , 1, 0x0F,  // CONTRASTMASTER
          0xB4                  , 3, 0xA0, 0xB5, 0x55, // SETVSL
          0xB6                  , 1, 0x01,  // PRECHARGE2
          CommandCommon::SLPOUT , 0,
          CommandCommon::DISPON , 0,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }

    uint8_t getMadCtl(uint8_t r) const {
      static constexpr uint8_t madctl_table[] = {
        0b00110100,
        0b00110111,
        0b00100110,
        0b00100101,
      };
      r = r & 3;
      return madctl_table[r];
    }
    uint8_t getColMod(uint8_t bpp) const {
      if (bpp == 16) return 0x40;
      return 0x80;
    }
  };


  class Panel_SSD1351_Common : public PanelSsdCommon
  {
  };


  template <typename CFG>
  class Panel_SSD1351 : public Panel_SSD1351_Common
  {
  public:
    Panel_SSD1351() : Panel_SSD1351_Common() { setConfig<CFG>(); }
  };
}

#endif
