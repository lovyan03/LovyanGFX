#ifndef LGFX_PANEL_SSD1351_HPP_
#define LGFX_PANEL_SSD1351_HPP_

#include "PanelCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_SSD1351 : public PanelCommon
  {
    Panel_SSD1351()
    {
      panel_width  = memory_width  = 128;
      panel_height = memory_height = 128;

      freq_write = 20000000;
      freq_read  = 16000000;
      freq_fill  = 20000000;

      read_depth = rgb666_3Byte;
      len_dummy_read_pixel = 8;
      len_dummy_read_rddid = 0;
      len_setwindow = 16;

      cmd_caset  = CommandCommon::CASET;
      cmd_raset  = CommandCommon::RASET;
      cmd_ramwr  = CommandCommon::RAMWR;
      cmd_ramrd  = CommandCommon::RAMRD;
    }

  protected:

    const uint8_t* getWindowCommands1(uint8_t* buf, uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override
    {
      if (_xs == xs && _xe == xe) return nullptr;
      (void)ys;
      (void)ye;
      _xs = xs;
      _xe = xe;
      buf[0] = (_internal_rotation & 1) ? CommandCommon::RASET : CommandCommon::CASET;
      buf[1] = 2;
      buf[2] = xs + _colstart;
      buf[3] = xe + _colstart;
      reinterpret_cast<uint16_t*>(buf)[2] = 0xFFFF;
      return buf;
    }

    const uint8_t* getWindowCommands2(uint8_t* buf, uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override
    {
      if (_ys == ys && _ye == ye) return nullptr;
      (void)xs;
      (void)xe;
      _ys = ys;
      _ye = ye;
      buf[0] = (_internal_rotation & 1) ? CommandCommon::CASET : CommandCommon::RASET;
      buf[1] = 2;
      buf[2] = ys + _rowstart;
      buf[3] = ye + _rowstart;
      reinterpret_cast<uint16_t*>(buf)[2] = 0xFFFF;
      return buf;
    }

    const uint8_t* getSleepInCommands(uint8_t* buf) override
    {
      reinterpret_cast<uint16_t*>(buf)[0] = CommandCommon::SLPIN;
      reinterpret_cast<uint16_t*>(buf)[1] = 0xFFFF;
      return buf;
    }

    const uint8_t* getSleepOutCommands(uint8_t* buf) override
    {
      reinterpret_cast<uint16_t*>(buf)[0] = CommandCommon::SLPOUT;
      reinterpret_cast<uint16_t*>(buf)[1] = 0xFFFF;
      return buf;
    }

    const uint8_t* getInvertDisplayCommands(uint8_t* buf, bool invert) override
    {
      this->invert = invert;
      buf[2] = buf[0] = (invert ^ reverse_invert) ? CommandCommon::INVON : CommandCommon::INVOFF;
      buf[3] = buf[1] = 0;
      buf[5] = buf[4] = 0xFF;
      return buf;
    }

    const uint8_t* getRotationCommands(uint8_t* buf, int_fast8_t r) override
    {
      PanelCommon::getRotationCommands(buf, r);
      buf[0] = CommandCommon::MADCTL;
      buf[1] = 1;
      buf[2] = getMadCtl(_internal_rotation, write_depth);
      buf[3] = buf[4] = 0xFF;
      return buf;
    }

    const uint8_t* getColorDepthCommands(uint8_t* buf, color_depth_t depth) override
    {
      PanelCommon::getColorDepthCommands(buf, depth);
      buf[0] = CommandCommon::MADCTL;
      buf[1] = 1;
      buf[2] = getMadCtl(rotation, write_depth);
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
          CommandCommon::STARTLINE,1,0x00,
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

  private:
    uint8_t getMadCtl(uint_fast8_t r, uint_fast8_t bpp) {
// Set Re-map & Dual COM Line Mode (A0h) 
      static constexpr uint8_t madctl_table[] = {
        0b00110100,
        0b00110111,
        0b00100110,
        0b00100101,
        0b00100100,
        0b00110101,
        0b00110110,
        0b00100111,
      };
      /*
      if (r & 1) {
        cmd_caset = CommandCommon::RASET;
        cmd_raset = CommandCommon::CASET;
      } else {
        cmd_caset = CommandCommon::CASET;
        cmd_raset = CommandCommon::RASET;
      }
      //*/
      return madctl_table[r] | (bpp == 16 ? 0x40 : 0x80);
    }
  };

//----------------------------------------------------------------------------
 }
}

#endif
