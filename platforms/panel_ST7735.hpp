#ifndef LGFX_PANEL_ST7735_HPP_
#define LGFX_PANEL_ST7735_HPP_

#include "panel_ilitek_common.hpp"

namespace lgfx
{
  template <class CFG>
  class Panel_ST7735 : public PanelIlitekCommon<CFG>
  {
  public:
    Panel_ST7735() : PanelIlitekCommon<CFG>()
    {
      if (!this->_ram_width   ) this->_ram_width = 132;
      if (!this->_ram_height  ) this->_ram_height = 162;
      if (!this->_panel_width ) this->_panel_width = 132;
      if (!this->_panel_height) this->_panel_height = 162;
      this->_len_command = 8;
      this->_len_read_pixel = 24;
      this->_len_dummy_read_pixel = 9;
      this->_len_dummy_read_rddid = 1;
    }

  protected:
    enum colmod_t
    { RGB444_2BYTE = 0x03
    , RGB565_2BYTE = 0x05
    , RGB666_3BYTE = 0x06
    };
    uint8_t getColMod(uint8_t bpp) const override { return (bpp > 16) ? RGB666_3BYTE : RGB565_2BYTE; }

    enum MAD
    { MY  = 0x80
    , MX  = 0x40
    , MV  = 0x20
    , ML  = 0x10
    , BGR = 0x08
    , MH  = 0x04
    , RGB = 0x00
    };
    uint8_t getMadCtl(uint8_t r) const override {
      static constexpr uint8_t madctl_table[] = {
        MAD::MX|        MAD::MY|MAD::MH,
                MAD::MV|MAD::MY        ,
                                      0,
        MAD::MX|MAD::MV                ,
      };
      r = r & 3;
      return madctl_table[r];
    }

    struct CMD : public PanelIlitekCommon<CFG>::CommandCommon
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
  };

