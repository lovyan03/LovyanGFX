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
#include "Panel_SSD1351.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Panel_SSD1351::setInvert(bool invert)
  {
    _invert = invert;
    startWrite();
    write_command((invert ^ _cfg.invert) ? CMD_INVON : CMD_INVOFF);
    endWrite();
  }

  void Panel_SSD1351::setSleep(bool flg)
  {
    startWrite();
    write_command(flg ? CMD_SLPIN : CMD_SLPOUT);
    endWrite();
  }

  void Panel_SSD1351::setPowerSave(bool flg)
  {
    startWrite();
    write_command(flg ? CMD_DISPOFF : CMD_DISPON);
    endWrite();
  }

  void Panel_SSD1351::update_madctl(void)
  {
    static constexpr uint8_t madctl_table[] = {
      0b00100000,
      0b00100011,
      0b00110010,
      0b00110001,
      0b00110000,
      0b00100001,
      0b00100010,
      0b00110011,
    };

    if (_bus == nullptr) return;

    startWrite();
    write_command(CMD_MADCTL);
    _bus->writeData( madctl_table[_internal_rotation]
                   | (_write_bits == 16 ? 0x40 : 0x80)
                   | (_cfg.rgb_order    ? 0x00 : 0x04)
                   , 8);
    endWrite();
  }

  void Panel_SSD1351::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
  {
    set_window_8(xs, ys, xe, ye, CMD_RAMWR);
  }

  void Panel_SSD1351::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    bool tr = _in_transaction;
    if (!tr) begin_transaction();

    set_window_8(x, y, x, y, CMD_RAMWR);
    _bus->writeData(rawcolor, _write_bits);

    if (!tr) end_transaction();
  }

  void Panel_SSD1351::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint32_t len = w * h;
    uint_fast16_t xe = w + x - 1;
    uint_fast16_t ye = y + h - 1;
    set_window_8(x, y, xe, ye, CMD_RAMWR);
    _bus->writeDataRepeat(rawcolor, _write_bits, len);
  }

  void Panel_SSD1351::set_window_8(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye, uint32_t cmd)
  {
    if (xs != _xs || xe != _xe)
    {
      _xs = xs;
      _xe = xe;
      _bus->writeCommand((_internal_rotation & 1) ? CMD_RASET : CMD_CASET, 8);
      xs += _colstart;
      xe += _colstart;
      _bus->writeData(xs | xe << 8, 16);
    }
    if (ys != _ys || ye != _ye)
    {
      _ys = ys;
      _ye = ye;
      _bus->writeCommand((_internal_rotation & 1) ? CMD_CASET : CMD_RASET, 8);
      ys += _rowstart;
      ye += _rowstart;
      _bus->writeData(ys | ye << 8, 16);
    }
    _bus->writeCommand(cmd, 8);
  }

//----------------------------------------------------------------------------
 }
}
