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
#ifndef LGFX_BASE_HPP_
#define LGFX_BASE_HPP_

#include "lgfx_common.hpp"
#include "../Fonts/lgfx_fonts.hpp"

#include <cmath>
#include <string>
#include <memory>

namespace lgfx
{
  class LGFX_Sprite;

  class LGFXBase
#if defined (ARDUINO)
  : public Print
#endif
  {
  friend LGFX_Sprite;
  friend IFont;
  public:
    LGFXBase() {}
    virtual ~LGFXBase() {}

// color param format:
// rgb888 : std::uint32_t
// rgb565 : std::uint16_t & std::int16_t & int
// rgb332 : std::uint8_t
    __attribute__ ((always_inline)) inline void setColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) { _color.raw = _write_conv.convert(lgfx::color888(r,g,b)); }
    template<typename T> __attribute__ ((always_inline)) inline void setColor(T color) { _color.raw = _write_conv.convert(color); }
                         __attribute__ ((always_inline)) inline void setRawColor(std::uint32_t c) { _color.raw = c; }

    template<typename T> __attribute__ ((always_inline)) inline void setBaseColor(T c) { _base_rgb888 = convert_to_rgb888(c); }
    std::uint32_t getBaseColor(void) const { return _base_rgb888; }

// AdafruitGFX compatible functions.
// However, startWrite and endWrite have an internal counter and are executed when the counter is 0.
// If you do not want to the counter, call the transaction function directly.
    __attribute__ ((always_inline)) inline void startWrite(void) {                           if (1 == ++_transaction_count) beginTransaction(); }
    __attribute__ ((always_inline)) inline void endWrite(void)   { if (_transaction_count) { if (0 == --_transaction_count) endTransaction(); } }
    __attribute__ ((always_inline)) inline void writePixel(std::int32_t x, std::int32_t y)  { if (x >= _clip_l && x <= _clip_r && y >= _clip_t && y <= _clip_b) writeFillRect_impl(x, y, 1, 1); }
    template<typename T> inline void writePixel    ( std::int32_t x, std::int32_t y                                , const T& color) { setColor(color); writePixel    (x, y      ); }
    template<typename T> inline void writeFastVLine( std::int32_t x, std::int32_t y                , std::int32_t h, const T& color) { setColor(color); writeFastVLine(x, y   , h); }
                                void writeFastVLine( std::int32_t x, std::int32_t y                , std::int32_t h);
    template<typename T> inline void writeFastHLine( std::int32_t x, std::int32_t y, std::int32_t w                , const T& color) { setColor(color); writeFastHLine(x, y, w   ); }
                                void writeFastHLine( std::int32_t x, std::int32_t y, std::int32_t w);
    template<typename T> inline void writeFillRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T& color) { setColor(color); writeFillRect (x, y, w, h); }
                                void writeFillRect ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    template<typename T> inline void writeColor    ( const T& color, std::int32_t length) {      if (0 >= length) return; setColor(color);    pushBlock_impl(length); }
    template<typename T> inline void writeFillRectPreclipped( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T& color) { setColor(color); writeFillRect_impl (x, y, w, h); }
                                void writeFillRectPreclipped( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)                 {                  writeFillRect_impl (x, y, w, h); }


    __attribute__ ((always_inline)) inline void drawPixel( std::int32_t x, std::int32_t y ) { if (x >= _clip_l && x <= _clip_r && y >= _clip_t && y <= _clip_b) drawPixel_impl(x, y); }
    template<typename T> inline void drawPixel       ( std::int32_t x, std::int32_t y                                                , const T& color) { setColor(color); drawPixel    (x, y         ); }
    template<typename T> inline void drawFastVLine   ( std::int32_t x, std::int32_t y                , std::int32_t h                , const T& color) { setColor(color); drawFastVLine(x, y   , h   ); }
                                void drawFastVLine   ( std::int32_t x, std::int32_t y                , std::int32_t h);
    template<typename T> inline void drawFastHLine   ( std::int32_t x, std::int32_t y, std::int32_t w                                , const T& color) { setColor(color); drawFastHLine(x, y, w      ); }
                                void drawFastHLine   ( std::int32_t x, std::int32_t y, std::int32_t w);
    template<typename T> inline void fillRect        ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h                , const T& color) { setColor(color); fillRect     (x, y, w, h   ); }
                                void fillRect        ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    template<typename T> inline void drawRect        ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h                , const T& color) { setColor(color); drawRect     (x, y, w, h   ); }
                                void drawRect        ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    template<typename T> inline void drawRoundRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r, const T& color) { setColor(color); drawRoundRect(x, y, w, h, r); }
                                void drawRoundRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r);
    template<typename T> inline void fillRoundRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r, const T& color) { setColor(color); fillRoundRect(x, y, w, h, r); }
                                void fillRoundRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r);
    template<typename T> inline void drawCircle      ( std::int32_t x, std::int32_t y                                , std::int32_t r, const T& color) { setColor(color); drawCircle   (x, y      , r); }
                                void drawCircle      ( std::int32_t x, std::int32_t y                                , std::int32_t r);
    template<typename T> inline void fillCircle      ( std::int32_t x, std::int32_t y                                , std::int32_t r, const T& color) { setColor(color); fillCircle   (x, y      , r); }
                                void fillCircle      ( std::int32_t x, std::int32_t y                                , std::int32_t r);
    template<typename T> inline void drawEllipse     ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry              , const T& color) { setColor(color); drawEllipse  (x, y, rx, ry ); }
                                void drawEllipse     ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry);     
    template<typename T> inline void fillEllipse     ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry              , const T& color) { setColor(color); fillEllipse  (x, y, rx, ry ); }
                                void fillEllipse     ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry);
    template<typename T> inline void drawLine        ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1            , const T& color)  { setColor(color); drawLine(    x0, y0, x1, y1        ); }
                                void drawLine        ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1);
    template<typename T> inline void drawTriangle    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawTriangle(x0, y0, x1, y1, x2, y2); }
                                void drawTriangle    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    template<typename T> inline void fillTriangle    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); fillTriangle(x0, y0, x1, y1, x2, y2); }
                                void fillTriangle    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    template<typename T> inline void drawBezier      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawBezier(x0, y0, x1, y1, x2, y2); }
                                void drawBezier      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    template<typename T> inline void drawBezier      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, std::int32_t x3, std::int32_t y3, const T& color)  { setColor(color); drawBezier(x0, y0, x1, y1, x2, y2, x3, y3); }
                                void drawBezier      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, std::int32_t x3, std::int32_t y3);
    template<typename T> inline void drawBezierHelper( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawBezierHelper(x0, y0, x1, y1, x2, y2); }
                                void drawBezierHelper( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    template<typename T> inline void drawArc         ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1, const T& color) { setColor(color); drawArc( x, y, r0, r1, angle0, angle1); }
                                void drawArc         ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1);
    template<typename T> inline void fillArc         ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1, const T& color) { setColor(color); fillArc( x, y, r0, r1, angle0, angle1); }
                                void fillArc         ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1);
    template<typename T> inline void drawCircleHelper( std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t cornername                 , const T& color)  { setColor(color); drawCircleHelper(x, y, r, cornername    ); }
                                void drawCircleHelper( std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t cornername);
    template<typename T> inline void fillCircleHelper( std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t corners, std::int32_t delta, const T& color)  { setColor(color); fillCircleHelper(x, y, r, corners, delta); }
                                void fillCircleHelper( std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t corners, std::int32_t delta);

    template<typename T> inline void floodFill( std::int32_t x, std::int32_t y, const T& color) { setColor(color); floodFill(x, y); }
                                void floodFill( std::int32_t x, std::int32_t y                );
    template<typename T> inline void paint    ( std::int32_t x, std::int32_t y, const T& color) { setColor(color); floodFill(x, y); }
                         inline void paint    ( std::int32_t x, std::int32_t y                ) {                  floodFill(x, y); }

    template<typename T> inline void drawGradientHLine( std::int32_t x, std::int32_t y, std::int32_t w, const T& colorstart, const T& colorend ) { drawGradientLine( x, y, x + w - 1, y, colorstart, colorend ); }
    template<typename T> inline void drawGradientVLine( std::int32_t x, std::int32_t y, std::int32_t h, const T& colorstart, const T& colorend ) { drawGradientLine( x, y, x, y + h - 1, colorstart, colorend ); }
    template<typename T> inline void drawGradientLine ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, const T& colorstart, const T& colorend ) { draw_gradient_line( x0, y0, x1, y1, convert_to_rgb888(colorstart), convert_to_rgb888(colorend) ); }

                         inline void clear      ( void )          { setColor(_base_rgb888); fillRect(0, 0, _width, _height); }
    template<typename T> inline void clear      ( const T& color) { setColor(color);        fillRect(0, 0, _width, _height); }
                         inline void fillScreen ( void )          {                         fillRect(0, 0, _width, _height); }
    template<typename T> inline void fillScreen ( const T& color) { setColor(color);        fillRect(0, 0, _width, _height); }

    template<typename T> inline void pushBlock  ( const T& color, std::int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }


    __attribute__ ((always_inline)) inline static std::uint8_t  color332(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color332(r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint16_t color565(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color565(r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint32_t color888(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color888(r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint16_t swap565( std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::swap565( r, g, b); }
    __attribute__ ((always_inline)) inline static std::uint32_t swap888( std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::swap888( r, g, b); }

    __attribute__ ((always_inline)) inline void setPivot(float x, float y) { _xpivot = x; _ypivot = y; }
    __attribute__ ((always_inline)) inline float getPivotX(void) const { return _xpivot; }
    __attribute__ ((always_inline)) inline float getPivotY(void) const { return _ypivot; }

    __attribute__ ((always_inline)) inline std::int32_t width        (void) const { return _width; }
    __attribute__ ((always_inline)) inline std::int32_t height       (void) const { return _height; }
    __attribute__ ((always_inline)) inline std::int_fast8_t getRotation(void) const { return getRotation_impl(); }
    __attribute__ ((always_inline)) inline color_depth_t getColorDepth(void) const { return _write_conv.depth; }
    __attribute__ ((always_inline)) inline color_conv_t* getColorConverter(void) { return &_write_conv; }
    __attribute__ ((always_inline)) inline RGBColor*     getPalette(void) const { return getPalette_impl(); }
    __attribute__ ((always_inline)) inline std::uint32_t getPaletteCount(void) const { return _palette_count; }
    __attribute__ ((always_inline)) inline bool hasPalette    (void) const { return _palette_count; }
    __attribute__ ((always_inline)) inline bool isSPIShared(void) const { return _spi_shared; }
    __attribute__ ((always_inline)) inline bool isReadable(void) const { return isReadable_impl(); }
    __attribute__ ((always_inline)) inline bool getSwapBytes    (void) const { return _swapBytes; }
    __attribute__ ((always_inline)) inline void setSwapBytes(bool swap) { _swapBytes = swap; }
    __attribute__ ((always_inline)) inline void setSPIShared(bool shared) { _spi_shared = shared; }

    __attribute__ ((always_inline)) inline void beginTransaction(void) { beginTransaction_impl(); }
    __attribute__ ((always_inline)) inline void endTransaction(void)   { endTransaction_impl(); }
    __attribute__ ((always_inline)) inline void initDMA(void)  { initDMA_impl(); }
    __attribute__ ((always_inline)) inline void waitDMA(void) { waitDMA_impl(); }
    __attribute__ ((always_inline)) inline bool dmaBusy(void)  { return dmaBusy_impl(); }
    __attribute__ ((always_inline)) inline void setWindow(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye) { setWindow_impl(xs, ys, xe, ye); }

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

    __attribute__ ((always_inline)) inline void pushPixelsDMA( const void* data, std::int32_t len) { if (len < 0) return; startWrite(); writePixelsDMA_impl(data, len); endWrite(); }
    __attribute__ ((always_inline)) inline void writePixelsDMA(const void* data, std::int32_t len) { if (len < 0) return;               writePixelsDMA_impl(data, len);             }

    template <typename T>
    __attribute__ ((always_inline)) inline void pushPixels(T*                   data, std::int32_t len           ) { startWrite(); writePixels(data, len            ); endWrite(); }
    __attribute__ ((always_inline)) inline void pushPixels(const std::uint8_t*  data, std::int32_t len           ) { startWrite(); writePixels((const rgb332_t*)data, len); endWrite(); }
    __attribute__ ((always_inline)) inline void pushPixels(const std::uint16_t* data, std::int32_t len           ) { startWrite(); writePixels(data, len, _swapBytes); endWrite(); }
    __attribute__ ((always_inline)) inline void pushPixels(const std::uint16_t* data, std::int32_t len, bool swap) { startWrite(); writePixels(data, len,  swap     ); endWrite(); }
    __attribute__ ((always_inline)) inline void pushPixels(const void*          data, std::int32_t len           ) { startWrite(); writePixels(data, len, _swapBytes); endWrite(); }
    __attribute__ ((always_inline)) inline void pushPixels(const void*          data, std::int32_t len, bool swap) { startWrite(); writePixels(data, len,  swap     ); endWrite(); }


    __attribute__ ((always_inline)) inline void writePixels(const std::uint8_t*  data, std::int32_t len           ) { writePixels((const rgb332_t*)data, len); }
    __attribute__ ((always_inline)) inline void writePixels(const std::uint16_t* data, std::int32_t len           ) { writePixels(data, len, _swapBytes); }
                                           void writePixels(const std::uint16_t* data, std::int32_t len, bool swap);
    __attribute__ ((always_inline)) inline void writePixels(const void*          data, std::int32_t len           ) { writePixels(data, len, _swapBytes); }
                                           void writePixels(const void*          data, std::int32_t len, bool swap);
    template <typename T>
    void writePixels(const T *data, std::int32_t len)
    {
      pixelcopy_t p(data, _write_conv.depth, get_depth<T>::value, hasPalette(), nullptr);
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_fast<T>(_write_conv.depth);
      }
      writePixels_impl(len, &p);
    }

    template <typename T>
    void writeIndexedPixels(const uint8_t *data, T* palette, std::int32_t len, lgfx::color_depth_t colordepth = lgfx::rgb332_1Byte)
    {
      pixelcopy_t p(data, _write_conv.depth, colordepth, hasPalette(), palette);
      p.no_convert = false;
      p.fp_copy = pixelcopy_t::get_fp_copy_palette_affine<T>(_write_conv.depth);
      writePixels_impl(len, &p);
    }

    template<typename T> void drawBitmap (std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& color                    ) { draw_bitmap (x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawBitmap (std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& fgcolor, const T& bgcolor) { draw_bitmap (x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }
    template<typename T> void drawXBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& color                    ) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawXBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& fgcolor, const T& bgcolor) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }

    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void*          data);
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint16_t* data);
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint8_t*  data);

    template<typename T>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data)
    {
      pixelcopy_t p(data, _write_conv.depth, get_depth<T>::value, hasPalette(), nullptr);
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_fast<T>(_write_conv.depth);
      }
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_copy_rgb_fast<T>(_write_conv.depth); }
      pushImage(x, y, w, h, &p);
    }

    template<typename T, typename U>
    void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data, const U& transparent)
    {
      std::uint32_t tr = (std::is_same<T, U>::value)
                       ? transparent
                       : get_fp_convert_src<U>(get_depth<T>::value, false)(transparent);
      pixelcopy_t p(data, _write_conv.depth, get_depth<T>::value, hasPalette(), nullptr, tr);
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value) {
        if (std::is_same<rgb565_t, T>::value) {
          p.transp = getSwap16(tr);
        } else {
          p.transp = getSwap24(tr);
        }
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine<T>(_write_conv.depth);
      }
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine<T>(_write_conv.depth); }
      if (p.fp_skip==nullptr) { p.fp_skip = pixelcopy_t::skip_rgb_affine<T>; }
      pushImage(x, y, w, h, &p);
    }

    template<typename U>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint8_t* data, const U& transparent)
    {
      pushImage(x, y, w, h, (const rgb332_t*)data, transparent);
    }

    template<typename U>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint16_t* data, const U& transparent)
    {
      if (_swapBytes && _write_conv.depth >= 8 && !hasPalette()) {
        pushImage(x, y, w, h, (const rgb565_t*)data, transparent);
      } else {
        pushImage(x, y, w, h, (const swap565_t*)data, transparent);
      }
    }

    template<typename U>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, const U& transparent)
    {
      if (_swapBytes && _write_conv.depth >= 8 && !hasPalette()) {
        pushImage(x, y, w, h, (const rgb888_t*)data, transparent);
      } else {
        pushImage(x, y, w, h, (const bgr888_t*)data, transparent);
      }
    }

    template<typename T>
    void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, const std::uint8_t bits, const T* palette) {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, hasPalette(), palette);
      p.fp_copy = pixelcopy_t::get_fp_copy_palette_affine<T>(_write_conv.depth);
      pushImage(x, y, w, h, &p);
    }
    template<typename T>
    void pushImage( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, std::uint32_t transparent, const std::uint8_t bits, const T* palette) {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, hasPalette(), palette, transparent );
      p.fp_copy = pixelcopy_t::get_fp_copy_palette_affine<T>(_write_conv.depth);
      pushImage(x, y, w, h, &p);
    }

    void pushImageDMA(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void*          data);
    void pushImageDMA(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint16_t* data);
    void pushImageDMA(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const std::uint8_t*  data);

    template<typename T>
    void pushImageDMA( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data)
    {
      pixelcopy_t p(data, _write_conv.depth, get_depth<T>::value, hasPalette(), nullptr  );
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_fast<T>(_write_conv.depth);
      }
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_copy_rgb_fast<T>(_write_conv.depth); }
      pushImage(x, y, w, h, &p, true);
    }

    template<typename T>
    void pushImageDMA( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, const std::uint8_t bits, const T* palette)
    {
      pixelcopy_t p(data, _write_conv.depth, (color_depth_t)bits, hasPalette(), palette);
      pushImage(x, y, w, h, &p, true);
    }

    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t *param, bool use_dma = false);

    bool pushImageRotateZoom(float dst_x, float dst_y, float src_x, float src_y, std::int32_t w, std::int32_t h, float angle, float zoom_x, float zoom_y, const void* data, std::uint32_t transparent, const std::uint8_t bits, const bgr888_t* palette);
    bool pushImageRotateZoomA(float dst_x, float dst_y, float src_x, float src_y, std::int32_t w, std::int32_t h, float angle, float zoom_x, float zoom_y, const void* data, std::uint32_t transparent, const std::uint8_t bits, const bgr888_t* palette);

    bool pushImageAffine(float matrix[6], const void* data, std::int32_t w, std::int32_t h, std::uint32_t transparent, const std::uint8_t bits, const bgr888_t* palette);
    bool pushImageAffineA(float matrix[6], const void* data, std::int32_t w, std::int32_t h, std::uint32_t transparent, const std::uint8_t bits, const bgr888_t* palette);

    /// read RGB565 16bit color
    std::uint16_t readPixel(std::int32_t x, std::int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return 0;

      pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, getPalette());
      std::uint_fast16_t data = 0;

      readRect_impl(x, y, 1, 1, &data, &p);

      return __builtin_bswap16(data);
    }

    /// read RGB888 24bit color
    RGBColor readPixelRGB(std::int32_t x, std::int32_t y)
    {
      RGBColor data[1];
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return data[0];

      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, getPalette());

      readRect_impl(x, y, 1, 1, data, &p);

      return data[0];
    }

    __attribute__ ((always_inline)) inline
    void readRectRGB( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint8_t* data) { readRectRGB(x, y, w, h, (bgr888_t*)data); }
    void readRectRGB( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, RGBColor* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, getPalette());
      read_rect(x, y, w, h, data, &p);
    }

    template<typename T> inline
    void readRect( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, T* data)
    {
      pixelcopy_t p(nullptr, get_depth<T>::value, _read_conv.depth, false, getPalette());
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value) {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_fast_dst<T>(_read_conv.depth);
      }
      if (p.fp_copy==nullptr) { p.fp_copy = pixelcopy_t::get_fp_copy_rgb_fast_dst<T>(_read_conv.depth); }
      read_rect(x, y, w, h, data, &p);
    }

    void readRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint8_t*  data);
    void readRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint16_t* data);
    void readRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void*          data);

    void scroll(std::int_fast16_t dx, std::int_fast16_t dy = 0);
    void copyRect(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y);


    [[deprecated("use IFont")]]
    void setCursor( std::int32_t x, std::int32_t y, std::uint8_t font) { _filled_x = 0; _cursor_x = x; _cursor_y = y; setFont(fontdata[font]); }
    void setCursor( std::int32_t x, std::int32_t y, const IFont* font) { _filled_x = 0; _cursor_x = x; _cursor_y = y; setFont(font); }
    void setCursor( std::int32_t x, std::int32_t y)                    { _filled_x = 0; _cursor_x = x; _cursor_y = y; }
    std::int32_t getCursorX(void) const { return _cursor_x; }
    std::int32_t getCursorY(void) const { return _cursor_y; }
    void setTextStyle(const TextStyle& text_style) { _text_style = text_style; }
    const TextStyle& getTextStyle(void) const { return _text_style; }
    void setTextSize(float size) { setTextSize(size, size); }
    void setTextSize(float sx, float sy) { _text_style.size_x = (sx > 0) ? sx : 1; _text_style.size_y = (sy > 0) ? sy : 1; }
    float getTextSizeX(void) const { return _text_style.size_x; }
    float getTextSizeY(void) const { return _text_style.size_y; }
    [[deprecated("use textdatum_t")]]
    void setTextDatum(std::uint8_t datum) { _text_style.datum = (textdatum_t)datum; }
    void setTextDatum(textdatum_t datum) { _text_style.datum = datum; }
    textdatum_t getTextDatum(void) const { return _text_style.datum; }
    void setTextPadding(std::uint32_t padding_x) { _padding_x = padding_x; }
    std::uint32_t getTextPadding(void) const { return _padding_x; }
    void setTextWrap( bool wrapX, bool wrapY = false) { _textwrap_x = wrapX; _textwrap_y = wrapY; }
    void setTextScroll(bool scroll) { _textscroll = scroll; if (_cursor_x < this->_sx) { _cursor_x = this->_sx; } if (_cursor_y < this->_sy) { _cursor_y = this->_sy; } }

    template<typename T>
    void setTextColor(T color) {
      if (this->hasPalette()) {
        _text_style.fore_rgb888 = _text_style.back_rgb888 = color;
      } else {
        _text_style.fore_rgb888 = _text_style.back_rgb888 = convert_to_rgb888(color);
      }
    }
    template<typename T1, typename T2>
    void setTextColor(T1 fgcolor, T2 bgcolor) {
      if (this->hasPalette()) {
        _text_style.fore_rgb888 = fgcolor;
        _text_style.back_rgb888 = bgcolor;
      } else {
        _text_style.fore_rgb888 = convert_to_rgb888(fgcolor);
        _text_style.back_rgb888 = convert_to_rgb888(bgcolor);
      }
    }

    [[deprecated("use IFont")]]
    std::int32_t fontHeight(std::uint8_t font) const { return ((const BaseFont*)fontdata[font])->height * _text_style.size_y; }
    std::int32_t fontHeight(const IFont* font) const { return ((const BaseFont*)font)->height * _text_style.size_y; }
    std::int32_t fontHeight(void) const { return _font_metrics.height * _text_style.size_y; }
    std::int32_t textLength(const char *string, std::int32_t width);
    std::int32_t textWidth(const char *string);

    [[deprecated("use IFont")]]
    inline std::size_t drawString(const char *string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, _text_style.datum); }
    inline std::size_t drawString(const char *string, std::int32_t x, std::int32_t y, const IFont* font) { setFont(font          ); return draw_string(string, x, y, _text_style.datum); }
    inline std::size_t drawString(const char *string, std::int32_t x, std::int32_t y                   ) {                          return draw_string(string, x, y, _text_style.datum); }

    [[deprecated("use IFont")]]
    inline std::size_t drawNumber(long long_num, std::int32_t poX, std::int32_t poY, std::uint8_t font) { setFont(fontdata[font]); return drawNumber(long_num, poX, poY); }
    inline std::size_t drawNumber(long long_num, std::int32_t poX, std::int32_t poY, const IFont* font) { setFont(font          ); return drawNumber(long_num, poX, poY); }
           std::size_t drawNumber(long long_num, std::int32_t poX, std::int32_t poY);
    inline std::size_t drawFloat(float floatNumber, std::uint8_t dp, std::int32_t poX, std::int32_t poY, const IFont* font) { setFont(font          ); return drawFloat(floatNumber, dp, poX, poY); }
    inline std::size_t drawFloat(float floatNumber, std::uint8_t dp, std::int32_t poX, std::int32_t poY, std::uint8_t font) { setFont(fontdata[font]); return drawFloat(floatNumber, dp, poX, poY); }
           std::size_t drawFloat(float floatNumber, std::uint8_t dp, std::int32_t poX, std::int32_t poY);

    [[deprecated("use setTextDatum() and drawString()")]] inline std::size_t drawCentreString(const char *string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_center); }
    [[deprecated("use setTextDatum() and drawString()")]] inline std::size_t drawCenterString(const char *string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_center); }
    [[deprecated("use setTextDatum() and drawString()")]] inline std::size_t drawRightString( const char *string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_right); }

  #if defined (ARDUINO)
    inline std::int32_t textLength(const String& string, std::int32_t width) { return textLength(string.c_str(), width); }
    inline std::int32_t textWidth(const String& string) { return textWidth(string.c_str()); }

    inline std::size_t drawString(const String& string, std::int32_t x, std::int32_t y                   ) {                          return draw_string(string.c_str(), x, y, _text_style.datum); }
    inline std::size_t drawString(const String& string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, _text_style.datum); }

    [[deprecated("use setTextDatum() and drawString()")]] inline std::size_t drawCentreString(const String& string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }
    [[deprecated("use setTextDatum() and drawString()")]] inline std::size_t drawCenterString(const String& string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }
    [[deprecated("use setTextDatum() and drawString()")]] inline std::size_t drawRightString( const String& string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_right); }
  #endif

           std::size_t drawChar(std::uint16_t uniCode, std::int32_t x, std::int32_t y, std::uint8_t font);
