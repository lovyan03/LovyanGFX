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
#include "Panel_SSD1306.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr std::uint8_t Bayer[16] = { 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88 };

  Panel_1bitOLED::~Panel_1bitOLED(void)
  {
    if (_buf) heap_free(_buf);
  }

  color_depth_t Panel_1bitOLED::setColorDepth(color_depth_t depth)
  {
    _write_bits = 16;
    _read_bits = 16;
    _write_depth = color_depth_t::rgb565_2Byte;
    _read_depth = color_depth_t::rgb565_2Byte;
    return color_depth_t::rgb565_2Byte;
  }

  void Panel_1bitOLED::setRotation(std::uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

    _width  = _cfg.panel_width;
    _height = _cfg.panel_height;
    if (_internal_rotation & 1) std::swap(_width, _height);
  }

  void Panel_1bitOLED::beginTransaction(void)
  {
    _bus->beginTransaction();
    cs_control(false);
  }

  void Panel_1bitOLED::endTransaction(void)
  {
    _bus->endTransaction();
    cs_control(true);
  }

  void Panel_1bitOLED::waitDisplay(void)
  {
    _bus->wait();
  }

  bool Panel_1bitOLED::displayBusy(void)
  {
    return _bus->busy();
  }

  bool Panel_1bitOLED::init(bool use_reset)
  {
    _range_new.top = 0;
    _range_new.left = 0;
    _range_new.right = _width - 1;
    _range_new.bottom = _height - 1;

    int len = ((_cfg.panel_height + 7) >> 3) * _cfg.panel_width;
    if (_buf) heap_free(_buf);
    _buf = static_cast<std::uint8_t*>(heap_alloc_dma(len));
    memset(_buf, 0, len);

    if (!Panel_Device::init(use_reset))
    {
      return false;
    }

    _bus->beginRead();
    std::uint8_t buf;
    bool res = _bus->readBytes(&buf, 1, true);
    _bus->endTransaction();

    if (!res)
    {
      return false;
    }

    startWrite(true);

    for (std::size_t i = 0; auto cmds = getInitCommands(i); i++)
    {
      std::size_t idx = 0;
      while (cmds[idx] != 0xFF || cmds[idx + 1] != 0xFF) ++idx;
      if (idx) { _bus->writeBytes(cmds, idx, false, true); }
    }

    setInvert(_invert);
    setRotation(_rotation);
    endWrite();

    return true;
  }

  void Panel_1bitOLED::setInvert(bool invert)
  {
    startWrite();
    _invert = invert;
    _bus->writeCommand((invert ^ _cfg.invert) ? CMD_INVERTDISPLAY : CMD_NORMALDISPLAY, 8);
    endWrite();
  }

  void Panel_1bitOLED::setSleep(bool flg)
  {
    startWrite();
    _bus->writeCommand(flg ? CMD_DISP_OFF : CMD_DISP_ON, 8);
    endWrite();
  }

  void Panel_1bitOLED::setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye)
  {
    _xpos = xs;
    _ypos = ys;
    _xs = xs;
    _ys = ys;
    _xe = xe;
    _ye = ye;
  }

  void Panel_1bitOLED::drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor)
  {
    bool need_transaction = !getStartCount();
    if (need_transaction) startWrite();
    writeFillRectPreclipped(x, y, 1, 1, rawcolor);
    if (need_transaction) endWrite();
  }

  void Panel_1bitOLED::writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor)
  {
    std::uint32_t xs = x, xe = x + w - 1;
    std::uint32_t ys = y, ye = y + h - 1;
    _xs = xs;
    _ys = ys;
    _xe = xe;
    _ye = ye;
    _update_transferred_rect(xs, ys, xe, ye);

    swap565_t color;
    color.raw = rawcolor;
    std::uint32_t value = (color.R8() + (color.G8() << 1) + color.B8()) >> 2;

    y = ys;
    do
    {
      x = xs;
      std::uint32_t idx = x + (y >> 3) * _cfg.panel_width;
      auto btbl = &Bayer[(y & 3) << 2];
      std::uint32_t mask = 1 << (y&7);
      do
      {
        bool flg = 256 <= value + btbl[x & 3];
        if (flg) _buf[idx] |=   mask;
        else     _buf[idx] &= ~ mask;
        ++idx;
      } while (++x <= xe);
    } while (++y <= ye);
  }

  void Panel_1bitOLED::writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    std::uint32_t xs = x, xe = x + w - 1;
    std::uint32_t ys = y, ye = y + h - 1;
    _update_transferred_rect(xs, ys, xe, ye);

    swap565_t readbuf[w];
    auto sx = param->src_x32;
    h += y;
    do
    {
      std::uint32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          do
          {
            auto color = readbuf[prev_pos];
            _draw_pixel(x + prev_pos, y, (color.R8() + (color.G8() << 1) + color.B8()) >> 2);
          } while (new_pos != ++prev_pos);
        }
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
    } while (++y < h);
  }

  void Panel_1bitOLED::writeBlock(std::uint32_t rawcolor, std::uint32_t length)
  {
    std::uint32_t xs = _xs;
    std::uint32_t xe = _xe;
    std::uint32_t ys = _ys;
    std::uint32_t ye = _ye;
    std::uint32_t xpos = _xpos;
    std::uint32_t ypos = _ypos;
    do
    {
      auto len = std::min<std::uint32_t>(length, xe + 1 - xpos);
      writeFillRectPreclipped(xpos, ypos, len, 1, rawcolor);
      xpos += len;
      if (xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye)
        {
          ypos = ys;
        }
      }
      length -= len;
    } while (length);
    _xs = xs;
    _xe = xe;
    _ys = ys;
    _ye = ye;
    _xpos = xpos;
    _ypos = ypos;
  }

  void Panel_1bitOLED::writePixels(pixelcopy_t* param, std::uint32_t length)
  {
    {
      std::uint32_t xs = _xs;
      std::uint32_t xe = _xe;
      std::uint32_t ys = _ys;
      std::uint32_t ye = _ye;
      _update_transferred_rect(xs, ys, xe, ye);
    }
    std::uint32_t xs   = _xs  ;
    std::uint32_t ys   = _ys  ;
    std::uint32_t xe   = _xe  ;
    std::uint32_t ye   = _ye  ;
    std::uint32_t xpos = _xpos;
    std::uint32_t ypos = _ypos;

    static constexpr uint32_t buflen = 16;
    swap565_t colors[buflen];
    int bufpos = buflen;
    do
    {
      if (bufpos == buflen) {
        param->fp_copy(colors, 0, std::min(length, buflen), param);
        bufpos = 0;
      }
      auto color = colors[bufpos++];
      _draw_pixel(xpos, ypos, (color.R8() + (color.G8() << 1) + color.B8()) >> 2);
      if (++xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye)
        {
          ypos = ys;
        }
      }
    } while (--length);
    _xpos = xpos;
    _ypos = ypos;
  }

  void Panel_1bitOLED::readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    swap565_t readbuf[w];
    param->src_data = readbuf;
    std::int32_t readpos = 0;
    h += y;
    do
    {
      std::uint32_t idx = 0;
      do
      {
        readbuf[idx] = _read_pixel(x + idx, y) ? ~0u : 0;
      } while (++idx != w);
      param->src_x32 = 0;
      readpos = param->fp_copy(dst, readpos, readpos + w, param);
    } while (++y < h);
  }

  void Panel_1bitOLED::_draw_pixel(std::int32_t x, std::int32_t y, std::uint32_t value)
  {
    if (_internal_rotation & 1) { std::swap(x, y); }
    switch (_internal_rotation) {
    case 1: case 2: case 6: case 7:  x = _cfg.panel_width - x - 1; break;
    default: break;
    }
    switch (_internal_rotation) {
    case 2: case 3: case 4: case 7:  y = _cfg.panel_height - y - 1; break;
    default: break;
    }
    std::uint32_t idx = x + (y >> 3) * _cfg.panel_width;
    std::uint32_t mask = 1 << (y&7);
    bool flg = 256 <= value + Bayer[(x & 3) | (y & 3) << 2];
    if (flg) _buf[idx] |=  mask;
    else     _buf[idx] &= ~mask;
  }

  bool Panel_1bitOLED::_read_pixel(std::int32_t x, std::int32_t y)
  {
    if (_internal_rotation & 1) { std::swap(x, y); }
    switch (_internal_rotation) {
    case 1: case 2: case 6: case 7:  x = _cfg.panel_width - x - 1; break;
    default: break;
    }
    switch (_internal_rotation) {
    case 2: case 3: case 4: case 7:  y = _cfg.panel_height - y - 1; break;
    default: break;
    }
    std::uint32_t idx = x + (y >> 3) * _cfg.panel_width;
    return _buf[idx] & (1 << (y&7));
  }

  void Panel_1bitOLED::_update_transferred_rect(std::uint32_t &xs, std::uint32_t &ys, std::uint32_t &xe, std::uint32_t &ye)
  {
    auto r = _internal_rotation;
    if (r & 1) { std::swap(xs, ys); std::swap(xe, ye); }
    if ((1<<r) & 0b11000110) // case 1:2:6:7:
    {
      std::swap(xs, xe);
      xs = _cfg.panel_width - 1 - xs;
      xe = _cfg.panel_width - 1 - xe;
    }
    if ((1<<r) & 0b10011100) // case 2:3:4:7:
    {
      std::swap(ys, ye);
      ys = _cfg.panel_height - 1 - ys;
      ye = _cfg.panel_height - 1 - ye;
    }
    _range_new.left   = std::min<std::int32_t>(xs, _range_new.left);
    _range_new.right  = std::max<std::int32_t>(xe, _range_new.right);
    _range_new.top    = std::min<std::int32_t>(ys, _range_new.top);
    _range_new.bottom = std::max<std::int32_t>(ye, _range_new.bottom);
  }

