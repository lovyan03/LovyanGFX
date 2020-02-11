#ifndef LGFX_SPRITE_HPP_
#define LGFX_SPRITE_HPP_

#include <algorithm>

#include "LovyanGFX.hpp"

namespace lgfx
{
  class LGFXSpriteBase : public LovyanGFX
  {
  public:

    virtual void setPsram( bool enabled ) {};

    LGFXSpriteBase(LovyanGFX* parent)
    : LovyanGFX()
    , _parent(parent)
    , _img  (nullptr)
    , _palette(nullptr)
    , _bitwidth(0)
    , _xptr (0)
    , _yptr (0)
    , _xs   (0)
    , _xe   (0)
    , _ys   (0)
    , _ye   (0)
    , _index(0)
    {
      _read_depth = _write_depth;
      _has_transaction = false;
      _transaction_count = 0xFFFF;
    }

    inline void* buffer() { return _img; }
    uint32_t bufferLength(void) const { return _bitwidth * _height * _write_depth.bits >> 3; }

    LGFXSpriteBase()
    : LGFXSpriteBase(nullptr)
    {}

    virtual ~LGFXSpriteBase() {
      deleteSprite();
    }

    void* createSprite(int32_t w, int32_t h)
    {
      if (w < 1 || h < 1) return nullptr;
      if (_img) deleteSprite();
      _bitwidth = (w + _write_depth.x_mask) & (~(uint32_t)_write_depth.x_mask);
      _img = (uint8_t*)_mem_alloc((h * _bitwidth * _write_depth.bits >> 3) + 1);
      if (!_img) return nullptr;

      _sw = _width = w;
      _xe = w - 1;
      _xpivot = w >> 1;
      _sh = _height = h;
      _ye = h - 1;
      _ypivot = h >> 1;
      _rotation = 0;
      _index = _sx = _sy = _xs = _ys = _xptr = _yptr = 0;

      clear();
      return _img;
    }

    bool createPalette(void)
    {
      if (_write_depth.depth > 8) return false;

      if (_palette != nullptr) { _mem_free(_palette); _palette = nullptr; }

      size_t palettes = 1 << _write_depth.bits;
      _palette = (swap888_t*)_mem_alloc(sizeof(swap888_t) * palettes);
      if (!_palette) {
        return false;
      }
/*
if (_write_depth.depth == palette_8bit) {
for (uint32_t i = 0; i < palettes; i++) {
  _palette[i] = *(rgb332_t*)&i;
}
} else
//*/
      { // create grayscale palette
        uint32_t k;
        switch (_write_depth.depth) {
        case 8: k = 0x010101; break;
        case 4: k = 0x111111; break;
        case 2: k = 0x555555; break;
        case 1: k = 0xFFFFFF; break;
        default: k = 1; break;
        }
        for (uint32_t i = 0; i < palettes; i++) {
          _palette[i] = i * k;
        }
      }
      return true;
    }

    void deleteSprite(void)
    {
      if (_img     != nullptr) { _mem_free(_img    ); _img     = nullptr; }
      if (_palette != nullptr) { _mem_free(_palette); _palette = nullptr; }
    }

    void setBitmapColor(uint16_t fgcolor, uint16_t bgcolor)  // For 1bpp sprites
    {
      if (_palette) {
        _palette[0] = *(rgb565_t*)&bgcolor;
        _palette[1] = *(rgb565_t*)&fgcolor;
      }
    }

    void setPalette(size_t index, const swap888_t& rgb) {
      if (_palette) { _palette[index & ((1<<_write_depth.bits)-1)] = rgb; }
    }

    void setPalette(size_t index, uint8_t r, uint8_t g, uint8_t b) {
      if (_palette) { _palette[index & ((1<<_write_depth.bits)-1)].set(r, g, b); }
    }

    template<typename T>
    __attribute__ ((always_inline)) inline void fillSprite (const T& color) { fillRect(0, 0, _width, _height, color); }

    __attribute__ ((always_inline)) inline void pushSprite(int32_t x, int32_t y) { pushSprite(_parent, x, y); }
    void pushSprite(LovyanGFX* lgfx, int32_t x, int32_t y)
    {
      if (_palette) {
        switch (getColorDepth()) {
        case 8: lgfx->pushIndexImage(x, y, _width, _height, (palette8_t*)_img, _palette); return;
        case 4: lgfx->pushIndexImage(x, y, _width, _height, (palette4_t*)_img, _palette); return;
  //    case 2: lgfx->pushIndexImage(x, y, _width, _height, (palette2_t*)_img, _palette); return;
        case 1: lgfx->pushIndexImage(x, y, _width, _height, (palette1_t*)_img, _palette); return;
        default: return;
        }
      } else {
        switch (getColorDepth()) {
        case rgb888_3Byte: lgfx->pushImage(x, y, _width, _height, (swap888_t*)_img); return;
        case rgb666_3Byte: lgfx->pushImage(x, y, _width, _height, (swap666_t*)_img); return;
        case rgb565_2Byte: lgfx->pushImage(x, y, _width, _height, (swap565_t*)_img); return;
        case rgb332_1Byte: lgfx->pushImage(x, y, _width, _height, (rgb332_t* )_img); return;
        default: return;
        }
      }
    }

    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(int32_t x, int32_t y, const T& transparent) { pushSprite(_parent, x, y, transparent); }

