/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#include <assert.h>

#include "LGFXBase.hpp"
#include "misc/SpriteBuffer.hpp"
#include "misc/bitmap.hpp"
#include "Panel.hpp"

namespace lgfx
{
 inline namespace v1
 {

#if defined ( _MSVC_LANG )
#define LGFX_INLINE inline
#else
#define LGFX_INLINE __attribute__ ((always_inline)) inline
#endif

//----------------------------------------------------------------------------
  class LGFX_Sprite;

  struct Panel_Sprite : public IPanel
  {
    friend LGFX_Sprite;

    Panel_Sprite(void) { _start_count = INT32_MAX; }

    void beginTransaction(void) override {}
    void endTransaction(void) override {}
    void setInvert(bool) override {}
    void setSleep(bool) override {}
    void setPowerSave(bool) override {}
    void writeCommand(uint32_t, uint_fast8_t) override {}
    void writeData(uint32_t, uint_fast8_t) override {}
    void initDMA(void) override {}
    void waitDMA(void) override {}
    bool dmaBusy(void) override { return false; }
    void waitDisplay(void) override {}
    bool displayBusy(void) override { return false; }
    void display(uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t) override {}
    bool isReadable(void) const override { return true; }
    bool isBusShared(void) const override { return false; }

    uint32_t readCommand(uint_fast8_t, uint_fast8_t, uint_fast8_t) override { return 0; }
    uint32_t readData(uint_fast8_t, uint_fast8_t) override { return 0; }


    void setBuffer(void* buffer, int32_t w, int32_t h, color_conv_t* conv);
    void deleteSprite(void);
    void* createSprite(int32_t w, int32_t h, color_conv_t* conv, bool psram);

    LGFX_INLINE void* getBuffer(void) const { return _img.get(); }
    LGFX_INLINE const SpriteBuffer* getSpriteBuffer(void) const { return &_img; }
    LGFX_INLINE uint32_t bufferLength(void) const { return (_bitwidth * _write_bits >> 3) * _panel_height; }


    color_depth_t setColorDepth(color_depth_t depth) override;
    void setRotation(uint_fast8_t r) override;

    void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override;
    void drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t raw_color) override;
    void writeBlock(uint32_t rawcolor, uint32_t len) override;
    void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override;
    void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool) override;
    void writeImageARGB(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param) override;

    void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override;
    void copyRect(uint_fast16_t dst_x, uint_fast16_t dst_y, uint_fast16_t w, uint_fast16_t h, uint_fast16_t src_x, uint_fast16_t src_y) override;

    uint32_t readPixelValue(uint_fast16_t x, uint_fast16_t y);

  protected:
    void _rotate_pixelcopy(uint_fast16_t& x, uint_fast16_t& y, uint_fast16_t& w, uint_fast16_t& h, pixelcopy_t* param, uint32_t& nextx, uint32_t& nexty);

    SpriteBuffer _img;

