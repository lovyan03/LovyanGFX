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
#include "lgfx_fonts.hpp"

#include <cmath>
#include <string>
#include <memory>

#if defined (ARDUINO)
#include <Print.h>
#endif

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  class PanelCommon;
  class TouchCommon;
  class LGFX_Sprite;
  class FileWrapper;

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
// rgb888 : uint32_t
// rgb565 : uint16_t & int16_t & int
// rgb332 : uint8_t
    __attribute__ ((always_inline)) inline void setColor(uint8_t r, uint8_t g, uint8_t b) { _color.raw = _write_conv.convert(lgfx::color888(r,g,b)); }
    template<typename T> __attribute__ ((always_inline)) inline void setColor(T color) { _color.raw = _write_conv.convert(color); }
                         __attribute__ ((always_inline)) inline void setRawColor(uint32_t c) { _color.raw = c; }

    template<typename T> __attribute__ ((always_inline)) inline void setBaseColor(T c) { _base_rgb888 = hasPalette() ? c : convert_to_rgb888(c); }
    uint32_t getBaseColor(void) const { return _base_rgb888; }

// AdafruitGFX compatible functions.
// However, startWrite and endWrite have an internal counter and are executed when the counter is 0.
// If you do not want to the counter, call the transaction function directly.
    __attribute__ ((always_inline)) inline void startWrite(void) {                           if (1 == ++_transaction_count) beginTransaction(); }
    __attribute__ ((always_inline)) inline void endWrite(void)   { if (_transaction_count) { if (1 == _transaction_count) { endTransaction(); } --_transaction_count; } }
    __attribute__ ((always_inline)) inline void writePixel(int32_t x, int32_t y)  { if (x >= _clip_l && x <= _clip_r && y >= _clip_t && y <= _clip_b) writeFillRect_impl(x, y, 1, 1); }
    template<typename T> inline void writePixel    ( int32_t x, int32_t y                      , const T& color) { setColor(color); writePixel    (x, y      ); }
    template<typename T> inline void writeFastVLine( int32_t x, int32_t y           , int32_t h, const T& color) { setColor(color); writeFastVLine(x, y   , h); }
                                void writeFastVLine( int32_t x, int32_t y           , int32_t h);
    template<typename T> inline void writeFastHLine( int32_t x, int32_t y, int32_t w           , const T& color) { setColor(color); writeFastHLine(x, y, w   ); }
                                void writeFastHLine( int32_t x, int32_t y, int32_t w);
    template<typename T> inline void writeFillRect ( int32_t x, int32_t y, int32_t w, int32_t h, const T& color) { setColor(color); writeFillRect (x, y, w, h); }
                                void writeFillRect ( int32_t x, int32_t y, int32_t w, int32_t h);
    template<typename T> inline void writeColor    ( const T& color, int32_t length) {      if (0 >= length) return; setColor(color);    pushBlock_impl(length); }
    template<typename T> inline void writeFillRectPreclipped( int32_t x, int32_t y, int32_t w, int32_t h, const T& color) { setColor(color); writeFillRect_impl (x, y, w, h); }
                                void writeFillRectPreclipped( int32_t x, int32_t y, int32_t w, int32_t h)                 {                  writeFillRect_impl (x, y, w, h); }


    __attribute__ ((always_inline)) inline void drawPixel( int32_t x, int32_t y ) { if (x >= _clip_l && x <= _clip_r && y >= _clip_t && y <= _clip_b) drawPixel_impl(x, y); }
    template<typename T> inline void drawPixel       ( int32_t x, int32_t y                                 , const T& color) { setColor(color); drawPixel    (x, y         ); }
    template<typename T> inline void drawFastVLine   ( int32_t x, int32_t y           , int32_t h           , const T& color) { setColor(color); drawFastVLine(x, y   , h   ); }
                                void drawFastVLine   ( int32_t x, int32_t y           , int32_t h);
    template<typename T> inline void drawFastHLine   ( int32_t x, int32_t y, int32_t w                      , const T& color) { setColor(color); drawFastHLine(x, y, w      ); }
                                void drawFastHLine   ( int32_t x, int32_t y, int32_t w);
    template<typename T> inline void fillRect        ( int32_t x, int32_t y, int32_t w, int32_t h           , const T& color) { setColor(color); fillRect     (x, y, w, h   ); }
                                void fillRect        ( int32_t x, int32_t y, int32_t w, int32_t h);
    template<typename T> inline void drawRect        ( int32_t x, int32_t y, int32_t w, int32_t h           , const T& color) { setColor(color); drawRect     (x, y, w, h   ); }
                                void drawRect        ( int32_t x, int32_t y, int32_t w, int32_t h);
    template<typename T> inline void drawRoundRect   ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, const T& color) { setColor(color); drawRoundRect(x, y, w, h, r); }
                                void drawRoundRect   ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r);
    template<typename T> inline void fillRoundRect   ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, const T& color) { setColor(color); fillRoundRect(x, y, w, h, r); }
                                void fillRoundRect   ( int32_t x, int32_t y, int32_t w, int32_t h, int32_t r);
    template<typename T> inline void drawCircle      ( int32_t x, int32_t y                      , int32_t r, const T& color) { setColor(color); drawCircle   (x, y      , r); }
                                void drawCircle      ( int32_t x, int32_t y                      , int32_t r);
    template<typename T> inline void fillCircle      ( int32_t x, int32_t y                      , int32_t r, const T& color) { setColor(color); fillCircle   (x, y      , r); }
                                void fillCircle      ( int32_t x, int32_t y                      , int32_t r);
    template<typename T> inline void drawEllipse     ( int32_t x, int32_t y, int32_t rx, int32_t ry         , const T& color) { setColor(color); drawEllipse  (x, y, rx, ry ); }
                                void drawEllipse     ( int32_t x, int32_t y, int32_t rx, int32_t ry);     
    template<typename T> inline void fillEllipse     ( int32_t x, int32_t y, int32_t rx, int32_t ry         , const T& color) { setColor(color); fillEllipse  (x, y, rx, ry ); }
                                void fillEllipse     ( int32_t x, int32_t y, int32_t rx, int32_t ry);
    template<typename T> inline void drawLine        ( int32_t x0, int32_t y0, int32_t x1, int32_t y1       , const T& color)  { setColor(color); drawLine(    x0, y0, x1, y1        ); }
                                void drawLine        ( int32_t x0, int32_t y0, int32_t x1, int32_t y1);
    template<typename T> inline void drawTriangle    ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); drawTriangle(x0, y0, x1, y1, x2, y2); }
                                void drawTriangle    ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    template<typename T> inline void fillTriangle    ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); fillTriangle(x0, y0, x1, y1, x2, y2); }
                                void fillTriangle    ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    template<typename T> inline void drawBezier      ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); drawBezier(x0, y0, x1, y1, x2, y2); }
                                void drawBezier      ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    template<typename T> inline void drawBezier      ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, const T& color)  { setColor(color); drawBezier(x0, y0, x1, y1, x2, y2, x3, y3); }
                                void drawBezier      ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3);
    template<typename T> inline void drawBezierHelper( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const T& color)  { setColor(color); drawBezierHelper(x0, y0, x1, y1, x2, y2); }
                                void drawBezierHelper( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    template<typename T> inline void drawEllipseArc  ( int32_t x, int32_t y, int32_t r0x, int32_t r1x, int32_t r0y, int32_t r1y, float angle0, float angle1, const T& color) { setColor(color); drawEllipseArc( x, y, r0x, r1x, r0y, r1y, angle0, angle1); }
                                void drawEllipseArc  ( int32_t x, int32_t y, int32_t r0x, int32_t r1x, int32_t r0y, int32_t r1y, float angle0, float angle1);
    template<typename T> inline void fillEllipseArc  ( int32_t x, int32_t y, int32_t r0x, int32_t r1x, int32_t r0y, int32_t r1y, float angle0, float angle1, const T& color) { setColor(color); fillEllipseArc( x, y, r0x, r1x, r0y, r1y, angle0, angle1); }
                                void fillEllipseArc  ( int32_t x, int32_t y, int32_t r0x, int32_t r1x, int32_t r0y, int32_t r1y, float angle0, float angle1);
    template<typename T> inline void drawArc         ( int32_t x, int32_t y, int32_t r0, int32_t r1, float angle0, float angle1, const T& color) { setColor(color); drawEllipseArc( x, y, r0, r1, r0, r1, angle0, angle1); }
                                void drawArc         ( int32_t x, int32_t y, int32_t r0, int32_t r1, float angle0, float angle1)                 {                  drawEllipseArc( x, y, r0, r1, r0, r1, angle0, angle1); }
    template<typename T> inline void fillArc         ( int32_t x, int32_t y, int32_t r0, int32_t r1, float angle0, float angle1, const T& color) { setColor(color); fillEllipseArc( x, y, r0, r1, r0, r1, angle0, angle1); }
                                void fillArc         ( int32_t x, int32_t y, int32_t r0, int32_t r1, float angle0, float angle1)                 {                  fillEllipseArc( x, y, r0, r1, r0, r1, angle0, angle1); }
    template<typename T> inline void drawCircleHelper( int32_t x, int32_t y, int32_t r, uint_fast8_t cornername                , const T& color) { setColor(color); drawCircleHelper(x, y, r, cornername    ); }
                                void drawCircleHelper( int32_t x, int32_t y, int32_t r, uint_fast8_t cornername);
    template<typename T> inline void fillCircleHelper( int32_t x, int32_t y, int32_t r, uint_fast8_t corners, int32_t delta, const T& color)  { setColor(color); fillCircleHelper(x, y, r, corners, delta); }
                                void fillCircleHelper( int32_t x, int32_t y, int32_t r, uint_fast8_t corners, int32_t delta);

    template<typename T> inline void floodFill( int32_t x, int32_t y, const T& color) { setColor(color); floodFill(x, y); }
                                void floodFill( int32_t x, int32_t y                );
    template<typename T> inline void paint    ( int32_t x, int32_t y, const T& color) { setColor(color); floodFill(x, y); }
                         inline void paint    ( int32_t x, int32_t y                ) {                  floodFill(x, y); }

    template<typename T> inline void fillAffine(const float matrix[6], int32_t w, int32_t h, const T& color) { setColor(color); fillAffine(matrix, w, h); }
                                void fillAffine(const float matrix[6], int32_t w, int32_t h);

    template<typename T> inline void drawGradientHLine( int32_t x, int32_t y, int32_t w, const T& colorstart, const T& colorend ) { drawGradientLine( x, y, x + w - 1, y, colorstart, colorend ); }
    template<typename T> inline void drawGradientVLine( int32_t x, int32_t y, int32_t h, const T& colorstart, const T& colorend ) { drawGradientLine( x, y, x, y + h - 1, colorstart, colorend ); }
    template<typename T> inline void drawGradientLine ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, const T& colorstart, const T& colorend ) { draw_gradient_line( x0, y0, x1, y1, convert_to_rgb888(colorstart), convert_to_rgb888(colorend) ); }

    template<typename T> inline void fillScreen  ( const T& color) { setColor(color); fillRect(0, 0, _width, _height); }
                         inline void fillScreen  ( void )          {                  fillRect(0, 0, _width, _height); }

    template<typename T> inline void clear       ( const T& color) { setBaseColor(color); clear(); }
                         inline void clear       ( void )          { setColor(_base_rgb888); fillScreen(); }
    template<typename T> inline void clearDisplay( const T& color) { setBaseColor(color); clear(); }
                         inline void clearDisplay( void )          { setColor(_base_rgb888); fillScreen(); }

    template<typename T> inline void pushBlock  ( const T& color, int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }


    __attribute__ ((always_inline)) inline static uint8_t  color332(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color332(r, g, b); }
    __attribute__ ((always_inline)) inline static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color565(r, g, b); }
    __attribute__ ((always_inline)) inline static uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return lgfx::color888(r, g, b); }
    __attribute__ ((always_inline)) inline static uint16_t swap565( uint8_t r, uint8_t g, uint8_t b) { return lgfx::swap565( r, g, b); }
    __attribute__ ((always_inline)) inline static uint32_t swap888( uint8_t r, uint8_t g, uint8_t b) { return lgfx::swap888( r, g, b); }
    __attribute__ ((always_inline)) inline static uint8_t  color16to8(uint32_t rgb565) { return lgfx::convert_rgb565_to_rgb332(rgb565); }
    __attribute__ ((always_inline)) inline static uint16_t color8to16(uint32_t rgb332) { return lgfx::convert_rgb332_to_rgb565(rgb332); }
    __attribute__ ((always_inline)) inline static uint32_t color16to24(uint32_t rgb565) { return lgfx::convert_rgb565_to_rgb888(rgb565); }
    __attribute__ ((always_inline)) inline static uint16_t color24to16(uint32_t rgb888) { return lgfx::convert_rgb888_to_rgb565(rgb888); }

    __attribute__ ((always_inline)) inline void setPivot(float x, float y) { _xpivot = x; _ypivot = y; }
    __attribute__ ((always_inline)) inline float getPivotX(void) const { return _xpivot; }
    __attribute__ ((always_inline)) inline float getPivotY(void) const { return _ypivot; }

    __attribute__ ((always_inline)) inline int32_t       width            (void) const { return _width; }
    __attribute__ ((always_inline)) inline int32_t       height           (void) const { return _height; }
    __attribute__ ((always_inline)) inline int_fast8_t   getRotation      (void) const { return getRotation_impl(); }
    __attribute__ ((always_inline)) inline color_depth_t getColorDepth    (void) const { return _write_conv.depth; }
    __attribute__ ((always_inline)) inline color_conv_t* getColorConverter(void)       { return &_write_conv; }
    __attribute__ ((always_inline)) inline RGBColor*     getPalette       (void) const { return getPalette_impl(); }
    __attribute__ ((always_inline)) inline uint32_t      getPaletteCount  (void) const { return _palette_count; }
    __attribute__ ((always_inline)) inline bool          hasPalette       (void) const { return _palette_count; }
    __attribute__ ((always_inline)) inline bool          isSPIShared      (void) const { return _spi_shared; }
    __attribute__ ((always_inline)) inline bool          isReadable       (void) const { return isReadable_impl(); }
    __attribute__ ((always_inline)) inline bool          getSwapBytes     (void) const { return _swapBytes; }
    __attribute__ ((always_inline)) inline void          setSwapBytes     (bool swap) { _swapBytes = swap; }
    __attribute__ ((always_inline)) inline void          setSPIShared     (bool shared) { _spi_shared = shared; }

    __attribute__ ((always_inline)) inline void beginTransaction(void) { beginTransaction_impl(); }
    __attribute__ ((always_inline)) inline void endTransaction(void)   { endTransaction_impl(); }
    __attribute__ ((always_inline)) inline void initDMA(void) { initDMA_impl(); }
    __attribute__ ((always_inline)) inline void waitDMA(void) { waitDMA_impl(); }
    __attribute__ ((always_inline)) inline bool dmaBusy(void) { return dmaBusy_impl(); }
    __attribute__ ((always_inline)) inline void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) { setWindow_impl(xs, ys, xe, ye); }

    void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h);

    void setClipRect(int32_t x, int32_t y, int32_t w, int32_t h);
    void getClipRect(int32_t *x, int32_t *y, int32_t *w, int32_t *h);
    void clearClipRect(void);

    template<typename T>
    void setScrollRect(int32_t x, int32_t y, int32_t w, int32_t h, const T& color) {
      _base_rgb888 = convert_to_rgb888(color);
      setScrollRect(x, y, w, h);
    }
    void setScrollRect(int32_t x, int32_t y, int32_t w, int32_t h);
    void getScrollRect(int32_t *x, int32_t *y, int32_t *w, int32_t *h);
    void clearScrollRect(void);

    __attribute__ ((always_inline)) inline void pushPixelsDMA( const void* data, int32_t len) { if (len < 0) return; startWrite(); writePixelsDMA_impl(data, len); endWrite(); }
    __attribute__ ((always_inline)) inline void writePixelsDMA(const void* data, int32_t len) { if (len < 0) return;               writePixelsDMA_impl(data, len);             }

    template<typename T>
    __attribute__ ((always_inline)) inline void pushPixels(T*              data, int32_t len           ) { startWrite(); writePixels(data, len      ); endWrite(); }
    __attribute__ ((always_inline)) inline void pushPixels(const uint16_t* data, int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }
    __attribute__ ((always_inline)) inline void pushPixels(const void*     data, int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }

    template<typename T>
    void writePixels(const T *data, int32_t len)                   { auto pc = create_pc_fast(data      ); writePixels_impl(len, &pc); }
    void writePixels(const uint16_t* data, int32_t len, bool swap) { auto pc = create_pc_fast(data, swap); writePixels_impl(len, &pc); }
    void writePixels(const void*     data, int32_t len, bool swap) { auto pc = create_pc_fast(data, swap); writePixels_impl(len, &pc); }

    template<typename T>
    void writeIndexedPixels(const uint8_t *data, T* palette, int32_t len, lgfx::color_depth_t colordepth = lgfx::rgb332_1Byte)
    {
      auto pc = create_pc_fast(data, palette, colordepth);
      writePixels_impl(len, &pc);
    }

    template<typename T> void drawBitmap (int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, const T& color                    ) { draw_bitmap (x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawBitmap (int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, const T& fgcolor, const T& bgcolor) { draw_bitmap (x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }
    template<typename T> void drawXBitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, const T& color                    ) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(color)); }
    template<typename T> void drawXBitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, const T& fgcolor, const T& bgcolor) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }

    template<typename T>
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const T* data)
    {
      auto pc = create_pc(data);
      pushImage(x, y, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr(data, transparent);
      pushImage(x, y, w, h, &pc);
    }

    template<typename T>
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth);
      pushImage(x, y, w, h, &pc);
    }

    template<typename T>
    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth, transparent);
      pushImage(x, y, w, h, &pc);
    }

    template<typename T>
    void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, const T* data)
    {
      auto pc = create_pc(data);
      pushImage(x, y, w, h, &pc, true);
    }

    template<typename T>
    void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth);
      pushImage(x, y, w, h, &pc, true);
    }

    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t *param, bool use_dma = false);

