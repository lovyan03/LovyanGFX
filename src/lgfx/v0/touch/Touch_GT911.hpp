#ifndef LGFX_TOUCH_I2C_GT911_HPP_
#define LGFX_TOUCH_I2C_GT911_HPP_

#include "TouchCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Touch_GT911 : public TouchCommon
  {
    Touch_GT911(void) {
      x_min = 0;
      x_max = 2048;
      y_min = 0;
      y_max = 2048;
    }

    bool init(void) override;

    void wakeup(void) override;

    void sleep(void) override;

    uint_fast8_t getTouch(touch_point_t* tp, int_fast8_t number) override;

    void setTouchNums(int_fast8_t nums);

  private:
    uint32_t _lasttime;
    uint32_t _refresh_rate = 5;
    uint8_t _readdata[42];

    void freshConfig(void);
  };

//----------------------------------------------------------------------------
 }
}
#endif
