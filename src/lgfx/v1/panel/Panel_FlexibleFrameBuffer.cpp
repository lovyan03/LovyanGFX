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

#include "Panel_FlexibleFrameBuffer.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"
#include "../misc/common_function.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Panel_FlexibleFrameBuffer::init(bool use_reset)
  {
    setInvert(_invert);
    setRotation(_rotation);

    if (!Panel_Device::init(use_reset))
    {
      return false;
    }
    return true;
  }

  void Panel_FlexibleFrameBuffer::setRotation(uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    auto pw = _cfg.panel_width;
    auto ph = _cfg.panel_height;
    if (r & 1)
    {
      std::swap(pw, ph);
    }
    _width  = pw;
    _height = ph;
    _xe = pw-1;
    _ye = ph-1;
    _xs = 0;
    _ys = 0;
    _xpos = 0;
    _ypos = 0;
  }

  void Panel_FlexibleFrameBuffer::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
  {
    xs = std::max<uint_fast16_t>(0u, std::min<uint_fast16_t>(_width  - 1, xs));
    xe = std::max<uint_fast16_t>(0u, std::min<uint_fast16_t>(_width  - 1, xe));
    ys = std::max<uint_fast16_t>(0u, std::min<uint_fast16_t>(_height - 1, ys));
    ye = std::max<uint_fast16_t>(0u, std::min<uint_fast16_t>(_height - 1, ye));
    _xpos = xs;
    _xs = xs;
    _xe = xe;
    _ypos = ys;
    _ys = ys;
    _ye = ye;
  }

  void Panel_FlexibleFrameBuffer::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    uint_fast8_t r = _rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { y = _height - (y + 1); }
      if (r & 2)                  { x = _width  - (x + 1); }
      if (r & 1) { std::swap(x, y); }
    }

    _draw_pixel_inner(x, y, rawcolor);
  }

  uint32_t Panel_FlexibleFrameBuffer::readPixelPreclipped(uint_fast16_t x, uint_fast16_t y)
  {
    uint_fast8_t r = _rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { y = _height - (y + 1); }
      if (r & 2)                  { x = _width  - (x + 1); }
      if (r & 1) { std::swap(x, y); }
    }

    return _read_pixel_inner(x, y);
  }

  void Panel_FlexibleFrameBuffer::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast8_t r = _rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { y = _height - (y + h); }
      if (r & 2)                  { x = _width  - (x + w); }
      if (r & 1) { std::swap(x, y);  std::swap(w, h); }
    }

    _fill_rect_inner(x, y, w, h, rawcolor);
  }

  void Panel_FlexibleFrameBuffer::_fill_rect_inner(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    h += y;
    uint_fast16_t ie = x + w;
    do
    {
      auto i = x;
      do
      {
        _draw_pixel_inner(i, y, rawcolor);
      } while (++i != ie);
    } while (++y < h);
  }

  void Panel_FlexibleFrameBuffer::writeBlock(uint32_t rawcolor, uint32_t length)
  {
    do
    {
      uint32_t h = 1;
      auto w = std::min<uint32_t>(length, _xe + 1 - _xpos);
      if (length >= (w << 1) && _xpos == _xs)
      {
        h = std::min<uint32_t>(length / w, _ye + 1 - _ypos);
      }
      writeFillRectPreclipped(_xpos, _ypos, w, h, rawcolor);
      if ((_xpos += w) <= _xe) return;
      _xpos = _xs;
      if (_ye < (_ypos += h)) { _ypos = _ys; }
      length -= w * h;
    } while (length);
  }

  void Panel_FlexibleFrameBuffer::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    (void)use_dma;
    uint_fast16_t xs = _xs;
    uint_fast16_t xe = _xe;
    uint_fast16_t ys = _ys;
    uint_fast16_t ye = _ye;
    uint_fast16_t x = _xpos;
    uint_fast16_t y = _ypos;
    // const size_t bits = _write_bits;
    // auto k = _bitwidth * bits >> 3;

    uint_fast8_t r = _rotation;

    int_fast16_t ax = 1;
    int_fast16_t ay = 1;
    if ((1u << r) & 0b10010110) { y = _height - (y + 1); ys = _height - (ys + 1); ye = _height - (ye + 1); ay = -1; }
    if (r & 2)                  { x = _width  - (x + 1); xs = _width  - (xs + 1); xe = _width  - (xe + 1); ax = -1; }

    uint32_t rawcolor;
    if (r & 1)
    {
      do
      {
        param->fp_copy(&rawcolor, 0, 1, param);
        /// xとyを入れ替えて処理する;
        _draw_pixel_inner(y, x, rawcolor);
        if (x != xe)
        {
          x += ax;
        }
        else
        {
          x = xs;
          y = (y != ye) ? (y + ay) : ys;
        }
      } while (--length);
    }
    else
    {
      do
      {
        param->fp_copy(&rawcolor, 0, 1, param);
        _draw_pixel_inner(x, y, rawcolor);
        if (x != xe)
        {
          x += ax;
        }
        else
        {
          x = xs;
          y = (y != ye) ? (y + ay) : ys;
        }
      } while (--length);
    }

    if ((1u << r) & 0b10010110) { y = _height - (y + 1); }
    if (r & 2)                  { x = _width  - (x + 1); }
    _xpos = x;
    _ypos = y;
  }

  void Panel_FlexibleFrameBuffer::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool)
  {
    uint32_t sx32 = param->src_x32;
    size_t bytes = _write_bits >> 3;
    size_t len = w * bytes;
    auto pixelbuf = (uint8_t*)alloca((len + 7) & ~3);

    h += y;
    do
    {
      uint32_t pos = 0;
      uint32_t raw;
      do
      {
        auto pos2 = param->fp_copy(pixelbuf, pos, w, param);
        if (pos != pos2)
        {
          uint8_t* buf = &pixelbuf[pos * bytes];
          do
          {
            raw = *buf++;
            for (size_t by = 1; by < bytes; ++by)
            {
              raw += (*buf++) << (by * 8);
            }
            drawPixelPreclipped(x + pos, y, raw);
          } while (++pos != pos2);
        }
        if (pos != w)
        {
          pos = param->fp_skip(pos, w, param);
        }
      } while (pos < w);
      param->src_y++;
      param->src_x32 = sx32;
    } while (++y != h);
  }

  void Panel_FlexibleFrameBuffer::writeImageARGB(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param)
  {
    auto src_x = param->src_x;
    auto buffer = reinterpret_cast<argb8888_t*>(const_cast<void*>(param->src_data));
    auto bytes = param->dst_bits >> 3;
// ESP_LOGI("LGFX","DEBUG: %d %d", param->dst_bits, bytes);
    // uint8_t* dmabuf = _bus->getFlipBuffer(w * bytes);
    // memset(dmabuf, 0, w * bytes);
    // param->fp_copy(dmabuf, 0, w, param);
    // setWindow(x, y, x + w - 1, y);
    // writeBytes(dmabuf, w * bytes, true);
    // return;
    pixelcopy_t pc_read(nullptr, _write_depth, _read_depth);
    pixelcopy_t pc_write(nullptr, _write_depth, _write_depth);
    auto dmabuf = (uint8_t*)alloca((w+1) * bytes);
    pc_write.src_data = dmabuf;
    for (;;)
    {
      uint32_t xstart = 0, drawed_x = 0;
      do
      {
        uint_fast8_t a = buffer[xstart].a;
        if (!a)
        {
          if (drawed_x < xstart)
          {
            param->src_x = drawed_x;
            param->fp_copy(dmabuf, drawed_x, xstart, param);

            pc_write.src_x = drawed_x;
            writeImage(x + drawed_x, y, xstart - drawed_x, 1, &pc_write, true);
          }
          drawed_x = xstart + 1;
        }
        else
        {
          while (255 == buffer[xstart].a && ++xstart != w);
          if (xstart == w) break;
          uint32_t j = xstart;
          while (++j != w && buffer[j].a && buffer[j].a != 255);
          readRect(x + xstart, y, j - xstart + 1, 1, &dmabuf[xstart * bytes], &pc_read);
          if (w == (xstart = j)) break;
        }
      } while (++xstart != w);
      if (drawed_x < xstart)
      {
        param->src_x = drawed_x;
        param->fp_copy(dmabuf, drawed_x, xstart, param);

        pc_write.src_x = drawed_x;
        writeImage(x + drawed_x, y, xstart - drawed_x, 1, &pc_write, true);
      }
      if (!--h) return;
      param->src_x = src_x;
      param->src_y++;
      ++y;
    }
  }

  void Panel_FlexibleFrameBuffer::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    size_t bytes = _write_bits >> 3;
    size_t len = w * bytes;
    auto pixelbuf = (uint8_t*)alloca((len + 7) & ~3);

    size_t pos = 0;

    h += y;
    do
    {
      uint8_t* buf = pixelbuf;
      uint32_t i = 0;
      do
      {
        uint32_t raw = readPixelPreclipped(x + i, y);
        *buf++ = raw;
        for (size_t by = 1; by < bytes; ++by)
        {
          *buf++ = raw >>= 8;
        }
      } while (++i != w);

      param->src_y32 = 0;
      param->src_x32 = 0;
      param->src_data = pixelbuf;

      pos = param->fp_copy(dst, pos, pos + w, param);
    } while (++y != h);
  }

//----------------------------------------------------------------------------
 }
}
