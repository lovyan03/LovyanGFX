#ifndef LGFX_PANEL_ILI9341_HPP_
#define LGFX_PANEL_ILI9341_HPP_

#include "panel_ilitek_common.hpp"

namespace lgfx
{
  template <class CFG>
  class Panel_ILI9341 : public PanelIlitekCommon<CFG>
  {
  public:
    Panel_ILI9341() : PanelIlitekCommon<CFG>()
    {
      if (!this->_ram_width   ) this->_ram_width = 240;
      if (!this->_ram_height  ) this->_ram_height = 320;
      if (!this->_panel_width ) this->_panel_width = 240;
      if (!this->_panel_height) this->_panel_height = 320;
      this->_len_command = 8;
      this->_len_read_pixel = 24;
      this->_len_dummy_read_pixel = 8;
      this->_len_dummy_read_rddid = 0;
    }

  protected:
    enum colmod_t
    { RGB565_2BYTE = 0x55
    , RGB666_3BYTE = 0x66
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
                MAD::MX        ,
        MAD::MV                ,
                        MAD::MY,
        MAD::MV|MAD::MX|MAD::MY,
                MAD::MX|MAD::MY,
        MAD::MV|MAD::MX        ,
                              0,
        MAD::MV|        MAD::MY,
      };
      r = r & 7;
      return madctl_table[r];
    }

    struct CMD : public PanelIlitekCommon<CFG>::CommandCommon
    {
      static constexpr uint8_t FRMCTR1 = 0xB1;
      static constexpr uint8_t FRMCTR2 = 0xB2;
      static constexpr uint8_t FRMCTR3 = 0xB3;
      static constexpr uint8_t INVCTR  = 0xB4;
      static constexpr uint8_t DFUNCTR = 0xB6;
      static constexpr uint8_t PWCTR1  = 0xC0;
      static constexpr uint8_t PWCTR2  = 0xC1;
      static constexpr uint8_t PWCTR3  = 0xC2;
      static constexpr uint8_t PWCTR4  = 0xC3;
      static constexpr uint8_t PWCTR5  = 0xC4;
      static constexpr uint8_t VMCTR1  = 0xC5;
      static constexpr uint8_t VMCTR2  = 0xC7;
      static constexpr uint8_t GMCTRP1 = 0xE0; // Positive Gamma Correction (E0h)
      static constexpr uint8_t GMCTRN1 = 0xE1; // Negative Gamma Correction (E1h)

      static constexpr uint8_t RDINDEX = 0xD9; // ili9341
      static constexpr uint8_t IDXRD   = 0xDD; // ILI9341 only, indexed control register read
    };

    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          0xEF       , 3, 0x03,0x80,0x02,
          0xCF       , 3, 0x00,0xC1,0x30,
          0xED       , 4, 0x64,0x03,0x12,0x81,
          0xE8       , 3, 0x85,0x00,0x78,
          0xCB       , 5, 0x39,0x2C,0x00,0x34,0x02,
          0xF7       , 1, 0x20,
          0xEA       , 2, 0x00,0x00,
          CMD::PWCTR1, 1, 0x23,
          CMD::PWCTR2, 1, 0x10,
          CMD::VMCTR1, 2, 0x3e,0x28,
          CMD::VMCTR2, 1, 0x86,
  //      CMD::MADCTL, 1, 0xA8,
  //      CMD::PIXSET, 1, (_bpp > 16) ? PIX::RGB666_3BYTE : PIX::RGB565_2BYTE,
          CMD::FRMCTR1,2, 0x00,0x18,
          CMD::DFUNCTR,3, 0x08,0x82,0x27,
          0xF2       , 1, 0x00,

          CMD::GAMMASET,1, 0x01,  // Gamma set, curve 1
          CMD::GMCTRP1,15, 0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00,
          CMD::GMCTRN1,15, 0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F,

          CMD::SLPOUT, 0,
          0xFF,0xFF, // end
      };
      static constexpr uint8_t list1[] = {
          CMD::DISPON, 0,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      case 1: return list1;
      default: return nullptr;
      }
    }

  };

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack
  struct CfgM5Stack {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 19;
    static constexpr int spi_sclk = 18;
    static constexpr int spi_cs   = 14;
    static constexpr int spi_dc   = 27;
    static constexpr int panel_rst = 33;
    static constexpr int freq_write = 40000000;
    static constexpr int freq_read  = 16000000;
    static constexpr int freq_fill  = 80000000;
    static constexpr bool spi_half_duplex = true;
  };

  struct Panel_M5Stack : public Panel_ILI9341<CfgM5Stack>
  {
  protected:
    uint8_t getMadCtl(uint8_t r) const override {
      static constexpr uint8_t madctl_table[] = {
        this->MAD::MV|this->MAD::MY,
        0,
        this->MAD::MV|this->MAD::MX,
        this->MAD::MX|this->MAD::MY,
        this->MAD::MV|this->MAD::MX|this->MAD::MY,
        this->MAD::MY,
        this->MAD::MV,
        this->MAD::MX,
      };
      r = r & 7;
      return madctl_table[r];
    }
  };
#endif
}

#endif
