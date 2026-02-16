// 2026-02-16 13:00 v2.2.1
//
// LGFX_ESP32_S3_Touch_LCD_2.h
//
// LovyanGFX device configuration for Waveshare ESP32-S3-Touch-LCD-2.
// Display: ST7789 240x320 IPS (SPI) | Touch: CST816S (I2C)
// MCU:     ESP32-S3
//
// Compatible with both Arduino framework and ESP-IDF.

#ifndef LGFX_ESP32_S3_TOUCH_LCD_2_H_
#define LGFX_ESP32_S3_TOUCH_LCD_2_H_

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 panel_instance_;
  lgfx::Bus_SPI bus_instance_;
  lgfx::Light_PWM light_instance_;
  lgfx::Touch_CST816S touch_instance_;

 public:
  LGFX(void) {
    // --- SPI bus ---
    {
      auto cfg = bus_instance_.config();
      cfg.spi_host = SPI3_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;    // 40 MHz write clock
      cfg.freq_read = 16000000;     // 16 MHz read clock
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 39;
      cfg.pin_mosi = 38;
      cfg.pin_miso = 40;
      cfg.pin_dc = 42;
      bus_instance_.config(cfg);
      panel_instance_.setBus(&bus_instance_);
    }

    // --- Display panel ---
    {
      auto cfg = panel_instance_.config();
      cfg.pin_cs = 45;
      cfg.pin_rst = -1;              // Not connected
      cfg.pin_busy = -1;             // Not used
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = true;             // Required for IPS panel
      cfg.rgb_order = false;         // RGB (not BGR)
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      panel_instance_.config(cfg);
    }

    // --- Backlight (PWM) ---
    {
      auto cfg = light_instance_.config();
      cfg.pin_bl = 1;
      cfg.invert = false;            // HIGH = bright
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      light_instance_.config(cfg);
      panel_instance_.setLight(&light_instance_);
    }

    // --- Touch (CST816S via I2C) ---
    {
      auto cfg = touch_instance_.config();
      cfg.i2c_port = 0;
      cfg.i2c_addr = 0x15;           // CST816S default address
      cfg.pin_sda = 48;
      cfg.pin_scl = 47;
      cfg.pin_int = -1;              // Not connected
      cfg.pin_rst = -1;              // Not connected
      cfg.freq = 400000;             // 400 kHz I2C clock
      cfg.x_min = 0;
      cfg.x_max = 239;
      cfg.y_min = 0;
      cfg.y_max = 319;
      cfg.offset_rotation = 0;
      touch_instance_.config(cfg);
      panel_instance_.setTouch(&touch_instance_);
    }

    setPanel(&panel_instance_);
  }
};

#endif  // LGFX_ESP32_S3_TOUCH_LCD_2_H_
