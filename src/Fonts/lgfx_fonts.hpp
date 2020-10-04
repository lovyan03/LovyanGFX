#ifndef LGFX_FONTS_HPP_
#define LGFX_FONTS_HPP_

#include <cstdint>

namespace lgfx
{
  struct LGFXBase;
  struct TextStyle;

  struct FontMetrics
  {
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
    virtual std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const = 0;
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
    font_type_t getType(void) const override { return ft_glcd; }

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;
  };

  struct BMPfont : public BaseFont {
    constexpr BMPfont(const std::uint8_t *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_bmp;  } 

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;
  };

  struct RLEfont : public BMPfont {
    constexpr RLEfont(const std::uint8_t *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline) : BMPfont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_rle; }
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;
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
    font_type_t getType(void) const override { return ft_bdf;  }

    void getDefaultMetric(FontMetrics *metrics) const override;
    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;
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

    font_type_t getType(void) const override { return font_type_t::ft_gfx; }

    void getDefaultMetric(lgfx::FontMetrics *metrics) const;

    bool updateFontMetric(lgfx::FontMetrics *metrics, std::uint16_t uniCode) const override;

    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;
  };

//----------------------------------------------------------------------------
// VLW font 
  struct DataWrapper;

  struct RunTimeFont : public IFont {
    virtual ~RunTimeFont() {}
  };

  struct VLWfont : public RunTimeFont
  {
    std::uint16_t gCount;     // Total number of characters
    std::uint16_t yAdvance;   // Line advance
    std::uint16_t spaceWidth; // Width of a space character
    std::int16_t  ascent;     // Height of top of 'd' above baseline, other characters may be taller
    std::int16_t  descent;    // Offset to bottom of 'p', other characters may have a larger descent
    std::uint16_t maxAscent;  // Maximum ascent found in font
    std::uint16_t maxDescent; // Maximum descent found in font

    // These are for the metrics for each individual glyph (so we don't need to seek this in file and waste time)
    std::uint16_t* gUnicode  = nullptr;  //UTF-16 code, the codes are searched so do not need to be sequential
    std::uint8_t*  gWidth    = nullptr;  //cwidth
    std::uint8_t*  gxAdvance = nullptr;  //setWidth
    std::int8_t*   gdX       = nullptr;  //leftExtent
    std::uint32_t* gBitmap   = nullptr;  //file pointer to greyscale bitmap

    DataWrapper* _fontData = nullptr;
    bool _fontLoaded = false; // Flags when a anti-aliased font is loaded

    font_type_t getType(void) const override { return ft_vlw; } 

    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;

    void getDefaultMetric(FontMetrics *metrics) const override;

    virtual ~VLWfont();

    bool unloadFont(void) override;

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;

    bool getUnicodeIndex(std::uint16_t unicode, std::uint16_t *index) const;

    bool loadFont(DataWrapper* data);
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

#ifndef _GFXFONT_H_
#define _GFXFONT_H_
typedef lgfx::GFXfont GFXfont;
typedef lgfx::GFXglyph GFXglyph;
#endif

#endif
