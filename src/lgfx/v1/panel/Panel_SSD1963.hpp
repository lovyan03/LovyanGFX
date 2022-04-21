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
/----------------------------------------------------------------------------*/
#pragma once

#include "Panel_LCD.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_SSD1963 : public Panel_LCD
  {
    enum panel_data_width_t : uint8_t
    {
      is18bit = 0x00,
      is18bit_dithering = 0x10,
      is18bit_FRC = 0x18,
      is24bit = 0x20,
    };

    struct timing_params_t
    {
      uint32_t xtal_clock = 10000000;  /// Oscillator frequency on board;
      uint32_t pll_clock = 120000000;
      uint8_t refresh_rate = 60;

      /// HSYNC parameter
      uint16_t h_branking_total = 128;
      uint8_t  hpw = 48;
      uint16_t hps = 46;
      uint16_t lps = 15;
      uint8_t  lpspp = 0;

      /// VSYNC parameter
      uint16_t v_branking_total = 45;
      uint8_t  vpw = 16;
      uint16_t vps = 16;
      uint16_t fps = 8;

      /// Command 0xB0 Parameter1: TFT Panel data width (18bit or 24bit)
      panel_data_width_t data_width = is24bit;
    };

    Panel_SSD1963(void)
    {
      _cfg.memory_width  = _cfg.panel_width  = 864;
      _cfg.memory_height = _cfg.panel_height = 480;
      _write_depth = rgb565_2Byte;
      _read_depth  = rgb565_2Byte;
    }

    void setHSync(uint_fast16_t front, uint_fast16_t sync, uint_fast16_t back, uint_fast16_t move = 0, uint_fast16_t lpspp = 0);
    void setVSync(uint_fast16_t front, uint_fast16_t sync, uint_fast16_t back, uint_fast16_t move = 0);

    bool init(bool use_reset) override;
    void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override;

    void setRotation(uint_fast8_t r) override;
    color_depth_t setColorDepth(color_depth_t depth) override;

    void setPanelDataWidth(panel_data_width_t panel_data_width) { _timing_params.data_width = panel_data_width; }

    const timing_params_t& config_timing_params(void) const { return _timing_params; }
    void config_timing_params(const timing_params_t& timing_params);

  protected:

    timing_params_t _timing_params;
  };

//----------------------------------------------------------------------------
 }
}