    template<typename T>
    void pushSprite(LovyanGFX* lgfx, int32_t x, int32_t y, const T& transparent)
    {
      uint32_t transp = _write_depth.convert(transparent);
      if (_palette) {
        switch (getColorDepth()) {
        case 8: lgfx->pushIndexImage(x, y, _width, _height, (rgb332_t  *)_img, _palette, transp); return;
        case 4: lgfx->pushIndexImage(x, y, _width, _height, (palette4_t*)_img, _palette, transp); return;
  //    case 2: lgfx->pushIndexImage(x, y, _width, _height, (palette2_t*)_img, _palette, transp); return;
        case 1: lgfx->pushIndexImage(x, y, _width, _height, (palette1_t*)_img, _palette, transp); return;
        }
      } else {
        switch (getColorDepth()) {
        case rgb888_3Byte: lgfx->pushImage(x, y, _width, _height, (swap888_t*)_img, transp); return;
        case rgb666_3Byte: lgfx->pushImage(x, y, _width, _height, (swap666_t*)_img, transp); return;
        case rgb565_2Byte: lgfx->pushImage(x, y, _width, _height, (swap565_t*)_img, transp); return;
        case rgb332_1Byte: lgfx->pushImage(x, y, _width, _height, (rgb332_t* )_img, transp); return;
        }
      }
    }


    __attribute__ ((always_inline)) inline void pushRotated_old(int16_t angle, int32_t transp = -1) { pushRotated_old(_parent, angle, transp); }

    bool pushRotated_old(LovyanGFX* lgfx, int16_t angle, int32_t transp = -1)
    {
      static constexpr int32_t FP_SCALE = 19;
      // Trig values for the rotation
      float radAngle = -angle * 0.0174532925; // Convert degrees to radians
      int32_t sinra = round(sin(radAngle) * (float)(1 << FP_SCALE));
      int32_t cosra = round(cos(radAngle) * (float)(1 << FP_SCALE));

      // Bounding box parameters
      int32_t min_x;
      int32_t max_x;
      min_x = max_x = -_xpivot * cosra - _ypivot * sinra;

      int32_t min_y;
      int32_t max_y;
      min_y = max_y = _xpivot * sinra - _ypivot * cosra;

      // Get the bounding box of this rotated source Sprite
      //getRotatedBounds(&min_x, &min_y, &max_x, &max_y);
      {
        int32_t w = width() - _xpivot; // w is now right edge coordinate relative to xp
        int32_t h = height() - _ypivot; // h is now bottom edge coordinate relative to yp

        int32_t tmp =  w * cosra - _ypivot * sinra;
        if (     tmp < min_x) min_x = tmp;
        else if (tmp > max_x) max_x = tmp;

        tmp = h * sinra + w * cosra;
        if (     tmp < min_x) min_x = tmp;
        else if (tmp > max_x) max_x = tmp;

        tmp = h * sinra - _xpivot * cosra;
        if (     tmp < min_x) min_x = tmp;
        else if (tmp > max_x) max_x = tmp;


        tmp = -w * sinra - _ypivot * cosra;
        if (     tmp < min_y) min_y = tmp;
        else if (tmp > max_y) max_y = tmp;

        tmp = h * cosra - w * sinra;
        if (     tmp < min_y) min_y = tmp;
        else if (tmp > max_y) max_y = tmp;

        tmp = h * cosra + _xpivot * sinra;
        if (     tmp < min_y) min_y = tmp;
        else if (tmp > max_y) max_y = tmp;
      }

      // Move bounding box so source Sprite pivot coincides with destination Sprite pivot
      max_x = ((max_x + (1 << (FP_SCALE - 1))) >> FP_SCALE) + lgfx->getPivotX() + 1;
      if (max_x < 0) return true;
      if (max_x > lgfx->width()) max_x = lgfx->width();

      min_x = ((min_x + (1 << (FP_SCALE - 1))) >> FP_SCALE) + lgfx->getPivotX();
      if (min_x > lgfx->width()) return true;
      if (min_x < 0) min_x = 0;

      max_y = ((max_y + (1 << (FP_SCALE - 1))) >> FP_SCALE) + lgfx->getPivotY() + 1;
      if (max_y < 0) return true;
      if (max_y > lgfx->height()) max_y = lgfx->height();

      min_y = ((min_y + (1 << (FP_SCALE - 1))) >> FP_SCALE) + lgfx->getPivotY();
      if (min_y > lgfx->height()) return true;
      if (min_y < 0) min_y = 0;

      // Scan destination bounding box and fetch transformed pixels from source Sprite

      int32_t xe = width() << FP_SCALE;
      int32_t ye = height() << FP_SCALE;

      swap565_t bufs[2][max_x - min_x];
      bool flip = false;
      int32_t yt = min_y - lgfx->getPivotY();
      lgfx->startWrite();

      for (int32_t y = min_y; y < max_y; y++, yt++)
      {
        size_t bufidx = 0;
        int32_t x = min_x;
        int32_t xs = (cosra * (x - lgfx->getPivotX()) - (sinra * yt - (_xpivot << FP_SCALE)) + (1 << (FP_SCALE - 1)) - 1);
        int32_t ys = (sinra * (x - lgfx->getPivotX()) + (cosra * yt + (_ypivot << FP_SCALE)) + (1 << (FP_SCALE - 1)) - 1);
        for (; x < max_x; x++, xs += cosra, ys += sinra)
        {
          if ((xs >= 0 && xs < xe) // Do not calculate ys unless xs is in bounds
           && (ys >= 0 && ys < ye))
          {
            int32_t rp = __builtin_bswap16( readPixel(xs >> FP_SCALE, ys >> FP_SCALE));
            if (rp != transp) {
              bufs[flip][bufidx++] = rp;
              continue;
            }
          }
          if (bufidx) {
            lgfx->setWindow(x - bufidx, y, x - 1, y);
            lgfx->writeBytesDMA((uint8_t*)bufs[flip], bufidx * 2);
            flip = !flip;
            bufidx = 0;
          }
        }
        if (bufidx) {
          lgfx->setWindow(x - bufidx, y, x - 1, y);
          lgfx->writeBytesDMA((uint8_t*)bufs[flip], bufidx * 2);
          flip = !flip;
        }
      }
      lgfx->endWrite();

      return true;
    }
//*/


