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

#if defined (ARDUINO)
 #include <Print.h>
#endif

#include <cstdint>

#include "platforms/common.hpp"
#include "misc/enum.hpp"
#include "misc/colortype.hpp"
#include "misc/pixelcopy.hpp"
#include "misc/DataWrapper.hpp"
#include "lgfx_fonts.hpp"
#include "Touch.hpp"
#include "panel/Panel_Device.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class LGFXBase
#if defined (ARDUINO)
  : public Print
#endif
  {
  public:
    LGFXBase(void);
    virtual ~LGFXBase(void) = default;

#define LGFX_INLINE                        __attribute__ ((always_inline)) inline
#define LGFX_INLINE_T template<typename T> __attribute__ ((always_inline)) inline

    LGFX_INLINE static constexpr std::uint8_t  color332(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color332(r, g, b); }
    LGFX_INLINE static constexpr std::uint16_t color565(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color565(r, g, b); }
    LGFX_INLINE static constexpr std::uint32_t color888(std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::color888(r, g, b); }
    LGFX_INLINE static constexpr std::uint16_t swap565( std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::swap565( r, g, b); }
    LGFX_INLINE static constexpr std::uint32_t swap888( std::uint8_t r, std::uint8_t g, std::uint8_t b) { return lgfx::swap888( r, g, b); }
    LGFX_INLINE static constexpr std::uint8_t  color16to8(std::uint32_t rgb565) { return lgfx::convert_rgb565_to_rgb332(rgb565); }
    LGFX_INLINE static constexpr std::uint16_t color8to16(std::uint32_t rgb332) { return lgfx::convert_rgb332_to_rgb565(rgb332); }
    LGFX_INLINE static constexpr std::uint32_t color16to24(std::uint32_t rgb565) { return lgfx::convert_rgb565_to_rgb888(rgb565); }
    LGFX_INLINE static constexpr std::uint16_t color24to16(std::uint32_t rgb888) { return lgfx::convert_rgb888_to_rgb565(rgb888); }

    LGFX_INLINE   void setColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) { setColor(lgfx::color888(r,g,b)); }
    LGFX_INLINE_T void setColor(T color) { setRawColor(_write_conv.convert(color)); }
    LGFX_INLINE   void setRawColor(std::uint32_t c) { *((std::uint32_t*)&_color) = c; }
    LGFX_INLINE   std::uint32_t getRawColor(void) const { return *((std::uint32_t*)&_color); }
    LGFX_INLINE_T void setBaseColor(T c) { _base_rgb888 = convert_to_rgb888(c); }
    LGFX_INLINE   std::uint32_t getBaseColor(void) const { return _base_rgb888; }
    LGFX_INLINE   color_conv_t* getColorConverter(void) { return &_write_conv; }
    LGFX_INLINE   color_depth_t getColorDepth(void) const { return _write_conv.depth; }

    LGFX_INLINE   void startWrite(bool transaction = true) { _panel->startWrite(transaction); }
    LGFX_INLINE   void endWrite(void)                      { _panel->endWrite(); }
    LGFX_INLINE   void beginTransaction(void)              { _panel->beginTransaction(); }
    LGFX_INLINE   void endTransaction(void)                { _panel->endTransaction(); }
    LGFX_INLINE   std::uint32_t getStartCount(void) const  { return _panel->getStartCount(); }

    LGFX_INLINE   void setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) { _panel->setWindow(xs, ys, xe, ye); }
    LGFX_INLINE   void writePixel(std::int32_t x, std::int32_t y)  { if (x >= _clip_l && x <= _clip_r && y >= _clip_t && y <= _clip_b) writeFillRectPreclipped(x, y, 1, 1); }
    LGFX_INLINE_T void writePixel      ( std::int32_t x, std::int32_t y                                , const T& color) { setColor(color); writePixel    (x, y      ); }
    LGFX_INLINE_T void writeFastVLine  ( std::int32_t x, std::int32_t y                , std::int32_t h, const T& color) { setColor(color); writeFastVLine(x, y   , h); }
                  void writeFastVLine  ( std::int32_t x, std::int32_t y                , std::int32_t h);
    LGFX_INLINE_T void writeFastHLine  ( std::int32_t x, std::int32_t y, std::int32_t w                , const T& color) { setColor(color); writeFastHLine(x, y, w   ); }
                  void writeFastHLine  ( std::int32_t x, std::int32_t y, std::int32_t w);
    LGFX_INLINE_T void writeFillRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T& color) { setColor(color); writeFillRect (x, y, w, h); }
                  void writeFillRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    LGFX_INLINE_T void writeFillRectPreclipped( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T& color) { setColor(color); writeFillRectPreclipped(x, y, w, h); }
    LGFX_INLINE   void writeFillRectPreclipped( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)                 { _panel->writeFillRectPreclipped(x, y, w, h, getRawColor()); }
    LGFX_INLINE_T void writeColor      ( const T& color, std::uint32_t length) { if (0 == length) return; setColor(color);               _panel->writeBlock(getRawColor(), length); }
    LGFX_INLINE_T void pushBlock       ( const T& color, std::uint32_t length) { if (0 == length) return; setColor(color); startWrite(); _panel->writeBlock(getRawColor(), length); endWrite(); }
    LGFX_INLINE   void drawPixel       ( std::int32_t x, std::int32_t y) { if (x >= _clip_l && x <= _clip_r && y >= _clip_t && y <= _clip_b) { _panel->drawPixelPreclipped(x, y, getRawColor()); } }
    LGFX_INLINE_T void drawPixel       ( std::int32_t x, std::int32_t y                                                , const T& color) { setColor(color); drawPixel    (x, y         ); }
    LGFX_INLINE_T void drawFastVLine   ( std::int32_t x, std::int32_t y                , std::int32_t h                , const T& color) { setColor(color); drawFastVLine(x, y   , h   ); }
                  void drawFastVLine   ( std::int32_t x, std::int32_t y                , std::int32_t h);
    LGFX_INLINE_T void drawFastHLine   ( std::int32_t x, std::int32_t y, std::int32_t w                                , const T& color) { setColor(color); drawFastHLine(x, y, w      ); }
                  void drawFastHLine   ( std::int32_t x, std::int32_t y, std::int32_t w);
    LGFX_INLINE_T void fillRect        ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h                , const T& color) { setColor(color); fillRect     (x, y, w, h   ); }
                  void fillRect        ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    LGFX_INLINE_T void drawRect        ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h                , const T& color) { setColor(color); drawRect     (x, y, w, h   ); }
                  void drawRect        ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    LGFX_INLINE_T void drawRoundRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r, const T& color) { setColor(color); drawRoundRect(x, y, w, h, r); }
                  void drawRoundRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r);
    LGFX_INLINE_T void fillRoundRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r, const T& color) { setColor(color); fillRoundRect(x, y, w, h, r); }
                  void fillRoundRect   ( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::int32_t r);
    LGFX_INLINE_T void drawCircle      ( std::int32_t x, std::int32_t y                                , std::int32_t r, const T& color) { setColor(color); drawCircle   (x, y      , r); }
                  void drawCircle      ( std::int32_t x, std::int32_t y                                , std::int32_t r);
    LGFX_INLINE_T void fillCircle      ( std::int32_t x, std::int32_t y                                , std::int32_t r, const T& color) { setColor(color); fillCircle   (x, y      , r); }
                  void fillCircle      ( std::int32_t x, std::int32_t y                                , std::int32_t r);
    LGFX_INLINE_T void drawEllipse     ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry              , const T& color) { setColor(color); drawEllipse  (x, y, rx, ry ); }
                  void drawEllipse     ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry);
    LGFX_INLINE_T void fillEllipse     ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry              , const T& color) { setColor(color); fillEllipse  (x, y, rx, ry ); }
                  void fillEllipse     ( std::int32_t x, std::int32_t y, std::int32_t rx, std::int32_t ry);
    LGFX_INLINE_T void drawLine        ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1            , const T& color) { setColor(color); drawLine     (x0,y0,x1, y1 ); }
                  void drawLine        ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1);
    LGFX_INLINE_T void drawTriangle    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawTriangle(x0, y0, x1, y1, x2, y2); }
                  void drawTriangle    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    LGFX_INLINE_T void fillTriangle    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); fillTriangle(x0, y0, x1, y1, x2, y2); }
                  void fillTriangle    ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    LGFX_INLINE_T void drawBezier      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawBezier(x0, y0, x1, y1, x2, y2); }
                  void drawBezier      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    LGFX_INLINE_T void drawBezier      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, std::int32_t x3, std::int32_t y3, const T& color)  { setColor(color); drawBezier(x0, y0, x1, y1, x2, y2, x3, y3); }
                  void drawBezier      ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, std::int32_t x3, std::int32_t y3);
    LGFX_INLINE_T void drawBezierHelper( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, const T& color)  { setColor(color); drawBezierHelper(x0, y0, x1, y1, x2, y2); }
                  void drawBezierHelper( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    LGFX_INLINE_T void drawEllipseArc  ( std::int32_t x, std::int32_t y, std::int32_t r0x, std::int32_t r1x, std::int32_t r0y, std::int32_t r1y, float angle0, float angle1, const T& color) { setColor(color); drawEllipseArc( x, y, r0x, r1x, r0y, r1y, angle0, angle1); }
                  void drawEllipseArc  ( std::int32_t x, std::int32_t y, std::int32_t r0x, std::int32_t r1x, std::int32_t r0y, std::int32_t r1y, float angle0, float angle1);
    LGFX_INLINE_T void fillEllipseArc  ( std::int32_t x, std::int32_t y, std::int32_t r0x, std::int32_t r1x, std::int32_t r0y, std::int32_t r1y, float angle0, float angle1, const T& color) { setColor(color); fillEllipseArc( x, y, r0x, r1x, r0y, r1y, angle0, angle1); }
                  void fillEllipseArc  ( std::int32_t x, std::int32_t y, std::int32_t r0x, std::int32_t r1x, std::int32_t r0y, std::int32_t r1y, float angle0, float angle1);
    LGFX_INLINE_T void drawArc         ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1, const T& color) { setColor(color); drawEllipseArc( x, y, r0, r1, r0, r1, angle0, angle1); }
                  void drawArc         ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1)                 {                  drawEllipseArc( x, y, r0, r1, r0, r1, angle0, angle1); }
    LGFX_INLINE_T void fillArc         ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1, const T& color) { setColor(color); fillEllipseArc( x, y, r0, r1, r0, r1, angle0, angle1); }
                  void fillArc         ( std::int32_t x, std::int32_t y, std::int32_t r0, std::int32_t r1, float angle0, float angle1)                 {                  fillEllipseArc( x, y, r0, r1, r0, r1, angle0, angle1); }
    LGFX_INLINE_T void drawCircleHelper( std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t cornername                 , const T& color)  { setColor(color); drawCircleHelper(x, y, r, cornername    ); }
                  void drawCircleHelper( std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t cornername);
    LGFX_INLINE_T void fillCircleHelper( std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t corners, std::int32_t delta, const T& color)  { setColor(color); fillCircleHelper(x, y, r, corners, delta); }
                  void fillCircleHelper( std::int32_t x, std::int32_t y, std::int32_t r, std::uint_fast8_t corners, std::int32_t delta);
    LGFX_INLINE_T void floodFill( std::int32_t x, std::int32_t y, const T& color) { setColor(color); floodFill(x, y); }
                  void floodFill( std::int32_t x, std::int32_t y                );
    LGFX_INLINE_T void paint    ( std::int32_t x, std::int32_t y, const T& color) { setColor(color); floodFill(x, y); }
    LGFX_INLINE   void paint    ( std::int32_t x, std::int32_t y                ) {                  floodFill(x, y); }

    LGFX_INLINE_T void fillAffine(const float matrix[6], std::int32_t w, std::int32_t h, const T& color) { setColor(color); fillAffine(matrix, w, h); }
                  void fillAffine(const float matrix[6], std::int32_t w, std::int32_t h);

    LGFX_INLINE_T void drawGradientHLine( std::int32_t x, std::int32_t y, std::int32_t w, const T& colorstart, const T& colorend ) { drawGradientLine( x, y, x + w - 1, y, colorstart, colorend ); }
    LGFX_INLINE_T void drawGradientVLine( std::int32_t x, std::int32_t y, std::int32_t h, const T& colorstart, const T& colorend ) { drawGradientLine( x, y, x, y + h - 1, colorstart, colorend ); }
    LGFX_INLINE_T void drawGradientLine ( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, const T& colorstart, const T& colorend ) { draw_gradient_line( x0, y0, x1, y1, convert_to_rgb888(colorstart), convert_to_rgb888(colorend) ); }

    LGFX_INLINE_T void fillScreen  ( const T& color) { setColor(color); fillRect(0, 0, width(), height()); }
    LGFX_INLINE   void fillScreen  ( void )          {                  fillRect(0, 0, width(), height()); }

    LGFX_INLINE_T void clear       ( const T& color) { setBaseColor(color); clear(); }
    LGFX_INLINE   void clear       ( void )          { setColor(_base_rgb888); fillScreen(); }
    LGFX_INLINE_T void clearDisplay( const T& color) { setBaseColor(color); clear(); }
    LGFX_INLINE   void clearDisplay( void )          { setColor(_base_rgb888); fillScreen(); }

    LGFX_INLINE   void  setPivot(float x, float y) { _xpivot = x; _ypivot = y; }
    LGFX_INLINE   float getPivotX(void) const { return _xpivot; }
    LGFX_INLINE   float getPivotY(void) const { return _ypivot; }

    LGFX_INLINE   std::int32_t width (void) const { return _panel->width(); }
    LGFX_INLINE   std::int32_t height(void) const { return _panel->height(); }
    LGFX_INLINE   bool hasPalette (void) const { return _palette_count; }
    LGFX_INLINE   std::uint32_t getPaletteCount(void) const { return _palette_count; }
    LGFX_INLINE   RGBColor*     getPalette(void) const { return getPalette_impl(); }
    LGFX_INLINE   bool isReadable(void) const { return _panel->isReadable(); }
    LGFX_INLINE   bool isEPD(void) const { return _panel->isEpd(); }
    LGFX_INLINE   bool getSwapBytes(void) const { return _swapBytes; }
    LGFX_INLINE   void setSwapBytes(bool swap) { _swapBytes = swap; }
    LGFX_INLINE   bool isBusShared(void) const { return _panel->isBusShared(); }
    [[deprecated("use isBusShared()")]]
    LGFX_INLINE   bool isSPIShared(void) const { return _panel->isBusShared(); }
                  void display(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    LGFX_INLINE   void display(void) { _panel->display(0, 0, 0, 0); }
    LGFX_INLINE   void waitDisplay(void) { _panel->waitDisplay(); }
    LGFX_INLINE   bool displayBusy(void) { return _panel->displayBusy(); }
    LGFX_INLINE   void setAutoDisplay(bool flg) { _panel->setAutoDisplay(flg); }
    LGFX_INLINE   void initDMA(void) { _panel->initDMA(); }
    LGFX_INLINE   void waitDMA(void) { _panel->waitDMA(); }
    LGFX_INLINE   bool dmaBusy(void) { return _panel->dmaBusy(); }

    LGFX_INLINE_T void setScrollRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T& color) { setBaseColor(color); setScrollRect(x, y, w, h); }

    LGFX_INLINE_T void writePixels(const T *data, std::int32_t len)                        { auto pc = create_pc_fast(data      ); _panel->writePixels(&pc, len); }
    LGFX_INLINE   void writePixels(const std::uint16_t* data, std::int32_t len, bool swap) { auto pc = create_pc_fast(data, swap); _panel->writePixels(&pc, len); }
    LGFX_INLINE   void writePixels(const void*          data, std::int32_t len, bool swap) { auto pc = create_pc_fast(data, swap); _panel->writePixels(&pc, len); }

    LGFX_INLINE_T void pushPixels(T*                   data, std::int32_t len           ) { startWrite(); writePixels(data, len      ); endWrite(); }
    LGFX_INLINE   void pushPixels(const std::uint16_t* data, std::int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }
    LGFX_INLINE   void pushPixels(const void*          data, std::int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }

    template<typename TFunc>
    void effect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, TFunc&& effector)
    {
      if (!_clipping(x, y, w, h)) return;
      auto ye = y + h;
      RGBColor buf[w];
      startWrite();
      do
      {
        readRectRGB(x, y, w, 1, buf);
        std::size_t i = 0;
        do
        {
          effector(x + i, y, buf[i]);
        } while (++i < w);
        pushImage(x, y, w, 1, buf);
      } while (++y < ye);
      endWrite();
    }

    template<typename T>
    void fillRectAlpha(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint8_t alpha, const T& color)
    {
      effect(x, y, w, h, effect_fill_alpha ( argb8888_t { convert_to_rgb888(color) | alpha << 24 } ) );
    }

    LGFX_INLINE_T void drawBitmap (std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& color                    ) { draw_bitmap (x, y, bitmap, w, h, _write_conv.convert(color)); }
    LGFX_INLINE_T void drawBitmap (std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& fgcolor, const T& bgcolor) { draw_bitmap (x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }
    LGFX_INLINE_T void drawXBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& color                    ) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(color)); }
    LGFX_INLINE_T void drawXBitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, const T& fgcolor, const T& bgcolor) { draw_xbitmap(x, y, bitmap, w, h, _write_conv.convert(fgcolor), _write_conv.convert(bgcolor)); }

    LGFX_INLINE_T
    void writeIndexedPixels(const std::uint8_t *data, T* palette, std::int32_t len, std::uint8_t depth = 8)
    {
      auto src_depth = (color_depth_t)(depth | color_depth_t::has_palette);
      auto pc = create_pc_fast(data, palette, src_depth);
      _panel->writePixels(&pc, len);
    }

