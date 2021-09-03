#ifndef LGFX_PANEL_ST7735_HPP_
#define LGFX_PANEL_ST7735_HPP_

#include "PanelIlitekCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_ST7735 : public PanelIlitekCommon
  {
    Panel_ST7735(void)
    {
// The ST7735 is available in three resolutions, 132x162 / 128x160 / 132x132, depending on the mode select pin status.
      panel_width  = memory_width  = 132;  // or 128
      panel_height = memory_height = 162;  // or 160 or 132

      freq_write = 27000000;
      freq_read  = 14000000;
      freq_fill  = 27000000;

      len_dummy_read_pixel = 9;
      len_dummy_read_rddid = 1;
    }

  protected:

    struct CMD : public PanelIlitekCommon::CommandCommon
    {
      static constexpr uint8_t FRMCTR1 = 0xB1;
      static constexpr uint8_t FRMCTR2 = 0xB2;
      static constexpr uint8_t FRMCTR3 = 0xB3;
      static constexpr uint8_t INVCTR  = 0xB4;
      static constexpr uint8_t DISSET5 = 0xB6;

      static constexpr uint8_t PWCTR1  = 0xC0;
      static constexpr uint8_t PWCTR2  = 0xC1;
      static constexpr uint8_t PWCTR3  = 0xC2;
      static constexpr uint8_t PWCTR4  = 0xC3;
      static constexpr uint8_t PWCTR5  = 0xC4;
      static constexpr uint8_t VMCTR1  = 0xC5;

      static constexpr uint8_t RDID1   = 0xDA;
      static constexpr uint8_t RDID2   = 0xDB;
      static constexpr uint8_t RDID3   = 0xDC;
      static constexpr uint8_t RDID4   = 0xDD;

      static constexpr uint8_t PWCTR6  = 0xFC;

      static constexpr uint8_t GMCTRP1 = 0xE0;
      static constexpr uint8_t GMCTRN1 = 0xE1;
    };

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t Bcmd[] = {                  // Initialization commands for 7735B screens
        CMD::SWRESET,   CMD_INIT_DELAY, 50,   // Software reset, no args, w/delay
        CMD::SLPOUT ,   CMD_INIT_DELAY, 255,  // Out of sleep mode, no args, w/delay
        CMD::FRMCTR1, 3+CMD_INIT_DELAY,  // Frame rate control, 3 args + delay:
          0x00,                   //     fastest refresh
          0x06,                   //     6 lines front porch
          0x03,                   //     3 lines back porch
          10,                     //     10 ms delay
        CMD::DISSET5, 2      ,  //  Display settings #5, 2 args, no delay:
          0x15,                   //     1 clk cycle nonoverlap, 2 cycle gate rise, 3 cycle osc equalize
          0x02,                   //     Fix on VTL
        CMD::INVCTR , 1      ,  //  Display inversion control, 1 arg:
          0x0,                    //     Line inversion
        CMD::PWCTR1 , 2+CMD_INIT_DELAY,  //  8: Power control, 2 args + delay:
          0x02,                   //     GVDD = 4.7V
          0x70,                   //     1.0uA
          10,                     //     10 ms delay
        CMD::PWCTR2 , 1      ,  //  Power control, 1 arg, no delay:
          0x05,                   //     VGH = 14.7V, VGL = -7.35V
        CMD::PWCTR3 , 2      ,  //  Power control, 2 args, no delay:
          0x01,                   //     Opamp current small
          0x02,                   //     Boost frequency
        CMD::VMCTR1 , 2+CMD_INIT_DELAY,  // 11: Power control, 2 args + delay:
          0x3C,                   //     VCOMH = 4V
          0x38,                   //     VCOML = -1.1V
          10,                     //     10 ms delay
        CMD::PWCTR6 , 2      ,  //  Power control, 2 args, no delay:
          0x11, 0x15,
        CMD::GMCTRP1,16      ,  //  Magical unicorn dust, 16 args, no delay:
          0x09, 0x16, 0x09, 0x20, //     (seriously though, not sure what
          0x21, 0x1B, 0x13, 0x19, //      these config values represent)
          0x17, 0x15, 0x1E, 0x2B,
          0x04, 0x05, 0x02, 0x0E,
        CMD::GMCTRN1,16+CMD_INIT_DELAY,  // Sparkles and rainbows, 16 args + delay:
          0x0B, 0x14, 0x08, 0x1E, //     (ditto)
          0x22, 0x1D, 0x18, 0x1E,
          0x1B, 0x1A, 0x24, 0x2B,
          0x06, 0x06, 0x02, 0x0F,
          10,                     //     10 ms delay
        CMD::NORON  ,   CMD_INIT_DELAY,  // Normal display on, no args, w/delay
          10,                     //     10 ms delay
        CMD::DISPON ,   CMD_INIT_DELAY,  // Main screen turn on, no args, w/delay
          255 ,                   //     255 = 500 ms delay
        0xFF, 0xFF
      };

      switch (listno) {
      case 0: return Bcmd;
      default: return nullptr;
      }
    }
  };

  struct Panel_ST7735S : public Panel_ST7735
  {
  protected:
    const uint8_t* getInitCommands(uint8_t listno) const override {

      static constexpr uint8_t Rcmd1[] = {                 // Init for 7735R, part 1 (red or green tab)
        CMD::SWRESET,   CMD_INIT_DELAY,  //  1: Software reset, 0 args, w/delay
          150,                    //     150 ms delay
        CMD::SLPOUT ,   CMD_INIT_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
          255,                    //     500 ms delay
        CMD::FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
          0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        CMD::FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
          0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        CMD::FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
          0x01, 0x2C, 0x2D,       //     Dot inversion mode
          0x01, 0x2C, 0x2D,       //     Line inversion mode
        CMD::INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
          0x07,                   //     No inversion
        CMD::PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
          0xA2,
          0x02,                   //     -4.6V
          0x84,                   //     AUTO mode
        CMD::PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
          0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
        CMD::PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
          0x0A,                   //     Opamp current small
          0x00,                   //     Boost frequency
        CMD::PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
          0x8A,                   //     BCLK/2, Opamp current small & Medium low
          0x2A,  
        CMD::PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
          0x8A, 0xEE,
        CMD::VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
          0x0E,
        0xFF,0xFF
      };
      static constexpr uint8_t Rcmd2[] = {  // Init for 7735R, part 2 (red or green tab)
        CMD::GMCTRP1, 16      , //  1: 16 args, no delay:
          0x02, 0x1c, 0x07, 0x12,
          0x37, 0x32, 0x29, 0x2d,
          0x29, 0x25, 0x2B, 0x39,
          0x00, 0x01, 0x03, 0x10,
        CMD::GMCTRN1, 16      , //  2: 16 args, no delay:
          0x03, 0x1d, 0x07, 0x06,
          0x2E, 0x2C, 0x29, 0x2D,
          0x2E, 0x2E, 0x37, 0x3F,
          0x00, 0x00, 0x02, 0x10,
        CMD::NORON  ,    CMD_INIT_DELAY, //  3: Normal display on, no args, w/delay
          10,                     //     10 ms delay
        CMD::DISPON ,    CMD_INIT_DELAY, //  4: Main screen turn on, no args w/delay
          100,                    //     100 ms delay
        0xFF,0xFF
      };

      switch (listno) {
      case 0: return Rcmd1;
      case 1: return Rcmd2;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------
 }
}

#endif