    template<typename T> bool pushRotated(                 float angle, const T& transp) { return push_rotated(_parent, angle, _write_depth.convert(transp) & _write_depth.colormask); }
    template<typename T> bool pushRotated(LovyanGFX* lgfx, float angle, const T& transp) { return push_rotated(lgfx   , angle, _write_depth.convert(transp) & _write_depth.colormask); }
    bool pushRotated(                 float angle                 ) { return push_rotated(_parent, angle, -1); }
    bool pushRotated(LovyanGFX* lgfx, float angle                 ) { return push_rotated(lgfx   , angle, -1); }

/*
    bool push_rotated(LovyanGFX* dst, float angle, uint32_t transp)
    {
      if (_img == nullptr) return false;
      static constexpr int32_t FP_SCALE = 20;

      float radAngle = -angle * 0.0174532925; // Convert degrees to radians
      int32_t sinra = round(sin(radAngle) * (float)(1 << FP_SCALE));
      int32_t cosra = round(cos(radAngle) * (float)(1 << FP_SCALE));

      int32_t min_x;
      int32_t max_x;
      min_x = max_x = -_xpivot * cosra - _ypivot * sinra;

      int32_t min_y;
      int32_t max_y;
      min_y = max_y = _xpivot * sinra - _ypivot * cosra;

      {
        int32_t w = width() - _xpivot;
        int32_t h = height() - _ypivot;
        int32_t tmp;
        tmp = w * cosra - _ypivot * sinra; if (tmp < min_x) min_x = tmp; else if (tmp > max_x) max_x = tmp;
        tmp = h * sinra +       w * cosra; if (tmp < min_x) min_x = tmp; else if (tmp > max_x) max_x = tmp;
        tmp = h * sinra - _xpivot * cosra; if (tmp < min_x) min_x = tmp; else if (tmp > max_x) max_x = tmp;

        tmp = -w * sinra - _ypivot * cosra; if (tmp < min_y) min_y = tmp; else if (tmp > max_y) max_y = tmp;
        tmp =  h * cosra -       w * sinra; if (tmp < min_y) min_y = tmp; else if (tmp > max_y) max_y = tmp;
        tmp =  h * cosra + _xpivot * sinra; if (tmp < min_y) min_y = tmp; else if (tmp > max_y) max_y = tmp;
      }

      max_x = (max_x + (1 << (FP_SCALE - 1)) >> FP_SCALE) + dst->getPivotX() + 1;
      if (max_x < 0) return true;
      if (max_x > dst->width()) max_x = dst->width();

      min_x = (min_x + (1 << (FP_SCALE - 1)) >> FP_SCALE) + dst->getPivotX();
      if (min_x > dst->width()) return true;
      if (min_x < 0) min_x = 0;

      max_y = (max_y + (1 << (FP_SCALE - 1)) >> FP_SCALE) + dst->getPivotY() + 1;
      if (max_y < 0) return true;
      if (max_y > dst->height()) max_y = dst->height();

      min_y = (min_y + (1 << (FP_SCALE - 1)) >> FP_SCALE) + dst->getPivotY();
      if (min_y > dst->height()) return true;
      if (min_y < 0) min_y = 0;

      uint32_t xe = width() << FP_SCALE;
      uint32_t ye = height() << FP_SCALE;

      auto dst_depth = dst->getWriteDepth();
//    auto fp_convert = get_convert_fp(dst_depth.depth, _palette ? rgb888_3Byte : _read_depth.depth);

//      auto fp_ptp = dst->get_ptp_fp(dst_depth.depth, dst_depth.depth_palette ? rgb888_3Byte : _read_depth.depth);
pixelcopy_param_t param;
param.src_palette = _palette;
void(*fp_ptp)(void*&, const void*&, int32_t, pixelcopy_param_t*);
if (_palette) {
  switch (_read_depth.depth) {
  case 8: fp_ptp = dst->get_palette_to_pixel_fp<rgb332_t, swap888_t>(); break;
  case 4: fp_ptp = dst->get_palette_to_pixel_fp<palette4_t, swap888_t>(); break;
  case 1: fp_ptp = dst->get_palette_to_pixel_fp<palette1_t, swap888_t>(); break;
  default: return false;
  }
} else {
  switch (_read_depth.depth) {
  case 24: fp_ptp = dst->get_pixel_to_pixel_fp<swap888_t>(); break;
  case 16: fp_ptp = dst->get_pixel_to_pixel_fp<swap565_t>(); break;
  case  8: fp_ptp = dst->get_pixel_to_pixel_fp<rgb332_t> (); break;
  default: return false;
  }
}
//      auto fp_ptp = dst->get_ptp_fp(dst_depth.depth, dst_depth.depth_palette ? rgb888_3Byte : _read_depth.depth);

      uint32_t dst_step = dst_depth.bytes ? dst_depth.bytes : 1;
      uint32_t src_bytes = _read_depth.bytes;
      uint32_t src_bits = _read_depth.bits;
      uint32_t colormask = _write_depth.colormask;

      uint8_t bufs[2][((max_x - min_x) * dst_step) + 3];
uint8_t rawbuf[(((max_x - min_x) * src_bits + 7) >> 3) + 3];
memset(rawbuf, 0, (((max_x - min_x) * src_bits + 7) >> 3) + 3);

      bool flip = false;
      int32_t yt = min_y - dst->getPivotY();
      dst->startWrite();
      for (int32_t y = min_y; y < max_y; y++, yt++) {
        size_t bufidx = 0;
        int32_t x = min_x;
        uint32_t xs = (cosra * (x - dst->getPivotX()) - (sinra * yt - (_xpivot << FP_SCALE)) + (1 << (FP_SCALE - 1)) - 1);
        uint32_t ys = (sinra * (x - dst->getPivotX()) + (cosra * yt + (_ypivot << FP_SCALE)) + (1 << (FP_SCALE - 1)) - 1);

        while ((xs >= xe || ys >= ye) && x < max_x) { x++; xs += cosra; ys += sinra; }
        if (x == max_x) continue;

        if (-1 == transp) {
          do {
            uint32_t idx = ((xs >> FP_SCALE) + (ys >> FP_SCALE) * _bitwidth) * src_bits;
            uint32_t rawpixel = ((*(uint32_t*)&_img[idx >> 3]) >> (-(idx+src_bits) & 7)) & colormask;
idx = bufidx * src_bits;
*(uint32_t*)&rawbuf[idx >> 3] |= rawpixel << (-(idx+src_bits) & 7);
//              *(uint32_t*)&bufs[flip][dst_step * bufidx] = fp_convert((*(uint32_t*)&_palette[rawpixel]) & 0xFFFFFF);
            ++bufidx;
          } while (++x < max_x && (xs += cosra) < xe && (ys += sinra) < ye);
        } else {
          do {
            uint32_t idx = ((xs >> FP_SCALE) + (ys >> FP_SCALE) * _bitwidth) * src_bits;
            uint32_t rawpixel = ((*(uint32_t*)&_img[idx >> 3]) >> (-(idx+src_bits) & 7)) & colormask;
            if (rawpixel != transp) {
idx = bufidx * src_bits;
*(uint32_t*)&rawbuf[idx >> 3] |= rawpixel << (-(idx+src_bits) & 7);
//                *(uint32_t*)&bufs[flip][dst_step * bufidx] = fp_convert((*(uint32_t*)&_palette[rawpixel]) & 0xFFFFFF);
              ++bufidx;
            } else
            if (bufidx) {
              dst->setWindow(x - bufidx, y, x - 1, y);
param.src_offset = 0;
void* d = &bufs[flip][0];
const void* s = rawbuf;
(*fp_ptp)(d, s, bufidx, &param);
                dst->writeBytesDMA((uint8_t*)bufs[flip], bufidx * dst_step);
memset(rawbuf, 0, bufidx * dst_step);
              flip = !flip;
              bufidx = 0;
            }
          } while (++x < max_x && (xs += cosra) < xe && (ys += sinra) < ye);
        }

        if (bufidx) {
          dst->setWindow(x - bufidx, y, x - 1, y);
param.src_offset = 0;
void* d = &bufs[flip][0];
const void* s = rawbuf;
(*fp_ptp)(d, s, bufidx, &param);
          dst->writeBytesDMA((uint8_t*)bufs[flip], bufidx * dst_step);
memset(rawbuf, 0, bufidx * dst_step);
          flip = !flip;
        }
      }
      dst->endWrite();

      return true;
    }

/*/
    bool push_rotated(LovyanGFX* dst, float angle, uint32_t transp)
    {
      if (_img == nullptr) return false;
      static constexpr uint32_t FP_SCALE = 20;

      float radAngle = -angle * 0.0174532925; // Convert degrees to radians
      int32_t sinra = round(sin(radAngle) * (float)(1 << FP_SCALE));
      int32_t cosra = round(cos(radAngle) * (float)(1 << FP_SCALE));

      int32_t min_x;
      int32_t max_x;
      min_x = max_x = -_xpivot * cosra - _ypivot * sinra;

      int32_t min_y;
      int32_t max_y;
      min_y = max_y = _xpivot * sinra - _ypivot * cosra;

      {
        int32_t w = width() - _xpivot;
        int32_t h = height() - _ypivot;
        int32_t tmp;
        tmp = w * cosra - _ypivot * sinra; if (tmp < min_x) min_x = tmp; else if (tmp > max_x) max_x = tmp;
        tmp = h * sinra +       w * cosra; if (tmp < min_x) min_x = tmp; else if (tmp > max_x) max_x = tmp;
        tmp = h * sinra - _xpivot * cosra; if (tmp < min_x) min_x = tmp; else if (tmp > max_x) max_x = tmp;

        tmp = -w * sinra - _ypivot * cosra; if (tmp < min_y) min_y = tmp; else if (tmp > max_y) max_y = tmp;
        tmp =  h * cosra -       w * sinra; if (tmp < min_y) min_y = tmp; else if (tmp > max_y) max_y = tmp;
        tmp =  h * cosra + _xpivot * sinra; if (tmp < min_y) min_y = tmp; else if (tmp > max_y) max_y = tmp;
      }

      max_x = ((max_x + (1 << (FP_SCALE - 1))) >> FP_SCALE) + dst->getPivotX() + 1;
      if (max_x < 0) return true;
      if (max_x > dst->width()) max_x = dst->width();

      min_x = ((min_x + (1 << (FP_SCALE - 1))) >> FP_SCALE) + dst->getPivotX();
      if (min_x > dst->width()) return true;
      if (min_x < 0) min_x = 0;

      max_y = ((max_y + (1 << (FP_SCALE - 1))) >> FP_SCALE) + dst->getPivotY() + 1;
      if (max_y < 0) return true;
      if (max_y > dst->height()) max_y = dst->height();

      min_y = ((min_y + (1 << (FP_SCALE - 1))) >> FP_SCALE) + dst->getPivotY();
      if (min_y > dst->height()) return true;
      if (min_y < 0) min_y = 0;

      uint32_t xe = width() << FP_SCALE;
      uint32_t ye = height() << FP_SCALE;

      auto dst_depth = dst->getWriteDepth();
      auto palette = _palette;
      auto fp_convert = palette ? get_conv_palette_to_pixel_fp(dst_depth.depth)
                                : get_conv_pixel_to_pixel_fp(dst_depth.depth, _read_depth.depth);

      uint32_t src_bytes = _read_depth.bytes;
      uint32_t src_bits = _read_depth.bits;
      uint32_t colormask = (1 << src_bits) - 1; //_read_depth.colormask;
      uint32_t dst_step = dst_depth.bytes ? dst_depth.bytes : 1;
      uint8_t bufs[2][((max_x - min_x) * dst_step) + 3];
//_palette[1].b += 192;
//_palette[1].g += 128;
//_palette[1].r += 96;
      bool flip = false;
      int32_t yt = min_y - dst->getPivotY();
      dst->startWrite();
      for (int32_t y = min_y; y < max_y; y++, yt++) {
        size_t bufidx = 0;
        int32_t x = min_x;
        uint32_t xs = (cosra * (x - dst->getPivotX()) - (sinra * yt - (_xpivot << FP_SCALE)) + (1 << (FP_SCALE - 1)) - 1);
        uint32_t ys = (sinra * (x - dst->getPivotX()) + (cosra * yt + (_ypivot << FP_SCALE)) + (1 << (FP_SCALE - 1)) - 1);

        while ((xs >= xe || ys >= ye) && x < max_x) { x++; xs += cosra; ys += sinra; }
        if (x == max_x) continue;

        if (src_bytes == 0) {
          if (-1 == transp) {
            do {
              uint32_t idx = ((xs >> FP_SCALE) + (ys >> FP_SCALE) * _bitwidth) * src_bits;
              uint32_t rawpixel = ((*(uint32_t*)&_img[idx >> 3]) >> (-(idx+src_bits) & 7)) & colormask;
              *(uint32_t*)&bufs[flip][dst_step * bufidx] = fp_convert(rawpixel, palette);
              ++bufidx;
            } while (++x < max_x && (xs += cosra) < xe && (ys += sinra) < ye);
          } else {
            do {
              uint32_t idx = ((xs >> FP_SCALE) + (ys >> FP_SCALE) * _bitwidth) * src_bits;
              uint32_t rawpixel = ((*(uint32_t*)&_img[idx >> 3]) >> (-(idx+src_bits) & 7)) & colormask;
              if (rawpixel == transp) {
                if (bufidx) {
                  dst->setWindow(x - bufidx, y, x - 1, y);
                  dst->writeBytesDMA((uint8_t*)bufs[flip], bufidx * dst_step);
                  flip = !flip;
                  bufidx = 0;
                }
              } else {
                *(uint32_t*)&bufs[flip][dst_step * bufidx] = fp_convert(rawpixel, palette);
                ++bufidx;
              }
            } while (++x < max_x && (xs += cosra) < xe && (ys += sinra) < ye);
          }
        } else {
          if (-1 == transp) {
            do {
              uint32_t idx = ((xs >> FP_SCALE) + (ys >> FP_SCALE) * _bitwidth) * src_bytes;
              uint32_t rawpixel = (*(uint32_t*)&_img[idx]) & colormask;
              *(uint32_t*)&bufs[flip][dst_step * bufidx] = fp_convert(rawpixel, palette);
              ++bufidx;
            } while (++x < max_x && (xs += cosra) < xe && (ys += sinra) < ye);
          } else {
            do {
              uint32_t idx = ((xs >> FP_SCALE) + (ys >> FP_SCALE) * _bitwidth) * src_bytes;
              uint32_t rawpixel = (*(uint32_t*)&_img[idx]) & colormask;
              if (rawpixel == transp) {
                if (bufidx) {
                  dst->setWindow(x - bufidx, y, x - 1, y);
                  dst->writeBytesDMA((uint8_t*)bufs[flip], bufidx * dst_step);
                  flip = !flip;
                  bufidx = 0;
                }
              } else {
                *(uint32_t*)&bufs[flip][dst_step * bufidx] = fp_convert(rawpixel, palette);
                ++bufidx;
              }
            } while (++x < max_x && (xs += cosra) < xe && (ys += sinra) < ye);
          }
        }
        if (bufidx) {
          dst->setWindow(x - bufidx, y, max_x - 1, y);
          dst->writeBytesDMA((uint8_t*)bufs[flip], bufidx * dst_step);
          flip = !flip;
        }
      }
      dst->endWrite();

      return true;
    }
//*/

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    LovyanGFX* _parent;
    union {
      uint8_t* _img;
      uint16_t* _img16;
      swap888_t* _img24;
    };
    swap888_t* _palette;
    int32_t _bitwidth;
    int32_t _xptr;
    int32_t _yptr;
    int32_t _xs;
    int32_t _xe;
    int32_t _ys;
    int32_t _ye;
    uint32_t _index;
    bool _disable_memcpy = false; // disable PSRAM to PSRAM memcpy flg.

