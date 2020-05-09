#ifndef LGFX_FONTS_HPP_
#define LGFX_FONTS_HPP_

#include <stdint.h>

namespace lgfx
{
  enum textdatum_t : uint8_t
  //  0:left   1:centre   2:right
  //  0:top    4:middle   8:bottom   16:baseline
  { top_left        =  0  // Top left (default)
  , top_center      =  1  // Top center
  , top_centre      =  1  // Top center
  , top_right       =  2  // Top right
  , middle_left     =  4  // Middle left
  , middle_center   =  5  // Middle center
  , middle_centre   =  5  // Middle center
  , middle_right    =  6  // Middle right
  , bottom_left     =  8  // Bottom left
  , bottom_center   =  9  // Bottom center
  , bottom_centre   =  9  // Bottom center
  , bottom_right    = 10  // Bottom right
  , baseline_left   = 16  // Baseline left (Line the 'A' character would sit on)
  , baseline_center = 17  // Baseline center
  , baseline_centre = 17  // Baseline center
  , baseline_right  = 18  // Baseline right
  };

  struct FontMetrics {
    int16_t width;
    int16_t x_advance;
    int16_t x_offset;
    int16_t height;
    int16_t y_advance;
    int16_t y_offset;
    int16_t baseline;
  };

  struct TextStyle {
    uint32_t fore_rgb888 = 0xFFFFFFU;
    uint32_t back_rgb888 = 0;
    int_fast8_t size_x = 1;
    int_fast8_t size_y = 1;
    textdatum_t datum = textdatum_t::top_left;
    bool utf8 = true;
    bool cp437 = false;
  };

  struct IFont
  {
    enum font_type_t
    { ft_unknown
    , ft_glcd
    , ft_bmp
    , ft_rle
    , ft_gfx
    , ft_bdf
    , ft_vlw
    };

    virtual font_type_t getType(void) const = 0;
    virtual void getDefaultMetric(FontMetrics *metrics) const = 0;
    virtual bool updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const = 0;
    virtual bool unloadFont(void) { return false; }
/*
    struct param {
      int32_t clip_left  ;
      int32_t clip_right ;
      int32_t clip_top   ;
      int32_t clip_bottom;
      int32_t filled_x   ;
      TextStyle* style   ;
    };
//*/
  };

  struct RunTimeFont : public IFont {
    virtual ~RunTimeFont() {}
  };

  struct BaseFont : public IFont {
    const uint8_t *chartbl;
    const uint8_t *widthtbl;
    const uint8_t width;
    const uint8_t height;
    const uint8_t baseline;
    BaseFont() = default;
    constexpr BaseFont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline)
     : chartbl  (chartbl  )
     , widthtbl (widthtbl )
     , width    (width    )
     , height   (height   )
     , baseline (baseline )
    {}
    void getDefaultMetric(FontMetrics *metrics) const override {
      metrics->width    = width;
      metrics->x_advance = width;
      metrics->x_offset  = 0;
      metrics->height    = height;
      metrics->y_advance = height;
      metrics->y_offset  = 0;
      metrics->baseline  = baseline;
    }
  };

  struct GLCDfont : public BaseFont {
    constexpr GLCDfont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    constexpr font_type_t getType(void) const override { return ft_glcd; }

    bool updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const override;
  };

  struct BMPfont : public BaseFont {
    constexpr BMPfont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    constexpr font_type_t getType(void) const override { return ft_bmp;  } 

    bool updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const override;
  };

  struct RLEfont : public BMPfont {
    constexpr RLEfont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline) : BMPfont(chartbl, widthtbl, width, height, baseline ) {}
    constexpr font_type_t getType(void) const override { return ft_rle; }
  };

  struct BDFfont : public BaseFont {
    const uint16_t *indextbl;
    uint16_t indexsize;
    uint8_t halfwidth;
    BDFfont() = default;
    constexpr BDFfont(const uint8_t *chartbl, const uint16_t *indextbl, uint16_t indexsize, uint8_t width, uint8_t halfwidth, uint8_t height, uint8_t baseline) 
     : BaseFont(chartbl, nullptr, width, height, baseline )
     , indextbl(indextbl)
     , indexsize(indexsize)
     , halfwidth(halfwidth)
     {}
    constexpr font_type_t getType(void) const override { return ft_bdf;  }

    bool updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const override;
  };

  // deprecated array.
  extern const IFont* fontdata [];
}
//----------------------------------------------------------------------------
// Adafruit GFX font 

  struct GFXglyph { // Data stored PER GLYPH
    uint32_t bitmapOffset;     // Pointer into GFXfont->bitmap
    uint8_t  width, height;    // Bitmap dimensions in pixels
    uint8_t  xAdvance;         // Distance to advance cursor (x axis)
    int8_t   xOffset, yOffset; // Dist from cursor pos to UL corner
  };

  struct GFXfont : public lgfx::IFont { // Data stored for FONT AS A WHOLE:
    struct EncodeRange {
      uint16_t start;
      uint16_t end;
      uint16_t base;
    };

    uint8_t  *bitmap;      // Glyph bitmaps, concatenated
    GFXglyph *glyph;       // Glyph array
    uint16_t  first, last; // ASCII extents
    uint8_t   yAdvance;    // Newline distance (y axis)

    uint16_t range_num;    // Number of EncodeRange
    EncodeRange *range;    // Array ofEncodeRange

    constexpr GFXfont ( uint8_t *bitmap
                      , GFXglyph *glyph
                      , uint16_t first
                      , uint16_t last
                      , uint8_t yAdvance
                      , uint16_t range_num = 0
                      , EncodeRange *range = nullptr
                      )
    : bitmap   (bitmap   )
    , glyph    (glyph    )
    , first    (first    )
    , last     (last     )
    , yAdvance (yAdvance )
    , range_num(range_num)
    , range    (range    )
    {}

    GFXglyph* getGlyph(uint16_t uniCode) const;

    constexpr font_type_t getType(void) const override { return font_type_t::ft_gfx; }

    void getDefaultMetric(lgfx::FontMetrics *metrics) const;

    bool updateFontMetric(lgfx::FontMetrics *metrics, uint16_t uniCode) const override;
  };


