#pragma once

#if defined ( ESP_PLATFORM )
#include <sdkconfig.h>
#include <soc/efuse_reg.h>

#define LGFX_USE_V1
#include <lgfx/v1/panel/Panel_M5HDMI.hpp>
#include <LovyanGFX.hpp>

#ifndef M5MODULEDISPLAY_LOGICAL_WIDTH
#define M5MODULEDISPLAY_LOGICAL_WIDTH 1280
#endif
#ifndef M5MODULEDISPLAY_LOGICAL_HEIGHT
#define M5MODULEDISPLAY_LOGICAL_HEIGHT 720
#endif
#ifndef M5MODULEDISPLAY_REFRESH_RATE
#define M5MODULEDISPLAY_REFRESH_RATE 0.0f
#endif
#ifndef M5MODULEDISPLAY_OUTPUT_WIDTH
#define M5MODULEDISPLAY_OUTPUT_WIDTH 0
#endif
#ifndef M5MODULEDISPLAY_OUTPUT_HEIGHT
#define M5MODULEDISPLAY_OUTPUT_HEIGHT 0
#endif
#ifndef M5MODULEDISPLAY_SCALE_W
#define M5MODULEDISPLAY_SCALE_W 0
#endif
#ifndef M5MODULEDISPLAY_SCALE_H
#define M5MODULEDISPLAY_SCALE_H 0
#endif
#ifndef M5MODULEDISPLAY_PIXELCLOCK
#define M5MODULEDISPLAY_PIXELCLOCK 74250000
#endif

#ifndef M5MODULEDISPLAY_SPI_DMA_CH
 #if __has_include(<esp_idf_version.h>)
  #include <esp_idf_version.h>
  #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
   #define M5MODULEDISPLAY_SPI_DMA_CH SPI_DMA_CH_AUTO
  #endif
 #endif
 #ifndef M5MODULEDISPLAY_SPI_DMA_CH
  #define M5MODULEDISPLAY_SPI_DMA_CH 1
 #endif
#endif

class M5ModuleDisplay : public lgfx::LGFX_Device
{
  lgfx::Panel_M5HDMI _panel_instance;
  lgfx::Bus_SPI      _bus_instance;

public:

  M5ModuleDisplay( uint16_t logical_width  = M5MODULEDISPLAY_LOGICAL_WIDTH
                 , uint16_t logical_height = M5MODULEDISPLAY_LOGICAL_HEIGHT
                 , float refresh_rate      = M5MODULEDISPLAY_REFRESH_RATE
                 , uint16_t output_width   = M5MODULEDISPLAY_OUTPUT_WIDTH
                 , uint16_t output_height  = M5MODULEDISPLAY_OUTPUT_HEIGHT
                 , uint_fast8_t scale_w    = M5MODULEDISPLAY_SCALE_W
                 , uint_fast8_t scale_h    = M5MODULEDISPLAY_SCALE_H
                 , uint32_t pixel_clock    = M5MODULEDISPLAY_PIXELCLOCK
                 )
  {
    lgfx::Panel_M5HDMI::config_resolution_t cfg_reso;
    cfg_reso.logical_width  = logical_width;
    cfg_reso.logical_height = logical_height;
    cfg_reso.refresh_rate   = refresh_rate;
    cfg_reso.output_width   = output_width;
    cfg_reso.output_height  = output_height;
    cfg_reso.scale_w        = scale_w;
    cfg_reso.scale_h        = scale_h;
    cfg_reso.pixel_clock    = pixel_clock;
    _panel_instance.config_resolution(cfg_reso);
    _board = lgfx::board_t::board_M5ModuleDisplay;
  }

