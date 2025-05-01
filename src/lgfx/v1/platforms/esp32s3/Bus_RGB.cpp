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
#include "Bus_RGB.hpp"

#include <esp_lcd_panel_rgb.h>
#include <esp_pm.h>
#include <esp_log.h>
#include <esp_rom_gpio.h>
#include <rom/gpio.h>
#include <hal/gdma_ll.h>
#include <hal/gpio_ll.h>
#include <hal/gpio_hal.h>
#include <hal/lcd_ll.h>
#include <hal/lcd_hal.h>
#include <soc/lcd_periph.h>
#include <soc/lcd_cam_reg.h>
#include <soc/lcd_cam_struct.h>
#include <soc/gdma_channel.h>
#include <soc/gdma_reg.h>
#include <soc/gdma_struct.h>

#if __has_include (<esp_private/periph_ctrl.h>)
 #include <esp_private/periph_ctrl.h>
#else
 #include <driver/periph_ctrl.h>
#endif

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


  IRAM_ATTR void Bus_RGB::lcd_default_isr_handler(void *args)
  {
    Bus_RGB *me = (Bus_RGB*)args;
    auto dev = getDev(me->config().port);

    uint32_t intr_status = dev->lc_dma_int_st.val & 0x03;
    dev->lc_dma_int_clr.val = intr_status;
    if (intr_status & LCD_LL_EVENT_VSYNC_END) {
      GDMA.channel[me->_dma_ch].out.conf0.out_rst = 1;
      GDMA.channel[me->_dma_ch].out.conf0.out_rst = 0;
      GDMA.channel[me->_dma_ch].out.link.addr = (uintptr_t)&(me->_dmadesc_restart);
      GDMA.channel[me->_dma_ch].out.link.start = 1;

    // bool need_yield = false;
        // call user registered callback
        // if (rgb_panel->on_vsync) {
        //     if (rgb_panel->on_vsync(&rgb_panel->base, NULL, rgb_panel->user_ctx)) {
        //         need_yield = true;
        //     }
        // }

        // check whether to update the PCLK frequency, it should be safe to update the PCLK frequency in the VSYNC interrupt
        // lcd_rgb_panel_try_update_pclk(rgb_panel);

        // if (need_yield) {
        //     portYIELD_FROM_ISR();
        // }
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
// ここでは ESP-IDFのLCDドライバに初期化部分だけ任せる
// 本来なら esp_lcd_rgb_panel_config_t を使ってRGBバスを作成するところだが、
// フレームバッファの確保やイベントハンドラは自前で処理したいので、敢えて i80バスを作成する。
/*
    esp_lcd_rgb_panel_config_t _panel_config;

    memset(&_panel_config, 0, sizeof(_panel_config));
    _panel_config.clk_src = LCD_CLK_SRC_PLL160M;
    _panel_config.timings.pclk_hz = _cfg.freq_write;
    _panel_config.timings.h_res = 1;//_cfg.panel->width();
    _panel_config.timings.v_res = 1;//_cfg.panel->height();

    _panel_config.data_width = 16;
    // _panel_config->data_width = _cfg.panel->getWriteDepth() & color_depth_t::bit_mask; // RGB565 in parallel mode, thus 16bit in width

    _panel_config.sram_trans_align = 8;
    _panel_config.psram_trans_align = 64;
    _panel_config.hsync_gpio_num = _cfg.pin_hsync;
    _panel_config.vsync_gpio_num = _cfg.pin_vsync;
    _panel_config.de_gpio_num = _cfg.pin_henable;
    _panel_config.pclk_gpio_num = _cfg.pin_pclk;
    _panel_config.disp_gpio_num = GPIO_NUM_NC;

    for (int i = 0; i < 16; ++ i) {
      _panel_config.data_gpio_nums[i] = _cfg.pin_data[i];
    }
    _panel_config.flags.fb_in_psram = 1;             // allocate frame buffer in PSRAM

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&_panel_config, &_panel_handle));
/*/
    // dummy settings.
    esp_lcd_i80_bus_config_t bus_config;
    memset(&bus_config, 0, sizeof(esp_lcd_i80_bus_config_t));
    // bus_config.dc_gpio_num = GPIO_NUM_NC;
    bus_config.dc_gpio_num = _cfg.pin_vsync;
    bus_config.wr_gpio_num = _cfg.pin_pclk;
    bus_config.clk_src = lcd_clock_source_t::LCD_CLK_SRC_PLL160M;
    for (int i = 0; i < 16; ++i)
    {
      bus_config.data_gpio_nums[i^8] = _cfg.pin_data[i];
    }
    bus_config.bus_width = 16;
    bus_config.max_transfer_bytes = 4092;

    if (ESP_OK != esp_lcd_new_i80_bus(&bus_config, &_i80_bus)) {
      return false;
    }
    uint8_t pixel_bytes = (_cfg.panel->getWriteDepth() & bit_mask) >> 3;
    auto dev = getDev(_cfg.port);

    {
      static constexpr const uint8_t rgb332sig_tbl[] = { 1, 0, 1, 0, 1, 2, 3, 4, 2, 3, 4, 5, 6, 5, 6, 7 };
      static constexpr const uint8_t rgb565sig_tbl[] = { 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7 };
      auto tbl = (pixel_bytes == 2) ? rgb565sig_tbl : rgb332sig_tbl;
#if SOC_LCDCAM_RGB_LCD_SUPPORTED
      auto sigs = &lcd_periph_rgb_signals.panels[_cfg.port];
#else
      auto sigs = &lcd_periph_signals.panels[_cfg.port];
#endif
      for (size_t i = 0; i < 16; i++) {
        _gpio_pin_sig(_cfg.pin_data[i], sigs->data_sigs[tbl[i]]);
      }
      _gpio_pin_sig(_cfg.pin_henable, sigs->de_sig);
      _gpio_pin_sig(_cfg.pin_hsync, sigs->hsync_sig);
      _gpio_pin_sig(_cfg.pin_vsync, sigs->vsync_sig);
      _gpio_pin_sig(_cfg.pin_pclk, sigs->pclk_sig);
    }

    // periph_module_enable(lcd_periph_signals.panels[_cfg.port].module);
    _dma_ch = search_dma_out_ch(SOC_GDMA_TRIG_PERIPH_LCD0);
    if (_dma_ch < 0)
    {
      esp_lcd_del_i80_bus(_i80_bus);
      ESP_LOGE("Bus_RGB", "DMA channel not found...");
      return false;
    }


    GDMA.channel[_dma_ch].out.peri_sel.sel = SOC_GDMA_TRIG_PERIPH_LCD0;

    typeof(GDMA.channel[0].out.conf0) conf0;
    conf0.val = 0;
    conf0.out_eof_mode = 1;
    conf0.outdscr_burst_en = 1;
    conf0.out_data_burst_en = 1;
    GDMA.channel[_dma_ch].out.conf0.val = conf0.val;

    typeof(GDMA.channel[0].out.conf1) conf1;
    conf1.val = 0;
    conf1.out_ext_mem_bk_size = GDMA_LL_EXT_MEM_BK_SIZE_64B;
    GDMA.channel[_dma_ch].out.conf1.val = conf1.val;



    size_t fb_len = (_cfg.panel->width() * pixel_bytes) * _cfg.panel->height();
    auto data = (uint8_t*)heap_alloc_psram(fb_len);
    _frame_buffer = data;
    static constexpr size_t MAX_DMA_LEN = (4096-64);
    size_t dmadesc_size = (fb_len - 1) / MAX_DMA_LEN + 1;
    auto dmadesc = (dma_descriptor_t*)heap_caps_malloc(sizeof(dma_descriptor_t) * dmadesc_size, MALLOC_CAP_DMA);
    _dmadesc = dmadesc;

    size_t len = fb_len;
    while (len > MAX_DMA_LEN)
    {
      len -= MAX_DMA_LEN;
      dmadesc->buffer = (uint8_t *)data;
      data += MAX_DMA_LEN;
      *(uint32_t*)dmadesc = MAX_DMA_LEN | MAX_DMA_LEN << 12 | 0x80000000;
      dmadesc->next = dmadesc + 1;
      dmadesc++;
    }
    *(uint32_t*)dmadesc = ((len + 3) & ( ~3 )) | len << 12 | 0xC0000000;
    dmadesc->buffer = (uint8_t *)data;
    dmadesc->next = _dmadesc;
    GDMA.channel[_dma_ch].out.link.addr = (uintptr_t)&(_dmadesc);
    GDMA.channel[_dma_ch].out.link.start = 1;
    //////////////////////////////////////////////

    memcpy(&_dmadesc_restart, _dmadesc, sizeof(_dmadesc_restart));
    int skip_bytes = (GDMA_LL_L2FIFO_BASE_SIZE + 1) * pixel_bytes;
    auto p = (uint8_t*)(_dmadesc_restart.buffer);
    _dmadesc_restart.buffer = &p[skip_bytes];
    _dmadesc_restart.dw0.length -= skip_bytes;
    _dmadesc_restart.dw0.size -= skip_bytes;


    uint32_t hsw = _cfg.hsync_pulse_width;
    uint32_t hbp = _cfg.hsync_back_porch;
    uint32_t active_width = _cfg.panel->width();
    uint32_t hfp = _cfg.hsync_front_porch;

    uint32_t vsw = _cfg.vsync_pulse_width;
    uint32_t vbp = _cfg.vsync_back_porch;
    uint32_t vfp = _cfg.vsync_front_porch;
    uint32_t active_height = _cfg.panel->height();

    uint32_t div_a, div_b, div_n, clkcnt;
    calcClockDiv(&div_a, &div_b, &div_n, &clkcnt, 240*1000*1000, std::min<uint32_t>(_cfg.freq_write, 40000000u));
    typeof(dev->lcd_clock) lcd_clock;
    lcd_clock.lcd_clkcnt_n = std::max<uint32_t>(1u, clkcnt - 1);
    lcd_clock.lcd_clk_equ_sysclk = (clkcnt == 1);
    lcd_clock.lcd_ck_idle_edge = false;
    lcd_clock.lcd_ck_out_edge = _cfg.pclk_idle_high;
    lcd_clock.lcd_clkm_div_num = div_n;
    lcd_clock.lcd_clkm_div_b = div_b;
    lcd_clock.lcd_clkm_div_a = div_a;
    lcd_clock.lcd_clk_sel = 2; // clock_select: 1=XTAL CLOCK / 2=240MHz / 3=160MHz
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
    lcd_user.lcd_2byte_en = pixel_bytes > 1; // RGB565 or RGB332
    lcd_user.lcd_dout = 1;
    // lcd_user.lcd_dummy = 0;
    // lcd_user.lcd_cmd = 0;
    lcd_user.lcd_update = 1;
    lcd_user.lcd_reset = 1; // self clear
    // lcd_user.lcd_reset = 0;
    lcd_user.lcd_dummy_cyclelen = 3;
    // lcd_user.lcd_cmd_2_cycle_en = 0;
    dev->lcd_user.val = lcd_user.val;

    typeof(dev->lcd_misc) lcd_misc;
    lcd_misc.val = 0;
    lcd_misc.lcd_afifo_reset = true;
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
    lcd_ctrl2.lcd_de_idle_pol = _cfg.de_idle_high;
    dev->lcd_ctrl2.val = lcd_ctrl2.val;

    dev->lc_dma_int_ena.val = 1;

    int isr_flags = ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_SHARED;

#if SOC_LCDCAM_RGB_LCD_SUPPORTED
      auto sigs = &lcd_periph_rgb_signals.panels[_cfg.port];
#else
      auto sigs = &lcd_periph_signals.panels[_cfg.port];
#endif

    esp_intr_alloc_intrstatus(sigs->irq_id, isr_flags,
                                   (uint32_t)&dev->lc_dma_int_st,
                                    LCD_LL_EVENT_VSYNC_END, lcd_default_isr_handler, this, &_intr_handle);
    esp_intr_enable(_intr_handle);

    dev->lcd_user.lcd_update = 1;
    dev->lcd_user.lcd_start = 1;

    return true;
  }

  uint8_t* Bus_RGB::getDMABuffer(uint32_t length)
  {
    return _frame_buffer;
    // return _rgb_panel->fb;
  }

  void Bus_RGB::release(void)
  {
    if (_intr_handle) {
      esp_intr_free(_intr_handle);
    }
    if (_i80_bus)
    {
      esp_lcd_del_i80_bus(_i80_bus);
    }
    if (_dmadesc)
    {
      heap_caps_free(_dmadesc);
      _dmadesc = nullptr;
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
#endif
#endif
