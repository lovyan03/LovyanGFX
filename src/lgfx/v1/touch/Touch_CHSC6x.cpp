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
 [erazor83](https://github.com/erazor83)
/----------------------------------------------------------------------------*/
#include "Touch_CHSC6x.hpp"

#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  
  bool Touch_CHSC6X::init(void)
  {
    _inited = false;
    if (isSPI()) {
      return false;
    }

    if (_cfg.pin_int >= 0) {
      lgfx::pinMode(_cfg.pin_int, pin_mode_t::input_pullup);
    } else {
      return false;
    }
    
    
    _previous_touch.size = 0;
    _last_poll = 0;
    _delay_ms = 50;
    return lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value();
  };

  uint_fast8_t Touch_CHSC6X::getTouchRaw(touch_point_t *tp, uint_fast8_t count)
  {
    tp[0].size = 0;
    tp[0].id = 0;

    if ((bool)gpio_in(_cfg.pin_int)) {
      // require a high for at least _delay_ms to detect release
      if ((lgfx::millis() < (_last_poll + _delay_ms)) ) {
        if (_previous_touch.size) {
          tp[0].size = _previous_touch.size;
          tp[0].x = _previous_touch.x;
          tp[0].y = _previous_touch.y;
          return 1;
        } else {
          _previous_touch.size = 0;
        }
      }
      return 0;
    }

    uint8_t data[REG_READ_LEN];
    uint8_t reg = REG_STATUS;

    _last_poll = lgfx::millis();
    if (! lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, &reg, 1, data, REG_READ_LEN, _cfg.freq).has_value() ) {
      return 0;
    }
    
#if 0
    if (data[REG_PRESSED] == 1) {
      // TODO:  0 even when pressed
      tp[0].size = 1;
      tp[0].x = data[REG_COL];
      tp[0].y = data[REG_ROW];
      _previous_touch.size = 1;
      _previous_touch.x = data[REG_COL];
      _previous_touch.y = data[REG_ROW];
    }
#else
    _previous_touch.size = 1;
    _previous_touch.x = data[REG_COL];
    _previous_touch.y = data[REG_ROW];
    tp[0].size = 1;
    tp[0].x = data[REG_COL];
    tp[0].y = data[REG_ROW];
#endif
    return 1;
  }

//----------------------------------------------------------------------------
 }
}
