#include "Bus_MIPI.hpp"

#if SOC_MIPI_DSI_SUPPORTED

#include "Panel_DSI.hpp"

#include <esp_check.h>
#include <esp_log.h>
#include <esp_ldo_regulator.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_mipi_dsi.h>
#include <esp_lcd_panel_commands.h>
#include <esp_lcd_panel_interface.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <stdint.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

  bool Bus_MIPI::init(void)
  {
    assert(_cfg.panel);

    if(_inited)
      return true;

    esp_ldo_channel_handle_t phy_pwr_chan = NULL;
    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_handle_t disp_panel = NULL;
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;

    esp_lcd_dsi_bus_config_t bus_config;
    bus_config.bus_id = 0;
    bus_config.num_data_lanes     = _cfg.dsi_num_data_lanes; // BSP_LCD_MIPI_DSI_LANE_NUM;
    bus_config.phy_clk_src        = MIPI_DSI_PHY_CLK_SRC_DEFAULT;
    bus_config.lane_bit_rate_mbps = _cfg.dsi_lane_bit_rate_mbps; // BSP_LCD_MIPI_DSI_LANE_BITRATE_MBPS;

    esp_ldo_channel_config_t ldo_cfg;
    memset(&ldo_cfg, 0, sizeof(ldo_cfg));
    ldo_cfg.chan_id    = _cfg.ldo_chan_id;// BSP_MIPI_DSI_PHY_PWR_LDO_CHAN;
    ldo_cfg.voltage_mv = _cfg.ldo_voltage_mv; // BSP_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV;

    esp_lcd_dbi_io_config_t dbi_config;
    dbi_config.virtual_channel = _cfg.dbi_virtual_channel; // 0;
    dbi_config.lcd_cmd_bits    = _cfg.dbi_lcd_cmd_bits; // 8;
    dbi_config.lcd_param_bits  = _cfg.dbi_lcd_param_bits; // 8;

    if (ESP_OK == esp_ldo_acquire_channel(&ldo_cfg, &phy_pwr_chan)
     && ESP_OK == esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus)
     && ESP_OK == esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &io))
    {
      _cfg.panel->_phy_pwr_chan_handle = phy_pwr_chan;
      _cfg.panel->_io_handle = io;
      _cfg.panel->_mipi_dsi_bus_handle = mipi_dsi_bus;

      esp_lcd_dpi_panel_config_t dpi_config;
      memset(&dpi_config, 0, sizeof(dpi_config));
      dpi_config.virtual_channel = 0;
      dpi_config.dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT;
      dpi_config.dpi_clock_freq_mhz = _cfg.dpi_clock_freq_mhz;
      dpi_config.pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB565;
      dpi_config.num_fbs = 1;
      dpi_config.video_timing.h_size = _cfg.panel->config().panel_width;
      dpi_config.video_timing.v_size = _cfg.panel->config().panel_height;
      dpi_config.video_timing.hsync_back_porch  = _cfg.hsync_back_porch;
      dpi_config.video_timing.hsync_pulse_width = _cfg.hsync_pulse_width;
      dpi_config.video_timing.hsync_front_porch = _cfg.hsync_front_porch;
      dpi_config.video_timing.vsync_back_porch  = _cfg.vsync_back_porch;
      dpi_config.video_timing.vsync_pulse_width = _cfg.vsync_pulse_width;
      dpi_config.video_timing.vsync_front_porch = _cfg.vsync_front_porch;
      dpi_config.flags.use_dma2d = true;

      ESP_LOGD("Bus_MIPI", "porch values: %d, %d, %d, %d, %d, %d", _cfg.hsync_back_porch, _cfg.hsync_pulse_width, _cfg.hsync_front_porch, _cfg.vsync_back_porch, _cfg.vsync_pulse_width, _cfg.vsync_front_porch);

      vendor_config_t vendor_config;
      vendor_config.init_cmds = _cfg.panel->getLcdInitCommands();
      vendor_config.init_cmds_size = _cfg.panel->getLcdInitCommandsize();
      vendor_config.mipi_config.dsi_bus = mipi_dsi_bus;
      vendor_config.mipi_config.dpi_config = &dpi_config;
      vendor_config.mipi_config.lane_num = _cfg.dsi_num_data_lanes;

      esp_lcd_panel_dev_config_t lcd_dev_config;
      memset(&lcd_dev_config, 0, sizeof(lcd_dev_config));
      lcd_dev_config.reset_gpio_num = _cfg.pin_rst;
      lcd_dev_config.rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB;
      lcd_dev_config.bits_per_pixel = 16;
      lcd_dev_config.vendor_config  = &vendor_config;

      auto callbacks = _cfg.panel->getCallbacks();

      if (ESP_OK == esp_lcd_new_panel(io, &lcd_dev_config, &callbacks, &disp_panel)) {
        _cfg.panel->_disp_panel_handle = disp_panel;
        if (ESP_OK == esp_lcd_panel_reset(disp_panel)) {
          if (ESP_OK == esp_lcd_panel_init(disp_panel)) {
            if (ESP_OK == esp_lcd_panel_disp_on_off(disp_panel, true)) {
              _inited = true;
              return true;
            }
          }
        }
      }
    }

    release();

    return false;
  }



  void Bus_MIPI::release(void)
  {

    if (_cfg.panel->_disp_panel_handle) {
      esp_lcd_panel_del(_cfg.panel->_disp_panel_handle);
    }
    if (_cfg.panel->_io_handle) {
      esp_lcd_panel_io_del(_cfg.panel->_io_handle);
    }
    if (_cfg.panel->_mipi_dsi_bus_handle) {
      esp_lcd_del_dsi_bus(_cfg.panel->_mipi_dsi_bus_handle);
    }
    if (_cfg.panel->_phy_pwr_chan_handle) {
        esp_ldo_release_channel(_cfg.panel->_phy_pwr_chan_handle);
    }

  }


  uint8_t* Bus_MIPI::getDMABuffer(uint32_t length)
  {
    return nullptr;
  }


//----------------------------------------------------------------------------
 }
}

#endif
