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
#include "Light_CH422G.hpp"
#include "../common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  // CH422G I2C addresses (fixed by hardware, not configurable).
  static constexpr uint8_t kAddrCfg = 0x24;  // write-enable register
  static constexpr uint8_t kAddrOut = 0x38;  // output data register

  bool Light_CH422G::init(uint8_t brightness)
  {
    lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl);
    _shadow = _cfg.shadow_init;
    write_shadow();
    setBrightness(brightness);
    return true;
  }

  void Light_CH422G::setBrightness(uint8_t brightness)
  {
    bool on = (brightness > 0);
    write_pin(_cfg.pin_bl, _cfg.invert ? !on : on);
  }

  void Light_CH422G::write_pin(uint8_t pin, bool state)
  {
    if (state)
    {
      _shadow |=  (1u << pin);
    }
    else
    {
      _shadow &= ~(1u << pin);
    }
    write_shadow();
  }

  void Light_CH422G::write_shadow(void)
  {
    uint8_t enable = 0x01;
    lgfx::i2c::transactionWrite(_cfg.i2c_port, kAddrCfg, &enable,  1, _cfg.freq);
    lgfx::i2c::transactionWrite(_cfg.i2c_port, kAddrOut,  &_shadow, 1, _cfg.freq);
  }

//----------------------------------------------------------------------------
 }
}
