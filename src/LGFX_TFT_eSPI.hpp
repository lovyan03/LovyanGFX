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


  This is a wrapper to make LovyanGFX behave like TFT_eSPI.
/----------------------------------------------------------------------------*/

#ifndef LGFX_TFT_ESPI_HPP_
#define LGFX_TFT_ESPI_HPP_

#include "LovyanGFX.hpp"

using TFT_eSPI = LGFX;

class TFT_eSprite : public LGFX_Sprite {
public:
  TFT_eSprite() : LGFX_Sprite() { _psram = true; }
  TFT_eSprite(LovyanGFX* parent) : LGFX_Sprite(parent) { _psram = true; }

  void* frameBuffer(uint8_t) { return getBuffer(); }
};

namespace lgfx
{
//----------------------------------------------------------------------------

  namespace ili9341_colors  // Color definitions for backwards compatibility with old sketches
  {
    #ifdef ILI9341_BLACK
    #undef ILI9341_BLACK
    #undef ILI9341_NAVY
    #undef ILI9341_DARKGREEN
    #undef ILI9341_DARKCYAN
    #undef ILI9341_MAROON
    #undef ILI9341_PURPLE
    #undef ILI9341_OLIVE
    #undef ILI9341_LIGHTGREY
    #undef ILI9341_DARKGREY
    #undef ILI9341_BLUE
    #undef ILI9341_GREEN
    #undef ILI9341_CYAN
    #undef ILI9341_RED
    #undef ILI9341_MAGENTA
    #undef ILI9341_YELLOW
    #undef ILI9341_WHITE
    #undef ILI9341_ORANGE
    #undef ILI9341_GREENYELLOW
    #undef ILI9341_PINK
    #endif

    #ifdef BLACK
    #undef BLACK
    #undef NAVY
    #undef DARKGREEN
    #undef DARKCYAN
    #undef MAROON
    #undef PURPLE
    #undef OLIVE
    #undef LIGHTGREY
    #undef DARKGREY
    #undef BLUE
    #undef GREEN
    #undef CYAN
    #undef RED
    #undef MAGENTA
    #undef YELLOW
    #undef WHITE
    #undef ORANGE
    #undef GREENYELLOW
    #undef PINK
    #endif

    static constexpr int ILI9341_BLACK       = 0x0000;      /*   0,   0,   0 */
    static constexpr int ILI9341_NAVY        = 0x000F;      /*   0,   0, 128 */
    static constexpr int ILI9341_DARKGREEN   = 0x03E0;      /*   0, 128,   0 */
    static constexpr int ILI9341_DARKCYAN    = 0x03EF;      /*   0, 128, 128 */
    static constexpr int ILI9341_MAROON      = 0x7800;      /* 128,   0,   0 */
    static constexpr int ILI9341_PURPLE      = 0x780F;      /* 128,   0, 128 */
    static constexpr int ILI9341_OLIVE       = 0x7BE0;      /* 128, 128,   0 */
    static constexpr int ILI9341_LIGHTGREY   = 0xC618;      /* 192, 192, 192 */
    static constexpr int ILI9341_DARKGREY    = 0x7BEF;      /* 128, 128, 128 */
    static constexpr int ILI9341_BLUE        = 0x001F;      /*   0,   0, 255 */
    static constexpr int ILI9341_GREEN       = 0x07E0;      /*   0, 255,   0 */
    static constexpr int ILI9341_CYAN        = 0x07FF;      /*   0, 255, 255 */
    static constexpr int ILI9341_RED         = 0xF800;      /* 255,   0,   0 */
    static constexpr int ILI9341_MAGENTA     = 0xF81F;      /* 255,   0, 255 */
    static constexpr int ILI9341_YELLOW      = 0xFFE0;      /* 255, 255,   0 */
    static constexpr int ILI9341_WHITE       = 0xFFFF;      /* 255, 255, 255 */
    static constexpr int ILI9341_ORANGE      = 0xFD20;      /* 255, 165,   0 */
    static constexpr int ILI9341_GREENYELLOW = 0xAFE5;      /* 173, 255,  47 */
    static constexpr int ILI9341_PINK        = 0xF81F;

    static constexpr int BLACK           = 0x0000;      /*   0,   0,   0 */
    static constexpr int NAVY            = 0x000F;      /*   0,   0, 128 */
    static constexpr int DARKGREEN       = 0x03E0;      /*   0, 128,   0 */
    static constexpr int DARKCYAN        = 0x03EF;      /*   0, 128, 128 */
    static constexpr int MAROON          = 0x7800;      /* 128,   0,   0 */
    static constexpr int PURPLE          = 0x780F;      /* 128,   0, 128 */
    static constexpr int OLIVE           = 0x7BE0;      /* 128, 128,   0 */
    static constexpr int LIGHTGREY       = 0xC618;      /* 192, 192, 192 */
    static constexpr int DARKGREY        = 0x7BEF;      /* 128, 128, 128 */
    static constexpr int BLUE            = 0x001F;      /*   0,   0, 255 */
    static constexpr int GREEN           = 0x07E0;      /*   0, 255,   0 */
    static constexpr int CYAN            = 0x07FF;      /*   0, 255, 255 */
    static constexpr int RED             = 0xF800;      /* 255,   0,   0 */
    static constexpr int MAGENTA         = 0xF81F;      /* 255,   0, 255 */
    static constexpr int YELLOW          = 0xFFE0;      /* 255, 255,   0 */
    static constexpr int WHITE           = 0xFFFF;      /* 255, 255, 255 */
    static constexpr int ORANGE          = 0xFD20;      /* 255, 165,   0 */
    static constexpr int GREENYELLOW     = 0xAFE5;      /* 173, 255,  47 */
    static constexpr int PINK            = 0xF81F;
  }

  namespace tft_command
  {
    static constexpr int TFT_DISPOFF = 0x28;
    static constexpr int TFT_DISPON  = 0x29;
    static constexpr int TFT_SLPIN   = 0x10;
    static constexpr int TFT_SLPOUT  = 0x11;
  }

//----------------------------------------------------------------------------
}

using namespace lgfx::textdatum;
using namespace lgfx::attribute;
using namespace lgfx::ili9341_colors;
using namespace lgfx::tft_command;


#endif
