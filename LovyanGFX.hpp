#ifndef LOVYANGFX_HPP_
#define LOVYANGFX_HPP_

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <cmath>
#include <cfloat>
#include <type_traits>
#include <algorithm>

#define LOAD_GFXFF

#include "LGFX_FontLoad.hpp"

#include "platforms/lgfx_common.hpp"

namespace lgfx
{
  class LGFXBase
  {
  public:
    LGFXBase() {}
    virtual ~LGFXBase() {}


                         __attribute__ ((always_inline)) inline void setColorRaw(uint32_t c) { _color.raw = c; }
    template<typename T> __attribute__ ((always_inline)) inline void setColor(T c) { _color.raw = _write_depth.convert(c); }

    template<typename T> inline void setTextColor(T c)      { _textcolor = _textbgcolor = _write_depth.convert(c); }
    template<typename T> inline void setTextColor(T c, T b) { _textcolor = _write_depth.convert(c); _textbgcolor = _write_depth.convert(b); }

                         inline void clear         ( void )          { _color.raw = 0;  fillRect(0, 0, _width, _height); }
    template<typename T> inline void clear         ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }
    template<typename T> inline void fillScreen    ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }

    template<typename T> inline void writeColor    ( const T& color, int32_t length) { if (0 >= length) return; setColor(color);               writeColor_impl(length);             }
                         inline void writeColorRaw ( uint32_t color, int32_t length) { if (0 >= length) return; setColorRaw(color);            writeColor_impl(length);             }
    template<typename T> inline void pushColor     ( const T& color, int32_t length) { if (0 >= length) return; setColor(color); startWrite(); writeColor_impl(length); endWrite(); }
    template<typename T> inline void pushColor     ( const T& color                ) {                          setColor(color); startWrite(); writeColor_impl(1);      endWrite(); }

    template<typename T> inline void drawPixel     ( int32_t x, int32_t y                      , const T& color) { setColor(color); drawPixel    (x, y      ); }
    template<typename T> inline void drawFastVLine ( int32_t x, int32_t y           , int32_t h, const T& color) { setColor(color); drawFastVLine(x, y   , h); }
    template<typename T> inline void drawFastHLine ( int32_t x, int32_t y, int32_t w           , const T& color) { setColor(color); drawFastHLine(x, y, w   ); }
    template<typename T> inline void fillRect      ( int32_t x, int32_t y, int32_t w, int32_t h, const T& color) { setColor(color); fillRect     (x, y, w, h); }
    template<typename T> inline void drawRect      ( int32_t x, int32_t y, int32_t w, int32_t h, const T& color) { setColor(color); drawRect     (x, y, w, h); }
    template<typename T> inline void drawRoundRect ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, const T& color) { setColor(color); drawRoundRect(x, y, w, h, r); }
    template<typename T> inline void fillRoundRect ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, const T& color) { setColor(color); fillRoundRect(x, y, w, h, r); }
    template<typename T> inline void drawCircle    ( int32_t x, int32_t y                      , int32_t r, const T& color) { setColor(color); drawCircle(x, y, r   ); }
    template<typename T> inline void fillCircle    ( int32_t x, int32_t y                      , int32_t r, const T& color) { setColor(color); fillCircle(x, y, r   ); }
    template<typename T> inline void drawLine      ( int32_t x0, int32_t y0, int32_t x1, int32_t y1                        , const T& color)  { setColor(color); drawLine(    x0, y0, x1, y1        ); }
    template<typename T> inline void drawTriangle  ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); drawTriangle(x0, y0, x1, y1, x2, y2); }
    template<typename T> inline void fillTriangle  ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); fillTriangle(x0, y0, x1, y1, x2, y2); }
    template<typename T> inline void readRect      ( int32_t x, int32_t y, int32_t w, int32_t h,       T* data) { read_rect(x, y, w, h, data, get_read_pixels_fp<T>()); }
                                void readRectRGB   ( int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* data) { read_rect(x, y, w, h, data, get_read_pixels_fp<swap888_t>()); }
    template<typename T> inline void pushRect      ( int32_t x, int32_t y, int32_t w, int32_t h, const T* data) { pushImage(x, y, w, h, data); }
    template<typename T> inline void pushImage     ( int32_t x, int32_t y, int32_t w, int32_t h, const T* data)                                                { push_image(x, y, w, h, data, nullptr                          , T::bits, get_write_pixels_fp<T>()); }
    template<typename T> inline void pushImage     ( int32_t x, int32_t y, int32_t w, int32_t h, const T* data                         , uint32_t transparent) { push_image(x, y, w, h, data, nullptr,              transparent, T::bits, get_write_pixels_fp<T>()); }
    template<typename T> inline void pushImage     ( int32_t x, int32_t y, int32_t w, int32_t h, const T* data                         , const T& transparent) { push_image(x, y, w, h, data, nullptr, *(uint32_t*)&transparent, T::bits, get_write_pixels_fp<T>()); }
    template<typename T, typename U> inline void pushIndexImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data, const U* palette)                       { push_image(x, y, w, h, data, palette                          , T::bits, get_write_palette_fp<T, U>()); }
    template<typename T, typename U> inline void pushIndexImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data, const U* palette, uint32_t transparent) { push_image(x, y, w, h, data, palette,              transparent, T::bits, get_write_palette_fp<T, U>()); }
    template<typename T, typename U> inline void pushIndexImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data, const U* palette, const T& transparent) { push_image(x, y, w, h, data, palette, *(uint32_t*)&transparent, T::bits, get_write_palette_fp<T, U>()); }

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
    __attribute__ ((always_inline)) inline color_depth_t getColorDepth(void) const { return _write_depth.depth; }
    __attribute__ ((always_inline)) inline dev_color_t getWriteDepth(void) const { return _write_depth; }
    __attribute__ ((always_inline)) inline dev_color_t getReadDepth(void) const { return _read_depth; }
    __attribute__ ((always_inline)) inline bool hasPalette    (void) const { return _has_palette; }
    __attribute__ ((always_inline)) inline bool hasTransaction(void) const { return _has_transaction; }
    __attribute__ ((always_inline)) inline void setSwapBytes(bool swap) { _swapBytes = swap; }
    __attribute__ ((always_inline)) inline void setRotation(uint8_t rotation) { setRotation_impl(rotation); }
    __attribute__ ((always_inline)) inline void* setColorDepth(uint8_t bpp)       { return setColorDepth_impl((color_depth_t)bpp); }
    __attribute__ ((always_inline)) inline void* setColorDepth(color_depth_t bpp) { return setColorDepth_impl(bpp); }

    __attribute__ ((always_inline)) inline void beginTransaction(void) { beginTransaction_impl(); }
    __attribute__ ((always_inline)) inline void endTransaction(void)   { endTransaction_impl(); }
    __attribute__ ((always_inline)) inline void flush(void) { flush_impl(); }
    __attribute__ ((always_inline)) inline void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) { setWindow_impl(xs, ys, xe, ye); }

    void startWrite(void) { if (0 == _transaction_count++) { beginTransaction(); } }
    void endWrite(void)   { if (_transaction_count) { if (0 == (--_transaction_count)) endTransaction(); } }

    void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      _adjust(x,w);
      _adjust(y,h);
      startWrite();
      setWindow(x, y, x + w - 1, y + h - 1);
      endWrite();
    }

    void drawPixel(int32_t x, int32_t y)
    {
      if (x < 0 || (x >= _width) || y < 0 || (y >= _height)) return;
      drawPixel_impl(x, y);
    }

    void drawFastVLine(int32_t x, int32_t y, int32_t h)
    {
      if ((x < 0) || (x >= _width)) return;
      if (_adjust(y,h)) return; 
      if (y < 0) { h += y; y = 0; }
      if ((y + h) > _height) h = _height - y;
      if (h < 1) return;

      fillRect_impl(x, y, 1, h);
    }

    void drawFastHLine(int32_t x, int32_t y, int32_t w)
    {
      if ((y < 0) || (y >= _height)) return;
      if (_adjust(x,w)) return; 
      if (x < 0) { w += x; x = 0; }
      if ((x + w) > _width) w = _width - x;
      if (w < 1) return;

      fillRect_impl(x, y, w, 1);
    }

    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      if (_adjust(x,w)||_adjust(y,h)) return; 
      if ((x >= _width) || (y >= _height)) return;
      if (x < 0) { w += x; x = 0; }
      if ((x + w) > _width)  w = _width  - x;
      if (w < 1) return;
      if (y < 0) { h += y; y = 0; }
      if ((y + h) > _height) h = _height - y;
      if (h < 1) return;

      fillRect_impl(x, y, w, h);
    }

    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      if (_adjust(x,w)||_adjust(y,h)) return;
      startWrite();
      drawFastHLine(x, y        , w);
      if (--h) {
        drawFastHLine(x, y + h    , w);
        if (--h) {
          drawFastVLine(x        , ++y, h);
          drawFastVLine(x + w - 1,   y, h);
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
      drawFastHLine(x - i, y + r, len);
      drawFastHLine(x - i, y - r, len);
      drawFastVLine(x - r, y - i, len);
      drawFastVLine(x + r, y - i, len);
      len = 0;
      for (r--; i <= r; i++) {
        if (p >= 0) {
          drawFastHLine(x - i          , y + r, len);
          drawFastHLine(x - i          , y - r, len);
          drawFastHLine(x + i - len + 1, y + r, len);
          drawFastHLine(x + i - len + 1, y - r, len);
          if (i == r && len == 1) break;
          drawFastVLine(x - r, y - i          , len);
          drawFastVLine(x + r, y - i          , len);
          dy -= 2;
          p -= dy;
          drawFastVLine(x + r, y + i - len + 1, len);
          drawFastVLine(x - r, y + i - len + 1, len);
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
      drawFastHLine(x - r, y, dy+1);

      for (int32_t i  = 0; i <= r; i++) {
        len++;
        if (p >= 0) {
          fillRect(x - r, y - i          , (r<<1) + 1, len);
          fillRect(x - r, y + i - len + 1, (r<<1) + 1, len);
          if (i == r) break;
          dy -= 2;
          p -= dy;
          len = 0;
          drawFastHLine(x - i, y + r, (i<<1) + 1);
          drawFastHLine(x - i, y - r, (i<<1) + 1);
          r--;
        }
        dx+=2;
        p+=dx;
      }

      endWrite();
    }

    void drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r)
    {
      if (_adjust(x,w)||_adjust(y,h)) return; 
      startWrite();

      w--;
      h--;
      int32_t len = (r << 1) + 1;
      int32_t y1 = y + h - r;
      int32_t y0 = y + r;
      drawFastVLine(x      , y0 + 1, h - len);
      drawFastVLine(x + w  , y0 + 1, h - len);

      int32_t x1 = x + w - r;
      int32_t x0 = x + r;
      drawFastHLine(x0 + 1, y      , w - len);
      drawFastHLine(x0 + 1, y + h  , w - len);

      int32_t f     = 1 - r;
      int32_t ddF_x = 1;
      int32_t ddF_y = -2 * r;

      len = 0;
      for (int32_t i = 0; i <= r; i++) {
        len++;
        if (f >= 0) {
          drawFastHLine(x0 - i          , y0 - r, len);
          drawFastHLine(x0 - i          , y1 + r, len);
          drawFastHLine(x1 + i - len + 1, y1 + r, len);
          drawFastHLine(x1 + i - len + 1, y0 - r, len);
          drawFastVLine(x1 + r, y1 + i - len + 1, len);
          drawFastVLine(x0 - r, y1 + i - len + 1, len);
          drawFastVLine(x1 + r, y0 - i, len);
          drawFastVLine(x0 - r, y0 - i, len);
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
      if (_adjust(x,w)||_adjust(y,h)) return; 
      startWrite();
      fillRect(x, y + r, w, h - (r << 1));
      int32_t x0 = x + r;
      int32_t y1 = y + h - r - 1;
      int32_t y2 = y + r;

      int32_t delta = w - (r << 1);
      int32_t f     = 1 - r;
      int32_t ddF_x = 1;
      int32_t ddF_y = -r - r;
      int32_t len = 0;
      for (int32_t i = 0; i <= r; i++) {
        len++;
        if (f >= 0) {
          fillRect(x0 - r, y2 - i          , (r << 1) + delta, len);
          fillRect(x0 - r, y1 + i - len + 1, (r << 1) + delta, len);
          if (i == r) break;
          len = 0;
          drawFastHLine(x0 - i, y1 + r, (i << 1) + delta);
          ddF_y += 2;
          f     += ddF_y;
          drawFastHLine(x0 - i, y2 - r, (i << 1) + delta);
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

      if (steep) {   swap_coord(x0, y0); swap_coord(x1, y1); }
      if (x0 > x1) { swap_coord(x0, x1); swap_coord(y0, y1); }

      int32_t dy = abs(y1 - y0);
      int32_t ystep = (y0 < y1) ? 1 : -1;
      int32_t dx = x1 - x0;
      int32_t err = dx >> 1;

      int32_t yend = steep ? _width : _height;
      while (x0 < 0 || y0 < 0 || y0 >= yend) {
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
        if (x1 >= _height) x1 = _height - 1;
        for (; x0 <= x1; x0++) {
          dlen++;
          if ((err -= dy) < 0) {
            fillRect_impl(y0, xs, 1, dlen);
            err += dx;
            xs = x0 + 1; dlen = 0; y0 += ystep;
            if ((y0 < 0) || (y0 >= _width)) break;
          }
        }
        if (dlen) fillRect_impl(y0, xs, 1, dlen);
      } else {
        if (x1 >= _width) x1 = _width - 1;
        for (; x0 <= x1; x0++) {
          dlen++;
          if ((err -= dy) < 0) {
            fillRect_impl(xs, y0, dlen, 1);
            err += dx;
            xs = x0 + 1; dlen = 0; y0 += ystep;
            if ((y0 < 0) || (y0 >= _height)) break;
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
      if (y0 > y1) { swap_coord(y0, y1); swap_coord(x0, x1); }
      if (y1 > y2) { swap_coord(y2, y1); swap_coord(x2, x1); }
      if (y0 > y1) { swap_coord(y0, y1); swap_coord(x0, x1); }

      if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)      a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a)      a = x2;
        else if (x2 > b) b = x2;
        drawFastHLine(a, y0, b - a + 1);
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

        if (a > b) swap_coord(a, b);
        drawFastHLine(a, y, b - a + 1);
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

        if (a > b) swap_coord(a, b);
        drawFastHLine(a, y, b - a + 1);
      }
      endWrite();
    }

    void pushColors(const uint8_t* data, int32_t len)
    {
      pushColors((rgb332_t*)data, len);
    }
    void pushColors(const uint16_t* data, int32_t len, bool swap = true)
    {
      if (swap) pushColors((rgb565_t *)data, len);
      else      pushColors((swap565_t*)data, len);
    }
    void pushColors(const void* data, int32_t len, bool swap = true)
    {
      if (swap) pushColors((rgb888_t *)data, len);
      else      pushColors((swap888_t*)data, len);
    }

    template <typename T>
    void pushColors(const T *src, int32_t len)
    {
      pixelcopy_param_t p;
      startWrite();
      auto fp = get_write_pixels_fp<T>();
      (this->*fp)(src, len, &p);
      endWrite();
    }

    void pushDMA(const uint8_t* src, int32_t len)
    {
      startWrite();
      writeBytesDMA_impl(src, len);
      endWrite();
    }
    void writeBytes(const uint8_t* src, int32_t len)
    {
      writeBytes_impl(src, len);
    }
    void writeBytesDMA(const uint8_t* src, int32_t len)
    {
      writeBytesDMA_impl(src, len);
    }

    uint16_t readPixel(int32_t x, int32_t y)
    {
      //rgb565_t buf;
      //read_rect(x, y, 1, 1, &buf, get_read_pixels_fp<rgb565_t>());
      //return buf.raw;
      if (x < 0 || (x >= _width) || y < 0 || (y >= _height)) return 0;
      return readPixel16_impl(x, y).raw;
    }

    uint32_t readPixelRAW(int32_t x, int32_t y)
    {
      if (x < 0 || (x >= _width) || y < 0 || (y >= _height)) return 0;
      return readPixelRAW_impl(x, y);
    }

    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* data)
    {
      read_rect(x, y, w, h, data, get_read_pixels_fp<rgb332_t>());
    }
    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data)
    {
      if (_swapBytes) read_rect(x, y, w, h, data, get_read_pixels_fp<rgb565_t>());
      else            read_rect(x, y, w, h, data, get_read_pixels_fp<swap565_t>());
    }
    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, void* data)
    {
      if (_swapBytes) read_rect(x, y, w, h, data, get_read_pixels_fp<rgb888_t>());
      else            read_rect(x, y, w, h, data, get_read_pixels_fp<swap888_t>());
    }

    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* data)
    {
      if (_swapBytes) push_image(x, y, w, h, data, nullptr, rgb565_t ::bits, get_write_pixels_fp<rgb565_t>());
      else            push_image(x, y, w, h, data, nullptr, swap565_t::bits, get_write_pixels_fp<swap565_t>());
    }
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const void* data)
    {
      if (_swapBytes) push_image(x, y, w, h, data, nullptr, rgb888_t ::bits, get_write_pixels_fp<rgb888_t>());
      else            push_image(x, y, w, h, data, nullptr, swap888_t::bits, get_write_pixels_fp<swap888_t>());
    }

    template <typename T>
    void setScrollRect(int32_t x, int32_t y, int32_t w, int32_t h, const T& color) {
      _scolor = _write_depth.convert(color);
      setScrollRect(x, y, w, h);
    }
    void setScrollRect(int32_t x, int32_t y, int32_t w, int32_t h) {
      _adjust(x,w);
      _adjust(y,h);
      if (x < 0) { w += x; x = 0; }
      if ((x + w) > _width)  w = _width  - x;
      if (w < 0) w = 0;
      if (y < 0) { h += y; y = 0; }
      if ((y + h) > _height) h = _height - y;
      if (h < 0) h = 0;
      _sx = x;
      _sy = y;
      _sw = w;
      _sh = h;
    }

    void scroll(int16_t dx, int16_t dy)
    {
      _color.raw = _scolor;
      int absx = abs(dx);
      int absy = abs(dy);
      if (absx >= _sw || absy >= _sh) {
        fillRect(_sx, _sy, _sw, _sh);
        return;
      }

      int32_t w  = _sw - absx;
      int32_t h  = _sh - absy;

      int32_t src_x = dx < 0 ? _sx - dx : _sx;
      int32_t src_y = dy < 0 ? _sy - dy : _sy;
      int32_t dst_x = src_x + dx;
      int32_t dst_y = src_y + dy;

      copyRect(dst_x, dst_y, w, h, src_x, src_y);

      if (     dx > 0) fillRect(_sx           , dst_y,  dx, h);
      else if (dx < 0) fillRect(_sx + _sw + dx, dst_y, -dx, h);
      if (     dy > 0) fillRect(_sx, _sy           , _sw,  dy);
      else if (dy < 0) fillRect(_sx, _sy + _sh + dy, _sw, -dy);
    }

    void copyRect(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y)
    {
      if ((src_x >= _width) || (dst_x >= _width)) return;
      if (src_x < dst_x) { if (src_x < 0) { w += src_x; dst_x -= src_x; src_x = 0; } if ((dst_x + w) > _width )  w = _width  - dst_x; }
      else               { if (dst_x < 0) { w += dst_x; src_x -= dst_x; dst_x = 0; } if ((src_x + w) > _width )  w = _width  - src_x; }
      if (w < 1) return;
      if ((src_y >= _height) || (dst_y >= _height)) return;
      if (src_y < dst_y) { if (src_y < 0) { h += src_y; dst_y -= src_y; src_y = 0; } if ((dst_y + h) > _height)  h = _height - dst_y; }
      else               { if (dst_y < 0) { h += dst_y; src_y -= dst_y; dst_y = 0; } if ((src_y + h) > _height)  h = _height - src_y; }
      if (h < 1) return;

      copyRect_impl(dst_x, dst_y, w, h, src_x, src_y);
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
//    if (len < 0) {
//      va_end(arg);
//      return 0;
//    };
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

      uint16_t uniCode = utf8;
      if (_decoderState != utf8_decode_state_t::utf8_no_use) {
        uniCode = decodeUTF8(utf8);
        if (uniCode == 0) return 1;
        if (utf8 == '\n') uniCode = 32; // Make it a valid space character to stop errors
        else if (uniCode < 32) return 1;
      }
      if (fpUpdateFontSize && !(fpUpdateFontSize)(this, uniCode)) return 1;

      if (0 == _font_size.width) return 1;

      if (utf8 == '\n') {
        _cursor_x = 0;
        _cursor_y += _font_size.yadvance * _textsize_y;
      } else {
        int16_t w  = _font_size.width    * _textsize_x;
        int16_t xo = _font_size.xoffset  * _textsize_x;
        int16_t h  = _font_size.height   * _textsize_y;
        int16_t yo = _font_size.yoffset  * _textsize_y;
        if (_textscroll || _textwrapX) {
          int32_t left = _textscroll ? _sx : 0;
          if (_cursor_x < left - xo) _cursor_x = left - xo;
          else {
            int32_t right = _textscroll ? _sx + _sw : _width;
            if (_cursor_x + xo + w > right) {
              _cursor_x = left - xo;
              _cursor_y += _font_size.yadvance * _textsize_y;
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
            _cursor_x = - xo;
            _cursor_y = - yo;
          } else
          if (_cursor_y < - yo) _cursor_y = - yo;
        }
        _cursor_x += drawChar(uniCode, _cursor_x, _cursor_y);
      }

      return 1;
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
    inline int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y)
    {
      return drawChar(x, y, uniCode, _textcolor, _textbgcolor, _textsize_x, _textsize_y);
    }

    inline int16_t drawChar(int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size)                   { return (fpDrawCharClassic)(this, x, y, c, color, bg, size, size); }
    inline int16_t drawChar(int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y) { return (fpDrawCharClassic)(this, x, y, c, color, bg, size_x, size_y); }

    int16_t getCursorX(void) const { return _cursor_x; }
    int16_t getCursorY(void) const { return _cursor_y; }
    void setCursor( int16_t x, int16_t y)               { _cursor_x = x; _cursor_y = y; }
    void setCursor( int16_t x, int16_t y, uint8_t font) { _cursor_x = x; _cursor_y = y; _textfont = font; }
    void setTextSize(uint8_t s) { setTextSize(s,s); }
    void setTextSize(uint8_t sx, uint8_t sy) { _textsize_x = (sx > 0) ? sx : 1; _textsize_y = (sy > 0) ? sy : 1; }
    int16_t getTextSizeX(void) const { return _textsize_x; }
    int16_t getTextSizeY(void) const { return _textsize_y; }

    void setTextDatum( uint8_t datum) { _textdatum = datum; }
    void setTextWrap( bool wrapX, bool wrapY = false) { _textwrapX = wrapX; _textwrapY = wrapY; }
    void setTextScroll(bool scroll) { _textscroll = scroll; if (_cursor_x < _sx) { _cursor_x = _sx; } if (_cursor_y < _sy) { _cursor_y = _sy; } }

    virtual void setTextFont(uint8_t f) {
      _textfont = (f > 0) ? f : 1;
      _decoderState = utf8_decode_state_t::utf8_no_use;
      _font_size.xoffset = _font_size.yoffset = 0;
      if (_textfont > 1 && fontdata[_textfont].height != 0) {
        _font_size.height = _font_size.yadvance = pgm_read_byte( &fontdata[_textfont].height );
        fpDrawCharClassic = (_textfont == 2) 
                          ? drawCharBMP
                          : drawCharRLE;
        fpUpdateFontSize = updateFontSizeBMP;
      } else {
        _font_size.width  = 6;
        _font_size.height = 8;
//      _font_size.xadvance = 6;
        _font_size.yadvance = 8;
        fpDrawCharClassic = drawCharGLCD;
        fpUpdateFontSize = nullptr; // updateFontSizeGLCD;
      }
    }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------

  protected:
    uint32_t _transaction_count = 0;
    int32_t _width = 0;
    int32_t _height = 0;
    int32_t _cursor_x = 0;
    int32_t _cursor_y = 0;
    int32_t  _sx, _sy, _sw, _sh; // for scroll zone
    uint32_t _textcolor = 0xFFFFFFFF;
    uint32_t _textbgcolor = 0;
    uint32_t _scolor;  // gap fill colour for scroll zone
    raw_color_t _color = 0xFFFFFFFF;

    dev_color_t _write_depth;
    dev_color_t _read_depth;

    int16_t _textsize_x = 1;
    int16_t _textsize_y = 1;
    int16_t _xpivot;   // x pivot point coordinate
    int16_t _ypivot;   // x pivot point coordinate
    uint16_t _decoderBuffer= 0;   // Unicode code-point buffer
    uint8_t _rotation = 0;
    uint8_t _textfont = 1;
    uint8_t _textdatum;
    bool _has_palette = false;
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
      int16_t xoffset = 0;
      int16_t yoffset = 0;
      int16_t width = 6;
      int16_t height = 8;
//    int16_t xadvance = 6;
      int16_t yadvance = 8;
    };
    font_size_t _font_size;

    template <typename T>
    __attribute__ ((always_inline)) inline static void swap_coord(T& a, T& b) { T t = a; a = b; b = t; }
    __attribute__ ((always_inline)) inline static bool _adjust(int32_t& x, int32_t& w) { if (w < 0) { x += w + 1; w = -w; } return 0==w; }

    static bool _adjust_width(int32_t& x, int32_t& dx, int32_t& dw, int32_t _width)
    {
      if ((dw < 1) || (x >= _width)) return true;
      if (x < 0) { dw += x; dx = -x; x = 0; }
      if ((x + dw) > _width ) dw = _width  - x;
      return (dw < 1);
    }

    virtual rgb565_t readPixel16_impl(int32_t x, int32_t y) { return 0; }
    virtual uint32_t readPixelRAW_impl(int32_t x, int32_t y) { return 0; }

    void read_rect(int32_t x, int32_t y, int32_t w, int32_t h, void* data, void(LGFXBase::*fp_read_pixels)(void*, int32_t, pixelcopy_param_t* param))
    {
      if (_adjust(x,w) || _adjust(y,h)) return;
      if ((x >= _width) || (y >= _height)) return;
      if (x < 0) { w += x; x = 0; }
      if ((x + w) > _width)  w = _width  - x;
      if (w < 1) return;
      if (y < 0) { h += y; y = 0; }
      if ((y + h) > _height) h = _height - y;
      if (h < 1) return;

      pixelcopy_param_t p;
      startWrite();
      readWindow_impl(x, y, x + w - 1, y + h - 1);
      (this->*fp_read_pixels)(data, w * h, &p);
      endRead_impl();
      endWrite();
    }
    void push_image(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, const void* palette, const uint8_t bits, void(LGFXBase::*fp_write_pixels)(const void*, int32_t, pixelcopy_param_t*))
    {
      int32_t dx=0, dw=w;
      if (_adjust_width(x, dx, dw, _width)) return;
      int32_t dy=0, dh=h;
      if (_adjust_width(y, dy, dh, _height)) return;

      pixelcopy_param_t param;
      param.src_palette = palette;

      startWrite();
      setWindow(x, y, x + dw - 1, y + dh - 1);
      bool indivisible = (w * bits) & 7;
      const int32_t len = indivisible + (w * bits >> 3);
      const uint8_t* src = (const uint8_t*)data + (dx * bits >> 3) + dy * len;
      if (dw == w && !indivisible) {
        (this->*fp_write_pixels)(src, dw * dh, &param);
      } else {
        uint8_t offset = (-dx * bits) & 7;
        do {
          param.src_offset = offset;
          (this->*fp_write_pixels)(src, dw, &param);
          src += len;
        } while (--dh);
      }
      endWrite();
    }
/*
    void push_image(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, const void* palette, uint32_t transp, const uint8_t bits, void(LGFXBase::*fp_write_pixels)(const void*, int32_t, pixelcopy_param_t*))
    {
      int32_t dx=0, dw=w;
      if (_adjust_width(x, dx, dw, _width)) return;
      int32_t dy=0, dh=h;
      if (_adjust_width(y, dy, dh, _height)) return;

      pixelcopy_param_t param;
      param.src_palette = palette;

      startWrite();
      bool indivisible = (w * bits) & 7;
      const int32_t len = indivisible + (w * bits >> 3);
      const uint8_t* src = (const uint8_t*)data + dy * len;
      const uint32_t colormask = (1 << bits)-1;
      transp &= colormask;
      do {
        int32_t i, j = 0;
        uint8_t offset = (-dx * bits) & 7;
        for (i = 0; i < dw; i++) {
          offset = (offset - bits) & 7;
          if ((((*(uint32_t*)&src[(dx+i)*bits >> 3]) >> offset) & colormask) == transp) {
            if (j != i) {
              setWindow(x + j, y, x + i, y);
              param.src_offset = (-(dx+j) * bits) & 7;
              (this->*fp_write_pixels)(&src[(dx+j)*bits >> 3], i - j, &param);
            }
            j = i + 1;
          }
        }
        if (j != i) {
          setWindow(x + j, y, x + i, y);
          param.src_offset = (-(dx+j) * bits) & 7;
          (this->*fp_write_pixels)(&src[(dx+j)*bits >> 3], i - j, &param);
        }
        y++;
        src += len;
      } while (--dh);

      endWrite();
    }
/*/


static constexpr uint32_t FP_SCALE = 16;

struct pixelcopy_t {
  int32_t src_x = 0;
  int32_t src_x_add = 1 << FP_SCALE;
  int32_t src_y = 0;
  int32_t src_y_add = 0;
  int32_t src_width = 0;
  swap888_t* palette = nullptr;
  uint32_t transp = ~0;
  int32_t dst_index = 0;
  uint32_t src_bits = 8;
  uint32_t dst_bits = 8;
  uint8_t src_mask = ~0;
  uint8_t dst_mask = ~0;
  __attribute__ ((always_inline)) inline int32_t idx(void) { return ((src_x += src_x_add) >> FP_SCALE) + ((src_y += src_y_add) >> FP_SCALE) * src_width; }

static int32_t rotate_raw_template(uint8_t* dst, const uint8_t* src, int32_t len, pixelcopy_t& param)
{
  int32_t res = 0;
  uint32_t transp = param.transp;
  uint8_t src_bits = param.src_bits;
  uint8_t src_mask = param.src_mask;
  do {
    int32_t idx = param.idx() * src_bits;
    uint32_t raw = (src[idx >> 3] >> (-(idx + src_bits) & 7)) & src_mask;
    if (transp != raw) {
      idx = (param.dst_index + res) * param.dst_bits;
      uint32_t shift = (-(idx + param.dst_bits) & 7);
      dst[idx >> 3] = (dst[idx >> 3] & ~(param.dst_mask << shift)) | ((param.dst_mask & raw) << shift);
    }
  } while (++res < len);
  return res;
}
template <typename TDst>
static int32_t rotate_palette_template(uint8_t* dst, const uint8_t* src, int32_t len, pixelcopy_t& param)
{
  auto d = (TDst*)dst;
  int32_t res = 0;
  uint32_t transp = param.transp;
  uint8_t src_mask = (1 << param.src_bits) - 1;
//  if (transp != ~0) {
    do {
      int32_t idx = param.idx() * param.src_bits;
      uint8_t raw = (src[idx >> 3] >> (-(idx + param.src_bits) & 7)) & src_mask;
      if (transp == raw) break;
      d[res] = param.palette[raw];
    } while (++res < len);
/*
  } else {
    do {
      int32_t idx = param.idx() * param.bits;
      d[res] = param.palette[(src[idx >> 3] >> (-(idx + param.bits) & 7)) & param.mask];
    } while (++res < len);
  }
//*/
  return res;
}
template <typename TDst, typename TSrc>
static int32_t rotate_template(uint8_t* dst, const uint8_t* src, int32_t len, pixelcopy_t& param)
{
  auto s = (const TSrc*)src;
  auto d = (TDst*)dst;
  int32_t res = 0;
  uint32_t transp = param.transp;
//  if (transp != ~0) {
    do {
      int32_t idx = param.idx();
      if (transp == (uint32_t)s[idx]) break;
      d[res] = s[idx];
    } while (++res < len);
/*
  } else {
    do {
      d[res] = s[param.idx()];
    } while (++res < len);
  }
//*/
  return res;
}
};

auto get_rotate_palette_fp(LGFXBase* dst, LGFXBase* src) -> int32_t(*)(uint8_t*, const uint8_t*, int32_t, pixelcopy_t&)
{
  auto dst_depth = dst->getWriteDepth().depth;
  if (dst->hasPalette()) {
    return pixelcopy_t::rotate_raw_template;
  }
  switch (dst_depth) {
  case rgb565_2Byte: return pixelcopy_t::rotate_palette_template<swap565_t>;
  case rgb888_3Byte: return pixelcopy_t::rotate_palette_template<swap888_t>;
  case rgb666_3Byte: return pixelcopy_t::rotate_palette_template<swap666_t>;
  case rgb332_1Byte: return pixelcopy_t::rotate_palette_template<rgb332_t >;
  default: break;
  }
  return nullptr;
}

auto get_rotate_fp(LGFXBase* dst, LGFXBase* src) -> int32_t(*)(uint8_t*, const uint8_t*, int32_t, pixelcopy_t&)
{
  auto dst_depth = dst->getWriteDepth().depth;
  auto src_depth = src->getReadDepth().depth;
  if (dst->hasPalette()) {
    return pixelcopy_t::rotate_raw_template;
  }
  switch (dst_depth) {
  case rgb565_2Byte:
    if (     src_depth == rgb332_1Byte) { return pixelcopy_t::rotate_template<swap565_t, rgb332_t >; }
    else if (src_depth == rgb565_2Byte) { return pixelcopy_t::rotate_template<swap565_t, swap565_t>; }
    else if (src_depth == rgb666_3Byte) { return pixelcopy_t::rotate_template<swap565_t, swap666_t>; }
    else if (src_depth == rgb888_3Byte) { return pixelcopy_t::rotate_template<swap565_t, swap888_t>; }
    break;
  case rgb888_3Byte:
    if (     src_depth == rgb332_1Byte) { return pixelcopy_t::rotate_template<swap888_t, rgb332_t >; }
    else if (src_depth == rgb565_2Byte) { return pixelcopy_t::rotate_template<swap888_t, swap565_t>; }
    else if (src_depth == rgb666_3Byte) { return pixelcopy_t::rotate_template<swap888_t, swap666_t>; }
    else if (src_depth == rgb888_3Byte) { return pixelcopy_t::rotate_template<swap888_t, swap888_t>; }
    break;
  case rgb666_3Byte:
    if (     src_depth == rgb332_1Byte) { return pixelcopy_t::rotate_template<swap666_t, rgb332_t >; }
    else if (src_depth == rgb565_2Byte) { return pixelcopy_t::rotate_template<swap666_t, swap565_t>; }
    else if (src_depth == rgb666_3Byte) { return pixelcopy_t::rotate_template<swap888_t, swap888_t>; }// swap888_t is chosen to reduce the generation of template functions.
    else if (src_depth == rgb888_3Byte) { return pixelcopy_t::rotate_template<swap666_t, swap888_t>; }
    break;
  case rgb332_1Byte:
    if (     src_depth == rgb332_1Byte) { return pixelcopy_t::rotate_template<rgb332_t, rgb332_t >; }
    else if (src_depth == rgb565_2Byte) { return pixelcopy_t::rotate_template<rgb332_t, swap565_t>; }
    else if (src_depth == rgb666_3Byte) { return pixelcopy_t::rotate_template<rgb332_t, swap666_t>; }
    else if (src_depth == rgb888_3Byte) { return pixelcopy_t::rotate_template<rgb332_t, swap888_t>; }
    break;
  }
  return nullptr;
}

    void push_image(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, const void* palette, uint32_t transp, const uint8_t bits, void(LGFXBase::*fp_write_pixels)(const void*, int32_t, pixelcopy_param_t*))
    {
      int32_t dx=0, dw=w;
      if (_adjust_width(x, dx, dw, _width)) return;
      int32_t dy=0, dh=h;
      if (_adjust_width(y, dy, dh, _height)) return;

      pixelcopy_param_t param;
      param.src_palette = palette;

      startWrite();
      bool indivisible = (w * bits) & 7;
      const int32_t len = indivisible + (w * bits >> 3);
      const uint8_t* src = (const uint8_t*)data + dy * len;
      const uint32_t colormask = (1 << bits)-1;
      transp &= colormask;
      do {
        int32_t i, j = 0;
        uint8_t offset = (-dx * bits) & 7;
        for (i = 0; i < dw; i++) {
          offset = (offset - bits) & 7;
          if ((((*(uint32_t*)&src[(dx+i)*bits >> 3]) >> offset) & colormask) == transp) {
            if (j != i) {
              setWindow(x + j, y, x + i, y);
              param.src_offset = (-(dx+j) * bits) & 7;
              (this->*fp_write_pixels)(&src[(dx+j)*bits >> 3], i - j, &param);
            }
            j = i + 1;
          }
        }
        if (j != i) {
          setWindow(x + j, y, x + i, y);
          param.src_offset = (-(dx+j) * bits) & 7;
          (this->*fp_write_pixels)(&src[(dx+j)*bits >> 3], i - j, &param);
        }
        y++;
        src += len;
      } while (--dh);

      endWrite();
    }
//*/
    template <int TByteSize>
    static void pixel_to_pixel_memcpy(void*& dst, const void*& src, int32_t len, pixelcopy_param_t* p) {
      auto& s = (const uint8_t*&)src;
      auto& d = (uint8_t*&)dst;
      memcpy(d, s, len * TByteSize);
      s += len * TByteSize;
      d += len * TByteSize;
    }

    template <class TDst, class TSrc>
    static void pixel_to_pixel_template(void*& dst, const void*& src, int32_t len, pixelcopy_param_t* p) {
      auto& s = (const TSrc*&)src;
      auto& d = (TDst*&)dst;
      if (std::is_same<TDst, TSrc>::value) {
        memcpy(d, s, len * sizeof(TDst));
        s += len;
        d += len;
      } else {
        do { *d++ = *s++; } while (--len);
      }
    }
    template <class TDst, class TSrc, class TPalette>
    static void palette_to_pixel_template(void*& dst, const void*& src, int32_t len, pixelcopy_param_t* p) {
      auto& s = (const uint8_t*&)src;
      auto& d = (TDst*&)dst;
      auto& src_palette = (TPalette*&)(p->src_palette);
      do {
        if (8 <= TSrc::bits) {
          *d++ = src_palette[*s++];
        } else {
          p->src_offset = (p->src_offset + (8-TSrc::bits)) & 7;
          *d++ = src_palette[(*s >> p->src_offset) & ((1 << TSrc::bits) - 1)];
          if (!p->src_offset) { s++; }
        }
      } while (--len);
    }

    template<class TDst, class TSrc>
    void read_pixels_template(void* dst, int32_t length, pixelcopy_param_t* param)
    {
      if (std::is_same<TDst, TSrc>::value) {
        readBytes_impl((uint8_t*)dst, length * (TDst::bits>>3));
      } else {
        read_pixels(dst, length, param, pixel_to_pixel_template<TDst, TSrc>);
      }
    }
    template<class T>
    auto get_read_pixels_fp(void) -> void(LGFXBase::*)(void*, int32_t, pixelcopy_param_t* param)
    {
      switch (_read_depth.depth) {
      case rgb888_3Byte: return &LGFXBase::read_pixels_template<T, swap888_t>;
      case rgb666_3Byte: return &LGFXBase::read_pixels_template<T, swap666_t>;
      case rgb565_2Byte: return &LGFXBase::read_pixels_template<T, swap565_t>;
      case rgb332_1Byte: return &LGFXBase::read_pixels_template<T, rgb332_t >;
      case palette_4bit:
//    case palette_2bit:
      case palette_1bit:
      default: break;
      }
      return &LGFXBase::read_pixels_template<T, T>;
    }

    template<class TDst, class TSrc>
    void write_pixels_template(const void* src, int32_t length, pixelcopy_param_t* param)
    {
      if (std::is_same<TDst, TSrc>::value) {
        writeBytes_impl((const uint8_t*)src, length * (TSrc::bits>>3));
      } else {
        write_pixels(src, length, param, pixel_to_pixel_template<TDst, TSrc>);
      }
    }
    template<class T>
    auto get_write_pixels_fp(void) -> void(LGFXBase::*)(const void*, int32_t, pixelcopy_param_t*)
    {
      switch (getColorDepth()) {
      case rgb888_3Byte: return &LGFXBase::write_pixels_template<swap888_t, T>;
      case rgb666_3Byte: return &LGFXBase::write_pixels_template<swap666_t, T>;
      case rgb565_2Byte: return &LGFXBase::write_pixels_template<swap565_t, T>;
      case rgb332_1Byte: return &LGFXBase::write_pixels_template<rgb332_t , T>;
      case palette_4bit:
//    case palette_2bit:
      case palette_1bit:
      default: break;
      }
      return &LGFXBase::write_pixels_template<T, T>;
    }

    template<class TDst, class TSrc, class TPalette>
    void write_palette_template(const void* src, int32_t length, pixelcopy_param_t* param)
    {
      write_pixels(src, length, param, palette_to_pixel_template<TDst, TSrc, TPalette>);
    }

    template<class TSrc, class TPalette>
    auto get_write_palette_fp(void) -> void(LGFXBase::*)(const void*, int32_t, pixelcopy_param_t*)
    {
      switch (getColorDepth()) {
      case rgb888_3Byte: return &LGFXBase::write_palette_template<swap888_t, TSrc, TPalette>;
      case rgb666_3Byte: return &LGFXBase::write_palette_template<swap666_t, TSrc, TPalette>;
      case rgb565_2Byte: return &LGFXBase::write_palette_template<swap565_t, TSrc, TPalette>;
      case rgb332_1Byte: return &LGFXBase::write_palette_template<rgb332_t , TSrc, TPalette>;
      case palette_4bit:
//    case palette_2bit:
      case palette_1bit:
      default: break;
      }
      return nullptr; // &LGFXBase::write_palette_template<T, T>;
    }

    virtual void read_pixels(void* dst, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) {}
    virtual void readBytes_impl(uint8_t* dst, int32_t length) {}
    virtual void write_pixels(const void* src, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) {}
    virtual void writeBytes_impl(const uint8_t* data, int32_t length) {}
    virtual void writeBytesDMA_impl(const uint8_t* src, int32_t len) { writeBytes_impl(src, len); }
    virtual void beginTransaction_impl() {}
    virtual void endTransaction_impl() {}
    virtual void flush_impl() {}
    virtual void setRotation_impl(uint8_t rotation) {}
    virtual void* setColorDepth_impl(color_depth_t bpp) { return nullptr; }
    virtual void writeColor_impl(int32_t len) = 0;
    virtual void drawPixel_impl(int32_t x, int32_t y) = 0;
    virtual void fillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) = 0;
    virtual void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) {}
    virtual void readWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) {}
    virtual void endRead_impl(void) {}

