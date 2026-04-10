/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

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
#pragma once

#include <stdint.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  namespace jpeg_div
  {
    enum jpeg_div_t
    {
      JPEG_DIV_NONE,
      JPEG_DIV_2,
      JPEG_DIV_4,
      JPEG_DIV_8,
      JPEG_DIV_MAX
    };
  }

//----------------------------------------------------------------------------

  namespace epd_mode
  {
    enum epd_mode_t : uint8_t
    {
      epd_quality = 1,
      epd_text    = 2,
      epd_fast    = 3,
      epd_fastest = 4,
    };
  }
  using namespace epd_mode;

//----------------------------------------------------------------------------

  namespace colors  // Colour enumeration
  {
    #ifdef TFT_BLACK
    #undef TFT_BLACK
    #undef TFT_NAVY
    #undef TFT_DARKGREEN
    #undef TFT_DARKCYAN
    #undef TFT_MAROON
    #undef TFT_PURPLE
    #undef TFT_OLIVE
    #undef TFT_LIGHTGREY
    #undef TFT_LIGHTGRAY
    #undef TFT_DARKGREY
    #undef TFT_DARKGRAY
    #undef TFT_BLUE
    #undef TFT_GREEN
    #undef TFT_CYAN
    #undef TFT_RED
    #undef TFT_MAGENTA
    #undef TFT_YELLOW
    #undef TFT_WHITE
    #undef TFT_ORANGE
    #undef TFT_GREENYELLOW
    #undef TFT_PINK
    #undef TFT_BROWN
    #undef TFT_GOLD
    #undef TFT_SILVER
    #undef TFT_SKYBLUE
    #undef TFT_VIOLET
    #undef TFT_TRANSPARENT
    #undef TFT_ALICEBLUE
    #undef TFT_ANTIQUEWHITE
    #undef TFT_AQUAMARINE
    #undef TFT_AZURE
    #undef TFT_BEIGE
    #undef TFT_BISQUE
    #undef TFT_BLANCHEDALMOND
    #undef TFT_BLUEVIOLET
    #undef TFT_BURLYWOOD
    #undef TFT_CADETBLUE
    #undef TFT_CHARTREUSE
    #undef TFT_CHOCOLATE
    #undef TFT_CORAL
    #undef TFT_CORNFLOWERBLUE
    #undef TFT_CORNSILK
    #undef TFT_CRIMSON
    #undef TFT_DARKGOLDENROD
    #undef TFT_DARKKHAKI
    #undef TFT_DARKMAGENTA
    #undef TFT_DARKOLIVEGREEN
    #undef TFT_DARKORANGE
    #undef TFT_DARKORCHID
    #undef TFT_DARKRED
    #undef TFT_DARKSALMON
    #undef TFT_DARKSEAGREEN
    #undef TFT_DARKSLATEBLUE
    #undef TFT_DARKSLATEGRAY
    #undef TFT_DARKTURQUOISE
    #undef TFT_DARKVIOLET
    #undef TFT_DEEPPINK
    #undef TFT_DEEPSKYBLUE
    #undef TFT_DIMGRAY
    #undef TFT_DODGERBLUE
    #undef TFT_FIREBRICK
    #undef TFT_FLORALWHITE
    #undef TFT_FORESTGREEN
    #undef TFT_GAINSBORO
    #undef TFT_GHOSTWHITE
    #undef TFT_GOLDENROD
    #undef TFT_HONEYDEW
    #undef TFT_HOTPINK
    #undef TFT_INDIANRED
    #undef TFT_INDIGO
    #undef TFT_IVORY
    #undef TFT_KHAKI
    #undef TFT_LAVENDER
    #undef TFT_LAVENDERBLUSH
    #undef TFT_LAWNGREEN
    #undef TFT_LEMONCHIFFON
    #undef TFT_LIGHTBLUE
    #undef TFT_LIGHTCORAL
    #undef TFT_LIGHTCYAN
    #undef TFT_LIGHTGOLDENRODYELLOW
    #undef TFT_LIGHTGREEN
    #undef TFT_LIGHTPINK
    #undef TFT_LIGHTSALMON
    #undef TFT_LIGHTSEAGREEN
    #undef TFT_LIGHTSKYBLUE
    #undef TFT_LIGHTSLATEGRAY
    #undef TFT_LIGHTSTEELBLUE
    #undef TFT_LIGHTYELLOW
    #undef TFT_LIMEGREEN
    #undef TFT_LINEN
    #undef TFT_MEDIUMAQUAMARINE
    #undef TFT_MEDIUMBLUE
    #undef TFT_MEDIUMORCHID
    #undef TFT_MEDIUMPURPLE
    #undef TFT_MEDIUMSEAGREEN
    #undef TFT_MEDIUMSLATEBLUE
    #undef TFT_MEDIUMSPRINGGREEN
    #undef TFT_MEDIUMTURQUOISE
    #undef TFT_MEDIUMVIOLETRED
    #undef TFT_MIDNIGHTBLUE
    #undef TFT_MINTCREAM
    #undef TFT_MISTYROSE
    #undef TFT_MOCCASIN
    #undef TFT_NAVAJOWHITE
    #undef TFT_OLDLACE
    #undef TFT_OLIVEDRAB
    #undef TFT_ORANGERED
    #undef TFT_ORCHID
    #undef TFT_PALEGOLDENROD
    #undef TFT_PALEGREEN
    #undef TFT_PALETURQUOISE
    #undef TFT_PALEVIOLETRED
    #undef TFT_PAPAYAWHIP
    #undef TFT_PEACHPUFF
    #undef TFT_PERU
    #undef TFT_PLUM
    #undef TFT_POWDERBLUE
    #undef TFT_REBECCAPURPLE
    #undef TFT_ROSYBROWN
    #undef TFT_ROYALBLUE
    #undef TFT_SADDLEBROWN
    #undef TFT_SALMON
    #undef TFT_SANDYBROWN
    #undef TFT_SEAGREEN
    #undef TFT_SEASHELL
    #undef TFT_SIENNA
    #undef TFT_SLATEBLUE
    #undef TFT_SLATEGRAY
    #undef TFT_SLATEGREY
    #undef TFT_GRAY
    #undef TFT_GREY
    #undef TFT_SNOW
    #undef TFT_SPRINGGREEN
    #undef TFT_STEELBLUE
    #undef TFT_TAN
    #undef TFT_THISTLE
    #undef TFT_TOMATO
    #undef TFT_TURQUOISE
    #undef TFT_WHEAT
    #undef TFT_WHITESMOKE
    #undef TFT_YELLOWGREEN
    #endif

    static constexpr int TFT_BLACK       = 0x0000;      /*   0,   0,   0 */
    static constexpr int TFT_NAVY        = 0x000F;      /*   0,   0, 128 */
    static constexpr int TFT_DARKGREEN   = 0x03E0;      /*   0, 128,   0 */
    static constexpr int TFT_DARKCYAN    = 0x03EF;      /*   0, 128, 128 */
    static constexpr int TFT_MAROON      = 0x7800;      /* 128,   0,   0 */
    static constexpr int TFT_PURPLE      = 0x780F;      /* 128,   0, 128 */
    static constexpr int TFT_OLIVE       = 0x7BE0;      /* 128, 128,   0 */
    static constexpr int TFT_LIGHTGREY   = 0xD69A;      /* 211, 211, 211 */
    static constexpr int TFT_LIGHTGRAY   = 0xD69A;      /* 211, 211, 211 */
    static constexpr int TFT_DARKGREY    = 0x7BEF;      /* 128, 128, 128 */
    static constexpr int TFT_DARKGRAY    = 0x7BEF;      /* 128, 128, 128 */
    static constexpr int TFT_BLUE        = 0x001F;      /*   0,   0, 255 */
    static constexpr int TFT_GREEN       = 0x07E0;      /*   0, 255,   0 */
    static constexpr int TFT_CYAN        = 0x07FF;      /*   0, 255, 255 */
    static constexpr int TFT_RED         = 0xF800;      /* 255,   0,   0 */
    static constexpr int TFT_MAGENTA     = 0xF81F;      /* 255,   0, 255 */
    static constexpr int TFT_YELLOW      = 0xFFE0;      /* 255, 255,   0 */
    static constexpr int TFT_WHITE       = 0xFFFF;      /* 255, 255, 255 */
    static constexpr int TFT_ORANGE      = 0xFDA0;      /* 255, 180,   0 */
    static constexpr int TFT_GREENYELLOW = 0xB7E0;      /* 180, 255,   0 */
    static constexpr int TFT_PINK        = 0xFE19;      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F
    static constexpr int TFT_BROWN       = 0x9A60;      /* 150,  75,   0 */
    static constexpr int TFT_GOLD        = 0xFEA0;      /* 255, 215,   0 */
    static constexpr int TFT_SILVER      = 0xC618;      /* 192, 192, 192 */
    static constexpr int TFT_SKYBLUE     = 0x867D;      /* 135, 206, 235 */
    static constexpr int TFT_VIOLET      = 0x915C;      /* 180,  46, 226 */
    static constexpr int TFT_TRANSPARENT = 0x0120;
    static constexpr int TFT_ALICEBLUE       = 0xF7DF;      /* 240, 248, 255 */
    static constexpr int TFT_ANTIQUEWHITE    = 0xFF5A;      /* 250, 235, 215 */
    static constexpr int TFT_AQUAMARINE      = 0x7FFA;      /* 127, 255, 212 */
    static constexpr int TFT_AZURE           = 0xF7FF;      /* 240, 255, 255 */
    static constexpr int TFT_BEIGE           = 0xF5F5;      /* 245, 245, 220 */
    static constexpr int TFT_BISQUE          = 0xFFB5;      /* 255, 228, 196 */
    static constexpr int TFT_BLANCHEDALMOND  = 0xFF7B;      /* 255, 235, 205 */
    static constexpr int TFT_BLUEVIOLET      = 0x901A;      /* 138,  43, 226 */
    static constexpr int TFT_BURLYWOOD       = 0xDEB8;      /* 222, 184, 135 */
    static constexpr int TFT_CADETBLUE       = 0x5F9E;      /*  95, 158, 160 */
    static constexpr int TFT_CHARTREUSE      = 0x7FFF;      /* 127, 255,   0 */
    static constexpr int TFT_CHOCOLATE       = 0xD269;      /* 210, 105,  30 */
    static constexpr int TFT_CORAL           = 0xFF7F;      /* 255, 127,  80 */
    static constexpr int TFT_CORNFLOWERBLUE  = 0x6495;      /* 100, 149, 237 */
    static constexpr int TFT_CORNSILK        = 0xFFDB;      /* 255, 248, 220 */
    static constexpr int TFT_CRIMSON         = 0xDC14;      /* 220,  20,  60 */
    static constexpr int TFT_DARKGOLDENROD   = 0xB886;      /* 184, 134,  11 */
    static constexpr int TFT_DARKKHAKI       = 0xBDB7;      /* 189, 183, 107 */
    static constexpr int TFT_DARKMAGENTA     = 0x8B00;      /* 139,   0, 139 */
    static constexpr int TFT_DARKOLIVEGREEN  = 0x556B;      /*  85, 107,  47 */
    static constexpr int TFT_DARKORANGE      = 0xFF8C;      /* 255, 140,   0 */
    static constexpr int TFT_DARKORCHID      = 0x9932;      /* 153,  50, 204 */
    static constexpr int TFT_DARKRED         = 0x8B00;      /* 139,   0,   0 */
    static constexpr int TFT_DARKSALMON      = 0xE996;      /* 233, 150, 122 */
    static constexpr int TFT_DARKSEAGREEN    = 0x8FBC;      /* 143, 188, 143 */
    static constexpr int TFT_DARKSLATEBLUE   = 0x483D;      /*  72,  61, 139 */
    static constexpr int TFT_DARKSLATEGRAY   = 0x2F4F;      /*  47,  79,  79 */
    static constexpr int TFT_DARKTURQUOISE   = 0x00CE;      /*   0, 206, 209 */
    static constexpr int TFT_DARKVIOLET      = 0x9400;      /* 148,   0, 211 */
    static constexpr int TFT_DEEPPINK        = 0xFF14;      /* 255,  20, 147 */
    static constexpr int TFT_DEEPSKYBLUE     = 0x00BF;      /*   0, 191, 255 */
    static constexpr int TFT_DIMGRAY         = 0x6969;      /* 105, 105, 105 */
    static constexpr int TFT_DODGERBLUE      = 0x1E90;      /*  30, 144, 255 */
    static constexpr int TFT_FIREBRICK       = 0xB222;      /* 178,  34,  34 */
    static constexpr int TFT_FLORALWHITE     = 0xFFFA;      /* 255, 250, 240 */
    static constexpr int TFT_FORESTGREEN     = 0x228B;      /*  34, 139,  34 */
    static constexpr int TFT_GAINSBORO       = 0xDCDc;      /* 220, 220, 220 */
    static constexpr int TFT_GHOSTWHITE      = 0xF8F8;      /* 248, 248, 255 */
    static constexpr int TFT_GOLDENROD       = 0xDAA5;      /* 218, 165,  32 */
    static constexpr int TFT_HONEYDEW        = 0xF0FF;      /* 240, 255, 240 */
    static constexpr int TFT_HOTPINK         = 0xFF69;      /* 255, 105, 180 */
    static constexpr int TFT_INDIANRED       = 0xCD5C;      /* 205,  92,  92 */
    static constexpr int TFT_INDIGO          = 0x4B00;      /*  75,   0, 132 */
    static constexpr int TFT_IVORY           = 0xFFFF;      /* 255, 255, 240 */
    static constexpr int TFT_KHAKI           = 0xF0E6;      /* 240, 230, 140 */
    static constexpr int TFT_LAVENDER        = 0xE6E6;      /* 230, 230, 250 */
    static constexpr int TFT_LAVENDERBLUSH   = 0xFFF0;      /* 255, 240, 245 */
    static constexpr int TFT_LAWNGREEN       = 0x7CFC;      /* 124, 252,   0 */
    static constexpr int TFT_LEMONCHIFFON    = 0xFFAC;      /* 255, 250, 205 */
    static constexpr int TFT_LIGHTBLUE       = 0xADD8;      /* 173, 216, 230 */
    static constexpr int TFT_LIGHTCORAL      = 0xF080;      /* 240, 128, 128 */
    static constexpr int TFT_LIGHTCYAN       = 0xE0FF;      /* 224, 255, 255 */
    static constexpr int TFT_LIGHTGOLDENRODYELLOW = 0xFAF0; /* 250, 250, 210 */
    static constexpr int TFT_LIGHTGREEN      = 0x90EE;      /* 144, 238, 144 */
    static constexpr int TFT_LIGHTPINK       = 0xFFB6;      /* 255, 182, 193 */
    static constexpr int TFT_LIGHTSALMON     = 0xFFA0;      /* 255, 160, 122 */
    static constexpr int TFT_LIGHTSEAGREEN   = 0x20B2;      /*  32, 178, 170 */
    static constexpr int TFT_LIGHTSKYBLUE    = 0x87CE;      /* 135, 206, 250 */
    static constexpr int TFT_LIGHTSLATEGRAY  = 0x7788;      /* 119, 136, 153 */
    static constexpr int TFT_LIGHTSTEELBLUE  = 0xB0C4;      /* 176, 196, 222 */
    static constexpr int TFT_LIGHTYELLOW     = 0xFFFF;      /* 255, 255, 224 */
    static constexpr int TFT_LIMEGREEN       = 0x32CD;      /*  50, 205,  50 */
    static constexpr int TFT_LINEN           = 0xFAF0;      /* 250, 240, 230 */
    static constexpr int TFT_MEDIUMAQUAMARINE = 0x66CD;     /* 102, 205, 170 */
    static constexpr int TFT_MEDIUMBLUE      = 0x000C;      /*   0,   0, 205 */
    static constexpr int TFT_MEDIUMORCHID    = 0xBA55;      /* 186,  85, 213 */
    static constexpr int TFT_MEDIUMPURPLE    = 0x9370;      /* 147, 112, 219 */
    static constexpr int TFT_MEDIUMSEAGREEN  = 0x3CB3;      /*  60, 179, 113 */
    static constexpr int TFT_MEDIUMSLATEBLUE = 0x7B68;      /* 123, 104, 238 */
    static constexpr int TFT_MEDIUMSPRINGGREEN = 0x00FA;   /*   0, 250, 154 */
    static constexpr int TFT_MEDIUMTURQUOISE = 0x48D1;      /*  72, 209, 204 */
    static constexpr int TFT_MEDIUMVIOLETRED = 0xC715;      /* 199,  21, 133 */
    static constexpr int TFT_MIDNIGHTBLUE    = 0x1919;      /*  25,  25, 112 */
    static constexpr int TFT_MINTCREAM       = 0xF5FF;      /* 245, 255, 250 */
    static constexpr int TFT_MISTYROSE       = 0xFFE4;      /* 255, 228, 225 */
    static constexpr int TFT_MOCCASIN        = 0xFFE4;      /* 255, 228, 181 */
    static constexpr int TFT_NAVAJOWHITE     = 0xFFDE;      /* 255, 222, 173 */
    static constexpr int TFT_OLDLACE         = 0xFDF5;      /* 253, 245, 230 */
    static constexpr int TFT_OLIVEDRAB       = 0x6B8E;      /* 107, 142,  35 */
    static constexpr int TFT_ORANGERED       = 0xFF45;      /* 255,  69,   0 */
    static constexpr int TFT_ORCHID          = 0xDA70;      /* 218, 112, 214 */
    static constexpr int TFT_PALEGOLDENROD   = 0xEEE8;      /* 238, 232, 170 */
    static constexpr int TFT_PALEGREEN       = 0x98FB;      /* 152, 251, 152 */
    static constexpr int TFT_PALETURQUOISE   = 0xAFEE;      /* 175, 238, 238 */
    static constexpr int TFT_PALEVIOLETRED   = 0xDB70;      /* 219, 112, 147 */
    static constexpr int TFT_PAPAYAWHIP      = 0xFFEF;      /* 255, 239, 213 */
    static constexpr int TFT_PEACHPUFF       = 0xFFDA;      /* 255, 218, 185 */
    static constexpr int TFT_PERU            = 0xCD85;      /* 205, 133,  63 */
    static constexpr int TFT_PLUM            = 0xDDA0;      /* 221, 160, 221 */
    static constexpr int TFT_POWDERBLUE      = 0xB0E0;      /* 176, 224, 230 */
    static constexpr int TFT_REBECCAPURPLE   = 0x6633;      /* 102,  51, 153 */
    static constexpr int TFT_ROSYBROWN       = 0xBC8F;      /* 188, 143, 143 */
    static constexpr int TFT_ROYALBLUE       = 0x4169;      /*  65, 105, 225 */
    static constexpr int TFT_SADDLEBROWN     = 0x8B45;      /* 139,  69,  19 */
    static constexpr int TFT_SALMON          = 0xFA80;      /* 250, 128, 114 */
    static constexpr int TFT_SANDYBROWN      = 0xF4A4;      /* 244, 164,  96 */
    static constexpr int TFT_SEAGREEN        = 0x2E8B;      /*  46, 139,  87 */
    static constexpr int TFT_SEASHELL        = 0xFFF5;      /* 255, 245, 238 */
    static constexpr int TFT_SIENNA          = 0xA052;      /* 160,  82,  45 */
    static constexpr int TFT_SLATEBLUE       = 0x6A5A;      /* 106,  90, 205 */
    static constexpr int TFT_SLATEGRAY       = 0x7080;      /* 112, 128, 144 */
    static constexpr int TFT_SLATEGREY       = 0x7080;      /* 112, 128, 144 */
    static constexpr int TFT_GRAY            = 0x8410;      /* 128, 128, 128 */
    static constexpr int TFT_GREY            = 0x8410;      /* 128, 128, 128 */
    static constexpr int TFT_SNOW            = 0xFFFA;      /* 255, 250, 250 */
    static constexpr int TFT_SPRINGGREEN     = 0x00FF;      /*   0, 255, 127 */
    static constexpr int TFT_STEELBLUE       = 0x4682;      /*  70, 130, 180 */
    static constexpr int TFT_TAN             = 0xD2B4;      /* 210, 180, 140 */
    static constexpr int TFT_THISTLE         = 0xD8BF;      /* 216, 191, 216 */
    static constexpr int TFT_TOMATO          = 0xFF63;      /* 255,  99,  71 */
    static constexpr int TFT_TURQUOISE       = 0x40E0;      /*  64, 224, 208 */
    static constexpr int TFT_WHEAT           = 0xF5DE;      /* 245, 222, 179 */
    static constexpr int TFT_WHITESMOKE      = 0xF5F5;      /* 245, 245, 245 */
    static constexpr int TFT_YELLOWGREEN     = 0x9ACD;      /* 154, 205,  50 */
  }