//----------------------------------------------------------------------------

    template<typename T>
    void pushImageRotateZoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, const T* data)
    {
      auto pc = create_pc(data);
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImageRotateZoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr(data, transparent);
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T>
    void pushImageRotateZoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth);
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T>
    void pushImageRotateZoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, const void* data, uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth, transparent);
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }


    template<typename T>
    void pushImageRotateZoomWithAA(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, const T* data)
    {
      auto pc = create_pc_antialias(data);
      push_image_rotate_zoom_aa(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImageRotateZoomWithAA(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr_antialias(data, transparent);
      push_image_rotate_zoom_aa(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T>
    void pushImageRotateZoomWithAA(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_antialias(data, palette, depth);
      push_image_rotate_zoom_aa(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T>
    void pushImageRotateZoomWithAA(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, const void* data, uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_antialias(data, palette, depth, transparent);
      push_image_rotate_zoom_aa(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

//----------------------------------------------------------------------------

    template<typename T>
    void pushImageAffine(const float matrix[6], int32_t w, int32_t h, const T* data)
    {
      auto pc = create_pc(data);
      push_image_affine(matrix, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImageAffine(const float matrix[6], int32_t w, int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr(data, transparent);
      push_image_affine(matrix, w, h, &pc);
    }

    template<typename T>
    void pushImageAffine(const float matrix[6], int32_t w, int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth);
      push_image_affine(matrix, w, h, &pc);
    }

    template<typename T>
    void pushImageAffine(const float matrix[6], int32_t w, int32_t h, const void* data, uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth, transparent);
      push_image_affine(matrix, w, h, &pc);
    }


    template<typename T>
    void pushImageAffineWithAA(const float matrix[6], int32_t w, int32_t h, const T* data)
    {
      auto pc = create_pc_antialias(data);
      push_image_affine_aa(matrix, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImageAffineWithAA(const float matrix[6], int32_t w, int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr_antialias(data, transparent);
      push_image_affine_aa(matrix, w, h, &pc);
    }

    template<typename T>
    void pushImageAffineWithAA(const float matrix[6], int32_t w, int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_antialias(data, palette, depth);
      push_image_affine_aa(matrix, w, h, &pc);
    }

    template<typename T>
    void pushImageAffineWithAA(const float matrix[6], int32_t w, int32_t h, const void* data, uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_antialias(data, palette, depth, transparent);
      push_image_affine_aa(matrix, w, h, &pc);
    }

//----------------------------------------------------------------------------

    /// read RGB565 16bit color
    uint16_t readPixel(int32_t x, int32_t y)
    {
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return 0;

      pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, getPalette());
      uint_fast16_t data = 0;

      readRect_impl(x, y, 1, 1, &data, &p);

      return __builtin_bswap16(data);
    }

    /// read RGB888 24bit color
    RGBColor readPixelRGB(int32_t x, int32_t y)
    {
      RGBColor data[1];
      if (x < _clip_l || x > _clip_r || y < _clip_t || y > _clip_b) return data[0];

      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, getPalette());

      readRect_impl(x, y, 1, 1, data, &p);

      return data[0];
    }

    __attribute__ ((always_inline)) inline
    void readRectRGB( int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* data) { readRectRGB(x, y, w, h, (bgr888_t*)data); }
    void readRectRGB( int32_t x, int32_t y, int32_t w, int32_t h, RGBColor* data)
    {
      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, getPalette());
      read_rect(x, y, w, h, data, &p);
    }

    template<typename T> inline
    void readRect( int32_t x, int32_t y, int32_t w, int32_t h, T* data)
    {
      pixelcopy_t p(nullptr, get_depth<T>::value, _read_conv.depth, false, getPalette());
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value || std::is_same<argb8888_t, T>::value || std::is_same<grayscale_t, T>::value || p.fp_copy == nullptr)
      {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine_dst<T>(_read_conv.depth);
      }
      read_rect(x, y, w, h, data, &p);
    }

    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t*  data);
    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data);
    void readRect(int32_t x, int32_t y, int32_t w, int32_t h, void*          data);

    void scroll(int_fast16_t dx, int_fast16_t dy = 0);
    void copyRect(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y);


    [[deprecated("use IFont")]]
    void setCursor( int32_t x, int32_t y, uint8_t font) { _filled_x = 0; _cursor_x = x; _cursor_y = y; setFont(fontdata[font]); }
    void setCursor( int32_t x, int32_t y, const IFont* font) { _filled_x = 0; _cursor_x = x; _cursor_y = y; setFont(font); }
    void setCursor( int32_t x, int32_t y)                    { _filled_x = 0; _cursor_x = x; _cursor_y = y; }
    int32_t getCursorX(void) const { return _cursor_x; }
    int32_t getCursorY(void) const { return _cursor_y; }
    void setTextStyle(const TextStyle& text_style) { _text_style = text_style; }
    const TextStyle& getTextStyle(void) const { return _text_style; }
    void setTextSize(float size) { setTextSize(size, size); }
    void setTextSize(float sx, float sy) { _text_style.size_x = (sx > 0) ? sx : 1; _text_style.size_y = (sy > 0) ? sy : 1; }
    float getTextSizeX(void) const { return _text_style.size_x; }
    float getTextSizeY(void) const { return _text_style.size_y; }
    //[[deprecated("use textdatum_t")]]
    void setTextDatum(uint8_t datum) { _text_style.datum = (textdatum_t)datum; }
    void setTextDatum(textdatum_t datum) { _text_style.datum = datum; }
    textdatum_t getTextDatum(void) const { return _text_style.datum; }
    void setTextPadding(uint32_t padding_x) { _padding_x = padding_x; }
    uint32_t getTextPadding(void) const { return _padding_x; }
    void setTextWrap( bool wrapX, bool wrapY = false) { _textwrap_x = wrapX; _textwrap_y = wrapY; }
    void setTextScroll(bool scroll) { _textscroll = scroll; if (_cursor_x < this->_sx) { _cursor_x = this->_sx; } if (_cursor_y < this->_sy) { _cursor_y = this->_sy; } }

    template<typename T>
    void setTextColor(T color) {
      _text_style.fore_rgb888 = _text_style.back_rgb888 = this->hasPalette() ? color : convert_to_rgb888(color);
    }
    template<typename T1, typename T2>
    void setTextColor(T1 fgcolor, T2 bgcolor) {
      if (!this->hasPalette()) {
        _text_style.fore_rgb888 = convert_to_rgb888(fgcolor);
        _text_style.back_rgb888 = convert_to_rgb888(bgcolor);
      } else {
        _text_style.fore_rgb888 = fgcolor;
        _text_style.back_rgb888 = bgcolor;
      }
    }

    [[deprecated("use IFont")]]
    int32_t fontHeight(uint8_t font) const { return ((const BaseFont*)fontdata[font])->height * _text_style.size_y; }
    int32_t fontHeight(const IFont* font) const;
    int32_t fontHeight(void) const { return _font_metrics.height * _text_style.size_y; }
    int32_t fontWidth(uint8_t font) const { return ((const BaseFont*)fontdata[font])->width * _text_style.size_x; }
    int32_t fontWidth(const IFont* font) const;
    int32_t fontWidth(void) const { return _font_metrics.width * _text_style.size_x; }
    int32_t textLength(const char *string, int32_t width);
    int32_t textWidth(const char *string);

    [[deprecated("use IFont")]]
    inline size_t drawString(const char *string, int32_t x, int32_t y, uint8_t      font) { setFont(fontdata[font]); return draw_string(string, x, y, _text_style.datum); }
    inline size_t drawString(const char *string, int32_t x, int32_t y, const IFont* font) { setFont(font          ); return draw_string(string, x, y, _text_style.datum); }
    inline size_t drawString(const char *string, int32_t x, int32_t y                   ) {                          return draw_string(string, x, y, _text_style.datum); }

    [[deprecated("use IFont")]]
    inline size_t drawNumber(long long_num, int32_t poX, int32_t poY, uint8_t      font) { setFont(fontdata[font]); return drawNumber(long_num, poX, poY); }
    inline size_t drawNumber(long long_num, int32_t poX, int32_t poY, const IFont* font) { setFont(font          ); return drawNumber(long_num, poX, poY); }
           size_t drawNumber(long long_num, int32_t poX, int32_t poY);
    inline size_t drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY, const IFont* font) { setFont(font          ); return drawFloat(floatNumber, dp, poX, poY); }
    inline size_t drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY, uint8_t      font) { setFont(fontdata[font]); return drawFloat(floatNumber, dp, poX, poY); }
           size_t drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY);

    [[deprecated("use setTextDatum() and drawString()")]] inline size_t drawCentreString(const char *string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_center); }
    [[deprecated("use setTextDatum() and drawString()")]] inline size_t drawCenterString(const char *string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_center); }
    [[deprecated("use setTextDatum() and drawString()")]] inline size_t drawRightString( const char *string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_right); }

  #if defined (ARDUINO)
    inline int32_t textLength(const String& string, int32_t width) { return textLength(string.c_str(), width); }
    inline int32_t textWidth(const String& string) { return textWidth(string.c_str()); }

    inline size_t drawString(const String& string, int32_t x, int32_t y              ) {                          return draw_string(string.c_str(), x, y, _text_style.datum); }
    inline size_t drawString(const String& string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, _text_style.datum); }

    [[deprecated("use setTextDatum() and drawString()")]] inline size_t drawCentreString(const String& string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }
    [[deprecated("use setTextDatum() and drawString()")]] inline size_t drawCenterString(const String& string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }
    [[deprecated("use setTextDatum() and drawString()")]] inline size_t drawRightString( const String& string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_right); }
  #endif

           size_t drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font);
