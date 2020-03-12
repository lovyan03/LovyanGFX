#ifndef LGFX_SPRITE_HPP_
#define LGFX_SPRITE_HPP_

#include <algorithm>

#include "lgfx_base.hpp"

namespace lgfx
{
  class LGFXSprite : public LovyanGFX
  {
  public:

    LGFXSprite(LovyanGFX* parent)
    : LovyanGFX()
    , _parent(parent)
    , _img  (nullptr)
    , _bitwidth(0)
    , _xptr (0)
    , _yptr (0)
    , _xs   (0)
    , _xe   (0)
    , _ys   (0)
    , _ye   (0)
    , _index(0)
    {
      _read_conv = _write_conv;
      _has_transaction = false;
      _transaction_count = 0xFFFF;
    }

    void* buffer(void) { return _img; }
    uint32_t bufferLength(void) const { return _bitwidth * _height * _write_conv.bits >> 3; }

    LGFXSprite()
    : LGFXSprite(nullptr)
    {}

    virtual ~LGFXSprite() {
      deleteSprite();
    }

    void* createSprite(int32_t w, int32_t h)
    {
      if (w < 1 || h < 1) return nullptr;
      if (_img) deleteSprite();
      _bitwidth = (w + _write_conv.x_mask) & (~(uint32_t)_write_conv.x_mask);
      _img = (uint8_t*)_mem_alloc((h * _bitwidth * _write_conv.bits >> 3) + 1);
      if (!_img) return nullptr;

      _sw = _width = w;
      _clip_r = _xe = w - 1;
      _xpivot = w >> 1;

      _sh = _height = h;
      _clip_b = _ye = h - 1;
      _ypivot = h >> 1;
      _rotation = 0;

      _clip_l = _clip_t = _index = _sx = _sy = _xs = _ys = _xptr = _yptr = 0;

      clear();
      return _img;
    }

    bool createPalette(void)
    {
      if (!create_palette()) return false;

      setPaletteGrayscale();
    }

    bool createPalette(const uint16_t* colors, uint32_t count)
    {
      if (!create_palette()) return false;

      if (count > _palette_count) count = _palette_count;
      for (uint32_t i = 0; i < count; ++i) {
        _palette[i] = convert_rgb565_to_bgr888(pgm_read_word(colors++));
      }
      return true;
    }

    void deleteSprite(void)
    {
      if (_img     != nullptr) { _mem_free(_img    ); _img     = nullptr; }
      if (_palette != nullptr) { _mem_free(_palette); _palette = nullptr; }
      _palette_count = 0;
    }

    void setPaletteGrayscale(void)
    { // create grayscale palette
      if (!_palette) return;
      uint32_t k;
      switch (_write_conv.depth) {
      case 8: k = 0x010101; break;
      case 4: k = 0x111111; break;
      case 2: k = 0x555555; break;
      case 1: k = 0xFFFFFF; break;
      default: k = 1; break;
      }
      for (uint32_t i = 0; i < _palette_count; i++) {
        _palette[i] = i * k;
      }
    }

    void setBitmapColor(uint16_t fgcolor, uint16_t bgcolor)  // For 1bpp sprites
    {
      if (_palette) {
        _palette[0] = *(rgb565_t*)&bgcolor;
        _palette[1] = *(rgb565_t*)&fgcolor;
      }
    }

    void setPaletteColor(size_t index, uint16_t rgb565)
    {
      if (_palette && index < _palette_count) { _palette[index] = *(rgb565_t*)&rgb565; }
    }

    void setPaletteColor(size_t index, const bgr888_t& rgb)
    {
      if (_palette && index < _palette_count) { _palette[index] = rgb; }
    }

    void setPaletteColor(size_t index, uint8_t r, uint8_t g, uint8_t b)
    {
      if (_palette && index < _palette_count) { _palette[index].set(r, g, b); }
    }

    __attribute__ ((always_inline)) inline void* setColorDepth(uint8_t bpp) { return setColorDepth((color_depth_t)bpp); }
    void* setColorDepth(color_depth_t depth)
    {
      _write_conv.setColorDepth(depth, _palette != nullptr) ;
      _read_conv = _write_conv;

      if (_img == nullptr) return nullptr;
      deleteSprite();
      return createSprite(_width, _height);
    }

/*
    void setRotation(uint8_t r)
    {
      r = r & 7;
      if ((_rotation&1) != (r&1)) {
        uint32_t tmp = _width;
        _width = _height;
        _height = tmp;
      }
      _rotation = r;
    }
//*/

