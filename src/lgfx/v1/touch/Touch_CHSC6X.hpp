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

#include "../Touch.hpp"


namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Touch_CHSC6X : public ITouch
  {

    static const uint8_t REG_STATUS   = 0x00;
    static const uint8_t REG_PRESSED  = 0x00;
    static const uint8_t REG_COL      = 0x02;
    static const uint8_t REG_ROW      = 0x04;
    static const uint8_t REG_READ_LEN = 0x05;


    Touch_CHSC6X(void)
    {
      _cfg.i2c_addr = 0x2e;
      _cfg.x_min = 0;
      _cfg.x_max = 4095;
      _cfg.y_min = 0;
      _cfg.y_max = 4095;
    }

    bool init(void) override
    {
      _inited = false;
      if (isSPI()) return false;

      if (_cfg.pin_int >= 0)
      {
        lgfx::pinMode(_cfg.pin_int, pin_mode_t::input_pullup);
      }
      return lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value();
    };

    void wakeup(void) override {}
    void sleep(void) override {}

    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override
    {
      tp[0].size = 0;
      tp[0].id = 0;
      if (_cfg.pin_int >= 0 && (bool)gpio_in(_cfg.pin_int))
      {
        return 0;
      }
      uint8_t data[REG_READ_LEN];
      uint8_t reg = REG_STATUS;

      if (! lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, &reg, 1, data, REG_READ_LEN, _cfg.freq).has_value() ) {
        return 0;
      }

      if (data[REG_PRESSED] == 1) {
        tp[0].size = 1;
        tp[0].x = data[REG_COL];
        tp[0].y = data[REG_ROW];
      }
      return 1;
    };

  };

//----------------------------------------------------------------------------
 }
}
