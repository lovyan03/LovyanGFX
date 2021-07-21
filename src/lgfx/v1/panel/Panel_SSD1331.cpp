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
#include "Panel_SSD1331.hpp"

#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Panel_SSD1331::setBrightness(uint8_t brightness)
  {
    startWrite();
    _bus->writeCommand(CMD_MASTERCURRENT | (brightness>>4) << 8, 16);
    _bus->writeCommand(CMD_PRECHARGELEVEL | (brightness>>3) << 8, 16);
    _bus->writeCommand(0x81 | brightness << 8, 16);
    _bus->writeCommand(0x82 | brightness << 8, 16);
    _bus->writeCommand(0x83 | brightness << 8, 16);
    endWrite();
  }

  void Panel_SSD1331::setInvert(bool invert)
  {
    startWrite();
    _invert = invert;
    _bus->writeCommand((invert ^ _cfg.invert) ? CMD_INVERTDISPLAY : CMD_NORMALDISPLAY, 8);
    endWrite();
  }
/*
  color_depth_t Panel_SSD1331::setColorDepth(color_depth_t depth)
  {
    setColorDepth_impl(depth);
    _write_bits = _write_depth & color_depth_t::bit_mask;
    _read_bits = _read_depth & color_depth_t::bit_mask;

    update_madctl();
    return _write_depth;
  }
  void Panel_SSD1331::setRotation(uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

    auto ox = _cfg.offset_x;
    auto oy = _cfg.offset_y;
    auto pw = _cfg.panel_width;
    auto ph = _cfg.panel_height;
    auto mw = _cfg.memory_width;
    auto mh = _cfg.memory_height;
    if (_internal_rotation & 1)
    {
      std::swap(ox, oy);
      std::swap(pw, ph);
      std::swap(mw, mh);
    }
    _width  = pw;
    _height = ph;
    _colstart = (_internal_rotation & 2)
              ? mw - (pw + ox) : ox;

    _rowstart = (((((_internal_rotation >> 1) & 2) + _internal_rotation) + 1) & 2)
              ? mh - (ph + oy) : oy;

    _xs = _xe = _ys = _ye = INT16_MAX;

    update_madctl();
  }
*/
  void Panel_SSD1331::update_madctl(void)
  {
    static constexpr uint8_t madctl_table[] = {
      0b00100000,
      0b00100011,
      0b00110010,
      0b00110001,
      0b00110000,
      0b00100001,
      0b00100010,
      0b00110011,
    };

    if (_bus == nullptr) return;

    startWrite();
    _bus->writeCommand(CMD_MADCTL, 8);
    _bus->writeCommand( madctl_table[_internal_rotation]
                      | (_write_bits == 16 ? 0x40 : 0x00)
                      | (_cfg.rgb_order    ? 0x00 : 0x04)
                      , 8);
    endWrite();
  }

  void Panel_SSD1331::setSleep(bool flg)
  {
    startWrite();
    write_command(flg ? CMD_DISPLAYOFF : CMD_DISPLAYON);
    endWrite();
  }

  void Panel_SSD1331::setPowerSave(bool flg)
  {
    startWrite();
    write_command(flg ? CMD_DISPONDIMMER : CMD_DISPLAYON);
    endWrite();
  }

  void Panel_SSD1331::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
  {
    if (_need_delay)
    {
      auto us = lgfx::micros() - _last_us;
      if (us < _need_delay)
      {
        delayMicroseconds(_need_delay - us);
      }
      _need_delay = 0;
    }

    if (xs != _xs || xe != _xe)
    {
      _xs = xs;
      _xe = xe;
      xs += _colstart;
      xe += _colstart;
      _bus->writeCommand((_internal_rotation & 1 ? CMD_RASET : CMD_CASET) | xs << 8 | xe << 16, 24);
    }
    if (ys != _ys || ye != _ye)
    {
      _ys = ys;
      _ye = ye;
      ys += _rowstart;
      ye += _rowstart;
      _bus->writeCommand((_internal_rotation & 1 ? CMD_CASET : CMD_RASET) | ys << 8 | ye << 16, 24);
    }
    _bus->writeCommand(CMD_RAMWR, 8);
  }

  void Panel_SSD1331::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    bool need_transaction = !getStartCount();
    if (need_transaction) startWrite();
    writeFillRectPreclipped(x, y, 1, 1, rawcolor);
    if (need_transaction) endWrite();
  }

  void Panel_SSD1331::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    auto us = lgfx::micros() - _last_us;
    if (us < _need_delay)
    {
      delayMicroseconds(_need_delay - us);
    }
    _bus->writeCommand(CMD_DRAWRECT, 8);
    x += _colstart;
    y += _rowstart;
    auto xe = x+w-1;
    auto ye = y+h-1;

    if (_internal_rotation & 1)
    {
      std::swap(x, y);
      std::swap(xe, ye);
    }
    _bus->writeCommand(x | y << 8 | xe << 16 | ye << 24, 32);
    uint32_t data;
    if (_write_bits == 16)
    {
      swap565_t color = rawcolor;
      data = color.R6() | color.G6() << 8 | color.B6() << 16;
    }
    else
    {
      rgb332_t color = rawcolor;
      data = color.R6() | color.G6() << 8 | color.B6() << 16;
    }

    _bus->writeCommand(data, 24);
    _need_delay = (w * h >> 3);
    _bus->writeCommand(data, 24);
    _last_us = lgfx::micros();
  }

  void Panel_SSD1331::copyRect(uint_fast16_t dst_x, uint_fast16_t dst_y, uint_fast16_t w, uint_fast16_t h, uint_fast16_t src_x, uint_fast16_t src_y)
  {
    auto us = lgfx::micros() - _last_us;
    if (us < _need_delay)
    {
      delayMicroseconds(_need_delay - us);
    }
    startWrite();
    _bus->writeCommand(CMD_COPY, 8);

    src_x += _colstart;
    dst_x += _colstart;
    src_y += _rowstart;
    dst_y += _rowstart;
    auto xe = src_x+w-1;
    auto ye = src_y+h-1;

    if (_internal_rotation & 1)
    {
      std::swap(src_x, src_y);
      std::swap(dst_x, dst_y);
      std::swap(xe, ye);
    }

    _bus->writeCommand(src_x | src_y << 8 , 16);
    _bus->writeCommand(xe    | ye    << 8 , 16);
    _bus->writeCommand(dst_x | dst_y << 8 , 16);
    _need_delay = (w * h >> 3);
    _last_us = lgfx::micros();
    endWrite();
  }

//----------------------------------------------------------------------------
 }
}
