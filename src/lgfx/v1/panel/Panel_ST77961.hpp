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

#include "Panel_LCD.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_ST77961  : public Panel_LCD
  {
    Panel_ST77961(void)
    {
      _cfg.panel_width  = _cfg.memory_width  = 360;
      _cfg.panel_height = _cfg.memory_height = 390;

      _cfg.dummy_read_pixel = 8;
    }

  protected:

    static constexpr uint8_t CMD_GAMCTRP1 = 0xE0; // Positive Gamma Correction
    static constexpr uint8_t CMD_GAMCTRN1 = 0xE1; // Negative Gamma Correction
    static constexpr uint8_t CMD_CSC1     = 0xF0; // Command Set Control 1
    static constexpr uint8_t CMD_CSC2     = 0xF1; // Command Set Control 2
    static constexpr uint8_t CMD_CSC3     = 0xF2; // Command Set Control 3
    static constexpr uint8_t CMD_CSC4     = 0xF3; // Command Set Control 4

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CMD_CSC1, 1, 0x08,      // Test command page enable
          CMD_CSC3, 1, 0x08,      // Test command page enable
          0x9B, 1, 0x51,      // ???
          0x86, 1, 0x53,      // ???
          CMD_CSC3, 1, 0x80,      // Command set 3 but argument ??
          CMD_CSC1, 1, 0x00,      // Command 2 disable
          CMD_CSC1, 1, 0x01,      // Command 2 enable
          CMD_CSC2, 1, 0x01,      // Command 2 enable
          0xB0, 1, 0x54,      // VRHPS - VRHP Set: / 5.750 + (vcom offset) / default =66h
          0xB1, 1, 0x3F,      // VRHNS - VRHN Set: / -3.450 + (vcom offset) / default =4Dh
          0xB2, 1, 0x2A,      // VCOMS - VCOM GND SET: 1.150 / default =2Ch
          0xB4, 1, 0x46,      // GAMOPPS - GVDD_GVEE_SET: Negative Gamma OP Power Set (VNDAC) = -3.8, Positive Gamma OP Power Set (VPDAC) = 6.1 / default =88h
          0xB5, 1, 0x34,      // STEP14S - STEP SET1: AVCL=-4.05, AVDD=6.42 / default =45h
          0xB6, 1, 0xD5,      // STEP23S - STEP SET2, VGL=-12.00, VGH/VGHS=12.0, default =89h
          0xB7, 1, 0x30,      // ???
          0xB8, 1, 0x04,      // ???
          0xBA, 1, 0x00,      // TCONS - TCON_SET: 1 line = default
          0xBB, 1, 0x08,      // RGBVBP - RGB_VBP: RGB interface Vsync back porch setting = default
          0xBC, 1, 0x08,      // RGBHBP - RGB_HBP: RGB interface Hsync back porch setting = default
          0xBD, 1, 0x00,      // RGBSET - RGB_SET: = default
          0xC0, 1, 0x80,      // FRCTRA1 - Frame Rate Control A1 in Normal Mode = default
          0xC1, 1, 0x10,      // FRCTRA2 - Frame Rate Control A2 in Normal Mode ? default = 10h
          0xC2, 1, 0x37,      // FRCTRA3 - Frame Rate Control A3 in Normal Mode = 53.86Hz / default =30h (61.71Hz)
          0xC3, 1, 0x80,      // FRCTRB1 - Frame Rate Control B1 in Idle Mode = dot inversion / default = 00h (column inversion)
          0xC4, 1, 0x10,      // FRCTRB2 - Frame Rate Control B2 in Idle Mode = 10h ??? / default = 21h (Back porch and Front porch setting in idle mode)
          0xC5, 1, 0x37,      // FRCTRB3 - Frame Rate Control B3 in Idle Mode = 53.86Hz / default =31h (60.45Hz)
          0xC6, 1, 0xA9,      // PWRCTRA1 - Power Control A1 in Normal Mode =default
          0xC7, 1, 0x41,      // PWRCTRA2 - Power Control A2 in Normal Mode =default
          0xC8, 1, 0x51,      // PWRCTRA3 - Power Control A3 in Normal Mode =default
          0xC9, 1, 0xA9,      // PWRCTRB1 - Power Control B1 in Idle Mode =default
          0xCA, 1, 0x41,      // PWRCTRB2 - Power Control B2 in Idle Mode =default
          0xCB, 1, 0x51,      // PWRCTRB3 - Power Control B3 in Idle Mode =default
          0xD0, 1, 0x91,      // RESSET1 - Resolution Set 1 =default
          0xD1, 1, 0x68,      // RESSET2 - Resolution Set 2 =default
          0xD2, 1, 0x69,      // RESSET3 - Resolution Set 3 / default=86h
          0xF5, 2, 0x00, 0xA5,
          0xDD, 1, 0x35,      // VCMOFSET - VCOM OFFSET SET / default=35h
          0xDE, 1, 0x35,      // VCMOFNSET - VCOM OFFSET NEW SET / default=40h
          CMD_CSC2, 1, 0x10,      // Exit from cmd set 2
          CMD_CSC1, 1, 0x00,
          CMD_CSC1, 1, 0x02,  // Enter gamma correction
          CMD_GAMCTRP1, 14, 0x70, 0x09, 0x12, 0x0C, 0x0B, 0x27, 0x38, 0x54, 0x4E, 0x19, 0x15, 0x15, 0x2C, 0x2F, // GAMCTRP1 - Positive Voltage Gamma Control
          CMD_GAMCTRN1, 14, 0x70, 0x08, 0x11, 0x0C, 0x0B, 0x27, 0x38, 0x43, 0x4C, 0x18, 0x14, 0x14, 0x2B, 0x2D, // GAMCTRN1 - Negative Voltage Gamma Control
          CMD_CSC1, 1, 0x10,      // Exit gamma correction
          CMD_CSC4, 1, 0x10,      // GIP Command page enable
          0xE0, 1, 0x0A,
          0xE1, 1, 0x00,
          0xE2, 1, 0x0B,
          0xE3, 1, 0x00,
          0xE4, 1, 0xE0,
          0xE5, 1, 0x06,
          0xE6, 1, 0x21,
          0xE7, 1, 0x00,
          0xE8, 1, 0x05,
          0xE9, 1, 0x82,
          0xEA, 1, 0xDF,
          0xEB, 1, 0x89,
          0xEC, 1, 0x20,
          0xED, 1, 0x14,
          0xEE, 1, 0xFF,
          0xEF, 1, 0x00,
          0xF8, 1, 0xFF,
          0xF9, 1, 0x00,
          0xFA, 1, 0x00,
          0xFB, 1, 0x30,
          0xFC, 1, 0x00,
          0xFD, 1, 0x00,
          0xFE, 1, 0x00,
          0xFF, 1, 0x00,
          0x60, 1, 0x42,
          0x61, 1, 0xE0,
          0x62, 1, 0x40,
          0x63, 1, 0x40,
          0x64, 1, 0x02,
          0x65, 1, 0x00,
          0x66, 1, 0x40,
          0x67, 1, 0x03,
          0x68, 1, 0x00,
          0x69, 1, 0x00,
          0x6A, 1, 0x00,
          0x6B, 1, 0x00,
          0x70, 1, 0x42,
          0x71, 1, 0xE0,
          0x72, 1, 0x40,
          0x73, 1, 0x40,
          0x74, 1, 0x02,
          0x75, 1, 0x00,
          0x76, 1, 0x40,
          0x77, 1, 0x03,
          0x78, 1, 0x00,
          0x79, 1, 0x00,
          0x7A, 1, 0x00,
          0x7B, 1, 0x00,
          0x80, 1, 0x38,
          0x81, 1, 0x00,
          0x82, 1, 0x04,
          0x83, 1, 0x02,
          0x84, 1, 0xDC,
          0x85, 1, 0x00,
          0x86, 1, 0x00,
          0x87, 1, 0x00,
          0x88, 1, 0x38,
          0x89, 1, 0x00,
          0x8A, 1, 0x06,
          0x8B, 1, 0x02,
          0x8C, 1, 0xDE,
          0x8D, 1, 0x00,
          0x8E, 1, 0x00,
          0x8F, 1, 0x00,
          0x90, 1, 0x38,
          0x91, 1, 0x00,
          0x92, 1, 0x08,
          0x93, 1, 0x02,
          0x94, 1, 0xE0,
          0x95, 1, 0x00,
          0x96, 1, 0x00,
          0x97, 1, 0x00,
          0x98, 1, 0x38,
          0x99, 1, 0x00,
          0x9A, 1, 0x0A,
          0x9B, 1, 0x02,
          0x9C, 1, 0xE2,
          0x9D, 1, 0x00,
          0x9E, 1, 0x00,
          0x9F, 1, 0x00,
          0xA0, 1, 0x38,
          0xA1, 1, 0x00,
          0xA2, 1, 0x03,
          0xA3, 1, 0x02,
          0xA4, 1, 0xDB,
          0xA5, 1, 0x00,
          0xA6, 1, 0x00,
          0xA7, 1, 0x00,
          0xA8, 1, 0x38,
          0xA9, 1, 0x00,
          0xAA, 1, 0x05,
          0xAB, 1, 0x02,
          0xAC, 1, 0xDD,
          0xAD, 1, 0x00,
          0xAE, 1, 0x00,
          0xAF, 1, 0x00,
          0xB0, 1, 0x38,
          0xB1, 1, 0x00,
          0xB2, 1, 0x07,
          0xB3, 1, 0x02,
          0xB4, 1, 0xDF,
          0xB5, 1, 0x00,
          0xB6, 1, 0x00,
          0xB7, 1, 0x00,
          0xB8, 1, 0x38,
          0xB9, 1, 0x00,
          0xBA, 1, 0x09,
          0xBB, 1, 0x02,
          0xBC, 1, 0xE1,
          0xBD, 1, 0x00,
          0xBE, 1, 0x00,
          0xBF, 1, 0x00,
          0xC0, 1, 0x22,
          0xC1, 1, 0xAA,
          0xC2, 1, 0x65,
          0xC3, 1, 0x74,
          0xC4, 1, 0x47,
          0xC5, 1, 0x56,
          0xC6, 1, 0x00,
          0xC7, 1, 0x88,
          0xC8, 1, 0x99,
          0xC9, 1, 0x33,
          0xD0, 1, 0x11,
          0xD1, 1, 0xAA,
          0xD2, 1, 0x65,
          0xD3, 1, 0x74,
          0xD4, 1, 0x47,
          0xD5, 1, 0x56,
          0xD6, 1, 0x00,
          0xD7, 1, 0x88,
          0xD8, 1, 0x99,
          0xD9, 1, 0x33,
          CMD_CSC4, 1, 0x01,      // Exit GIP
          CMD_CSC1, 1, 0x00,      // Disable all commands
          CMD_CSC1, 1, 0x01,      // Command2 page enable
          CMD_CSC2, 1, 0x01,      // Command2 page enable
          0xA0, 1, 0x0B,      // OTP MODE SEL
          0xA3, 1, 0x2A,      // OTP PAGE ADDR
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,  // OTP CMD ACK
          0xA3, 1, 0x2B,
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,
          0xA3, 1, 0x2C,
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,
          0xA3, 1, 0x2D,
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,
          0xA3, 1, 0x2E,
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,
          0xA3, 1, 0x2F,
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,
          0xA3, 1, 0x30,
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,
          0xA3, 1, 0x31,
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,
          0xA3, 1, 0x32,
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,
          0xA3, 1, 0x33,
          0xA5, 1 + CMD_INIT_DELAY, 0xC3, 1,
          0xA0, 1, 0x09,      // OTP MODE SEL
          CMD_CSC2, 1, 0x10,      // ?
          CMD_CSC1, 1, 0x00,      // Disable all
          CMD_CASET, 4, 0x00, 0x00, 0x01, 0x67,  // CASET - Column Address Set =default (0:359)
          CMD_RASET, 4, 0x01, 0x68, 0x01, 0x68,  // RASET - Row Address Set =360 ?
          0x4D, 1, 0x00,      // RAMCLSETR - Clear RED
          0x4E, 1, 0x00,      // RAMCLSETG - Clear GREEN
          0x4F, 1, 0x00,      // RAMCLSETB - Clear BLUE
          0x4C, 1 + CMD_INIT_DELAY, 0x01, 10, // RAMCLACT - Memory Clear Act (Trigger IC to fill all pixels data in RAM)
          0x4C, 1, 0x00,      // Same - no function
          CMD_CASET, 4, 0x00, 0x00, 0x01, 0x67,  // CASET - Column Address Set =default (0:359)
          CMD_RASET, 4, 0x00, 0x00, 0x01, 0x67,  // CASET - Column Address Set =default (0:359)
          CMD_INVON, 0,            // INVON - Display Inversion On
          CMD_SLPOUT, CMD_INIT_DELAY, 120,  // SLPOUT - Sleep Out
          CMD_DISPON, 0,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
    void setColorDepth_impl(color_depth_t depth) override 
    {
      _write_depth = ((int)depth & color_depth_t::bit_mask) > 16 ? rgb888_3Byte : rgb565_2Byte;
      _read_depth = _write_depth;
    }

  };

//----------------------------------------------------------------------------
 }
}