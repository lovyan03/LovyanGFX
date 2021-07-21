#ifndef LGFX_PANEL_HX8357_HPP_
#define LGFX_PANEL_HX8357_HPP_

#include "PanelIlitekCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_HX8357 : public PanelIlitekCommon
  {
    Panel_HX8357(void)
    {
      panel_width  = memory_width  = 320;
      panel_height = memory_height = 480;

      freq_write = 27000000;
      freq_read  = 12000000;
      freq_fill  = 40000000;

      spi_mode_read = 1;
      len_dummy_read_pixel = 8;
    }
  };

  struct Panel_HX8357B : public Panel_HX8357
  {
  protected:
    struct CMD : public CommandCommon
    {
      static constexpr uint8_t SETDISPMODE       = 0xB4;
      static constexpr uint8_t SET_PANEL_DRIVING = 0xC0;
      static constexpr uint8_t SETDISPLAYFRAME   = 0xC5;
      static constexpr uint8_t SETGAMMA          = 0xC8;
      static constexpr uint8_t SETPOWER          = 0xD0;
      static constexpr uint8_t SETVCOM           = 0xD1;
      static constexpr uint8_t SETPWRNORMAL      = 0xD2;
      static constexpr uint8_t SETPANELRELATED   = 0xE9;
    };
    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CMD::SETPOWER,         3, 0x44, 0x41, 0x06,
          CMD::SETVCOM,          2, 0x40, 0x10,
          CMD::SETPWRNORMAL,     2, 0x05, 0x12,
          CMD::SET_PANEL_DRIVING,5, 0x14, 0x3b, 0x00, 0x02, 0x11,
          CMD::SETDISPLAYFRAME,  1, 0x0c, // 6.8mhz
          CMD::SETPANELRELATED,  1, 0x01, // BGR
          0xEA,                  3, 0x03, 0x00, 0x00,
          0xEB,                  4, 0x40, 0x54, 0x26, 0xdb,
          CMD::SETGAMMA,        12, 0x00, 0x15, 0x00, 0x22, 0x00, 0x08, 0x77, 0x26, 0x66, 0x22, 0x04, 0x00,
          CMD::SETDISPMODE,      1, 0x00, // CPU (DBI) and internal oscillation ??
          CMD::SLPOUT, CMD_INIT_DELAY, 120, // Exit sleep, then delay 120 ms
          CMD::DISPON, CMD_INIT_DELAY,  10, // Main screen turn on, delay 10 ms
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

  struct Panel_HX8357D : public Panel_HX8357
  {
  protected:
    struct CMD : public CommandCommon
    {
      static constexpr uint8_t TEON   = 0x35;
      static constexpr uint8_t TEARLINE=0x44;

      static constexpr uint8_t SETOSC = 0xB0;
      static constexpr uint8_t SETPWR1= 0xB1;
      static constexpr uint8_t SETRGB = 0xB3;
      static constexpr uint8_t SETCYC = 0xB4;
      static constexpr uint8_t SETCOM = 0xB6;
      static constexpr uint8_t SETC   = 0xB9;
      static constexpr uint8_t SETSTBA= 0xC0;

      static constexpr uint8_t SETPANEL= 0xCC;
      static constexpr uint8_t SETGAMMA= 0xE0;
    };
    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CMD::SWRESET, CMD_INIT_DELAY, 20, 
          CMD::SETC   , 3+CMD_INIT_DELAY, 0xFF, 0x83, 0x57, 100, 
          CMD::SETRGB , 4, 0x80, 0x00, 0x06, 0x06,
          CMD::SETCOM , 1, 0x25,
          CMD::SETOSC , 1, 0x68,
          CMD::SETPANEL,1, 0x05,
          CMD::SETPWR1, 6, 0x00, 0x15, 0x1C, 0x1C, 0x83, 0xAA,
          CMD::SETSTBA, 6, 0x50, 0x50, 0x01, 0x3C, 0x1E, 0x08,
          CMD::SETCYC , 7, 0x02, 0x40, 0x00, 0x2A, 0x2A, 0x0D, 0x78,

          CMD::SETGAMMA,34,0x02, 0x0A, 0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b,
                           0x4b, 0x42, 0x3A, 0x27, 0x1B, 0x08, 0x09, 0x03,
                           0x02, 0x0A, 0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b,
                           0x4b, 0x42, 0x3A, 0x27, 0x1B, 0x08, 0x09, 0x03,
                           0x00, 0x01,

          CMD::TEON   , 1, 0x00,
          CMD::TEARLINE,2, 0x00, 0x02,
          CMD::SLPOUT  , CMD_INIT_DELAY, 150, // Exit Sleep
          CMD::DISPON  , CMD_INIT_DELAY, 50,  // display on
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------
 }
}

#endif
