#ifndef LGFX_SPRITE_HPP_
#define LGFX_SPRITE_HPP_

#include "LovyanGFX.hpp"

namespace lgfx
{
  class LGFXSpriteBase : public LovyanGFX
  {
  public:

    virtual void setPsram( bool enabled ) {};

    LGFXSpriteBase(LovyanGFX* parent)
    : LovyanGFX()
    , _img  (nullptr)
    , _palette(nullptr)
    , _xptr (0)
    , _yptr (0)
    , _xs   (0)
    , _xe   (0)
    , _ys   (0)
    , _ye   (0)
    , _index(0)
    , _malloc_cap(MALLOC_CAP_8BIT)
    , _parent(parent)
    {
      _start_write_count = 0xFFFF;
    }

    LGFXSpriteBase()
    : LGFXSpriteBase(nullptr)
    {}

    virtual ~LGFXSpriteBase() {
      deleteSprite();
    }

    // set MALLOC_CAP_SPIRAM or MALLOC_CAP_DMA
    void setMallocCap(uint32_t flg) {
      _malloc_cap = flg;
    }

    void* createSprite(int32_t w, int32_t h)
    {
      if (w < 1 || h < 1) return nullptr;
      if (_img) deleteSprite();

      if (_color.bpp >= palette_1bit && _color.bpp <= palette_8bit) {
        _bitwidth = (w + _color.x_mask) & (~_color.x_mask);
        _img = (uint8_t*)_mem_alloc((_bitwidth >> (4 - _color.bpp)) * h + 1, _malloc_cap);
        if (_img) {
          size_t palettes = ((size_t)_color.colormask+1);
          _palette = (rgb888_t*)_mem_alloc(sizeof(rgb888_t) * palettes, _malloc_cap);

if (_color.bpp == palette_8bit) {
  for (uint32_t i = 0; i < palettes; i++) {
    _palette[i] = *(rgb332_t*)&i;
  }
} else
//*/
{
uint32_t k;
switch (_color.bpp) {
case palette_8bit: k = 0x010101; break;
case palette_4bit: k = 0x111111; break;
case palette_2bit: k = 0x555555; break;
case palette_1bit: k = 0xFFFFFF; break;
}
  for (uint32_t i = 0; i < palettes; i++) {
    _palette[i] = i * k;
  }
}
        }
      } else {
        _bitwidth = w;
        _img = (uint8_t*)_mem_alloc((_bitwidth * _color.bytes) * h + 1, _malloc_cap);
      }
      if (!_img) return nullptr;
      _width = w;
      _xe = w - 1;
      _height = h;
      _ye = h - 1;
      _rotation = _index = _xs = _ys = _xptr = _yptr = 0;
      fillRect(0, 0, w, h, 0);
      return _img;
    }

    void deleteSprite(void)
    {
      if (_img     != nullptr) { _mem_free(_img    ); _img     = nullptr; }
      if (_palette != nullptr) { _mem_free(_palette); _palette = nullptr; }
    }

    template<typename T>
    __attribute__ ((always_inline)) inline void fillSprite (const T& color) { fillRect(0, 0, _width, _height, color); }

    __attribute__ ((always_inline)) inline void pushSprite(int32_t x, int32_t y) { pushSprite(_parent, x, y); }
    void pushSprite(LovyanGFX* lgfx, int32_t x, int32_t y) {
      switch (getColorDepth()) {
      case rgb888_3Byte: lgfx->pushImage(x, y, _width, _height, (swap888_t*)_img); return;
      case rgb666_3Byte: lgfx->pushImage(x, y, _width, _height, (swap666_t*)_img); return;
      case rgb565_2Byte: lgfx->pushImage(x, y, _width, _height, (swap565_t*)_img); return;
      case rgb332_1Byte: lgfx->pushImage(x, y, _width, _height, (rgb332_t* )_img); return;
      case palette_8bit: lgfx->pushIndexImage(x, y, _width, _height, (palette8_t*)_img, _palette); return;
      case palette_4bit: lgfx->pushIndexImage(x, y, _width, _height, (palette4_t*)_img, _palette); return;
      case palette_2bit: lgfx->pushIndexImage(x, y, _width, _height, (palette2_t*)_img, _palette); return;
      case palette_1bit: lgfx->pushIndexImage(x, y, _width, _height, (palette1_t*)_img, _palette); return;
      }
    }

    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(int32_t x, int32_t y, const T& transparent) { pushSprite(_parent, x, y, transparent); }

