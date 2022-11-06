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

#include "Panel_HUB75.hpp"
#include "../Bus.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  Panel_HUB75::~Panel_HUB75(void)
  {
    _frame_buffer.release();
  }

  bool Panel_HUB75::init(bool use_reset)
  {
    if (_bus->busType() != bus_type_t::bus_image_push)
    {
      return false;
    }

    size_t single_width  = _cfg.panel_width  / _config_detail.x_panel_count;
    size_t single_height = _cfg.panel_height / _config_detail.y_panel_count;
    size_t buffer_length = single_width * _config_detail.panel_count * sizeof(swap565_t);

    if (_frame_buffer.create(buffer_length, single_height, single_height >> 1) == nullptr)
    {
      return false;
    }

    _single_width = single_width;
    _single_height = single_height;

    _write_depth = color_depth_t::rgb565_2Byte;
    _read_depth = color_depth_t::rgb565_2Byte;

    ((Bus_ImagePush*)_bus)->setImageBuffer((void*)&_frame_buffer);

    if (!Panel_CoordinateConvertFB::init(use_reset))
    {
      return false;
    }

    // 転送開始
    _bus->beginTransaction();
    return true;
  }

  uint32_t Panel_HUB75::_read_pixel_inner(uint_fast16_t x, uint_fast16_t y)
  {
    auto buf = (uint16_t*)_frame_buffer.getLineBuffer(y);
    return buf[x];
  }

  void Panel_HUB75::_draw_pixel_inner(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    uint_fast8_t px = x / _single_width;
    uint_fast8_t py = y / _single_height;

    x -= px * _single_width;
    y -= py * _single_height;
    uint_fast8_t panel_index = px + _config_detail.x_panel_count * py;

    auto buf = (uint16_t*)_frame_buffer.getLineBuffer(y);
    buf[x + panel_index * _single_width] = rawcolor;
  }

  void Panel_HUB75::setBrightness(uint8_t brightness)
  {
    ((Bus_ImagePush*)_bus)->setBrightness(brightness);
  }

//----------------------------------------------------------------------------
 }
}