    virtual void* _mem_alloc(uint32_t bytes) = 0;
    virtual void _mem_free(void* buf) = 0;

    void set_window(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      if (xs > xe) swap_coord(xs, xe);
      if (ys > ye) swap_coord(ys, ye);

      if ((xe < 0) || (ye < 0) || (xs >= _width) || (ys >= _height))
      {
        _xptr = _xs = _xe = 0;
        _yptr = _ys = _ye = _height;
      } else {
        _xptr = _xs = (xs < 0) ? 0 : xs;
        _yptr = _ys = (ys < 0) ? 0 : ys;
        _xe = (xe >= _width ) ? _width  - 1 : xe;
        _ye = (ye >= _height) ? _height - 1 : ye;
      }
      _index = xs + ys * _bitwidth;
    }

    void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
    }
    void readWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
    }

    void* setColorDepth_impl(color_depth_t depth) override
    {
      _write_depth.setColorDepth(depth);
      _read_depth = _write_depth;

      if (_img == nullptr) return nullptr;
      deleteSprite();
      return createSprite(_width, _height);
    }

    void setRotation_impl(uint8_t r) override
    {
      r = r & 7;
      if ((_rotation&1) != (r&1)) {
        uint32_t tmp = _width;
        _width = _height;
        _height = tmp;
      }
      _rotation = r;
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      if (_write_depth.bytes == 2) {
        _img16[x + y * _bitwidth] = _color.rawL;
      } else if (_write_depth.bytes == 1) {
        _img[x + y * _bitwidth] = _color.raw0;
      } else if (_write_depth.bytes == 3) {
        _img24[x + y * _bitwidth] = *(swap888_t*)&_color;
      } else {
        uint8_t mask = (uint8_t)(~(0xFF >> _write_depth.bits)) >> (x * _write_depth.bits & 7);
        auto dst = &_img[(x + y * _bitwidth) * _write_depth.bits >> 3];
        *dst = (*dst & ~mask) | (_color.raw0 & mask);
      }
    }

    void fillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
