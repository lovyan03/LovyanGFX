#ifndef LOVYANGFX_HPP_
#define LOVYANGFX_HPP_

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <cmath>
#include <cfloat>
#include <type_traits>

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


    template<typename T> __attribute__ ((always_inline)) inline void setColor(T c) { _color.setColor(c); }

    template<typename T> inline void setTextColor(T c)      { _textcolor = _textbgcolor = _color.convertColor(c); }
    template<typename T> inline void setTextColor(T c, T b) { _textcolor = _color.convertColor(c); _textbgcolor = _color.convertColor(b); }

                         inline void clear         ( void )          { _color.raw = 0;  fillRect(0, 0, _width, _height); }
    template<typename T> inline void clear         ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }
    template<typename T> inline void fillScreen    ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }

    template<typename T> inline void writeColor    ( const T& color, int32_t length) { if (0 >= length) return; setColor(color);               writeColor_impl(length);             }
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
    template<typename T> inline void pushIndexImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data, const rgb888_t* palette)                       { push_image(x, y, w, h, data, palette                          , T::bits, get_write_palette_fp<T>()); }
    template<typename T> inline void pushIndexImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data, const rgb888_t* palette, uint32_t transparent) { push_image(x, y, w, h, data, palette,              transparent, T::bits, get_write_palette_fp<T>()); }
    template<typename T> inline void pushIndexImage( int32_t x, int32_t y, int32_t w, int32_t h, const T* data, const rgb888_t* palette, const T& transparent) { push_image(x, y, w, h, data, palette, *(uint32_t*)&transparent, T::bits, get_write_palette_fp<T>()); }

    __attribute__ ((always_inline)) inline static uint8_t  color332(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color332(r, g, b); }
    __attribute__ ((always_inline)) inline static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color565(r, g, b); }
    __attribute__ ((always_inline)) inline static uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color888(r, g, b); }

    __attribute__ ((always_inline)) inline void setPivot(int16_t x, int16_t y) { _xpivot = x; _ypivot = y; }
    __attribute__ ((always_inline)) inline int16_t getPivotX(void) const { return _xpivot; }
    __attribute__ ((always_inline)) inline int16_t getPivotY(void) const { return _ypivot; }

    __attribute__ ((always_inline)) inline int32_t width        (void) const { return _width; }
    __attribute__ ((always_inline)) inline int32_t height       (void) const { return _height; }
    __attribute__ ((always_inline)) inline uint8_t getRotation  (void) const { return _rotation; }
    __attribute__ ((always_inline)) inline color_depth_t getColorDepth(void) const { return _color.depth; }
    __attribute__ ((always_inline)) inline bool getInvert       (void) const { return _invert; }
    __attribute__ ((always_inline)) inline bool getSwapBytes    (void) const { return _swapBytes; }
    __attribute__ ((always_inline)) inline void setSwapBytes(bool swap) { _swapBytes = swap; }
    __attribute__ ((always_inline)) inline void invertDisplay(bool invert) { invertDisplay_impl(invert); }
    __attribute__ ((always_inline)) inline void setRotation(uint8_t rotation) { setRotation_impl(rotation); }
    __attribute__ ((always_inline)) inline void* setColorDepth(uint8_t bpp)       { return setColorDepth_impl((color_depth_t)bpp); }
    __attribute__ ((always_inline)) inline void* setColorDepth(color_depth_t bpp) { return setColorDepth_impl(bpp); }

    __attribute__ ((always_inline)) inline void startWrite(void) { if (0 == _start_write_count++) { beginTransaction(); } }
    __attribute__ ((always_inline)) inline void endWrite(void)   { if (_start_write_count) { if (0 == (--_start_write_count)) endTransaction(); } }
    __attribute__ ((always_inline)) inline void beginTransaction(void) { beginTransaction_impl(); }
    __attribute__ ((always_inline)) inline void endTransaction(void)   { endTransaction_impl(); }
    __attribute__ ((always_inline)) inline void flush(void) {}
    __attribute__ ((always_inline)) inline void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) { setWindow_impl(xs, ys, xe, ye); }

    void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      _adj(x,w);
      _adj(y,h);
      startWrite();
      setWindow(x, y, x + w - 1, y + h - 1);
      endWrite();
    }

    __attribute__ ((always_inline)) inline void drawPixel(int32_t x, int32_t y)
    {
      if (x < 0 || (x >= _width) || y < 0 || (y >= _height)) return;
      drawPixel_impl(x, y);
    }

    void drawFastVLine(int32_t x, int32_t y, int32_t h)
    {
      if ((x < 0) || (x >= _width)) return;
      if (_adj(y,h)) return; 
      if (y < 0) { h += y; y = 0; }
      if ((y + h) > _height) h = _height - y;
      if (h < 1) return;

      fillRect_impl(x, y, 1, h);
    }

    void drawFastHLine(int32_t x, int32_t y, int32_t w)
    {
      if ((y < 0) || (y >= _height)) return;
      if (_adj(x,w)) return; 
      if (x < 0) { w += x; x = 0; }
      if ((x + w) > _width) w = _width - x;
      if (w < 1) return;

      fillRect_impl(x, y, w, 1);
    }

    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h)
    {
      if (_adj(x,w)||_adj(y,h)) return; 
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
      if (_adj(x,w)||_adj(y,h)) return;
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
      int32_t dy = r << 1;
      dy -= 2;
      p -= dy;
      int32_t len = (i << 1)+1;
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
          dy -= 2;
          p -= dy;
          drawFastVLine(x - r, y - i          , len);
          drawFastVLine(x + r, y - i          , len);
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
      if (_adj(x,w)||_adj(y,h)) return; 
      startWrite();

      w--;
      h--;
      int32_t len = (r << 1) + 1;
      drawFastVLine(x      , y + r+1, h - len);
      drawFastVLine(x + w  , y + r+1, h - len);
      drawFastHLine(x + r+1, y      , w - len);
      drawFastHLine(x + r+1, y + h  , w - len);

      int32_t x0 = x + r;
      int32_t x1 = x + w - r;
      int32_t y0 = y + r;
      int32_t y1 = y + h - r;

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
      if (_adj(x,w)||_adj(y,h)) return; 
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
          ddF_y += 2;
          f     += ddF_y;
          drawFastHLine(x0 - i, y1 + r, (i << 1) + delta);
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
      pushDMA_impl(src, len);
      endWrite();
    }

    uint16_t readPixel(int32_t x, int32_t y)
    {
      //rgb565_t buf;
      //read_rect(x, y, 1, 1, &buf, get_read_pixels_fp<rgb565_t>());
      //return buf.raw;
      if (x < 0 || (x >= _width) || y < 0 || (y >= _height)) return 0;
      return readPixel16_impl(x, y).raw;
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
      if (len < 0) {
        va_end(arg);
        return 0;
      };
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

      uint16_t uniCode = decodeUTF8(utf8);
      if (uniCode == 0) return 1;

      _drawCharMoveCursor = true;
      drawChar(uniCode, _cursor_x, _cursor_y, _textfont);
      _drawCharMoveCursor = false;

      return 1;
    }

    uint16_t decodeUTF8(uint8_t c)
    {
      // 7 bit Unicode Code Point
      if ((c & 0x80) == 0x00) {
        _decoderState = 0;
        return (uint16_t)c;
      }

      if (_decoderState == 0)
      {
        // 11 bit Unicode Code Point
        if ((c & 0xE0) == 0xC0)
        {
          _decoderBuffer = ((c & 0x1F)<<6);
          _decoderState = 1;
          return 0;
        }

        // 16 bit Unicode Code Point
        if ((c & 0xF0) == 0xE0)
        {
          _decoderBuffer = ((c & 0x0F)<<12);
          _decoderState = 2;
          return 0;
        }
        // 21 bit Unicode  Code Point not supported so fall-back to extended ASCII
        //if ((c & 0xF8) == 0xF0) return (uint16_t)c;
      }
      else
      {
        if (_decoderState == 2)
        {
          _decoderBuffer |= ((c & 0x3F)<<6);
          _decoderState--;
          return 0;
        }
        else
        {
          _decoderBuffer |= (c & 0x3F);
          _decoderState = 0;
          return _decoderBuffer;
        }
      }

      _decoderState = 0;

      return (uint16_t)c; // fall-back to extended ASCII
    }

    inline int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y) { return drawChar(uniCode, x, y, _textfont); }
    inline int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font)
    {
      return drawChar(x, y, uniCode, _textcolor, _textbgcolor, _textsize_x, _textsize_y);
    }

    int16_t (LGFXBase::*fpDrawCharClassic)(int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y) = &LGFXBase::drawCharGLCD;
    inline int16_t drawChar(int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size)                   { return (this->*fpDrawCharClassic)(x, y, c, color, bg, size, size); }
    inline int16_t drawChar(int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y) { return (this->*fpDrawCharClassic)(x, y, c, color, bg, size_x, size_y); }
    int16_t drawCharGLCD(int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y)
    { // glcd font
      static constexpr int fontWidth  = 6;
      static constexpr int fontHeight = 8;

      if (_drawCharMoveCursor) {
        if (c == '\n') {
          _cursor_x  = 0;
          _cursor_y = y + (int16_t)size_y * fontHeight;
          return 0;
        } else
        if (_textwrapX && ((x + (int16_t)size_x * (fontWidth)) > _width)) {
          x  = 0;
          y += (int16_t)size_y * fontHeight;
        }
        _cursor_x = x;
        _cursor_y = y;
      }
      if (c < 32) return 0;

      if ((x < _width)
       && (x + fontWidth * size_x > 0)
       && (y < _height)
       && (y + fontHeight * size_y > 0)) {
        auto font_addr = font + (c * 5);
        uint32_t colortbl[2] = {bg, color};
        bool fillbg = (bg != color);
        if (fillbg && size_y == 1 && x >= 0 && y >= 0 && y + fontHeight <= _height && x + fontWidth * size_x <= _width) {
          uint8_t col[fontWidth];
          for (uint8_t i = 0; i < 5; i++) {
            col[i] = pgm_read_byte(font_addr + i);
          }
          col[5] = 0;
          startWrite();
          setAddrWindow(x, y, fontWidth * size_x, fontHeight);
          bool flg = col[0] & 1;
          uint32_t len = 0;
          for (uint8_t i = 0; i < fontHeight; i++) {
            for (uint8_t j = 0; j < fontWidth; j++) {
              if (flg != (bool)(col[j] & 1 << i)) {
                _color.raw = colortbl[flg];
                writeColor_impl(len);
                len = 0;
                flg = !flg;
              }
              len += size_x;
            }
          }
          _color.raw = bg;
          writeColor_impl(len);
          endWrite();
        } else {
          int32_t xpos = x;
          startWrite();
          for (uint8_t i = 0; i < fontWidth-1; i++) {
            uint32_t len = 1;
            int32_t ypos = y;
            uint8_t line = pgm_read_byte(font_addr + i);
            bool flg = (line & 0x1);
            for (uint8_t j = 1; j < fontHeight; j++) {
              if (flg != (bool)(line & 1 << j)) {
                if (flg || fillbg) {
                  _color.raw = colortbl[flg];
                  fillRect(xpos, ypos, size_x, len * size_y);
                }
                ypos += len * size_y;
                len = 0;
                flg = !flg;
              }
              len++;
            }
            if (flg || fillbg) {
              _color.raw = colortbl[flg];
              fillRect(xpos, ypos, size_x, len * size_y);
            }
            xpos += size_x;
          }
          if (fillbg) {
            _color.raw = bg;
            fillRect(xpos, y, size_x, fontHeight * size_y); 
          }
          endWrite();
        }
      }
      if (_drawCharMoveCursor) {
        _cursor_x += fontWidth * size_x;
      }
      return fontWidth * size_x;
    }

    int16_t drawCharBMP(int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y)
    { // BMP font
      uint16_t uniCode = c - 32;
      const int fontWidth = ((c < 32) || (c > 127)) ? 0 : pgm_read_byte(widtbl_f16 + uniCode);
      constexpr int fontHeight = chr_hgt_f16;

      if (_drawCharMoveCursor) {
        if (c == '\n') {
          _cursor_x  = 0;
          _cursor_y += (int16_t)size_y * fontHeight;
          return 0;
        } else
        if (_textwrapX && ((x + (int16_t)size_x * (fontWidth)) > _width)) {
          _cursor_x  = 0;
          _cursor_y += (int16_t)size_y * fontHeight;
        }
        x = _cursor_x;
        y = _cursor_y;
      }
      if ((c < 32) || (c > 127)) return 0;
      auto font_addr = (const uint8_t*)pgm_read_dword(&chrtbl_f16[uniCode]);

      if ((x < _width)
       && (x + fontWidth * size_x > 0)
       && (y < _height)
       && (y + fontHeight * size_y > 0)) {
        int32_t ypos = y;
        uint8_t w = (fontWidth + 6) >> 3;
        uint32_t colortbl[2] = {bg, color};
        bool fillbg = (bg != color);
        if (fillbg && size_y == 1 && x >= 0 && y >= 0 && y + fontHeight <= _height && x + fontWidth * size_x <= _width) {
          uint32_t len = 0;
          uint8_t line = 0;
          bool flg = false;
          startWrite();
          setAddrWindow(x, y, fontWidth * size_x, fontHeight);
          for (uint8_t i = 0; i < fontHeight; i++) {
            for (uint8_t j = 0; j < fontWidth; j++) {
              if (j & 7) {
                line <<= 1;
              } else {
                line = (j == fontWidth - 1) ? 0 : pgm_read_byte(font_addr + (j >> 3));
              }
              if (flg != (bool)(line & 0x80)) {
                _color.raw = colortbl[flg];
                writeColor_impl(len);
                flg = !flg;
                len = 0;
              }
              len += size_x;
            }
            font_addr += w;
          }
          _color.raw = colortbl[flg];
          writeColor_impl(len);
          endWrite();
        } else {
          startWrite();
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
                  _color.raw = colortbl[flg];
                  fillRect( x + (j - len) * size_x, ypos, len * size_x, size_y); 
                }
                len = 1;
                flg = !flg;
              } else {
                len++;
              }
            }
            if (flg || fillbg) {
              _color.raw = colortbl[flg];
              fillRect( x + (j - len) * size_x, ypos, len * size_x, size_y); 
            }
            ypos += size_y;
            font_addr += w;
          }
          if (fillbg) {
            _color.raw = bg;
            fillRect( x + (fontWidth - 1) * size_x, y, size_x, fontHeight * size_y);
          }
          endWrite();
        }
      }
      if (_drawCharMoveCursor) {
        _cursor_x += fontWidth * size_x;
      }

      return fontWidth * size_x;
    }

    int16_t drawCharRLE(int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y)
    { // RLE font
      uint16_t uniCode = c - 32;
      const int fontWidth = ((c < 32) || (c > 127)) ? 0 : pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[_textfont].widthtbl ) ) + uniCode );
      const int fontHeight = pgm_read_byte( &fontdata[_textfont].height );

      if (_drawCharMoveCursor) {
        if (c == '\n') {
          _cursor_x  = 0;
          _cursor_y += (int16_t)size_y * fontHeight;
          return 0;
        } else
        if (_textwrapX && ((x + (int16_t)size_x * (fontWidth)) > _width)) {
          _cursor_x  = 0;
          _cursor_y += (int16_t)size_y * fontHeight;
        }
        x = _cursor_x;
        y = _cursor_y;
      }
      if ((c < 32) || (c > 127)) return 0;
      auto font_addr = (const uint8_t*)pgm_read_dword( (const void*)(pgm_read_dword( &(fontdata[_textfont].chartbl ) ) + uniCode*sizeof(void *)) );
      if ((x < _width)
       && (x + fontWidth * size_x > 0)
       && (y < _height)
       && (y + fontHeight * size_y > 0)) {
        uint32_t colortbl[2] = {bg, color};
        bool fillbg = (bg != color);
        if (fillbg && size_y == 1 && x >= 0 && y >= 0 && y + fontHeight <= _height && x + fontWidth * size_x <= _width) {
          uint32_t line = 0;
          startWrite();
          uint32_t len = fontWidth * size_x * fontHeight;
          setAddrWindow(x, y, fontWidth * size_x, fontHeight);
          do {
            line = pgm_read_byte(font_addr++);
            bool flg = line & 0x80;
            line = ((line & 0x7F) + 1) * size_x;
            _color.raw = colortbl[flg];
            writeColor_impl(line);
          } while (len -= line);
          endWrite();
        } else {
          bool flg = false;
          uint8_t line = 0, i = 0, j = 0;
          int32_t len;
          startWrite();
          do {
            line = pgm_read_byte(font_addr++);
            flg = line & 0x80;
            line = (line & 0x7F)+1;
            do {
              len = (j + line > fontWidth) ? fontWidth - j : line;
              line -= len;
              if (fillbg || flg) {
                _color.raw = colortbl[flg];
                fillRect( x + j * size_x, y + (i * size_y), len * size_x, size_y);
              }
              j += len;
              if (j == fontWidth) {
                j = 0;
                i++;
              }
            } while (line);
          } while (i < fontHeight);
          endWrite();
        }
      }
      if (_drawCharMoveCursor) {
        _cursor_x += fontWidth * size_x;
      }

      return fontWidth * size_x;
    }

    int16_t drawCharGFXFF(int32_t x, int32_t y, uint16_t c, const uint32_t& color, const uint32_t& bg, uint8_t size_x, uint8_t size_y)
    {
      int fontHeight = pgm_read_byte(&_gfxFont->yAdvance);
      if (c == '\n') {
        _cursor_x  = 0;
        _cursor_y += (int16_t)size_y * fontHeight;
        return 0;
      }
      if (c > pgm_read_word(&_gfxFont->last )) return 0;
      if (c < pgm_read_word(&_gfxFont->first)) return 0;

      uint16_t c2 = c - pgm_read_word(&_gfxFont->first);
      GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&_gfxFont->glyph))[c2]);
  //  int fontWidth = pgm_read_byte(&glyph->width);
      int fontWidth = pgm_read_byte(&glyph->xAdvance);
      uint8_t   w     = pgm_read_byte(&glyph->width),
                h     = pgm_read_byte(&glyph->height);

      if (_drawCharMoveCursor) {

        if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
          int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
          if (_textwrapX && ((_cursor_x + (int16_t)size_x * (xo + w)) > _width)) {
            _cursor_x  = 0;
            _cursor_y += (int16_t)size_y * fontHeight;
          }
          if (_textwrapY && (_cursor_y + _glyph_ab >= (int32_t)_height)) {
            _cursor_y = 0;
          }
        }
        x = _cursor_x;
        y = _cursor_y;
      }

      if (c < 32) return 0;

      if ((x < _width)
       && (x + fontWidth * size_x > 0)
       && (y < _height)
       && (y + fontHeight * size_y > 0)) {
      //bool fillbg = (bg != color);
        _color.raw = color;
        // Filter out bad characters not present in font
        if ((c >= pgm_read_word(&_gfxFont->first)) && (c <= pgm_read_word(&_gfxFont->last )))
        {
          bool flg = false;
          c -= pgm_read_word(&_gfxFont->first);
          GFXglyph *glyph  = &(((GFXglyph *)pgm_read_dword(&_gfxFont->glyph))[c]);
          uint8_t  *bitmap = (uint8_t *)pgm_read_dword(&_gfxFont->bitmap);

          uint32_t bo = pgm_read_word(&glyph->bitmapOffset);
          uint8_t  w  = pgm_read_byte(&glyph->width),
                   h  = pgm_read_byte(&glyph->height);
                   //xa = pgm_read_byte(&glyph->xAdvance);
          int8_t  xo = pgm_read_byte(&glyph->xOffset),
                  yo = pgm_read_byte(&glyph->yOffset);
          uint8_t  xx, yy, bits=0, bit=0;

          int16_t hpc = 0; // Horizontal foreground pixel count

          startWrite();

          int16_t xo16 = xo;
          int16_t yo16 = yo;

          for (yy=0; yy<h; yy++) {
            for (xx=0; xx<w; xx++) {
              if (bit == 0) {
                bits = pgm_read_byte(&bitmap[bo++]);
                bit  = 0x80;
              }
              if(bits & bit) hpc++;
              else {
               if (hpc) {
                  fillRect(x+(xo16+xx-hpc)*size_x, y+(yo16+yy)*size_y, size_x*hpc, size_y);
                  hpc=0;
                }
              }
              bit >>= 1;
            }
          // Draw pixels for this line as we are about to increment yy
            if (hpc) {
              fillRect(x+(xo16+xx-hpc)*size_x, y+(yo16+yy)*size_y, size_x*hpc, size_y);
              hpc=0;
            }
          }

          endWrite();
        }
      }
      if (_drawCharMoveCursor) {
        _cursor_x += fontWidth * size_x;
      }
      return fontWidth * size_x;
    }

    int16_t getCursorX(void) const { return _cursor_x; }
    int16_t getCursorY(void) const { return _cursor_y; }
    void setCursor( int16_t x, int16_t y)               { _cursor_x = x; _cursor_y = y; }
    void setCursor( int16_t x, int16_t y, uint8_t font) { _cursor_x = x; _cursor_y = y; _textfont = font; }
    void setTextSize(uint8_t s) { setTextSize(s,s); }
    void setTextSize(uint8_t sx, uint8_t sy) { _textsize_x = (sx > 0) ? sx : 1; _textsize_y = (sy > 0) ? sy : 1; }

    void setTextDatum( uint8_t datum) { _textdatum = datum; }
    void setTextWrap( bool wrapX, bool wrapY = false) { _textwrapX = wrapX; _textwrapY = wrapY; }

    void setTextFont(uint8_t f) {
      _textfont = (f > 0) ? f : 1;
      _gfxFont = nullptr; 
      if (_textfont == 1) {
        fpDrawCharClassic = &LGFXBase::drawCharGLCD;
      } else if (_textfont == 2) {
        fpDrawCharClassic = &LGFXBase::drawCharBMP;
      } else{
        fpDrawCharClassic = &LGFXBase::drawCharRLE;
      }
    }

    void setFreeFont(const GFXfont *f = nullptr)
    {
  #ifdef LOAD_GFXFF
      if (f == nullptr) { setTextFont(1); return; } // Use GLCD font
      fpDrawCharClassic = &LGFXBase::drawCharGFXFF;

      _textfont = 1;
      _gfxFont = f; // (GFXfont *)f;

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
  #endif
    }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------

  protected:
    uint32_t _start_write_count = 0;
    int32_t _width = 0;
    int32_t _height = 0;
    int32_t _cursor_x = 0;
    int32_t _cursor_y = 0;
    uint32_t _textcolor = 0xFFFFFFFF;
    uint32_t _textbgcolor = 0;
    dev_color_t _color = 0xFFFFFFFF;

    int16_t _xpivot;   // x pivot point coordinate
    int16_t _ypivot;   // x pivot point coordinate
    uint16_t _decoderBuffer= 0;   // Unicode code-point buffer
    uint8_t  _decoderState = 0;   // UTF8 decoder state
    uint8_t _rotation = 0;
    uint8_t _textsize_x = 1;
    uint8_t _textsize_y = 1;
    uint8_t _textfont = 1;
    uint8_t _textdatum;
    bool _invert = false;
    bool _swapBytes = false;
    bool _textwrapX = true;
    bool _textwrapY = false;
    bool _drawCharMoveCursor = false;

    color_depth_t _readColorDepth;

    template <typename T>
    __attribute__ ((always_inline)) inline static void swap_coord(T& a, T& b) { T t = a; a = b; b = t; }
    __attribute__ ((always_inline)) inline static bool _adj(int32_t& x, int32_t& w) { if (w < 0) { x += w + 1; w = -w; } return 0==w; }

    static bool _adj_width(int32_t& x, int32_t& dx, int32_t& dw, int32_t _width)
    {
      if ((dw < 1) || (x >= _width)) return true;
      if (x < 0) { dw += x; dx = -x; x = 0; }
      if ((x + dw) > _width ) dw = _width  - x;
      return (dw < 1);
    }

  #ifdef LOAD_GFXFF
    const GFXfont  *_gfxFont;
    uint8_t _glyph_ab;   // glyph delta Y (height) above baseline
    uint8_t _glyph_bb;   // glyph delta Y (height) below baseline
  #endif


    virtual rgb565_t readPixel16_impl(int32_t x, int32_t y) { return 0; }

    struct pixelcopy_param_t {
      int32_t src_offset = 0;
    //int32_t dst_offset = 0;
      const rgb888_t* src_palette = nullptr;
    //const rgb888_t* dst_palette = nullptr;
    };

    template <class TDst, class TSrc>
    static void pixel_to_pixel_template(void*& dst, const void* &src, int32_t len, pixelcopy_param_t* p) {
      const TSrc*& s = (const TSrc*&)src;
      TDst*& d = (TDst*&)dst;
      do { *d++ = *s++; } while (--len);
    }
    template <class TDst, class TSrc>
    static void palette_to_pixel_template(void*& dst, const void* &src, int32_t len, pixelcopy_param_t* p) {
      const uint8_t*& s = (const uint8_t*&)src;
      TDst*& d = (TDst*&)dst;
      do {
        p->src_offset = (p->src_offset + (8-TSrc::bits)) & 7;
        *d++ = p->src_palette[(*s >> p->src_offset) & TSrc::mask];
        if (!p->src_offset) { s++; }
      } while (--len);
    }

    void read_rect(int32_t x, int32_t y, int32_t w, int32_t h, void* data, void(LGFXBase::*fp_read_pixels)(void*, int32_t, pixelcopy_param_t* param))
    {
      if (_adj(x,w) || _adj(y,h)) return;
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
    void push_image(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, const rgb888_t* palette, const uint8_t bits, void(LGFXBase::*fp_write_pixels)(const void*, int32_t, pixelcopy_param_t*))
    {
      int32_t dx=0, dw=w;
      if (_adj_width(x, dx, dw, _width)) return;
      int32_t dy=0, dh=h;
      if (_adj_width(y, dy, dh, _height)) return;

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

    void push_image(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, const rgb888_t* palette, uint32_t transp, const uint8_t bits, void(LGFXBase::*fp_write_pixels)(const void*, int32_t, pixelcopy_param_t*))
    {
      int32_t dx=0, dw=w;
      if (_adj_width(x, dx, dw, _width)) return;
      int32_t dy=0, dh=h;
      if (_adj_width(y, dy, dh, _height)) return;

      pixelcopy_param_t param;
      param.src_palette = palette;

      startWrite();
      bool indivisible = (w * bits) & 7;
      const int32_t len = indivisible + (w * bits >> 3);
      const uint8_t* src = (const uint8_t*)data + dy * len;
      const uint32_t color_mask = (1<<bits)-1;
      transp &= color_mask;
      do {
        int32_t i, j = 0;
        uint8_t offset = (-dx * bits) & 7;
        for (i = 0; i < dw; i++) {
          offset = (offset - bits) & 7;
          if ((((*(uint32_t*)&src[(dx+i)*bits >> 3]) >> offset) & color_mask) == transp) {
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

    template<class TDst, class TSrc>
    void read_pixels_template(void* dst, int32_t length, pixelcopy_param_t* param)
    {
      if (std::is_same<TDst, TSrc>::value) {
        read_bytes((uint8_t*)dst, length * (TDst::bits>>3));
      } else {
        read_pixels(dst, length, param, pixel_to_pixel_template<TDst, TSrc>);
      }
    }
    template<class T>
    auto get_read_pixels_fp(void) -> void(LGFXBase::*)(void*, int32_t, pixelcopy_param_t* param)
    {
      switch (_readColorDepth) {
      case rgb888_3Byte: return &LGFXBase::read_pixels_template<T, swap888_t>;
      case rgb666_3Byte: return &LGFXBase::read_pixels_template<T, swap666_t>;
      case rgb565_2Byte: return &LGFXBase::read_pixels_template<T, swap565_t>;
      case rgb332_1Byte: return &LGFXBase::read_pixels_template<T, rgb332_t >;
      case palette_8bit:
      case palette_4bit:
      case palette_2bit:
      case palette_1bit:
      default: break;
      }
      return &LGFXBase::read_pixels_template<T, T>;
    }

    template<class TDst, class TSrc>
    void write_pixels_template(const void* src, int32_t length, pixelcopy_param_t* param)
    {
      if (std::is_same<TDst, TSrc>::value) {
        write_bytes((const uint8_t*)src, length * (TSrc::bits>>3));
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
      case palette_8bit:
      case palette_4bit:
      case palette_2bit:
      case palette_1bit:
      default: break;
      }
      return &LGFXBase::write_pixels_template<T, T>;
    }

    template<class TDst, class TSrc>
    void write_palette_template(const void* src, int32_t length, pixelcopy_param_t* param)
    {
      write_pixels(src, length, param, palette_to_pixel_template<TDst, TSrc>);
    }
    template<class T>
    auto get_write_palette_fp(void) -> void(LGFXBase::*)(const void*, int32_t, pixelcopy_param_t*)
    {
      switch (getColorDepth()) {
      case rgb888_3Byte: return &LGFXBase::write_palette_template<swap888_t, T>;
      case rgb666_3Byte: return &LGFXBase::write_palette_template<swap666_t, T>;
      case rgb565_2Byte: return &LGFXBase::write_palette_template<swap565_t, T>;
      case rgb332_1Byte: return &LGFXBase::write_palette_template<rgb332_t , T>;
      case palette_8bit:
      case palette_4bit:
      case palette_2bit:
      case palette_1bit:
      default: break;
      }
      return nullptr; // &LGFXBase::write_palette_template<T, T>;
    }

    virtual void read_pixels(void* dst, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) {}
    virtual void read_bytes(uint8_t* dst, int32_t length) {}
    virtual void write_pixels(const void* src, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) {}
    virtual void write_bytes(const uint8_t* data, int32_t length) {}

    virtual void beginTransaction_impl() {}
    virtual void endTransaction_impl() {}
    virtual void setRotation_impl(uint8_t rotation) {}
    virtual void* setColorDepth_impl(color_depth_t bpp) { return nullptr; }
    virtual void invertDisplay_impl(bool invert) {}
    virtual void writeColor_impl(int32_t len) = 0;
    virtual void drawPixel_impl(int32_t x, int32_t y) = 0;
    virtual void fillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) = 0;
    virtual void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) {}
    virtual void readWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) {}
    virtual void pushDMA_impl(const uint8_t* src, int32_t len) { write_bytes(src, len); }
    virtual void endRead_impl(void) {}

    virtual void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y)
    {
      auto fp_write_pixels = get_write_pixels_fp<swap888_t>();
      startWrite();
      if (w < h) {
        swap888_t buf[h+2];
        int32_t add = (src_x < dst_x) ? -1 : 1;
        int32_t pos = (src_x < dst_x) ? w - 1 : 0;
        for (int count = 0; count < w; count++) {
          readRect(src_x + pos, src_y, 1, h, buf);
          setWindow(dst_x + pos, dst_y, dst_x + pos, dst_y + h - 1);
          (this->*fp_write_pixels)(buf, h, nullptr);
          pos += add;
        }
      } else {
        swap888_t buf[w+2];
        int32_t add = (src_y < dst_y) ? -1 : 1;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        for (int count = 0; count < h; count++) {
          readRect(src_x, src_y + pos, w, 1, buf);
          setWindow(dst_x, dst_y + pos, dst_x + w - 1, dst_y + pos);
          (this->*fp_write_pixels)(buf, w, nullptr);
          pos += add;
        }
      }
      endWrite();
    }

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
  };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#if defined (ARDUINO) && defined (_SD_H_)
  template <class Base>
  class LGFX_SD_Support : public Base {
    uint16_t read16(fs::File &f) {
      uint16_t result;
      f.read(reinterpret_cast<uint8_t*>(&result), 2);
      return result;
    }

    uint32_t read32(fs::File &f) {
      uint32_t result;
      f.read(reinterpret_cast<uint8_t*>(&result), 4);
      return result;
    }

    void skip(fs::File &f, uint16_t len) {
      f.seek(len, SeekCur);
    }

  public:
    inline void drawBmp(fs::FS &fs, const char *path, int32_t x, int32_t y) { drawBmpFile(fs, path, x, y); }
    void drawBmpFile(fs::FS &fs, const char *path, int32_t x, int32_t y) {
      if ((x >= this->_width) || (y >= this->_height)) return;

      this->endTransaction();
      // Open requested file on SD card
      File bmpFS = fs.open(path, "r");

      if (!bmpFS) {
//Serial.print("file can't open :"); Serial.println(path);
        if (this->_start_write_count) { this->beginTransaction(); }
        return;
      }
Serial.println(path);

      uint32_t startTime = millis();

      if (read16(bmpFS) == 0x4D42) {  // bmp header "BM"
        skip(bmpFS, 8);
        uint32_t seekOffset = read32(bmpFS);
        skip(bmpFS, 4);
        int32_t row, col;
        int32_t w = read32(bmpFS);
        int32_t h = read32(bmpFS);  // bcHeight Image height (pixels)
        //If the value of bcHeight is positive, the image data is from bottom to top
        //If the value of bcHeight is negative, the image data is from top to bottom.
        int32_t flow = (h > 0) ? -1 : 1;
        if (h < 0) h = -h;
        if (flow < 0) y += h - 1;

        if (read16(bmpFS) == 1) {  // bcPlanes always 1
          uint16_t bpp = read16(bmpFS); // 24 bcBitCount 24=RGB24bit
          uint32_t biComp = read32(bmpFS); // biCompression 0=BI_RGB
          if ((bpp == 24 || bpp == 16 || bpp == 32)
           && (biComp == 0 || biComp == 3)) {
            bmpFS.seek(seekOffset);
            //this->setSwapBytes(true);
            uint16_t padding = (4 - (w & 3)) & 3;
            uint8_t lineBuffer[w * (bpp >> 3) + padding];
            while (h--) {
              this->endTransaction();
              bmpFS.read(lineBuffer, sizeof(lineBuffer));
              this->beginTransaction();
              if (bpp == 24) {      this->pushImage(x, y, w, 1, reinterpret_cast<rgb888_t*>(lineBuffer));  }
              else if (bpp == 16) { this->pushImage(x, y, w, 1, reinterpret_cast<rgb565_t*>(lineBuffer));  }
              else if (bpp == 32) { this->pushImage(x, y, w, 1, reinterpret_cast<argb8888_t*>(lineBuffer)); }
              y += flow;
            }
          } else
          if ((bpp == 8 || bpp == 4 || bpp == 1) && biComp == 0) {
            skip(bmpFS, 12);
            uint32_t biClrUsed = read32(bmpFS);
            skip(bmpFS, 4);
            argb8888_t palette[1<<bpp];
            bmpFS.seek(seekOffset-(1<<bpp)*sizeof(argb8888_t));
            bmpFS.read((uint8_t*)palette, (1<<bpp)*sizeof(argb8888_t)); // load palette
            bmpFS.seek(seekOffset);
            if (bpp == 1) {
              uint8_t lineBuffer[((w+31) >> 5) << 2];
              while (h--) {
                this->endTransaction();
                bmpFS.read(lineBuffer, sizeof(lineBuffer));
                this->beginTransaction();
                uint8_t* src = lineBuffer;
                bool flg = lineBuffer[0] & 0x80;
                int32_t len = 1;
                for (int32_t i = 1; i < w; i++) {
                  if (0 == (i & 7)) src++;
                  if (flg != (bool)(*src & (0x80 >> (i&7)))) {
                    this->drawFastHLine(x + i - len, y, len, palette[flg]);
                    flg = !flg;
                    len = 0;
                  }
                  len++;
                }
                this->drawFastHLine(x + w - len, y, len, palette[flg]);
                y += flow;
              }
            } else {
              //this->setSwapBytes(true);
              uint16_t readlen = (bpp == 4)
                               ? ((w+1)>>1) + ((4 - ((w+1)>>1) & 3) & 3)
                               : (w + (4 - (w & 3) & 3));
              uint8_t lineBuffer[w * 3];
              while (h--) {
                this->endTransaction();
                bmpFS.read(lineBuffer, readlen);
                this->beginTransaction();
                if (bpp == 8) {
                  for (int16_t i = 1; i <= w; i++) {
                    reinterpret_cast<rgb888_t*>(lineBuffer)[w - i] = palette[lineBuffer[w - i]];
                  }
                } else {
                  for (int16_t i = 1; i <= w; i++) {
                    reinterpret_cast<rgb888_t*>(lineBuffer)[w - i] = palette[(lineBuffer[(w - i)>>1]>>((i&1)?0:4))&0x0F];
                  }
                }
                this->pushImage(x, y, w, 1, reinterpret_cast<rgb888_t*>(lineBuffer));
                y += flow;
              }
            }
          }
          Serial.print("Loaded in "); Serial.print(millis() - startTime);   Serial.println(" ms");
        }
        else Serial.println("BMP format not recognized.");
      }

      bmpFS.close();
      if (this->_start_write_count) { this->beginTransaction(); }
    }
  };
#endif

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

  class LovyanGFX : public  // ここで追加機能の実装を継承する
  #if defined (ARDUINO) && defined (_SD_H_)
    LGFX_SD_Support<
  #endif
    LGFXBase // これが本体
  #if defined (ARDUINO) && defined (_SD_H_)
    >
  #endif
  {
  protected:
  };
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
