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

#include "Panel_LT8912B.hpp"

#if SOC_MIPI_DSI_SUPPORTED

#include "../common.hpp"

#include <algorithm>
#include <cstdlib>
#include <new>

#include <esp_attr.h>
#include <esp_err.h>
#include <esp_idf_version.h>
#include <esp_log.h>
#include <esp_lcd_mipi_dsi.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_io_interface.h>

#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_lcd_panel_commands.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_vendor.h>
#include <freertos/task.h>
#include <esp_lcd_panel_ops.h>

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 5, 3)
#include <inttypes.h>
#include <esp_pm.h>
#include <hal/mipi_dsi_hal.h>
#include <hal/mipi_dsi_host_ll.h>

// ESP-IDF 5.5.1 keeps the DSI bus internals private, but the handle is this
// struct in esp_lcd/dsi/mipi_dsi_priv.h. Keep this shim version-gated.
struct esp_lcd_dsi_bus_t {
    int bus_id;
    mipi_dsi_hal_context_t hal;
    esp_pm_lock_handle_t pm_lock;
};
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
#define esp_lcd_new_panel_io_i2c esp_lcd_new_panel_io_i2c_v2
#endif

#if __has_include(<utility/I2C_Class.hpp>)
#include <utility/I2C_Class.hpp>
#define LGFX_PANEL_LT8912B_HAS_M5_I2C 1
#endif

// LT8912B esp_lcd compatibility implementation. Kept in this file to match
// the M5GFX Panel_xxx convention while Panel_LT8912B wraps it for LovyanGFX.
static const char *TAG = "lt8912b";

static constexpr uint8_t LT8912B_IO_I2C_MAIN_ADDRESS = 0x48;
static constexpr uint8_t LT8912B_IO_I2C_CEC_ADDRESS  = 0x49;
static constexpr uint8_t LT8912B_IO_I2C_AVI_ADDRESS  = 0x4A;
static constexpr uint8_t LT8912B_ASPECT_RATIO_16_9   = 0x02;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 5, 3)
static void apply_idf551_dsi_compat(esp_lcd_dsi_bus_handle_t dsi_bus,
                                    const esp_lcd_dpi_panel_config_t* dpi_config,
                                    uint32_t lane_bit_rate_mbps)
{
    if (!dsi_bus) {
        return;
    }

    auto round_u32 = [](float value) -> uint32_t { return static_cast<uint32_t>(value + 0.5f); };
    auto hal = &dsi_bus->hal;

    if (!dpi_config) {
        const uint32_t timeout_div = round_u32(lane_bit_rate_mbps / 8.0f / 10.0f);
        const uint32_t esc_div = round_u32(lane_bit_rate_mbps / 8.0f / 18.0f);
        mipi_dsi_host_ll_set_timeout_clock_division(hal->host, timeout_div);
        mipi_dsi_host_ll_set_escape_clock_division(hal->host, esc_div);
        ESP_LOGI(TAG, "Applied IDF 5.5.1 DSI divider compat: TimeoutDiv=%" PRIu32 ", EscDiv=%" PRIu32,
                 timeout_div, esc_div);
        return;
    }

    if (dpi_config->dpi_clock_freq_mhz == 0) {
        return;
    }

    const auto& t = dpi_config->video_timing;
    const float ratio = static_cast<float>(lane_bit_rate_mbps) / dpi_config->dpi_clock_freq_mhz / 8.0f;
    const uint32_t host_hsw = round_u32(t.hsync_pulse_width * ratio);
    const uint32_t host_hbp = round_u32(t.hsync_back_porch * ratio);
    const uint32_t host_hfp = round_u32(t.hsync_front_porch * ratio);
    int32_t host_act = static_cast<int32_t>(round_u32(t.h_size * ratio));

    const uint32_t htotal = t.hsync_pulse_width + t.hsync_back_porch + t.h_size + t.hsync_front_porch;
    const uint32_t host_htotal = round_u32(htotal * ratio);
    const int32_t compensation = static_cast<int32_t>(host_htotal)
                               - static_cast<int32_t>(host_hsw + host_hbp + host_act + host_hfp);
    host_act += compensation;
    if (host_act < 0) {
        host_act = 0;
    }

    mipi_dsi_host_ll_dpi_set_horizontal_timing(hal->host, host_hsw, host_hbp,
                                               static_cast<uint32_t>(host_act), host_hfp);
    ESP_LOGI(TAG, "Applied IDF 5.5.1 DSI horizontal compat: hsw=%" PRIu32
                  ", hbp=%" PRIu32 ", act=%" PRId32 ", hfp=%" PRIu32,
             host_hsw, host_hbp, host_act, host_hfp);
}
#endif

struct lt8912b_io_t {
    esp_lcd_panel_io_handle_t main;
    esp_lcd_panel_io_handle_t cec_dsi;
    esp_lcd_panel_io_handle_t avi;
};

struct lt8912b_video_timing_t {
    uint16_t hfp;
    uint16_t hs;
    uint16_t hbp;
    uint16_t hact;
    uint16_t htotal;
    uint16_t vfp;
    uint16_t vs;
    uint16_t vbp;
    uint16_t vact;
    uint16_t vtotal;
    bool h_polarity;
    bool v_polarity;
    uint16_t vic;
    uint8_t aspect_ratio;
    uint32_t pclk_mhz;
};

struct lt8912b_vendor_config_t {
    lt8912b_video_timing_t video_timing;
    const uint8_t* init_commands;
    struct {
        esp_lcd_dsi_bus_handle_t dsi_bus;
        const esp_lcd_dpi_panel_config_t* dpi_config;
        uint8_t lane_num;
    } mipi_config;
};

