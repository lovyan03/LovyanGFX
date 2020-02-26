#ifndef LGFX_SPRITE_HPP_
#define LGFX_SPRITE_HPP_

#include <algorithm>

#include "LovyanGFX.hpp"

namespace lgfx
{
  class LGFXSpriteBase : public LovyanGFX
  {
  public:

    virtual void setPsram( bool enabled ) {};

    LGFXSpriteBase(LovyanGFX* parent)
    : LovyanGFX()
    , _parent(parent)
    , _img  (nullptr)
    , _palette(nullptr)
    , _bitwidth(0)
    , _xptr (0)
    , _yptr (0)
    , _xs   (0)
    , _xe   (0)
    , _ys   (0)
    , _ye   (0)
    , _index(0)
    {
      _read_depth = _write_depth;
      _has_transaction = false;
      _transaction_count = 0xFFFF;
    }

    void* buffer(void) override { return _img; }
    uint32_t bufferLength(void) const { return _bitwidth * _height * _write_depth.bits >> 3; }

    LGFXSpriteBase()
    : LGFXSpriteBase(nullptr)
    {}

    virtual ~LGFXSpriteBase() {
      deleteSprite();
    }

    void* createSprite(int32_t w, int32_t h)
    {
      if (w < 1 || h < 1) return nullptr;
      if (_img) deleteSprite();
      _bitwidth = (w + _write_depth.x_mask) & (~(uint32_t)_write_depth.x_mask);
      _img = (uint8_t*)_mem_alloc((h * _bitwidth * _write_depth.bits >> 3) + 1);
      if (!_img) return nullptr;

      _sw = _width = w;
      _xe = w - 1;
      _xpivot = w >> 1;
      _sh = _height = h;
      _ye = h - 1;
      _ypivot = h >> 1;
      _rotation = 0;
      _index = _sx = _sy = _xs = _ys = _xptr = _yptr = 0;

      clear();
      return _img;
    }

    bool createPalette(void)
    {
      if (_write_depth.depth > 8) return false;

      if (_palette != nullptr) { _mem_free(_palette); _palette = nullptr; }
      _has_palette = false;

      size_t palettes = 1 << _write_depth.bits;
      _palette = (swap888_t*)_mem_alloc(sizeof(swap888_t) * palettes);
      if (!_palette) {
        return false;
      }
      _has_palette = true;
      _write_depth.setColorDepth(_write_depth.depth, _palette != nullptr);
/*
if (_write_depth.depth == palette_8bit) {
for (uint32_t i = 0; i < palettes; i++) {
  _palette[i] = *(rgb332_t*)&i;
}
} else
//*/
      { // create grayscale palette
        uint32_t k;
        switch (_write_depth.depth) {
        case 8: k = 0x010101; break;
        case 4: k = 0x111111; break;
        case 2: k = 0x555555; break;
        case 1: k = 0xFFFFFF; break;
        default: k = 1; break;
        }
        for (uint32_t i = 0; i < palettes; i++) {
          _palette[i] = i * k;
        }
      }
      return true;
    }

    void deleteSprite(void)
    {
      if (_img     != nullptr) { _mem_free(_img    ); _img     = nullptr; }
      if (_palette != nullptr) { _mem_free(_palette); _palette = nullptr; }
      _has_palette = false;
    }

    void setBitmapColor(uint16_t fgcolor, uint16_t bgcolor)  // For 1bpp sprites
    {
      if (_palette) {
        _palette[0] = *(rgb565_t*)&bgcolor;
        _palette[1] = *(rgb565_t*)&fgcolor;
      }
    }

    void setPalette(size_t index, const swap888_t& rgb)
    {
      if (_palette) { _palette[index & ((1<<_write_depth.bits)-1)] = rgb; }
    }

    void setPalette(size_t index, uint8_t r, uint8_t g, uint8_t b)
    {
      if (_palette) { _palette[index & ((1<<_write_depth.bits)-1)].set(r, g, b); }
    }

    void* getPalette_impl(void) const override
    {
      return _palette;
    }

