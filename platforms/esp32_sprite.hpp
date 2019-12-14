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
    inline uint32_t getColorFromRGB(uint8_t r, uint8_t g, uint8_t b) { return (_bpp == 16) ? getColor565(r,g,b) : getColor888(r,g,b); }

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

    void* createSprite(int16_t w, int16_t h)
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

    inline uint16_t width(void) { return _width; }
    inline uint16_t height(void) { return _height; }
    inline uint8_t getRotation(void) { return _rotation; }
    inline uint8_t getColorDepth(void) { return _bpp; }

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

    inline void fillWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye, uint32_t color) {
      setWindow(xs, ys, xe, ye);
      writeColor(color, (xe-xs+1) * (ye-ys+1));
    }

    inline void readWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      setWindow(xs, ys, xe, ye);
    }

    inline static void startRead() {}
    inline static void endRead() {}

    void writeColor(uint32_t color, uint32_t length = 1)
    {
      if (!length) return;
      uint8_t *dst = ptr_img();
      if (_bytes) {
        uint8_t i, b = _bytes;
        for (;;) {
          for (i = 0; i < b; i++) { *dst++ = color >> (i << 3); }
          if (0 == --length) return;
          if (ptr_advance()) dst = ptr_img();
        }
      } else {
        for (;;) {
          if (color) *dst |=  (0x80>>(_index & 7));
          else       *dst &= ~(0x80>>(_index & 7));
          if (0 == --length) return;
          if (ptr_advance()) dst = ptr_img();
          else if (!(_index & 0x07)) ++dst;
        }
      }
    }

    // need data size = length * (24bpp=3 : 16bpp=2)
    void writePixels(const void* src, uint32_t length, bool swap)
    {
      if (!length) return;
      const uint8_t* s(static_cast<const uint8_t*>(src));
      uint8_t p = 0;
      uint8_t *dst = ptr_img();
      if (_bytes) {
        uint8_t i, b = _bytes;
        for (;;) {
          for (i = 0; i < b; i++) { *dst++ = *s++; }
          if (0 == --length) return;
          if (ptr_advance()) dst = ptr_img();
        }
      } else {
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

    uint32_t readPixel(void)
    {
      uint32_t res = 0;
      const uint8_t *src = ptr_img();
      if (_bytes) {
        for (uint32_t i = 0; i < _bytes; i++) {
          res |= (*src++) << (i<<3);
        }
        return res;
      }
      return *src & (0x80 >> (_index & 0x07));
    }

    void readPixels(uint8_t* buf, uint32_t length, bool swapBytes)
    {
      if (!length) return;
      uint8_t p = 0;
      const uint8_t *src = ptr_img();
      if (_bytes) {
        uint8_t i, b = _bytes;
        for (;;) {
          for (i = 0; i < b; i++) { *buf++ = *src++; }
          if (0 == --length) return;
          if (ptr_advance()) src = ptr_img();
        }
      } else {
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

    uint8_t* buffer() { return _img; }

  private:
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
    uint32_t _xptr;
    uint32_t _yptr;
    uint32_t _index;
    uint8_t _iptr;
    uint32_t _xs;
    uint32_t _xe;
    uint32_t _ys;
    uint32_t _ye;
    uint32_t _bitmap_fg;
    uint32_t _bitmap_bg;
    uint32_t _malloc_cap;
  };
}

#endif
