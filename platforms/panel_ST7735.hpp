#ifndef LGFX_PANEL_ST7735_HPP_
#define LGFX_PANEL_ST7735_HPP_

#include "panel_lcd_common.hpp"

namespace lgfx
{
  struct Panel_ST7735_COMMON : public PanelLcdCommon
  {
    static constexpr uint8_t SPI_MODE = 0;

    static constexpr uint8_t LEN_CMD = 8;
    static constexpr uint8_t LEN_PIXEL_READ = 24;
    static constexpr uint8_t LEN_DUMMY_READ_PIXEL = 9;
    static constexpr uint8_t LEN_DUMMY_READ_RDDID = 1;

    struct CMD : public CommandCommon
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

    enum MAD
    { MY  = 0x80
    , MX  = 0x40
    , MV  = 0x20
    , ML  = 0x10
    , BGR = 0x08
    , MH  = 0x04
    , RGB = 0x00
    };
    enum PIX
    { RGB444_2BYTE = 0x03
    , RGB565_2BYTE = 0x05
    , RGB666_3BYTE = 0x06
    };

    inline static uint8_t getAdjustBpp(uint8_t bpp) { return (bpp > 17 ) ? 24 : 16; }
    inline static PIX getPixset(uint8_t bpp) { return (bpp > 16) ? RGB666_3BYTE : RGB565_2BYTE; }

    static constexpr uint8_t PROGMEM Bcmd[] = {                  // Initialization commands for 7735B screens
      18,                       // 18 commands in list:
      CMD::SWRESET,   CMD_INIT_DELAY,  //  1: Software reset, no args, w/delay
        50,                     //     50 ms delay
      CMD::SLPOUT ,   CMD_INIT_DELAY,  //  2: Out of sleep mode, no args, w/delay
        255,                    //     255 = 500 ms delay
      CMD::COLMOD , 1+CMD_INIT_DELAY,  //  3: Set color mode, 1 arg + delay:
        0x05,                   //     16-bit color
        10,                     //     10 ms delay
      CMD::FRMCTR1, 3+CMD_INIT_DELAY,  //  4: Frame rate control, 3 args + delay:
        0x00,                   //     fastest refresh
        0x06,                   //     6 lines front porch
        0x03,                   //     3 lines back porch
        10,                     //     10 ms delay
      CMD::MADCTL , 1      ,  //  5: Memory access ctrl (directions), 1 arg:
        0x40,                   //     Row addr/col addr, bottom to top refresh
      CMD::DISSET5, 2      ,  //  6: Display settings #5, 2 args, no delay:
        0x15,                   //     1 clk cycle nonoverlap, 2 cycle gate
                                //     rise, 3 cycle osc equalize
        0x02,                   //     Fix on VTL
      CMD::INVCTR , 1      ,  //  7: Display inversion control, 1 arg:
        0x0,                    //     Line inversion
      CMD::PWCTR1 , 2+CMD_INIT_DELAY,  //  8: Power control, 2 args + delay:
        0x02,                   //     GVDD = 4.7V
        0x70,                   //     1.0uA
        10,                     //     10 ms delay
      CMD::PWCTR2 , 1      ,  //  9: Power control, 1 arg, no delay:
        0x05,                   //     VGH = 14.7V, VGL = -7.35V
      CMD::PWCTR3 , 2      ,  // 10: Power control, 2 args, no delay:
        0x01,                   //     Opamp current small
        0x02,                   //     Boost frequency
      CMD::VMCTR1 , 2+CMD_INIT_DELAY,  // 11: Power control, 2 args + delay:
        0x3C,                   //     VCOMH = 4V
        0x38,                   //     VCOML = -1.1V
        10,                     //     10 ms delay
      CMD::PWCTR6 , 2      ,  // 12: Power control, 2 args, no delay:
        0x11, 0x15,
      CMD::GMCTRP1,16      ,  // 13: Magical unicorn dust, 16 args, no delay:
        0x09, 0x16, 0x09, 0x20, //     (seriously though, not sure what
        0x21, 0x1B, 0x13, 0x19, //      these config values represent)
        0x17, 0x15, 0x1E, 0x2B,
        0x04, 0x05, 0x02, 0x0E,
      CMD::GMCTRN1,16+CMD_INIT_DELAY,  // 14: Sparkles and rainbows, 16 args + delay:
        0x0B, 0x14, 0x08, 0x1E, //     (ditto)
        0x22, 0x1D, 0x18, 0x1E,
        0x1B, 0x1A, 0x24, 0x2B,
        0x06, 0x06, 0x02, 0x0F,
        10,                     //     10 ms delay
      CMD::CASET  , 4      ,  // 15: Column addr set, 4 args, no delay:
        0x00, 0x02,             //     XSTART = 2
        0x00, 0x81,             //     XEND = 129
      CMD::RASET  , 4      ,  // 16: Row addr set, 4 args, no delay:
        0x00, 0x02,             //     XSTART = 1
        0x00, 0x81,             //     XEND = 160
      CMD::NORON  ,   CMD_INIT_DELAY,  // 17: Normal display on, no args, w/delay
        10,                     //     10 ms delay
      CMD::DISPON ,   CMD_INIT_DELAY,  // 18: Main screen turn on, no args, w/delay
        255 ,                   //     255 = 500 ms delay
      0xFF, 0xFF
    };