//----------------------------------------------------------------------------

namespace fonts {
#ifdef __EFONT_FONT_DATA_H__
  static constexpr lgfx::BDFfont efont = { (const uint8_t *)efontFontData, efontFontList, sizeof(efontFontList)>>1, 16, 8, 16, 14 };
#endif

#ifdef misakiUTF16FontData_h
  static constexpr lgfx::BDFfont misaki = { (const uint8_t *)fdata, ftable, sizeof(ftable)>>1, 8, 4, 7, 6 };
#endif

  extern const lgfx::GLCDfont Font0;
  extern const lgfx::BMPfont  Font2;
  extern const lgfx::RLEfont  Font4;
  extern const lgfx::RLEfont  Font6;
  extern const lgfx::RLEfont  Font7;
  extern const lgfx::RLEfont  Font8;

  extern const GFXfont TomThumb                 ;
  extern const GFXfont FreeMono9pt7b            ;
  extern const GFXfont FreeMono12pt7b           ;
  extern const GFXfont FreeMono18pt7b           ;
  extern const GFXfont FreeMono24pt7b           ;
  extern const GFXfont FreeMonoBold9pt7b        ;
  extern const GFXfont FreeMonoBold12pt7b       ;
  extern const GFXfont FreeMonoBold18pt7b       ;
  extern const GFXfont FreeMonoBold24pt7b       ;
  extern const GFXfont FreeMonoOblique9pt7b     ;
  extern const GFXfont FreeMonoOblique12pt7b    ;
  extern const GFXfont FreeMonoOblique18pt7b    ;
  extern const GFXfont FreeMonoOblique24pt7b    ;
  extern const GFXfont FreeMonoBoldOblique9pt7b ;
  extern const GFXfont FreeMonoBoldOblique12pt7b;
  extern const GFXfont FreeMonoBoldOblique18pt7b;
  extern const GFXfont FreeMonoBoldOblique24pt7b;
  extern const GFXfont FreeSans9pt7b            ;
  extern const GFXfont FreeSans12pt7b           ;
  extern const GFXfont FreeSans18pt7b           ;
  extern const GFXfont FreeSans24pt7b           ;
  extern const GFXfont FreeSansBold9pt7b        ;
  extern const GFXfont FreeSansBold12pt7b       ;
  extern const GFXfont FreeSansBold18pt7b       ;
  extern const GFXfont FreeSansBold24pt7b       ;
  extern const GFXfont FreeSansOblique9pt7b     ;
  extern const GFXfont FreeSansOblique12pt7b    ;
  extern const GFXfont FreeSansOblique18pt7b    ;
  extern const GFXfont FreeSansOblique24pt7b    ;
  extern const GFXfont FreeSansBoldOblique9pt7b ;
  extern const GFXfont FreeSansBoldOblique12pt7b;
  extern const GFXfont FreeSansBoldOblique18pt7b;
  extern const GFXfont FreeSansBoldOblique24pt7b;
  extern const GFXfont FreeSerif9pt7b           ;
  extern const GFXfont FreeSerif12pt7b          ;
  extern const GFXfont FreeSerif18pt7b          ;
  extern const GFXfont FreeSerif24pt7b          ;
  extern const GFXfont FreeSerifItalic9pt7b     ;
  extern const GFXfont FreeSerifItalic12pt7b    ;
  extern const GFXfont FreeSerifItalic18pt7b    ;
  extern const GFXfont FreeSerifItalic24pt7b    ;
  extern const GFXfont FreeSerifBold9pt7b       ;
  extern const GFXfont FreeSerifBold12pt7b      ;
  extern const GFXfont FreeSerifBold18pt7b      ;
  extern const GFXfont FreeSerifBold24pt7b      ;
  extern const GFXfont FreeSerifBoldItalic9pt7b ;
  extern const GFXfont FreeSerifBoldItalic12pt7b;
  extern const GFXfont FreeSerifBoldItalic18pt7b;
  extern const GFXfont FreeSerifBoldItalic24pt7b;

  extern const GFXfont Orbitron_Light_24        ;
  extern const GFXfont Orbitron_Light_32        ;
  extern const GFXfont Roboto_Thin_24           ;
  extern const GFXfont Satisfy_24               ;
  extern const GFXfont Yellowtail_32            ;
}
using namespace fonts;

#endif
