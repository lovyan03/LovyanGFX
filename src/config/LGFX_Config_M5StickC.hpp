#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 15;
    static constexpr int spi_miso = 14;
    static constexpr int spi_sclk = 13;
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    static lgfx::Panel_M5StickC panel;

    setPanel(&panel);
  }

  void initPanel(void) override
  {
    if (!_panel) return;
    _panel->init();
    lgfx::LGFX_SPI<lgfx::LGFX_Config>::initPanel();

    std::uint32_t id = readPanelID();
    if (id != 0xf0897c) {  // check panel (ST7735 or ST7789)
      ESP_LOGI("LovyanGFX", "[Autodetect] Using Panel_ST7789");

      static lgfx::Panel_ST7789 panel;
      panel.spi_3wire  = true;
      panel.invert     = true;
      panel.spi_cs   =  5;
      panel.spi_dc   = 23;
      panel.gpio_rst = 18;
      panel.freq_write = 80000000;
      panel.freq_read  = 16000000;
      panel.freq_fill  = 80000000;
      panel.spi_mode_read = 1;
      panel.len_dummy_read_pixel = 16;
      panel.panel_width  = 135;
      panel.panel_height = 240;
      panel.offset_x = 52;
      panel.offset_y = 40;
      panel.offset_rotation = 2;

      setPanel(&panel);
      lgfx::LGFX_SPI<lgfx::LGFX_Config>::initPanel();
    } else {
      ESP_LOGI("LovyanGFX", "[Autodetect] Using Panel_ST7735");
    }
  }

};

#endif