    uint_fast16_t _xpos;
    uint_fast16_t _ypos;
    uint_fast16_t _panel_width;   // rotationしていない状態の幅;
    uint_fast16_t _panel_height;  // rotationしていない状態の高さ;
    uint_fast16_t _bitwidth;
  };

  class LGFX_Sprite : public LovyanGFX
  {
  public:

    LGFX_Sprite(LovyanGFX* parent)
    : LovyanGFX()
    , _parent(parent)
//    , _bitwidth(0)
    {
      _panel = &_panel_sprite;
      setColorDepth(_write_conv.depth);
    }

    LGFX_INLINE void* getBuffer(void) const { return _panel_sprite.getBuffer(); }
    uint32_t bufferLength(void) const { return _panel_sprite.bufferLength(); }

    LGFX_Sprite()
    : LGFX_Sprite(nullptr)
    {}

    virtual ~LGFX_Sprite() {
      deleteSprite();
      deletePalette();
    }

    void deletePalette(void)
    {
      _palette_count = 0;
      _palette.release();
    }

    void deleteSprite(void)
    {
//      _bitwidth = 0;
      _clip_l = 0;
      _clip_t = 0;
      _clip_r = -1;
      _clip_b = -1;

      _panel_sprite.deleteSprite();
      _img = nullptr;
    }

    void setPsram( bool enabled )
    {
      if (_psram == enabled) return;
      _psram = enabled;
      deleteSprite();
    }

    void setBuffer(void* buffer, int32_t w, int32_t h, uint8_t bpp = 0)
    {
      deleteSprite();
      if (bpp != 0) _write_conv.setColorDepth((color_depth_t)bpp, hasPalette());

      _panel_sprite.setBuffer(buffer, w, h, &_write_conv);
      _img = _panel_sprite.getBuffer();

//      _bitwidth = (w + _write_conv.x_mask) & (~(uint32_t)_write_conv.x_mask);
      _sw = w;
      _clip_r = w - 1;
      _xpivot = w >> 1;

      _sh = h;
      _clip_b = h - 1;
      _ypivot = h >> 1;
    }

    void* createSprite(int32_t w, int32_t h)
    {
      _img = _panel_sprite.createSprite(w, h, &_write_conv, _psram);
      if (_img) {
        if (!_palette && 0 == _write_conv.bytes)
        {
          createPalette();
        }
      }
//      _bitwidth = (w + _write_conv.x_mask) & (~(uint32_t)_write_conv.x_mask);
      setRotation(getRotation());

      _sw = width();
      _clip_r = _sw - 1;
      _xpivot = _sw >> 1;

      _sh = height();
      _clip_b = _sh - 1;
      _ypivot = _sh >> 1;

      _clip_l = _clip_t = _sx = _sy = 0;


      return _img;
    }

#if defined (SdFat_h)

    inline void createFromBmp(SdBase<FsVolume> &fs, const char *path) { createFromBmpFile(fs, path); }
    void createFromBmpFile(SdBase<FsVolume> &fs, const char *path) {
      SdFatWrapper file;
      file.setFS(fs);
      createFromBmpFile(&file, path);
    }

#endif

#if defined (ARDUINO)
 #if defined (FS_H) || defined (__SEEED_FS__)

    inline void createFromBmp(fs::FS &fs, const char *path) { createFromBmpFile(fs, path); }
    void createFromBmpFile(fs::FS &fs, const char *path) {
      FileWrapper file;
      file.setFS(fs);
      createFromBmpFile(&file, path);
    }

 #endif

#elif defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (CONFIG_IDF_TARGET_ESP32S2) || defined (ESP_PLATFORM)

    void createFromBmpFile(const char *path) {
      FileWrapper file;
      createFromBmpFile(&file, path);
    }

#endif

    void createFromBmp(const uint8_t *bmp_data, uint32_t bmp_len = ~0u) {
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

    // create palette from RGB565(uint16_t) array
    bool createPalette(const uint16_t* colors, uint32_t count)
    {
      if (!create_palette()) return false;

      if (count > _palette_count) count = _palette_count;
      for (uint32_t i = 0; i < count; ++i) {
        _palette.img24()[i] = color_convert<bgr888_t, rgb565_t>(colors[i]);
      }
      return true;
    }

    // create palette from RGB888(uint32_t) array
    bool createPalette(const uint32_t* colors, uint32_t count)
    {
      if (!create_palette()) return false;

      if (count > _palette_count) count = _palette_count;
      for (uint32_t i = 0; i < count; ++i) {
        _palette.img24()[i] = color_convert<bgr888_t, rgb888_t>(colors[i]);
      }
      return true;
    }

    void setPaletteGrayscale(void)
    {
      if (!_palette) return;
      uint32_t k;
      switch (_write_conv.bits) {
      case 8: k = 0x010101; break;
      case 4: k = 0x111111; break;
      case 2: k = 0x555555; break;
      case 1: k = 0xFFFFFF; break;
      default: k = 1; break;
      }
      for (uint32_t i = 0; i < _palette_count; i++) {
        _palette.img24()[i] = i * k;
      }
    }

    void setBitmapColor(uint16_t fgcolor, uint16_t bgcolor)  // For 1bpp sprites
    {
      if (_palette) {
        _palette.img24()[0].set(color_convert<bgr888_t, rgb565_t>(bgcolor));
        _palette.img24()[1].set(color_convert<bgr888_t, rgb565_t>(fgcolor));
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
        if (_palette.img24()[res] == color) return res;
      } while (++res < _palette_count);
      return -1;
    }

    template<typename T> LGFX_INLINE
    void setPaletteColor(size_t index, T color) {
      if (!_palette || index >= _palette_count) return;
      rgb888_t c = convert_to_rgb888(color);
      _palette.img24()[index] = c;
    }

    void setPaletteColor(size_t index, const bgr888_t& rgb)
    {
      if (_palette && index < _palette_count) { _palette.img24()[index] = rgb; }
    }

    void setPaletteColor(size_t index, uint8_t r, uint8_t g, uint8_t b)
    {
      if (_palette && index < _palette_count) { _palette.img24()[index].set(r, g, b); }
    }

    LGFX_INLINE void* setColorDepth(uint8_t bpp) { return setColorDepth((color_depth_t)bpp); }
    void* setColorDepth(color_depth_t depth)
    {
      _panel_sprite.setColorDepth(depth);

      _write_conv.setColorDepth(depth, hasPalette());
      _read_conv = _write_conv;

      if (_panel_sprite.getBuffer() == nullptr) return nullptr;
      auto w = _panel_sprite._panel_width;
      auto h = _panel_sprite._panel_height;
      deleteSprite();
      deletePalette();
      return createSprite(w, h);
    }

    uint32_t readPixelValue(int32_t x, int32_t y) { return _panel_sprite.readPixelValue(x, y); }

    template<typename T>
    LGFX_INLINE void fillSprite (const T& color) { fillScreen(color); }

    template<typename T>
    LGFX_INLINE void pushSprite(                int32_t x, int32_t y, const T& transp) { push_sprite(_parent, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T>
    LGFX_INLINE void pushSprite(LovyanGFX* dst, int32_t x, int32_t y, const T& transp) { push_sprite(    dst, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    LGFX_INLINE void pushSprite(                int32_t x, int32_t y) { push_sprite(_parent, x, y); }
    LGFX_INLINE void pushSprite(LovyanGFX* dst, int32_t x, int32_t y) { push_sprite(    dst, x, y); }

    template<typename T> void pushRotated(                float angle, const T& transp) { push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushRotated(LovyanGFX* dst, float angle, const T& transp) { push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
                         void pushRotated(                float angle                 ) { push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f); }
                         void pushRotated(LovyanGFX* dst, float angle                 ) { push_rotate_zoom(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f); }

    template<typename T> void pushRotatedWithAA(                float angle, const T& transp) { push_rotate_zoom_aa(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushRotatedWithAA(LovyanGFX* dst, float angle, const T& transp) { push_rotate_zoom_aa(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f, _write_conv.convert(transp) & _write_conv.colormask); }
                         void pushRotatedWithAA(                float angle                 ) { push_rotate_zoom_aa(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, 1.0f, 1.0f); }
                         void pushRotatedWithAA(LovyanGFX* dst, float angle                 ) { push_rotate_zoom_aa(dst    , dst    ->getPivotX(), dst    ->getPivotY(), angle, 1.0f, 1.0f); }

    template<typename T> void pushRotateZoom(                                          float angle, float zoom_x, float zoom_y, const T& transp) { push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushRotateZoom(LovyanGFX* dst                          , float angle, float zoom_x, float zoom_y, const T& transp) { push_rotate_zoom(    dst,     dst->getPivotX(),     dst->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushRotateZoom(                float dst_x, float dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushRotateZoom(LovyanGFX* dst, float dst_x, float dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
                         void pushRotateZoom(                                          float angle, float zoom_x, float zoom_y)                  { push_rotate_zoom(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y); }
                         void pushRotateZoom(LovyanGFX* dst                          , float angle, float zoom_x, float zoom_y)                  { push_rotate_zoom(    dst,     dst->getPivotX(),     dst->getPivotY(), angle, zoom_x, zoom_y); }
                         void pushRotateZoom(                float dst_x, float dst_y, float angle, float zoom_x, float zoom_y)                  { push_rotate_zoom(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y); }
                         void pushRotateZoom(LovyanGFX* dst, float dst_x, float dst_y, float angle, float zoom_x, float zoom_y)                  { push_rotate_zoom(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y); }

    template<typename T> void pushRotateZoomWithAA(                                          float angle, float zoom_x, float zoom_y, const T& transp) { push_rotate_zoom_aa(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushRotateZoomWithAA(LovyanGFX* dst                          , float angle, float zoom_x, float zoom_y, const T& transp) { push_rotate_zoom_aa(    dst,     dst->getPivotX(),     dst->getPivotY(), angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushRotateZoomWithAA(                float dst_x, float dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { push_rotate_zoom_aa(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushRotateZoomWithAA(LovyanGFX* dst, float dst_x, float dst_y, float angle, float zoom_x, float zoom_y, const T& transp) { push_rotate_zoom_aa(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y, _write_conv.convert(transp) & _write_conv.colormask); }
                         void pushRotateZoomWithAA(                                          float angle, float zoom_x, float zoom_y)                  { push_rotate_zoom_aa(_parent, _parent->getPivotX(), _parent->getPivotY(), angle, zoom_x, zoom_y); }
                         void pushRotateZoomWithAA(LovyanGFX* dst                          , float angle, float zoom_x, float zoom_y)                  { push_rotate_zoom_aa(    dst,     dst->getPivotX(),     dst->getPivotY(), angle, zoom_x, zoom_y); }
                         void pushRotateZoomWithAA(                float dst_x, float dst_y, float angle, float zoom_x, float zoom_y)                  { push_rotate_zoom_aa(_parent,                dst_x,                dst_y, angle, zoom_x, zoom_y); }
                         void pushRotateZoomWithAA(LovyanGFX* dst, float dst_x, float dst_y, float angle, float zoom_x, float zoom_y)                  { push_rotate_zoom_aa(    dst,                dst_x,                dst_y, angle, zoom_x, zoom_y); }

    template<typename T> void pushAffine(                float matrix[6], const T& transp) { push_affine(_parent, matrix, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushAffine(LovyanGFX* dst, float matrix[6], const T& transp) { push_affine(    dst, matrix, _write_conv.convert(transp) & _write_conv.colormask); }
                         void pushAffine(                float matrix[6])                  { push_affine(_parent, matrix); }
                         void pushAffine(LovyanGFX* dst, float matrix[6])                  { push_affine(    dst, matrix); }

    template<typename T> void pushAffineWithAA(                float matrix[6], const T& transp) { push_affine_aa(_parent, matrix, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T> void pushAffineWithAA(LovyanGFX* dst, float matrix[6], const T& transp) { push_affine_aa(    dst, matrix, _write_conv.convert(transp) & _write_conv.colormask); }
                         void pushAffineWithAA(                float matrix[6])                  { push_affine_aa(_parent, matrix); }
                         void pushAffineWithAA(LovyanGFX* dst, float matrix[6])                  { push_affine_aa(    dst, matrix); }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:

    Panel_Sprite _panel_sprite;
    union
    {
      void* _img;
      uint8_t* _img8;
      uint16_t* _img16;
      lgfx::bgr888_t* _img24;
    };

    LovyanGFX* _parent;

    SpriteBuffer _palette;
//    int32_t _bitwidth;

    bool _psram = false;

    bool create_palette(void)
    {
      if (_write_conv.bits > 8) return false;

      deletePalette();

      size_t palettes = 1 << _write_conv.bits;
      _palette.reset(palettes * sizeof(bgr888_t), AllocationSource::Normal);
      if (!_palette) {
        _write_conv.setColorDepth(_write_conv.depth, false);
        return false;
      }
      _palette_count = palettes;
      _write_conv.setColorDepth(_write_conv.depth, true);
      return true;
    }

    void createFromBmpFile(DataWrapper* file, const char *path) {
      file->need_transaction = false;
      if (file->open(path)) {
        create_from_bmp(file);
        file->close();
      }
    }

    bool create_from_bmp(DataWrapper* data)
    {
      bitmap_header_t bmpdata;

      if (!bmpdata.load_bmp_header(data)
       || ( bmpdata.biCompression > 3)) {
        return false;
      }
      uint32_t seekOffset = bmpdata.bfOffBits;
      uint_fast16_t bpp = bmpdata.biBitCount; // 24 bcBitCount 24=RGB24bit
      setColorDepth(bpp < 32 ? bpp : 24);
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
          _palette.img24()[i].set(color_convert<bgr888_t, argb8888_t>(palette[i].get()));
        }
        delete[] palette;
      }

      data->seek(seekOffset);

      auto bitwidth = _panel_sprite._bitwidth;

      size_t buffersize = ((w * bpp + 31) >> 5) << 2;  // readline 4Byte align.
      auto lineBuffer = (uint8_t*)alloca(buffersize);
      if (bpp <= 8) {
        do {
          if (bmpdata.biCompression == 1) {
            bmpdata.load_bmp_rle8(data, lineBuffer, w);
          } else
          if (bmpdata.biCompression == 2) {
            bmpdata.load_bmp_rle4(data, lineBuffer, w);
          } else {
            data->read(lineBuffer, buffersize);
          }
          memcpy(&_img8[y * bitwidth * bpp >> 3], lineBuffer, (w * bpp + 7) >> 3);
          y += flow;
        } while (--h);
      } else if (bpp == 16) {
        do {
          data->read(lineBuffer, buffersize);
          auto img = (uint16_t*)(&_img8[y * bitwidth * bpp >> 3]);
          y += flow;
          for (size_t i = 0; i < w; ++i)
          {
            img[i] = (lineBuffer[i << 1] << 8) + lineBuffer[(i << 1) + 1];
          }
        } while (--h);
      } else if (bpp == 24) {
        do {
          data->read(lineBuffer, buffersize);
          auto img = &_img8[y * bitwidth * bpp >> 3];
          y += flow;
          for (size_t i = 0; i < w; ++i) {
            img[i * 3    ] = lineBuffer[i * 3 + 2];
            img[i * 3 + 1] = lineBuffer[i * 3 + 1];
            img[i * 3 + 2] = lineBuffer[i * 3    ];
          }
        } while (--h);
      } else if (bpp == 32) {
        do {
          data->read(lineBuffer, buffersize);
          auto img = &_img8[y * bitwidth * 3];
          y += flow;
          for (size_t i = 0; i < w; ++i) {
            img[i * 3    ] = lineBuffer[(i << 2) + 2];
            img[i * 3 + 1] = lineBuffer[(i << 2) + 1];
            img[i * 3 + 2] = lineBuffer[(i << 2) + 0];
          }
        } while (--h);
      }
      return true;
    }

    void push_sprite(LovyanGFX* dst, int32_t x, int32_t y, uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      pixelcopy_t p(_img, dst->getColorDepth(), getColorDepth(), dst->hasPalette(), _palette, transp);
      dst->pushImage(x, y, _panel_sprite._panel_width, _panel_sprite._panel_height, &p, _panel_sprite.getSpriteBuffer()->use_dma()); // DMA disable with use SPIRAM
    }

    void push_rotate_zoom(LovyanGFX* dst, float x, float y, float angle, float zoom_x, float zoom_y, uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      dst->pushImageRotateZoom(x, y, _xpivot, _ypivot, angle, zoom_x, zoom_y, _panel_sprite._panel_width, _panel_sprite._panel_height, _img, transp, getColorDepth(), _palette.img24());
    }

    void push_rotate_zoom_aa(LovyanGFX* dst, float x, float y, float angle, float zoom_x, float zoom_y, uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      dst->pushImageRotateZoomWithAA(x, y, _xpivot, _ypivot, angle, zoom_x, zoom_y, _panel_sprite._panel_width, _panel_sprite._panel_height, _img, transp, getColorDepth(), _palette.img24());
    }

    void push_affine(LovyanGFX* dst, float matrix[6], uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      dst->pushImageAffine(matrix, _panel_sprite._panel_width, _panel_sprite._panel_height, _img, transp, getColorDepth(), _palette.img24());
    }

    void push_affine_aa(LovyanGFX* dst, float matrix[6], uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      dst->pushImageAffineWithAA(matrix, _panel_sprite._panel_width, _panel_sprite._panel_height, _img, transp, getColorDepth(), _palette.img24());
    }

    RGBColor* getPalette_impl(void) const override { return _palette.img24(); }

  };

//----------------------------------------------------------------------------
#undef LGFX_INLINE

 }
}

using LGFX_Sprite = lgfx::LGFX_Sprite;