  bool init_impl(bool use_reset, bool use_clear) override
  {
#if defined (CONFIG_IDF_TARGET_ESP32S3)

    // for CoreS3
    int i2c_port = 1;
    int i2c_sda  = GPIO_NUM_12;
    int i2c_scl  = GPIO_NUM_11;
    int spi_cs   = GPIO_NUM_7;
    int spi_mosi = GPIO_NUM_37;
    int spi_miso = GPIO_NUM_35;
    int spi_sclk = GPIO_NUM_36;
    spi_host_device_t spi_host = SPI2_HOST;

#elif !defined (CONFIG_IDF_TARGET) || defined (CONFIG_IDF_TARGET_ESP32)

    int i2c_port = 1;
    int i2c_sda  = GPIO_NUM_21;
    int i2c_scl  = GPIO_NUM_22;
    int spi_cs   = GPIO_NUM_19;
    int spi_miso = GPIO_NUM_38;
    int spi_mosi = GPIO_NUM_23;
    int spi_sclk = GPIO_NUM_18;
    spi_host_device_t spi_host = VSPI_HOST;

    static constexpr const int axp192_addr = 0x34;
    if (0x03 == lgfx::i2c::readRegister8(1, axp192_addr, 0x03, 400000))
    { // M5Stack Core2 / Tough
#if defined ( ESP_LOGD )
      ESP_LOGD("LGFX","ModuleDisplay with Core2/Tough");
#endif
      // トランスミッタと通信できなければAXP192に外部5V出力を要求
      if (lgfx::i2c::beginTransaction(1, 0x39, 400000).has_error()
       || lgfx::i2c::endTransaction(1).has_error())
      {
        lgfx::i2c::writeRegister8(1, axp192_addr, 0x90, 0x02); // reg90h GPIO0 : enable=LDO
        lgfx::i2c::bitOn(1, axp192_addr, 0x12, 1 << 6);        // reg12h setExtEn
      }
    }
    else
    { // M5Stack BASIC / FIRE / GO
#if defined ( ESP_LOGD )
      ESP_LOGD("LGFX","ModuleDisplay with Core Basic/Fire/Go");
#endif
      i2c_port =  0;
      spi_cs   = GPIO_NUM_13;
      spi_miso = GPIO_NUM_19;
    }
#endif
    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = 80000000;
      cfg.freq_read  = 16000000;
      cfg.spi_host = spi_host;
      cfg.spi_mode = 3;
      cfg.use_lock = true;
      cfg.pin_mosi = spi_mosi;
      cfg.pin_miso = spi_miso;
      cfg.pin_sclk = spi_sclk;
      cfg.spi_3wire = false;
      cfg.dma_channel = M5MODULEDISPLAY_SPI_DMA_CH;

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
      auto cfg = _panel_instance.config();
      cfg.offset_rotation = 3;
      cfg.pin_cs     = spi_cs;
      cfg.readable   = false;
      cfg.bus_shared = false;
      _panel_instance.config(cfg);
      _panel_instance.setRotation(1);
    }

    setPanel(&_panel_instance);

    return LGFX_Device::init_impl(use_reset, use_clear);
  }

  bool setResolution( uint16_t logical_width  = M5MODULEDISPLAY_LOGICAL_WIDTH
                    , uint16_t logical_height = M5MODULEDISPLAY_LOGICAL_HEIGHT
                    , float refresh_rate      = M5MODULEDISPLAY_REFRESH_RATE
                    , uint16_t output_width   = M5MODULEDISPLAY_OUTPUT_WIDTH
                    , uint16_t output_height  = M5MODULEDISPLAY_OUTPUT_HEIGHT
                    , uint_fast8_t scale_w    = M5MODULEDISPLAY_SCALE_W
                    , uint_fast8_t scale_h    = M5MODULEDISPLAY_SCALE_H
                    , uint32_t pixel_clock    = M5MODULEDISPLAY_PIXELCLOCK
                    )
  {
    bool res = _panel_instance.setResolution
      ( logical_width
      , logical_height
      , refresh_rate
      , output_width
      , output_height
      , scale_w
      , scale_h
      , pixel_clock
      );
    setRotation(getRotation());
    return res;
  }
};

#endif
