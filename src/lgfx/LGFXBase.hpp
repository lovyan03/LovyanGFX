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
#ifndef LGFX_BASE_HPP_
#define LGFX_BASE_HPP_

#include "lgfx_common.hpp"

namespace lgfx
{
  class LGFXBase
  {
  public:
    LGFXBase() {}
    virtual ~LGFXBase() {}

// color param format:
// rgb888 : std::uint32_t
// rgb565 : std::uint16_t & std::int16_t & int
// rgb332 : std::uint8_t
    __attribute__ ((always_inline)) inline void setColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) { _color.raw = _write_conv.convert(lgfx::color888(r,g,b)); }
    template<typename T> __attribute__ ((always_inline)) inline void setColor(T c) { _color.raw = _write_conv.convert(c); }
                         __attribute__ ((always_inline)) inline void setRawColor(std::uint32_t c) { _color.raw = c; }

    template<typename T> __attribute__ ((always_inline)) inline void setBaseColor(T c) { _base_rgb888 = convert_to_rgb888(c); }
    std::uint32_t getBaseColor(void) const { return _base_rgb888; }

                         inline void clear      ( void )          { _color.raw = 0;  fillRect(0, 0, _width, _height); }
    template<typename T> inline void clear      ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }
                         inline void fillScreen ( void )          {                  fillRect(0, 0, _width, _height); }
    template<typename T> inline void fillScreen ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }

    template<typename T> inline void pushBlock  ( const T& color, std::int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }

// Deprecated, use pushBlock()
    template<typename T> inline void pushColor  ( const T& color, std::int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }
    template<typename T> inline void pushColor  ( const T& color                ) {                          setColor(color); startWrite(); pushBlock_impl(1);      endWrite(); }


