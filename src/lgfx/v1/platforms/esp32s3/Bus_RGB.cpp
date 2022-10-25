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
#if defined (CONFIG_IDF_TARGET_ESP32S3)
#if __has_include (<esp_lcd_panel_rgb.h>)
#include <esp_lcd_panel_rgb.h>
#include <esp_pm.h>

#include "Bus_RGB.hpp"

#include <esp_log.h>
#include <rom/gpio.h>
#include <esp_rom_gpio.h>
#include <hal/gpio_ll.h>
#include <hal/gpio_hal.h>
#include <hal/lcd_hal.h>
#include <soc/lcd_periph.h>
#include <soc/lcd_cam_reg.h>
#include <soc/lcd_cam_struct.h>

#include <soc/gdma_channel.h>
#include <soc/gdma_reg.h>
#if !defined (DMA_OUT_LINK_CH0_REG)
  #define DMA_OUT_LINK_CH0_REG       GDMA_OUT_LINK_CH0_REG
  #define DMA_OUTFIFO_STATUS_CH0_REG GDMA_OUTFIFO_STATUS_CH0_REG
  #define DMA_OUTLINK_START_CH0      GDMA_OUTLINK_START_CH0
  #define DMA_OUTFIFO_EMPTY_CH0      GDMA_OUTFIFO_EMPTY_L3_CH0
#endif

