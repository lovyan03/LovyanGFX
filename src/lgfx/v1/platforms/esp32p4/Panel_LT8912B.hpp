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

#include "../../panel/Panel_FrameBufferBase.hpp"

#include "Bus_DSI.hpp"

#include <driver/i2c_master.h>
#include <esp_lcd_mipi_dsi.h>
#include <esp_lcd_panel_ops.h>
#include <freertos/semphr.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  class Bus_DSI;

  namespace detail
  {
    enum lt8912b_init_token_t : uint8_t
    {
      lt8912b_seq_main          = 0x00,
      lt8912b_seq_cec_dsi       = 0x01,
      lt8912b_seq_avi           = 0x02,
      lt8912b_seq_mipi_analog   = 0xF0,
      lt8912b_seq_mipi_basic    = 0xF1,
      lt8912b_seq_video_setup   = 0xF2,
      lt8912b_seq_detect_mipi   = 0xF3,
      lt8912b_seq_avi_infoframe = 0xF4,
      lt8912b_seq_mipi_rx_reset = 0xF5,
      lt8912b_seq_lvds_off      = 0xF6,
      lt8912b_seq_hdmi_on       = 0xF7,
      lt8912b_seq_dpi_init      = 0xF8,
      lt8912b_seq_end           = 0xFF,
    };
  }

  struct Panel_LT8912B : public Panel_FrameBufferBase
  {
  public:
    struct config_detail_t
    {
      uint16_t h_res = 1280;
      uint16_t v_res = 720;
      uint8_t refresh_rate = 60;

      // The bridge is reached through lgfx::i2c on this hardware port (software
      // I2C ports, negative numbers, are not supported here). When the port is
      // already open (e.g. the board's internal bus) it is used as it is and left
      // open; otherwise the panel opens it with these pins and closes it on release.
      // lgfx::i2c has no reference count: whoever opened the port has to outlive
      // the other users. An application that drives the bus through the ESP-IDF
      // driver at the same time should hand its handle over in i2c_master_bus
      // instead, since lgfx::i2c does not serialize with that driver.
      int i2c_port = 1;
      int i2c_sda = GPIO_NUM_0;
      int i2c_scl = GPIO_NUM_1;
      uint32_t i2c_freq = 100000;
      // Alternatively an ESP-IDF driver bus the application owns; lgfx::i2c is not used then.
      i2c_master_bus_handle_t i2c_master_bus = nullptr;

      uint8_t fb_num = 1;
      uint8_t lane_num = 2;
      bool use_draw_bitmap = false;

      color_depth_t output_depth = rgb888_nonswapped;
    };

    Panel_LT8912B(void) { _cfg.bus_shared = false; }
    ~Panel_LT8912B(void) override;

    bool init(bool use_reset) override;
    void waitDisplay(void) override;
    bool displayBusy(void) override { return false; }
    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

    color_depth_t setColorDepth(color_depth_t depth) override;
    void setSleep(bool flg_sleep) override;
    void setPowerSave(bool flg_idle) override { setSleep(flg_idle); }

    const config_detail_t& config_detail(void) const { return _config_detail; }
    void config_detail(const config_detail_t& config_detail) { _config_detail = config_detail; }

    esp_lcd_panel_handle_t getPanelHandle(void) const { return _panel_handle; }
    void* getFrameBuffer(size_t index = 0) const { return index < 3 ? _frame_buffers[index] : nullptr; }

    Bus_DSI* getBusDSI(void) const
    {
      auto b = getBus();
      return (b && b->busType() == bus_type_t::bus_dsi)
           ? static_cast<Bus_DSI*>(b)
           : nullptr;
    }

  private:
    bool get_i2c_bus(void);
    bool init_panel(Bus_DSI* bus);
    bool setup_framebuffer(void);
    void release_panel(void);

    static bool on_refresh_done(esp_lcd_panel_handle_t panel,
                                esp_lcd_dpi_panel_event_data_t* edata,
                                void* user_ctx);

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      using namespace detail;
      // Custom sequence: target, register, data length, data...; action tokens handle dynamic steps.
      static constexpr uint8_t list0[] = {
        lt8912b_seq_main,    0x02, 1, 0xF7,
        lt8912b_seq_main,    0x08, 1, 0xFF,
        lt8912b_seq_main,    0x09, 1, 0xFF,
        lt8912b_seq_main,    0x0A, 1, 0xFF,
        lt8912b_seq_main,    0x0B, 1, 0x7C,
        lt8912b_seq_main,    0x0C, 1, 0xFF,
        lt8912b_seq_main,    0x31, 1, 0xE1,
        lt8912b_seq_main,    0x32, 1, 0xE1,
        lt8912b_seq_main,    0x33, 1, 0x0C,
        lt8912b_seq_main,    0x37, 1, 0x00,
        lt8912b_seq_main,    0x38, 1, 0x22,
        lt8912b_seq_main,    0x60, 1, 0x82,
        lt8912b_seq_main,    0x39, 1, 0x45,
        lt8912b_seq_main,    0x3A, 1, 0x00,
        lt8912b_seq_main,    0x3B, 1, 0x00,
        lt8912b_seq_main,    0x44, 1, 0x31,
        lt8912b_seq_main,    0x55, 1, 0x44,
        lt8912b_seq_main,    0x57, 1, 0x01,
        lt8912b_seq_main,    0x5A, 1, 0x02,

        lt8912b_seq_mipi_analog,
        lt8912b_seq_mipi_basic,

        lt8912b_seq_cec_dsi, 0x4E, 1, 0x93,
        lt8912b_seq_cec_dsi, 0x4F, 1, 0x3E,
        lt8912b_seq_cec_dsi, 0x50, 1, 0x29,
        lt8912b_seq_cec_dsi, 0x51, 1, 0x80,
        lt8912b_seq_cec_dsi, 0x1E, 1, 0x4F,
        lt8912b_seq_cec_dsi, 0x1F, 1, 0x5E,
        lt8912b_seq_cec_dsi, 0x20, 1, 0x01,
        lt8912b_seq_cec_dsi, 0x21, 1, 0x2C,
        lt8912b_seq_cec_dsi, 0x22, 1, 0x01,
        lt8912b_seq_cec_dsi, 0x23, 1, 0xFA,
        lt8912b_seq_cec_dsi, 0x24, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x25, 1, 0xC8,
        lt8912b_seq_cec_dsi, 0x26, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x27, 1, 0x5E,
        lt8912b_seq_cec_dsi, 0x28, 1, 0x01,
        lt8912b_seq_cec_dsi, 0x29, 1, 0x2C,
        lt8912b_seq_cec_dsi, 0x2A, 1, 0x01,
        lt8912b_seq_cec_dsi, 0x2B, 1, 0xFA,
        lt8912b_seq_cec_dsi, 0x2C, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x2D, 1, 0xC8,
        lt8912b_seq_cec_dsi, 0x2E, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x42, 1, 0x64,
        lt8912b_seq_cec_dsi, 0x43, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x44, 1, 0x04,
        lt8912b_seq_cec_dsi, 0x45, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x46, 1, 0x59,
        lt8912b_seq_cec_dsi, 0x47, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x48, 1, 0xF2,
        lt8912b_seq_cec_dsi, 0x49, 1, 0x06,
        lt8912b_seq_cec_dsi, 0x4A, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x4B, 1, 0x72,
        lt8912b_seq_cec_dsi, 0x4C, 1, 0x45,
        lt8912b_seq_cec_dsi, 0x4D, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x52, 1, 0x08,
        lt8912b_seq_cec_dsi, 0x53, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x54, 1, 0xB2,
        lt8912b_seq_cec_dsi, 0x55, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x56, 1, 0xE4,
        lt8912b_seq_cec_dsi, 0x57, 1, 0x0D,
        lt8912b_seq_cec_dsi, 0x58, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x59, 1, 0xE4,
        lt8912b_seq_cec_dsi, 0x5A, 1, 0x8A,
        lt8912b_seq_cec_dsi, 0x5B, 1, 0x00,
        lt8912b_seq_cec_dsi, 0x5C, 1, 0x34,
        lt8912b_seq_cec_dsi, 0x51, 1, 0x00,

        lt8912b_seq_video_setup,
        lt8912b_seq_detect_mipi,
        lt8912b_seq_video_setup,
        lt8912b_seq_avi_infoframe,
        lt8912b_seq_mipi_rx_reset,

        lt8912b_seq_main,    0xB2, 1, 0x01,
        lt8912b_seq_avi,     0x06, 1, 0x08,
        lt8912b_seq_avi,     0x07, 1, 0xF0,
        lt8912b_seq_avi,     0x34, 1, 0xD2,
        lt8912b_seq_avi,     0x0F, 1, 0x2B,

        lt8912b_seq_main,    0x44, 1, 0x30,
        lt8912b_seq_main,    0x51, 1, 0x05,
        lt8912b_seq_main,    0x50, 1, 0x24,
        lt8912b_seq_main,    0x51, 1, 0x2D,
        lt8912b_seq_main,    0x52, 1, 0x04,
        lt8912b_seq_main,    0x69, 1, 0x0E,
        lt8912b_seq_main,    0x69, 1, 0x8E,
        lt8912b_seq_main,    0x6A, 1, 0x00,
        lt8912b_seq_main,    0x6C, 1, 0xB8,
        lt8912b_seq_main,    0x6B, 1, 0x51,
        lt8912b_seq_main,    0x04, 1, 0xFB,
        lt8912b_seq_main,    0x04, 1, 0xFF,
        lt8912b_seq_main,    0x7F, 1, 0x00,
        lt8912b_seq_main,    0xA8, 1, 0x13,

        lt8912b_seq_lvds_off,
        lt8912b_seq_hdmi_on,
        lt8912b_seq_dpi_init,
        lt8912b_seq_end,
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }

    config_detail_t _config_detail;

    esp_lcd_panel_handle_t _panel_handle = nullptr;
    esp_lcd_panel_io_handle_t _io_main = nullptr;
    esp_lcd_panel_io_handle_t _io_cec = nullptr;
    esp_lcd_panel_io_handle_t _io_avi = nullptr;
    i2c_master_bus_handle_t _i2c_bus = nullptr;
    bool _i2c_port_owned = false; // this panel opened the lgfx::i2c port below and closes it on release
    int _i2c_port_opened = 0;
    void* _frame_buffers[3] = {};
    SemaphoreHandle_t _refresh_done_sem = nullptr;
  };

//----------------------------------------------------------------------------
 }
}

#endif
#endif
