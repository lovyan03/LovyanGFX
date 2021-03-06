#ifndef LGFX_FONTS_HPP_
#define LGFX_FONTS_HPP_

#include <cstdint>
#include "misc/enum.hpp"

namespace lgfx
{
 inline namespace v1
 {
  struct LGFXBase;
  struct DataWrapper;

  struct TextStyle
  {
    std::uint32_t fore_rgb888 = 0xFFFFFFU;
    std::uint32_t back_rgb888 = 0;
    float size_x = 1;
    float size_y = 1;
    textdatum_t datum = textdatum_t::top_left;
    bool utf8 = true;
    bool cp437 = false;
  };

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
    , ft_u8g2
    , ft_ttf
    };

    virtual font_type_t getType(void) const { return font_type_t::ft_unknown; }
    virtual void getDefaultMetric(FontMetrics *metrics) const = 0;
    virtual bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const = 0;
    virtual bool unloadFont(void) { return false; }
    virtual std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const = 0;

  protected:
    std::size_t drawCharDummy(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, const TextStyle* style) const;
  };

  struct BaseFont : public IFont {
    union
    {
      const void *void_chartbl;
      const std::uint8_t *chartbl;
    };
    const std::uint8_t *widthtbl;
    const std::uint8_t width;
    const std::uint8_t height;
    const std::uint8_t baseline;
    BaseFont() = default;
    constexpr BaseFont(const void *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline)
     : void_chartbl(chartbl  )
     , widthtbl (widthtbl )
     , width    (width    )
     , height   (height   )
     , baseline (baseline )
    {}
    void getDefaultMetric(FontMetrics *metrics) const override;
  };

  struct GLCDfont : public BaseFont {
    constexpr GLCDfont(const void *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_glcd; }

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;
  };

  struct FixedBMPfont : public BaseFont {
    constexpr FixedBMPfont(const void *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_bmp;  }

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;
  };

  struct BMPfont : public BaseFont {
    constexpr BMPfont(const void *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_bmp;  }

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;
  };

  struct RLEfont : public BMPfont {
    constexpr RLEfont(const void *chartbl, const std::uint8_t *widthtbl, std::uint8_t width, std::uint8_t height, std::uint8_t baseline) : BMPfont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_rle; }
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;
  };

  struct BDFfont : public BaseFont {
    const std::uint16_t *indextbl;
    std::uint16_t indexsize;
    std::uint8_t halfwidth;
    std::uint8_t y_advance;
    BDFfont() = default;
    constexpr BDFfont(const void *chartbl, const std::uint16_t *indextbl, std::uint16_t indexsize, std::uint8_t width, std::uint8_t halfwidth, std::uint8_t height, std::uint8_t baseline, std::uint8_t y_advance)
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

  struct GFXfont : public lgfx::IFont
  { // Data stored for FONT AS A WHOLE:
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

    font_type_t getType(void) const override { return font_type_t::ft_gfx; }
    void getDefaultMetric(FontMetrics *metrics) const override;
    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;

  private:
    GFXglyph* getGlyph(std::uint16_t uniCode) const;
  };

//----------------------------------------------------------------------------
// u8g2 font

  struct U8g2font : public lgfx::IFont
  {
    constexpr U8g2font(const std::uint8_t *u8g2_font) : _font(u8g2_font) {}
    font_type_t getType(void) const override { return ft_u8g2; }

    std::uint8_t glyph_cnt (void) const { return _font[0]; }
    std::uint8_t bbx_mode  (void) const { return _font[1]; }
    std::uint8_t bits_per_0(void) const { return _font[2]; }
    std::uint8_t bits_per_1(void) const { return _font[3]; }
    std::uint8_t bits_per_char_width (void) const { return _font[4]; }
    std::uint8_t bits_per_char_height(void) const { return _font[5]; }
    std::uint8_t bits_per_char_x     (void) const { return _font[6]; }
    std::uint8_t bits_per_char_y     (void) const { return _font[7]; }
    std::uint8_t bits_per_delta_x    (void) const { return _font[8]; }
    std::int8_t max_char_width (void) const { return _font[ 9]; }
    std::int8_t max_char_height(void) const { return _font[10]; } /* overall height, NOT ascent. Instead ascent = max_char_height + y_offset */
    std::int8_t x_offset       (void) const { return _font[11]; }
    std::int8_t y_offset       (void) const { return _font[12]; }
    std::int8_t ascent_A    (void) const { return _font[13]; }
    std::int8_t descent_g   (void) const { return _font[14]; }  /* usually a negative value */
    std::int8_t ascent_para (void) const { return _font[15]; }
    std::int8_t descent_para(void) const { return _font[16]; }

    std::uint16_t start_pos_upper_A(void) const { return _font[17] << 8 | _font[18]; }
    std::uint16_t start_pos_lower_a(void) const { return _font[19] << 8 | _font[20]; }
    std::uint16_t start_pos_unicode(void) const { return _font[21] << 8 | _font[22]; }

    void getDefaultMetric(FontMetrics *metrics) const override;
    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;
    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;

  private:
    const uint8_t* getGlyph(std::uint16_t encoding) const;
    const std::uint8_t* _font;
  };

//----------------------------------------------------------------------------

  struct RunTimeFont : public IFont
  {
    virtual ~RunTimeFont() = default;
    virtual bool loadFont(DataWrapper* data) = 0;

    DataWrapper* _fontData = nullptr;
    bool _fontLoaded = false;
  };

//----------------------------------------------------------------------------
// VLW font
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

    font_type_t getType(void) const override { return ft_vlw; }

    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;

    void getDefaultMetric(FontMetrics *metrics) const override;

    virtual ~VLWfont();

    bool loadFont(DataWrapper* data) override;

    bool unloadFont(void) override;

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;

    bool getUnicodeIndex(std::uint16_t unicode, std::uint16_t *index) const;
  };

