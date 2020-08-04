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
#include "LGFXBase.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <list>

namespace lgfx
{
  static constexpr float deg_to_rad = 0.017453292519943295769236907684886;

  void LGFXBase::setAddrWindow(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
  {
    if (_adjust_abs(x, w)||_adjust_abs(y, h)) return;
    bool tr = !_transaction_count;
    if (tr) beginTransaction();
    setWindow(x, y, x + w - 1, y + h - 1);
    if (tr) endTransaction();
  }

  void LGFXBase::setClipRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
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

  void LGFXBase::getClipRect(std::int32_t *x, std::int32_t *y, std::int32_t *w, std::int32_t *h)
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

  void LGFXBase::setScrollRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
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

  void LGFXBase::getScrollRect(std::int32_t *x, std::int32_t *y, std::int32_t *w, std::int32_t *h)
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

  void LGFXBase::drawFastVLine(std::int32_t x, std::int32_t y, std::int32_t h)
  {
    _adjust_abs(y, h);
    bool tr = !_transaction_count;
    if (tr) beginTransaction();
    writeFastVLine(x, y, h);
    if (tr) endTransaction();
  }

  void LGFXBase::writeFastVLine(std::int32_t x, std::int32_t y, std::int32_t h)
  {
    if (x < _clip_l || x > _clip_r) return;
    auto ct = _clip_t;
    if (y < ct) { h += y - ct; y = ct; }
    auto cb = _clip_b + 1 - y;
    if (h > cb) h = cb;
    if (h < 1) return;

    writeFillRect_impl(x, y, 1, h);
  }

  void LGFXBase::drawFastHLine(std::int32_t x, std::int32_t y, std::int32_t w)
  {
    _adjust_abs(x, w);
    bool tr = !_transaction_count;
    if (tr) beginTransaction();
    writeFastHLine(x, y, w);
    if (tr) endTransaction();
  }

  void LGFXBase::writeFastHLine(std::int32_t x, std::int32_t y, std::int32_t w)
  {
    if (y < _clip_t || y > _clip_b) return;
    auto cl = _clip_l;
    if (x < cl) { w += x - cl; x = cl; }
    auto cr = _clip_r + 1 - x;
    if (w > cr) w = cr;
    if (w < 1) return;

    writeFillRect_impl(x, y, w, 1);
  }

  void LGFXBase::fillRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
  {
    _adjust_abs(x, w);
    _adjust_abs(y, h);
    bool tr = !_transaction_count;
    if (tr) beginTransaction();
    writeFillRect(x, y, w, h);
    if (tr) endTransaction();
  }

  void LGFXBase::writeFillRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
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


  void LGFXBase::drawRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
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

  void LGFXBase::drawCircle(std::int32_t x, std::int32_t y, std::int32_t r)
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

  void LGFXBase::drawCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t cornername)
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

  void LGFXBase::fillCircle(std::int32_t x, std::int32_t y, std::int32_t r) {
    startWrite();
    writeFastHLine(x - r, y, (r << 1) + 1);
    fillCircleHelper(x, y, r, 3, 0);
    endWrite();
  }