// AdafruitGFX compatible functions.
// However, startWrite and endWrite have an internal counter and are executed when the counter is 0.
// If you do not want to the counter, call the transaction function directly.
    __attribute__ ((always_inline)) inline void startWrite(void) {                           if (1 == ++_transaction_count) beginTransaction(); }
    __attribute__ ((always_inline)) inline void endWrite(void)   { if (_transaction_count) { if (0 == --_transaction_count) endTransaction(); } }
    template<typename T> inline void writePixel    ( std::int32_t x, std::int32_t y                                , const T& color) { setColor(color); writePixel    (x, y         ); }
    template<typename T> inline void writeFastVLine( std::int32_t x, std::int32_t y                , std::int32_t h, const T& color) { setColor(color); writeFastVLine(x, y   , h   ); }
                                void writeFastVLine( std::int32_t x, std::int32_t y                , std::int32_t h);
    template<typename T> inline void writeFastHLine( std::int32_t x, std::int32_t y, std::int32_t w                , const T& color) { setColor(color); writeFastHLine(x, y, w      ); }
                                void writeFastHLine( std::int32_t x, std::int32_t y, std::int32_t w);
    template<typename T> inline void writeFillRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T& color) { setColor(color); writeFillRect (x, y, w, h   ); }
                                void writeFillRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    template<typename T> inline void writeColor    ( const T& color, std::int32_t length) {      if (0 >= length) return; setColor(color);    pushBlock_impl(length); }
                         inline void writeRawColor ( std::uint32_t color, std::int32_t length) { if (0 >= length) return; setRawColor(color); pushBlock_impl(length); }

    template<typename T> inline void drawPixel     ( std::int32_t x, std::int32_t y                                           , const T& color) { setColor(color); drawPixel    (x, y         ); }
    template<typename T> inline void drawFastVLine ( std::int32_t x, std::int32_t y                , std::int32_t h           , const T& color) { setColor(color); drawFastVLine(x, y   , h   ); }
                                void drawFastVLine ( std::int32_t x, std::int32_t y                , std::int32_t h);
    template<typename T> inline void drawFastHLine ( std::int32_t x, std::int32_t y, std::int32_t w                           , const T& color) { setColor(color); drawFastHLine(x, y, w      ); }
                                void drawFastHLine ( std::int32_t x, std::int32_t y, std::int32_t w);
    template<typename T> inline void fillRect      ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h           , const T& color) { setColor(color); fillRect     (x, y, w, h   ); }
                                void fillRect      ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    template<typename T> inline void drawRect      ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h           , const T& color) { setColor(color); drawRect     (x, y, w, h   ); }
                                void drawRect      ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    template<typename T> inline void drawRoundRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r, const T& color) { setColor(color); drawRoundRect(x, y, w, h, r); }
                                void drawRoundRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r);
    template<typename T> inline void fillRoundRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r, const T& color) { setColor(color); fillRoundRect(x, y, w, h, r); }
                                void fillRoundRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r);
    template<typename T> inline void drawCircle    ( std::int32_t x, std::int32_t y                                , std::int32_t r, const T& color) { setColor(color); drawCircle   (x, y      , r); }
                                void drawCircle    ( std::int32_t x, std::int32_t y                                , std::int32_t r);
    template<typename T> inline void fillCircle    ( std::int32_t x, std::int32_t y                                , std::int32_t r, const T& color) { setColor(color); fillCircle   (x, y      , r); }
                                void fillCircle    ( std::int32_t x, std::int32_t y                                , std::int32_t r);
    template<typename T> inline void drawEllipse   ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry         , const T& color) { setColor(color); drawEllipse  (x, y, rx, ry ); }
                                void drawEllipse   ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry);
    template<typename T> inline void fillEllipse   ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry         , const T& color) { setColor(color); fillEllipse  (x, y, rx, ry ); }
                                void fillEllipse   ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry);
    template<typename T> inline void drawLine      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1                        , const T& color)  { setColor(color); drawLine(    x0, y0, x1, y1        ); }
                                void drawLine      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1);
    template<typename T> inline void drawTriangle  ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawTriangle(x0, y0, x1, y1, x2, y2); }
                                void drawTriangle  ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    template<typename T> inline void fillTriangle  ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); fillTriangle(x0, y0, x1, y1, x2, y2); }
                                void fillTriangle  ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    template<typename T> inline void drawBezier    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawBezier(x0, y0, x1, y1, x2, y2); }
                                void drawBezier    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    template<typename T> inline void drawBezierHelper(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawBezierHelper(x0, y0, x1, y1, x2, y2); }
                                void drawBezierHelper(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    template<typename T> inline void drawArc       ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1, const T& color) { setColor(color); drawArc( x, y, r0, r1, angle0, angle1); }
                                void drawArc       ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1);
    template<typename T> inline void fillArc       ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1, const T& color) { setColor(color); fillArc( x, y, r0, r1, angle0, angle1); }
                                void fillArc       ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1);
    template<typename T> inline void drawCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t cornername            , const T& color)  { setColor(color); drawCircleHelper(x, y, r, cornername    ); }
                                void drawCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t cornername);
    template<typename T> inline void fillCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t corners, std::int32_t delta, const T& color)  { setColor(color); fillCircleHelper(x, y, r, corners, delta); }
                                void fillCircleHelper(std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t corners, std::int32_t delta);

    template<typename T> inline void paint    ( std::int32_t x, std::int32_t y, const T& color) { setColor(color); paint(x, y); }
    template<typename T> inline void floodFill( std::int32_t x, std::int32_t y, const T& color) { setColor(color); paint(x, y); }
                         inline void floodFill( std::int32_t x, std::int32_t y                ) {                  paint(x, y); }

    __attribute__ ((always_inline)) inline static std::uint8_t  color332(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color332(r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint16_t color565(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color565(r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint32_t color888(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color888(r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint16_t swap565( std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::swap565( r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint32_t swap888( std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::swap888( r, g, b); }

    __attribute__ ((always_inline)) inline void setPivot(std::int16_t x, std::int16_t y) { _xpivot = x; _ypivot = y; }
    __attribute__ ((always_inline)) inline std::int16_t getPivotX(void) const { return _xpivot; }
    __attribute__ ((always_inline)) inline std::int16_t getPivotY(void) const { return _ypivot; }

    __attribute__ ((always_inline)) inline std::int32_t width        (void) const { return _width; }
    __attribute__ ((always_inline)) inline std::int32_t height       (void) const { return _height; }
    __attribute__ ((always_inline)) inline color_depth_t getColorDepth(void) const { return _write_conv.depth; }
    __attribute__ ((always_inline)) inline color_conv_t* getColorConverter(void) { return &_write_conv; }
    __attribute__ ((always_inline)) inline bgr888_t* getPalette(void) const { return _palette; }
    __attribute__ ((always_inline)) inline std::uint32_t getPaletteCount(void) const { return _palette_count; }
    __attribute__ ((always_inline)) inline std::int_fast8_t getRotation(void) const { return getRotation_impl(); }
    __attribute__ ((always_inline)) inline bool hasPalette    (void) const { return _palette_count; }
    __attribute__ ((always_inline)) inline bool isSPIShared(void) const { return _spi_shared; }
    __attribute__ ((always_inline)) inline bool isReadable(void) const { return isReadable_impl(); }
    __attribute__ ((always_inline)) inline bool getSwapBytes    (void) const { return _swapBytes; }
    __attribute__ ((always_inline)) inline void setSwapBytes(bool swap) { _swapBytes = swap; }

    __attribute__ ((always_inline)) inline void beginTransaction(void) { beginTransaction_impl(); }
    __attribute__ ((always_inline)) inline void endTransaction(void)   { endTransaction_impl(); }
    __attribute__ ((always_inline)) inline void initDMA(void)  { }  // TFT_eSPI compatible
    __attribute__ ((always_inline)) inline void waitDMA(void)  { waitDMA_impl(); }
    __attribute__ ((always_inline)) inline void setWindow(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye) { setWindow_impl(xs, ys, xe, ye); }

    void setSPIShared(bool shared) { _spi_shared = shared; }

    void setAddrWindow(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    void setClipRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    void getClipRect(std::int32_t *x, std::int32_t *y, std::int32_t *w, std::int32_t *h);
    void clearClipRect(void);

    template <typename T>
    void setScrollRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T& color) {
      _base_rgb888 = convert_to_rgb888(color);
      setScrollRect(x, y, w, h);
    }
    void setScrollRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    void getScrollRect(std::int32_t *x, std::int32_t *y, std::int32_t *w, std::int32_t *h);
    void clearScrollRect(void);

    __attribute__ ((always_inline))
    inline void drawPixel(std::int32_t x, std::int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;

      drawPixel_impl(x, y);
    }

    __attribute__ ((always_inline))
    inline void writePixel(std::int32_t x, std::int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return;

      writeFillRect_impl(x, y, 1, 1);
    }

    __attribute__ ((always_inline)) inline 
    void writeFillRectPreclipped(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
    {
      writeFillRect_impl(x, y, w, h);
    }

    __attribute__ ((always_inline)) inline
    void drawGradientHLine( std::int32_t x, std::int32_t y, std::int32_t w, bgr888_t colorstart, bgr888_t colorend ) {
      drawGradientLine( x, y, x + w - 1, y, colorstart, colorend );
    }

    __attribute__ ((always_inline)) inline
    void drawGradientVLine( std::int32_t x, std::int32_t y, std::int32_t h, bgr888_t colorstart, bgr888_t colorend ) {
      drawGradientLine( x, y, x, y + h - 1, colorstart, colorend );
    }

    void drawGradientLine( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, bgr888_t colorstart, bgr888_t colorend );

    template<typename T> void drawBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& color                    ) { draw_bitmap(x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& fgcolor, const T& bgcolor) { draw_bitmap(x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }
    template<typename T> void drawXBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& color                    ) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawXBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& fgcolor, const T& bgcolor) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }

    void pushPixelsDMA(const void* data, std::uint32_t length) { pushPixelsDMA_impl(data, length); }

    inline void writePixels(uint16_t * colors, uint32_t len) { pushColors(colors, len, true); }

    void pushColors(const std::uint8_t* data, std::int32_t len)
    {
      pushColors((rgb332_t*)data, len);
    }
    void pushColors(const std::uint16_t* data, std::int32_t len, bool swap = true)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb565_2Byte, _palette_count);
      if (swap && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb565_t>(_write_conv.depth);
      }
      startWrite();
      pushColors_impl(len, &p);
      endWrite();
    }
    void pushColors(const void* data, std::int32_t len, bool swap = true)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb888_3Byte, _palette_count);
      if (swap && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb888_t>(_write_conv.depth);
      }
      startWrite();
      pushColors_impl(len, &p);
      endWrite();
    }
    template <typename T>
    void pushColors(const T *data, std::int32_t len)
    {
      pixelcopy_t p(data, _write_conv.depth, get_depth<T>::value, _palette_count, nullptr);
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<T>(_write_conv.depth);
      }
      startWrite();
      pushColors_impl(len, &p);
      endWrite();
    }

    std::uint16_t readPixel(std::int32_t x, std::int32_t y)
    {
      pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, _palette);
      std::uint_fast16_t data = 0;
      read_rect(x, y, 1, 1, &data, &p);
      return __builtin_bswap16(data);
    }

    __attribute__ ((always_inline)) inline
    void readRectRGB( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint8_t* data) { readRectRGB(x, y, w, h, (bgr888_t*)data); }
    void readRectRGB( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, bgr888_t* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, _palette);
      read_rect(x, y, w, h, data, &p);
    }

    template<typename T> inline
    void readRect( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, T* data)
    {
      pixelcopy_t p(nullptr, get_depth<T>::value, _read_conv.depth, false, _palette);
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<T>(_read_conv.depth); }
      read_rect(x, y, w, h, data, &p);
    }

    void readRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint8_t* data)
    {
      pixelcopy_t p(nullptr, rgb332_t::depth, _read_conv.depth, false, _palette);
      read_rect(x, y, w, h, data, &p);
    }
    void readRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint16_t* data)
    {
      pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, _palette);
      if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<rgb565_t>(_read_conv.depth);
      }
      read_rect(x, y, w, h, data, &p);
    }
    void readRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, _palette);
      if (_swapBytes && !_palette_count && _read_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy_dst<rgb888_t>(_read_conv.depth);
      }
      read_rect(x, y, w, h, data, &p);
    }

    template<typename T> void pushRect(  std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data) { pushImage(x, y, w, h, data); }
    template<typename T> void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data)
    {
      pixelcopy_t p(data, _write_conv.depth, get_depth<T>::value, _palette_count, nullptr);
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<T>(_write_conv.depth);
      }
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_normalcopy<T>(_write_conv.depth); }
      push_image(x, y, w, h, &p);
    }

    template<typename T, typename U>
    void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data, const U& transparent)
    {
      uint32_t tr = (std::is_same<T, U>::value)
                  ? transparent
                  : get_fp_convert_src<U>(get_depth<T>::value, false)(transparent);
      pixelcopy_t p(data, _write_conv.depth, get_depth<T>::value, _palette_count, nullptr, tr);
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value) {
        if (std::is_same<rgb565_t, T>::value) {
          p.transp = getSwap16(tr);
        } else {
          p.transp = getSwap24(tr);
        }
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<T>(_write_conv.depth);
      }
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_normalcopy<T>(_write_conv.depth); }
      if (p.fp_skip==nullptr) { p.fp_skip = pixelcopy_t::normalskip<T>; }
      push_image(x, y, w, h, &p);
    }

    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint16_t* data)
    {
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        pushImage(x, y, w, h, (const rgb565_t*)data);
      } else {
        pushImage(x, y, w, h, (const swap565_t*)data);
      }
    }

    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data)
    {
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        pushImage(x, y, w, h, (const rgb888_t*)data);
      } else {
        pushImage(x, y, w, h, (const bgr888_t*)data);
      }
    }

    template<typename U>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint16_t* data, const U& transparent)
    {
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        pushImage(x, y, w, h, (const rgb565_t*)data, transparent);
      } else {
        pushImage(x, y, w, h, (const swap565_t*)data, transparent);
      }
    }

    template<typename U>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, const U& transparent)
    {
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        pushImage(x, y, w, h, (const rgb888_t*)data, transparent);
      } else {
        pushImage(x, y, w, h, (const bgr888_t*)data, transparent);
      }
    }

    template<typename T>
    void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, const std::uint8_t bits, const T* palette) {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette);
      p.fp_copy = pixelcopy_t::get_fp_palettecopy<T>(_write_conv.depth);
      push_image(x, y, w, h, &p);
    }
    template<typename T>
    void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, std::uint32_t transparent, const std::uint8_t bits, const T* palette) {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette, transparent );
      p.fp_copy = pixelcopy_t::get_fp_palettecopy<T>(_write_conv.depth);
      push_image(x, y, w, h, &p);
    }

    template<typename T>
    void pushImageDMA( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data)
    {
      pixelcopy_t p(data, _write_conv.depth, get_depth<T>::value, _palette_count, nullptr  );
      push_image(x, y, w, h, &p, true);
    }

    template<typename T>
    void pushImageDMA( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, const std::uint8_t bits, const T* palette)
    {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, _palette_count, palette);
      push_image(x, y, w, h, &p, true);
    }

    void pushImageDMA(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint16_t* data)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb565_2Byte, _palette_count, nullptr);
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb565_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p, true);
    }

    void pushImageDMA(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data)
    {
      pixelcopy_t p(data, _write_conv.depth, rgb888_3Byte, _palette_count, nullptr);
      if (_swapBytes && !_palette_count && _write_conv.depth >= 8) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb888_t>(_write_conv.depth);
      }
      push_image(x, y, w, h, &p, true);
    }

    void push_image(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t *param, bool use_dma = false);

    bool pushImageRotateZoom(std::int32_t dst_x, std::int32_t dst_y, const void* data, std::int32_t src_x, std::int32_t src_y, std::int32_t w, std::int32_t h, float angle, float zoom_x, float zoom_y, std::uint32_t transparent, const std::uint8_t bits, const bgr888_t* palette);

    void scroll(std::int_fast16_t dx, std::int_fast16_t dy = 0);

    void copyRect(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y);

    void paint(std::int32_t x, std::int32_t y);


