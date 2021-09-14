#ifndef LGFX_TTFFONT_HPP_
#define LGFX_TTFFONT_HPP_

#include "lgfx_fonts.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
// TTF font

  struct TTFfont : public RunTimeFont
  {
    virtual ~TTFfont();

    font_type_t getType(void) const override { return ft_ttf; }

    size_t drawChar(LGFXBase* gfx, int32_t x, int32_t y, uint16_t c, const TextStyle* style, FontMetrics* metrics, int32_t& filled_x) const override;

    void getDefaultMetric(FontMetrics *metrics) const override;

    bool loadFont(DataWrapper* data) override;

    bool unloadFont(void) override;

    bool updateFontMetric(FontMetrics *metrics, uint16_t uniCode) const override;

  private:

    bool _get_glyph_index(uint16_t unicode, uint16_t *index) const;

    bool tt_face_build_cmaps( void );

#pragma pack(push)
#pragma pack(1)

    struct TTC_HeaderRec
    {
      int32_t version = 0;
      uint32_t count = 0;
      size_t* offsets = nullptr;
    };
    TTC_HeaderRec _ttc_header;

    struct TT_TableRec
    {
      uint32_t Tag;
      uint32_t CheckSum;
      uint32_t Offset;
      uint32_t Length;
    };

    struct SFNT_HeaderRec
    {
      uint32_t format_tag;
      uint16_t num_tables;
      uint16_t search_range;
      uint16_t entry_selector;
      uint16_t range_shift;
      uint32_t offset;  // not in file
    };

    struct TT_Header
    {
      int32_t Table_Version;
      int32_t Font_Revision;

      int32_t CheckSum_Adjust;
      int32_t Magic_Number;

      uint16_t Flags;
      uint16_t Units_Per_EM;

      int32_t Created [2];
      int32_t Modified[2];

      int16_t xMin;
      int16_t yMin;
      int16_t xMax;
      int16_t yMax;

      uint16_t Mac_Style;
      uint16_t Lowest_Rec_PPEM;

      int16_t Font_Direction;
      int16_t Index_To_Loc_Format;
      int16_t Glyph_Data_Format;
      void load(DataWrapper* data);
    };
    struct TT_MaxProfile
    {
      int32_t version;
      uint16_t numGlyphs;
      uint16_t maxPoints;
      uint16_t maxContours;
      uint16_t maxCompositePoints;
      uint16_t maxCompositeContours;
      uint16_t maxZones;
      uint16_t maxTwilightPoints;
      uint16_t maxStorage;
      uint16_t maxFunctionDefs;
      uint16_t maxInstructionDefs;
      uint16_t maxStackElements;
      uint16_t maxSizeOfInstructions;
      uint16_t maxComponentElements;
      uint16_t maxComponentDepth;
      void load(DataWrapper* data);
    };

    struct TT_HoriHeader
    {
      int32_t Version;
      int16_t Ascender;
      int16_t Descender;
      int16_t Line_Gap;
      union
      {
        struct
        {
          uint16_t advance_Width_Max;      /* advance width maximum */
          int16_t min_Left_Side_Bearing;  /* minimum left-sb       */
          int16_t min_Right_Side_Bearing; /* minimum right-sb      */
          int16_t xMax_Extent;            /* xmax extents          */
        };
        struct
        {
          uint16_t advance_Height_Max;      /* advance height maximum */
          int16_t min_Top_Side_Bearing;    /* minimum left-sb or top-sb       */
          int16_t min_Bottom_Side_Bearing; /* minimum right-sb or bottom-sb   */
          int16_t yMax_Extent;             /* xmax or ymax extents            */
        };
      };
      int16_t caret_Slope_Rise;
      int16_t caret_Slope_Run;
      int16_t caret_Offset;
      int16_t Reserved[4];
      int16_t metric_Data_Format;
      union
      {
        uint16_t number_Of_HMetrics;
        uint16_t number_Of_VMetrics;
      };
      void* long_metrics;
      void* short_metrics;
      void load(DataWrapper* data);
    };

    struct TT_OS2
    {
      uint16_t version;
      int16_t  xAvgCharWidth;
      uint16_t usWeightClass;
      uint16_t usWidthClass;
      int16_t  fsType;
      int16_t  ySubscriptXSize;
      int16_t  ySubscriptYSize;
      int16_t  ySubscriptXOffset;
      int16_t  ySubscriptYOffset;
      int16_t  ySuperscriptXSize;
      int16_t  ySuperscriptYSize;
      int16_t  ySuperscriptXOffset;
      int16_t  ySuperscriptYOffset;
      int16_t  yStrikeoutSize;
      int16_t  yStrikeoutPosition;
      int16_t  sFamilyClass;
      uint8_t  panose[10];
      uint32_t ulUnicodeRange1;
      uint32_t ulUnicodeRange2;
      uint32_t ulUnicodeRange3;
      uint32_t ulUnicodeRange4;
      uint8_t achVendID[4];

      uint16_t fsSelection;
      uint16_t usFirstCharIndex;
      uint16_t usLastCharIndex;
      int16_t  sTypoAscender;
      int16_t  sTypoDescender;
      int16_t  sTypoLineGap;
      uint16_t usWinAscent;
      uint16_t usWinDescent;

      // only version 1 tables:
      uint32_t ulCodePageRange1;
      uint32_t ulCodePageRange2;

      // only version 2 tables:
      int16_t  sxHeight;
      int16_t  sCapHeight;
      uint16_t usDefaultChar;
      uint16_t usBreakChar;
      uint16_t usMaxContext;

      void load(DataWrapper* data);
    };

    struct cmap_t  // 自作;
    {
      uint16_t* rawdata = nullptr;
      bool load(DataWrapper* data);
    };

#define FT_ENC_TAG( value, a, b, c, d )         \
          value = ( ( (uint32_t)(a) << 24 ) |  \
                    ( (uint32_t)(b) << 16 ) |  \
                    ( (uint32_t)(c) <<  8 ) |  \
                      (uint32_t)(d)         )

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
      uint16_t platform_id;
      uint16_t encoding_id;
    };

    struct Metrics_t
    {
      int16_t  left_bearing   = 0;
      int16_t  top_bearing    = 0;
      uint16_t advance_width  = 0;
      uint16_t advance_height = 0;
    };

#pragma pack(pop)

    SFNT_HeaderRec _sfnt;
    TT_Header     _header;
    TT_HoriHeader _horizontal;   /* TrueType horizontal header     */
    TT_MaxProfile _max_profile;
    bool          _vertical_info = false;
    TT_HoriHeader _vertical;     /* TT Vertical header, if present */
    TT_OS2        _os2;

    TT_TableRec* _dir_tables = nullptr;
    //uint8_t* _cmap_table = nullptr;
    //uint32_t _cmap_size = 0;
    cmap_t _cmap;

    uint32_t _horz_metrics_size;
    uint32_t _horz_metrics_offset;
    uint32_t _vert_metrics_size;
    uint32_t _vert_metrics_offset;


    TT_TableRec* tt_face_lookup_table(uint32_t tag);
    bool tt_face_goto_table( uint32_t tag
                            , DataWrapper* data
                            , uint32_t* length = nullptr);

    Metrics_t _get_metrics(uint32_t glyph_index) const;

    uint_fast8_t face_index = 0;
  };

//----------------------------------------------------------------------------
 }
}
//----------------------------------------------------------------------------

#endif
