#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = HSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 19;
    static constexpr int spi_miso = -1;
    static constexpr int spi_sclk = 18;
  };

  struct Panel_TTGO_TWatch : public Panel_ST7789
  {
    Panel_TTGO_TWatch() : Panel_ST7789() {
      freq_write = 80000000;
      freq_read  = 20000000;
      freq_fill  = 80000000;
      panel_height = 240;
      invert    = true;
      spi_3wire = true;
      spi_cs    =  5;
      spi_dc    = 27;
      gpio_bl   = 12;
      pwm_ch_bl = 7;
    }
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    static lgfx::Panel_TTGO_TWatch panel;

    setPanel(&panel);
  }
};

#endif
