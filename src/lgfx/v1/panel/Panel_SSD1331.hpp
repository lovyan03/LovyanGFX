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

#include "Panel_SSD1351.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_SSD1331 : public Panel_SSD1351
  {
    Panel_SSD1331(void) : Panel_SSD1351()
    {
      _cfg.memory_width  = _cfg.panel_width  = 96;
      _cfg.memory_height = _cfg.panel_height = 64;
      _cfg.readable = false;
    }

    void setBrightness(uint8_t brightness) override;
    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override;
    void drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
    void copyRect(uint_fast16_t dst_x, uint_fast16_t dst_y, uint_fast16_t w, uint_fast16_t h, uint_fast16_t src_x, uint_fast16_t src_y) override;

  protected:

    uint32_t _last_us = 0;
    uint32_t _need_delay = 0;

    static constexpr uint8_t CMD_DRAWLINE       = 0x21;   //!< Draw line
    static constexpr uint8_t CMD_DRAWRECT       = 0x22;   //!< Draw rectangle
    static constexpr uint8_t CMD_COPY           = 0x23;
    static constexpr uint8_t CMD_FILL           = 0x26;   //!< Fill enable/disable
    static constexpr uint8_t CMD_SETCOLUMN      = 0x15;   //!< Set column address
    static constexpr uint8_t CMD_SETROW         = 0x75;   //!< Set row adress
    static constexpr uint8_t CMD_CONTRASTA      = 0x81;   //!< Set contrast for color A
    static constexpr uint8_t CMD_CONTRASTB      = 0x82;   //!< Set contrast for color B
    static constexpr uint8_t CMD_CONTRASTC      = 0x83;   //!< Set contrast for color C
    static constexpr uint8_t CMD_MASTERCURRENT  = 0x87;   //!< Master current control
    static constexpr uint8_t CMD_SETREMAP       = 0xA0;   //!< Set re-map & data format
    static constexpr uint8_t CMD_STARTLINE      = 0xA1;   //!< Set display start line
    static constexpr uint8_t CMD_DISPLAYOFFSET  = 0xA2;   //!< Set display offset
    static constexpr uint8_t CMD_NORMALDISPLAY  = 0xA4;   //!< Set display to normal mode
    static constexpr uint8_t CMD_DISPLAYALLON   = 0xA5;   //!< Set entire display ON
    static constexpr uint8_t CMD_DISPLAYALLOFF  = 0xA6;   //!< Set entire display OFF
    static constexpr uint8_t CMD_INVERTDISPLAY  = 0xA7;   //!< Invert display
    static constexpr uint8_t CMD_SETMULTIPLEX   = 0xA8;   //!< Set multiplex ratio
    static constexpr uint8_t CMD_DISPONDIMMER   = 0xAC;
    static constexpr uint8_t CMD_SETMASTER      = 0xAD;   //!< Set master configuration
    static constexpr uint8_t CMD_DISPLAYOFF     = 0xAE;   //!< Display OFF (sleep mode)
    static constexpr uint8_t CMD_DISPLAYON      = 0xAF;   //!< Normal Brightness Display ON
    static constexpr uint8_t CMD_POWERMODE      = 0xB0;   //!< Power save mode
    static constexpr uint8_t CMD_PRECHARGE      = 0xB1;   //!< Phase 1 and 2 period adjustment
    static constexpr uint8_t CMD_CLOCKDIV       = 0xB3;   //!< Set display clock divide ratio/oscillator frequency
    static constexpr uint8_t CMD_PRECHARGEA     = 0x8A;   //!< Set second pre-charge speed for color A
    static constexpr uint8_t CMD_PRECHARGEB     = 0x8B;   //!< Set second pre-charge speed for color B
    static constexpr uint8_t CMD_PRECHARGEC     = 0x8C;   //!< Set second pre-charge speed for color C
    static constexpr uint8_t CMD_PRECHARGELEVEL = 0xBB;   //!< Set pre-charge voltage
    static constexpr uint8_t CMD_VCOMH          = 0xBE;   //!< Set Vcomh voltge
    static constexpr uint8_t CMD_NOP            = 0xE3;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
          CMD_DISPLAYOFF    , 0,  // 0xAE
          CMD_STARTLINE     , 0, 0x00, 0,
          CMD_DISPLAYOFFSET , 0, 0x00, 0,
          CMD_NORMALDISPLAY , 0,
          CMD_SETMULTIPLEX  , 0, 0x3F, 0,
          CMD_SETMASTER     , 0, 0x8E, 0,
          CMD_POWERMODE     , 0, 0x0B, 0,
          CMD_PRECHARGE     , 0, 0x31, 0,
          CMD_CLOCKDIV      , 0, 0xF0, 0,

          CMD_PRECHARGEA    , 0, 0x64, 0,
          CMD_PRECHARGEB    , 0, 0x78, 0,
          CMD_PRECHARGEC    , 0, 0x64, 0,
/*
          CMD_PRECHARGEA    , 0, 0x64, 0,
          CMD_PRECHARGEB    , 0, 0x78, 0,
          CMD_PRECHARGEC    , 0, 0x64, 0,
          CMD_PRECHARGELEVEL, 0, 0x3A, 0,
*/
          CMD_PRECHARGEA    , 0, 0x80, 0,
          CMD_PRECHARGEB    , 0, 0x80, 0,
          CMD_PRECHARGEC    , 0, 0x80, 0,
          CMD_PRECHARGELEVEL, 0, 0x1A, 0,
          CMD_VCOMH         , 0, 0x3E, 0,
/*
          CMD_MASTERCURRENT , 0, 0x06, 0,
          CMD_CONTRASTA     , 0, 0x91, 0,
          CMD_CONTRASTB     , 0, 0x50, 0,
          CMD_CONTRASTC     , 0, 0x7D, 0,
*/
          CMD_FILL          , 0, 0x01, 0,
          CMD_DISPLAYON     , 0,
          0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }

    void update_madctl(void) override;

    void setColorDepth_impl(color_depth_t depth) override { _write_depth = ((int)depth & color_depth_t::bit_mask) > 8 ? rgb565_2Byte : rgb332_1Byte; _read_depth = rgb565_2Byte; }
  };

//----------------------------------------------------------------------------
 }
}
