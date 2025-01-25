


#pragma once

#include "Panel_LCD.hpp"
#include <stdio.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_ST7365  : public Panel_LCD
  {
    Panel_ST7365(void)
    {
      _cfg.panel_height = _cfg.memory_height = 480;
      _cfg.panel_width = _cfg.memory_width = 320;

      _cfg.dummy_read_pixel = 16;
    }

  protected:

    // static constexpr uint8_t CMD_RAMCTRL  = 0xB0;
    // static constexpr uint8_t CMD_PORCTRL  = 0xB2;      // Porch control
    // static constexpr uint8_t CMD_GCTRL    = 0xB7;      // Gate control
    // static constexpr uint8_t CMD_VCOMS    = 0xC5;      // VCOMS setting
    // static constexpr uint8_t CMD_LCMCTRL  = 0xC0;      // LCM control
    // static constexpr uint8_t CMD_VDVVRHEN = 0xC2;      // VDV and VRH command enable
    // static constexpr uint8_t CMD_VRHS     = 0xC3;      // VRH set
    // static constexpr uint8_t CMD_VDVSET   = 0xC4;      // VDV setting
    // static constexpr uint8_t CMD_FRCTR2   = 0xC6;      // FR Control 2
    // static constexpr uint8_t CMD_PWCTRL1  = 0xD0;      // Power control 1
    // static constexpr uint8_t CMD_PVGAMCTRL= 0xE0;      // Positive voltage gamma control
    // static constexpr uint8_t CMD_NVGAMCTRL= 0xE1;      // Negative voltage gamma control
    static constexpr uint8_t ST7365_CSCON   = 0xF0;  // Command Set Control
    static constexpr uint8_t ST7365_DIC     = 0xB4;  // Display Inversion Control
    static constexpr uint8_t ST7365_EM      = 0xB7;  // Entry Mode Set
    static constexpr uint8_t ST7365_PWR1    = 0xC0;  // Power Control 1
    static constexpr uint8_t ST7365_PWR2    = 0xC1;  // Power Control 1
    static constexpr uint8_t ST7365_PWR3    = 0xC2;  // Power Control 1
    static constexpr uint8_t ST7365_VCMPCTL = 0xC5;  // VCOM Control
    static constexpr uint8_t ST7365_DOCA    = 0xE8;  // Display Output Ctrl Adjust
    static constexpr uint8_t ST7365_PGC     = 0xE0;  // Positive Gamma Control
    static constexpr uint8_t ST7365_NGC     = 0xE1;  

    static constexpr uint8_t ST_CMD_DELAY    = 0x80;

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
      CMD_SWRESET, ST_CMD_DELAY,     //  1: Software reset, no args, w/delay
      150,                              //     ~150 ms delay
    0x11, ST_CMD_DELAY,
      120,
    CMD_COLMOD, 1,
      0x55,
    CMD_MADCTL, 1,
      MAD_MX | MAD_RGB,
    CMD_CASET, 4,                  //  5: Column addr set, 4 args, no delay:
      0x00,
      0,  //     XSTART = 0
      320>>8,
      320 & 0xFF,              //     XEND = 320
    CMD_RASET, 4,  //  6: Row addr set, 4 args, no delay:
      0x00,
      0,  //     YSTART = 0
      480 >> 8,
      480 & 0xFF,                   //     YEND = 480
    ST7365_CSCON, 1,
      0xC3,
    ST7365_CSCON, 1,
      0x96,
    ST7365_DIC, 1,
      0x01,
    ST7365_EM, 1,
      0xC6,
    ST7365_PWR1, 2,
      0x80,
      0x45,
    ST7365_PWR2, 1,
      0x0F,
    ST7365_PWR3, 1,
      0xA7,
    ST7365_VCMPCTL, 1,
      0x0A,
    ST7365_DOCA, 8,
      0x40,
      0x8A,
      0x00,
      0x00,
      0x29,
      0x19,
      0xA5,
      0x33,
    ST7365_PGC, 14,
      0xD0,
      0x08,
      0x0F,
      0x06,
      0x06,
      0x33,
      0x30,
      0x33,
      0x47,
      0x17,
      0x13,
      0x13,
      0x2B,
      0x31,
    ST7365_NGC, 14,
      0xD0,
      0x0A,
      0x11,
      0x0B,
      0x09,
      0x07,
      0x2F,
      0x33,
      0x47,
      0x38,
      0x15,
      0x16,
      0x2C,
      0x32,
    ST7365_CSCON, 1,
      0x3C,
    ST7365_CSCON, ST_CMD_DELAY+1,
      0x69,
      120,
    CMD_INVON, 0,  // INV ON
      // 0,
    CMD_DISPON, ST_CMD_DELAY,
      20,
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
