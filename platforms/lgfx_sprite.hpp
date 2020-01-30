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
      _has_transaction = false;
      _transaction_count = 0xFFFF;
    }

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

      int32_t x_mask = (_write_depth.bits & 7) ? (8 / (_write_depth.bits & 7)) - 1 : 0;
      _bitwidth = (w + x_mask) & ~x_mask;
      _img = (uint8_t*)_mem_alloc((h * _bitwidth * _write_depth.bits >> 3) + 1);
      if (!_img) return nullptr;

      if (_write_depth.depth <= palette_8bit) {
        size_t palettes = 1 << _write_depth.bits;
        _palette = (rgb888_t*)_mem_alloc(sizeof(rgb888_t) * palettes);
        if (!_palette) {
          deleteSprite();
          return nullptr;
        }
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
          case palette_8bit: k = 0x010101; break;
          case palette_4bit: k = 0x111111; break;
          case palette_2bit: k = 0x555555; break;
          case palette_1bit: k = 0xFFFFFF; break;
          default: k = 1; break;
          }
          for (uint32_t i = 0; i < palettes; i++) {
            _palette[i] = i * k;
          }
        }
      }
      _sw = _width = w;
      _xe = w - 1;
      _sh = _height = h;
      _ye = h - 1;
      _rotation = 0;
      _index = _sx = _sy = _xs = _ys = _xptr = _yptr = 0;

      clear();
      return _img;
    }

    void deleteSprite(void)
    {
      if (_img     != nullptr) { _mem_free(_img    ); _img     = nullptr; }
      if (_palette != nullptr) { _mem_free(_palette); _palette = nullptr; }
    }

    void setBitmapColor(uint16_t fgcolor, uint16_t bgcolor)  // For 1bpp sprites
    {
      if (_palette) {
        _palette[0] = *(rgb565_t*)&bgcolor;
        _palette[1] = *(rgb565_t*)&fgcolor;
      }
    }

    void setPalette(size_t index, const rgb888_t& rgb) {
      if (_palette) { _palette[index & ((1<<_write_depth.bits)-1)] = rgb; }
    }

    void setPalette(size_t index, uint8_t r, uint8_t g, uint8_t b) {
      if (_palette) { _palette[index & ((1<<_write_depth.bits)-1)].set(r, g, b); }
    }

    template<typename T>
    __attribute__ ((always_inline)) inline void fillSprite (const T& color) { fillRect(0, 0, _width, _height, color); }

    __attribute__ ((always_inline)) inline void pushSprite(int32_t x, int32_t y) { pushSprite(_parent, x, y); }
    void pushSprite(LovyanGFX* lgfx, int32_t x, int32_t y)
    {
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
    void pushSprite(LovyanGFX* lgfx, int32_t x, int32_t y, const T& transparent)
    {
      uint32_t transp = _write_depth.convert(transparent);
      switch (getColorDepth()) {
      case rgb888_3Byte: lgfx->pushImage(x, y, _width, _height, (swap888_t*)_img, transp); return;
      case rgb666_3Byte: lgfx->pushImage(x, y, _width, _height, (swap666_t*)_img, transp); return;
      case rgb565_2Byte: lgfx->pushImage(x, y, _width, _height, (swap565_t*)_img, transp); return;
      case rgb332_1Byte: lgfx->pushImage(x, y, _width, _height, (rgb332_t* )_img, transp); return;
      case palette_8bit: lgfx->pushIndexImage(x, y, _width, _height, (palette8_t*)_img, _palette, transp); return;
      case palette_4bit: lgfx->pushIndexImage(x, y, _width, _height, (palette4_t*)_img, _palette, transp); return;
      case palette_2bit: lgfx->pushIndexImage(x, y, _width, _height, (palette2_t*)_img, _palette, transp); return;
      case palette_1bit: lgfx->pushIndexImage(x, y, _width, _height, (palette1_t*)_img, _palette, transp); return;
      }
    }


    static void getRotatedBounds(float sina, float cosa, int16_t w, int16_t h, int16_t xp, int16_t yp,
                                       int16_t *min_x, int16_t *min_y, int16_t *max_x, int16_t *max_y)
    {
      w -= xp; // w is now right edge coordinate relative to xp
      h -= yp; // h is now bottom edge coordinate relative to yp

      // Calculate new corner coordinates
      int16_t x0 = -xp * cosa - yp * sina;
      int16_t y0 =  xp * sina - yp * cosa;

      int16_t x1 =  w * cosa - yp * sina;
      int16_t y1 = -w * sina - yp * cosa;

      int16_t x2 =  h * sina + w * cosa;
      int16_t y2 =  h * cosa - w * sina;

      int16_t x3 =  h * sina - xp * cosa;
      int16_t y3 =  h * cosa + xp * sina;

      // Find bounding box extremes, enlarge box to accomodate rounding errors
      *min_x = x0-2;
      if (x1 < *min_x) *min_x = x1-2;
      if (x2 < *min_x) *min_x = x2-2;
      if (x3 < *min_x) *min_x = x3-2;

      *max_x = x0+2;
      if (x1 > *max_x) *max_x = x1+2;
      if (x2 > *max_x) *max_x = x2+2;
      if (x3 > *max_x) *max_x = x3+2;

      *min_y = y0-2;
      if (y1 < *min_y) *min_y = y1-2;
      if (y2 < *min_y) *min_y = y2-2;
      if (y3 < *min_y) *min_y = y3-2;

      *max_y = y0+2;
      if (y1 > *max_y) *max_y = y1+2;
      if (y2 > *max_y) *max_y = y2+2;
      if (y3 > *max_y) *max_y = y3+2;

    }


    __attribute__ ((always_inline)) inline void pushRotated(int16_t angle, int32_t transp = -1) { pushRotated(_parent, angle, transp); }

    bool pushRotated(LovyanGFX* lgfx, int16_t angle, int32_t transp = -1)
    {
      // Trig values for the rotation
      float radAngle = -angle * 0.0174532925; // Convert degrees to radians
      float sinra = sin(radAngle);
      float cosra = cos(radAngle);

      // Bounding box parameters
      int16_t min_x;
      int16_t min_y;
      int16_t max_x;
      int16_t max_y;

      // Get the bounding box of this rotated source Sprite
      getRotatedBounds(sinra, cosra, width(), height(), _xpivot, _ypivot, &min_x, &min_y, &max_x, &max_y);

      // Move bounding box so source Sprite pivot coincides with destination Sprite pivot
      min_x += lgfx->getPivotX();
      max_x += lgfx->getPivotX();
      min_y += lgfx->getPivotY();
      max_y += lgfx->getPivotY();

      // Test only to show bounding box
      // lgfx->fillSprite(TFT_BLACK);
      // lgfx->drawRect(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1, 0x0F00);

      // Return if bounding box is completely outside of destination Sprite
      if (min_x > lgfx->width()) return true;
      if (min_y > lgfx->height()) return true;
      if (max_x < 0) return true;
      if (max_y < 0) return true;

      // Clip bounding box if it is partially within destination Sprite
      if (min_x < 0) min_x = 0;
      if (min_y < 0) min_y = 0;
      if (max_x > lgfx->width()) max_x = lgfx->width();
      if (max_y > lgfx->height()) max_y = lgfx->height();

      lgfx->startWrite();
      // Scan destination bounding box and fetch transformed pixels from source Sprite
      for (int32_t x = min_x; x <= max_x; x++)
      {
        int32_t xt = x - lgfx->getPivotX();
        float cxt = cosra * xt + _xpivot;
        float sxt = sinra * xt + _ypivot;
        bool column_drawn = false;
        for (int32_t y = min_y; y <= max_y; y++)
        {
          int32_t yt = y - lgfx->getPivotY();
          int32_t xs = (int32_t)round(cxt - sinra * yt);
          // Do not calculate ys unless xs is in bounds
          if (xs >= 0 && xs < width())
          {
            int32_t ys = (int32_t)round(sxt + cosra * yt);
            // Check if ys is in bounds
            if (ys >= 0 && ys < height())
            {
              int32_t rp = readPixel(xs, ys);
              if (rp != transp) lgfx->drawPixel(x, y, rp);
              column_drawn = true;
            }
          }
          else if (column_drawn) y = max_y; // Skip the remaining pixels below the Sprite
        }
      }
      lgfx->endWrite();

      return true;
    }


    inline void* buffer() { return _img; }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    LovyanGFX* _parent;
    union {
      uint8_t* _img;
      uint16_t* _img16;
      swap888_t* _img24;
    };
    rgb888_t* _palette;
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
      _write_depth.setColorDepth(depth);
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
        uint8_t addrshift = (4 - _write_depth.depth);
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
// unimplemented
/*
        int32_t sx = (src_x + (src_y + pos) * _bitwidth) * _write_depth.bits;
        int32_t dx = (dst_x + (dst_y + pos) * _bitwidth) * _write_depth.bits;
        uint8_t* src = &_img[sx >> 3];
        uint8_t* dst = &_img[dx >> 3];
        uint8_t buf[w];
//*/
      } else {
        int32_t add = _bitwidth * _write_depth.bytes;
        if (src_y < dst_y) add = -add;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        uint8_t* src = &_img[(src_x + (src_y + pos) * _bitwidth) * _write_depth.bytes];
        uint8_t* dst = &_img[(dst_x + (dst_y + pos) * _bitwidth) * _write_depth.bytes];
        size_t len = w * _write_depth.bytes;
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
      if (     _write_depth.depth == rgb332_1Byte) { res = *(( rgb332_t*)src); return res;}
      else if (_write_depth.depth == rgb565_2Byte) { res = *((swap565_t*)src); return res;}
//    else if (_write_depth.depth == rgb666_3Byte) { res = *((swap888_t*)src); return res;}
      else if (_write_depth.depth == rgb888_3Byte) { res = *((swap888_t*)src); return res;}

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
