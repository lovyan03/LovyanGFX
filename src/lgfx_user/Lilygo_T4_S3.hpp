
#include <LovyanGFX.hpp>

#if !defined LGFX_USE_QSPI
  #error "This device does not support QSPI"
#endif

// AMOLED Panel used by LilyGO T4-S3

class Lilygo_T4_S3 : public lgfx::LGFX_Device
{

  lgfx::Bus_SPI       _bus_instance;
  lgfx::Panel_RM690B0 _panel_instance;
  lgfx::Touch_CST226  _touch_instance; // I2C sda:6, scl:7, int:8, rst: 17

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


      {
        auto cfg = _touch_instance.config();
        cfg.i2c_addr = 0x5A; // 0x5a (CST226SE) or 0x1A (CST226)
        cfg.pin_sda  = GPIO_NUM_6;
        cfg.pin_scl  = GPIO_NUM_7;
        cfg.pin_int  = GPIO_NUM_8;
        cfg.pin_rst  = GPIO_NUM_17;

        cfg.x_min      = 0;
        cfg.y_min      = 0;
        cfg.x_max      = 450;
        cfg.y_max      = 600;

        cfg.freq       = 400000;
        cfg.bus_shared = false;
        cfg.offset_rotation = _offset_rotation = 0;

        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
      }

      setPanel(&_panel_instance);
    }


    bool enableFrameBuffer(bool auto_display=false)
    {
      if( _panel_instance.initPanelFb() ) {
        auto fbPanel = _panel_instance.getPanelFb();
        if( fbPanel ) {
          fbPanel->setBus(&_bus_instance);
          auto touch_cfg = _touch_instance.config();
          _offset_rotation = touch_cfg.offset_rotation; // memoize for later restoration
          touch_cfg.offset_rotation = _panel_instance.getRotation(); // apply display rotation
          if(touch_cfg.offset_rotation & 1)
            std::swap(touch_cfg.x_max, touch_cfg.y_max);
          _touch_instance.config(touch_cfg);
          fbPanel->setTouch(&_touch_instance); // attach touch to the framebuffer
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
        auto touch_cfg = _touch_instance.config();
        if(touch_cfg.offset_rotation & 1)
          std::swap(touch_cfg.x_max, touch_cfg.y_max);
        touch_cfg.offset_rotation = _offset_rotation; // restore memoized offset rotation
        _touch_instance.config(touch_cfg);
        _panel_instance.setTouch(&_touch_instance); // reattach touch to the display
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

  protected:
    uint8_t _offset_rotation = 0; // memoize offset rotation when toggling framebuffer

};