static esp_lcd_panel_io_i2c_config_t make_lt8912b_io_config(uint32_t clk_speed_hz, uint8_t address)
{
    esp_lcd_panel_io_i2c_config_t config = {};
    config.dev_addr = address;
    config.control_phase_bytes = 1;
    config.lcd_cmd_bits = 8;
    config.lcd_param_bits = 8;
    config.flags.disable_control_phase = 1;
    config.scl_speed_hz = clk_speed_hz;
    return config;
}

static void fill_dpi_config(esp_lcd_dpi_panel_config_t* dpi_config,
                            const lt8912b_video_timing_t& timing,
                            uint8_t fb_num)
{
    *dpi_config = {};
    dpi_config->virtual_channel = 0;
    dpi_config->dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT;
    dpi_config->dpi_clock_freq_mhz = timing.pclk_mhz;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    dpi_config->in_color_format = LCD_COLOR_FMT_RGB888;
    dpi_config->out_color_format = LCD_COLOR_FMT_RGB888;
#else
    dpi_config->pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB888;
#endif
    dpi_config->num_fbs = fb_num;
    dpi_config->video_timing.h_size = timing.hact;
    dpi_config->video_timing.v_size = timing.vact;
    dpi_config->video_timing.hsync_pulse_width = timing.hs;
    dpi_config->video_timing.hsync_back_porch = timing.hbp;
    dpi_config->video_timing.hsync_front_porch = timing.hfp;
    dpi_config->video_timing.vsync_pulse_width = timing.vs;
    dpi_config->video_timing.vsync_back_porch = timing.vbp;
    dpi_config->video_timing.vsync_front_porch = timing.vfp;
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
    dpi_config->flags.use_dma2d = true;
#endif
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
    dpi_config->flags.disable_lp = true;
#endif
}

static bool make_lt8912b_timing(uint16_t h_res,
                                uint16_t v_res,
                                uint8_t refresh_rate,
                                uint8_t fb_num,
                                esp_lcd_dpi_panel_config_t* dpi_config,
                                lt8912b_video_timing_t* video_timing)
{
    if (!dpi_config || !video_timing) {
        return false;
    }

    if (h_res == 1280 && v_res == 720 && refresh_rate == 60) {
        *video_timing = {
            .hfp = 110,
            .hs = 40,
            .hbp = 220,
            .hact = 1280,
            .htotal = 1650,
            .vfp = 5,
            .vs = 5,
            .vbp = 20,
            .vact = 720,
            .vtotal = 750,
            .h_polarity = true,
            .v_polarity = true,
            .vic = 4,
            .aspect_ratio = LT8912B_ASPECT_RATIO_16_9,
            .pclk_mhz = 80,
        };
        fill_dpi_config(dpi_config, *video_timing, fb_num);
        return true;
    }

    if (h_res == 1920 && v_res == 1080 && refresh_rate == 30) {
        *video_timing = {
            .hfp = 48,
            .hs = 32,
            .hbp = 80,
            .hact = 1920,
            .htotal = 2080,
            .vfp = 3,
            .vs = 5,
            .vbp = 8,
            .vact = 1080,
            .vtotal = 1096,
            .h_polarity = true,
            .v_polarity = false,
            .vic = 0,
            .aspect_ratio = LT8912B_ASPECT_RATIO_16_9,
            .pclk_mhz = 80,
        };
        fill_dpi_config(dpi_config, *video_timing, fb_num);
        return true;
    }

    return false;
}

static esp_err_t panel_lt8912b_del(esp_lcd_panel_t *panel);
static esp_err_t panel_lt8912b_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_lt8912b_init(esp_lcd_panel_t *panel);
static esp_err_t panel_lt8912b_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_lt8912b_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_lt8912b_disp_on_off(esp_lcd_panel_t *panel, bool off);
static esp_err_t panel_lt8912b_sleep(esp_lcd_panel_t *panel, bool sleep);

static esp_err_t _panel_lt8912b_detect_input_mipi(esp_lcd_panel_t *panel);

typedef struct {
    lt8912b_io_t io;
    lt8912b_video_timing_t video_timing;
    int reset_gpio_num;
    bool reset_level;
    uint8_t lane_num;
    // To save the original functions of MIPI DPI panel
    esp_err_t (*del)(esp_lcd_panel_t *panel);
    esp_err_t (*init)(esp_lcd_panel_t *panel);
    const uint8_t* init_commands;
} lt8912b_panel_t;

// Store the LT8912B context globally because panel->user_data must stay owned by DPI.
static lt8912b_panel_t *g_lt8912b_context = nullptr;

static esp_err_t new_panel_lt8912b(const lt8912b_io_t *io, const lt8912b_vendor_config_t *vendor_config, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
    ESP_RETURN_ON_FALSE(io && io->main && io->cec_dsi && io->avi && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG, "invalid vendor config");


    esp_err_t ret = ESP_OK;
    auto lt8912b = static_cast<lt8912b_panel_t*>(std::calloc(1, sizeof(lt8912b_panel_t)));
    ESP_RETURN_ON_FALSE(lt8912b, ESP_ERR_NO_MEM, TAG, "no mem for lt8912b panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {};
        io_conf.pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num;
        io_conf.mode = GPIO_MODE_OUTPUT;
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    lt8912b->io = *io;
    lt8912b->video_timing = vendor_config->video_timing;
    lt8912b->init_commands = vendor_config->init_commands;
    lt8912b->lane_num = vendor_config->mipi_config.lane_num;
    lt8912b->reset_gpio_num = panel_dev_config->reset_gpio_num;
    lt8912b->reset_level = panel_dev_config->flags.reset_active_high;

    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, ret_panel), err, TAG,
                      "create MIPI DPI panel failed");
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    (void)esp_lcd_dpi_panel_enable_dma2d(*ret_panel);
#endif
    ESP_LOGD(TAG, "new MIPI DPI panel @%p", *ret_panel);

    lt8912b->del = (*ret_panel)->del;
    lt8912b->init = (*ret_panel)->init;

    (*ret_panel)->del = panel_lt8912b_del;
    (*ret_panel)->init = panel_lt8912b_init;
    (*ret_panel)->reset = panel_lt8912b_reset;
    (*ret_panel)->mirror = panel_lt8912b_mirror;
    (*ret_panel)->invert_color = panel_lt8912b_invert_color;
    (*ret_panel)->disp_on_off = panel_lt8912b_disp_on_off;
    (*ret_panel)->disp_sleep = panel_lt8912b_sleep;

    g_lt8912b_context = lt8912b;
    ESP_LOGD(TAG, "new lt8912b panel @%p", lt8912b);

    return ESP_OK;

