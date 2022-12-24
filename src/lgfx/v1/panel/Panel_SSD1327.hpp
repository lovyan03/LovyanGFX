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

#include "Panel_HasBuffer.hpp"
#include "../misc/range.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_SSD1327 : public Panel_HasBuffer
  {
  public:
    Panel_SSD1327(void) : Panel_HasBuffer()
    {
      _cfg.memory_width  = _cfg.panel_width  = 128;
      _cfg.memory_height = _cfg.panel_height = 128;
    }

    bool init(bool use_reset) override;

    void waitDisplay(void) override;
    bool displayBusy(void) override;
    color_depth_t setColorDepth(color_depth_t depth) override;

    void setBrightness(uint8_t brightness) override;
    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
    void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override;

    uint32_t readCommand(uint_fast16_t, uint_fast8_t, uint_fast8_t) override { return 0; }
    uint32_t readData(uint_fast8_t, uint_fast8_t) override { return 0; }

    void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override;

  protected:

    static constexpr uint8_t CMD_NOP     = 0xE3;
    static constexpr uint8_t CMD_SWRESET = 0x01;
    static constexpr uint8_t CMD_SLPIN   = 0xAE;
    static constexpr uint8_t CMD_SLPOUT  = 0xAF;
    static constexpr uint8_t CMD_INVOFF  = 0xA4;
    static constexpr uint8_t CMD_INVON   = 0xA7;
    static constexpr uint8_t CMD_DISPOFF = 0xA6;
    static constexpr uint8_t CMD_DISPON  = 0xA4;
    static constexpr uint8_t CMD_CASET   = 0x15;
    static constexpr uint8_t CMD_RASET   = 0x75;
    static constexpr uint8_t CMD_PASET   = 0x75;
    static constexpr uint8_t CMD_RAMRD   = 0x5D;
    static constexpr uint8_t CMD_MADCTL  = 0xA0;
    static constexpr uint8_t CMD_CMDLOCK = 0xFD;
    static constexpr uint8_t CMD_STARTLINE = 0xA1;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
        CMD_CMDLOCK , 0x12, // unlock the controller
        CMD_DISPOFF , // set display off
        0xB9        , // Select Default Linear Gray Scale Table
        0xB3        , 0xF1, // set clock divider
        0xA8        , 0x7F, // Set MUX ratio (default=0x7F)
        0xA0        , 0x51, // set display remap
        0xA1        , 0x00, // set display start line
        0xA2        , 0x00, // set display offset
        0xB5        , 0x00, // disable GPIO
        0xAB        , 0x01, // select external VDD regulator (none)
        0xB1        , 0x21, // set phase length
        CMD_SLPOUT  ,
        CMD_DISPON  ,
        0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }

    size_t _get_buffer_length(void) const override;
    uint8_t _read_pixel(uint_fast16_t x, uint_fast16_t y);
    void _draw_pixel(uint_fast16_t x, uint_fast16_t y, uint32_t value);
    void _update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye);

  };

//----------------------------------------------------------------------------
 }
}
