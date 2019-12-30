#ifndef LGFX_ESP32_SPRITE_HPP_
#define LGFX_ESP32_SPRITE_HPP_

// #pragma GCC optimize ("O3")

#include <esp_heap_caps.h>

#include "lgfx_common.hpp"
#include "esp32_common.hpp"

namespace lgfx
{
  class Esp32Sprite
  {
  public:
#define TYPECHECK(dType) template < typename tType, typename std::enable_if < (sizeof(tType) == sizeof(dType)) && (std::is_signed<tType>::value == std::is_signed<dType>::value), std::nullptr_t >::type=nullptr >
    TYPECHECK(uint8_t ) __attribute__ ((always_inline)) inline dev_color_t getDevColor(tType c) { return getDevColor(*(const rgb332_t*)&c); }
    TYPECHECK(uint16_t) __attribute__ ((always_inline)) inline dev_color_t getDevColor(tType c) { return getDevColor(*(const rgb565_t*)&c); }
    TYPECHECK(uint32_t) __attribute__ ((always_inline)) inline dev_color_t getDevColor(tType c) { return getDevColor(*(const rgb888_t*)&c); }
    TYPECHECK(int     ) __attribute__ ((always_inline)) inline dev_color_t getDevColor(tType c) { return getDevColor(*(const rgb565_t*)&c); }
#undef TYPECHECK
    inline dev_color_t getDevColor(const rgb888_t& c) {
      if (_bpp ==  8) return color332(c.r, c.g, c.b);
      if (_bpp == 16) return swap565( c.r, c.g, c.b);
      if (_bpp == 24) return c.r | c.g<<8 | c.b<<16;
      return (bool)c;
    }
    inline dev_color_t getDevColor(const rgb565_t& c) {
      if (_bpp ==  8) return ((c.r<<3)&0xE0) | ((c.g>>1)&0x1C) | (c.b>>3);
      if (_bpp == 16) return c.raw << 8 | c.raw >> 8;
      if (_bpp == 24) return c.R8() | (c.G8()<<8) | (c.B8()<<16);
      return (bool)c;
    }
    inline dev_color_t getDevColor(const rgb332_t& c) {
      if (_bpp ==  8) return c.raw;
      if (_bpp == 16) return swap565(c.R8(), c.G8(), c.B8());
      if (_bpp == 24) return swap888(c.R8(), c.G8(), c.B8());
      return (bool)c;
    }

    Esp32Sprite()
    : _img     (nullptr)
    , _bpp     (16)
    , _bytes   (2)
    , _rotation(0)
    , _width   (0)
    , _height  (0)
    , _xptr    (0)
    , _yptr    (0)
    , _xs      (0)
    , _xe      (0)
    , _ys      (0)
    , _ye      (0)
    , _malloc_cap(MALLOC_CAP_8BIT)
    {
    }

    virtual ~Esp32Sprite() {
      deleteSprite();
    }

    inline static void init(void) {}

    // set MALLOC_CAP_SPIRAM or MALLOC_CAP_DMA
    void setMallocCap(uint32_t flg) {
      _malloc_cap = flg;
    }

    void* createSprite(int32_t w, int32_t h)
    {
      if (w < 1 || h < 1) return nullptr;
      if (_img) deleteSprite();

      _bitwidth = (_bpp == 1) ? (w + 7) & 0xFFF8 : w;
      _img = (uint8_t*)heap_caps_malloc((_bitwidth * _bytes) * h + 1, _malloc_cap);
      if (!_img) return nullptr;
      _width = w;
      _xe = w - 1;
      _height = h;
      _ye = h - 1;
      _rotation = _index = _xs = _ys = _xptr = _yptr = 0;
      return _img;
    }

    void deleteSprite(void)
    {
      if (_img == nullptr) return;
      heap_caps_free(_img);
      _img = nullptr;
    }


    inline static void beginTransaction(void) {}
    inline static void endTransaction(void) {}
    inline static void invertDisplay(bool i) {}
    inline static bool getInvert(void) { return false; }
    inline static uint32_t readPanelID(void) { return 0; }
    inline static uint32_t readPanelIDSub(uint8_t cmd) { return 0; }

    inline int32_t width(void) const { return _width; }
    inline int32_t height(void) const { return _height; }
    inline uint8_t getRotation(void) const { return _rotation; }
    inline uint8_t getColorDepth(void) const { return _bpp; }

    inline uint8_t* buffer() { return _img; }

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

    void* setColorDepth(uint8_t bpp)  // bpp : 1 , 8 , 16 , 24
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

    inline void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      _xe = xe;
      _ye = ye;
      _xptr = _xs = xs;
      _yptr = _ys = ys;
      _index = xs + ys * _bitwidth;
    }

