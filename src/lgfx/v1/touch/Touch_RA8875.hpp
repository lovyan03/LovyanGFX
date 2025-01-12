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

  struct Touch_RA8875 : public ITouch
  {
    Touch_RA8875(void)
    {
      _cfg.freq = 1000000;
      _cfg.x_min = 300;
      _cfg.x_max = 3900;
      _cfg.y_min = 400;
      _cfg.y_max = 3900;
    }

    bool init(void) override;

    void wakeup(void) override {}
    void sleep(void) override {}

    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override;
    
  private:
    uint8_t readRegister8(uint8_t reg);
    void writeRegister8(uint8_t reg, uint8_t data);
  };

//----------------------------------------------------------------------------
 }
}