//  inline size_t drawChar(uint16_t uniCode, int32_t x, int32_t y              ) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, &_text_style, _font); }
    inline size_t drawChar(uint16_t uniCode, int32_t x, int32_t y              ) { _filled_x = 0; return _font->drawChar(this, x, y, uniCode, &_text_style); }

    template<typename T>
    inline size_t drawChar(int32_t x, int32_t y, uint16_t uniCode, T color, T bg, float size) { return drawChar(x, y, uniCode, color, bg, size, size); }
    template<typename T>
    inline size_t drawChar(int32_t x, int32_t y, uint16_t uniCode, T color, T bg, float size_x, float size_y)
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
    uint8_t getTextFont(void) const
    {
      size_t i = 0;
      do {
        if (fontdata[i] == _font) return i;
      } while (fontdata[++i]);
      return 0;
    }

    //[[deprecated("use setFont(&fonts::Font)")]]
    void setTextFont(int f)
    {
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
    bool loadFont(const uint8_t* array);

    /// unload VLW font
    void unloadFont(void);

    /// show VLW font
    void showFont(uint32_t td);

    void cp437(bool enable = true) { _text_style.cp437 = enable; }  // AdafruitGFX compatible.

    epd_mode_t setEpdMode(epd_mode_t flg) { _epd_mode = flg; return flg; }
    epd_mode_t getEpdMode(void) const { return _epd_mode; }

    void setAttribute(attribute_t attr_id, uint8_t param);
    uint8_t getAttribute(attribute_t attr_id);
    uint8_t getAttribute(uint8_t attr_id) { return getAttribute((attribute_t)attr_id); }

    int32_t _get_text_filled_x(void) const { return _filled_x; }
    void _set_text_filled_x(int32_t x) { _filled_x = x; }
    FontMetrics _get_font_metrics(void) const { return _font_metrics; }

//----------------------------------------------------------------------------
// print & text support
//----------------------------------------------------------------------------
// Arduino Print.h compatible
  #if !defined (ARDUINO)
    size_t print(const char str[])      { return write(str); }
    size_t print(char c)                { return write(c); }
    size_t print(int  n, int base = 10) { return print((long)n, base); }
    size_t print(long n, int base = 10)
    {
      if (base == 0) { return write(n); }
      if (base == 10) {
        if (n < 0) {
          size_t t = print('-');
          return printNumber(-n, 10) + t;
        }
        return printNumber(n, 10);
      }
      return printNumber(n, base);
    }

    size_t print(unsigned char n, int base = 10) { return print((unsigned long)n, base); }
    size_t print(unsigned int  n, int base = 10) { return print((unsigned long)n, base); }
    size_t print(unsigned long n, int base = 10) { return (base) ? printNumber(n, base) : write(n); }
    size_t print(double        n, int digits= 2) { return printFloat(n, digits); }

    size_t println(void) { return print("\r\n"); }
    size_t println(const char c[])                 { size_t t = print(c); return println() + t; }
    size_t println(char c        )                 { size_t t = print(c); return println() + t; }
    size_t println(int  n, int base = 10)          { size_t t = print(n,base); return println() + t; }
    size_t println(long n, int base = 10)          { size_t t = print(n,base); return println() + t; }
    size_t println(unsigned char n, int base = 10) { size_t t = print(n,base); return println() + t; }
    size_t println(unsigned int  n, int base = 10) { size_t t = print(n,base); return println() + t; }
    size_t println(unsigned long n, int base = 10) { size_t t = print(n,base); return println() + t; }
    size_t println(double        n, int digits= 2) { size_t t = print(n, digits); return println() + t; }

  //size_t print(const String &s) { return write(s.c_str(), s.length()); }
  //size_t print(const __FlashStringHelper *s)   { return print(reinterpret_cast<const char *>(s)); }
  //size_t println(const String &s)              { size_t t = print(s); return println() + t; }
  //size_t println(const __FlashStringHelper *s) { size_t t = print(s); return println() + t; }

    size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)));

    size_t write(const char* str)                 { return (!str) ? 0 : write((const uint8_t*)str, strlen(str)); }
    size_t write(const char *buf, size_t size)    { return write((const uint8_t *) buf, size); }
  #else
    using Print::write;
  #endif
    size_t write(const uint8_t *buf, size_t size) { size_t n = 0; this->startWrite(); while (size--) { n += write(*buf++); } this->endWrite(); return n; }
    size_t write(uint8_t utf8);



