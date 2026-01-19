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

#include "Bus_MIPI.hpp"
#include "Panel_DSI.hpp"


#if SOC_MIPI_DSI_SUPPORTED

#include "../common.hpp"

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

static const char *TAG = "Panel_DSI";

#ifdef __cplusplus
extern "C" {
#endif


  static esp_err_t default_panel_del(esp_lcd_panel_t *panel);
  static esp_err_t default_panel_init(esp_lcd_panel_t *panel);
  static esp_err_t default_panel_reset(esp_lcd_panel_t *panel);
  static esp_err_t default_panel_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
  static esp_err_t default_panel_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
  static esp_err_t default_panel_disp_on_off(esp_lcd_panel_t *panel, bool on_off);
  static esp_err_t default_panel_sleep(esp_lcd_panel_t *panel, bool sleep);



  esp_err_t esp_lcd_new_panel(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                       panel_callbacks_t *callbacks, esp_lcd_panel_handle_t *ret_panel)
  {
      ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
      vendor_config_t *vendor_config = (vendor_config_t *)panel_dev_config->vendor_config;
      ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG,
                          "invalid vendor config");

      esp_err_t ret = ESP_OK;
      panel_t *panel = (panel_t *)calloc(1, sizeof(panel_t));
      ESP_RETURN_ON_FALSE(panel, ESP_ERR_NO_MEM, TAG, "no mem for Panel_DSI");

      if (panel_dev_config->reset_gpio_num >= 0) {
          gpio_config_t io_conf;
          memset(&io_conf, 0, sizeof(io_conf));
          io_conf.mode = GPIO_MODE_OUTPUT;
          io_conf.pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num;
          ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
      }

      switch (panel_dev_config->rgb_ele_order) {
      case LCD_RGB_ELEMENT_ORDER_RGB:
          panel->madctl_val = 0;
          break;
      case LCD_RGB_ELEMENT_ORDER_BGR:
          panel->madctl_val |= LCD_CMD_BGR_BIT;
          break;
      default:
          ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
          break;
      }

      switch (panel_dev_config->bits_per_pixel) {
      case 16: // RGB565
          panel->colmod_val = 0x55;
          break;
      case 18: // RGB666
          panel->colmod_val = 0x66;
          break;
      case 24: // RGB888
          panel->colmod_val = 0x77;
          break;
      default:
          ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
          break;
      }

      panel->io = io;
      panel->init_cmds = vendor_config->init_cmds;
      panel->init_cmds_size = vendor_config->init_cmds_size;
      panel->lane_num = vendor_config->mipi_config.lane_num;
      panel->reset_gpio_num = panel_dev_config->reset_gpio_num;
      panel->flags.reset_level = panel_dev_config->flags.reset_active_high;

      // Create MIPI DPI panel
      ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, ret_panel), err, TAG,
                        "create MIPI DPI panel failed");
      ESP_LOGD(TAG, "new MIPI DPI panel @%p", *ret_panel);

      // Save the original functions of MIPI DPI panel
      panel->del = (*ret_panel)->del;
      panel->init = (*ret_panel)->init;
      // Overwrite the functions of MIPI DPI panel
      (*ret_panel)->del = callbacks->panel_del;
      (*ret_panel)->init = callbacks->panel_init;
      (*ret_panel)->reset = callbacks->panel_reset;
      (*ret_panel)->mirror = callbacks->panel_mirror;
      (*ret_panel)->invert_color = callbacks->panel_invert_color;
      (*ret_panel)->disp_on_off = callbacks->panel_disp_on_off;
      (*ret_panel)->disp_sleep = callbacks->panel_sleep;
      (*ret_panel)->user_data = panel;
      ESP_LOGD(TAG, "new Panel_DSI @%p", panel);

      return ESP_OK;

  err:
      if (panel) {
          if (panel_dev_config->reset_gpio_num >= 0) {
              gpio_reset_pin((gpio_num_t)panel_dev_config->reset_gpio_num);
          }
          free(panel);
      }
      return ret;
  }


  static esp_err_t default_panel_del(esp_lcd_panel_t *panel)
  {
      panel_t *paneldata = (panel_t *)panel->user_data;

      if (paneldata->reset_gpio_num >= 0) {
          gpio_reset_pin((gpio_num_t)paneldata->reset_gpio_num);
      }
      // Delete MIPI DPI panel
      paneldata->del(panel);
      ESP_LOGD(TAG, "del Panel_DSI @%p", paneldata);
      free(paneldata);

      return ESP_OK;
  }

  static esp_err_t default_panel_init(esp_lcd_panel_t *panel)
  {
      return ESP_FAIL;
  }

  static esp_err_t default_panel_reset(esp_lcd_panel_t *panel)
  {
      panel_t *paneldata = (panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = paneldata->io;

      // Perform hardware reset
      if (paneldata->reset_gpio_num >= 0) {
          gpio_set_level((gpio_num_t)paneldata->reset_gpio_num, paneldata->flags.reset_level);
          vTaskDelay(pdMS_TO_TICKS(10));
          gpio_set_level((gpio_num_t)paneldata->reset_gpio_num, !paneldata->flags.reset_level);
          vTaskDelay(pdMS_TO_TICKS(10));
      } else if (io) { // Perform software reset
          ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
          vTaskDelay(pdMS_TO_TICKS(20));
      }

      return ESP_OK;
  }

  static esp_err_t default_panel_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
  {
      panel_t *paneldata = (panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = paneldata->io;
      uint8_t command = 0;

      ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

      if (invert_color_data) {
          command = LCD_CMD_INVON;
      } else {
          command = LCD_CMD_INVOFF;
      }
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");

      return ESP_OK;
  }

  static esp_err_t default_panel_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
  {
      return ESP_FAIL;
  }

  static esp_err_t default_panel_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
  {
      panel_t *paneldata = (panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = paneldata->io;
      int command = 0;

      if (on_off) {
          command = LCD_CMD_DISPON;
      } else {
          command = LCD_CMD_DISPOFF;
      }
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
      return ESP_OK;
  }

  static esp_err_t default_panel_sleep(esp_lcd_panel_t *panel, bool sleep)
  {
      panel_t *paneldata = (panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = paneldata->io;
      int command = 0;

      if (sleep) {
          command = LCD_CMD_SLPIN;
      } else {
          command = LCD_CMD_SLPOUT;
      }
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
      vTaskDelay(pdMS_TO_TICKS(100));

      return ESP_OK;
  }

#ifdef __cplusplus
}
#endif




namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------


  panel_callbacks_t Panel_DSI::getCallbacks()
  {
    panel_callbacks_t cb =
    {
      default_panel_del,
      default_panel_init,
      default_panel_reset,
      default_panel_invert_color,
      default_panel_mirror,
      default_panel_disp_on_off,
      default_panel_sleep
    };
    return cb;
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

    auto mipi_bus = (Bus_MIPI*)_bus;
    if(mipi_bus->init())
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

    if (!Panel_FrameBufferBase::init(use_reset))
    {
      return false;
    }
    return true;
  }

//----------------------------------------------------------------------------

  void Panel_DSI::setInvert(bool invert)
  {
    _invert = invert;
    if (_disp_panel_handle != nullptr) {
    _disp_panel_handle->invert_color( (esp_lcd_panel_t*)(_disp_panel_handle), invert);
    }
  }

  void Panel_DSI::setSleep(bool flg_sleep)
  {
    if (_disp_panel_handle != nullptr) {
      _disp_panel_handle->disp_sleep( (esp_lcd_panel_t*)(_disp_panel_handle), flg_sleep);
    }
  }

  void Panel_DSI::setPowerSave(bool flg_idle)
  {
    if (_disp_panel_handle != nullptr) {
      _disp_panel_handle->disp_on_off((esp_lcd_panel_t*)(_disp_panel_handle), flg_idle);
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
