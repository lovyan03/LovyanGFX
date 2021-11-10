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

  M5AtomDisplay(uint16_t logical_width = 0, uint16_t logical_height = 0, uint16_t output_width = 0, uint16_t output_height = 0, uint_fast8_t x_scale = 0, uint_fast8_t y_scale = 0)
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
      static constexpr int X_SCALE_MAX = 16;
      static constexpr int Y_SCALE_MAX = 16;
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

      if (output_width == 0 && output_height == 0 && x_scale == 0 && y_scale == 0)
      {
        x_scale = 1;
        y_scale = 1;
        uint32_t prev_total = 0;
        float prev_aspect = INT_MAX;
        for (int ys = 1; ys <= Y_SCALE_MAX; ++ys)
        {
          uint32_t scale_height = ys * logical_height;
          if (scale_height > 1920)
          {
            break;
          }
          for (int xs = 1; xs <= X_SCALE_MAX; ++xs)
          {
            uint32_t scale_width = xs * logical_width;
            if (scale_width > 1920)
            {
              break;
            }
            uint32_t total = scale_width * scale_height;
            if (total <= 384000 || total > 1024000) continue;
            float aspect = fabsf(1.67f - (float)scale_width / scale_height);
            if (prev_total < total && prev_aspect >= aspect )
            {
              prev_total = total;
              prev_aspect = aspect + 0.001f;
              x_scale = xs;
              y_scale = ys;
            }
          }
        }
        output_width  = x_scale * logical_width;
        output_height = y_scale * logical_height;
      }
      else
      {
        if (output_width)
        {
          if (output_width  > 1920) { output_width  = 1920; }
          if (logical_width > output_width) { logical_width > output_width; }
        }
        if (output_height)
        {
          if (output_height > 1920) { output_height = 1920; }
          if (logical_height > output_height) { logical_height > output_height; }
        }

        if (y_scale == 0)
        {
          y_scale = output_height / logical_height;
        }
        if (y_scale > Y_SCALE_MAX) { y_scale = Y_SCALE_MAX; }

        if (x_scale == 0)
        {
          x_scale = output_width  / logical_width;
        }
        uint32_t w = output_width / x_scale;
        while (x_scale > X_SCALE_MAX || w * x_scale != output_width)
        {
          w = output_width / --x_scale;
        }
      }

      auto cfg = _panel_instance.config();
      cfg.memory_width     = output_width  ;
      cfg.memory_height    = output_height ;
      cfg.panel_width      = logical_width ;
      cfg.panel_height     = logical_height;
      cfg.offset_x         = (output_width  / x_scale - logical_width ) >> 1;
      cfg.offset_y         = (output_height / y_scale - logical_height) >> 1;
      cfg.offset_rotation  = 0;
      cfg.pin_cs       = spi_cs;
      cfg.readable   = false;
      cfg.bus_shared = false;

      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};