//----------------------------------------------------------------------------

  namespace gradient_fill_styles
  {
    enum fill_style_t
    {
      horizontal_linear = 0,
      vertical_linear   = 1,
      radial_center     = 2
    };
    static constexpr const fill_style_t HLINEAR = fill_style_t::horizontal_linear;
    static constexpr const fill_style_t VLINEAR = fill_style_t::vertical_linear;
    static constexpr const fill_style_t RADIAL  = fill_style_t::radial_center;
  }
  using namespace gradient_fill_styles;

//----------------------------------------------------------------------------

  namespace textdatum
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

    #ifdef TL_DATUM
    #undef TL_DATUM
    #undef TC_DATUM
    #undef TR_DATUM
    #undef ML_DATUM
    #undef CL_DATUM
    #undef MC_DATUM
    #undef CC_DATUM
    #undef MR_DATUM
    #undef CR_DATUM
    #undef BL_DATUM
    #undef BC_DATUM
    #undef BR_DATUM
    #undef L_BASELINE
    #undef C_BASELINE
    #undef R_BASELINE
    #endif

    static constexpr textdatum_t TL_DATUM   = textdatum_t::top_left;
    static constexpr textdatum_t TC_DATUM   = textdatum_t::top_center;
    static constexpr textdatum_t TR_DATUM   = textdatum_t::top_right;
    static constexpr textdatum_t ML_DATUM   = textdatum_t::middle_left;
    static constexpr textdatum_t CL_DATUM   = textdatum_t::middle_left;
    static constexpr textdatum_t MC_DATUM   = textdatum_t::middle_center;
    static constexpr textdatum_t CC_DATUM   = textdatum_t::middle_center;
    static constexpr textdatum_t MR_DATUM   = textdatum_t::middle_right;
    static constexpr textdatum_t CR_DATUM   = textdatum_t::middle_right;
    static constexpr textdatum_t BL_DATUM   = textdatum_t::bottom_left;
    static constexpr textdatum_t BC_DATUM   = textdatum_t::bottom_center;
    static constexpr textdatum_t BR_DATUM   = textdatum_t::bottom_right;
    static constexpr textdatum_t L_BASELINE = textdatum_t::baseline_left;
    static constexpr textdatum_t C_BASELINE = textdatum_t::baseline_center;
    static constexpr textdatum_t R_BASELINE = textdatum_t::baseline_right;
  };
  using namespace textdatum;

