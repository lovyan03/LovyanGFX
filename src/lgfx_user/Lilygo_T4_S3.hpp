
#include <LovyanGFX.hpp>

#include <lgfx/v1/panel/Panel_RM690B0.hpp>

// AMOLED Panel used by LilyGO T4-S3

class Lilygo_T4_S3 : public lgfx::LGFX_Device
{

  lgfx::Bus_QSPI      _bus_instance;
  lgfx::Panel_RM690B0 _panel_instance;

  /*
   * NOTE: RM690B0 has a limitation: for any given area to write [x, y, w, h], x and w
   *       must always be even or the command will be ignored.
   *
   * Because this limitation affects a lot of GFX functions, the Panel_RM690B0 driver
   * comes with an optional framebuffer.
   *
   * With framebuffer disabled (default):
   *   Most GFX functions will misbehave when x-coords and area-width aren't even.
   *   This profile is appropriate to pick when LovyanGFX is acting as a display
   *   driver only (e.g. LVGL, camera streaming), or when the GFX functions are used
   *   within RM690B0's limitations.
   *
   * With framebuffer enabled:
   *   All GFX functions are available without limitation, but overall performance
   *   is lower since some writes will happen twice.
   *   Framebuffer can be enabled in two diffent ways:
   *   - tft.enableFrameBuffer(true)  << screen draws are triggered automatically
   *   - tft.enableFrameBuffer(false) << screen draws must be explicitely triggered using
   *                                      tft.display() or tft.display(x, y, w, h).
   *   This profile is appropriate when maximum compatibility with GFX functions is
   *   more important than performance.
   *
  */

  public:

    Lilygo_T4_S3(void)
    {
      {
        auto cfg = _bus_instance.config();

        cfg.freq_write = 40000000;
        cfg.freq_read  = 20000000; // irrelevant

        cfg.pin_sclk  = GPIO_NUM_15;
        cfg.pin_io0   = GPIO_NUM_14;
        cfg.pin_io1   = GPIO_NUM_10;
        cfg.pin_io2   = GPIO_NUM_16;
        cfg.pin_io3   = GPIO_NUM_12;

        cfg.spi_host   = SPI2_HOST;
        cfg.spi_mode   = SPI_MODE0;
        cfg.dma_channel = SPI_DMA_CH_AUTO;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance.config();
        cfg.pin_rst          = GPIO_NUM_13;
        cfg.pin_cs           = GPIO_NUM_11;
        cfg.panel_width      = 452;
        cfg.panel_height     = 600;

        cfg.readable = true;

        cfg.offset_x = 16;
        cfg.offset_y = 0;

        _panel_instance.config(cfg);
      }

      setPanel(&_panel_instance);
    }


    bool enableFrameBuffer(bool auto_display=false)
    {
      if( _panel_instance.initPanelFb() ) {
        auto fbPanel = _panel_instance.getPanelFb();
        if( fbPanel ) {
          fbPanel->setBus(&_bus_instance);
          fbPanel->setAutoDisplay(auto_display);
          setPanel(fbPanel);
          return true;
        }
      }
      return false;
    }


    void disableFrameBuffer()
    {
      _panel_instance.deinitPanelFb();
      setPanel(&_panel_instance);
    }


    bool init()
    {
      lgfx::pinMode(GPIO_NUM_9, lgfx::pin_mode_t::output); // power enable for LCD
      lgfx::gpio_hi(GPIO_NUM_9);
      bool ret = lgfx::LGFX_Device::init();
      return ret;
    }

};