/*
    virtual void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y)
    {
      void(*fp_pixel_to_pixel)(void*& dst, const void* &src, int32_t len, pixelcopy_param_t* p) = nullptr;

      if (_read_depth.depth != _write_depth.depth) {
        switch (_read_depth.depth) {
        case rgb888_3Byte:
          switch (_write_depth.depth) {
          case rgb565_2Byte: fp_pixel_to_pixel = pixel_to_pixel_template<swap565_t, swap888_t>; break;
          default: return;
          }
          break;
        case rgb666_3Byte:
          switch (_write_depth.depth) {
          case rgb565_2Byte: fp_pixel_to_pixel = pixel_to_pixel_template<swap565_t, swap666_t>; break;
          default: return;
          }
          break;
        default: return;
        }
      }

      startWrite();
      const uint32_t readlen = w * _read_depth.bytes;
      const uint32_t writelen = w * _write_depth.bytes;
      uint8_t buf[2][(readlen < writelen) ? writelen : readlen];
      bool flip = false;
      int32_t add = (src_y < dst_y) ?   - 1 : 1;
      int32_t pos = (src_y < dst_y) ? h - 1 : 0;
      readWindow_impl(src_x, src_y + pos, src_x + w - 1, src_y + pos);
      readBytes_impl(buf[flip], readlen);
      endRead_impl();
      while (--h) {
        pos += add;
        flip = !flip;
        readWindow_impl(src_x, src_y + pos, src_x + w - 1, src_y + pos);
        readBytes_impl(buf[flip], readlen);
        if (fp_pixel_to_pixel) {
          void* wb = buf[!flip];
          const void* rb = wb;
          fp_pixel_to_pixel(wb, rb, w, nullptr);
        }
        endRead_impl();
        setWindow_impl(dst_x, dst_y + pos - add, dst_x + w - 1, dst_y + pos - add);
        write_bytes_dma(buf[!flip], writelen);
      }
      if (fp_pixel_to_pixel) {
        void* wb = buf[flip];
        const void* rb = wb;
        fp_pixel_to_pixel(wb, rb, w, nullptr);
      }
      setWindow_impl(dst_x, dst_y + pos, dst_x + w - 1, dst_y + pos);
      write_bytes_dma(buf[flip], writelen);

      endWrite();
    }
/*/
//*
    virtual void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y)
    {
      void(LGFXBase::*fp_write_pixels)(const void*, int32_t, pixelcopy_param_t*);
      switch (_read_depth.depth) {
      case rgb888_3Byte: fp_write_pixels = get_write_pixels_fp<swap888_t>(); break;
      case rgb666_3Byte: fp_write_pixels = get_write_pixels_fp<swap666_t>(); break;
//    case rgb565_2Byte: fp_write_pixels = get_write_pixels_fp<swap565_t>(); break;
//    case rgb332_1Byte: fp_write_pixels = get_write_pixels_fp<rgb332_t >(); break;
      default: return;
      }

      startWrite();

      if (w < h) {
        const uint32_t buflen = h * _read_depth.bytes;
        uint8_t buf[buflen+3];
        int32_t add = (src_x < dst_x) ?   - 1 : 1;
        int32_t pos = (src_x < dst_x) ? w - 1 : 0;
        while (w--) {
          readWindow_impl(src_x + pos, src_y, src_x + pos, src_y + h - 1);
          readBytes_impl(buf, buflen);
          endRead_impl();
          setWindow_impl(dst_x + pos, dst_y, dst_x + pos, dst_y + h - 1);
          (this->*fp_write_pixels)(buf, h, nullptr);
          pos += add;
        }
      } else {
        const uint32_t buflen = w * _read_depth.bytes;
        uint8_t buf[buflen+3];
        int32_t add = (src_y < dst_y) ?   - 1 : 1;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        while (h--) {
          readWindow_impl(src_x, src_y + pos, src_x + w - 1, src_y + pos);
          readBytes_impl(buf, buflen);
          endRead_impl();
          setWindow_impl(dst_x, dst_y + pos, dst_x + w - 1, dst_y + pos);
          (this->*fp_write_pixels)(buf, w, nullptr);
          pos += add;
        }
      }
      endWrite();
    }
