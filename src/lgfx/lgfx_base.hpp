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

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <cstdint>
#include <cfloat>
#include <type_traits>
#include <algorithm>
#include <string>
#include <list>

#include "lgfx_common.hpp"


namespace lgfx
{
  class LGFXBase
  {
  public:
    LGFXBase() {}
    virtual ~LGFXBase() {}

// color param format:
// rgb888 : std::uint32_t
// rgb565 : std::uint16_t & std::int16_t & int
// rgb332 : std::uint8_t
    __attribute__ ((always_inline)) inline void setColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) { _color.raw = _write_conv.convert(lgfx::color888(r,g,b)); }
    template<typename T> __attribute__ ((always_inline)) inline void setColor(T c) { _color.raw = _write_conv.convert(c); }
                         __attribute__ ((always_inline)) inline void setRawColor(std::uint32_t c) { _color.raw = c; }

                         inline void clear      ( void )          { _color.raw = 0;  fillRect(0, 0, _width, _height); }
    template<typename T> inline void clear      ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }
                         inline void fillScreen ( void )          {                  fillRect(0, 0, _width, _height); }
    template<typename T> inline void fillScreen ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }

    template<typename T> inline void pushBlock  ( const T& color, std::int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }

// Deprecated, use pushBlock()
    template<typename T> inline void pushColor  ( const T& color, std::int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }
    template<typename T> inline void pushColor  ( const T& color                ) {                          setColor(color); startWrite(); pushBlock_impl(1);      endWrite(); }


