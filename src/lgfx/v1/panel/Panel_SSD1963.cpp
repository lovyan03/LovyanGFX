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

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Panel_SSD1963::init(bool use_reset)
  {
    if (!Panel_LCD::init(use_reset)) return false;

    uint_fast16_t w = _cfg.panel_width  - 1;
    uint_fast16_t h = _cfg.panel_height - 1;

    uint8_t cmds[] = 
    { 0xB0, 7, 0x20, 0x00
             , (uint8_t)(w >> 8), (uint8_t)w
             , (uint8_t)(h >> 8), (uint8_t)h
             , 0x2D
    // , 0xF0, 1, _cfg.dlen_16bit ?  : 0x00
 
    , 0xFF, 0xFF
    };

    startWrite();
    command_list(cmds);
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
        _write_depth = rgb888_3Byte;
        mode = 0x02;
      }
      else
      {
        _write_depth = rgb565_2Byte;
        mode = 0x03;
      }
    }
    else
    {
      _write_depth = rgb888_3Byte;
    }
    _read_depth = _write_depth;

    uint8_t cmds[] = 
    { 0xF0, 1, mode
    , 0xFF, 0xFF
    };

    startWrite();
    command_list(cmds);
    endWrite();
    _bus->flush();

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

    // writeCommand(0xB4, 1);
    // writeData(getSwap16( ht  ), 2);
    // writeData(getSwap16(hps  ), 2);
    // writeData(          hpw   , 1);
    // writeData(getSwap16(lps  ), 2);
    // writeData(          lpspp , 1);
    // _bus->flush();
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


    // writeCommand(0xB6, 1);
    // writeData(getSwap16( vt  ), 2);
    // writeData(getSwap16(vps  ), 2);
    // writeData(          vpw   , 1);
    // writeData(getSwap16(fps  ), 2);
    // _bus->flush();
    // endWrite();
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
