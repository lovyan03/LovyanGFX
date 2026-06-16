#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <lgfx/v1/platforms/esp32/Light_CH422G.hpp>


// LGFX for Waveshare ESP32-S3-Touch-LCD-4.3
// https://www.waveshare.com/esp32-s3-touch-lcd-4.3.htm
// Hardware: 800x480 RGB565 display with GT911 touch controller.
// Special:  CH422G I/O expander controls GT911 reset, LCD reset, and backlight.

// CH422G output-pin assignments (board-specific, not reconfigurable).
static constexpr uint8_t kCh422gPinTpRst  = 1;
static constexpr uint8_t kCh422gPinLcdBl  = 2;
static constexpr uint8_t kCh422gPinLcdRst = 3;
static constexpr uint8_t kCh422gPinSdCs   = 4;  // active-low; HIGH = deselected

class LGFX : public lgfx::LGFX_Device
{
public:
  lgfx::Bus_RGB      _bus_instance;
  lgfx::Panel_RGB    _panel_instance;
  lgfx::Light_CH422G _light_instance;
  lgfx::Touch_GT911  _touch_instance;

  LGFX(void)
  {
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width  = 800;
      cfg.memory_height = 480;
      cfg.panel_width   = 800;
      cfg.panel_height  = 480;
      cfg.offset_x      = 0;
      cfg.offset_y      = 0;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();
      cfg.use_psram = 1;
      _panel_instance.config_detail(cfg);
    }

    {
      auto cfg = _bus_instance.config();
      cfg.panel    = &_panel_instance;
      cfg.pin_d0   = GPIO_NUM_14;   // B0
      cfg.pin_d1   = GPIO_NUM_38;   // B1
      cfg.pin_d2   = GPIO_NUM_18;   // B2
      cfg.pin_d3   = GPIO_NUM_17;   // B3
      cfg.pin_d4   = GPIO_NUM_10;   // B4
      cfg.pin_d5   = GPIO_NUM_39;   // G0
      cfg.pin_d6   = GPIO_NUM_0;    // G1
      cfg.pin_d7   = GPIO_NUM_45;   // G2
      cfg.pin_d8   = GPIO_NUM_48;   // G3
      cfg.pin_d9   = GPIO_NUM_47;   // G4
      cfg.pin_d10  = GPIO_NUM_21;   // G5
      cfg.pin_d11  = GPIO_NUM_1;    // R0
      cfg.pin_d12  = GPIO_NUM_2;    // R1
      cfg.pin_d13  = GPIO_NUM_42;   // R2
      cfg.pin_d14  = GPIO_NUM_41;   // R3
      cfg.pin_d15  = GPIO_NUM_40;   // R4

      cfg.pin_henable = GPIO_NUM_5;
      cfg.pin_vsync   = GPIO_NUM_3;
      cfg.pin_hsync   = GPIO_NUM_46;
      cfg.pin_pclk    = GPIO_NUM_7;
      cfg.freq_write  = 16000000;

      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 8;
      cfg.hsync_pulse_width = 4;
      cfg.hsync_back_porch  = 8;
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 16;
      cfg.vsync_pulse_width = 4;
      cfg.vsync_back_porch  = 16;
      cfg.pclk_idle_high    = 1;
      _bus_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);

    {
      auto cfg = _light_instance.config();
      cfg.pin_sda     = GPIO_NUM_8;
      cfg.pin_scl     = GPIO_NUM_9;
      cfg.i2c_port    = 0;
      cfg.freq        = 400000;
      cfg.pin_bl      = kCh422gPinLcdBl;
      // All active-low pins (SD_CS) start HIGH (deselected); all others HIGH too.
      cfg.shadow_init = (1 << kCh422gPinTpRst) | (1 << kCh422gPinLcdBl)
                      | (1 << kCh422gPinLcdRst) | (1 << kCh422gPinSdCs);
      _light_instance.config(cfg);
      _panel_instance.light(&_light_instance);
    }

    {
      auto cfg = _touch_instance.config();
      cfg.x_min        = 0;
      cfg.y_min        = 0;
      cfg.x_max        = 800;
      cfg.y_max        = 480;
      cfg.bus_shared   = false;
      cfg.offset_rotation = 0;
      cfg.i2c_port     = 0;
      cfg.i2c_addr     = 0x5D;  // INT low during reset selects 0x5D.
      cfg.pin_sda      = GPIO_NUM_8;
      cfg.pin_scl      = GPIO_NUM_9;
      cfg.pin_int      = GPIO_NUM_4;
      cfg.pin_rst      = -1;    // Reset handled via CH422G in init_impl().
      cfg.freq         = 400000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }

  bool init_impl(bool use_reset, bool use_clear) override
  {
    // Initialise CH422G: sets up I2C, drives all pins to shadow_init state.
    _light_instance.init(255);

    // GT911 address selection: INT must be LOW while reset is asserted.
    // Holding INT low causes GT911 to select I2C address 0x5D on release.
    lgfx::pinMode(GPIO_NUM_4, lgfx::pin_mode_t::output);
    lgfx::gpio_lo(GPIO_NUM_4);
    lgfx::delay(10);

    _light_instance.write_pin(kCh422gPinTpRst, false);
    lgfx::delay(100);
    _light_instance.write_pin(kCh422gPinTpRst, true);
    lgfx::delay(10);

    lgfx::pinMode(GPIO_NUM_4, lgfx::pin_mode_t::input);

    return lgfx::LGFX_Device::init_impl(use_reset, use_clear);
  }
};

using LGFX_Waveshare = LGFX;
