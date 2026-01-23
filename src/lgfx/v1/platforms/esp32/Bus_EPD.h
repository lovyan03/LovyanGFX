/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#if __has_include(<soc/soc_caps.h>)
#include <soc/soc_caps.h> 
#endif
#if SOC_LCD_I80_SUPPORTED

#include <esp_lcd_panel_io.h>
#include <stdint.h>
#include <stddef.h>

#include "../../Bus.hpp"
#include "../../misc/enum.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

class Bus_EPD : public Bus_NULL
{
  public:
  struct config_t {
    uint32_t bus_speed;
    union
    {
      int8_t pin_data[16] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
      struct
      {
        int8_t pin_d0;
        int8_t pin_d1;
        int8_t pin_d2;
        int8_t pin_d3;
        int8_t pin_d4;
        int8_t pin_d5;
        int8_t pin_d6;
        int8_t pin_d7;
        int8_t pin_d8;
        int8_t pin_d9;
        int8_t pin_d10;
        int8_t pin_d11;
        int8_t pin_d12;
        int8_t pin_d13;
        int8_t pin_d14;
        int8_t pin_d15;
      };
    };
    union
    {
      int8_t pin_ctrl[7] = { -1,-1,-1,-1,-1,-1,-1 };
      struct
      {
        int8_t pin_pwr;
        int8_t pin_sph; // start pulse source driver (XSTL)
        int8_t pin_spv; // start pulse gate driver
        int8_t pin_oe;  // output enable source driver (XOE)
        int8_t pin_le;  // latch enable source driver (XLE)
        int8_t pin_cl;  // clock source driver (XCL)
        int8_t pin_ckv; // clock gate driver
      };
    };
    uint8_t bus_width;
  };
  bool init(void) override;

  bool busy(void) const override { return _bus_busy; }
  void wait(void) override;

  void config(const config_t& cfg) { _config = cfg; };
  const config_t& config(void) { return _config; };

  void writeScanLine(const uint8_t *data, uint32_t length);

  void beginTransaction(void) override;
  void endTransaction(void) override;

  virtual bool powerControl(bool power_on);
  virtual void scanlineDone(void);
protected:
  config_t _config;
  esp_lcd_i80_bus_handle_t _i80_bus_handle = nullptr;
  esp_lcd_panel_io_handle_t _io_handle = nullptr;

  bool _pwr_on = false;
  
  volatile bool _bus_busy = false;
  
  static bool notify_line_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *me);
};

// extern Bus_EPD bus_epd_tmp;

//----------------------------------------------------------------------------
 }
}

#endif
