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

#include "../../Light.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  // Backlight driver for the CH422G I/O expander.
  //
  // The CH422G uses two fixed I2C addresses: 0x24 (write-enable) and 0x38
  // (output data). It has no read-back, so a shadow register tracks the
  // current output state. All pin writes go through write_pin() to keep the
  // shadow consistent.
  //
  // Boards that route non-backlight signals (e.g. touch reset, LCD reset)
  // through the same CH422G should call write_pin() directly from their
  // init_impl() after calling init().
  class Light_CH422G : public ILight
  {
  public:
    struct config_t
    {
      uint32_t freq        = 400000;
      int16_t  pin_sda     = -1;
      int16_t  pin_scl     = -1;
      uint8_t  i2c_port    = 0;
      uint8_t  pin_bl      = 0;
      uint8_t  shadow_init = 0;
      bool     invert      = false;
    };

    const config_t& config(void) const { return _cfg; }
    void config(const config_t& cfg) { _cfg = cfg; }

    bool init(uint8_t brightness) override;
    void setBrightness(uint8_t brightness) override;

    void write_pin(uint8_t pin, bool state);

  private:
    config_t _cfg;
    uint8_t  _shadow = 0;

    void write_shadow(void);
  };

//----------------------------------------------------------------------------
 }
}
