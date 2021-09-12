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

  class Panel_SSD1351 : public Panel_LCD
  {
  public:
    Panel_SSD1351(void) : Panel_LCD()
    {
      _cfg.memory_width  = _cfg.panel_width  = 128;
      _cfg.memory_height = _cfg.panel_height = 128;

      _cmd_nop = CMD_NOP;
      _cmd_ramrd = CMD_RAMRD;
    }

    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override;
    void drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;

  protected:

    static constexpr uint8_t CMD_NOP     = 0xE3;
    static constexpr uint8_t CMD_SWRESET = 0x01;
    static constexpr uint8_t CMD_SLPIN   = 0xAE;
    static constexpr uint8_t CMD_SLPOUT  = 0xAF;
    static constexpr uint8_t CMD_INVOFF  = 0xA6;
    static constexpr uint8_t CMD_INVON   = 0xA7;
    static constexpr uint8_t CMD_DISPOFF = 0xA4;
    static constexpr uint8_t CMD_DISPON  = 0xA5;
    static constexpr uint8_t CMD_CASET   = 0x15;
    static constexpr uint8_t CMD_RASET   = 0x75;
    static constexpr uint8_t CMD_PASET   = 0x75;
    static constexpr uint8_t CMD_RAMWR   = 0x5C;
    static constexpr uint8_t CMD_RAMRD   = 0x5D;
    static constexpr uint8_t CMD_MADCTL  = 0xA0;
    static constexpr uint8_t CMD_CMDLOCK = 0xFD;
    static constexpr uint8_t CMD_STARTLINE = 0xA1;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
          CMD_CMDLOCK  , 1, 0x12,
          CMD_CMDLOCK  , 1, 0xB1,
          CMD_SLPIN    , 0,
          //CMD_DISPOFF, 0,
          0xB3         , 1, 0xF1,  // CLOCKDIV
          0xCA         , 1, 0x7F,  // MUXRATIO
          0xA2         , 1, 0x00,  // DISPLAYOFFSET
          0xB5         , 1, 0x00,  // SETGPIO
          0xAB         , 1, 0x01,  // FUNCTIONSELECT
          0xB1         , 1, 0x32,  // PRECHARGE

          0xBE         , 1, 0x05,  // VCOMH
          CMD_STARTLINE, 1, 0x00,
          CMD_INVOFF   , 0,
          0xC1         , 3, 0xC8, 0x80, 0xC8, // CONTRASTABC
          0xC7         , 1, 0x0F,  // CONTRASTMASTER
          0xB4         , 3, 0xA0, 0xB5, 0x55, // SETVSL
          0xB6         , 1, 0x01,  // PRECHARGE2
          CMD_SLPOUT   , 0,
          CMD_DISPON   , 0,
          0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }

    void set_window_8(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye, uint32_t cmd);
    void set_window_16(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye, uint32_t cmd);

    void update_madctl(void) override;

    /// SSD1351の18bitカラーモードはrgb666の24bitなので注意すること;
    void setColorDepth_impl(color_depth_t depth) override { _write_depth = ((int)depth & color_depth_t::bit_mask) > 16 ? rgb666_3Byte : rgb565_2Byte; _read_depth = rgb666_3Byte; }
  };

//----------------------------------------------------------------------------
 }
}