err:
    if (lt8912b) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(static_cast<gpio_num_t>(panel_dev_config->reset_gpio_num));
        }
        std::free(lt8912b);
    }
    return ret;
}


static esp_err_t panel_lt8912b_del(esp_lcd_panel_t *panel)
{
    lt8912b_panel_t *lt8912b = g_lt8912b_context;

    if (!lt8912b) {
        ESP_LOGE(TAG, "LT8912B context is NULL!");
        return ESP_ERR_INVALID_STATE;
    }

    if (lt8912b->reset_gpio_num >= 0) {
        gpio_reset_pin(static_cast<gpio_num_t>(lt8912b->reset_gpio_num));
    }
    lt8912b->del(panel);
    std::free(lt8912b);
    g_lt8912b_context = nullptr;
    return ESP_OK;
}

static esp_err_t panel_lt8912b_reset(esp_lcd_panel_t *panel)
{
    lt8912b_panel_t *lt8912b = g_lt8912b_context;

    if (!lt8912b) {
        ESP_LOGE(TAG, "LT8912B context is NULL!");
        return ESP_ERR_INVALID_STATE;
    }

    esp_lcd_panel_io_handle_t io_main = lt8912b->io.main;

    // perform hardware reset
    if (lt8912b->reset_gpio_num >= 0) {
        gpio_set_level(static_cast<gpio_num_t>(lt8912b->reset_gpio_num), lt8912b->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(static_cast<gpio_num_t>(lt8912b->reset_gpio_num), !lt8912b->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    } else { // perform software reset
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io_main, LCD_CMD_SWRESET, nullptr, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(20)); // spec, wait at least 5ms before sending new command
    }

    return ESP_OK;
}

static esp_err_t _panel_lt8912b_send_data(esp_lcd_panel_io_handle_t io, uint8_t reg, uint8_t data)
{
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, reg, &data, 1), TAG, "send command failed");

    return ESP_OK;
}

struct lt8912b_reg_t {
    uint8_t reg;
    uint8_t data;
};

template <size_t N>
static esp_err_t _panel_lt8912b_send_regs(esp_lcd_panel_io_handle_t io, const lt8912b_reg_t (&regs)[N])
{
    for (const auto& reg : regs) {
        ESP_RETURN_ON_ERROR(_panel_lt8912b_send_data(io, reg.reg, reg.data), TAG, "send command failed");
    }
    return ESP_OK;
}

static esp_err_t _panel_lt8912b_send_word(esp_lcd_panel_io_handle_t io, uint8_t reg_lsb, uint16_t value)
{
    ESP_RETURN_ON_ERROR(_panel_lt8912b_send_data(io, reg_lsb, static_cast<uint8_t>(value & 0xff)), TAG,
                        "send low byte failed");
    ESP_RETURN_ON_ERROR(_panel_lt8912b_send_data(io, reg_lsb + 1, static_cast<uint8_t>(value >> 8)), TAG,
                        "send high byte failed");
    return ESP_OK;
}

static esp_err_t _panel_lt8912b_pulse_reg(esp_lcd_panel_io_handle_t io,
                                          uint8_t reg,
                                          uint8_t active_value,
                                          uint8_t idle_value,
                                          uint32_t delay_ms = 10)
{
    ESP_RETURN_ON_ERROR(_panel_lt8912b_send_data(io, reg, active_value), TAG, "send pulse active failed");
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
    ESP_RETURN_ON_ERROR(_panel_lt8912b_send_data(io, reg, idle_value), TAG, "send pulse idle failed");
    return ESP_OK;
}

static uint8_t _panel_lt8912b_lane_cfg(uint8_t lane_num)
{
    switch (lane_num) {
    case 1:
    case 2:
    case 3:
        return lane_num;
    default:
        return 0x00;
    }
}

static esp_err_t _panel_lt8912b_send_mipi_analog(esp_lcd_panel_io_handle_t io_main, bool pn_swap)
{
    const lt8912b_reg_t regs[] = {
        {0x3e, static_cast<uint8_t>(pn_swap ? 0xf6 : 0xd6)},
        {0x3f, 0xd4},
        {0x41, 0x3c},
    };
    return _panel_lt8912b_send_regs(io_main, regs);
}

static esp_err_t _panel_lt8912b_send_mipi_basic_set(esp_lcd_panel_io_handle_t io_cec, uint8_t lane_count, bool lane_swap)
{
    const lt8912b_reg_t regs[] = {
        {0x10, 0x01},  // term enable
        {0x11, 0x10},  // settle
        {0x13, lane_count},
        {0x14, 0x00},
        {0x15, static_cast<uint8_t>(lane_swap ? 0xa8 : 0x00)},
        {0x1a, 0x03},
        {0x1b, 0x03},
    };
    return _panel_lt8912b_send_regs(io_cec, regs);
}

