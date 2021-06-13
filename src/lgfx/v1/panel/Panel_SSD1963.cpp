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
#include "Panel_SSD1963.hpp"

#include "../Bus.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Panel_SSD1963::init(bool use_reset)
  {
    if (!Panel_LCD::init(use_reset)) return false;

    startWrite();

    writeCommand(0xB0, 1);
    writeData(0x20, 2);
    writeData(__builtin_bswap16(_cfg.panel_width  - 1), 2);
    writeData(__builtin_bswap16(_cfg.panel_height - 1), 2);
    writeData(0x2D, 1);

    endWrite();

    return true;
  }

  void Panel_SSD1963::setHSync(std::uint_fast16_t front, std::uint_fast16_t sync, std::uint_fast16_t back, std::uint_fast16_t move, std::uint_fast16_t lpspp)
  {
    startWrite();
    std::uint_fast16_t ht = _cfg.panel_width + front + sync + back;
    std::uint_fast16_t hpw = sync;
    std::uint_fast16_t hps = move + sync + back;
    std::uint_fast16_t lps = move;

    writeCommand(0xB4, 1);
    writeData(__builtin_bswap16( ht-1), 2);
    writeData(__builtin_bswap16(hps  ), 2);
    writeData(                  hpw-1 , 1);
    writeData(__builtin_bswap16(lps  ), 2);
    writeData(                  lpspp , 1);

    _bus->flush();
    endWrite();
  }

  void Panel_SSD1963::setVSync(std::uint_fast16_t front, std::uint_fast16_t sync, std::uint_fast16_t back, std::uint_fast16_t move)
  {
    startWrite();
    std::uint_fast16_t vt = _cfg.panel_height + front + sync + back;
    std::uint_fast16_t vpw = sync;
    std::uint_fast16_t vps = move + sync + back;
    std::uint_fast16_t fps = move;

    writeCommand(0xB6, 1);
    writeData(__builtin_bswap16( vt-1), 2);
    writeData(__builtin_bswap16(vps  ), 2);
    writeData(                  vpw-1 , 1);
    writeData(__builtin_bswap16(fps  ), 2);

    _bus->flush();
    endWrite();
  }

  void Panel_SSD1963::setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye)
  {
    set_window(xs, ys, xe, ye, CMD_RAMWR);
  }

  void Panel_SSD1963::drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor)
  {
    bool tr = _in_transaction;
    if (!tr) begin_transaction();

    set_window(x, y, x, y, CMD_RAMWR);
    _bus->writeData(rawcolor, _write_bits);

    if (!tr) end_transaction();
  }

  void Panel_SSD1963::writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor)
  {
    std::uint32_t len = w * h;
    std::uint_fast16_t xe = w + x - 1;
    std::uint_fast16_t ye = y + h - 1;
    set_window(x, y, xe, ye, CMD_RAMWR);
    _bus->writeDataRepeat(rawcolor, _write_bits, len);
  }

  void Panel_SSD1963::setRotation(std::uint_fast8_t r)
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

  void Panel_SSD1963::set_window(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye, std::uint32_t cmd)
  {
    std::uint_fast8_t rb = 1u << _internal_rotation;
    if (_internal_rotation & 1)
    {
      std::swap(xs, ys);
      std::swap(xe, ye);
    }

    if (xs != _xs || xe != _xe)
    {
      _xs = xs;
      _xe = xe;
      _bus->writeCommand(CMD_CASET, 8);
      if (rb & 0b11000110) // case 1:2:6:7:
      {
        std::swap(xs, xe);
        xs = _cfg.panel_width - xs - 1;
        xe = _cfg.panel_width - xe - 1;
      }
      xs += _colstart;
      xe += _colstart;
      _bus->writeData(xs >> 8 | (xs & 0xFF) << 8 | (xe << 8 | xe >> 8) << 16, 32);
    }
    if (ys != _ys || ye != _ye)
    {
      _ys = ys;
      _ye = ye;
      _bus->writeCommand(CMD_RASET, 8);
      if (rb & 0b10011100) // case 2:3:4:7:
      {
        std::swap(ys, ye);
        ys = _cfg.panel_height - ys - 1;
        ye = _cfg.panel_height - ye - 1;
      }
      ys += _rowstart;
      ye += _rowstart;
      _bus->writeData(ys >> 8 | (ys & 0xFF) << 8 | (ye << 8 | ye >> 8) << 16, 32);
    }
    _bus->writeCommand(cmd, 8);
  }

//----------------------------------------------------------------------------
 }
}
