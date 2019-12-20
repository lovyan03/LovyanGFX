#ifndef LOVYANGFX_HPP_
#define LOVYANGFX_HPP_

// #pragma GCC optimize ("O3")

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <cmath>
#include <cfloat>

#include "platforms/lgfx_common.hpp"

#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)
  #include "platforms/esp32_common.hpp"
  #include "platforms/esp32_spi.hpp"
  #include "platforms/esp32_sprite.hpp"
#elif defined (__AVR__)
  #include "platforms/avr_spi.hpp"
#endif

#include "platforms/panel_ILI9341.hpp"
#include "platforms/panel_ST7735.hpp"
#include "platforms/panel_ST7789.hpp"


#define LOAD_GFXFF

#include "LGFX_FontLoad.hpp"


template<typename TDevice>
class LovyanGFX
{
public:
  LovyanGFX() {
    _swapBytes = false;
    _drawCharMoveCursor = false;
  }
  virtual ~LovyanGFX() {}

  inline TDevice* getDevice(void) { return &_dev; }
  inline int16_t width(void) const { return _width; }
  inline int16_t height(void) const { return _height; }
  inline uint8_t getInvert(void) const { return _dev.getInvert(); }
  inline uint8_t getRotation(void) const { return _dev.getRotation(); }
  inline uint8_t getColorDepth(void) const { return _dev.getColorDepth(); }
  inline bool getSwapBytes(void) const { return _swapBytes; }

  inline void setSwapBytes(bool swap) { _swapBytes = swap; }

  inline static uint16_t color565(uint32_t color888) { return lgfx::color565(color888); }
  inline static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color565(r, g, b); }
  inline static uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color888(r, g, b); }

  inline uint32_t color(uint8_t r, uint8_t g, uint8_t b) { return _dev.getColorFromRGB(r, g, b); }
  inline uint32_t getColorFromRGB(uint8_t r, uint8_t g, uint8_t b) { return _dev.getColorFromRGB(r, g, b); }

  void flush() {}
  void begin() { init(); }
  void init()
  {
    _dev.init();
    startWrite();
    setRotation(getRotation());
    invertDisplay(getInvert());
    setColorDepth(getColorDepth());
    fillScreen(0);
    endWrite();
  }

  inline void startWrite(void) __attribute__ ((always_inline))
  {
    if (0 == _start_write_count++) { _dev.beginTransaction(); }
  }

  inline void endWrite(void) __attribute__ ((always_inline))
  {
    if (_start_write_count) {
      if (0 == (--_start_write_count)) _dev.endTransaction();
    }
  }

  void setRotation(uint8_t rotation)
  {
    startWrite();
    _dev.setRotation(rotation);
    endWrite();
    _width = _dev.width();
    _height = _dev.height();
  }

  void* setColorDepth(uint8_t bpp)
  {
    startWrite();
    void* res = _dev.setColorDepth(bpp);
    endWrite();
    return res;
  }

  void invertDisplay(bool i)
  {
    startWrite();
    _dev.invertDisplay(i);
    endWrite();
  }


  void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
  {
    startWrite();
    _dev.setWindow(xs, ys, xe, ye);
    endWrite();
  }

  void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h)
  {
    if ((x >= _width) || (y >= _height)) return;
    if (x < 0) { w += x; x = 0; }
    if ((x + w) > _width)  w = _width  - x;
    if (w < 1) return;
    if (y < 0) { h += y; y = 0; }
    if ((y + h) > _height) h = _height - y;
    if (h < 1) return;

    setWindow(x, y, x + w - 1, y + h - 1);
  }

  inline void writeColor(uint32_t color, uint32_t len) { if (len) {               _dev.writeColor(color, len);             } }
  inline void pushColor( uint32_t color, uint32_t len) { if (len) { startWrite(); _dev.writeColor(color, len); endWrite(); } }
  inline void pushColor( uint32_t color              ) {            startWrite(); _dev.writeColor(color);      endWrite();   }

  void pushColors(const uint8_t *data, uint32_t len) {
    startWrite();
    _dev.writeBytes(data, len);
    endWrite();
  }
