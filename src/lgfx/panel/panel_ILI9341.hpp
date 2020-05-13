#ifndef LGFX_PANEL_ILI9341_HPP_
#define LGFX_PANEL_ILI9341_HPP_

#include "panel_ilitek_common.hpp"

namespace lgfx
{
  struct Panel_ILI9341 : public PanelIlitekCommon
  {
    Panel_ILI9341(void)
    {
      panel_width  = memory_width  = 240;
      panel_height = memory_height = 320;

      freq_write = 40000000;
      freq_read  = 16000000;
      freq_fill  = 40000000;
    }

  protected:

//    enum colmod_t
//    { RGB565_2BYTE = 0x55
//    , RGB666_3BYTE = 0x66
//    };
//    uint8_t getColMod(uint8_t bpp) const override { return (bpp > 16) ? RGB666_3BYTE : RGB565_2BYTE; }

    struct CMD : public PanelIlitekCommon::CommandCommon
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
          CMD::FRMCTR1,2, 0x00,0x13,
          0xF2       , 1, 0x00,

          CMD::GAMMASET,1, 0x01,  // Gamma set, curve 1
          CMD::GMCTRP1,15, 0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00,
          CMD::GMCTRN1,15, 0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F,
          0xFF,0xFF, // end
      };
      static constexpr uint8_t list1[] = {
          CMD::DFUNCTR,3, 0x08,0xC2,0x27,
          0xFF,0xFF, // end
      };
      static constexpr uint8_t list2[] = {
          CMD::SLPOUT, 0,
          CMD::DISPON, 0,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      case 1: return list1;
      case 2: return list2;
      default: return nullptr;
      }
    }
  };

  struct Panel_ILI9342 : public Panel_ILI9341
  {
    Panel_ILI9342(void) {
      panel_width  = memory_width  = 320;
      panel_height = memory_height = 240;
    }
  protected:
    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list1[] = {
          CMD::DFUNCTR,4, 0x08,0x82,0x1D,0x04,
          0xFF,0xFF, // end
      };
      if (listno == 1)  return list1;
      return Panel_ILI9341::getInitCommands(listno);
    }
  };

#if defined(ESP32) || (CONFIG_IDF_TARGET_ESP32)
  struct Panel_ODROID_GO : public Panel_ILI9341
  {
    Panel_ODROID_GO(void) {
      freq_fill  = 80000000;
      spi_3wire = true;
      spi_cs =  5;
      spi_dc = 21;
      rotation = 1;
      gpio_bl = 14;
      pwm_ch_bl = 7;
    }
  };

  struct Panel_M5Stack : public Panel_ILI9342
  {
    bool isIPS = false;

    Panel_M5Stack(void) {
      spi_3wire = true;
      spi_cs = 14;
      spi_dc = 27;
      rotation = 1;
      offset_rotation = 3;
      gpio_rst = 33;
      gpio_bl  = 32;
      pwm_ch_bl = 7;
    }

    void init(void) override {
      gpio_lo(gpio_rst);
      lgfxPinMode(gpio_rst, pin_mode_t::input);

      isIPS = gpio_in(gpio_rst);       // get panel type (IPS or TN)

      Panel_ILI9342::init();
    }

  protected:

    const uint8_t* getInvertDisplayCommands(uint8_t* buf, bool invert) override {
      if (!isIPS) return Panel_ILI9342::getInvertDisplayCommands(buf, invert);
      this->invert = invert;
      buf[2] = buf[0] = invert ? CommandCommon::INVOFF : CommandCommon::INVON;
      buf[3] = buf[1] = 0;
      buf[4] = CMD::GAMMASET;
      buf[5] = 1;
    //buf[6] = 0x08;  // Gamma set, curve 8
    //buf[6] = 0x04;  // Gamma set, curve 4
      buf[6] = 0x02;  // Gamma set, curve 2
    //buf[6] = 0x01;  // Gamma set, curve 1
      buf[8] = buf[7] = 0xFF;
      return buf;
    }

  };
#endif
}

#endif