/*
setWindow(x,y,x+w-1,y+h-1);
writeColor_impl(w*h);
return;
//*/
      if (_write_depth.bytes == 0) {
        size_t d = _bitwidth * _write_depth.bits >> 3;
        uint8_t* dst = &_img[(x + y * _bitwidth) * _write_depth.bits >> 3];
        size_t len = ((x + w) * _write_depth.bits >> 3) - (x * _write_depth.bits >> 3);
        uint8_t mask = 0xFF >> (x * _write_depth.bits & 7);
        uint8_t c = _color.raw0;
        if (!len) {
          mask ^= mask >> (w * _write_depth.bits);
          c &= mask;
        } else {
          if (mask != 0xFF) {
            auto dst2 = dst;
            auto h2 = h;
            c &= mask;
            do { *dst2 = (*dst2 & ~mask) | c; dst2 += d; } while (--h2);
            dst++; len--;
          }
          mask = ~(0xFF>>((x+w) * _write_depth.bits & 7));
          if (mask == 0xFF) len--;
          c = _color.raw0;
          if (len) {
            auto dst2 = dst;
            auto h2 = h;
            do { memset(dst2, c, len); dst2 += d; } while (--h2);
            dst += len;
          }
          if (mask == 0xFF) return;
          c &= mask;
        }
        do { *dst = (*dst & ~mask) | c; dst += d; } while (--h);
      } else
      if (w == 1) {
        uint8_t* dst = &_img[(x + y * _bitwidth) * _write_depth.bytes];
        if (_write_depth.bytes == 2) {
          do { *(uint16_t*)dst = *(uint16_t*)&_color;   dst += _bitwidth << 1; } while (--h);
        } else if (_write_depth.bytes == 1) {
          do {             *dst = _color.raw0;          dst += _bitwidth;      } while (--h);
        } else if (_write_depth.bytes == 3) {
          do { *(swap888_t*)dst = *(swap888_t*)&_color; dst += _bitwidth * 3;  } while (--h);
        } else {
        }
      } else {
        uint8_t* dst = &_img[(x + y * _bitwidth) * _write_depth.bytes];
        if (_write_depth.bytes == 1 || (_color.raw0 == _color.raw1 && (_write_depth.bytes == 2 || (_color.raw0 == _color.raw2)))) {
          if (x == 0 && w == _width) {
            memset(dst, _color.raw0, _width * h * _write_depth.bytes);
          } else {
            do {
              memset(dst, _color.raw0, w * _write_depth.bytes);
              dst += _width * _write_depth.bytes;
            } while (--h);
          }
        } else {
          size_t len = w * _write_depth.bytes;
          uint32_t down = _width * _write_depth.bytes;
          if (_disable_memcpy) {
            uint8_t linebuf[len];
            memset_multi(linebuf, _color.raw, _write_depth.bytes, w);
            do {
              memcpy(dst, linebuf, len);
              dst += down;
            } while (--h);
          } else {
            memset_multi(dst, _color.raw, _write_depth.bytes, w);
            while (--h) {
              memcpy(dst + down, dst, len);
              dst += down;
            }
          }
        }
      }
    }

    void writeColor_impl(int32_t length) override
    {
      if (0 >= length) return;
      if (_write_depth.bytes == 0) {
        uint8_t c = _color.raw0;
        //uint8_t addrshift = (4 - _write_depth.depth);
        uint8_t addrshift = 2 / _write_depth.bits + 1;
        int32_t linelength;
        do {
          uint8_t* dst = &_img[(_xptr + _yptr * _bitwidth) >> addrshift];
          linelength = std::min(_xe - _xptr + 1, length);
          size_t len = ((_xptr + linelength) >> addrshift) - (_xptr >> addrshift);
          uint8_t mask = 0xFF >> ((_xptr & _write_depth.x_mask) * _write_depth.bits);
          if (!len) {
            mask &= ~(mask >> (linelength * _write_depth.bits));
          } else {
            *dst = (*dst & ~mask) | (c & mask);
            if (len != 1) {
              memset(dst+1, c, len-1);
            }
            dst += len;
            mask = ~(0xFF>>(((_xptr + linelength) & _write_depth.x_mask) * _write_depth.bits));
          }
          *dst = (*dst & ~mask) | (c & mask);
          ptr_advance(linelength);
        } while (length -= linelength);

      } else
      if (_write_depth.bytes == 1 || (_color.raw0 == _color.raw1 && (_write_depth.bytes == 2 || (_color.raw0 == _color.raw2)))) {
        int32_t linelength;
        do {
          linelength = std::min(_xe - _xptr + 1, length);
          memset(&_img[_index * _write_depth.bytes], _color.raw0, linelength * _write_depth.bytes);
          ptr_advance(linelength);
        } while (length -= linelength);
      } else {
        uint8_t* dst = &_img[_index * _write_depth.bytes];
        if (_disable_memcpy) {
          int32_t buflen = std::min(_xe - _xs + 1, length);
          uint8_t linebuf[buflen * _write_depth.bytes];
          memset_multi(linebuf, _color.raw, _write_depth.bytes, buflen);
          uint32_t len;
          do  {
            len = std::min(length, _xe - _xptr + 1);
            memcpy(&_img[_index * _write_depth.bytes], linebuf, len * _write_depth.bytes);
            ptr_advance(len);
          } while (length -= len);
          return;
        }

        uint32_t advance = std::min(_xe - _xptr + 1, length);
        if (_xs != _xptr) {
          memset_multi(dst, _color.raw, _write_depth.bytes, advance);
          ptr_advance(advance);
          if (0 == (length -= advance)) return;
          dst = &_img[_index * _write_depth.bytes];
          advance = std::min(_xe - _xptr + 1, length);
        }
        memset_multi(dst, _color.raw, _write_depth.bytes, advance);
        ptr_advance(advance);
        if (0 == (length -= advance)) return;
        uint32_t down = _width * _write_depth.bytes;
        while (length > advance) {
          memcpy(dst+down, dst, advance * _write_depth.bytes);
          dst += down;
          ptr_advance(advance);
          length -= advance;
        }
        if (length) {
          memcpy(dst + down, dst, length * _write_depth.bytes);
          ptr_advance(length);
        }
      }
    }

    void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) override
    {
      if (_write_depth.bits < 8) {
// unimplemented
/*
        const uint32_t colormask = (1 << _write_depth.bits)-1;

        int32_t add = _bitwidth * _write_depth.bits;
        if (src_y < dst_y) add = -add;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        int32_t sx = (src_x + (src_y + pos) * _bitwidth) * _write_depth.bits;
        int32_t dx = (dst_x + (dst_y + pos) * _bitwidth) * _write_depth.bits;

        size_t len = (w * _write_depth.bits + 7) >> 3;
        uint8_t buf[len];
        for (int count = 0; count < h; count++) {
          uint8_t* src = &_img[sx >> 3];
          uint8_t* dst = &_img[dx >> 3];
          memcpy(buf, src, len);

          uint8_t offset = (-dx * bits) & 7;
          for (int i = 0; i < w; i++) {
            offset = (offset - bits) & 7;
            (((*(uint32_t*)&src[(dx+i)*bits >> 3]) >> offset) & colormask)

            dx + i * _write_depth.bits

          }

          memcpy(dst, buf, len);

          sx += add;
          dx += add;
        }

//*/
      } else {
        int32_t add = _bitwidth * _write_depth.bytes;
        if (src_y < dst_y) add = -add;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        uint8_t* src = &_img[(src_x + (src_y + pos) * _bitwidth) * _write_depth.bytes];
        uint8_t* dst = &_img[(dst_x + (dst_y + pos) * _bitwidth) * _write_depth.bytes];
        size_t len = w * _write_depth.bytes;
        if (_disable_memcpy) {
          uint8_t buf[len];
          for (int count = 0; count < h; count++) {
            memcpy(buf, src, len);
            memcpy(dst, buf, len);
            src += add;
            dst += add;
          }
        } else {
          for (int count = 0; count < h; count++) {
            memcpy(dst, src, len);
            src += add;
            dst += add;
          }
        }
      }
    }

    rgb565_t readPixel16_impl(int32_t x, int32_t y) override
    {
      _xptr = _xs = _xe = x;
      _yptr = _ys = _ye = y;
      _index = x + y * _bitwidth;

      const uint8_t *src = ptr_img();
      rgb565_t res;
      if (     _write_depth.depth == rgb332_1Byte) { res = *(( rgb332_t*)src); return res;}
      else if (_write_depth.depth == rgb565_2Byte) { res = *((swap565_t*)src); return res;}
//    else if (_write_depth.depth == rgb666_3Byte) { res = *((swap888_t*)src); return res;}
      else if (_write_depth.depth == rgb888_3Byte) { res = *((swap888_t*)src); return res;}

      return (bool)(*src & (0x80 >> (_index & 0x07)));
    }

    uint32_t readPixelRAW_impl(int32_t x, int32_t y) override
    {
      return _write_depth.colormask & (*(const uint32_t*)&_img[(x + y * _bitwidth) * _write_depth.bits >> 3]);
    }
