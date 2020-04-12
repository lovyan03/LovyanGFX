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
#ifndef LGFX_SPRITE_HPP_
#define LGFX_SPRITE_HPP_

#include <algorithm>

#include "lgfx_base.hpp"

namespace lgfx
{
  class LGFX_Sprite : public LovyanGFX
  {
  public:

    LGFX_Sprite(LovyanGFX* parent)
    : LovyanGFX()
    , _parent(parent)
    , _img  (nullptr)
    , _bitwidth(0)
    , _xptr (0)
    , _yptr (0)
    , _xs   (0)
    , _xe   (0)
    , _ys   (0)
    , _ye   (0)
    , _index(0)
    {
      _read_conv = _write_conv;
      _spi_shared = false;
      _transaction_count = 0xFFFF;
    }

    __attribute__ ((always_inline)) inline void* getBuffer(void) const { return _img; }
    uint32_t bufferLength(void) const { return _bitwidth * _height * _write_conv.bits >> 3; }

    LGFX_Sprite()
    : LGFX_Sprite(nullptr)
    {}

    virtual ~LGFX_Sprite() {
      deleteSprite();
    }

    void deletePalette(void)
    {
      if (_palette != nullptr) { _mem_free(_palette); _palette = nullptr; }
      _palette_count = 0;
    }

    void deleteSprite(void)
    {
      deletePalette();
      if (_img     != nullptr) { _mem_free(_img    ); _img     = nullptr; }
      _width = 0;
      _height = 0;
    }

    void* createSprite(int32_t w, int32_t h)
    {
      if (w < 1 || h < 1) return nullptr;
      if (_img) deleteSprite();
      _bitwidth = (w + _write_conv.x_mask) & (~(uint32_t)_write_conv.x_mask);
      size_t len = (h * _bitwidth * _write_conv.bits >> 3) + 1;
      _img = (uint8_t*)_mem_alloc(len);
      if (!_img) return nullptr;
      memset(_img, 0, len);
      if (0 == _write_conv.bytes) createPalette();

      _sw = _width = w;
      _clip_r = _xe = w - 1;
      _xpivot = w >> 1;

      _sh = _height = h;
      _clip_b = _ye = h - 1;
      _ypivot = h >> 1;

      _clip_l = _clip_t = _index = _sx = _sy = _xs = _ys = _xptr = _yptr = 0;

      return _img;
    }


#if defined (ARDUINO)
 #if defined (FS_H)

    inline void createFromBmp(fs::FS &fs, const char *path) { createFromBmpFile(fs, path); }
    void createFromBmpFile(fs::FS &fs, const char *path) {
      FileWrapper file;
      file.setFS(fs);
      createFromBmpFile(&file, path);
    }

 #endif

#elif defined (CONFIG_IDF_TARGET_ESP32)  // ESP-IDF

    void createFromBmpFile(const char *path) {
      FileWrapper file;
      createFromBmpFile(&file, path);
    }

#endif

    void createFromBmp(const uint8_t *bmp_data, uint32_t bmp_len) {
      PointerWrapper data;
      data.set(bmp_data, bmp_len);
      create_from_bmp(&data);
    }

    void createFromBmpFile(FileWrapper* file, const char *path) {
      file->need_transaction = false;
      if (file->open(path, "rb")) {
        create_from_bmp(file);
        file->close();
      }
    }

    bool createPalette(void)
    {
      if (!create_palette()) return false;

      setPaletteGrayscale();
    }

    // create palette from RGB565(uint16_t) array
    bool createPalette(const uint16_t* colors, uint32_t count)
    {
      if (!create_palette()) return false;

      if (count > _palette_count) count = _palette_count;
      for (uint32_t i = 0; i < count; ++i) {
        _palette[i] = convert_rgb565_to_bgr888(pgm_read_word(&colors[i]));
      }
      return true;
    }

    // create palette from RGB888(uint32_t) array
    bool createPalette(const uint32_t* colors, uint32_t count)
    {
      if (!create_palette()) return false;

      if (count > _palette_count) count = _palette_count;
      for (uint32_t i = 0; i < count; ++i) {
        _palette[i] = convert_rgb888_to_bgr888(pgm_read_dword(&colors[i]));
      }
      return true;
    }

