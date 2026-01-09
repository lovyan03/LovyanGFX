
#include "Panel_ILI9881C.hpp"

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

static const char *TAG = "ili9881c";

namespace lgfx
{
 inline namespace v1
 {

#ifdef __cplusplus
extern "C" {
#endif

  static esp_err_t ili9881c_init(esp_lcd_panel_t *ili9881c);
  static esp_err_t ili9881c_mirror(esp_lcd_panel_t *ili9881c, bool mirror_x, bool mirror_y);


  static esp_err_t ili9881c_init(esp_lcd_panel_t *ili9881c)
  {
      panel_t *panel = (panel_t *)ili9881c->user_data;
      esp_lcd_panel_io_handle_t io = panel->io;
      const lcd_init_cmd_t *init_cmds = NULL;
      uint16_t init_cmds_size = 0;
      uint8_t lane_command = Panel_ILI9881C::DSI_2_LANE;
      bool is_command0_enable = false;
      bool is_cmd_overwritten = false;

      switch (panel->lane_num) {
      case 0:
      case 2:
          lane_command = Panel_ILI9881C::DSI_2_LANE;
          break;
      case 3:
      case 4:
          lane_command = Panel_ILI9881C::DSI_3_4_LANE;
          break;
      default:
          ESP_LOGE(TAG, "Invalid lane number %d", panel->lane_num);
          return ESP_ERR_INVALID_ARG;
      }

      // The ID register is on the CMD_Page 1
      uint8_t ID1, ID2, ID3;
      esp_lcd_panel_io_tx_param(io, Panel_ILI9881C::CMD_CNDBKxSEL, (const uint8_t[]) {
          Panel_ILI9881C::CMD_BKxSEL_BYTE0, Panel_ILI9881C::CMD_BKxSEL_BYTE1, Panel_ILI9881C::CMD_BKxSEL_BYTE2_PAGE1
      }, 3);
      esp_lcd_panel_io_rx_param(io, 0x00, &ID1, 1);
      esp_lcd_panel_io_rx_param(io, 0x01, &ID2, 1);
      esp_lcd_panel_io_rx_param(io, 0x02, &ID3, 1);
      ESP_LOGI(TAG, "ID1: 0x%x, ID2: 0x%x, ID3: 0x%x", ID1, ID2, ID3);

      // For modifying MIPI-DSI lane settings
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, Panel_ILI9881C::PAD_CONTROL, &lane_command, 1), TAG, "send command failed");

      // back to CMD_Page 0
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, Panel_ILI9881C::CMD_CNDBKxSEL, (const uint8_t[]) {
          Panel_ILI9881C::CMD_BKxSEL_BYTE0, Panel_ILI9881C::CMD_BKxSEL_BYTE1, Panel_ILI9881C::CMD_BKxSEL_BYTE2_PAGE0
      }, 3), TAG, "send command failed");
      // exit sleep mode
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0), TAG,
                          "io tx param failed");
      vTaskDelay(pdMS_TO_TICKS(120));

      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, &(panel->madctl_val), 1), TAG, "send command failed");
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, &(panel->colmod_val), 1), TAG, "send command failed");

      // vendor specific initialization, it can be different between manufacturers
      // should consult the LCD supplier for initialization sequence code
      if (panel->init_cmds) {
          init_cmds = panel->init_cmds;
          init_cmds_size = panel->init_cmds_size;
     //  } else {
     //      init_cmds = vendor_specific_init_default;
     //      init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(lcd_init_cmd_t);
      }

      for (int i = 0; i < init_cmds_size; i++) {
          // Check if the command has been used or conflicts with the internal
          if (is_command0_enable && init_cmds[i].data_bytes > 0) {
              switch (init_cmds[i].cmd) {
              case LCD_CMD_MADCTL:
                  is_cmd_overwritten = true;
                  panel->madctl_val = ((uint8_t *)init_cmds[i].data)[0];
                  break;
              case LCD_CMD_COLMOD:
                  is_cmd_overwritten = true;
                  panel->colmod_val = ((uint8_t *)init_cmds[i].data)[0];
                  break;
              default:
                  is_cmd_overwritten = false;
                  break;
              }

              if (is_cmd_overwritten) {
                  is_cmd_overwritten = false;
                  ESP_LOGW(TAG, "The %02Xh command has been used and will be overwritten by external initialization sequence",
                           init_cmds[i].cmd);
              }
          }

          // Send command
          ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].data_bytes), TAG, "send command failed");
          vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));

          if ((init_cmds[i].cmd == Panel_ILI9881C::CMD_CNDBKxSEL) && (((uint8_t *)init_cmds[i].data)[2] == Panel_ILI9881C::CMD_BKxSEL_BYTE2_PAGE0)) {
              is_command0_enable = true;
          } else if ((init_cmds[i].cmd == Panel_ILI9881C::CMD_CNDBKxSEL) && (((uint8_t *)init_cmds[i].data)[2] != Panel_ILI9881C::CMD_BKxSEL_BYTE2_PAGE0)) {
              is_command0_enable = false;
          }
      }
      ESP_LOGD(TAG, "send init commands success");

      ESP_RETURN_ON_ERROR(panel->init(ili9881c), TAG, "init MIPI DPI panel failed");

      return ESP_OK;
  }


  static esp_err_t ili9881c_mirror(esp_lcd_panel_t *ili9881c, bool mirror_x, bool mirror_y)
  {
      panel_t *panel = (panel_t *)ili9881c->user_data;
      esp_lcd_panel_io_handle_t io = panel->io;
      uint8_t madctl_val = panel->madctl_val;

      ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

      // Control mirror through LCD command
      if (mirror_x) {
          madctl_val |= Panel_ILI9881C::CMD_GS_BIT;
      } else {
          madctl_val &= ~Panel_ILI9881C::CMD_GS_BIT;
      }
      if (mirror_y) {
          madctl_val |= Panel_ILI9881C::CMD_SS_BIT;
      } else {
          madctl_val &= ~Panel_ILI9881C::CMD_SS_BIT;
      }

      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, &madctl_val, 1), TAG, "send command failed");
      panel->madctl_val = madctl_val;

      return ESP_OK;
  }


#ifdef __cplusplus
}
#endif



  panel_callbacks_t Panel_ILI9881C::getCallbacks()
  {
    auto default_cb = Panel_DSI::getCallbacks();
    panel_callbacks_t cb =
    {
      default_cb.panel_del,
      ili9881c_init,
      default_cb.panel_reset,
      default_cb.panel_invert_color,
      ili9881c_mirror,
      default_cb.panel_disp_on_off,
      default_cb.panel_sleep,
    };
    return cb;
  }



//----------------------------------------------------------------------------
 }
}


#endif
