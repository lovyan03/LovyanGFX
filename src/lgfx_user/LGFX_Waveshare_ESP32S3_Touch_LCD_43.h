#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <driver/i2c.h>

// Platform-specific I2C includes.
#ifdef ARDUINO
#include <Wire.h>
#else
#include <driver/i2c_master.h>
#endif

// LGFX for Waveshare ESP32-S3-Touch-LCD-4.3
// https://www.waveshare.com/esp32-s3-touch-lcd-4.3.htm
// Hardware: 800x480 RGB565 display with GT911 touch controller
// Special: CH422G I/O expander controls GT911 reset and LCD backlight

namespace lgfx {

// Custom backlight class for CH422G I/O expander control.
class Light_CH422G : public ILight {
 public:
  Light_CH422G() : backlight_pin_(0), exio_shadow_(0) {}

  bool init(uint8_t brightness) override {
    setBrightness(brightness);
    return true;
  }

  void setBrightness(uint8_t brightness) override {
    // CH422G has no PWM - digital on/off only.
    bool state = (brightness > 0);
    writeEXIO(backlight_pin_, state);
  }

  void setBacklightPin(uint8_t pin) { backlight_pin_ = pin; }
  void setInitialState(uint8_t shadow) { exio_shadow_ = shadow; }

 private:
  uint8_t backlight_pin_;
  uint8_t exio_shadow_;

  void writeEXIO(uint8_t pin, bool state) {
    if (state) {
      exio_shadow_ |= (1 << pin);
    } else {
      exio_shadow_ &= ~(1 << pin);
    }

    // CH422G dual-address protocol.
#ifdef ARDUINO
    Wire.beginTransmission(0x24);
    Wire.write(0x01);  // Write enable.
    Wire.endTransmission();
    Wire.beginTransmission(0x38);
    Wire.write(exio_shadow_);  // Output data.
    Wire.endTransmission();
#else
    // ESP-IDF i2c_master write.
    uint8_t enable_cmd = 0x01;
    i2c_master_transmit(dev_handle_24_, &enable_cmd, 1, 100);
    i2c_master_transmit(dev_handle_38_, &exio_shadow_, 1, 100);
#endif
  }

#ifndef ARDUINO
  i2c_master_dev_handle_t dev_handle_24_;
  i2c_master_dev_handle_t dev_handle_38_;
#endif

  friend class LGFX;
};

}  // namespace lgfx

class LGFX : public lgfx::LGFX_Device {
 public:
  lgfx::Bus_RGB _bus_instance;
  lgfx::Panel_RGB _panel_instance;
  lgfx::Light_CH422G _light_instance;
  lgfx::Touch_GT911 _touch_instance;

  LGFX(void) {
    // Panel configuration.
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width = 800;
      cfg.memory_height = 480;
      cfg.panel_width = 800;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();
      cfg.use_psram = 1;
      _panel_instance.config_detail(cfg);
    }

    // RGB bus configuration.
    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;
      cfg.pin_d0 = GPIO_NUM_14;   // B0
      cfg.pin_d1 = GPIO_NUM_38;   // B1
      cfg.pin_d2 = GPIO_NUM_18;   // B2
      cfg.pin_d3 = GPIO_NUM_17;   // B3
      cfg.pin_d4 = GPIO_NUM_10;   // B4
      cfg.pin_d5 = GPIO_NUM_39;   // G0
      cfg.pin_d6 = GPIO_NUM_0;    // G1
      cfg.pin_d7 = GPIO_NUM_45;   // G2
      cfg.pin_d8 = GPIO_NUM_48;   // G3
      cfg.pin_d9 = GPIO_NUM_47;   // G4
      cfg.pin_d10 = GPIO_NUM_21;  // G5
      cfg.pin_d11 = GPIO_NUM_1;   // R0
      cfg.pin_d12 = GPIO_NUM_2;   // R1
      cfg.pin_d13 = GPIO_NUM_42;  // R2
      cfg.pin_d14 = GPIO_NUM_41;  // R3
      cfg.pin_d15 = GPIO_NUM_40;  // R4

