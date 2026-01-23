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
#if defined (ESP_PLATFORM)
#include <sdkconfig.h>
#if defined (CONFIG_IDF_TARGET_ESP32P4)
#include "Bus_DSI.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Bus_DSI::init(void)
  {
    if (_mipi_dsi_bus) {
      return true;
    }
    esp_lcd_dsi_bus_config_t bus_config;
    bus_config.bus_id = _cfg.bus_id;
    bus_config.num_data_lanes = _cfg.lane_num;
    bus_config.phy_clk_src = static_cast<typeof(bus_config.phy_clk_src)>(MIPI_DSI_PHY_CLK_SRC_DEFAULT);
    bus_config.lane_bit_rate_mbps = _cfg.lane_mbps;

    esp_ldo_channel_config_t ldo_cfg;
    memset(&ldo_cfg, 0, sizeof(ldo_cfg));
    ldo_cfg.chan_id = _cfg.ldo_chan_id;
    ldo_cfg.voltage_mv = _cfg.ldo_voltage_mv;

    esp_lcd_dbi_io_config_t dbi_config;
    dbi_config.virtual_channel = 0;
    dbi_config.lcd_cmd_bits = _cfg.lcd_cmd_bits;
    dbi_config.lcd_param_bits = _cfg.lcd_param_bits;

    if (ESP_OK == esp_ldo_acquire_channel(&ldo_cfg, &_phy_pwr_chan)
     && ESP_OK == esp_lcd_new_dsi_bus(&bus_config, &_mipi_dsi_bus)
     && ESP_OK == esp_lcd_new_panel_io_dbi(_mipi_dsi_bus, &dbi_config, &_io_dbi))
    {
      return true;
    }
    release();
    return false;
  }

  void Bus_DSI::release(void)
  {
    if (_io_dbi) {
      esp_lcd_panel_io_del(_io_dbi);
      _io_dbi = nullptr;
    }
    if (_mipi_dsi_bus) {
      esp_lcd_del_dsi_bus(_mipi_dsi_bus);
      _mipi_dsi_bus = nullptr;
    }
    if (_phy_pwr_chan) {
      esp_ldo_release_channel(_phy_pwr_chan);
      _phy_pwr_chan = nullptr;
    }
  }

  bool Bus_DSI::writeParams(uint32_t cmd, const uint8_t* param, size_t param_length)
  {
    return (_io_dbi != nullptr) && (ESP_OK == esp_lcd_panel_io_tx_param(_io_dbi, cmd, param, param_length));
  }

  bool Bus_DSI::readParams(uint32_t cmd, uint8_t* param, size_t param_length)
  {
    return (_io_dbi != nullptr) && (ESP_OK == esp_lcd_panel_io_rx_param(_io_dbi, cmd, param, param_length));
  }

//----------------------------------------------------------------------------
 }
}
//----------------------------------------------------------------------------

#endif
#endif
