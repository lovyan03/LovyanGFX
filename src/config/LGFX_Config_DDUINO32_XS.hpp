#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 26;
    static constexpr int spi_miso = -1;
    static constexpr int spi_sclk = 27;
  };

  struct Panel_DDUINO32_XS : public Panel_ST7789
  {
    Panel_DDUINO32_XS() : Panel_ST7789() {
      freq_write = 40000000;
      freq_fill  = 40000000;
      panel_height = 240;
      invert    = true;
      spi_3wire = true;
      spi_read  = false;
      spi_cs    = -1;
      spi_dc    = 23;
      gpio_rst  = 32;
      gpio_bl   = 22;
      pwm_ch_bl = 7;
    }
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    static lgfx::Panel_DDUINO32_XS panel;

    setPanel(&panel);
  }
};

#endif
