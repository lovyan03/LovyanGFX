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
#include "Touch_NS2009.hpp"

#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  int Touch_NS2009::_read_cmd(uint8_t reg)
  {
    uint8_t buf[2];
    return (lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, &reg, 1, buf, 2, _cfg.freq).has_value())
         ? (buf[1] >> 4) + (buf[0] << 4)
         : -1;
  }

  bool Touch_NS2009::init(void)
  {
    _inited = false;
    if (isSPI()) return false;

    if (_cfg.pin_int >= 0)
    {
      lgfx::pinMode(_cfg.pin_int, pin_mode_t::input_pullup);
    }
    return lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value();
  }

  uint_fast8_t Touch_NS2009::getTouchRaw(touch_point_t *tp, uint_fast8_t count)
  {
    tp[0].size = 0;
    tp[0].id = 0;
    if (_cfg.pin_int >= 0 && (bool)gpio_in(_cfg.pin_int))
    {
      return 0;
    }

    int z1 = _read_cmd(0xE0);
    if (z1 < 32) { return 0; }
    uint_fast16_t x  = _read_cmd(0xC0);
    uint_fast16_t y  = _read_cmd(0xD0);
    uint_fast16_t z2 = _read_cmd(0xF0);
    if (y >= 4095 || x >= 4095 || z2 >= 4095)
    {
      return 0;
    }
    else
    {
      int size = 384 - (( x *  ((z2*256 / z1) - 256)) >> 12);
      tp[0].x = (int16_t)x;
      tp[0].y = (int16_t)y;
      tp[0].size = std::max(1, size);
    }
    return 1;
  }

//----------------------------------------------------------------------------
 }
}
