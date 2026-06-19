#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <lgfx/v1/platforms/esp32/Light_CH422G.hpp>

// LGFX for Waveshare ESP32-S3-Touch-LCD-7
// https://www.waveshare.com/esp32-s3-touch-lcd-7.htm
// Hardware: 800x480 RGB565 display (ST7262) with GT911 touch controller.
// Special:  CH422G I/O expander controls GT911 reset, LCD reset, backlight,
//           SD card CS, USB/CAN mux, and LCD power enable.

// CH422G output-pin assignments (board-specific, not reconfigurable).
static constexpr uint8_t kCh422gPinTpRst    = 1;
static constexpr uint8_t kCh422gPinLcdBl    = 2;
static constexpr uint8_t kCh422gPinLcdRst   = 3;
static constexpr uint8_t kCh422gPinSdCs     = 4;  // active-low; HIGH = deselected
// EXIO5 controls the USB/CAN mux (U9, FSUSB42UMX).
// Default: low = USB mode (consistent with R117 hardware pull-down).
// To enable CAN: call _light_instance.write_pin(kCh422gPinUsbSel, true)
// after display.init(), then initialise the TWAI driver.
static constexpr uint8_t kCh422gPinUsbSel   = 5;
static constexpr uint8_t kCh422gPinLcdVddEn = 6;  // LCD panel power enable; HIGH = on

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

      // 16-bit RGB565 data bus — pin order matches ST7262 panel connector.
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

      // Timing parameters verified against Waveshare supplier config
      // (esp_panel_board_custom_conf.h).
      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 8;
      cfg.hsync_pulse_width = 4;
      cfg.hsync_back_porch  = 8;
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 8;
      cfg.vsync_pulse_width = 4;
      cfg.vsync_back_porch  = 8;
      cfg.pclk_idle_high    = 1;
      _bus_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);

    {
      auto cfg = _light_instance.config();
      cfg.pin_sda  = GPIO_NUM_8;
      cfg.pin_scl  = GPIO_NUM_9;
      cfg.i2c_port = 0;
      cfg.freq     = 400000;
      cfg.pin_bl   = kCh422gPinLcdBl;
      // Initial shadow state:
      //   - kCh422gPinLcdVddEn HIGH : panel power on
      //   - kCh422gPinLcdBl   HIGH : backlight on
      //   - kCh422gPinTpRst   HIGH : touch not in reset
      //   - kCh422gPinLcdRst  HIGH : LCD not in reset
      //   - kCh422gPinSdCs    HIGH : SD card deselected (active-low)
      //   - kCh422gPinUsbSel  LOW  : USB mode (consistent with R117 pull-down)
      cfg.shadow_init = (1 << kCh422gPinLcdVddEn)
                      | (1 << kCh422gPinLcdBl)
                      | (1 << kCh422gPinTpRst)
                      | (1 << kCh422gPinLcdRst)
                      | (1 << kCh422gPinSdCs);
      _light_instance.config(cfg);
      _panel_instance.light(&_light_instance);
    }

    {
      auto cfg = _touch_instance.config();
      cfg.x_min           = 0;
      cfg.y_min           = 0;
      cfg.x_max           = 800;
      cfg.y_max           = 480;
      cfg.bus_shared      = false;
      cfg.offset_rotation = 0;
      cfg.i2c_port        = 0;
      cfg.i2c_addr        = 0x5D;  // INT low during reset selects 0x5D.
      cfg.pin_sda         = GPIO_NUM_8;
      cfg.pin_scl         = GPIO_NUM_9;
      cfg.pin_int         = GPIO_NUM_4;
      cfg.pin_rst         = -1;    // Reset handled via CH422G in init_impl().
      cfg.freq            = 400000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }

  bool init_impl(bool use_reset, bool use_clear) override
  {
    // Enable LCD panel power before any other init.
    // CH422G init also drives all shadow_init pins to their defined states.
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
