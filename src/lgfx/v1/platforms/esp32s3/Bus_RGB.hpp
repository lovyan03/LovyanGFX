/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#if __has_include (<esp_lcd_panel_rgb.h>)
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_interface.h>


#include <esp_private/gdma.h>
#include <hal/dma_types.h>

#include "../../Bus.hpp"
#include "../../panel/Panel_FrameBufferBase.hpp"
#include "../common.hpp"

struct lcd_cam_dev_t;
struct esp_rgb_panel_t;

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Bus_RGB : public IBus
  {
  public:
    struct config_t
    {
      Panel_FrameBufferBase* panel = nullptr;

      // LCD_CAM peripheral number. No need to change (only 0 for ESP32-S3.)
      int8_t port = 0;

      // pixel clock
      uint32_t freq_write = 16000000;

      int8_t pin_pclk = -1;
      int8_t pin_vsync = -1;
      int8_t pin_hsync = -1;
      int8_t pin_henable = -1;
      union
      {
        int8_t pin_data[16];
        struct
        {
          int8_t pin_d0;
          int8_t pin_d1;
          int8_t pin_d2;
          int8_t pin_d3;
          int8_t pin_d4;
          int8_t pin_d5;
          int8_t pin_d6;
          int8_t pin_d7;
          int8_t pin_d8;
          int8_t pin_d9;
          int8_t pin_d10;
          int8_t pin_d11;
          int8_t pin_d12;
          int8_t pin_d13;
          int8_t pin_d14;
          int8_t pin_d15;
        };
      };

      int8_t hsync_pulse_width = 0;
      int8_t hsync_back_porch = 0;
      int8_t hsync_front_porch = 0;
      int8_t vsync_pulse_width = 0;
      int8_t vsync_back_porch = 0;
      int8_t vsync_front_porch = 0;
      bool hsync_polarity = 0;
      bool vsync_polarity = 0;
      bool pclk_active_neg = 1;
      bool de_idle_high = 0;
      bool pclk_idle_high = 0;
    };

    const config_t& config(void) const { return _cfg; }
    void config(const config_t& config);

    bus_type_t busType(void) const override { return bus_type_t::bus_unknown; }

    bool init(void) override;
    void release(void) override;

    void beginTransaction(void) override {}
    void endTransaction(void) override {}
    void wait(void) override {}
    bool busy(void) const override { return false; }

    void flush(void) override {}
    bool writeCommand(uint32_t data, uint_fast8_t bit_length) override { return true; }
    void writeData(uint32_t data, uint_fast8_t bit_length) override {}
    void writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t count) override {}
    void writePixels(pixelcopy_t* param, uint32_t length) override {}
    void writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma) override {}

    void initDMA(void) override {}
    void addDMAQueue(const uint8_t* data, uint32_t length) override {}
    void execDMAQueue(void) override {}
    uint8_t* getDMABuffer(uint32_t length) override;

    void beginRead(void) override {}
    void endRead(void) override {}
    uint32_t readData(uint_fast8_t bit_length) override { return 0; }
    bool readBytes(uint8_t* dst, uint32_t length, bool use_dma) override { return false; }
    void readPixels(void* dst, pixelcopy_t* param, uint32_t length) override {}

  private:
    config_t _cfg;

    dma_descriptor_t _dmadesc_restart;
    dma_descriptor_t* _dmadesc = nullptr;
    esp_lcd_i80_bus_handle_t _i80_bus = nullptr;
    int32_t _dma_ch;

    esp_lcd_panel_handle_t _panel_handle = nullptr;

    uint8_t *_frame_buffer = nullptr;
    intr_handle_t _intr_handle;
    static void lcd_default_isr_handler(void *args);
  };

//----------------------------------------------------------------------------
 }
}
#endif