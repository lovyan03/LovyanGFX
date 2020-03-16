#ifndef LGFX_BASE_HPP_
#define LGFX_BASE_HPP_

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <cmath>
#include <cfloat>
#include <type_traits>
#include <algorithm>

#include "lgfx_common.hpp"

namespace lgfx
{
  #include "Fonts/glcdfont.h"

  #include "Fonts/Font16.h"
  #include "Fonts/Font32rle.h"
  #include "Fonts/Font64rle.h"
  #include "Fonts/Font7srle.h"
  #include "Fonts/Font72rle.h"

  const  uint8_t widtbl_null[1] = {0};
  PROGMEM const uint8_t chr_null[1] = {0};
  PROGMEM const uint8_t* const chrtbl_null[1] = {chr_null};

  enum font_type_t
  { ft_unknown
  , ft_glcd
  , ft_bmp
  , ft_rle
  };

  struct fontinfo {
    font_type_t type;
    const uint8_t *chartbl;
    const uint8_t *widthtbl;
    uint8_t height;
    uint8_t baseline;
  };

  const PROGMEM fontinfo fontdata [] = {
    { ft_unknown, (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },
    // GLCD font (Font 1) does not have all parameters
    { ft_glcd, (const uint8_t *)font, widtbl_null, 8, 6 },

    { ft_bmp,  (const uint8_t *)chrtbl_f16, widtbl_f16, chr_hgt_f16, baseline_f16},

    // Font 3 current unused
    { ft_unknown, (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },

    { ft_rle, (const uint8_t *)chrtbl_f32, widtbl_f32, chr_hgt_f32, baseline_f32},

    // Font 5 current unused
    { ft_unknown, (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },

    { ft_rle, (const uint8_t *)chrtbl_f64, widtbl_f64, chr_hgt_f64, baseline_f64},

    { ft_rle, (const uint8_t *)chrtbl_f7s, widtbl_f7s, chr_hgt_f7s, baseline_f7s},

    { ft_rle, (const uint8_t *)chrtbl_f72, widtbl_f72, chr_hgt_f72, baseline_f72}
  };

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
  class LGFXBase
  {
  public:
    LGFXBase() {}
    virtual ~LGFXBase() {}

// color param format:
// rgb888 : uint32_t
// rgb565 : int & uint16_t
// rgb332 : uint8_t
    template<typename T> __attribute__ ((always_inline)) inline void setColor(T c) { _color.raw = _write_conv.convert(c); }
                         __attribute__ ((always_inline)) inline void setColorRaw(uint32_t c) { _color.raw = c; }

                         inline void clear         ( void )          { _color.raw = 0;  fillRect(0, 0, _width, _height); }
    template<typename T> inline void clear         ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }
    template<typename T> inline void fillScreen    ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }

    template<typename T> inline void writeColor    ( const T& color, int32_t length) { if (0 >= length) return; setColor(color);               writeColor_impl(length);             }
                         inline void writeColorRaw ( uint32_t color, int32_t length) { if (0 >= length) return; setColorRaw(color);            writeColor_impl(length);             }
    template<typename T> inline void pushColor     ( const T& color, int32_t length) { if (0 >= length) return; setColor(color); startWrite(); writeColor_impl(length); endWrite(); }
    template<typename T> inline void pushColor     ( const T& color                ) {                          setColor(color); startWrite(); writeColor_impl(1);      endWrite(); }

    template<typename T> inline void drawPixel     ( int32_t x, int32_t y                                 , const T& color) { setColor(color); drawPixel    (x, y      ); }
    template<typename T> inline void drawFastVLine ( int32_t x, int32_t y           , int32_t h           , const T& color) { setColor(color); drawFastVLine(x, y   , h); }
    template<typename T> inline void drawFastHLine ( int32_t x, int32_t y, int32_t w                      , const T& color) { setColor(color); drawFastHLine(x, y, w   ); }
    template<typename T> inline void fillRect      ( int32_t x, int32_t y, int32_t w, int32_t h           , const T& color) { setColor(color); fillRect     (x, y, w, h); }
    template<typename T> inline void drawRect      ( int32_t x, int32_t y, int32_t w, int32_t h           , const T& color) { setColor(color); drawRect     (x, y, w, h); }
    template<typename T> inline void drawRoundRect ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, const T& color) { setColor(color); drawRoundRect(x, y, w, h, r); }
    template<typename T> inline void fillRoundRect ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, const T& color) { setColor(color); fillRoundRect(x, y, w, h, r); }
    template<typename T> inline void drawCircle    ( int32_t x, int32_t y                      , int32_t r, const T& color) { setColor(color); drawCircle(x, y, r   ); }
    template<typename T> inline void fillCircle    ( int32_t x, int32_t y                      , int32_t r, const T& color) { setColor(color); fillCircle(x, y, r   ); }
    template<typename T> inline void drawEllipse   ( int32_t x, int32_t y, int32_t rx, int32_t ry         , const T& color) { setColor(color); drawEllipse(x, y, rx, ry); }
    template<typename T> inline void fillEllipse   ( int32_t x, int32_t y, int32_t rx, int32_t ry         , const T& color) { setColor(color); fillEllipse(x, y, rx, ry); }
    template<typename T> inline void drawLine      ( int32_t x0, int32_t y0, int32_t x1, int32_t y1                        , const T& color)  { setColor(color); drawLine(    x0, y0, x1, y1        ); }
    template<typename T> inline void drawTriangle  ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); drawTriangle(x0, y0, x1, y1, x2, y2); }
    template<typename T> inline void fillTriangle  ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); fillTriangle(x0, y0, x1, y1, x2, y2); }

    __attribute__ ((always_inline)) inline static uint8_t  color332(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color332(r, g, b); }
    __attribute__ ((always_inline)) inline static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color565(r, g, b); }
    __attribute__ ((always_inline)) inline static uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color888(r, g, b); }

    __attribute__ ((always_inline)) inline void setPivot(int16_t x, int16_t y) { _xpivot = x; _ypivot = y; }
    __attribute__ ((always_inline)) inline int16_t getPivotX(void) const { return _xpivot; }
    __attribute__ ((always_inline)) inline int16_t getPivotY(void) const { return _ypivot; }

    __attribute__ ((always_inline)) inline int32_t width        (void) const { return _width; }
    __attribute__ ((always_inline)) inline int32_t height       (void) const { return _height; }
    __attribute__ ((always_inline)) inline bool getInvert       (void) const { return _invert; }
    __attribute__ ((always_inline)) inline bool getSwapBytes    (void) const { return _swapBytes; }
    __attribute__ ((always_inline)) inline uint8_t getRotation  (void) const { return _rotation; }
    __attribute__ ((always_inline)) inline uint8_t getTextFont  (void) const { return _textfont; }
    __attribute__ ((always_inline)) inline color_depth_t getColorDepth(void) const { return _write_conv.depth; }
    __attribute__ ((always_inline)) inline bool hasPalette    (void) const { return _palette_count; }
    __attribute__ ((always_inline)) inline bool hasTransaction(void) const { return _has_transaction; }
    __attribute__ ((always_inline)) inline void setSwapBytes(bool swap) { _swapBytes = swap; }

    __attribute__ ((always_inline)) inline void beginTransaction(void) { beginTransaction_impl(); }
    __attribute__ ((always_inline)) inline void endTransaction(void)   { endTransaction_impl(); }
    __attribute__ ((always_inline)) inline void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) { setWindow_impl(xs, ys, xe, ye); }

    __attribute__ ((always_inline)) inline void startWrite(void) { if (1 == ++_transaction_count) { beginTransaction(); } }
    __attribute__ ((always_inline)) inline void endWrite(void)   { if (_transaction_count) { if (0 == (--_transaction_count)) endTransaction(); } }

    void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      if (_adjust_abs(x, w)||_adjust_abs(y, h)) return;
      startWrite();
      setWindow(x, y, x + w - 1, y + h - 1);
      endWrite();
    }

    void setClipRect(int32_t x, int32_t y, int32_t w, int32_t h) {
      if (x < 0) { w += x; x = 0; }
      if (w > _width - x)  w = _width  - x;
      if (w < 1) { x = 0; w = 0; }
      _clip_l = x;
      _clip_r = x + w - 1;

      if (y < 0) { h += y; y = 0; }
      if (h > _height - y) h = _height - y;
      if (h < 1) { y = 0; h = 0; }
      _clip_t = y;
      _clip_b = y + h - 1;
    }

    void clearClipRect(void) {
      _clip_l = 0;
      _clip_t = 0;
      _clip_r = _width - 1;
      _clip_b = _height - 1;
    }

    void drawPixel(int32_t x, int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;

      drawPixel_impl(x, y);
    }

    void _drawPixel(int32_t x, int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;

      fillRect_impl(x, y, 1, 1);
    }

    void drawFastVLine(int32_t x, int32_t y, int32_t h)
    {
      _adjust_abs(y, h);
      startWrite();
      _drawFastVLine(x, y, h);
      endWrite();
    }

    void _drawFastVLine(int32_t x, int32_t y, int32_t h)
    {
      if (x < _clip_l || x > _clip_r) return;
      auto ct = _clip_t;
      if (0 > y - ct) { h += y - ct; y = ct; }
      auto cb = _clip_b + 1;
      if (h > (cb - y)) h = (cb - y);
      if (h < 1) return;

      fillRect_impl(x, y, 1, h);
    }

    void drawFastHLine(int32_t x, int32_t y, int32_t w)
    {
      _adjust_abs(x, w);
      startWrite();
      _drawFastHLine(x, y, w);
      endWrite();
    }

    void _drawFastHLine(int32_t x, int32_t y, int32_t w)
    {
      if (y < _clip_t || y > _clip_b) return;
      auto cl = _clip_l;
      if (0 > x - cl) { w += x - cl; x = cl; }
      auto cr = _clip_r + 1;
      if (w > (cr - x)) w = (cr - x);
      if (w < 1) return;

      fillRect_impl(x, y, w, 1);
    }

    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      _adjust_abs(x, w);
      _adjust_abs(y, h);
      startWrite();
      _fillRect(x, y, w, h);
      endWrite();
    }

    void _fillRect(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      auto cl = _clip_l;
      if (0 > x - cl) { w += x - cl; x = cl; }
      auto cr = _clip_r + 1;
      if (w > (cr - x)) w = (cr - x);
      if (w < 1) return;

      auto ct = _clip_t;
      if (0 > y - ct) { h += y - ct; y = ct; }
      auto cb = _clip_b + 1;
      if (h > (cb - y)) h = (cb - y);
      if (h < 1) return;

      fillRect_impl(x, y, w, h);
    }

    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      if (_adjust_abs(x, w)||_adjust_abs(y, h)) return;
      startWrite();
      _drawFastHLine(x, y        , w);
      if (--h) {
        _drawFastHLine(x, y + h    , w);
        if (--h) {
          _drawFastVLine(x        , ++y, h);
          _drawFastVLine(x + w - 1,   y, h);
        }
      }
      endWrite();
    }

    void drawCircle(int32_t x, int32_t y, int32_t r)
    {
      startWrite();

      int32_t dx = 1;
      int32_t p  = -(r >> 1);
      int32_t i = 0;
      while (p < 0) {
        dx+=2; p+=dx; i++;
      }
      int32_t dy = (r << 1) - 2;
      p -= dy;
      int32_t len = (i << 1) + 1;
      _drawFastHLine(x - i, y + r, len);
      _drawFastHLine(x - i, y - r, len);
      _drawFastVLine(x - r, y - i, len);
      _drawFastVLine(x + r, y - i, len);
      len = 0;
      for (r--; i <= r; i++) {
        if (p >= 0) {
          _drawFastHLine(x - i          , y + r, len);
          _drawFastHLine(x - i          , y - r, len);
          _drawFastHLine(x + i - len + 1, y + r, len);
          _drawFastHLine(x + i - len + 1, y - r, len);
          if (i == r && len == 1) break;
          _drawFastVLine(x - r, y - i          , len);
          _drawFastVLine(x + r, y - i          , len);
          dy -= 2;
          p -= dy;
          _drawFastVLine(x + r, y + i - len + 1, len);
          _drawFastVLine(x - r, y + i - len + 1, len);
          len = 0;
          r--;
        }
        len++;
        dx+=2;
        p+=dx;
      }

      endWrite();
    }

    void fillCircle(int32_t x, int32_t y, int32_t r)
    {
      startWrite();

      int32_t dx = 1;
      int32_t dy = r << 1;
      int32_t p  = -(r >> 1);
      int32_t len = 0;
      _drawFastHLine(x - r, y, dy+1);

      for (int32_t i  = 0; i <= r; i++) {
        len++;
        if (p >= 0) {
          _fillRect(x - r, y - i          , (r<<1) + 1, len);
          _fillRect(x - r, y + i - len + 1, (r<<1) + 1, len);
          if (i == r) break;
          dy -= 2;
          p -= dy;
          len = 0;
          _drawFastHLine(x - i, y + r, (i<<1) + 1);
          _drawFastHLine(x - i, y - r, (i<<1) + 1);
          r--;
        }
        dx+=2;
        p+=dx;
      }

      endWrite();
    }

    void drawEllipse(int32_t x0, int32_t y0, int32_t rx, int32_t ry)
    {
      if (rx<2) return;
      if (ry<2) return;
      int32_t x, y;
      int32_t rx2 = rx * rx;
      int32_t fx2 = rx2 << 2;
      int32_t ry2 = ry * ry;
      int32_t fy2 = ry2 << 2;
      int32_t s;

      startWrite();

      for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
        // These are ordered to minimise coordinate changes in x or y
        // drawPixel can then send fewer bounding box commands
        drawPixel(x0 + x, y0 + y);
        drawPixel(x0 - x, y0 + y);
        drawPixel(x0 - x, y0 - y);
        drawPixel(x0 + x, y0 - y);
        if (s >= 0) {
          s += fx2 * (1 - y);
          y--;
        }
        s += ry2 * ((4 * x) + 6);
      }

      for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
        // These are ordered to minimise coordinate changes in x or y
        // drawPixel can then send fewer bounding box commands
        drawPixel(x0 + x, y0 + y);
        drawPixel(x0 - x, y0 + y);
        drawPixel(x0 - x, y0 - y);
        drawPixel(x0 + x, y0 - y);
        if (s >= 0)
        {
          s += fy2 * (1 - x);
          x--;
        }
        s += rx2 * ((4 * y) + 6);
      }

      endWrite();
    }

    void fillEllipse(int32_t x0, int32_t y0, int32_t rx, int32_t ry)
    {
      if (rx<2) return;
      if (ry<2) return;
      int32_t x, y;
      int32_t rx2 = rx * rx;
      int32_t fx2 = rx2 << 2;
      int32_t ry2 = ry * ry;
      int32_t fy2 = ry2 << 2;
      int32_t s;

      startWrite();

      for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
        drawFastHLine(x0 - x, y0 - y, x + x + 1);
        drawFastHLine(x0 - x, y0 + y, x + x + 1);

        if (s >= 0) {
          s += fx2 * (1 - y);
          y--;
        }
        s += ry2 * ((x << 2) + 6);
      }

      for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
        drawFastHLine(x0 - x, y0 - y, x + x + 1);
        drawFastHLine(x0 - x, y0 + y, x + x + 1);

        if (s >= 0) {
          s += fy2 * (1 - x);
          x--;
        }
        s += rx2 * ((y << 2) + 6);
      }

      endWrite();
    }


    void drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r)
    {
      if (_adjust_abs(x, w)||_adjust_abs(y, h)) return; 
      startWrite();

      w--;
      h--;
      int32_t len = (r << 1) + 1;
      int32_t y1 = y + h - r;
      int32_t y0 = y + r;
      _drawFastVLine(x      , y0 + 1, h - len);
      _drawFastVLine(x + w  , y0 + 1, h - len);

      int32_t x1 = x + w - r;
      int32_t x0 = x + r;
      _drawFastHLine(x0 + 1, y      , w - len);
      _drawFastHLine(x0 + 1, y + h  , w - len);

      int32_t f     = 1 - r;
      int32_t ddF_y = -(r << 1);
      int32_t ddF_x = 1;

      len = 0;
      for (int32_t i = 0; i <= r; i++) {
        len++;
        if (f >= 0) {
          _drawFastHLine(x0 - i          , y0 - r, len);
          _drawFastHLine(x0 - i          , y1 + r, len);
          _drawFastHLine(x1 + i - len + 1, y1 + r, len);
          _drawFastHLine(x1 + i - len + 1, y0 - r, len);
          _drawFastVLine(x1 + r, y1 + i - len + 1, len);
          _drawFastVLine(x0 - r, y1 + i - len + 1, len);
          _drawFastVLine(x1 + r, y0 - i, len);
          _drawFastVLine(x0 - r, y0 - i, len);
          len = 0;
          r--;
          ddF_y += 2;
          f     += ddF_y;
        }
        ddF_x += 2;
        f     += ddF_x;
      }
      endWrite();
    }

    void fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r)
    {
      if (_adjust_abs(x, w)||_adjust_abs(y, h)) return; 
      startWrite();
      int32_t y2 = y + r;
      int32_t y1 = y + h - r - 1;
      int32_t ddF_y = - (r << 1);
      int32_t delta = w + ddF_y;
      _fillRect(x, y2, w, h + ddF_y);
      int32_t x0 = x + r;
      int32_t f     = 1 - r;
      int32_t ddF_x = 1;
      int32_t len = 0;
      for (int32_t i = 0; i <= r; i++) {
        len++;
        if (f >= 0) {
          _fillRect(x0 - r, y2 - i          , (r << 1) + delta, len);
          _fillRect(x0 - r, y1 + i - len + 1, (r << 1) + delta, len);
          if (i == r) break;
          len = 0;
          _drawFastHLine(x0 - i, y1 + r, (i << 1) + delta);
          ddF_y += 2;
          f     += ddF_y;
          _drawFastHLine(x0 - i, y2 - r, (i << 1) + delta);
          r--;
        }
        ddF_x += 2;
        f     += ddF_x;
      }
      endWrite();
    }

    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
    {
      bool steep = abs(y1 - y0) > abs(x1 - x0);

      if (steep) {   std::swap(x0, y0); std::swap(x1, y1); }
      if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }

      int32_t dy = abs(y1 - y0);
      int32_t ystep = (y1 > y0) ? 1 : -1;
      int32_t dx = x1 - x0;
      int32_t err = dx >> 1;

      int32_t xstart = steep ? _clip_t : _clip_l;
      int32_t ystart = steep ? _clip_l : _clip_t;
      int32_t yend   = steep ? _clip_r : _clip_b;
      while (x0 < xstart || y0 < ystart || y0 > yend) {
        err -= dy;
        if (err < 0) {
          err += dx;
          y0 += ystep;
        }
        if (++x0 > x1) return;
      }
      int32_t xs = x0;
      int32_t dlen = 0;

      startWrite();
      if (steep) {
        if (x1 > (_clip_b)) x1 = (_clip_b);
        for (; x0 <= x1; x0++) {
          dlen++;
          if ((err -= dy) < 0) {
            fillRect_impl(y0, xs, 1, dlen);
            err += dx;
            xs = x0 + 1; dlen = 0; y0 += ystep;
            if ((y0 < _clip_l) || (y0 > _clip_r)) break;
          }
        }
        if (dlen) fillRect_impl(y0, xs, 1, dlen);
      } else {
        if (x1 > (_clip_r)) x1 = (_clip_r);
        for (; x0 <= x1; x0++) {
          dlen++;
          if ((err -= dy) < 0) {
            fillRect_impl(xs, y0, dlen, 1);
            err += dx;
            xs = x0 + 1; dlen = 0; y0 += ystep;
            if ((y0 < _clip_t) || (y0 > _clip_b)) break;
          }
        }
        if (dlen) fillRect_impl(xs, y0, dlen, 1);
      }
      endWrite();
    }

    void drawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
    {
      startWrite();
      drawLine(x0, y0, x1, y1);
      drawLine(x1, y1, x2, y2);
      drawLine(x2, y2, x0, y0);
      endWrite();
    }

    void fillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
    {
      int32_t a, b, y, last;

      // Sort coordinates by Y order (y2 >= y1 >= y0)
      if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
      if (y1 > y2) { std::swap(y2, y1); std::swap(x2, x1); }
      if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }

      if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)      a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a)      a = x2;
        else if (x2 > b) b = x2;
        _drawFastHLine(a, y0, b - a + 1);
        return;
      }

      int32_t
      dx01 = x1 - x0,
      dy01 = y1 - y0,
      dx02 = x2 - x0,
      dy02 = y2 - y0,
      dx12 = x2 - x1,
      dy12 = y2 - y1,
      sa   = 0,
      sb   = 0;

      // For upper part of triangle, find scanline crossings for segments
      // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
      // is included here (and second loop will be skipped, avoiding a /0
      // error there), otherwise scanline y1 is skipped here and handled
      // in the second loop...which also avoids a /0 error here if y0=y1
      // (flat-topped triangle).
      if (y1 == y2) last = y1;  // Include y1 scanline
      else         last = y1 - 1; // Skip it

      startWrite();
      for (y = y0; y <= last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;

        if (a > b) std::swap(a, b);
        _drawFastHLine(a, y, b - a + 1);
      }

      // For lower part of triangle, find scanline crossings for segments
      // 0-2 and 1-2.  This loop is skipped if y1=y2.
      sa = dx12 * (y - y1);
      sb = dx02 * (y - y0);
      for (; y <= y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;

        if (a > b) std::swap(a, b);
        _drawFastHLine(a, y, b - a + 1);
      }
      endWrite();
    }

    __attribute__ ((always_inline)) inline
    void drawGradientHLine( int32_t x, int32_t y, int32_t w, bgr888_t colorstart, bgr888_t colorend ) {
      drawGradientLine( x, y, x + w - 1, y, colorstart, colorend );
    }

    __attribute__ ((always_inline)) inline
    void drawGradientVLine( int32_t x, int32_t y, int32_t h, bgr888_t colorstart, bgr888_t colorend ) {
      drawGradientLine( x, y, x, y + h - 1, colorstart, colorend );
    }

    void drawGradientLine( int32_t x0, int32_t y0, int32_t x1, int32_t y1, bgr888_t colorstart, bgr888_t colorend )
    {
      if ( colorstart == colorend || (x0 == x1 && y0 == y1)) {
        setColor(color888( colorstart.r, colorstart.g, colorstart.b ) );
        drawLine( x0, y0, x1, y1);
        return;
      }

      bool steep = abs(y1 - y0) > abs(x1 - x0);
      if (steep) { // swap axis
        std::swap(x0, y0);
        std::swap(x1, y1);
      }

      if (x0 > x1) { // swap points
        std::swap(x0, x1);
        std::swap(y0, y1);
        std::swap(colorstart, colorend);
      }

      int32_t dx = x1 - x0;
      int32_t err = dx >> 1;
      int32_t dy = abs(y1 - y0);
      int32_t ystep = (y0 < y1) ? 1 : -1;

      int32_t diff_r = colorend.r - colorstart.r;
      int32_t diff_g = colorend.g - colorstart.g;
      int32_t diff_b = colorend.b - colorstart.b;

      startWrite();
      for (int32_t x = x0; x <= x1; x++) {
        setColor(color888( (x - x0) * diff_r / dx + colorstart.r
                         , (x - x0) * diff_g / dx + colorstart.g
                         , (x - x0) * diff_b / dx + colorstart.b));
        if (steep) _drawPixel(y0, x);
        else       _drawPixel(x, y0);
        err -= dy;
        if (err < 0) {
          err += dx;
          y0 += ystep;
        }
      }
      endWrite();
    }


    void pushColors(const uint8_t* data, int32_t len)
    {
      pushColors((rgb332_t*)data, len);
    }
    void pushColors(const uint16_t* data, int32_t len, bool swap = true)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb565_2Byte, _palette_count);
      if (swap && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb565_t>(_write_conv.depth);
      }
      startWrite();
      pushColors_impl(len, &p);
      endWrite();
    }
    void pushColors(const void* data, int32_t len, bool swap = true)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb888_3Byte, _palette_count);
      if (swap && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb888_t>(_write_conv.depth);
      }
      startWrite();
      pushColors_impl(len, &p);
      endWrite();
    }
    template <typename T>
    void pushColors(const T *src, int32_t len)
    {
      pixelcopy_t p(src, _write_conv.depth, T::depth, _palette_count);
      startWrite();
      pushColors_impl(len, &p);
      endWrite();
    }

    uint16_t readPixel(int32_t x, int32_t y)
    {
      pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, _palette);
      uint16_t data = 0;
      read_rect(x, y, 1, 1, &data, &p);
      return __builtin_bswap16(data);
    }

    template<typename T> inline
    void readRect( int32_t x, int32_t y, int32_t w, int32_t h, T* data)
    {
      pixelcopy_t p(nullptr, T::depth, _read_conv.depth, false, _palette);
      read_rect(x, y, w, h, data, &p);
    }

    void readRectRGB( int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, _palette);
      read_rect(x, y, w, h, data, &p);
    }

    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* data)
    {
      pixelcopy_t p(nullptr, rgb332_t::depth, _read_conv.depth, false, _palette);
      read_rect(x, y, w, h, data, &p);
    }
    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data)
    {
      pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, _palette);
