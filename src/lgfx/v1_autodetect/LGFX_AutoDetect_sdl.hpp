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

#include "../v1_init.hpp"
#include "common.hpp"
#include "../v1/platforms/sdl/Panel_sdl.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class LGFX : public LGFX_Device
  {
    lgfx::Panel_sdl _panel_instance;

    bool init_impl(bool use_reset, bool use_clear)
    {
      return LGFX_Device::init_impl(false, use_clear);
    }

  public:

    LGFX(int width = 320, int height = 240, uint_fast8_t scaling_x = 0, uint_fast8_t scaling_y = 0)
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width = width;
      cfg.panel_width = width;
      cfg.memory_height = height;
      cfg.panel_height = height;
      _panel_instance.config(cfg);
      if (scaling_x == 0) { scaling_x = 1; }
      if (scaling_y == 0) { scaling_y = scaling_x; }
      _panel_instance.setScaling(scaling_x, scaling_y);
      setPanel(&_panel_instance);
      _board = board_t::board_SDL;
    }
  };

//----------------------------------------------------------------------------
 }
}

using LGFX = lgfx::LGFX;