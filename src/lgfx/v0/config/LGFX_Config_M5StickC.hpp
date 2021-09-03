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

  struct Panel_M5StickC : public Panel_ST7735S
  {
    Panel_M5StickC() {
      spi_3wire  = true;
      invert     = true;
      spi_cs     =  5;
      spi_dc     = 23;
      gpio_rst   = 18;
      panel_width  = 80;
      panel_height = 160;
      offset_x     = 26;
      offset_y     = 1;
      offset_rotation = 2;
    }
  protected:
    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list[] = {
          CMD::GAMMASET, 1, 0x08,  // Gamma set, curve 4
          0xFF,0xFF, // end
      };
      if (listno == 2)  return list;
      return Panel_ST7735S::getInitCommands(listno);
    }
  };

  struct Panel_M5StickCPlus: public Panel_ST7789
  {
    Panel_M5StickCPlus() {
      spi_3wire  = true;
      invert     = true;
      spi_cs     =  5;
      spi_dc     = 23;
      gpio_rst   = 18;
      freq_write = 80000000;
      freq_read  = 16000000;
      freq_fill  = 80000000;
      spi_mode_read = 1;
      len_dummy_read_pixel = 16;
      panel_width  = 135;
      panel_height = 240;
      offset_x = 52;
      offset_y = 40;
    }
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

    uint32_t id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x85) {  //  check panel (ST7735 or ST7789)
      ESP_LOGW("LovyanGFX", "[Autodetect] Using Panel_ST7789");

      static lgfx::Panel_M5StickCPlus panel;

      setPanel(&panel);
      lgfx::LGFX_SPI<lgfx::LGFX_Config>::initPanel();
    } else {  // 0x7C
      ESP_LOGW("LovyanGFX", "[Autodetect] Using Panel_ST7735");
    }
  }

};

#endif