static esp_err_t _panel_lt8912b_send_video_setup(esp_lcd_panel_t *panel)
{
    lt8912b_panel_t *lt8912b = g_lt8912b_context;

    if (!lt8912b) {
        ESP_LOGE(TAG, "LT8912B context is NULL!");
        return ESP_ERR_INVALID_STATE;
    }
    (void)panel;

    esp_lcd_panel_io_handle_t io_cec_dsi = lt8912b->io.cec_dsi;
    const auto& timing = lt8912b->video_timing;

    const lt8912b_reg_t regs8[] = {
        {0x18, static_cast<uint8_t>(timing.hs)},
        {0x19, static_cast<uint8_t>(timing.vs)},
        {0x2f, 0x0c},
    };
    ESP_RETURN_ON_ERROR(_panel_lt8912b_send_regs(io_cec_dsi, regs8), TAG, "send video timing byte regs failed");

    const struct {
        uint8_t reg_lsb;
        uint16_t value;
    } regs[] = {
        {0x1c, timing.hact},
        {0x34, timing.htotal},
        {0x36, timing.vtotal},
        {0x38, timing.vbp},
        {0x3a, timing.vfp},
        {0x3c, timing.hbp},
        {0x3e, timing.hfp},
    };
    for (const auto& reg : regs) {
        ESP_RETURN_ON_ERROR(_panel_lt8912b_send_word(io_cec_dsi, reg.reg_lsb, reg.value), TAG, "send video timing failed");
    }

    return ESP_OK;
}

static esp_err_t _panel_lt8912b_send_avi_infoframe(esp_lcd_panel_t *panel)
{
    lt8912b_panel_t *lt8912b = g_lt8912b_context;

    if (!lt8912b) {
        ESP_LOGE(TAG, "LT8912B context is NULL!");
        return ESP_ERR_INVALID_STATE;
    }
    (void)panel;

    esp_lcd_panel_io_handle_t io_main = lt8912b->io.main;
    esp_lcd_panel_io_handle_t io_avi = lt8912b->io.avi;
    const auto& timing = lt8912b->video_timing;

    const uint8_t sync_polarity = (timing.h_polarity ? 0x02 : 0x00) | (timing.v_polarity ? 0x01 : 0x00);
    const uint8_t pb2 = (timing.aspect_ratio << 4) + 0x08;
    const uint8_t pb4 = timing.vic;
    const uint8_t pb0 = (((pb2 + pb4) <= 0x5f) ? (0x5f - pb2 - pb4) : (0x15f - pb2 - pb4));

    const lt8912b_reg_t avi_pre[] = {
        {0x3c, 0x41},  // enable null packet
    };
    const lt8912b_reg_t main_regs[] = {
        {0xab, sync_polarity},
    };
    const lt8912b_reg_t avi_infoframe[] = {
        {0x43, pb0},
        {0x44, 0x10},
        {0x45, pb2},
        {0x46, 0x00},
        {0x47, pb4},
    };

    ESP_RETURN_ON_ERROR(_panel_lt8912b_send_regs(io_avi, avi_pre), TAG, "send AVI preamble failed");
    ESP_RETURN_ON_ERROR(_panel_lt8912b_send_regs(io_main, main_regs), TAG, "send sync polarity failed");
    ESP_RETURN_ON_ERROR(_panel_lt8912b_send_regs(io_avi, avi_infoframe), TAG, "send AVI infoframe failed");

    return ESP_OK;
}

static esp_err_t _panel_lt8912b_mipi_rx_logic_reset(esp_lcd_panel_io_handle_t io_main)
{
    ESP_RETURN_ON_ERROR(_panel_lt8912b_pulse_reg(io_main, 0x03, 0x7f, 0xff), TAG, "reset MIPI RX logic failed");
    ESP_RETURN_ON_ERROR(_panel_lt8912b_pulse_reg(io_main, 0x05, 0xfb, 0xff), TAG, "reset DDS failed");
    return ESP_OK;
}

static esp_err_t _panel_lt8912b_detect_input_mipi(esp_lcd_panel_t *panel)
{
    lt8912b_panel_t *lt8912b = g_lt8912b_context;

    if (!lt8912b) {
        ESP_LOGE(TAG, "LT8912B context is NULL!");
        return ESP_ERR_INVALID_STATE;
    }
    (void)panel;

    esp_lcd_panel_io_handle_t io_main = lt8912b->io.main;

    uint8_t val_c2 = 0, val_c3 = 0, val_c4 = 0;
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_rx_param(io_main, 0xC2, &val_c2, 1), TAG, "read 0xC2 failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_rx_param(io_main, 0xC3, &val_c3, 1), TAG, "read 0xC3 failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_rx_param(io_main, 0xC4, &val_c4, 1), TAG, "read 0xC4 failed");

    [[maybe_unused]] const uint16_t h_res = ((uint16_t)(val_c4 & 0xF0) << 4) | val_c2;
    [[maybe_unused]] const uint16_t v_res = ((uint16_t)(val_c4 & 0x0F) << 8) | val_c3;
    ESP_LOGD(TAG, "MIPI input: H=%u, V=%u", h_res, v_res);

    return ESP_OK;
}


static esp_err_t _panel_lt8912b_lvds_output(esp_lcd_panel_io_handle_t io_main, bool on)
{
    if (on) {
        const lt8912b_reg_t regs[] = {
            {0x02, 0xf7},
            {0x02, 0xff},
            {0x03, 0xcb},
            {0x03, 0xfb},
            {0x03, 0xff},
            {0x44, 0x30},
        };
        ESP_RETURN_ON_ERROR(_panel_lt8912b_send_regs(io_main, regs), TAG, "enable LVDS output failed");
        ESP_LOGD(TAG, "LT8912B LVDS output enabled");
    } else {
        const lt8912b_reg_t regs[] = {{0x44, 0x31}};
        ESP_RETURN_ON_ERROR(_panel_lt8912b_send_regs(io_main, regs), TAG, "disable LVDS output failed");
        ESP_LOGD(TAG, "LT8912B LVDS output disabled");
    }

    return ESP_OK;
}

