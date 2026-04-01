#pragma once

#include <LovyanGFX.hpp>


/*\
 *

  Description: Waveshare 3.5inch RPi LCD (A) hat
  Product URL: https://www.waveshare.com/wiki/3.5inch_RPi_LCD_(A)
  Platform:    ESP32 family
  Panel:       ILI9486
  Touch:       XPT2046
  Resolution:  320x480
  Bus Mode:    SPI0

 *
\*/




class LGFX_RPI_LDC35A : public lgfx::LGFX_Device
{
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Panel_ILI9486 _panel_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:

  LGFX_RPI_LDC35A(void)
  {

    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000; // glitching at 80MHz
      //cfg.freq_read  = 8000000; // TODO: test read support
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 0; // edit this
      cfg.pin_mosi = 3; // edit this
      cfg.pin_miso = 2; // edit this
      cfg.pin_dc   = 4; // edit this

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.readable = false;
      cfg.panel_width  = 320;
      cfg.panel_height = 480;
      cfg.dlen_16bit = true;
      cfg.bus_shared = true;
      cfg.offset_rotation = 1;
      cfg.pin_cs    = 36; // edit this
      cfg.pin_rst   = 1;  // edit this
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _touch_instance.config();
      auto buscfg = _bus_instance.config();
      cfg.spi_host = buscfg.spi_host;
      cfg.pin_sclk = buscfg.pin_sclk;
      cfg.pin_mosi = buscfg.pin_mosi;
      cfg.pin_miso = buscfg.pin_miso;
      cfg.pin_cs   = 32;  // edit this
      cfg.pin_int  = 21;  // edit this
      cfg.freq = 8000000; // don't go higher than 10MHz
      cfg.x_min = 400;
      cfg.x_max = 3900;
      cfg.y_min = 300;
      cfg.y_max = 3900;
      cfg.bus_shared = true;
      cfg.offset_rotation = 2;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }

};
