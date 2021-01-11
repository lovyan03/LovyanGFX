/*----------------------------------------------------------------------------/
  Lovyan GFX library - LCD graphics library .
  
  support platform:
    ESP32 (SPI/I2S) with Arduino/ESP-IDF
    ATSAMD51 (SPI) with Arduino
  
  This is a wrapper to make LovyanGFX behave like TFT_eSPI.
/----------------------------------------------------------------------------*/

#ifndef LGFX_TFT_ESPI_HPP_
#define LGFX_TFT_ESPI_HPP_

#include "LovyanGFX.hpp"

typedef LGFX TFT_eSPI;

class TFT_eSprite : public lgfx::LGFX_Sprite {
public:
  TFT_eSprite() : LGFX_Sprite() {}
  TFT_eSprite(LovyanGFX* parent) : LGFX_Sprite(parent) {}

  void* frameBuffer(uint8_t) { return getBuffer(); }
};

namespace lgfx
{
  namespace textdatum
  {
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

  namespace attribute
  {
    static constexpr attribute_t CP437_SWITCH = attribute_t::cp437_switch;
    static constexpr attribute_t UTF8_SWITCH  = attribute_t::utf8_switch;
  }

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

    [[deprecated("use TFT_BLACK"      )]] static constexpr int ILI9341_BLACK       = 0x0000;      /*   0,   0,   0 */
    [[deprecated("use TFT_NAVY"       )]] static constexpr int ILI9341_NAVY        = 0x000F;      /*   0,   0, 128 */
    [[deprecated("use TFT_DARKGREEN"  )]] static constexpr int ILI9341_DARKGREEN   = 0x03E0;      /*   0, 128,   0 */
    [[deprecated("use TFT_DARKCYAN"   )]] static constexpr int ILI9341_DARKCYAN    = 0x03EF;      /*   0, 128, 128 */
    [[deprecated("use TFT_MAROON"     )]] static constexpr int ILI9341_MAROON      = 0x7800;      /* 128,   0,   0 */
    [[deprecated("use TFT_PURPLE"     )]] static constexpr int ILI9341_PURPLE      = 0x780F;      /* 128,   0, 128 */
    [[deprecated("use TFT_OLIVE"      )]] static constexpr int ILI9341_OLIVE       = 0x7BE0;      /* 128, 128,   0 */
    [[deprecated("use TFT_LIGHTGREY"  )]] static constexpr int ILI9341_LIGHTGREY   = 0xC618;      /* 192, 192, 192 */
    [[deprecated("use TFT_DARKGREY"   )]] static constexpr int ILI9341_DARKGREY    = 0x7BEF;      /* 128, 128, 128 */
    [[deprecated("use TFT_BLUE"       )]] static constexpr int ILI9341_BLUE        = 0x001F;      /*   0,   0, 255 */
    [[deprecated("use TFT_GREEN"      )]] static constexpr int ILI9341_GREEN       = 0x07E0;      /*   0, 255,   0 */
    [[deprecated("use TFT_CYAN"       )]] static constexpr int ILI9341_CYAN        = 0x07FF;      /*   0, 255, 255 */
    [[deprecated("use TFT_RED"        )]] static constexpr int ILI9341_RED         = 0xF800;      /* 255,   0,   0 */
    [[deprecated("use TFT_MAGENTA"    )]] static constexpr int ILI9341_MAGENTA     = 0xF81F;      /* 255,   0, 255 */
    [[deprecated("use TFT_YELLOW"     )]] static constexpr int ILI9341_YELLOW      = 0xFFE0;      /* 255, 255,   0 */
    [[deprecated("use TFT_WHITE"      )]] static constexpr int ILI9341_WHITE       = 0xFFFF;      /* 255, 255, 255 */
    [[deprecated("use TFT_ORANGE"     )]] static constexpr int ILI9341_ORANGE      = 0xFD20;      /* 255, 165,   0 */
    [[deprecated("use TFT_GREENYELLOW")]] static constexpr int ILI9341_GREENYELLOW = 0xAFE5;      /* 173, 255,  47 */
    [[deprecated("use TFT_PINK"       )]] static constexpr int ILI9341_PINK        = 0xF81F;
  }

  namespace tft_command
  {
    static constexpr int TFT_DISPOFF = 0x28;
    static constexpr int TFT_DISPON  = 0x29;
    static constexpr int TFT_SLPIN   = 0x10;
    static constexpr int TFT_SLPOUT  = 0x11;
  }
}

using namespace lgfx::textdatum;
using namespace lgfx::attribute;
using namespace lgfx::ili9341_colors;
using namespace lgfx::tft_command;


#endif