//*
      if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<rgb565_t>(_read_conv.depth);
      }
//*/
      read_rect(x, y, w, h, data, &p);
    }
    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, void* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, _palette);
//*
      if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<rgb888_t>(_read_conv.depth);
      }
//*/
      read_rect(x, y, w, h, data, &p);
    }

    template<typename T> void pushRect(  int32_t x, int32_t y, int32_t w, int32_t h, const T* data) { pushImage(x, y, w, h, data); }
    template<typename T> void pushImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data)                          { pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr                                   ); push_image(x, y, w, h, &p); }
    template<typename T> void pushImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data   , const T& transparent) { pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr, _write_conv.convert(transparent)); push_image(x, y, w, h, &p); }
    template<typename T> void pushImage( int32_t x, int32_t y, int32_t w, int32_t h, const bgr888_t* data                      , const uint8_t bits, const T* palette) { pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette              ); push_image(x, y, w, h, &p); }
    template<typename T> void pushImage( int32_t x, int32_t y, int32_t w, int32_t h, const bgr888_t* data, uint32_t transparent, const uint8_t bits, const T* palette) { pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette, transparent ); push_image(x, y, w, h, &p); }

    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* data, uint32_t transp = ~0)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb565_2Byte, _palette_count, nullptr, transp == ~0 ? ~0 : _write_conv.convert((uint16_t)transp));
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb565_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p);
    }
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, uint32_t transp = ~0)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb888_3Byte, _palette_count, nullptr, transp == ~0 ? ~0 : _write_conv.convert(transp));
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb888_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p);
    }

    template<typename T> void pushImageDMA( int32_t x, int32_t y, int32_t w, int32_t h, const T* data)                          { pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr                                   ); push_image(x, y, w, h, &p, true); }
    template<typename T> void pushImageDMA( int32_t x, int32_t y, int32_t w, int32_t h, const T* data   , const T& transparent) { pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr, _write_conv.convert(transparent)); push_image(x, y, w, h, &p, true); }
    template<typename T> void pushImageDMA( int32_t x, int32_t y, int32_t w, int32_t h, const bgr888_t* data                      , const uint8_t bits, const T* palette) { pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette              ); push_image(x, y, w, h, &p, true); }
    template<typename T> void pushImageDMA( int32_t x, int32_t y, int32_t w, int32_t h, const bgr888_t* data, uint32_t transparent, const uint8_t bits, const T* palette) { pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette, transparent ); push_image(x, y, w, h, &p, true); }

    void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* data, uint32_t transp = ~0)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb565_2Byte, _palette_count, nullptr, transp == ~0 ? ~0 : _write_conv.convert((uint16_t)transp));
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb565_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p, true);
    }
    void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, uint32_t transp = ~0)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb888_3Byte, _palette_count, nullptr, transp == ~0 ? ~0 : _write_conv.convert(transp));
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb888_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p, true);
    }

    void push_image(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t *param, bool use_dma = false)
    {
      param->src_width = w;
      if (param->src_bits < 8) {        // get bitwidth
        uint32_t x_mask = (1 << ((~(param->src_bits>>1)) & 3)) - 1;
        param->src_width = (w + x_mask) & (~x_mask);
      }

      int32_t dx=0, dw=w;
      if (0 < _clip_l - x) { dx = _clip_l - x; dw -= dx; x = _clip_l; }

      if (_adjust_width(x, dx, dw, _clip_l, _clip_r - _clip_l + 1)) return;
      param->src_x = dx;


      int32_t dy=0, dh=h;
      if (0 < _clip_t - y) { dy = _clip_t - y; dh -= dy; y = _clip_t; }
      if (_adjust_width(y, dy, dh, _clip_t, _clip_b - _clip_t + 1)) return;
      param->src_y = dy;

      startWrite();
      pushImage_impl(x, y, dw, dh, param, use_dma);
      endWrite();
    }

    void pushImageRotateZoom(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y, float angle, float zoom_x, float zoom_y, pixelcopy_t *param)
    {
      if (zoom_x == 0.0 || zoom_y == 0.0) return;

      angle *= - 0.0174532925; // Convert degrees to radians
      float sin_f = sin(angle) * (1 << FP_SCALE);
      float cos_f = cos(angle) * (1 << FP_SCALE);
      int32_t min_y, max_y;
      {
        int32_t sinra = round(sin_f * zoom_x);
        int32_t cosra = round(cos_f * zoom_y);
        int32_t wp = (src_x - w + 1);
        int32_t hp = h - src_y - 1;
        int32_t tmp;
        if ((sinra < 0) == (cosra < 0)) {
          min_y = max_y =    wp * sinra - src_y * cosra;
          tmp           = src_x * sinra +    hp * cosra;
        } else {
          min_y = max_y = src_x * sinra - src_y * cosra;
          tmp           =    wp * sinra +    hp * cosra;
        }
        if (tmp < min_y) {
          min_y = tmp;
        } else {
          max_y = tmp;
        }
      }
      max_y = std::min(_clip_b, ((max_y + (1 << (FP_SCALE - 1))) >> FP_SCALE) + dst_y) + 1;
      min_y = std::max(_clip_t, ((min_y + (1 << (FP_SCALE - 1))) >> FP_SCALE) + dst_y);
      if (min_y >= max_y) return;

      param->no_convert = false;
      if (param->src_bits < 8) {        // get bitwidth
// if (bits==1) { shift=3 }
// if (bits==2) { shift=2 }
// if (bits==4) { shift=1 }
        uint8_t shift = 3 & (~param->src_bits >> 1);
        uint32_t x_mask = (1 << shift) - 1;
        param->src_width = (w + x_mask) & (~x_mask);
      } else {
        param->src_width = w;
      }

      int32_t xt =       - dst_x;
      int32_t yt = min_y - dst_y - 1;

      int32_t cos_x = round(cos_f / zoom_x);
      param->src_x32_add = cos_x;
      int32_t sin_x = - round(sin_f / zoom_x);
      int32_t xstart = cos_x * xt + sin_x * yt + (src_x << FP_SCALE) + (1 << (FP_SCALE - 1));
      int32_t scale_w = w << FP_SCALE;
      int32_t xs1 = (cos_x < 0 ?   - scale_w :   1) - cos_x;
      int32_t xs2 = (cos_x < 0 ? 0 : (1 - scale_w)) - cos_x;
      if (cos_x == 0) cos_x = 1;
      cos_x = -cos_x;

      int32_t sin_y = round(sin_f / zoom_y);
      param->src_y32_add = sin_y;
      int32_t cos_y = round(cos_f / zoom_y);
      int32_t ystart = sin_y * xt + cos_y * yt + (src_y << FP_SCALE) + (1 << (FP_SCALE - 1));
      int32_t scale_h = h << FP_SCALE;
      int32_t ys1 = (sin_y < 0 ?   - scale_h :   1) - sin_y;
      int32_t ys2 = (sin_y < 0 ? 0 : (1 - scale_h)) - sin_y;
      if (sin_y == 0) sin_y = 1;
      sin_y = -sin_y;

      int32_t cl = _clip_l;
      int32_t cr = _clip_r + 1;

      startWrite();
      do {
        int32_t left = cl;
        int32_t right = cr;
        xstart += sin_x;
        //if (cos_x != 0)
        {
          int32_t tmp = (xstart + xs1) / cos_x; if (left  < tmp) left  = tmp;
                  tmp = (xstart + xs2) / cos_x; if (right > tmp) right = tmp;
        }
        ystart += cos_y;
        //if (sin_y != 0)
        {
          int32_t tmp = (ystart + ys1) / sin_y; if (left  < tmp) left  = tmp;
                  tmp = (ystart + ys2) / sin_y; if (right > tmp) right = tmp;
        }
        if (left < right) {
          param->src_x32 = xstart - left * cos_x;
          param->src_y32 = ystart - left * sin_y;
          pushImage_impl(left, min_y, right - left, 1, param, true);
        }
      } while (++min_y != max_y);
      endWrite();
    }

    template <typename T>
    void setScrollRect(int32_t x, int32_t y, int32_t w, int32_t h, const T& color) {
      _scolor = _write_conv.convert(color);
      setScrollRect(x, y, w, h);
    }
    void setScrollRect(int32_t x, int32_t y, int32_t w, int32_t h) {
      _adjust_abs(x, w);
      if (x < 0) { w += x; x = 0; }
      if (w > _width - x)  w = _width  - x;
      if (w < 0) w = 0;
      _sx = x;
      _sw = w;

      _adjust_abs(y, h);
      if (y < 0) { h += y; y = 0; }
      if (h > _height - y) h = _height - y;
      if (h < 0) h = 0;
      _sy = y;
      _sh = h;
    }

    void scroll(int16_t dx, int16_t dy)
    {
      _color.raw = _scolor;
      int32_t absx = abs(dx);
      int32_t absy = abs(dy);
      if (absx >= _sw || absy >= _sh) {
        _fillRect(_sx, _sy, _sw, _sh);
        return;
      }

      int32_t w  = _sw - absx;
      int32_t h  = _sh - absy;

      int32_t src_x = dx < 0 ? _sx - dx : _sx;
      int32_t dst_x = src_x + dx;
      int32_t src_y = dy < 0 ? _sy - dy : _sy;
      int32_t dst_y = src_y + dy;

      startWrite();
      copyRect_impl(dst_x, dst_y, w, h, src_x, src_y);

      if (     dx > 0) _fillRect(_sx           , dst_y,  dx, h);
      else if (dx < 0) _fillRect(_sx + _sw + dx, dst_y, -dx, h);
      if (     dy > 0) _fillRect(_sx, _sy           , _sw,  dy);
      else if (dy < 0) _fillRect(_sx, _sy + _sh + dy, _sw, -dy);
      endWrite();
    }

    void copyRect(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y)
    {
      if (src_x < dst_x) { if (src_x < 0) { w += src_x; dst_x -= src_x; src_x = 0; } if (w > _width  - dst_x)  w = _width  - dst_x; }
      else               { if (dst_x < 0) { w += dst_x; src_x -= dst_x; dst_x = 0; } if (w > _width  - src_x)  w = _width  - src_x; }
      if (w < 1) return;

      if (src_y < dst_y) { if (src_y < 0) { h += src_y; dst_y -= src_y; src_y = 0; } if (h > _height - dst_y)  h = _height - dst_y; }
      else               { if (dst_y < 0) { h += dst_y; src_y -= dst_y; dst_y = 0; } if (h > _height - src_y)  h = _height - src_y; }
      if (h < 1) return;

      startWrite();
      copyRect_impl(dst_x, dst_y, w, h, src_x, src_y);
      endWrite();
    }

