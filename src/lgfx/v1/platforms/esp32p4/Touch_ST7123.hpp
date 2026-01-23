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

#include "../../Touch.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Touch_ST7123 : public ITouch
  {
    static constexpr const uint8_t default_addr = 0x55;

    Touch_ST7123(void) {
      _cfg.i2c_addr = default_addr;
      _cfg.x_min = 0;
      _cfg.x_max = 2047;
      _cfg.y_min = 0;
      _cfg.y_max = 2047;
    }

    bool init(void) override;
    void wakeup(void) override {};
    void sleep(void) override {};

    uint_fast8_t getTouchRaw(touch_point_t *tp, uint_fast8_t count) override;

  protected:
    enum
    {
      max_touch_points = 10
    };
    bool _readParams(uint16_t reg, uint8_t* read_data, size_t read_len);
    bool _read_fw_info(void);
  };

//----------------------------------------------------------------------------
 }
}