  void LGFXBase::fillCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t corners, std::int32_t delta)
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

  void LGFXBase::drawEllipse(std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry)
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

    std::int32_t xt, yt, s, i;
    std::int32_t rx2 = rx * rx;
    std::int32_t ry2 = ry * ry;

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

  void LGFXBase::fillEllipse(std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry)
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

    std::int32_t xt, yt, i;
    std::int32_t rx2 = rx * rx;
    std::int32_t ry2 = ry * ry;
    std::int32_t s;

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

  void LGFXBase::drawRoundRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r)
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

  void LGFXBase::fillRoundRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r)
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

  void LGFXBase::drawLine(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1)
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

  void LGFXBase::drawTriangle(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2)
  {
    startWrite();
    drawLine(x0, y0, x1, y1);
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x0, y0);
    endWrite();
  }

  void LGFXBase::fillTriangle(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2)
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

  void LGFXBase::drawBezier( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2)
  {
    std::int32_t x = x0 - x1, y = y0 - y1;
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
      drawBezierHelper(x0, y0, x, floor(r + 0.5), x, y);
      r = (y1 - y2) * (t - x2) / (x1 - x2) + y2;
      x0 = x1 = x; y0 = y; y1 = floor(r + 0.5);
    }
    if ((y0 - y1) * (y2 - y1) > 0) {
      t = y0 - 2 * y1 + y2; t = (y0 - y1) / t;
      r = (1 - t) * ((1 - t) * x0 + 2.0 * t * x1) + t * t * x2;
      t = (y0 * y2 - y1 * y1) * t / (y0 - y1);
      x = floor(r + 0.5); y = floor(t + 0.5);
      r = (x1 - x0) * (t - y0) / (y1 - y0) + x0;
      drawBezierHelper(x0, y0, floor(r + 0.5), y, x, y);
      r = (x1 - x2) * (t - y2) / (y1 - y2) + x2;
      x0 = x; x1 = floor(r + 0.5); y0 = y1 = y;
    }
    drawBezierHelper(x0, y0, x1, y1, x2, y2);

    endWrite();
  }

  void LGFXBase::drawBezierHelper( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2)
  {
    // Check if coordinates are sequential (replaces assert)
    if (((x2 >= x1 && x1 >= x0) || (x2 <= x1 && x1 <= x0))
        && ((y2 >= y1 && y1 >= y0) || (y2 <= y1 && y1 <= y0)))
    {
      // Coordinates are sequential
      std::int32_t sx = x2 - x1, sy = y2 - y1;
      std::int32_t xx = x0 - x1, yy = y0 - y1, xy;
      float dx, dy, err, cur = xx * sy - yy * sx;

      if (sx * (std::int32_t)sx + sy * (std::int32_t)sy > xx * xx + yy * yy) {
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

  void LGFXBase::drawBezier( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, std::int32_t x3, std::int32_t y3)
  {
    std::int32_t w = x0-x1;
    std::int32_t h = y0-y1;
    std::int32_t len = w*w+h*h;
    w = x1-x2;
    h = y1-y2;
    std::int32_t len2 = w*w+h*h;
    if (len < len2) len = len2;
    w = x2-x3;
    h = y2-y3;
    len2 = w*w+h*h;
    if (len < len2) len = len2;
    len = (std::int32_t)round(sqrt(len)) >> 2;

    float fx0 = x0;
    float fy0 = y0;
    float fx1 = x1;
    float fy1 = y1;
    float fx2 = x2;
    float fy2 = y2;
    float fx3 = x3;
    float fy3 = y3;

    std::int32_t i = 0;
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
      x1 = round( fx0 * f0 + fx1 * f1 + fx2 * f2 + fx3 * f3);
      y1 = round( fy0 * f0 + fy1 * f1 + fy2 * f2 + fy3 * f3);
      if (x0 != x1 || y0 != y1) {
        drawLine(x0, y0, x1, y1);
//drawCircle(x1, y1, 3);
        x0 = x1;
        y0 = y1;
      }
      x2 = round( fx0 * f3 + fx1 * f2 + fx2 * f1 + fx3 * f0);
      y2 = round( fy0 * f3 + fy1 * f2 + fy2 * f1 + fy3 * f0);
      if (x3 != x2 || y3 != y2) {
        drawLine(x3, y3, x2, y2);
//drawCircle(x2, y2, 3);
        x3 = x2;
        y3 = y2;
      }
    } while (++i <= len);
    endWrite();
  }

  void LGFXBase::draw_gradient_line( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, uint32_t colorstart, uint32_t colorend )
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

    std::int32_t dx = x1 - x0;
    std::int32_t err = dx >> 1;
    std::int32_t dy = abs(y1 - y0);
    std::int32_t ystep = (y0 < y1) ? 1 : -1;

    std::int32_t r = (colorstart >> 16)&0xFF;
    std::int32_t g = (colorstart >> 8 )&0xFF;
    std::int32_t b = (colorstart      )&0xFF;

    std::int32_t diff_r = ((colorend >> 16)&0xFF) - r;
    std::int32_t diff_g = ((colorend >> 8 )&0xFF) - g;
    std::int32_t diff_b = ((colorend      )&0xFF) - b;

    startWrite();
    for (std::int32_t x = x0; x <= x1; x++) {
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

  void LGFXBase::drawArc(std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float start, float end)
  {
    if (r0 < r1) std::swap(r0, r1);
    if (r0 < 1) r0 = 1;
    if (r1 < 1) r1 = 1;

    bool equal = fabsf(start - end) < std::numeric_limits<float>::epsilon();
    start = fmodf(start, 360);
    end = fmodf(end, 360);
    if (start < 0) start += 360.0;
    if (end < 0) end += 360.0;

    startWrite();
    fill_arc_helper(x, y, r0, r1, start, start);
    fill_arc_helper(x, y, r0, r1, end  , end);
    if (!equal && (fabsf(start - end) <= 0.0001)) { start = .0; end = 360.0; }
    fill_arc_helper(x, y, r0, r0, start, end);
    fill_arc_helper(x, y, r1, r1, start, end);
    endWrite();
  }

  void LGFXBase::fillArc(std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float start, float end)
  {
    if (r0 < r1) std::swap(r0, r1);
    if (r0 < 1) r0 = 1;
    if (r1 < 1) r1 = 1;

    bool equal = fabsf(start - end) < std::numeric_limits<float>::epsilon();
    start = fmodf(start, 360);
    end = fmodf(end, 360);
    if (start < 0) start += 360.0;
    if (end < 0) end += 360.0;
    if (!equal && (fabsf(start - end) <= 0.0001)) { start = .0; end = 360.0; }

    startWrite();
    fill_arc_helper(x, y, r0, r1, start, end);
    endWrite();
  }

  void LGFXBase::fill_arc_helper(std::int32_t cx, std::int32_t cy, std::int32_t oradius, std::int32_t iradius, float start, float end)
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

  void LGFXBase::draw_bitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor)
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

  void LGFXBase::draw_xbitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor)
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

  void LGFXBase::push_image(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t *param, bool use_dma)
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

  bool LGFXBase::pushImageRotateZoom(std::int32_t dst_x, std::int32_t dst_y, const void* data, std::int32_t src_x, std::int32_t src_y, std::int32_t w, std::int32_t h, float angle, float zoom_x, float zoom_y, std::uint32_t transparent, const std::uint8_t bits, const bgr888_t* palette)
  {
    if (nullptr == data) return false;
    if (zoom_x == 0.0 || zoom_y == 0.0) return true;
    pixelcopy_t pc(data, getColorDepth(), (color_depth_t)bits, hasPalette(), palette, transparent );
    push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, w, h, angle, zoom_x, zoom_y, &pc);
    return true;
  }

  void LGFXBase::push_image_rotate_zoom(std::int32_t dst_x, std::int32_t dst_y, std::int32_t src_x, std::int32_t src_y, std::int32_t w, std::int32_t h, float angle, float zoom_x, float zoom_y, pixelcopy_t *param)
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
//    if (cos_x == 0) cos_x = 1;
    cos_x = -cos_x;

    std::int32_t sin_y = round(sin_f / zoom_y);
    param->src_y32_add = sin_y;
    std::int32_t cos_y = round(cos_f / zoom_y);
    std::int32_t ystart = sin_y * xt + cos_y * yt + (src_y << FP_SCALE) + (1 << (FP_SCALE - 1));
    std::int32_t scale_h = h << FP_SCALE;
    std::int32_t ys1 = (sin_y < 0 ?   - scale_h :   1) - sin_y;
    std::int32_t ys2 = (sin_y < 0 ? 0 : (1 - scale_h)) - sin_y;
