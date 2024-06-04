#include "LovyanGFX.h"
#include "Arduino.h"
#include <stdio.h>
#include <SPI.h>
#include "linux/gpio/LinuxGPIOPin.h"
#include "PortduinoGPIO.h"
#include <iostream>

int initGPIOPin(int pinNum, std::string gpioChipName);

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Touch_FT5x06 _touch_instance;

public:
  LGFX(void)
  {
    {
      // Create I2C

      //Init the TFT and touch
      auto buscfg = _bus_instance.config();
      buscfg.spi_mode = 0;
      buscfg.pin_dc = 25;
      
      // to use spidevX.Y
      int x = 0;
      int y = 0;
      buscfg.spi_host = x + y << 4;
      _bus_instance.config(buscfg);            // applies the set value to the bus.
      _panel_instance.setBus(&_bus_instance); // set the bus on the panel.
    }
    {
      auto cfg = _panel_instance.config(); // Gets a structure for display panel settings.

      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;                       // Rotation direction value offset 0~7 (4~7 is mirrored)
      cfg.invert = 0;

      _panel_instance.config(cfg);
    }
    {
      auto touch_cfg = _touch_instance.config();
      touch_cfg.x_min = 0;
      touch_cfg.x_max = 319;
      touch_cfg.y_min = 0;
      touch_cfg.y_max = 239;
      touch_cfg.pin_int = 24;
      touch_cfg.bus_shared = true;
      touch_cfg.offset_rotation = 1;
      touch_cfg.i2c_addr = 0x38;
      _touch_instance.config(touch_cfg);
      _panel_instance.setTouch(&_touch_instance);
    }
    setPanel(&_panel_instance); // Sets the panel to use.
  }
};

LGFX *display;
LGFX_Sprite canvas;