#ifdef _LGFX_QRCODE_H_
#ifdef ARDUINO
    void qrcode(const String &string, std::int_fast16_t x = -1, std::int_fast16_t y = -1, std::int_fast16_t width = -1, std::uint8_t version = 1) {
      qrcode(string.c_str(), x, y, width, version);
    }
#endif
    void qrcode(const char *string, std::int_fast16_t x = -1, std::int_fast16_t y = -1, std::int_fast16_t width = -1, std::uint8_t version = 1) {
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
        std::uint8_t qrcodeData[lgfx_qrcode_getBufferSize(version)];
        if (0 != lgfx_qrcode_initText(&qrcode, qrcodeData, version, 0, string)) continue;
        std::int_fast16_t thickness = width / qrcode.size;
        if (!thickness) break;
        std::int_fast16_t lineLength = qrcode.size * thickness;
        std::int_fast16_t xOffset = x + ((width - lineLength) >> 1);
        std::int_fast16_t yOffset = y + ((width - lineLength) >> 1);
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
#endif

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    std::uint32_t _transaction_count = 0;
    std::int32_t _width = 0, _height = 0;
    std::int32_t  _sx, _sy, _sw, _sh; // for scroll zone

    std::int32_t _clip_l = 0, _clip_r = -1, _clip_t = 0, _clip_b = -1; // clip rect
    std::uint32_t _base_rgb888 = 0;  // gap fill colour for scroll zone 
    raw_color_t _color = 0xFFFFFFU;

    color_conv_t _write_conv;
    color_conv_t _read_conv;

    bgr888_t* _palette = nullptr; // for sprite palette mode.
    std::uint32_t _palette_count = 0;

    std::int16_t _xpivot;   // x pivot point coordinate
    std::int16_t _ypivot;   // x pivot point coordinate

    bool _spi_shared = true;
    bool _swapBytes = false;

    __attribute__ ((always_inline)) inline static bool _adjust_abs(std::int32_t& x, std::int32_t& w) { if (w < 0) { x += w + 1; w = -w; } return !w; }

    static bool _adjust_width(std::int32_t& x, std::int32_t& dx, std::int32_t& dw, std::int32_t left, std::int32_t width)
    {
      if (x < left) { dx = -x; dw += x; x = left; }
      if (dw > left + width - x) dw = left + width  - x;
      return (dw <= 0);
    }

    void read_rect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param)
    {
      _adjust_abs(x, w);
      if (x < 0) { w += x; x = 0; }
      if (w > _width - x)  w = _width  - x;
      if (w < 1) return;

      _adjust_abs(y, h);
      if (y < 0) { h += y; y = 0; }
      if (h > _height - y) h = _height - y;
      if (h < 1) return;

      startWrite();
      readRect_impl(x, y, w, h, dst, param);
      endWrite();
    }

    void fill_arc_helper(std::int32_t cx, std::int32_t cy, std::int32_t oradius, std::int32_t iradius, float start, float end);
    void draw_bitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor = ~0u);
    void draw_xbitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor = ~0u);
    void push_image_rotate_zoom(std::int32_t dst_x, std::int32_t dst_y, std::int32_t src_x, std::int32_t src_y, std::int32_t w, std::int32_t h, float angle, float zoom_x, float zoom_y, pixelcopy_t *param);

    virtual void beginTransaction_impl(void) = 0;
    virtual void endTransaction_impl(void) = 0;
    virtual void waitDMA_impl(void) = 0;
    virtual void pushPixelsDMA_impl(const void* data, std::uint32_t length) = 0;

    virtual void drawPixel_impl(std::int32_t x, std::int32_t y) = 0;
    virtual void writeFillRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) = 0;
    virtual void copyRect_impl(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y) = 0;
    virtual void readRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param) = 0;
    virtual void pushImage_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param, bool use_dma) = 0;
    virtual void pushColors_impl(std::int32_t length, pixelcopy_t* param) = 0;
    virtual void pushBlock_impl(std::int32_t len) = 0;
    virtual void setWindow_impl(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye) = 0;
    virtual bool isReadable_impl(void) const = 0;
    virtual std::int_fast8_t getRotation_impl(void) const = 0;

    static void tmpBeginTransaction(void* lgfx) {
      auto me = (LGFXBase*)lgfx;
      if (me->_transaction_count) me->beginTransaction();
    }
    static void tmpEndTransaction(void* lgfx) {
      auto me = (LGFXBase*)lgfx;
      if (me->_transaction_count) me->endTransaction();
    }
    void prepareTmpTransaction(DataWrapper* data) {
      if (data->need_transaction && isSPIShared()) {
        data->parent = this;
        data->fp_pre_read  = tmpEndTransaction;
        data->fp_post_read = tmpBeginTransaction;
      }
    }
    __attribute__ ((always_inline)) inline void startWrite(bool transaction) {
      if (1 == ++_transaction_count && transaction) beginTransaction();
    }
  };
}


class LovyanGFX : public
#ifdef LGFX_FONT_SUPPORT_HPP_
   lgfx::LGFX_Font_Support<
#endif
#ifdef LGFX_IMG_SUPPORT_HPP_
    lgfx::LGFX_IMG_Support<
#endif
     lgfx::LGFXBase
#ifdef LGFX_IMG_SUPPORT_HPP_
    >
#endif
#ifdef LGFX_FONT_SUPPORT_HPP_
   >
#endif
{};

#endif
