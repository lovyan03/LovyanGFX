/**
 * @file LGFX_JC3636W518.hpp
 * @brief LovyanGFX board config for the Guition JC3636W518C (JC3636W518C_I_Y)
 *        1.85" 360x360 round display, ST77916 controller, QSPI, ESP32-S3.
 *
 * Pins taken from the manufacturer bundle (JC3636W518EN, pincfg.h).
 *
 * Backlight (TFT_BLK, GPIO15) on this board is driven by LEDC in the vendor
 * demo; Light_PWM here does the equivalent. If the panel lights but stays
 * black, that's a data/init issue, not backlight.
 */
#pragma once

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST77916 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;

public:
  LGFX(void)
  {
    { // --- QSPI bus ---
      auto cfg = _bus_instance.config();

      cfg.spi_host    = SPI2_HOST;
      cfg.spi_mode    = 0;
      cfg.freq_write  = 40000000UL;   // flightradar-proven 40 MHz
      cfg.freq_read   = 16000000UL;
      cfg.spi_3wire   = true;
      cfg.use_lock    = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;

      // Quad-SPI data lines (D0..D3). pin_mosi/miso stay unused in QSPI mode.
      cfg.pin_sclk = GPIO_NUM_9;
      cfg.pin_io0  = GPIO_NUM_11;   // TFT_SDA0
      cfg.pin_io1  = GPIO_NUM_12;   // TFT_SDA1
      cfg.pin_io2  = GPIO_NUM_13;   // TFT_SDA2
      cfg.pin_io3  = GPIO_NUM_14;   // TFT_SDA3

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // --- Panel ---
      auto cfg = _panel_instance.config();

      cfg.pin_cs   = GPIO_NUM_10;   // TFT_CS
      cfg.pin_rst  = GPIO_NUM_47;   // TFT_RST
      cfg.pin_busy = -1;

      cfg.panel_width   = 360;
      cfg.panel_height  = 360;
      cfg.memory_width  = 360;
      cfg.memory_height = 360;
      cfg.offset_x      = 0;
      cfg.offset_y      = 0;
      cfg.offset_rotation = 0;

      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable   = false;   // QSPI read path not implemented
      cfg.invert     = true;  // true=INVON, false=INVOFF; init seq sends 0x21 so flip here to correct colors
      cfg.rgb_order  = true;   // start false; flip if red/blue swap
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;

      _panel_instance.config(cfg);
    }

    { // --- Backlight ---
      auto cfg = _light_instance.config();
      cfg.pin_bl      = GPIO_NUM_15;   // TFT_BLK
      cfg.invert      = false;
      cfg.freq        = 5000;
      cfg.pwm_channel = 0;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};
