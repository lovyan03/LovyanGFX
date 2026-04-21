
#include <LovyanGFX.hpp>

#if !defined (CONFIG_IDF_TARGET_ESP32P4)
  #error "CONFIG_IDF_TARGET_ESP32P4 should be set"
#endif

#include "lgfx/v1/platforms/esp32p4/Bus_DSI.hpp"
#include "lgfx/v1/platforms/esp32p4/Panel_ILI9881C.hpp"
#include "lgfx/v1/platforms/esp32p4/Panel_ST7123.hpp"
#include "lgfx/v1/platforms/esp32p4/Touch_ST7123.hpp"

static constexpr int_fast16_t in_i2c_port = I2C_NUM_1;

class M5Tab5 : public lgfx::LGFX_Device
{
  lgfx::Bus_DSI       _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Panel_DSI*    _panel_ptr = nullptr;
  lgfx::ITouch*       _touch_ptr = nullptr;

protected:

  // M5Tab5 io multiplexer addresses
  static constexpr uint_fast8_t pi4io1_i2c_addr = 0x43;
  static constexpr uint_fast8_t pi4io2_i2c_addr = 0x44;

  void i2c_write_register8_array(int_fast16_t i2c_port, uint_fast8_t i2c_addr, const uint8_t* reg_data_mask, uint32_t freq)
  {
    while (reg_data_mask[0] != 0xFF || reg_data_mask[1] != 0xFF || reg_data_mask[2] != 0xFF)
    {
      lgfx::i2c::writeRegister8(i2c_port, i2c_addr, reg_data_mask[0], reg_data_mask[1], reg_data_mask[2], freq);
      reg_data_mask += 3;
    }
  }

public:

  M5Tab5() {}

