/*----------------------------------------------------------------------------/
  Lovyan GFX library - LCD graphics library .
  
  support platform:
    ESP32 (SPI/I2S) with Arduino/ESP-IDF
    ATSAMD51 (SPI) with Arduino
  
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
#include "lgfx_common.hpp"
#if defined (LGFX_ENABLE_V0)

#include "LGFXBase.hpp"

#include "../utility/lgfx_tjpgd.h"    // JPEG decode support
#include "../utility/lgfx_pngle.h"    // PNG decode support
#include "../utility/lgfx_qrcode.h"   // QR code support
#include "../utility/miniz.h"

#include <algorithm>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <list>
#include <string>

#if __has_include(<alloca.h>)
#include <alloca.h>
#else
#include <malloc.h>
#define alloca _alloca
#endif

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------
  static constexpr float deg_to_rad = 0.017453292519943295769236907684886;

  void LGFXBase::setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h)
  {
    if (_adjust_abs(x, w)||_adjust_abs(y, h)) return;
    if (x < 0) { w += x; x = 0; }
    if (w > _width - x)  w = _width  - x;
    if (w < 1) { x = 0; w = 0; }
    if (y < 0) { h += y; y = 0; }
    if (h > _height - y) h = _height - y;
    if (h < 1) { y = 0; h = 0; }

    startWrite();
    setWindow(x, y, x + w - 1, y + h - 1);
    endWrite();
  }

  void LGFXBase::setClipRect(int32_t x, int32_t y, int32_t w, int32_t h)
  {
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

  void LGFXBase::getClipRect(int32_t *x, int32_t *y, int32_t *w, int32_t *h)
  {
    *x = _clip_l;
    *w = _clip_r - *x + 1;
    *y = _clip_t;
    *h = _clip_b - *y + 1;
  }

  void LGFXBase::clearClipRect(void)
  {
    _clip_l = 0;
    _clip_r = _width - 1;
    _clip_t = 0;
    _clip_b = _height - 1;
  }

  void LGFXBase::setScrollRect(int32_t x, int32_t y, int32_t w, int32_t h)
  {
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

  void LGFXBase::getScrollRect(int32_t *x, int32_t *y, int32_t *w, int32_t *h)
  {
    *x = _sx;
    *y = _sy;
    *w = _sw;
    *h = _sh;
  }

  void LGFXBase::clearScrollRect(void)
  {
    _sx = 0;
    _sw = _width;
    _sy = 0;
    _sh = _height;
  }

  void LGFXBase::drawFastVLine(int32_t x, int32_t y, int32_t h)
  {
    _adjust_abs(y, h);
    startWrite();
    writeFastVLine(x, y, h);
    endWrite();
  }

  void LGFXBase::writeFastVLine(int32_t x, int32_t y, int32_t h)
  {
    if (x < _clip_l || x > _clip_r) return;
    auto ct = _clip_t;
    if (y < ct) { h += y - ct; y = ct; }
    auto cb = _clip_b + 1 - y;
    if (h > cb) h = cb;
    if (h < 1) return;

    writeFillRect_impl(x, y, 1, h);
  }

  void LGFXBase::drawFastHLine(int32_t x, int32_t y, int32_t w)
  {
    _adjust_abs(x, w);
    startWrite();
    writeFastHLine(x, y, w);
    endWrite();
  }

  void LGFXBase::writeFastHLine(int32_t x, int32_t y, int32_t w)
  {
    if (y < _clip_t || y > _clip_b) return;
    auto cl = _clip_l;
    if (x < cl) { w += x - cl; x = cl; }
    auto cr = _clip_r + 1 - x;
    if (w > cr) w = cr;
    if (w < 1) return;

    writeFillRect_impl(x, y, w, 1);
  }

  void LGFXBase::fillRect(int32_t x, int32_t y, int32_t w, int32_t h)
  {
    _adjust_abs(x, w);
    _adjust_abs(y, h);
    startWrite();
    writeFillRect(x, y, w, h);
    endWrite();
  }

  void LGFXBase::writeFillRect(int32_t x, int32_t y, int32_t w, int32_t h)
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


  void LGFXBase::drawRect(int32_t x, int32_t y, int32_t w, int32_t h)
  {
    if (_adjust_abs(x, w)||_adjust_abs(y, h)) return;
    startWrite();
    writeFastHLine(x, y        , w);
    if (--h) {
      writeFastHLine(x, y + h    , w);
      if (--h) {
        writeFastVLine(x        , ++y, h);
        writeFastVLine(x + w - 1,   y, h);
      }
    }
    endWrite();
  }

  void LGFXBase::drawCircle(int32_t x, int32_t y, int32_t r)
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

      writeFastVLine(x + r, y + j + 1, i - j);
      writeFastVLine(x + r, y - i    , i - j);
      writeFastVLine(x - r, y - i    , i - j);
      writeFastVLine(x - r, y + j + 1, i - j);
      j = i;
    } while (i < --r);
    endWrite();
  }

  void LGFXBase::drawCircleHelper(int32_t x, int32_t y, int32_t r, uint_fast8_t cornername)
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

  void LGFXBase::fillCircle(int32_t x, int32_t y, int32_t r) {
    startWrite();
    writeFastHLine(x - r, y, (r << 1) + 1);
    fillCircleHelper(x, y, r, 3, 0);
    endWrite();
  }

  void LGFXBase::fillCircleHelper(int32_t x, int32_t y, int32_t r, uint_fast8_t corners, int32_t delta)
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

  void LGFXBase::drawEllipse(int32_t x, int32_t y, int32_t rx, int32_t ry)
  {
    if (ry == 0) {
      drawFastHLine(x - rx, y, (ry << 2) + 1);
      return;
    }
    if (rx == 0) {
      drawFastVLine(x, y - ry, (rx << 2) + 1);
      return;
    }
    if (rx < 0 || ry < 0) return;

    int32_t xt, yt, s, i;
    int32_t rx2 = rx * rx;
    int32_t ry2 = ry * ry;

    startWrite();

    i = -1;
    xt = 0;
    yt = ry;
    s = (ry2 << 1) + rx2 * (1 - (ry << 1));
    do {
      while ( s < 0 ) s += ry2 * ((++xt << 2) + 2);
      writeFastHLine(x - xt   , y - yt, xt - i);
      writeFastHLine(x + i + 1, y - yt, xt - i);
      writeFastHLine(x + i + 1, y + yt, xt - i);
      writeFastHLine(x - xt   , y + yt, xt - i);
      i = xt;
      s -= (--yt) * rx2 << 2;
    } while (ry2 * xt <= rx2 * yt);

    i = -1;
    yt = 0;
    xt = rx;
    s = (rx2 << 1) + ry2 * (1 - (rx << 1));
    do {
      while ( s < 0 ) s += rx2 * ((++yt << 2) + 2);
      writeFastVLine(x - xt, y - yt   , yt - i);
      writeFastVLine(x - xt, y + i + 1, yt - i);
      writeFastVLine(x + xt, y + i + 1, yt - i);
      writeFastVLine(x + xt, y - yt   , yt - i);
      i = yt;
      s -= (--xt) * ry2 << 2;
    } while (rx2 * yt <= ry2 * xt);

    endWrite();
  }

  void LGFXBase::fillEllipse(int32_t x, int32_t y, int32_t rx, int32_t ry)
  {
    if (ry == 0) {
      drawFastHLine(x - rx, y, (ry << 2) + 1);
      return;
    }
    if (rx == 0) {
      drawFastVLine(x, y - ry, (rx << 2) + 1);
      return;
    }
    if (rx < 0 || ry < 0) return;

    int32_t xt, yt, i;
    int32_t rx2 = rx * rx;
    int32_t ry2 = ry * ry;
    int32_t s;

    startWrite();

    writeFastHLine(x - rx, y, (rx << 1) + 1);
    i = 0;
    yt = 0;
    xt = rx;
    s = (rx2 << 1) + ry2 * (1 - (rx << 1));
    do {
      while (s < 0) s += rx2 * ((++yt << 2) + 2);
      writeFillRect(x - xt, y - yt   , (xt << 1) + 1, yt - i);
      writeFillRect(x - xt, y + i + 1, (xt << 1) + 1, yt - i);
      i = yt;
      s -= (--xt) * ry2 << 2;
    } while (rx2 * yt <= ry2 * xt);

    xt = 0;
    yt = ry;
    s = (ry2 << 1) + rx2 * (1 - (ry << 1));
    do {
      while (s < 0) s += ry2 * ((++xt << 2) + 2);
      writeFastHLine(x - xt, y - yt, (xt << 1) + 1);
      writeFastHLine(x - xt, y + yt, (xt << 1) + 1);
      s -= (--yt) * rx2 << 2;
    } while(ry2 * xt <= rx2 * yt);

    endWrite();
  }

  void LGFXBase::drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r)
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

  void LGFXBase::fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r)
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

  void LGFXBase::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
  {
    bool steep = abs(y1 - y0) > abs(x1 - x0);

    int32_t xstart = _clip_l;
    int32_t xend   = _clip_r;
    int32_t ystart = _clip_t;
    int32_t yend   = _clip_b;

    if (steep) {
      std::swap(xstart, ystart);
      std::swap(xend, yend);
      std::swap(x0, y0);
      std::swap(x1, y1); 
    }
    if (x0 > x1) {
      std::swap(x0, x1);
      std::swap(y0, y1);
    }
    if (x0 > xend || x1 < xstart) return;
    xend = std::min(x1, xend);

    int32_t dy = abs(y1 - y0);
    int32_t ystep = (y1 > y0) ? 1 : -1;
    int32_t dx = x1 - x0;
    int32_t err = dx >> 1;

    while (x0 < xstart || y0 < ystart || y0 > yend) {
      if (++x0 > xend) return;
      err -= dy;
      if (err < 0) {
        err += dx;
        y0 += ystep;
      }
    }
    int32_t xs = x0;
    int32_t dlen = 0;
    if (ystep < 0) std::swap(ystart, yend);
    yend += ystep;

    startWrite();
    if (steep) {
      do {
        ++dlen;
        if ((err -= dy) < 0) {
          writeFillRectPreclipped(y0, xs, 1, dlen);
          err += dx;
          xs = x0 + 1; dlen = 0; y0 += ystep;
          if (y0 == yend) break;
        }
      } while (++x0 <= xend);
      if (dlen) writeFillRectPreclipped(y0, xs, 1, dlen);
    } else {
      do {
        ++dlen;
        if ((err -= dy) < 0) {
          writeFillRectPreclipped(xs, y0, dlen, 1);
          err += dx;
          xs = x0 + 1; dlen = 0; y0 += ystep;
          if (y0 == yend) break;
        }
      } while (++x0 <= xend);
      if (dlen) writeFillRectPreclipped(xs, y0, dlen, 1);
    }
    endWrite();
  }

  void LGFXBase::drawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
  {
    startWrite();
    drawLine(x0, y0, x1, y1);
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x0, y0);
    endWrite();
  }

  void LGFXBase::fillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
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

  void LGFXBase::drawBezier( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
  {
    int32_t x = x0 - x1, y = y0 - y1;
    double t = x0 - 2 * x1 + x2, r;

    startWrite();

    if (x * (x2 - x1) > 0) {
      if (y * (y2 - y1) > 0)
        if (fabs((y0 - 2 * y1 + y2) / t * x) > abs(y)) {
          x0 = x2; x2 = x + x1; y0 = y2; y2 = y + y1;
        }
      t = (x0 - x1) / t;
      r = (1 - t) * ((1 - t) * y0 + 2.0 * t * y1) + t * t * y2;
      t = (x0 * x2 - x1 * x1) * t / (x0 - x1);
      x = floor(t + 0.5); y = floor(r + 0.5);
      r = (y1 - y0) * (t - x0) / (x1 - x0) + y0;
      draw_bezier_helper(x0, y0, x, floor(r + 0.5), x, y);
      r = (y1 - y2) * (t - x2) / (x1 - x2) + y2;
      x0 = x1 = x; y0 = y; y1 = floor(r + 0.5);
    }
    if ((y0 - y1) * (y2 - y1) > 0) {
      t = y0 - 2 * y1 + y2; t = (y0 - y1) / t;
      r = (1 - t) * ((1 - t) * x0 + 2.0 * t * x1) + t * t * x2;
      t = (y0 * y2 - y1 * y1) * t / (y0 - y1);
      x = floor(r + 0.5); y = floor(t + 0.5);
      r = (x1 - x0) * (t - y0) / (y1 - y0) + x0;
      draw_bezier_helper(x0, y0, floor(r + 0.5), y, x, y);
      r = (x1 - x2) * (t - y2) / (y1 - y2) + x2;
      x0 = x; x1 = floor(r + 0.5); y0 = y1 = y;
    }
    draw_bezier_helper(x0, y0, x1, y1, x2, y2);

    endWrite();
  }

  void LGFXBase::draw_bezier_helper( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
  {
    // Check if coordinates are sequential (replaces assert)
    if (((x2 >= x1 && x1 >= x0) || (x2 <= x1 && x1 <= x0))
        && ((y2 >= y1 && y1 >= y0) || (y2 <= y1 && y1 <= y0)))
    {
      // Coordinates are sequential
      int32_t sx = x2 - x1, sy = y2 - y1;
      int32_t xx = x0 - x1, yy = y0 - y1, xy;
      float dx, dy, err, cur = xx * sy - yy * sx;

      if (sx * (int32_t)sx + sy * (int32_t)sy > xx * xx + yy * yy) {
        x2 = x0; x0 = sx + x1; y2 = y0; y0 = sy + y1; cur = -cur;
      }
      if (cur != 0) {
        xx += sx; xx *= sx = x0 < x2 ? 1 : -1;
        yy += sy; yy *= sy = y0 < y2 ? 1 : -1;
        xy = 2 * xx * yy; xx *= xx; yy *= yy;
        if (cur * sx * sy < 0) {
          xx = -xx; yy = -yy; xy = -xy; cur = -cur;
        }
        dx = 4.0 * sy * cur * (x1 - x0) + xx - xy;
        dy = 4.0 * sx * cur * (y0 - y1) + yy - xy;
        xx += xx; yy += yy; err = dx + dy + xy;
        do {
          drawPixel(x0, y0);
          if (x0 == x2 && y0 == y2)
          {
            return;
          }
          y1 = 2 * err < dx;
          if (2 * err > dy) {
            x0 += sx;
            dx -= xy;
            err += dy += yy;
          }
          if (    y1    ) {
            y0 += sy;
            dy -= xy;
            err += dx += xx;
          }
        } while (dy < dx );
      }
      drawLine(x0, y0, x2, y2);
    }
  }

  void LGFXBase::drawBezier( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3)
  {
    int32_t w = x0-x1;
    int32_t h = y0-y1;
    int32_t len = w*w+h*h;
    w = x1-x2;
    h = y1-y2;
    int32_t len2 = w*w+h*h;
    if (len < len2) len = len2;
    w = x2-x3;
    h = y2-y3;
    len2 = w*w+h*h;
    if (len < len2) len = len2;
    len = (int32_t)round(sqrt(len)) >> 2;

    float fx0 = x0;
    float fy0 = y0;
    float fx1 = x1;
    float fy1 = y1;
    float fx2 = x2;
    float fy2 = y2;
    float fx3 = x3;
    float fy3 = y3;

    int32_t i = 0;
    startWrite();
//drawLine(x0, y0, x1, y1);
//drawLine(x1, y1, x2, y2);
//drawLine(x2, y2, x3, y3);
//drawCircle(x0, y0, 3);
//drawCircle(x1, y1, 3);
//drawCircle(x2, y2, 3);
//drawCircle(x3, y3, 3);
    do {
      float t = i;
      t = t / (len<<1);
      float tr = 1 - t;
      float f0 = tr * tr;
      float f1 = f0 * t * 3;
      f0 = f0 * tr;
      float f3 = t * t;
      float f2 = tr * f3 * 3;
      f3 = f3 * t;
      x1 = roundf( fx0 * f0 + fx1 * f1 + fx2 * f2 + fx3 * f3);
      y1 = roundf( fy0 * f0 + fy1 * f1 + fy2 * f2 + fy3 * f3);
      if (x0 != x1 || y0 != y1) {
        drawLine(x0, y0, x1, y1);
//drawCircle(x1, y1, 3);
        x0 = x1;
        y0 = y1;
      }
      x2 = roundf( fx0 * f3 + fx1 * f2 + fx2 * f1 + fx3 * f0);
      y2 = roundf( fy0 * f3 + fy1 * f2 + fy2 * f1 + fy3 * f0);
      if (x3 != x2 || y3 != y2) {
        drawLine(x3, y3, x2, y2);
//drawCircle(x2, y2, 3);
        x3 = x2;
        y3 = y2;
      }
    } while (++i <= len);
    endWrite();
  }

  void LGFXBase::draw_gradient_line( int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colorstart, uint32_t colorend )
  {
    if ( colorstart == colorend || (x0 == x1 && y0 == y1)) {
      setColor(colorstart);
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

    int32_t r = (colorstart >> 16)&0xFF;
    int32_t g = (colorstart >> 8 )&0xFF;
    int32_t b = (colorstart      )&0xFF;

    int32_t diff_r = ((colorend >> 16)&0xFF) - r;
    int32_t diff_g = ((colorend >> 8 )&0xFF) - g;
    int32_t diff_b = ((colorend      )&0xFF) - b;

    startWrite();
    for (int32_t x = x0; x <= x1; x++) {
      setColor(color888( (x - x0) * diff_r / dx + r
                       , (x - x0) * diff_g / dx + g
                       , (x - x0) * diff_b / dx + b));
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

  void LGFXBase::drawEllipseArc(int32_t x, int32_t y, int32_t r0x, int32_t r1x, int32_t r0y, int32_t r1y, float start, float end)
  {
    if (r0x < r1x) std::swap(r0x, r1x);
    if (r0y < r1y) std::swap(r0y, r1y);
    if (r1x < 0) return;
    if (r1y < 0) return;

    bool equal = fabsf(start - end) < std::numeric_limits<float>::epsilon();
    start = fmodf(start, 360);
    end = fmodf(end, 360);
    if (start < 0) start += 360.0;
    if (end < 0) end += 360.0;

    startWrite();
    fill_arc_helper(x, y, r0x, r1x, r0y, r1y, start, start);
    fill_arc_helper(x, y, r0x, r1x, r0y, r1y, end, end);
    if (!equal && (fabsf(start - end) <= 0.0001)) { start = .0; end = 360.0; }
    fill_arc_helper(x, y, r0x, r0x, r0y, r0y, start, end);
    fill_arc_helper(x, y, r1x, r1x, r1y, r1y, start, end);
    endWrite();
  }

  void LGFXBase::fillEllipseArc(int32_t x, int32_t y, int32_t r0x, int32_t r1x, int32_t r0y, int32_t r1y, float start, float end)
  {
    if (r0x < r1x) std::swap(r0x, r1x);
    if (r0y < r1y) std::swap(r0y, r1y);
    if (r1x < 0) return;
    if (r1y < 0) return;

    bool equal = fabsf(start - end) < std::numeric_limits<float>::epsilon();
    start = fmodf(start, 360);
    end = fmodf(end, 360);
    if (start < 0) start += 360.0;
    if (end < 0) end += 360.0;
    if (!equal && (fabsf(start - end) <= 0.0001)) { start = .0; end = 360.0; }

    startWrite();
    fill_arc_helper(x, y, r0x, r1x, r0y, r1y, start, end);
    endWrite();
  }

  void LGFXBase::fill_arc_helper(int32_t cx, int32_t cy, int32_t oradius_x, int32_t iradius_x, int32_t oradius_y, int32_t iradius_y, float start, float end)
  {
    float s_cos = (cosf(start * deg_to_rad));
    float e_cos = (cosf(end * deg_to_rad));
    float sslope = s_cos / (sinf(start * deg_to_rad));
    float eslope = -1000000;
    if (end != 360.0) eslope = e_cos / (sinf(end * deg_to_rad));
    float swidth =  0.5 / s_cos;
    float ewidth = -0.5 / e_cos;

    bool start180 = !(start < 180);
    bool end180 = end < 180;
    bool reversed = start + 180 < end || (end < start && start < end + 180);

    int32_t xleft  = -oradius_x;
    int32_t xright = oradius_x + 1;
    int32_t y = -oradius_y;
    int32_t ye = oradius_y;
    if (!reversed)
    {
      if (    (end >= 270 || end <  90) && (start >= 270 || start <  90)) xleft = 0;
      else if (end <  270 && end >= 90  &&  start <  270 && start >= 90) xright = 1;
      if (     end >= 180 && start >= 180) ye = 0;
      else if (end <  180 && start <  180) y = 0;
    }
    if (y  < _clip_t - cy    ) y  = _clip_t - cy;
    if (ye > _clip_b - cy + 1) ye = _clip_b - cy + 1;

    if (xleft  < _clip_l - cx    ) xleft  = _clip_l - cx;
    if (xright > _clip_r - cx + 1) xright = _clip_r - cx + 1;

    bool trueCircle = (oradius_x == oradius_y) && (iradius_x == iradius_y);

    int32_t iradius_y2 = iradius_y * (iradius_y - 1);
    int32_t iradius_x2 = iradius_x * (iradius_x - 1);
    float irad_rate = iradius_x2 && iradius_y2 ? (float)iradius_x2 / (float)iradius_y2 : 0;

    int32_t oradius_y2 = oradius_y * (oradius_y + 1);
    int32_t oradius_x2 = oradius_x * (oradius_x + 1);
    float orad_rate = oradius_x2 && oradius_y2 ? (float)oradius_x2 / (float)oradius_y2 : 0;

    do
    {
      int32_t y2 = y * y;
      int32_t compare_o = oradius_y2 - y2;
      int32_t compare_i = iradius_y2 - y2;
      if (!trueCircle)
      {
        compare_i = floorf(compare_i * irad_rate);
        compare_o = ceilf (compare_o * orad_rate);
      }
      int32_t xe = ceilf(sqrtf(compare_o));
      int32_t x = 1 - xe;

      if ( x < xleft )  x = xleft;
      if (xe > xright) xe = xright;
      float ysslope = (y + swidth) * sslope;
      float yeslope = (y + ewidth) * eslope;
      int len = 0;
      do
      {
        bool flg1 = start180 != (x <= ysslope);
        bool flg2 =   end180 != (x <= yeslope);
        int32_t x2 = x * x;
        if (x2 >= compare_i
         && ((flg1 && flg2) || (reversed && (flg1 || flg2)))
         && x != xe
         && x2 < compare_o)
        {
          ++len;
        }
        else
        {
          if (len)
          {
            writeFastHLine(cx + x - len, cy + y, len);
            len = 0;
          }
          if (x2 >= compare_o) break;
          if (x < 0 && x2 < compare_i) { x = -x; }
        }
      } while (++x <= xe);
    } while (++y <= ye);
  }

  void LGFXBase::draw_bitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, uint32_t fg_rawcolor, uint32_t bg_rawcolor)
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

  void LGFXBase::draw_xbitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, uint32_t fg_rawcolor, uint32_t bg_rawcolor)
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

  void LGFXBase::pushImage(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t *param, bool use_dma)
  {
    param->src_bitwidth = w;
    if (param->src_bits < 8) {        // get bitwidth
//      uint32_t x_mask = (1 << (4 - __builtin_ffs(param->src_bits))) - 1;
//      uint32_t x_mask = (1 << ((~(param->src_bits>>1)) & 3)) - 1;
      uint32_t x_mask = (param->src_bits == 1) ? 7
                           : (param->src_bits == 2) ? 3
                                                    : 1;
      param->src_bitwidth = (w + x_mask) & (~x_mask);
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

  void LGFXBase::make_rotation_matrix(float* result, float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y)
  {
    float rad = fmodf(angle, 360) * deg_to_rad;
    float sin_f = sinf(rad);
    float cos_f = cosf(rad);
    result[0] =  cos_f * zoom_x;
    result[1] = -sin_f * zoom_y;
    result[2] =  dst_x - src_x * result[0] - src_y * result[1];
    result[3] =  sin_f * zoom_x;
    result[4] =  cos_f * zoom_y;
    result[5] =  dst_y - src_x * result[3] - src_y * result[4];
  }

  static bool make_invert_affine32(int32_t* __restrict result, const float* __restrict matrix)
  {
    float det = matrix[0] * matrix[4] - matrix[1] * matrix[3];
    if (det == 0.0f) return false;
    det = (1 << FP_SCALE) / det;
    result[0] = roundf(det *  matrix[4]);
    result[1] = roundf(det * -matrix[1]);
    result[2] = roundf(det * (matrix[1] * matrix[5] - matrix[2] * matrix[4]));
    result[3] = roundf(det * -matrix[3]);
    result[4] = roundf(det *  matrix[0]);
    result[5] = roundf(det * (matrix[2] * matrix[3] - matrix[0] * matrix[5]));
    return true;
  }

  void LGFXBase::push_image_rotate_zoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, pixelcopy_t* pc)
  {
    float matrix[6];
    make_rotation_matrix(matrix, dst_x + 0.5, dst_y + 0.5, src_x + 0.5, src_y + 0.5, angle, zoom_x, zoom_y);
    push_image_affine(matrix, w, h, pc);
  }

  void LGFXBase::push_image_rotate_zoom_aa(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, pixelcopy_t* pc)
  {
    float matrix[6];
    make_rotation_matrix(matrix, dst_x + 0.5, dst_y + 0.5, src_x + 0.5, src_y + 0.5, angle, zoom_x, zoom_y);
    push_image_affine_aa(matrix, w, h, pc);
  }

  void LGFXBase::push_image_affine(const float* matrix, int32_t w, int32_t h, pixelcopy_t* pc)
  {
    pc->no_convert = false;
    pc->src_height = h;
    pc->src_width = w;
    pc->src_bitwidth = w;
    if (pc->src_bits < 8) {
      uint32_t x_mask = (pc->src_bits == 1) ? 7
                           : (pc->src_bits == 2) ? 3
                                                 : 1;
      pc->src_bitwidth = (w + x_mask) & (~x_mask);
    }
    push_image_affine(matrix, pc);
  }

  void LGFXBase::push_image_affine_aa(const float* matrix, int32_t w, int32_t h, pixelcopy_t* pc)
  {
    pc->no_convert = false;
    pc->src_height = h;
    pc->src_width = w;
    pc->src_bitwidth = w;
    if (pc->src_bits < 8) {
      uint32_t x_mask = (pc->src_bits == 1) ? 7
                           : (pc->src_bits == 2) ? 3
                                                 : 1;
      pc->src_bitwidth = (w + x_mask) & (~x_mask);
    }
    pixelcopy_t pc_post;
    auto dst_depth = getColorDepth();
    if (hasPalette() || dst_depth < 8)
    {
      pc_post.dst_bits = dst_depth;
      pc_post.dst_mask = (1 << dst_depth) - 1;
      pc_post.fp_copy = pixelcopy_t::blend_palette_fast;
    }
    else
    if (dst_depth > rgb565_2Byte) {
      if (dst_depth == rgb888_3Byte) {
        pc_post.fp_copy = pixelcopy_t::blend_rgb_fast<bgr888_t>;
      } else {
        pc_post.fp_copy = pixelcopy_t::blend_rgb_fast<bgr666_t>;
      }
    } else {
      if (dst_depth == rgb565_2Byte) {
        pc_post.fp_copy = pixelcopy_t::blend_rgb_fast<swap565_t>;
      } else { // src_depth == rgb332_1Byte:
        pc_post.fp_copy = pixelcopy_t::blend_rgb_fast<rgb332_t>;
      }
    }
    push_image_affine_aa(matrix, pc, &pc_post);
  }

  void LGFXBase::fillAffine(const float matrix[6], int32_t w, int32_t h)
  {
    int32_t min_y = matrix[3] * (w << FP_SCALE);
    int32_t max_y = matrix[4] * (h << FP_SCALE);
    if ((min_y < 0) == (max_y < 0))
    {
      max_y += min_y;
      min_y = 0;
    }
    if (min_y > max_y) 
    {
      std::swap(min_y, max_y);
    }

    {
      int32_t offset_y32 = matrix[5] * (1 << FP_SCALE) + (1 << (FP_SCALE-1));
      min_y = std::max(_clip_t    , (offset_y32 + min_y - 1) >> FP_SCALE);
      max_y = std::min(_clip_b + 1, (offset_y32 + max_y + 1) >> FP_SCALE);
      if (min_y >= max_y) return;
    }


    int32_t iA[6];
    if (!make_invert_affine32(iA, matrix)) return;

    int32_t offset = (min_y << 1) - 1;
    iA[2] += ((iA[0] + iA[1] * offset) >> 1);
    iA[5] += ((iA[3] + iA[4] * offset) >> 1);

    int32_t scale_w = w << FP_SCALE;
    int32_t xs1 = (iA[0] < 0 ?   - scale_w :   1) - iA[0];
    int32_t xs2 = (iA[0] < 0 ? 0 : (1 - scale_w)) - iA[0];

    int32_t scale_h = h << FP_SCALE;
    int32_t ys1 = (iA[3] < 0 ?   - scale_h :   1) - iA[3];
    int32_t ys2 = (iA[3] < 0 ? 0 : (1 - scale_h)) - iA[3];

    int32_t cl = _clip_l    ;
    int32_t cr = _clip_r + 1;

    int32_t div1 = iA[0] ? - iA[0] : -1;
    int32_t div2 = iA[3] ? - iA[3] : -1;
    int32_t y = min_y - max_y;

    startWrite();
    do
    {
      iA[2] += iA[1];
      iA[5] += iA[4];
      int32_t left  = std::max(cl, std::max((iA[2] + xs1) / div1, (iA[5] + ys1) / div2));
      int32_t right = std::min(cr, std::min((iA[2] + xs2) / div1, (iA[5] + ys2) / div2));
      if (left < right)
      {
        writeFillRectPreclipped(left, y + max_y, right - left, 1);
      }
    } while (++y);
    endWrite();
  }

  void LGFXBase::push_image_affine(const float* matrix, pixelcopy_t* pc)
  {
    int32_t min_y = matrix[3] * (pc->src_width  << FP_SCALE);
    int32_t max_y = matrix[4] * (pc->src_height << FP_SCALE);
    if ((min_y < 0) == (max_y < 0))
    {
      max_y += min_y;
      min_y = 0;
    }
    if (min_y > max_y) 
    {
      std::swap(min_y, max_y);
    }

    {
      int32_t offset_y32 = matrix[5] * (1 << FP_SCALE) + (1 << (FP_SCALE-1));
      min_y = std::max(_clip_t    , (offset_y32 + min_y - 1) >> FP_SCALE);
      max_y = std::min(_clip_b + 1, (offset_y32 + max_y + 1) >> FP_SCALE);
      if (min_y >= max_y) return;
    }


    int32_t iA[6];
    if (!make_invert_affine32(iA, matrix)) return;

    pc->src_x32_add = iA[0];
    pc->src_y32_add = iA[3];

    int32_t offset = (min_y << 1) - 1;
    iA[2] += ((iA[0] + iA[1] * offset) >> 1);
    iA[5] += ((iA[3] + iA[4] * offset) >> 1);

    int32_t scale_w = pc->src_width << FP_SCALE;
    int32_t xs1 = (iA[0] < 0 ?   - scale_w :   1) - iA[0];
    int32_t xs2 = (iA[0] < 0 ? 0 : (1 - scale_w)) - iA[0];

    int32_t scale_h = pc->src_height << FP_SCALE;
    int32_t ys1 = (iA[3] < 0 ?   - scale_h :   1) - iA[3];
    int32_t ys2 = (iA[3] < 0 ? 0 : (1 - scale_h)) - iA[3];

    int32_t cl = _clip_l    ;
    int32_t cr = _clip_r + 1;

    int32_t y = min_y - max_y;

    startWrite();
    do
    {
      iA[2] += iA[1];
      iA[5] += iA[4];
      int32_t left  = std::max(cl, std::max(iA[0] ? (iA[2] + xs1) / - iA[0] : cl, iA[3] ? (iA[5] + ys1) / - iA[3] : cl));
      int32_t right = std::min(cr, std::min(iA[0] ? (iA[2] + xs2) / - iA[0] : cr, iA[3] ? (iA[5] + ys2) / - iA[3] : cr));
      if (left < right)
      {
        pc->src_x32 = iA[2] + left * iA[0];
        if (static_cast<uint32_t>(pc->src_x) < pc->src_width)
        {
          pc->src_y32 = iA[5] + left * iA[3];
          if (static_cast<uint32_t>(pc->src_y) < pc->src_height)
          {
            pushImage_impl(left, y + max_y, right - left, 1, pc, true);
          }
        }
      }
    } while (++y);
    endWrite();
  }

  void LGFXBase::push_image_affine_aa(const float* matrix, pixelcopy_t* pc, pixelcopy_t* pc2)
  {
    int32_t min_y = matrix[3] * (pc->src_width  << FP_SCALE);
    int32_t max_y = matrix[4] * (pc->src_height << FP_SCALE);
    if ((min_y < 0) == (max_y < 0))
    {
      max_y += min_y;
      min_y = 0;
    }
    if (min_y > max_y) 
    {
      std::swap(min_y, max_y);
    }

    {
      int32_t offset_y32 = matrix[5] * (1 << FP_SCALE);
      min_y = std::max(_clip_t, (offset_y32 + min_y   ) >> FP_SCALE);
      max_y = std::min(_clip_b, (offset_y32 + max_y -1) >> FP_SCALE) + 1;
      if (min_y >= max_y) return;
    }

    int32_t iA[6];
    if (!make_invert_affine32(iA, matrix)) return;

    pc->src_x32_add = iA[0];
    pc->src_y32_add = iA[3];
    uint32_t x32_diff = std::min<uint32_t>(8 << FP_SCALE, std::max(abs(iA[0]), abs(iA[1])) - 1) >> 1;
    uint32_t y32_diff = std::min<uint32_t>(8 << FP_SCALE, std::max(abs(iA[3]), abs(iA[4])) - 1) >> 1;

    int32_t offset = (min_y << 1) - 1;
    iA[2] += ((iA[0] + iA[1] * offset) >> 1);
    iA[5] += ((iA[3] + iA[4] * offset) >> 1);

    int32_t scale_w = (pc->src_width << FP_SCALE) + (x32_diff << 1);
    int32_t xs1 = (iA[0] < 0 ?   - scale_w :   1) - iA[0] + x32_diff;
    int32_t xs2 = (iA[0] < 0 ? 0 : (1 - scale_w)) - iA[0] + x32_diff;

    int32_t scale_h = (pc->src_height << FP_SCALE) + (y32_diff << 1);
    int32_t ys1 = (iA[3] < 0 ?   - scale_h :   1) - iA[3] + y32_diff;
    int32_t ys2 = (iA[3] < 0 ? 0 : (1 - scale_h)) - iA[3] + y32_diff;

    int32_t cl = _clip_l    ;
    int32_t cr = _clip_r + 1;
    argb8888_t buffer[cr - cl];
    pc2->src_data = buffer;

    startWrite();
    do
    {
      iA[2] += iA[1];
      iA[5] += iA[4];
      int32_t left  = std::max(cl, std::max(iA[0] ? (iA[2] + xs1) / - iA[0] : cl, iA[3] ? (iA[5] + ys1) / - iA[3] : cl));
      int32_t right = std::min(cr, std::min(iA[0] ? (iA[2] + xs2) / - iA[0] : cr, iA[3] ? (iA[5] + ys2) / - iA[3] : cr));
      if (left < right)
      {
        int32_t len = right - left;

        uint32_t xs = iA[2] + left * iA[0];
        pc->src_x32 = xs - x32_diff;
        pc->src_xe32 = xs + x32_diff;
        uint32_t ys = iA[5] + left * iA[3];
        pc->src_y32 = ys - y32_diff;
        pc->src_ye32 = ys + y32_diff;

        pc->fp_copy(buffer, 0, len, pc);
        pc2->src_x = 0;
        pc2->src_y = 0;
        pushImageARGB_impl(left, min_y, len, 1, pc2);
      }
    } while (++min_y != max_y);
    endWrite();
  }

  void LGFXBase::readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* data)
  {
    pixelcopy_t p(nullptr, rgb332_t::depth, _read_conv.depth, false, getPalette());
    read_rect(x, y, w, h, data, &p);
  }
  void LGFXBase::readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data)
  {
    pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, getPalette());
    if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
      p.no_convert = false;
      p.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine_dst<rgb565_t>(_read_conv.depth);
    }
    read_rect(x, y, w, h, data, &p);
  }
  void LGFXBase::readRect(int32_t x, int32_t y, int32_t w, int32_t h, void* data)
  {
    pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, getPalette());
    if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
      p.no_convert = false;
      p.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine_dst<rgb888_t>(_read_conv.depth);
    }
    read_rect(x, y, w, h, data, &p);
  }

  void LGFXBase::scroll(int_fast16_t dx, int_fast16_t dy)
  {
    setColor(_base_rgb888);
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

  void LGFXBase::copyRect(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y)
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

  void LGFXBase::read_rect(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param)
  {
    _adjust_abs(x, w);
    if (x < 0) { w += x; x = 0; }
    if (w > _width - x)  w = _width  - x;
    if (w < 1) return;

    _adjust_abs(y, h);
    if (y < 0) { h += y; y = 0; }
    if (h > _height - y) h = _height - y;
    if (h < 1) return;

    readRect_impl(x, y, w, h, dst, param);
  }

  struct paint_point_t { int32_t lx,rx,y,oy; };

  static void paint_add_points(std::list<paint_point_t>& points, int lx, int rx, int y, int oy, bool* linebuf)
  {
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
  void LGFXBase::floodFill(int32_t x, int32_t y) {
    if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;
    bgr888_t target;
    readRectRGB(x, y, 1, 1, &target);
    if (_color.raw == _write_conv.convert(lgfx::color888(target.r, target.g, target.b))) return;

    pixelcopy_t p;
    p.transp = _read_conv.convert(lgfx::color888(target.r, target.g, target.b));
    switch (_read_conv.depth) {
    case 24: p.fp_copy = pixelcopy_t::compare_rgb_fast<bgr888_t>;  break;
    case 18: p.fp_copy = pixelcopy_t::compare_rgb_fast<bgr666_t>;  break;
    case 16: p.fp_copy = pixelcopy_t::compare_rgb_fast<swap565_t>; break;
    case  8: p.fp_copy = pixelcopy_t::compare_rgb_fast<rgb332_t>;  break;
    default: p.fp_copy = pixelcopy_t::compare_bit_fast;
      p.src_bits = _read_conv.depth;
      p.src_mask = (1 << p.src_bits) - 1;
      p.transp &= p.src_mask;
      break;
    }

    int32_t cl = _clip_l;
    int w = _clip_r - cl + 1;
    uint8_t bufIdx = 0;
    bool* linebufs[3] = { new bool[w], new bool[w], new bool[w] };
    int32_t bufY[3] = {-2, -2, -2};  // 3 line buffer (default: out of range.)
    bufY[0] = y;
    read_rect(cl, y, w, 1, linebufs[0], &p);
    std::list<paint_point_t> points;
    points.push_back({x, x, y, y});

    startWrite();
    while (!points.empty()) {
      int32_t y0 = bufY[bufIdx];
      auto it = points.begin();
      int32_t counter = 0;
      while (it->y != y0 && ++it != points.end()) ++counter;
      if (it == points.end()) {
        if (counter < 256) {
          ++bufIdx;
          int32_t y1 = bufY[(bufIdx  )%3];
          int32_t y2 = bufY[(bufIdx+1)%3];
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
        while (newy != bufY[bidx] && ++bidx != 3);
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
    int i = 0;
    do { delete[] linebufs[i]; } while (++i != 3);
    endWrite();
  }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

    static char* numberToStr(unsigned long n, char* buf, size_t buflen, uint8_t base)
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

    static char* numberToStr(long n, char* buf, size_t buflen, uint8_t base)
    {
      if (n >= 0) return numberToStr((unsigned long) n, buf, buflen, base);
      auto res = numberToStr(- n, buf, buflen, 10) - 1;
      res[0] = '-';
      return res;
    }

    static char* floatToStr(double number, char* buf, size_t /*buflen*/, uint8_t digits)
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

    uint16_t LGFXBase::decodeUTF8(uint8_t c)
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

      return c; // fall-back to extended ASCII
    }

    int32_t LGFXBase::fontHeight(const IFont* font) const
    {
      FontMetrics fm;
      font->getDefaultMetric(&fm);
      return fm.height * _text_style.size_y;
    }

    int32_t LGFXBase::fontWidth(const IFont* font) const
    {
      FontMetrics fm;
      font->getDefaultMetric(&fm);
      return fm.width * _text_style.size_x;
    }

    int32_t LGFXBase::textLength(const char *string, int32_t width)
    {
      if (!string || !string[0]) return 0;

      auto sx = _text_style.size_x;

      int32_t left = 0;
      int32_t right = 0;
      auto str = string;
      do {
        uint16_t uniCode = *string;
        if (_text_style.utf8) {
          do {
            uniCode = decodeUTF8(*string);
          } while (uniCode < 0x20 && *(++string));
          if (uniCode < 0x20) break;
        }

        //if (!_font->updateFontMetric(&_font_metrics, uniCode)) continue;
        _font->updateFontMetric(&_font_metrics, uniCode);
        if (left == 0 && right == 0 && _font_metrics.x_offset < 0) left = right = - (int)(_font_metrics.x_offset * sx);
        right = left + std::max<int>(_font_metrics.x_advance * sx, int(_font_metrics.width * sx) + int(_font_metrics.x_offset * sx));
        //right = left + (int)(std::max<int>(_font_metrics.x_advance, _font_metrics.width + _font_metrics.x_offset) * sx);
        left += (int)(_font_metrics.x_advance * sx);
        if (width <= right) return string - str;
      } while (*(++string));
      return string - str;
    }

    int32_t LGFXBase::textWidth(const char *string)
    {
      if (!string || !string[0]) return 0;

      auto sx = _text_style.size_x;

      int32_t left = 0;
      int32_t right = 0;
      do {
        uint16_t uniCode = *string;
        if (_text_style.utf8) {
          do {
            uniCode = decodeUTF8(*string);
          } while (uniCode < 0x20 && *(++string));
          if (uniCode < 0x20) break;
        }

        //if (!_font->updateFontMetric(&_font_metrics, uniCode)) continue;
        _font->updateFontMetric(&_font_metrics, uniCode);
        if (left == 0 && right == 0 && _font_metrics.x_offset < 0) left = right = - (int)(_font_metrics.x_offset * sx);
        right = left + std::max<int>(_font_metrics.x_advance*sx, int(_font_metrics.width*sx) + int(_font_metrics.x_offset * sx));
        //right = left + (int)(std::max<int>(_font_metrics.x_advance, _font_metrics.width + _font_metrics.x_offset) * sx);
        left += (int)(_font_metrics.x_advance * sx);
      } while (*(++string));
      return right;
    }


    size_t LGFXBase::drawNumber(long long_num, int32_t poX, int32_t poY)
    {
      constexpr size_t len = 8 * sizeof(long) + 1;
      char buf[len];
      return drawString(numberToStr(long_num, buf, len, 10), poX, poY);
    }

    size_t LGFXBase::drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY)
    {
      size_t len = 14 + dp;
      char buf[len];
      return drawString(floatToStr(floatNumber, buf, len, dp), poX, poY);
    }

    size_t LGFXBase::drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font) {
      if (_font == fontdata[font]) return drawChar(uniCode, x, y);
      _filled_x = 0;
      return fontdata[font]->drawChar(this, x, y, uniCode, &_text_style);
    }


    size_t LGFXBase::draw_string(const char *string, int32_t x, int32_t y, textdatum_t datum)
    {
      int16_t sumX = 0;
      int32_t cwidth = textWidth(string); // Find the pixel width of the string in the font
      int32_t cheight = _font_metrics.height * _text_style.size_y;

      if (string && string[0]) {
        auto tmp = string;
        do {
          uint16_t uniCode = *tmp;
          if (_text_style.utf8) {
            do {
              uniCode = decodeUTF8(*tmp); 
            } while (uniCode < 0x20 && *++tmp);
            if (uniCode < 0x20) break;
          }
          //if (_font->updateFontMetric(&_font_metrics, uniCode))
          {
            _font->updateFontMetric(&_font_metrics, uniCode);
            if (_font_metrics.x_offset < 0) sumX = - _font_metrics.x_offset * _text_style.size_x;
            break;
          }
        } while (*++tmp);
      }
      if (datum & middle_left) {          // vertical: middle
        y -= cheight >> 1;
      } else if (datum & bottom_left) {   // vertical: bottom
        y -= cheight;
      } else if (datum & baseline_left) { // vertical: baseline
        y -= (int)(_font_metrics.baseline * _text_style.size_y);
      }

      this->startWrite();
      int32_t padx = _padding_x;
      if ((_text_style.fore_rgb888 != _text_style.back_rgb888) && (padx > cwidth)) {
        this->setColor(_text_style.back_rgb888);
        if (datum & top_center) {
          auto halfcwidth = cwidth >> 1;
          auto halfpadx = (padx >> 1);
          this->writeFillRect(x - halfpadx, y, halfpadx - halfcwidth, cheight);
          halfcwidth = cwidth - halfcwidth;
          halfpadx = padx - halfpadx;
          this->writeFillRect(x + halfcwidth, y, halfpadx - halfcwidth, cheight);
        } else if (datum & top_right) {
          this->writeFillRect(x - padx, y, padx - cwidth, cheight);
        } else {
          this->writeFillRect(x + cwidth, y, padx - cwidth, cheight);
        }
      }

      if (datum & top_center) {           // Horizontal: middle
        x -= cwidth >> 1;
      } else if (datum & top_right) {     // Horizontal: right
        x -= cwidth;
      }

      y -= int(_font_metrics.y_offset * _text_style.size_y);

      _filled_x = 0;
      if (string && string[0]) {
        do {
          uint16_t uniCode = *string;
          if (_text_style.utf8) {
            do {
              uniCode = decodeUTF8(*string);
            } while (uniCode < 0x20 && *++string);
            if (uniCode < 0x20) break;
          }
//          sumX += (fpDrawChar)(this, x + sumX, y, uniCode, &_text_style, _font);
          sumX += _font->drawChar(this, x + sumX, y, uniCode, &_text_style);
        } while (*(++string));
      }
      this->endWrite();

      return sumX;
    }

    size_t LGFXBase::write(uint8_t utf8)
    {
      if (utf8 == '\r') return 1;
      if (utf8 == '\n') {
        _filled_x = (_textscroll) ? this->_sx : 0;
        _cursor_x = _filled_x;
        _cursor_y += _font_metrics.y_advance * _text_style.size_y;
      } else {
        uint16_t uniCode = utf8;
        if (_text_style.utf8) {
          uniCode = decodeUTF8(utf8);
          if (uniCode < 0x20) return 1;
        }
        //if (!(fpUpdateFontSize)(this, uniCode)) return 1;
        //if (!_font->updateFontMetric(&_font_metrics, uniCode)) return 1;
        _font->updateFontMetric(&_font_metrics, uniCode);

        int32_t xo = _font_metrics.x_offset  * _text_style.size_x;
        int32_t w  = std::max(xo + _font_metrics.width * _text_style.size_x, _font_metrics.x_advance * _text_style.size_x);
        if (_textscroll || _textwrap_x) {
          int32_t llimit = _textscroll ? this->_sx : this->_clip_l;
          int32_t rlimit = _textscroll ? this->_sx + this->_sw : (this->_clip_r + 1);
          if (_cursor_x + w > rlimit) {
            _filled_x = llimit;
            _cursor_x = llimit - std::min<int32_t>(0, xo);
            _cursor_y += _font_metrics.y_advance * _text_style.size_y;
          }
          if (_cursor_x < llimit - xo) _cursor_x = llimit - xo;
        }

        int32_t h  = _font_metrics.height * _text_style.size_y;

        int32_t ydiff = 0;
        if (_text_style.datum & middle_left) {          // vertical: middle
          ydiff -= h >> 1;
        } else if (_text_style.datum & bottom_left) {   // vertical: bottom
          ydiff -= h;
        } else if (_text_style.datum & baseline_left) { // vertical: baseline
          ydiff -= (int)(_font_metrics.baseline * _text_style.size_y);
        }
        int32_t y = _cursor_y + ydiff;

        if (_textscroll) {
          if (y < this->_sy) y = this->_sy;
          else {
            int yshift = (this->_sy + this->_sh) - (y + h);
            if (yshift < 0) {
              this->scroll(0, yshift);
              y += yshift;
            }
          }
        } else if (_textwrap_y) {
          if (y + h > (this->_clip_b + 1)) {
            y = this->_clip_t;
          } else
          if (y < this->_clip_t) y = this->_clip_t;
        }
        _cursor_y = y - ydiff;
        y -= int(_font_metrics.y_offset  * _text_style.size_y);
        //_cursor_x += (fpDrawChar)(this, _cursor_x, y, uniCode, &_text_style, _font);
        _cursor_x += _font->drawChar(this, _cursor_x, y, uniCode, &_text_style);
      }

      return 1;
    }

    size_t LGFXBase::printNumber(unsigned long n, uint8_t base)
    {
      size_t len = 8 * sizeof(long) + 1;
      char buf[len];
      return write(numberToStr(n, buf, len, base));
    }

    size_t LGFXBase::printFloat(double number, uint8_t digits)
    {
      size_t len = 14 + digits;
      char buf[len];
      return write(floatToStr(number, buf, len, digits));
    }

  #if !defined (ARDUINO)
    size_t LGFXBase::printf(const char * format, ...) 
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
  #endif

    void LGFXBase::setFont(const IFont* font)
    {
      _runtime_font.reset();
      if (font == nullptr) font = &fonts::Font0;
      _font = font;
      //_decoderState = utf8_decode_state_t::utf8_state0;

      font->getDefaultMetric(&_font_metrics);

      //switch (font->getType()) {
      //default:
      //case IFont::font_type_t::ft_glcd: fpDrawChar = drawCharGLCD;  break;
      //case IFont::font_type_t::ft_bmp:  fpDrawChar = drawCharBMP;   break;
      //case IFont::font_type_t::ft_rle:  fpDrawChar = drawCharRLE;   break;
      //case IFont::font_type_t::ft_bdf:  fpDrawChar = drawCharBDF;   break;
      //case IFont::font_type_t::ft_gfx:  fpDrawChar = drawCharGFXFF; break;
      //}
    }

    /// load VLW font
    bool LGFXBase::loadFont(const uint8_t* array)
    {
      this->unloadFont();
      _font_data.set(array);

      auto font = new VLWfont();
      this->_runtime_font.reset(font);

      if (font->loadFont(&_font_data)) {
        this->_font = font;
        this->_font->getDefaultMetric(&this->_font_metrics);
        return true;
      } else {
        this->unloadFont();
        return false;
      }
    }

    void LGFXBase::unloadFont(void)
    {
      if (_runtime_font.get() != nullptr) { setFont(&fonts::Font0); }
    }

    void LGFXBase::showFont(uint32_t td)
    {
      auto font = (const VLWfont*)this->_font;
      if (!font->_fontLoaded) return;

      int16_t x = this->width();
      int16_t y = this->height();
      uint32_t timeDelay = 0;    // No delay before first page

      this->fillScreen(this->_text_style.back_rgb888);

      for (uint16_t i = 0; i < font->gCount; i++)
      {
        // Check if this will need a new screen
        if (x + font->gdX[i] + font->gWidth[i] >= this->width())  {
          x = - font->gdX[i];

          y += font->yAdvance;
          if (y + font->maxAscent + font->descent >= this->height()) {
            x = - font->gdX[i];
            y = 0;
            delay(timeDelay);
            timeDelay = td;
            this->fillScreen(this->_text_style.back_rgb888);
          }
        }

        this->drawChar(font->gUnicode[i], x, y);
        x += font->gxAdvance[i];
        //yield();
      }

      delay(timeDelay);
      this->fillScreen(this->_text_style.back_rgb888);
      //fontFile.close();
    }

    void LGFXBase::setAttribute(attribute_t attr_id, uint8_t param) {
      switch (attr_id) {
      case cp437_switch:
        _text_style.cp437 = param;
        break;
      case utf8_switch:
        _text_style.utf8  = param;
        _decoderState = utf8_decode_state_t::utf8_state0;
        break;
      case epd_mode_switch:
        _epd_mode = (epd_mode_t)param;
        break;
      default: break;
      }
    }

    uint8_t LGFXBase::getAttribute(attribute_t attr_id) {
      switch (attr_id) {
        case cp437_switch: return _text_style.cp437;
        case utf8_switch: return _text_style.utf8;
        case epd_mode_switch: return _epd_mode;
        default: return 0;
      }
    }