/*
  inline void pushColors(const uint16_t *data, uint32_t len, bool swap = true) { pushColors((const void*)data, len, swap); }
  void pushColors(const void *src, uint32_t len, bool swap)
  {
    startWrite();
    _dev.writePixels(src, len, swap);
    endWrite();
  }
*/
  template <typename T>
  void pushColors(const T *src, uint32_t len)
  {
    startWrite();
    _dev.writePixels(src, len);
    endWrite();
  }

  uint32_t readPanelID()
  {
    startWrite();
    uint32_t res = _dev.readPanelID();
    endWrite();
    return res;
  }

  uint32_t readPanelIDSub(uint8_t cmd)
  {
    startWrite();
    uint32_t res = _dev.readPanelIDSub(cmd);
    endWrite();
    return res;
  }

  uint32_t readPixel(int32_t x, int32_t y)
  {
    startWrite();
    _dev.readWindow(x, y, x, y);
    uint32_t res = _dev.readPixel();
    _dev.endRead();
    endWrite();
    return res;
  }

  template<typename T>
  void readRect(int32_t x, int32_t y, int32_t w, int32_t h, T* buf)
  {
    if ((x >= _width) || (y >= _height)) return;
    if (x < 0) { w += x; x = 0; }
    if ((x + w) > _width)  w = _width  - x;
    if (w < 1) return;
    if (y < 0) { h += y; y = 0; }
    if ((y + h) > _height) h = _height - y;
    if (h < 1) return;

    startWrite();
    _dev.readWindow(x, y, x + w - 1, y + h - 1);
    _dev.readPixels((uint8_t*)buf, w * h, _swapBytes);
    _dev.endRead();
    endWrite();
  }

  void drawPixel(int32_t x, int32_t y, uint32_t color)
  {
    if (x < 0 || (x >= _width) || y < 0 || (y >= _height)) return;

    startWrite();
    _dev.fillWindow(x, y, x, y, color);
    endWrite();
  }

  void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color)
  {
    if ((x < 0) || (x >= _width) || (y >= _height)) return;
    if (y < 0) { h += y; y = 0; }
    if ((y + h) > _height) h = _height - y;
    if (h < 1) return;

    startWrite();
    _dev.fillWindow(x, y, x, y + h - 1, color);
    endWrite();
  }

  void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color)
  {
    if ((y < 0) || (x >= _width) || (y >= _height)) return;
    if (x < 0) { w += x; x = 0; }
    if ((x + w) > _width) w = _width - x;
    if (w < 1) return;

    startWrite();
    _dev.fillWindow(x, y, x + w - 1, y, color);
    endWrite();
  }

  void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
  {
    if ((x >= _width) || (y >= _height)) return;
    if (x < 0) { w += x; x = 0; }
    if ((x + w) > _width)  w = _width  - x;
    if (w < 1) return;
    if (y < 0) { h += y; y = 0; }
    if ((y + h) > _height) h = _height - y;
    if (h < 1) return;

    startWrite();
    _dev.fillWindow(x, y, x + w - 1, y + h - 1, color);
    endWrite();
  }

  inline void fillScreen(uint32_t color)
  {
    fillRect(0, 0, _width, _height, color);
  }


  template<typename T>
  inline void pushRect(int32_t x, int32_t y, int32_t w, int32_t h, const T* data) { pushImage(x, y, w, h, data); }

  template<typename T>
  void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const T* data)
  {
    if ((x >= _width) || (y >= _height)) return;

    int32_t dx = 0;
    int32_t dw = w;
    if (x < 0) { dw += x; dx = -x; x = 0; }
    if ((x + dw) > _width ) dw = _width  - x;
    if (dw < 1) return;

    int32_t dy = 0;
    int32_t dh = h;
    if (y < 0) { dh += y; dy = -y; y = 0; }
    if ((y + dh) > _height) dh = _height - y;
    if (dh < 1) return;

    startWrite();
    _dev.setWindow(x, y, x + dw - 1, y + dh - 1);
    data += dx + dy * w;
    while (dh--)
    {
      pushColors(data, dw);
      data += w;
    }
    endWrite();
  }

  void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* data)
  {
    if (_swapBytes) pushImage(x, y, w, h, (const lgfx::rgb565_t*)data);
    else            pushImage(x, y, w, h, (const lgfx::swap565_t*)data);
  }
