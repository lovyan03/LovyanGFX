#pragma once

#include <LovyanGFX.hpp>


/*\
 *

 Generic SharpDisplay Memory LCD configless example

 See https://github.com/lovyan03/LovyanGFX/issues/694

 Usage:

    static SharpDisplay tft(SHARP_SCK, SHARP_MOSI, SHARP_SS, 400, 240);

    void setup()
    {
      // Either start without clearing:
      tft.init_without_reset(false);
      // .. or:
      tft.begin();
      tft.clearDisplay();
    }



 *
\*/

class SharpDisplay : public lgfx::LGFX_Device
{
  lgfx::Panel_SharpLCD _panel_instance;
  lgfx::Bus_SPI        _bus_instance;

  public:

    // overwrite LGFX_Base::clearDisplay(), Panel_SharpLCD::clearDisplay() is faster
    void clearDisplay() { _panel_instance.clearDisplay(); }

    // provide means to get/push the buffer, GFX functions aren't fast with 1bit colors
    uint8_t *getBuffer() { return _panel_instance.getBuffer(); }
    uint32_t getBufferSize() { return _panel_instance.getBufferSize(); }
    void pushBuffer(uint8_t*buf) { _panel_instance.display(buf,0,0,0,0); }

    void setDithering(bool enable)
    {
      auto cfg = _panel_instance.config_detail();
      cfg.enable_dithering = enable;
      _panel_instance.config_detail(cfg);
    }

    SharpDisplay(const uint8_t pin_sck, const uint8_t pin_mosi, const uint8_t pin_ss, const uint16_t width, const uint16_t height, const int16_t pin_dispon=-1, const int16_t pin_extmod=-1)
    {
      {
        auto cfg = _bus_instance.config();
        cfg.freq_write = 4000000;
        cfg.pin_mosi   = pin_mosi;
        cfg.pin_sclk   = pin_sck;
        _bus_instance.config(cfg);
        _panel_instance.bus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance.config();
        cfg.pin_cs = pin_ss;
        cfg.panel_width  = cfg.memory_width  = width; // assuming width is a multiple of 8
        cfg.panel_height = cfg.memory_height = height;
        _panel_instance.config(cfg);
      }

      {
        auto cfg = _panel_instance.config_detail();
        cfg.pin_dispon = pin_dispon;
        cfg.pin_extmod = pin_extmod;
        cfg.prefix_bytes = height>255 ? 2 : 1; // should be 2 when panel height > 255, otherwise 1
        cfg.suffix_bytes = 1; // your mileage may vary
        cfg.enable_dithering   = true; // lose 2-3 fps by enabling this
        cfg.enable_autodisplay = true; // disable this with tft.setAutoDisplay(false)
        _panel_instance.config_detail(cfg);
      }


      setPanel(&_panel_instance);
    }

};
