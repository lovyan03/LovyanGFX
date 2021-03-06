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

#include "LGFX_Sprite.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static void memset_multi(std::uint8_t* buf, std::uint32_t c, std::size_t size, std::size_t length)
  {
    std::size_t l = length;
    if (l & ~0xF)
    {
      while ((l >>= 1) & ~0xF);
      ++l;
    }
    std::size_t len = l * size;
    length = (length * size) - len;
    std::uint8_t* dst = buf;
    if (size == 2) {
      do { // 2byte speed tweak
        *(std::uint16_t*)dst = c;
        dst += 2;
      } while (--l);
    } else {
      do {
        std::size_t i = 0;
        do {
          *dst++ = *(((std::uint8_t*)&c) + i);
        } while (++i != size);
      } while (--l);
    }
    if (!length) return;
    while (length > len) {
      memcpy(dst, buf, len);
      dst += len;
      length -= len;
      len <<= 1;
    }
    if (length) {
      memcpy(dst, buf, length);
    }
  }

  void Panel_Sprite::setBuffer(void* buffer, std::int32_t w, std::int32_t h, color_conv_t* conv)
  {
    deleteSprite();

    _img.reset(buffer);
    _bitwidth = (w + conv->x_mask) & (~(std::uint32_t)conv->x_mask);
    _width = w;
    _xe = w - 1;
    _height = h;
    _ye = h - 1;
  }

  void Panel_Sprite::deleteSprite(void)
  {
    _width = _height = _bitwidth = _xs = _xe = _ys = _ye = _xptr = _yptr = _index = 0;
    _img.release();
  }

  void* Panel_Sprite::createSprite(std::int32_t w, std::int32_t h, color_conv_t* conv, bool psram)
  {
    if (w < 1 || h < 1) return nullptr;
    _width = w;
    _height = h;
    _bitwidth = (w + conv->x_mask) & (~(std::uint32_t)conv->x_mask);
    std::size_t len = h * (_bitwidth * _write_bits >> 3) + 1;

    _img.reset(len, psram ? AllocationSource::Psram : AllocationSource::Dma);

    if (!_img)
    {
      deleteSprite();
      return nullptr;
    }
    memset(_img, 0, len);

    _index = _xs = _ys = _xe = _ye = _xptr = _yptr = 0;

    return _img;
  }

  color_depth_t Panel_Sprite::setColorDepth(color_depth_t depth)
  {
    _write_depth = depth;
    _read_depth = depth;
    _write_bits = depth & color_depth_t::bit_mask;
    _read_bits = _write_bits;
    return depth;
  }
  void Panel_Sprite::setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye)
  {
    _index = xs + ys * _bitwidth;
    _xptr = xs;
    _xs = xs;
    _xe = xe;
    _yptr = ys;
    _ys = ys;
    _ye = ye;
  }

  void Panel_Sprite::drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor)
  {
    auto bits = _write_bits;
    if (bits >= 8)
    {
      std::uint32_t index = x + y * _bitwidth;
      if (bits == 8)
      {
        _img.img8()[index] = rawcolor;
      }
      else if (bits == 16)
      {
        _img.img16()[index] = rawcolor;
      }
      else
      {
        _img.img24()[index] = rawcolor;
      }
    }
    else
    {
      std::uint32_t index = (x + y * _bitwidth) * bits;
      std::uint8_t* dst = &_img.img8()[index >> 3];
      std::uint8_t mask = (std::uint8_t)(~(0xFF >> bits)) >> (index & 7);
      *dst = (*dst & ~mask) | (rawcolor & mask);
    }
  }

  void Panel_Sprite::writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor)
  {
    std::uint_fast8_t bits = _write_bits;
    if (bits >= 8)
    {
      if (w == 1)
      {
        std::uint_fast16_t bw = _bitwidth;
        std::uint32_t index = x + y * bw;
        if (bits == 8)
        {
          auto img = &_img[index];
          do { *img = rawcolor;  img += bw; } while (--h);
        }
        else if (bits == 16)
        {
          auto img = &_img.img16()[index];
          do { *img = rawcolor;  img += bw; } while (--h);
        }
        else
        {  // if (_write_conv.bytes == 3)
          //auto c = _color;
          auto img = &_img.img24()[index];
          do { *img = rawcolor; img += bw; } while (--h);
          //do { img->r = c.raw0; img->g = c.raw1; img->b = c.raw2; img += bw; } while (--h);
        }
      }
      else
      {
        std::uint_fast8_t bytes = bits >> 3;
        std::uint32_t bw = _bitwidth;
        std::uint8_t* dst = &_img[(x + y * bw) * bytes];
        std::uint8_t c = rawcolor;
        if (bytes == 1 || (c == ((rawcolor >> 8) & 0xFF) && (bytes == 2 || (c == ((rawcolor >> 16) & 0xFF)))))
        {
          if (w == bw)
          {
            memset(dst, rawcolor, w * bytes * h);
          }
          else
          {
            std::uint32_t add_dst = bw * bytes;
            do
            {
              memset(dst, c, w * bytes);
              dst += add_dst;
            } while (--h);
          }
        }
        else
        {
          std::size_t len = w * bytes;
          std::uint32_t add_dst = bw * bytes;
          if (_img.use_memcpy())
          {
            if (w == bw)
            {
              memset_multi(dst, rawcolor, bytes, w * h);
            }
            else
            {
              memset_multi(dst, rawcolor, bytes, w);
              while (--h)
              {
                memcpy(dst + add_dst, dst, len);
                dst += add_dst;
              }
            }
          }
          else
          {
            std::uint8_t linebuf[len];
            memset_multi(linebuf, rawcolor, bytes, w);
            do
            {
              memcpy(dst, linebuf, len);
              dst += add_dst;
            } while (--h);
          }
        }
      }
    }
    else
    {
      x *= bits;
      w *= bits;
      std::size_t add_dst = _bitwidth * bits >> 3;
      std::uint8_t* dst = &_img[y * add_dst + (x >> 3)];
      std::uint32_t len = ((x + w) >> 3) - (x >> 3);
      std::uint8_t mask = 0xFF >> (x & 7);
      if (len)
      {
        if (mask != 0xFF)
        {
          --len;
          auto d = dst++;
          std::uint8_t mc = rawcolor & mask;
          auto i = h;
          do { *d = (*d & ~mask) | mc; d += add_dst; } while (--i);
        }
        mask = ~(0xFF>>((x + w) & 7));
        if (len)
        {
          auto d = dst;
          auto i = h;
          do { memset(d, rawcolor, len); d += add_dst; } while (--i);
          dst += len;
        }
        if (mask == 0) return;
      }
      else
      {
        mask ^= mask >> w;
      }
      rawcolor &= mask;
      do { *dst = (*dst & ~mask) | rawcolor; dst += add_dst; } while (--h);
    }
  }

  void Panel_Sprite::writeBlock(std::uint32_t rawcolor, std::uint32_t length)
  {
    if (0 >= length) return;
    auto bytes = _write_bits >> 3;
    if (bytes == 0) {
      std::int32_t bits = _write_bits;
      std::uint8_t c = rawcolor;
      std::int32_t ll;
      std::int32_t index = _index;
      do
      {
        std::uint8_t* dst = &_img.img8()[index * bits >> 3];
        ll = std::min<std::uint32_t>(_xe - _xptr + 1, length);
        std::int32_t w = ll * bits;
        std::int32_t x = _xptr * bits;
        std::size_t len = ((x + w) >> 3) - (x >> 3);
        std::uint8_t mask = 0xFF >> (x & 7);
        if (!len)
        {
          mask ^= mask >> w;
          *dst = (*dst & ~mask) | (c & mask);
        } else {
          if (mask != 0xFF) {
            *dst = (*dst & ~mask) | (c & mask);
            ++dst;
            --len;
          }
          if (len) {
            memset(dst, c, len);
            dst += len;
          }
          mask = 0xFF >> ((x + w) & 7);
          if (mask != 0xFF) *dst = (*dst & mask) | (c & ~mask);
        }
        index = ptr_advance(ll);
      } while (length -= ll);
    } else {
      std::uint8_t c = rawcolor;
      if (bytes == 1 || (c == ((rawcolor >> 8) & 0xFF) && (bytes == 2 || (c == ((rawcolor >> 16) & 0xFF)))))
      {
        std::int32_t ll;
        std::int32_t index = _index;
        do {
          ll = std::min<std::uint32_t>(_xe - _xptr + 1, length);
          memset(&_img[index * bytes], rawcolor, ll * bytes);
          index = ptr_advance(ll);
        } while (length -= ll);
      }
      else if (_img.use_memcpy())
      {
        std::uint32_t ll;
        std::uint32_t index = _index;
        do {
          ll = std::min<std::uint32_t>(_xe - _xptr + 1, length);
          memset_multi(&_img[index * bytes], rawcolor, bytes, ll);
          index = ptr_advance(ll);
        } while (length -= ll);
      } else {
        std::uint32_t buflen = std::min<std::uint32_t>(_xe - _xs + 1, length);
        std::uint8_t linebuf[buflen * bytes];
        memset_multi(linebuf, rawcolor, bytes, buflen);
        std::int32_t ll;
        std::int32_t index = _index;
        do  {
          ll = std::min<std::uint32_t>(_xe - _xptr + 1, length);
          memcpy(&_img[index * bytes], linebuf, ll * bytes);
          index = ptr_advance(ll);
        } while (length -= ll);
      }
    }
  }

  void Panel_Sprite::writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool)
  {
    auto sx = param->src_x;
    if (param->transp == ~0u && param->no_convert && _img.use_memcpy())
    {
      auto bits = param->src_bits;
      std::uint_fast8_t mask = (bits == 1) ? 7
                             : (bits == 2) ? 3
                                           : 1;
      if (0 == (bits & 7) || ((sx & mask) == (x & mask) && (w == this->_width || 0 == (w & mask))))
      {
        auto bw = _bitwidth * bits >> 3;
        auto dst = &_img[bw * y];
        auto sw = param->src_bitwidth * bits >> 3;
        auto src = &((std::uint8_t*)param->src_data)[param->src_y * sw];
        if (sw == bw && this->_width == w && sx == 0 && x == 0)
        {
          memcpy(dst, src, bw * h);
          return;
        }
        y = 0;
        dst +=  x * bits >> 3;
        src += sx * bits >> 3;
        w    =  w * bits >> 3;
        do
        {
          memcpy(&dst[y * bw], &src[y * sw], w);
        } while (++y != h);
        return;
      }
    }

    y *= _bitwidth;
    do
    {
      std::int32_t pos = x + y;
      std::int32_t end = pos + w;
      y += _bitwidth;
      while (end != (pos = param->fp_copy(_img, pos, end, param)))
      {
        if ( end == (pos = param->fp_skip(      pos, end, param))) break;
      }
      param->src_x = sx;
      param->src_y++;
    } while (--h);
  }

  void Panel_Sprite::writeImageARGB(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param)
  {
    std::uint32_t pos = x + y * _bitwidth;
    std::uint32_t end = pos + w;
    param->fp_copy(_img, pos, end, param);
    while (--h)
    {
      pos += _bitwidth;
      end = pos + w;
      param->src_y++;
      param->fp_copy(_img, pos, end, param);
    }
  }


  void Panel_Sprite::readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    h += y;
    if (param->no_convert && _write_bits >= 8)
    {
      auto bytes = _write_bits >> 3;
      auto bw = _bitwidth;
      auto d = (std::uint8_t*)dst;
      do {
        memcpy(d, &_img[(x + y * bw) * bytes], w * bytes);
        d += w * bytes;
      } while (++y != h);
    } else {
      param->src_bitwidth = _bitwidth;
      param->src_data = _img;
      std::int32_t dstindex = 0;
      do {
        param->src_x = x;
        param->src_y = y;
        dstindex = param->fp_copy(dst, dstindex, dstindex + w, param);
      } while (++y != h);
    }
  }


  void Panel_Sprite::copyRect(std::uint_fast16_t dst_x, std::uint_fast16_t dst_y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint_fast16_t src_x, std::uint_fast16_t src_y)
  {
    if (_write_bits < 8) {
      pixelcopy_t param(_img, _write_depth, _write_depth);
      param.src_bitwidth = _bitwidth;
      std::int32_t add_y = (src_y < dst_y) ? -1 : 1;
      if (src_y != dst_y) {
        if (src_y < dst_y) {
          src_y += h - 1;
          dst_y += h - 1;
        }
        param.src_y = src_y;
        do
        {
          param.src_x = src_x;
          auto idx = dst_x + dst_y * _bitwidth;
          param.fp_copy(_img, idx, idx + w, &param);
          dst_y += add_y;
          param.src_y += add_y;
        } while (--h);
      } else {
        std::size_t len = (_bitwidth * _write_bits) >> 3;
        std::uint8_t buf[len];
        param.src_data = buf;
        param.src_y32 = 0;
        do {
          memcpy(buf, &_img[src_y * len], len);
          param.src_x = src_x;
          auto idx = dst_x + dst_y * _bitwidth;
          param.fp_copy(_img, idx, idx + w, &param);
          dst_y += add_y;
          src_y += add_y;
        } while (--h);
      }
    }
    else
    {
      std::size_t bytes = _write_bits >> 3;
      std::size_t len = w * bytes;
      std::int32_t add = _bitwidth * bytes;
      if (src_y < dst_y) add = -add;
      std::int32_t pos = (src_y < dst_y) ? h - 1 : 0;
      std::uint8_t* src = &_img.img8()[(src_x + (src_y + pos) * _bitwidth) * bytes];
      std::uint8_t* dst = &_img.img8()[(dst_x + (dst_y + pos) * _bitwidth) * bytes];
      if (_img.use_memcpy())
      {
        do
        {
          memmove(dst, src, len);
          src += add;
          dst += add;
        } while (--h);
      }
      else
      {
        std::uint8_t buf[len];
        do
        {
          memcpy(buf, src, len);
          memcpy(dst, buf, len);
          src += add;
          dst += add;
        } while (--h);
      }
    }
  }

//----------------------------------------------------------------------------
 }
}