//----------------------------------------------------------------------------
// print & text support
//----------------------------------------------------------------------------
// Arduino Print.h compatible
    size_t print(const char str[])      { return write(str); }
    size_t print(char c)                { return write(c); }
    size_t print(int  n, int base = 10) { return print((long)n, base); }
    size_t print(long n, int base = 10)
    {
      if (base == 0) { return write(n); }
      if (base == 10) {
        if (n < 0) {
          size_t t = print('-');
          return printNumber(-n, 10) + t;
        }
        return printNumber(n, 10);
      }
      return printNumber(n, base);
    }

    size_t print(unsigned char n, int base = 10) { return print((unsigned long)n, base); }
    size_t print(unsigned int  n, int base = 10) { return print((unsigned long)n, base); }
    size_t print(unsigned long n, int base = 10) { return (base) ? printNumber(n, base) : write(n); }
    size_t print(double        n, int digits= 2) { return printFloat(n, digits); }

    size_t println(void) { return print("\r\n"); }
    size_t println(const char c[])                 { size_t t = print(c); return println() + t; }
    size_t println(char c        )                 { size_t t = print(c); return println() + t; }
    size_t println(int  n, int base = 10)          { size_t t = print(n,base); return println() + t; }
    size_t println(long n, int base = 10)          { size_t t = print(n,base); return println() + t; }
    size_t println(unsigned char n, int base = 10) { size_t t = print(n,base); return println() + t; }
    size_t println(unsigned int  n, int base = 10) { size_t t = print(n,base); return println() + t; }
    size_t println(unsigned long n, int base = 10) { size_t t = print(n,base); return println() + t; }
    size_t println(double        n, int digits= 2) { size_t t = print(n, digits); return println() + t; }

  #if defined (ARDUINO)
    size_t print(const String &s) { return write(s.c_str(), s.length()); }
    size_t print(const __FlashStringHelper *s)   { return print(reinterpret_cast<const char *>(s)); }
    size_t println(const String &s)              { size_t t = print(s); return println() + t; }
    size_t println(const __FlashStringHelper *s) { size_t t = print(s); return println() + t; }
  #endif

    size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)))
    {
      char loc_buf[64];
      char * temp = loc_buf;
      va_list arg;
      va_list copy;
      va_start(arg, format);
      va_copy(copy, arg);
      size_t len = vsnprintf(temp, sizeof(loc_buf), format, copy);
      va_end(copy);

      if (len >= sizeof(loc_buf)){
        temp = (char*) malloc(len+1);
        if (temp == nullptr) {
          va_end(arg);
          return 0;
        }
        len = vsnprintf(temp, len+1, format, arg);
      }
      va_end(arg);
      len = write((uint8_t*)temp, len);
      if (temp != loc_buf){
        free(temp);
      }
      return len;
    }

    size_t write(const char* str)                 { return (!str) ? 0 : write((const uint8_t*)str, strlen(str)); }
    size_t write(const char *buf, size_t size)    { return write((const uint8_t *) buf, size); }
    size_t write(const uint8_t *buf, size_t size) { size_t n = 0; startWrite(); while (size--) { n += write(*buf++); } endWrite(); return n; }
    size_t write(uint8_t utf8)
    {
      if (utf8 == '\r') return 1;
      if (utf8 == '\n') {
        _filled_x = 0;
        _cursor_x = 0;
        _cursor_y += _font_size_y.advance * _textsize_y;
      } else {
        uint16_t uniCode = utf8;
        if (_decoderState != utf8_decode_state_t::utf8_no_use) {
          uniCode = decodeUTF8(utf8);
          if (uniCode == 0) return 1;
//        if (uniCode < 32) return 1;
        }
        if (fpUpdateFontSize && !(fpUpdateFontSize)(this, uniCode)) return 1;

        if (0 == _font_size_x.size) return 1;

        int16_t w  = _font_size_x.size    * _textsize_x;
        int16_t xo = _font_size_x.offset  * _textsize_x;
        int16_t h  = _font_size_y.size    * _textsize_y;
        int16_t yo = _font_size_y.offset  * _textsize_y;
        if (_textscroll || _textwrapX) {
          int32_t left = _textscroll ? _sx : 0;
          if (_cursor_x < left - xo) _cursor_x = left - xo;
          else {
            int32_t right = _textscroll ? _sx + _sw : _width;
            if (_cursor_x + xo + w > right) {
              _cursor_x = left - xo;
              _cursor_y += _font_size_y.advance * _textsize_y;
            }
          }
        }
        if (_textscroll) {
          if (_cursor_y < _sy - yo) _cursor_y = _sy - yo;
          else {
            int yshift = (_sy + _sh) - (_cursor_y + yo + h);
            if (yshift < 0) {
              scroll(0, yshift);
              _cursor_y += yshift;
            }
          }
        } else if (_textwrapY) {
          if (_cursor_y + yo + h > _height) {
            _filled_x = 0;
            _cursor_x = - xo;
            _cursor_y = - yo;
          } else
          if (_cursor_y < - yo) _cursor_y = - yo;
        }
//      _cursor_x += drawChar(uniCode, _cursor_x, _cursor_y);
        _cursor_x += (fpDrawChar)(this, _cursor_x, _cursor_y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y);
      }

      return 1;
    }

    int32_t textWidth(const char *string)
    {
      if (!string) return 0;

      int32_t left = 0;
      int32_t right = 0;
      do {
        uint16_t uniCode = *string;
        if (_decoderState != utf8_decode_state_t::utf8_no_use) {
          uniCode = decodeUTF8(uniCode);
          if (uniCode == 0) continue;
        }
        if (fpUpdateFontSize && !(fpUpdateFontSize)(this, uniCode)) continue;
        if (left == 0 && right == 0 && _font_size_x.offset < 0) left = right = -_font_size_x.offset;
        right = left + std::max((int32_t)_font_size_x.advance, _font_size_x.size - _font_size_x.offset);
        left += _font_size_x.advance;
      } while (*(++string));

      return right * _textsize_x;
    }



    __attribute__ ((always_inline)) inline int16_t drawString(const char *string, int32_t x, int32_t y)
    {
      return drawString(string, x, y, _textdatum);
    }
    int16_t drawString(const char *string, int32_t x, int32_t y, textdatum_t datum)
    {
      int16_t sumX = 0;
      int32_t cwidth = textWidth(string); // Find the pixel width of the string in the font
      int32_t cheight = _font_size_y.size * _textsize_y;

      if (fpUpdateFontSize) {
        uint16_t uniCode = *string;
        if (_decoderState != utf8_decode_state_t::utf8_no_use) {
          auto tmp = string;
          do {
            uniCode = decodeUTF8(*tmp++);
          } while (uniCode == 0 && *tmp);
        }
        if ((fpUpdateFontSize)(this, uniCode)) {
          if (_font_size_x.offset < 0) sumX = - _font_size_x.offset * _textsize_x;
        }
      }

      if (datum & middle_left) {          // vertical: middle
        y -= cheight >> 1;
      } else if (datum & bottom_left) {   // vertical: bottom
        y -= cheight;
      } else if (datum & baseline_left) { // vertical: baseline
        y -= _font_baseline * _textsize_y;
      }

      startWrite();
      int32_t padx = _padding_x;
      if ((_text_fore_rgb888 != _text_back_rgb888) && (padx > cwidth)) {
        setColor(_text_back_rgb888);
        if (datum & top_center) {
          auto halfcwidth = cwidth >> 1;
          auto halfpadx = (padx >> 1);
          _fillRect(x - halfpadx, y, halfpadx - halfcwidth, cheight);
          halfcwidth = cwidth - halfcwidth;
          halfpadx = padx - halfpadx;
          _fillRect(x + halfcwidth, y, halfpadx - halfcwidth, cheight);
        } else if (datum & top_right) {
          _fillRect(x - padx, y, padx - cwidth, cheight);
        } else {
          _fillRect(x + cwidth, y, padx - cwidth, cheight);
        }
      }

      if (datum & top_center) {           // Horizontal: middle
        x -= cwidth >> 1;
      } else if (datum & top_right) {     // Horizontal: right
        x -= cwidth;
      }

      y -= _font_size_y.offset * _textsize_y;

      _filled_x = 0;
      do {
        uint16_t uniCode = *string;
        if (_decoderState != utf8_decode_state_t::utf8_no_use) {
          uniCode = decodeUTF8(uniCode);
          if (uniCode == 0) continue;
        }

        sumX += (fpDrawChar)(this, x + sumX, y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y);
      } while (*(++string));
      endWrite();

      return sumX;
    }

    int16_t drawNumber(long long_num, int32_t poX, int32_t poY)
    {
      constexpr size_t len = 8 * sizeof(long) + 1;
      char buf[len];
      return drawString(numberToStr(long_num, buf, len, 10), poX, poY);
    }

    int16_t drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY)
    {
      size_t len = 14 + dp;
      char buf[len];
      return drawString(floatToStr(floatNumber, buf, len, dp), poX, poY);
    }

    uint16_t decodeUTF8(uint8_t c)
    {
      // 7 bit Unicode Code Point
      if (!(c & 0x80)) {
        _decoderState = utf8_decode_state_t::utf8_state0;
        return c;
      }

      if (_decoderState == utf8_decode_state_t::utf8_state0)
      {
        // 11 bit Unicode Code Point
        if ((c & 0xE0) == 0xC0)
        {
          _decoderBuffer = ((c & 0x1F)<<6);
          _decoderState = utf8_decode_state_t::utf8_state1;
          return 0;
        }

        // 16 bit Unicode Code Point
        if ((c & 0xF0) == 0xE0)
        {
          _decoderBuffer = ((c & 0x0F)<<12);
          _decoderState = utf8_decode_state_t::utf8_state2;
          return 0;
        }
        // 21 bit Unicode  Code Point not supported so fall-back to extended ASCII
        //if ((c & 0xF8) == 0xF0) return (uint16_t)c;
      }
      else
      {
        if (_decoderState == utf8_decode_state_t::utf8_state2)
        {
          _decoderBuffer |= ((c & 0x3F)<<6);
          _decoderState = utf8_decode_state_t::utf8_state1;
          return 0;
        }
        _decoderBuffer |= (c & 0x3F);
        _decoderState = utf8_decode_state_t::utf8_state0;
        return _decoderBuffer;
      }

      _decoderState = utf8_decode_state_t::utf8_state0;

      return (uint16_t)c; // fall-back to extended ASCII
    }
