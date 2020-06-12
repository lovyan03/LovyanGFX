#ifndef LGFX_FONTS_HPP_
#define LGFX_FONTS_HPP_

#include <cstdint>

namespace lgfx
{
  struct FontMetrics {
    std::int16_t width;
    std::int16_t x_advance;
    std::int16_t x_offset;
    std::int16_t height;
    std::int16_t y_advance;
    std::int16_t y_offset;
    std::int16_t baseline;
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
    virtual bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const = 0;
    virtual bool unloadFont(void) { return false; }
  };

  struct RunTimeFont : public IFont {
    virtual ~RunTimeFont() {}
  };

  struct BaseFont : public IFont {
    const std::uint8_t *chartbl;
    const std::uint8_t *widthtbl;
    const std::uint8_t width;
    const std::uint8_t height;
    const std::uint8_t baseline;
    BaseFont() = default;
    constexpr BaseFont(const std::uint8_t *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline)
     : chartbl  (chartbl  )
     , widthtbl (widthtbl )
     , width    (width    )
     , height   (height   )
     , baseline (baseline )
    {}
    void getDefaultMetric(FontMetrics *metrics) const override;
  };

  struct GLCDfont : public BaseFont {
    constexpr GLCDfont(const std::uint8_t *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    constexpr font_type_t getType(void) const override { return ft_glcd; }

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
  };

  struct BMPfont : public BaseFont {
    constexpr BMPfont(const std::uint8_t *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    constexpr font_type_t getType(void) const override { return ft_bmp;  } 

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
  };

  struct RLEfont : public BMPfont {
    constexpr RLEfont(const std::uint8_t *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline) : BMPfont(chartbl, widthtbl, width, height, baseline ) {}
    constexpr font_type_t getType(void) const override { return ft_rle; }
  };

  struct BDFfont : public BaseFont {
    const std::uint16_t *indextbl;
    std::uint16_t indexsize;
    std::uint8_t halfwidth;
    std::uint8_t y_advance;
    BDFfont() = default;
    constexpr BDFfont(const std::uint8_t *chartbl, const std::uint16_t *indextbl, std::uint16_t indexsize, std::uint8_t width, std::uint8_t halfwidth, std::uint8_t height, std::uint8_t baseline, std::uint8_t y_advance) 
     : BaseFont(chartbl, nullptr, width, height, baseline )
     , indextbl(indextbl)
     , indexsize(indexsize)
     , halfwidth(halfwidth)
     , y_advance(y_advance)
     {}
    constexpr font_type_t getType(void) const override { return ft_bdf;  }

    void getDefaultMetric(FontMetrics *metrics) const override;
    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
  };

  // deprecated array.
  extern const IFont* fontdata [];

//----------------------------------------------------------------------------
// Adafruit GFX font 

  struct EncodeRange {
    std::uint16_t start;
    std::uint16_t end;
    std::uint16_t base;
  };

  struct GFXglyph { // Data stored PER GLYPH
    std::uint32_t bitmapOffset;     // Pointer into GFXfont->bitmap
    std::uint8_t  width, height;    // Bitmap dimensions in pixels
    std::uint8_t  xAdvance;         // Distance to advance cursor (x axis)
    std::int8_t   xOffset, yOffset; // Dist from cursor pos to UL corner
  };

  struct GFXfont : public lgfx::IFont { // Data stored for FONT AS A WHOLE:
    std::uint8_t  *bitmap;      // Glyph bitmaps, concatenated
    GFXglyph *glyph;            // Glyph array
    std::uint16_t  first, last; // ASCII extents
    std::uint8_t   yAdvance;    // Newline distance (y axis)

    std::uint16_t range_num;    // Number of EncodeRange
    EncodeRange *range;         // Array ofEncodeRange

    constexpr GFXfont ( std::uint8_t *bitmap
                      , GFXglyph *glyph
                      , std::uint16_t first
                      , std::uint16_t last
                      , std::uint8_t yAdvance
                      , std::uint16_t range_num = 0
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

    GFXglyph* getGlyph(std::uint16_t uniCode) const;

    constexpr font_type_t getType(void) const override { return font_type_t::ft_gfx; }

    void getDefaultMetric(lgfx::FontMetrics *metrics) const;

    bool updateFontMetric(lgfx::FontMetrics *metrics, std::uint16_t uniCode) const override;
  };

}

//----------------------------------------------------------------------------

namespace fonts {
#ifdef __EFONT_FONT_DATA_H__
  static constexpr lgfx::BDFfont efont = { (const std::uint8_t *)efontFontData, efontFontList, sizeof(efontFontList)>>1, 16, 8, 16, 14, 16 };
#endif