    inline void fillWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye, const dev_color_t& color)
    {
      setWindow(xs, ys, xe, ye);
      writeColor(color, (xe-xs+1) * (ye-ys+1));
/*
      while (int32_t 
        auto dst = (T*)ptr_img();
        *dst = color;
        if (0 == --length) { ptr_advance(); return; }

        auto cpysrc = dst++;
        uint32_t step = 1;
        uint32_t line_rest = _xe - _xptr;
        while (length) {
          if (ptr_advance()) {
            dst = (T*)ptr_img();
            line_rest = 1 + (_xe - _xptr);
            if (line_rest > length) {
              line_rest = length;
              if (step > line_rest) step = line_rest;
            }
            memcpy(dst, cpysrc, sizeof(T) * step);
            _xptr += step;
            if (0 == (length -= step)) return;
            if (0 == (line_rest -= step)) continue;
            cpysrc = dst;
            dst += step;
          } else {
            line_rest = _xe - _xptr;
          }
          if (line_rest > length) {
            line_rest = length;
          }
          while (line_rest) {
            if (step > line_rest) step = line_rest;
            memcpy(dst, cpysrc, sizeof(T) * step);
            dst += step;
            _xptr += step;
            length -= step;
            line_rest -= step;
            step << 1;
          }
        }
      }
//*/
    }

    inline void readWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      setWindow(xs, ys, xe, ye);
    }

    inline static void startRead() {}
    inline static void endRead() {}

    inline void writeColor(const dev_color_t& color, uint32_t length = 1)
    {
      if (!length) return;
      if (     _bpp ==  8) { write_color(*(rgb332_t* )&color.raw, length); }
      else if (_bpp == 16) { write_color(*(swap565_t*)&color.raw, length); }
      else if (_bpp == 24) { write_color(*(swap888_t*)&color.raw, length); }
      else {
        uint8_t *dst = ptr_img();
        bool col = color;
        for (;;) {
          if (col) *dst |=  (0x80>>(_index & 7));
          else     *dst &= ~(0x80>>(_index & 7));
          if (0 == --length) return;
          if (ptr_advance()) dst = ptr_img();
          else if (!(_index & 0x07)) ++dst;
        }
      }
    }

    __attribute__ ((always_inline)) inline void writePixels(const rgb332_t*   src, uint32_t length) { writePixelsTemplate(src, length); }
    __attribute__ ((always_inline)) inline void writePixels(const rgb565_t*   src, uint32_t length) { writePixelsTemplate(src, length); }
    __attribute__ ((always_inline)) inline void writePixels(const rgb888_t*   src, uint32_t length) { writePixelsTemplate(src, length); }
    __attribute__ ((always_inline)) inline void writePixels(const swap565_t*  src, uint32_t length) { writePixelsTemplate(src, length); }
    __attribute__ ((always_inline)) inline void writePixels(const swap888_t*  src, uint32_t length) { writePixelsTemplate(src, length); }
    __attribute__ ((always_inline)) inline void writePixels(const argb8888_t* src, uint32_t length) { writePixelsTemplate(src, length); }

    inline rgb565_t readPixel(void)
    {
      const uint8_t *src = ptr_img();
      rgb565_t res;
      if (     _bpp ==  8) { res = *(( rgb332_t*)src); return res;}
      else if (_bpp == 16) { res = *((swap565_t*)src); return res;}
      else if (_bpp == 24) { res = *((swap888_t*)src); return res;}

      return (bool)(*src & (0x80 >> (_index & 0x07)));
    }

    __attribute__ ((always_inline)) inline void readPixels(rgb332_t*   buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline void readPixels(rgb565_t*   buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline void readPixels(rgb888_t*   buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline void readPixels(swap565_t*  buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline void readPixels(swap888_t*  buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline void readPixels(argb8888_t* buf, uint32_t length) { readPixelsTemplate(buf, length); }

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

    template <class T>
    void write_color(const T& color, uint32_t length)
    {
      auto *dst = (T*)ptr_img();
      while (length--) {
        *dst++ = color;
        if (ptr_advance()) dst = (T*)ptr_img();
      }
    }

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

    inline bool ptr_advance() {
      if (++_xptr > _xe) {
        _xptr = _xs;
        if (++_yptr > _ye) {
          _yptr = _ys;
        }
        _index = _xptr + _yptr * _bitwidth;
        return true;
      }
      ++_index;
      return false;
    }

    inline uint8_t* ptr_img() {
      return &_img[_bytes ? (_index * _bytes) : (_index >> 3)];
    }

    uint8_t* _img;
    uint8_t _bpp;
    uint8_t _bytes;
    uint8_t _invert;
    uint8_t _rotation;
    int32_t _bitwidth;
    int32_t _width;
    int32_t _height;
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