struct esp_rgb_panel_t {
    esp_lcd_panel_t base;  // Base class of generic lcd panel
    int panel_id;          // LCD panel ID
    lcd_hal_context_t hal; // Hal layer object
    size_t data_width;     // Number of data lines (e.g. for RGB565, the data width is 16)
    size_t sram_trans_align;  // Alignment for framebuffer that allocated in SRAM
    size_t psram_trans_align; // Alignment for framebuffer that allocated in PSRAM
    int disp_gpio_num;     // Display control GPIO, which is used to perform action like "disp_off"
    intr_handle_t intr;    // LCD peripheral interrupt handle
    esp_pm_lock_handle_t pm_lock; // Power management lock
    size_t num_dma_nodes;  // Number of DMA descriptors that used to carry the frame buffer
    uint8_t *fb;           // Frame buffer
    size_t fb_size;        // Size of frame buffer
    int data_gpio_nums[SOC_LCD_RGB_DATA_WIDTH]; // GPIOs used for data lines, we keep these GPIOs for action like "invert_color"
    size_t resolution_hz;    // Peripheral clock resolution
    esp_lcd_rgb_timing_t timings;   // RGB timing parameters (e.g. pclk, sync pulse, porch width)
    gdma_channel_handle_t dma_chan; // DMA channel handle
    int new_frame_id;               // ID for new frame, we use ID to identify whether the frame content has been updated
    int cur_frame_id;               // ID for current transferring frame
    SemaphoreHandle_t done_sem;     // Binary semaphore, indicating if the new frame has been flushed to LCD
    esp_lcd_rgb_panel_frame_trans_done_cb_t on_frame_trans_done; // Callback, invoked after frame trans done
    void *user_ctx;                // Reserved user's data of callback functions
    int x_gap;                      // Extra gap in x coordinate, it's used when calculate the flush window
    int y_gap;                      // Extra gap in y coordinate, it's used when calculate the flush window
    struct {
        unsigned int disp_en_level: 1; // The level which can turn on the screen by `disp_gpio_num`
        unsigned int stream_mode: 1;   // If set, the LCD transfers data continuously, otherwise, it stops refreshing the LCD when transaction done
        unsigned int new_frame: 1;     // Whether the frame we're going to flush is a new one
        unsigned int fb_in_psram: 1;   // Whether the frame buffer is in PSRAM
    } flags;
    dma_descriptor_t dma_nodes[]; // DMA descriptor pool of size `num_dma_nodes`
};

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }

  static lcd_cam_dev_t* getDev(int port)
  {
    return &LCD_CAM;
  }

  void Bus_RGB::config(const config_t& cfg)
  {
    _cfg = cfg;
  }

  static void _gpio_pin_init(int pin)
  {
    if (pin >= 0)
    {
      gpio_pad_select_gpio(pin);
      gpio_hi(pin);
      gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    }
  }

  static void _gpio_pin_sig(uint32_t pin, uint32_t sig)
  {
    gpio_hal_iomux_func_sel(GPIO_PIN_MUX_REG[pin], PIN_FUNC_GPIO);
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    esp_rom_gpio_connect_out_signal(pin, sig, false, false);
  }

  bool Bus_RGB::init(void)
  {
    esp_lcd_rgb_panel_config_t *_panel_config = (esp_lcd_rgb_panel_config_t *)heap_caps_calloc(1, sizeof(esp_lcd_rgb_panel_config_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    _panel_config->clk_src = LCD_CLK_SRC_PLL160M;

    _panel_config->timings.pclk_hz = _cfg.freq_write;
    _panel_config->timings.h_res = _cfg.panel->width();
    _panel_config->timings.v_res = _cfg.panel->height();
    // The following parameters should refer to LCD spec
    _panel_config->timings.hsync_pulse_width = _cfg.hsync_pulse_width;
    _panel_config->timings.hsync_back_porch = _cfg.hsync_back_porch;
    _panel_config->timings.hsync_front_porch = _cfg.hsync_front_porch;
    _panel_config->timings.vsync_pulse_width = _cfg.vsync_pulse_width;
    _panel_config->timings.vsync_back_porch = _cfg.vsync_back_porch;
    _panel_config->timings.vsync_front_porch = _cfg.vsync_front_porch;
    _panel_config->timings.flags.hsync_idle_low = (_cfg.hsync_polarity == 0) ? 1 : 0;
    _panel_config->timings.flags.vsync_idle_low = (_cfg.vsync_polarity == 0) ? 1 : 0;
    _panel_config->timings.flags.de_idle_high = 0;
    _panel_config->timings.flags.pclk_active_neg = _cfg.pclk_idle_high;
    _panel_config->timings.flags.pclk_idle_high = 0;

    // _panel_config->data_width = _cfg.panel->getWriteDepth() & color_depth_t::bit_mask; // RGB565 in parallel mode, thus 16bit in width
    _panel_config->data_width = 16; // RGB565 in parallel mode, thus 16bit in width
    _panel_config->sram_trans_align = 8;
    _panel_config->psram_trans_align = 64;
    _panel_config->hsync_gpio_num = _cfg.pin_hsync;
    _panel_config->vsync_gpio_num = _cfg.pin_vsync;
    _panel_config->de_gpio_num = _cfg.pin_henable;
    _panel_config->pclk_gpio_num = _cfg.pin_pclk;
    _panel_config->disp_gpio_num = GPIO_NUM_NC;

    for (int i = 0; i < 16; ++ i) {
      _panel_config->data_gpio_nums[i^8] = _cfg.pin_data[i];
    }

    _panel_config->flags.disp_active_low = 0;
    _panel_config->flags.relax_on_idle = 0;
    _panel_config->flags.fb_in_psram = 1;             // allocate frame buffer in PSRAM

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(_panel_config, &_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(_panel_handle));

    _rgb_panel = __containerof(_panel_handle, esp_rgb_panel_t, base);

////////////////////////////
//////////////////////////// 以下 自前コード

    {
      static constexpr const uint8_t rgb332sig_tbl[] = { 1, 0, 1, 0, 1, 2, 3, 4, 2, 3, 4, 5, 6, 5, 6, 7 };
      static constexpr const uint8_t rgb565sig_tbl[] = { 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7 };
      auto tbl = ((_cfg.panel->getWriteDepth() & bit_mask) > 8) ? rgb565sig_tbl : rgb332sig_tbl;
      auto sigs = lcd_periph_signals.panels[_rgb_panel->panel_id].data_sigs;
      for (size_t i = 0; i < 16; i++) {
        _gpio_pin_sig(_cfg.pin_data[i], sigs[tbl[i]]);
      }
    }

    uint32_t pclk_prescale = _rgb_panel->resolution_hz / _cfg.freq_write;
    _rgb_panel->timings.pclk_hz = _rgb_panel->resolution_hz / pclk_prescale;

    auto dev = getDev(_cfg.port);

    uint32_t hsw = _cfg.hsync_pulse_width;
    uint32_t hbp = _cfg.hsync_back_porch;
    uint32_t active_width = _cfg.panel->width();
    uint32_t hfp = _cfg.hsync_front_porch;

    uint32_t vsw = _cfg.vsync_pulse_width;
    uint32_t vbp = _cfg.vsync_back_porch;
    uint32_t vfp = _cfg.vsync_front_porch;
    uint32_t active_height = _cfg.panel->height();

    typeof(dev->lcd_clock) lcd_clock;
    lcd_clock.val = dev->lcd_clock.val;
  // ESP_LOGE("DEBUG","lcd_clock: %08x", lcd_clock.val);
  //   lcd_clock.val = 0xe0000502;
// ok:e0000502
// ng:60000443
    lcd_clock.lcd_clkcnt_n = pclk_prescale - 1;
    lcd_clock.lcd_clk_equ_sysclk = 0; // 0:pixel_clk == lcd_clk
    lcd_clock.lcd_ck_idle_edge = false;
    lcd_clock.lcd_ck_out_edge = _cfg.pclk_idle_high;
    lcd_clock.lcd_clk_sel = 3; // 3=PLL_F160M
    lcd_clock.clk_en = true;
    dev->lcd_clock.val = lcd_clock.val;

    typeof(dev->lcd_user) lcd_user;
    lcd_user.val = 0;
    // lcd_user.lcd_dout_cyclelen = 0;
    lcd_user.lcd_always_out_en = true;
    // lcd_user.lcd_8bits_order = false;
    // lcd_user.lcd_update = false;
    // lcd_user.lcd_bit_order = false;
    // lcd_user.lcd_byte_order = false;
    lcd_user.lcd_2byte_en = (_cfg.panel->getWriteDepth() >> 4) & 1; // RGB565 or RGB332
    lcd_user.lcd_dout = 1;
    // lcd_user.lcd_dummy = 0;
    // lcd_user.lcd_cmd = 0;
    lcd_user.lcd_start = 1;
    // lcd_user.lcd_reset = 0;
    lcd_user.lcd_dummy_cyclelen = 3;
    // lcd_user.lcd_cmd_2_cycle_en = 0;
    dev->lcd_user.val = lcd_user.val;

    typeof(dev->lcd_misc) lcd_misc;
    lcd_misc.val = 0;
    lcd_misc.lcd_next_frame_en = true;
    lcd_misc.lcd_bk_en = true;
    // lcd_misc.lcd_vfk_cyclelen = 0;
    // lcd_misc.lcd_vbk_cyclelen = 0;
    dev->lcd_misc.val = lcd_misc.val;

    typeof(dev->lcd_ctrl) lcd_ctrl;
    lcd_ctrl.lcd_hb_front = hbp + hsw - 1;
    lcd_ctrl.lcd_va_height = active_height - 1;
    lcd_ctrl.lcd_vt_height = vsw + vbp + active_height + vfp - 1;
    lcd_ctrl.lcd_rgb_mode_en = true;
    dev->lcd_ctrl.val = lcd_ctrl.val;

    typeof(dev->lcd_ctrl1) lcd_ctrl1;
    lcd_ctrl1.lcd_vb_front = vbp + vsw - 1;
    lcd_ctrl1.lcd_ha_width = active_width - 1;
    lcd_ctrl1.lcd_ht_width = hsw + hbp + active_width + hfp - 1;
    dev->lcd_ctrl1.val = lcd_ctrl1.val;

    typeof(dev->lcd_ctrl2) lcd_ctrl2;
    lcd_ctrl2.val = 0;
    lcd_ctrl2.lcd_vsync_width = vsw - 1;
    lcd_ctrl2.lcd_vsync_idle_pol = _cfg.vsync_polarity;
    lcd_ctrl2.lcd_hs_blank_en = true;
    lcd_ctrl2.lcd_hsync_width = hsw - 1;
    lcd_ctrl2.lcd_hsync_idle_pol = _cfg.hsync_polarity;
    // lcd_ctrl2.lcd_hsync_position = 0;
    dev->lcd_ctrl2.val = lcd_ctrl2.val;

    dev->lc_dma_int_ena.val = 1;

    return true;
  }

  uint8_t* Bus_RGB::getDMABuffer(uint32_t length)
  {
    return _rgb_panel->fb;
  }

  void Bus_RGB::release(void)
  {
  }

//----------------------------------------------------------------------------
 }
}

#endif
#endif
#endif