static esp_err_t _panel_lt8912b_hdmi_output(esp_lcd_panel_io_handle_t io_main, bool on)
{
    const lt8912b_reg_t regs[] = {
        {0x33, static_cast<uint8_t>(on ? 0x0e : 0x0c)},
    };
    ESP_RETURN_ON_ERROR(_panel_lt8912b_send_regs(io_main, regs), TAG, "set HDMI output failed");
    if (on) {
        ESP_LOGD(TAG, "LT8912B HDMI output enabled");
    } else {
        ESP_LOGD(TAG, "LT8912B HDMI output disabled");
    }

    return ESP_OK;
}

static esp_lcd_panel_io_handle_t _panel_lt8912b_get_io(lt8912b_panel_t *lt8912b, uint8_t token)
{
    using namespace lgfx::v1::detail;
    switch (token) {
    case lt8912b_seq_main:
        return lt8912b->io.main;
    case lt8912b_seq_cec_dsi:
        return lt8912b->io.cec_dsi;
    case lt8912b_seq_avi:
        return lt8912b->io.avi;
    default:
        return nullptr;
    }
}

static esp_err_t _panel_lt8912b_exec_init_sequence(esp_lcd_panel_t *panel, const uint8_t *cmds)
{
    using namespace lgfx::v1::detail;

    lt8912b_panel_t *lt8912b = g_lt8912b_context;
    ESP_RETURN_ON_FALSE(lt8912b && cmds, ESP_ERR_INVALID_STATE, TAG, "invalid LT8912B init state");

    for (;;) {
        const uint8_t token = *cmds++;
        if (token == lt8912b_seq_end) {
            return ESP_OK;
        }

        if (auto io = _panel_lt8912b_get_io(lt8912b, token)) {
            const uint8_t reg = *cmds++;
            const uint8_t len = *cmds++;
            ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, reg, cmds, len), TAG, "send command failed");
            cmds += len;
            continue;
        }

        switch (token) {
        case lt8912b_seq_mipi_analog:
            ESP_RETURN_ON_ERROR(_panel_lt8912b_send_mipi_analog(lt8912b->io.main, false), TAG, "mipi analog failed");
            break;
        case lt8912b_seq_mipi_basic:
            ESP_RETURN_ON_ERROR(_panel_lt8912b_send_mipi_basic_set(lt8912b->io.cec_dsi,
                                                                    _panel_lt8912b_lane_cfg(lt8912b->lane_num),
                                                                    false),
                                TAG, "mipi basic set failed");
            break;
        case lt8912b_seq_video_setup:
            ESP_RETURN_ON_ERROR(_panel_lt8912b_send_video_setup(panel), TAG, "video setup failed");
            break;
        case lt8912b_seq_detect_mipi:
            ESP_RETURN_ON_ERROR(_panel_lt8912b_detect_input_mipi(panel), TAG, "detect MIPI failed");
            break;
        case lt8912b_seq_avi_infoframe:
            ESP_RETURN_ON_ERROR(_panel_lt8912b_send_avi_infoframe(panel), TAG, "avi infoframe failed");
            break;
        case lt8912b_seq_mipi_rx_reset:
            ESP_RETURN_ON_ERROR(_panel_lt8912b_mipi_rx_logic_reset(lt8912b->io.main), TAG, "mipi rx reset failed");
            break;
        case lt8912b_seq_lvds_off:
            ESP_RETURN_ON_ERROR(_panel_lt8912b_lvds_output(lt8912b->io.main, false), TAG, "lvds disable failed");
            break;
        case lt8912b_seq_hdmi_on:
            ESP_RETURN_ON_ERROR(_panel_lt8912b_hdmi_output(lt8912b->io.main, true), TAG, "enable HDMI failed");
            break;
        case lt8912b_seq_dpi_init:
            ESP_RETURN_ON_ERROR(lt8912b->init(panel), TAG, "init MIPI DPI panel failed");
            break;
        default:
            ESP_LOGE(TAG, "unknown init sequence token: 0x%02x", token);
            return ESP_ERR_INVALID_ARG;
        }
    }
}

static esp_err_t panel_lt8912b_init(esp_lcd_panel_t *panel)
{
    lt8912b_panel_t *lt8912b = g_lt8912b_context;

    if (!lt8912b) {
        ESP_LOGE(TAG, "LT8912B context is NULL!");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Initializing LT8912B HDMI bridge");
    ESP_RETURN_ON_ERROR(_panel_lt8912b_exec_init_sequence(panel, lt8912b->init_commands), TAG, "init sequence failed");
    ESP_LOGI(TAG, "LT8912B HDMI bridge initialized");
    return ESP_OK;
}

static esp_err_t panel_lt8912b_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    (void)panel;
    (void)invert_color_data;
    return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t panel_lt8912b_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    (void)panel;
    (void)mirror_x;
    (void)mirror_y;
    ESP_LOGW(TAG, "Mirror is not supported in LT8912B driver. Please use SW rotation.");
    return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t panel_lt8912b_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    (void)panel;
    (void)on_off;
    return ESP_ERR_NOT_SUPPORTED;
}