// AdafruitGFX compatible functions.
// However, startWrite and endWrite have an internal counter and are executed when the counter is 0.
// If you do not want to the counter, call the transaction function directly.
    __attribute__ ((always_inline)) inline void startWrite(void) {                           if (1 == ++_transaction_count) beginTransaction(); }
    __attribute__ ((always_inline)) inline void endWrite(void)   { if (_transaction_count) { if (0 == --_transaction_count) endTransaction(); } }
    template<typename T> inline void writePixel    ( std::int32_t x, std::int32_t y                      , const T& color) { setColor(color); writePixel    (x, y         ); }
    template<typename T> inline void writeFastVLine( std::int32_t x, std::int32_t y           , std::int32_t h, const T& color) { setColor(color); writeFastVLine(x, y   , h   ); }
    template<typename T> inline void writeFastHLine( std::int32_t x, std::int32_t y, std::int32_t w           , const T& color) { setColor(color); writeFastHLine(x, y, w      ); }
    template<typename T> inline void writeFillRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T& color) { setColor(color); writeFillRect (x, y, w, h   ); }
    template<typename T> inline void writeColor    ( const T& color, std::int32_t length) { if (0 >= length) return; setColor(color);    pushBlock_impl(length);             }
                         inline void writeRawColor ( std::uint32_t color, std::int32_t length) { if (0 >= length) return; setRawColor(color); pushBlock_impl(length);             }

    template<typename T> inline void drawPixel     ( std::int32_t x, std::int32_t y                                 , const T& color) { setColor(color); drawPixel    (x, y         ); }
    template<typename T> inline void drawFastVLine ( std::int32_t x, std::int32_t y           , std::int32_t h           , const T& color) { setColor(color); drawFastVLine(x, y   , h   ); }
    template<typename T> inline void drawFastHLine ( std::int32_t x, std::int32_t y, std::int32_t w                      , const T& color) { setColor(color); drawFastHLine(x, y, w      ); }
    template<typename T> inline void fillRect      ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h           , const T& color) { setColor(color); fillRect     (x, y, w, h   ); }
    template<typename T> inline void drawRect      ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h           , const T& color) { setColor(color); drawRect     (x, y, w, h   ); }
    template<typename T> inline void drawRoundRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r, const T& color) { setColor(color); drawRoundRect(x, y, w, h, r); }
    template<typename T> inline void fillRoundRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r, const T& color) { setColor(color); fillRoundRect(x, y, w, h, r); }
    template<typename T> inline void drawCircle    ( std::int32_t x, std::int32_t y                      , std::int32_t r, const T& color) { setColor(color); drawCircle   (x, y      , r); }
    template<typename T> inline void fillCircle    ( std::int32_t x, std::int32_t y                      , std::int32_t r, const T& color) { setColor(color); fillCircle   (x, y      , r); }
    template<typename T> inline void drawEllipse   ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry         , const T& color) { setColor(color); drawEllipse  (x, y, rx, ry ); }
    template<typename T> inline void fillEllipse   ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry         , const T& color) { setColor(color); fillEllipse  (x, y, rx, ry ); }
    template<typename T> inline void drawLine      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1                        , const T& color)  { setColor(color); drawLine(    x0, y0, x1, y1        ); }
    template<typename T> inline void drawTriangle  ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawTriangle(x0, y0, x1, y1, x2, y2); }
    template<typename T> inline void fillTriangle  ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); fillTriangle(x0, y0, x1, y1, x2, y2); }
    template<typename T> inline void drawArc       ( std::int32_t x, std::int32_t y, std::int32_t r1, std::int32_t r2, float start, float end, const T& color) { setColor(color); drawArc( x, y, r1, r2, start, end); }
    template<typename T> inline void fillArc       ( std::int32_t x, std::int32_t y, std::int32_t r1, std::int32_t r2, float start, float end, const T& color) { setColor(color); fillArc( x, y, r1, r2, start, end); }
    template<typename T> inline void drawCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t cornername            , const T& color)  { setColor(color); drawCircleHelper(x, y, r, cornername    ); }
    template<typename T> inline void fillCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t corners, std::int32_t delta, const T& color)  { setColor(color); fillCircleHelper(x, y, r, corners, delta); }

    __attribute__ ((always_inline)) inline static std::uint8_t  color332(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color332(r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint16_t color565(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color565(r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint32_t color888(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color888(r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint16_t swap565( std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::swap565( r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint32_t swap888( std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::swap888( r, g, b); }

    __attribute__ ((always_inline)) inline void setPivot(std::int16_t x, std::int16_t y) { _xpivot = x; _ypivot = y; }
    __attribute__ ((always_inline)) inline std::int16_t getPivotX(void) const { return _xpivot; }
    __attribute__ ((always_inline)) inline std::int16_t getPivotY(void) const { return _ypivot; }

    __attribute__ ((always_inline)) inline std::int32_t width        (void) const { return _width; }
    __attribute__ ((always_inline)) inline std::int32_t height       (void) const { return _height; }
    __attribute__ ((always_inline)) inline color_depth_t getColorDepth(void) const { return _write_conv.depth; }
    __attribute__ ((always_inline)) inline color_conv_t* getColorConverter(void) { return &_write_conv; }
    __attribute__ ((always_inline)) inline bool hasPalette    (void) const { return _palette_count; }
    __attribute__ ((always_inline)) inline bool isSPIShared(void) const { return _spi_shared; }
    __attribute__ ((always_inline)) inline bool getSwapBytes    (void) const { return _swapBytes; }
    __attribute__ ((always_inline)) inline void setSwapBytes(bool swap) { _swapBytes = swap; }

    __attribute__ ((always_inline)) inline void beginTransaction(void) { beginTransaction_impl(); }
    __attribute__ ((always_inline)) inline void endTransaction(void)   { endTransaction_impl(); }
    __attribute__ ((always_inline)) inline void waitDMA(void)  { waitDMA_impl(); }
    __attribute__ ((always_inline)) inline void setWindow(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye) { setWindow_impl(xs, ys, xe, ye); }

    void setSPIShared(bool shared) { _spi_shared = shared; }

    void setAddrWindow(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
    {
      if (_adjust_abs(x, w)||_adjust_abs(y, h)) return;
      bool tr = !_transaction_count;
      if (tr) beginTransaction();
      setWindow(x, y, x + w - 1, y + h - 1);
      if (tr) endTransaction();
    }

    void setClipRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) {
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

    void getClipRect(std::int32_t *x, std::int32_t *y, std::int32_t *w, std::int32_t *h) {
      *x = _clip_l;
      *w = _clip_r - *x + 1;
      *y = _clip_t;
      *h = _clip_b - *y + 1;
    }

    void clearClipRect(void) {
      _clip_l = 0;
      _clip_r = _width - 1;
      _clip_t = 0;
      _clip_b = _height - 1;
    }

    __attribute__ ((always_inline))
    inline void drawPixel(std::int32_t x, std::int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;

      drawPixel_impl(x, y);
    }

    __attribute__ ((always_inline))
    inline void writePixel(std::int32_t x, std::int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;

      writeFillRect_impl(x, y, 1, 1);
    }

    void drawFastVLine(std::int32_t x, std::int32_t y, std::int32_t h)
    {
      _adjust_abs(y, h);
      bool tr = !_transaction_count;
      if (tr) beginTransaction();
      writeFastVLine(x, y, h);
      if (tr) endTransaction();
    }

    void writeFastVLine(std::int32_t x, std::int32_t y, std::int32_t h)
    {
      if (x < _clip_l || x > _clip_r) return;
      auto ct = _clip_t;
      if (y < ct) { h += y - ct; y = ct; }
      auto cb = _clip_b + 1 - y;
      if (h > cb) h = cb;
      if (h < 1) return;

      writeFillRect_impl(x, y, 1, h);
    }

    void drawFastHLine(std::int32_t x, std::int32_t y, std::int32_t w)
    {
      _adjust_abs(x, w);
      bool tr = !_transaction_count;
      if (tr) beginTransaction();
      writeFastHLine(x, y, w);
      if (tr) endTransaction();
    }

    void writeFastHLine(std::int32_t x, std::int32_t y, std::int32_t w)
    {
      if (y < _clip_t || y > _clip_b) return;
      auto cl = _clip_l;
      if (x < cl) { w += x - cl; x = cl; }
      auto cr = _clip_r + 1 - x;
      if (w > cr) w = cr;
      if (w < 1) return;

      writeFillRect_impl(x, y, w, 1);
    }

    void fillRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
    {
      _adjust_abs(x, w);
      _adjust_abs(y, h);
      bool tr = !_transaction_count;
      if (tr) beginTransaction();
      writeFillRect(x, y, w, h);
      if (tr) endTransaction();
    }

    void writeFillRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
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
    void writeFillRectPreclipped(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
    {
      writeFillRect_impl(x, y, w, h);
    }

    void drawRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
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

    void drawCircle(std::int32_t x, std::int32_t y, std::int32_t r)
    {
      if ( r <= 0 ) {
        drawPixel(x, y);
        return;
      }

      startWrite();
      std::int32_t f = 1 - r;
      std::int32_t ddF_y = - (r << 1);
      std::int32_t ddF_x = 1;
      std::int32_t i = 0;
      std::int32_t j = -1;
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

        writeFastVLine(x + r, y + j + 1, i - j);
        writeFastVLine(x + r, y - i    , i - j);
        writeFastVLine(x - r, y - i    , i - j);
        writeFastVLine(x - r, y + j + 1, i - j);
        j = i;
      } while (i < --r);
      endWrite();
    }

    void drawCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint8_t cornername)
    {
      if (r <= 0) return;
      std::int32_t f     = 1 - r;
      std::int32_t ddF_y = - (r << 1);
      std::int32_t ddF_x = 1;
      std::int32_t i     = 0;
      std::int32_t j     = 0;

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

    void fillCircle(std::int32_t x, std::int32_t y, std::int32_t r) {
      startWrite();
      writeFastHLine(x - r, y, (r << 1) + 1);
      fillCircleHelper(x, y, r, 3, 0);
      endWrite();
    }

    void fillCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t corners, std::int32_t delta)
    {
      if (r <= 0) return;

      ++delta;

      std::int32_t f     = 1 - r;
      std::int32_t ddF_y = - (r << 1);
      std::int32_t ddF_x = 1;
      std::int32_t i     = 0;

      startWrite();
      do {
        std::int32_t len = 0;
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

    void drawEllipse(std::int32_t x0, std::int32_t y0, std::int32_t rx, std::int32_t ry)
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

      std::int32_t x, y, s, i;
      std::int32_t rx2 = rx * rx;
      std::int32_t ry2 = ry * ry;

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

    void fillEllipse(std::int32_t x0, std::int32_t y0, std::int32_t rx, std::int32_t ry)
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

      std::int32_t x, y, i;
      std::int32_t rx2 = rx * rx;
      std::int32_t ry2 = ry * ry;
      std::int32_t s;

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


    void drawRoundRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r)
    {
      if (_adjust_abs(x, w)||_adjust_abs(y, h)) return; 
      startWrite();

      w--;
      h--;
      std::int32_t len = (r << 1) + 1;
      std::int32_t y1 = y + h - r;
      std::int32_t y0 = y + r;
      writeFastVLine(x      , y0 + 1, h - len);
      writeFastVLine(x + w  , y0 + 1, h - len);

      std::int32_t x1 = x + w - r;
      std::int32_t x0 = x + r;
      writeFastHLine(x0 + 1, y      , w - len);
      writeFastHLine(x0 + 1, y + h  , w - len);

      std::int32_t f     = 1 - r;
      std::int32_t ddF_y = -(r << 1);
      std::int32_t ddF_x = 1;

      len = 0;
      for (std::int32_t i = 0; i <= r; i++) {
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

    void fillRoundRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r)
    {
      if (_adjust_abs(x, w)||_adjust_abs(y, h)) return; 
      startWrite();
      std::int32_t y2 = y + r;
      std::int32_t y1 = y + h - r - 1;
      std::int32_t ddF_y = - (r << 1);
      std::int32_t delta = w + ddF_y;
      writeFillRect(x, y2, w, h + ddF_y);
      std::int32_t x0 = x + r;
      std::int32_t f     = 1 - r;
      std::int32_t ddF_x = 1;
      std::int32_t len = 0;
      for (std::int32_t i = 0; i <= r; i++) {
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

    void drawLine(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1)
    {
      bool steep = abs(y1 - y0) > abs(x1 - x0);

      if (steep) {   std::swap(x0, y0); std::swap(x1, y1); }
      if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }

      std::int32_t dy = abs(y1 - y0);
      std::int32_t ystep = (y1 > y0) ? 1 : -1;
      std::int32_t dx = x1 - x0;
      std::int32_t err = dx >> 1;

      std::int32_t xstart = steep ? _clip_t : _clip_l;
      std::int32_t ystart = steep ? _clip_l : _clip_t;
      std::int32_t yend   = steep ? _clip_r : _clip_b;
      while (x0 < xstart || y0 < ystart || y0 > yend) {
        err -= dy;
        if (err < 0) {
          err += dx;
          y0 += ystep;
        }
        if (++x0 > x1) return;
      }
      std::int32_t xs = x0;
      std::int32_t dlen = 0;

      startWrite();
      if (steep) {
        if (x1 > (_clip_b)) x1 = (_clip_b);
        do {
          ++dlen;
          if ((err -= dy) < 0) {
            writeFillRect(y0, xs, 1, dlen);
            err += dx;
            xs = x0 + 1; dlen = 0; y0 += ystep;
            if ((y0 < _clip_l) || (y0 > _clip_r)) break;
          }
        } while (++x0 <= x1);
        if (dlen) writeFillRect(y0, xs, 1, dlen);
      } else {
        if (x1 > (_clip_r)) x1 = (_clip_r);
        do {
          ++dlen;
          if ((err -= dy) < 0) {
            writeFillRect(xs, y0, dlen, 1);
            err += dx;
            xs = x0 + 1; dlen = 0; y0 += ystep;
            if ((y0 < _clip_t) || (y0 > _clip_b)) break;
          }
        } while (++x0 <= x1);
        if (dlen) writeFillRect(xs, y0, dlen, 1);
      }
      endWrite();
    }

    void drawTriangle(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2)
    {
      startWrite();
      drawLine(x0, y0, x1, y1);
      drawLine(x1, y1, x2, y2);
      drawLine(x2, y2, x0, y0);
      endWrite();
    }

    void fillTriangle(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2)
    {
      std::int32_t a, b;

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

      std::int32_t dy1 = y1 - y0;
      std::int32_t dy2 = y2 - y0;
      bool change = ((x1 - x0) * dy2 > (x2 - x0) * dy1);
      std::int32_t dx1 = abs(x1 - x0);
      std::int32_t dx2 = abs(x2 - x0);
      std::int32_t xstep1 = x1 < x0 ? -1 : 1;
      std::int32_t xstep2 = x2 < x0 ? -1 : 1;
      a = b = x0;
      if (change) {
        std::swap(dx1, dx2);
        std::swap(dy1, dy2);
        std::swap(xstep1, xstep2);
      }
      std::int32_t err1 = (std::max(dx1, dy1) >> 1)
                   + (xstep1 < 0
                     ? std::min(dx1, dy1)
                     : dx1);
      std::int32_t err2 = (std::max(dx2, dy2) >> 1)
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
    void drawGradientHLine( std::int32_t x, std::int32_t y, std::int32_t w, bgr888_t colorstart, bgr888_t colorend ) {
      drawGradientLine( x, y, x + w - 1, y, colorstart, colorend );
    }

    __attribute__ ((always_inline)) inline
    void drawGradientVLine( std::int32_t x, std::int32_t y, std::int32_t h, bgr888_t colorstart, bgr888_t colorend ) {
      drawGradientLine( x, y, x, y + h - 1, colorstart, colorend );
    }

    void drawGradientLine( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, bgr888_t colorstart, bgr888_t colorend )
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

      std::int32_t dx = x1 - x0;
      std::int32_t err = dx >> 1;
      std::int32_t dy = abs(y1 - y0);
      std::int32_t ystep = (y0 < y1) ? 1 : -1;

      std::int32_t diff_r = colorend.r - colorstart.r;
      std::int32_t diff_g = colorend.g - colorstart.g;
      std::int32_t diff_b = colorend.b - colorstart.b;

      startWrite();
      for (std::int32_t x = x0; x <= x1; x++) {
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

    void drawArc(std::int32_t x, std::int32_t y, std::int32_t r1, std::int32_t r2, float start, float end)
    {
      if (r1 < r2) std::swap(r1, r2);
      if (r1 < 1) r1 = 1;
      if (r2 < 1) r2 = 1;

      bool equal = fabsf(start - end) < std::numeric_limits<float>::epsilon();
      start = fmodf(start, 360);
      end = fmodf(end, 360);
      if (start < 0) start += 360.0;
      if (end < 0) end += 360.0;

      startWrite();
      fill_arc_helper(x, y, r1, r2, start, start);
      fill_arc_helper(x, y, r1, r2, end  , end);
      if (!equal && (fabsf(start - end) <= 0.0001)) { start = .0; end = 360.0; }
      fill_arc_helper(x, y, r1, r1, start, end);
      fill_arc_helper(x, y, r2, r2, start, end);
      endWrite();
    }

    void fillArc(std::int32_t x, std::int32_t y, std::int32_t r1, std::int32_t r2, float start, float end)
    {
      if (r1 < r2) std::swap(r1, r2);
      if (r1 < 1) r1 = 1;
      if (r2 < 1) r2 = 1;

      bool equal = fabsf(start - end) < std::numeric_limits<float>::epsilon();
      start = fmodf(start, 360);
      end = fmodf(end, 360);
      if (start < 0) start += 360.0;
      if (end < 0) end += 360.0;
      if (!equal && (fabsf(start - end) <= 0.0001)) { start = .0; end = 360.0; }

      startWrite();
      fill_arc_helper(x, y, r1, r2, start, end);
      endWrite();
    }

    template<typename T> void drawBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& color                    ) { draw_bitmap(x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& fgcolor, const T& bgcolor) { draw_bitmap(x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }
    template<typename T> void drawXBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& color                    ) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawXBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& fgcolor, const T& bgcolor) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }

    void draw_bitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor = ~0u)
    {
      if (w < 1 || h < 1) return;
      setRawColor(fg_rawcolor);
      std::int32_t byteWidth = (w + 7) >> 3;
      std::uint_fast8_t byte = 0;

      bool fg = true;
      std::int32_t j = 0;
      startWrite();
      do {
        std::int32_t i = 0;
        do {
          std::int32_t ip = i;
          for (;;) {
            if (!(i & 7)) byte = bitmap[i >> 3];
            if (fg != (bool)(byte & 0x80) || (++i >= w)) break;
            byte <<= 1;
          }
          if ((ip != i) && (fg || bg_rawcolor != ~0u)) {
            writeFastHLine(x + ip, y + j, i - ip);
          }
          fg = !fg;
          if (bg_rawcolor != ~0u) setRawColor(fg ? fg_rawcolor : bg_rawcolor);
        } while (i < w);
        bitmap += byteWidth;
      } while (++j < h);
      endWrite();
    }

    void draw_xbitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor = ~0u)
    {
      if (w < 1 || h < 1) return;
      setRawColor(fg_rawcolor);
      std::int32_t byteWidth = (w + 7) >> 3;
      std::uint_fast8_t byte = 0;

      bool fg = true;
      std::int32_t j = 0;
      startWrite();
      do {
        std::int32_t i = 0;
        do {
          std::int32_t ip = i;
          for (;;) {
            if (!(i & 7)) byte = bitmap[i >> 3];
            if (fg != (bool)(byte & 0x01) || (++i >= w)) break;
            byte >>= 1;
          }
          if ((ip != i) && (fg || bg_rawcolor != ~0u)) {
            writeFastHLine(x + ip, y + j, i - ip);
          }
          fg = !fg;
          if (bg_rawcolor != ~0u) setRawColor(fg ? fg_rawcolor : bg_rawcolor);
        } while (i < w);
        bitmap += byteWidth;
      } while (++j < h);
      endWrite();
    }

    void pushColors(const std::uint8_t* data, std::int32_t len)
    {
      pushColors((rgb332_t*)data, len);
    }
    void pushColors(const std::uint16_t* data, std::int32_t len, bool swap = true)
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
    void pushColors(const void* data, std::int32_t len, bool swap = true)
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
    void pushColors(const T *src, std::int32_t len)
    {
      pixelcopy_t p(src, _write_conv.depth, T::depth, _palette_count);
      startWrite();
      pushColors_impl(len, &p);
      endWrite();
    }

    std::uint16_t readPixel(std::int32_t x, std::int32_t y)
    {
      pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, _palette);
      std::uint_fast16_t data = 0;
      read_rect(x, y, 1, 1, &data, &p);
      return __builtin_bswap16(data);
    }

    __attribute__ ((always_inline)) inline
    void readRectRGB( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint8_t* data) { readRectRGB(x, y, w, h, (bgr888_t*)data); }
    void readRectRGB( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, bgr888_t* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, _palette);
      read_rect(x, y, w, h, data, &p);
    }

    template<typename T> inline
    void readRect( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, T* data)
    {
      pixelcopy_t p(nullptr, T::depth, _read_conv.depth, false, _palette);
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<T>(_read_conv.depth); }
      read_rect(x, y, w, h, data, &p);
    }

    void readRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint8_t* data)
    {
      pixelcopy_t p(nullptr, rgb332_t::depth, _read_conv.depth, false, _palette);
      read_rect(x, y, w, h, data, &p);
    }
    void readRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint16_t* data)
    {
      pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, _palette);
      if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<rgb565_t>(_read_conv.depth);
      }
      read_rect(x, y, w, h, data, &p);
    }
    void readRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, _palette);
      if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<rgb888_t>(_read_conv.depth);
      }
      read_rect(x, y, w, h, data, &p);
    }

    template<typename T> void pushRect(  std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data) { pushImage(x, y, w, h, data); }
    template<typename T> void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data) {
      pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr);
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_normalcopy<T>(_write_conv.depth); }
      push_image(x, y, w, h, &p);
    }
    template<typename T> void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data, const T& transparent) {
      pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr, _write_conv.convert(transparent));
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_normalcopy<T>(_write_conv.depth); }
      if (p.fp_skip==nullptr) { p.fp_skip = pixelcopy_t::normalskip<T>; }
      push_image(x, y, w, h, &p);
    }
    template<typename T> void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data                      , const std::uint8_t bits, const T* palette) {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette              );
      p.fp_copy = pixelcopy_t::get_fp_palettecopy<T>(_write_conv.depth);
      push_image(x, y, w, h, &p);
    }
    template<typename T> void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, std::uint32_t transparent, const std::uint8_t bits, const T* palette) {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette, transparent );
      p.fp_copy = pixelcopy_t::get_fp_palettecopy<T>(_write_conv.depth);
      push_image(x, y, w, h, &p);
    }

    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint16_t* data, std::uint32_t transp = ~0u)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb565_2Byte, _palette_count, nullptr, transp == ~0u ? ~0u : _write_conv.convert((std::uint16_t)transp));
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb565_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p);
    }
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, std::uint32_t transp = ~0u)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb888_3Byte, _palette_count, nullptr, transp == ~0u ? ~0u : _write_conv.convert(transp));
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb888_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p);
    }

    template<typename T> void pushImageDMA( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data)                          { pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr                                   ); push_image(x, y, w, h, &p, true); }
    template<typename T> void pushImageDMA( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data   , const T& transparent) { pixelcopy_t p(data, _write_conv.depth, T::depth, _palette_count, nullptr, _write_conv.convert(transparent)); push_image(x, y, w, h, &p, true); }
    template<typename T> void pushImageDMA( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data                      , const std::uint8_t bits, const T* palette) { pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette              ); push_image(x, y, w, h, &p, true); }
    template<typename T> void pushImageDMA( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, std::uint32_t transparent, const std::uint8_t bits, const T* palette) { pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette, transparent ); push_image(x, y, w, h, &p, true); }

    void pushImageDMA(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint16_t* data, std::uint32_t transp = ~0u)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb565_2Byte, _palette_count, nullptr, transp == ~0u? ~0u : _write_conv.convert((std::uint16_t)transp));
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb565_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p, true);
    }
    void pushImageDMA(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, std::uint32_t transp = ~0u)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb888_3Byte, _palette_count, nullptr, transp == ~0u ? ~0u : _write_conv.convert(transp));
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb888_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p, true);
    }

    void push_image(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t *param, bool use_dma = false)
    {
      param->src_width = w;
      if (param->src_bits < 8) {        // get bitwidth
//      std::uint32_t x_mask = (1 << (4 - __builtin_ffs(param->src_bits))) - 1;
//      std::uint32_t x_mask = (1 << ((~(param->src_bits>>1)) & 3)) - 1;
        std::uint32_t x_mask = (param->src_bits == 1) ? 7
                        : (param->src_bits == 2) ? 3
                                                 : 1;
        param->src_width = (w + x_mask) & (~x_mask);
      }

      std::int32_t dx=0, dw=w;
      if (0 < _clip_l - x) { dx = _clip_l - x; dw -= dx; x = _clip_l; }

      if (_adjust_width(x, dx, dw, _clip_l, _clip_r - _clip_l + 1)) return;
      param->src_x = dx;


      std::int32_t dy=0, dh=h;
      if (0 < _clip_t - y) { dy = _clip_t - y; dh -= dy; y = _clip_t; }
      if (_adjust_width(y, dy, dh, _clip_t, _clip_b - _clip_t + 1)) return;
      param->src_y = dy;

      startWrite();
      pushImage_impl(x, y, dw, dh, param, use_dma);
      endWrite();
    }

    bool pushImageRotateZoom(std::int32_t dst_x, std::int32_t dst_y, const void* data, std::int32_t src_x, std::int32_t src_y, std::int32_t w, std::int32_t h, float angle, float zoom_x, float zoom_y, std::uint32_t transparent, const std::uint8_t bits, const bgr888_t* palette) {
      if (nullptr == data) return false;
      if (zoom_x == 0.0 || zoom_y == 0.0) return true;
      pixelcopy_t pc(data, getColorDepth(), (color_depth_t)bits, hasPalette(), palette, transparent );
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, w, h, angle, zoom_x, zoom_y, &pc);
      return true;
    }

    void push_image_rotate_zoom(std::int32_t dst_x, std::int32_t dst_y, std::int32_t src_x, std::int32_t src_y, std::int32_t w, std::int32_t h, float angle, float zoom_x, float zoom_y, pixelcopy_t *param)
    {
      angle *= - deg_to_rad; // Convert degrees to radians
      float sin_f = sin(angle) * (1 << FP_SCALE);
      float cos_f = cos(angle) * (1 << FP_SCALE);
      std::int32_t min_y, max_y;
      {
        std::int32_t sinra = round(sin_f * zoom_x);
        std::int32_t cosra = round(cos_f * zoom_y);
        std::int32_t wp = (src_x - w) * sinra;
        std::int32_t sx = (src_x + 1) * sinra;
        std::int32_t hp = (h - src_y) * cosra;
        std::int32_t sy = (-1 -src_y) * cosra;
        std::int32_t tmp;
        if ((sinra < 0) == (cosra < 0)) {
          min_y = max_y = wp + sy;
          tmp           = sx + hp;
        } else {
          min_y = max_y = sx + sy;
          tmp           = wp + hp;
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
//      std::uint32_t x_mask = (1 << (4 - __builtin_ffs(param->src_bits))) - 1;
//      std::uint32_t x_mask = (1 << ((~(param->src_bits>>1)) & 3)) - 1;
        std::uint32_t x_mask = (param->src_bits == 1) ? 7
                        : (param->src_bits == 2) ? 3
                                                 : 1;
        param->src_width = (w + x_mask) & (~x_mask);
      } else {
        param->src_width = w;
      }

      std::int32_t xt =       - dst_x;
      std::int32_t yt = min_y - dst_y - 1;

      std::int32_t cos_x = round(cos_f / zoom_x);
      param->src_x32_add = cos_x;
      std::int32_t sin_x = - round(sin_f / zoom_x);
      std::int32_t xstart = cos_x * xt + sin_x * yt + (src_x << FP_SCALE) + (1 << (FP_SCALE - 1));
      std::int32_t scale_w = w << FP_SCALE;
      std::int32_t xs1 = (cos_x < 0 ?   - scale_w :   1) - cos_x;
      std::int32_t xs2 = (cos_x < 0 ? 0 : (1 - scale_w)) - cos_x;
      if (cos_x == 0) cos_x = 1;
      cos_x = -cos_x;

      std::int32_t sin_y = round(sin_f / zoom_y);
      param->src_y32_add = sin_y;
      std::int32_t cos_y = round(cos_f / zoom_y);
      std::int32_t ystart = sin_y * xt + cos_y * yt + (src_y << FP_SCALE) + (1 << (FP_SCALE - 1));
      std::int32_t scale_h = h << FP_SCALE;
      std::int32_t ys1 = (sin_y < 0 ?   - scale_h :   1) - sin_y;
      std::int32_t ys2 = (sin_y < 0 ? 0 : (1 - scale_h)) - sin_y;
      if (sin_y == 0) sin_y = 1;
      sin_y = -sin_y;

      std::int32_t cl = _clip_l;
      std::int32_t cr = _clip_r + 1;

      startWrite();
      do {
        std::int32_t left = cl;
        std::int32_t right = cr;
        xstart += sin_x;
        //if (cos_x != 0)
        {
          std::int32_t tmp = (xstart + xs1) / cos_x; if (left  < tmp) left  = tmp;
                  tmp = (xstart + xs2) / cos_x; if (right > tmp) right = tmp;
        }
        ystart += cos_y;
        //if (sin_y != 0)
        {
          std::int32_t tmp = (ystart + ys1) / sin_y; if (left  < tmp) left  = tmp;
                  tmp = (ystart + ys2) / sin_y; if (right > tmp) right = tmp;
        }
        if (left < right) {
          param->src_x32 = xstart - left * cos_x;
          std::int32_t y32 = ystart - left * sin_y;
          if (y32 >= 0) {
            param->src_y32 = y32;
            pushImage_impl(left, min_y, right - left, 1, param, true);
          }
        }
      } while (++min_y != max_y);
      endWrite();
    }

    template <typename T>
    void setScrollRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T& color) {
      _scolor = _write_conv.convert(color);
      setScrollRect(x, y, w, h);
    }
    void setScrollRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) {
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

    void getScrollRect(std::int32_t *x, std::int32_t *y, std::int32_t *w, std::int32_t *h) {
      *x = _sx;
      *y = _sy;
      *w = _sw;
      *h = _sh;
    }

    void scroll(std::int_fast16_t dx, std::int_fast16_t dy = 0)
    {
      _color.raw = _scolor;
      std::int32_t absx = abs(dx);
      std::int32_t absy = abs(dy);
      if (absx >= _sw || absy >= _sh) {
        writeFillRect(_sx, _sy, _sw, _sh);
        return;
      }

      std::int32_t w  = _sw - absx;
      std::int32_t h  = _sh - absy;

      std::int32_t src_x = dx < 0 ? _sx - dx : _sx;
      std::int32_t dst_x = src_x + dx;
      std::int32_t src_y = dy < 0 ? _sy - dy : _sy;
      std::int32_t dst_y = src_y + dy;

      startWrite();
      copyRect_impl(dst_x, dst_y, w, h, src_x, src_y);

      if (     dx > 0) writeFillRect(_sx           , dst_y,  dx, h);
      else if (dx < 0) writeFillRect(_sx + _sw + dx, dst_y, -dx, h);
      if (     dy > 0) writeFillRect(_sx, _sy           , _sw,  dy);
      else if (dy < 0) writeFillRect(_sx, _sy + _sh + dy, _sw, -dy);
      endWrite();
    }

    void copyRect(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y)
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


#ifdef _LGFX_QRCODE_H_
#ifdef ARDUINO
    void qrcode(const String &string, std::int_fast16_t x = -1, std::int_fast16_t y = -1, std::int_fast16_t width = -1, std::uint8_t version = 1) {
      qrcode(string.c_str(), x, y, width, version);
    }
#endif
    void qrcode(const char *string, std::int_fast16_t x = -1, std::int_fast16_t y = -1, std::int_fast16_t width = -1, std::uint8_t version = 1) {
      if (width == -1) {
        width = std::min(_width, _height) * 9 / 10;
      }
      if (x == -1 || y == -1) {
        x = (_width - width) >> 1;
        y = (_height- width) >> 1;
      }

      setColor(0xFFFFFFU);
      startWrite();
      writeFillRect(x, y, width, width);
      for (; version <= 40; ++version) {
        QRCode qrcode;
        std::uint8_t qrcodeData[lgfx_qrcode_getBufferSize(version)];
        if (0 != lgfx_qrcode_initText(&qrcode, qrcodeData, version, 0, string)) continue;
        std::int_fast16_t thickness = width / qrcode.size;
        if (!thickness) break;
        std::int_fast16_t lineLength = qrcode.size * thickness;
        std::int_fast16_t xOffset = x + ((width - lineLength) >> 1);
        std::int_fast16_t yOffset = y + ((width - lineLength) >> 1);
        setColor(0);
        y = 0;
        do {
          x = 0;
          do {
            if (lgfx_qrcode_getModule(&qrcode, x, y)) writeFillRect(x * thickness + xOffset, y * thickness + yOffset, thickness, thickness);
          } while (++x < qrcode.size);
        } while (++y < qrcode.size);
        break;
      }
      endWrite();
    }
#endif

    template<typename T> inline void paint( std::int32_t x, std::int32_t y, const T& color) { setColor(color); paint(x, y); }
    void paint(std::int32_t x, std::int32_t y) {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;
      bgr888_t target;
      readRectRGB(x, y, 1, 1, &target);
      if (_color.raw == _write_conv.convert(lgfx::color888(target.r, target.g, target.b))) return;

      pixelcopy_t p;
      p.transp = _read_conv.convert(lgfx::color888(target.r, target.g, target.b));
      switch (_read_conv.depth) {
      case 24: p.fp_copy = pixelcopy_t::normalcompare<bgr888_t>;  break;
      case 18: p.fp_copy = pixelcopy_t::normalcompare<bgr666_t>;  break;
      case 16: p.fp_copy = pixelcopy_t::normalcompare<swap565_t>; break;
      case  8: p.fp_copy = pixelcopy_t::normalcompare<rgb332_t>;  break;
      default: p.fp_copy = pixelcopy_t::bitcompare;
        p.src_bits = _read_conv.depth;
        p.src_mask = (1 << p.src_bits) - 1;
        p.transp &= p.src_mask;
        break;
      }

      std::int32_t cl = _clip_l;
      int w = _clip_r - cl + 1;
      std::uint8_t bufIdx = 0;
      bool buf0[w], buf1[w], buf2[w];
      bool* linebufs[3] = { buf0, buf1, buf2 };
      std::int32_t bufY[3] = {-2, -2, -2};
      bufY[0] = y;
      read_rect(cl, y, w, 1, linebufs[0], &p);
      std::list<paint_point_t> points;
      points.push_back({x, x, y, y});

      startWrite();
      while (!points.empty()) {
        std::int32_t y0 = bufY[bufIdx];
        auto it = points.begin();
        std::int32_t counter = 0;
        while (it->y != y0 && ++it != points.end()) ++counter;
        if (it == points.end()) {
          if (counter < 256) {
            ++bufIdx;
            std::int32_t y1 = bufY[(bufIdx  )%3];
            std::int32_t y2 = bufY[(bufIdx+1)%3];
            it = points.begin();
            while ((it->y != y1) && (it->y != y2) && (++it != points.end()));
          }
        }

        bufIdx = 0;
        if (it == points.end()) {
          it = points.begin();
          bufY[0] = it->y;
          read_rect(cl, it->y, w, 1, linebufs[0], &p);
        } else {
          for (; bufIdx < 2; ++bufIdx) if (it->y == bufY[bufIdx]) break;
        }
        bool* linebuf = &linebufs[bufIdx][- cl];

        int lx = it->lx;
        int rx = it->rx;
        int ly = it->y;
        int oy = it->oy;
        points.erase(it);
        if (!linebuf[lx]) continue;

        int lxsav = lx - 1;
        int rxsav = rx + 1;

        int cr = _clip_r;
        while (lx > cl && linebuf[lx - 1]) --lx;
        while (rx < cr && linebuf[rx + 1]) ++rx;

        writeFastHLine(lx, ly, rx - lx + 1);
        memset(&linebuf[lx], 0, rx - lx + 1);

        int newy = ly - 1;
        do {
          if (newy == oy && lx >= lxsav && rxsav >= rx) continue;
          if (newy < _clip_t) continue;
          if (newy > _clip_b) continue;
          int bidx = 0;
          for (; bidx < 3; ++bidx) if (newy == bufY[bidx]) break;
          if (bidx == 3) {
            for (bidx = 0; bidx < 2 && (abs(bufY[bidx] - ly) <= 1); ++bidx);
            bufY[bidx] = newy;
            read_rect(cl, newy, w, 1, linebufs[bidx], &p);
          }
          bool* linebuf = &linebufs[bidx][- cl];
          if (newy == oy) {
            paint_add_points(points, lx ,lxsav, newy, ly, linebuf);
            paint_add_points(points, rxsav ,rx, newy, ly, linebuf);
          } else {
            paint_add_points(points, lx ,rx, newy, ly, linebuf);
          }
        } while ((newy += 2) < ly + 2);
      }
      endWrite();
    }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    std::uint32_t _transaction_count = 0;
    std::int32_t _width = 0, _height = 0;
    std::int32_t  _sx, _sy, _sw, _sh; // for scroll zone

    std::int32_t _clip_l = 0, _clip_r = -1, _clip_t = 0, _clip_b = -1; // clip rect
    std::uint32_t _scolor;  // gap fill colour for scroll zone
    raw_color_t _color = 0xFFFFFFU;

    color_conv_t _write_conv;
    color_conv_t _read_conv;

    bgr888_t* _palette = nullptr; // for sprite palette mode.
    std::uint32_t _palette_count = 0;

    std::int16_t _xpivot;   // x pivot point coordinate
    std::int16_t _ypivot;   // x pivot point coordinate

    bool _spi_shared = true;
    bool _swapBytes = false;

    __attribute__ ((always_inline)) inline static bool _adjust_abs(std::int32_t& x, std::int32_t& w) { if (w < 0) { x += w + 1; w = -w; } return !w; }

    static bool _adjust_width(std::int32_t& x, std::int32_t& dx, std::int32_t& dw, std::int32_t left, std::int32_t width)
    {
      if (x < left) { dx = -x; dw += x; x = left; }
      if (dw > left + width - x) dw = left + width  - x;
      return (dw <= 0);
    }

    void read_rect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param)
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
    void fill_arc_helper(std::int32_t cx, std::int32_t cy, std::int32_t oradius, std::int32_t iradius, float start, float end)
    {
      float s_cos = (cos(start * deg_to_rad));
      float e_cos = (cos(end * deg_to_rad));
      float sslope = s_cos / (sin(start * deg_to_rad));
      float eslope = -1000000;
      if (end != 360.0) eslope = e_cos / (sin(end * deg_to_rad));
      float swidth =  0.5 / s_cos;
      float ewidth = -0.5 / e_cos;
      --iradius;
      int ir2 = iradius * iradius + iradius;
      int or2 = oradius * oradius + oradius;

      bool start180 = !(start < 180);
      bool end180 = end < 180;
      bool reversed = start + 180 < end || (end < start && start < end + 180);

      int xs = -oradius;
      int y = -oradius;
      int ye = oradius;
      int xe = oradius + 1;
      if (!reversed) {
        if (   (end >= 270 || end < 90) && (start >= 270 || start < 90)) xs = 0;
        else if (end < 270 && end >= 90 && start < 270 && start >= 90) xe = 1;
        if (     end >= 180 && start >= 180) ye = 0;
        else if (end < 180 && start < 180) y = 0;
      }
      do {
        int y2 = y * y;
        int x = xs;
        if (x < 0) {
          while (x * x + y2 >= or2) ++x;
          if (xe != 1) xe = 1 - x;
        }
        float ysslope = (y + swidth) * sslope;
        float yeslope = (y + ewidth) * eslope;
        int len = 0;
        do {
          bool flg1 = start180 != (x <= ysslope);
          bool flg2 =   end180 != (x <= yeslope);
          int distance = x * x + y2;
          if (distance >= ir2
           && ((flg1 && flg2) || (reversed && (flg1 || flg2)))
           && x != xe
           && distance < or2
            ) {
            ++len;
          } else {
            if (len) {
              writeFastHLine(cx + x - len, cy + y, len);
              len = 0;
            }
            if (distance >= or2) break;
            if (x < 0 && distance < ir2) { x = -x; }
          }
        } while (++x <= xe);
      } while (++y <= ye);
    }

    struct paint_point_t { std::int32_t lx,rx,y,oy; };
    static void paint_add_points(std::list<paint_point_t>& points, int lx, int rx, int y, int oy, bool* linebuf) {
      paint_point_t pt { 0, 0, y, oy };
      while (lx <= rx) {
        while (lx < rx && !linebuf[lx]) ++lx;
        if (!linebuf[lx]) break;
        pt.lx = lx;
        while (++lx <= rx && linebuf[lx]);
        pt.rx = lx - 1;
        points.push_back(pt);
      }
    }

    virtual void beginTransaction_impl(void) = 0;
    virtual void endTransaction_impl(void) = 0;
    virtual void waitDMA_impl(void) = 0;

    virtual void drawPixel_impl(std::int32_t x, std::int32_t y) = 0;
    virtual void writeFillRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) = 0;
    virtual void copyRect_impl(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y) = 0;
    virtual void readRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param) = 0;
    virtual void pushImage_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param, bool use_dma) = 0;
    virtual void pushColors_impl(std::int32_t length, pixelcopy_t* param) = 0;
    virtual void pushBlock_impl(std::int32_t len) = 0;
    virtual void setWindow_impl(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye) = 0;

    static void tmpBeginTransaction(void* lgfx) {
      auto me = (LGFXBase*)lgfx;
      if (me->_transaction_count) me->beginTransaction();
    }
    static void tmpEndTransaction(void* lgfx) {
      auto me = (LGFXBase*)lgfx;
      if (me->_transaction_count) me->endTransaction();
    }
    void prepareTmpTransaction(DataWrapper* data) {
      if (data->need_transaction && isSPIShared()) {
        data->parent = this;
        data->fp_pre_read  = tmpEndTransaction;
        data->fp_post_read = tmpBeginTransaction;
      }
    }
    __attribute__ ((always_inline)) inline void startWrite(bool transaction) {
      if (1 == ++_transaction_count && transaction) beginTransaction();
    }
  };
}


class LovyanGFX : public
#ifdef LGFX_FONT_SUPPORT_HPP_
   lgfx::LGFX_Font_Support<
#endif
#ifdef LGFX_IMG_SUPPORT_HPP_
    lgfx::LGFX_IMG_Support<
#endif
     lgfx::LGFXBase
#ifdef LGFX_IMG_SUPPORT_HPP_
    >
#endif
#ifdef LGFX_FONT_SUPPORT_HPP_
   >
#endif
{};

#endif
