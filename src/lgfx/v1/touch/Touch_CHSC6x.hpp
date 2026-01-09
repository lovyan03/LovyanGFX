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
 [erazor83](https://github.com/mongonta0716)
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
    
    bool init(void) override;
    void wakeup(void) override {}
    void sleep(void) override {}

    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override;

    private:
      touch_point_t _previous_touch;
      uint32_t _last_poll;
      uint16_t _delay_ms;

  };

//----------------------------------------------------------------------------
 }
}