/*
    template<class T>
    __attribute__ ((always_inline)) inline void readRectTemplate(int32_t x, int32_t y, int32_t w, int32_t h, T* buf)
    {
      if (     _color.bpp ==  8) { read_pixels< rgb332_t>(x, y ,w ,h ,buf); }
      else if (_color.bpp == 16) { read_pixels<swap565_t>(x, y ,w ,h ,buf); }
      else if (_color.bpp == 24) { read_pixels<swap888_t>(x, y ,w ,h ,buf); }
      else {

        uint8_t p = 0;
        const uint8_t *src = ptr_img();
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
*/
/*
    template <class TSrc>
    void writePixelsTemplate(const TSrc* src, uint32_t length)
    {
      if (!length) return;
      if (     _color.bpp ==  8) { write_pixels< rgb332_t>(src, length); }
      else if (_color.bpp == 16) { write_pixels<swap565_t>(src, length); }
      else if (_color.bpp == 24) { write_pixels<swap888_t>(src, length); }
      else {

        const uint8_t* s(static_cast<const uint8_t*>(src));
        uint8_t p = 0;
        uint8_t *dst = ptr_img();
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
*/
/*
    static void dupe_fill(uint8_t* dst, const uint8_t* src, size_t len, size_t size) {
      while (len > size) {
        memcpy(dst, src, size);
        dst += size;
        len -= size;
        size <<= 1;
      }
      if (len) {
        memcpy(dst, src, len);
      }
    }
//*/
    static void memset_multi(uint8_t* buf, uint32_t c, size_t size, size_t length) {
      size_t l = length;
      if (l & ~0xF) {
        while ((l >>= 1) & ~0xF);
        ++l;
      }
      length = (length - l) * size;
      size_t len = l * size;
      uint8_t* dst = buf;
      if (size == 2) {
        do { // 2byte speed tweak
          *(uint16_t*)dst = c; dst += 2;
        } while (--l);
      } else {
        size_t i = 0;
        l *= size;
        do {
          *dst++ = *(((uint8_t*)&c)+i);
          if (++i == size) i = 0;
        } while (--l);
      }
      if (!length) return;
      while (length > len) {
        memcpy(dst, buf, len);
        dst += len;
        length -= len;
        len <<= 1;
      }
      if (length) {
        memcpy(dst, buf, length);
      }
    }

    void read_pixels(void* dst, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) override
    {
      int32_t linelength;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        const void* src = ptr_img();
        fp_copy(dst, src, linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void read_bytes(uint8_t* dst, int32_t length) override
    {
      uint8_t b = _write_depth.bytes ? _write_depth.bytes : 1;
      length /= b;
      while (length) {
        int32_t linelength = std::min(_xe - _xptr + 1, length);
        memcpy(dst, ptr_img(), linelength * b);
        dst += linelength * b;
        ptr_advance(linelength);
        length -= linelength;
      }
    }

    void write_pixels(const void* src, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) override
    {
      int32_t linelength;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        void* dst = ptr_img();
        fp_copy(dst, src, linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void writeBytes_impl(const uint8_t* data, int32_t length) override
    {
      uint8_t b = _write_depth.bytes ? _write_depth.bytes : 1;
      length /= b;
      while (length) {
        int32_t linelength = std::min(_xe - _xptr + 1, length);
        memcpy(ptr_img(), data, linelength * b);
        data += linelength * b;
        ptr_advance(linelength);
        length -= linelength;
      }
    }

    inline bool ptr_advance(int32_t length = 1) {
      if ((_xptr += length) > _xe) {
        _xptr = _xs;
        if (++_yptr > _ye) {
          _yptr = _ys;
        }
        _index = _xptr + _yptr * _bitwidth;
        return true;
      }
      _index += length;
      return false;
    }

    inline uint8_t* ptr_img() {
      return &_img[_index * _write_depth.bits >> 3];
    }

  };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)

  class LGFXSprite : public LGFXSpriteBase
  {
  public:
    LGFXSprite(LovyanGFX* parent)
    : LGFXSpriteBase(parent)
    , _malloc_cap(MALLOC_CAP_8BIT)
    {}
    LGFXSprite() : LGFXSprite(nullptr) {}

    void setPsram( bool enabled ) override {
      if (enabled) _malloc_cap |= MALLOC_CAP_SPIRAM;
      else         _malloc_cap &= ~MALLOC_CAP_SPIRAM;
    }

    // set MALLOC_CAP_SPIRAM or MALLOC_CAP_DMA
    void setMallocCap(uint32_t flg) {
      _malloc_cap = flg;
    }

  protected:
    uint32_t _malloc_cap;
    void* _mem_alloc(uint32_t bytes) override
    {
      _disable_memcpy = (_malloc_cap & MALLOC_CAP_SPIRAM);
      void* res = heap_caps_malloc(bytes, _malloc_cap);

      // if can't malloc with PSRAM, try without using PSRAM malloc.
      if (res == nullptr && _disable_memcpy) {
        _disable_memcpy = false;
        res = heap_caps_malloc(bytes, _malloc_cap & ~MALLOC_CAP_SPIRAM);
      }
      return res;
    }
    void _mem_free(void* buf) override
    {
      heap_caps_free(buf);
    }
  };

#elif defined (ESP8226)

#elif defined (STM32F7)

#elif defined (__AVR__)

#endif

}

#endif
