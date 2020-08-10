#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32) || (ESP_PLATFORM)

#include "esp32_common.hpp"

#include <driver/rtc_io.h>
#include <soc/rtc.h>

#if defined ARDUINO
 #include <esp32-hal-ledc.h>
#else
 #include <driver/ledc.h>
#endif

namespace lgfx
{

  void lgfxPinMode(std::int_fast8_t pin, pin_mode_t mode)
  {
    if (pin < 0) return;
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_DISABLE);
#if defined (ARDUINO)
    int m;
    switch (mode)
    {
    case pin_mode_t::output:         m = OUTPUT;         break;
    default:
    case pin_mode_t::input:          m = INPUT;          break;
    case pin_mode_t::input_pullup:   m = INPUT_PULLUP;   break;
    case pin_mode_t::input_pulldown: m = INPUT_PULLDOWN; break;
    }
    pinMode(pin, m);
#else
    if (rtc_gpio_is_valid_gpio((gpio_num_t)pin)) rtc_gpio_deinit((gpio_num_t)pin);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (std::uint64_t)1 << pin;
    switch (mode)
    {
    case pin_mode_t::output:
      io_conf.mode = GPIO_MODE_OUTPUT;
      break;
    default:
      io_conf.mode = GPIO_MODE_INPUT;
      break;
    }
    io_conf.mode         = (mode == pin_mode_t::output) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;
    io_conf.pull_down_en = (mode == pin_mode_t::input_pulldown) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en   = (mode == pin_mode_t::input_pullup  ) ? GPIO_PULLUP_ENABLE   : GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
#endif
  }

  void initPWM(std::int_fast8_t pin, std::uint32_t pwm_ch, std::uint8_t duty)
  {
#ifdef ARDUINO

    ledcSetup(pwm_ch, 12000, 8);
    ledcAttachPin(pin, pwm_ch);
    ledcWrite(pwm_ch, duty);

#else

    static ledc_channel_config_t ledc_channel;
    {
     ledc_channel.gpio_num   = (gpio_num_t)pin;
     ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
     ledc_channel.channel    = (ledc_channel_t)pwm_ch;
     ledc_channel.intr_type  = LEDC_INTR_DISABLE;
     ledc_channel.timer_sel  = (ledc_timer_t)((pwm_ch >> 1) & 3);
     ledc_channel.duty       = duty; // duty;
     ledc_channel.hpoint     = 0;
    };
    ledc_channel_config(&ledc_channel);
    static ledc_timer_config_t ledc_timer;
    {
      ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;     // timer mode
      ledc_timer.duty_resolution = (ledc_timer_bit_t)8; // resolution of PWM duty
      ledc_timer.freq_hz = 12000;                        // frequency of PWM signal
      ledc_timer.timer_num = ledc_channel.timer_sel;    // timer index
    };
    ledc_timer_config(&ledc_timer);

#endif
  }

  void setPWMDuty(std::uint32_t pwm_ch, std::uint8_t duty)
  {
#ifdef ARDUINO
    ledcWrite(pwm_ch, duty);
#else
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)pwm_ch, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)pwm_ch);
#endif
  }


};

#endif
