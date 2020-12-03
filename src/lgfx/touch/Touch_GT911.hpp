#ifndef LGFX_TOUCH_I2C_GT911_HPP_
#define LGFX_TOUCH_I2C_GT911_HPP_

#include "TouchCommon.hpp"

namespace lgfx
{
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

    std::uint_fast8_t getTouch(std::int32_t* x, std::int32_t* y, std::int_fast8_t number) override;

  private:
    std::uint32_t _lasttime;
    std::uint32_t _refresh_rate = 5;
    std::uint8_t _readdata[42];
  };

//----------------------------------------------------------------------------

}
#endif