/*
    inline int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y) { return drawChar(uniCode, x, y, _textfont); }
    inline int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font)
    {
      return drawChar(x, y, uniCode, _textcolor, _textbgcolor, _textsize_x, _textsize_y, font);
    }
*/
    template<typename T> inline void setTextColor(T c)      { _text_fore_rgb888 = _text_back_rgb888 = convert_to_rgb888(c); }
    template<typename T> inline void setTextColor(T c, T b) { _text_fore_rgb888 = convert_to_rgb888(c); _text_back_rgb888 = convert_to_rgb888(b); }

    inline int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y); }
    template<typename T>
    inline int16_t drawChar(int32_t x, int32_t y, uint16_t uniCode, T color, T bg, uint8_t size) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, convert_to_rgb888(color), convert_to_rgb888(bg), size, size); }
    template<typename T>
    inline int16_t drawChar(int32_t x, int32_t y, uint16_t uniCode, T color, T bg, uint8_t size_x, uint8_t size_y) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, convert_to_rgb888(color), convert_to_rgb888(bg), size_x, size_y); }

    int16_t getCursorX(void) const { return _cursor_x; }
    int16_t getCursorY(void) const { return _cursor_y; }
    void setCursor( int16_t x, int16_t y)               { _filled_x = 0; _cursor_x = x; _cursor_y = y; }
    void setCursor( int16_t x, int16_t y, uint8_t font) { _filled_x = 0; _cursor_x = x; _cursor_y = y; _textfont = font; }
    void setTextSize(uint8_t s) { setTextSize(s,s); }
    void setTextSize(uint8_t sx, uint8_t sy) { _textsize_x = (sx > 0) ? sx : 1; _textsize_y = (sy > 0) ? sy : 1; }
    int16_t getTextSizeX(void) const { return _textsize_x; }
    int16_t getTextSizeY(void) const { return _textsize_y; }
    int16_t fontHeight(void) const { return _font_size_y.size * _textsize_y; }

    void setTextDatum(uint8_t datum) { _textdatum = (textdatum_t)datum; }
    void setTextDatum(textdatum_t datum) { _textdatum = datum; }
    void setTextPadding(uint16_t padding_x) { _padding_x = padding_x; }
    void setTextWrap( bool wrapX, bool wrapY = false) { _textwrapX = wrapX; _textwrapY = wrapY; }
    void setTextScroll(bool scroll) { _textscroll = scroll; if (_cursor_x < _sx) { _cursor_x = _sx; } if (_cursor_y < _sy) { _cursor_y = _sy; } }


    virtual void setTextFont(uint8_t f) {
      _decoderState = utf8_decode_state_t::utf8_no_use;
      _filled_x = 0; 
      _font_size_x.offset = 0;
      _font_size_y.offset = 0;
      if (f == 0) f = 1;
      _textfont = f;
      _font_size_y.size = _font_size_y.advance = pgm_read_byte( &fontdata[f].height );
      _font_baseline = pgm_read_byte( &fontdata[f].baseline );

      switch (pgm_read_byte( &fontdata[f].type)) {
      default:
      case font_type_t::ft_glcd:
        fpDrawChar = drawCharGLCD;
        fpUpdateFontSize = nullptr; // updateFontSizeGLCD;
        _font_size_x.size = _font_size_x.advance = 6;
        break;
      case font_type_t::ft_bmp:
        fpDrawChar = drawCharBMP;
        fpUpdateFontSize = updateFontSizeBMP;
        break;
      case font_type_t::ft_rle:
        fpDrawChar = drawCharRLE;
        fpUpdateFontSize = updateFontSizeBMP;
        break;
      }
    }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    uint32_t _transaction_count = 0;
    int32_t _width = 0, _height = 0;
    int32_t  _sx, _sy, _sw, _sh; // for scroll zone
    int32_t  _clip_l = 0, _clip_t = 0, _clip_r = 0, _clip_b = 0; // clip rect
    int32_t _cursor_x = 0, _cursor_y = 0, _filled_x = 0;
    uint32_t _text_fore_rgb888 = 0xFFFFFFU;
    uint32_t _text_back_rgb888 = 0;
    uint32_t _scolor;  // gap fill colour for scroll zone
    raw_color_t _color = 0xFFFFFFU;

    color_conv_t _write_conv;
    color_conv_t _read_conv;

    bgr888_t* _palette = nullptr; // for sprite palette mode.

    int16_t _textsize_x = 1;
    int16_t _textsize_y = 1;
    int16_t _xpivot;   // x pivot point coordinate
    int16_t _ypivot;   // x pivot point coordinate
    uint16_t _decoderBuffer = 0;   // Unicode code-point buffer
    uint8_t _rotation = 0;
    uint8_t _textfont = 1;
    textdatum_t _textdatum;
    int16_t _padding_x = 0;

    uint32_t _palette_count = 0;
    bool _has_transaction = true;
    bool _invert = false;
    bool _swapBytes = false;
    bool _textwrapX = true;
    bool _textwrapY = false;
    bool _textscroll = false;

    enum utf8_decode_state_t
    { utf8_state0 = 0
    , utf8_state1 = 1
    , utf8_state2 = 2
    , utf8_no_use = 3
    };
    utf8_decode_state_t _decoderState = utf8_no_use;   // UTF8 decoder state

    struct font_size_t {
      int16_t size;
      int16_t advance;
      int16_t offset;
    };
    font_size_t _font_size_x = { 6, 6, 0 };
    font_size_t _font_size_y = { 8, 8, 0 };
    int8_t _font_baseline = 6;

    __attribute__ ((always_inline)) inline static bool _adjust_abs(int32_t& x, int32_t& w) { if (w < 0) { x += w + 1; w = -w; } return !w; }

    static bool _adjust_width(int32_t& x, int32_t& dx, int32_t& dw, int32_t left, int32_t width)
    {
      if (x < left) { dx = -x; dw += x; x = left; }
      if (dw > left + width - x) dw = left + width  - x;
      return (dw <= 0);
    }

    void read_rect(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param)
    {
      _adjust_abs(x, w);
      if (x < 0) { w += x; x = 0; }
      if (w > _width - x)  w = _width  - x;
      if (w < 1) return;

      _adjust_abs(y, h);
      if (y < 0) { h += y; y = 0; }
      if (h > _height - y) h = _height - y;
      if (h < 1) return;

      startWrite();
      readRect_impl(x, y, w, h, dst, param);
      endWrite();
    }

    virtual void beginTransaction_impl(void) = 0;
    virtual void endTransaction_impl(void) = 0;

    virtual void drawPixel_impl(int32_t x, int32_t y) = 0;
    virtual void fillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) = 0;
    virtual void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) = 0;
    virtual void readRect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) = 0;
    virtual void pushImage_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param, bool use_dma) = 0;
    virtual void pushColors_impl(int32_t length, pixelcopy_t* param) = 0;
    virtual void writeColor_impl(int32_t len) = 0;
    virtual void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) = 0;

