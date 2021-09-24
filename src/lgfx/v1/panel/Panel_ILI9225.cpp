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
#include "Panel_ILI9225.hpp"

#include "../../internal/memory.h"
#include "../misc/colortype.hpp"
#include "../Bus.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Panel_ILI9225::setInvert(bool invert)
  {
    _invert = invert;
    startWrite();
    _bus->writeCommand(CMD_DISPLAY_CTRL1, 8);
    uint_fast16_t data = (invert ^ _cfg.invert)
                            ? getSwap16(0x1013)
                            : getSwap16(0x1017)
                            ;
    _bus->writeData(data, 16);
    endWrite();
  }

  void Panel_ILI9225::setSleep(bool flg)
  {
    startWrite();
    _bus->writeCommand(CMD_POWER_CTRL1, 8);
    uint_fast16_t data = flg 
                            ? getSwap16(0x0802)
                            : getSwap16(0x0800)
                            ;
    _bus->writeData(data, 16);
    endWrite();
  }

  void Panel_ILI9225::setPowerSave(bool flg)
  {
    startWrite();
    _bus->writeCommand(CMD_POWER_CTRL1, 8);
    uint_fast16_t data = flg 
                            ? getSwap16(0x0801)
                            : getSwap16(0x0800)
                            ;
    _bus->writeData(data, 16);
    endWrite();
  }

  void Panel_ILI9225::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
  {
    set_window(xs, ys, xe, ye, CMD_RAMWR);
  }

  void Panel_ILI9225::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    bool tr = _in_transaction;
    if (!tr) begin_transaction();

    set_window(x, y, x, y, CMD_RAMWR);
    _bus->writeData(rawcolor, _write_bits);

    if (!tr) end_transaction();
  }

  void Panel_ILI9225::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint32_t len = w * h;
    uint_fast16_t xe = w + x - 1;
    uint_fast16_t ye = y + h - 1;
    set_window(x, y, xe, ye, CMD_RAMWR);
    _bus->writeDataRepeat(rawcolor, _write_bits, len);
  }

  color_depth_t Panel_ILI9225::setColorDepth(color_depth_t depth)
  {
    setColorDepth_impl(depth);
    _write_bits = _write_depth & color_depth_t::bit_mask;
    _read_bits = _read_depth & color_depth_t::bit_mask;

    update_madctl();
    return _write_depth;
  }

  void Panel_ILI9225::setRotation(uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

    _width  = _cfg.panel_width;
    _height = _cfg.panel_height;
    if (_internal_rotation & 1) std::swap(_width, _height);

    _colstart = _cfg.offset_x;
    _rowstart = _cfg.offset_y;

    _xs = _xe = _ys = _ye = INT16_MAX;

    update_madctl();
  }

  void Panel_ILI9225::update_madctl(void)
  {
    if (_bus == nullptr) return;

    startWrite();
    _bus->writeCommand(CMD_ENTRY_MODE, 8);
    uint32_t data = 0;
    switch (_internal_rotation)
    {
    case 0: data = 0x0030; break;
    case 1: data = 0x0028; break;
    case 2: data = 0x0000; break;
    case 3: data = 0x0018; break;
    case 4: data = 0x0010; break;
    case 5: data = 0x0038; break;
    case 6: data = 0x0020; break;
    case 7: data = 0x0008; break;
    }
    if (!_cfg.rgb_order) data |= 0x1000;
    if (_write_bits != 16) data |= 0x0300;
    _bus->writeData(getSwap16(data), 16);
    endWrite();
  }

  void Panel_ILI9225::set_window(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye, uint32_t cmd)
  {
    if (_internal_rotation & 1)
    {
      std::swap(xs, ys);
      std::swap(xe, ye);
    }

    uint_fast8_t rb = 1u << _internal_rotation;
    if (xs != _xs || xe != _xe)
    {
      auto tmp = xs;
      _xs = xs;
      _xe = xe;

      if (rb & 0b11000110) // case 1:2:6:7:
      {
        xs = _cfg.panel_width - xe - 1;
        xe = _cfg.panel_width - tmp - 1;
        tmp = xe;
      }

      _bus->writeCommand(CMD_H_ADDR2, 8);
      xs += _colstart;
      _bus->writeData(xs >> 8 | xs << 8, 16);

      _bus->writeCommand(CMD_H_ADDR1, 8);
      xe += _colstart;
      _bus->writeData(xe >> 8 | xe << 8, 16);

      _bus->writeCommand(0x20, 8);
      tmp += _colstart;
      _bus->writeData(tmp >> 8 | tmp << 8, 16);
    }
    if (ys != _ys || ye != _ye)
    {
      auto tmp = ys;
      _ys = ys;
      _ye = ye;

      if (rb & 0b10011100) // case 2:3:4:7:
      {
        ys = _cfg.panel_height - ye - 1;
        ye = _cfg.panel_height - tmp - 1;
        tmp = ye;
      }

      _bus->writeCommand(CMD_V_ADDR2, 8);
      ys += _rowstart;
      _bus->writeData(ys >> 8 | ys << 8, 16);

      _bus->writeCommand(CMD_V_ADDR1, 8);
      ye += _rowstart;
      _bus->writeData(ye >> 8 | ye << 8, 16);

      _bus->writeCommand(0x21, 8);
      tmp += _rowstart;
      _bus->writeData(tmp >> 8 | tmp << 8, 16);
    }
    _bus->writeCommand(cmd, 8);
  }

//----------------------------------------------------------------------------
 }
}
