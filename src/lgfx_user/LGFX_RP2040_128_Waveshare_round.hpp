#pragma once

#include <LovyanGFX.h>

/**
 * LGFX configuration example for the Waveshare 1.28" round LCD module
 * https://www.waveshare.com/wiki/1.28inch_LCD_Module
 */
class LGFX : public lgfx::LGFX_Device
{
private:
  lgfx::Panel_GC9A01 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;

public:
  bool begin(int16_t pin_sclk,
             int16_t pin_mosi,
             int16_t pin_dc,
             int16_t pin_cs,
             int16_t pin_rst,
             int16_t pin_bl)
  {
    // SPI bus config
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = 1;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.pin_sclk = pin_sclk;
      cfg.pin_mosi = pin_mosi;
      cfg.pin_miso = -1;
      cfg.pin_dc = pin_dc;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    // LCD panel config
    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = pin_cs;
      cfg.pin_rst = pin_rst;
      cfg.pin_busy = -1;

      cfg.panel_width = 240;
      cfg.panel_height = 240;
      cfg.memory_width = 240;
      cfg.memory_height = 240;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 16;
      cfg.readable = true;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;

      _panel_instance.config(cfg);
    }

    // LCD backlight config
    {
      auto cfg = _light_instance.config();

      cfg.pin_bl = pin_bl;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);

    return lgfx::LGFX_Device::begin();
  }
};