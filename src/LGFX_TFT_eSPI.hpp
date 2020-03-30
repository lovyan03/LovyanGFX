/*----------------------------------------------------------------------------/
  Lovyan GFX library - ESP32 hardware SPI graphics library .

  This is a wrapper to make LovyanGFX behave like TFT_eSPI.
/----------------------------------------------------------------------------*/

#ifndef LGFX_TFT_ESPI_HPP_
#define LGFX_TFT_ESPI_HPP_

#include <LovyanGFX.hpp>


typedef lgfx::bgr888_t RGBColor;


// Font datum enumeration
// LEFT=0   CENTER=1   RIGHT=2
// TOP=0   MIDDLE=4   BOTTOM=8   BASELINE=16

#define TL_DATUM 0 // Top left (default)
#define TC_DATUM 1 // Top centre
#define TR_DATUM 2 // Top right
#define ML_DATUM 4 // Middle left
#define CL_DATUM 4 // Centre left, same as above
#define MC_DATUM 5 // Middle centre
#define CC_DATUM 5 // Centre centre, same as above
#define MR_DATUM 6 // Middle right
#define CR_DATUM 6 // Centre right, same as above
#define BL_DATUM 8 // Bottom left
#define BC_DATUM 9// Bottom centre
#define BR_DATUM 10 // Bottom right
#define L_BASELINE 16 // Left character baseline (Line the 'A' character would sit on)
#define C_BASELINE 17 // Centre character baseline
#define R_BASELINE 18 // Right character baseline


// Colour enumeration

// Default color definitions
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F      
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SILVER      0xC618      /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */

#define TFT_TRANSPARENT 0x0120



class TFT_eSPI : public lgfx::LGFX_SPI<lgfx::LGFX_Config> {};

class TFT_eSprite : public lgfx::LGFX_Sprite {
public:
  TFT_eSprite() : LGFX_Sprite() {}
  TFT_eSprite(LovyanGFX* parent) : LGFX_Sprite(parent) {}

  void* frameBuffer(uint8_t dummy) { return buffer(); }
};

#endif