    void setPaletteGrayscale(void)
    {
      if (!_palette) return;
      uint32_t k;
      switch (_write_conv.depth) {
      case 8: k = 0x010101; break;
      case 4: k = 0x111111; break;
      case 2: k = 0x555555; break;
      case 1: k = 0xFFFFFF; break;
      default: k = 1; break;
      }
      for (uint32_t i = 0; i < _palette_count; i++) {
        _palette[i] = i * k;
      }
    }

    void setBitmapColor(uint16_t fgcolor, uint16_t bgcolor)  // For 1bpp sprites
    {
      if (_palette) {
        _palette[0] = *(rgb565_t*)&bgcolor;
        _palette[1] = *(rgb565_t*)&fgcolor;
      }
    }

    template<typename T> inline int32_t getPaletteIndex(const T& color)
    {
      uint32_t rgb = convert_to_rgb888(color);
      bgr888_t bgr((uint8_t)(rgb >> 16), (uint8_t)(rgb >> 8), (uint8_t)rgb);
      return getPaletteIndex(bgr);
    }
    int32_t getPaletteIndex(const bgr888_t& color)
    {
      size_t res = 0;
      do {
        if (_palette[res] == color) return res;
      } while (++res < _palette_count);
      return -1;
    }

    template<typename T> __attribute__ ((always_inline)) inline 
    void setPaletteColor(size_t index, T color) {
      if (!_palette || index >= _palette_count) return;
      rgb888_t c = convert_to_rgb888(color);
      _palette[index] = c;
    }

    void setPaletteColor(size_t index, const bgr888_t& rgb)
    {
      if (_palette && index < _palette_count) { _palette[index] = rgb; }
    }

    void setPaletteColor(size_t index, uint8_t r, uint8_t g, uint8_t b)
    {
      if (_palette && index < _palette_count) { _palette[index].set(r, g, b); }
    }

    __attribute__ ((always_inline)) inline void* setColorDepth(uint8_t bpp) { return setColorDepth((color_depth_t)bpp); }
    void* setColorDepth(color_depth_t depth)
    {
      _write_conv.setColorDepth(depth, _palette != nullptr) ;
      _read_conv = _write_conv;

      if (_img == nullptr) return nullptr;
      deleteSprite();
      return createSprite(_width, _height);
    }

    uint32_t readPixelValue(int32_t x, int32_t y)
    {
      auto bits = _read_conv.bits;
      if (bits >= 8) {
        int32_t index = x + y * _bitwidth;
        if (bits == 8) {
          return _img[index];
        } else if (bits == 16) {
          return _img16[index];
        } else {
          return (uint32_t)_img24[index];
        }
      } else {
        int32_t index = (x + y * _bitwidth) * bits;
        uint8_t mask = (1 << bits) - 1;
        return (_img[index >> 3] >> (-(index + bits) & 7)) & mask;
      }
    }

    template<typename T>
    __attribute__ ((always_inline)) inline void fillSprite (const T& color) { fillRect(0, 0, _width, _height, color); }

    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(                int32_t x, int32_t y, const T& transp) { push_sprite(_parent, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, int32_t x, int32_t y, const T& transp) { push_sprite(    dst, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    __attribute__ ((always_inline)) inline void pushSprite(                int32_t x, int32_t y) { push_sprite(_parent, x, y); }
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, int32_t x, int32_t y) { push_sprite(    dst, x, y); }

    template<typename T> bool pushRotated(                float angle, const T& transp) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotated(LovyanGFX* dst, float angle, const T& transp) { return push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
                         bool pushRotated(                float angle) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f); }
                         bool pushRotated(LovyanGFX* dst, float angle) { return push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f); }

