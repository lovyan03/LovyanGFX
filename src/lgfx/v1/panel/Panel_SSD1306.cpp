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

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr uint8_t Bayer[16] = { 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88 };

  color_depth_t Panel_1bitOLED::setColorDepth(color_depth_t depth)
  {
    _write_bits = 16;
    _read_bits = 16;
    _write_depth = color_depth_t::rgb565_2Byte;
    _read_depth = color_depth_t::rgb565_2Byte;
    return color_depth_t::rgb565_2Byte;
  }

  void Panel_1bitOLED::waitDisplay(void)
  {
    _bus->wait();
  }

  bool Panel_1bitOLED::displayBusy(void)
  {
    return _bus->busy();
  }

  size_t Panel_1bitOLED::_get_buffer_length(void) const
  {
    return ((_cfg.panel_height + 7) >> 3) * _cfg.panel_width;
  }

  bool Panel_1bitOLED::init(bool use_reset)
  {
    if (!Panel_HasBuffer::init(use_reset))
    {
      return false;
    }
    //memset(_buf, 0, len);

    _bus->beginRead();
    uint8_t buf;
    bool res = _bus->readBytes(&buf, 1, true);
    _bus->endTransaction();

    if (!res)
    {
      return false;
    }

    startWrite(true);

    for (size_t i = 0; auto cmds = getInitCommands(i); i++)
    {
      size_t idx = 0;
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
    _invert = invert;
    startWrite();
    _bus->writeCommand((invert ^ _cfg.invert) ? CMD_INVERTDISPLAY : CMD_NORMALDISPLAY, 8);
    endWrite();
  }

  void Panel_1bitOLED::setSleep(bool flg)
  {
    startWrite();
    _bus->writeCommand(flg ? CMD_DISP_OFF : CMD_DISP_ON, 8);
    endWrite();
  }

  void Panel_1bitOLED::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _xs = xs;
    _ys = ys;
    _xe = xe;
    _ye = ye;
    _update_transferred_rect(xs, ys, xe, ye);

    swap565_t color;
    color.raw = rawcolor;
    uint32_t value = (color.R8() + (color.G8() << 1) + color.B8()) >> 2;

    y = ys;
    do
    {
      x = xs;
      uint32_t idx = x + (y >> 3) * _cfg.panel_width;
      auto btbl = &Bayer[(y & 3) << 2];
      uint32_t mask = 1 << (y&7);
      do
      {
        bool flg = 256 <= value + btbl[x & 3];
        if (flg) _buf[idx] |=   mask;
        else     _buf[idx] &= ~ mask;
        ++idx;
      } while (++x <= xe);
    } while (++y <= ye);
  }

  void Panel_1bitOLED::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _update_transferred_rect(xs, ys, xe, ye);

    auto readbuf = (swap565_t*)alloca(w * sizeof(swap565_t));
    auto sx = param->src_x32;
    h += y;
    do
    {
      uint32_t prev_pos = 0, new_pos = 0;
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

  void Panel_1bitOLED::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    {
      uint_fast16_t xs = _xs;
      uint_fast16_t xe = _xe;
      uint_fast16_t ys = _ys;
      uint_fast16_t ye = _ye;
      _update_transferred_rect(xs, ys, xe, ye);
    }
    uint_fast16_t xs   = _xs  ;
    uint_fast16_t ys   = _ys  ;
    uint_fast16_t xe   = _xe  ;
    uint_fast16_t ye   = _ye  ;
    uint_fast16_t xpos = _xpos;
    uint_fast16_t ypos = _ypos;

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

  void Panel_1bitOLED::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    auto readbuf = (swap565_t*)alloca(w * sizeof(swap565_t));
    param->src_data = readbuf;
    int32_t readpos = 0;
    h += y;
    do
    {
      uint32_t idx = 0;
      do
      {
        readbuf[idx] = _read_pixel(x + idx, y) ? ~0u : 0;
      } while (++idx != w);
      param->src_x32 = 0;
      readpos = param->fp_copy(dst, readpos, readpos + w, param);
    } while (++y < h);
  }

  void Panel_1bitOLED::_draw_pixel(uint_fast16_t x, uint_fast16_t y, uint32_t value)
  {
    _rotate_pos(x, y);
    uint32_t idx = x + (y >> 3) * _cfg.panel_width;
    uint32_t mask = 1 << (y&7);
    bool flg = 256 <= value + Bayer[(x & 3) | (y & 3) << 2];
    if (flg) _buf[idx] |=  mask;
    else     _buf[idx] &= ~mask;
  }

  bool Panel_1bitOLED::_read_pixel(uint_fast16_t x, uint_fast16_t y)
  {
    _rotate_pos(x, y);
    uint32_t idx = x + (y >> 3) * _cfg.panel_width;
    return _buf[idx] & (1 << (y&7));
  }

  void Panel_1bitOLED::_update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye)
  {
    _rotate_pos(xs, ys, xe, ye);
    _range_mod.left   = std::min<int32_t>(xs, _range_mod.left);
    _range_mod.right  = std::max<int32_t>(xe, _range_mod.right);
    _range_mod.top    = std::min<int32_t>(ys, _range_mod.top);
    _range_mod.bottom = std::max<int32_t>(ye, _range_mod.bottom);
  }

