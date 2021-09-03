#ifndef LGFX_PANEL_ST7796_HPP_
#define LGFX_PANEL_ST7796_HPP_

#include "PanelIlitekCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_ST7796 : public PanelIlitekCommon
  {
    Panel_ST7796(void)
    {
      panel_width  = memory_width  = 320;
      panel_height = memory_height = 480;

      freq_write = 40000000;
      freq_read  = 14000000;
      freq_fill  = 40000000;

      len_dummy_read_pixel = 8;
    }

  protected:

    struct CMD : public CommandCommon
    {
      static constexpr uint8_t FRMCTR1 = 0xB1;
      static constexpr uint8_t FRMCTR2 = 0xB2;
      static constexpr uint8_t FRMCTR3 = 0xB3;
      static constexpr uint8_t INVCTR  = 0xB4;
      static constexpr uint8_t DFUNCTR = 0xB6;
      static constexpr uint8_t ETMOD   = 0xB7;
      static constexpr uint8_t PWCTR1  = 0xC0;
      static constexpr uint8_t PWCTR2  = 0xC1;
      static constexpr uint8_t PWCTR3  = 0xC2;
      static constexpr uint8_t PWCTR4  = 0xC3;
      static constexpr uint8_t PWCTR5  = 0xC4;
      static constexpr uint8_t VMCTR   = 0xC5;
      static constexpr uint8_t GMCTRP1 = 0xE0; // Positive Gamma Correction
      static constexpr uint8_t GMCTRN1 = 0xE1; // Negative Gamma Correction
      static constexpr uint8_t DOCA    = 0xE8; // Display Output Ctrl Adjust 
      static constexpr uint8_t CSCON   = 0xF0; // Command Set Control
    };

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CMD::SWRESET, CMD_INIT_DELAY, 120, 
          CMD::SLPOUT,  CMD_INIT_DELAY, 120, 
          CMD::CSCON,   1, 0xC3,  // Enable extension command 2 partI
          CMD::CSCON,   1, 0x96,  // Enable extension command 2 partII
          CMD::INVCTR,  1, 0x01,  //1-dot inversion
          CMD::DFUNCTR, 3, 0x80,  //Display Function Control //Bypass
                           0x02,  //Source Output Scan from S1 to S960, Gate Output scan from G1 to G480, scan cycle=2
                           0x3B,  //LCD Drive Line=8*(59+1)
          CMD::DOCA,    8, 0x40,
                           0x8A,
                           0x00,
                           0x00,
                           0x29,  //Source eqaulizing period time= 22.5 us
                           0x19,  //Timing for "Gate start"=25 (Tclk)
                           0xA5,  //Timing for "Gate End"=37 (Tclk), Gate driver EQ function ON
                           0x33,
          CMD::PWCTR2,  1, 0x06,  //Power control2   //VAP(GVDD)=3.85+( vcom+vcom offset), VAN(GVCL)=-3.85+( vcom+vcom offset)
          CMD::PWCTR3,  1, 0xA7,  //Power control 3  //Source driving current level=low, Gamma driving current level=High
          CMD::VMCTR,   1+CMD_INIT_DELAY, 0x18, 120, //VCOM Control    //VCOM=0.9
          CMD::GMCTRP1,14, 0xF0, 0x09, 0x0B, 0x06, 0x04, 0x15, 0x2F,
                           0x54, 0x42, 0x3C, 0x17, 0x14, 0x18, 0x1B,
          CMD::GMCTRN1,14+CMD_INIT_DELAY, 
                           0xE0, 0x09, 0x0B, 0x06, 0x04, 0x03, 0x2B,
                           0x43, 0x42, 0x3B, 0x16, 0x14, 0x17, 0x1B,
                           120,
          CMD::CSCON,   1, 0x3C, //Command Set control // Disable extension command 2 partI
          CMD::CSCON,   1, 0x69, //Command Set control // Disable extension command 2 partII
          0xFF,0xFF, // end
      };
      static constexpr uint8_t list1[] = {
          CMD::SLPOUT,  CMD_INIT_DELAY, 120,    // Exit sleep mode
          CMD::DISPON,  CMD_INIT_DELAY,  20,    // Set display on
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      case 1: return list1;
      default: return nullptr;
      }
    }
    uint8_t getMadCtl(uint8_t r) const override {
      static constexpr uint8_t madctl_table[] = {
               MAD_MX|MAD_MH              ,
        MAD_MV                            ,
                             MAD_MY|MAD_ML,
        MAD_MV|MAD_MX|MAD_MY|MAD_MH|MAD_ML,
               MAD_MX|MAD_MH|MAD_MY|MAD_ML,
        MAD_MV|MAD_MX|MAD_MH              ,
                                         0,
        MAD_MV|              MAD_MY|MAD_ML,
      };
      return madctl_table[r];
    }

    const uint8_t* getColorDepthCommands(uint8_t* buf, color_depth_t depth) override
    {
      auto res = PanelIlitekCommon::getColorDepthCommands(buf, depth);
      read_depth = write_depth; // 読込時のデータは書込と同じ並びになる
      return res;
    }
  };

//----------------------------------------------------------------------------
 }
}

#endif