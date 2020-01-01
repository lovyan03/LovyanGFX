#ifndef LGFX_PANEL_SPRITE_HPP_
#define LGFX_PANEL_SPRITE_HPP_


//#include "panel_common.hpp"
#include "LovyanGFX.hpp"

namespace lgfx
{
  class LGFXSpriteBase : public LovyanGFX
  {
    LovyanGFX* _parent = nullptr;
    virtual void* mallocSprite(uint32_t bytes, uint32_t param) = 0;
    virtual void freeSprite(void* buf) = 0;

  public:

    LGFXSpriteBase(LovyanGFX* parent)
    : LovyanGFX()
    , _img  (nullptr)
    , _bytes(2)
    , _xptr (0)
    , _yptr (0)
    , _xs   (0)
    , _xe   (0)
    , _ys   (0)
    , _ye   (0)
    , _index(0)
    , _malloc_cap(MALLOC_CAP_8BIT)
    , _parent(parent)
    {}

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

      _bitwidth = (_bpp == 1) ? (w + 7) & 0xFFF8 : w;
      _img = (uint8_t*)mallocSprite((_bitwidth * _bytes) * h + 1, _malloc_cap);
      if (!_img) return nullptr;
      _width = w;
      _xe = w - 1;
      _height = h;
      _ye = h - 1;
      _rotation = _index = _xs = _ys = _xptr = _yptr = 0;
      this->fillRect(0, 0, w, h, 0);
      return _img;
    }

    void deleteSprite(void)
    {
      if (_img == nullptr) return;
      freeSprite(_img);
      _img = nullptr;
    }

    template<typename T>
    inline void fillSprite (const T& color) {
      this->fillRect(0, 0, this->_width, this->_height, color);
    }

    __attribute__ ((always_inline)) inline void pushSprite(int32_t x, int32_t y) { pushSprite(_parent, x, y); }
    void pushSprite(LovyanGFX* lgfx, int32_t x, int32_t y) {
      switch (this->getColorDepth()) {
    //case  1: // unimplimented
      case  8: lgfx->pushImage(x, y, this->_width, this->_height, (rgb332_t*)_img); break;
      case 16: lgfx->pushImage(x, y, this->_width, this->_height, (swap565_t*)_img); break;
      case 24: lgfx->pushImage(x, y, this->_width, this->_height, (swap888_t*)_img); break;
      }
    }

    inline void* buffer() { return _img; }

    void* setColorDepth(uint8_t bpp) override  // bpp : 1 , 8 , 16 , 24
    {
      if (     bpp > 16) _bpp = 24;
      else if (bpp >  8) _bpp = 16;
      else if (bpp >  1) _bpp =  8;
      else               _bpp =  1;
      _bytes = _bpp >> 3;
      if (_img == nullptr) return nullptr;
      deleteSprite();
      return createSprite(_width, _height);
    }

    void setRotation(uint8_t r) override
    {
      r = r & 7;
      if ((_rotation&1) != (r&1)) {
        uint32_t tmp = _width;
        _width = _height;
        _height = tmp;
      }
      _rotation = r;
    }

    void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      _xe = xe;
      _ye = ye;
      _xptr = _xs = xs;
      _yptr = _ys = ys;
      _index = xs + ys * _bitwidth;
    }

    void readWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      setWindow(xs, ys, xe, ye);
    }
