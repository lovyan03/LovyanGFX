/*----------------------------------------------------------------------------/
  Lovyan GFX library - ESP32 hardware SPI graphics library .

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

using namespace textdatum;
using namespace attribute;


#endif