  extern const lgfx::GLCDfont Font0;
  extern const lgfx::BMPfont  Font2;
  extern const lgfx::RLEfont  Font4;
  extern const lgfx::RLEfont  Font6;
  extern const lgfx::RLEfont  Font7;
  extern const lgfx::RLEfont  Font8;

  extern const lgfx::GFXfont TomThumb                 ;
  extern const lgfx::GFXfont FreeMono9pt7b            ;
  extern const lgfx::GFXfont FreeMono12pt7b           ;
  extern const lgfx::GFXfont FreeMono18pt7b           ;
  extern const lgfx::GFXfont FreeMono24pt7b           ;
  extern const lgfx::GFXfont FreeMonoBold9pt7b        ;
  extern const lgfx::GFXfont FreeMonoBold12pt7b       ;
  extern const lgfx::GFXfont FreeMonoBold18pt7b       ;
  extern const lgfx::GFXfont FreeMonoBold24pt7b       ;
  extern const lgfx::GFXfont FreeMonoOblique9pt7b     ;
  extern const lgfx::GFXfont FreeMonoOblique12pt7b    ;
  extern const lgfx::GFXfont FreeMonoOblique18pt7b    ;
  extern const lgfx::GFXfont FreeMonoOblique24pt7b    ;
  extern const lgfx::GFXfont FreeMonoBoldOblique9pt7b ;
  extern const lgfx::GFXfont FreeMonoBoldOblique12pt7b;
  extern const lgfx::GFXfont FreeMonoBoldOblique18pt7b;
  extern const lgfx::GFXfont FreeMonoBoldOblique24pt7b;
  extern const lgfx::GFXfont FreeSans9pt7b            ;
  extern const lgfx::GFXfont FreeSans12pt7b           ;
  extern const lgfx::GFXfont FreeSans18pt7b           ;
  extern const lgfx::GFXfont FreeSans24pt7b           ;
  extern const lgfx::GFXfont FreeSansBold9pt7b        ;
  extern const lgfx::GFXfont FreeSansBold12pt7b       ;
  extern const lgfx::GFXfont FreeSansBold18pt7b       ;
  extern const lgfx::GFXfont FreeSansBold24pt7b       ;
  extern const lgfx::GFXfont FreeSansOblique9pt7b     ;
  extern const lgfx::GFXfont FreeSansOblique12pt7b    ;
  extern const lgfx::GFXfont FreeSansOblique18pt7b    ;
  extern const lgfx::GFXfont FreeSansOblique24pt7b    ;
  extern const lgfx::GFXfont FreeSansBoldOblique9pt7b ;
  extern const lgfx::GFXfont FreeSansBoldOblique12pt7b;
  extern const lgfx::GFXfont FreeSansBoldOblique18pt7b;
  extern const lgfx::GFXfont FreeSansBoldOblique24pt7b;
  extern const lgfx::GFXfont FreeSerif9pt7b           ;
  extern const lgfx::GFXfont FreeSerif12pt7b          ;
  extern const lgfx::GFXfont FreeSerif18pt7b          ;
  extern const lgfx::GFXfont FreeSerif24pt7b          ;
  extern const lgfx::GFXfont FreeSerifItalic9pt7b     ;
  extern const lgfx::GFXfont FreeSerifItalic12pt7b    ;
  extern const lgfx::GFXfont FreeSerifItalic18pt7b    ;
  extern const lgfx::GFXfont FreeSerifItalic24pt7b    ;
  extern const lgfx::GFXfont FreeSerifBold9pt7b       ;
  extern const lgfx::GFXfont FreeSerifBold12pt7b      ;
  extern const lgfx::GFXfont FreeSerifBold18pt7b      ;
  extern const lgfx::GFXfont FreeSerifBold24pt7b      ;
  extern const lgfx::GFXfont FreeSerifBoldItalic9pt7b ;
  extern const lgfx::GFXfont FreeSerifBoldItalic12pt7b;
  extern const lgfx::GFXfont FreeSerifBoldItalic18pt7b;
  extern const lgfx::GFXfont FreeSerifBoldItalic24pt7b;

  extern const lgfx::GFXfont Orbitron_Light_24        ;
  extern const lgfx::GFXfont Orbitron_Light_32        ;
  extern const lgfx::GFXfont Roboto_Thin_24           ;
  extern const lgfx::GFXfont Satisfy_24               ;
  extern const lgfx::GFXfont Yellowtail_32            ;
}
using namespace fonts;

#endif
