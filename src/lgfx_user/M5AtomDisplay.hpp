#pragma once

#include <esp_efuse.h>
#include <soc/efuse_reg.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/panel/Panel_M5HDMI.hpp>

class M5AtomDisplay : public lgfx::LGFX_Device
{
  lgfx::Panel_M5HDMI _panel_instance;
  lgfx::Bus_SPI      _bus_instance;

public:

  M5AtomDisplay(uint16_t logical_width = 0, uint16_t logical_height = 0, float refresh_rate = 60.0f, uint16_t output_width = 0, uint16_t output_height = 0, uint_fast8_t scale_w = 0, uint_fast8_t scale_h = 0)
  {
    static constexpr int i2c_port =  1;
    static constexpr int i2c_sda  = 25;
    static constexpr int i2c_scl  = 21;
    static constexpr int spi_cs   = 33;
    static constexpr int spi_mosi = 19;
    static constexpr int spi_miso = 22;

    int spi_sclk = (esp_efuse_get_pkg_ver() == EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4)
                 ? 23  // for ATOM Lite / Matrix
                 : 5   // for ATOM PSRAM
                 ;
    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = 80000000;
      cfg.freq_read  = 20000000;
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 3;
      cfg.dma_channel = 1;
      cfg.use_lock = true;
      cfg.pin_mosi = spi_mosi;
      cfg.pin_miso = spi_miso;
      cfg.pin_sclk = spi_sclk;
      cfg.spi_3wire = false;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config_transmitter();
      cfg.freq_read = 400000;
      cfg.freq_write = 400000;
      cfg.pin_scl = i2c_scl;
      cfg.pin_sda = i2c_sda;
      cfg.i2c_port = i2c_port;
      cfg.i2c_addr = 0x39;
      cfg.prefix_cmd = 0x00;
      cfg.prefix_data = 0x00;
      cfg.prefix_len = 0;
      _panel_instance.config_transmitter(cfg);
    }

    {
      static constexpr int SCALE_MAX = 16;
      static constexpr int RANGE_MAX = 2048;

      if (refresh_rate < 8.0f) { refresh_rate = 8.0f; }
      if (refresh_rate > 256.0f) { refresh_rate = 256.0f; }

      if (output_width)
      {
        if (output_width  > RANGE_MAX) { output_width  = RANGE_MAX; }
        if (logical_width > output_width) { logical_width = output_width; }
      }
      if (output_height)
      {
        if (output_height > RANGE_MAX) { output_height = RANGE_MAX; }
        if (logical_height > output_height) { logical_height = output_height; }
      }

      if (logical_width == 0)
      {
        if (logical_height == 0)
        {
          logical_width  = 1280;
          logical_height = 720;
        }
        else
        {
          logical_width = (logical_height << 4) / 9;
        }
      }
      else
      if (logical_height == 0)
      {
        logical_height = (logical_width * 9) >> 4;
      }
      if (logical_width  > RANGE_MAX) { logical_width  = RANGE_MAX; }
      if (logical_height > RANGE_MAX) { logical_height = RANGE_MAX; }

      int limit = 55296000 / refresh_rate;

      if (output_width == 0 && output_height == 0 && scale_w == 0 && scale_h == 0)
      {
        scale_w = 1;
        scale_h = 1;
        for (int scale = 2; scale <= SCALE_MAX; ++scale)
        {
          uint32_t scale_height = scale * logical_height;
          uint32_t scale_width = scale * logical_width;
          uint32_t total = scale_width * scale_height;
          if (scale_width > 1920 || scale_height > 1920 || total >= limit) { break; }
          scale_w = scale;
          scale_h = scale;
        }
        output_width  = scale_w * logical_width;
        output_height = scale_h * logical_height;
        if (output_height & 1) { output_height++; }
        if ((output_width & 1) && (scale_w & 1)) { output_width += scale_w; }
      }
      else
      {
        if (scale_h == 0)
        {
          scale_h = output_height / logical_height;
        }
        if (scale_h > SCALE_MAX) { scale_h = SCALE_MAX; }
        while (logical_height * scale_h > output_height) { --scale_h; }

        if (scale_w == 0)
        {
          scale_w = output_width  / logical_width;
        }
        uint32_t w = output_width / scale_w;
        while (scale_w > SCALE_MAX || w * scale_w != output_width || logical_width * scale_w > output_width)
        {
          w = output_width / --scale_w;
        }
      }

      auto cfg = _panel_instance.config();
      cfg.memory_width     = output_width  ;
      cfg.memory_height    = output_height ;
      cfg.panel_width      = logical_width ;
      cfg.panel_height     = logical_height;
      cfg.offset_x         = (output_width  / scale_w - logical_width ) >> 1;
      cfg.offset_y         = (output_height / scale_h - logical_height) >> 1;
      cfg.offset_rotation  = 0;
      cfg.pin_cs       = spi_cs;
      cfg.readable   = false;
      cfg.bus_shared = false;

      _panel_instance.config(cfg, scale_w, scale_h, refresh_rate);
    }
    setPanel(&_panel_instance);
  }
};
