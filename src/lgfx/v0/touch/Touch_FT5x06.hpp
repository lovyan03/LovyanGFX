#ifndef LGFX_TOUCH_I2C_FT5x06_HPP_
#define LGFX_TOUCH_I2C_FT5x06_HPP_

#include "TouchCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Touch_FT5x06 : public TouchCommon
  {
    Touch_FT5x06(void) {
      x_min = 0;
      x_max = 320;
      y_min = 0;
      y_max = 320;
    }

    bool init(void) override;

    void wakeup(void) override;

    void sleep(void) override;

    uint_fast8_t getTouch(touch_point_t* tp, int_fast8_t number) override;
  };

//----------------------------------------------------------------------------
 }
}
#endif