/*/
    virtual void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y)
    {
      void(LGFXBase::*fp_read_pixels)(void*, int32_t, pixelcopy_param_t*);
      switch (_write_depth.depth) {
      case rgb888_3Byte: fp_read_pixels = get_read_pixels_fp<swap888_t>(); break;
      case rgb666_3Byte: fp_read_pixels = get_read_pixels_fp<swap666_t>(); break;
      case rgb565_2Byte: fp_read_pixels = get_read_pixels_fp<swap565_t>(); break;
//    case rgb332_1Byte: fp_read_pixels = get_read_pixels_fp<rgb332_t >(); break;
      default: return;
      }

      startWrite();

      if (w < h) {
        //const uint32_t buflen = h * _read_depth.bytes;
        const uint32_t buflen = h * _write_depth.bytes;
        uint8_t buf[buflen+4];
        int32_t add = (src_x < dst_x) ?   - 1 : 1;
        int32_t pos = (src_x < dst_x) ? w - 1 : 0;
        while (w--) {
          readWindow_impl(src_x + pos, src_y, src_x + pos, src_y + h - 1);
          //readBytes_impl(buf, buflen);
          (this->*fp_read_pixels)(buf, h, nullptr);
          endRead_impl();
          setWindow_impl(dst_x + pos, dst_y, dst_x + pos, dst_y + h - 1);
          //(this->*fp_write_pixels)(buf, h, nullptr);
          writeBytesDMA(buf, buflen);
          pos += add;
        }
      } else {
        //const uint32_t buflen = w * _read_depth.bytes;
        const uint32_t buflen = w * _write_depth.bytes;
        uint8_t buf[buflen+4];
        int32_t add = (src_y < dst_y) ?   - 1 : 1;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        while (h--) {
          readWindow_impl(src_x, src_y + pos, src_x + w - 1, src_y + pos);
          //readBytes_impl(buf, buflen);
          (this->*fp_read_pixels)(buf, w, nullptr);
          endRead_impl();
          setWindow_impl(dst_x, dst_y + pos, dst_x + w - 1, dst_y + pos);
          //(this->*fp_write_pixels)(buf, w, nullptr);
          writeBytesDMA(buf, buflen);
          pos += add;
        }
      }
      flush();
      endWrite();
    }
//*/

  //----------------------------------------------------------------------------
  // print & text support
  //----------------------------------------------------------------------------
  // Arduino Print.h compatible
    size_t printNumber(unsigned long n, uint8_t base)
    {
      char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
      char *str = &buf[sizeof(buf) - 1];

      *str = '\0';

      if (base < 2) { base = 10; }  // prevent crash if called with base == 1
      do {
        unsigned long m = n;
        n /= base;
        char c = m - base * n;
        *--str = c < 10 ? c + '0' : c + 'A' - 10;
      } while (n);

      return write(str);
    }

    size_t printFloat(double number, uint8_t digits)
    {
      size_t n = 0;
      if (std::isnan(number))    { return print("nan"); }
      if (std::isinf(number))    { return print("inf"); }
      if (number > 4294967040.0) { return print("ovf"); } // constant determined empirically
      if (number <-4294967040.0) { return print("ovf"); } // constant determined empirically

      // Handle negative numbers
      if(number < 0.0) {
        n += print('-');
        number = -number;
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
      n += print(int_part);

      // Print the decimal point, but only if there are digits beyond
      if (digits > 0) n += print(".");

      // Extract digits from the remainder one at a time
      while (digits-- > 0) {
        remainder *= 10.0;
        unsigned int toPrint = (unsigned int)(remainder);
        n += print(toPrint);
        remainder -= toPrint;
      }
      return n;
    }

    bool(*fpUpdateFontSize)(LGFXBase* me, uint16_t uniCode) = nullptr;
    static bool updateFontSizeBMP(LGFXBase* me, uint16_t uniCode) {
      if ((uniCode -= 32) >= 96) return false;
//    me->_font_size.xadvance =
      me->_font_size.width = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[me->_textfont].widthtbl ) ) + uniCode );
      return true;
    }

    int16_t (*fpDrawCharClassic)(LGFXBase* me, int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y) = &LGFXBase::drawCharGLCD;
    static int16_t drawCharGLCD(LGFXBase* me, int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y)
    { // glcd font
      static constexpr int fontWidth  = 6;
      static constexpr int fontHeight = 8;

      if (c > 255) return 0;

      if ((x < me->width())
       && (x + fontWidth * size_x > 0)
       && (y < me->height())
       && (y + fontHeight * size_y > 0)) {
        auto font_addr = font + (c * 5);
        uint32_t colortbl[2] = {bg, color};
        bool fillbg = (bg != color);
        if (fillbg && size_y == 1 && x >= 0 && y >= 0 && y + fontHeight <= me->height() && x + fontWidth * size_x <= me->width()) {
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
          me->writeColorRaw(bg, len);
          me->endWrite();
        } else {
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
                  me->fillRect(xpos, ypos, size_x, len * size_y);
                }
                ypos += len * size_y;
                len = 0;
                flg = !flg;
              }
              len++;
            }
            if (flg || fillbg) {
              me->setColorRaw(colortbl[flg]);
              me->fillRect(xpos, ypos, size_x, len * size_y);
            }
            xpos += size_x;
          }
          if (fillbg) {
            me->setColorRaw(bg);
            me->fillRect(xpos, y, size_x, fontHeight * size_y); 
          }
          me->endWrite();
        }
      }
      return fontWidth * size_x;
    }

    static int16_t drawCharBMP(LGFXBase* me, int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y)
    { // BMP font
      uint16_t uniCode = c - 32;
      const int fontWidth = ((c < 32) || (c > 127)) ? 0 : pgm_read_byte(widtbl_f16 + uniCode);
      constexpr int fontHeight = chr_hgt_f16;

      if ((c < 32) || (c > 127)) return 0;
      auto font_addr = (const uint8_t*)pgm_read_dword(&chrtbl_f16[uniCode]);

      if ((x < me->width())
       && (x + fontWidth * size_x > 0)
       && (y < me->height())
       && (y + fontHeight * size_y > 0)) {
        int32_t ypos = y;
        uint8_t w = (fontWidth + 6) >> 3;
        uint32_t colortbl[2] = {bg, color};
        bool fillbg = (bg != color);
        if (fillbg && size_y == 1 && x >= 0 && y >= 0 && y + fontHeight <= me->height() && x + fontWidth * size_x <= me->width()) {
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
        } else {
          me->startWrite();
          for (uint8_t i = 0; i < fontHeight; i++) {
            uint8_t line = pgm_read_byte(font_addr);
            bool flg = line & 0x80;
            uint32_t len = 1;
            uint8_t j = 1;
            for (; j < fontWidth-1; j++) {
              if (j & 7) {
                line <<= 1;
              } else {
                line = pgm_read_byte(font_addr + (j >> 3));
              }
              if (flg != (bool)(line & 0x80)) {
                if (flg || fillbg) {
                  me->setColorRaw(colortbl[flg]);
                  me->fillRect( x + (j - len) * size_x, ypos, len * size_x, size_y); 
                }
                len = 1;
                flg = !flg;
              } else {
                len++;
              }
            }
            if (flg || fillbg) {
              me->setColorRaw(colortbl[flg]);
              me->fillRect( x + (j - len) * size_x, ypos, len * size_x, size_y); 
            }
            ypos += size_y;
            font_addr += w;
          }
          if (fillbg) {
            me->setColorRaw(bg);
            me->fillRect( x + (fontWidth - 1) * size_x, y, size_x, fontHeight * size_y);
          }
          me->endWrite();
        }
      }

      return fontWidth * size_x;
    }

    static int16_t drawCharRLE(LGFXBase* me, int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y)
    { // RLE font
      auto fontdat = &fontdata[me->getTextFont()];
      uint16_t code = c - 32;
      const int fontWidth = ((c < 32) || (c > 127)) ? 0 : pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdat->widthtbl ) ) + code );
      const int fontHeight = pgm_read_byte( &fontdat->height );

      if ((c < 32) || (c > 127)) return 0;
      auto font_addr = (const uint8_t*)pgm_read_dword( (const void*)(pgm_read_dword( &(fontdat->chartbl ) ) + code * sizeof(void *)) );
      if ((x < me->width())
       && (x + fontWidth * size_x > 0)
       && (y < me->height())
       && (y + fontHeight * size_y > 0)) {
        uint32_t colortbl[2] = {bg, color};
        bool fillbg = (bg != color);
        if (fillbg && size_y == 1 && x >= 0 && y >= 0 && y + fontHeight <= me->height() && x + fontWidth * size_x <= me->width()) {
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
        } else {
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
                me->fillRect( x + j * size_x, y + (i * size_y), len * size_x, size_y);
              }
              j += len;
              if (j == fontWidth) {
                j = 0;
                i++;
              }
            } while (line);
          } while (i < fontHeight);
          me->endWrite();
        }
      }

      return fontWidth * size_x;
    }

  };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

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
      me->_font_size.xoffset = (int8_t)pgm_read_byte(&glyph->xOffset);
      me->_font_size.width  = pgm_read_byte(&glyph->width);