#undef LGFX_INLINE
#undef LGFX_INLINE_T

    std::uint8_t getRotation(void) const { return _panel->getRotation(); }
    void setRotation(std::uint_fast8_t rotation);
    void setColorDepth(int bits) { setColorDepth((color_depth_t)(bits & color_depth_t::bit_mask));}
    void setColorDepth(color_depth_t depth);

    void setAddrWindow(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);

    void setClipRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    void getClipRect(std::int32_t *x, std::int32_t *y, std::int32_t *w, std::int32_t *h);
    void clearClipRect(void);

    void setScrollRect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
    void getScrollRect(std::int32_t *x, std::int32_t *y, std::int32_t *w, std::int32_t *h);
    void clearScrollRect(void);

    template<typename T>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data)
    {
      auto pc = create_pc(data);
      pushImage(x, y, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr(data, transparent);
      pushImage(x, y, w, h, &pc);
    }

    template<typename T>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth);
      pushImage(x, y, w, h, &pc);
    }

    template<typename T>
    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, std::uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth, transparent);
      pushImage(x, y, w, h, &pc);
    }

    template<typename T>
    void pushImageDMA(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data)
    {
      auto pc = create_pc(data);
      pushImage(x, y, w, h, &pc, true);
    }

    template<typename T>
    void pushImageDMA(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth);
      pushImage(x, y, w, h, &pc, true);
    }

    void pushImage(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t *param, bool use_dma = false);


