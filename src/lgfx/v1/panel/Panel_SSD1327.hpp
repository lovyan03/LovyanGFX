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

  class Panel_SSD1327 : public Panel_HasBuffer
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

    void setBrightness(std::uint8_t brightness) override;
    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h) override;

    void writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor) override;
    void writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writePixels(pixelcopy_t* param, std::uint32_t len) override;

    std::uint32_t readCommand(std::uint_fast8_t cmd, std::uint_fast8_t index = 0, std::uint_fast8_t length = 4) override { return 0; }
    std::uint32_t readData(std::uint_fast8_t index = 0, std::uint_fast8_t length = 4) override { return 0; }

    void readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param) override;

  protected:

    static constexpr std::uint8_t CMD_NOP     = 0xE3;
    static constexpr std::uint8_t CMD_SWRESET = 0x01;
    static constexpr std::uint8_t CMD_SLPIN   = 0xAE;
    static constexpr std::uint8_t CMD_SLPOUT  = 0xAF;
    static constexpr std::uint8_t CMD_INVOFF  = 0xA4;
    static constexpr std::uint8_t CMD_INVON   = 0xA7;
    static constexpr std::uint8_t CMD_DISPOFF = 0xA6;
    static constexpr std::uint8_t CMD_DISPON  = 0xA4;
    static constexpr std::uint8_t CMD_CASET   = 0x15;
    static constexpr std::uint8_t CMD_RASET   = 0x75;
    static constexpr std::uint8_t CMD_PASET   = 0x75;
    static constexpr std::uint8_t CMD_RAMRD   = 0x5D;
    static constexpr std::uint8_t CMD_MADCTL  = 0xA0;
    static constexpr std::uint8_t CMD_CMDLOCK = 0xFD;
    static constexpr std::uint8_t CMD_STARTLINE = 0xA1;

    const std::uint8_t* getInitCommands(std::uint8_t listno) const override
    {
      static constexpr std::uint8_t list0[] =
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

    std::size_t _get_buffer_length(void) const override;
    std::uint8_t _read_pixel(std::uint_fast16_t x, std::uint_fast16_t y);
    void _draw_pixel(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t value);
    void _update_transferred_rect(std::uint_fast16_t &xs, std::uint_fast16_t &ys, std::uint_fast16_t &xe, std::uint_fast16_t &ye);

  };

//----------------------------------------------------------------------------
 }
}
