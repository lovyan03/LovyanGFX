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
#if defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040) || defined(USE_PICO_SDK)

#include "Light_PWM.hpp"

#include <hardware/pwm.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#if __has_include(<pico/stdio.h>)
#include <pico/stdio.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Light_PWM::init( uint8_t brightness )
  {

    gpio_init( _cfg.pin_bl );
    gpio_set_dir( _cfg.pin_bl, GPIO_OUT );
    gpio_put( _cfg.pin_bl, 1 );

    gpio_set_function( _cfg.pin_bl, GPIO_FUNC_PWM );
    _slice_num = pwm_gpio_to_slice_num( _cfg.pin_bl );
    pwm_set_wrap( _slice_num, 100 );
    pwm_set_chan_level( _slice_num, _cfg.pwm_channel, 1 );
    pwm_set_clkdiv( _slice_num, 50 );
    pwm_set_enabled( _slice_num, true );

    setBrightness(brightness);

    return true;
  }

  void Light_PWM::setBrightness( uint8_t brightness )
  {
    if (_cfg.invert) brightness = ~brightness;
    // uint32_t duty = brightness + (brightness >> 7);
    pwm_set_chan_level( _slice_num, _cfg.pwm_channel, brightness );
  }

//----------------------------------------------------------------------------
 }
}
#endif 
#endif
