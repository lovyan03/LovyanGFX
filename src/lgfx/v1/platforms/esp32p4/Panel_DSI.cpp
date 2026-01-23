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

#include "Panel_DSI.hpp"

#if SOC_MIPI_DSI_SUPPORTED

#include "../common.hpp"

#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_io.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  bool Panel_DSI::init_panel(void)
  {
    auto bus = getBusDSI();
    if (bus == nullptr) { return false; }

    const uint8_t* params;
    for (size_t i = 0; nullptr != (params = getInitParams(i)); ++i)
    {
      size_t len;
      while (0 != (len = params[0])) {
// printf("cmd: %02x, len: %d\n", params[1], len);
        bus->writeParams(params[1], &params[2], len - 1);
        params += len + 1;
      }
      vTaskDelay(pdMS_TO_TICKS(getInitDelay(i)));
    }

    uint8_t madctl_val = 0;
    if (_cfg.rgb_order == false) {
      madctl_val = 1<<3; //LCD_CMD_BGR_BIT;
    }

    uint8_t colmod_val = 0x55;
    if (_write_bits >= 24) {
      colmod_val = 0x77;
    }

    bus->writeParams(CMD_MADCTL, &(madctl_val), 1);
    bus->writeParams(CMD_COLMOD, &(colmod_val), 1);

    return (ESP_OK == esp_lcd_panel_init(_disp_panel_handle));
  }


  bool Panel_DSI::init_dpi(Bus_DSI* bus)
  {
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = bus->getMipiDsiBus();

    esp_lcd_dpi_panel_config_t dpi_config;
    memset(&dpi_config, 0, sizeof(dpi_config));
    dpi_config.virtual_channel = 0;
    dpi_config.dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT;
    dpi_config.dpi_clock_freq_mhz = _config_detail.dpi_freq_mhz;
    dpi_config.pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB565;
    dpi_config.num_fbs = 1;
    dpi_config.video_timing.h_size = _cfg.panel_width;
    dpi_config.video_timing.v_size = _cfg.panel_height;
    dpi_config.video_timing.hsync_back_porch  = _config_detail.hsync_back_porch;
    dpi_config.video_timing.hsync_pulse_width = _config_detail.hsync_pulse_width;
    dpi_config.video_timing.hsync_front_porch = _config_detail.hsync_front_porch;
    dpi_config.video_timing.vsync_back_porch  = _config_detail.vsync_back_porch;
    dpi_config.video_timing.vsync_pulse_width = _config_detail.vsync_pulse_width;
    dpi_config.video_timing.vsync_front_porch = _config_detail.vsync_front_porch;
    dpi_config.flags.use_dma2d = true;

    return ESP_OK == esp_lcd_new_panel_dpi(mipi_dsi_bus, &dpi_config, &_disp_panel_handle);
  }

  color_depth_t Panel_DSI::setColorDepth(color_depth_t depth)
  {
    _write_depth = color_depth_t::rgb565_nonswapped;
    _read_depth = color_depth_t::rgb565_nonswapped;
    return color_depth_t::rgb565_nonswapped;
  }

  bool Panel_DSI::init(bool use_reset)
  {
    if (_lines_buffer != nullptr) return false;

    if (!Panel_FrameBufferBase::init(use_reset))
    {
      return false;
    }

    auto bus = getBusDSI();
    if (bus == nullptr) { return false; }
    if (init_dpi(bus) && init_panel())
    {
        esp_lcd_dpi_panel_get_frame_buffer(_disp_panel_handle, 1, &(_config_detail.buffer));
    }

    auto ptr = (uint8_t*)_config_detail.buffer;
// printf ("ptr : %08x \n", (uintptr_t)ptr);
    if (ptr == nullptr) { return false; }


//--------------------------------------------------------------------------------
    const auto height = _cfg.panel_height;
// printf ("panel_width  : %d \n", _cfg.panel_width );
// printf ("panel_height : %d \n", _cfg.panel_height);
    size_t la_size = height * sizeof(void*);
    uint8_t** lineArray = (uint8_t**)heap_alloc_dma(la_size);

// printf ("lineArray ptr : %08x \n", (uintptr_t)lineArray);
    if ( nullptr == lineArray ) { return false; }
    memset(lineArray, 0, la_size);

    const size_t line_length = ((_cfg.panel_width * _write_bits >> 3) + 3) & ~3;

    _lines_buffer = lineArray;

    for (int y = 0; y < height; y ++)
    {
      lineArray[y] = ptr;
      ptr = ptr + line_length;
    }

    return true;
  }

//----------------------------------------------------------------------------

  bool Panel_DSI::write_params(uint32_t cmd, const uint8_t* data, size_t length)
  {
    bool res = false;
    auto b = getBusDSI();
    if (b) {
      startWrite();
      res = b->writeParams(cmd, data, length);
      b->flush();
      endWrite();
    }
    return res;
  }

  void Panel_DSI::setInvert(bool invert)
  {
    _invert = invert;
    write_params((invert ^ _cfg.invert) ? CMD_INVON : CMD_INVOFF);
  }

  void Panel_DSI::setSleep(bool flg_sleep)
  {
    write_params(flg_sleep ? CMD_SLPIN : CMD_SLPOUT);
  }

  void Panel_DSI::setPowerSave(bool flg_idle)
  {
    write_params(flg_idle ? CMD_IDMON : CMD_IDMOFF);
  }

//----------------------------------------------------------------------------
}
}

#endif
