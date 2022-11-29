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

  struct Panel_NT35510 : public Panel_LCD
  {
    Panel_NT35510(void)
    {
      _cfg.panel_width = _cfg.memory_width = 480;
      _cfg.panel_height = _cfg.memory_height = 864;

      _cfg.dummy_read_pixel = 8;
    }

    bool init(bool use_reset) override;

    void setInvert(bool invert) override;
    void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override;

    void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override;

  protected:

    const uint8_t* getInitCommands(uint8_t listno) const override;

    void writeCommandList(const uint8_t *addr);

    void writeRegister(uint16_t cmd, uint8_t data);

    void update_madctl(void) override;

    void setColorDepth_impl(color_depth_t depth) override { _write_depth = ((int)depth & color_depth_t::bit_mask) > 16 ? rgb888_3Byte : rgb565_2Byte; _read_depth = rgb888_3Byte; }
  };

//----------------------------------------------------------------------------
 }
}
