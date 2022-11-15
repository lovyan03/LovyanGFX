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
#include "../platforms/common.hpp"

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
    if (_initialized)
    {
      return true;
    }

    if (_bus->busType() != bus_type_t::bus_image_push)
    {
      return false;
    }

    if (!Panel_CoordinateConvertFB::init(use_reset))
    {
      return false;
    }

    if (!_init_frame_buffer())
    {
      return false;
    }

    // 転送開始
    _bus->beginTransaction();
    _initialized = true;

    return true;
  }

  bool Panel_HUB75::_init_frame_buffer(void)
  {
    _frame_buffer.release();
    uint_fast8_t bytes = _write_bits >> 3;

    size_t single_width  = _cfg.panel_width  / _config_detail.x_panel_count;
    size_t single_height = _cfg.panel_height / _config_detail.y_panel_count;
    size_t buffer_length = single_width * _config_detail.panel_count * bytes;

    if (_frame_buffer.create(buffer_length, single_height, single_height >> 1) == nullptr)
    {
      ESP_LOGE("DEBUG", "memory allocate error.");
      return false;
    }

    _single_width = single_width;
    _single_height = single_height;

    ((Bus_ImagePush*)_bus)->setImageBuffer((void*)&_frame_buffer, _write_depth);
    return true;
  }

  uint32_t Panel_HUB75::_read_pixel_inner(uint_fast16_t x, uint_fast16_t y)
  {
    uint_fast8_t px = x / _single_width;
    uint_fast8_t py = y / _single_height;

    x -= px * _single_width;
    y -= py * _single_height;
    uint_fast8_t panel_index = px + _config_detail.x_panel_count * py;

    auto buf = _frame_buffer.getLineBuffer(y);
    switch (_read_bits >> 3)
    {
      default:
        return buf [x + panel_index * _single_width];

      case 2:
      { // swap565ではなく rgb565で扱う
        uint32_t tmp = ((uint16_t*)buf)[x + panel_index * _single_width];
        return ((uint8_t)tmp << 8) | (tmp >> 8);
      }

      case 3:
        return ((lgfx::bgr888_t*)buf)[x + panel_index * _single_width].get();
    }
  }

  void Panel_HUB75::_draw_pixel_inner(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    uint_fast8_t px = x / _single_width;
    uint_fast8_t py = y / _single_height;

    x -= px * _single_width;
    y -= py * _single_height;
    uint_fast8_t panel_index = px + _config_detail.x_panel_count * py;

    auto buf = _frame_buffer.getLineBuffer(y);
    switch (_write_bits >> 3)
    {
      default:
        buf [x + panel_index * _single_width] = rawcolor;
        break;

      case 2:
        // swap565ではなく rgb565で扱う
        ((uint16_t*)buf)[x + panel_index * _single_width] = rawcolor << 8 | rawcolor >> 8;
        break;

      case 3:
        ((lgfx::bgr888_t*)buf)[x + panel_index * _single_width] = rawcolor;
        break;
    }
  }

  color_depth_t Panel_HUB75::setColorDepth(color_depth_t depth)
  {
    depth = ((depth & color_depth_t::bit_mask) < 16)
          ? color_depth_t::rgb332_1Byte
          : color_depth_t::rgb565_2Byte;
    if (_write_depth != depth)
    {
      _write_depth = depth;
      _read_depth = depth;
      if (_initialized)
      {
        _bus->endTransaction();
        if (_init_frame_buffer())
        {
          _bus->beginTransaction();
        }
      }
    }
    return depth;
  }

  void Panel_HUB75::setBrightness(uint8_t brightness)
  {
    ((Bus_ImagePush*)_bus)->setBrightness(brightness);
  }

//----------------------------------------------------------------------------
 }
}