    template<typename T>
    __attribute__ ((always_inline)) inline void fillSprite (const T& color) { fillRect(0, 0, _width, _height, color); }

//*

    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(                int32_t x, int32_t y, const T& transp) { push_sprite(_parent, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, int32_t x, int32_t y, const T& transp) { push_sprite(    dst, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    __attribute__ ((always_inline)) inline void pushSprite(                int32_t x, int32_t y) { push_sprite(_parent, x, y); }
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, int32_t x, int32_t y) { push_sprite(    dst, x, y); }

    template<typename T> bool pushRotated(                float angle, const T& transp) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotated(LovyanGFX* dst, float angle, const T& transp) { return push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
                         bool pushRotated(                float angle) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f); }
                         bool pushRotated(LovyanGFX* dst, float angle) { return push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f); }

    template<typename T> bool pushRotateZoom(                                              float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotateZoom(LovyanGFX* dst                              , float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(    dst, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotateZoom(                int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotateZoom(LovyanGFX* dst, int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
                         bool pushRotateZoom(                                              float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(LovyanGFX* dst                              , float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(    dst, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(                int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(LovyanGFX* dst, int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y); }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    LovyanGFX* _parent;
    union {
      uint8_t*  _img;
      uint16_t* _img16;
      bgr888_t* _img24;
    };
    int32_t _bitwidth;
    int32_t _xptr;
    int32_t _yptr;
    int32_t _xs;
    int32_t _xe;
    int32_t _ys;
    int32_t _ye;
    int32_t _index;
    bool _use_spiram = false; // disable PSRAM to PSRAM memcpy flg.

    bool create_palette(void)
    {
      if (_write_conv.depth > 8) return false;

      if (_palette != nullptr) { _mem_free(_palette); _palette = nullptr; }
      _palette_count = 0;

      size_t palettes = 1 << _write_conv.bits;
      _palette = (bgr888_t*)_mem_alloc(sizeof(bgr888_t) * palettes);
      if (!_palette) {
        _write_conv.setColorDepth(_write_conv.depth, false);
        return false;
      }
      _palette_count = palettes;
      _write_conv.setColorDepth(_write_conv.depth, true);
      return true;
    }

    void push_sprite(LovyanGFX* dst, int32_t x, int32_t y, uint32_t transp = ~0)
    {
      pixelcopy_t p(_img, dst->getColorDepth(), getColorDepth(), dst->hasPalette(), _palette, transp);
      dst->push_image(x, y, _width, _height, &p, !_use_spiram); // DMA disable with use SPIRAM
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
      if (xs > xe) std::swap(xs, xe);
      if (ys > ye) std::swap(ys, ye);
      if ((xe < 0) || (ye < 0) || (xs >= _width) || (ys >= _height))
      {
        _xptr = _xs = _xe = 0;
        _yptr = _ys = _ye = _height;
      } else {
        _xptr = _xs = (xs < 0) ? 0 : xs;
        _yptr = _ys = (ys < 0) ? 0 : ys;
        _xe = std::min(xe, _width  - 1);
        _ye = std::min(ye, _height - 1);
      }
      _index = xs + ys * _bitwidth;
    }

    void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      auto bits = _write_conv.bits;
      if (bits >= 8) {
        int32_t index = x + y * _bitwidth;
        if (bits == 8) {
          _img[index] = _color.raw0;
        } else if (bits == 16) {
          _img16[index] = _color.rawL;
        } else {
          _img24[index] = *(bgr888_t*)&_color;
        }
      } else {
        int32_t index = (x + y * _bitwidth) * bits;
        uint8_t* dst = &_img[index >> 3];
        uint8_t mask = (uint8_t)(~(0xFF >> bits)) >> (index & 7);
        *dst = (*dst & ~mask) | (_color.raw0 & mask);
      }
    }

    void fillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
/* // for debug writeColor_impl
setWindow(x,y,x+w-1,y+h-1);
writeColor_impl(w*h);
return;
//*/
      uint32_t bits = _write_conv.bits;
      if (bits >= 8) {
        if (w == 1) {
          uint32_t bw = _bitwidth;
          uint32_t index = x + y * bw;
          if (bits == 8) {
            uint8_t c = _color.raw0;
            auto img = &_img[index];
            do { *img = c;  img += bw; } while (--h);
          } else if (bits == 16) {
            uint16_t c = _color.rawL;
            auto img = &_img16[index];
            do { *img = c;  img += bw; } while (--h);
          } else {  // if (_write_conv.bytes == 3)
            auto img = &_img24[index];
            uint32_t c = _color.raw;
            do { *img = *(bgr888_t*)&c; img += bw; } while (--h);
          }
        } else {
          uint32_t bytes = bits >> 3;
          uint32_t bw = _bitwidth;
          uint8_t* dst = &_img[(x + y * bw) * bytes];
          uint8_t c = _color.raw0;
          if (bytes == 1 || (c == _color.raw1 && (bytes == 2 || (c == _color.raw2)))) {
            if (w == bw) {
              memset(dst, c, w * bytes * h);
            } else {
              uint32_t add_dst = bw * bytes;
              do {
                memset(dst, c, w * bytes);
                dst += add_dst;
              } while (--h);
            }
          } else {
            size_t len = w * bytes;
            uint32_t add_dst = bw * bytes;
            uint32_t color = _color.raw;
            if (_use_spiram) {
              uint8_t linebuf[len];
              memset_multi(linebuf, color, bytes, w);
              do {
                memcpy(dst, linebuf, len);
                dst += add_dst;
              } while (--h);
            } else {
              if (w == bw) {
                memset_multi(dst, color, bytes, w * h);
              } else {
                memset_multi(dst, color, bytes, w);
                while (--h) {
                  memcpy(dst + add_dst, dst, len);
                  dst += add_dst;
                }
              }
            }
          }
        }
      } else {
        x *= bits;
        w *= bits;
        size_t add_dst = _bitwidth * bits >> 3;
        uint8_t* dst = &_img[y * add_dst + (x >> 3)];
        uint32_t len = ((x + w) >> 3) - (x >> 3);
        uint8_t mask = 0xFF >> (x & 7);
        uint8_t c = _color.raw0;
        if (len) {
          if (mask != 0xFF) {
            --len;
            auto d = dst++;
            uint8_t mc = c & mask;
            auto i = h;
            do { *d = (*d & ~mask) | mc; d += add_dst; } while (--i);
          }
          mask = ~(0xFF>>((x + w) & 7));
          if (len) {
            auto d = dst;
            auto i = h;
            do { memset(d, c, len); d += add_dst; } while (--i);
            dst += len;
          }
          if (mask == 0) return;
        } else {
          mask ^= mask >> w;
        }
        c &= mask;
        do { *dst = (*dst & ~mask) | c; dst += add_dst; } while (--h);
      }
    }

    void writeColor_impl(int32_t length) override
    {
      if (0 >= length) return;
      if (_write_conv.bytes == 0) {
        int32_t bits = _write_conv.bits;
        uint8_t c = _color.raw0;
        int32_t ll;
        int32_t index = _index;
        do {
          uint8_t* dst = &_img[index * bits >> 3];
          ll = std::min(_xe - _xptr + 1, length);
          int32_t w = ll * bits;
          int32_t x = _xptr * bits;
          size_t len = ((x + w) >> 3) - (x >> 3);
          uint8_t mask = 0xFF >> (x & 7);
          if (!len) {
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
        uint32_t bytes = _write_conv.bytes;
        if (bytes == 1 || (_color.raw0 == _color.raw1 && (bytes == 2 || (_color.raw0 == _color.raw2)))) {
          uint32_t color = _color.raw;
          int32_t ll;
          int32_t index = _index;
          do {
            ll = std::min(_xe - _xptr + 1, length);
            memset(&_img[index * bytes], color, ll * bytes);
            index = ptr_advance(ll);
          } while (length -= ll);
        } else
        if (_use_spiram) {
          int32_t buflen = std::min(_xe - _xs + 1, length);
          uint8_t linebuf[buflen * bytes];
          memset_multi(linebuf, _color.raw, bytes, buflen);
          int32_t ll;
          int32_t index = _index;
          do  {
            ll = std::min(_xe - _xptr + 1, length);
            memcpy(&_img[index * bytes], linebuf, ll * bytes);
            index = ptr_advance(ll);
          } while (length -= ll);
          return;
        } else {
          uint32_t color = _color.raw;
          int32_t ll;
          int32_t index = _index;
          do {
            ll = std::min(_xe - _xptr + 1, length);
            memset_multi(&_img[index * bytes], color, bytes, ll);
            index = ptr_advance(ll);
          } while (length -= ll);
        }
      }
    }

    void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) override
    {
      if (_write_conv.bits < 8) {
        pixelcopy_t param(_img, _write_conv.depth, _write_conv.depth);
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
          size_t len = (_bitwidth * _write_conv.bits) >> 3;
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
        size_t len = w * _write_conv.bytes;
        int32_t add = _bitwidth * _write_conv.bytes;
        if (src_y < dst_y) add = -add;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        uint8_t* src = &_img[(src_x + (src_y + pos) * _bitwidth) * _write_conv.bytes];
        uint8_t* dst = &_img[(dst_x + (dst_y + pos) * _bitwidth) * _write_conv.bytes];
        if (_use_spiram) {
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

    void readRect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) override
    {
      set_window(x, y, x + w - 1, y + h - 1);
      if (param->no_convert) {
        read_bytes((uint8_t*)dst, w * h * _read_conv.bytes);
      } else {
        read_pixels(dst, w * h, param);
      }
    }

    void read_pixels(void* dst, int32_t length, pixelcopy_t* param)
    {
      int32_t linelength;
      int32_t dstindex = 0;
      param->src_data = _img;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        param->src_x = _xptr;
        param->src_y = _yptr;
        dstindex = param->fp_copy(dst, dstindex, dstindex + linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void read_bytes(uint8_t* dst, int32_t length)
    {
      uint8_t b = _write_conv.bytes ? _write_conv.bytes : 1;
      length /= b;
      while (length) {
        int32_t linelength = std::min(_xe - _xptr + 1, length);
        memcpy(dst, &_img[_index * _write_conv.bytes], linelength * b);
        dst += linelength * b;
        ptr_advance(linelength);
        length -= linelength;
      }
    }

    void pushImage_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param, bool use_dma) override
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

    void pushColors_impl(int32_t length, pixelcopy_t* param) override
    {
      auto k = _bitwidth * _write_conv.bits >> 3;
      int32_t linelength;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        param->fp_copy(&_img[_yptr * k], _xptr, _xptr + linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void beginTransaction_impl(void) override {}
    void endTransaction_impl(void) override {}

    inline int32_t ptr_advance(int32_t length = 1) {
      if ((_xptr += length) > _xe) {
        _xptr = _xs;
        if (++_yptr > _ye) {
          _yptr = _ys;
        }
        return (_index = _xptr + _yptr * _bitwidth);
      } else {
        return (_index += length);
      }
    }



//----------------------------------------------------------------------------
#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)
//----------------------------------------------------------------------------

  public:
    void setPsram( bool enabled ) {
      if (enabled) _malloc_cap |= MALLOC_CAP_SPIRAM;
      else         _malloc_cap &= ~MALLOC_CAP_SPIRAM;
    }

    // set MALLOC_CAP_SPIRAM or MALLOC_CAP_DMA
    void setMallocCap(uint32_t flg) {
      _malloc_cap = flg;
    }

  protected:
    uint32_t _malloc_cap = MALLOC_CAP_8BIT;
    void* _mem_alloc(uint32_t bytes)
    {
      _use_spiram = (_malloc_cap & MALLOC_CAP_SPIRAM);
      void* res = heap_caps_malloc(bytes, _malloc_cap);

      // if can't malloc with PSRAM, try without using PSRAM malloc.
      if (res == nullptr && _use_spiram) {
        _use_spiram = false;
        res = heap_caps_malloc(bytes, _malloc_cap & ~MALLOC_CAP_SPIRAM);
      }
      return res;
    }
    void _mem_free(void* buf)
    {
      heap_caps_free(buf);
    }

//----------------------------------------------------------------------------
#elif defined (ESP8226)
//----------------------------------------------------------------------------
// not implemented.
//----------------------------------------------------------------------------
#elif defined (STM32F7)
//----------------------------------------------------------------------------
// not implemented.
//----------------------------------------------------------------------------
#elif defined (__AVR__)
//----------------------------------------------------------------------------
// not implemented.
//----------------------------------------------------------------------------

#endif

  };

}

#endif