/*
    void _drawPixel(int32_t x, int32_t y) override
    {
      if (x < 0 || (x >= _width) || y < 0 || (y >= _height)) return;
      if (_bpp == 8) {
        _img[x + y * _bitwidth] = _color.raw0;
      } else if (_bpp == 16) {
        _img16[x + y * _bitwidth] = *(swap565_t*)&_color;
      } else if (_bpp == 24) {
        _img24[x + y * _bitwidth] = *(swap888_t*)&_color;
      }
    }

    void _draw_fast_vline(int32_t x, int32_t y, int32_t h) override
    {
      if (_bpp == 8) {
        for (int32_t i = 0; i < h; i++) _img[x + (y + i) * _bitwidth] = _color.raw0;
      } else if (_bpp == 16) {
        for (int32_t i = 0; i < h; i++) _img16[x + (y + i) * _bitwidth] = *(swap565_t*)&_color;
      } else if (_bpp == 24) {
        for (int32_t i = 0; i < h; i++) _img24[x + (y + i) * _bitwidth] = *(swap888_t*)&_color;
      }
    }
//*/
    void fillWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      if (xs == xe) {
        if (_bpp == 16) {
          do { _img16[xs + ys * _bitwidth] = *(swap565_t*)&_color; } while (++ys <= ye);
        } else if (_bpp == 8) {
          do { _img[xs + ys * _bitwidth] = _color.raw0; } while (++ys <= ye);
        } else if (_bpp == 24) {
          do { _img24[xs + ys * _bitwidth] = *(swap888_t*)&_color; } while (++ys <= ye);
        } else { // _bpp == 1
      // unimplemanted
        }
        return;
      }
      if (_bpp == 1) {
        // unimplemanted
      } else {
        if (_bpp == 8 || (_color.raw0 == _color.raw1 && (_bpp == 16 || (_color.raw0 == _color.raw2)))) {
          if (xs == 0 && xe == _width) {
            memset(&_img[ys * _bitwidth * _bytes], _color.raw0, _width *  (ye - ys + 1) * _bytes);
          } else {
            for (int32_t y = ys; y <= ye; y++) {
              memset(&_img[(xs + y * _bitwidth) * _bytes], _color.raw0, (xe - xs + 1) * _bytes);
            }
          }
        } else {
          _index = xs + ys * _bitwidth;
          uint8_t* src = &_img[_index * _bytes];
          if (_bpp == 8) {
            *src = _color.raw0;
          } else if (_bpp == 16) {
            _img16[xs + ys * _bitwidth] = *(swap565_t*)&_color;
          } else if (_bpp == 24) {
            _img24[xs + ys * _bitwidth] = *(swap888_t*)&_color;
          }
          if (xs == 0 && xe == _width) {
            dupe_fill(src + _bytes, src, (_bitwidth * (ye - ys + 1) - 1) * _bytes, _bytes);
            return;
          }
          dupe_fill(src + _bytes, src, (xe - xs) * _bytes, _bytes);
          if (ye == ys) return;
          uint32_t len = (xe - xs + 1) * _bytes;
          uint8_t* dst = src;
          for (int32_t y = ys + 1; y <= ye; y++) {
            dst += _bitwidth * _bytes;
            memcpy(dst, src, len);
          }
        }
      }
    }
    void _writeColor(uint32_t length) override
    {
      if (!length) return;
      if (_bpp == 1) {
        // unimplemanted
        uint8_t *dst = ptr_img();
        bool col = _color;
        for (;;) {
          if (col) *dst |=  (0x80>>(_index & 7));
          else     *dst &= ~(0x80>>(_index & 7));
          if (0 == --length) return;
          if (ptr_advance()) dst = ptr_img();
          else if (!(_index & 0x07)) ++dst;
        }
      } else
      if (_bpp == 8 || (_color.raw0 == _color.raw1 && (_bpp == 16 || (_color.raw0 == _color.raw2)))) {
        uint32_t len;
        do {
          len = (_xe - _xptr + 1);
          if (len > length) len = length;
          memset(&_img[_index * _bytes], _color.raw0, len * _bytes);
          ptr_advance(len);
        } while (length -= len);
      } else {
        uint8_t* src = &_img[_index * _bytes];
        memcpy(src, &_color.raw, _bytes);
        if (0 == --length) {
          ptr_advance();
          return;
        }
        uint32_t len = 1;
        if (!ptr_advance()) {
          len = (_xe - _xptr) + 1;
          if (len > length) len = length;
          dupe_fill(&_img[_index * _bytes], src, len * _bytes, _bytes);
          ptr_advance(len);
          if (0 == (length -= len)) return;
          len++;
        }
        uint8_t* dst = &_img[_index * _bytes];
        if (len > length) len = length;
        memcpy(dst, src, len * _bytes);
        if (!ptr_advance(len)) {
          if (0 == (length -= len)) return;
          src = dst;
          dst = &_img[_index * _bytes];
          uint32_t step = len * _bytes;
          len = (_xe - _xptr);
          if (len > length) len = length;
          dupe_fill(dst, src, len * _bytes, step);
          ptr_advance(len);
        }
        if (0 == (length -= len)) return;
        len = (_xe - _xs + 1);
        do {
          if (len > length) len = length;
          memcpy(&_img[_index * _bytes], src, len * _bytes);
          ptr_advance(len);
        } while (length -= len);
      }
    }

    rgb565_t _readPixel(void) override
    {
      const uint8_t *src = ptr_img();
      rgb565_t res;
      if (     _bpp ==  8) { res = *(( rgb332_t*)src); return res;}
      else if (_bpp == 16) { res = *((swap565_t*)src); return res;}
      else if (_bpp == 24) { res = *((swap888_t*)src); return res;}

      return (bool)(*src & (0x80 >> (_index & 0x07)));
    }

    void _readPixels(rgb332_t*   buf, uint32_t length) override { readPixelsTemplate(buf, length); }
    void _readPixels(rgb565_t*   buf, uint32_t length) override { readPixelsTemplate(buf, length); }
    void _readPixels(rgb888_t*   buf, uint32_t length) override { readPixelsTemplate(buf, length); }
    void _readPixels(swap565_t*  buf, uint32_t length) override { readPixelsTemplate(buf, length); }
    void _readPixels(swap888_t*  buf, uint32_t length) override { readPixelsTemplate(buf, length); }
    void _readPixels(argb8888_t* buf, uint32_t length) override { readPixelsTemplate(buf, length); }

    void _writePixels(const rgb332_t*   src, uint32_t length) override { writePixelsTemplate(src, length); }
    void _writePixels(const rgb565_t*   src, uint32_t length) override { writePixelsTemplate(src, length); }
    void _writePixels(const rgb888_t*   src, uint32_t length) override { writePixelsTemplate(src, length); }
    void _writePixels(const swap565_t*  src, uint32_t length) override { writePixelsTemplate(src, length); }
    void _writePixels(const swap888_t*  src, uint32_t length) override { writePixelsTemplate(src, length); }
    void _writePixels(const argb8888_t* src, uint32_t length) override { writePixelsTemplate(src, length); }

