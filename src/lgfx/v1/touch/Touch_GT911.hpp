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

  struct Touch_GT911 : public ITouch
  {
    Touch_GT911(void)
    {
      _cfg.x_min = 0;
      _cfg.x_max = 2047;
      _cfg.y_min = 0;
      _cfg.y_max = 2047;
    }

    bool init(void) override;

    void wakeup(void) override;

    void sleep(void) override;

    std::uint_fast8_t getTouchRaw(touch_point_t *tp, std::uint_fast8_t number) override;

    void setTouchNums(std::int_fast8_t nums);

  private:
    std::uint32_t _lasttime;
    std::uint32_t _refresh_rate = 5;
    std::uint8_t _readdata[42];

    void freshConfig(void);
    bool writeBytes(const std::uint8_t* data, std::size_t len);
    bool writeReadBytes(const std::uint8_t* write_data, std::size_t write_len, std::uint8_t* read_data, std::size_t read_len);
  };

//----------------------------------------------------------------------------
 }
}
