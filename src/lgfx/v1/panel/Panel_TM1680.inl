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
#ifndef LGFX_V1_IMPLEMENTATION
#error "Panel_TM1680.inl is part of lgfx_v1.cpp and is not meant to be included on its own"
#endif
#include "Panel_TM1680.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"

#include <string.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Panel_TM1680::init(bool use_reset)
  {
    // The TM1680 does not respond to I2C reads, so the read probe of
    // Panel_1bitOLED::init cannot be used; send the init commands without probing.
    if (!Panel_HasBuffer::init(use_reset))
    {
      return false;
    }
    // Panel_HasBuffer::init does not clear the allocated buffer.
    memset(_buf, 0, _get_buffer_length());

    // The init commands do not touch the LED RAM, and nothing is pushed here, so the
    // image on the LEDs survives init() (init without clear keeps it until the first draw).
    startWrite(true);
    for (size_t i = 0; auto cmds = getInitCommands(i); i++)
    {
      size_t idx = 0;
      while (cmds[idx] != 0xFF || cmds[idx + 1] != 0xFF) ++idx;
      if (idx) { _bus->writeBytes(cmds, idx, false, true); }
    }
    setRotation(_rotation);
    endWrite();

    return true;
  }

  void Panel_TM1680::setInvert(bool invert)
  {
    // The TM1680 has no display-invert command; invert while packing in display().
    // Only a change is pushed: LGFX_Device::init_impl() re-applies the current value,
    // and pushing the cleared buffer there would wipe the LEDs on init without clear.
    if (_invert == invert) { return; }
    _invert = invert;
    if (_buf)
    {
      startWrite();
      display(0, 0, _cfg.panel_width, _cfg.panel_height);
      endWrite();
    }
  }

  void Panel_TM1680::setSleep(bool flg)
  {
    startWrite();
    if (flg)
    {
      _bus->writeCommand(CMD_LED_OFF, 8);
      _bus->writeCommand(CMD_SYS_DIS, 8);
    }
    else
    {
      _bus->writeCommand(CMD_SYS_EN, 8);
      _bus->writeCommand(CMD_LED_ON, 8);
    }
    endWrite();
  }

  void Panel_TM1680::setBrightness(uint8_t brightness)
  {
    startWrite();
    if (brightness)
    {
      _bus->writeCommand(CMD_PWM_DUTY + (((brightness * 16) >> 8) & 0x0F), 8);
      _bus->writeCommand(CMD_LED_ON, 8);
    }
    else
    {
      _bus->writeCommand(CMD_LED_OFF, 8);
    }
    endWrite();
  }

  void Panel_TM1680::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      uint_fast16_t xs = x, xe = x + w - 1;
      uint_fast16_t ys = y, ye = y + h - 1;
      _update_transferred_rect(xs, ys, xe, ye);
    }
    if (_range_mod.empty()) { return; }

    // The whole frame is only 32 bytes; always send it in full.
    // The leading byte is the RAM write address.
    uint8_t sendbuf[1 + ((16 * 16) >> 3)];
    sendbuf[0] = 0x00;
    bool invert = _invert ^ _cfg.invert;

    for (uint_fast8_t py = 0; py < 16; ++py)
    {
      uint32_t mask = 1 << (py & 7);
      auto src = &_buf[(py >> 3) * 16];
      for (uint_fast8_t xb = 0; xb < 2; ++xb)
      {
        uint_fast8_t v = 0;
        for (uint_fast8_t bit = 0; bit < 8; ++bit)
        {
          if (src[(xb << 3) + bit] & mask)
          { // in TM1680 RAM, rows A3-A0 occupy the upper nibble half
            v |= 1 << ((bit > 3) ? (bit & 3) : (4 + bit));
          }
        }
        sendbuf[1 + py * 2 + xb] = invert ? (v ^ 0xFF) : v;
      }
    }

    _bus->writeBytes(sendbuf, sizeof(sendbuf), true, true);
    _range_mod.top    = INT16_MAX;
    _range_mod.left   = INT16_MAX;
    _range_mod.right  = 0;
    _range_mod.bottom = 0;
  }

//----------------------------------------------------------------------------
 }
}