//  inline std::size_t drawChar(std::uint16_t uniCode, std::int32_t x, std::int32_t y                   ) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, &_text_style, _font); }
    inline std::size_t drawChar(std::uint16_t uniCode, std::int32_t x, std::int32_t y                   ) { _filled_x = 0; return _font->drawChar(this, x, y, uniCode, &_text_style); }

    template<typename T>
    inline std::size_t drawChar(std::int32_t x, std::int32_t y, std::uint16_t uniCode, T color, T bg, float size) { return drawChar(x, y, uniCode, color, bg, size, size); }
    template<typename T>
    inline std::size_t drawChar(std::int32_t x, std::int32_t y, std::uint16_t uniCode, T color, T bg, float size_x, float size_y)
    {
      TextStyle style = _text_style;
      style.back_rgb888 = convert_to_rgb888(color);
      style.fore_rgb888 = convert_to_rgb888(bg);
      style.size_x = size_x;
      style.size_y = size_y;
      _filled_x = 0;
      return _font->drawChar(this, x, y, uniCode, &style);
      //return (fpDrawChar)(this, x, y, uniCode, &style, _font);
    }

    [[deprecated("use getFont()")]]
    std::uint8_t getTextFont(void) const {
      std::size_t i = 0;
      do {
        if (fontdata[i] == _font) return i;
      } while (fontdata[++i]);
      return 0;
    }

    [[deprecated("use setFont(&fonts::Font)")]]
    void setTextFont(int f) {
      if (f == 1 && _font && _font->getType() == IFont::font_type_t::ft_gfx) return;
      setFont(fontdata[f]);
    }

    [[deprecated("use setFont(&fonts::Font)")]]
    void setTextFont(const IFont* font = nullptr) { setFont(font); }

    [[deprecated("use setFont()")]]
    void setFreeFont(const IFont* font = nullptr) { setFont(font); }

    __attribute__ ((always_inline)) inline const IFont* getFont (void) const { return _font; }

    void setFont(const IFont* font);

    /// load VLW font
    bool loadFont(const std::uint8_t* array);

    /// unload VLW font
    void unloadFont(void);

    /// show VLW font
    void showFont(std::uint32_t td);

    void cp437(bool enable = true) { _text_style.cp437 = enable; }  // AdafruitGFX compatible.

    void setAttribute(attribute_t attr_id, std::uint8_t param);
    std::uint8_t getAttribute(attribute_t attr_id);
    std::uint8_t getAttribute(std::uint8_t attr_id) { return getAttribute((attribute_t)attr_id); }

    std::int32_t _get_text_filled_x(void) const { return _filled_x; }
    void _set_text_filled_x(std::int32_t x) { _filled_x = x; }
    FontMetrics _get_font_metrics(void) const { return _font_metrics; }