      cfg.pin_henable = GPIO_NUM_5;   // DE (Data Enable)
      cfg.pin_vsync = GPIO_NUM_3;
      cfg.pin_hsync = GPIO_NUM_46;
      cfg.pin_pclk = GPIO_NUM_7;
      cfg.freq_write = 16000000;

      cfg.hsync_polarity = 0;
      cfg.hsync_front_porch = 8;
      cfg.hsync_pulse_width = 4;
      cfg.hsync_back_porch = 8;
      cfg.vsync_polarity = 0;
      cfg.vsync_front_porch = 16;
      cfg.vsync_pulse_width = 4;
      cfg.vsync_back_porch = 16;
      cfg.pclk_idle_high = 1;
      _bus_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);

    // Backlight configuration.
    _panel_instance.light(&_light_instance);

    // Touch configuration.
    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 0;
      cfg.y_min = 0;
      cfg.x_max = 800;
      cfg.y_max = 480;
      cfg.bus_shared = false;
      cfg.offset_rotation = 0;
      // I2C connection
      cfg.i2c_port = I2C_NUM_0;
      cfg.i2c_addr = 0x5D;  // Selected via INT pin during reset.
      cfg.pin_sda = GPIO_NUM_8;
      cfg.pin_scl = GPIO_NUM_9;
      cfg.pin_int = GPIO_NUM_4;
      cfg.pin_rst = -1;  // Reset via CH422G, not direct GPIO.
      cfg.freq = 400000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }

  // Hardware initialization after Arduino core ready.
  bool init_impl(bool use_reset, bool use_clear) override {
    // I2C initialization for touch and CH422G.
#ifdef ARDUINO
    Wire.begin(8, 9);  // SDA=8, SCL=9
    Wire.setClock(400000);
#else
    // ESP-IDF i2c_master initialization.
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_8,
        .scl_io_num = GPIO_NUM_9,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {.enable_internal_pullup = true},
    };
    i2c_master_bus_handle_t bus_handle;
    i2c_new_master_bus(&bus_config, &bus_handle);

    // CH422G device handles.
    i2c_device_config_t dev_cfg_24 = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x24,
        .scl_speed_hz = 400000,
    };
    i2c_device_config_t dev_cfg_38 = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x38,
        .scl_speed_hz = 400000,
    };
    i2c_master_bus_add_device(bus_handle, &dev_cfg_24,
                               &_light_instance.dev_handle_24_);
    i2c_master_bus_add_device(bus_handle, &dev_cfg_38,
                               &_light_instance.dev_handle_38_);
#endif

    // CH422G initial state: LCD_RST=1, LCD_BL=1, TP_RST=1.
    // Pin assignments: TP_RST=1, LCD_BL=2, LCD_RST=3.
    uint8_t initial_state = (1 << 1) | (1 << 2) | (1 << 3);
    _light_instance.setInitialState(initial_state);
    _light_instance.setBacklightPin(2);

    writeCH422G(initial_state);
    delay(10);

    // GT911 reset sequence.
    // GT911 samples INT pin during reset to select I2C address 0x5D.
    // INT must be LOW during reset.

    // Step 1: Pull INT pin LOW.
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);
    delay(10);

    // Step 2: Assert GT911 reset (TP_RST=0) via CH422G.
    uint8_t reset_state = (1 << 2) | (1 << 3);  // LCD_BL=1, LCD_RST=1, TP_RST=0
    writeCH422G(reset_state);
    delay(100);

    // Step 3: Release GT911 reset (TP_RST=1).
    writeCH422G(initial_state);
    delay(10);

    // Step 4: Configure INT pin as input for touch interrupts.
    pinMode(4, INPUT);

    // Call parent init.
    return lgfx::LGFX_Device::init_impl(use_reset, use_clear);
  }

 private:
  void writeCH422G(uint8_t data) {
#ifdef ARDUINO
    Wire.beginTransmission(0x24);
    Wire.write(0x01);  // Write enable.
    Wire.endTransmission();
    Wire.beginTransmission(0x38);
    Wire.write(data);  // Output data.
    Wire.endTransmission();
#else
    uint8_t enable_cmd = 0x01;
    i2c_master_transmit(_light_instance.dev_handle_24_, &enable_cmd, 1, 100);
    i2c_master_transmit(_light_instance.dev_handle_38_, &data, 1, 100);
#endif
  }
};