//----------------------------------------------------------------------------
// print & text support
//----------------------------------------------------------------------------
// Arduino Print.h compatible
    size_t printNumber(unsigned long n, uint8_t base)
    {
      size_t len = 8 * sizeof(long) + 1;
      char buf[len];
      return write(numberToStr(n, buf, len, base));
    }

    size_t printFloat(double number, uint8_t digits)
    {
      size_t len = 14 + digits;
      char buf[len];
      return write(floatToStr(number, buf, len, digits));
    }

    char* numberToStr(long n, char* buf, size_t buflen, uint8_t base)
    {
      if (n >= 0) return numberToStr((unsigned long) n, buf, buflen, base);
      auto res = numberToStr(- n, buf, buflen, 10) - 1;
      res[0] = '-';
      return res;
    }

    char* numberToStr(unsigned long n, char* buf, size_t buflen, uint8_t base)
    {
      char *str = &buf[buflen - 1];

      *str = '\0';

      if (base < 2) { base = 10; }  // prevent crash if called with base == 1
      do {
        unsigned long m = n;
        n /= base;
        char c = m - base * n;
        *--str = c < 10 ? c + '0' : c + 'A' - 10;
      } while (n);

      return str;
    }

    char* floatToStr(double number, char* buf, size_t buflen, uint8_t digits)
    {
      if (std::isnan(number))    { return strcpy(buf, "nan"); }
      if (std::isinf(number))    { return strcpy(buf, "inf"); }
      if (number > 4294967040.0) { return strcpy(buf, "ovf"); } // constant determined empirically
      if (number <-4294967040.0) { return strcpy(buf, "ovf"); } // constant determined empirically

      char* dst = buf;
      // Handle negative numbers
      //bool negative = (number < 0.0);
      if (number < 0.0) {
        number = -number;
        *dst++ = '-';
      }

      // Round correctly so that print(1.999, 2) prints as "2.00"
      double rounding = 0.5;
      for(uint8_t i = 0; i < digits; ++i) {
        rounding /= 10.0;
      }

      number += rounding;

      // Extract the integer part of the number and print it
      unsigned long int_part = (unsigned long) number;
      double remainder = number - (double) int_part;

      {
        constexpr size_t len = 14;
        char numstr[len];
        auto tmp = numberToStr(int_part, numstr, len, 10);
        auto slen = strlen(tmp);
        memcpy(dst, tmp, slen);
        dst += slen;
      }

      // Print the decimal point, but only if there are digits beyond
      if (digits > 0) {
        dst[0] = '.';
        ++dst;
      }
      // Extract digits from the remainder one at a time
      while (digits-- > 0) {
        remainder *= 10.0;
        unsigned int toPrint = (unsigned int)(remainder);
        dst[0] = '0' + toPrint;
        ++dst;
        remainder -= toPrint;
      }
      dst[0] = 0;
      return buf;
    }

    bool(*fpUpdateFontSize)(LGFXBase* me, uint16_t uniCode) = nullptr;
    static bool updateFontSizeBMP(LGFXBase* me, uint16_t uniCode) {
      if ((uniCode -= 32) >= 96) return false;
      me->_font_size_x.advance = me->_font_size_x.size = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[me->_textfont].widthtbl ) ) + uniCode );
      return true;
    }

    int16_t (*fpDrawChar)(LGFXBase* me, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, uint8_t size_x, uint8_t size_y) = &LGFXBase::drawCharGLCD;

    static int16_t drawCharGLCD(LGFXBase* me, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, uint8_t size_x, uint8_t size_y)
    { // glcd font
      const int32_t fontWidth  = me->_font_size_x.size;
      const int32_t fontHeight = me->_font_size_y.size;

      if (c > 255) return 0;

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      auto font_addr = fontdata[me->getTextFont()].chartbl + (c * 5);
      uint32_t colortbl[2] = {me->_write_conv.convert(back_rgb888), me->_write_conv.convert(fore_rgb888)};
      bool fillbg = (back_rgb888 != fore_rgb888);

      if ((x <= clip_right) && (clip_left < (x + fontWidth * size_x ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * size_y ))) {
        if (!fillbg || size_y > 1 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * size_x > clip_right) {
          int32_t xpos = x;
          me->startWrite();
          for (uint8_t i = 0; i < fontWidth-1; i++) {
            uint32_t len = 1;
            int32_t ypos = y;
            uint8_t line = pgm_read_byte(font_addr + i);
            bool flg = (line & 0x1);
            for (uint8_t j = 1; j < fontHeight; j++) {
              if (flg != (bool)(line & 1 << j)) {
                if (flg || fillbg) {
                  me->setColorRaw(colortbl[flg]);
                  me->_fillRect(xpos, ypos, size_x, len * size_y);
                }
                ypos += len * size_y;
                len = 0;
                flg = !flg;
              }
              len++;
            }
            if (flg || fillbg) {
              me->setColorRaw(colortbl[flg]);
              me->_fillRect(xpos, ypos, size_x, len * size_y);
            }
            xpos += size_x;
          }
          if (fillbg) {
            me->setColorRaw(colortbl[0]);
            me->_fillRect(xpos, y, size_x, fontHeight * size_y); 
          }
          me->endWrite();
        } else {
          uint8_t col[fontWidth];
          for (uint8_t i = 0; i < 5; i++) {
            col[i] = pgm_read_byte(font_addr + i);
          }
          col[5] = 0;
          me->startWrite();
          me->setAddrWindow(x, y, fontWidth * size_x, fontHeight);
          bool flg = col[0] & 1;
          uint32_t len = 0;
          for (uint8_t i = 0; i < fontHeight; i++) {
            for (uint8_t j = 0; j < fontWidth; j++) {
              if (flg != (bool)(col[j] & 1 << i)) {
                me->writeColorRaw(colortbl[flg], len);
                len = 0;
                flg = !flg;
              }
              len += size_x;
            }
          }
          me->writeColorRaw(colortbl[0], len);
          me->endWrite();
        }
      }
      return fontWidth * size_x;
    }

    static int16_t drawCharBMP(LGFXBase* me, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, uint8_t size_x, uint8_t size_y)
    { // BMP font
      uint16_t uniCode = c - 32;
      const int fontWidth = ((c < 32) || (c > 127)) ? 0 : pgm_read_byte(widtbl_f16 + uniCode);
      constexpr int fontHeight = chr_hgt_f16;

      if ((c < 32) || (c > 127)) return 0;
      auto font_addr = (const uint8_t*)pgm_read_dword(&chrtbl_f16[uniCode]);

      uint8_t w = (fontWidth + 6) >> 3;
      uint32_t colortbl[2] = {me->_write_conv.convert(back_rgb888), me->_write_conv.convert(fore_rgb888)};
      bool fillbg = (back_rgb888 != fore_rgb888);

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + fontWidth * size_x ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * size_y ))) {
        if (!fillbg || size_y > 1 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * size_x > clip_right) {
          me->startWrite();
          if (fillbg) {
            me->setColorRaw(colortbl[0]);
            me->_fillRect( x + (fontWidth - 1) * size_x, y, size_x, fontHeight * size_y);
          }
          for (uint8_t i = 0; i < fontHeight; i++) {
            uint8_t line = pgm_read_byte(font_addr);
            bool flg = line & 0x80;
            uint32_t len = 1;
            uint_fast8_t j = 1;
            for (; j < fontWidth-1; j++) {
              if (j & 7) {
                line <<= 1;
              } else {
                line = pgm_read_byte(font_addr + (j >> 3));
              }
              if (flg != (bool)(line & 0x80)) {
                if (flg || fillbg) {
                  me->setColorRaw(colortbl[flg]);
                  me->_fillRect( x + (j - len) * size_x, y, len * size_x, size_y); 
                }
                len = 1;
                flg = !flg;
              } else {
                len++;
              }
            }
            if (flg || fillbg) {
              me->setColorRaw(colortbl[flg]);
              me->_fillRect( x + (j - len) * size_x, y, len * size_x, size_y); 
            }
            y += size_y;
            font_addr += w;
          }
          me->endWrite();
        } else {
          uint32_t len = 0;
          uint8_t line = 0;
          bool flg = false;
          me->startWrite();
          me->setAddrWindow(x, y, fontWidth * size_x, fontHeight);
          for (uint8_t i = 0; i < fontHeight; i++) {
            for (uint8_t j = 0; j < fontWidth; j++) {
              if (j & 7) {
                line <<= 1;
              } else {
                line = (j == fontWidth - 1) ? 0 : pgm_read_byte(font_addr + (j >> 3));
              }
              if (flg != (bool)(line & 0x80)) {
                me->writeColorRaw(colortbl[flg], len);
                flg = !flg;
                len = 0;
              }
              len += size_x;
            }
            font_addr += w;
          }
          me->writeColorRaw(colortbl[flg], len);
          me->endWrite();
        }
      }

      return fontWidth * size_x;
    }

    static int16_t drawCharRLE(LGFXBase* me, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, uint8_t size_x, uint8_t size_y)
    { // RLE font
      auto fontdat = &fontdata[me->getTextFont()];
      uint16_t code = c - 32;
      const int fontWidth = ((c < 32) || (c > 127)) ? 0 : pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdat->widthtbl ) ) + code );
      const int fontHeight = pgm_read_byte( &fontdat->height );

      if ((c < 32) || (c > 127)) return 0;
      auto font_addr = (const uint8_t*)pgm_read_dword( (const void*)(pgm_read_dword( &(fontdat->chartbl ) ) + code * sizeof(void *)) );

      uint32_t colortbl[2] = {me->_write_conv.convert(back_rgb888), me->_write_conv.convert(fore_rgb888)};
      bool fillbg = (back_rgb888 != fore_rgb888);

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + fontWidth * size_x ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * size_y ))) {
        if (!fillbg || size_y > 1 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * size_x > clip_right) {
          bool flg = false;
          uint8_t line = 0, i = 0, j = 0;
          int32_t len;
          me->startWrite();
          do {
            line = pgm_read_byte(font_addr++);
            flg = line & 0x80;
            line = (line & 0x7F)+1;
            do {
              len = (j + line > fontWidth) ? fontWidth - j : line;
              line -= len;
              if (fillbg || flg) {
                me->setColorRaw(colortbl[flg]);
                me->_fillRect( x + j * size_x, y + (i * size_y), len * size_x, size_y);
              }
              j += len;
              if (j == fontWidth) {
                j = 0;
                i++;
              }
            } while (line);
          } while (i < fontHeight);
          me->endWrite();
        } else {
          uint32_t line = 0;
          me->startWrite();
          uint32_t len = fontWidth * size_x * fontHeight;
          me->setAddrWindow(x, y, fontWidth * size_x, fontHeight);
          do {
            line = pgm_read_byte(font_addr++);
            bool flg = line & 0x80;
            line = ((line & 0x7F) + 1) * size_x;
            me->writeColorRaw(colortbl[flg], line);
          } while (len -= line);
          me->endWrite();
        }
      }

      return fontWidth * size_x;
    }

  };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#ifdef LGFX_GFXFONT_HPP_

  template <class Base>
  class LGFX_GFXFont_Support : public Base {

    const GFXfont  *_gfxFont;
    uint8_t _glyph_ab;   // glyph delta Y (height) above baseline
    uint8_t _glyph_bb;   // glyph delta Y (height) below baseline

    static bool updateFontSizeGFXFF(LGFXBase* lgfxbase, uint16_t uniCode) {
      auto me = (LGFX_GFXFont_Support*)lgfxbase;
      auto gfxFont = me->_gfxFont;
      if (uniCode > pgm_read_word(&gfxFont->last )
      ||  uniCode < pgm_read_word(&gfxFont->first)) return false;
      uniCode -= pgm_read_word(&gfxFont->first);
      GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[uniCode]);
      me->_font_size_x.offset  = (int8_t)pgm_read_byte(&glyph->xOffset);
      me->_font_size_x.size    = pgm_read_byte(&glyph->width);
      me->_font_size_x.advance = pgm_read_byte(&glyph->xAdvance);
      return true;
    }

    static int16_t drawCharGFXFF(LGFXBase* lgfxbase, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, uint8_t size_x, uint8_t size_y)
    {
      auto me = (LGFX_GFXFont_Support*)lgfxbase;
      auto gfxFont = me->_gfxFont;
      if (c > pgm_read_word(&gfxFont->last )
      ||  c < pgm_read_word(&gfxFont->first)) return 0;

      c -= pgm_read_word(&gfxFont->first);
      GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c]);

      int32_t w = pgm_read_byte(&glyph->width),
              h = pgm_read_byte(&glyph->height);

      int32_t xAdvance = (int32_t)size_x * (int8_t)pgm_read_byte(&glyph->xAdvance);
      int32_t xoffset  = (int32_t)size_x * (int8_t)pgm_read_byte(&glyph->xOffset);
      int32_t yoffset  = (int32_t)size_y * (int8_t)pgm_read_byte(&glyph->yOffset);

      me->startWrite();
      uint32_t colortbl[2] = {me->_write_conv.convert(back_rgb888), me->_write_conv.convert(fore_rgb888)};
      bool fillbg = (back_rgb888 != fore_rgb888);
      if (fillbg) {
        int32_t left  = std::max(me->_filled_x, x + (xoffset < 0 ? xoffset : 0));
        int32_t right = x + std::max((int32_t)(w * size_x + xoffset), (int32_t)(xAdvance));
        if (right > left) {
          me->setColorRaw(colortbl[0]);
          me->_fillRect(left, y + me->_font_size_y.offset * size_y, right - left, (me->_glyph_bb + me->_glyph_ab) * size_y);
          me->_filled_x = right;
        }
      } else {
        me->_filled_x = 0;
      }

      x += xoffset;
      y += yoffset;

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + w * size_x ))
       && (y <= clip_bottom) && (clip_top < (y + h * size_y ))) {
        uint8_t  *bitmap = (uint8_t *)pgm_read_dword(&gfxFont->bitmap)
                         + pgm_read_word(&glyph->bitmapOffset);
        uint8_t bits=0, bit=0;

        me->setColorRaw(colortbl[1]);
        while (h--) {
          int32_t len = 0;
          int32_t i = 0;
          for (i = 0; i < w; i++) {
            if (bit == 0) {
              bit  = 0x80;
              bits = pgm_read_byte(bitmap++);
            }
            if (bits & bit) len++;
            else if (len) {
              me->_fillRect(x + (i-len) * size_x, y, size_x * len, size_y);
              len=0;
            }
            bit >>= 1;
          }
          if (len) {
            me->_fillRect(x + (i-len) * size_x, y, size_x * len, size_y);
          }
          y += size_y;
        }
      }
      me->endWrite();
      return xAdvance;
    }
  public:

    void setTextFont(uint8_t f) override {
      _gfxFont = nullptr;
      Base::setTextFont(f);
    }

    void setFreeFont(const GFXfont *f = nullptr)
    {
      if (f == nullptr) { this->setTextFont(1); return; } // Use GLCD font
      this->fpDrawChar = drawCharGFXFF;
      this->fpUpdateFontSize = updateFontSizeGFXFF;

      this->_textfont = 1;
      _gfxFont = f; // (GFXfont *)f;
      this->_decoderState = Base::utf8_decode_state_t::utf8_state0;

      _glyph_ab = 0;
      _glyph_bb = 0;
      uint16_t numChars = pgm_read_word(&_gfxFont->last) - pgm_read_word(&_gfxFont->first);
      
      // Find the biggest above and below baseline offsets
      for (uint8_t c = 0; c < numChars; c++)
      {
        GFXglyph *glyph1 = &(((GFXglyph *)pgm_read_dword(&_gfxFont->glyph))[c]);
        int8_t ab = -pgm_read_byte(&glyph1->yOffset);
        if (ab > _glyph_ab) _glyph_ab = ab;
        int8_t bb = pgm_read_byte(&glyph1->height) - ab;
        if (bb > _glyph_bb) _glyph_bb = bb;
      }

      this->_font_baseline = _glyph_ab;
      this->_font_size_y.offset = - _glyph_ab;
      this->_font_size_y.size = _glyph_bb + _glyph_ab;
      this->_font_size_y.advance = (uint8_t)pgm_read_byte(&_gfxFont->yAdvance);
    }
  };

