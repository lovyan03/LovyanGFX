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

#include <esp_lcd_mipi_dsi.h>
#include <esp_ldo_regulator.h>

#include "../../panel/Panel_FrameBufferBase.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_DSI : public Panel_FrameBufferBase
  {
  public:

    struct config_detail_t
    {
      void* buffer = nullptr;
      uint32_t buffer_length = 0;
    };

    bool init(bool use_reset) override;

    color_depth_t setColorDepth(color_depth_t depth) override;

    const config_detail_t& config_detail(void) const { return _config_detail; }
    void config_detail(const config_detail_t& config_detail) { _config_detail = config_detail; };

  protected:

    bool init_bus(void);
    bool init_panel(void);

    config_detail_t _config_detail;

    esp_ldo_channel_handle_t _phy_pwr_chan_handle = NULL;
    esp_lcd_panel_io_handle_t _io_handle = NULL;
    esp_lcd_panel_handle_t _disp_panel_handle = NULL;
    esp_lcd_dsi_bus_handle_t _mipi_dsi_bus_handle = NULL;
  };

//----------------------------------------------------------------------------
 }
}

#endif
#endif