//----------------------------------------------------------------------------

  void Panel_SSD1306::setBrightness(std::uint8_t brightness)
  {
    startWrite();
    _bus->writeCommand(CMD_SETPRECHARGE | ((brightness+15)/17)*0x11 << 8, 16);
    _bus->writeCommand(CMD_SETVCOMDETECT | (brightness>>1) << 8, 16);
    _bus->writeCommand(CMD_SETCONTRAST | brightness << 8, 16);
    endWrite();
  }

  void Panel_SSD1306::display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      _range_new.left   = std::min<std::int16_t>(_range_new.left  , x        );
      _range_new.right  = std::max<std::int16_t>(_range_new.right , x + w - 1);
      _range_new.top    = std::min<std::int16_t>(_range_new.top   , y        );
      _range_new.bottom = std::max<std::int16_t>(_range_new.bottom, y + h - 1);
    }
    if (_range_new.empty()) { return; }

    std::uint_fast8_t xs = _range_new.left;
    std::uint_fast8_t xe = _range_new.right;
    _bus->writeCommand(CMD_COLUMNADDR| (xs + _cfg.offset_x) << 8 | (xe + _cfg.offset_x) << 16, 24);

    std::uint_fast8_t ys = _range_new.top    >> 3;
    std::uint_fast8_t ye = _range_new.bottom >> 3;
    _bus->writeCommand(CMD_PAGEADDR | (ys + (_cfg.offset_y >> 3)) << 8 | (ye + (_cfg.offset_y >> 3)) << 16, 24);
    _bus->endTransaction();
    _bus->beginTransaction();
    do
    {
      auto buf = &_buf[xs + ys * _cfg.panel_width];
      _bus->writeBytes(buf, xe - xs + 1, true, true);
    } while (++ys <= ye);

    _range_new.top    = INT16_MAX;
    _range_new.left   = INT16_MAX;
    _range_new.right  = 0;
    _range_new.bottom = 0;
  }