    template<typename T> bool pushRotateZoom(                                              float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotateZoom(LovyanGFX* dst                              , float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(    dst, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotateZoom(                int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotateZoom(LovyanGFX* dst, int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
                         bool pushRotateZoom(                                              float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(LovyanGFX* dst                              , float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(    dst, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(                int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(LovyanGFX* dst, int32_t dst_x, int32_t dst_y, float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y); }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    LovyanGFX* _parent;
    union {
      uint8_t*  _img;
      uint16_t* _img16;
      bgr888_t* _img24;
    };
    int32_t _bitwidth;
    int32_t _xptr;
    int32_t _yptr;
    int32_t _xs;
    int32_t _xe;
    int32_t _ys;
    int32_t _ye;
    int32_t _index;
    bool _use_spiram = false; // disable PSRAM to PSRAM memcpy flg.

    bool create_palette(void)
    {
      if (_write_conv.depth > 8) return false;

      deletePalette();

      size_t palettes = 1 << _write_conv.bits;
      _palette = (bgr888_t*)_mem_alloc(sizeof(bgr888_t) * palettes);
      if (!_palette) {
        _write_conv.setColorDepth(_write_conv.depth, false);
        return false;
      }
      _palette_count = palettes;
      _write_conv.setColorDepth(_write_conv.depth, true);
      return true;
    }

    bool create_from_bmp(DataWrapper* data) {
      //uint32_t startTime = millis();
      bitmap_header_t bmpdata;

      if (!load_bmp_header(data, &bmpdata)
       || ( bmpdata.biCompression > 3)) {
        return false;
      }
      uint32_t seekOffset = bmpdata.bfOffBits;
      uint_fast16_t bpp = bmpdata.biBitCount; // 24 bcBitCount 24=RGB24bit
      setColorDepth(bpp);
      int32_t w = bmpdata.biWidth;
      int32_t h = bmpdata.biHeight;  // bcHeight Image height (pixels)
      if (!createSprite(w, h)) return false;

        //If the value of Height is positive, the image data is from bottom to top
        //If the value of Height is negative, the image data is from top to bottom.
      int32_t flow = (h < 0) ? 1 : -1;
      int32_t y = 0;
      if (h < 0) h = -h;
      else y = h - 1;

      if (bpp <= 8) {
        if (!_palette) createPalette();
        uint_fast16_t palettecount = 1 << bpp;
        argb8888_t *palette = new argb8888_t[palettecount];
        data->seek(bmpdata.biSize + 14);
        data->read((uint8_t*)palette, (palettecount * sizeof(argb8888_t))); // load palette
        for (uint_fast16_t i = 0; i < _palette_count; ++i) {
          _palette[i] = palette[i];
        }
        delete[] palette;
      }

      data->seek(seekOffset);

      size_t buffersize = ((w * bpp + 31) >> 5) << 2;  // readline 4Byte align.
      uint8_t lineBuffer[buffersize];  // readline 4Byte align.
      if (bpp <= 8) {
        do {
          if (bmpdata.biCompression == 1) {
            load_bmp_rle8(data, lineBuffer, w);
          } else
          if (bmpdata.biCompression == 2) {
            load_bmp_rle4(data, lineBuffer, w);
          } else {
            data->read(lineBuffer, buffersize);
          }
          memcpy(&_img[y * _bitwidth * bpp >> 3], lineBuffer, w * bpp >> 3);
          y += flow;
        } while (--h);
      } else if (bpp == 16) {
        do {
          data->read(lineBuffer, buffersize);
          auto img = &_img[y * _bitwidth * bpp >> 3];
          y += flow;
          for (size_t i = 0; i < buffersize; ++i) {
            img[i] = lineBuffer[i ^ 1];
          }
        } while (--h);
      } else if (bpp == 24) {
        do {
          data->read(lineBuffer, buffersize);
          auto img = &_img[y * _bitwidth * bpp >> 3];
          y += flow;
          for (size_t i = 0; i < buffersize; i += 3) {
            img[i    ] = lineBuffer[i + 2];
            img[i + 1] = lineBuffer[i + 1];
            img[i + 2] = lineBuffer[i    ];
          }
        } while (--h);
      } else if (bpp == 32) {
        do {
          data->read(lineBuffer, buffersize);
          auto img = &_img[y * _bitwidth * 3];
          y += flow;
          for (size_t i = 0; i < buffersize; i += 4) {
            img[(i>>2)*3    ] = lineBuffer[i + 2];
            img[(i>>2)*3 + 1] = lineBuffer[i + 1];
            img[(i>>2)*3 + 2] = lineBuffer[i + 0];
          }
        } while (--h);
      }
      return true;
    }

    void push_sprite(LovyanGFX* dst, int32_t x, int32_t y, uint32_t transp = ~0)
    {
      pixelcopy_t p(_img, dst->getColorDepth(), getColorDepth(), dst->hasPalette(), _palette, transp);
      dst->push_image(x, y, _width, _height, &p, !_use_spiram); // DMA disable with use SPIRAM
    }

    inline bool push_rotate_zoom(LovyanGFX* dst, int32_t x, int32_t y, float angle, float zoom_x, float zoom_y, uint32_t transp = ~0)
    {
      return dst->pushImageRotateZoom(x, y, _img, _xpivot, _ypivot, _width, _height, angle, zoom_x, zoom_y, transp, getColorDepth(), _palette);
    }

    void set_window(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      if (xs > xe) std::swap(xs, xe);
      if (ys > ye) std::swap(ys, ye);
      if ((xe < 0) || (ye < 0) || (xs >= _width) || (ys >= _height))
      {
        _xptr = _xs = _xe = 0;
        _yptr = _ys = _ye = _height;
      } else {
        _xptr = _xs = (xs < 0) ? 0 : xs;
        _yptr = _ys = (ys < 0) ? 0 : ys;
        _xe = std::min(xe, _width  - 1);
        _ye = std::min(ye, _height - 1);
      }
      _index = xs + ys * _bitwidth;
    }

    void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      auto bits = _write_conv.bits;
      if (bits >= 8) {
        int32_t index = x + y * _bitwidth;
        if (bits == 8) {
          _img[index] = _color.raw0;
        } else if (bits == 16) {
          _img16[index] = _color.rawL;
        } else {
          _img24[index] = *(bgr888_t*)&_color;
        }
      } else {
        int32_t index = (x + y * _bitwidth) * bits;
        uint8_t* dst = &_img[index >> 3];
        uint8_t mask = (uint8_t)(~(0xFF >> bits)) >> (index & 7);
        *dst = (*dst & ~mask) | (_color.raw0 & mask);
      }
    }

    void writeFillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
/* // for debug pushBlock_impl
setWindow(x,y,x+w-1,y+h-1);
pushBlock_impl(w*h);
return;
//*/
      uint32_t bits = _write_conv.bits;
      if (bits >= 8) {
        if (w == 1) {
          uint32_t bw = _bitwidth;
          uint32_t index = x + y * bw;
          if (bits == 8) {
            uint8_t c = _color.raw0;
            auto img = &_img[index];
            do { *img = c;  img += bw; } while (--h);
          } else if (bits == 16) {
            uint16_t c = _color.rawL;
            auto img = &_img16[index];
            do { *img = c;  img += bw; } while (--h);
          } else {  // if (_write_conv.bytes == 3)
            auto c = _color;
            auto img = &_img24[index];
            do { img->r = c.raw0; img->g = c.raw1; img->b = c.raw2; img += bw; } while (--h);
          }
        } else {
          uint32_t bytes = bits >> 3;
          uint32_t bw = _bitwidth;
          uint8_t* dst = &_img[(x + y * bw) * bytes];
          uint8_t c = _color.raw0;
          if (bytes == 1 || (c == _color.raw1 && (bytes == 2 || (c == _color.raw2)))) {
            if (w == bw) {
              memset(dst, c, w * bytes * h);
            } else {
              uint32_t add_dst = bw * bytes;
              do {
                memset(dst, c, w * bytes);
                dst += add_dst;
              } while (--h);
            }
          } else {
            size_t len = w * bytes;
            uint32_t add_dst = bw * bytes;
            uint32_t color = _color.raw;
            if (_use_spiram) {
              uint8_t linebuf[len];
              memset_multi(linebuf, color, bytes, w);
              do {
                memcpy(dst, linebuf, len);
                dst += add_dst;
              } while (--h);
            } else {
              if (w == bw) {
                memset_multi(dst, color, bytes, w * h);
              } else {
                memset_multi(dst, color, bytes, w);
                while (--h) {
                  memcpy(dst + add_dst, dst, len);
                  dst += add_dst;
                }
              }
            }
          }
        }
      } else {
        x *= bits;
        w *= bits;
        size_t add_dst = _bitwidth * bits >> 3;
        uint8_t* dst = &_img[y * add_dst + (x >> 3)];
        uint32_t len = ((x + w) >> 3) - (x >> 3);
        uint8_t mask = 0xFF >> (x & 7);
        uint8_t c = _color.raw0;
        if (len) {
          if (mask != 0xFF) {
            --len;
            auto d = dst++;
            uint8_t mc = c & mask;
            auto i = h;
            do { *d = (*d & ~mask) | mc; d += add_dst; } while (--i);
          }
          mask = ~(0xFF>>((x + w) & 7));
          if (len) {
            auto d = dst;
            auto i = h;
            do { memset(d, c, len); d += add_dst; } while (--i);
            dst += len;
          }
          if (mask == 0) return;
        } else {
          mask ^= mask >> w;
        }
        c &= mask;
        do { *dst = (*dst & ~mask) | c; dst += add_dst; } while (--h);
      }
    }

    void pushBlock_impl(int32_t length) override
    {
      if (0 >= length) return;
      if (_write_conv.bytes == 0) {
        int32_t bits = _write_conv.bits;
        uint8_t c = _color.raw0;
        int32_t ll;
        int32_t index = _index;
        do {
          uint8_t* dst = &_img[index * bits >> 3];
          ll = std::min(_xe - _xptr + 1, length);
          int32_t w = ll * bits;
          int32_t x = _xptr * bits;
          size_t len = ((x + w) >> 3) - (x >> 3);
          uint8_t mask = 0xFF >> (x & 7);
          if (!len) {
            mask ^= mask >> w;
            *dst = (*dst & ~mask) | (c & mask);
          } else {
            if (mask != 0xFF) {
              *dst = (*dst & ~mask) | (c & mask);
              ++dst;
              --len;
            }
            if (len) {
              memset(dst, c, len);
              dst += len;
            }
            mask = 0xFF >> ((x + w) & 7);
            if (mask != 0xFF) *dst = (*dst & mask) | (c & ~mask);
          }
          index = ptr_advance(ll);
        } while (length -= ll);
      } else {
        uint32_t bytes = _write_conv.bytes;
        if (bytes == 1 || (_color.raw0 == _color.raw1 && (bytes == 2 || (_color.raw0 == _color.raw2)))) {
          uint32_t color = _color.raw;
          int32_t ll;
          int32_t index = _index;
          do {
            ll = std::min(_xe - _xptr + 1, length);
            memset(&_img[index * bytes], color, ll * bytes);
            index = ptr_advance(ll);
          } while (length -= ll);
        } else
        if (_use_spiram) {
          int32_t buflen = std::min(_xe - _xs + 1, length);
          uint8_t linebuf[buflen * bytes];
          memset_multi(linebuf, _color.raw, bytes, buflen);
          int32_t ll;
          int32_t index = _index;
          do  {
            ll = std::min(_xe - _xptr + 1, length);
            memcpy(&_img[index * bytes], linebuf, ll * bytes);
            index = ptr_advance(ll);
          } while (length -= ll);
          return;
        } else {
          uint32_t color = _color.raw;
          int32_t ll;
          int32_t index = _index;
          do {
            ll = std::min(_xe - _xptr + 1, length);
            memset_multi(&_img[index * bytes], color, bytes, ll);
            index = ptr_advance(ll);
          } while (length -= ll);
        }
      }
    }

    void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) override
    {
      if (_write_conv.bits < 8) {
        pixelcopy_t param(_img, _write_conv.depth, _write_conv.depth);
        param.src_width = _bitwidth;
        int32_t add_y = (src_y < dst_y) ? -1 : 1;
        if (src_y != dst_y) {
          if (src_y < dst_y) {
            src_y += h - 1;
            dst_y += h - 1;
          }
          param.src_y = src_y;
          do {
            param.src_x = src_x;
            auto idx = dst_x + dst_y * _bitwidth;
            param.fp_copy(_img, idx, idx + w, &param);
            dst_y += add_y;
            param.src_y += add_y;
          } while (--h);
        } else {
          size_t len = (_bitwidth * _write_conv.bits) >> 3;
          uint8_t buf[len];
          param.src_data = buf;
          param.src_y32 = 0;
          do {
            memcpy(buf, &_img[src_y * len], len);
            param.src_x = src_x;
            auto idx = dst_x + dst_y * _bitwidth;
            param.fp_copy(_img, idx, idx + w, &param);
            dst_y += add_y;
            src_y += add_y;
          } while (--h);
        }
      } else {
        size_t len = w * _write_conv.bytes;
        int32_t add = _bitwidth * _write_conv.bytes;
        if (src_y < dst_y) add = -add;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        uint8_t* src = &_img[(src_x + (src_y + pos) * _bitwidth) * _write_conv.bytes];
        uint8_t* dst = &_img[(dst_x + (dst_y + pos) * _bitwidth) * _write_conv.bytes];
        if (_use_spiram) {
          uint8_t buf[len];
          do {
            memcpy(buf, src, len);
            memcpy(dst, buf, len);
            dst += add;
            src += add;
          } while (--h);
        } else {
          do {
            memmove(dst, src, len);
            src += add;
            dst += add;
          } while (--h);
        }
      }
    }

    void readRect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) override
    {
      h += y;
      if (param->no_convert && _read_conv.bytes) {
        auto b = _read_conv.bytes;
        auto bw = _bitwidth;
        auto d = (uint8_t*)dst;
        do {
          memcpy(d, &_img[(x + y * bw) * b], w * b);
          d += w * b;
        } while (++y != h);
      } else {
        param->src_width = _bitwidth;
        param->src_data = _img;
        int32_t dstindex = 0;
        do {
          param->src_x = x;
          param->src_y = y;
          dstindex = param->fp_copy(dst, dstindex, dstindex + w, param);
        } while (++y != h);
      }
    }

    void pushImage_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param, bool use_dma) override
    {
      auto sx = param->src_x;
      do {
        int32_t pos = x + (y++) * _bitwidth;
        int32_t end = pos + w;
        while (end != (pos = param->fp_copy(_img, pos, end, param))) {
          if ( end == (pos = param->fp_skip(      pos, end, param))) break;
        }
        param->src_x = sx;
        param->src_y++;
      } while (--h);
    }

    void pushColors_impl(int32_t length, pixelcopy_t* param) override
    {
      auto k = _bitwidth * _write_conv.bits >> 3;
      int32_t linelength;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        param->fp_copy(&_img[_yptr * k], _xptr, _xptr + linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void beginTransaction_impl(void) override {}
    void endTransaction_impl(void) override {}
    void waitDMA_impl(void) override {}

    inline int32_t ptr_advance(int32_t length = 1) {
      if ((_xptr += length) > _xe) {
        _xptr = _xs;
        if (++_yptr > _ye) {
          _yptr = _ys;
        }
        return (_index = _xptr + _yptr * _bitwidth);
      } else {
        return (_index += length);
      }
    }



//----------------------------------------------------------------------------
#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)
//----------------------------------------------------------------------------

  public:
    void setPsram( bool enabled ) {
      if (enabled) _malloc_cap |= MALLOC_CAP_SPIRAM;
      else         _malloc_cap &= ~MALLOC_CAP_SPIRAM;
    }

    // set MALLOC_CAP_SPIRAM or MALLOC_CAP_DMA
    void setMallocCap(uint32_t flg) {
      _malloc_cap = flg;
    }

  protected:
    uint32_t _malloc_cap = MALLOC_CAP_8BIT;
    void* _mem_alloc(uint32_t bytes)
    {
      _use_spiram = (_malloc_cap & MALLOC_CAP_SPIRAM);
      void* res = heap_caps_malloc(bytes, _malloc_cap);

      // if can't malloc with PSRAM, try without using PSRAM malloc.
      if (res == nullptr && _use_spiram) {
        _use_spiram = false;
        res = heap_caps_malloc(bytes, _malloc_cap & ~MALLOC_CAP_SPIRAM);
      }
      return res;
    }
    void _mem_free(void* buf)
    {
      heap_caps_free(buf);
    }

//----------------------------------------------------------------------------
#elif defined (ESP8226)
//----------------------------------------------------------------------------
// not implemented.
//----------------------------------------------------------------------------
#elif defined (STM32F7)
//----------------------------------------------------------------------------
// not implemented.
//----------------------------------------------------------------------------
#elif defined (__AVR__)
//----------------------------------------------------------------------------
// not implemented.
//----------------------------------------------------------------------------

#endif

  };

}

typedef lgfx::LGFX_Sprite LGFX_Sprite;

#endif