//----------------------------------------------------------------------------
// print & text support
//----------------------------------------------------------------------------
// Arduino Print.h compatible
  #if !defined (ARDUINO)
    std::size_t print(const char str[])      { return write(str); }
    std::size_t print(char c)                { return write(c); }
    std::size_t print(int  n, int base = 10) { return print((long)n, base); }
    std::size_t print(long n, int base = 10)
    {
      if (base == 0) { return write(n); }
      if (base == 10) {
        if (n < 0) {
          std::size_t t = print('-');
          return printNumber(-n, 10) + t;
        }
        return printNumber(n, 10);
      }
      return printNumber(n, base);
    }

    std::size_t print(unsigned char n, int base = 10) { return print((unsigned long)n, base); }
    std::size_t print(unsigned int  n, int base = 10) { return print((unsigned long)n, base); }
    std::size_t print(unsigned long n, int base = 10) { return (base) ? printNumber(n, base) : write(n); }
    std::size_t print(double        n, int digits= 2) { return printFloat(n, digits); }

    std::size_t println(void) { return print("\r\n"); }
    std::size_t println(const char c[])                 { std::size_t t = print(c); return println() + t; }
    std::size_t println(char c        )                 { std::size_t t = print(c); return println() + t; }
    std::size_t println(int  n, int base = 10)          { std::size_t t = print(n,base); return println() + t; }
    std::size_t println(long n, int base = 10)          { std::size_t t = print(n,base); return println() + t; }
    std::size_t println(unsigned char n, int base = 10) { std::size_t t = print(n,base); return println() + t; }
    std::size_t println(unsigned int  n, int base = 10) { std::size_t t = print(n,base); return println() + t; }
    std::size_t println(unsigned long n, int base = 10) { std::size_t t = print(n,base); return println() + t; }
    std::size_t println(double        n, int digits= 2) { std::size_t t = print(n, digits); return println() + t; }

  //std::size_t print(const String &s) { return write(s.c_str(), s.length()); }
  //std::size_t print(const __FlashStringHelper *s)   { return print(reinterpret_cast<const char *>(s)); }
  //std::size_t println(const String &s)              { std::size_t t = print(s); return println() + t; }
  //std::size_t println(const __FlashStringHelper *s) { std::size_t t = print(s); return println() + t; }

    std::size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)));

    std::size_t write(const char* str)                 { return (!str) ? 0 : write((const std::uint8_t*)str, strlen(str)); }
    std::size_t write(const char *buf, std::size_t size)    { return write((const std::uint8_t *) buf, size); }
  #else
    using Print::write;
  #endif
    std::size_t write(const std::uint8_t *buf, std::size_t size) { std::size_t n = 0; this->startWrite(); while (size--) { n += write(*buf++); } this->endWrite(); return n; }
    std::size_t write(std::uint8_t utf8);



