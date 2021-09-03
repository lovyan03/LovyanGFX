#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_mosi = 19;
    static constexpr int spi_miso = -1;
    static constexpr int spi_sclk = 18;
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
  struct Panel_TTGO_TWristband : public lgfx::Panel_ST7735S
  {
    Panel_TTGO_TWristband() {
      len_dummy_read_pixel = 17;
      spi_3wire  = true;
      invert     = true;
      spi_cs     =  5;
      spi_dc     = 23;
      gpio_rst   = 26;
      gpio_bl    = 27;
      pwm_ch_bl  = 7;
      panel_width  = 80;
      panel_height = 160;
      offset_x     = 26;
      offset_y     = 1;
      offset_rotation = 2;
    }
/*
  protected:
    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list[] = {
          CMD::GAMMASET, 1, 0x08,  // Gamma set, curve 4
          0xFF,0xFF, // end
      };
      if (listno == 2)  return list;
      return Panel_ST7735S::getInitCommands(listno);
    }
//*/
  };

public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    static Panel_TTGO_TWristband panel;

    setPanel(&panel);
  }
};

#endif
