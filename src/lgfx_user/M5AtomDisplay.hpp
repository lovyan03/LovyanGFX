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

  M5AtomDisplay(uint16_t panel_width = 1280, uint16_t panel_height = 720)
  {
    int i2c_port =  1;
    int i2c_sda  = 25;
    int i2c_scl  = 21;
    int spi_cs   = 33;
    int spi_mosi = 19;
    int spi_miso = 22;

    int spi_sclk =  5; // for ATOM PSRAM
    if (esp_efuse_get_pkg_ver() == EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4)
    {
      spi_sclk = 23;  // for ATOM Lite / Matrix
    }

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
      static constexpr uint32_t max_width  = 1280;
      static constexpr uint32_t max_height =  720;
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = spi_cs;

      panel_width  = (panel_width  && panel_width  < max_width ) ? panel_width  : max_width;
      panel_height = (panel_height && panel_height < max_height) ? panel_height : max_height;
      cfg.panel_width  = panel_width ;
      cfg.panel_height = panel_height;

      uint32_t x_scale = max_width  / panel_width;
      uint32_t w = max_width / x_scale;
      while (w * x_scale != max_width)
      {
        w = max_width / --x_scale;
      }
      uint32_t y_scale = max_height / panel_height;
      uint32_t h = max_height / y_scale;

      cfg.offset_x         = (w - panel_width ) >> 1;
      cfg.offset_y         = (h - panel_height) >> 1;
      cfg.offset_rotation  = 0;
      cfg.readable   = false;
      cfg.bus_shared = false;

      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};