//    me->_font_size.xadvance = pgm_read_byte(&glyph->xAdvance);
      return true;
    }

    static int16_t drawCharGFXFF(LGFXBase* lgfxbase, int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y)
    {
      auto me = (LGFX_GFXFont_Support*)lgfxbase;
      auto gfxFont = me->_gfxFont;
      if (c > pgm_read_word(&gfxFont->last )
      ||  c < pgm_read_word(&gfxFont->first)) return 0;

      c -= pgm_read_word(&gfxFont->first);
      GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c]);

      uint16_t w = pgm_read_byte(&glyph->width),
               h = pgm_read_byte(&glyph->height);

      x += (int8_t)pgm_read_byte(&glyph->xOffset) * size_x;
      y += (int8_t)pgm_read_byte(&glyph->yOffset) * size_y;

      if ((x < me->width())
       && (x + w * size_x > 0)
       && (y < me->height())
       && (y + h * size_y > 0)) {
      //bool fillbg = (bg != color);
        me->setColorRaw(color);
        uint8_t  *bitmap = (uint8_t *)pgm_read_dword(&gfxFont->bitmap)
                         + pgm_read_word(&glyph->bitmapOffset);
        uint8_t  xx, bits=0, bit=0;

        me->startWrite();

        while (h--) {
          int16_t len = 0;
          for (xx=0; xx < w; xx++) {
            if (bit == 0) {
              bit  = 0x80;
              bits = pgm_read_byte(bitmap++);
            }
            if (bits & bit) len++;
            else if (len) {
              me->fillRect(x + (xx-len) * size_x, y, size_x * len, size_y);
              len=0;
            }
            bit >>= 1;
          }
          if (len) {
            me->fillRect(x + (xx-len) * size_x, y, size_x * len, size_y);
          }
          y += size_y;
        }

        me->endWrite();
      }
      return pgm_read_byte(&glyph->xAdvance) * size_x;
    }
  public:

    void setTextFont(uint8_t f) override {
      _gfxFont = nullptr;
      Base::setTextFont(f);
    }

    void setFreeFont(const GFXfont *f = nullptr)
    {
      if (f == nullptr) { this->setTextFont(1); return; } // Use GLCD font
      this->fpDrawCharClassic = drawCharGFXFF;
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

      this->_font_size.yoffset = - _glyph_ab;
      this->_font_size.height = _glyph_bb + _glyph_ab;
      this->_font_size.yadvance = (uint8_t)pgm_read_byte(&_gfxFont->yAdvance);
    }
  };

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
      if (!fontLoaded) return;
      fontLoaded = false;
      fontFile.close();
      if (gUnicode)  { free(gUnicode);  gUnicode = nullptr; }
      if (gWidth)    { free(gWidth);    gWidth   = nullptr; }
      if (gdX)       { free(gdX);       gdX      = nullptr; }
      if (gBitmap)   { free(gBitmap);   gBitmap  = nullptr; }
    }

    void showFont(uint32_t td);
    uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc);

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
    uint16_t* gUnicode = NULL;  //UTF-16 code, the codes are searched so do not need to be sequential
    uint8_t*  gWidth = NULL;    //cwidth
    int8_t*   gdX = NULL;       //leftExtent
    uint32_t* gBitmap = NULL;   //file pointer to greyscale bitmap

    bool     fontLoaded = false; // Flags when a anti-aliased font is loaded

