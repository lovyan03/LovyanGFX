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

#include <esp_ldo_regulator.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_io.h>
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
  } lcd_init_cmd_t;

  typedef struct {
      esp_lcd_panel_io_handle_t io;
      int reset_gpio_num;
      uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
      uint8_t colmod_val; // save surrent value of LCD_CMD_COLMOD register
      const lcd_init_cmd_t *init_cmds;
      uint16_t init_cmds_size;
      uint8_t lane_num;
      struct {
          unsigned int reset_level: 1;
      } flags;
      // To save the original functions of MIPI DPI panel
      esp_err_t (*del)(esp_lcd_panel_t *panel);
      esp_err_t (*init)(esp_lcd_panel_t *panel);
  } panel_t;

 /**
  * @brief LCD panel vendor configuration.
  *
  * @note  This structure needs to be passed to the `esp_lcd_panel_dev_config_t::vendor_config`.
  *
  */
 typedef struct {
     const lcd_init_cmd_t *init_cmds;       /*!< Pointer to initialization commands array. Set to NULL if using default commands.
                                                      *   The array should be declared as `static const` and positioned outside the function.
                                                      *   Please refer to `vendor_specific_init_default` in source file.
                                                      */
     uint16_t init_cmds_size;                        /*<! Number of commands in above array */
     struct {
         esp_lcd_dsi_bus_handle_t dsi_bus;               /*!< MIPI-DSI bus configuration */
         const esp_lcd_dpi_panel_config_t *dpi_config;   /*!< MIPI-DPI panel configuration */
         uint8_t  lane_num;                              /*!< Number of MIPI-DSI lanes */
     } mipi_config;
 } vendor_config_t;


 /**
  * @brief LCD panel non-drawing functions
  *
  * @note  This structure need to be passed to `esp_lcd_new_panel()`, it is a group of non-drawing lcd functions
  *
  */
  typedef struct {
    esp_err_t (*panel_del)(esp_lcd_panel_t *panel);
    esp_err_t (*panel_init)(esp_lcd_panel_t *panel);
    esp_err_t (*panel_reset)(esp_lcd_panel_t *panel);
    esp_err_t (*panel_invert_color)(esp_lcd_panel_t *panel, bool invert_color_data);
    esp_err_t (*panel_mirror)(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
    esp_err_t (*panel_disp_on_off)(esp_lcd_panel_t *panel, bool on_off);
    esp_err_t (*panel_sleep)(esp_lcd_panel_t *panel, bool sleep);
  } panel_callbacks_t;


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
  esp_err_t esp_lcd_new_panel(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                      panel_callbacks_t *callbacks, esp_lcd_panel_handle_t *ret_panel);


#ifdef __cplusplus
}
#endif

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


    void setInvert(bool invert) override;
    void setSleep(bool flg_sleep) override;
    void setPowerSave(bool flg_idle) override;

    virtual panel_callbacks_t getCallbacks();
    virtual const lcd_init_cmd_t* getLcdInitCommands(uint8_t listno=0) const { (void)listno; return nullptr; }
    virtual size_t getLcdInitCommandsize(uint8_t listno=0) { (void)listno; return 0; }

    esp_lcd_panel_handle_t _disp_panel_handle = NULL;
    esp_ldo_channel_handle_t _phy_pwr_chan_handle = NULL;
    esp_lcd_panel_io_handle_t _io_handle = NULL;
    esp_lcd_dsi_bus_handle_t _mipi_dsi_bus_handle = NULL;

  protected:

    config_detail_t _config_detail;
  };

//----------------------------------------------------------------------------
 }
}

#endif
#endif