    template<typename T>
    void pushSprite(LovyanGFX* lgfx, int32_t x, int32_t y, const T& transparent) {
      auto transp = _color.convertColor(transparent);
      switch (getColorDepth()) {
      case rgb888_3Byte: lgfx->pushImage(x, y, _width, _height, (swap888_t*)_img, *(swap888_t*)&transp); return;
      case rgb666_3Byte: lgfx->pushImage(x, y, _width, _height, (swap666_t*)_img, *(swap666_t*)&transp); return;
      case rgb565_2Byte: lgfx->pushImage(x, y, _width, _height, (swap565_t*)_img, *(swap565_t*)&transp); return;
      case rgb332_1Byte: lgfx->pushImage(x, y, _width, _height, (rgb332_t* )_img, *(rgb332_t* )&transp); return;
      case palette_8bit: lgfx->pushIndexImage(x, y, _width, _height, (palette8_t*)_img, _palette, *(palette8_t*)&transp); return;
      case palette_4bit: lgfx->pushIndexImage(x, y, _width, _height, (palette4_t*)_img, _palette, *(palette4_t*)&transp); return;
      case palette_2bit: lgfx->pushIndexImage(x, y, _width, _height, (palette2_t*)_img, _palette, *(palette2_t*)&transp); return;
      case palette_1bit: lgfx->pushIndexImage(x, y, _width, _height, (palette1_t*)_img, _palette, *(palette1_t*)&transp); return;
      }
    }

    inline void* buffer() { return _img; }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    LovyanGFX* _parent = nullptr;

    virtual void* _mem_alloc(uint32_t bytes, uint32_t param) = 0;
    virtual void _mem_free(void* buf) = 0;