#if defined (ARDUINO) && defined (FS_H)

    void loadFont(const char *path, fs::FS &fs) {
      fontFile.setFS(fs);
      loadFont(path);
    }

#endif

    void loadFont(const char *path) {
      fontFile.need_transaction &= this->_has_transaction;
      if (fontFile.need_transaction && this->_transaction_count) this->endTransaction();

      unloadFont();
      bool result = fontFile.open(path, "rb");
      if (!result) {
        std::string filename = "/";
        filename += path;
        filename += ".vlw";
        result = fontFile.open(filename.c_str(), "rb");
      }
      if (result) {
        {
          uint32_t buf[6];
          fontFile.read((uint8_t*)buf, 6 * 4); // 24 Byte read

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

        fontLoaded = true;
        this->_decoderState = Base::utf8_decode_state_t::utf8_state0;
        this->fpDrawCharClassic = drawCharVLW;
        this->fpUpdateFontSize = updateFontSizeVLW;
        this->_font_size.xoffset = 0;
        this->_font_size.yoffset = 0;

        // Fetch the metrics for each glyph
        loadMetrics(gFont.gCount);

        this->_font_size.height   = gFont.yAdvance;
        this->_font_size.yadvance = gFont.yAdvance;
      } else {
        this->setTextFont(1);
      }
      if (fontFile.need_transaction && this->_transaction_count) { this->beginTransaction(); }
    }

  private:

    FileWrapper fontFile;

    void loadMetrics(uint16_t gCount)
    {
      if (!gCount) return;

      uint32_t bitmapPtr = 24 + (uint32_t)gCount * 28;

      {
        gBitmap   = (uint32_t*)malloc( gCount * 4); // seek pointer to glyph bitmap in the file
        gUnicode  = (uint16_t*)malloc( gCount * 2); // Unicode 16 bit Basic Multilingual Plane (0-FFFF)
        gWidth    =  (uint8_t*)malloc( gCount );    // Width of glyph
        gdX       =   (int8_t*)malloc( gCount );    // offset for bitmap left edge relative to cursor X
      }

      uint16_t gNum = 0;
      fontFile.seek(24);  // headerPtr
      uint32_t buffer[7];
      do {
        fontFile.read((uint8_t*)buffer, 7 * 4); // 28 Byte read
        uint16_t unicode = __builtin_bswap32(buffer[0]); // Unicode code point value
        gUnicode[gNum]  = unicode;
        gWidth[gNum]    = (uint8_t)__builtin_bswap32(buffer[2]); // Width of glyph
        gdX[gNum]       =  (int8_t)__builtin_bswap32(buffer[5]); // x delta from cursor

        uint16_t height = __builtin_bswap32(buffer[1]); // Height of glyph
        if ((unicode > 0xFF) || ((unicode > 0x20) && (unicode < 0xA0) && (unicode != 0x7F))) {
          int16_t dY =  (int16_t)__builtin_bswap32(buffer[4]); // y delta from baseline
          if (gFont.maxDescent < (height - dY)) {
//ESP_LOGI("LGFX", "maxDescent:%d", gFont.maxDescent);
            gFont.maxDescent = height - dY;
          }
        }

        gBitmap[gNum] = bitmapPtr;
        bitmapPtr += (uint16_t)gWidth[gNum] * height;
      } while (++gNum < gCount);

      this->_font_size.yadvance = gFont.yAdvance = gFont.maxAscent + gFont.maxDescent;
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
        me->_font_size.width = me->gWidth[gNum];
        me->_font_size.xoffset = me->gdX[gNum];
        return true;
      }
      return false;
    }

    static int16_t drawCharVLW(LGFXBase* lgfxbase, int32_t x, int32_t y, uint16_t code, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y)
    {
      auto me = (LGFX_VLWFont_Support*)lgfxbase;

      uint16_t gNum = 0;
      if (!me->getUnicodeIndex(code, &gNum)) {
        // Not a Unicode in font so draw a rectangle and move on cursor
        me->setColorRaw(color);
        me->drawRect(x, y + me->gFont.maxAscent - me->gFont.ascent, me->gFont.spaceWidth, me->gFont.ascent);
        return me->gFont.spaceWidth + 1;
      }

      auto file = &me->fontFile;

      if (file->need_transaction) me->endTransaction();

      file->seek(28 + gNum * 28);  // headerPtr
      uint32_t buffer[6];
      file->read((uint8_t*)buffer, 24);
      uint32_t h        = __builtin_bswap32(buffer[0]); // Height of glyph
      uint32_t w        = __builtin_bswap32(buffer[1]); // Width of glyph
      uint32_t xAdvance = __builtin_bswap32(buffer[2]); // xAdvance - to move x cursor
      int32_t dY        = (int16_t)__builtin_bswap32(buffer[3]); // y delta from baseline
      int32_t dX        =  (int8_t)__builtin_bswap32(buffer[4]); // x delta from cursor

      int32_t cy = y + (int32_t)(me->gFont.maxAscent - dY) * (int32_t)size_y;
      int32_t cx = x + dX * (int32_t)size_x;
      uint8_t pbuffer[w * h];
      uint8_t* pixel = pbuffer;

      file->seek(me->gBitmap[gNum]);  // headerPtr
      file->read(pixel, w * h);
      if (file->need_transaction) me->beginTransaction();

      me->startWrite();
      for (int y = 0; y < h; y++) {
        uint32_t dl = 0;
        for (int x = 0; x < w; x++) {
          if (*pixel == 0xFF) dl++;
          else {
            if (dl) {
              me->setColorRaw(color);
              me->fillRect((x - dl) * size_x + cx, cy, dl * size_x, size_y);
              dl = 0;
            }
            if (*pixel) {
// todo : alpha blending
me->setColor(color888(*pixel,*pixel,*pixel)); // color);
me->fillRect(x * size_x + cx, cy, size_x, size_y);
            }
          }
          ++pixel;
        }
        if (dl) {
          me->setColorRaw(color);
          me->fillRect((w - dl) * size_x + cx, cy, dl * size_x, size_y);
        }
        cy += size_y;
      }
      me->endWrite();

      return xAdvance * size_x;
    }
  };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  template <class Base>
  class LGFX_BMP_Support : public Base {
  public:

#if defined (ARDUINO) && defined (FS_H)

    inline void drawBmp(fs::FS &fs, const char *path, int32_t x, int32_t y) { drawBmpFile(fs, path, x, y); }
    void drawBmpFile(fs::FS &fs, const char *path, int32_t x, int32_t y) {
      FileWrapper file;
      file.setFS(fs);
      drawBmpFile(file, path, x, y);
    }

#elif defined (CONFIG_IDF_TARGET_ESP32)  // ESP-IDF

    void drawBmpFile(const char *path, int32_t x, int32_t y) {
      FileWrapper file;
      drawBmpFile(file, path, x, y);
    }

#endif

  private:

    void drawBmpFile(FileWrapper& file, const char *path, int32_t x, int32_t y) {
      file.need_transaction &= this->_has_transaction;
      if (file.need_transaction) this->endTransaction();
      if (file.open(path, "rb")) {
        drawBmpFile(file, x, y);
        file.close();
      }
      if (file.need_transaction && this->_transaction_count) { this->beginTransaction(); }
    }
    void drawBmpFile(FileWrapper& file, int32_t x, int32_t y) {
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
        file.read(bmpdata.raw, sizeof(bmpdata));
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

      void(LGFXBase::*fp)(const void*, int32_t, pixelcopy_param_t*) = nullptr;
      switch (bpp) {
      case  1: fp = this->template get_write_palette_fp<palette1_t, argb8888_t>(); break;
      case  4: fp = this->template get_write_palette_fp<palette4_t, argb8888_t>(); break;
      case  8: fp = this->template get_write_palette_fp<rgb332_t  , argb8888_t>(); break;
      case 16: fp = this->template get_write_pixels_fp<rgb565_t>()               ; break;
      case 24: fp = this->template get_write_pixels_fp<rgb888_t>()               ; break;
      case 32: fp = this->template get_write_pixels_fp<argb8888_t>()             ; break;
      default: return;
      }

        //If the value of Height is positive, the image data is from bottom to top
        //If the value of Height is negative, the image data is from top to bottom.
      int32_t flow = (h < 0) ? 1 : -1;
      if (h < 0) h = -h;
      else y += h - 1;

      argb8888_t *palette = nullptr;
      if (bpp <= 8) {
        palette = new argb8888_t[1 << bpp];
        file.seek(seekOffset-(1 << bpp)*sizeof(argb8888_t));
        file.read((uint8_t*)palette, (1 << bpp)*sizeof(argb8888_t)); // load palette
      }

      file.seek(seekOffset);

      uint8_t lineBuffer[((w * bpp + 31) >> 5) << 2];  // readline 4Byte align.

      while (h--) {
        if (file.need_transaction) this->endTransaction();
        file.read(lineBuffer, sizeof(lineBuffer));
        if (file.need_transaction) this->beginTransaction();

        this->push_image(x, y, w, 1, lineBuffer, (void*)palette, bpp, fp);
        y += flow;
      }

      if (palette) delete[] palette;
      //Serial.print("Loaded in "); Serial.print(millis() - startTime);   Serial.println(" ms");
    }
  };

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

  class LovyanGFX : public
  #ifdef LOAD_GFXFF
   LGFX_GFXFont_Support<
  #endif
    LGFX_VLWFont_Support<
     LGFX_BMP_Support<
      LGFXBase
     >
    >
  #ifdef LOAD_GFXFF
   >
  #endif
  {};
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)

  #include "platforms/lgfx_spi_esp32.hpp"

#elif defined (ESP8266)

  #include "platforms/lgfx_spi_esp8266.hpp"

#elif defined (STM32F7)

  #include "platforms/lgfx_spi_stm32_spi.hpp"

#elif defined (__AVR__)

  #include "platforms/lgfx_spi_avr.hpp"

#endif

#include "platforms/lgfx_sprite.hpp"

#include "platforms/panel_ILI9341.hpp"
#include "platforms/panel_ILI9163.hpp"
#include "platforms/panel_ST7735.hpp"
#include "platforms/panel_ST7789.hpp"
#include "platforms/panel_ssd_common.hpp"

#endif