  bool init_impl(bool use_reset, bool use_clear)
  {
    // SDA = GPIO_NUM_31
    // SCL = GPIO_NUM_32
    // TP INT = GPIO_NUM_23
    lgfx::pinMode(GPIO_NUM_23, lgfx::pin_mode_t::output); // TP INT
    lgfx::gpio_hi(GPIO_NUM_23); // select I2C Addr (high=0x14 / low=0x5D)
    lgfx::i2c::init(in_i2c_port, GPIO_NUM_31, GPIO_NUM_32);

    auto id = lgfx::i2c::readRegister8(in_i2c_port, pi4io1_i2c_addr, 0x01).has_value()
            && lgfx::i2c::readRegister8(in_i2c_port, pi4io2_i2c_addr, 0x01).has_value();
    if (id != 0) {

      static constexpr const uint8_t reg_data_io1_1[] = {
        0x03, 0b01111111, 0,   // PI4IO_REG_IO_DIR
        0x05, 0b01000110, 0,   // PI4IO_REG_OUT_SET (bit4=LCD Reset,bit5=GT911 TouchReset  LOW)
        0x07, 0b00000000, 0,   // PI4IO_REG_OUT_H_IM
        0x0D, 0b01111111, 0,   // PI4IO_REG_PULL_SEL
        0x0B, 0b01111111, 0,   // PI4IO_REG_PULL_EN
        0xFF,0xFF,0xFF,
      };
      static constexpr const uint8_t reg_data_io1_2[] = {
        0x05, 0b01110110, 0,   // PI4IO_REG_OUT_SET (bit4=LCD Reset,bit5=GT911 TouchReset  HIGH)
        0xFF,0xFF,0xFF,
      };

      static constexpr const uint8_t reg_data_io2[] = {
        0x03, 0b10111001, 0,   // PI4IO_REG_IO_DIR
        0x07, 0b00000110, 0,   // PI4IO_REG_OUT_H_IM
        0x0D, 0b10111001, 0,   // PI4IO_REG_PULL_SEL
        0x0B, 0b11111001, 0,   // PI4IO_REG_PULL_EN
        0x09, 0b01000000, 0,   // PI4IO_REG_IN_DEF_STA
        0x11, 0b10111111, 0,   // PI4IO_REG_INT_MASK
        0x05, 0b10001001, 0,   // PI4IO_REG_OUT_SET
        0xFF,0xFF,0xFF,
      };

      i2c_write_register8_array(in_i2c_port, pi4io1_i2c_addr, reg_data_io1_1, 100000);
      i2c_write_register8_array(in_i2c_port, pi4io2_i2c_addr, reg_data_io2, 100000);
      lgfx::delay(10);
      i2c_write_register8_array(in_i2c_port, pi4io1_i2c_addr, reg_data_io1_2, 100000);

#if !CONFIG_SPIRAM
      ESP_LOGE("LGFX", "M5Tab5 need PSRAM enabled");
      if (false)
#elif CONFIG_SPIRAM_SPEED <= 80
      ESP_LOGE("LGFX", "M5Tab5 need PSRAM SPEED 200MHz");
#if defined (ESP_ARDUINO_VERSION)
#if ESP_ARDUINO_VERSION == ESP_ARDUINO_VERSION_VAL(3,3,0)
#warning "The Arduino-ESP32 v3.3.0 has a problem that PSRAM does not work at 200MHz. Please use v3.3.1 or later or v3.2.x."
#endif
#endif
#endif

      {
        auto bus_dsi = &_bus_instance;
        auto bus_cfg = bus_dsi->config();
        bus_cfg.bus_id = 0;
        bus_cfg.lane_num = 2;
        bus_cfg.lane_mbps = 960;
        bus_cfg.ldo_chan_id = 3;
        bus_cfg.ldo_voltage_mv = 2500;
        bus_dsi->config(bus_cfg);
        if (bus_dsi->init()) {
          bool hit_ili9881 = false;
          bool hit_st7123 = false;
          lgfx::delay(80);
          for (int i = 0; i < 3; ++i) {
            uint8_t id[3] = { 0, };
            bus_dsi->readParams( 0xF4, id, 2 );
            ESP_LOGD("LGFX", "ST ID %02x %02x", id[0], id[1]);
            if (id[0] == 0x71 && id[1] == 0x23) {
              hit_st7123 = true;
              break;
            }
            static constexpr uint8_t params_page1[] = { 0x98, 0x81, 0x01 };
            bus_dsi->writeParams( 0xFF, params_page1, 3);
            bus_dsi->readParams( 0x00, &id[0], 1 );
            bus_dsi->readParams( 0x01, &id[1], 1 );
            bus_dsi->readParams( 0x02, &id[2], 1 );
            ESP_LOGD("LGFX", "ILI ID %02x %02x %02x", id[0], id[1], id[2]);
            if (id[0] == 0x98 && id[1] == 0x81) {
              static constexpr uint8_t params_page0[] = { 0x98, 0x81, 0x00 };
              bus_dsi->writeParams( 0xFF, params_page0, 3);
              hit_ili9881 = true;
              break;
            }
          }
          if (hit_ili9881) {
            auto p = new lgfx::Panel_ILI9881C();
            if (p == nullptr) { return false; }
            _panel_ptr = p;
            _touch_ptr = static_cast<lgfx::ITouch*>(new lgfx::Touch_GT911());
            auto det = p->config_detail();
            det.dpi_freq_mhz = 80;
            det.hsync_back_porch = 140;
            det.hsync_pulse_width = 40;
            det.hsync_front_porch = 40;
            det.vsync_back_porch = 20;
            det.vsync_pulse_width = 4;
            det.vsync_front_porch = 20;
            p->config_detail(det);
          } else if (hit_st7123) {
            auto p = new lgfx::Panel_ST7123();
            if (p == nullptr) { return false; }
            _panel_ptr = p;
            _touch_ptr = new lgfx::Touch_ST7123();
            auto det = p->config_detail();

            det.dpi_freq_mhz = 80;
            det.hsync_back_porch = 40;
            det.hsync_pulse_width = 2;
            det.hsync_front_porch = 40;
            // note: back + pulse == 10. If it is out of sync, the display position will shift vertically.
            det.vsync_back_porch = 8;
            det.vsync_pulse_width = 2;
            // note: reducing the front porch will cause the touch panel to stop working.
            det.vsync_front_porch = 220;
            p->config_detail(det);
          } else {
            return false;
          }
          setPanel(_panel_ptr);
          {
            auto p = _panel_ptr;
            auto cfg = p->config();
            cfg.memory_width = 720;
            cfg.memory_height = 1280;
            cfg.panel_width = 720;
            cfg.panel_height = 1280;
            cfg.readable = true;
            cfg.rgb_order = true;
            cfg.bus_shared = false;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.pin_cs = GPIO_NUM_NC;
            cfg.pin_rst = GPIO_NUM_NC;
            p->config(cfg);
            p->setBus(bus_dsi);
          }
          {
            auto t = _touch_ptr;
            auto cfg = t->config();

            cfg.pin_rst = -1;
            cfg.pin_sda = GPIO_NUM_31;
            cfg.pin_scl = GPIO_NUM_32;
            cfg.pin_int = GPIO_NUM_23;
            cfg.freq = 400000;
            cfg.x_min = 0;
            cfg.x_max = 719;
            cfg.y_min = 0;
            cfg.y_max = 1279;
            cfg.i2c_port = 1;
            cfg.bus_shared = false;
            cfg.offset_rotation = 0;
            t->config(cfg);
            _panel_ptr->setTouch(t);
          }
        }
        {
          auto cfg = _light_instance.config();
          cfg.pin_bl = GPIO_NUM_22;
          cfg.freq   = 44100;
          cfg.pwm_channel = 7;
          cfg.offset = 0;
          cfg.invert = false;
          _light_instance.config(cfg);
          _panel_ptr->setLight(&_light_instance);
        }
      }
    }
    return lgfx::LGFX_Device::init_impl(use_reset, use_clear);
  }

};