#endif
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  template <class Base>
  class LGFX_VLWFont_Support : public Base {
  public:

    ~LGFX_VLWFont_Support()
    {
      unloadFont();
    }

    void setTextFont(uint8_t f) override {
      unloadFont();
      Base::setTextFont(f);
    }

    void unloadFont( void )
    {
      if (!_fontLoaded) return;
      _fontLoaded = false;
      _fontData->close();
      if (gUnicode)  { free(gUnicode);  gUnicode  = nullptr; }
      if (gWidth)    { free(gWidth);    gWidth    = nullptr; }
      if (gxAdvance) { free(gxAdvance); gxAdvance = nullptr; }
      if (gdX)       { free(gdX);       gdX       = nullptr; }
      if (gBitmap)   { free(gBitmap);   gBitmap   = nullptr; }
    }

#if defined (ARDUINO) && defined (FS_H)

    void loadFont(const char *path, fs::FS &fs) {
      unloadFont();

      _fontFile.setFS(fs);

      loadFont(path);
    }

#endif

    void loadFont(const uint8_t* array) {
      unloadFont();

      _fontPointer.set(array);
      _fontData = &_fontPointer;

      load_font();
    }

    void loadFont(const char *path) {
      unloadFont();

      _fontData = &_fontFile;
      _fontFile.need_transaction &= this->hasTransaction();

      if (_fontFile.need_transaction && this->_transaction_count) this->endTransaction();

      bool result = _fontFile.open(path, "rb");
      if (!result) {
        std::string filename = "/";
        filename += path;
        filename += ".vlw";
        result = _fontFile.open(filename.c_str(), "rb");
      }
      if (result) {
        load_font();
      } else {
        this->setTextFont(1);
      }

      if (_fontFile.need_transaction && this->_transaction_count) { this->beginTransaction(); }
    }


    void showFont(uint32_t td)
    {
      if(!_fontLoaded) return;

      int16_t x = this->width();
      int16_t y = this->height();
      uint32_t timeDelay = 0;    // No delay before first page

      this->fillScreen(this->_text_back_rgb888);

      for (uint16_t i = 0; i < gFont.gCount; i++)
      {
        // Check if this will need a new screen
        if (x + gdX[i] + gWidth[i] >= this->width())  {
          x = -gdX[i];

          y += gFont.yAdvance;
          if (y + gFont.maxAscent + gFont.descent >= this->height()) {
            x = -gdX[i];
            y = 0;
            delay(timeDelay);
            timeDelay = td;
            this->fillScreen(this->_text_back_rgb888);
          }
        }

        this->drawChar(gUnicode[i], x, y);
        x += gxAdvance[i];
        //yield();
      }

      delay(timeDelay);
      this->fillScreen(this->_text_back_rgb888);
      //fontFile.close();
    }

  protected:
    typedef struct
    {
      uint16_t gCount;     // Total number of characters
      uint16_t yAdvance;   // Line advance
      uint16_t spaceWidth; // Width of a space character
      int16_t  ascent;     // Height of top of 'd' above baseline, other characters may be taller
      int16_t  descent;    // Offset to bottom of 'p', other characters may have a larger descent
      uint16_t maxAscent;  // Maximum ascent found in font
      uint16_t maxDescent; // Maximum descent found in font
    } fontMetrics;

    fontMetrics gFont = { 0, 0, 0, 0, 0, 0, 0 };

    // These are for the metrics for each individual glyph (so we don't need to seek this in file and waste time)
    uint16_t* gUnicode  = nullptr;  //UTF-16 code, the codes are searched so do not need to be sequential
    uint8_t*  gWidth    = nullptr;  //cwidth
    uint8_t*  gxAdvance = nullptr;  //setWidth
    int8_t*   gdX       = nullptr;  //leftExtent
    uint32_t* gBitmap   = nullptr;  //file pointer to greyscale bitmap

    bool     _fontLoaded = false; // Flags when a anti-aliased font is loaded

    DataWrapper* _fontData;
    FileWrapper _fontFile;
    PointerWrapper _fontPointer;

    void load_font(void) {
    {
      uint32_t buf[6];
      _fontData->read((uint8_t*)buf, 6 * 4); // 24 Byte read

      gFont.gCount   = __builtin_bswap32(buf[0]); // glyph count in file
                     //__builtin_bswap32(buf[1]); // vlw encoder version - discard
      gFont.yAdvance = __builtin_bswap32(buf[2]); // Font size in points, not pixels
                     //__builtin_bswap32(buf[3]); // discard
      gFont.ascent   = __builtin_bswap32(buf[4]); // top of "d"
      gFont.descent  = __builtin_bswap32(buf[5]); // bottom of "p"
    }

    // These next gFont values might be updated when the Metrics are fetched
    gFont.maxAscent  = gFont.ascent;   // Determined from metrics
    gFont.maxDescent = gFont.descent;  // Determined from metrics
    gFont.yAdvance   = gFont.ascent + gFont.descent;
    gFont.spaceWidth = gFont.yAdvance * 2 / 7;  // Guess at space width

ESP_LOGI("LGFX", "ascent:%d  descent:%d", gFont.ascent, gFont.descent);

    _fontLoaded = true;
    this->_decoderState = Base::utf8_decode_state_t::utf8_state0;
    this->fpDrawChar = drawCharVLW;
    this->fpUpdateFontSize = updateFontSizeVLW;
    this->_font_size_x.offset = 0;
    this->_font_size_y.offset = 0;

    // Fetch the metrics for each glyph
    loadMetrics(gFont.gCount);

    this->_font_size_y.size    = gFont.yAdvance;
    this->_font_size_y.advance = gFont.yAdvance;
    }

    void loadMetrics(uint16_t gCount)
    {
      if (!gCount) return;

      uint32_t bitmapPtr = 24 + (uint32_t)gCount * 28;

      {
        gBitmap   = (uint32_t*)malloc( gCount * 4); // seek pointer to glyph bitmap in the file
        gUnicode  = (uint16_t*)malloc( gCount * 2); // Unicode 16 bit Basic Multilingual Plane (0-FFFF)
        gWidth    =  (uint8_t*)malloc( gCount );    // Width of glyph
        gxAdvance =  (uint8_t*)malloc( gCount );    // xAdvance - to move x cursor
        gdX       =   (int8_t*)malloc( gCount );    // offset for bitmap left edge relative to cursor X
      }

      uint16_t gNum = 0;
      _fontData->seek(24);  // headerPtr
      uint32_t buffer[7];
      do {
        _fontData->read((uint8_t*)buffer, 7 * 4); // 28 Byte read
        uint16_t unicode = __builtin_bswap32(buffer[0]); // Unicode code point value
        gUnicode[gNum]  = unicode;
        gWidth[gNum]    = (uint8_t)__builtin_bswap32(buffer[2]); // Width of glyph
        gxAdvance[gNum] = (uint8_t)__builtin_bswap32(buffer[3]); // xAdvance - to move x cursor
        gdX[gNum]       =  (int8_t)__builtin_bswap32(buffer[5]); // x delta from cursor

        uint16_t height = __builtin_bswap32(buffer[1]); // Height of glyph
        if ((unicode > 0xFF) || ((unicode > 0x20) && (unicode < 0xA0) && (unicode != 0x7F))) {
          int16_t dY =  (int16_t)__builtin_bswap32(buffer[4]); // y delta from baseline
//ESP_LOGI("LGFX", "unicode:%x  dY:%d", unicode, dY);
          if (gFont.maxAscent < dY) {
            gFont.maxAscent = dY;
          }
          if (gFont.maxDescent < (height - dY)) {
//ESP_LOGI("LGFX", "maxDescent:%d", gFont.maxDescent);
            gFont.maxDescent = height - dY;
          }
        }

        gBitmap[gNum] = bitmapPtr;
        bitmapPtr += (uint16_t)gWidth[gNum] * height;
      } while (++gNum < gCount);

      this->_font_baseline = gFont.maxAscent;
      this->_font_size_y.advance = gFont.yAdvance = gFont.maxAscent + gFont.maxDescent;
//ESP_LOGI("LGFX", "maxDescent:%d", gFont.maxDescent);
    }

    bool getUnicodeIndex(uint16_t unicode, uint16_t *index)
    {
      auto poi = std::lower_bound(gUnicode, &gUnicode[gFont.gCount], unicode);
      *index = std::distance(gUnicode, poi);
      return (*poi == unicode);
    }

    static bool updateFontSizeVLW(LGFXBase* lgfxbase, uint16_t uniCode) {
      auto me = (LGFX_VLWFont_Support*)lgfxbase;
      uint16_t gNum = 0;
      if (me->getUnicodeIndex(uniCode, &gNum)) {
        me->_font_size_x.size    = me->gWidth[gNum];
        me->_font_size_x.advance = me->gxAdvance[gNum];
        me->_font_size_x.offset  = me->gdX[gNum];
        return true;
      }
      return false;
    }

    static int16_t drawCharVLW(LGFXBase* lgfxbase, int32_t x, int32_t y, uint16_t code, uint32_t fore_rgb888, uint32_t back_rgb888, uint8_t size_x, uint8_t size_y)
    {
      auto me = (LGFX_VLWFont_Support*)lgfxbase;

      uint16_t gNum = 0;
      if (!me->getUnicodeIndex(code, &gNum)) {
        return 0;
      }

      auto file = me->_fontData;

      if (file->need_transaction && me->_transaction_count) me->endTransaction();

      file->seek(28 + gNum * 28);  // headerPtr
      uint32_t buffer[6];
      file->read((uint8_t*)buffer, 24);
      uint32_t h        = __builtin_bswap32(buffer[0]); // Height of glyph
      uint32_t w        = __builtin_bswap32(buffer[1]); // Width of glyph
      uint32_t xAdvance = __builtin_bswap32(buffer[2]) * size_x; // xAdvance - to move x cursor
      int32_t xoffset   = (int32_t)((int8_t)__builtin_bswap32(buffer[4])) * size_x; // x delta from cursor
      int32_t dY        = (int16_t)__builtin_bswap32(buffer[3]); // y delta from baseline
      int32_t yoffset = ((int32_t)me->gFont.maxAscent - dY) * (int32_t)size_y;

      uint8_t pbuffer[w * h];
      uint8_t* pixel = pbuffer;

      file->seek(me->gBitmap[gNum]);  // headerPtr
      file->read(pixel, w * h);

      if (file->need_transaction && me->_transaction_count) me->beginTransaction();
      me->startWrite();

      uint32_t colortbl[2] = {me->_write_conv.convert(back_rgb888), me->_write_conv.convert(fore_rgb888)};
      bool fillbg = (back_rgb888 != fore_rgb888);
      if (fillbg) {
        int32_t left  = std::max(me->_filled_x, x + (xoffset < 0 ? xoffset : 0));
        int32_t right = x + std::max((int32_t)(w * size_x + xoffset), (int32_t)(xAdvance));
        if (right > left) {
          me->setColorRaw(colortbl[0]);
          me->_fillRect(left, y, right - left, me->gFont.yAdvance * size_y);
 //me->setColorRaw(colortbl[1]);
 //me->drawRect(left, y, right - left, me->gFont.yAdvance * size_y);
          me->_filled_x = right;
        }
      } else {
        me->_filled_x = 0;
      }

      y += yoffset;
      x += xoffset;
      int32_t left = 0;
      int32_t bx = x;
      int32_t bw = w * size_x;
      int32_t clip_left = me->_clip_l;
      if (x < clip_left) { left = -((x - clip_left) / size_x); bw += (x - clip_left); bx = clip_left; }
      int32_t clip_right = me->_clip_r + 1;
      if (bw > clip_right - bx) bw = clip_right - bx;
      if (bw > 0 && (y <= me->_clip_b) && (me->_clip_t < (y + h * size_y))) {
        int32_t fore_r = ((fore_rgb888>>16)&0xFF);
        int32_t fore_g = ((fore_rgb888>> 8)&0xFF);
        int32_t fore_b = ((fore_rgb888)    &0xFF);
        if (fillbg) {
          int32_t back_r = ((back_rgb888>>16)&0xFF);
          int32_t back_g = ((back_rgb888>> 8)&0xFF);
          int32_t back_b = ((back_rgb888)    &0xFF);
          int32_t right = (clip_right - x + size_x - 1) / size_x;
          if (right > w) right = w;
          do {
            int32_t i = left;
            do {
              while (pixel[i] != 0xFF) {
                if (pixel[i] != 0) {
                  int32_t p = 1 + (uint32_t)pixel[i];
                  me->setColor(color888( ( fore_r * p + back_r * (257 - p)) >> 8
                                       , ( fore_g * p + back_g * (257 - p)) >> 8
                                       , ( fore_b * p + back_b * (257 - p)) >> 8 ));
                  me->_fillRect(i * size_x + x, y, size_x, size_y);
                }
                if (++i == right) break;
              }
              if (i == right) break;
              int32_t dl = 1;
              while (i + dl != right && pixel[i + dl] == 0xFF) { ++dl; }
              me->setColorRaw(colortbl[1]);
              me->_fillRect(x + i * size_x, y, dl * size_x, size_y);
              i += dl;
            } while (i != right);
            pixel += w;
            y += size_y;
          } while (--h);
        } else {
          int32_t xshift = (bx - x) % size_x;
          bgr888_t buf[bw * size_y];
          pixelcopy_t p(buf, me->_write_conv.depth, rgb888_3Byte, me->hasPalette());
          do {
            int32_t by = y;
            int32_t bh = size_y;
            if (by < 0) { bh += by; by = 0; }
            if (bh > 0) {
              me->readRectRGB(bx, by, bw, bh, (uint8_t*)buf);
              for (int32_t sx = 0; sx < bw; sx++) {
                int32_t p = 1 + pixel[left + (sx+xshift) / size_x];
                for (int32_t sy = 0; sy < bh; sy++) {
                  auto bgr = &buf[sx + sy * bw];
                  bgr->r = ( fore_r * p + bgr->r * (257 - p)) >> 8;
                  bgr->g = ( fore_g * p + bgr->g * (257 - p)) >> 8;
                  bgr->b = ( fore_b * p + bgr->b * (257 - p)) >> 8;
                }
              }
              me->push_image(bx, by, bw, bh, &p);
            }
            pixel += w;
            if ((y += size_y) >= me->height()) break;
          } while (--h);
        }
      }
      me->endWrite();
      return xAdvance;
    }
  };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  template <class Base>
  class LGFX_IMAGE_FORMAT_Support : public Base {
  public:

#if defined (ARDUINO)
 #if defined (FS_H)

    inline void drawBmp(fs::FS &fs, const char *path, int32_t x=0, int32_t y=0) { drawBmpFile(fs, path, x, y); }
    void drawBmpFile(fs::FS &fs, const char *path, int32_t x=0, int32_t y=0) {
      FileWrapper file;
      file.setFS(fs);
      drawBmpFile(&file, path, x, y);
    }

    bool drawJpgFile( fs::FS &fs, const char *path, int16_t x=0, int16_t y=0, int16_t maxWidth=0, int16_t maxHeight=0, int16_t offX=0, int16_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      bool res;
      FileWrapper file;
      file.setFS(fs);
//      drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);

      file.need_transaction &= this->_has_transaction;
      if (file.need_transaction) this->endTransaction();
      if (file.open(path, "rb")) {
        res = draw_jpg(&file, x, y, maxWidth, maxHeight, offX, offY, scale);
        file.close();
      }
      if (file.need_transaction && this->_transaction_count) { this->beginTransaction(); }
      return res;
    }
 #endif
 #if defined (Stream_h)

    void drawJpg(Stream *dataSource, int16_t x=0, int16_t y=0, int16_t maxWidth=0, int16_t maxHeight=0, int16_t offX=0, int16_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      StreamWrapper data;
      data.set(dataSource);
      return draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

 #endif

#elif defined (CONFIG_IDF_TARGET_ESP32)  // ESP-IDF

    void drawBmpFile(const char *path, int32_t x, int32_t y) {
      FileWrapper file;
      drawBmpFile(&file, path, x, y);
    }

    bool drawJpgFile(const char *path, int16_t x=0, int16_t y=0, int16_t maxWidth=0, int16_t maxHeight=0, int16_t offX=0, int16_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      bool res;
      FileWrapper file;
//      drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
      file.need_transaction &= this->_has_transaction;
      if (file.need_transaction) this->endTransaction();
      if (file.open(path, "rb")) {
        res = draw_jpg(&file, x, y, maxWidth, maxHeight, offX, offY, scale);
        file.close();
      }
      if (file.need_transaction && this->_transaction_count) { this->beginTransaction(); }
      return res;
    }

#endif

    void drawBmp(const uint8_t *bmp_data, uint32_t bmp_len, int32_t x, int32_t y) {
      PointerWrapper data;
      data.set(bmp_data, bmp_len);
      draw_bmp(&data, x, y);
    }
    bool drawJpg(const uint8_t *jpg_data, uint32_t jpg_len, int16_t x=0, int16_t y=0, int16_t maxWidth=0, int16_t maxHeight=0, int16_t offX=0, int16_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      PointerWrapper data;
      data.set(jpg_data, jpg_len);
      return draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

  private:

    void drawBmpFile(FileWrapper* file, const char *path, int32_t x, int32_t y) {
      file->need_transaction &= this->_has_transaction;
      if (file->need_transaction) this->endTransaction();
      if (file->open(path, "rb")) {
        draw_bmp(file, x, y);
        file->close();
      }
      if (file->need_transaction && this->_transaction_count) { this->beginTransaction(); }
    }

    void draw_bmp(DataWrapper* data, int32_t x, int32_t y) {
      if ((x >= this->_width) || (y >= this->_height)) return;

      //uint32_t startTime = millis();
      uint32_t seekOffset;
      int32_t w;
      int32_t h;
      uint16_t bpp;
      {
        struct {
          union {
            uint8_t raw[34];
    //      uint8_t raw[54];
            #pragma pack(1)
            struct {
              uint16_t bfType; 
              uint32_t bfSize;
              uint16_t bfReserved1;
              uint16_t bfReserved2;
              uint32_t bfOffBits;
              uint32_t biSize; 
              int32_t  biWidth;
              int32_t  biHeight;
              uint16_t biPlanes; 
              uint16_t biBitCount;
              uint32_t biCompression;
            //uint32_t biSizeImage; 
            //int32_t  biXPelsPerMeter;
            //int32_t  biYPelsPerMeter;
            //uint32_t biClrUsed; 
            //uint32_t biClrImportant;
            };
            #pragma pack()
          };
        } bmpdata;
        data->read(bmpdata.raw, sizeof(bmpdata));
        if ((bmpdata.bfType != 0x4D42)   // bmp header "BM"
         || (bmpdata.biPlanes != 1)  // bcPlanes always 1
         || (bmpdata.biWidth + x < 0)
         || (bmpdata.biHeight + y < 0)
         || (bmpdata.biBitCount > 32)
         || (bmpdata.biBitCount == 0)
         || (bmpdata.biCompression != 0 && bmpdata.biCompression != 3)) { // RLE not supported
// Serial.println("BMP format not recognized.");
          return;
        }
        seekOffset = bmpdata.bfOffBits;
        w = bmpdata.biWidth;
        h = bmpdata.biHeight;  // bcHeight Image height (pixels)
        bpp = bmpdata.biBitCount; // 24 bcBitCount 24=RGB24bit
      }
        //If the value of Height is positive, the image data is from bottom to top
        //If the value of Height is negative, the image data is from top to bottom.
      int32_t flow = (h < 0) ? 1 : -1;
      if (h < 0) h = -h;
      else y += h - 1;

      argb8888_t *palette = nullptr;
      if (bpp <= 8) {
        palette = new argb8888_t[1 << bpp];
        data->seek(seekOffset - (1 << bpp) * sizeof(argb8888_t));
        data->read((uint8_t*)palette, (1 << bpp)*sizeof(argb8888_t)); // load palette
      }

      data->seek(seekOffset);

      auto dst_depth = this->_write_conv.depth;
      uint8_t lineBuffer[((w * bpp + 31) >> 5) << 2];  // readline 4Byte align.
      pixelcopy_t p(lineBuffer, dst_depth, (color_depth_t)bpp, this->_palette_count, palette);
      p.no_convert = false;
      if (8 >= bpp && !this->_palette_count) {
        p.fp_copy = pixelcopy_t::get_fp_palettecopy<argb8888_t>(dst_depth);
      } else {
        if (bpp == 16) {
          p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb565_t>(dst_depth);
        } else if (bpp == 24) {
          p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb888_t>(dst_depth);
        } else if (bpp == 32) {
          p.fp_copy = pixelcopy_t::get_fp_normalcopy<argb8888_t>(dst_depth);
        }
      }

      while (--h >= 0) {
        if (data->need_transaction) this->endTransaction();
        data->read(lineBuffer, sizeof(lineBuffer));
        if (data->need_transaction) this->beginTransaction();
        this->push_image(x, y, w, 1, &p);
        y += flow;
      }

      if (palette) delete[] palette;
      //Serial.print("Loaded in "); Serial.print(millis() - startTime);   Serial.println(" ms");
    }


//#if !defined (_TJPGDEC_H_)
#if !defined (DEF_TJPGDEC)

    bool draw_jpg(DataWrapper* data, int16_t x, int16_t y, int16_t maxWidth, int16_t maxHeight, int16_t offX, int16_t offY, jpeg_div_t scale)
    {
      ESP_LOGI("LGFX","drawJpg need include utility/tjpgdClass.h");
    }

#else

    struct draw_jpg_info_t {
      int32_t x;
      int32_t y;
      DataWrapper *data;
      LGFXBase *tft;
      pixelcopy_t *pc;
    };

    static uint32_t jpg_read_data(JDEC  *decoder, uint8_t *buf, uint32_t len) {
      draw_jpg_info_t *jpeg = (draw_jpg_info_t *)decoder->device;
      auto data = (DataWrapper*)jpeg->data;
      auto res = len;
      if (data->need_transaction) jpeg->tft->endTransaction();
      if (buf) {
        res = data->read(buf, len);
      } else {
        data->skip(len);
      }
      if (data->need_transaction) jpeg->tft->beginTransaction();
      return res;
    }

    static uint32_t jpg_push_image(JDEC  *decoder, void *bitmap, JRECT *rect) {
      draw_jpg_info_t *jpeg = (draw_jpg_info_t *)decoder->device;
      jpeg->pc->src_data = bitmap;
      jpeg->tft->push_image( jpeg->x + rect->left
                           , jpeg->y + rect->top 
                           , rect->right  - rect->left + 1
                           , rect->bottom - rect->top + 1
                           , jpeg->pc
                           , false);
      return 1;
    }

    bool draw_jpg(DataWrapper* data, int16_t x, int16_t y, int16_t maxWidth, int16_t maxHeight, int16_t offX, int16_t offY, jpeg_div_t scale)
    {
      draw_jpg_info_t jpeg;
      pixelcopy_t pc(nullptr, this->getColorDepth(), bgr888_t::depth, this->hasPalette());
      jpeg.pc = &pc;
      jpeg.tft = this;
      jpeg.data = data;
      jpeg.x = x - offX;
      jpeg.y = y - offY;

      //TJpgD jpegdec;
      JDEC jpegdec;

      static constexpr uint16_t sz_pool = 3100;
      uint8_t pool[sz_pool];
//    uint8_t *pool = new uint8_t[sz_pool];
//      if (!pool) {
//        ESP_LOGE("LGFX","memory allocation failure");
//        delete[] pool;
//        return false;
//      }

//      auto jres = jpegdec.prepare(jpg_read_data, pool, sz_pool, &jpeg);
      auto jres = jd_prepare(&jpegdec, jpg_read_data, pool, sz_pool, &jpeg);

      if (jres != JDR_OK) {
        ESP_LOGE("LGFX","jpeg prepare error:%x", jres);
//        delete[] pool;
        return false;
      }

      if (!maxWidth) maxWidth = this->width();
      auto cl = this->_clip_l;
      if (0 > x - cl) { maxWidth += x - cl; x = cl; }
      auto cr = this->_clip_r + 1;
      if (maxWidth > (cr - x)) maxWidth = (cr - x);

      if (!maxHeight) maxHeight = this->height();
      auto ct = this->_clip_t;
      if (0 > y - ct) { maxHeight += y - ct; y = ct; }
      auto cb = this->_clip_b + 1;
      if (maxHeight > (cb - y)) maxHeight = (cb - y);

      if (maxWidth > 0 && maxHeight > 0) {
        this->setClipRect(x, y, maxWidth, maxHeight);
        this->startWrite();
  //      jres = jpegdec.decomp(jpg_push_image, nullptr);
        jres = jd_decomp(&jpegdec, jpg_push_image, scale);

        this->_clip_l = cl;
        this->_clip_t = ct;
        this->_clip_r = cr-1;
        this->_clip_b = cb-1;
        this->endWrite();
      }
//      delete[] pool;

      if (jres != JDR_OK) {
        ESP_LOGE("LGFX","jpeg decomp error:%x", jres);
        return false;
      }
      return true;
    }
#endif

  };
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

  class LovyanGFX : public
  #ifdef LGFX_GFXFONT_HPP_
   LGFX_GFXFont_Support<
  #endif
    LGFX_VLWFont_Support<
     LGFX_IMAGE_FORMAT_Support<
      LGFXBase
     >
    >
  #ifdef LGFX_GFXFONT_HPP_
   >
  #endif
  {};
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif
