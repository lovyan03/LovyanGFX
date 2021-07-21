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

  struct Panel_ILI9225 : public Panel_LCD
  {
    Panel_ILI9225(void)
    {
      _cfg.memory_width  = _cfg.panel_width  = 176;
      _cfg.memory_height = _cfg.panel_height = 220;
      _cfg.readable = false; // RW pin not supported.
//    _cmd_ramrd = CMD_RAMWR;
    }

    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override;
    void drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
    color_depth_t setColorDepth(color_depth_t depth) override;

    void setRotation(uint_fast8_t r) override;

  protected:

    void update_madctl(void);
    void set_window(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye, uint32_t cmd);

    static constexpr uint8_t CMD_RAMWR = 0x22;

    static constexpr uint8_t CMD_H_ADDR1 = 0x36;
    static constexpr uint8_t CMD_H_ADDR2 = 0x37;
    static constexpr uint8_t CMD_V_ADDR1 = 0x38;
    static constexpr uint8_t CMD_V_ADDR2 = 0x39;
    static constexpr uint8_t CMD_OUTPUT_CTRL = 0x01;
    static constexpr uint8_t CMD_ENTRY_MODE  = 0x03;
    static constexpr uint8_t CMD_POWER_CTRL1 = 0x10;
    static constexpr uint8_t CMD_POWER_CTRL2 = 0x11;
    static constexpr uint8_t CMD_POWER_CTRL3 = 0x12;
    static constexpr uint8_t CMD_POWER_CTRL4 = 0x13;
    static constexpr uint8_t CMD_POWER_CTRL5 = 0x14;
    static constexpr uint8_t CMD_DISPLAY_CTRL1 = 0x07;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
          CMD_POWER_CTRL2  , 2, 0x00, 0x18,
          CMD_POWER_CTRL3  , 2, 0x61, 0x21,
          CMD_POWER_CTRL4  , 2, 0x00, 0x6F,
          CMD_POWER_CTRL5  , 2, 0x49, 0x5F,
          CMD_POWER_CTRL1  , 2+CMD_INIT_DELAY, 0x08, 0x00, 10,
          CMD_POWER_CTRL2  , 2, 0x10, 0x3B,
          CMD_DISPLAY_CTRL1, 2+CMD_INIT_DELAY, 0x00, 0x12, 50,
          CMD_OUTPUT_CTRL  , 2, 0x02, 0x00,
          0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }

    virtual void setColorDepth_impl(color_depth_t depth) { _write_depth = ((int)depth & color_depth_t::bit_mask) > 16 ? rgb888_3Byte : rgb565_2Byte; _read_depth = rgb888_3Byte; }
  };

//----------------------------------------------------------------------------
 }
}