//----------------------------------------------------------------------------

  void Panel_SSD1306::setBrightness(uint8_t brightness)
  {
    startWrite();
    _bus->writeCommand(CMD_SETPRECHARGE | ((brightness+15)/17)*0x11 << 8, 16);
    _bus->writeCommand(CMD_SETVCOMDETECT | (brightness>>1) << 8, 16);
    _bus->writeCommand(CMD_SETCONTRAST | brightness << 8, 16);
    endWrite();
  }

  void Panel_SSD1306::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      _range_mod.left   = std::min<int16_t>(_range_mod.left  , x        );
      _range_mod.right  = std::max<int16_t>(_range_mod.right , x + w - 1);
      _range_mod.top    = std::min<int16_t>(_range_mod.top   , y        );
      _range_mod.bottom = std::max<int16_t>(_range_mod.bottom, y + h - 1);
    }
    if (_range_mod.empty()) { return; }

    uint_fast8_t xs = _range_mod.left;
    uint_fast8_t xe = _range_mod.right;
    _bus->writeCommand(CMD_COLUMNADDR| (xs + _cfg.offset_x) << 8 | (xe + _cfg.offset_x) << 16, 24);

    uint_fast8_t ys = _range_mod.top    >> 3;
    uint_fast8_t ye = _range_mod.bottom >> 3;
    _bus->writeCommand(CMD_PAGEADDR | (ys + (_cfg.offset_y >> 3)) << 8 | (ye + (_cfg.offset_y >> 3)) << 16, 24);
    do
    {
      auto buf = &_buf[xs + ys * _cfg.panel_width];
      _bus->writeBytes(buf, xe - xs + 1, true, true);
    } while (++ys <= ye);

    _range_mod.top    = INT16_MAX;
    _range_mod.left   = INT16_MAX;
    _range_mod.right  = 0;
    _range_mod.bottom = 0;
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

  void Panel_SH110x::setBrightness(uint8_t brightness)
  {
    startWrite();
    _bus->writeCommand(CMD_SETCONTRAST | brightness << 8, 16);
    endWrite();
  }

  void Panel_SH110x::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      _range_mod.left   = std::min<int16_t>(_range_mod.left  , x        );
      _range_mod.right  = std::max<int16_t>(_range_mod.right , x + w - 1);
      _range_mod.top    = std::min<int16_t>(_range_mod.top   , y        );
      _range_mod.bottom = std::max<int16_t>(_range_mod.bottom, y + h - 1);
    }
    if (_range_mod.empty()) { return; }

    uint_fast8_t xs = _range_mod.left ;
    uint_fast8_t xe = _range_mod.right;
    uint_fast8_t ys = _range_mod.top    >> 3;
    uint_fast8_t ye = _range_mod.bottom >> 3;

    uint_fast8_t offset_y = _cfg.offset_y >> 3;

    do
    {
      _bus->writeCommand(  CMD_SETPAGEADDR | (ys + offset_y)
                        | (CMD_SETHIGHCOLUMN | (xs >> 4)) << 8
                        | (CMD_SETLOWCOLUMN  | (xs & 0x0F)) << 16
                        , 24);
      auto buf = &_buf[xs + ys * _cfg.panel_width];
      _bus->writeBytes(buf, xe - xs + 1, true, true);
    } while (++ys <= ye);

    _range_mod.top    = INT16_MAX;
    _range_mod.left   = INT16_MAX;
    _range_mod.right  = 0;
    _range_mod.bottom = 0;
  }

//----------------------------------------------------------------------------
 }
}