static esp_err_t panel_lt8912b_sleep(esp_lcd_panel_t *panel, bool sleep)
{
    lt8912b_panel_t *lt8912b = g_lt8912b_context;

    if (!lt8912b) {
        ESP_LOGE(TAG, "LT8912B context is NULL!");
        return ESP_ERR_INVALID_STATE;
    }
    (void)panel;

    esp_lcd_panel_io_handle_t io_main = lt8912b->io.main;

    if (sleep) {
        const lt8912b_reg_t regs[] = {
            {0x54, 0x1d},
            {0x51, 0x15},
            {0x44, 0x31},
            {0x41, 0xbd},
            {0x5c, 0x11},
        };
        ESP_RETURN_ON_ERROR(_panel_lt8912b_send_regs(io_main, regs), TAG, "enter sleep failed");
    } else {
        const lt8912b_reg_t regs[] = {
            {0x5c, 0x10},
            {0x54, 0x1c},
            {0x51, 0x2d},
            {0x44, 0x30},
            {0x41, 0xbc},
        };
        ESP_RETURN_ON_ERROR(_panel_lt8912b_send_regs(io_main, regs), TAG, "exit sleep failed");
        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_RETURN_ON_ERROR(_panel_lt8912b_pulse_reg(io_main, 0x03, 0x7f, 0xff), TAG, "reset MIPI RX logic failed");
        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_RETURN_ON_ERROR(_panel_lt8912b_pulse_reg(io_main, 0x05, 0xfb, 0xff), TAG, "reset DDS failed");
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  static constexpr const char* TAG = "Panel_LT8912B";

#if defined(LGFX_PANEL_LT8912B_HAS_M5_I2C)
  struct M5I2CPanelIO
  {
    esp_lcd_panel_io_t base;
    m5::I2C_Class* i2c = nullptr;
    uint8_t address = 0;
    uint32_t freq = 100000;
  };

  static M5I2CPanelIO* to_m5_i2c_io(esp_lcd_panel_io_t* io)
  {
    return reinterpret_cast<M5I2CPanelIO*>(io);
  }

  static esp_err_t m5_i2c_io_tx_param(esp_lcd_panel_io_t* io, int lcd_cmd, const void* param, size_t param_size)
  {
    auto ctx = to_m5_i2c_io(io);
    if (!ctx || !ctx->i2c || !ctx->i2c->isEnabled() || lcd_cmd < 0 || lcd_cmd > 0xFF) {
      return ESP_ERR_INVALID_ARG;
    }
    if (param_size && !param) {
      return ESP_ERR_INVALID_ARG;
    }

    if (!ctx->i2c->start(ctx->address, false, ctx->freq)) {
      return ESP_FAIL;
    }

    bool ok = ctx->i2c->write(static_cast<uint8_t>(lcd_cmd));
    if (ok && param_size) {
      ok = ctx->i2c->write(static_cast<const uint8_t*>(param), param_size);
    }
    ok = ctx->i2c->stop() && ok;
    return ok ? ESP_OK : ESP_FAIL;
  }

  static esp_err_t m5_i2c_io_rx_param(esp_lcd_panel_io_t* io, int lcd_cmd, void* param, size_t param_size)
  {
    auto ctx = to_m5_i2c_io(io);
    if (!ctx || !ctx->i2c || !ctx->i2c->isEnabled() || lcd_cmd < 0 || lcd_cmd > 0xFF) {
      return ESP_ERR_INVALID_ARG;
    }
    if (param_size == 0) {
      return ESP_OK;
    }
    if (!param) {
      return ESP_ERR_INVALID_ARG;
    }

    const bool ok = ctx->i2c->readRegister(ctx->address,
                                           static_cast<uint8_t>(lcd_cmd),
                                           static_cast<uint8_t*>(param),
                                           param_size,
                                           ctx->freq);
    return ok ? ESP_OK : ESP_FAIL;
  }

  static esp_err_t m5_i2c_io_tx_color(esp_lcd_panel_io_t* io, int lcd_cmd, const void* color, size_t color_size)
  {
    return m5_i2c_io_tx_param(io, lcd_cmd, color, color_size);
  }

  static esp_err_t m5_i2c_io_del(esp_lcd_panel_io_t* io)
  {
    delete to_m5_i2c_io(io);
    return ESP_OK;
  }

  static esp_err_t m5_i2c_io_register_event_callbacks(esp_lcd_panel_io_t* io,
                                                      const esp_lcd_panel_io_callbacks_t* cbs,
                                                      void* user_ctx)
  {
    (void)io;
    (void)cbs;
    (void)user_ctx;
    return ESP_OK;
  }

  static esp_lcd_panel_io_handle_t new_m5_i2c_panel_io(m5::I2C_Class* i2c, uint8_t address, uint32_t freq)
  {
    auto io = new (std::nothrow) M5I2CPanelIO();
    if (!io) {
      return nullptr;
    }

    io->base.rx_param = m5_i2c_io_rx_param;
    io->base.tx_param = m5_i2c_io_tx_param;
    io->base.tx_color = m5_i2c_io_tx_color;
    io->base.del = m5_i2c_io_del;
    io->base.register_event_callbacks = m5_i2c_io_register_event_callbacks;
    io->i2c = i2c;
    io->address = address;
    io->freq = freq;
    return &io->base;
  }
#endif

  Panel_LT8912B::~Panel_LT8912B(void)
  {
    release_panel();
  }

  color_depth_t Panel_LT8912B::setColorDepth(color_depth_t depth)
  {
    (void)depth;
    _write_depth = _config_detail.output_depth;
    _read_depth = _config_detail.output_depth;
    return _write_depth;
  }

  bool Panel_LT8912B::get_i2c_bus(void)
  {
    if (_config_detail.i2c_master_bus) {
      _i2c_bus = _config_detail.i2c_master_bus;
      _i2c_bus_owned = false;
      return true;
    }
    if (_i2c_bus) {
      return true;
    }

    const auto port = static_cast<i2c_port_num_t>(_config_detail.i2c_port);
    esp_err_t ret = ESP_FAIL;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
    ret = i2c_master_get_bus_handle(port, &_i2c_bus);
    if (ret == ESP_OK) {
      _i2c_bus_owned = false;
      return true;
    }
    ESP_LOGW(TAG, "get existing I2C bus %d failed: %s; create fallback bus SDA=%d SCL=%d",
             _config_detail.i2c_port, esp_err_to_name(ret),
             _config_detail.i2c_sda, _config_detail.i2c_scl);
#endif

    i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = port;
    bus_config.sda_io_num = static_cast<gpio_num_t>(_config_detail.i2c_sda);
    bus_config.scl_io_num = static_cast<gpio_num_t>(_config_detail.i2c_scl);
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;
    bus_config.intr_priority = 1;

    ret = i2c_new_master_bus(&bus_config, &_i2c_bus);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "create fallback I2C bus %d failed: %s", _config_detail.i2c_port, esp_err_to_name(ret));
      _i2c_bus = nullptr;
      return false;
    }
    _i2c_bus_owned = true;
    return true;
  }

  bool Panel_LT8912B::init_panel(Bus_DSI* bus)
  {
    if (_panel_handle) {
      return true;
    }
    if (!bus || !bus->getMipiDsiBus()) {
      return false;
    }

    _config_detail.fb_num = std::max<uint8_t>(1, std::min<uint8_t>(_config_detail.fb_num, 3));

    esp_lcd_dpi_panel_config_t dpi_config;
    lt8912b_video_timing_t video_timing;
    if (!make_lt8912b_timing(_config_detail.h_res, _config_detail.v_res,
                             _config_detail.refresh_rate, _config_detail.fb_num,
                             &dpi_config, &video_timing)) {
      ESP_LOGE(TAG, "unsupported timing: %ux%u@%u", _config_detail.h_res, _config_detail.v_res,
               _config_detail.refresh_rate);
      return false;
    }

    esp_err_t ret = ESP_OK;
#if defined(LGFX_PANEL_LT8912B_HAS_M5_I2C)
    if (_config_detail.i2c) {
      if (!_config_detail.i2c->isEnabled()) {
        ESP_LOGE(TAG, "M5.In_I2C is not initialized");
        return false;
      }
      _io_main = new_m5_i2c_panel_io(_config_detail.i2c, LT8912B_IO_I2C_MAIN_ADDRESS, _config_detail.i2c_freq);
      _io_cec = new_m5_i2c_panel_io(_config_detail.i2c, LT8912B_IO_I2C_CEC_ADDRESS, _config_detail.i2c_freq);
      _io_avi = new_m5_i2c_panel_io(_config_detail.i2c, LT8912B_IO_I2C_AVI_ADDRESS, _config_detail.i2c_freq);
      if (!_io_main || !_io_cec || !_io_avi) {
        ESP_LOGE(TAG, "create LT8912B M5.In_I2C IO failed");
        return false;
      }
    } else
#else
    if (_config_detail.i2c) {
      ESP_LOGE(TAG, "M5Unified I2C_Class is not available");
      return false;
    }
#endif
    {
      if (!get_i2c_bus()) {
        return false;
      }

      esp_lcd_panel_io_i2c_config_t main_cfg = make_lt8912b_io_config(_config_detail.i2c_freq, LT8912B_IO_I2C_MAIN_ADDRESS);
      esp_lcd_panel_io_i2c_config_t cec_cfg = make_lt8912b_io_config(_config_detail.i2c_freq, LT8912B_IO_I2C_CEC_ADDRESS);
      esp_lcd_panel_io_i2c_config_t avi_cfg = make_lt8912b_io_config(_config_detail.i2c_freq, LT8912B_IO_I2C_AVI_ADDRESS);

      ret = esp_lcd_new_panel_io_i2c(_i2c_bus, &main_cfg, &_io_main);
      if (ret == ESP_OK) {
        ret = esp_lcd_new_panel_io_i2c(_i2c_bus, &cec_cfg, &_io_cec);
      }
      if (ret == ESP_OK) {
        ret = esp_lcd_new_panel_io_i2c(_i2c_bus, &avi_cfg, &_io_avi);
      }
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "create LT8912B I2C IO failed: %s", esp_err_to_name(ret));
        return false;
      }
    }

    if (!_io_main || !_io_cec || !_io_avi) {
      ESP_LOGE(TAG, "create LT8912B I2C IO failed");
      return false;
    }

    lt8912b_vendor_config_t vendor_config = {};
    vendor_config.video_timing = video_timing;
    vendor_config.init_commands = getInitCommands(0);
    vendor_config.mipi_config.dsi_bus = bus->getMipiDsiBus();
    vendor_config.mipi_config.dpi_config = &dpi_config;
    vendor_config.mipi_config.lane_num = _config_detail.lane_num;

    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = (gpio_num_t)-1;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    panel_config.bits_per_pixel = 24;
    lt8912b_io_t panel_io = {};
    panel_io.main = _io_main;
    panel_io.cec_dsi = _io_cec;
    panel_io.avi = _io_avi;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 5, 3)
    auto dsi_bus = bus->getMipiDsiBus();
    apply_idf551_dsi_compat(dsi_bus, nullptr, bus->config().lane_mbps);
