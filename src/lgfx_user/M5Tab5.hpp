
#include <LovyanGFX.hpp>

#if !defined (CONFIG_IDF_TARGET_ESP32P4)
  #error "CONFIG_IDF_TARGET_ESP32P4 should be set"
#endif

#include "lgfx/v1/platforms/esp32p4/Panel_ILI9881C.hpp"


class Tab5Display : public lgfx::LGFX_Device
{
  lgfx::Bus_MIPI       _bus_instance;
  lgfx::Panel_ILI9881C _panel_instance; // NOTE: this panel does not use IBus
  lgfx::Light_PWM      _light_instance;
  lgfx::Touch_GT911    _touch_instance;


protected:

  // M5Tab5 io multiplexer addresses
  static constexpr std::uint_fast8_t pi4io1_i2c_addr = 0x43;
  static constexpr std::uint_fast8_t pi4io2_i2c_addr = 0x44;

  // M5Tab5 io multiplexer registers
  static constexpr const uint8_t reg_data_io1_1[] = {
    0x03, 0b01111111, 0,   // PI4IO_REG_IO_DIR
    0x05, 0b01010110, 0,   // PI4IO_REG_OUT_SET (bit5=GT911 TouchReset LOW)
    0x07, 0b00000000, 0,   // PI4IO_REG_OUT_H_IM
    0x0D, 0b01111111, 0,   // PI4IO_REG_PULL_SEL
    0x0B, 0b01111111, 0,   // PI4IO_REG_PULL_EN
    0xFF,0xFF,0xFF,
  };
  static constexpr const uint8_t reg_data_io1_2[] = {
    0x05, 0b01110110, 0,   // PI4IO_REG_OUT_SET (bit5=GT911 TouchReset HIGH)
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

  void i2c_write_register8_array(int_fast16_t i2c_port, uint_fast8_t i2c_addr, const uint8_t* reg_data_mask, uint32_t freq)
  {
    while (reg_data_mask[0] != 0xFF || reg_data_mask[1] != 0xFF || reg_data_mask[2] != 0xFF)
    {
      lgfx::i2c::writeRegister8(i2c_port, i2c_addr, reg_data_mask[0], reg_data_mask[1], reg_data_mask[2], freq);
      reg_data_mask += 3;
    }
  }


public:

  Tab5Display()
  {
    std::uint32_t pkg_ver = lgfx::get_pkg_ver();
    ESP_LOGD("LGFX", "pkg_ver : %02x", (int)pkg_ver);

    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;
      cfg.dsi_num_data_lanes     = 2;
      cfg.dsi_lane_bit_rate_mbps = 730; // mpbs

      cfg.ldo_chan_id    = 3;
      cfg.ldo_voltage_mv = 2500;

      cfg.dbi_virtual_channel = 0;
      cfg.dbi_lcd_cmd_bits    = 8;
      cfg.dbi_lcd_param_bits  = 8;

      cfg.dpi_clock_freq_mhz = 60;

      cfg.hsync_back_porch  = 140;
      cfg.hsync_pulse_width = 40;
      cfg.hsync_front_porch = 40;
      cfg.vsync_back_porch  = 20;
      cfg.vsync_pulse_width = 4;
      cfg.vsync_front_porch = 20;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }


    {
      auto cfg = _panel_instance.config();
      cfg.panel_width   = 720;
      cfg.panel_height  = 1280;
      cfg.memory_width  = 720;
      cfg.memory_height = 1280;
      cfg.readable = true;
      _panel_instance.config(cfg);
    }


    {
      auto cfg = _touch_instance.config();
      cfg.pin_rst = -1;
      cfg.pin_sda = GPIO_NUM_31;
      cfg.pin_scl = GPIO_NUM_32;
      cfg.pin_int = GPIO_NUM_23;
      cfg.freq  = 400000;
      cfg.x_min = 0;
      cfg.x_max = 719;
      cfg.y_min = 0;
      cfg.y_max = 1279;
      cfg.i2c_port = 1;
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }


    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = GPIO_NUM_22 ;  // Backlight pin
      cfg.pwm_channel = 7;
      cfg.invert = false;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }


  bool init()
  {
    static constexpr int_fast16_t in_i2c_port = 1;// I2C_NUM_1;

    lgfx::pinMode(GPIO_NUM_23, lgfx::pin_mode_t::output); // TP INT
    lgfx::gpio_hi(GPIO_NUM_23); // select I2C Addr (high=0x14 / low=0x5D)
    lgfx::i2c::init(in_i2c_port, GPIO_NUM_31, GPIO_NUM_32);

    std::uint32_t id = lgfx::i2c::readRegister8(in_i2c_port, pi4io1_i2c_addr, 0x01).has_value()
      && lgfx::i2c::readRegister8(in_i2c_port, pi4io2_i2c_addr, 0x01).has_value();

    if( id == 0 )
    {
      ESP_LOGE("LGFX", "M5Tab5 detection failed");
      return false;
    }

    ESP_LOGI("LGFX", "M5Tab5 detected: 0x%x", id);

    i2c_write_register8_array(in_i2c_port, pi4io1_i2c_addr, reg_data_io1_1, 400000);
    i2c_write_register8_array(in_i2c_port, pi4io2_i2c_addr, reg_data_io2, 400000);
    i2c_write_register8_array(in_i2c_port, pi4io1_i2c_addr, reg_data_io1_2, 400000);

    return lgfx::LGFX_Device::init();
  }

};