//----------------------------------------------------------------------------

    template<typename T>
    void pushImageRotateZoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, const T* data)
    {
      auto pc = create_pc(data);
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImageRotateZoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr(data, transparent);
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T>
    void pushImageRotateZoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth);
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T>
    void pushImageRotateZoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, const void* data, std::uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth, transparent);
      push_image_rotate_zoom(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }


    template<typename T>
    void pushImageRotateZoomWithAA(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, const T* data)
    {
      auto pc = create_pc_antialias(data);
      push_image_rotate_zoom_aa(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImageRotateZoomWithAA(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr_antialias(data, transparent);
      push_image_rotate_zoom_aa(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T>
    void pushImageRotateZoomWithAA(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_antialias(data, palette, depth);
      push_image_rotate_zoom_aa(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

    template<typename T>
    void pushImageRotateZoomWithAA(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, const void* data, std::uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_antialias(data, palette, depth, transparent);
      push_image_rotate_zoom_aa(dst_x, dst_y, src_x, src_y, angle, zoom_x, zoom_y, w, h, &pc);
    }

//----------------------------------------------------------------------------

    template<typename T>
    void pushImageAffine(const float matrix[6], std::int32_t w, std::int32_t h, const T* data)
    {
      auto pc = create_pc(data);
      push_image_affine(matrix, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImageAffine(const float matrix[6], std::int32_t w, std::int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr(data, transparent);
      push_image_affine(matrix, w, h, &pc);
    }

    template<typename T>
    void pushImageAffine(const float matrix[6], std::int32_t w, std::int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth);
      push_image_affine(matrix, w, h, &pc);
    }

    template<typename T>
    void pushImageAffine(const float matrix[6], std::int32_t w, std::int32_t h, const void* data, std::uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_palette(data, palette, depth, transparent);
      push_image_affine(matrix, w, h, &pc);
    }


    template<typename T>
    void pushImageAffineWithAA(const float matrix[6], std::int32_t w, std::int32_t h, const T* data)
    {
      auto pc = create_pc_antialias(data);
      push_image_affine_aa(matrix, w, h, &pc);
    }

    template<typename T1, typename T2>
    void pushImageAffineWithAA(const float matrix[6], std::int32_t w, std::int32_t h, const T1* data, const T2& transparent)
    {
      auto pc = create_pc_tr_antialias(data, transparent);
      push_image_affine_aa(matrix, w, h, &pc);
    }

    template<typename T>
    void pushImageAffineWithAA(const float matrix[6], std::int32_t w, std::int32_t h, const void* data, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_antialias(data, palette, depth);
      push_image_affine_aa(matrix, w, h, &pc);
    }

    template<typename T>
    void pushImageAffineWithAA(const float matrix[6], std::int32_t w, std::int32_t h, const void* data, std::uint32_t transparent, color_depth_t depth, const T* palette)
    {
      auto pc = create_pc_antialias(data, palette, depth, transparent);
      push_image_affine_aa(matrix, w, h, &pc);
    }

//----------------------------------------------------------------------------

    /// read RGB565 16bit color
    std::uint16_t readPixel(std::int32_t x, std::int32_t y)
    {
      if (x < 0 || x >= width() || y < 0 || y >= height()) return 0;

      pixelcopy_t p(nullptr, swap565_t::depth, _read_conv.depth, false, getPalette());
      std::uint16_t data = 0;

      _panel->readRect(x, y, 1, 1, &data, &p);

      return __builtin_bswap16(data);
    }

    /// read RGB888 24bit color
    RGBColor readPixelRGB(std::int32_t x, std::int32_t y)
    {
      RGBColor data[1];
      if (x < 0 || x >= width() || y < 0 || y >= height()) return data[0];

      pixelcopy_t p(nullptr, bgr888_t::depth, _read_conv.depth, false, getPalette());

      _panel->readRect(x, y, 1, 1, data, &p);

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
      if (std::is_same<rgb565_t, T>::value || std::is_same<rgb888_t, T>::value || std::is_same<argb8888_t, T>::value || std::is_same<grayscale_t, T>::value || p.fp_copy == nullptr)
      {
        p.no_convert = false;
        p.fp_copy = pixelcopy_t::get_fp_copy_rgb_affine_dst<T>(_read_conv.depth);
      }
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
    //[[deprecated("use textdatum_t")]]
    void setTextDatum(std::uint8_t datum) { _text_style.datum = (textdatum_t)datum; }
    void setTextDatum(textdatum_t datum) { _text_style.datum = datum; }
    textdatum_t getTextDatum(void) const { return _text_style.datum; }
    void setTextPadding(std::uint32_t padding_x) { _padding_x = padding_x; }
    std::uint32_t getTextPadding(void) const { return _padding_x; }
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
    std::int32_t fontHeight(std::uint8_t font) const { return ((const BaseFont*)fontdata[font])->height * _text_style.size_y; }
    std::int32_t fontHeight(const IFont* font) const;
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
    std::uint8_t getTextFont(void) const
    {
      std::size_t i = 0;
      do {
        if (fontdata[i] == _font) return i;
      } while (fontdata[++i]);
      return 0;
    }

    /// [[deprecated("use setFont(&fonts::Font)")]]
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


    bool drawBmp(const std::uint8_t *bmp_data, std::uint32_t bmp_len, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      PointerWrapper data;
      data.set(bmp_data, bmp_len);
      return this->draw_bmp(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    bool drawJpg(const std::uint8_t *jpg_data, std::uint32_t jpg_len, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      PointerWrapper data;
      data.set(jpg_data, jpg_len);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]] bool drawJpg(const std::uint8_t *jpg_data, std::uint32_t jpg_len, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpg(jpg_data, jpg_len, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }
    bool drawPng(const std::uint8_t *png_data, std::uint32_t png_len, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      PointerWrapper data;
      data.set(png_data, png_len);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    inline bool drawBmp(DataWrapper *data, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return this->draw_bmp(data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawJpg(DataWrapper *data, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return this->draw_jpg(data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]] inline bool drawJpg(DataWrapper *data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpg(data, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }
    inline bool drawPng(DataWrapper *data, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return this->draw_png(data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    void* createPng( std::size_t* datalen, std::int32_t x = 0, std::int32_t y = 0, std::int32_t width = 0, std::int32_t height = 0);



    template<typename T>
    [[deprecated("use pushImage")]] void pushRect( std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const T* data) { pushImage(x, y, w, h, data); }

    template<typename T>
    [[deprecated("use pushBlock")]] void pushColor(const T& color, std::uint32_t length) { if (0 != length) { setColor(color); startWrite(); _panel->writeBlock(getRawColor(), length); endWrite(); } }
    template<typename T>
    [[deprecated("use pushBlock")]] void pushColor(const T& color                     ) {                     setColor(color); startWrite(); _panel->writeBlock(getRawColor(), 1);      endWrite(); }

    template<typename T>
    [[deprecated("use pushPixels")]] void pushColors(T*                   data, std::int32_t len           ) { startWrite(); writePixels(data, len            ); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const void*          data, std::int32_t len           ) { startWrite(); writePixels(data, len, _swapBytes); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const std::uint16_t* data, std::int32_t len           ) { startWrite(); writePixels(data, len, _swapBytes); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const std::uint8_t*  data, std::int32_t len           ) { startWrite(); writePixels((const rgb332_t*)data, len); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const void*          data, std::int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }
    [[deprecated("use pushPixels")]] void pushColors(const std::uint16_t* data, std::int32_t len, bool swap) { startWrite(); writePixels(data, len, swap); endWrite(); }

//----------------------------------------------------------------------------

  protected:

    virtual RGBColor* getPalette_impl(void) const { return nullptr; }
    void prepareTmpTransaction(DataWrapper* data);

    IPanel* _panel = nullptr;

    std::int32_t _sx = 0, _sy = 0, _sw = 0, _sh = 0; // for scroll zone
    std::int32_t _clip_l = 0, _clip_r = -1, _clip_t = 0, _clip_b = -1; // clip rect

    std::uint32_t _base_rgb888 = 0;  // gap fill colour for clear and scroll zone 
    raw_color_t _color = 0xFFFFFFU;

    color_conv_t _write_conv;
    color_conv_t _read_conv;

    std::uint16_t _palette_count = 0;

    float _xpivot = 0.0f;   // x pivot point coordinate
    float _ypivot = 0.0f;   // x pivot point coordinate

    bool _swapBytes = false;

    enum utf8_decode_state_t
    { utf8_state0 = 0
    , utf8_state1 = 1
    , utf8_state2 = 2
    };
    utf8_decode_state_t _decoderState = utf8_state0;   // UTF8 decoder state
    std::uint_fast16_t _unicode_buffer = 0;   // Unicode code-point buffer

    std::int32_t _cursor_x = 0;  // print text cursor
    std::int32_t _cursor_y = 0;
    std::int32_t _filled_x = 0;  // print filled position
    std::int32_t _padding_x = 0;

    TextStyle _text_style;
    FontMetrics _font_metrics = { 6, 6, 0, 8, 8, 0, 7 }; // Font0 default metric
    const IFont* _font = &fonts::Font0;

    std::shared_ptr<RunTimeFont> _runtime_font;  // run-time generated font
    DataWrapper* _font_file = nullptr;
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

    bool _clipping(std::int32_t& x, std::int32_t& y, std::int32_t& w, std::int32_t& h)
    {
      auto cl = _clip_l;
      if (x < cl) { w += x - cl; x = cl; }
      auto cr = _clip_r + 1 - x;
      if (w > cr) w = cr;
      if (w < 1) return false;

      auto ct = _clip_t;
      if (y < ct) { h += y - ct; y = ct; }
      auto cb = _clip_b + 1 - y;
      if (h > cb) h = cb;
      if (h < 1) return false;

      return true;
    }

//----------------------------------------------------------------------------

    template<typename T>
    pixelcopy_t create_pc_fast(const T *data)
    {
      auto dst_depth = _write_conv.depth;
      pixelcopy_t pc(data, dst_depth, get_depth<T>::value, hasPalette());
      if (hasPalette() || pc.dst_bits < 8)
      {
        pc.fp_copy = pixelcopy_t::copy_bit_fast;
      }
      else
      if (pc.dst_bits > 16)
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
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const std::uint8_t  *data) { return create_pc_fast(reinterpret_cast<const rgb332_t*>(data)); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const std::uint16_t *data) { return create_pc_fast(data, _swapBytes); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const void          *data) { return create_pc_fast(data, _swapBytes); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const std::uint16_t *data, bool swap)
    {
      return swap && !hasPalette() && _write_conv.bits >= 8
           ? create_pc_fast(reinterpret_cast<const rgb565_t* >(data))
           : create_pc_fast(reinterpret_cast<const swap565_t*>(data));
    }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_fast(const void *data, bool swap)
    {
      return swap && !hasPalette() && _write_conv.bits >= 8
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
      //pc.src_bits  = src_depth > 8 ? (src_depth + 7) & ~7 : src_depth;
      //pc.dst_bits  = dst_depth > 8 ? (dst_depth + 7) & ~7 : dst_depth;
      pc.src_depth = src_depth;
      pc.dst_depth = dst_depth;
      pc.src_mask  = (1 << pc.src_bits) - 1 ;
      pc.dst_mask  = (1 << pc.dst_bits) - 1 ;
      pc.no_convert= src_depth == dst_depth;
//*/
      if (hasPalette() || pc.dst_bits < 8)
      {
        if (palette && (pc.src_bits == 8) && (pc.dst_bits == 8))
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
        if (pc.dst_bits > 16)
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

    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const std::uint8_t  *data) { return create_pc(reinterpret_cast<const rgb332_t*>(data)); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const std::uint16_t *data) { return create_pc(data, _swapBytes); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const void          *data) { return create_pc(data, _swapBytes); }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const std::uint16_t *data, bool swap)
    {
      return swap && !hasPalette() && _write_conv.bits >= 8
           ? create_pc(reinterpret_cast<const rgb565_t* >(data))
           : create_pc(reinterpret_cast<const swap565_t*>(data));
    }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc(const void *data, bool swap)
    {
      return swap && !hasPalette() && _write_conv.bits >= 8
           ? create_pc(reinterpret_cast<const rgb888_t*>(data))
           : create_pc(reinterpret_cast<const bgr888_t*>(data));
    }

    template<typename T>
    pixelcopy_t create_pc_rawtr(const T *data, std::uint32_t raw_transparent)
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

    template<typename T> pixelcopy_t create_pc_tr(const std::uint8_t  *data, const T& transparent) { return create_pc_tr(reinterpret_cast<const rgb332_t*>(data), transparent); }
    template<typename T> pixelcopy_t create_pc_tr(const std::uint16_t *data, const T& transparent) { return create_pc_tr(data, transparent, _swapBytes); }
    template<typename T> pixelcopy_t create_pc_tr(const void          *data, const T& transparent) { return create_pc_tr(data, transparent, _swapBytes); }
    template<typename T> pixelcopy_t create_pc_tr(const std::uint16_t *data, const T& transparent, bool swap)
    {
      return swap && _write_conv.bits >= 8 && !hasPalette()
           ? create_pc_tr(reinterpret_cast<const rgb565_t* >(data), transparent)
           : create_pc_tr(reinterpret_cast<const swap565_t*>(data), transparent);
    }
    template<typename T> pixelcopy_t create_pc_tr(const void *data, const T& transparent, bool swap)
    {
      return swap && _write_conv.bits >= 8 && !hasPalette()
           ? create_pc_tr(reinterpret_cast<const rgb888_t*>(data), transparent)
           : create_pc_tr(reinterpret_cast<const bgr888_t*>(data), transparent);
    }

    pixelcopy_t create_pc_palette(const void *data, const bgr888_t *palette, lgfx::color_depth_t depth, std::uint32_t transparent = pixelcopy_t::NON_TRANSP)
    {
      return pixelcopy_t (data, _write_conv.depth, depth, hasPalette(), palette, transparent);
    }

    template<typename T>
    pixelcopy_t create_pc_palette(const void *data, const T *palette, lgfx::color_depth_t depth, std::uint32_t transparent = pixelcopy_t::NON_TRANSP)
    {
      pixelcopy_t pc(data, getColorDepth(), depth, hasPalette(), palette, transparent);
      if (!hasPalette() && palette && _write_conv.bits >= 8)
      {
        pc.fp_copy = pixelcopy_t::get_fp_copy_palette_affine<T>(getColorDepth());
      }
      return pc;
    }


    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_antialias(const std::uint8_t *data, std::uint32_t raw_transparent = pixelcopy_t::NON_TRANSP)
    {
      return create_pc_antialias(reinterpret_cast<const rgb332_t*>(data), raw_transparent);
    }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_antialias(const std::uint16_t *data, std::uint32_t raw_transparent = pixelcopy_t::NON_TRANSP)
    {
      return _swapBytes
           ? create_pc_antialias(reinterpret_cast<const rgb565_t* >(data), raw_transparent)
           : create_pc_antialias(reinterpret_cast<const swap565_t*>(data), raw_transparent);
    }
    __attribute__ ((always_inline)) inline pixelcopy_t create_pc_antialias(const void *data, std::uint32_t raw_transparent = pixelcopy_t::NON_TRANSP)
    {
      return _swapBytes
           ? create_pc_antialias(reinterpret_cast<const rgb888_t*>(data), raw_transparent)
           : create_pc_antialias(reinterpret_cast<const bgr888_t*>(data), raw_transparent);
    }

    template<typename T>
    pixelcopy_t create_pc_antialias(const T* data, std::uint32_t raw_transparent = pixelcopy_t::NON_TRANSP)
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
    static pixelcopy_t create_pc_antialias(const void* data, const T* palette, lgfx::color_depth_t depth, std::uint32_t transparent = pixelcopy_t::NON_TRANSP)
    {
      pixelcopy_t pc(data, argb8888_t::depth, depth, false, palette, transparent);
      if (palette)
      {
        pc.fp_copy = pixelcopy_t::copy_palette_antialias<T>;
      }
      else if ((depth & color_depth_t::bit_mask) > 16)
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

    void read_rect(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param);
    void draw_gradient_line( std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, uint32_t colorstart, uint32_t colorend );
    void fill_arc_helper(std::int32_t cx, std::int32_t cy, std::int32_t oradius_x, std::int32_t iradius_x, std::int32_t oradius_y, std::int32_t iradius_y, float start, float end);
    void draw_bezier_helper(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2);
    void draw_bitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor = ~0u);
    void draw_xbitmap(std::int32_t x, std::int32_t y, const std::uint8_t *bitmap, std::int32_t w, std::int32_t h, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor = ~0u);
    void push_image_rotate_zoom(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, pixelcopy_t* pc);
    void push_image_rotate_zoom_aa(float dst_x, float dst_y, float src_x, float src_y, float angle, float zoom_x, float zoom_y, std::int32_t w, std::int32_t h, pixelcopy_t* pc);
    void push_image_affine(const float* matrix, std::int32_t w, std::int32_t h, pixelcopy_t *pc);
    void push_image_affine(const float* matrix, pixelcopy_t *pc);
    void push_image_affine_aa(const float* matrix, std::int32_t w, std::int32_t h, pixelcopy_t *pc);
    void push_image_affine_aa(const float* matrix, pixelcopy_t *pre_pc, pixelcopy_t *post_pc);

    std::uint16_t decodeUTF8(std::uint8_t c);

    std::size_t printNumber(unsigned long n, std::uint8_t base);
    std::size_t printFloat(double number, std::uint8_t digits);
    std::size_t draw_string(const char *string, std::int32_t x, std::int32_t y, textdatum_t datum);
    bool load_font(lgfx::DataWrapper* data);

    bool draw_bmp(DataWrapper* data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, float scale_x, float scale_y, datum_t datum);
    bool draw_jpg(DataWrapper* data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, float scale_x, float scale_y, datum_t datum);
    bool draw_png(DataWrapper* data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, float scale_x, float scale_y, datum_t datum);

    static void tmpBeginTransaction(LGFXBase* lgfx)
    {
      if (lgfx->getStartCount()) { lgfx->beginTransaction(); }
    }

    static void tmpEndTransaction(LGFXBase* lgfx)
    {
      if (lgfx->getStartCount())
      {
        lgfx->endTransaction();
      }
    }

  };

//----------------------------------------------------------------------------

  /// LovyanGFXクラス。ファイルシステム等、利用環境側のinclude順に依存する機能はLGFX_FILESYSTEM_Supportから継承する。
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
  };

//----------------------------------------------------------------------------

  class Panel_Device;

  class LGFX_Device : public LovyanGFX
  {
  public:
    LGFX_Device(void);

    bool init(void)               { return init_impl(true , true); };
    bool begin(void)              { return init_impl(true , true); };
    bool init_without_reset(void) { return init_impl(false, false); };
    void initBus(void);
    void releaseBus(void);
    void setPanel(Panel_Device* panel);

    void setEpdMode(epd_mode_t epd_mode) { _panel->setEpdMode(epd_mode); }
    epd_mode_t getEpdMode(void) const { return _panel->getEpdMode(); }
    inline void invertDisplay(bool i) { _panel->setInvert(i); }
    inline bool getInvert(void) const { return _panel->getInvert(); }

    inline void sleep(void) { _panel->setBrightness(0); _panel->setSleep(true); }
    inline void wakeup(void) { _panel->setSleep(false); _panel->setBrightness(_brightness); }
    inline void powerSave(bool flg) { _panel->setPowerSave(flg); }
    inline void powerSaveOn(void) { _panel->setPowerSave(true); }
    inline void powerSaveOff(void) { _panel->setPowerSave(false); }

    inline Panel_Device* panel(void) const { return reinterpret_cast<Panel_Device*>(_panel); }
    inline Panel_Device* getPanel(void) const { return reinterpret_cast<Panel_Device*>(_panel); }
    inline void panel(Panel_Device* panel) { setPanel(panel); }

    inline void writeCommand(  std::uint8_t  cmd) { _panel->writeCommand(                  cmd , 1); } // AdafruitGFX compatible
    inline void writecommand(  std::uint8_t  cmd) { _panel->writeCommand(                  cmd , 1); } // TFT_eSPI compatible
    inline void writeCommand16(std::uint16_t cmd) { _panel->writeCommand(__builtin_bswap16(cmd), 2); }
    inline void spiWrite(   std::uint8_t  data) { _panel->writeData(                  data , 1); } // AdafruitGFX compatible
    inline void writedata(  std::uint8_t  data) { _panel->writeData(                  data , 1); } // TFT_eSPI compatible
    inline void writeData(  std::uint8_t  data) { _panel->writeData(                  data , 1); }
    inline void writeData16(std::uint16_t data) { _panel->writeData(__builtin_bswap16(data), 2); }
    inline void writeData32(std::uint32_t data) { _panel->writeData(__builtin_bswap32(data), 4); }
    inline std::uint8_t  readData8( std::uint8_t index=0) { return                   _panel->readData(index, 1) ; }
    inline std::uint16_t readData16(std::uint8_t index=0) { return __builtin_bswap16(_panel->readData(index, 2)); }
    inline std::uint32_t readData32(std::uint8_t index=0) { return __builtin_bswap32(_panel->readData(index, 4)); }

    inline ILight* light(void) const { return _panel ? panel()->light() : nullptr; }
    inline void setBrightness(std::uint8_t brightness) { _brightness = brightness; if (_panel) { _panel->setBrightness(brightness); } }
    inline std::uint8_t getBrightness(void) const { return _brightness; }

    inline ITouch* touch(void) const { return _panel ? panel()->touch() : nullptr; }
    inline void convertRawXY(std::int32_t *x, std::int32_t *y) { panel()->convertRawXY(x, y); }
    std::uint_fast8_t getTouchRaw(touch_point_t *tp, std::uint_fast8_t number = 0) { return panel()->getTouchRaw(tp, number); }
    std::uint_fast8_t getTouch(touch_point_t *tp, std::uint_fast8_t number = 0) { return panel()->getTouch(tp, number); }
    touch_point_t getTouch(std::int_fast8_t number = 0)
    {
      touch_point_t res;
      getTouch(&res, number);
      return res;
    }

    std::uint_fast8_t getTouchRaw(std::int32_t *x = nullptr, std::int32_t *y = nullptr, std::uint_fast8_t number = 0)
    {
      touch_point_t tp;
      auto res = getTouchRaw(&tp, number);
      if (x) *x = tp.x;
      if (y) *y = tp.y;
      return res;
    }

    template <typename T>
    std::uint_fast8_t getTouch(T *x, T *y, std::uint_fast8_t number = 0)
    {
      touch_point_t tp;
      auto res = getTouch(&tp, number);
      if (x) *x = tp.x;
      if (y) *y = tp.y;
      return res;
    }

    void convertRawXY(std::uint16_t *x, std::uint16_t *y)
    {
      std::int32_t tx = *x, ty = *y;
      convertRawXY(&tx, &ty);
      *x = tx;
      *y = ty;
    }

    // This requires a uint16_t array with 8 elements. ( or nullptr )
    template <typename T>
    void calibrateTouch(uint16_t *parameters, const T& color_fg, const T& color_bg, uint8_t size = 10)
    {
      calibrate_touch(parameters, _write_conv.convert(color_fg), _write_conv.convert(color_bg), size);
    }
/*
    void updateTouchCalibrate(void)
    {
      auto cfg = _touch->config();
      std::uint16_t parameters[8] =
        { cfg.x_min, cfg.y_min
        , cfg.x_min, cfg.y_max
        , cfg.x_max, cfg.y_min
        , cfg.x_max, cfg.y_max };
      setTouchCalibrate(parameters);
    }

    // This requires a uint16_t array with 8 elements.
    void setTouchCalibrate(std::uint16_t *parameters)
    {
      if (_touch == nullptr) return;
      //bool r = getRotation() & 1;
      std::int32_t w = width();
      std::int32_t h = height();
      if (getRotation() & 1) std::swap(w, h);
      _touch->setCalibrate(parameters, w, h);
    }
//*/

  protected:
    std::uint8_t _brightness = 127;

    virtual bool init_impl(bool use_reset, bool use_clear);

    void draw_calibrate_point(std::int32_t x, std::int32_t y, std::int32_t r, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor);

    void calibrate_touch(std::uint16_t *parameters, std::uint32_t fg_rawcolor, std::uint32_t bg_rawcolor, std::uint8_t size);

//----------------------------------------------------------------------------

  };

//----------------------------------------------------------------------------
 }
}

using LovyanGFX = lgfx::LovyanGFX;
using LGFX_Device = lgfx::LGFX_Device;