#endif

    ESP_LOGI(TAG, "init LT8912B HDMI: %ux%u@%u, fb=%u", _config_detail.h_res, _config_detail.v_res,
             _config_detail.refresh_rate, _config_detail.fb_num);

    ret = new_panel_lt8912b(&panel_io, &vendor_config, &panel_config, &_panel_handle);
    if (ret == ESP_OK) {
      ret = esp_lcd_panel_reset(_panel_handle);
    }
    if (ret == ESP_OK) {
      ret = esp_lcd_panel_init(_panel_handle);
    }
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "init LT8912B panel failed: %s", esp_err_to_name(ret));
      return false;
    }
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 5, 3)
    apply_idf551_dsi_compat(dsi_bus, &dpi_config, bus->config().lane_mbps);
#endif

    switch (_config_detail.fb_num) {
    case 1:
      ret = esp_lcd_dpi_panel_get_frame_buffer(_panel_handle, 1, &_frame_buffers[0]);
      break;
    case 2:
      ret = esp_lcd_dpi_panel_get_frame_buffer(_panel_handle, 2, &_frame_buffers[0], &_frame_buffers[1]);
      break;
    default:
      ret = esp_lcd_dpi_panel_get_frame_buffer(_panel_handle, 3, &_frame_buffers[0], &_frame_buffers[1],
                                               &_frame_buffers[2]);
      break;
    }
    if (ret != ESP_OK || !_frame_buffers[0]) {
      ESP_LOGE(TAG, "get framebuffer failed: %s", esp_err_to_name(ret));
      return false;
    }

    if (!_refresh_done_sem) {
      _refresh_done_sem = xSemaphoreCreateBinary();
    }
    if (_refresh_done_sem) {
      esp_lcd_dpi_panel_event_callbacks_t callbacks = {};
#if defined LGFX_ESP_LCD_USE_ON_REFRESH_DONE // idf 5.4x and previous
      callbacks.on_refresh_done = on_refresh_done;
#else
      callbacks.on_frame_buf_complete = on_refresh_done;
#endif

      (void)esp_lcd_dpi_panel_register_event_callbacks(_panel_handle, &callbacks, this);
    }

    return true;
  }

  bool Panel_LT8912B::setup_framebuffer(void)
  {
    auto fb = static_cast<uint8_t*>(_frame_buffers[0]);
    if (!fb) {
      return false;
    }

    const auto height = _cfg.panel_height;
    const size_t line_array_size = height * sizeof(void*);
    auto line_array = static_cast<uint8_t**>(heap_alloc_dma(line_array_size));
    if (!line_array) {
      return false;
    }
    std::fill_n(line_array, height, nullptr);

    const size_t line_length = ((_cfg.panel_width * _write_bits >> 3) + 3) & ~3;
    for (int y = 0; y < height; ++y) {
      line_array[y] = fb;
      fb += line_length;
    }
    _lines_buffer = line_array;
    return true;
  }

  bool Panel_LT8912B::init(bool use_reset)
  {
    if (_lines_buffer) {
      return false;
    }

    _cfg.memory_width = _config_detail.h_res;
    _cfg.memory_height = _config_detail.v_res;
    _cfg.panel_width = _config_detail.h_res;
    _cfg.panel_height = _config_detail.v_res;
    setColorDepth(_config_detail.output_depth);

    if (!Panel_FrameBufferBase::init(use_reset)) {
      return false;
    }

    auto bus = getBusDSI();
    if (!bus) {
      ESP_LOGE(TAG, "Bus_DSI is required");
      return false;
    }

    if (!init_panel(bus) || !setup_framebuffer()) {
      release_panel();
      return false;
    }
    return true;
  }

  void Panel_LT8912B::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    Panel_FrameBufferBase::display(x, y, w, h);
    if (_config_detail.use_draw_bitmap && _panel_handle && _frame_buffers[0]) {
      (void)esp_lcd_panel_draw_bitmap(_panel_handle, 0, 0, _cfg.panel_width, _cfg.panel_height, _frame_buffers[0]);
    }
  }

  void Panel_LT8912B::waitDisplay(void)
  {
    if (_refresh_done_sem) {
      (void)xSemaphoreTake(_refresh_done_sem, pdMS_TO_TICKS(100));
    }
  }

  void Panel_LT8912B::setSleep(bool flg_sleep)
  {
    if (_panel_handle) {
      (void)esp_lcd_panel_disp_sleep(_panel_handle, flg_sleep);
    }
  }

  bool IRAM_ATTR Panel_LT8912B::on_refresh_done(esp_lcd_panel_handle_t panel,
                                                esp_lcd_dpi_panel_event_data_t* edata,
                                                void* user_ctx)
  {
    (void)panel;
    (void)edata;

    auto self = static_cast<Panel_LT8912B*>(user_ctx);
    if (!self || !self->_refresh_done_sem) {
      return false;
    }

    BaseType_t high_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(self->_refresh_done_sem, &high_task_woken);
    return high_task_woken == pdTRUE;
  }

  void Panel_LT8912B::release_panel(void)
  {
    if (_refresh_done_sem) {
      vSemaphoreDelete(_refresh_done_sem);
      _refresh_done_sem = nullptr;
    }
    if (_lines_buffer) {
      heap_free(_lines_buffer);
      _lines_buffer = nullptr;
    }
    if (_panel_handle) {
      esp_lcd_panel_del(_panel_handle);
      _panel_handle = nullptr;
    }
    if (_io_avi) {
      esp_lcd_panel_io_del(_io_avi);
      _io_avi = nullptr;
    }
    if (_io_cec) {
      esp_lcd_panel_io_del(_io_cec);
      _io_cec = nullptr;
    }
    if (_io_main) {
      esp_lcd_panel_io_del(_io_main);
      _io_main = nullptr;
    }
    if (_i2c_bus_owned && _i2c_bus) {
      i2c_del_master_bus(_i2c_bus);
      _i2c_bus_owned = false;
    }
    _i2c_bus = nullptr;
    for (auto& fb : _frame_buffers) {
      fb = nullptr;
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
