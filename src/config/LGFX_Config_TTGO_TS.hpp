#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = -1;
    static constexpr int spi_sclk =  5;
  };

  struct Panel_TTGO_TS : public Panel_ST7735S
  {
    Panel_TTGO_TS(void) {
      freq_write = 20000000;
      panel_width  = 128;
      panel_height = 160;
      offset_x     = 2;
      offset_y     = 1;
      offset_rotation = 2;
      rgb_order = true;
      spi_3wire = true;
      spi_cs    = 16;
      spi_dc    = 17;
      gpio_rst  = 9;
      gpio_bl   = 27;
      pwm_ch_bl = 7;
    }
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    static lgfx::Panel_TTGO_TS panel;

    setPanel(&panel);
  }
};

#endif