#ifdef ARDUINO
    void qrcode(const String &string, int32_t x = -1, int32_t y = -1, int32_t width = -1, uint8_t version = 1) {
      qrcode(string.c_str(), x, y, width, version);
    }
#endif
    void qrcode(const char *string, int32_t x = -1, int32_t y = -1, int32_t width = -1, uint8_t version = 1);


    bool drawBmp(const uint8_t *bmp_data, uint32_t bmp_len, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      PointerWrapper data;
      data.set(bmp_data, bmp_len);
      return this->draw_bmp(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    bool drawJpg(const uint8_t *jpg_data, uint32_t jpg_len, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      PointerWrapper data;
      data.set(jpg_data, jpg_len);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]] bool drawJpg(const uint8_t *jpg_data, uint32_t jpg_len, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpg(jpg_data, jpg_len, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }
    bool drawPng(const uint8_t *png_data, uint32_t png_len, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      PointerWrapper data;
      data.set(png_data, png_len);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    inline bool drawBmp(DataWrapper *data, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return this->draw_bmp(data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawJpg(DataWrapper *data, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return this->draw_jpg(data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]] inline bool drawJpg(DataWrapper *data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpg(data, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }
    inline bool drawPng(DataWrapper *data, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return this->draw_png(data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    void* createPng( size_t* datalen, int32_t x = 0, int32_t y = 0, int32_t width = 0, int32_t height = 0);



    template<typename T>
    [[deprecated("use pushImage")]] void pushRect( int32_t x, int32_t y, int32_t w, int32_t h, const T* data) { pushImage(x, y, w, h, data); }

    template<typename T>
    [[deprecated("use pushBlock")]] void pushColor(const T& color, int32_t length) { if (0 >= length) return; setColor(color); startWrite(); pushBlock_impl(length); endWrite(); }
    template<typename T>
    [[deprecated("use pushBlock")]] void pushColor(const T& color                     ) {                          setColor(color); startWrite(); pushBlock_impl(1);      endWrite(); }

    template<typename T>
    [[deprecated("use pushPixels")]] void pushColors(T*              data, int32_t len           ) { startWrite(); writePixels(data, len            ); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const void*     data, int32_t len           ) { startWrite(); writePixels(data, len, _swapBytes); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const uint16_t* data, int32_t len           ) { startWrite(); writePixels(data, len, _swapBytes); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const uint8_t*  data, int32_t len           ) { startWrite(); writePixels((const rgb332_t*)data, len); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const void*     data, int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const uint16_t* data, int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  protected:
    bool _auto_display = true;
    PanelCommon* _panel = nullptr;
    TouchCommon* _touch = nullptr;
    FileWrapper* _font_file = nullptr;
    uint32_t _transaction_count = 0;
    int32_t _width = 0, _height = 0;
    int32_t  _sx, _sy, _sw, _sh; // for scroll zone

    int32_t _clip_l = 0, _clip_r = -1, _clip_t = 0, _clip_b = -1; // clip rect
    uint32_t _base_rgb888 = 0;  // gap fill colour for clear and scroll zone 
    raw_color_t _color = 0xFFFFFFU;

    color_conv_t _write_conv;
    color_conv_t _read_conv;

    uint32_t _palette_count = 0;

    float _xpivot;   // x pivot point coordinate
    float _ypivot;   // x pivot point coordinate

    bool _spi_shared = true;
    bool _swapBytes = false;
    epd_mode_t _epd_mode = epd_mode_t::epd_quality;

    enum utf8_decode_state_t
    { utf8_state0 = 0
    , utf8_state1 = 1
    , utf8_state2 = 2
    };
    utf8_decode_state_t _decoderState = utf8_state0;   // UTF8 decoder state
    uint_fast16_t _unicode_buffer = 0;   // Unicode code-point buffer

    int32_t _cursor_x = 0;  // print text cursor
    int32_t _cursor_y = 0;
    int32_t _filled_x = 0;  // print filled position
    int32_t _padding_x = 0;

    TextStyle _text_style;
    FontMetrics _font_metrics = { 6, 6, 0, 8, 8, 0, 7 }; // Font0 default metric
    const IFont* _font = &fonts::Font0;

    std::shared_ptr<RunTimeFont> _runtime_font;  // run-time generated font
    PointerWrapper _font_data;

    bool _textwrap_x = true;
    bool _textwrap_y = false;
    bool _textscroll = false;

    __attribute__ ((always_inline)) inline static bool _adjust_abs(int32_t& x, int32_t& w) { if (w < 0) { x += w + 1; w = -w; } return !w; }

    static bool _adjust_width(int32_t& x, int32_t& dx, int32_t& dw, int32_t left, int32_t width)
    {
      if (x < left) { dx = -x; dw += x; x = left; }
      if (dw > left + width - x) dw = left + width  - x;
      return (dw <= 0);
    }

//----------------------------------------------------------------------------

    template<typename T>
    pixelcopy_t create_pc_fast(const T *data)
    {
      auto dst_depth = _write_conv.depth;
      pixelcopy_t pc(data, dst_depth, get_depth<T>::value, hasPalette());
      if (hasPalette() || dst_depth < rgb332_1Byte)
      {
        pc.fp_copy = pixelcopy_t::copy_bit_fast;
      }
      else
      if (dst_depth > rgb565_2Byte)
      {
        if (     dst_depth == rgb888_3Byte) { pc.fp_copy = pixelcopy_t::copy_rgb_fast<bgr888_t, T>; }
        else if (dst_depth == rgb666_3Byte) { pc.fp_copy = pixelcopy_t::copy_rgb_fast<bgr666_t, T>; }
        else                                { pc.fp_copy = pixelcopy_t::copy_rgb_fast<argb8888_t, T>; }
      }
      else
      {
        if (dst_depth == rgb565_2Byte) { pc.fp_copy = pixelcopy_t::copy_rgb_fast<swap565_t, T>; }
        else                           { pc.fp_copy = pixelcopy_t::copy_rgb_fast<rgb332_t, T>; }
      }
      return pc;
    }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const uint8_t  *data) { return create_pc_fast(reinterpret_cast<const rgb332_t*>(data)); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const uint16_t *data) { return create_pc_fast(data, _swapBytes); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const void     *data) { return create_pc_fast(data, _swapBytes); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const uint16_t *data, bool swap)
    {
      return swap && !hasPalette() && _write_conv.depth >= 8
           ? create_pc_fast(reinterpret_cast<const rgb565_t* >(data))
           : create_pc_fast(reinterpret_cast<const swap565_t*>(data));
    }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const void *data, bool swap)
    {
      return swap && !hasPalette() && _write_conv.depth >= 8
           ? create_pc_fast(reinterpret_cast<const rgb888_t*>(data))
           : create_pc_fast(reinterpret_cast<const bgr888_t*>(data));
    }

    template<typename T>
    pixelcopy_t create_pc_fast(const void *data, const T *palette, lgfx::color_depth_t src_depth)
    {
      auto dst_depth = _write_conv.depth;
/*
      pixelcopy_t pc(data, dst_depth, src_depth, hasPalette(), palette);
/*/
      pixelcopy_t pc;
      pc.src_data  = data   ;
      pc.palette   = palette;
      pc.src_bits  = src_depth > 8 ? (src_depth + 7) & ~7 : src_depth;
      pc.dst_bits  = dst_depth > 8 ? (dst_depth + 7) & ~7 : dst_depth;
      pc.src_mask  = (1 << pc.src_bits) - 1 ;
      pc.dst_mask  = (1 << pc.dst_bits) - 1 ;
      pc.no_convert= src_depth == dst_depth;
//*/
      if (hasPalette() || dst_depth < rgb332_1Byte)
      {
        if (palette && (dst_depth == rgb332_1Byte) && (src_depth == rgb332_1Byte))
        {
          pc.fp_copy = pixelcopy_t::copy_rgb_fast<rgb332_t, rgb332_t>;
        }
        else
        {
          pc.fp_copy = pixelcopy_t::copy_bit_fast;
        }
      }
      else
      {
        if (dst_depth > rgb565_2Byte)
        {
          if (     dst_depth == rgb888_3Byte) { pc.fp_copy = pixelcopy_t::copy_palette_fast<bgr888_t, T>; }
          else if (dst_depth == rgb666_3Byte) { pc.fp_copy = pixelcopy_t::copy_palette_fast<bgr666_t, T>; }
          else                                { pc.fp_copy = pixelcopy_t::copy_palette_fast<argb8888_t, T>; }
        }
        else
        {
          if (dst_depth == rgb565_2Byte) { pc.fp_copy = pixelcopy_t::copy_palette_fast<swap565_t, T>; }
          else                           { pc.fp_copy = pixelcopy_t::copy_palette_fast<rgb332_t, T>; }
        }
      }
      return pc;
    }


    template<typename T>
    pixelcopy_t create_pc(const T *data)
    {
      pixelcopy_t pc(data, _write_conv.depth, get_depth<T>::value, hasPalette());
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value || std::is_same<argb8888_t, T>::value || std::is_same<grayscale_t, T>::value)
      {
        pc.no_convert = false;
        pc.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine<T>(_write_conv.depth);
      }
      return pc;
    }

    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const uint8_t  *data) { return create_pc(reinterpret_cast<const rgb332_t*>(data)); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const uint16_t *data) { return create_pc(data, _swapBytes); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const void          *data) { return create_pc(data, _swapBytes); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const uint16_t *data, bool swap)
    {
      return swap && !hasPalette() && _write_conv.depth >= 8
           ? create_pc(reinterpret_cast<const rgb565_t* >(data))
           : create_pc(reinterpret_cast<const swap565_t*>(data));
    }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const void *data, bool swap)
    {
      return swap && !hasPalette() && _write_conv.depth >= 8
           ? create_pc(reinterpret_cast<const rgb888_t*>(data))
           : create_pc(reinterpret_cast<const bgr888_t*>(data));
    }

    template<typename T>
    pixelcopy_t create_pc_rawtr(const T *data, uint32_t raw_transparent)
    {
      if (std::is_same<rgb565_t, T>::value) { raw_transparent = getSwap16(raw_transparent); }
      if (std::is_same<rgb888_t, T>::value) { raw_transparent = getSwap24(raw_transparent); }
      pixelcopy_t pc(data, _write_conv.depth, get_depth<T>::value, hasPalette(), nullptr, raw_transparent);
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value || std::is_same<argb8888_t, T>::value || std::is_same<grayscale_t, T>::value)
      {
        pc.no_convert = false;
        pc.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine<T>(_write_conv.depth);
        pc.fp_skip = pixelcopy_t::skip_rgb_affine<T>;
      }
      return pc;
    }

    template<typename T1, typename T2>
    pixelcopy_t create_pc_tr(const T1 *data, const T2& transparent)
    {
      return create_pc_rawtr( data
                            , (std::is_same<T1, T2>::value)
                              ? transparent
                              : get_fp_convert_src<T2>(get_depth<T1>::value, false)(transparent));
    }

    template<typename T> pixelcopy_t create_pc_tr(const uint8_t  *data, const T& transparent) { return create_pc_tr(reinterpret_cast<const rgb332_t*>(data), transparent); }
    template<typename T> pixelcopy_t create_pc_tr(const uint16_t *data, const T& transparent) { return create_pc_tr(data, transparent, _swapBytes); }
    template<typename T> pixelcopy_t create_pc_tr(const void          *data, const T& transparent) { return create_pc_tr(data, transparent, _swapBytes); }
    template<typename T> pixelcopy_t create_pc_tr(const uint16_t *data, const T& transparent, bool swap)
    {
      return swap && _write_conv.depth >= 8 && !hasPalette()
           ? create_pc_tr(reinterpret_cast<const rgb565_t* >(data), transparent)
           : create_pc_tr(reinterpret_cast<const swap565_t*>(data), transparent);
    }
    template<typename T> pixelcopy_t create_pc_tr(const void *data, const T& transparent, bool swap)
    {
      return swap && _write_conv.depth >= 8 && !hasPalette()
           ? create_pc_tr(reinterpret_cast<const rgb888_t*>(data), transparent)
           : create_pc_tr(reinterpret_cast<const bgr888_t*>(data), transparent);
    }

    pixelcopy_t create_pc_palette(const void *data, const bgr888_t *palette, lgfx::color_depth_t depth, uint32_t transparent = ~0u)
    {
      return pixelcopy_t (data, _write_conv.depth, depth, hasPalette(), palette, transparent);
    }

    template<typename T>
    pixelcopy_t create_pc_palette(const void *data, const T *palette, lgfx::color_depth_t depth, uint32_t transparent = ~0u)
    {
      pixelcopy_t pc(data, getColorDepth(), depth, hasPalette(), palette, transparent);
      if (!hasPalette() && palette && getColorDepth() >= 8)
      {
        pc.fp_copy = pixelcopy_t::get_fp_copy_palette_affine<T>(getColorDepth());
      }
      return pc;
    }


    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_antialias(const uint8_t *data, uint32_t raw_transparent = ~0u)
    {
      return create_pc_antialias(reinterpret_cast<const rgb332_t*>(data), raw_transparent); 
    }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_antialias(const uint16_t *data, uint32_t raw_transparent = ~0u)
    {
      return _swapBytes
           ? create_pc_antialias(reinterpret_cast<const rgb565_t* >(data), raw_transparent)
           : create_pc_antialias(reinterpret_cast<const swap565_t*>(data), raw_transparent);
    }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_antialias(const void *data, uint32_t raw_transparent = ~0u)
    {
      return _swapBytes
           ? create_pc_antialias(reinterpret_cast<const rgb888_t*>(data), raw_transparent)
           : create_pc_antialias(reinterpret_cast<const bgr888_t*>(data), raw_transparent);
    }

    template<typename T>
    pixelcopy_t create_pc_antialias(const T* data, uint32_t raw_transparent = ~0u)
    {
      pixelcopy_t pc(data, argb8888_t::depth, get_depth<T>::value, false, nullptr, raw_transparent);
      pc.src_data = data;
      pc.fp_copy = pixelcopy_t::copy_rgb_antialias<T>;
      return pc;
    }

    template<typename T1, typename T2>
    pixelcopy_t create_pc_tr_antialias(const T1* data, const T2& transparent)
    {
      return create_pc_antialias( data
                                , std::is_same<T1, T2>::value
                                  ? transparent
                                  : get_fp_convert_src<T2>(get_depth<T1>::value, false)(transparent));
    }

    template<typename T>
    static pixelcopy_t create_pc_antialias(const void* data, const T* palette, lgfx::color_depth_t depth, uint32_t transparent = ~0u)
    {
      pixelcopy_t pc(data, argb8888_t::depth, depth, false, palette, transparent);
      if (palette)
      {
        pc.fp_copy = pixelcopy_t::copy_palette_antialias<T>;
      }
      else if (depth > rgb565_2Byte)
      {
        if (depth == rgb888_3Byte) {
          pc.fp_copy = pixelcopy_t::copy_rgb_antialias<bgr888_t>;
        } else {
          pc.fp_copy = pixelcopy_t::copy_rgb_antialias<bgr666_t>;
        }
      }
      else
      {
        if (depth == rgb565_2Byte) {
          pc.fp_copy = pixelcopy_t::copy_rgb_antialias<swap565_t>;
        } else {
          pc.fp_copy = pixelcopy_t::copy_rgb_antialias<rgb332_t>;
        }
      }
      return pc;
    }

//----------------------------------------------------------------------------

    static void make_rotation_matrix(float* result, float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y);

    void writeRawColor( uint32_t color, int32_t length) { if (0 >= length) return; setRawColor(color); pushBlock_impl(length); }
    void read_rect(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param);
    void draw_gradient_line( int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colorstart, uint32_t colorend );
    void fill_arc_helper(int32_t cx, int32_t cy, int32_t oradius_x, int32_t iradius_x, int32_t oradius_y, int32_t iradius_y, float start, float end);
    void draw_bezier_helper(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    void draw_bitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, uint32_t fg_rawcolor, uint32_t bg_rawcolor = ~0u);
    void draw_xbitmap(int32_t x, int32_t y, const uint8_t *bitmap, int32_t w, int32_t h, uint32_t fg_rawcolor, uint32_t bg_rawcolor = ~0u);
    void push_image_rotate_zoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, pixelcopy_t* pc);
    void push_image_rotate_zoom_aa(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, int32_t w, int32_t h, pixelcopy_t* pc);
    void push_image_affine(const float* matrix, int32_t w, int32_t h, pixelcopy_t *pc);
    void push_image_affine(const float* matrix, pixelcopy_t *pc);
    void push_image_affine_aa(const float* matrix, int32_t w, int32_t h, pixelcopy_t *pc);
    void push_image_affine_aa(const float* matrix, pixelcopy_t *pre_pc, pixelcopy_t *post_pc);

    uint16_t decodeUTF8(uint8_t c);

    size_t printNumber(unsigned long n, uint8_t base);
    size_t printFloat(double number, uint8_t digits);
    size_t draw_string(const char *string, int32_t x, int32_t y, textdatum_t datum);

    bool draw_bmp(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float scale_x, float scale_y, datum_t datum);
    bool draw_jpg(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float scale_x, float scale_y, datum_t datum);
    bool draw_png(DataWrapper* data, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float scale_x, float scale_y, datum_t datum);


    virtual void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) = 0;
    virtual void writeFillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) = 0;
    virtual void drawPixel_impl(int32_t x, int32_t y) = 0;
    virtual void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) = 0;
    virtual void readRect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) = 0;
    virtual void pushImage_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param, bool use_dma) = 0;
    virtual void pushImageARGB_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param) = 0;
    virtual void writePixels_impl(int32_t length, pixelcopy_t* param) = 0;
    virtual void writePixelsDMA_impl(const void* data, int32_t length) = 0;
    virtual void pushBlock_impl(int32_t len) = 0;
    virtual bool isReadable_impl(void) const = 0;
    virtual int_fast8_t getRotation_impl(void) const = 0;
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
      if (me->_transaction_count)
      {
        auto ad = me->_auto_display;
        me->_auto_display = false;
        me->endTransaction();
        me->_auto_display = ad;
      }
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
}

using LovyanGFX = lgfx::LovyanGFX;

#endif
