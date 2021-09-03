#ifndef LGFX_TOUCH_STMPE610_HPP_
#define LGFX_TOUCH_STMPE610_HPP_

#include "TouchCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Touch_STMPE610 : public TouchCommon
  {
    Touch_STMPE610() {
      freq = 1000000;
      x_min = 240;
      x_max = 3750;
      y_min = 200;
      y_max = 3700;
    }
    bool init(void) override;

    uint_fast8_t getTouch(touch_point_t* tp, int_fast8_t number) override;

  private:
    int _spi_mode = 0;
    bool _last_press = false;

    uint8_t readRegister8(uint8_t reg);
    void writeRegister8(uint8_t reg, uint8_t val);
    int getVersion(void);
    bool bufferEmpty(void);
  };

//----------------------------------------------------------------------------
 }
}
#endif
