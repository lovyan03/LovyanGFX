#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 19;
    static constexpr int spi_sclk = 18;
  };

  struct Panel_ODROID_GO : public Panel_ILI9341
  {
    Panel_ODROID_GO(void) {
      freq_fill  = 80000000;
      spi_3wire = true;
      spi_cs =  5;
      spi_dc = 21;
      rotation = 1;
      gpio_bl = 14;
      pwm_ch_bl = 7;
    }
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    static lgfx::Panel_ODROID_GO panel;

    setPanel(&panel);
  }
};

#endif
