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
    _width  = width();
    _height = height();
    _swapBytes = false;
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

  inline static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return lgfx::getColor565(r, g, b); }
  inline static uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return lgfx::getColor888(r, g, b); }

  inline uint32_t getColorFromRGB(uint8_t r, uint8_t g, uint8_t b) { return _dev.getColorFromRGB(r, g, b); }

  void begin(uint8_t tc) { init(); }
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

  inline void startWrite(void)
  {
    if (0 == _start_write_count++) _dev.startWrite();
  }

  inline void startWriteFill(void)
  {
    if (0 == _start_write_count++) _dev.startWrite();
    if (0 == _start_fill_count++) _dev.startFill();
  }

  inline void endWrite(void)
  {
    if (_start_write_count) {
      if (0 == (--_start_write_count)) _dev.endWrite();
    }
  }

  inline void endWriteFill(void)
  {
    if (_start_write_count) {
      if (0 == (--_start_write_count)) {
        _dev.endWrite();
        _start_fill_count = 0;
      }
    }
    if (_start_fill_count) {
      if (0 == (--_start_fill_count)) _dev.endFill();
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
    _dev.writeMode();
    endWrite();
  }

  void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h)
  {
    if ((x >= _width) || (y >= _height)) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if ((x + w) > _width)  w = _width  - x;
    if ((y + h) > _height) h = _height - y;
    if ((w < 1) || (h < 1)) return;

    setWindow(x, y, x + w - 1, y + h - 1);
  }

  void writeColor(uint32_t color, uint32_t len) {               _dev.writeColor(color, len);             }
  void pushColor( uint32_t color, uint32_t len) { startWrite(); _dev.writeColor(color, len); endWrite(); }
  void pushColor( uint32_t color              ) { startWrite(); _dev.writeColor(color);      endWrite(); }

  void pushColors(const uint8_t *data, uint32_t len) {
    startWrite();
    _dev.writeBytes(data, len);
    endWrite();
  }

  inline void pushColors(const uint16_t *data, uint32_t len, bool swap = true) { pushColors((const void*)data, len, swap); }
  void pushColors(const void *src, uint32_t len, bool swap)
  {
    startWrite();
    _dev.writePixels(src, len, swap);
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
    _dev.setWindow(x, y, x, y);
    _dev.startRead();
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
    if (y < 0) { h += y; y = 0; }
    if ((x + w) > _width)  w = _width  - x;
    if ((y + h) > _height) h = _height - y;
    if ((w < 1) || (h < 1)) return;

    startWrite();
    _dev.setWindow(x, y, x + w - 1, y + h - 1);
    _dev.startRead();
    _dev.readPixels(w * h, (uint8_t*)buf, _swapBytes);
    _dev.endRead();
    endWrite();
  }

  void drawPixel(int32_t x, int32_t y, uint32_t color)
  {
    if (x < 0 || y < 0 || (x >= _width) || (y >= _height)) return;

    startWriteFill();
    _dev.setWindow(x, y, x, y);
    _dev.writeMode();
    _dev.writeColor(color);
    endWriteFill();
  }

  void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color)
  {
    if ((x < 0) || (x >= _width) || (y >= _height)) return;
    if (y < 0) { h += y; y = 0; }
    if ((y + h) > _height) h = _height - y;
    if (h < 1) return;

    startWriteFill();
    _dev.setWindowFill(x, y, x, y + h - 1, color);
    endWriteFill();
  }

  void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color)
  {
    if ((y < 0) || (x >= _width) || (y >= _height)) return;
    if (x < 0) { w += x; x = 0; }
    if ((x + w) > _width) w = _width - x;
    if (w < 1) return;

    startWriteFill();
    _dev.setWindowFill(x, y, x + w - 1, y, color);
    endWriteFill();
  }

  void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
  {
    if ((x >= _width) || (y >= _height)) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if ((x + w) > _width)  w = _width  - x;
    if ((y + h) > _height) h = _height - y;
    if ((w < 1) || (h < 1)) return;

    startWriteFill();
    _dev.setWindowFill(x, y, x + w - 1, y + h - 1, color);
    endWriteFill();
  }

  inline void fillScreen(uint32_t color)
  {
    fillRect(0, 0, _width, _height, color);
  }


  template<typename T>
  inline uint32_t pushRect(int32_t x, int32_t y, int32_t w, int32_t h, const T* buf) { return pushImage(x, y, w, h, buf); }

  template<typename T>
  uint32_t pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const T* buf)
  {
    if ((x >= _width) || (y >= _height)) return 0;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if ((x + w) > _width)  w = _width  - x;
    if ((y + h) > _height) h = _height - y;
    if ((w < 1) || (h < 1)) return 0;

    startWrite();
    _dev.setWindow(x, y, x + w - 1, y + h - 1);
    _dev.writeMode();
    pushColors((const void*)buf, w * h, _swapBytes);
    endWrite();
    return w * h;
  }

  void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
  {
    bool steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {   swap_coord(x0, y0); swap_coord(x1, y1); }
    if (x0 > x1) { swap_coord(x0, x1); swap_coord(y0, y1); }

    int32_t dx = x1 - x0, dy = abs(y1 - y0);;

    int32_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

    if (y0 < y1) ystep = 1;

    startWriteFill();
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
    endWriteFill();
  }

  void drawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
  {
    startWriteFill();
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
    endWriteFill();
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

    startWriteFill();
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
    endWriteFill();
  }

  void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
  {
    startWriteFill();
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y + 1, h-2, color);
    drawFastVLine(x + w - 1, y+1, h-2, color);
    endWriteFill();
  }

  void drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
  {
    startWriteFill();
    // smarter version
    drawFastHLine(x + r  , y    , w - r - r, color); // Top
    drawFastHLine(x + r  , y + h - 1, w - r - r, color); // Bottom
    drawFastVLine(x    , y + r  , h - r - r, color); // Left
    drawFastVLine(x + w - 1, y + r  , h - r - r, color); // Right
    // draw four corners
    drawCircleHelper(x + r    , y + r    , r, 1, color);
    drawCircleHelper(x + w - r - 1, y + r    , r, 2, color);
    drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    drawCircleHelper(x + r    , y + h - r - 1, r, 8, color);
    endWriteFill();
  }


  void fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
  {
    startWriteFill();
    fillRect(x, y + r, w, h - r - r, color);

    fillCircleHelper(x + r, y + h - r - 1, r, 1, w - r - r - 1, color);
    fillCircleHelper(x + r, y + r        , r, 2, w - r - r - 1, color);
    endWriteFill();
  }


  void drawCircleHelper( int32_t x0, int32_t y0, int32_t r, uint8_t cornername, uint32_t color)
  {
    int32_t f     = 1 - r;
    int32_t ddF_x = 1;
    int32_t ddF_y = -2 * r;
    int32_t x     = 0;

    while (x < r) {
      if (f >= 0) {
        r--;
        ddF_y += 2;
        f     += ddF_y;
      }
      x++;
      ddF_x += 2;
      f     += ddF_x;
      if (cornername & 0x4) {
        drawPixel(x0 + x, y0 + r, color);
        drawPixel(x0 + r, y0 + x, color);
      }
      if (cornername & 0x2) {
        drawPixel(x0 + x, y0 - r, color);
        drawPixel(x0 + r, y0 - x, color);
      }
      if (cornername & 0x8) {
        drawPixel(x0 - r, y0 + x, color);
        drawPixel(x0 - x, y0 + r, color);
      }
      if (cornername & 0x1) {
        drawPixel(x0 - r, y0 - x, color);
        drawPixel(x0 - x, y0 - r, color);
      }
    }
  }


  void drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color)
  {
    int32_t x  = 0;
    int32_t dx = 1;
    int32_t dy = r << 1;
    int32_t p  = -(r >> 1);

    startWriteFill();

    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0, y0 + r, color);
    int32_t len = 0;
    while (x < r) {
      len++;
      if (p >= 0) {
        drawFastHLine(x0 + x - len + 1, y0 + r, len, color);
        drawFastHLine(x0 - x          , y0 + r, len, color);
        drawFastHLine(x0 - x          , y0 - r, len, color);
        drawFastHLine(x0 + x - len + 1, y0 - r, len, color);
        drawFastVLine(x0 + r, y0 + x - len + 1, len, color);
        drawFastVLine(x0 - r, y0 + x - len + 1, len, color);
        drawFastVLine(x0 - r, y0 - x          , len, color);
        drawFastVLine(x0 + r, y0 - x          , len, color);
        len = 0;
        dy -= 2;
        p -= dy;
        r--;
      }
      dx+=2;
      p+=dx;
      x++;
    }
    drawPixel(x0 + x, y0 + r, color);
    drawPixel(x0 - x, y0 + r, color);
    drawPixel(x0 - x, y0 - r, color);
    drawPixel(x0 + x, y0 - r, color);
    drawPixel(x0 + r, y0 + x, color);
    drawPixel(x0 - r, y0 + x, color);
    drawPixel(x0 - r, y0 - x, color);
    drawPixel(x0 + r, y0 - x, color);

    endWriteFill();
  }

  void fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color)
  {
    int32_t x  = 0;
    int32_t dx = 1;
    int32_t dy = r << 1;
    int32_t p  = -(r>>1);

    startWriteFill();

    drawFastHLine(x0 - r, y0, dy+1, color);
    while(x < r) {
      if(p>=0) {
        drawFastHLine(x0 - x, y0 + r, 2 * x+1, color);
        drawFastHLine(x0 - x, y0 - r, 2 * x+1, color);
        dy-=2;
        p-=dy;
        r--;
      }

      dx+=2;
      p+=dx;

      x++;
      drawFastHLine(x0 - r, y0 + x, 2 * r+1, color);
      drawFastHLine(x0 - r, y0 - x, 2 * r+1, color);
    }

    endWriteFill();
  }

  void fillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint32_t color)
  {
    int32_t f     = 1 - r;
    int32_t ddF_x = 1;
    int32_t ddF_y = -r - r;
    int32_t y     = 0;

    delta++;
    while (y < r) {
      if (f >= 0) {
        if (cornername & 0x1) { drawFastHLine(x0 - y, y0 + r, y + y + delta, color); }
        if (cornername & 0x2) { drawFastHLine(x0 - y, y0 - r, y + y + delta, color); }

        r--;
        ddF_y += 2;
        f     += ddF_y;
      }
      y++;
      //x++;
      ddF_x += 2;
      f     += ddF_x;

      if (cornername & 0x1) { drawFastHLine(x0 - r, y0 + y, r + r + delta, color); }
      if (cornername & 0x2) { drawFastHLine(x0 - r, y0 - y, r + r + delta, color); }
    }
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
      if (temp == NULL) {
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
  size_t write(const uint8_t *buf, size_t size) { size_t n = 0; while (size--) { n += write(*buf++); } return n; }
  size_t write(uint8_t utf8)
  {
    if (utf8 == '\r') return 1;

    uint16_t uniCode = utf8;
    uniCode = decodeUTF8(utf8);
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

  int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y) { return drawChar(uniCode, x, y, _textfont); }
  int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font)
  {
    if (!uniCode) return 0;

    if (font == 1)
    {
      return drawChar(x, y, uniCode, _textcolor, _textbgcolor, _textsize_x, _textsize_y);
    }
    return 0;
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
        _cursor_y += (int16_t)size_y * fontHeight;
        return 0;
      }
      x = _cursor_x;
      y = _cursor_y;
    }
    if (c < 32) return 0;

    bool nodraw = ((x >= _width)  || // Clip right
                   (y >= _height) || // Clip bottom
                   ((x + fontWidth  * size_x - 1) < 0) || // Clip left
                   ((y + fontHeight * size_y - 1) < 0));  // Clip top
    if (!nodraw) {
      bool fillbg = (bg != color);
      bool flg;
      uint8_t line, i, j;
      int32_t len = 1;
      startWriteFill();
      for (i = 0; i < 6; i++ ) {
        line = (i == 5) ? 0 : pgm_read_byte(font + (c * 5) + i);
        flg = (line & 0x1);
        for (j = 1; j <= 8; j++) {
          line >>= 1;
          if (j < 8 && flg == (bool)(line & 0x1)) { len++; }
          else {
            if (fillbg || flg) { fillRect(x + (i * size_x), y + (j - len) * size_y, size_x, len * size_y, flg ? color : bg); }
            flg = !flg;
            len = 1;
          }
        }
      }
      endWriteFill();
    }
    if (_drawCharMoveCursor) {
      _cursor_x += fontWidth * size_x;
    }
