


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

    static constexpr uint8_t CMD_NOP    = 0x00;
    static constexpr uint8_t CMD_SWRESET= 0x01;
    static constexpr uint8_t CMD_RDDID  = 0x04;
    static constexpr uint8_t CMD_RDDST  = 0x09;
    static constexpr uint8_t CMD_SLPIN  = 0x10;
    static constexpr uint8_t CMD_SLPOUT = 0x11;
    static constexpr uint8_t CMD_PTLON  = 0x12;
    static constexpr uint8_t CMD_NORON  = 0x13;
    static constexpr uint8_t CMD_INVOFF = 0x20;
    static constexpr uint8_t CMD_INVON  = 0x21;
    static constexpr uint8_t CMD_DISPOFF= 0x28;
    static constexpr uint8_t CMD_DISPON = 0x29;
    static constexpr uint8_t CMD_CASET  = 0x2A;
    static constexpr uint8_t CMD_RASET  = 0x2B;
    static constexpr uint8_t CMD_RAMWR  = 0x2C;
    static constexpr uint8_t CMD_RAMRD  = 0x2E;
    static constexpr uint8_t CMD_PTLAR  = 0x30;
    static constexpr uint8_t CMD_COLMOD = 0x3A;
    static constexpr uint8_t CMD_MADCTL = 0x36;
    static constexpr uint8_t CMD_MADCTL_MY = 0x80;
    static constexpr uint8_t CMD_MADCTL_MX = 0x40;
    static constexpr uint8_t CMD_MADCTL_MV = 0x20;
    static constexpr uint8_t CMD_MADCTL_ML = 0x10;
    static constexpr uint8_t CMD_MADCTL_RGB = 0x08;
    static constexpr uint8_t CMD_RDID1  = 0xDA;
    static constexpr uint8_t CMD_RDID2  = 0xDB;
    static constexpr uint8_t CMD_RDID3  = 0xDC;
    static constexpr uint8_t CMD_RDID4  = 0xDD;


    static constexpr uint8_t CMD_CSCON   = 0xF0;  // Command Set Control
    static constexpr uint8_t CMD_DIC     = 0xB4;  // Display Inversion Control
    static constexpr uint8_t CMD_EM      = 0xB7;  // Entry Mode Set
    static constexpr uint8_t CMD_PWR1    = 0xC0;  // Power Control 1
    static constexpr uint8_t CMD_PWR2    = 0xC1;  // Power Control 1
    static constexpr uint8_t CMD_PWR3    = 0xC2;  // Power Control 1
    static constexpr uint8_t CMD_VCMPCTL = 0xC5;  // VCOM Control
    static constexpr uint8_t CMD_DOCA    = 0xE8;  // Display Output Ctrl Adjust
    static constexpr uint8_t CMD_PGC     = 0xE0;  // Positive Gamma Control
    static constexpr uint8_t CMD_NGC     = 0xE1;  

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
      CMD_MADCTL_MX | CMD_MADCTL_RGB,
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
    CMD_CSCON, 1,
      0xC3,
    CMD_CSCON, 1,
      0x96,
    CMD_DIC, 1,
      0x01,
    CMD_EM, 1,
      0xC6,
    CMD_PWR1, 2,
      0x80,
      0x45,
    CMD_PWR2, 1,
      0x0F,
    CMD_PWR3, 1,
      0xA7,
    CMD_VCMPCTL, 1,
      0x0A,
    CMD_DOCA, 8,
      0x40,
      0x8A,
      0x00,
      0x00,
      0x29,
      0x19,
      0xA5,
      0x33,
    CMD_PGC, 14,
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
    CMD_NGC, 14,
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
    CMD_CSCON, 1,
      0x3C,
    CMD_CSCON, ST_CMD_DELAY+1,
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
