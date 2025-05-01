
#include <LovyanGFX.hpp>

#if !defined LGFX_USE_QSPI
  #error "This device does not support QSPI"
#endif

// AMOLED Panel used by LilyGO T-Display-S3-AMOLED

class Lilygo_T_Display_S3_AMOLED : public lgfx::LGFX_Device
{

  lgfx::Bus_SPI       _bus_instance;
  lgfx::Panel_RM67162 _panel_instance; // 1.91 inch RM67162 IPS AMOLED:

  /*
   * NOTE: RM67162 has a limitation: for any given area to write [x, y, w, h], x and w
   *       must always be even or the command will be ignored.
   *
   * Because this limitation affects a lot of GFX functions, the Panel_RM67162 driver
   * comes with an optional framebuffer.
   *
   * With framebuffer disabled (default):
   *   Most GFX functions will misbehave when x-coords and area-width aren't even.
   *   This profile is appropriate to pick when LovyanGFX is acting as a display
   *   driver only (e.g. LVGL, camera streaming), or when the GFX functions are used
   *   within RM67162's limitations.
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

    Lilygo_T_Display_S3_AMOLED(void)
    {
      {
        auto cfg = _bus_instance.config();

        cfg.freq_write = 75000000;
        cfg.freq_read  = 20000000; // irrelevant

        cfg.pin_sclk  = GPIO_NUM_47;
        cfg.pin_io0   = GPIO_NUM_18;
        cfg.pin_io1   = GPIO_NUM_7;
        cfg.pin_io2   = GPIO_NUM_48;
        cfg.pin_io3   = GPIO_NUM_5;

        cfg.spi_host   = SPI2_HOST;
        cfg.spi_mode   = SPI_MODE0;
        cfg.dma_channel = SPI_DMA_CH_AUTO;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance.config();
        cfg.pin_rst          = GPIO_NUM_17;
        cfg.pin_cs           = GPIO_NUM_6;
        cfg.panel_width      = 240;
        cfg.panel_height     = 536;

        cfg.readable = true;

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
      auto fbPanel = _panel_instance.getPanelFb();
      if(fbPanel) {
        _panel_instance.deinitPanelFb();
        setPanel(&_panel_instance);
      }
    }


    bool init()
    {
      lgfx::pinMode(GPIO_NUM_9, lgfx::pin_mode_t::output); // power enable for LCD
      lgfx::gpio_hi(GPIO_NUM_9);
      bool ret = lgfx::LGFX_Device::init();
      return ret;
    }


};