//----------------------------------------------------------------------------
// TTF font
  struct TTFfont : public RunTimeFont
  {
    virtual ~TTFfont();

    font_type_t getType(void) const override { return ft_ttf; }

    std::size_t drawChar(LGFXBase* gfx, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const override;

    void getDefaultMetric(FontMetrics *metrics) const override;

    bool loadFont(DataWrapper* data) override;

    bool unloadFont(void) override;

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override;

    bool getUnicodeIndex(std::uint16_t unicode, std::uint16_t *index) const;

  private:
    struct TTC_HeaderRec
    {
      std::int32_t version = 0;
      std::int32_t count = 0;
      std::size_t* offsets = nullptr;
    };
    TTC_HeaderRec _ttc_header;

    struct TT_TableRec
    {
      std::uint32_t Tag;
      std::uint32_t CheckSum;
      std::uint32_t Offset;
      std::uint32_t Length;
    };

    struct SFNT_HeaderRec
    {
      std::uint32_t format_tag;
      std::uint16_t num_tables;
      std::uint16_t search_range;
      std::uint16_t entry_selector;
      std::uint16_t range_shift;
      std::uint32_t offset;  // not in file
    };

    struct TT_Header
    {
      std::int32_t Table_Version;
      std::int32_t Font_Revision;

      std::int32_t CheckSum_Adjust;
      std::int32_t Magic_Number;

      std::uint16_t Flags;
      std::uint16_t Units_Per_EM;

      std::int32_t Created [2];
      std::int32_t Modified[2];

      std::int16_t xMin;
      std::int16_t yMin;
      std::int16_t xMax;
      std::int16_t yMax;

      std::uint16_t Mac_Style;
      std::uint16_t Lowest_Rec_PPEM;

      std::int16_t Font_Direction;
      std::int16_t Index_To_Loc_Format;
      std::int16_t Glyph_Data_Format;
      void load(DataWrapper* data);
    };
    struct TT_MaxProfile
    {
      std::int32_t version;
      std::uint16_t numGlyphs;
      std::uint16_t maxPoints;
      std::uint16_t maxContours;
      std::uint16_t maxCompositePoints;
      std::uint16_t maxCompositeContours;
      std::uint16_t maxZones;
      std::uint16_t maxTwilightPoints;
      std::uint16_t maxStorage;
      std::uint16_t maxFunctionDefs;
      std::uint16_t maxInstructionDefs;
      std::uint16_t maxStackElements;
      std::uint16_t maxSizeOfInstructions;
      std::uint16_t maxComponentElements;
      std::uint16_t maxComponentDepth;
      void load(DataWrapper* data);
    };

    struct TT_HoriHeader
    {
      std::int32_t Version;
      std::int16_t Ascender;
      std::int16_t Descender;
      std::int16_t Line_Gap;
      std::uint16_t advance_Width_Max;      /* advance width maximum */
      std::int16_t min_Left_Side_Bearing;  /* minimum left-sb       */
      std::int16_t min_Right_Side_Bearing; /* minimum right-sb      */
      std::int16_t xMax_Extent;            /* xmax extents          */
      std::int16_t caret_Slope_Rise;
      std::int16_t caret_Slope_Run;
      std::int16_t caret_Offset;
      std::int16_t Reserved[4];
      std::int16_t metric_Data_Format;
      std::uint16_t number_Of_HMetrics;

      void* long_metrics;
      void* short_metrics;
      void load(DataWrapper* data);
    };
    struct TT_VertHeader
    {
      std::int32_t Version;
      std::int16_t Ascender;
      std::int16_t Descender;
      std::int16_t Line_Gap;
      std::uint16_t advance_Height_Max;      /* advance height maximum */
      std::int16_t min_Top_Side_Bearing;    /* minimum left-sb or top-sb       */
      std::int16_t min_Bottom_Side_Bearing; /* minimum right-sb or bottom-sb   */
      std::int16_t yMax_Extent;             /* xmax or ymax extents            */
      std::int16_t caret_Slope_Rise;
      std::int16_t caret_Slope_Run;
      std::int16_t caret_Offset;
      std::int16_t Reserved[4];
      std::int16_t metric_Data_Format;
      std::uint16_t number_Of_VMetrics;

      void* long_metrics;
      void* short_metrics;
      void load(DataWrapper* data);
    };

#define FT_ENC_TAG( value, a, b, c, d )         \
          value = ( ( (std::uint32_t)(a) << 24 ) |  \
                    ( (std::uint32_t)(b) << 16 ) |  \
                    ( (std::uint32_t)(c) <<  8 ) |  \
                      (std::uint32_t)(d)         )

    enum FT_Encoding
    {
      FT_ENC_TAG( FT_ENCODING_NONE, 0, 0, 0, 0 ),

      FT_ENC_TAG( FT_ENCODING_MS_SYMBOL, 's', 'y', 'm', 'b' ),
      FT_ENC_TAG( FT_ENCODING_UNICODE,   'u', 'n', 'i', 'c' ),

      FT_ENC_TAG( FT_ENCODING_SJIS,    's', 'j', 'i', 's' ),
      FT_ENC_TAG( FT_ENCODING_GB2312,  'g', 'b', ' ', ' ' ),
      FT_ENC_TAG( FT_ENCODING_BIG5,    'b', 'i', 'g', '5' ),
      FT_ENC_TAG( FT_ENCODING_WANSUNG, 'w', 'a', 'n', 's' ),
      FT_ENC_TAG( FT_ENCODING_JOHAB,   'j', 'o', 'h', 'a' ),

      /* for backwards compatibility */
      FT_ENCODING_MS_SJIS    = FT_ENCODING_SJIS,
      FT_ENCODING_MS_GB2312  = FT_ENCODING_GB2312,
      FT_ENCODING_MS_BIG5    = FT_ENCODING_BIG5,
      FT_ENCODING_MS_WANSUNG = FT_ENCODING_WANSUNG,
      FT_ENCODING_MS_JOHAB   = FT_ENCODING_JOHAB,

      FT_ENC_TAG( FT_ENCODING_ADOBE_STANDARD, 'A', 'D', 'O', 'B' ),
      FT_ENC_TAG( FT_ENCODING_ADOBE_EXPERT,   'A', 'D', 'B', 'E' ),
      FT_ENC_TAG( FT_ENCODING_ADOBE_CUSTOM,   'A', 'D', 'B', 'C' ),
      FT_ENC_TAG( FT_ENCODING_ADOBE_LATIN_1,  'l', 'a', 't', '1' ),

      FT_ENC_TAG( FT_ENCODING_OLD_LATIN_2, 'l', 'a', 't', '2' ),

      FT_ENC_TAG( FT_ENCODING_APPLE_ROMAN, 'a', 'r', 'm', 'n' )
    };

    struct FT_CharMapRec
    {
//    FT_Face      face;
      FT_Encoding  encoding;
      std::uint16_t platform_id;
      std::uint16_t encoding_id;
    };

    SFNT_HeaderRec _sfnt;
    TT_Header     _header;
    TT_HoriHeader _horizontal;   /* TrueType horizontal header     */
    TT_MaxProfile _max_profile;
    bool          _vertical_info = false;
    TT_VertHeader _vertical;     /* TT Vertical header, if present */

    TT_TableRec* _dir_tables = nullptr;
    std::uint8_t* _cmap_table = nullptr;
    std::uint32_t _cmap_size = 0;

    std::uint32_t horz_metrics_size;
    std::uint32_t horz_metrics_offset;
    std::uint32_t vert_metrics_size;
    std::uint32_t vert_metrics_offset;

    TT_TableRec* tt_face_lookup_table(std::uint32_t tag);
    bool tt_face_goto_table( std::uint32_t tag
                            , DataWrapper* data
                            , std::uint32_t* length = nullptr);

    std::uint_fast8_t face_index = 0;
  };

//----------------------------------------------------------------------------
 }
}
//----------------------------------------------------------------------------

