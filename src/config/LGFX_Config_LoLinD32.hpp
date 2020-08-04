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

  struct Panel_LoLinD32 : public Panel_ST7735S
  {
    Panel_LoLinD32(void) {
      panel_width  = 128;
      panel_height = 128;
      memory_width  = 132;
      memory_height = 132;
      offset_x  = 2;
      offset_y  = 1;
      spi_3wire = true;
      spi_cs    = 14;
      spi_dc    = 27;
      gpio_rst  = 33;
      gpio_bl   = 32;
      pwm_ch_bl = 7;
    }
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    static lgfx::Panel_LoLinD32 panel;

    setPanel(&panel);
  }
};

#endif
