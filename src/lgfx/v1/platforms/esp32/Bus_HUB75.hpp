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

#if __has_include(<rom/lldesc.h>)
 #include <rom/lldesc.h>
#else
 #include <esp32/rom/lldesc.h>
#endif

#if __has_include(<freertos/FreeRTOS.h>)
 #include <freertos/FreeRTOS.h>
#endif

#if __has_include(<driver/i2s_std.h>)
 #include <driver/i2s_std.h>
#else
 #include <driver/i2s.h>
#endif

#include <soc/i2s_struct.h>

#include "../../Bus.hpp"
#include "../common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Bus_HUB75 : public IBus
  {
  public:
    struct config_t
    {
      union {
        i2s_port_t i2s_port = I2S_NUM_0;
        int port;
      };

      // 1秒間の表示更新回数 (この値に基づいて送信クロックが自動計算される)
      uint8_t refresh_rate = 60;

      union
      {
        int8_t pin_data[15] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, };
        struct
        {
          int8_t pin_r1;
          int8_t pin_g1;
          int8_t pin_b1;
          int8_t pin_r2;
          int8_t pin_g2;
          int8_t pin_b2;
          int8_t pin_lat;
          int8_t pin_clk;
          int8_t pin_addr_a;
          int8_t pin_addr_b;
          int8_t pin_addr_c;
          int8_t pin_addr_d;
          int8_t pin_addr_e;
          int8_t pin_addr_f;
          int8_t pin_oe;
        };
      };
    };

    const config_t& config(void) const { return _cfg; }
    void config(const config_t& config);

    bus_type_t busType(void) const override { return bus_type_t::bus_unknown; }

    bool init(void) override;
    void release(void) override;

    void beginTransaction(void) override;
    void endTransaction(void) override;
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
    void execDMAQueue(void) override {};
    uint8_t* getDMABuffer(uint32_t length) override { return nullptr; }

    void beginRead(void) override {}
    void endRead(void) override {}
    uint32_t readData(uint_fast8_t bit_length) override { return 0; }

    bool readBytes(uint8_t* dst, uint32_t length, bool use_dma) override { return false; }
    void readPixels(void* dst, pixelcopy_t* param, uint32_t length) override {}

    void setBrightness(uint8_t brightness);
    uint8_t getBrightness(void) const { return _brightness; }

    void setFrameBuffer(void* buf, color_depth_t depth, uint16_t panel_width, uint16_t panel_height) {
      _panel_width = panel_width;
      _panel_height = panel_height;
      _frame_buffer = buf;
      _frame_buffer_depth = depth;
    }

  private:

    static constexpr const uint32_t _mask_lat    = 0x00000040;
    static constexpr const uint32_t _mask_oe     = 0x40004000;
    static constexpr const uint32_t _mask_clock  = 0x00000080;
    static constexpr int32_t TRANSFER_PERIOD_COUNT = 8;
    static constexpr int32_t SHINE_PERIOD_COUNT = 34;
    static constexpr const int32_t TOTAL_PERIOD_COUNT = TRANSFER_PERIOD_COUNT + SHINE_PERIOD_COUNT;

    static void i2s_intr_handler_hub75(void *arg);

    config_t _cfg;

    int_fast16_t _panel_width = 64;
    int_fast16_t _panel_height = 32;

    uint32_t* _dma_buf[2] = { nullptr, nullptr };

    intr_handle_t _isr_handle = nullptr;

    void* _frame_buffer;
    color_depth_t _frame_buffer_depth;
    int _dma_y = 0;

    volatile void *_dev;

    lldesc_t* _dmadesc = nullptr;
    uint8_t _brightness = 128;
  };

//----------------------------------------------------------------------------
 }
}
