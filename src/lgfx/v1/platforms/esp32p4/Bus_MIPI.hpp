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

#if __has_include(<soc/soc_caps.h>)
#include <soc/soc_caps.h>
#if SOC_MIPI_DSI_SUPPORTED

#include "../../Bus.hpp"
#include "../../panel/Panel_FrameBufferBase.hpp"
#include "../common.hpp"

#include <esp_ldo_regulator.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_mipi_dsi.h>
#include <stdint.h>

namespace lgfx
{
 inline namespace v1
 {
#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

  struct Panel_DSI;

  class Bus_MIPI : public IBus
  {
  public:
    struct config_t
    {
      Panel_DSI* panel = nullptr;

      int16_t pin_rst = -1;

      uint8_t dsi_num_data_lanes      = 2;
      uint32_t dsi_lane_bit_rate_mbps = 730; // mpbs

      int ldo_chan_id     = 3;
      int ldo_voltage_mv = 2500;

      uint8_t dbi_virtual_channel = 0;
      int dbi_lcd_cmd_bits    = 8;
      int dbi_lcd_param_bits  = 8;

      uint16_t dpi_clock_freq_mhz = 60;

      uint32_t hsync_pulse_width = 0;
      uint32_t hsync_back_porch  = 0;
      uint32_t hsync_front_porch = 0;
      uint32_t vsync_pulse_width = 0;
      uint32_t vsync_back_porch  = 0;
      uint32_t vsync_front_porch = 0;
    };

    const config_t& config(void) const { return _cfg; }
    void config(const config_t& config) { _cfg=config; }

    bus_type_t busType(void) const override { return bus_type_t::bus_unknown; }

    bool init(void) override; // <<<<<<<<<
    void release(void) override; // <<<<<<<<<

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

    uint8_t* getDMABuffer(uint32_t length);  // <<<<<<<<<  // <<<<<<<<<  // <<<<<<<<<

    void beginRead(void) override {}
    void endRead(void) override {}
    uint32_t readData(uint_fast8_t bit_length) override { return 0; }
    bool readBytes(uint8_t* dst, uint32_t length, bool use_dma) override { return false; }
    void readPixels(void* dst, pixelcopy_t* param, uint32_t length) override {}

  private:

    config_t _cfg;
    bool _inited = false;
  };

//----------------------------------------------------------------------------
 }
}

#endif
#endif