  template <class CFG>
  class Panel_ST7735B : public Panel_ST7735<CFG>
  {
    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t PROGMEM Bcmd[] = {                  // Initialization commands for 7735B screens
        Panel_ST7735<CFG>::CMD::SWRESET,   PanelIlitekCommon<CFG>::CMD_INIT_DELAY, 50,   // Software reset, no args, w/delay
        Panel_ST7735<CFG>::CMD::SLPOUT ,   PanelIlitekCommon<CFG>::CMD_INIT_DELAY, 255,  // Out of sleep mode, no args, w/delay
        Panel_ST7735<CFG>::CMD::FRMCTR1, 3+PanelIlitekCommon<CFG>::CMD_INIT_DELAY,  // Frame rate control, 3 args + delay:
          0x00,                   //     fastest refresh
          0x06,                   //     6 lines front porch
          0x03,                   //     3 lines back porch
          10,                     //     10 ms delay
        Panel_ST7735<CFG>::CMD::DISSET5, 2      ,  //  Display settings #5, 2 args, no delay:
          0x15,                   //     1 clk cycle nonoverlap, 2 cycle gate rise, 3 cycle osc equalize
          0x02,                   //     Fix on VTL
        Panel_ST7735<CFG>::CMD::INVCTR , 1      ,  //  Display inversion control, 1 arg:
          0x0,                    //     Line inversion
        Panel_ST7735<CFG>::CMD::PWCTR1 , 2+PanelIlitekCommon<CFG>::CMD_INIT_DELAY,  //  8: Power control, 2 args + delay:
          0x02,                   //     GVDD = 4.7V
          0x70,                   //     1.0uA
          10,                     //     10 ms delay
        Panel_ST7735<CFG>::CMD::PWCTR2 , 1      ,  //  Power control, 1 arg, no delay:
          0x05,                   //     VGH = 14.7V, VGL = -7.35V
        Panel_ST7735<CFG>::CMD::PWCTR3 , 2      ,  //  Power control, 2 args, no delay:
          0x01,                   //     Opamp current small
          0x02,                   //     Boost frequency
        Panel_ST7735<CFG>::CMD::VMCTR1 , 2+PanelIlitekCommon<CFG>::CMD_INIT_DELAY,  // 11: Power control, 2 args + delay:
          0x3C,                   //     VCOMH = 4V
          0x38,                   //     VCOML = -1.1V
          10,                     //     10 ms delay
        Panel_ST7735<CFG>::CMD::PWCTR6 , 2      ,  //  Power control, 2 args, no delay:
          0x11, 0x15,
        Panel_ST7735<CFG>::CMD::GMCTRP1,16      ,  //  Magical unicorn dust, 16 args, no delay:
          0x09, 0x16, 0x09, 0x20, //     (seriously though, not sure what
          0x21, 0x1B, 0x13, 0x19, //      these config values represent)
          0x17, 0x15, 0x1E, 0x2B,
          0x04, 0x05, 0x02, 0x0E,
        Panel_ST7735<CFG>::CMD::GMCTRN1,16+PanelIlitekCommon<CFG>::CMD_INIT_DELAY,  // Sparkles and rainbows, 16 args + delay:
          0x0B, 0x14, 0x08, 0x1E, //     (ditto)
          0x22, 0x1D, 0x18, 0x1E,
          0x1B, 0x1A, 0x24, 0x2B,
          0x06, 0x06, 0x02, 0x0F,
          10,                     //     10 ms delay
        Panel_ST7735<CFG>::CMD::NORON  ,   PanelIlitekCommon<CFG>::CMD_INIT_DELAY,  // Normal display on, no args, w/delay
          10,                     //     10 ms delay
        Panel_ST7735<CFG>::CMD::DISPON ,   PanelIlitekCommon<CFG>::CMD_INIT_DELAY,  // Main screen turn on, no args, w/delay
          255 ,                   //     255 = 500 ms delay
        0xFF, 0xFF
      };
      switch (listno) {
      case 0: return Bcmd;
      default: return nullptr;
      }
    }
  };

  template <class CFG>
  class Panel_ST7735R : public Panel_ST7735<CFG>
  {
    const uint8_t* getInitCommands(uint8_t listno) const override {

      static constexpr uint8_t PROGMEM Rcmd1[] = {                 // Init for 7735R, part 1 (red or green tab)
        Panel_ST7735<CFG>::CMD::SWRESET,   PanelIlitekCommon<CFG>::CMD_INIT_DELAY,  //  1: Software reset, 0 args, w/delay
          150,                    //     150 ms delay
        Panel_ST7735<CFG>::CMD::SLPOUT ,   PanelIlitekCommon<CFG>::CMD_INIT_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
          255,                    //     500 ms delay
        Panel_ST7735<CFG>::CMD::FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
          0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        Panel_ST7735<CFG>::CMD::FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
          0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        Panel_ST7735<CFG>::CMD::FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
          0x01, 0x2C, 0x2D,       //     Dot inversion mode
          0x01, 0x2C, 0x2D,       //     Line inversion mode
        Panel_ST7735<CFG>::CMD::INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
          0x07,                   //     No inversion
        Panel_ST7735<CFG>::CMD::PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
          0xA2,
          0x02,                   //     -4.6V
          0x84,                   //     AUTO mode
        Panel_ST7735<CFG>::CMD::PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
          0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
        Panel_ST7735<CFG>::CMD::PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
          0x0A,                   //     Opamp current small
          0x00,                   //     Boost frequency
        Panel_ST7735<CFG>::CMD::PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
          0x8A,                   //     BCLK/2, Opamp current small & Medium low
          0x2A,  
        Panel_ST7735<CFG>::CMD::PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
          0x8A, 0xEE,
        Panel_ST7735<CFG>::CMD::VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
          0x0E,
        0xFF,0xFF
      };

      static constexpr uint8_t PROGMEM Rcmd2[] = {  // Init for 7735R, part 2 (red or green tab)
        Panel_ST7735<CFG>::CMD::GMCTRP1, 16      , //  1: 16 args, no delay:
          0x02, 0x1c, 0x07, 0x12,
          0x37, 0x32, 0x29, 0x2d,
          0x29, 0x25, 0x2B, 0x39,
          0x00, 0x01, 0x03, 0x10,
        Panel_ST7735<CFG>::CMD::GMCTRN1, 16      , //  2: 16 args, no delay:
          0x03, 0x1d, 0x07, 0x06,
          0x2E, 0x2C, 0x29, 0x2D,
          0x2E, 0x2E, 0x37, 0x3F,
          0x00, 0x00, 0x02, 0x10,
        Panel_ST7735<CFG>::CMD::NORON  ,    PanelIlitekCommon<CFG>::CMD_INIT_DELAY, //  3: Normal display on, no args, w/delay
          10,                     //     10 ms delay
        Panel_ST7735<CFG>::CMD::DISPON ,    PanelIlitekCommon<CFG>::CMD_INIT_DELAY, //  4: Main screen turn on, no args w/delay
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

//#if defined(ARDUINO_M5Stick_C) // M5Stick C
  struct CfgM5StickC {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr bool spi_half_duplex = true;
    static constexpr int spi_mosi = 15;
    static constexpr int spi_miso = 14;
    static constexpr int spi_sclk = 13;
    static constexpr int spi_cs   =  5;
    static constexpr int spi_dc   = 23;
    static constexpr int panel_rst = 18;
    static constexpr int freq_write = 27000000;
    static constexpr int freq_read  = 14000000;
    static constexpr int freq_fill  = 27000000;
    static constexpr int panel_x      = 26;
    static constexpr int panel_y      = 1;
    static constexpr int panel_width  = 80;
    static constexpr int panel_height = 160;
  };

  struct Panel_M5StickC : public Panel_ST7735R<CfgM5StickC>
  {};
//#endif

}

#endif