//----------------------------------------------------------------------------
    void LGFXBase::qrcode(const char *string, int32_t x, int32_t y, int32_t width, uint8_t version) {
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
        uint8_t qrcodeData[lgfx_qrcode_getBufferSize(version)];
        if (0 != lgfx_qrcode_initText(&qrcode, qrcodeData, version, 0, string)) continue;
        int_fast16_t thickness = width / qrcode.size;
        if (!thickness) break;
        int_fast16_t lineLength = qrcode.size * thickness;
        int_fast16_t xOffset = x + ((width - lineLength) >> 1);
        int_fast16_t yOffset = y + ((width - lineLength) >> 1);
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
//----------------------------------------------------------------------------

  struct image_info_t
  {
    LGFXBase* gfx;
    int32_t x;
    int32_t y;
    int32_t maxWidth;
    int32_t maxHeight;
    int32_t offX;
    int32_t offY;
    float zoom_x;
    float zoom_y;
    datum_t datum;

  protected:
    int32_t _cl;
    int32_t _ct;
    int32_t _cw;
    int32_t _ch;

  public:

    void end(void)
    {
      gfx->setClipRect(_cl, _ct, _cw, _ch);
    }

    bool begin( LGFXBase* gfx
              , int32_t x
              , int32_t y
              , int32_t maxWidth
              , int32_t maxHeight
              , int32_t offX
              , int32_t offY
              , float zoom_x
              , float zoom_y
              , datum_t datum
              , int32_t w
              , int32_t h
              )
    {
      gfx->getClipRect(&_cl, &_ct, &_cw, &_ch);

      if (zoom_y <= 0.0f || zoom_x <= 0.0f)
      {
        float fit_width  = (maxWidth  > 0) ? maxWidth  : gfx->width();
        float fit_height = (maxHeight > 0) ? maxHeight : gfx->height();
        if (zoom_x <= -1.0f) { zoom_x = fit_width  / w; }
        if (zoom_y <= -1.0f) { zoom_y = fit_height / h; }
        if (zoom_x <= 0.0f)
        {
          if (zoom_y <= 0.0f)
          {
            zoom_y = std::min<float>(fit_width / w, fit_height / h);
          }
          zoom_x = zoom_y;
        }
        if (zoom_y <= 0.0f)
        {
          zoom_y = zoom_x;
        }
      }

      if (datum)
      {
        if (datum & (datum_t::top_center | datum_t::top_right))
        {
          float fit_width  = (maxWidth  > 0) ? maxWidth  : gfx->width();
          float fw = fit_width - w * zoom_x;
          if (datum & datum_t::top_center) { fw /= 2; }
          offX -= fw;
        }
        if (datum & (datum_t::middle_left | datum_t::bottom_left | datum_t::baseline_left))
        {
          float fit_height = (maxHeight > 0) ? maxHeight : gfx->height();
          float fh = fit_height - h * zoom_y;
          if (datum & datum_t::middle_left) { fh /= 2; }
          offY -= fh;
        }
      }

      if (maxWidth <= 0) { maxWidth = INT16_MAX; }
      int32_t right = x + maxWidth;
      const int32_t cr = _cw + _cl;
      if (right > cr) { right = cr; }
      if (x < _cl) { offX -= x - _cl; x = _cl; }
      if (offX < 0) { x -= offX; offX = 0; }
      if (maxWidth > right - x) { maxWidth = right - x; }
      int32_t ww = ((int32_t)ceilf(w * zoom_x)) - offX;
      if (maxWidth > ww) { maxWidth = ww; }

      if (maxHeight <= 0) { maxHeight = INT16_MAX; }
      int32_t bottom = y + maxHeight;
      const int32_t cb = _ch + _ct;
      if (bottom > cb) { bottom = cb; }
      if (y < _ct) { offY -= y - _ct; y = _ct; }
      if (offY < 0) { y -= offY; offY = 0; }
      if (maxHeight > bottom - y) { maxHeight = bottom - y; }
      int32_t hh = ((int32_t)ceilf(h * zoom_y)) - offY;
      if (maxHeight > hh) { maxHeight = hh; }

      this->gfx       = gfx      ;
      this->x         = x        ;
      this->y         = y        ;
      this->maxWidth  = maxWidth ;
      this->maxHeight = maxHeight;
      this->offX      = offX     ;
      this->offY      = offY     ;
      this->zoom_x    = zoom_x   ;
      this->zoom_y    = zoom_y   ;
      this->datum     = datum    ;

      if (maxWidth  <= 0) return false;
      if (maxHeight <= 0) return false;

      gfx->setClipRect(x, y, maxWidth, maxHeight);
      return true;
    }
  };

  bool LGFXBase::draw_bmp(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float zoom_x, float zoom_y, datum_t datum)
  {
    prepareTmpTransaction(data);
    bitmap_header_t bmpdata;
    if (!bmpdata.load_bmp_header(data) || (bmpdata.biCompression > 3)) {
      return false;
    }

    uint32_t seekOffset = bmpdata.bfOffBits;
    uint_fast16_t bpp = bmpdata.biBitCount; // 24 bcBitCount 24=RGB24bit
    int32_t w = bmpdata.biWidth;
    int32_t h = abs(bmpdata.biHeight);  // bcHeight Image height (pixels)

    image_info_t info;
    if (!info.begin( this
                   , x
                   , y
                   , maxWidth
                   , maxHeight
                   , offX
                   , offY
                   , zoom_x
                   , zoom_y
                   , datum
                   , w, h))
    {
      return true;
    }

    argb8888_t *palette = nullptr;
    if (bpp <= 8) {
      palette = (argb8888_t*)alloca(sizeof(argb8888_t*) * (1 << bpp));
      if (!palette) { return false; }
      data->seek(bmpdata.biSize + 14);
      data->read((uint8_t*)palette, (1 << bpp)*sizeof(argb8888_t)); // load palette
    }

    x = info.x - info.offX;
    y = info.y - info.offY;
    zoom_x = info.zoom_x;
    zoom_y = info.zoom_y;

    data->seek(seekOffset);

    auto dst_depth = this->_write_conv.depth;
    uint32_t buffersize = ((w * bpp + 31) >> 5) << 2;  // readline 4Byte align.
    auto lineBuffer = (uint8_t*)alloca(buffersize + 4);

    pixelcopy_t p(lineBuffer, dst_depth, (color_depth_t)bpp, this->_palette_count, palette);
    p.no_convert = false;
    if (8 >= bpp && !this->_palette_count) {
      p.fp_copy = pixelcopy_t::get_fp_copy_palette_affine<argb8888_t>(dst_depth);
    } else {
      if (bpp == 16) {
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine<rgb565_t>(dst_depth);
      } else if (bpp == 24) {
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine<rgb888_t>(dst_depth);
      } else if (bpp == 32) {
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine<argb8888_t>(dst_depth);
      }
    }
    p.src_x32_add = (1u << FP_SCALE) / zoom_x;

      //If the value of Height is positive, the image data is from bottom to top
      //If the value of Height is negative, the image data is from top to bottom.
    int32_t flow = (bmpdata.biHeight > 0) ? -1 : 1;
    if (bmpdata.biHeight > 0) y += ceilf(h * zoom_y) - 1;

    int32_t y32 = (y << FP_SCALE);
    int32_t dst_y32_add = (1u << FP_SCALE) * zoom_y;
    if (bmpdata.biHeight > 0) dst_y32_add = - dst_y32_add;

    this->startWrite(!data->hasParent());

    float affine[6] = { zoom_x, 0.0f, (float)x, 0.0f, 1.0f, 0.0f };
    p.src_bitwidth = w;
    p.src_width = w;
    p.src_height = 1;

    do
    {
      data->preRead();
      if (bmpdata.biCompression == 1)
      {
        bmpdata.load_bmp_rle8(data, lineBuffer, w);
      } else
      if (bmpdata.biCompression == 2)
      {
        bmpdata.load_bmp_rle4(data, lineBuffer, w);
      }
      else
      {
        data->read(lineBuffer, buffersize);
      }
      data->postRead();
      y32 += dst_y32_add;
      int32_t next_y = y32 >> FP_SCALE;
      while (y != next_y)
      {
        p.src_x32 = 0;
        affine[5] = y;
        this->push_image_affine(affine, &p);
        y += flow;
      }
    } while (--h);

    info.end();

    this->endWrite();

    return true;
  }


  struct image_decoder_t : public image_info_t
  {
    DataWrapper* data;

    static uint32_t read_data(void* self, uint8_t* buf, uint32_t len)
    {
      auto data = ((image_decoder_t*)self)->data;
      auto res = len;
      data->preRead();
      if (buf) {
        res = data->read(buf, len);
      } else {
        data->skip(len);
      }
      return res;
    }
  };

  struct draw_jpg_info_t : public image_decoder_t
  {
    pixelcopy_t *pc;
  };

  static uint32_t jpg_push_image(void *device, void *bitmap, JRECT *rect)
  {
    draw_jpg_info_t *jpeg = static_cast<draw_jpg_info_t*>(device);
    jpeg->pc->src_data = bitmap;
    auto data = static_cast<DataWrapper*>(jpeg->data);
    data->postRead();
    int32_t x = rect->left;
    int32_t y = rect->top;
    int32_t w = rect->right  - rect->left + 1;
    int32_t h = rect->bottom - rect->top + 1;
    jpeg->gfx->pushImage( jpeg->x + x
                        , jpeg->y + y
                        , w
                        , h
                        , jpeg->pc
                        , false);
    return 1;
  }

  static uint32_t jpg_push_image_affine(void *device, void *bitmap, JRECT *rect)
  {
    draw_jpg_info_t *jpeg = static_cast<draw_jpg_info_t*>(device);
    jpeg->pc->src_data = bitmap;
    auto data = static_cast<DataWrapper*>(jpeg->data);
    data->postRead();

    int32_t x = rect->left;
    int32_t y = rect->top;
    int32_t w = rect->right  - rect->left + 1;
    int32_t h = rect->bottom - rect->top + 1;
    float affine[6] =
    { jpeg->zoom_x, 0.0f , x * jpeg->zoom_x + jpeg->x
    , 0.0f , jpeg->zoom_y, y * jpeg->zoom_y + jpeg->y
    };
    jpeg->gfx->pushImageAffine( affine, w, h, (bgr888_t*)jpeg->pc->src_data );
    return 1;
  }

  bool LGFXBase::draw_jpg(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float zoom_x, float zoom_y, datum_t datum)
  {
    draw_jpg_info_t jpeg;
    pixelcopy_t pc(nullptr, this->getColorDepth(), bgr888_t::depth, this->hasPalette());
    jpeg.pc = &pc;
    jpeg.data = data;

    lgfxJdec jpegdec;

    static constexpr uint16_t sz_pool = 3100;
    uint8_t *pool = (uint8_t*)heap_alloc_dma(sz_pool);
    if (!pool) {
//        ESP_LOGE("LGFX","memory allocation failure");
      return false;
    }

    auto jres = lgfx_jd_prepare(&jpegdec, jpeg.read_data, pool, sz_pool, &jpeg);

    if (jres != JDR_OK) {
//ESP_LOGE("LGFX","jpeg prepare error:%x", jres);
      heap_free(pool);
      return false;
    }

    if (!jpeg.begin( this
                   , x
                   , y
                   , maxWidth
                   , maxHeight
                   , offX
                   , offY
                   , zoom_x
                   , zoom_y
                   , datum
                   , jpegdec.width, jpegdec.height))
    {
      heap_free(pool);
      return false;
    }

    if (jpeg.offX) { jpeg.x -= jpeg.offX; jpeg.offX = 0; }
    if (jpeg.offY) { jpeg.y -= jpeg.offY; jpeg.offY = 0; }

    jpeg_div::jpeg_div_t div = jpeg_div::jpeg_div_t::JPEG_DIV_NONE;
    float scale_max = std::max(jpeg.zoom_x, jpeg.zoom_y);
    if (scale_max <= 0.5f)
    {
      if (     scale_max <= 0.125f) { div = jpeg_div::jpeg_div_t::JPEG_DIV_8; }
      else if (scale_max <= 0.25f)  { div = jpeg_div::jpeg_div_t::JPEG_DIV_4; }
      else                          { div = jpeg_div::jpeg_div_t::JPEG_DIV_2; }

      jpeg.zoom_x *= 1 << div;
      jpeg.zoom_y *= 1 << div;
    }

    this->startWrite(!data->hasParent());

    jres = lgfx_jd_decomp(&jpegdec, jpeg.zoom_x == 1.0f && jpeg.zoom_y == 1.0f ? jpg_push_image : jpg_push_image_affine, div);

    jpeg.end();
    this->endWrite();

    heap_free(pool);

    if (jres != JDR_OK) {
//ESP_LOGE("LGFX","jpeg decomp error:%x", jres);
      return false;
    }
    return true;
  }



  struct png_file_decoder_t : public image_decoder_t
  {
    bgra8888_t* lineBuffer;
    pixelcopy_t *pc;
  };

//-----


  static void png_draw_alpha_callback(void *user_data, uint32_t x, uint32_t y, uint_fast8_t div_x, size_t len, const uint8_t* argb)
  {
    auto p = (png_file_decoder_t*)user_data;

    int32_t y0 = (int32_t)y - p->offY;
    int32_t y1 = y0 + 1;
    if (y0 < 0) y0 = 0;
    if (y1 > p->maxHeight) y1 = p->maxHeight;
    if (y0 >= y1) return;

/*
    while ((argb[idx * 4 + 3] == 0) && ++idx != len);
    if (idx == len) return;
    while ((argb[len * 4 - 1] == 0) && idx != --len);
    if (idx)
    {
      len -= idx;
      argb += idx * 4;
      x += idx * div_x;
      idx = 0;
    }
///*/

    while ((int32_t)x < p->offX && --len)
    {
      x += div_x;
      argb += 4;
    }
    x -= p->offX;

    if (!len || (int32_t)x >= p->maxWidth) return;

    size_t idx = 0;
    while ((argb[idx * 4] == 255) && ++idx != len);
    p->data->postRead();

    bool hasAlpha = (idx != len);
    if (hasAlpha)
    {
      p->gfx->readRect(p->x, p->y + y0, p->maxWidth, 1, p->lineBuffer);
      do
      {
        uint_fast8_t a = argb[0];
        if (a) {
          if (a == 255) {
            p->lineBuffer[x].set(*(uint32_t*)argb);
          } else {
            auto data = &p->lineBuffer[x];
            uint_fast8_t inv = 255 - a;
            data->set( (argb[1] * a + data->r * inv + 255) >> 8
                     , (argb[2] * a + data->g * inv + 255) >> 8
                     , (argb[3] * a + data->b * inv + 255) >> 8
                     );
          }
        }
        x += div_x;
        if ((int32_t)x >= p->maxWidth) break;
        argb += 4;
      } while (--len);
      p->pc->src_data = p->lineBuffer;
      p->gfx->pushImage(p->x, p->y + y0, p->maxWidth, 1, p->pc, false);
    }
    else
    if (div_x == 1)
    {
      p->pc->src_data = argb;
      p->gfx->pushImage(p->x, p->y + y0, p->maxWidth, 1, p->pc, false);
    }
    else
    {
      do
      {
        p->gfx->setColor(color888(argb[1], argb[2], argb[3]));
        p->gfx->writeFillRectPreclipped(p->x + x, p->y + y0, 1, 1);
        x += div_x;
        if ((int32_t)x >= p->maxWidth) break;
        argb += 4;
      } while (--len);
    }
  }

  static void png_draw_alpha_scale_callback(void *user_data, uint32_t x, uint32_t y, uint_fast8_t div_x, size_t len, const uint8_t* argb)
  {
    auto p = (png_file_decoder_t*)user_data;

    int32_t y0 = ceilf( y      * p->zoom_y) - p->offY;
    if (y0 < 0) y0 = 0;
    int32_t y1 = ceilf((y + 1) * p->zoom_y) - p->offY;
    if (y1 > p->maxHeight) y1 = p->maxHeight;
    if (y0 >= y1) return;

    size_t idx = 0;
/*
    while ((argb[idx * 4 + 3] == 0) && ++idx != len);
    if (idx == len) return;
    while ((argb[len * 4 - 1] == 0) && idx != --len);
    if (idx)
    {
      len -= idx;
      argb += idx * 4;
      x += idx * div_x;
      idx = 0;
    }
//*/

/*
    int32_t left = ceilf( x      * p->zoom_x) - p->offX;
    if (left < 0) left = 0;
    int32_t right = ceilf((x + (len-1) * div_x + 1) * p->zoom_x) - p->offX;
    if (right > p->maxWidth) right = p->maxWidth;
//*/
    p->data->postRead();

    while ((argb[idx * 4] == 255) && ++idx != len);
    bool hasAlpha = (idx != len);
    if (hasAlpha)
    {
// ESP_LOGE("TR","alpha:%d", argb[idx * 4 + 3]);
    //   uint32_t left = idx;
    //   idx = len;
    //   while (idx-- && (0 == argb[idx * 4 + 3] || argb[idx * 4 + 3] == 255));
    //   uint32_t right = idx;
      do
      {
        p->gfx->readRect(p->x, p->y + y0, p->maxWidth, 1, p->lineBuffer);
        const uint8_t* argbbuf = argb;
        size_t loop = len;
        size_t xtmp = x;
        do
        {
          int32_t l = ceilf( xtmp      * p->zoom_x) - p->offX;
          if (l < 0) l = 0;
          int32_t r = ceilf((xtmp + 1) * p->zoom_x) - p->offX;
          if (r > p->maxWidth) r = p->maxWidth;
          if (l < r)
          {
            uint_fast8_t a = argbbuf[0];
            if (a) {
              if (a == 255)
              {
                do
                {
                  p->lineBuffer[l].set(*(uint32_t*)argbbuf);
                } while (++l < r);
              }
              else
              {
                uint_fast8_t inv = 255 - a;
                size_t ar = argbbuf[1] * a + 255;
                size_t ag = argbbuf[2] * a + 255;
                size_t ab = argbbuf[3] * a + 255;
                do
                {
                  auto data = &p->lineBuffer[l];
                  data->set( (ar + data->r * inv) >> 8
                           , (ag + data->g * inv) >> 8
                           , (ab + data->b * inv) >> 8);
                } while (++l < r);
              }
            }
          }
          argbbuf += 4;
          xtmp += div_x;
        } while (--loop);
        p->gfx->pushImage(p->x, p->y + y0, p->maxWidth, 1, p->pc, true);
      } while (++y0 != y1);
    }
    else
    if (div_x == 1)
    {
//*  
      p->gfx->waitDMA();
      do
      {
        int32_t l = ceilf( x      * p->zoom_x) - p->offX;
        if (l < 0) l = 0;
        int32_t r = ceilf((x + 1) * p->zoom_x) - p->offX;
        if (r > p->maxWidth) r = p->maxWidth;
        if (l < r)
        {
          do {
            p->lineBuffer[l].set(*(uint32_t*)argb);
          } while (++l < r);
        }
        argb += 4;
        ++x;
      } while (--len);
      do {
        p->gfx->pushImage(p->x, p->y + y0, p->maxWidth, 1, p->pc, true);
      } while (++y0 != y1);
/*/
      p->pc->src_x32_add = (1 << FP_SCALE) / p->zoom_x;
      p->pc->src_data = argb;
      int32_t l = - p->offX;
      do {
        p->pc->src_x32 = 0;
        p->gfx->pushImage(p->x + l, p->y + y0, p->maxWidth - l, 1, p->pc, true);
      } while (++y0 != y1);
      p->pc->src_x32_add = (1 << FP_SCALE);
      p->pc->src_data = p->lineBuffer;
//*/
    }
    else
    {
      size_t h = y1 - y0;
      do
      {
        int32_t l = ceilf( x      * p->zoom_x) - p->offX;
        if (l < 0) l = 0;
        int32_t r = ceilf((x + 1) * p->zoom_x) - p->offX;
        if (r > p->maxWidth) r = p->maxWidth;
        if (l < r)
        {
          p->gfx->setColor(color888(argb[1], argb[2], argb[3]));
          p->gfx->writeFillRectPreclipped(p->x + l, p->y + y0, r - l, h);
        }
        argb += 4;
        x += div_x;
      } while (--len);
    }
  }

  bool LGFXBase::draw_png(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float zoom_x, float zoom_y, datum_t datum)
  {
    pngle_t *pngle = lgfx_pngle_new();
    if (pngle == nullptr) { return false; }

    prepareTmpTransaction(data);
    png_file_decoder_t png;
    png.lineBuffer = nullptr;
    png.data = data;

    if (lgfx_pngle_prepare(pngle, image_decoder_t::read_data, &png) < 0)
    {
      lgfx_pngle_destroy(pngle);
      return false;
    }

    // png_init_callback(pngle, lgfx_pngle_get_width(pngle), lgfx_pngle_get_height(pngle));
    if (!png.begin( this
                  , x
                  , y
                  , maxWidth
                  , maxHeight
                  , offX
                  , offY
                  , zoom_x
                  , zoom_y
                  , datum
                  , lgfx_pngle_get_width(pngle), lgfx_pngle_get_height(pngle)))
    {
      lgfx_pngle_destroy(pngle);
      return true;
    }

//  pixelcopy_t pc(nullptr, this->getColorDepth(), bgr888_t::depth, this->_palette_count);
    pixelcopy_t pc(nullptr, this->getColorDepth(), bgra8888_t::depth, this->_palette_count);
    if (this->hasPalette() || pc.dst_bits < 8) {
      pc.fp_copy = pixelcopy_t::copy_bit_affine;
      pc.fp_skip = pixelcopy_t::skip_bit_affine;
    }
    else
    {
      pc.fp_skip = pixelcopy_t::skip_rgb_affine<bgra8888_t>;
      pc.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine<bgra8888_t>(this->getColorDepth());
    }
    png.lineBuffer = (bgra8888_t*)heap_alloc_dma(sizeof(bgra8888_t) * png.maxWidth);
    pc.src_data = png.lineBuffer;

    png.pc = &pc;


    this->startWrite(!data->hasParent());

    auto res = lgfx_pngle_decomp(pngle, png.zoom_x == 1.0f && png.zoom_y == 1.0f ? png_draw_alpha_callback : png_draw_alpha_scale_callback);

    this->endWrite();
    if (png.lineBuffer) {
      this->waitDMA();
      heap_free(png.lineBuffer);
    }
    png.end();
    lgfx_pngle_destroy(pngle);

    return res < 0 ? false : true;
  }

  struct png_encoder_t
  {
    LGFXBase* gfx;
    int32_t x;
    int32_t y;
  };

  static uint8_t *png_encoder_get_row( uint8_t *pImage, int flip, int w, int h, int y, int, void *target )
  {
    auto enc = static_cast<png_encoder_t*>(target);
    uint32_t ypos = (flip ? (h - 1 - y) : y);
    enc->gfx->readRectRGB( enc->x, enc->y + ypos, w, 1, pImage );
    return pImage;
  }

  void* LGFXBase::createPng(size_t* datalen, int32_t x, int32_t y, int32_t width, int32_t height)
  {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (width  == 0) width  = _width  - x;
    if (height == 0) height = _height - y;
    if (width  > _width  - x) width  = _width  - x;
    if (height > _height - y) height = _height - y;

    if (width < 0 || height < 0) return nullptr;

    void* rgbBuffer = heap_alloc_dma(width * 3);

    png_encoder_t enc = { this, x, y };
    
    auto res = tdefl_write_image_to_png_file_in_memory_ex_with_cb(rgbBuffer, width, height, 3, datalen, 6, 0, (tdefl_get_png_row_func)png_encoder_get_row, &enc);

    heap_free(rgbBuffer);

    return res;
  }
//----------------------------------------------------------------------------
 }
}
#endif