//----------------------------------------------------------------------------

  bool Panel_SH110x::init(bool use_reset)
  {
    if (!Panel_1bitOLED::init(use_reset))
    {
      return false;
    }

    startWrite(true);
    _bus->writeCommand(CMD_SETMULTIPLEX | (((_cfg.panel_width-1) & 0x7F) << 8), 16);
    _bus->writeCommand(CMD_SETDISPLAYOFFSET | ((uint8_t)(-_cfg.offset_x) << 8), 16);
    endWrite();

    return true;
  }

  void Panel_SH110x::beginTransaction(void)
  {
    _bus->beginTransaction();
    cs_control(false);
//    _bus->writeCommand(CMD_READMODIFYWRITE_END, 8);
  }

  void Panel_SH110x::setBrightness(std::uint8_t brightness)
  {
    startWrite();
    _bus->writeCommand(CMD_SETCONTRAST | brightness << 8, 16);
    endWrite();
  }

  void Panel_SH110x::display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      _range_new.left   = std::min<std::int16_t>(_range_new.left  , x        );
      _range_new.right  = std::max<std::int16_t>(_range_new.right , x + w - 1);
      _range_new.top    = std::min<std::int16_t>(_range_new.top   , y        );
      _range_new.bottom = std::max<std::int16_t>(_range_new.bottom, y + h - 1);
    }
    if (_range_new.empty()) { return; }

    std::uint_fast8_t xs = _range_new.left ;
    std::uint_fast8_t xe = _range_new.right;
    std::uint_fast8_t ys = _range_new.top    >> 3;
    std::uint_fast8_t ye = _range_new.bottom >> 3;

    std::uint_fast8_t offset_y = _cfg.offset_y >> 3;

    do
    {
      //_bus->endTransaction();
      //_bus->beginTransaction();
      _bus->writeCommand(  CMD_SETPAGEADDR | (ys + offset_y)
                        | (CMD_SETHIGHCOLUMN | (xs >> 4)) << 8
                        | (CMD_SETLOWCOLUMN  | (xs & 0x0F)) << 16
                        , 24);
      auto buf = &_buf[xs + ys * _cfg.panel_width];
      //_bus->endTransaction();
      //_bus->beginTransaction();
      _bus->writeBytes(buf, xe - xs + 1, true, true);
    } while (++ys <= ye);

    _range_new.top    = INT16_MAX;
    _range_new.left   = INT16_MAX;
    _range_new.right  = 0;
    _range_new.bottom = 0;
  }

//----------------------------------------------------------------------------
 }
}
