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

  struct Panel_1bitOLED : public Panel_HasBuffer
  {
    bool init(bool use_reset) override;

    void waitDisplay(void) override;
    bool displayBusy(void) override;
    color_depth_t setColorDepth(color_depth_t depth) override;

    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool) override {}

    void writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor) override;
    void writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writePixels(pixelcopy_t* param, std::uint32_t len, bool use_dma) override;

    std::uint32_t readCommand(std::uint_fast8_t, std::uint_fast8_t, std::uint_fast8_t) override { return 0; }
    std::uint32_t readData(std::uint_fast8_t, std::uint_fast8_t) override { return 0; }

    void readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param) override;

  protected:

    static constexpr std::uint8_t CMD_SETSTARTLINE        = 0x40;
    static constexpr std::uint8_t CMD_DISPLAYALLON_RESUME = 0xA4;
    static constexpr std::uint8_t CMD_DISPLAYALLON        = 0xA5;
    static constexpr std::uint8_t CMD_NORMALDISPLAY       = 0xA6;
    static constexpr std::uint8_t CMD_INVERTDISPLAY       = 0xA7;
    static constexpr std::uint8_t CMD_SETMULTIPLEX        = 0xA8;
    static constexpr std::uint8_t CMD_DISP_OFF            = 0xAE;
    static constexpr std::uint8_t CMD_DISP_ON             = 0xAF;
    static constexpr std::uint8_t CMD_SETPRECHARGE        = 0xD9;
    static constexpr std::uint8_t CMD_SETVCOMDETECT       = 0xDB;

    std::size_t _get_buffer_length(void) const override;
    bool _read_pixel(std::uint_fast16_t x, std::uint_fast16_t y);
    void _draw_pixel(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t value);
    void _update_transferred_rect(std::uint_fast16_t &xs, std::uint_fast16_t &ys, std::uint_fast16_t &xe, std::uint_fast16_t &ye);

  };

  struct Panel_SSD1306 : public Panel_1bitOLED
  {
    Panel_SSD1306(void)
    {
      _cfg.memory_width  = _cfg.panel_width  = 128;
      _cfg.memory_height = _cfg.panel_height = 64;
    }

    void setBrightness(std::uint8_t brightness) override;

    void display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h) override;

  protected:

    static constexpr std::uint8_t CMD_MEMORYMODE  = 0x20;

    static constexpr std::uint8_t CMD_COLUMNADDR  = 0x21;
    static constexpr std::uint8_t CMD_PAGEADDR    = 0x22;

    static constexpr std::uint8_t CMD_SETCONTRAST = 0x81;
    static constexpr std::uint8_t CMD_CHARGEPUMP  = 0x8D;
    static constexpr std::uint8_t CMD_SEGREMAP    = 0xA0;
//  static constexpr std::uint8_t CMD_SETMLTPLX   = 0xA8;
    static constexpr std::uint8_t CMD_COMSCANINC  = 0xC0;
//  static constexpr std::uint8_t CMD_COMSCANDEC  = 0xC8;
    static constexpr std::uint8_t CMD_SETOFFSET   = 0xD3;
    static constexpr std::uint8_t CMD_SETCLKDIV   = 0xD5;
    static constexpr std::uint8_t CMD_SETCOMPINS  = 0xDA;

    static constexpr std::uint8_t CMD_DEACTIVATE_SCROLL   = 0x2E;

    const std::uint8_t* getInitCommands(std::uint8_t listno) const override
    {
      static constexpr std::uint8_t list0[] = {
        CMD_DISP_OFF           ,
        CMD_SETCLKDIV          , 0x80,
        CMD_SETMULTIPLEX       , 0x3F,
        CMD_SETOFFSET          , 0x00,
        CMD_SETSTARTLINE       ,
        CMD_MEMORYMODE         , 0x00,
        CMD_SEGREMAP           ,
        CMD_COMSCANINC         ,
        CMD_SETCOMPINS         , 0x12,
        CMD_SETVCOMDETECT      , 0x10,
        CMD_DISPLAYALLON_RESUME,
        CMD_DEACTIVATE_SCROLL  ,
        CMD_CHARGEPUMP         , 0x14,
        CMD_DISP_ON            ,
        CMD_SETCONTRAST        , 0x00,
        CMD_SETPRECHARGE       , 0x11,
        0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

  struct Panel_SH110x : public Panel_1bitOLED
  {
    Panel_SH110x(void)
    {
      _cfg.memory_width  = _cfg.panel_width  = 128;
      _cfg.memory_height = _cfg.panel_height = 128;
      _auto_display = true;
    }

    bool init(bool use_reset) override;

    void beginTransaction(void) override;

    void setBrightness(std::uint8_t brightness) override;

    void display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h) override;

  protected:
    static constexpr std::uint8_t CMD_PAGEADDRESSINGMODE  = 0x20;
    static constexpr std::uint8_t CMD_VERTADDRESSINGMODE  = 0x21;
    static constexpr std::uint8_t CMD_PAGEADDR            = 0x22;
    static constexpr std::uint8_t CMD_SETCONTRAST         = 0x81;
//  static constexpr std::uint8_t CMD_CHARGEPUMP          = 0x8D;
    static constexpr std::uint8_t CMD_SEGREMAP            = 0xA0;
    static constexpr std::uint8_t CMD_DCDC                = 0xAD;
    static constexpr std::uint8_t CMD_SETPAGEADDR         = 0xB0;
    static constexpr std::uint8_t CMD_COMSCANINC          = 0xC0;
    static constexpr std::uint8_t CMD_COMSCANDEC          = 0xC8;
    static constexpr std::uint8_t CMD_SETDISPLAYOFFSET    = 0xD3;
    static constexpr std::uint8_t CMD_SETDISPLAYCLOCKDIV  = 0xD5;
    static constexpr std::uint8_t CMD_SETCOMPINS          = 0xDA;
    static constexpr std::uint8_t CMD_SETDISPSTARTLINE    = 0xDC;
    static constexpr std::uint8_t CMD_SETLOWCOLUMN        = 0x00;
    static constexpr std::uint8_t CMD_SETHIGHCOLUMN       = 0x10;
    static constexpr std::uint8_t CMD_READMODIFYWRITE     = 0xE0;
    static constexpr std::uint8_t CMD_READMODIFYWRITE_END = 0xEE;

    const std::uint8_t* getInitCommands(std::uint8_t listno) const override
    {
      static constexpr std::uint8_t list0[] = {
        CMD_DISP_OFF   ,
        CMD_SETSTARTLINE       ,
        CMD_READMODIFYWRITE_END,
        CMD_PAGEADDRESSINGMODE ,
        CMD_SETDISPSTARTLINE   , 0x00,
        CMD_SETDISPLAYCLOCKDIV , 0x50,
        CMD_DCDC               , 0x8B,
        CMD_SEGREMAP           ,
        CMD_COMSCANINC         ,
        CMD_SETPRECHARGE       , 0x20,
        CMD_SETVCOMDETECT      , 0x35,
        CMD_DISPLAYALLON_RESUME,
        CMD_SETCONTRAST, 0x00,
        CMD_SETCOMPINS, 0x12,
        CMD_DISP_ON    ,
        0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------
 }
}
