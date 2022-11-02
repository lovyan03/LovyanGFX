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

#if __has_include(<esp32/rom/lldesc.h>)
 #include <esp32/rom/lldesc.h>
#else
 #include <rom/lldesc.h>
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

#include "../common.hpp"
#include "../../Bus.hpp"
#include "../../misc/DividedFrameBuffer.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Bus_HUB75 : public Bus_ImagePush
  {
  public:
    struct config_t
    {
      union {
        i2s_port_t i2s_port = I2S_NUM_0;
        int port;
      };

      // 1秒間の表示更新回数 (この値に基づいて送信クロックが自動計算される)
      uint16_t refresh_rate = 120;

      union
      {
        int8_t pin_data[14] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  };
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
          int8_t pin_oe;
          int8_t pin_addr_a;
          int8_t pin_addr_b;
          int8_t pin_addr_c;
          int8_t pin_addr_d;
          int8_t pin_addr_e;
        };
      };
    };

    const config_t& config(void) const { return _cfg; }
    void config(const config_t& config);

    bool init(void) override;
    void release(void) override;

    void beginTransaction(void) override;
    void endTransaction(void) override;

    void flush(void) override {}

    void setBrightness(uint8_t brightness);
    uint8_t getBrightness(void) const { return _brightness; }

    void setImageBuffer(void* buffer) override;

  private:

    static constexpr int32_t TRANSFER_PERIOD_COUNT = 8;
    static constexpr int32_t LINECHANGE_PERIOD_COUNT = 1;
    static constexpr int32_t EXTEND_PERIOD_COUNT = 11;
    static constexpr const int32_t TOTAL_PERIOD_COUNT = TRANSFER_PERIOD_COUNT + LINECHANGE_PERIOD_COUNT + EXTEND_PERIOD_COUNT;
    static constexpr const uint32_t _mask_lat    = 0x00000040;
    static constexpr const uint32_t _mask_oe     = 0x01000100;
    static constexpr const uint32_t _mask_addr   = 0x3E003E00;
    static constexpr const uint32_t _mask_pin_a_clk = 0x00000200;
    static constexpr const uint32_t _mask_pin_c_dat = 0x08000800;
    static constexpr const uint32_t _mask_pin_b_lat = 0x04000400;

    static void i2s_intr_handler_hub75(void *arg);

    static uint32_t* _gamma_tbl;
    static uint8_t* _bitinvert_tbl;

    config_t _cfg;

    int_fast16_t _panel_width = 64;
    int_fast16_t _panel_height = 32;

    uint16_t* _dma_buf[2] = { nullptr, nullptr };

    uint16_t _light_period[TRANSFER_PERIOD_COUNT + 3];

    intr_handle_t _isr_handle = nullptr;

    DividedFrameBuffer* _frame_buffer;

    int _dma_y = 0;

    volatile void *_dev;

    lldesc_t* _dmadesc = nullptr;
    uint8_t _brightness = 128;
  };

//----------------------------------------------------------------------------
 }
}
