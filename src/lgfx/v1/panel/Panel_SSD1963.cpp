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
#include "Panel_SSD1963.hpp"

#include "../../internal/memory.h"
#include "../Bus.hpp"
#include "../misc/colortype.hpp"

#include <stdlib.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Panel_SSD1963::config_timing_params(const timing_params_t& timing_params)
  {
    _timing_params = timing_params;
  }

  bool Panel_SSD1963::init(bool use_reset)
  {
    if (!Panel_LCD::init(use_reset)) return false;

/*
 PLLクロック算出方法;
 XTAL_CLOCK = ボードに搭載されているクロックオシレータの周波数 (デフォルトとして10MHzを設定。)
 VCO  = XTAL_CLOCK * ( M + 1 )
 PLL  = VCO / ( N + 1 )
 ただし、250MHz < VCO  < 800MHz であること。（つまり Mは 25 ～ 78 であること）;
 XTAL_CLOCK 10MHz で PLLを 100MHzに設定したい場合
 M = 29 , N = 2 の場合、 VCO = 10*(29+1) == 300MHz、 PLL = 300/(2+1) == 100MHz となる。;
 M = 39 , N = 3 の場合、 VCO = 10*(39+1) == 400MHz、 PLL = 400/(3+1) == 100MHz となる。;
*/
    auto xtal = _timing_params.xtal_clock;
    auto target_pll = _timing_params.pll_clock;
    uint32_t m = 250000000 / xtal;  /// VCO > 250MHz limit
    uint32_t vco = xtal * (m + 1);

    uint32_t save_m = m, save_n = 0;
    uint32_t save_diff = ~0u;
    uint32_t n = vco / (target_pll + 1);
    for (;;)
    {
      uint32_t pll = vco / (n + 1);
      uint32_t diff = abs((int32_t)target_pll - (int32_t)pll);
      if (save_diff > diff)
      {
        save_diff = diff;
        save_m = m;
        save_n = n;
        if (diff == 0) { break; }
      }

      if (target_pll < pll)
      {
        if (++n > 31) { break; }
      }
      else
      {
        if (++m > 255) { break; }
        vco = xtal * (m + 1);
        if (vco >= 800000000) { break; } /// VCO < 800MHz limit
      }
    }
    uint32_t pll_clock = (xtal * (save_m + 1)) / (save_n + 1);

    uint_fast16_t hori = _cfg.panel_width  - 1;
    uint_fast16_t vert = _cfg.panel_height - 1;
    uint_fast16_t h_total = hori + _timing_params.h_branking_total;
    uint_fast16_t v_total = vert + _timing_params.v_branking_total;

    uint32_t lcdc_fpr = (h_total << 20) / (pll_clock / (_timing_params.refresh_rate * v_total));

// ESP_LOGD("DEBUG","XTAL:%d M:%d N:%d PLL:%d", _xtal_clock, _clock_m, _clock_n, _pll_clock);
// ESP_LOGD("DEBUG","LCDC_FPR:%d", lcdc_fpr);

    uint8_t cmds[] =
    { 0xE2, 3, (uint8_t)save_m         // set pll clock
             , (uint8_t)save_n
             , 0x54
    , 0xE0, 1+CMD_INIT_DELAY, 0x01, 1  // PLL enable
    , 0xE0, 1               , 0x03
    , 0x01, 0+CMD_INIT_DELAY, 10       // software reset

    , 0xE6, 3, (uint8_t)(lcdc_fpr >> 16)  // Pixel Clock
             , (uint8_t)(lcdc_fpr >>  8)
             , (uint8_t)(lcdc_fpr      )

    , 0xB0, 7, _timing_params.data_width, 0x00             // Panel Resolution
             , (uint8_t)(hori >> 8), (uint8_t)hori
             , (uint8_t)(vert >> 8), (uint8_t)vert
             , 0x2D

    , 0xB4, 8, (uint8_t)(h_total >> 8), (uint8_t)h_total  // HSYNC
             , (uint8_t)(_timing_params.hps >> 8), (uint8_t)_timing_params.hps
             , (uint8_t) _timing_params.hpw
             , (uint8_t)(_timing_params.lps >> 8), (uint8_t)_timing_params.lps
             , (uint8_t)_timing_params.lpspp

    , 0xB6, 7, (uint8_t)(v_total >> 8), (uint8_t)v_total  // VSYNC
             , (uint8_t)(_timing_params.vps >> 8), (uint8_t)_timing_params.vps
             , (uint8_t) _timing_params.vpw
             , (uint8_t)(_timing_params.fps >> 8), (uint8_t)_timing_params.fps

    , 0xFF, 0xFF
    };

    static constexpr const uint8_t fixed_cmds[] =
    {
      0xBA, 1, 0x0F,    //GPIO[3:0] out 1
      0xB8, 2, 0x07, 0x01,      //GPIO3=input, GPIO[2:0]=output  GPIO0 normal
      0xB8, 2, 0x0F, 0x01,    //GPIO is controlled by host GPIO[3:0]=output   GPIO[0]=1  LCD ON  GPIO[0]=1  LCD OFF GPIO0 normal
      0xBA, 1, 0x01,     //GPIO[0] out 1 --- LCD display on/off control PIN
      0x29, 0,           //display on
      0xBE, 6, 0x06, 0xf0, 0x01, 0xf0, 0x00, 0x00,   //set PWM for B/L
      0xD0, 1, 0x0D,
      0xFF, 0xFF
    };

    startWrite();
    command_list(cmds);
    command_list(fixed_cmds);
    endWrite();

    return true;
  }

  color_depth_t Panel_SSD1963::setColorDepth(color_depth_t depth)
  {
    uint8_t mode = 0x00;
    if (_cfg.dlen_16bit)
    {
      if (((int)depth & color_depth_t::bit_mask) > 16)
      {
        depth = rgb888_3Byte;
        mode = 0x02;
      }
      else
      {
        depth = rgb565_2Byte;
        mode = 0x03;
      }
    }
    else
    {
      depth = rgb888_3Byte;
    }
    _write_depth = depth;
    _read_depth = _write_depth;

    uint8_t cmds[] =
    { 0xF0, 1, mode
    , 0xFF, 0xFF
    };

    startWrite();
    command_list(cmds);
    endWrite();

    return _write_depth;
  }

  void Panel_SSD1963::setHSync(uint_fast16_t front, uint_fast16_t sync, uint_fast16_t back, uint_fast16_t move, uint_fast16_t lpspp)
  {
    uint_fast16_t ht = _cfg.panel_width + front + sync + back - 1;
    uint_fast16_t hpw = sync - 1;
    uint_fast16_t hps = move + sync + back;
    uint_fast16_t lps = move;

    uint8_t cmds[] =
    { 0xB4, 8, (uint8_t)(ht  >> 8), (uint8_t)ht
             , (uint8_t)(hps >> 8), (uint8_t)hps
             , (uint8_t)hpw
             , (uint8_t)(lps >> 8), (uint8_t)lps
             , (uint8_t)lpspp
    , 0xFF, 0xFF
    };

    startWrite();
    command_list(cmds);
    endWrite();
  }

  void Panel_SSD1963::setVSync(uint_fast16_t front, uint_fast16_t sync, uint_fast16_t back, uint_fast16_t move)
  {
    uint_fast16_t vt = _cfg.panel_height + front + sync + back - 1;
    uint_fast16_t vpw = sync - 1;
    uint_fast16_t vps = move + sync + back;
    uint_fast16_t fps = move;

    uint8_t cmds[] =
    { 0xB6, 7, (uint8_t)(vt  >> 8), (uint8_t)vt
             , (uint8_t)(vps >> 8), (uint8_t)vps
             , (uint8_t)vpw
             , (uint8_t)(fps >> 8), (uint8_t)fps
    , 0xFF, 0xFF
    };

    startWrite();
    command_list(cmds);
    endWrite();
  }

  void Panel_SSD1963::setRotation(uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

    _width  = _cfg.panel_width;
    _height = _cfg.panel_height;
    if (_internal_rotation & 1) std::swap(_width, _height);

    _colstart = _cfg.offset_x;
    _rowstart = _cfg.offset_y;

    _xs = _xe = _ys = _ye = INT16_MAX;

    update_madctl();
  }

  void Panel_SSD1963::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
  {
    bool dlen_16bit = _cfg.dlen_16bit;
    if (dlen_16bit && _has_align_data)
    {
      _bus->writeData(0, 8);
      _has_align_data = false;
    }

    uint_fast8_t rb = 1u << _internal_rotation;
    if (_internal_rotation & 1)
    {
      std::swap(xs, ys);
      std::swap(xe, ye);
    }

    if (xs != _xs || xe != _xe)
    {
      _xs = xs;
      _xe = xe;
      if (rb & 0b11000110) // case 1:2:6:7:
      {
        std::swap(xs, xe);
        xs = _cfg.panel_width - xs - 1;
        xe = _cfg.panel_width - xe - 1;
      }
      xs += _colstart;
      xe += _colstart;
      if (dlen_16bit)
      {
        _bus->writeCommand(CMD_CASET << 8, 16);
        _bus->writeData((xs >> 8) << 8 | xs << 24, 32);
        _bus->writeData((xe >> 8) << 8 | xe << 24, 32);
      }
      else
      {
        _bus->writeCommand(CMD_CASET, 8);
        _bus->writeData(xs >> 8 | (xs & 0xFF) << 8 | (xe << 8 | xe >> 8) << 16, 32);
      }
    }
    if (ys != _ys || ye != _ye)
    {
      _ys = ys;
      _ye = ye;
      if (rb & 0b10011100) // case 2:3:4:7:
      {
        std::swap(ys, ye);
        ys = _cfg.panel_height - ys - 1;
        ye = _cfg.panel_height - ye - 1;
      }
      ys += _rowstart;
      ye += _rowstart;

      if (dlen_16bit)
      {
        _bus->writeCommand(CMD_RASET << 8, 16);
        _bus->writeData((ys >> 8) << 8 | ys << 24, 32);
        _bus->writeData((ye >> 8) << 8 | ye << 24, 32);
      }
      else
      {
        _bus->writeCommand(CMD_RASET, 8);
        _bus->writeData(ys >> 8 | (ys & 0xFF) << 8 | (ye << 8 | ye >> 8) << 16, 32);
      }
    }
    if (dlen_16bit)
    {
      _bus->writeCommand(CMD_RAMWR << 8, 16);
    }
    else
    {
      _bus->writeCommand(CMD_RAMWR, 8);
    }
  }

//----------------------------------------------------------------------------
 }
}