    __attribute__ ((always_inline)) inline void set_window(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      _xe = xe;
      _ye = ye;
      _xptr = _xs = xs;
      _yptr = _ys = ys;
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

    void* setColorDepth_impl(color_depth_t bpp) override
    {
      _color.setColorDepth(bpp);
      _readColorDepth = getColorDepth();

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
      if (_color.bytes == 2) {
        ((uint16_t*)_img16)[x + y * _bitwidth] = *(uint16_t*)&_color;
      } else if (_color.bytes == 1) {
        _img[x + y * _bitwidth] = _color.raw0;
      } else if (_color.bytes == 3) {
        _img24[x + y * _bitwidth] = *(swap888_t*)&_color;
      } else {
        uint8_t mask = _color.colormask << (((~x) & _color.x_mask) << (_color.bpp - 1));
        auto dst = &_img[(x + y * _bitwidth) >> (4 - _color.bpp)];
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
      if (_color.bytes == 0) {
        uint8_t addrshift = (4 - _color.bpp);
        size_t d = _bitwidth >> addrshift;
        uint8_t* dst = &_img[(x + y * _bitwidth) >> addrshift];
        size_t len = ((x+w) >> addrshift) - (x >> addrshift);
        uint8_t mask = 0xFF >> ((x & _color.x_mask) * _color.bits);
        uint8_t c = _color.raw0;
        if (!len) {
          mask &= ~(mask >> (w * _color.bits));
          c &= mask;
        } else {
          if (mask != 0xFF) {
            auto dst2 = dst;
            auto h2 = h;
            c &= mask;
            do { *dst2 = (*dst2 & ~mask) | c; dst2 += d; } while (--h2);
            dst++; len--;
          }
          mask = ~(0xFF>>(((x+w) & _color.x_mask) * _color.bits));
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
        uint8_t* dst = &_img[(x + y * _bitwidth) * _color.bytes];
        if (_color.bytes == 2) {
          do { *(uint16_t*)dst = *(uint16_t*)&_color;   dst += _bitwidth << 1; } while (--h);
        } else if (_color.bytes == 1) {
          do {             *dst = _color.raw0;          dst += _bitwidth;      } while (--h);
        } else if (_color.bytes == 3) {
          do { *(swap888_t*)dst = *(swap888_t*)&_color; dst += _bitwidth * 3;  } while (--h);
        } else {
        }
      } else {
        uint8_t* dst = &_img[(x + y * _bitwidth) * _color.bytes];
        if (_color.bytes == 1 || (_color.raw0 == _color.raw1 && (_color.bytes == 2 || (_color.raw0 == _color.raw2)))) {
          if (x == 0 && w == _width) {
            memset(dst, _color.raw0, _width * h * _color.bytes);
          } else {
            do {
              memset(dst, _color.raw0, w * _color.bytes);
              dst += _width * _color.bytes;
            } while (--h);
          }
        } else {
          size_t len = w * _color.bytes;
          uint32_t down = _width * _color.bytes;
          if (_disable_memcpy) {
            uint8_t linebuf[len];
            memset_multi(linebuf, _color.raw, _color.bytes, w);
            do {
              memcpy(dst, linebuf, len);
              dst += down;
            } while (--h);
          } else {
            memset_multi(dst, _color.raw, _color.bytes, w);
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
      if (_color.bytes == 0) {
        uint8_t c = _color.raw0;
        uint8_t addrshift = (4 - _color.bpp);
        int32_t linelength;
        do {
          uint8_t* dst = &_img[(_xptr + _yptr * _bitwidth) >> addrshift];
          linelength = std::min(_xe - _xptr + 1, length);
          size_t len = ((_xptr + linelength) >> addrshift) - (_xptr >> addrshift);
          uint8_t mask = 0xFF >> ((_xptr & _color.x_mask) * _color.bits);
          if (!len) {
            mask &= ~(mask >> (linelength * _color.bits));
          } else {
            *dst = (*dst & ~mask) | (c & mask);
            if (len != 1) {
              memset(dst+1, c, len-1);
            }
            dst += len;
            mask = ~(0xFF>>(((_xptr + linelength) & _color.x_mask) * _color.bits));
          }
          *dst = (*dst & ~mask) | c & mask;
          ptr_advance(linelength);
        } while (length -= linelength);

      } else
      if (_color.bytes == 1 || (_color.raw0 == _color.raw1 && (_color.bytes == 2 || (_color.raw0 == _color.raw2)))) {
        int32_t linelength;
        do {
          linelength = std::min(_xe - _xptr + 1, length);
          memset(&_img[_index * _color.bytes], _color.raw0, linelength * _color.bytes);
          ptr_advance(linelength);
        } while (length -= linelength);
      } else {
        uint8_t* dst = &_img[_index * _color.bytes];
        if (_disable_memcpy) {
          int32_t buflen = std::min(_xe - _xs + 1, length);
          uint8_t linebuf[buflen * _color.bytes];
          memset_multi(linebuf, _color.raw, _color.bytes, buflen);
          uint32_t len;
          do  {
            len = std::min(length, _xe - _xptr + 1);
            memcpy(&_img[_index * _color.bytes], linebuf, len * _color.bytes);
            ptr_advance(len);
          } while (length -= len);
          return;
        }

        uint32_t advance = std::min(_xe - _xptr + 1, length);
        if (_xs != _xptr) {
          memset_multi(dst, _color.raw, _color.bytes, advance);
          ptr_advance(advance);
          if (0 == (length -= advance)) return;
          dst = &_img[_index * _color.bytes];
          advance = std::min(_xe - _xptr + 1, length);
        }
        memset_multi(dst, _color.raw, _color.bytes, advance);
        ptr_advance(advance);
        if (0 == (length -= advance)) return;
        uint32_t down = _width * _color.bytes;
        while (length > advance) {
          memcpy(dst+down, dst, advance * _color.bytes);
          dst += down;
          ptr_advance(advance);
          length -= advance;
        }
        if (length) {
          memcpy(dst + down, dst, length * _color.bytes);
          ptr_advance(length);
        }
      }
    }

    void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) override
    {
      if (_color.bpp == 1) {
      } else {
        int32_t add = _width * _color.bytes;
        if (src_y < dst_y) add = -add;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        uint8_t* src = &_img[(src_x + (src_y + pos) * _width) * _color.bytes];
        uint8_t* dst = &_img[(dst_x + (dst_y + pos) * _width) * _color.bytes];
        size_t len = w * _color.bytes;
        if (_disable_memcpy) {
          uint8_t buf[len];
          for (int count = 0; count < h; count++) {
            memcpy(buf, src, len);
            memcpy(dst, buf, len);
            src += add;
            dst += add;
          }
        } else {
          for (int count = 0; count < h; count++) {
            memcpy(dst, src, len);
            src += add;
            dst += add;
          }
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
      if (     _color.bpp ==  8) { res = *(( rgb332_t*)src); return res;}
      else if (_color.bpp == 16) { res = *((swap565_t*)src); return res;}
//    else if (_color.bpp == 18) { res = *((swap888_t*)src); return res;}
      else if (_color.bpp == 24) { res = *((swap888_t*)src); return res;}

      return (bool)(*src & (0x80 >> (_index & 0x07)));
    }
/*
    template<class T>
    __attribute__ ((always_inline)) inline void readRectTemplate(int32_t x, int32_t y, int32_t w, int32_t h, T* buf)
    {
      if (     _color.bpp ==  8) { read_pixels< rgb332_t>(x, y ,w ,h ,buf); }
      else if (_color.bpp == 16) { read_pixels<swap565_t>(x, y ,w ,h ,buf); }
      else if (_color.bpp == 24) { read_pixels<swap888_t>(x, y ,w ,h ,buf); }
      else {

        uint8_t p = 0;
        const uint8_t *src = ptr_img();
        for (;;) {
          if (*src & (0x80 >> (_index & 0x07))) *buf |=  (0x80>>p);
          else                                  *buf &= ~(0x80>>p);
          if (0 == --length) return;
          if (ptr_advance()) src = ptr_img();
          else if (!(_index & 0x07)) ++src;
          if (++p == 8) {
            p = 0;
            ++buf;
          }
        }

      }
    }
*/
/*
    template <class TSrc>
    void writePixelsTemplate(const TSrc* src, uint32_t length)
    {
      if (!length) return;
      if (     _color.bpp ==  8) { write_pixels< rgb332_t>(src, length); }
      else if (_color.bpp == 16) { write_pixels<swap565_t>(src, length); }
      else if (_color.bpp == 24) { write_pixels<swap888_t>(src, length); }
      else {

        const uint8_t* s(static_cast<const uint8_t*>(src));
        uint8_t p = 0;
        uint8_t *dst = ptr_img();
        for (;;) {
          if (*s & (0x80 >> p)) *dst |=  (0x80>>(_index & 0x07));
          else                  *dst &= ~(0x80>>(_index & 0x07));
          if (0 == --length) return;
          if (ptr_advance()) dst = ptr_img();
          else if (!(_index & 0x07)) ++dst;
          if (++p == 8) {
            p = 0;
            ++s;
          }
        }
      }
    }
*/
/*
    static void dupe_fill(uint8_t* dst, const uint8_t* src, size_t len, size_t size) {
      while (len > size) {
        memcpy(dst, src, size);
        dst += size;
        len -= size;
        size <<= 1;
      }
      if (len) {
        memcpy(dst, src, len);
      }
    }
//*/
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
          *(uint16_t*)dst = *(uint16_t*)&c; dst += 2;
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

    void read_pixels(void* dst, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) override
    {
      int32_t linelength;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        const void* src = ptr_img();
        fp_copy(dst, src, linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void read_bytes(uint8_t* dst, int32_t length) override
    {
      uint8_t b = _color.bytes ? _color.bytes : 1;
      length /= b;
      while (length) {
        int32_t linelength = std::min(_xe - _xptr + 1, length);
        memcpy(dst, ptr_img(), linelength * b);
        dst += linelength * b;
        ptr_advance(linelength);
        length -= linelength;
      }
    }

    void write_pixels(const void* src, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) override
    {
      int32_t linelength;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        void* dst = ptr_img();
        fp_copy(dst, src, linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void write_bytes(const uint8_t* data, int32_t length) override
    {
      uint8_t b = _color.bytes ? _color.bytes : 1;
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
      return &_img[_color.bytes ? (_index * _color.bytes) : (_index >> (4-_color.bpp))];
    }
    union {
    uint8_t* _img;
    swap565_t* _img16;
    swap888_t* _img24;
    };
    rgb888_t* _palette;
    bool _disable_memcpy = false;
    int32_t _bitwidth;
    int32_t _xptr;
    int32_t _yptr;
    int32_t _xs;
    int32_t _xe;
    int32_t _ys;
    int32_t _ye;
    uint32_t _index;
    uint32_t _malloc_cap;
    //uint8_t _iptr;
    //uint32_t _bitmap_fg;
    //uint32_t _bitmap_bg;
  };


#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)
  class LGFXSprite : public LGFXSpriteBase
  {
  public:
    LGFXSprite() : LGFXSpriteBase() {}
    LGFXSprite(LovyanGFX* parent) : LGFXSpriteBase(parent) {}

    void setPsram( bool enabled ) override {
      if (enabled) _malloc_cap |= MALLOC_CAP_SPIRAM;
      else         _malloc_cap &= ~MALLOC_CAP_SPIRAM;
    }

  protected:
    void* _mem_alloc(uint32_t bytes, uint32_t param) override
    {
      if ( !psramFound() ) param &= ~MALLOC_CAP_SPIRAM;
      _disable_memcpy = (param & MALLOC_CAP_SPIRAM);
      return heap_caps_malloc(bytes, param);
    }
    void _mem_free(void* buf) override
    {
      heap_caps_free(buf);
    }
  };

#elif defined (STM32F7)

#elif defined (__AVR__)

#endif

}

#endif