//*/
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

    bool nodraw = ((x >= _width)  || // Clip right
                   (y >= _height) || // Clip bottom
                   ((x + fontWidth  * size_x - 1) < 0) || // Clip left
                   ((y + fontHeight * size_y - 1) < 0));  // Clip top

    if (c < 32) return 0;
    bool fillbg = (bg != color);
    if (!nodraw) {
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

        startWriteFill();

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

        endWriteFill();
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
    _gfxFont = NULL; 
    fpDrawCharClassic = &LovyanGFX<TDevice>::drawCharGLCD;
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
  uint32_t _start_fill_count = 0;
  int32_t _width, _height;
  bool _swapBytes;

  uint8_t  _textfont = 1;
  uint8_t  _textsize_x = 1;
  uint8_t  _textsize_y = 1;
  uint8_t  _textdatum;
  int32_t  _cursor_x = 0;
  int32_t  _cursor_y = 0;
  uint32_t _textcolor = 0xFFFF;
  uint32_t _textbgcolor = 0;
  bool     _textwrapX;
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
    if (digits > 0) {
      n += print(".");
    }

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

/*
// 'Classic' built-in font AND _gfxFont
  int16_t drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size)
  {
    int fontWidth  = 6;
    int fontHeight = 8;

    if (_drawCharMoveCursor) {
      if (c == '\n') {
        _cursor_x  = 0;
        _cursor_y += (int16_t)size * fontHeight;
        return 0;
      }

#ifdef LOAD_GFXFF
      if (_gfxFont) {
        if (c > pgm_read_word(&_gfxFont->last )) return 0;
        if (c < pgm_read_word(&_gfxFont->first)) return 0;
        fontHeight = pgm_read_byte(&_gfxFont->yAdvance);

        uint16_t c2 = c - pgm_read_word(&_gfxFont->first);

        GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&_gfxFont->glyph))[c2]);
  //      fontWidth = pgm_read_byte(&glyph->width);
        fontWidth = pgm_read_byte(&glyph->xAdvance);
        uint8_t   w     = pgm_read_byte(&glyph->width),
                  h     = pgm_read_byte(&glyph->height);
        if((w > 0) && (h > 0)) { // Is there an associated bitmap?
          int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
          if (_textwrapX && ((_cursor_x + (int16_t)size * (xo + w)) > _width)) {
            _cursor_x  = 0;
            _cursor_y += (int16_t)size * fontHeight;
          }
          if (_textwrapY && (_cursor_y + _glyph_ab >= (int32_t)_height)) {
            _cursor_y = 0;
          }
        }
      }
#endif

      x = _cursor_x;
      y = _cursor_y;
    }

    bool nodraw = ((x >= _width)  || // Clip right
                   (y >= _height) || // Clip bottom
                   ((x + fontWidth  * size - 1) < 0) || // Clip left
                   ((y + fontHeight * size - 1) < 0));  // Clip top

    if (c < 32) return 0;
    bool fillbg = (bg != color);
    if (!nodraw) {
#ifdef LOAD_GFXFF
      if (!_gfxFont)  // 'Classic' built-in font
#endif
      {
        bool flg;
        uint8_t line, i, j;
        int32_t len = 1;
        startWriteFill();
        for (i = 0; i < 6; i++ ) {
          line = (i == 5) ? 0 : pgm_read_byte(font + (c * 5) + i);
          flg = (line & 0x1);
          for (j = 1; j <= 8; j++) {
            line >>= 1;
            if (j < 8 && flg == (bool)(line & 0x1)) { len++; }
            else {
              if (fillbg || flg) { fillRect(x + (i * size), y + (j - len) * size, size, len * size, flg ? color : bg); }
              flg = !flg;
              len = 1;
            }
          }
        }
        endWriteFill();
      }
      else  // use _gfxFont
      {

#ifdef LOAD_GFXFF
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

          startWriteFill();

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
                  if(size == 1) drawFastHLine(x+xo+xx-hpc, y+yo+yy, hpc, color);
                  else fillRect(x+(xo16+xx-hpc)*size, y+(yo16+yy)*size, size*hpc, size, color);
                  hpc=0;
                }
              }
              bit >>= 1;
            }
          // Draw pixels for this line as we are about to increment yy
            if (hpc) {
              if(size == 1) drawFastHLine(x+xo+xx-hpc, y+yo+yy, hpc, color);
              else fillRect(x+(xo16+xx-hpc)*size, y+(yo16+yy)*size, size*hpc, size, color);
              hpc=0;
            }
          }

          endWriteFill();
        }
#endif

      }
    }
    if (_drawCharMoveCursor) {
      _cursor_x += fontWidth * size;
    }
    return fontWidth * size;
  }
//*/
/*
  size_t write(uint8_t utf8)
  {
    if (utf8 == '\r') return 1;

    uint16_t uniCode = utf8;
    uniCode = decodeUTF8(utf8);
    if (uniCode == 0) return 1;

    _drawCharMoveCursor = true;
    drawChar(uniCode, _cursor_x, _cursor_y, _textfont);
    _drawCharMoveCursor = false;

    int32_t width = 0;
    int32_t height = 0;
    if (!_gfxFont) {
      if (_textfont == 1)
      {
        width =  6;
        height = 8;
      } else
      if (_textfont == 2)
      {
        if (uniCode > 127) return 1;

        width = pgm_read_byte(widtbl_f16 + uniCode-32);
        height = chr_hgt_f16;
        // Font 2 is rendered in whole byte widths so we must allow for this
        width = (width + 6) / 8;  // Width in whole bytes for font 2, should be + 7 but must allow for font width change
        width = width * 8;        // Width converted back to pixels
      } else
      if ((_textfont > 2) && (_textfont < 9))
      {
        if (uniCode > 127) return 1;
        // Uses the fontinfo struct array to avoid lots of 'if' or 'switch' statements
        width = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[_textfont].widthtbl ) ) + uniCode-32 );
        height= pgm_read_byte( &fontdata[_textfont].height );
      }

      height = height * _textsize;

      if (utf8 == '\n') {
        _cursor_y += height;
        _cursor_x  = 0;
      }
      else
      {
        if (_textwrapX && (_cursor_x + width * _textsize > _width))
        {
          _cursor_y += height;
          _cursor_x = 0;
        }
        if (_textwrapY && (_cursor_y + height >= (int32_t)_height)) _cursor_y = 0;
        _cursor_x += drawChar(uniCode, _cursor_x, _cursor_y, _textfont);
      }
    }
    return 1;
  }
*/



#endif
