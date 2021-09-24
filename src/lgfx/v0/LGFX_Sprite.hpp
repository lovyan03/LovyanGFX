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
#ifndef LGFX_SPRITE_HPP_
#define LGFX_SPRITE_HPP_

#include <algorithm>
#include <assert.h>

#include "LGFXBase.hpp"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  class LGFX_Sprite : public LovyanGFX
  {
  public:

    LGFX_Sprite(LovyanGFX* parent)
    : LovyanGFX()
    , _parent(parent)
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

    __attribute__ ((always_inline)) inline void* getBuffer(void) const { return _img.get(); }
    uint32_t bufferLength(void) const { return (_bitwidth * _write_conv.bits >> 3) * _height; }

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
      _bitwidth = 0;
      _width = 0;
      _height = 0;
      _clip_l = 0;
      _clip_t = 0;
      _clip_r = -1;
      _clip_b = -1;
      _sx = 0;
      _sy = 0;
      _sw = 0;
      _sh = 0;
      _xs = 0;
      _xe = 0;
      _ys = 0;
      _ye = 0;
      _xptr = 0;
      _yptr = 0;
      _index = 0;

      _img.release();
    }

    void setPsram( bool enabled )
    {
      _psram = enabled;
    }

    void setBuffer(void* buffer, int32_t w, int32_t h, uint8_t bpp = 0)
    {
      deleteSprite();
      if (bpp != 0) _write_conv.setColorDepth((color_depth_t)bpp, hasPalette());

      _img.reset(buffer);
      _bitwidth = (w + _write_conv.x_mask) & (~(uint32_t)_write_conv.x_mask);
      _sw = _width = w;
      _clip_r = _xe = w - 1;
      _xpivot = w >> 1;

      _sh = _height = h;
      _clip_b = _ye = h - 1;
      _ypivot = h >> 1;
    }

    void* createSprite(int32_t w, int32_t h)
    {
      if (w < 1 || h < 1) return nullptr;

      _bitwidth = (w + _write_conv.x_mask) & (~(uint32_t)_write_conv.x_mask);
      size_t len = h * (_bitwidth * _write_conv.bits >> 3) + 1;

      if (!_img || len != bufferLength())
      {
        _img.reset(len, _psram ? AllocationSource::Psram : AllocationSource::Dma);

        if (!_img)
        {
          deleteSprite();
          return nullptr;
        }
      }
      memset(_img, 0, len);
      if (!_palette && 0 == _write_conv.bytes)
      {
        createPalette();
      }
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
        _palette.img24()[i] = convert_rgb565_to_bgr888(colors[i]);
      }
      return true;
    }

    // create palette from RGB888(uint32_t) array
    bool createPalette(const uint32_t* colors, uint32_t count)
    {
      if (!create_palette()) return false;

      if (count > _palette_count) count = _palette_count;
      for (uint32_t i = 0; i < count; ++i) {
        _palette.img24()[i] = convert_rgb888_to_bgr888(colors[i]);
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
        _palette.img24()[i] = i * k;
      }
    }

    void setBitmapColor(uint16_t fgcolor, uint16_t bgcolor)  // For 1bpp sprites
    {
      if (_palette) {
        _palette.img24()[0] = *(rgb565_t*)&bgcolor;
        _palette.img24()[1] = *(rgb565_t*)&fgcolor;
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

    template<typename T> __attribute__ ((always_inline)) inline 
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

    __attribute__ ((always_inline)) inline void* setColorDepth(uint8_t bpp) { return setColorDepth((color_depth_t)bpp); }
    void* setColorDepth(color_depth_t depth)
    {
      _write_conv.setColorDepth(depth, hasPalette()) ;
      _read_conv = _write_conv;

      if (_img == nullptr) return nullptr;
      deleteSprite();
      deletePalette();
      return createSprite(_width, _height);
    }

    uint32_t readPixelValue(int32_t x, int32_t y)
    {
      auto bits = _read_conv.bits;
      if (bits >= 8) {
        int32_t index = x + y * _bitwidth;
        if (bits == 8) {
          return _img.img8()[index];
        } else if (bits == 16) {
          return _img.img16()[index];
        } else {
          return (uint32_t)_img.img24()[index];
        }
      } else {
        int32_t index = (x + y * _bitwidth) * bits;
        uint8_t mask = (1 << bits) - 1;
        return (_img.img8()[index >> 3] >> (-(index + bits) & 7)) & mask;
      }
    }

    template<typename T>
    __attribute__ ((always_inline)) inline void fillSprite (const T& color) { fillScreen(color); }

    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(                int32_t x, int32_t y, const T& transp) { push_sprite(_parent, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    template<typename T>
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, int32_t x, int32_t y, const T& transp) { push_sprite(    dst, x, y, _write_conv.convert(transp) & _write_conv.colormask); }
    __attribute__ ((always_inline)) inline void pushSprite(                int32_t x, int32_t y) { push_sprite(_parent, x, y); }
    __attribute__ ((always_inline)) inline void pushSprite(LovyanGFX* dst, int32_t x, int32_t y) { push_sprite(    dst, x, y); }

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

    enum AllocationSource
    {
      Normal,
      Dma,
      Psram,
      Preallocated,
    };

    class SpriteBuffer
    {
    private:
      size_t _length;
      AllocationSource _source;
      uint8_t* _buffer;

    public:
      SpriteBuffer(void) : _length(0), _source(Dma), _buffer(nullptr) {}

      SpriteBuffer(size_t length, AllocationSource source = AllocationSource::Dma) : _length(0), _source(source), _buffer(nullptr)
      {
        if (length)
        {
          assert (source != AllocationSource::Preallocated);
          this->reset(length, source);
        }
      }

      SpriteBuffer(uint8_t* buffer, size_t length) : _length(length), _source(AllocationSource::Preallocated), _buffer(buffer)
      {
      }

      SpriteBuffer(const SpriteBuffer& rhs) : _buffer(nullptr)
      {
        if ( rhs._source == AllocationSource::Preallocated )
        {
          this->_buffer = rhs._buffer;
          this->_length = rhs._length;
          this->_source = rhs._source;
        }
        else
        {
          this->reset(rhs._length, rhs._source);
          if( _buffer != nullptr && rhs._buffer != nullptr )
          {
            std::copy(rhs._buffer, rhs._buffer + _length, _buffer);
          }
        }
      }

      SpriteBuffer(SpriteBuffer&& rhs) : _buffer(nullptr)
      {
        if ( rhs._source == AllocationSource::Preallocated ) {
          this->_buffer = rhs._buffer;
          this->_length = rhs._length;
          this->_source = rhs._source;
        }
        else {
          this->reset(rhs._length, rhs._source);
          if( _buffer != nullptr && rhs._buffer != nullptr ) {
            std::copy(rhs._buffer, rhs._buffer + _length, _buffer);
            rhs.release();
          }
        }
      }

      SpriteBuffer& operator=(const SpriteBuffer& rhs)
      {
        if ( rhs._source == AllocationSource::Preallocated ) {
          this->_buffer = rhs._buffer;
          this->_length = rhs._length;
          this->_source = rhs._source;
        }
        else {
          this->reset(rhs._length, rhs._source);
          if ( _buffer != nullptr && rhs._buffer != nullptr ) {
            std::copy(rhs._buffer, rhs._buffer + _length, _buffer);
          }
        }
        return *this;
      }

      SpriteBuffer& operator=(SpriteBuffer&& rhs)
      {
        if( rhs._source == AllocationSource::Preallocated ) {
          this->_buffer = rhs._buffer;
          this->_length = rhs._length;
          this->_source = rhs._source;
        }
        else {
          this->reset(rhs._length, rhs._source);
          if( _buffer != nullptr && rhs._buffer != nullptr ) {
            std::copy(rhs._buffer, rhs._buffer + _length, _buffer);
            rhs.release();
          }
        }
        return *this;
      }

      operator uint8_t*() const { return _buffer; }
      operator bool() const { return _buffer != nullptr; }

      uint8_t* get() const { return _buffer; }
      uint8_t* img8() const { return _buffer; }
      uint16_t* img16() const { return reinterpret_cast<uint16_t*>(_buffer); }
      bgr888_t* img24() const { return reinterpret_cast<bgr888_t*>(_buffer); }

      void reset(void* buffer)
      {
        this->release();
        _source = AllocationSource::Preallocated;
        _buffer = reinterpret_cast<uint8_t*>(buffer);
        _length = 0;
      }

      void reset(size_t length, AllocationSource source)
      {
        this->release();
        void* buffer = nullptr;
        _source = source;
        switch (source)
        {
          default:
          case AllocationSource::Normal:
            buffer = heap_alloc(length);
            break;
          case AllocationSource::Dma:
            buffer = heap_alloc_dma(length);
            break;
          case AllocationSource::Psram:
            buffer = heap_alloc_psram(length);
            if (!buffer)
            {
              _source = AllocationSource::Dma;
              buffer = heap_alloc_dma(length);
            }
            break;
        }
        _buffer = reinterpret_cast<uint8_t*>(buffer);
        if ( _buffer != nullptr ) {
          _length = length;
        }
      }

      void release() {
        _length = 0;
        if ( _buffer != nullptr ) {
          if (_source != AllocationSource::Preallocated)
          {
            heap_free(_buffer);
          }
          _buffer = nullptr;
        }
      }

      bool use_dma() { return _source == AllocationSource::Dma; }
      bool use_memcpy() { return _source != AllocationSource::Psram; }
    };

    LovyanGFX* _parent;

    SpriteBuffer _img;
    SpriteBuffer _palette;

    int32_t _bitwidth;
    int32_t _xptr;
    int32_t _yptr;
    int32_t _xs;
    int32_t _xe;
    int32_t _ys;
    int32_t _ye;
    int32_t _index;
    bool _psram = false;

    bool create_palette(void)
    {
      if (_write_conv.depth > 8) return false;

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

    void createFromBmpFile(FileWrapper* file, const char *path) {
      file->need_transaction = false;
      if (file->open(path, "r")) {
        create_from_bmp(file);
        file->close();
      }
    }

    bool create_from_bmp(DataWrapper* data) {
      bitmap_header_t bmpdata;

      if (!bmpdata.load_bmp_header(data)
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
          _palette.img24()[i] = palette[i];
        }
        delete[] palette;
      }

      data->seek(seekOffset);

      size_t buffersize = ((w * bpp + 31) >> 5) << 2;  // readline 4Byte align.
      uint8_t lineBuffer[buffersize];  // readline 4Byte align.
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
          memcpy(&_img[y * _bitwidth * bpp >> 3], lineBuffer, (w * bpp + 7) >> 3);
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
          auto img = &_img.img8()[y * _bitwidth * 3];
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
      dst->pushImage(x, y, _width, _height, &p, _img.use_dma()); // DMA disable with use SPIRAM
    }

    void push_rotate_zoom(LovyanGFX* dst, float x, float y, float angle, float zoom_x, float zoom_y, uint32_t transp = ~0)
    {
      dst->pushImageRotateZoom(x, y, _xpivot, _ypivot, angle, zoom_x, zoom_y, _width, _height, _img, transp, getColorDepth(), _palette.img24());
    }

    void push_rotate_zoom_aa(LovyanGFX* dst, float x, float y, float angle, float zoom_x, float zoom_y, uint32_t transp = ~0)
    {
      dst->pushImageRotateZoomWithAA(x, y, _xpivot, _ypivot, angle, zoom_x, zoom_y, _width, _height, _img, transp, getColorDepth(), _palette.img24());
    }

    void push_affine(LovyanGFX* dst, float matrix[6], uint32_t transp = ~0)
    {
      dst->pushImageAffine(matrix, _width, _height, _img, transp, getColorDepth(), _palette.img24());
    }

    void push_affine_aa(LovyanGFX* dst, float matrix[6], uint32_t transp = ~0)
    {
      dst->pushImageAffineWithAA(matrix, _width, _height, _img, transp, getColorDepth(), _palette.img24());
    }

    void set_window(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      if (xs > xe) std::swap(xs, xe);
      if (ys > ye) std::swap(ys, ye);
      if ((xe >= 0) && (ye >= 0) && (xs < _width) && (ys < _height))
      {
        xs = std::max<int32_t>(xs, 0);
        ys = std::max<int32_t>(ys, 0);
        _xptr = _xs = xs;
        _yptr = _ys = ys;
        _index = xs + ys * _bitwidth;
        _xe = std::min(xe, _width  - 1);
        _ye = std::min(ye, _height - 1);
      }
      else 
      {
        _xptr = _xs = _xe = 0;
        _yptr = _ys = _ye = _height;
        _index = _height * _bitwidth;
      }
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
          _img.img8()[index] = _color.raw0;
        } else if (bits == 16) {
          _img.img16()[index] = _color.rawL;
        } else {
          _img.img24()[index] = *(bgr888_t*)&_color;
        }
      } else {
        int32_t index = (x + y * _bitwidth) * bits;
        uint8_t* dst = &_img.img8()[index >> 3];
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
            auto img = &_img.img16()[index];
            do { *img = c;  img += bw; } while (--h);
          } else {  // if (_write_conv.bytes == 3)
            auto c = _color;
            auto img = &_img.img24()[index];
            do { img->r = c.raw0; img->g = c.raw1; img->b = c.raw2; img += bw; } while (--h);
          }
        } else {
          uint32_t bytes = bits >> 3;
          int32_t bw = _bitwidth;
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
            if (_img.use_memcpy()) {
              if (w == bw) {
                memset_multi(dst, color, bytes, w * h);
              } else {
                memset_multi(dst, color, bytes, w);
                while (--h) {
                  memcpy(dst + add_dst, dst, len);
                  dst += add_dst;
                }
              }
            } else {
              uint8_t linebuf[len];
              memset_multi(linebuf, color, bytes, w);
              do {
                memcpy(dst, linebuf, len);
                dst += add_dst;
              } while (--h);
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
          uint8_t* dst = &_img.img8()[index * bits >> 3];
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
        if (_img.use_memcpy()) {
          uint32_t color = _color.raw;
          int32_t ll;
          int32_t index = _index;
          do {
            ll = std::min(_xe - _xptr + 1, length);
            memset_multi(&_img[index * bytes], color, bytes, ll);
            index = ptr_advance(ll);
          } while (length -= ll);
        } else {
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
        }
      }
    }

    void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) override
    {
      if (_write_conv.bits < 8) {
        pixelcopy_t param(_img, _write_conv.depth, _write_conv.depth);
        param.src_bitwidth = _bitwidth;
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
        uint8_t* src = &_img.img8()[(src_x + (src_y + pos) * _bitwidth) * _write_conv.bytes];
        uint8_t* dst = &_img.img8()[(dst_x + (dst_y + pos) * _bitwidth) * _write_conv.bytes];
        if (_img.use_memcpy()) {
          do {
            memmove(dst, src, len);
            src += add;
            dst += add;
          } while (--h);
        } else {
          uint8_t buf[len];
          do {
            memcpy(buf, src, len);
            memcpy(dst, buf, len);
            dst += add;
            src += add;
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
          memcpy(d, &_img.img8()[(x + y * bw) * b], w * b);
          d += w * b;
        } while (++y != h);
      } else {
        param->src_bitwidth = _bitwidth;
        param->src_data = _img;
        int32_t dstindex = 0;
        do {
          param->src_x = x;
          param->src_y = y;
          dstindex = param->fp_copy(dst, dstindex, dstindex + w, param);
        } while (++y != h);
      }
    }

    void pushImage_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param, bool) override
    {
      auto sx = param->src_x;
      if (param->transp == ~0u && param->no_convert && _img.use_memcpy()) {
        auto bits = param->src_bits;
        uint_fast8_t mask = (bits == 1) ? 7
                               : (bits == 2) ? 3
                                             : 1;
        if (0 == (bits & 7) || ((sx & mask) == (x & mask) && (w == this->_width || 0 == (w & mask)))) {
          auto bw = _bitwidth * bits >> 3;
          auto dd = &_img[bw * y];
          auto sw = param->src_bitwidth * bits >> 3;
          auto sd = &((uint8_t*)param->src_data)[param->src_y * sw];
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
      y *= _bitwidth;
      do {
        int32_t pos = x + y;
        int32_t end = pos + w;
        y += _bitwidth;
        while (end != (pos = param->fp_copy(_img, pos, end, param))) {
          if ( end == (pos = param->fp_skip(      pos, end, param))) break;
        }
        param->src_x = sx;
        param->src_y++;
      } while (--h);
    }

    void pushImageARGB_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param) override
    {
      int32_t pos = x + y * _bitwidth;
      int32_t end = pos + w;
      param->fp_copy(_img.img8(), pos, end, param);
      while (--h)
      {
        pos += _bitwidth;
        end = pos + w;
        param->src_y++;
        param->fp_copy(_img.img8(), pos, end, param);
      }
    }

    void writePixelsDMA_impl(const void* data, int32_t length) override
    {
      auto src = static_cast<const uint8_t*>(data);
      auto k = _bitwidth * _write_conv.bits >> 3;
      int32_t linelength;
      do {
        linelength = std::min<int32_t>(_xe - _xptr + 1, length);
        auto len = linelength * _write_conv.bits >> 3;
        memcpy(&_img.img8()[(_xptr * _write_conv.bits >> 3) + _yptr * k], src, len);
        src += len;
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    void writePixels_impl(int32_t length, pixelcopy_t* param) override
    {
      auto k = _bitwidth * _write_conv.bits >> 3;
      int32_t linelength;
      do {
        linelength = std::min<int32_t>(_xe - _xptr + 1, length);
        param->fp_copy(&_img.img8()[_yptr * k], _xptr, _xptr + linelength, param);
        ptr_advance(linelength);
      } while (length -= linelength);
    }

    bool isReadable_impl(void) const override { return true; }
    int_fast8_t getRotation_impl(void) const override { return 0; }
    void beginTransaction_impl(void) override {}
    void endTransaction_impl(void) override {}
    void initDMA_impl(void) override {}
    void waitDMA_impl(void) override {}
    bool dmaBusy_impl(void) override { return false; }
    RGBColor* getPalette_impl(void) const override { return _palette.img24(); }

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

    static void memset_multi(uint8_t* buf, uint32_t c, size_t size, size_t length)
    {
      size_t l = length;
      if (l & ~0xF) {
        while ((l >>= 1) & ~0xF);
        ++l;
      }
      size_t len = l * size;
      length = (length * size) - len;
      uint8_t* dst = buf;
      if (size == 2) {
        do { // 2byte speed tweak
          *(uint16_t*)dst = c;
          dst += 2;
        } while (--l);
      } else {
        do {
          size_t i = 0;
          do {
            *dst++ = *(((uint8_t*)&c) + i);
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
  };
//----------------------------------------------------------------------------
 }
}

using LGFX_Sprite = lgfx::LGFX_Sprite;

#endif
