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

  Panel_HUB75_Multi::~Panel_HUB75_Multi(void)
  {
    if (_panel_position)
    {
      heap_free(_panel_position);
      _panel_position = nullptr;
    }
    _panel_position_count = 0;
  }

  bool Panel_HUB75::init(bool use_reset)
  {
    return _init_impl(_cfg.panel_width, _cfg.panel_height);
  }

  bool Panel_HUB75_Multi::init(bool use_reset)
  {
    if (_init_impl(_config_detail.panel_count * _config_detail.single_width, _config_detail.single_height))
    {
      _init_hitcheck();
      return true;
    }
    return false;
  }

  bool Panel_HUB75::_init_impl(uint_fast16_t width, uint_fast16_t height)
  {
    if (_initialized)
    {
      return true;
    }

    if (_bus->busType() != bus_type_t::bus_image_push)
    {
      return false;
    }

    if (!Panel_FlexibleFrameBuffer::init(false))
    {
      return false;
    }

    if (!_init_frame_buffer(width, height))
    {
      return false;
    }

    _initialized = true;
    // 転送開始
    _bus->beginTransaction();

    return true;
  }

  color_depth_t Panel_HUB75::setColorDepth(color_depth_t depth)
  {
    depth = ((depth & color_depth_t::bit_mask) < 16)
          ? color_depth_t::rgb332_1Byte
          : color_depth_t::rgb565_2Byte;
    if (_write_depth != depth)
    {
      uint32_t prev_bytes = _write_bits >> 3;
      _write_depth = depth;
      _read_depth = depth;
      if (_initialized)
      {
        _bus->endTransaction();
        if (_init_frame_buffer(_frame_buffer.getLineSize() / prev_bytes, _frame_buffer.getTotalLines()))
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

  bool Panel_HUB75::_init_frame_buffer(uint_fast16_t total_width, uint_fast16_t single_height)
  {
    _frame_buffer.release();

    uint_fast8_t bytes = _write_bits >> 3;

    if (_frame_buffer.create(total_width * bytes, single_height, single_height >> 1) == nullptr)
    {
      return false;
    }

    ((Bus_ImagePush*)_bus)->setImageBuffer((void*)&_frame_buffer, _write_depth);

    return true;
  }

  void Panel_HUB75::_draw_pixel_inner(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    if (convertCoordinate)
    {
      convertCoordinate(x, y);
    }
    auto buf = _frame_buffer.getLineBuffer(y);
    switch (_write_bits >> 3)
    {
      default:
        buf [x] = rawcolor;
        break;

      case 2:
        // swap565ではなく rgb565で扱う
        ((uint16_t*)buf)[x] = rawcolor << 8 | ((rawcolor >> 8) & 0xFF);
        break;

      case 3:
        ((lgfx::bgr888_t*)buf)[x] = rawcolor;
        break;
    }
  }

  uint32_t Panel_HUB75::_read_pixel_inner(uint_fast16_t x, uint_fast16_t y)
  {
    if (convertCoordinate)
    {
      convertCoordinate(x, y);
    }
    auto buf = _frame_buffer.getLineBuffer(y);
    switch (_read_bits >> 3)
    {
      default:
        return buf [x];

      case 2:
      { // swap565ではなく rgb565で扱う
        uint32_t tmp = ((uint16_t*)buf)[x];
        return ((uint8_t)tmp << 8) | (tmp >> 8);
      }

      case 3:
        return ((lgfx::bgr888_t*)buf)[x].get();
    }
  }

  void Panel_HUB75_Multi::_draw_pixel_inner(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    uint32_t indexmask = _x_hitcheck_mask[x >> _x_hitcheck_shift] & _y_hitcheck_mask[y >> _y_hitcheck_shift];
    if (!indexmask) { return; }

    auto single_width = _config_detail.single_width;
    auto single_height = _config_detail.single_height;
    for (uint_fast8_t panel_index = 0; indexmask; ++panel_index, indexmask >>= 1)
    {
      if (indexmask & 1)
      {
        auto r = _panel_position[panel_index].rotation;
        uint_fast16_t ix = x - _panel_position[panel_index].x;
        uint_fast16_t iy = y - _panel_position[panel_index].y;
        if (r & 1)
        {
          std::swap(ix, iy);
        }
        if (ix >= single_width) { continue; }
        if (iy >= single_height) { continue; }
        if (r)
        {
          r = 1 << r;
          if (0b11001100 & r) { ix = single_width  - ix - 1; }
          if (0b10010110 & r) { iy = single_height - iy - 1; }
        }

        Panel_HUB75::_draw_pixel_inner(ix + panel_index * single_width, iy, rawcolor);
      }
    }
  }

  uint32_t Panel_HUB75_Multi::_read_pixel_inner(uint_fast16_t x, uint_fast16_t y)
  {
    uint32_t indexmask = _x_hitcheck_mask[x >> _x_hitcheck_shift] & _y_hitcheck_mask[y >> _y_hitcheck_shift];
    if (!indexmask) { return 0; }
    {
      auto single_width = _config_detail.single_width;
      auto single_height = _config_detail.single_height;
      for (uint_fast8_t panel_index = 0; indexmask; ++panel_index, indexmask >>= 1)
      {
        if (indexmask & 1)
        {
          uint_fast8_t r = _panel_position[panel_index].rotation;
          uint_fast16_t ix = x - _panel_position[panel_index].x;
          uint_fast16_t iy = y - _panel_position[panel_index].y;
          if (r & 1)
          {
            std::swap(ix, iy);
          }
          if (ix >= single_width) { continue; }
          if (iy >= single_height) { continue; }
          if (r)
          {
            r = 1 << r;
            if (0b11001100 & r) { ix = single_width  - ix - 1; }
            if (0b10010110 & r) { iy = single_height - iy - 1; }
          }
          return Panel_HUB75::_read_pixel_inner(ix + panel_index * single_width, iy);
        }
      }
    }
    return 0;
  }

  bool Panel_HUB75_Multi::_init_hitcheck(void)
  {
    if (_panel_position == nullptr)
    {
      auto panel_count = _config_detail.panel_count;
      if (panel_count < 1)
      {
        return false;
      }
      _panel_position_count = panel_count;
      _panel_position = (panel_position_t*)heap_alloc_dma(panel_count * sizeof(panel_position_t));
      memset(_panel_position, 0, panel_count * sizeof(panel_position_t));
      memset(_x_hitcheck_mask, 0, sizeof(_x_hitcheck_mask));
      memset(_y_hitcheck_mask, 0, sizeof(_y_hitcheck_mask));

      uint32_t s = 0;
      uint32_t tmp = _cfg.panel_width - 1;
      while (tmp >>= 1) { ++s; }
      _x_hitcheck_shift = (s > 2) ? (s - 2) : 0;

      s = 0;
      tmp = _cfg.panel_height - 1;
      while (tmp >>= 1) { ++s; }
      _y_hitcheck_shift = (s > 2) ? (s - 2) : 0;
    }
    return _panel_position != nullptr;
  }

  bool Panel_HUB75_Multi::setPanelPosition(uint_fast8_t index, uint_fast16_t x, uint_fast16_t y, uint_fast8_t rotation)
  {
    if (!_init_hitcheck()) { return false; }
    auto panel_count = _config_detail.panel_count;
    if (index < panel_count)
    {
      _panel_position[index].x = x;
      _panel_position[index].y = y;
      _panel_position[index].rotation = rotation;

      static constexpr const size_t x_size = sizeof(_x_hitcheck_mask) / sizeof(_x_hitcheck_mask[0]);
      static constexpr const size_t y_size = sizeof(_y_hitcheck_mask) / sizeof(_y_hitcheck_mask[0]);

      uint32_t mask_index = 1 << index;

      auto w = _config_detail.single_width;
      auto h = _config_detail.single_height;
      if (rotation & 1)
      {
        std::swap(w, h);
      }
      for (size_t i = 0; i < x_size; ++i)
      {
        _x_hitcheck_mask[i] &= ~mask_index;
      }
      uint_fast16_t xe = (x + w - 1) >> _x_hitcheck_shift;
      if (xe > x_size - 1) { xe = x_size - 1; }
      x >>= _x_hitcheck_shift;
      while (x <= xe)
      {
        _x_hitcheck_mask[x] |= mask_index;
        ++x;
      }

      for (size_t i = 0; i < y_size; ++i)
      {
        _y_hitcheck_mask[i] &= ~mask_index;
      }
      uint_fast16_t ye = (y + h - 1) >> _y_hitcheck_shift;
      if (ye > y_size - 1) { ye = y_size - 1; }
      y >>= _y_hitcheck_shift;
      while (y <= ye)
      {
        _y_hitcheck_mask[y] |= mask_index;
        ++y;
      }
    }
    return true;
  }

//----------------------------------------------------------------------------
 }
}