//----------------------------------------------------------------------------

  namespace datum
  {
    using datum_t = textdatum::textdatum_t;
  }
  using namespace datum;

//----------------------------------------------------------------------------

  namespace attribute
  {
    enum attribute_t
    { cp437_switch = 1
    , utf8_switch  = 2
    , epd_mode_switch = 4
    };

    #ifdef CP437_SWITCH
    #undef CP437_SWITCH
    #undef UTF8_SWITCH
//  #undef PSRAM_ENABLE
    #endif

    static constexpr attribute_t CP437_SWITCH = attribute_t::cp437_switch;
    static constexpr attribute_t UTF8_SWITCH  = attribute_t::utf8_switch;
  }
  using namespace attribute;

//----------------------------------------------------------------------------

  enum color_depth_t : uint16_t
  {
    bit_mask     = 0x00FF   , /// ビット数取得用マスク値 (下位1Byteはビット数表現専用とする。変更しないこと);
    has_palette  = 0x0800   , /// パレット値として扱う;
    nonswapped   = 0x0100   , /// バイトスワップしていない値;
    alternate    = 0x1000   , /// ビット数が同一な色表現が複数ある場合の相違表現用;

    grayscale_1bit      =   1                         , //                            _______L
    grayscale_2bit      =   2                         , //                            ______LL
    grayscale_4bit      =   4                         , //                            ____LLLL
    grayscale_8bit      =   8 |              alternate, // ________ ________ ________ LLLLLLLL
    palette_1bit        =   1 | has_palette           , //                            _______I   2 color
    palette_2bit        =   2 | has_palette           , //                            ______II   4 color
    palette_4bit        =   4 | has_palette           , //                            ____IIII  16 color
    palette_8bit        =   8 | has_palette           , //                            IIIIIIII 256 color
    rgb332_1Byte        =   8                         , // ________ ________ ________ RRRGGGBB
    rgb565_2Byte        =  16                         , // ________ ________ GGGBBBBB RRRRRGGG
    rgb666_3Byte        =  24 |              alternate, // ________ __BBBBBB __GGGGGG __RRRRRR
    rgb888_3Byte        =  24                         , // ________ BBBBBBBB GGGGGGGG RRRRRRRR
    argb8888_4Byte      =  32                         , // BBBBBBBB GGGGGGGG RRRRRRRR AAAAAAAA
    rgb565_nonswapped   =  16 | nonswapped            , // ________ ________ RRRRRGGG GGGBBBBB
    rgb666_nonswapped   =  24 | nonswapped | alternate, // ________ __RRRRRR __GGGGGG __BBBBBB
    rgb888_nonswapped   =  24 | nonswapped            , // ________ RRRRRRRR GGGGGGGG BBBBBBBB
    argb8888_nonswapped =  32 | nonswapped            , // AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
  };

//----------------------------------------------------------------------------

  enum error_t
  { unknown_err
  , invalid_arg         // 引数が範囲外等でinvalidな場合のエラー;
  , connection_lost     // 通信が切断されたり正しく行えない場合のエラー;
  , mode_mismatch       // I2C通信が書込みモード時に読込みを指示するなど不一致な操作を行った場合のエラー;
  , periph_device_err   // ペリフェラルが動作していない等のエラー;
  };

//----------------------------------------------------------------------------
 }
}

using namespace lgfx::jpeg_div;
using namespace lgfx::colors;
using namespace lgfx::textdatum;
using namespace lgfx::datum;
using namespace lgfx::attribute;
using namespace lgfx::epd_mode;
