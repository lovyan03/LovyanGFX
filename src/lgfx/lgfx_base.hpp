/*----------------------------------------------------------------------------/
  Lovyan GFX library - ESP32 hardware SPI graphics library .  
  
    for Arduino and ESP-IDF  
  
Original Source:  
 https://github.com/lovyan03/LovyanGFX/  

Licence:  
 [BSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)  

Author:  
 [lovyan03](https://twitter.com/lovyan03)  

Contributors:  
 [ciniml](https://github.com/ciniml)  
 [mongonta0716](https://github.com/mongonta0716)  
 [tobozo](https://github.com/tobozo)  
/----------------------------------------------------------------------------*/
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
#include <string>

#include "lgfx_common.hpp"

namespace lgfx
{
  #include "../Fonts/glcdfont.h"

  #include "../Fonts/Font16.h"
  #include "../Fonts/Font32rle.h"
  #include "../Fonts/Font64rle.h"
  #include "../Fonts/Font7srle.h"
  #include "../Fonts/Font72rle.h"

  const  uint8_t widtbl_null[1] = {0};
  PROGMEM const uint8_t chr_null[1] = {0};
  PROGMEM const uint8_t* const chrtbl_null[1] = {chr_null};

  enum font_type_t
  { ft_unknown
  , ft_glcd
  , ft_bmp
  , ft_bdf
  , ft_rle
  };

  enum attribute_t
  { cp437_switch = 1
  , utf8_switch  = 2
  };

  struct fontinfo {
    font_type_t type;
    const uint8_t *chartbl;
    const uint8_t *widthtbl;
    uint8_t height;
    uint8_t baseline;
    const uint16_t *indextbl;
    const uint16_t indexsize;
  };

  const PROGMEM fontinfo fontdata [] = {
    // GLCD font (Font 0)
    { ft_glcd, (const uint8_t *)font, widtbl_null, 8, 6, nullptr, 0},
    // GLCD font (or GFX font)
    { ft_glcd, (const uint8_t *)font, widtbl_null, 8, 6, nullptr, 0 },

    { ft_bmp,  (const uint8_t *)chrtbl_f16, widtbl_f16, chr_hgt_f16, baseline_f16, nullptr, 0},
#ifdef __EFONT_FONT_DATA_H__
    // Font 3 efont
    { ft_bdf, (const uint8_t *)efontFontData, nullptr, 16, 14, efontFontList, sizeof(efontFontList)>>1 },
#else
    // Font 3 current unused
    { ft_unknown, (const uint8_t *)chrtbl_null, widtbl_null, 0, 0, nullptr, 0 },
#endif
    { ft_rle, (const uint8_t *)chrtbl_f32, widtbl_f32, chr_hgt_f32, baseline_f32, nullptr, 0},

    // Font 5 current unused
    { ft_unknown, (const uint8_t *)chrtbl_null, widtbl_null, 0, 0, nullptr, 0 },

    { ft_rle, (const uint8_t *)chrtbl_f64, widtbl_f64, chr_hgt_f64, baseline_f64, nullptr, 0},

    { ft_rle, (const uint8_t *)chrtbl_f7s, widtbl_f7s, chr_hgt_f7s, baseline_f7s, nullptr, 0},

    { ft_rle, (const uint8_t *)chrtbl_f72, widtbl_f72, chr_hgt_f72, baseline_f72, nullptr, 0}

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
// rgb565 : uint16_t & int16_t & int
// rgb332 : uint8_t
    __attribute__ ((always_inline)) inline void setColor(uint8_t r, uint8_t g, uint8_t b) { _color.raw = _write_conv.convert(lgfx::color888(r,g,b)); }
    template<typename T> __attribute__ ((always_inline)) inline void setColor(T c) { _color.raw = _write_conv.convert(c); }
                         __attribute__ ((always_inline)) inline void setRawColor(uint32_t c) { _color.raw = c; }

                         inline void clear      ( void )          { _color.raw = 0;  fillRect(0, 0, _width, _height); }
    template<typename T> inline void clear      ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }
    template<typename T> inline void fillScreen ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }

    template<typename T> inline void pushBlock  ( const T& color, int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }

// Deprecated, use pushBlock()
    template<typename T> inline void pushColor  ( const T& color, int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }
    template<typename T> inline void pushColor  ( const T& color                ) {                          setColor(color); startWrite(); pushBlock_impl(1);      endWrite(); }


