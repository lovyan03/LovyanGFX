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

#include <algorithm>
#include <cassert>

#include "LGFXBase.hpp"
#include "misc/SpriteBuffer.hpp"
#include "misc/bitmap.hpp"
#include "Panel.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  class LGFX_Sprite;

  struct Panel_Sprite : public IPanel
  {
    friend LGFX_Sprite;

    Panel_Sprite(void) { _start_count = INT32_MAX; }

    void beginTransaction(void) override {}
    void endTransaction(void) override {}
    void setInvert(bool invert) override {}
    void setSleep(bool flg_sleep) override {}
    void setPowerSave(bool flg_partial) override {}
    void writeCommand(std::uint32_t cmd, std::uint_fast8_t length) override {}
    void writeData(std::uint32_t data, std::uint_fast8_t length) override {}
    void initDMA(void) override {}
    void waitDMA(void) override {}
    bool dmaBusy(void) override { return false; }
    void waitDisplay(void) override {}
    bool displayBusy(void) override { return false; }
    void display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h) override {}
    bool isReadable(void) const override { return true; }
    bool isBusShared(void) const override { return false; }

    std::uint32_t readCommand(std::uint_fast8_t cmd, std::uint_fast8_t index = 0, std::uint_fast8_t length = 4) override { return 0; }
    std::uint32_t readData(std::uint_fast8_t index = 0, std::uint_fast8_t length = 4) override { return 0; }


    void setBuffer(void* buffer, std::int32_t w, std::int32_t h, color_conv_t* conv);
    void deleteSprite(void);
    void* createSprite(std::int32_t w, std::int32_t h, color_conv_t* conv, bool psram);

    __attribute__ ((always_inline)) inline void* getBuffer(void) const { return _img.get(); }
    __attribute__ ((always_inline)) inline const SpriteBuffer* getSpriteBuffer(void) const { return &_img; }
    __attribute__ ((always_inline)) inline std::uint32_t bufferLength(void) const { return (_bitwidth * _write_bits >> 3) * _panel_height; }


    color_depth_t setColorDepth(color_depth_t depth) override;
    void setRotation(std::uint_fast8_t r) override;

    void setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) override;
    void drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor) override;
    void writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t raw_color) override;
    void writeBlock(std::uint32_t rawcolor, std::uint32_t len) override;
    void writePixels(pixelcopy_t* param, std::uint32_t len) override;
    void writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool) override;
    void writeImageARGB(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param) override;

    void readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param) override;
    void copyRect(std::uint_fast16_t dst_x, std::uint_fast16_t dst_y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint_fast16_t src_x, std::uint_fast16_t src_y) override;

    std::uint32_t readPixelValue(std::uint_fast16_t x, std::uint_fast16_t y);

  protected:
    void _rotate_pixelcopy(std::uint_fast16_t& x, std::uint_fast16_t& y, std::uint_fast16_t& w, std::uint_fast16_t& h, pixelcopy_t* param, std::uint32_t& nextx, std::uint32_t& nexty);

    SpriteBuffer _img;

    std::uint_fast16_t _xpos;
    std::uint_fast16_t _ypos;
    std::uint_fast16_t _panel_width;   // rotationしていない状態の幅
    std::uint_fast16_t _panel_height;  // rotationしていない状態の高さ
    std::uint_fast16_t _bitwidth;
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

    __attribute__ ((always_inline)) inline void* getBuffer(void) const { return _panel_sprite.getBuffer(); }
    std::uint32_t bufferLength(void) const { return _panel_sprite.bufferLength(); }

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

    void setBuffer(void* buffer, std::int32_t w, std::int32_t h, std::uint8_t bpp = 0)
    {
      deleteSprite();
      if (bpp != 0) _write_conv.setColorDepth((color_depth_t)bpp, hasPalette());

      _panel_sprite.setBuffer(buffer, w, h, &_write_conv);
      _img = _panel_sprite.getBuffer();

//      _bitwidth = (w + _write_conv.x_mask) & (~(std::uint32_t)_write_conv.x_mask);
      _sw = w;
      _clip_r = w - 1;
      _xpivot = w >> 1;

      _sh = h;
      _clip_b = h - 1;
      _ypivot = h >> 1;
    }

    void* createSprite(std::int32_t w, std::int32_t h)
    {
      _img = _panel_sprite.createSprite(w, h, &_write_conv, _psram);
      if (_img) {
        if (!_palette && 0 == _write_conv.bytes)
        {
          createPalette();
        }
      }
//      _bitwidth = (w + _write_conv.x_mask) & (~(std::uint32_t)_write_conv.x_mask);
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

    void createFromBmp(const std::uint8_t *bmp_data, std::uint32_t bmp_len = ~0u) {
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
        _palette.img24()[i] = convert_rgb565_to_bgr888(colors[i]);
      }
      return true;
    }

    // create palette from RGB888(std::uint32_t) array
    bool createPalette(const std::uint32_t* colors, std::uint32_t count)
    {
      if (!create_palette()) return false;

      if (count > _palette_count) count = _palette_count;
      for (std::uint32_t i = 0; i < count; ++i) {
        _palette.img24()[i] = convert_rgb888_to_bgr888(colors[i]);
      }
      return true;
    }

    void setPaletteGrayscale(void)
    {
      if (!_palette) return;
      std::uint32_t k;
      switch (_write_conv.bits) {
      case 8: k = 0x010101; break;
      case 4: k = 0x111111; break;
      case 2: k = 0x555555; break;
      case 1: k = 0xFFFFFF; break;
      default: k = 1; break;
      }
      for (std::uint32_t i = 0; i < _palette_count; i++) {
        _palette.img24()[i] = i * k;
      }
    }

    void setBitmapColor(std::uint16_t fgcolor, std::uint16_t bgcolor)  // For 1bpp sprites
    {
      if (_palette) {
        _palette.img24()[0] = *(rgb565_t*)&bgcolor;
        _palette.img24()[1] = *(rgb565_t*)&fgcolor;
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
      std::size_t res = 0;
      do {
        if (_palette.img24()[res] == color) return res;
      } while (++res < _palette_count);
      return -1;
    }

    template<typename T> __attribute__ ((always_inline)) inline
    void setPaletteColor(std::size_t index, T color) {
      if (!_palette || index >= _palette_count) return;
      rgb888_t c = convert_to_rgb888(color);
      _palette.img24()[index] = c;
    }

    void setPaletteColor(std::size_t index, const bgr888_t& rgb)
    {
      if (_palette && index < _palette_count) { _palette.img24()[index] = rgb; }
    }

    void setPaletteColor(std::size_t index, std::uint8_t r, std::uint8_t g, std::uint8_t b)
    {
      if (_palette && index < _palette_count) { _palette.img24()[index].set(r, g, b); }
    }

    __attribute__ ((always_inline)) inline void* setColorDepth(std::uint8_t bpp) { return setColorDepth((color_depth_t)bpp); }
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

    std::uint32_t readPixelValue(std::int32_t x, std::int32_t y) { return _panel_sprite.readPixelValue(x, y); }

    template<typename T>
    __attribute__ ((always_inline)) inline void fillSprite (const T& color) { fillScreen(color); }

    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(                std::int32_t x, std::int32_t y, const T& transp) { push_sprite(_parent, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, std::int32_t x, std::int32_t y, const T& transp) { push_sprite(    dst, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    __attribute__ ((always_inline)) inline void pushSprite(                std::int32_t x, std::int32_t y) { push_sprite(_parent, x, y); }
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, std::int32_t x, std::int32_t y) { push_sprite(    dst, x, y); }

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
//    std::int32_t _bitwidth;

    bool _psram = false;

    bool create_palette(void)
    {
      if (_write_conv.bits > 8) return false;

      deletePalette();

      std::size_t palettes = 1 << _write_conv.bits;
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
          _palette.img24()[i] = palette[i];
        }
        delete[] palette;
      }

      data->seek(seekOffset);

      auto bitwidth = _panel_sprite._bitwidth;

      std::size_t buffersize = ((w * bpp + 31) >> 5) << 2;  // readline 4Byte align.
      std::uint8_t lineBuffer[buffersize];  // readline 4Byte align.
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
          auto img = &_img8[y * bitwidth * bpp >> 3];
          y += flow;
          for (std::size_t i = 0; i < buffersize; ++i) {
            img[i] = lineBuffer[i ^ 1];
          }
        } while (--h);
      } else if (bpp == 24) {
        do {
          data->read(lineBuffer, buffersize);
          auto img = &_img8[y * bitwidth * bpp >> 3];
          y += flow;
          for (std::size_t i = 0; i < buffersize; i += 3) {
            img[i    ] = lineBuffer[i + 2];
            img[i + 1] = lineBuffer[i + 1];
            img[i + 2] = lineBuffer[i    ];
          }
        } while (--h);
      } else if (bpp == 32) {
        do {
          data->read(lineBuffer, buffersize);
          auto img = &_img8[y * bitwidth * 3];
          y += flow;
          for (std::size_t i = 0; i < buffersize; i += 4) {
            img[(i>>2)*3    ] = lineBuffer[i + 2];
            img[(i>>2)*3 + 1] = lineBuffer[i + 1];
            img[(i>>2)*3 + 2] = lineBuffer[i + 0];
          }
        } while (--h);
      }
      return true;
    }

    void push_sprite(LovyanGFX* dst, std::int32_t x, std::int32_t y, std::uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      pixelcopy_t p(_img, dst->getColorDepth(), getColorDepth(), dst->hasPalette(), _palette, transp);
      dst->pushImage(x, y, _panel_sprite._panel_width, _panel_sprite._panel_height, &p, _panel_sprite.getSpriteBuffer()->use_dma()); // DMA disable with use SPIRAM
    }

    void push_rotate_zoom(LovyanGFX* dst, float x, float y, float angle, float zoom_x, float zoom_y, std::uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      dst->pushImageRotateZoom(x, y, _xpivot, _ypivot, angle, zoom_x, zoom_y, _panel_sprite._panel_width, _panel_sprite._panel_height, _img, transp, getColorDepth(), _palette.img24());
    }

    void push_rotate_zoom_aa(LovyanGFX* dst, float x, float y, float angle, float zoom_x, float zoom_y, std::uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      dst->pushImageRotateZoomWithAA(x, y, _xpivot, _ypivot, angle, zoom_x, zoom_y, _panel_sprite._panel_width, _panel_sprite._panel_height, _img, transp, getColorDepth(), _palette.img24());
    }

    void push_affine(LovyanGFX* dst, float matrix[6], std::uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      dst->pushImageAffine(matrix, _panel_sprite._panel_width, _panel_sprite._panel_height, _img, transp, getColorDepth(), _palette.img24());
    }

    void push_affine_aa(LovyanGFX* dst, float matrix[6], std::uint32_t transp = pixelcopy_t::NON_TRANSP)
    {
      dst->pushImageAffineWithAA(matrix, _panel_sprite._panel_width, _panel_sprite._panel_height, _img, transp, getColorDepth(), _palette.img24());
    }

    RGBColor* getPalette_impl(void) const override { return _palette.img24(); }

  };

//----------------------------------------------------------------------------
 }
}

using LGFX_Sprite = lgfx::LGFX_Sprite;
