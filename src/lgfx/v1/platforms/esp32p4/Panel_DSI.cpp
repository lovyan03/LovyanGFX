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

#include <esp_ldo_regulator.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_io.h>



#if 0
 #include "esp_lcd_ili9881c.h"
#else

 #include <esp_lcd_panel_vendor.h>
 #include <esp_lcd_mipi_dsi.h>
 #include <stdint.h>
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief LCD panel initialization commands.
  *
  */
 typedef struct {
     int cmd;                /*<! The specific LCD command */
     const void *data;       /*<! Buffer that holds the command specific data */
     size_t data_bytes;      /*<! Size of `data` in memory, in bytes */
     unsigned int delay_ms;  /*<! Delay in milliseconds after this command */
 } ili9881c_lcd_init_cmd_t;
 
 /**
  * @brief LCD panel vendor configuration.
  *
  * @note  This structure needs to be passed to the `esp_lcd_panel_dev_config_t::vendor_config`.
  *
  */
 typedef struct {
     const ili9881c_lcd_init_cmd_t *init_cmds;       /*!< Pointer to initialization commands array. Set to NULL if using default commands.
                                                      *   The array should be declared as `static const` and positioned outside the function.
                                                      *   Please refer to `vendor_specific_init_default` in source file.
                                                      */
     uint16_t init_cmds_size;                        /*<! Number of commands in above array */
     struct {
         esp_lcd_dsi_bus_handle_t dsi_bus;               /*!< MIPI-DSI bus configuration */
         const esp_lcd_dpi_panel_config_t *dpi_config;   /*!< MIPI-DPI panel configuration */
         uint8_t  lane_num;                              /*!< Number of MIPI-DSI lanes */
     } mipi_config;
 } ili9881c_vendor_config_t;
 
 /**
  * @brief Create LCD panel for model ILI9881C
  *
  * @note  Vendor specific initialization can be different between manufacturers, should consult the LCD supplier for initialization sequence code.
  *
  * @param[in]  io LCD panel IO handle
  * @param[in]  panel_dev_config General panel device configuration
  * @param[out] ret_panel Returned LCD panel handle
  * @return
  *      - ESP_ERR_INVALID_ARG   if parameter is invalid
  *      - ESP_OK                on success
  *      - Otherwise             on fail
  */
 esp_err_t esp_lcd_new_panel_ili9881c(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                      esp_lcd_panel_handle_t *ret_panel);
 
 
 


 #endif
 
 #ifdef __cplusplus
 }
 #endif

 

 #include <esp_check.h>
 #include <esp_log.h>
 #include <esp_lcd_panel_commands.h>
 #include <esp_lcd_panel_interface.h>
 #include <esp_lcd_panel_io.h>
 #include <esp_lcd_mipi_dsi.h>
 #include <esp_lcd_panel_vendor.h>
 #include <freertos/FreeRTOS.h>
 #include <freertos/task.h>
 #include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

  #define ILI9881C_CMD_CNDBKxSEL                (0xFF)
  #define ILI9881C_CMD_BKxSEL_BYTE0             (0x98)
  #define ILI9881C_CMD_BKxSEL_BYTE1             (0x81)
  #define ILI9881C_CMD_BKxSEL_BYTE2_PAGE0       (0x00)
  #define ILI9881C_CMD_BKxSEL_BYTE2_PAGE1       (0x01)
  #define ILI9881C_CMD_BKxSEL_BYTE2_PAGE2       (0x02)
  #define ILI9881C_CMD_BKxSEL_BYTE2_PAGE3       (0x03)
  #define ILI9881C_CMD_BKxSEL_BYTE2_PAGE4       (0x04)
  
  #define ILI9881C_PAD_CONTROL                  (0xB7)
  #define ILI9881C_DSI_2_LANE                   (0x03)
  #define ILI9881C_DSI_3_4_LANE                 (0x02)
  
  #define ILI9881C_CMD_GS_BIT       (1 << 0)
  #define ILI9881C_CMD_SS_BIT       (1 << 1)
  
  typedef struct {
      esp_lcd_panel_io_handle_t io;
      int reset_gpio_num;
      uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
      uint8_t colmod_val; // save surrent value of LCD_CMD_COLMOD register
      const ili9881c_lcd_init_cmd_t *init_cmds;
      uint16_t init_cmds_size;
      uint8_t lane_num;
      struct {
          unsigned int reset_level: 1;
      } flags;
      // To save the original functions of MIPI DPI panel
      esp_err_t (*del)(esp_lcd_panel_t *panel);
      esp_err_t (*init)(esp_lcd_panel_t *panel);
  } ili9881c_panel_t;
  
  static const char *TAG = "ili9881c";
  
  static esp_err_t panel_ili9881c_del(esp_lcd_panel_t *panel);
  static esp_err_t panel_ili9881c_init(esp_lcd_panel_t *panel);
  static esp_err_t panel_ili9881c_reset(esp_lcd_panel_t *panel);
  static esp_err_t panel_ili9881c_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
  static esp_err_t panel_ili9881c_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
  static esp_err_t panel_ili9881c_disp_on_off(esp_lcd_panel_t *panel, bool on_off);
  static esp_err_t panel_ili9881c_sleep(esp_lcd_panel_t *panel, bool sleep);
  
  esp_err_t esp_lcd_new_panel_ili9881c(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                       esp_lcd_panel_handle_t *ret_panel)
  {
      ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
      ili9881c_vendor_config_t *vendor_config = (ili9881c_vendor_config_t *)panel_dev_config->vendor_config;
      ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG,
                          "invalid vendor config");
  
      esp_err_t ret = ESP_OK;
      ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)calloc(1, sizeof(ili9881c_panel_t));
      ESP_RETURN_ON_FALSE(ili9881c, ESP_ERR_NO_MEM, TAG, "no mem for ili9881c panel");
  
      if (panel_dev_config->reset_gpio_num >= 0) {
          gpio_config_t io_conf;
          memset(&io_conf, 0, sizeof(io_conf));
          io_conf.mode = GPIO_MODE_OUTPUT;
          io_conf.pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num;
          ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
      }
  
      switch (panel_dev_config->rgb_ele_order) {
      case LCD_RGB_ELEMENT_ORDER_RGB:
          ili9881c->madctl_val = 0;
          break;
      case LCD_RGB_ELEMENT_ORDER_BGR:
          ili9881c->madctl_val |= LCD_CMD_BGR_BIT;
          break;
      default:
          ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
          break;
      }
  
      switch (panel_dev_config->bits_per_pixel) {
      case 16: // RGB565
          ili9881c->colmod_val = 0x55;
          break;
      case 18: // RGB666
          ili9881c->colmod_val = 0x66;
          break;
      case 24: // RGB888
          ili9881c->colmod_val = 0x77;
          break;
      default:
          ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
          break;
      }

      ili9881c->io = io;
      ili9881c->init_cmds = vendor_config->init_cmds;
      ili9881c->init_cmds_size = vendor_config->init_cmds_size;
      ili9881c->lane_num = vendor_config->mipi_config.lane_num;
      ili9881c->reset_gpio_num = panel_dev_config->reset_gpio_num;
      ili9881c->flags.reset_level = panel_dev_config->flags.reset_active_high;
  
      // Create MIPI DPI panel
      ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, ret_panel), err, TAG,
                        "create MIPI DPI panel failed");
      ESP_LOGD(TAG, "new MIPI DPI panel @%p", *ret_panel);
  
      // Save the original functions of MIPI DPI panel
      ili9881c->del = (*ret_panel)->del;
      ili9881c->init = (*ret_panel)->init;
      // Overwrite the functions of MIPI DPI panel
      (*ret_panel)->del = panel_ili9881c_del;
      (*ret_panel)->init = panel_ili9881c_init;
      (*ret_panel)->reset = panel_ili9881c_reset;
      (*ret_panel)->mirror = panel_ili9881c_mirror;
      (*ret_panel)->invert_color = panel_ili9881c_invert_color;
      (*ret_panel)->disp_on_off = panel_ili9881c_disp_on_off;
      (*ret_panel)->disp_sleep = panel_ili9881c_sleep;
      (*ret_panel)->user_data = ili9881c;
      ESP_LOGD(TAG, "new ili9881c panel @%p", ili9881c);
  
      return ESP_OK;
  
  err:
      if (ili9881c) {
          if (panel_dev_config->reset_gpio_num >= 0) {
              gpio_reset_pin((gpio_num_t)panel_dev_config->reset_gpio_num);
          }
          free(ili9881c);
      }
      return ret;
  }
  
  
  static esp_err_t panel_ili9881c_del(esp_lcd_panel_t *panel)
  {
      ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
  
      if (ili9881c->reset_gpio_num >= 0) {
          gpio_reset_pin((gpio_num_t)ili9881c->reset_gpio_num);
      }
      // Delete MIPI DPI panel
      ili9881c->del(panel);
      ESP_LOGD(TAG, "del ili9881c panel @%p", ili9881c);
      free(ili9881c);
  
      return ESP_OK;
  }
  
  static esp_err_t panel_ili9881c_init(esp_lcd_panel_t *panel)
  {
      ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = ili9881c->io;
      const ili9881c_lcd_init_cmd_t *init_cmds = NULL;
      uint16_t init_cmds_size = 0;
      uint8_t lane_command = ILI9881C_DSI_2_LANE;
      bool is_command0_enable = false;
      bool is_cmd_overwritten = false;
  
      switch (ili9881c->lane_num) {
      case 0:
      case 2:
          lane_command = ILI9881C_DSI_2_LANE;
          break;
      case 3:
      case 4:
          lane_command = ILI9881C_DSI_3_4_LANE;
          break;
      default:
          ESP_LOGE(TAG, "Invalid lane number %d", ili9881c->lane_num);
          return ESP_ERR_INVALID_ARG;
      }
  
      // The ID register is on the CMD_Page 1
      uint8_t ID1, ID2, ID3;
      esp_lcd_panel_io_tx_param(io, ILI9881C_CMD_CNDBKxSEL, (const uint8_t[]) {
          ILI9881C_CMD_BKxSEL_BYTE0, ILI9881C_CMD_BKxSEL_BYTE1, ILI9881C_CMD_BKxSEL_BYTE2_PAGE1
      }, 3);
      esp_lcd_panel_io_rx_param(io, 0x00, &ID1, 1);
      esp_lcd_panel_io_rx_param(io, 0x01, &ID2, 1);
      esp_lcd_panel_io_rx_param(io, 0x02, &ID3, 1);
      ESP_LOGI(TAG, "ID1: 0x%x, ID2: 0x%x, ID3: 0x%x", ID1, ID2, ID3);

      // For modifying MIPI-DSI lane settings
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, ILI9881C_PAD_CONTROL, &lane_command, 1), TAG, "send command failed");

      // back to CMD_Page 0
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, ILI9881C_CMD_CNDBKxSEL, (const uint8_t[]) {
          ILI9881C_CMD_BKxSEL_BYTE0, ILI9881C_CMD_BKxSEL_BYTE1, ILI9881C_CMD_BKxSEL_BYTE2_PAGE0
      }, 3), TAG, "send command failed");
      // exit sleep mode
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0), TAG,
                          "io tx param failed");
      vTaskDelay(pdMS_TO_TICKS(120));

      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, &(ili9881c->madctl_val), 1), TAG, "send command failed");
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, &(ili9881c->colmod_val), 1), TAG, "send command failed");
  
      // vendor specific initialization, it can be different between manufacturers
      // should consult the LCD supplier for initialization sequence code
      if (ili9881c->init_cmds) {
          init_cmds = ili9881c->init_cmds;
          init_cmds_size = ili9881c->init_cmds_size;
     //  } else {
     //      init_cmds = vendor_specific_init_default;
     //      init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(ili9881c_lcd_init_cmd_t);
      }
  
      for (int i = 0; i < init_cmds_size; i++) {
          // Check if the command has been used or conflicts with the internal
          if (is_command0_enable && init_cmds[i].data_bytes > 0) {
              switch (init_cmds[i].cmd) {
              case LCD_CMD_MADCTL:
                  is_cmd_overwritten = true;
                  ili9881c->madctl_val = ((uint8_t *)init_cmds[i].data)[0];
                  break;
              case LCD_CMD_COLMOD:
                  is_cmd_overwritten = true;
                  ili9881c->colmod_val = ((uint8_t *)init_cmds[i].data)[0];
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
  
          if ((init_cmds[i].cmd == ILI9881C_CMD_CNDBKxSEL) && (((uint8_t *)init_cmds[i].data)[2] == ILI9881C_CMD_BKxSEL_BYTE2_PAGE0)) {
              is_command0_enable = true;
          } else if ((init_cmds[i].cmd == ILI9881C_CMD_CNDBKxSEL) && (((uint8_t *)init_cmds[i].data)[2] != ILI9881C_CMD_BKxSEL_BYTE2_PAGE0)) {
              is_command0_enable = false;
          }
      }
      ESP_LOGD(TAG, "send init commands success");
  
      ESP_RETURN_ON_ERROR(ili9881c->init(panel), TAG, "init MIPI DPI panel failed");
  
      return ESP_OK;
  }
  
  static esp_err_t panel_ili9881c_reset(esp_lcd_panel_t *panel)
  {
      ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = ili9881c->io;
  
      // Perform hardware reset
      if (ili9881c->reset_gpio_num >= 0) {
          gpio_set_level((gpio_num_t)ili9881c->reset_gpio_num, ili9881c->flags.reset_level);
          vTaskDelay(pdMS_TO_TICKS(10));
          gpio_set_level((gpio_num_t)ili9881c->reset_gpio_num, !ili9881c->flags.reset_level);
          vTaskDelay(pdMS_TO_TICKS(10));
      } else if (io) { // Perform software reset
          ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
          vTaskDelay(pdMS_TO_TICKS(20));
      }
  
      return ESP_OK;
  }
  
  static esp_err_t panel_ili9881c_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
  {
      ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = ili9881c->io;
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
  
  static esp_err_t panel_ili9881c_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
  {
      ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = ili9881c->io;
      uint8_t madctl_val = ili9881c->madctl_val;
  
      ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");
  
      // Control mirror through LCD command
      if (mirror_x) {
          madctl_val |= ILI9881C_CMD_GS_BIT;
      } else {
          madctl_val &= ~ILI9881C_CMD_GS_BIT;
      }
      if (mirror_y) {
          madctl_val |= ILI9881C_CMD_SS_BIT;
      } else {
          madctl_val &= ~ILI9881C_CMD_SS_BIT;
      }

      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, &madctl_val, 1), TAG, "send command failed");
      ili9881c->madctl_val = madctl_val;
  
      return ESP_OK;
  }
  
  static esp_err_t panel_ili9881c_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
  {
      ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = ili9881c->io;
      int command = 0;
  
      if (on_off) {
          command = LCD_CMD_DISPON;
      } else {
          command = LCD_CMD_DISPOFF;
      }
      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
      return ESP_OK;
  }
  
  static esp_err_t panel_ili9881c_sleep(esp_lcd_panel_t *panel, bool sleep)
  {
      ili9881c_panel_t *ili9881c = (ili9881c_panel_t *)panel->user_data;
      esp_lcd_panel_io_handle_t io = ili9881c->io;
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


#define BSP_LCD_MIPI_DSI_LANE_NUM           (2)   // 2 data lanes
#define BSP_LCD_MIPI_DSI_LANE_BITRATE_MBPS  (730) // 720*1280 RGB24 60Hz //(900) // 900Mbps
#define BSP_MIPI_DSI_PHY_PWR_LDO_CHAN       (3)    // LDO_VO3 is connected to VDD_MIPI_DPHY
#define BSP_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV (2500)

static const ili9881c_lcd_init_cmd_t tab5_lcd_ili9881c_specific_init_code_default[] = {
  // {cmd, { data }, data_size, delay}
  /**** CMD_Page 1 ****/
  {0xFF, (uint8_t[]){0x98, 0x81, 0x01}, 3, 0},
  {0xB7, (uint8_t[]){0x03}, 1, 0},  // set 2 lane
  /**** CMD_Page 3 ****/
  {0xFF, (uint8_t[]){0x98, 0x81, 0x03}, 3, 0},
  {0x01, (uint8_t[]){0x00}, 1, 0},
  {0x02, (uint8_t[]){0x00}, 1, 0},
  {0x03, (uint8_t[]){0x73}, 1, 0},
  {0x04, (uint8_t[]){0x00}, 1, 0},
  {0x05, (uint8_t[]){0x00}, 1, 0},
  {0x06, (uint8_t[]){0x08}, 1, 0},
  {0x07, (uint8_t[]){0x00}, 1, 0},
  {0x08, (uint8_t[]){0x00}, 1, 0},
  {0x09, (uint8_t[]){0x1B}, 1, 0},
  {0x0a, (uint8_t[]){0x01}, 1, 0},
  {0x0b, (uint8_t[]){0x01}, 1, 0},
  {0x0c, (uint8_t[]){0x0D}, 1, 0},
  {0x0d, (uint8_t[]){0x01}, 1, 0},
  {0x0e, (uint8_t[]){0x01}, 1, 0},
  {0x0f, (uint8_t[]){0x26}, 1, 0},
  {0x10, (uint8_t[]){0x26}, 1, 0},
  {0x11, (uint8_t[]){0x00}, 1, 0},
  {0x12, (uint8_t[]){0x00}, 1, 0},
  {0x13, (uint8_t[]){0x02}, 1, 0},
  {0x14, (uint8_t[]){0x00}, 1, 0},
  {0x15, (uint8_t[]){0x00}, 1, 0},
  {0x16, (uint8_t[]){0x00}, 1, 0},
  {0x17, (uint8_t[]){0x00}, 1, 0},
  {0x18, (uint8_t[]){0x00}, 1, 0},
  {0x19, (uint8_t[]){0x00}, 1, 0},
  {0x1a, (uint8_t[]){0x00}, 1, 0},
  {0x1b, (uint8_t[]){0x00}, 1, 0},
  {0x1c, (uint8_t[]){0x00}, 1, 0},
  {0x1d, (uint8_t[]){0x00}, 1, 0},
  {0x1e, (uint8_t[]){0x40}, 1, 0},
  {0x1f, (uint8_t[]){0x00}, 1, 0},
  {0x20, (uint8_t[]){0x06}, 1, 0},
  {0x21, (uint8_t[]){0x01}, 1, 0},
  {0x22, (uint8_t[]){0x00}, 1, 0},
  {0x23, (uint8_t[]){0x00}, 1, 0},
  {0x24, (uint8_t[]){0x00}, 1, 0},
  {0x25, (uint8_t[]){0x00}, 1, 0},
  {0x26, (uint8_t[]){0x00}, 1, 0},
  {0x27, (uint8_t[]){0x00}, 1, 0},
  {0x28, (uint8_t[]){0x33}, 1, 0},
  {0x29, (uint8_t[]){0x03}, 1, 0},
  {0x2a, (uint8_t[]){0x00}, 1, 0},
  {0x2b, (uint8_t[]){0x00}, 1, 0},
  {0x2c, (uint8_t[]){0x00}, 1, 0},
  {0x2d, (uint8_t[]){0x00}, 1, 0},
  {0x2e, (uint8_t[]){0x00}, 1, 0},
  {0x2f, (uint8_t[]){0x00}, 1, 0},
  {0x30, (uint8_t[]){0x00}, 1, 0},
  {0x31, (uint8_t[]){0x00}, 1, 0},
  {0x32, (uint8_t[]){0x00}, 1, 0},
  {0x33, (uint8_t[]){0x00}, 1, 0},
  {0x34, (uint8_t[]){0x00}, 1, 0},
  {0x35, (uint8_t[]){0x00}, 1, 0},
  {0x36, (uint8_t[]){0x00}, 1, 0},
  {0x37, (uint8_t[]){0x00}, 1, 0},
  {0x38, (uint8_t[]){0x00}, 1, 0},
  {0x39, (uint8_t[]){0x00}, 1, 0},
  {0x3a, (uint8_t[]){0x00}, 1, 0},
  {0x3b, (uint8_t[]){0x00}, 1, 0},
  {0x3c, (uint8_t[]){0x00}, 1, 0},
  {0x3d, (uint8_t[]){0x00}, 1, 0},
  {0x3e, (uint8_t[]){0x00}, 1, 0},
  {0x3f, (uint8_t[]){0x00}, 1, 0},
  {0x40, (uint8_t[]){0x00}, 1, 0},
  {0x41, (uint8_t[]){0x00}, 1, 0},
  {0x42, (uint8_t[]){0x00}, 1, 0},
  {0x43, (uint8_t[]){0x00}, 1, 0},
  {0x44, (uint8_t[]){0x00}, 1, 0},

  {0x50, (uint8_t[]){0x01}, 1, 0},
  {0x51, (uint8_t[]){0x23}, 1, 0},
  {0x52, (uint8_t[]){0x45}, 1, 0},
  {0x53, (uint8_t[]){0x67}, 1, 0},
  {0x54, (uint8_t[]){0x89}, 1, 0},
  {0x55, (uint8_t[]){0xab}, 1, 0},
  {0x56, (uint8_t[]){0x01}, 1, 0},
  {0x57, (uint8_t[]){0x23}, 1, 0},
  {0x58, (uint8_t[]){0x45}, 1, 0},
  {0x59, (uint8_t[]){0x67}, 1, 0},
  {0x5a, (uint8_t[]){0x89}, 1, 0},
  {0x5b, (uint8_t[]){0xab}, 1, 0},
  {0x5c, (uint8_t[]){0xcd}, 1, 0},
  {0x5d, (uint8_t[]){0xef}, 1, 0},

  {0x5e, (uint8_t[]){0x11}, 1, 0},
  {0x5f, (uint8_t[]){0x02}, 1, 0},
  {0x60, (uint8_t[]){0x00}, 1, 0},
  {0x61, (uint8_t[]){0x07}, 1, 0},
  {0x62, (uint8_t[]){0x06}, 1, 0},
  {0x63, (uint8_t[]){0x0E}, 1, 0},
  {0x64, (uint8_t[]){0x0F}, 1, 0},
  {0x65, (uint8_t[]){0x0C}, 1, 0},
  {0x66, (uint8_t[]){0x0D}, 1, 0},
  {0x67, (uint8_t[]){0x02}, 1, 0},
  {0x68, (uint8_t[]){0x02}, 1, 0},
  {0x69, (uint8_t[]){0x02}, 1, 0},
  {0x6a, (uint8_t[]){0x02}, 1, 0},
  {0x6b, (uint8_t[]){0x02}, 1, 0},
  {0x6c, (uint8_t[]){0x02}, 1, 0},
  {0x6d, (uint8_t[]){0x02}, 1, 0},
  {0x6e, (uint8_t[]){0x02}, 1, 0},
  {0x6f, (uint8_t[]){0x02}, 1, 0},
  {0x70, (uint8_t[]){0x02}, 1, 0},
  {0x71, (uint8_t[]){0x02}, 1, 0},
  {0x72, (uint8_t[]){0x02}, 1, 0},
  {0x73, (uint8_t[]){0x05}, 1, 0},
  {0x74, (uint8_t[]){0x01}, 1, 0},
  {0x75, (uint8_t[]){0x02}, 1, 0},
  {0x76, (uint8_t[]){0x00}, 1, 0},
  {0x77, (uint8_t[]){0x07}, 1, 0},
  {0x78, (uint8_t[]){0x06}, 1, 0},
  {0x79, (uint8_t[]){0x0E}, 1, 0},
  {0x7a, (uint8_t[]){0x0F}, 1, 0},
  {0x7b, (uint8_t[]){0x0C}, 1, 0},
  {0x7c, (uint8_t[]){0x0D}, 1, 0},
  {0x7d, (uint8_t[]){0x02}, 1, 0},
  {0x7e, (uint8_t[]){0x02}, 1, 0},
  {0x7f, (uint8_t[]){0x02}, 1, 0},
  {0x80, (uint8_t[]){0x02}, 1, 0},
  {0x81, (uint8_t[]){0x02}, 1, 0},
  {0x82, (uint8_t[]){0x02}, 1, 0},
  {0x83, (uint8_t[]){0x02}, 1, 0},
  {0x84, (uint8_t[]){0x02}, 1, 0},
  {0x85, (uint8_t[]){0x02}, 1, 0},
  {0x86, (uint8_t[]){0x02}, 1, 0},
  {0x87, (uint8_t[]){0x02}, 1, 0},
  {0x88, (uint8_t[]){0x02}, 1, 0},
  {0x89, (uint8_t[]){0x05}, 1, 0},
  {0x8A, (uint8_t[]){0x01}, 1, 0},

  /**** CMD_Page 4 ****/
  {0xFF, (uint8_t[]){0x98, 0x81, 0x04}, 3, 0},
  {0x38, (uint8_t[]){0x01}, 1, 0},
  {0x39, (uint8_t[]){0x00}, 1, 0},
  {0x6C, (uint8_t[]){0x15}, 1, 0},
  {0x6E, (uint8_t[]){0x1A}, 1, 0},
  {0x6F, (uint8_t[]){0x25}, 1, 0},
  {0x3A, (uint8_t[]){0xA4}, 1, 0},
  {0x8D, (uint8_t[]){0x20}, 1, 0},
  {0x87, (uint8_t[]){0xBA}, 1, 0},
  {0x3B, (uint8_t[]){0x98}, 1, 0},

  /**** CMD_Page 1 ****/
  {0xFF, (uint8_t[]){0x98, 0x81, 0x01}, 3, 0},
  {0x22, (uint8_t[]){0x0A}, 1, 0},
  {0x31, (uint8_t[]){0x00}, 1, 0},
  {0x50, (uint8_t[]){0x6B}, 1, 0},
  {0x51, (uint8_t[]){0x66}, 1, 0},
  {0x53, (uint8_t[]){0x73}, 1, 0},
  {0x55, (uint8_t[]){0x8B}, 1, 0},
  {0x60, (uint8_t[]){0x1B}, 1, 0},
  {0x61, (uint8_t[]){0x01}, 1, 0},
  {0x62, (uint8_t[]){0x0C}, 1, 0},
  {0x63, (uint8_t[]){0x00}, 1, 0},

  // Gamma P
  {0xA0, (uint8_t[]){0x00}, 1, 0},
  {0xA1, (uint8_t[]){0x15}, 1, 0},
  {0xA2, (uint8_t[]){0x1F}, 1, 0},
  {0xA3, (uint8_t[]){0x13}, 1, 0},
  {0xA4, (uint8_t[]){0x11}, 1, 0},
  {0xA5, (uint8_t[]){0x21}, 1, 0},
  {0xA6, (uint8_t[]){0x17}, 1, 0},
  {0xA7, (uint8_t[]){0x1B}, 1, 0},
  {0xA8, (uint8_t[]){0x6B}, 1, 0},
  {0xA9, (uint8_t[]){0x1E}, 1, 0},
  {0xAA, (uint8_t[]){0x2B}, 1, 0},
  {0xAB, (uint8_t[]){0x5D}, 1, 0},
  {0xAC, (uint8_t[]){0x19}, 1, 0},
  {0xAD, (uint8_t[]){0x14}, 1, 0},
  {0xAE, (uint8_t[]){0x4B}, 1, 0},
  {0xAF, (uint8_t[]){0x1D}, 1, 0},
  {0xB0, (uint8_t[]){0x27}, 1, 0},
  {0xB1, (uint8_t[]){0x49}, 1, 0},
  {0xB2, (uint8_t[]){0x5D}, 1, 0},
  {0xB3, (uint8_t[]){0x39}, 1, 0},

  // Gamma N
  {0xC0, (uint8_t[]){0x00}, 1, 0},
  {0xC1, (uint8_t[]){0x01}, 1, 0},
  {0xC2, (uint8_t[]){0x0C}, 1, 0},
  {0xC3, (uint8_t[]){0x11}, 1, 0},
  {0xC4, (uint8_t[]){0x15}, 1, 0},
  {0xC5, (uint8_t[]){0x28}, 1, 0},
  {0xC6, (uint8_t[]){0x1B}, 1, 0},
  {0xC7, (uint8_t[]){0x1C}, 1, 0},
  {0xC8, (uint8_t[]){0x62}, 1, 0},
  {0xC9, (uint8_t[]){0x1C}, 1, 0},
  {0xCA, (uint8_t[]){0x29}, 1, 0},
  {0xCB, (uint8_t[]){0x60}, 1, 0},
  {0xCC, (uint8_t[]){0x16}, 1, 0},
  {0xCD, (uint8_t[]){0x17}, 1, 0},
  {0xCE, (uint8_t[]){0x4A}, 1, 0},
  {0xCF, (uint8_t[]){0x23}, 1, 0},
  {0xD0, (uint8_t[]){0x24}, 1, 0},
  {0xD1, (uint8_t[]){0x4F}, 1, 0},
  {0xD2, (uint8_t[]){0x5F}, 1, 0},
  {0xD3, (uint8_t[]){0x39}, 1, 0},

  /**** CMD_Page 0 ****/
  {0xFF, (uint8_t[]){0x98, 0x81, 0x00}, 3, 0},
  {0x35, (uint8_t[]){0x00}, 0, 0},
  // {0x11, (uint8_t []){0x00}, 0},
  {0xFE, (uint8_t[]){0x00}, 0, 0},
  {0x29, (uint8_t[]){0x00}, 0, 0},
  //============ Gamma END===========
};

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Panel_DSI::init_bus(void)
  {
    esp_ldo_channel_handle_t phy_pwr_chan = NULL;
    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_handle_t disp_panel = NULL;
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;

    esp_lcd_dsi_bus_config_t bus_config;
    bus_config.bus_id = 0;
    bus_config.num_data_lanes = BSP_LCD_MIPI_DSI_LANE_NUM;
    bus_config.phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT;
    bus_config.lane_bit_rate_mbps = BSP_LCD_MIPI_DSI_LANE_BITRATE_MBPS;

    esp_ldo_channel_config_t ldo_cfg;
    memset(&ldo_cfg, 0, sizeof(ldo_cfg));
    ldo_cfg.chan_id = BSP_MIPI_DSI_PHY_PWR_LDO_CHAN;
    ldo_cfg.voltage_mv = BSP_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV;

    esp_lcd_dbi_io_config_t dbi_config;
    dbi_config.virtual_channel = 0;
    dbi_config.lcd_cmd_bits = 8;
    dbi_config.lcd_param_bits = 8;

    if (ESP_OK == esp_ldo_acquire_channel(&ldo_cfg, &phy_pwr_chan)
     && ESP_OK == esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus)
     && ESP_OK == esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &io))
    {
      esp_lcd_dpi_panel_config_t dpi_config;
      memset(&dpi_config, 0, sizeof(dpi_config));
      dpi_config.virtual_channel = 0;
      dpi_config.dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT;
      dpi_config.dpi_clock_freq_mhz = 60;
      dpi_config.pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB565;
      dpi_config.num_fbs = 1;
      dpi_config.video_timing.h_size = _cfg.panel_width;
      dpi_config.video_timing.v_size = _cfg.panel_height;
      dpi_config.video_timing.hsync_back_porch = 140;
      dpi_config.video_timing.hsync_pulse_width = 40;
      dpi_config.video_timing.hsync_front_porch = 40;
      dpi_config.video_timing.vsync_back_porch = 20;
      dpi_config.video_timing.vsync_pulse_width = 4;
      dpi_config.video_timing.vsync_front_porch = 20;
      dpi_config.flags.use_dma2d = true;

      ili9881c_vendor_config_t vendor_config;
      vendor_config.init_cmds = tab5_lcd_ili9881c_specific_init_code_default;
      vendor_config.init_cmds_size = sizeof(tab5_lcd_ili9881c_specific_init_code_default)
                                   / sizeof(tab5_lcd_ili9881c_specific_init_code_default[0]);
      vendor_config.mipi_config.dsi_bus = mipi_dsi_bus;
      vendor_config.mipi_config.dpi_config = &dpi_config;
      vendor_config.mipi_config.lane_num = 2;

      esp_lcd_panel_dev_config_t lcd_dev_config;
      memset(&lcd_dev_config, 0, sizeof(lcd_dev_config));
      lcd_dev_config.reset_gpio_num = -1;
      lcd_dev_config.rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB;
      lcd_dev_config.bits_per_pixel = 16;
      lcd_dev_config.vendor_config  = &vendor_config;

      if (ESP_OK == esp_lcd_new_panel_ili9881c(io, &lcd_dev_config, &disp_panel)) {
        if (ESP_OK == esp_lcd_panel_reset(disp_panel)) {
          if (ESP_OK == esp_lcd_panel_init(disp_panel)) {
            if (ESP_OK == esp_lcd_panel_disp_on_off(disp_panel, true)) {

              _phy_pwr_chan_handle = phy_pwr_chan;
              _io_handle = io;
              _disp_panel_handle = disp_panel;
              _mipi_dsi_bus_handle = mipi_dsi_bus;
              return true;
            }
          }
        }
      }
    }

    if (disp_panel) {
      esp_lcd_panel_del(disp_panel);
    }
    if (io) {
      esp_lcd_panel_io_del(io);
    }
    if (mipi_dsi_bus) {
      esp_lcd_del_dsi_bus(mipi_dsi_bus);
    }
    if (phy_pwr_chan) {
        esp_ldo_release_channel(phy_pwr_chan);
    }
    return false;
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

    if (init_bus()) {
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
 }
}

#endif