// AdafruitGFX compatible functions.
// However, startWrite and endWrite have an internal counter and are executed when the counter is 0.
// If you do not want to the counter, call the transaction function directly.
    __attribute__ ((always_inline)) inline void startWrite(void) {                           if (1 == ++_transaction_count) beginTransaction(); }
    __attribute__ ((always_inline)) inline void endWrite(void)   { if (_transaction_count) { if (0 == --_transaction_count) endTransaction(); } }
    template<typename T> inline void writePixel    ( int32_t x, int32_t y                      , const T& color) { setColor(color); writePixel    (x, y         ); }
    template<typename T> inline void writeFastVLine( int32_t x, int32_t y           , int32_t h, const T& color) { setColor(color); writeFastVLine(x, y   , h   ); }
    template<typename T> inline void writeFastHLine( int32_t x, int32_t y, int32_t w           , const T& color) { setColor(color); writeFastHLine(x, y, w      ); }
    template<typename T> inline void writeFillRect ( int32_t x, int32_t y, int32_t w, int32_t h, const T& color) { setColor(color); writeFillRect (x, y, w, h   ); }
    template<typename T> inline void writeColor    ( const T& color, int32_t length) { if (0 >= length) return; setColor(color);    pushBlock_impl(length);             }
                         inline void writeRawColor ( uint32_t color, int32_t length) { if (0 >= length) return; setRawColor(color); pushBlock_impl(length);             }

    template<typename T> inline void drawPixel     ( int32_t x, int32_t y                                 , const T& color) { setColor(color); drawPixel    (x, y         ); }
    template<typename T> inline void drawFastVLine ( int32_t x, int32_t y           , int32_t h           , const T& color) { setColor(color); drawFastVLine(x, y   , h   ); }
    template<typename T> inline void drawFastHLine ( int32_t x, int32_t y, int32_t w                      , const T& color) { setColor(color); drawFastHLine(x, y, w      ); }
    template<typename T> inline void fillRect      ( int32_t x, int32_t y, int32_t w, int32_t h           , const T& color) { setColor(color); fillRect     (x, y, w, h   ); }
    template<typename T> inline void drawRect      ( int32_t x, int32_t y, int32_t w, int32_t h           , const T& color) { setColor(color); drawRect     (x, y, w, h   ); }
    template<typename T> inline void drawRoundRect ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, const T& color) { setColor(color); drawRoundRect(x, y, w, h, r); }
    template<typename T> inline void fillRoundRect ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, const T& color) { setColor(color); fillRoundRect(x, y, w, h, r); }
    template<typename T> inline void drawCircle    ( int32_t x, int32_t y                      , int32_t r, const T& color) { setColor(color); drawCircle   (x, y      , r); }
    template<typename T> inline void fillCircle    ( int32_t x, int32_t y                      , int32_t r, const T& color) { setColor(color); fillCircle   (x, y      , r); }
    template<typename T> inline void drawEllipse   ( int32_t x, int32_t y, int32_t rx, int32_t ry         , const T& color) { setColor(color); drawEllipse  (x, y, rx, ry ); }
    template<typename T> inline void fillEllipse   ( int32_t x, int32_t y, int32_t rx, int32_t ry         , const T& color) { setColor(color); fillEllipse  (x, y, rx, ry ); }
    template<typename T> inline void drawLine      ( int32_t x0, int32_t y0, int32_t x1, int32_t y1                        , const T& color)  { setColor(color); drawLine(    x0, y0, x1, y1        ); }
    template<typename T> inline void drawTriangle  ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); drawTriangle(x0, y0, x1, y1, x2, y2); }
    template<typename T> inline void fillTriangle  ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); fillTriangle(x0, y0, x1, y1, x2, y2); }
    template<typename T> inline void drawArc       ( int32_t x, int32_t y, int32_t r1, int32_t r2, float start, float end, const T& color) { setColor(color); drawArc( x, y, r1, r2, start, end); }
    template<typename T> inline void fillArc       ( int32_t x, int32_t y, int32_t r1, int32_t r2, float start, float end, const T& color) { setColor(color); fillArc( x, y, r1, r2, start, end); }
    template<typename T> inline void drawCircleHelper(int32_t x, int32_t y, int32_t r, uint_fast8_t cornername            , const T& color)  { setColor(color); drawCircleHelper(x, y, r, cornername    ); }
    template<typename T> inline void fillCircleHelper(int32_t x, int32_t y, int32_t r, uint_fast8_t corners, int32_t delta, const T& color)  { setColor(color); fillCircleHelper(x, y, r, corners, delta); }

    __attribute__ ((always_inline)) inline static uint8_t  color332(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color332(r, g, b); }
    __attribute__ ((always_inline)) inline static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color565(r, g, b); }
    __attribute__ ((always_inline)) inline static uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color888(r, g, b); }
    __attribute__ ((always_inline)) inline static uint16_t swap565( uint8_t r, uint8_t g, uint8_t b) { return lgfx::swap565( r, g, b); }
    __attribute__ ((always_inline)) inline static uint32_t swap888( uint8_t r, uint8_t g, uint8_t b) { return lgfx::swap888( r, g, b); }

    __attribute__ ((always_inline)) inline void setPivot(int16_t x, int16_t y) { _xpivot = x; _ypivot = y; }
    __attribute__ ((always_inline)) inline int16_t getPivotX(void) const { return _xpivot; }
    __attribute__ ((always_inline)) inline int16_t getPivotY(void) const { return _ypivot; }

    __attribute__ ((always_inline)) inline int32_t width        (void) const { return _width; }
    __attribute__ ((always_inline)) inline int32_t height       (void) const { return _height; }
    __attribute__ ((always_inline)) inline uint8_t getTextFont  (void) const { return _textfont; }
    __attribute__ ((always_inline)) inline color_depth_t getColorDepth(void) const { return _write_conv.depth; }
    __attribute__ ((always_inline)) inline color_conv_t* getColorConverter(void) { return &_write_conv; }
    __attribute__ ((always_inline)) inline bool hasPalette    (void) const { return _palette_count; }
    __attribute__ ((always_inline)) inline bool isSPIShared(void) const { return _spi_shared; }
    __attribute__ ((always_inline)) inline bool getSwapBytes    (void) const { return _swapBytes; }
    __attribute__ ((always_inline)) inline void setSwapBytes(bool swap) { _swapBytes = swap; }

    __attribute__ ((always_inline)) inline void beginTransaction(void) { beginTransaction_impl(); }
    __attribute__ ((always_inline)) inline void endTransaction(void)   { endTransaction_impl(); }
    __attribute__ ((always_inline)) inline void waitDMA(void)  { waitDMA_impl(); }
    __attribute__ ((always_inline)) inline void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) { setWindow_impl(xs, ys, xe, ye); }

    void setSPIShared(bool shared) { _spi_shared = shared; }

    void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      if (_adjust_abs(x, w)||_adjust_abs(y, h)) return;
      bool tr = !_transaction_count;
      if (tr) beginTransaction();
      setWindow(x, y, x + w - 1, y + h - 1);
      if (tr) endTransaction();
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

    void getClipRect(int32_t *x, int32_t *y, int32_t *w, int32_t *h) {
      *x = _clip_l;
      *y = _clip_t;
      *w = _clip_r - _clip_l + 1;
      *h = _clip_b - _clip_t + 1;
    }

    void clearClipRect(void) {
      _clip_l = 0;
      _clip_t = 0;
      _clip_r = _width - 1;
      _clip_b = _height - 1;
    }

    __attribute__ ((always_inline))
    inline void drawPixel(int32_t x, int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;

      drawPixel_impl(x, y);
    }

    __attribute__ ((always_inline))
    inline void writePixel(int32_t x, int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;

      writeFillRect_impl(x, y, 1, 1);
    }

    void drawFastVLine(int32_t x, int32_t y, int32_t h)
    {
      _adjust_abs(y, h);
      bool tr = !_transaction_count;
      if (tr) beginTransaction();
      writeFastVLine(x, y, h);
      if (tr) endTransaction();
    }

    void writeFastVLine(int32_t x, int32_t y, int32_t h)
    {
      if (x < _clip_l || x > _clip_r) return;
      auto ct = _clip_t;
      if (y < ct) { h += y - ct; y = ct; }
      auto cb = _clip_b + 1 - y;
      if (h > cb) h = cb;
      if (h < 1) return;

      writeFillRect_impl(x, y, 1, h);
    }

    void drawFastHLine(int32_t x, int32_t y, int32_t w)
    {
      _adjust_abs(x, w);
      bool tr = !_transaction_count;
      if (tr) beginTransaction();
      writeFastHLine(x, y, w);
      if (tr) endTransaction();
    }

    void writeFastHLine(int32_t x, int32_t y, int32_t w)
    {
      if (y < _clip_t || y > _clip_b) return;
      auto cl = _clip_l;
      if (x < cl) { w += x - cl; x = cl; }
      auto cr = _clip_r + 1 - x;
      if (w > cr) w = cr;
      if (w < 1) return;

      writeFillRect_impl(x, y, w, 1);
    }

    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      _adjust_abs(x, w);
      _adjust_abs(y, h);
      bool tr = !_transaction_count;
      if (tr) beginTransaction();
      writeFillRect(x, y, w, h);
      if (tr) endTransaction();
    }

    void writeFillRect(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      auto cl = _clip_l;
      if (x < cl) { w += x - cl; x = cl; }
      auto cr = _clip_r + 1 - x;
      if (w > cr) w = cr;
      if (w < 1) return;

      auto ct = _clip_t;
      if (y < ct) { h += y - ct; y = ct; }
      auto cb = _clip_b + 1 - y;
      if (h > cb) h = cb;
      if (h < 1) return;

      writeFillRect_impl(x, y, w, h);
    }

    __attribute__ ((always_inline)) inline 
    void writeFillRectPreclipped(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      writeFillRect_impl(x, y, w, h);
    }

    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      if (_adjust_abs(x, w)||_adjust_abs(y, h)) return;
      bool tr = !_transaction_count;
      if (tr) beginTransaction();
      writeFastHLine(x, y        , w);
      if (--h) {
        writeFastHLine(x, y + h    , w);
        if (--h) {
          writeFastVLine(x        , ++y, h);
          writeFastVLine(x + w - 1,   y, h);
        }
      }
      if (tr) endTransaction();
    }

    void drawCircle(int32_t x, int32_t y, int32_t r)
    {
      if ( r <= 0 ) {
        drawPixel(x, y);
        return;
      }

      startWrite();
      int32_t f = 1 - r;
      int32_t ddF_y = - (r << 1);
      int32_t ddF_x = 1;
      int32_t i = 0;
      int32_t j = -1;
      do {
        while (f < 0) {
          ++i;
          f += (ddF_x += 2);
        }
        f += (ddF_y += 2);

        writeFastHLine(x - i    , y + r, i - j);
        writeFastHLine(x - i    , y - r, i - j);
        writeFastHLine(x + j + 1, y - r, i - j);
        writeFastHLine(x + j + 1, y + r, i - j);

        writeFastVLine(x - r, y - i    , i - j);
        writeFastVLine(x + r, y - i    , i - j);
        writeFastVLine(x + r, y + j + 1, i - j);
        writeFastVLine(x - r, y + j + 1, i - j);
        j = i;
      } while (i < --r);
      endWrite();
    }

    void drawCircleHelper(int32_t x, int32_t y, int32_t r, uint8_t cornername)
    {
      if (r <= 0) return;
      int32_t f     = 1 - r;
      int32_t ddF_y = - (r << 1);
      int32_t ddF_x = 1;
      int32_t i     = 0;
      int32_t j     = 0;

      startWrite();
      do {
        while (f < 0) {
          ++i;
          f += (ddF_x += 2);
        }
        f += (ddF_y += 2);

        if (cornername & 0x1) { // left top
          writeFastHLine(x - i, y - r, i - j);
          writeFastVLine(x - r, y - i, i - j);
        }
        if (cornername & 0x2) { // right top
          writeFastVLine(x + r    , y - i, i - j);
          writeFastHLine(x + j + 1, y - r, i - j);
        }
        if (cornername & 0x4) { // right bottom
          writeFastHLine(x + j + 1, y + r    , i - j);
          writeFastVLine(x + r    , y + j + 1, i - j);
        }
        if (cornername & 0x8) { // left bottom
          writeFastVLine(x - r, y + j + 1, i - j);
          writeFastHLine(x - i, y + r    , i - j);
        }
        j = i;
      } while (i < --r);
      endWrite();
    }

    void fillCircle(int32_t x, int32_t y, int32_t r) {
      startWrite();
      writeFastHLine(x - r, y, (r << 1) + 1);
      fillCircleHelper(x, y, r, 3, 0);
      endWrite();
    }

    void fillCircleHelper(int32_t x, int32_t y, int32_t r, uint_fast8_t corners, int32_t delta)
    {
      if (r <= 0) return;

      ++delta;

      int32_t f     = 1 - r;
      int32_t ddF_y = - (r << 1);
      int32_t ddF_x = 1;
      int32_t i     = 0;

      startWrite();
      do {
        int32_t len = 0;
        while (f < 0) {
          f += (ddF_x += 2);
          ++len;
        }
        i += len;
        f += (ddF_y += 2);

        if (corners & 0x1) {
          if (len) writeFillRect(x - r, y + i - len + 1, (r << 1) + delta, len);
          writeFastHLine(x - i, y + r, (i << 1) + delta);
        }
        if (corners & 0x2) {
          writeFastHLine(x - i, y - r, (i << 1) + delta);
          if (len) writeFillRect(x - r, y - i, (r << 1) + delta, len);
        }
      } while (i < --r);
      endWrite();
    }

    void drawEllipse(int32_t x0, int32_t y0, int32_t rx, int32_t ry)
    {
      if (ry == 0) {
        drawFastHLine(x0 - rx, y0, (ry << 2) + 1);
        return;
      }
      if (rx == 0) {
        drawFastVLine(x0, y0 - ry, (rx << 2) + 1);
        return;
      }
      if (rx < 0 || ry < 0) return;

      int32_t x, y, s, i;
      int32_t rx2 = rx * rx;
      int32_t ry2 = ry * ry;

      startWrite();

      i = -1;
      x = 0;
      y = ry;
      s = (ry2 << 1) + rx2 * (1 - (ry << 1));
      do {
        while ( s < 0 ) s += ry2 * ((++x << 2) + 2);
        writeFastHLine(x0 - x    , y0 - y, x - i);
        writeFastHLine(x0 + i + 1, y0 - y, x - i);
        writeFastHLine(x0 + i + 1, y0 + y, x - i);
        writeFastHLine(x0 - x    , y0 + y, x - i);
        i = x;
        s -= (--y) * rx2 << 2;
      } while (ry2 * x <= rx2 * y);

      i = -1;
      y = 0;
      x = rx;
      s = (rx2 << 1) + ry2 * (1 - (rx << 1));
      do {
        while ( s < 0 ) s += rx2 * ((++y << 2) + 2);
        writeFastVLine(x0 - x, y0 - y    , y - i);
        writeFastVLine(x0 - x, y0 + i + 1, y - i);
        writeFastVLine(x0 + x, y0 + i + 1, y - i);
        writeFastVLine(x0 + x, y0 - y    , y - i);
        i = y;
        s -= (--x) * ry2 << 2;
      } while (rx2 * y <= ry2 * x);

      endWrite();
    }

    void fillEllipse(int32_t x0, int32_t y0, int32_t rx, int32_t ry)
    {
      if (ry == 0) {
        drawFastHLine(x0 - rx, y0, (ry << 2) + 1);
        return;
      }
      if (rx == 0) {
        drawFastVLine(x0, y0 - ry, (rx << 2) + 1);
        return;
      }
      if (rx < 0 || ry < 0) return;

      int32_t x, y, i;
      int32_t rx2 = rx * rx;
      int32_t ry2 = ry * ry;
      int32_t s;

      startWrite();

      writeFastHLine(x0 - rx, y0, (rx << 1) + 1);
      i = 0;
      y = 0;
      x = rx;
      s = (rx2 << 1) + ry2 * (1 - (rx << 1));
      do {
        while (s < 0) s += rx2 * ((++y << 2) + 2);
        writeFillRect(x0 - x, y0 - y    , (x << 1) + 1, y - i);
        writeFillRect(x0 - x, y0 + i + 1, (x << 1) + 1, y - i);
        i = y;
        s -= (--x) * ry2 << 2;
      } while (rx2 * y <= ry2 * x);

      x = 0;
      y = ry;
      s = (ry2 << 1) + rx2 * (1 - (ry << 1));
      do {
        while (s < 0) s += ry2 * ((++x << 2) + 2);
        writeFastHLine(x0 - x, y0 - y, (x << 1) + 1);
        writeFastHLine(x0 - x, y0 + y, (x << 1) + 1);
        s -= (--y) * rx2 << 2;
      } while(ry2 * x <= rx2 * y);

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
      writeFastVLine(x      , y0 + 1, h - len);
      writeFastVLine(x + w  , y0 + 1, h - len);

      int32_t x1 = x + w - r;
      int32_t x0 = x + r;
      writeFastHLine(x0 + 1, y      , w - len);
      writeFastHLine(x0 + 1, y + h  , w - len);

      int32_t f     = 1 - r;
      int32_t ddF_y = -(r << 1);
      int32_t ddF_x = 1;

      len = 0;
      for (int32_t i = 0; i <= r; i++) {
        len++;
        if (f >= 0) {
          writeFastHLine(x0 - i          , y0 - r, len);
          writeFastHLine(x0 - i          , y1 + r, len);
          writeFastHLine(x1 + i - len + 1, y1 + r, len);
          writeFastHLine(x1 + i - len + 1, y0 - r, len);
          writeFastVLine(x1 + r, y1 + i - len + 1, len);
          writeFastVLine(x0 - r, y1 + i - len + 1, len);
          writeFastVLine(x1 + r, y0 - i, len);
          writeFastVLine(x0 - r, y0 - i, len);
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
      writeFillRect(x, y2, w, h + ddF_y);
      int32_t x0 = x + r;
      int32_t f     = 1 - r;
      int32_t ddF_x = 1;
      int32_t len = 0;
      for (int32_t i = 0; i <= r; i++) {
        len++;
        if (f >= 0) {
          writeFillRect(x0 - r, y2 - i          , (r << 1) + delta, len);
          writeFillRect(x0 - r, y1 + i - len + 1, (r << 1) + delta, len);
          if (i == r) break;
          len = 0;
          writeFastHLine(x0 - i, y1 + r, (i << 1) + delta);
          ddF_y += 2;
          f     += ddF_y;
          writeFastHLine(x0 - i, y2 - r, (i << 1) + delta);
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
        do {
          ++dlen;
          if ((err -= dy) < 0) {
            writeFillRect_impl(y0, xs, 1, dlen);
            err += dx;
            xs = x0 + 1; dlen = 0; y0 += ystep;
            if ((y0 < _clip_l) || (y0 > _clip_r)) break;
          }
        } while (++x0 <= x1);
        if (dlen) writeFillRect_impl(y0, xs, 1, dlen);
      } else {
        if (x1 > (_clip_r)) x1 = (_clip_r);
        do {
          ++dlen;
          if ((err -= dy) < 0) {
            writeFillRect_impl(xs, y0, dlen, 1);
            err += dx;
            xs = x0 + 1; dlen = 0; y0 += ystep;
            if ((y0 < _clip_t) || (y0 > _clip_b)) break;
          }
        } while (++x0 <= x1);
        if (dlen) writeFillRect_impl(xs, y0, dlen, 1);
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
      int32_t a, b;

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
        drawFastHLine(a, y0, b - a + 1);
        return;
      }
      if ((x1-x0) * (y2-y0) == (x2-x0) * (y1-y0)) {
        drawLine(x0,y0,x2,y2);
        return;
      }

      int32_t dy1 = y1 - y0;
      int32_t dy2 = y2 - y0;
      bool change = ((x1 - x0) * dy2 > (x2 - x0) * dy1);
      int32_t dx1 = abs(x1 - x0);
      int32_t dx2 = abs(x2 - x0);
      int32_t xstep1 = x1 < x0 ? -1 : 1;
      int32_t xstep2 = x2 < x0 ? -1 : 1;
      a = b = x0;
      if (change) {
        std::swap(dx1, dx2);
        std::swap(dy1, dy2);
        std::swap(xstep1, xstep2);
      }
      int32_t err1 = (std::max(dx1, dy1) >> 1)
                   + (xstep1 < 0
                     ? std::min(dx1, dy1)
                     : dx1);
      int32_t err2 = (std::max(dx2, dy2) >> 1)
                   + (xstep2 > 0
                     ? std::min(dx2, dy2)
                     : dx2);
      startWrite();
      if (y0 != y1) {
        do {
          err1 -= dx1;
          while (err1 < 0) { err1 += dy1; a += xstep1; }
          err2 -= dx2;
          while (err2 < 0) { err2 += dy2; b += xstep2; }
          writeFastHLine(a, y0, b - a + 1);
        } while (++y0 < y1);
      }

      if (change) {
        b = x1;
        xstep2 = x2 < x1 ? -1 : 1;
        dx2 = abs(x2 - x1);
        dy2 = y2 - y1;
        err2 = (std::max(dx2, dy2) >> 1)
             + (xstep2 > 0
               ? std::min(dx2, dy2)
               : dx2);
      } else {
        a = x1;
        dx1 = abs(x2 - x1);
        dy1 = y2 - y1;
        xstep1 = x2 < x1 ? -1 : 1;
        err1 = (std::max(dx1, dy1) >> 1)
             + (xstep1 < 0
               ? std::min(dx1, dy1)
               : dx1);
      }
      do {
        err1 -= dx1;
        while (err1 < 0) { err1 += dy1; if ((a += xstep1) == x2) break; }
        err2 -= dx2;
        while (err2 < 0) { err2 += dy2; if ((b += xstep2) == x2) break; }
        writeFastHLine(a, y0, b - a + 1);
      } while (++y0 <= y2);
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
        if (steep) writePixel(y0, x);
        else       writePixel(x, y0);
        err -= dy;
        if (err < 0) {
          err += dx;
          y0 += ystep;
        }
      }
      endWrite();
    }

    void drawArc(int32_t x, int32_t y, int32_t r1, int32_t r2, float start, float end)
    {
      if (r1 < r2) std::swap(r1, r2);
      if (r1 < 1) r1 = 1;
      if (r2 < 1) r2 = 1;

      bool diff = start > end || start < end;
      float astart = fmodf(start, 360);
      float aend = fmodf(end, 360);

      if (astart < 0) astart += (float)360;
      if (aend < 0) aend += (float)360;
      if (diff && aend == 0) aend = (float)360;

      startWrite();
      if (astart > aend) {
        fill_arc_helper(x, y, r1, 1, astart, 360);
        fill_arc_helper(x, y, r1, 1, 0, aend);
        fill_arc_helper(x, y, r2, 1, astart, 360);
        fill_arc_helper(x, y, r2, 1, 0, aend);
      } else {
        fill_arc_helper(x, y, r1, 1, astart, aend);
        fill_arc_helper(x, y, r2, 1, astart, aend);
      }
      if (--r1 > ++r2) {
        int32_t cos_tmp = cos(astart * DEG_TO_RAD) * (1 << FP_SCALE);
        int32_t sin_tmp = sin(astart * DEG_TO_RAD) * (1 << FP_SCALE);
//        for ( int r = r2; r < r1; ++r) {
//          drawPixel(x + (r * cos_tmp+(1<<(FP_SCALE - 1)) >> FP_SCALE), y + (r * sin_tmp+(1<<(FP_SCALE - 1)) >> FP_SCALE));
//        }
        drawLine(x + ((r1 * cos_tmp+(1<<(FP_SCALE-1))) >> FP_SCALE), y + ((r1 * sin_tmp+(1<<(FP_SCALE-1))) >> FP_SCALE),
                 x + ((r2 * cos_tmp+(1<<(FP_SCALE-1))) >> FP_SCALE), y + ((r2 * sin_tmp+(1<<(FP_SCALE-1))) >> FP_SCALE));
        cos_tmp = cos(aend * DEG_TO_RAD) * (1 << FP_SCALE);
        sin_tmp = sin(aend * DEG_TO_RAD) * (1 << FP_SCALE);
//        for ( int r = r2; r < r1; ++r) {
//          drawPixel(x + (r * cos_tmp+(1<<(FP_SCALE - 1)) >> FP_SCALE), y + (r * sin_tmp+(1<<(FP_SCALE - 1)) >> FP_SCALE));
//        }
        drawLine(x + ((r1 * cos_tmp+(1<<(FP_SCALE-1))) >> FP_SCALE), y + ((r1 * sin_tmp+(1<<(FP_SCALE-1))) >> FP_SCALE),
                 x + ((r2 * cos_tmp+(1<<(FP_SCALE-1))) >> FP_SCALE), y + ((r2 * sin_tmp+(1<<(FP_SCALE-1))) >> FP_SCALE));
      }
      endWrite();
    }

    void fillArc(int32_t x, int32_t y, int32_t r1, int32_t r2, float start, float end)
    {
      if (r1 < r2) std::swap(r1, r2);
      if (r1 < 1) r1 = 1;
      if (r2 < 1) r2 = 1;

      bool diff = start > end || start < end;
      float astart = fmodf(start, 360);
      float aend = fmodf(end, 360);

      if (astart < 0) astart += (float)360;
      if (aend < 0) aend += (float)360;
      if (diff && aend == 0) aend = (float)360;

      int32_t th = r1 - r2 + 1;
      startWrite();
      if (astart > aend) {
        fill_arc_helper(x, y, r1, th, astart, 360);
        fill_arc_helper(x, y, r1, th, 0, aend);
      } else {
        fill_arc_helper(x, y, r1, th, astart, aend);
      }
      endWrite();
    }

    template<typename T> void drawBitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, const T& color                    ) { draw_bitmap(x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawBitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, const T& fgcolor, const T& bgcolor) { draw_bitmap(x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }
    template<typename T> void drawXBitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, const T& color                    ) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawXBitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, const T& fgcolor, const T& bgcolor) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }

    void draw_bitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, uint32_t fg_rawcolor, uint32_t bg_rawcolor = ~0)
    {
      if (w < 1 || h < 1) return;
      setRawColor(fg_rawcolor);
      int32_t byteWidth = (w + 7) >> 3;
      uint_fast8_t byte = 0;

      bool fg = true;
      int32_t j = 0;
      startWrite();
      do {
        int32_t i = 0;
        do {
          int32_t ip = i;
          for (;;) {
            if (!(i & 7)) byte = pgm_read_byte(&bitmap[i >> 3]);
            if (fg != (bool)(byte & 0x80) || (++i >= w)) break;
            byte <<= 1;
          }
          if ((ip != i) && (fg || bg_rawcolor != ~0)) {
            writeFastHLine(x + ip, y + j, i - ip);
          }
          fg = !fg;
          if (bg_rawcolor != ~0) setRawColor(fg ? fg_rawcolor : bg_rawcolor);
        } while (i < w);
        bitmap += byteWidth;
      } while (++j < h);
      endWrite();
    }

    void draw_xbitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, uint32_t fg_rawcolor, uint32_t bg_rawcolor = ~0)
    {
      if (w < 1 || h < 1) return;
      setRawColor(fg_rawcolor);
      int32_t byteWidth = (w + 7) >> 3;
      uint_fast8_t byte = 0;

      bool fg = true;
      int32_t j = 0;
      startWrite();
      do {
        int32_t i = 0;
        do {
          int32_t ip = i;
          for (;;) {
            if (!(i & 7)) byte = pgm_read_byte(&bitmap[i >> 3]);
            if (fg != (bool)(byte & 0x01) || (++i >= w)) break;
            byte >>= 1;
          }
          if ((ip != i) && (fg || bg_rawcolor != ~0)) {
            writeFastHLine(x + ip, y + j, i - ip);
          }
          fg = !fg;
          if (bg_rawcolor != ~0) setRawColor(fg ? fg_rawcolor : bg_rawcolor);
        } while (i < w);
        bitmap += byteWidth;
      } while (++j < h);
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
      uint_fast16_t data = 0;
      read_rect(x, y, 1, 1, &data, &p);
      return __builtin_bswap16(data);
    }

    __attribute__ ((always_inline)) inline
    void readRectRGB( int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* data) { readRectRGB(x, y, w, h, (bgr888_t*)data); }
    void readRectRGB( int32_t x, int32_t y, int32_t w, int32_t h, bgr888_t* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, _palette);
      read_rect(x, y, w, h, data, &p);
    }

    template<typename T> inline
    void readRect( int32_t x, int32_t y, int32_t w, int32_t h, T* data)
    {
      pixelcopy_t p(nullptr, T::depth, _read_conv.depth, false, _palette);
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<T>(_read_conv.depth); }
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
      if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<rgb565_t>(_read_conv.depth);
      }
      read_rect(x, y, w, h, data, &p);
    }
    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, void* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, _palette);
      if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<rgb888_t>(_read_conv.depth);
      }
      read_rect(x, y, w, h, data, &p);
    }

    template<typename T> void pushRect(  int32_t x, int32_t y, int32_t w, int32_t h, const T* data) { pushImage(x, y, w, h, data); }
    template<typename T> void pushImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data) {
      pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr);
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_normalcopy<T>(_write_conv.depth); }
      push_image(x, y, w, h, &p);
    }
    template<typename T> void pushImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data, const T& transparent) {
      pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr, _write_conv.convert(transparent));
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_normalcopy<T>(_write_conv.depth); }
      if (p.fp_skip==nullptr) { p.fp_skip = pixelcopy_t::normalskip<T>; }
      push_image(x, y, w, h, &p);
    }
    template<typename T> void pushImage( int32_t x, int32_t y, int32_t w, int32_t h, const void* data                      , const uint8_t bits, const T* palette) {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette              );
      p.fp_copy = pixelcopy_t::get_fp_palettecopy<T>(_write_conv.depth);
      push_image(x, y, w, h, &p);
    }
    template<typename T> void pushImage( int32_t x, int32_t y, int32_t w, int32_t h, const void* data, uint32_t transparent, const uint8_t bits, const T* palette) {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette, transparent );
      p.fp_copy = pixelcopy_t::get_fp_palettecopy<T>(_write_conv.depth);
      push_image(x, y, w, h, &p);
    }

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
    template<typename T> void pushImageDMA( int32_t x, int32_t y, int32_t w, int32_t h, const void* data                      , const uint8_t bits, const T* palette) { pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette              ); push_image(x, y, w, h, &p, true); }
    template<typename T> void pushImageDMA( int32_t x, int32_t y, int32_t w, int32_t h, const void* data, uint32_t transparent, const uint8_t bits, const T* palette) { pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette, transparent ); push_image(x, y, w, h, &p, true); }

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
//      uint32_t x_mask = (1 << (4 - __builtin_ffs(param->src_bits))) - 1;
//      uint32_t x_mask = (1 << ((~(param->src_bits>>1)) & 3)) - 1;
        uint32_t x_mask = (param->src_bits == 1) ? 7
                        : (param->src_bits == 2) ? 3
                                                 : 1;
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

    bool pushImageRotateZoom(int32_t dst_x, int32_t dst_y, const void* data, int32_t src_x, int32_t src_y, int32_t w, int32_t h, float angle, float zoom_x, float zoom_y, uint32_t transparent, const uint8_t bits, const bgr888_t* palette) {
      if (nullptr == data) return false;
      if (zoom_x == 0.0 || zoom_y == 0.0) return true;
      pixelcopy_t pc(data, getColorDepth(), (color_depth_t)bits, hasPalette(), palette, transparent );
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, w, h, angle, zoom_x, zoom_y, &pc);
      return true;
    }

    void push_image_rotate_zoom(int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, int32_t w, int32_t h, float angle, float zoom_x, float zoom_y, pixelcopy_t *param)
    {
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
//      uint32_t x_mask = (1 << (4 - __builtin_ffs(param->src_bits))) - 1;
//      uint32_t x_mask = (1 << ((~(param->src_bits>>1)) & 3)) - 1;
        uint32_t x_mask = (param->src_bits == 1) ? 7
                        : (param->src_bits == 2) ? 3
                                                 : 1;
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

    void getScrollRect(int32_t *x, int32_t *y, int32_t *w, int32_t *h) {
      *x = _sx;
      *y = _sy;
      *w = _sw;
      *h = _sh;
    }

    void scroll(int_fast16_t dx, int_fast16_t dy = 0)
    {
      _color.raw = _scolor;
      int32_t absx = abs(dx);
      int32_t absy = abs(dy);
      if (absx >= _sw || absy >= _sh) {
        writeFillRect(_sx, _sy, _sw, _sh);
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

      if (     dx > 0) writeFillRect(_sx           , dst_y,  dx, h);
      else if (dx < 0) writeFillRect(_sx + _sw + dx, dst_y, -dx, h);
      if (     dy > 0) writeFillRect(_sx, _sy           , _sw,  dy);
      else if (dy < 0) writeFillRect(_sx, _sy + _sh + dy, _sw, -dy);
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
        if (_utf8) {
          uniCode = decodeUTF8(utf8);
          if (uniCode < 32) return 1;
        }
        if (!(fpUpdateFontSize)(this, uniCode)) return 1;

        if (0 == _font_size_x.size) return 1;

        int16_t w  = _font_size_x.size    * _textsize_x;
        int16_t xo = _font_size_x.offset  * _textsize_x;
        int16_t h  = _font_size_y.size    * _textsize_y;
        int16_t yo = _font_size_y.offset  * _textsize_y;
        if (_textscroll || _textwrap_x) {
          int32_t left = _textscroll ? _sx : 0;
          if (_cursor_x < left - xo) _cursor_x = left - xo;
          else {
            int32_t right = _textscroll ? _sx + _sw : _width;
            if (_cursor_x + xo + w > right) {
              _filled_x = 0;
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
        } else if (_textwrap_y) {
          if (_cursor_y + yo + h > _height) {
            _filled_x = 0;
            _cursor_x = - xo;
            _cursor_y = - yo;
          } else
          if (_cursor_y < - yo) _cursor_y = - yo;
        }
//      _cursor_x += drawChar(uniCode, _cursor_x, _cursor_y);
        _cursor_x += (fpDrawChar)(this, _cursor_x, _cursor_y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y, &fontdata[_textfont]);
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
        if (_utf8) {
          do {
            uniCode = decodeUTF8(*string);
          } while (uniCode < 32 && *(++string));
          if (uniCode < 32) break;
        }
        if (!(fpUpdateFontSize)(this, uniCode)) continue;
        if (left == 0 && right == 0 && _font_size_x.offset < 0) left = right = -_font_size_x.offset;
        right = left + std::max((int32_t)_font_size_x.advance, _font_size_x.size - _font_size_x.offset);
        left += _font_size_x.advance;
      } while (*(++string));

      return right * _textsize_x;
    }

  #if defined (ARDUINO)
    inline int32_t textWidth(const String& string) { return textWidth(string.c_str()); }

    inline int_fast16_t drawString(const String& string, int32_t x, int32_t y) { return draw_string(string.c_str(), x, y, _textdatum); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline int_fast16_t drawCentreString(const String& string, int32_t x, int32_t y, uint8_t font) { setTextFont(font); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline int_fast16_t drawCenterString(const String& string, int32_t x, int32_t y, uint8_t font) { setTextFont(font); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline int_fast16_t drawRightString( const String& string, int32_t x, int32_t y, uint8_t font) { setTextFont(font); return draw_string(string.c_str(), x, y, textdatum_t::top_right); }

    inline int_fast16_t drawString(const String& string, int32_t x, int32_t y, uint8_t font) { setTextFont(font); return draw_string(string.c_str(), x, y, _textdatum); }

  #endif
    inline int_fast16_t drawString(const char *string, int32_t x, int32_t y) { return draw_string(string, x, y, _textdatum); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline int_fast16_t drawCentreString(const char *string, int32_t x, int32_t y, uint8_t font) { setTextFont(font); return draw_string(string, x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline int_fast16_t drawCenterString(const char *string, int32_t x, int32_t y, uint8_t font) { setTextFont(font); return draw_string(string, x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline int_fast16_t drawRightString( const char *string, int32_t x, int32_t y, uint8_t font) { setTextFont(font); return draw_string(string, x, y, textdatum_t::top_right); }

    inline int_fast16_t drawString(const char *string, int32_t x, int32_t y, uint8_t font) { setTextFont(font); return draw_string(string, x, y, _textdatum); }

    inline int_fast16_t drawNumber(long long_num, int32_t poX, int32_t poY, uint8_t font) { setTextFont(font); return drawNumber(long_num, poX, poY); }

    inline int_fast16_t drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY, uint8_t font) { setTextFont(font); return drawFloat(floatNumber, dp, poX, poY); }

    int_fast16_t drawNumber(long long_num, int32_t poX, int32_t poY)
    {
      constexpr size_t len = 8 * sizeof(long) + 1;
      char buf[len];
      return drawString(numberToStr(long_num, buf, len, 10), poX, poY);
    }

    int_fast16_t drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY)
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
          _unicode_buffer = ((c & 0x1F)<<6);
          _decoderState = utf8_decode_state_t::utf8_state1;
          return 0;
        }

        // 16 bit Unicode Code Point
        if ((c & 0xF0) == 0xE0)
        {
          _unicode_buffer = ((c & 0x0F)<<12);
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
          _unicode_buffer |= ((c & 0x3F)<<6);
          _decoderState = utf8_decode_state_t::utf8_state1;
          return 0;
        }
        _unicode_buffer |= (c & 0x3F);
        _decoderState = utf8_decode_state_t::utf8_state0;
        return _unicode_buffer;
      }

      _decoderState = utf8_decode_state_t::utf8_state0;

      return (uint16_t)c; // fall-back to extended ASCII
    }

    template<typename T>
    void setTextColor(T color) {
      if (hasPalette()) {
        _text_fore_rgb888 = _text_back_rgb888 = color;
      } else {
        _text_fore_rgb888 = _text_back_rgb888 = convert_to_rgb888(color);
      }
    }
    template<typename T1, typename T2>
    void setTextColor(T1 fgcolor, T2 bgcolor) {
      if (hasPalette()) {
        _text_fore_rgb888 = fgcolor;
        _text_back_rgb888 = bgcolor;
      } else {
        _text_fore_rgb888 = convert_to_rgb888(fgcolor);
        _text_back_rgb888 = convert_to_rgb888(bgcolor);
      }
    }

    inline int_fast16_t drawChar(uint16_t uniCode, int32_t x, int32_t y) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y, &fontdata[_textfont]); }

    int_fast16_t drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font) {
      if (font == _textfont) return drawChar(uniCode, x, y);
      _filled_x = 0;
      switch (pgm_read_byte( &fontdata[font].type)) {
      default:
      case font_type_t::ft_glcd: return drawCharGLCD(this, x, y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y, &fontdata[font]);
      case font_type_t::ft_bmp:  return drawCharBMP( this, x, y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y, &fontdata[font]);
      case font_type_t::ft_rle:  return drawCharRLE( this, x, y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y, &fontdata[font]);
      case font_type_t::ft_bdf:  return drawCharBDF( this, x, y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y, &fontdata[font]);
      }
    }

    template<typename T>
    inline int_fast16_t drawChar(int32_t x, int32_t y, uint16_t uniCode, T color, T bg, int_fast8_t size) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, convert_to_rgb888(color), convert_to_rgb888(bg), size, size, &fontdata[_textfont]); }
    template<typename T>
    inline int_fast16_t drawChar(int32_t x, int32_t y, uint16_t uniCode, T color, T bg, int_fast8_t size_x, int_fast8_t size_y) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, convert_to_rgb888(color), convert_to_rgb888(bg), size_x, size_y, &fontdata[_textfont]); }

    int16_t getCursorX(void) const { return _cursor_x; }
    int16_t getCursorY(void) const { return _cursor_y; }
    void setCursor( int16_t x, int16_t y)               { _filled_x = 0; _cursor_x = x; _cursor_y = y; }
    void setCursor( int16_t x, int16_t y, uint8_t font) { _filled_x = 0; _cursor_x = x; _cursor_y = y; _textfont = font; }
    void setTextSize(uint8_t s) { setTextSize(s,s); }
    void setTextSize(uint8_t sx, uint8_t sy) { _textsize_x = (sx > 0) ? sx : 1; _textsize_y = (sy > 0) ? sy : 1; }
    int16_t getTextSizeX(void) const { return _textsize_x; }
    int16_t getTextSizeY(void) const { return _textsize_y; }
    int16_t fontHeight(void) const { return _font_size_y.size * _textsize_y; }
    int16_t fontHeight(uint8_t font) const { if (_textfont == font) return fontHeight(); return pgm_read_byte( &fontdata[font].height ); }
    textdatum_t getTextDatum(void) const { return _textdatum; }

    void setTextDatum(uint8_t datum) { _textdatum = (textdatum_t)datum; }
    void setTextDatum(textdatum_t datum) { _textdatum = datum; }
    void setTextPadding(uint16_t padding_x) { _padding_x = padding_x; }
    void setTextWrap( bool wrapX, bool wrapY = false) { _textwrap_x = wrapX; _textwrap_y = wrapY; }
    void setTextScroll(bool scroll) { _textscroll = scroll; if (_cursor_x < _sx) { _cursor_x = _sx; } if (_cursor_y < _sy) { _cursor_y = _sy; } }

    void setTextEFont() { setTextFont(3); }

    virtual void setTextFont(uint8_t f) {
      _decoderState = utf8_decode_state_t::utf8_state0;
      _filled_x = 0; 
      _font_size_x.offset = 0;
      _font_size_y.offset = 0;

      _textfont = f;
      _font_size_y.size = _font_size_y.advance = pgm_read_byte( &fontdata[f].height );
      _font_baseline = pgm_read_byte( &fontdata[f].baseline );

      switch (pgm_read_byte( &fontdata[f].type)) {
      default:
      case font_type_t::ft_glcd:
        fpDrawChar = drawCharGLCD;
        fpUpdateFontSize = updateFontSizeGLCD;
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
      case font_type_t::ft_bdf:
        fpDrawChar = drawCharBDF;
        fpUpdateFontSize = updateFontSizeBDF;
        break;
      }
    }

    void cp437(bool enable = true) { _cp437 = enable; }  // AdafruitGFX compatible.

    void setAttribute(uint8_t attr_id, uint8_t param) { setAttribute((attribute_t)attr_id, param); }
    void setAttribute(attribute_t attr_id, uint8_t param) {
      switch (attr_id) {
        case cp437_switch:
            _cp437 = param;
            break;
        case utf8_switch:
            _utf8  = param;
            _decoderState = utf8_decode_state_t::utf8_state0;
            break;
        default: break;
      }
    }

    uint8_t getAttribute(uint8_t attr_id) { return getAttribute((attribute_t)attr_id); }
    uint8_t getAttribute(attribute_t attr_id) {
      switch (attr_id) {
        case cp437_switch: return _cp437;
        case utf8_switch: return _utf8;
        default: return 0;
      }
    }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    uint32_t _transaction_count = 0;
    int32_t _width = 0, _height = 0;
    int32_t  _sx, _sy, _sw, _sh; // for scroll zone

    int32_t _clip_l = 0, _clip_t = 0, _clip_r = 0, _clip_b = 0; // clip rect
    int32_t _cursor_x = 0, _cursor_y = 0, _filled_x = 0;
    uint32_t _text_fore_rgb888 = 0xFFFFFFU;
    uint32_t _text_back_rgb888 = 0;
    uint32_t _scolor;  // gap fill colour for scroll zone
    raw_color_t _color = 0xFFFFFFU;

    color_conv_t _write_conv;
    color_conv_t _read_conv;

    bgr888_t* _palette = nullptr; // for sprite palette mode.
    uint32_t _palette_count = 0;

    int16_t _textsize_x = 1;
    int16_t _textsize_y = 1;
    int16_t _xpivot;   // x pivot point coordinate
    int16_t _ypivot;   // x pivot point coordinate
    uint16_t _unicode_buffer = 0;   // Unicode code-point buffer
    uint8_t _textfont = 1;
    textdatum_t _textdatum;
    int16_t _padding_x = 0;

    bool _spi_shared = true;
    bool _swapBytes = false;
    bool _textwrap_x = true;
    bool _textwrap_y = false;
    bool _textscroll = false;
    bool _cp437      = false;
    bool _utf8       = true;

    enum utf8_decode_state_t
    { utf8_state0 = 0
    , utf8_state1 = 1
    , utf8_state2 = 2
    };
    utf8_decode_state_t _decoderState = utf8_state0;   // UTF8 decoder state

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

    void fill_arc_helper(int32_t cx, int32_t cy, int32_t radius, int32_t thickness, float start, float end)
    {
      float sslope = (cos(start * DEG_TO_RAD)) / (sin(start * DEG_TO_RAD));
    //float eslope = (cos(end   * DEG_TO_RAD)) / (sin(end   * DEG_TO_RAD));
      float eslope = -1000000;
      if (end != 360.0) eslope = (cos(end * DEG_TO_RAD)) / (sin(end * DEG_TO_RAD));

      int ir2 = (radius - thickness) * (radius - thickness) + (radius - thickness);
      int or2 = radius * radius + radius;

      int y = -radius;
      do {
        int y2 = y * y;
        int ysslope = y * sslope;
        int yeslope = y * eslope;
        int len = 0;
        int x = -radius;
        do {
          int distance = x * x + y2;
          if ((distance < or2 && distance >= ir2)
            && ( (y >= 0 && (start < 180 && x <= ysslope)
                         && ( end >= 180 || x >= yeslope)
                 )
              || (y < 0 && (start <= 180 || x >= ysslope)
                        && (   end > 180 && x <= yeslope)
                 )
//            || (y == 0 && ((start == 0 && x > 0) || (x < 0 && start <= 180 && end >= 180)))
              )
            ) {
            ++len;
          } else if (len) {
            writeFastHLine(cx + x - len, cy + y, len);
            len = 0;
          }
        } while (++x <= radius + 1);
      } while (++y <= radius);
    }

    virtual void beginTransaction_impl(void) = 0;
    virtual void endTransaction_impl(void) = 0;
    virtual void waitDMA_impl(void) = 0;

    virtual void drawPixel_impl(int32_t x, int32_t y) = 0;
    virtual void writeFillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) = 0;
    virtual void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) = 0;
    virtual void readRect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) = 0;
    virtual void pushImage_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param, bool use_dma) = 0;
    virtual void pushColors_impl(int32_t length, pixelcopy_t* param) = 0;
    virtual void pushBlock_impl(int32_t len) = 0;
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

    int_fast16_t draw_string(const char *string, int32_t x, int32_t y, textdatum_t datum)
    {
      int16_t sumX = 0;
      int32_t cwidth = textWidth(string); // Find the pixel width of the string in the font
      int32_t cheight = _font_size_y.size * _textsize_y;

      {
        auto tmp = string;
        do {
          uint16_t uniCode = *tmp;
          if (_utf8) {
            do {
              uniCode = decodeUTF8(*tmp); 
            } while (uniCode < 32 && *++tmp);
            if (uniCode < 32) break;
          }
          if ((fpUpdateFontSize)(this, uniCode)) {
            if (_font_size_x.offset < 0) sumX = - _font_size_x.offset * _textsize_x;
            break;
          }
        } while (*++tmp);
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
          writeFillRect(x - halfpadx, y, halfpadx - halfcwidth, cheight);
          halfcwidth = cwidth - halfcwidth;
          halfpadx = padx - halfpadx;
          writeFillRect(x + halfcwidth, y, halfpadx - halfcwidth, cheight);
        } else if (datum & top_right) {
          writeFillRect(x - padx, y, padx - cwidth, cheight);
        } else {
          writeFillRect(x + cwidth, y, padx - cwidth, cheight);
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
        if (_utf8) {
          do {
            uniCode = decodeUTF8(*string);
          } while (uniCode < 32 && *++string);
          if (uniCode < 32) break;
        }
        sumX += (fpDrawChar)(this, x + sumX, y, uniCode, _text_fore_rgb888, _text_back_rgb888, _textsize_x, _textsize_y, &fontdata[_textfont]);
      } while (*(++string));
      endWrite();

      return sumX;
    }

    bool(*fpUpdateFontSize)(LGFXBase* me, uint16_t uniCode) = updateFontSizeGLCD;

    static bool updateFontSizeGLCD(LGFXBase* me, uint16_t uniCode) {
      return uniCode < 256;
    }

    static bool updateFontSizeBMP(LGFXBase* me, uint16_t uniCode) {
      if ((uniCode -= 32) >= 96) return false;
      me->_font_size_x.advance = me->_font_size_x.size = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[me->_textfont].widthtbl ) ) + uniCode );
      return true;
    }

    static bool updateFontSizeBDF(LGFXBase* me, uint16_t uniCode) {
      auto size = me->_font_size_y.size;
      me->_font_size_x.size = me->_font_size_x.advance = (uniCode < 0x0100) ? size >> 1 : size;
      return true;
    }

    int_fast16_t (*fpDrawChar)(LGFXBase* me, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, int_fast8_t size_x, int_fast8_t size_y, const fontinfo* font) = &LGFXBase::drawCharGLCD;

    static int_fast16_t drawCharGLCD(LGFXBase* me, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, int_fast8_t size_x, int_fast8_t size_y, const fontinfo* fontdat)
    { // glcd font
      if (c > 255) return 0;

      if (!me->_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

      const int32_t fontWidth  = me->_font_size_x.size;
      const int32_t fontHeight = me->_font_size_y.size;

      auto font_addr = fontdat->chartbl + (c * 5);
      uint32_t colortbl[2] = {me->_write_conv.convert(back_rgb888), me->_write_conv.convert(fore_rgb888)};
      bool fillbg = (back_rgb888 != fore_rgb888);

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + fontWidth * size_x ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * size_y ))) {
        if (!fillbg || size_y > 1 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * size_x > clip_right) {
          int32_t xpos = x;
          me->startWrite();

          int_fast8_t i = 0;
          do {
            int_fast16_t ypos = y;
            uint8_t line = pgm_read_byte(&font_addr[i]);
            uint8_t flg = (line & 0x01);
            int_fast8_t j = 1;
            int_fast8_t jp = 0;
            do {
              while (flg == ((line >> j) & 0x01) && ++j < fontHeight);
              jp = j - jp;
              if (flg || fillbg) {
                me->setRawColor(colortbl[flg]);
                me->writeFillRect(xpos, ypos, size_x, jp * size_y);
              }
              ypos += jp * size_y;
              flg = !flg;
              jp = j;
            } while (j < fontHeight);
            xpos += size_x;
          } while (++i < fontWidth - 1);

          if (fillbg) {
            me->setRawColor(colortbl[0]);
            me->writeFillRect(xpos, y, size_x, fontHeight * size_y); 
          }
          me->endWrite();
        } else {
          uint8_t col[fontWidth];
          int_fast8_t i = 0;
          do {
            col[i] = pgm_read_byte(&font_addr[i]);
          } while (++i < 5);
          col[5] = 0;
          me->startWrite();
          me->setAddrWindow(x, y, fontWidth * size_x, fontHeight);
          uint8_t flg = col[0] & 1;
          uint32_t len = 0;
          i = 0;
          do {
            int_fast8_t j = 0;
            do {
              if (flg != ((col[j] >> i) & 1)) {
                me->writeRawColor(colortbl[flg], len);
                len = 0;
                flg = !flg;
              }
              len += size_x;
            } while (++j < fontWidth);
          } while (++i < fontHeight);
          me->writeRawColor(colortbl[0], len);
          me->endWrite();
        }
      }
      return fontWidth * size_x;
    }

    static int_fast16_t drawCharBMP(LGFXBase* me, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, int_fast8_t size_x, int_fast8_t size_y, const fontinfo* fontdat)
    { // BMP font
      if ((c < 32) || (c > 127)) return 0;
      uint16_t uniCode = c - 32;

      const int_fast8_t fontWidth = pgm_read_byte(fontdat->widthtbl + uniCode);
      const int_fast8_t fontHeight = pgm_read_byte(&fontdat->height);

      auto font_addr = (const uint8_t*)pgm_read_dword(&((const uint8_t**)fontdat->chartbl)[uniCode]);
      return draw_char_bmp(me, x, y, fore_rgb888, back_rgb888, size_x, size_y, font_addr, fontWidth, fontHeight, (fontWidth + 6) >> 3, 1);
    }

    static int_fast16_t drawCharBDF(LGFXBase* me, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, int_fast8_t size_x, int_fast8_t size_y, const fontinfo* fontdat)
    {
      const int_fast8_t bytesize = 2;
      const int_fast8_t fontHeight = fontdat->height;
      const int_fast8_t fontWidth = (c < 0x0100) ? fontHeight >> 1 : fontHeight;
      auto it = std::lower_bound(fontdat->indextbl, &fontdat->indextbl[fontdat->indexsize], c);
      if (*it != c) {
        if (fore_rgb888 != back_rgb888) {
          me->fillRect(x, y, fontWidth * size_x, fontHeight * size_y, back_rgb888);
        }
        return fontWidth * size_x;
      }
      const uint8_t* font_addr = &fontdat->chartbl[std::distance(fontdat->indextbl, it) * fontHeight * bytesize];
      return LGFXBase::draw_char_bmp(me, x, y, fore_rgb888, back_rgb888, size_x, size_y, font_addr, fontWidth, fontHeight, bytesize, 0);
    }

    static int_fast16_t draw_char_bmp(LGFXBase* me, int32_t x, int32_t y, uint32_t fore_rgb888, uint32_t back_rgb888, int_fast8_t size_x, int_fast8_t size_y, const uint8_t* font_addr, int_fast8_t fontWidth, int_fast8_t fontHeight, int_fast8_t w, int_fast8_t margin )
    {
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
            me->setRawColor(colortbl[0]);
            if (margin)
              me->writeFillRect(x + (fontWidth - margin) * size_x, y, size_x, fontHeight * size_y);
          }
          int_fast8_t i = 0;
          do {
            uint8_t line = pgm_read_byte(font_addr);
            bool flg = line & 0x80;
            int_fast8_t len = 1;
            int_fast8_t j = 1;
            int_fast8_t je = fontWidth - margin;
            do {
              if (j & 7) {
                line <<= 1;
              } else {
                line = pgm_read_byte(&font_addr[j >> 3]);
              }
              if (flg != (bool)(line & 0x80)) {
                if (flg || fillbg) {
                  me->setRawColor(colortbl[flg]);
                  me->writeFillRect( x + (j - len) * size_x, y, len * size_x, size_y); 
                }
                len = 1;
                flg = !flg;
              } else {
                ++len;
              }
            } while (++j < je);
            if (flg || fillbg) {
              me->setRawColor(colortbl[flg]);
              me->writeFillRect( x + (j - len) * size_x, y, len * size_x, size_y); 
            }
            y += size_y;
            font_addr += w;
          } while (++i < fontHeight);
          me->endWrite();
        } else {
          int_fast8_t len = 0;
          uint8_t line = 0;
          bool flg = false;
          me->startWrite();
          me->setAddrWindow(x, y, fontWidth * size_x, fontHeight);
          int_fast8_t i = 0;
          int_fast8_t je = fontWidth - margin;
          do {
            int_fast8_t j = 0;
            do {
              if (j & 7) {
                line <<= 1;
              } else {
                line = (j == je) ? 0 : pgm_read_byte(&font_addr[j >> 3]);
              }
              if (flg != (bool)(line & 0x80)) {
                me->writeRawColor(colortbl[flg], len);
                flg = !flg;
                len = 0;
              }
              len += size_x;
            } while (++j < fontWidth);
            font_addr += w;
          } while (++i < fontHeight);
          me->writeRawColor(colortbl[flg], len);
          me->endWrite();
        }
      }

      return fontWidth * size_x;
    }

    static int_fast16_t drawCharRLE(LGFXBase* me, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, int_fast8_t size_x, int_fast8_t size_y, const fontinfo* fontdat)
    { // RLE font
      if ((c < 32) || (c > 127)) return 0;
      uint16_t code = c - 32;
      const int fontWidth = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdat->widthtbl ) ) + code );
      const int fontHeight = pgm_read_byte( &fontdat->height );

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
                me->setRawColor(colortbl[flg]);
                me->writeFillRect( x + j * size_x, y + (i * size_y), len * size_x, size_y);
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
          me->setAddrWindow(x, y, fontWidth * size_x, fontHeight);
          uint32_t len = fontWidth * size_x * fontHeight;
          do {
            line = pgm_read_byte(font_addr++);
            bool flg = line & 0x80;
            line = ((line & 0x7F) + 1) * size_x;
            me->writeRawColor(colortbl[flg], line);
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

    static int_fast16_t drawCharGFXFF(LGFXBase* lgfxbase, int32_t x, int32_t y, uint16_t c, uint32_t fore_rgb888, uint32_t back_rgb888, int_fast8_t size_x, int_fast8_t size_y, const fontinfo* fontdat)
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
      int32_t left  = 0;
      int32_t right = 0;
      if (fillbg) {
        left  = std::max(me->_filled_x, x + (xoffset < 0 ? xoffset : 0));
        right = x + std::max((int32_t)(w * size_x + xoffset), (int32_t)(xAdvance));
      }

      x += xoffset;
      y += yoffset;

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + w * size_x ))
       && (y <= clip_bottom) && (clip_top < (y + h * size_y ))) {

        if (right > left) {
          me->setRawColor(colortbl[0]);
          int tmp = yoffset - (me->_font_size_y.offset * size_y);
          if (tmp > 0)
            me->writeFillRect(left, y - yoffset + me->_font_size_y.offset * size_y, right - left, tmp);

          tmp = (me->_font_size_y.offset + me->_font_size_y.size - h) * size_y - yoffset;
          if (tmp > 0)
            me->writeFillRect(left, y + h * size_y, right - left, tmp);
        }

        uint8_t  *bitmap = (uint8_t *)pgm_read_dword(&gfxFont->bitmap)
                         + pgm_read_word(&glyph->bitmapOffset);
        uint8_t bits=0, bit=0;

        me->setRawColor(colortbl[1]);
        while (h--) {
          if (right > left) {
            me->setRawColor(colortbl[0]);
            me->writeFillRect(left, y, right - left, size_y);
            me->setRawColor(colortbl[1]);
          }

          int32_t len = 0;
          int32_t i = 0;
          for (i = 0; i < w; i++) {
            if (bit == 0) {
              bit  = 0x80;
              bits = pgm_read_byte(bitmap++);
            }
            if (bits & bit) len++;
            else if (len) {
              me->writeFillRect(x + (i-len) * size_x, y, size_x * len, size_y);
              len=0;
            }
            bit >>= 1;
          }
          if (len) {
            me->writeFillRect(x + (i-len) * size_x, y, size_x * len, size_y);
          }
          y += size_y;
        }
      } else {
        if (right > left) {
          me->setRawColor(colortbl[0]);
          me->writeFillRect(left, y - yoffset + me->_font_size_y.offset * size_y, right - left, (me->_font_size_y.size) * size_y);
        }
      }
      me->_filled_x = right;
      me->endWrite();
      return xAdvance;
    }
  public:

    void setTextFont(uint8_t f) override {
      if (f == 1 && _gfxFont) return;
      _gfxFont = nullptr;
      Base::setTextFont(f);
    }

    void setFreeFont(const GFXfont *f = nullptr)
    {
      _gfxFont = f;
      if (f == nullptr) { this->setTextFont(1); return; } // Use GLCD font
      this->fpDrawChar = drawCharGFXFF;
      this->fpUpdateFontSize = updateFontSizeGFXFF;

      this->_textfont = 1;
      this->_decoderState = Base::utf8_decode_state_t::utf8_state0;

      int_fast8_t glyph_ab = 0;   // glyph delta Y (height) above baseline
      int_fast8_t glyph_bb = 0;   // glyph delta Y (height) below baseline
      uint16_t numChars = pgm_read_word(&_gfxFont->last) - pgm_read_word(&_gfxFont->first);
      
      // Find the biggest above and below baseline offsets
      for (uint8_t c = 0; c < numChars; c++)
      {
        GFXglyph *glyph1 = &(((GFXglyph *)pgm_read_dword(&_gfxFont->glyph))[c]);
        int8_t ab = -pgm_read_byte(&glyph1->yOffset);
        if (ab > glyph_ab) glyph_ab = ab;
        int8_t bb = pgm_read_byte(&glyph1->height) - ab;
        if (bb > glyph_bb) glyph_bb = bb;
      }

      this->_font_baseline = glyph_ab;
      this->_font_size_y.offset = - glyph_ab;
      this->_font_size_y.size = glyph_bb + glyph_ab;
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
      if (gUnicode)  { heap_free(gUnicode);  gUnicode  = nullptr; }
      if (gWidth)    { heap_free(gWidth);    gWidth    = nullptr; }
      if (gxAdvance) { heap_free(gxAdvance); gxAdvance = nullptr; }
      if (gdX)       { heap_free(gdX);       gdX       = nullptr; }
      if (gBitmap)   { heap_free(gBitmap);   gBitmap   = nullptr; }
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
      _fontFile.need_transaction &= this->isSPIShared();

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

    bool load_font(void)
    {
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

//ESP_LOGI("LGFX", "ascent:%d  descent:%d", gFont.ascent, gFont.descent);

      this->_decoderState = Base::utf8_decode_state_t::utf8_state0;
      this->fpDrawChar = drawCharVLW;
      this->fpUpdateFontSize = updateFontSizeVLW;
      this->_font_size_x.offset = 0;
      this->_font_size_y.offset = 0;

      _fontLoaded = true;
      // Fetch the metrics for each glyph
      bool res = loadMetrics(gFont.gCount);
      if (res) {
        this->_font_size_y.size    = gFont.yAdvance;
        this->_font_size_y.advance = gFont.yAdvance;
        return true;
      }
      unloadFont();
      return false;
    }

    bool loadMetrics(uint16_t gCount)
    {
      if (!gCount) return false;

//ESP_LOGI("LGFX", "font count:%d", gCount);

      uint32_t bitmapPtr = 24 + (uint32_t)gCount * 28;

      gBitmap   = (uint32_t*)heap_alloc_psram( gCount * 4); // seek pointer to glyph bitmap in the file
      gUnicode  = (uint16_t*)heap_alloc_psram( gCount * 2); // Unicode 16 bit Basic Multilingual Plane (0-FFFF)
      gWidth    =  (uint8_t*)heap_alloc_psram( gCount );    // Width of glyph
      gxAdvance =  (uint8_t*)heap_alloc_psram( gCount );    // xAdvance - to move x cursor
      gdX       =   (int8_t*)heap_alloc_psram( gCount );    // offset for bitmap left edge relative to cursor X

      if (!gUnicode
       || !gBitmap
       || !gWidth
       || !gxAdvance
       || !gdX) {
//ESP_LOGE("LGFX", "can not alloc font table");
        return false;
      }

      size_t gNum = 0;
      _fontData->seek(24);  // headerPtr
      uint32_t buffer[7];
      do {
        _fontData->read((uint8_t*)buffer, 7 * 4); // 28 Byte read
        uint16_t unicode = __builtin_bswap32(buffer[0]); // Unicode code point value
        uint32_t w = (uint8_t)__builtin_bswap32(buffer[2]); // Width of glyph
        if (gUnicode)   gUnicode[gNum]  = unicode;
        if (gWidth)     gWidth[gNum]    = w;
        if (gxAdvance)  gxAdvance[gNum] = (uint8_t)__builtin_bswap32(buffer[3]); // xAdvance - to move x cursor
        if (gdX)        gdX[gNum]       =  (int8_t)__builtin_bswap32(buffer[5]); // x delta from cursor

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

        if (gBitmap)  gBitmap[gNum] = bitmapPtr;
        bitmapPtr += w * height;
      } while (++gNum < gCount);

      this->_font_baseline = gFont.maxAscent;
      this->_font_size_y.advance = gFont.yAdvance = gFont.maxAscent + gFont.maxDescent;
//ESP_LOGI("LGFX", "maxDescent:%d", gFont.maxDescent);
      return true;
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
        if (me->gWidth && me->gxAdvance && me->gdX[gNum]) {
          me->_font_size_x.size    = me->gWidth[gNum];
          me->_font_size_x.advance = me->gxAdvance[gNum];
          me->_font_size_x.offset  = me->gdX[gNum];
        } else {
          auto file = me->_fontData;

          if (file->need_transaction && me->_transaction_count) me->endTransaction();

          file->seek(28 + gNum * 28);  // headerPtr
          uint32_t buffer[6];
          file->read((uint8_t*)buffer, 24);
          me->_font_size_x.size    = __builtin_bswap32(buffer[1]); // Width of glyph
          me->_font_size_x.advance = __builtin_bswap32(buffer[2]); // xAdvance - to move x cursor
          me->_font_size_x.offset  = (int32_t)((int8_t)__builtin_bswap32(buffer[4])); // x delta from cursor

          if (file->need_transaction && me->_transaction_count) me->beginTransaction();
        }
        return true;
      }
      return false;
    }

    static int_fast16_t drawCharVLW(LGFXBase* lgfxbase, int32_t x, int32_t y, uint16_t code, uint32_t fore_rgb888, uint32_t back_rgb888, int_fast8_t size_x, int_fast8_t size_y, const fontinfo*)
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
          me->setRawColor(colortbl[0]);
          me->writeFillRect(left, y, right - left, me->gFont.yAdvance * size_y);
 //me->setRawColor(colortbl[1]);
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

        if (fillbg) { // fill background mode

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
                  me->writeFillRect(i * size_x + x, y, size_x, size_y);
                }
                if (++i == right) break;
              }
              if (i == right) break;
              int32_t dl = 1;
              while (i + dl != right && pixel[i + dl] == 0xFF) { ++dl; }
              me->setRawColor(colortbl[1]);
              me->writeFillRect(x + i * size_x, y, dl * size_x, size_y);
              i += dl;
            } while (i != right);
            pixel += w;
            y += size_y;
          } while (--h);

        } else { // alpha blend mode

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
                int32_t sy = 0;
                do {
                  auto bgr = &buf[sx + sy * bw];
                  bgr->r = ( fore_r * p + bgr->r * (257 - p)) >> 8;
                  bgr->g = ( fore_g * p + bgr->g * (257 - p)) >> 8;
                  bgr->b = ( fore_b * p + bgr->b * (257 - p)) >> 8;
                } while (++sy < bh);
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
      FileWrapper file(fs);
      drawBmpFile(&file, path, x, y);
    }

    inline bool drawJpgFile( fs::FS &fs, const char *path, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      FileWrapper file(fs);
      return drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline bool drawPngFile( fs::FS &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, double scale = 1.0)
    {
      FileWrapper file(fs);
      return drawPngFile( &file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

 #endif
 #if defined (Stream_h)

    inline bool drawJpg(Stream *dataSource, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      StreamWrapper data;
      data.set(dataSource);
      return draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline void drawPng( Stream *dataSource, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, double scale = 1.0) {
      StreamWrapper data;
      data.set(dataSource);
      return draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

 #endif

#elif defined (CONFIG_IDF_TARGET_ESP32)  // ESP-IDF

    inline void drawBmpFile(const char *path, int32_t x, int32_t y) {
      FileWrapper file;
      drawBmpFile(&file, path, x, y);
    }
    inline bool drawJpgFile(const char *path, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      FileWrapper file;
      return drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }
    inline bool drawPngFile( const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, double scale = 1.0)
    {
      FileWrapper file;
      return drawPngFile( &file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

#endif

    void drawBmp(const uint8_t *bmp_data, uint32_t bmp_len, int32_t x=0, int32_t y=0) {
      PointerWrapper data;
      data.set(bmp_data, bmp_len);
      draw_bmp(&data, x, y);
    }
    bool drawJpg(const uint8_t *jpg_data, uint32_t jpg_len, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      PointerWrapper data;
      data.set(jpg_data, jpg_len);
      return draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }
    bool drawPng( const uint8_t *png_data, uint32_t png_len, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, double scale = 1.0)
    {
      PointerWrapper data;
      data.set(png_data, png_len);
      return draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

  protected:

    struct bitmap_header_t {
      union {
        uint8_t raw[54];
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
          uint32_t biSizeImage; 
          int32_t  biXPelsPerMeter;
          int32_t  biYPelsPerMeter;
          uint32_t biClrUsed; 
          uint32_t biClrImportant;
        };
        #pragma pack()
      };
    };

    bool load_bmp_header(DataWrapper* data, bitmap_header_t* result) {
      data->read((uint8_t*)result, sizeof(bitmap_header_t));
      return ((result->bfType == 0x4D42)   // bmp header "BM"
           && (result->biPlanes == 1)  // bcPlanes always 1
           && (result->biWidth > 0)
           && (result->biHeight > 0)
           && (result->biBitCount <= 32)
           && (result->biBitCount != 0));
    }

    bool load_bmp_rle8(DataWrapper* data, uint8_t* linebuf, uint_fast16_t width) {
      width = (width + 3) & ~3;
      uint8_t code[2];
      uint_fast16_t xidx = 0;
      bool eol = false;
      do {
        data->read(code, 2);
        if (code[0] == 0) {
          switch (code[1]) {
          case 0x00: // EOL
          case 0x01: // EOB
            eol = true;
            break;

          case 0x02: // move info  (not support)
            return false;

          default:
            data->read(&linebuf[xidx], (code[1] + 1) & ~1); // word align
            xidx += code[1];
            break;
          }
        } else if (xidx + code[0] <= width) {
          memset(&linebuf[xidx], code[1], code[0]);
          xidx += code[0];
        } else {
          return false;
        }
      } while (!eol);
      return true;
    }

    bool load_bmp_rle4(DataWrapper* data, uint8_t* linebuf, uint_fast16_t width) {
      width = (width + 3) & ~3;
      uint8_t code[2];
      uint_fast16_t xidx = 0;
      bool eol = false;
      do {
        data->read(code, 2);
        if (code[0] == 0) { // 1バイト目ぁE0 ならエンコードデータ以夁E
          switch (code[1]) {
          case 0x00: // EOL
          case 0x01: // EOB
            eol = true;
            break;

          case 0x02: // move info  (not support)
            return false;

          default:  // 絶対モードデータ
            {
              int_fast16_t len = code[1];
              int_fast16_t dbyte = ((int_fast16_t)code[1] + 1) >> 1;

              data->read(&linebuf[(xidx + 1) >> 1], (dbyte + 1) & ~1); // word align
              if (xidx & 1) {
                linebuf[xidx >> 1] |= linebuf[(xidx >> 1) + 1] >> 4;
                for (long i = 1; i < dbyte; ++i) {
                  linebuf[((xidx + i) >> 1)] = (linebuf[((xidx + i) >> 1)    ] << 4)
                                             |  linebuf[((xidx + i) >> 1) + 1] >> 4;
                }
              }
              xidx += len;
            }
            break;
          }
        } else if (xidx + code[0] <= width) {
          if (xidx & 1) {
            linebuf[xidx >> 1] |= code[1] >> 4;
            code[1] = (code[1] >> 4) | (code[1] << 4);
          }
          memset(&linebuf[(xidx + 1) >> 1], code[1], (code[0] + 1) >> 1);
          xidx += code[0];
          if (xidx & 1) linebuf[xidx >> 1] &= 0xF0;
        } else {
          return false;
        }
      } while (!eol);
      return true;
    }
//*/
  private:

    void drawBmpFile(FileWrapper* file, const char *path, int32_t x=0, int32_t y=0) {
      file->need_transaction &= this->isSPIShared();
      if (file->need_transaction) this->endTransaction();
      if (file->open(path, "rb")) {
        draw_bmp(file, x, y);
        file->close();
      }
      if (file->need_transaction && this->_transaction_count) { this->beginTransaction(); }
    }

    bool drawJpgFile(FileWrapper* file, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div_t scale) {
      bool res = false;
      file->need_transaction &= this->isSPIShared();
      if (file->need_transaction) this->endTransaction();
      if (file->open(path, "rb")) {
        res = draw_jpg(file, x, y, maxWidth, maxHeight, offX, offY, scale);
        file->close();
      }
//    if (file->need_transaction && this->_transaction_count) { this->beginTransaction(); }
      return res;
    }

    bool drawPngFile( FileWrapper* file, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, double scale)
    {
      bool res = false;
      file->need_transaction &= this->isSPIShared();
      if (file->need_transaction) this->endTransaction();
      if (file->open(path, "rb")) {
        res = draw_png(file, x, y, maxWidth, maxHeight, offX, offY, scale);
        file->close();
      }
      if (file->need_transaction && this->_transaction_count) { this->beginTransaction(); }
      return res;
    }

    void draw_bmp(DataWrapper* data, int32_t x, int32_t y) {
      if ((x >= this->_width) || (y >= this->_height)) return;

      bitmap_header_t bmpdata;
      if (!load_bmp_header(data, &bmpdata)
       || (bmpdata.biCompression > 3)) {
        return;
      }

      //uint32_t startTime = millis();
      uint32_t seekOffset = bmpdata.bfOffBits;
      int32_t w = bmpdata.biWidth;
      int32_t h = bmpdata.biHeight;  // bcHeight Image height (pixels)
      uint_fast16_t bpp = bmpdata.biBitCount; // 24 bcBitCount 24=RGB24bit

        //If the value of Height is positive, the image data is from bottom to top
        //If the value of Height is negative, the image data is from top to bottom.
      int32_t flow = (h < 0) ? 1 : -1;
      if (h < 0) h = -h;
      else y += h - 1;

      argb8888_t *palette = nullptr;
      if (bpp <= 8) {
        palette = new argb8888_t[1 << bpp];
        data->seek(bmpdata.biSize + 14);
        data->read((uint8_t*)palette, (1 << bpp)*sizeof(argb8888_t)); // load palette
      }

      data->seek(seekOffset);

      auto dst_depth = this->_write_conv.depth;
      uint32_t buffersize = ((w * bpp + 31) >> 5) << 2;  // readline 4Byte align.
      uint8_t lineBuffer[buffersize + 4];
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

      auto nt = data->need_transaction;
      if (bmpdata.biCompression == 1) {
        do {
          if (nt) this->endTransaction();
          load_bmp_rle8(data, lineBuffer, w);
          if (nt) this->beginTransaction();
          this->push_image(x, y, w, 1, &p);
          y += flow;
        } while (--h);
      } else
      if (bmpdata.biCompression == 2) {
        do {
          if (nt) this->endTransaction();
          load_bmp_rle4(data, lineBuffer, w);
          if (nt) this->beginTransaction();
          this->push_image(x, y, w, 1, &p);
          y += flow;
        } while (--h);
      } else {
        do {
          if (nt) this->endTransaction();
          data->read(lineBuffer, buffersize);
          if (nt) this->beginTransaction();
          this->push_image(x, y, w, 1, &p);
          y += flow;
        } while (--h);
      }
      if (palette) delete[] palette;
      //Serial.print("Loaded in "); Serial.print(millis() - startTime);   Serial.println(" ms");
    }


protected:

#if !defined (__LGFX_TJPGDEC_H__)

    bool draw_jpg(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div_t scale)
    {
//ESP_LOGI("LGFX","drawJpg need include utility/tjpgd.h");
      return false;
    }

#else

    bool draw_jpg(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div_t scale)
    {
      draw_jpg_info_t jpeg;
      pixelcopy_t pc(nullptr, this->getColorDepth(), bgr888_t::depth, this->hasPalette());
      jpeg.pc = &pc;
      jpeg.lgfx = this;
      jpeg.data = data;
      jpeg.x = x - offX;
      jpeg.y = y - offY;

      //TJpgD jpegdec;
      lgfxJdec jpegdec;

      static constexpr uint16_t sz_pool = 3100;
      uint8_t *pool = (uint8_t*)heap_alloc_dma(sz_pool);
      if (!pool) {
//        ESP_LOGE("LGFX","memory allocation failure");
        return false;
      }

      auto jres = lgfx_jd_prepare(&jpegdec, jpg_read_data, pool, sz_pool, &jpeg);

      if (jres != JDR_OK) {
//ESP_LOGE("LGFX","jpeg prepare error:%x", jres);
        heap_free(pool);
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
        jres = lgfx_jd_decomp(&jpegdec, jpg_push_image, scale);

        this->_clip_l = cl;
        this->_clip_t = ct;
        this->_clip_r = cr-1;
        this->_clip_b = cb-1;
        this->endWrite();
      }
      heap_free(pool);

      if (jres != JDR_OK) {
//ESP_LOGE("LGFX","jpeg decomp error:%x", jres);
        return false;
      }
      return true;
    }

    struct draw_jpg_info_t {
      int32_t x;
      int32_t y;
      DataWrapper *data;
      LGFXBase *lgfx;
      pixelcopy_t *pc;
    };

    static uint32_t jpg_read_data(lgfxJdec  *decoder, uint8_t *buf, uint32_t len) {
      auto jpeg = (draw_jpg_info_t *)decoder->device;
      auto data = (DataWrapper*)jpeg->data;
      auto res = len;
      if (data->need_transaction) jpeg->lgfx->endTransaction();
      if (buf) {
        res = data->read(buf, len);
      } else {
        data->skip(len);
      }
      return res;
    }

    static uint32_t jpg_push_image(lgfxJdec *decoder, void *bitmap, JRECT *rect) {
      draw_jpg_info_t *jpeg = (draw_jpg_info_t *)decoder->device;
      jpeg->pc->src_data = bitmap;
      auto data = (DataWrapper*)jpeg->data;
      if (data->need_transaction) jpeg->lgfx->beginTransaction();
      jpeg->lgfx->push_image( jpeg->x + rect->left
                           , jpeg->y + rect->top 
                           , rect->right  - rect->left + 1
                           , rect->bottom - rect->top + 1
                           , jpeg->pc
                           , false);
      return 1;
    }

#endif


#ifndef __LGFX_PNGLE_H__

    bool draw_png(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, double scale)
    {
//ESP_LOGI("LGFX","drawPng need include utility/pngle.h");
      return false;
    }

#else

    struct png_file_decoder_t {
      int32_t x;
      int32_t y;
      int32_t offX;
      int32_t offY;
      int32_t maxWidth;
      int32_t maxHeight;
      double scale;
      bgr888_t* lineBuffer;
      pixelcopy_t *pc;
      LGFXBase *lgfx;
      int32_t last_y;
      int32_t scale_y0;
      int32_t scale_y1;
    };

    static bool png_ypos_update(png_file_decoder_t *p, uint32_t y)
    {
      p->scale_y0 = ceil( y      * p->scale) - p->offY;
      if (p->scale_y0 < 0) p->scale_y0 = 0;
      p->scale_y1 = ceil((y + 1) * p->scale) - p->offY;
      if (p->scale_y1 > p->maxHeight) p->scale_y1 = p->maxHeight;
      return (p->scale_y0 < p->scale_y1);
    }

    static void png_post_line(png_file_decoder_t *p, uint32_t y)
    {
      int32_t h = p->scale_y1 - p->scale_y0;
      if (0 < h)
        p->lgfx->push_image(p->x, p->y + p->scale_y0, p->maxWidth, h, p->pc, true);
    }

    static void png_prepare_line(png_file_decoder_t *p, uint32_t y)
    {
      p->last_y = y;
      if (png_ypos_update(p, y))      // read next line
        p->lgfx->readRectRGB(p->x, p->y + p->scale_y0, p->maxWidth, p->scale_y1 - p->scale_y0, p->lineBuffer);
    }

    static void png_done_callback(pngle_t *pngle)
    {
      auto p = (png_file_decoder_t *)pngle_get_user_data(pngle);
      png_post_line(p, p->last_y);
    }

    static void png_draw_normal_callback(pngle_t *pngle, uint32_t x, uint32_t y, uint8_t rgba[4])
    {
      auto p = (png_file_decoder_t*)pngle_get_user_data(pngle);

      int32_t t = y - p->offY;
      if (t < 0 || t >= p->maxHeight) return;

      int32_t l = x - p->offX;
      if (l < 0 || l >= p->maxWidth) return;

      p->lgfx->setColor(color888(rgba[0], rgba[1], rgba[2]));
      p->lgfx->writeFillRectPreclipped(p->x + l, p->y + t, 1, 1);
    }

    static void png_draw_normal_scale_callback(pngle_t *pngle, uint32_t x, uint32_t y, uint8_t rgba[4])
    {
      auto p = (png_file_decoder_t*)pngle_get_user_data(pngle);

      if (y != p->last_y) {
        p->last_y = y;
        png_ypos_update(p, y);
      }

      int32_t t = p->scale_y0;
      int32_t h = p->scale_y1 - t;
      if (h <= 0) return;

      int32_t l = ceil( x      * p->scale) - p->offX;
      if (l < 0) l = 0;
      int32_t r = ceil((x + 1) * p->scale) - p->offX;
      if (r > p->maxWidth) r = p->maxWidth;
      if (l >= r) return;

      p->lgfx->setColor(color888(rgba[0], rgba[1], rgba[2]));
      p->lgfx->writeFillRectPreclipped(p->x + l, p->y + t, r - l, h);
    }

    static void png_draw_alpha_callback(pngle_t *pngle, uint32_t x, uint32_t y, uint8_t rgba[4])
    {
      auto p = (png_file_decoder_t*)pngle_get_user_data(pngle);
      if (y != p->last_y) {
        png_post_line(p, p->last_y);
        png_prepare_line(p, y);
      }

      if (p->scale_y0 >= p->scale_y1) return;

      int32_t l = ( x      ) - p->offX;
      if (l < 0) l = 0;
      int32_t r = ((x + 1) ) - p->offX;
      if (r > p->maxWidth) r = p->maxWidth;
      if (l >= r) return;

      if (rgba[3] == 255) {
        memcpy(&p->lineBuffer[l], rgba, 3);
      } else {
        auto data = &p->lineBuffer[l];
        uint_fast8_t alpha = rgba[3] + 1;
        data->r = (rgba[0] * alpha + data->r * (257 - alpha)) >> 8;
        data->g = (rgba[1] * alpha + data->g * (257 - alpha)) >> 8;
        data->b = (rgba[2] * alpha + data->b * (257 - alpha)) >> 8;
      }
    }

    static void png_draw_alpha_scale_callback(pngle_t *pngle, uint32_t x, uint32_t y, uint8_t rgba[4])
    {
      auto p = (png_file_decoder_t*)pngle_get_user_data(pngle);
      if (y != p->last_y) {
        png_post_line(p, p->last_y);
        png_prepare_line(p, y);
      }

      int32_t b = p->scale_y1 - p->scale_y0;
      if (b <= 0) return;

      int32_t l = ceil( x      * p->scale) - p->offX;
      if (l < 0) l = 0;
      int32_t r = ceil((x + 1) * p->scale) - p->offX;
      if (r > p->maxWidth) r = p->maxWidth;
      if (l >= r) return;

      if (rgba[3] == 255) {
        int32_t i = l;
        do {
          for (int32_t j = 0; j < b; ++j) {
            auto data = &p->lineBuffer[i + j * p->maxWidth];
            memcpy(data, rgba, 3);
          }
        } while (++i < r);
      } else {
        uint_fast8_t alpha = rgba[3] + 1;
        int32_t i = l;
        do {
          for (int32_t j = 0; j < b; ++j) {
            auto data = &p->lineBuffer[i + j * p->maxWidth];
            data->r = (rgba[0] * alpha + data->r * (257 - alpha)) >> 8;
            data->g = (rgba[1] * alpha + data->g * (257 - alpha)) >> 8;
            data->b = (rgba[2] * alpha + data->b * (257 - alpha)) >> 8;
          }
        } while (++i < r);
      }
    }

    static void png_init_callback(pngle_t *pngle, uint32_t w, uint32_t h, uint_fast8_t hasTransparent)
    {
//    auto ihdr = pngle_get_ihdr(pngle);

      auto p = (png_file_decoder_t*)pngle_get_user_data(pngle);

      if (p->scale != 1.0) {
        w = ceil(w * p->scale);
        h = ceil(h * p->scale);
      }

      int32_t ww = w - abs(p->offX);
      if (p->maxWidth > ww) p->maxWidth = ww;
      if (p->maxWidth < 0) return;
      if (p->offX < 0) { p->offX = 0; }

      int32_t hh = h - abs(p->offY);
      if (p->maxHeight > hh) p->maxHeight = hh;
      if (p->maxHeight < 0) return;
      if (p->offY < 0) { p->offY = 0; }

      if (hasTransparent) { // need pixel read ?
        p->lineBuffer = (bgr888_t*)heap_alloc_dma(sizeof(bgr888_t) * p->maxWidth * ceil(p->scale));
        p->pc->src_data = p->lineBuffer;
        png_prepare_line(p, 0);
        pngle_set_done_callback(pngle, png_done_callback);

        if (p->scale == 1.0) {
          pngle_set_draw_callback(pngle, png_draw_alpha_callback);
        } else {
          pngle_set_draw_callback(pngle, png_draw_alpha_scale_callback);
        }
      } else {
        if (p->scale == 1.0) {
          pngle_set_draw_callback(pngle, png_draw_normal_callback);
        } else {
          p->last_y = 0;
          png_ypos_update(p, 0);
          pngle_set_draw_callback(pngle, png_draw_normal_scale_callback);
        }
        return;
      }
    }

    bool draw_png(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, double scale)
    {
      if (!maxHeight) maxHeight = INT32_MAX;
      auto ct = this->_clip_t;
      if (0 > y - ct) { maxHeight += y - ct; offY -= y - ct; y = ct; }
      if (0 > offY) { y -= offY; maxHeight += offY; offY = 0; }
      auto cb = this->_clip_b + 1;
      if (maxHeight > (cb - y)) maxHeight = (cb - y);
      if (maxHeight < 0) return true;

      if (!maxWidth) maxWidth = INT32_MAX;
      auto cl = this->_clip_l;
      if (0 > x - cl) { maxWidth += x - cl; offX -= x - cl; x = cl; }
      if (0 > offX) { x -= offX; maxWidth  += offX; offX = 0; }
      auto cr = this->_clip_r + 1;
      if (maxWidth > (cr - x)) maxWidth = (cr - x);
      if (maxWidth < 0) return true;

      png_file_decoder_t png;
      png.x = x;
      png.y = y;
      png.offX = offX;
      png.offY = offY;
      png.maxWidth = maxWidth;
      png.maxHeight = maxHeight;
      png.scale = scale;
      png.lgfx = this;
      png.lineBuffer = nullptr;

      pixelcopy_t pc(nullptr, this->getColorDepth(), bgr888_t::depth, this->_palette_count);
      png.pc = &pc;

      pngle_t *pngle = pngle_new();

      pngle_set_user_data(pngle, &png);

      pngle_set_init_callback(pngle, png_init_callback);

      // Feed data to pngle
      uint8_t buf[512];
      int remain = 0;
      int len;
      bool res = true;

      len = data->read(buf, sizeof(buf));

      if (data->need_transaction && this->_transaction_count) this->beginTransaction();
      this->startWrite();
      while (len > 0) {

        int fed = pngle_feed(pngle, buf, remain + len);

        if (fed < 0) {
//ESP_LOGE("LGFX", "[pngle error] %s", pngle_error(pngle));
          res = false;
          break;
        }

        remain = remain + len - fed;
        if (remain > 0) memmove(buf, buf + fed, remain);
        if (data->need_transaction) this->endTransaction();
        len = data->read(buf + remain, sizeof(buf) - remain);
        if (data->need_transaction) this->beginTransaction();
      }
      this->endWrite();
      if (png.lineBuffer) {
        this->waitDMA();
        heap_free(png.lineBuffer);
      }
      pngle_destroy(pngle);
      return res;
    }

#endif


  };
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
}

class LovyanGFX : public
#ifdef LGFX_GFXFONT_HPP_
  lgfx::LGFX_GFXFont_Support<
#endif
   lgfx::LGFX_VLWFont_Support<
    lgfx::LGFX_IMAGE_FORMAT_Support<
     lgfx::LGFXBase
    >
   >
#ifdef LGFX_GFXFONT_HPP_
  >
#endif
{};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif
