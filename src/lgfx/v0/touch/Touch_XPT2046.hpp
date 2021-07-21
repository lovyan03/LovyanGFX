#ifndef LGFX_TOUCH_XPT2046_HPP_
#define LGFX_TOUCH_XPT2046_HPP_

#include "TouchCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Touch_XPT2046 : public TouchCommon
  {
    int threshold = 512;

    Touch_XPT2046() {
      freq = 2500000;
      x_min = 300;
      x_max = 3900;
      y_min = 400;
      y_max = 3900;
    }
    bool init(void) override;

    uint_fast8_t getTouch(touch_point_t* tp, int_fast8_t number) override;
  };

//----------------------------------------------------------------------------
 }
}
#endif