//    if (sin_y == 0) sin_y = 1;
    sin_y = -sin_y;

    std::int32_t cl = _clip_l;
    std::int32_t cr = _clip_r + 1;

    startWrite();
    do {
      std::int32_t left = cl;
      std::int32_t right = cr;
      xstart += sin_x;
      if (cos_x != 0)
      {
        std::int32_t tmp = (xstart + xs1) / cos_x; if (left  < tmp) left  = tmp;
                     tmp = (xstart + xs2) / cos_x; if (right > tmp) right = tmp;
      }
      ystart += cos_y;
      if (sin_y != 0)
      {
        std::int32_t tmp = (ystart + ys1) / sin_y; if (left  < tmp) left  = tmp;
                     tmp = (ystart + ys2) / sin_y; if (right > tmp) right = tmp;
      }
      if (left < right) {
        param->src_x32 = xstart - left * cos_x;
        if (param->src_x < param->src_width) {
          param->src_y32 = ystart - left * sin_y;
          if (param->src_y < h) {
            pushImage_impl(left, min_y, right - left, 1, param, true);
          }
        }
      }
    } while (++min_y != max_y);
    endWrite();
  }

  void LGFXBase::scroll(std::int_fast16_t dx, std::int_fast16_t dy)
  {
    setColor(_base_rgb888);
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

  void LGFXBase::copyRect(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y)
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

  struct paint_point_t { std::int32_t lx,rx,y,oy; };

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
  void LGFXBase::paint(std::int32_t x, std::int32_t y) {
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
    bool* linebufs[3] = { new bool[w], new bool[w], new bool[w] };
    std::int32_t bufY[3] = {-2, -2, -2};  // 3 line buffer (default: out of range.)
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
}