    template<typename T>
    __attribute__ ((always_inline)) inline void fillSprite (const T& color) { fillRect(0, 0, _width, _height, color); }

//*

    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(                int32_t x, int32_t y, const T& transp) { push_sprite(_parent, x, y, _write_depth.convert(transp) & _write_depth.colormask); }
    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, int32_t x, int32_t y, const T& transp) { push_sprite(    dst, x, y, _write_depth.convert(transp) & _write_depth.colormask); }
    __attribute__ ((always_inline)) inline void pushSprite(                int32_t x, int32_t y) { push_sprite(_parent, x, y); }
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, int32_t x, int32_t y) { push_sprite(    dst, x, y); }

    template<typename T> bool pushRotated(                float angle, const T& transp) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f, _write_depth.convert(transp) & _write_depth.colormask); }
    template<typename T> bool pushRotated(LovyanGFX* dst, float angle, const T& transp) { return push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f, _write_depth.convert(transp) & _write_depth.colormask); }
                         bool pushRotated(                float angle) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f); }
                         bool pushRotated(LovyanGFX* dst, float angle) { return push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f); }

    template<typename T> bool pushRotateZoom(                                              float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y, _write_depth.convert(transp) & _write_depth.colormask); }
    template<typename T> bool pushRotateZoom(LovyanGFX* dst                              , float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(    dst, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y, _write_depth.convert(transp) & _write_depth.colormask); }
    template<typename T> bool pushRotateZoom(                int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_depth.convert(transp) & _write_depth.colormask); }
    template<typename T> bool pushRotateZoom(LovyanGFX* dst, int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_depth.convert(transp) & _write_depth.colormask); }
                         bool pushRotateZoom(                                              float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(LovyanGFX* dst                              , float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(    dst, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(                int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(LovyanGFX* dst, int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y); }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    LovyanGFX* _parent;
    union {
      uint8_t*   _img;
      uint16_t*  _img16;
      swap888_t* _img24;
    };
    swap888_t* _palette;
    int32_t _bitwidth;
    int32_t _xptr;
    int32_t _yptr;
    int32_t _xs;
    int32_t _xe;
    int32_t _ys;
    int32_t _ye;
    uint32_t _index;
    bool _disable_memcpy = false; // disable PSRAM to PSRAM memcpy flg.

    virtual void* _mem_alloc(uint32_t bytes) = 0;
    virtual void _mem_free(void* buf) = 0;

    void push_sprite(LovyanGFX* dst, int32_t x, int32_t y, uint32_t transp = ~0)
    {
      pixelcopy_t p(_img, dst->getColorDepth(), getColorDepth(), dst->hasPalette(), _palette, transp);
      dst->push_image(x, y, _width, _height, &p);
    }

    bool push_rotate_zoom(LovyanGFX* dst, int32_t x, int32_t y, float angle, float zoom_x, float zoom_y, uint32_t transp = ~0)
    {
      if (_img == nullptr) return false;
      pixelcopy_t p(_img, dst->getColorDepth(), getColorDepth(), dst->hasPalette(), _palette, transp);
      dst->pushImageRotateZoom(x, y, _width, _height, _xpivot, _ypivot, angle, zoom_x, zoom_y, &p);
      return true;
    }

    void set_window(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      if (xs > xe) swap_coord(xs, xe);
      if (ys > ye) swap_coord(ys, ye);

      if ((xe < 0) || (ye < 0) || (xs >= _width) || (ys >= _height))
      {
        _xptr = _xs = _xe = 0;
        _yptr = _ys = _ye = _height;
      } else {
        _xptr = _xs = (xs < 0) ? 0 : xs;
        _yptr = _ys = (ys < 0) ? 0 : ys;
        _xe = (xe >= _width ) ? _width  - 1 : xe;
        _ye = (ye >= _height) ? _height - 1 : ye;
      }
      _index = xs + ys * _bitwidth;
    }

    void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
    }
    void readWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
    }

    void* setColorDepth_impl(color_depth_t depth) override
    {
      _write_depth.setColorDepth(depth, _palette != nullptr) ;
      _read_depth = _write_depth;

      if (_img == nullptr) return nullptr;
      deleteSprite();
      return createSprite(_width, _height);
    }

    void setRotation_impl(uint8_t r) override
    {
      r = r & 7;
      if ((_rotation&1) != (r&1)) {
        uint32_t tmp = _width;
        _width = _height;
        _height = tmp;
      }
      _rotation = r;
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      if (_write_depth.bytes == 2) {
        _img16[x + y * _bitwidth] = _color.rawL;
      } else if (_write_depth.bytes == 1) {
        _img[x + y * _bitwidth] = _color.raw0;
      } else if (_write_depth.bytes == 3) {
        _img24[x + y * _bitwidth] = *(swap888_t*)&_color;
      } else {
        uint8_t mask = (uint8_t)(~(0xFF >> _write_depth.bits)) >> (x * _write_depth.bits & 7);
        auto dst = &_img[(x + y * _bitwidth) * _write_depth.bits >> 3];
        *dst = (*dst & ~mask) | (_color.raw0 & mask);
      }
    }

    void fillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
/*
setWindow(x,y,x+w-1,y+h-1);
writeColor_impl(w*h);
return;
//*/
      if (_write_depth.bytes == 0) {
        size_t d = _bitwidth * _write_depth.bits >> 3;
        uint8_t* dst = &_img[(x + y * _bitwidth) * _write_depth.bits >> 3];
        size_t len = ((x + w) * _write_depth.bits >> 3) - (x * _write_depth.bits >> 3);
        uint8_t mask = 0xFF >> (x * _write_depth.bits & 7);
        uint8_t c = _color.raw0;
        if (!len) {
          mask ^= mask >> (w * _write_depth.bits);
          c &= mask;
        } else {
          if (mask != 0xFF) {
            auto dst2 = dst;
            auto h2 = h;
            c &= mask;
            do { *dst2 = (*dst2 & ~mask) | c; dst2 += d; } while (--h2);
            dst++; len--;
          }
          mask = ~(0xFF>>((x+w) * _write_depth.bits & 7));
          if (mask == 0xFF) len--;
          c = _color.raw0;
          if (len) {
            auto dst2 = dst;
            auto h2 = h;
            do { memset(dst2, c, len); dst2 += d; } while (--h2);
            dst += len;
          }
          if (mask == 0xFF) return;
          c &= mask;
        }
        do { *dst = (*dst & ~mask) | c; dst += d; } while (--h);
      } else
      if (w == 1) {
        uint8_t* dst = &_img[(x + y * _bitwidth) * _write_depth.bytes];
        if (_write_depth.bytes == 2) {
          do { *(uint16_t*)dst = *(uint16_t*)&_color;   dst += _bitwidth << 1; } while (--h);
        } else if (_write_depth.bytes == 1) {
          do {             *dst = _color.raw0;          dst += _bitwidth;      } while (--h);
        } else if (_write_depth.bytes == 3) {
          do { *(swap888_t*)dst = *(swap888_t*)&_color; dst += _bitwidth * 3;  } while (--h);
        } else {
        }
      } else {
        uint8_t* dst = &_img[(x + y * _bitwidth) * _write_depth.bytes];
        if (_write_depth.bytes == 1 || (_color.raw0 == _color.raw1 && (_write_depth.bytes == 2 || (_color.raw0 == _color.raw2)))) {
          if (x == 0 && w == _width) {
            memset(dst, _color.raw0, _width * h * _write_depth.bytes);
          } else {
            do {
              memset(dst, _color.raw0, w * _write_depth.bytes);
              dst += _width * _write_depth.bytes;
            } while (--h);
          }
        } else {
          size_t len = w * _write_depth.bytes;
          uint32_t down = _width * _write_depth.bytes;
          if (_disable_memcpy) {
            uint8_t linebuf[len];
            memset_multi(linebuf, _color.raw, _write_depth.bytes, w);
            do {
              memcpy(dst, linebuf, len);
              dst += down;
            } while (--h);
          } else {
            memset_multi(dst, _color.raw, _write_depth.bytes, w);
            while (--h) {
              memcpy(dst + down, dst, len);
              dst += down;
            }
          }
        }
      }
    }

    void writeColor_impl(int32_t length) override
    {
      if (0 >= length) return;
      if (_write_depth.bytes == 0) {
        uint8_t c = _color.raw0;
        //uint8_t addrshift = (4 - _write_depth.depth);
        uint8_t addrshift = 2 / _write_depth.bits + 1;
        int32_t linelength;
        do {
          uint8_t* dst = &_img[(_xptr + _yptr * _bitwidth) >> addrshift];
          linelength = std::min(_xe - _xptr + 1, length);
          size_t len = ((_xptr + linelength) >> addrshift) - (_xptr >> addrshift);
          uint8_t mask = 0xFF >> ((_xptr & _write_depth.x_mask) * _write_depth.bits);
          if (!len) {
            mask &= ~(mask >> (linelength * _write_depth.bits));
          } else {
            *dst = (*dst & ~mask) | (c & mask);
            if (len != 1) {
              memset(dst+1, c, len-1);
            }
            dst += len;
            mask = ~(0xFF>>(((_xptr + linelength) & _write_depth.x_mask) * _write_depth.bits));
          }
          *dst = (*dst & ~mask) | (c & mask);
          ptr_advance(linelength);
        } while (length -= linelength);

      } else
      if (_write_depth.bytes == 1 || (_color.raw0 == _color.raw1 && (_write_depth.bytes == 2 || (_color.raw0 == _color.raw2)))) {
        int32_t linelength;
        do {
          linelength = std::min(_xe - _xptr + 1, length);
          memset(&_img[_index * _write_depth.bytes], _color.raw0, linelength * _write_depth.bytes);
          ptr_advance(linelength);
        } while (length -= linelength);
      } else {
        uint8_t* dst = &_img[_index * _write_depth.bytes];
        if (_disable_memcpy) {
          int32_t buflen = std::min(_xe - _xs + 1, length);
          uint8_t linebuf[buflen * _write_depth.bytes];
          memset_multi(linebuf, _color.raw, _write_depth.bytes, buflen);
          uint32_t len;
          do  {
            len = std::min(length, _xe - _xptr + 1);
            memcpy(&_img[_index * _write_depth.bytes], linebuf, len * _write_depth.bytes);
            ptr_advance(len);
          } while (length -= len);
          return;
        }

        uint32_t advance = std::min(_xe - _xptr + 1, length);
        if (_xs != _xptr) {
          memset_multi(dst, _color.raw, _write_depth.bytes, advance);
          ptr_advance(advance);
          if (0 == (length -= advance)) return;
          dst = &_img[_index * _write_depth.bytes];
          advance = std::min(_xe - _xptr + 1, length);
        }
        memset_multi(dst, _color.raw, _write_depth.bytes, advance);
        ptr_advance(advance);
        if (0 == (length -= advance)) return;
        uint32_t down = _width * _write_depth.bytes;
        while (length > advance) {
          memcpy(dst+down, dst, advance * _write_depth.bytes);
          dst += down;
          ptr_advance(advance);
          length -= advance;
        }
        if (length) {
          memcpy(dst + down, dst, length * _write_depth.bytes);
          ptr_advance(length);
        }
      }
    }

    void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) override
    {
      if (_write_depth.bits < 8) {
        pixelcopy_t param(_img, _write_depth.depth, _write_depth.depth);
        param.src_width = _bitwidth;
        int32_t add_y = (src_y < dst_y) ? -1 : 1;
        if (src_y != dst_y) {
          if (src_y < dst_y) {
            src_y += h - 1;
            dst_y += h - 1;
          }
          param.src_y = src_y;
          do {
            param.src_x = src_x;
            auto idx = dst_x + dst_y * _bitwidth;
            param.fp_copy(_img, idx, idx + w, &param);
            dst_y += add_y;
            param.src_y += add_y;
          } while (--h);
        } else {
          size_t len = (_bitwidth * _write_depth.bits) >> 3;
          uint8_t buf[len];
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
      } else {
        size_t len = w * _write_depth.bytes;
        int32_t add = _bitwidth * _write_depth.bytes;
        if (src_y < dst_y) add = -add;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        uint8_t* src = &_img[(src_x + (src_y + pos) * _bitwidth) * _write_depth.bytes];
        uint8_t* dst = &_img[(dst_x + (dst_y + pos) * _bitwidth) * _write_depth.bytes];
        if (_disable_memcpy) {
          uint8_t buf[len];
          do {
            memcpy(buf, src, len);
            memcpy(dst, buf, len);
            dst += add;
            src += add;
          } while (--h);
        } else {
          do {
            memmove(dst, src, len);
            src += add;
            dst += add;
          } while (--h);
        }
      }
    }

    rgb565_t readPixel16_impl(int32_t x, int32_t y) override
    {
      _xptr = _xs = _xe = x;
      _yptr = _ys = _ye = y;
      _index = x + y * _bitwidth;

      const uint8_t *src = ptr_img();
      rgb565_t res;
      if (     _write_depth.depth == rgb332_1Byte) { res = *(( rgb332_t*)src); return res;}
      else if (_write_depth.depth == rgb565_2Byte) { res = *((swap565_t*)src); return res;}
//    else if (_write_depth.depth == rgb666_3Byte) { res = *((swap888_t*)src); return res;}
      else if (_write_depth.depth == rgb888_3Byte) { res = *((swap888_t*)src); return res;}

      return (bool)(*src & (0x80 >> (_index & 0x07)));
    }

    uint32_t readPixelRAW_impl(int32_t x, int32_t y) override
    {
      return _write_depth.colormask & (*(const uint32_t*)&_img[(x + y * _bitwidth) * _write_depth.bits >> 3]);
    }

    static void memset_multi(uint8_t* buf, uint32_t c, size_t size, size_t length) {
      size_t l = length;
      if (l & ~0xF) {
        while ((l >>= 1) & ~0xF);
        ++l;
      }
      length = (length - l) * size;
      size_t len = l * size;
      uint8_t* dst = buf;
      if (size == 2) {
        do { // 2byte speed tweak
          *(uint16_t*)dst = c; dst += 2;
        } while (--l);
      } else {
        size_t i = 0;
        l *= size;
        do {
          *dst++ = *(((uint8_t*)&c)+i);
          if (++i == size) i = 0;
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

    void read_rect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) override
    {
      set_window(x, y, x + w - 1, y + h - 1);
      if (param->no_convert) {
        readBytes_impl((uint8_t*)dst, w * h * _read_depth.bytes);
      } else {
        read_pixels(dst, w * h, param);
      }
    }

    void read_pixels(void* dst, int32_t length, pixelcopy_t* param)
    {
      int32_t linelength;
      int32_t dstindex;
      param->src_data = _img;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        param->src_x = _xptr;
        param->src_y = _yptr;
        dstindex = param->fp_copy(dst, dstindex, dstindex + linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void readBytes_impl(uint8_t* dst, int32_t length) override
    {
      uint8_t b = _write_depth.bytes ? _write_depth.bytes : 1;
      length /= b;
      while (length) {
        int32_t linelength = std::min(_xe - _xptr + 1, length);
        memcpy(dst, ptr_img(), linelength * b);
        dst += linelength * b;
        ptr_advance(linelength);
        length -= linelength;
      }
    }

    void push_image_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param) override
    {
      auto sx = param->src_x;
      do {
        int32_t pos = x + (y++) * _bitwidth;
        int32_t end = pos + w;
        while (end != (pos = param->fp_copy(_img, pos, end, param))) {
          if ( end == (pos = param->fp_skip(      pos, end, param))) break;
        }
        param->src_x = sx;
        param->src_y++;
      } while (--h);
    }

    void push_colors_impl(int32_t length, pixelcopy_t* param) override
    {
      auto k = _bitwidth * _write_depth.bits >> 3;
      int32_t linelength;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        param->fp_copy(&_img[_yptr * k], _xptr, _xptr + linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void writeBytes_impl(const uint8_t* data, int32_t length) override
    {
      uint8_t b = _write_depth.bytes ? _write_depth.bytes : 1;
      length /= b;
      while (length) {
        int32_t linelength = std::min(_xe - _xptr + 1, length);
        memcpy(ptr_img(), data, linelength * b);
        data += linelength * b;
        ptr_advance(linelength);
        length -= linelength;
      }
    }

    inline bool ptr_advance(int32_t length = 1) {
      if ((_xptr += length) > _xe) {
        _xptr = _xs;
        if (++_yptr > _ye) {
          _yptr = _ys;
        }
        _index = _xptr + _yptr * _bitwidth;
        return true;
      }
      _index += length;
      return false;
    }

    inline uint8_t* ptr_img() {
      return &_img[_index * _write_depth.bits >> 3];
    }

  };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)

  class LGFXSprite : public LGFXSpriteBase
  {
  public:
    LGFXSprite(LovyanGFX* parent)
    : LGFXSpriteBase(parent)
    , _malloc_cap(MALLOC_CAP_8BIT)
    {}
    LGFXSprite() : LGFXSprite(nullptr) {}

    void setPsram( bool enabled ) override {
      if (enabled) _malloc_cap |= MALLOC_CAP_SPIRAM;
      else         _malloc_cap &= ~MALLOC_CAP_SPIRAM;
    }

    // set MALLOC_CAP_SPIRAM or MALLOC_CAP_DMA
    void setMallocCap(uint32_t flg) {
      _malloc_cap = flg;
    }

  protected:
    uint32_t _malloc_cap;
    void* _mem_alloc(uint32_t bytes) override
    {
      _disable_memcpy = (_malloc_cap & MALLOC_CAP_SPIRAM);
      void* res = heap_caps_malloc(bytes, _malloc_cap);

      // if can't malloc with PSRAM, try without using PSRAM malloc.
      if (res == nullptr && _disable_memcpy) {
        _disable_memcpy = false;
        res = heap_caps_malloc(bytes, _malloc_cap & ~MALLOC_CAP_SPIRAM);
      }
      return res;
    }
    void _mem_free(void* buf) override
    {
      heap_caps_free(buf);
    }
  };

#elif defined (ESP8226)

#elif defined (STM32F7)

#elif defined (__AVR__)

#endif

}

#endif
