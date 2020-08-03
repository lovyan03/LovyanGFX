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

#include "LGFXBase.hpp"

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
    std::uint32_t bufferLength(void) const { return (_bitwidth * _write_conv.bits >> 3) * _height; }

    LGFX_Sprite()
    : LGFX_Sprite(nullptr)
    {}

    virtual ~LGFX_Sprite() {
      deleteSprite();
    }

    void deletePalette(void)
    {
      _palette_count = 0;
      if (_palette != nullptr) {
        _mem_free(_palette);
        _palette = nullptr;
      }
    }

    void deleteSprite(void)
    {
      _bitwidth = 0;
      _width = 0;
      _height = 0;
      _clip_l = 0;
      _clip_t = 0;
      _clip_r = -1;
      _clip_b = -1;
      _sw = 0;
      _sh = 0;
      deletePalette();
      if (_img != nullptr) {
        _mem_free(_img);
        _img = nullptr;
      }
    }

    void setPsram( bool enabled )
    {
      _psram = enabled;
    }

    void* createSprite(std::int32_t w, std::int32_t h)
    {
      if (w < 1 || h < 1) return nullptr;
      if (_img != nullptr) {
        _mem_free(_img);
        _img = nullptr;
      }
      _bitwidth = (w + _write_conv.x_mask) & (~(std::uint32_t)_write_conv.x_mask);
      size_t len = (h * _bitwidth * _write_conv.bits >> 3) + 1;
      _img = (std::uint8_t*)_mem_alloc(len);
      if (!_img) {
        deleteSprite();
        return nullptr;
      }
      memset(_img, 0, len);
      if (_palette == nullptr && 0 == _write_conv.bytes) createPalette();

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
 #if defined (FS_H) || defined (__SEEED_FS__)

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

    void createFromBmp(const std::uint8_t *bmp_data, std::uint32_t bmp_len) {
      PointerWrapper data;
      data.set(bmp_data, bmp_len);
      create_from_bmp(&data);
    }

    bool createPalette(void)
    {
      if (!create_palette()) return false;

      setPaletteGrayscale();
      return true;
    }

    // create palette from RGB565(std::uint16_t) array
    bool createPalette(const std::uint16_t* colors, std::uint32_t count)
    {
      if (!create_palette()) return false;

      if (count > _palette_count) count = _palette_count;
      for (std::uint32_t i = 0; i < count; ++i) {
        _palette[i] = convert_rgb565_to_bgr888(colors[i]);
      }
      return true;
    }

    // create palette from RGB888(std::uint32_t) array
    bool createPalette(const std::uint32_t* colors, std::uint32_t count)
    {
      if (!create_palette()) return false;

      if (count > _palette_count) count = _palette_count;
      for (std::uint32_t i = 0; i < count; ++i) {
        _palette[i] = convert_rgb888_to_bgr888(colors[i]);
      }
      return true;
    }

    void setPaletteGrayscale(void)
    {
      if (!_palette) return;
      std::uint32_t k;
      switch (_write_conv.depth) {
      case 8: k = 0x010101; break;
      case 4: k = 0x111111; break;
      case 2: k = 0x555555; break;
      case 1: k = 0xFFFFFF; break;
      default: k = 1; break;
      }
      for (std::uint32_t i = 0; i < _palette_count; i++) {
        _palette[i] = i * k;
      }
    }

    void setBitmapColor(std::uint16_t fgcolor, std::uint16_t bgcolor)  // For 1bpp sprites
    {
      if (_palette) {
        _palette[0] = *(rgb565_t*)&bgcolor;
        _palette[1] = *(rgb565_t*)&fgcolor;
      }
    }

    template<typename T> inline std::int32_t getPaletteIndex(const T& color)
    {
      std::uint32_t rgb = convert_to_rgb888(color);
      bgr888_t bgr((std::uint8_t)(rgb >> 16), (std::uint8_t)(rgb >> 8), (std::uint8_t)rgb);
      return getPaletteIndex(bgr);
    }
    std::int32_t getPaletteIndex(const bgr888_t& color)
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

    void setPaletteColor(size_t index, std::uint8_t r, std::uint8_t g, std::uint8_t b)
    {
      if (_palette && index < _palette_count) { _palette[index].set(r, g, b); }
    }

    __attribute__ ((always_inline)) inline void* setColorDepth(std::uint8_t bpp) { return setColorDepth((color_depth_t)bpp); }
    void* setColorDepth(color_depth_t depth)
    {
      _write_conv.setColorDepth(depth, _palette != nullptr) ;
      _read_conv = _write_conv;

      if (_img == nullptr) return nullptr;
      deleteSprite();
      return createSprite(_width, _height);
    }

    std::uint32_t readPixelValue(std::int32_t x, std::int32_t y)
    {
      auto bits = _read_conv.bits;
      if (bits >= 8) {
        std::int32_t index = x + y * _bitwidth;
        if (bits == 8) {
          return _img[index];
        } else if (bits == 16) {
          return _img16[index];
        } else {
          return (std::uint32_t)_img24[index];
        }
      } else {
        std::int32_t index = (x + y * _bitwidth) * bits;
        std::uint8_t mask = (1 << bits) - 1;
        return (_img[index >> 3] >> (-(index + bits) & 7)) & mask;
      }
    }

    template<typename T>
    __attribute__ ((always_inline)) inline void fillSprite (const T& color) { fillRect(0, 0, _width, _height, color); }

    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(                std::int32_t x, std::int32_t y, const T& transp) { push_sprite(_parent, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, std::int32_t x, std::int32_t y, const T& transp) { push_sprite(    dst, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    __attribute__ ((always_inline)) inline void pushSprite(                std::int32_t x, std::int32_t y) { push_sprite(_parent, x, y); }
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, std::int32_t x, std::int32_t y) { push_sprite(    dst, x, y); }

    template<typename T> bool pushRotated(                float angle, const T& transp) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotated(LovyanGFX* dst, float angle, const T& transp) { return push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
                         bool pushRotated(                float angle) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f); }
                         bool pushRotated(LovyanGFX* dst, float angle) { return push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f); }

    template<typename T> bool pushRotateZoom(                                              float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotateZoom(LovyanGFX* dst                              , float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(    dst,     dst->getPivotX(),     dst->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotateZoom(                std::int32_t dst_x, std::int32_t dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> bool pushRotateZoom(LovyanGFX* dst, std::int32_t dst_x, std::int32_t dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { return push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
                         bool pushRotateZoom(                                              float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(LovyanGFX* dst                              , float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(    dst,     dst->getPivotX(),     dst->getPivotY(), angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(                std::int32_t dst_x, std::int32_t dst_y, float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y); }
                         bool pushRotateZoom(LovyanGFX* dst, std::int32_t dst_x, std::int32_t dst_y, float angle, float zoom_x, float zoom_y)                  { return push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y); }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    LovyanGFX* _parent;
    union {
      std::uint8_t*  _img;
      std::uint16_t* _img16;
      bgr888_t* _img24;
    };
    std::int32_t _bitwidth;
    std::int32_t _xptr;
    std::int32_t _yptr;
    std::int32_t _xs;
    std::int32_t _xe;
    std::int32_t _ys;
    std::int32_t _ye;
    std::int32_t _index;
    bool _disable_memcpy = false; // disable PSRAM to PSRAM memcpy flg.
    bool _psram = false;

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

    void createFromBmpFile(FileWrapper* file, const char *path) {
      file->need_transaction = false;
      if (file->open(path, "r")) {
        create_from_bmp(file);
        file->close();
      }
    }

    bool create_from_bmp(DataWrapper* data) {
      bitmap_header_t bmpdata;

      if (!load_bmp_header(data, &bmpdata)
       || ( bmpdata.biCompression > 3)) {
        return false;
      }
      std::uint32_t seekOffset = bmpdata.bfOffBits;
      std::uint_fast16_t bpp = bmpdata.biBitCount; // 24 bcBitCount 24=RGB24bit
      setColorDepth(bpp);
      std::int32_t w = bmpdata.biWidth;
      std::int32_t h = bmpdata.biHeight;  // bcHeight Image height (pixels)
      if (!createSprite(w, h)) return false;

        //If the value of Height is positive, the image data is from bottom to top
        //If the value of Height is negative, the image data is from top to bottom.
      std::int32_t flow = (h < 0) ? 1 : -1;
      std::int32_t y = 0;
      if (h < 0) h = -h;
      else y = h - 1;

      if (bpp <= 8) {
        if (!_palette) createPalette();
        std::uint_fast16_t palettecount = 1 << bpp;
        argb8888_t *palette = new argb8888_t[palettecount];
        data->seek(bmpdata.biSize + 14);
        data->read((std::uint8_t*)palette, (palettecount * sizeof(argb8888_t))); // load palette
        for (std::uint_fast16_t i = 0; i < _palette_count; ++i) {
          _palette[i] = palette[i];
        }
        delete[] palette;
      }

      data->seek(seekOffset);

      size_t buffersize = ((w * bpp + 31) >> 5) << 2;  // readline 4Byte align.
      std::uint8_t lineBuffer[buffersize];  // readline 4Byte align.
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

    void push_sprite(LovyanGFX* dst, std::int32_t x, std::int32_t y, std::uint32_t transp = ~0)
    {
      pixelcopy_t p(_img, dst->getColorDepth(), getColorDepth(), dst->hasPalette(), _palette, transp);
      dst->push_image(x, y, _width, _height, &p, !_disable_memcpy); // DMA disable with use SPIRAM
    }

    inline bool push_rotate_zoom(LovyanGFX* dst, std::int32_t x, std::int32_t y, float angle, float zoom_x, float zoom_y, std::uint32_t transp = ~0)
    {
      return dst->pushImageRotateZoom(x, y, _img, _xpivot, _ypivot, _width, _height, angle, zoom_x, zoom_y, transp, getColorDepth(), _palette);
    }

    void set_window(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye)
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

    void setWindow_impl(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
    }

    void drawPixel_impl(std::int32_t x, std::int32_t y) override
    {
      auto bits = _write_conv.bits;
      if (bits >= 8) {
        std::int32_t index = x + y * _bitwidth;
        if (bits == 8) {
          _img[index] = _color.raw0;
        } else if (bits == 16) {
          _img16[index] = _color.rawL;
        } else {
          _img24[index] = *(bgr888_t*)&_color;
        }
      } else {
        std::int32_t index = (x + y * _bitwidth) * bits;
        std::uint8_t* dst = &_img[index >> 3];
        std::uint8_t mask = (std::uint8_t)(~(0xFF >> bits)) >> (index & 7);
        *dst = (*dst & ~mask) | (_color.raw0 & mask);
      }
    }

    void writeFillRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) override
    {
/* // for debug pushBlock_impl
setWindow(x,y,x+w-1,y+h-1);
pushBlock_impl(w*h);
return;
//*/
      std::uint32_t bits = _write_conv.bits;
      if (bits >= 8) {
        if (w == 1) {
          std::uint32_t bw = _bitwidth;
          std::uint32_t index = x + y * bw;
          if (bits == 8) {
            std::uint8_t c = _color.raw0;
            auto img = &_img[index];
            do { *img = c;  img += bw; } while (--h);
          } else if (bits == 16) {
            std::uint16_t c = _color.rawL;
            auto img = &_img16[index];
            do { *img = c;  img += bw; } while (--h);
          } else {  // if (_write_conv.bytes == 3)
            auto c = _color;
            auto img = &_img24[index];
            do { img->r = c.raw0; img->g = c.raw1; img->b = c.raw2; img += bw; } while (--h);
          }
        } else {
          std::uint32_t bytes = bits >> 3;
          std::int32_t bw = _bitwidth;
          std::uint8_t* dst = &_img[(x + y * bw) * bytes];
          std::uint8_t c = _color.raw0;
          if (bytes == 1 || (c == _color.raw1 && (bytes == 2 || (c == _color.raw2)))) {
            if (w == bw) {
              memset(dst, c, w * bytes * h);
            } else {
              std::uint32_t add_dst = bw * bytes;
              do {
                memset(dst, c, w * bytes);
                dst += add_dst;
              } while (--h);
            }
          } else {
            size_t len = w * bytes;
            std::uint32_t add_dst = bw * bytes;
            std::uint32_t color = _color.raw;
            if (_disable_memcpy) {
              std::uint8_t linebuf[len];
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
        std::uint8_t* dst = &_img[y * add_dst + (x >> 3)];
        std::uint32_t len = ((x + w) >> 3) - (x >> 3);
        std::uint8_t mask = 0xFF >> (x & 7);
        std::uint8_t c = _color.raw0;
        if (len) {
          if (mask != 0xFF) {
            --len;
            auto d = dst++;
            std::uint8_t mc = c & mask;
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

    void pushBlock_impl(std::int32_t length) override
    {
      if (0 >= length) return;
      if (_write_conv.bytes == 0) {
        std::int32_t bits = _write_conv.bits;
        std::uint8_t c = _color.raw0;
        std::int32_t ll;
        std::int32_t index = _index;
        do {
          std::uint8_t* dst = &_img[index * bits >> 3];
          ll = std::min(_xe - _xptr + 1, length);
          std::int32_t w = ll * bits;
          std::int32_t x = _xptr * bits;
          size_t len = ((x + w) >> 3) - (x >> 3);
          std::uint8_t mask = 0xFF >> (x & 7);
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
        std::uint32_t bytes = _write_conv.bytes;
        if (bytes == 1 || (_color.raw0 == _color.raw1 && (bytes == 2 || (_color.raw0 == _color.raw2)))) {
          std::uint32_t color = _color.raw;
          std::int32_t ll;
          std::int32_t index = _index;
          do {
            ll = std::min(_xe - _xptr + 1, length);
            memset(&_img[index * bytes], color, ll * bytes);
            index = ptr_advance(ll);
          } while (length -= ll);
        } else
        if (_disable_memcpy) {
          std::int32_t buflen = std::min(_xe - _xs + 1, length);
          std::uint8_t linebuf[buflen * bytes];
          memset_multi(linebuf, _color.raw, bytes, buflen);
          std::int32_t ll;
          std::int32_t index = _index;
          do  {
            ll = std::min(_xe - _xptr + 1, length);
            memcpy(&_img[index * bytes], linebuf, ll * bytes);
            index = ptr_advance(ll);
          } while (length -= ll);
          return;
        } else {
          std::uint32_t color = _color.raw;
          std::int32_t ll;
          std::int32_t index = _index;
          do {
            ll = std::min(_xe - _xptr + 1, length);
            memset_multi(&_img[index * bytes], color, bytes, ll);
            index = ptr_advance(ll);
          } while (length -= ll);
        }
      }
    }

    void copyRect_impl(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y) override
    {
      if (_write_conv.bits < 8) {
        pixelcopy_t param(_img, _write_conv.depth, _write_conv.depth);
        param.src_width = _bitwidth;
        std::int32_t add_y = (src_y < dst_y) ? -1 : 1;
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
          std::uint8_t buf[len];
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
        std::int32_t add = _bitwidth * _write_conv.bytes;
        if (src_y < dst_y) add = -add;
        std::int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        std::uint8_t* src = &_img[(src_x + (src_y + pos) * _bitwidth) * _write_conv.bytes];
        std::uint8_t* dst = &_img[(dst_x + (dst_y + pos) * _bitwidth) * _write_conv.bytes];
        if (_disable_memcpy) {
          std::uint8_t buf[len];
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

    void readRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param) override
    {
      h += y;
      if (param->no_convert && _read_conv.bytes) {
        auto b = _read_conv.bytes;
        auto bw = _bitwidth;
        auto d = (std::uint8_t*)dst;
        do {
          memcpy(d, &_img[(x + y * bw) * b], w * b);
          d += w * b;
        } while (++y != h);
      } else {
        param->src_width = _bitwidth;
        param->src_data = _img;
        std::int32_t dstindex = 0;
        do {
          param->src_x = x;
          param->src_y = y;
          dstindex = param->fp_copy(dst, dstindex, dstindex + w, param);
        } while (++y != h);
      }
    }

    void pushImage_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param, bool) override
    {
      auto sx = param->src_x;
      if (param->transp == ~0u && param->no_convert && !_disable_memcpy) {
        auto bits = param->src_bits;
        std::uint_fast8_t mask = (bits == 1) ? 7
                               : (bits == 2) ? 3
                                             : 1;
        if (0 == (bits & 7) || ((sx & mask) == (x & mask) && (w == this->_width || 0 == (w & mask)))) {
          auto bw = _bitwidth * bits >> 3;
          auto dd = &_img[bw * y];
          auto sw = param->src_width * bits >> 3;
          auto sd = &((std::uint8_t*)param->src_data)[param->src_y * sw];
          if (sw == bw && this->_width == w && sx == 0 && x == 0) {
            memcpy(dd, sd, bw * h);
            return;
          }
          y = 0;
          w =   w * bits >> 3;
          x =   x * bits >> 3;
          sx = sx * bits >> 3;
          do {
            memcpy(&dd[y * bw + x], &sd[y * sw + sx], w);
          } while (++y != h);
          return;
        }
      }

      do {
        std::int32_t pos = x + (y++) * _bitwidth;
        std::int32_t end = pos + w;
        while (end != (pos = param->fp_copy(_img, pos, end, param))) {
          if ( end == (pos = param->fp_skip(      pos, end, param))) break;
        }
        param->src_x = sx;
        param->src_y++;
      } while (--h);
    }

    void writePixelsDMA_impl(const void* data, std::int32_t length) override
    {
      auto src = static_cast<const uint8_t*>(data);
      auto k = _bitwidth * _write_conv.bits >> 3;
      std::int32_t linelength;
      do {
        linelength = std::min<int>(_xe - _xptr + 1, length);
        auto len = linelength * _write_conv.bits >> 3;
        memcpy(&_img[(_xptr * _write_conv.bits >> 3) + _yptr * k], src, len);
        src += len;
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void writePixels_impl(std::int32_t length, pixelcopy_t* param) override
    {
      auto k = _bitwidth * _write_conv.bits >> 3;
      std::int32_t linelength;
      do {
        linelength = std::min(_xe - _xptr + 1, length);
        param->fp_copy(&_img[_yptr * k], _xptr, _xptr + linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void beginTransaction_impl(void) override {}
    void endTransaction_impl(void) override {}
    void initDMA_impl(void) override {}
    void waitDMA_impl(void) override {}
    bool dmaBusy_impl(void) override { return false; }

    inline std::int32_t ptr_advance(std::int32_t length = 1) {
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

    static void memset_multi(std::uint8_t* buf, std::uint32_t c, size_t size, size_t length)
    {
      size_t l = length;
      if (l & ~0xF) {
        while ((l >>= 1) & ~0xF);
        ++l;
      }
      size_t len = l * size;
      length = (length * size) - len;
      std::uint8_t* dst = buf;
      if (size == 2) {
        do { // 2byte speed tweak
          *(std::uint16_t*)dst = c;
          dst += 2;
        } while (--l);
      } else {
        do {
          size_t i = 0;
          do {
            *dst++ = *(((std::uint8_t*)&c) + i);
          } while (++i != size);
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

    void* _mem_alloc(std::uint32_t bytes)
    {
      if (_psram)
      {
        void* res = heap_alloc_psram(bytes);
        if (res != nullptr) {
          _disable_memcpy = true;
          return res;
        }
      }

      _disable_memcpy = false;
      return heap_alloc_dma(bytes);
    }
    void _mem_free(void* buf)
    {
      heap_free(buf);
    }

    bool isReadable_impl(void) const { return true; }
    std::int_fast8_t getRotation_impl(void) const { return 0; }
  };

}

typedef lgfx::LGFX_Sprite LGFX_Sprite;

#endif