/*
  void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* data)
  {
    if ((x >= _width) || (y >= _height)) return;

    int32_t dx = 0;
    int32_t dw = w;
    if (x < 0) { dw += x; dx = -x; x = 0; }
    if ((x + dw) > _width ) dw = _width  - x;
    if (dw < 1) return;

    int32_t dy = 0;
    int32_t dh = h;
    if (y < 0) { dh += y; dy = -y; y = 0; }
    if ((y + dh) > _height) dh = _height - y;
    if (dh < 1) return;

    const uint8_t colorBytes = getColorDepth() >> 3;
    auto src = (const uint8_t*)data;
    startWrite();
    _dev.setWindow(x, y, x + dw - 1, y + dh - 1);
    src += (dx + dy * w) * colorBytes;
    while (dh--)
    {
      pushColors(src, dw, _swapBytes);
      src += w * colorBytes;
    }
    endWrite();
  }
*/
  void copyRect(int32_t src_x, int32_t src_y, int32_t w, int32_t h, int32_t dst_x, int32_t dst_y)
  {
    if ((src_x >= _width) || (src_y >= _height)) return;
    if ((dst_x >= _width) || (dst_y >= _height)) return;
    if (src_x < dst_x) { if (src_x < 0) { w += src_x; dst_x -= src_x; src_x = 0; } if ((dst_x + w) > _width )  w = _width  - dst_x; }
    else               { if (dst_x < 0) { w += dst_x; src_x -= dst_x; dst_x = 0; } if ((src_x + w) > _width )  w = _width  - src_x; }
    if (src_y < dst_y) { if (src_y < 0) { h += src_y; dst_y -= src_y; src_y = 0; } if ((dst_y + h) > _height)  h = _height - dst_y; }
    else               { if (dst_y < 0) { h += dst_y; src_y -= dst_y; dst_y = 0; } if ((src_y + h) > _height)  h = _height - src_y; }
    if ((w < 1) || (h < 1)) return;

    uint16_t bytes = getColorDepth() >> 3;
    if (0 == bytes) return;
    startWrite();
    if (w < h) {
      uint8_t buf[h * bytes];
      int16_t add = (src_x < dst_x) ? -1 : 1;
      uint32_t pos = (src_x < dst_x) ? w - 1 : 0;
      for (int count = 0; count < w; count++) {
        _dev.readWindow(src_x + pos, src_y, src_x + pos, src_y + h - 1);
        _dev.readPixels(buf, h, false);
        _dev.endRead();
        _dev.setWindow(dst_x + pos, dst_y, dst_x + pos, dst_y + h - 1);
        _dev.writeBytes(buf, h * bytes);
        pos += add;
      }
    } else {
      uint8_t buf[w * bytes];
      int16_t add = (src_y < dst_y) ? -1 : 1;
      uint32_t pos = (src_y < dst_y) ? h - 1 : 0;
      for (int count = 0; count < h; count++) {
        _dev.readWindow(src_x, src_y + pos, src_x + w - 1, src_y + pos);
        _dev.readPixels(buf, w, false);
        _dev.endRead();
        _dev.setWindow(dst_x, dst_y + pos, dst_x + w - 1, dst_y + pos);
        _dev.writeBytes(buf, w * bytes);
        pos += add;
      }
    }
    endWrite();
  }

  void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
  {
    bool steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {   swap_coord(x0, y0); swap_coord(x1, y1); }
    if (x0 > x1) { swap_coord(x0, x1); swap_coord(y0, y1); }

    int32_t xs = x0;
    int32_t dx = x1 - x0;
    int32_t err = dx >> 1;
    int32_t dy = abs(y1 - y0);
    int32_t ystep = (y0 < y1) ? 1 : -1;
    int32_t dlen = 0;

    startWrite();
    if (steep) {
      for (; x0 <= x1; x0++) {
        dlen++;
        err -= dy;
        if (err < 0) {
          err += dx;
          drawFastVLine(y0, xs, dlen, color);
          dlen = 0; y0 += ystep; xs = x0 + 1;
        }
      }
      if (dlen) drawFastVLine(y0, xs, dlen, color);
    }
    else
    {
      for (; x0 <= x1; x0++) {
        dlen++;
        err -= dy;
        if (err < 0) {
          err += dx;
          drawFastHLine(xs, y0, dlen, color);
          dlen = 0; y0 += ystep; xs = x0 + 1;
        }
      }
      if (dlen) drawFastHLine(xs, y0, dlen, color);
    }
    endWrite();
  }

  void drawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
  {
    startWrite();
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
    endWrite();
  }

  void fillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
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
      drawFastHLine(a, y0, b - a + 1, color);
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
      drawFastHLine(a, y, b - a + 1, color);
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
      drawFastHLine(a, y, b - a + 1, color);
    }
    endWrite();
  }

  void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
  {
    startWrite();
    drawFastHLine(x, y        , w, color);
    drawFastHLine(x, y + (--h), w, color);
    drawFastVLine(x        , ++y, --h, color);
    drawFastVLine(x + w - 1,   y,   h, color);
    endWrite();
  }

  void drawCircleHelper( int32_t x0, int32_t y0, int32_t r, uint8_t cornername, uint32_t color)
  {
    int32_t f     = 1 - r;
    int32_t ddF_x = 1;
    int32_t ddF_y = -2 * r;
    int32_t len   = 0;
    for (int32_t i = 0; i <= r; i++) {
      len++;
      if (f >= 0) {
        if (cornername & 0x4) {
          drawFastHLine(x0 + i - len + 1, y0 + r, len, color);
          drawFastVLine(x0 + r, y0 + i - len + 1, len, color);
        }
        if (cornername & 0x2) {
          drawFastHLine(x0 + i - len + 1, y0 - r, len, color);
          drawFastVLine(x0 + r, y0 - i          , len, color);
        }
        if (cornername & 0x8) {
          drawFastHLine(x0 - i          , y0 + r, len, color);
          drawFastVLine(x0 - r, y0 + i - len + 1, len, color);
        }
        if (cornername & 0x1) {
          drawFastHLine(x0 - i          , y0 - r, len, color);
          drawFastVLine(x0 - r, y0 - i          , len, color);
        }
        len = 0;
        r--;
        ddF_y += 2;
        f     += ddF_y;
      }
      ddF_x += 2;
      f     += ddF_x;
    }
  }

  void drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
  {
    startWrite();
    drawCircleHelper(x + r    , y + r    , r, 1, color);
    w--;
    drawCircleHelper(x + w - r, y + r    , r, 2, color);
    h--;
    drawCircleHelper(x + w - r, y + h - r, r, 4, color);
    drawCircleHelper(x + r    , y + h - r, r, 8, color);
    int32_t len = (r << 1) + 1;
    r++;
    drawFastHLine(x + r, y    , w - len, color);
    drawFastHLine(x + r, y + h, w - len, color);
    drawFastVLine(x    , y + r, h - len, color);
    drawFastVLine(x + w, y + r, h - len, color);
    endWrite();
  }

  void fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
  {
    startWrite();
    fillRect(x, y + r, w, h - (r << 1), color);
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
        fillRect(x0 - r, y2 - i          , (r << 1) + delta, len, color);
        fillRect(x0 - r, y1 + i - len + 1, (r << 1) + delta, len, color);
        if (i == r) break;
        len = 0;
        ddF_y += 2;
        f     += ddF_y;
        drawFastHLine(x0 - i, y1 + r, (i << 1) + delta, color);
        drawFastHLine(x0 - i, y2 - r, (i << 1) + delta, color);
        r--;
      }
      ddF_x += 2;
      f     += ddF_x;
    }
    endWrite();
  }

  void drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color)
  {
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
    startWrite();
    drawFastHLine(x0 - i, y0 + r, len, color);
    drawFastHLine(x0 - i, y0 - r, len, color);
    drawFastVLine(x0 - r, y0 - i, len, color);
    drawFastVLine(x0 + r, y0 - i, len, color);
    len = 0;
    for (r--; i <= r; i++) {
      if (p >= 0) {
        drawFastHLine(x0 - i          , y0 + r, len, color);
        drawFastHLine(x0 - i          , y0 - r, len, color);
        drawFastHLine(x0 + i - len + 1, y0 + r, len, color);
        drawFastHLine(x0 + i - len + 1, y0 - r, len, color);
        if (i == r && len == 1) break;
        dy -= 2;
        p -= dy;
        drawFastVLine(x0 - r, y0 - i          , len, color);
        drawFastVLine(x0 + r, y0 - i          , len, color);
        drawFastVLine(x0 + r, y0 + i - len + 1, len, color);
        drawFastVLine(x0 - r, y0 + i - len + 1, len, color);
        len = 0;
        r--;
      }
      len++;
      dx+=2;
      p+=dx;
    }
    endWrite();
  }

  void fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color)
  {
    int32_t dx = 1;
    int32_t dy = r << 1;
    int32_t p  = -(r >> 1);
    int32_t len = 0;
    startWrite();
    drawFastHLine(x0 - r, y0, dy+1, color);

    for (int32_t i  = 0; i <= r; i++) {
      len++;
      if (p >= 0) {
        fillRect(x0 - r, y0 - i          , (r<<1) + 1, len, color);
        fillRect(x0 - r, y0 + i - len + 1, (r<<1) + 1, len, color);
        if (i == r) break;
        dy -= 2;
        p -= dy;
        len = 0;
        drawFastHLine(x0 - i, y0 + r, (i<<1) + 1, color);
        drawFastHLine(x0 - i, y0 - r, (i<<1) + 1, color);
        r--;
      }
      dx+=2;
      p+=dx;
    }
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

  int16_t (LovyanGFX<TDevice>::*fpDrawCharClassic)(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y) = &LovyanGFX<TDevice>::drawCharGLCD;
  inline int16_t drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size)                   { return (this->*fpDrawCharClassic)(x, y, c, color, bg, size, size); }
  inline int16_t drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y) { return (this->*fpDrawCharClassic)(x, y, c, color, bg, size_x, size_y); }
  int16_t drawCharGLCD(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y)
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
              writeColor(flg ? color : bg , len);
              len = 0;
              flg = !flg;
            }
            len += size_x;
          }
        }
        writeColor(bg, len);
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
              if (flg || fillbg) { fillRect(xpos, ypos, size_x, len * size_y, flg ? color : bg); }
              ypos += len * size_y;
              len = 0;
              flg = !flg;
            }
            len++;
          }
          if (flg || fillbg) { fillRect(xpos, ypos, size_x, len * size_y, flg ? color : bg); }
          xpos += size_x;
        }
        if (fillbg) { fillRect(xpos, y, size_x, fontHeight * size_y, bg); }
        endWrite();
      }
    }
    if (_drawCharMoveCursor) {
      _cursor_x += fontWidth * size_x;
    }
    return fontWidth * size_x;
  }

  int16_t drawCharBMP(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y)
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
              writeColor(flg ? color : bg , len);
              flg = !flg;
              len = 0;
            }
            len += size_x;
          }
          font_addr += w;
        }
        writeColor(flg ? color : bg , len);
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
              if (flg || fillbg) { fillRect( x + (j - len) * size_x, ypos, len * size_x, size_y, flg ? color : bg); }
              len = 1;
              flg = !flg;
            } else {
              len++;
            }
          }
          if (flg || fillbg) { fillRect( x + (j - len) * size_x, ypos, len * size_x, size_y, flg ? color : bg); }
          ypos += size_y;
          font_addr += w;
        }
        if (fillbg) { fillRect( x + (fontWidth - 1) * size_x, y, size_x, fontHeight * size_y, bg); }
        endWrite();
      }
    }
    if (_drawCharMoveCursor) {
      _cursor_x += fontWidth * size_x;
    }

    return fontWidth * size_x;
  }

  int16_t drawCharRLE(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y)
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
          _dev.writeColor(flg ? color : bg, line);
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
            if (fillbg || flg) { fillRect( x + j * size_x, y + (i * size_y), len * size_x, size_y, flg ? color : bg); }
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

  int16_t drawCharGFXFF(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y)
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
      bool fillbg = (bg != color);
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
                fillRect(x+(xo16+xx-hpc)*size_x, y+(yo16+yy)*size_y, size_x*hpc, size_y, color);
                hpc=0;
              }
            }
            bit >>= 1;
          }
        // Draw pixels for this line as we are about to increment yy
          if (hpc) {
            fillRect(x+(xo16+xx-hpc)*size_x, y+(yo16+yy)*size_y, size_x*hpc, size_y, color);
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

  void setTextColor( uint16_t c)             { _textcolor =    _textbgcolor = c; }
  void setTextColor( uint16_t c, uint16_t b) { _textcolor = c; _textbgcolor = b; }
  void setTextDatum( uint8_t datum) { _textdatum = datum; }
  void setTextWrap( bool wrapX, bool wrapY = false) { _textwrapX = wrapX; _textwrapY = wrapY; }

  void setTextFont(uint8_t f) {
    _textfont = (f > 0) ? f : 1;
    _gfxFont = nullptr; 
    if (_textfont == 1) {
      fpDrawCharClassic = &LovyanGFX<TDevice>::drawCharGLCD;
    } else if (_textfont == 2) {
      fpDrawCharClassic = &LovyanGFX<TDevice>::drawCharBMP;
    } else{
      fpDrawCharClassic = &LovyanGFX<TDevice>::drawCharRLE;
    }
  }

  void setFreeFont(const GFXfont *f = nullptr)
  {
#ifdef LOAD_GFXFF
    if (f == nullptr) { setTextFont(1); return; } // Use GLCD font
    fpDrawCharClassic = &LovyanGFX<TDevice>::drawCharGFXFF;

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
  TDevice _dev;
  uint32_t _start_write_count = 0;
  int32_t _width = 0;
  int32_t _height = 0;
  bool _swapBytes = false;

  uint8_t  _textfont = 1;
  uint8_t  _textsize_x = 1;
  uint8_t  _textsize_y = 1;
  uint8_t  _textdatum;
  int32_t  _cursor_x = 0;
  int32_t  _cursor_y = 0;
  uint32_t _textcolor = 0xFFFFFF;
  uint32_t _textbgcolor = 0;
  bool     _textwrapX = true;
  bool     _textwrapY;
  bool     _drawCharMoveCursor;

  uint8_t  _decoderState = 0;   // UTF8 decoder state
  uint16_t _decoderBuffer= 0;   // Unicode code-point buffer

#ifdef LOAD_GFXFF
  const GFXfont  *_gfxFont;
  uint8_t _glyph_ab;   // glyph delta Y (height) above baseline
  uint8_t _glyph_bb;   // glyph delta Y (height) below baseline
#endif

  template <typename T>
  inline static void swap_coord(T& a, T& b) { T t = a; a = b; b = t; }


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

#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)
  template <typename CFG>
  class LGFX : public // Ç±Ç±Ç≈í«â¡ã@î\ÇÃé¿ëïÇåpè≥Ç∑ÇÈ
  #ifdef LGFX_JPGSUPPORT_HPP_
    LGFX_JpgSupport<
  #endif
  #ifdef LGFX_BMPSUPPORT_HPP_
    LGFX_BmpSupport<
  #endif
    LovyanGFX<lgfx::Esp32Spi<CFG> > // Ç±ÇÍÇ™ñ{ëÃ
  #ifdef LGFX_BMPSUPPORT_HPP_
    >
  #endif
  #ifdef LGFX_JPGSUPPORT_HPP_
    >
  #endif
  {};



  class LGFXSprite : public
  #ifdef LGFX_JPGSUPPORT_HPP_
    LGFX_JpgSupport<
  #endif
  #ifdef LGFX_BMPSUPPORT_HPP_
    LGFX_BmpSupport<
  #endif
    LovyanGFX<lgfx::Esp32Sprite>
  #ifdef LGFX_BMPSUPPORT_HPP_
    >
  #endif
  #ifdef LGFX_JPGSUPPORT_HPP_
    >
  #endif
  {
  public:
    void* createSprite(uint16_t w, uint16_t h) {
      void* res = _dev.createSprite(w, h);
      _width = _dev.width();
      _height = _dev.height();
      fillRect(0, 0, w, h, 0);
      return res;
    }
  };
#endif


#endif
