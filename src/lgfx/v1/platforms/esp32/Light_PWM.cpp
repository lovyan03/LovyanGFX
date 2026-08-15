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
#if defined (ESP_PLATFORM)
#include <sdkconfig.h>

#include "Light_PWM.hpp"
#include "common.hpp"

#include <driver/gpio.h>

#if defined ( ARDUINO )
 #include <esp32-hal-ledc.h>
 #if __has_include(<esp_arduino_version.h>)
  #include <esp_arduino_version.h>
 #endif
#else
 #include <driver/ledc.h>
#endif

#if defined ( ARDUINO )

 #if defined ESP_ARDUINO_VERSION // use ledc* functions shipped with arduino-esp32 core
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
   #define LGFX_LEDCINIT(pin_bl, freq, bits, pwm_channel) ledcAttach(pin_bl, freq, bits)
   #define LGFX_LEDCWRITE(pin_bl, pwm_channel, duty) ledcWrite(pin_bl, duty)
  #endif
 #endif
 #if !defined LGFX_LEDCINIT // pre-3.x cores, including 1.x where ESP_ARDUINO_VERSION is absent
  #define LGFX_LEDCINIT(pin_bl, freq, bits, pwm_channel) ( ledcSetup(pwm_channel, freq, bits) > 0 && (ledcAttachPin(pin_bl, pwm_channel), true) )
  #define LGFX_LEDCWRITE(pin_bl, pwm_channel, duty) ledcWrite(pwm_channel, duty)
 #endif

#else // esp-idf

  #if SOC_LEDC_SUPPORT_HS_MODE
    #define LGFX_LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
  #else
    #define LGFX_LEDC_SPEED_MODE LEDC_LOW_SPEED_MODE
  #endif

#endif


namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr const uint8_t PWM_BITS = 9;


  bool Light_PWM::init(uint8_t brightness)
  {
    if ((size_t)_cfg.pin_bl >= GPIO_NUM_MAX || !GPIO_IS_VALID_OUTPUT_GPIO((gpio_num_t)_cfg.pin_bl))
    {
      return false;
    }

    // The LEDC driver does not fully normalize the pad state: the open-drain
    // flag is never cleared, and ESP-IDF v6 no longer selects the GPIO
    // function in IO_MUX. Reconfigure the pin as a push-pull GPIO output
    // first, latching it to the "off" level to avoid a visible glitch.
    if (_cfg.invert) { gpio_hi(_cfg.pin_bl); } else { gpio_lo(_cfg.pin_bl); }
    lgfx::pinMode(_cfg.pin_bl, pin_mode_t::output);

#if defined ( ARDUINO )

    bool result = LGFX_LEDCINIT(_cfg.pin_bl, _cfg.freq, PWM_BITS, _cfg.pwm_channel);

#else // esp-idf

    static ledc_channel_config_t ledc_channel;
    {
     ledc_channel.gpio_num   = (gpio_num_t)_cfg.pin_bl;
     ledc_channel.speed_mode = LGFX_LEDC_SPEED_MODE;
     ledc_channel.channel    = (ledc_channel_t)_cfg.pwm_channel;
     ledc_channel.timer_sel  = (ledc_timer_t)((_cfg.pwm_channel >> 1) & 3);
     ledc_channel.duty       = _cfg.invert ? (1 << PWM_BITS) : 0;
     ledc_channel.hpoint     = 0;
 #if defined ESP_IDF_VERSION_VAL
  // when ledc_channel_config_t.intr_type is deprecated, no need to explicitly configure interrupt, handled in the driver
  #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
     ledc_channel.intr_type  = LEDC_INTR_DISABLE;
  #endif
 #endif
    };
    bool result = (ESP_OK == ledc_channel_config(&ledc_channel));
    static ledc_timer_config_t ledc_timer;
    {
      ledc_timer.speed_mode = LGFX_LEDC_SPEED_MODE;     // timer mode
      ledc_timer.duty_resolution = (ledc_timer_bit_t)PWM_BITS; // resolution of PWM duty
      ledc_timer.freq_hz = _cfg.freq;                        // frequency of PWM signal
      ledc_timer.timer_num = ledc_channel.timer_sel;    // timer index
    };
    result = (ESP_OK == ledc_timer_config(&ledc_timer)) && result;

#endif

    if (!result) { return false; }
    setBrightness(brightness);
    return true;
  }

  void Light_PWM::setBrightness(uint8_t brightness)
  {
    uint32_t duty = 0;
    if (brightness)
    {
      uint_fast16_t ofs = _cfg.offset;
      if (ofs) { ofs = ofs * 259 >> 8; }
      duty = brightness * (257 - ofs);
      duty += ofs * 255;
      duty += 1 << (15 - PWM_BITS);
      duty >>= 16 - PWM_BITS;
    }
    if (_cfg.invert) duty = (1 << PWM_BITS) - duty;

#if defined ( ARDUINO )

    LGFX_LEDCWRITE(_cfg.pin_bl, _cfg.pwm_channel, duty);

#else // esp-idf

    ledc_set_duty(LGFX_LEDC_SPEED_MODE, (ledc_channel_t)_cfg.pwm_channel, duty);
    ledc_update_duty(LGFX_LEDC_SPEED_MODE, (ledc_channel_t)_cfg.pwm_channel);

#endif
  }


//----------------------------------------------------------------------------
 }
}

#endif
