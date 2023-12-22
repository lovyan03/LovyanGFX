#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/panel/Panel_ILI948x.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_Parallel16.hpp>

// LGFX for Elecrow 3.5" 480x320 ESP32 parallel touch display with ILI9488 (DLC35010R)
// https://www.elecrow.com/esp-terminal-with-esp32-3-5-inch-parallel-480x320-tft-capacitive-touch-display-rgb-by-chip-ili9488.html

class LGFX : public lgfx::LGFX_Device
{
public:
  lgfx::Bus_Parallel16 _bus_instance;
  lgfx::Panel_ILI9488 _panel_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_FT5x06  _touch_instance;

  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      cfg.freq_write = 40000000;
      cfg.pin_wr = GPIO_NUM_18;
      cfg.pin_rd = GPIO_NUM_48;
      cfg.pin_rs = GPIO_NUM_45;

      cfg.pin_d0  = GPIO_NUM_47;
      cfg.pin_d1  = GPIO_NUM_21;
      cfg.pin_d2  = GPIO_NUM_14;
      cfg.pin_d3  = GPIO_NUM_13;
      cfg.pin_d4  = GPIO_NUM_12;
      cfg.pin_d5  = GPIO_NUM_11;
      cfg.pin_d6  = GPIO_NUM_10;
      cfg.pin_d7  = GPIO_NUM_9;
      cfg.pin_d8  = GPIO_NUM_3;
      cfg.pin_d9  = GPIO_NUM_8;
      cfg.pin_d10 = GPIO_NUM_16;
      cfg.pin_d11 = GPIO_NUM_15;
      cfg.pin_d12 = GPIO_NUM_7;
      cfg.pin_d13 = GPIO_NUM_6;
      cfg.pin_d14 = GPIO_NUM_5;
      cfg.pin_d15 = GPIO_NUM_4;
      _bus_instance.config(cfg);
      _panel_instance.bus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.panel_width     = 320;
      cfg.panel_height    = 480;
      cfg.offset_x        = 0;
      cfg.offset_y        = 0;
      cfg.pin_cs          = GPIO_NUM_NC;
      cfg.pin_rst         = GPIO_NUM_NC;
      cfg.pin_busy        = GPIO_NUM_NC;
      cfg.offset_rotation = 3;
      cfg.readable        = true;
      cfg.invert          = false;
      cfg.rgb_order       = false;
      cfg.dlen_16bit      = true;
      cfg.bus_shared      = false;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();

      cfg.pin_bl = GPIO_NUM_46;

      _light_instance.config(cfg);
      _panel_instance.light(&_light_instance);
    }

    {
      auto cfg = _touch_instance.config();
      cfg.i2c_port = 0;
      cfg.i2c_addr = 0x38;
      cfg.pin_sda  = GPIO_NUM_38;
      cfg.pin_scl  = GPIO_NUM_39;
      cfg.pin_int  = GPIO_NUM_NC;
      cfg.freq     = 400000;
      
      cfg.x_min = 0;
      cfg.x_max = 319;
      cfg.y_min = 0;
      cfg.y_max = 479;

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};
