#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 25;
    static constexpr int spi_sclk = 19;
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
  struct Panel_default : public lgfx::Panel_ILI9341
  {
    Panel_default(void) {
      spi_3wire = false;
      spi_cs   = 22;
      spi_dc   = 21;
      gpio_rst = 18;
      gpio_bl  = 5;
      pwm_ch_bl = 7;
      freq_write = 40000000;
      freq_read  = 20000000;
      freq_fill  = 80000000;
      backlight_level = false;
    }
  };

public:

  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    static Panel_default panel;

    setPanel(&panel);
  }

  void initPanel(void) override
  {
    if (!_panel) return;
    _panel->init();

    if (readPanelID() > 0) {  // check panel (ILI9341 or ST7789)
      ESP_LOGW("LovyanGFX", "[Autodetect] Using Panel_ST7789");

      static lgfx::Panel_ST7789 panel;
      panel.spi_3wire = false;
      panel.spi_cs   = 22;
      panel.spi_dc   = 21;
      panel.gpio_rst = 18;
      panel.gpio_bl  = 5;
      panel.pwm_ch_bl = 7;
      panel.freq_write = 80000000;
      panel.freq_read  = 16000000;
      panel.freq_fill  = 80000000;
      panel.backlight_level = false;
      panel.offset_rotation = 2;
      panel.spi_mode_read = 1;
      panel.len_dummy_read_pixel = 16;

      setPanel(&panel);
    } else {
      ESP_LOGW("LovyanGFX", "[Autodetect] Using Panel_ILI9341");
    }
    lgfx::LGFX_SPI<lgfx::LGFX_Config>::initPanel();
  }

};

#endif