namespace fonts
{
 inline namespace v1
 {
#ifdef __EFONT_FONT_DATA_H__
  static constexpr lgfx::BDFfont efont = { (const std::uint8_t *)efontFontData, efontFontList, sizeof(efontFontList)>>1, 16, 8, 16, 14, 16 };
#endif

  extern const lgfx::GLCDfont Font0;
  extern const lgfx::BMPfont  Font2;
  extern const lgfx::RLEfont  Font4;
  extern const lgfx::RLEfont  Font6;
  extern const lgfx::RLEfont  Font7;
  extern const lgfx::RLEfont  Font8;
  extern const lgfx::GLCDfont Font8x8C64;
  extern const lgfx::FixedBMPfont AsciiFont8x16;
  extern const lgfx::FixedBMPfont AsciiFont24x48;

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

  extern const lgfx::GFXfont Orbitron_Light_24;
  extern const lgfx::GFXfont Orbitron_Light_32;
  extern const lgfx::GFXfont Roboto_Thin_24   ;
  extern const lgfx::GFXfont Satisfy_24       ;
  extern const lgfx::GFXfont Yellowtail_32    ;

  extern const lgfx::U8g2font lgfxJapanMincho_8  ;
  extern const lgfx::U8g2font lgfxJapanMincho_12 ;
  extern const lgfx::U8g2font lgfxJapanMincho_16 ;
  extern const lgfx::U8g2font lgfxJapanMincho_20 ;
  extern const lgfx::U8g2font lgfxJapanMincho_24 ;
  extern const lgfx::U8g2font lgfxJapanMincho_28 ;
  extern const lgfx::U8g2font lgfxJapanMincho_32 ;
  extern const lgfx::U8g2font lgfxJapanMincho_36 ;
  extern const lgfx::U8g2font lgfxJapanMincho_40 ;
  extern const lgfx::U8g2font lgfxJapanMinchoP_8 ;
  extern const lgfx::U8g2font lgfxJapanMinchoP_12;
  extern const lgfx::U8g2font lgfxJapanMinchoP_16;
  extern const lgfx::U8g2font lgfxJapanMinchoP_20;
  extern const lgfx::U8g2font lgfxJapanMinchoP_24;
  extern const lgfx::U8g2font lgfxJapanMinchoP_28;
  extern const lgfx::U8g2font lgfxJapanMinchoP_32;
  extern const lgfx::U8g2font lgfxJapanMinchoP_36;
  extern const lgfx::U8g2font lgfxJapanMinchoP_40;
  extern const lgfx::U8g2font lgfxJapanGothic_8  ;
  extern const lgfx::U8g2font lgfxJapanGothic_12 ;
  extern const lgfx::U8g2font lgfxJapanGothic_16 ;
  extern const lgfx::U8g2font lgfxJapanGothic_20 ;
  extern const lgfx::U8g2font lgfxJapanGothic_24 ;
  extern const lgfx::U8g2font lgfxJapanGothic_28 ;
  extern const lgfx::U8g2font lgfxJapanGothic_32 ;
  extern const lgfx::U8g2font lgfxJapanGothic_36 ;
  extern const lgfx::U8g2font lgfxJapanGothic_40 ;
  extern const lgfx::U8g2font lgfxJapanGothicP_8 ;
  extern const lgfx::U8g2font lgfxJapanGothicP_12;
  extern const lgfx::U8g2font lgfxJapanGothicP_16;
  extern const lgfx::U8g2font lgfxJapanGothicP_20;
  extern const lgfx::U8g2font lgfxJapanGothicP_24;
  extern const lgfx::U8g2font lgfxJapanGothicP_28;
  extern const lgfx::U8g2font lgfxJapanGothicP_32;
  extern const lgfx::U8g2font lgfxJapanGothicP_36;
  extern const lgfx::U8g2font lgfxJapanGothicP_40;

  extern const lgfx::U8g2font efontCN_10   ;
  extern const lgfx::U8g2font efontCN_10_b ;
  extern const lgfx::U8g2font efontCN_10_bi;
  extern const lgfx::U8g2font efontCN_10_i ;
  extern const lgfx::U8g2font efontCN_12   ;
  extern const lgfx::U8g2font efontCN_12_b ;
  extern const lgfx::U8g2font efontCN_12_bi;
  extern const lgfx::U8g2font efontCN_12_i ;
  extern const lgfx::U8g2font efontCN_14   ;
  extern const lgfx::U8g2font efontCN_14_b ;
  extern const lgfx::U8g2font efontCN_14_bi;
  extern const lgfx::U8g2font efontCN_14_i ;
  extern const lgfx::U8g2font efontCN_16   ;
  extern const lgfx::U8g2font efontCN_16_b ;
  extern const lgfx::U8g2font efontCN_16_bi;
  extern const lgfx::U8g2font efontCN_16_i ;
  extern const lgfx::U8g2font efontCN_24   ;
  extern const lgfx::U8g2font efontCN_24_b ;
  extern const lgfx::U8g2font efontCN_24_bi;
  extern const lgfx::U8g2font efontCN_24_i ;

  extern const lgfx::U8g2font efontJA_10   ;
  extern const lgfx::U8g2font efontJA_10_b ;
  extern const lgfx::U8g2font efontJA_10_bi;
  extern const lgfx::U8g2font efontJA_10_i ;
  extern const lgfx::U8g2font efontJA_12   ;
  extern const lgfx::U8g2font efontJA_12_b ;
  extern const lgfx::U8g2font efontJA_12_bi;
  extern const lgfx::U8g2font efontJA_12_i ;
  extern const lgfx::U8g2font efontJA_14   ;
  extern const lgfx::U8g2font efontJA_14_b ;
  extern const lgfx::U8g2font efontJA_14_bi;
  extern const lgfx::U8g2font efontJA_14_i ;
  extern const lgfx::U8g2font efontJA_16   ;
  extern const lgfx::U8g2font efontJA_16_b ;
  extern const lgfx::U8g2font efontJA_16_bi;
  extern const lgfx::U8g2font efontJA_16_i ;
  extern const lgfx::U8g2font efontJA_24   ;
  extern const lgfx::U8g2font efontJA_24_b ;
  extern const lgfx::U8g2font efontJA_24_bi;
  extern const lgfx::U8g2font efontJA_24_i ;

  extern const lgfx::U8g2font efontKR_10   ;
  extern const lgfx::U8g2font efontKR_10_b ;
  extern const lgfx::U8g2font efontKR_10_bi;
  extern const lgfx::U8g2font efontKR_10_i ;
  extern const lgfx::U8g2font efontKR_12   ;
  extern const lgfx::U8g2font efontKR_12_b ;
  extern const lgfx::U8g2font efontKR_12_bi;
  extern const lgfx::U8g2font efontKR_12_i ;
  extern const lgfx::U8g2font efontKR_14   ;
  extern const lgfx::U8g2font efontKR_14_b ;
  extern const lgfx::U8g2font efontKR_14_bi;
  extern const lgfx::U8g2font efontKR_14_i ;
  extern const lgfx::U8g2font efontKR_16   ;
  extern const lgfx::U8g2font efontKR_16_b ;
  extern const lgfx::U8g2font efontKR_16_bi;
  extern const lgfx::U8g2font efontKR_16_i ;
  extern const lgfx::U8g2font efontKR_24   ;
  extern const lgfx::U8g2font efontKR_24_b ;
  extern const lgfx::U8g2font efontKR_24_bi;
  extern const lgfx::U8g2font efontKR_24_i ;

  extern const lgfx::U8g2font efontTW_10   ;
  extern const lgfx::U8g2font efontTW_10_b ;
  extern const lgfx::U8g2font efontTW_10_bi;
  extern const lgfx::U8g2font efontTW_10_i ;
  extern const lgfx::U8g2font efontTW_12   ;
  extern const lgfx::U8g2font efontTW_12_b ;
  extern const lgfx::U8g2font efontTW_12_bi;
  extern const lgfx::U8g2font efontTW_12_i ;
  extern const lgfx::U8g2font efontTW_14   ;
  extern const lgfx::U8g2font efontTW_14_b ;
  extern const lgfx::U8g2font efontTW_14_bi;
  extern const lgfx::U8g2font efontTW_14_i ;
  extern const lgfx::U8g2font efontTW_16   ;
  extern const lgfx::U8g2font efontTW_16_b ;
  extern const lgfx::U8g2font efontTW_16_bi;
  extern const lgfx::U8g2font efontTW_16_i ;
  extern const lgfx::U8g2font efontTW_24   ;
  extern const lgfx::U8g2font efontTW_24_b ;
  extern const lgfx::U8g2font efontTW_24_bi;
  extern const lgfx::U8g2font efontTW_24_i ;
 }
}

using namespace fonts;

#ifndef _GFXFONT_H_
#define _GFXFONT_H_
using GFXfont = lgfx::GFXfont;
using GFXglyph = lgfx::GFXglyph;
#endif

#endif