#ifdef ARDUINO
    void qrcode(const String &string, std::int32_t x = -1, std::int32_t y = -1, std::int32_t width = -1, std::uint8_t version = 1) {
      qrcode(string.c_str(), x, y, width, version);
    }
#endif
    void qrcode(const char *string, std::int32_t x = -1, std::int32_t y = -1, std::int32_t width = -1, std::uint8_t version = 1);


    void drawBmp(const std::uint8_t *bmp_data, std::uint32_t bmp_len, std::int32_t x=0, std::int32_t y=0) {
      PointerWrapper data;
      data.set(bmp_data, bmp_len);
      this->draw_bmp(&data, x, y);
    }
    bool drawJpg(const std::uint8_t *jpg_data, std::uint32_t jpg_len, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div::jpeg_div_t scale=jpeg_div::jpeg_div_t::JPEG_DIV_NONE) {
      PointerWrapper data;
      data.set(jpg_data, jpg_len);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }
    bool drawPng(const std::uint8_t *png_data, std::uint32_t png_len, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, double scale = 1.0)
    {
      PointerWrapper data;
      data.set(png_data, png_len);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline void drawBmp(DataWrapper *data, std::int32_t x=0, std::int32_t y=0) {
      this->draw_bmp(data, x, y);
    }
    inline bool drawJpg(DataWrapper *data, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div::jpeg_div_t scale=jpeg_div::jpeg_div_t::JPEG_DIV_NONE) {
      return this->draw_jpg(data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }
    inline bool drawPng(DataWrapper *data, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, double scale = 1.0) {
      return this->draw_png(data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }






    template<typename T>
    [[deprecated("use pushImage")]] void pushRect( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data) { pushImage(x, y, w, h, data); }

    template<typename T>
    [[deprecated("use pushBlock")]] void pushColor(const T& color, std::int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }
    template<typename T>
    [[deprecated("use pushBlock")]] void pushColor(const T& color                     ) {                          setColor(color); startWrite(); pushBlock_impl(1);      endWrite(); }

    template <typename T>
    [[deprecated("use pushPixels")]] void pushColors(T*                   data, std::int32_t len           ) { startWrite(); writePixels(data, len            ); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const void*          data, std::int32_t len           ) { startWrite(); writePixels(data, len, _swapBytes); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const std::uint16_t* data, std::int32_t len           ) { startWrite(); writePixels(data, len, _swapBytes); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const std::uint8_t*  data, std::int32_t len           ) { startWrite(); writePixels((const rgb332_t*)data, len); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const void*          data, std::int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const std::uint16_t* data, std::int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }

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

    std::uint32_t _palette_count = 0;

    float _xpivot;   // x pivot point coordinate
    float _ypivot;   // x pivot point coordinate

    bool _spi_shared = true;
    bool _swapBytes = false;


    enum utf8_decode_state_t
    { utf8_state0 = 0
    , utf8_state1 = 1
    , utf8_state2 = 2
    };
    utf8_decode_state_t _decoderState = utf8_state0;   // UTF8 decoder state
    std::uint16_t _unicode_buffer = 0;   // Unicode code-point buffer

    std::int32_t _cursor_x = 0;  // print text cursor
    std::int32_t _cursor_y = 0;
    std::int32_t _filled_x = 0;  // print filled position
    std::int32_t _padding_x = 0;

    TextStyle _text_style;
    FontMetrics _font_metrics = { 6, 6, 0, 8, 8, 0, 7 }; // Font0 Metric
    const IFont* _font = &fonts::Font0;

    std::shared_ptr<RunTimeFont> _runtime_font;  // run-time generated font
    PointerWrapper _font_data;

    bool _textwrap_x = true;
    bool _textwrap_y = false;
    bool _textscroll = false;

    __attribute__ ((always_inline)) inline static bool _adjust_abs(std::int32_t& x, std::int32_t& w) { if (w < 0) { x += w + 1; w = -w; } return !w; }

    static bool _adjust_width(std::int32_t& x, std::int32_t& dx, std::int32_t& dw, std::int32_t left, std::int32_t width)
    {
      if (x < left) { dx = -x; dw += x; x = left; }
      if (dw > left + width - x) dw = left + width  - x;
      return (dw <= 0);
    }

    static void make_rotation_matrix(float* result, float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y);

    void writeRawColor( std::uint32_t color, std::int32_t length) { if (0 >= length) return; setRawColor(color); pushBlock_impl(length); }
    void read_rect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param);
    void draw_gradient_line( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, uint32_t colorstart, uint32_t colorend );
    void fill_arc_helper(std::int32_t cx, std::int32_t cy, std::int32_t oradius, std::int32_t iradius, float start, float end);
    void draw_bitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor = ~0u);
    void draw_xbitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor = ~0u);
    void push_image_affine(float* affine, pixelcopy_t *pc);
    void push_image_affine_a(float* affine, pixelcopy_t *pre_pc, pixelcopy_t *post_pc);
    void draw_bezier_helper(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);

    std::uint16_t decodeUTF8(std::uint8_t c);

    std::size_t printNumber(unsigned long n, std::uint8_t base);
    std::size_t printFloat(double number, std::uint8_t digits);
    std::size_t draw_string(const char *string, std::int32_t x, std::int32_t y, textdatum_t datum);

    bool draw_bmp(DataWrapper* data, std::int32_t x, std::int32_t y);
    bool draw_jpg(DataWrapper* data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, jpeg_div::jpeg_div_t scale);
    bool draw_png(DataWrapper* data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, double scale);


    virtual void setWindow_impl(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye) = 0;
    virtual void writeFillRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) = 0;
    virtual void drawPixel_impl(std::int32_t x, std::int32_t y) = 0;
    virtual void copyRect_impl(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y) = 0;
    virtual void readRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param) = 0;
    virtual void pushImage_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param, bool use_dma) = 0;
    virtual void pushImageARGB_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param) = 0;
    virtual void writePixels_impl(std::int32_t length, pixelcopy_t* param) = 0;
    virtual void writePixelsDMA_impl(const void* data, std::int32_t length) = 0;
    virtual void pushBlock_impl(std::int32_t len) = 0;
    virtual bool isReadable_impl(void) const = 0;
    virtual std::int_fast8_t getRotation_impl(void) const = 0;
    virtual RGBColor* getPalette_impl(void) const { return nullptr; }

    virtual void initDMA_impl(void) = 0;
    virtual void waitDMA_impl(void) = 0;
    virtual bool dmaBusy_impl(void) = 0;
    virtual void beginTransaction_impl(void) = 0;
    virtual void endTransaction_impl(void) = 0;

    static void tmpBeginTransaction(void* lgfx)
    {
      auto me = (LGFXBase*)lgfx;
      if (me->_transaction_count) { me->beginTransaction(); }
    }

    static void tmpEndTransaction(void* lgfx)
    {
      auto me = (LGFXBase*)lgfx;
      if (me->_transaction_count) { me->endTransaction(); }
    }

    void prepareTmpTransaction(DataWrapper* data)
    {
      if (data->need_transaction && isSPIShared())
      {
        data->parent = this;
        data->fp_pre_read  = tmpEndTransaction;
        data->fp_post_read = tmpBeginTransaction;
      }
    }
    __attribute__ ((always_inline)) inline void startWrite(bool transaction)
    {
      if (1 == ++_transaction_count && transaction) { beginTransaction(); }
    }
  };

//----------------------------------------------------------------------------

  class LovyanGFX : public
  #ifdef LGFX_FILESYSTEM_SUPPORT_HPP_
      LGFX_FILESYSTEM_Support<
  #endif
       LGFXBase
  #ifdef LGFX_FILESYSTEM_SUPPORT_HPP_
      >
  #endif
  {
  private:
    using LGFXBase::_get_text_filled_x;
    using LGFXBase::_set_text_filled_x;
    using LGFXBase::_get_font_metrics;
    using LGFXBase::writeRawColor;
  };

//----------------------------------------------------------------------------

}

typedef lgfx::LovyanGFX LovyanGFX;

#endif