    static constexpr uint8_t PROGMEM Rcmd1[] = {                 // Init for 7735R, part 1 (red or green tab)
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
      CMD::INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
      CMD::MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
        0xC8,                   //     row addr/col addr, bottom to top refresh
      CMD::COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
        0x05,                   //     16-bit color
      0xFF,0xFF
    };

    static constexpr uint8_t PROGMEM Rcmd2green[] = {            // Init for 7735R, part 2 (green tab only)
      CMD::CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
        0x00, 0x02,             //     XSTART = 0
        0x00, 0x7F+0x02,        //     XEND = 127
      CMD::RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
        0x00, 0x01,             //     XSTART = 0
        0x00, 0x9F+0x01,        //     XEND = 159
      0xFF,0xFF
    };

    static constexpr uint8_t PROGMEM Rcmd2red[] = {              // Init for 7735R, part 2 (red tab only)
      CMD::CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,             //     XSTART = 0
        0x00, 0x7F,             //     XEND = 127
      CMD::RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,             //     XSTART = 0
        0x00, 0x9F,             //     XEND = 159
      0xFF,0xFF
    };

    static constexpr uint8_t PROGMEM Rcmd3[] = {                 // Init for 7735R, part 3 (red or green tab)
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
  };
  constexpr uint8_t Panel_ST7735_COMMON::Bcmd[];
  constexpr uint8_t Panel_ST7735_COMMON::Rcmd1[];
  constexpr uint8_t Panel_ST7735_COMMON::Rcmd2green[];
  constexpr uint8_t Panel_ST7735_COMMON::Rcmd2red[];
  constexpr uint8_t Panel_ST7735_COMMON::Rcmd3[];


  struct Panel_ST7735_GREENTAB160x80 : public Panel_ST7735_COMMON
  {
    static constexpr int16_t PANEL_WIDTH  = 80;
    static constexpr int16_t PANEL_HEIGHT = 160;
    static constexpr bool HAS_OFFSET = true;

    static rotation_data_t* getRotationData(uint8_t r) {
      static rotation_data_t res {0,0,0};
      r = r & 3;
      res.colstart = (r & 1) ? 1 : 26;
      res.rowstart = (r & 1) ? 26 : 1;
      static constexpr uint8_t madctl_table[] = {
        MAD::MX|        MAD::MY|MAD::MH|MAD::BGR,
                MAD::MV|MAD::MY|        MAD::BGR,
                                        MAD::BGR,
        MAD::MX|MAD::MV|                MAD::BGR,
      };
      res.madctl = madctl_table[r];
      return &res;
    }

    static const uint8_t* getInitCommands(uint8_t listno = 0) {
      switch (listno) {
      case 0: return Panel_ST7735_COMMON::Rcmd1;
      case 1: return Panel_ST7735_COMMON::Rcmd2green;
      case 2: return Panel_ST7735_COMMON::Rcmd3;
      default: return nullptr;
    }
/*
      static constexpr uint8_t init_commands[] = {
      SWRESET,   CMD_INIT_DELAY,  //  1: Software reset, 0 args, w/delay
        150,                    //     150 ms delay
      SLPOUT ,   CMD_INIT_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
        255,                    //     500 ms delay
      FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
        0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
      FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
        0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
      FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
        0x01, 0x2C, 0x2D,       //     Dot inversion mode
        0x01, 0x2C, 0x2D,       //     Line inversion mode
      INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
        0x07,                   //     No inversion
      PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
        0xA2,
        0x02,                   //     -4.6V
        0x84,                   //     AUTO mode
      PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
        0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
      PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
        0x0A,                   //     Opamp current small
        0x00,                   //     Boost frequency
      PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
        0x8A,                   //     BCLK/2, Opamp current small & Medium low
        0x2A,  
      PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
        0x8A, 0xEE,
      VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
        0x0E,
      INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
      MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
        0xC8,                   //     row addr/col addr, bottom to top refresh
      PIXSET , 1      ,  // 15: set color mode, 1 arg, no delay:
        0x05,                   //     16-bit color
      CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
        0x00, 0x02,             //     XSTART = 0
        0x00, 0x7F+0x02,        //     XEND = 127
      PASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
        0x00, 0x01,             //     XSTART = 0
        0x00, 0x9F+0x01,        //     XEND = 159
      GMCTRP1, 16      , //  1: 16 args, no delay:
        0x02, 0x1c, 0x07, 0x12,
        0x37, 0x32, 0x29, 0x2d,
        0x29, 0x25, 0x2B, 0x39,
        0x00, 0x01, 0x03, 0x10,
      GMCTRN1, 16      , //  2: 16 args, no delay:
        0x03, 0x1d, 0x07, 0x06,
        0x2E, 0x2C, 0x29, 0x2D,
        0x2E, 0x2E, 0x37, 0x3F,
        0x00, 0x00, 0x02, 0x10,
      NORON  ,    CMD_INIT_DELAY, //  3: Normal display on, no args, w/delay
        10,                     //     10 ms delay
      DISPON ,    CMD_INIT_DELAY, //  4: Main screen turn on, no args w/delay
        100,                  //     100 ms delay
      0xFF,0xFF               // end
     };
      return init_commands;
*/
    }
  };

}

#endif
