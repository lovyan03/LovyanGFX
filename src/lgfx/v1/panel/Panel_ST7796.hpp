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

  struct Panel_ST7796  : public Panel_LCD
  {
    Panel_ST7796(void)
    {
      _cfg.panel_width  = _cfg.memory_width  = 320;
      _cfg.panel_height = _cfg.memory_height = 480;

      _cfg.dummy_read_pixel = 8;
    }

  protected:

    static constexpr uint8_t CMD_FRMCTR1 = 0xB1;
    static constexpr uint8_t CMD_FRMCTR2 = 0xB2;
    static constexpr uint8_t CMD_FRMCTR3 = 0xB3;
    static constexpr uint8_t CMD_INVCTR  = 0xB4;
    static constexpr uint8_t CMD_DFUNCTR = 0xB6;
    static constexpr uint8_t CMD_ETMOD   = 0xB7;
    static constexpr uint8_t CMD_PWCTR1  = 0xC0;
    static constexpr uint8_t CMD_PWCTR2  = 0xC1;
    static constexpr uint8_t CMD_PWCTR3  = 0xC2;
    static constexpr uint8_t CMD_PWCTR4  = 0xC3;
    static constexpr uint8_t CMD_PWCTR5  = 0xC4;
    static constexpr uint8_t CMD_VMCTR   = 0xC5;
    static constexpr uint8_t CMD_GMCTRP1 = 0xE0; // Positive Gamma Correction
    static constexpr uint8_t CMD_GMCTRN1 = 0xE1; // Negative Gamma Correction
    static constexpr uint8_t CMD_DOCA    = 0xE8; // Display Output Ctrl Adjust 
    static constexpr uint8_t CMD_CSCON   = 0xF0; // Command Set Control

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CMD_CSCON,   1, 0xC3,  // Enable extension command 2 partI
          CMD_CSCON,   1, 0x96,  // Enable extension command 2 partII
          CMD_INVCTR,  1, 0x01,  //1-dot inversion
          CMD_DFUNCTR, 3, 0x80,  //Display Function Control //Bypass
                          0x22,  //Source Output Scan from S1 to S960, Gate Output scan from G1 to G480, scan cycle=2
                          0x3B,  //LCD Drive Line=8*(59+1)
          CMD_DOCA,    8, 0x40,
                          0x8A,
                          0x00,
                          0x00,
                          0x29,  //Source eqaulizing period time= 22.5 us
                          0x19,  //Timing for "Gate start"=25 (Tclk)
                          0xA5,  //Timing for "Gate End"=37 (Tclk), Gate driver EQ function ON
                          0x33,
          CMD_PWCTR2,  1, 0x06,  //Power control2   //VAP(GVDD)=3.85+( vcom+vcom offset), VAN(GVCL)=-3.85+( vcom+vcom offset)
          CMD_PWCTR3,  1, 0xA7,  //Power control 3  //Source driving current level=low, Gamma driving current level=High
          CMD_VMCTR,   1+CMD_INIT_DELAY, 0x18, 120, //VCOM Control    //VCOM=0.9
          CMD_GMCTRP1,14, 0xF0, 0x09, 0x0B, 0x06, 0x04, 0x15, 0x2F,
                          0x54, 0x42, 0x3C, 0x17, 0x14, 0x18, 0x1B,
          CMD_GMCTRN1,14+CMD_INIT_DELAY, 
                          0xE0, 0x09, 0x0B, 0x06, 0x04, 0x03, 0x2B,
                          0x43, 0x42, 0x3B, 0x16, 0x14, 0x17, 0x1B,
                          120,
          CMD_CSCON,   1, 0x3C, //Command Set control // Disable extension command 2 partI
          CMD_CSCON,   1, 0x69, //Command Set control // Disable extension command 2 partII

          CMD_SLPOUT, 0+CMD_INIT_DELAY, 130,    // Exit sleep mode
          CMD_IDMOFF, 0,
          CMD_DISPON, 0+CMD_INIT_DELAY, 100,
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
      _read_depth = _write_depth; // 読込時のデータは書込時と同じ並びになる;
    }
  };

//----------------------------------------------------------------------------
 }
}