//----------------------------------------------------------------------------
  protected:

    template <class TSrc>
    void writePixelsTemplate(const TSrc* src, uint32_t length)
    {
      if (!length) return;
      if (     _bpp ==  8) { write_pixels< rgb332_t>(src, length); }
      else if (_bpp == 16) { write_pixels<swap565_t>(src, length); }
      else if (_bpp == 24) { write_pixels<swap888_t>(src, length); }
/*
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
*/
    }

    template <class TDst>
    void readPixelsTemplate(TDst* buf, uint32_t length)
    {
      if (!length) return;
      if (     _bpp ==  8) { read_pixels< rgb332_t>(buf, length); }
      else if (_bpp == 16) { read_pixels<swap565_t>(buf, length); }
      else if (_bpp == 24) { read_pixels<swap888_t>(buf, length); }
/*
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
*/
    }

    void dupe_fill(uint8_t* dst, uint8_t* src, size_t len, size_t size) {
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
/*
    void memset_multi(uint8_t* buf, uint32_t c, size_t len, size_t size) {
      memcpy(buf, &c, size);
      if (!len) return;
      len = (len - 1) * size;
      void* dst = buf + size;
      while (len > size) {
        memcpy(dst, buf, size);
        dst += size;
        len -= size;
        size <<= 1;
      }
      if (len) {
        memcpy(dst, buf, len);
      }
    }
//*/
    template <class TDst, class TSrc>
    void write_pixels(const TSrc* src, uint32_t length)
    {
      auto *dst = (TDst*)ptr_img();
      while (length--) {
        *dst++ = *src++;
        if (ptr_advance()) dst = (TDst*)ptr_img();
      }
    }

    template <class TSrc, class TDst>
    void read_pixels(TDst* buf, uint32_t length)
    {
      auto src = (const TSrc*)ptr_img();
      while (length--) {
        *buf++ = *src++;
        if (ptr_advance()) src = (const TSrc*)ptr_img();
        auto src = (TSrc*)ptr_img();
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
      return &_img[_bytes ? (_index * _bytes) : (_index >> 3)];
    }
    union {
    uint8_t* _img;
    swap565_t* _img16;
    swap888_t* _img24;
    };
    uint8_t _bytes;
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
}

#endif
