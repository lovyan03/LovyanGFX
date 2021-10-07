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

#include "../Bus.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_ILI948x : public Panel_LCD
  {
    Panel_ILI948x(void)
    {
      _cfg.memory_width  = _cfg.panel_width  = 320;
      _cfg.memory_height = _cfg.panel_height = 480;
    }

    void setColorDepth_impl(color_depth_t depth) override 
    { // ILI9481,ILI9486 は SPIバス接続時は16bppが使用できない (ILI9486 spec Page 125 of 312);
      _write_depth = (((int)depth & color_depth_t::bit_mask) > 16
                    || (_bus && _bus->busType() == bus_spi))
                    ? rgb888_3Byte
                    : rgb565_2Byte;

      _read_depth = _write_depth;
    }
  };

//----------------------------------------------------------------------------

  struct Panel_ILI9481 : public Panel_ILI948x
  {
  protected:

    static constexpr uint8_t CMD_PNLDRV = 0xC0;
    static constexpr uint8_t CMD_FRMCTR = 0xC5;
    static constexpr uint8_t CMD_IFCTR  = 0xC6;
    static constexpr uint8_t CMD_PWSET  = 0xD0;
    static constexpr uint8_t CMD_VMCTR  = 0xD1;
    static constexpr uint8_t CMD_PWSETN = 0xD2;
    static constexpr uint8_t CMD_GMCTR  = 0xC8;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
          CMD_SLPOUT , 0+CMD_INIT_DELAY, 120,    // Exit sleep mode
          CMD_PWSET  , 3, 0x07, 0x42, 0x18,
          CMD_VMCTR  , 3, 0x00, 0x07, 0x10,
          CMD_PWSETN , 2, 0x01, 0x02,
          CMD_PNLDRV , 5, 0x10, 0x3B, 0x00, 0x02, 0x11,
          CMD_FRMCTR , 1, 0x03,
          CMD_GMCTR  ,12, 0x00, 0x44, 0x06, 0x44, 0x0A, 0x08,
                          0x17, 0x33, 0x77, 0x44, 0x08, 0x0C,
          CMD_SLPOUT , 0+CMD_INIT_DELAY, 120,    // Exit sleep mode
          CMD_IDMOFF , 0,
          CMD_DISPON , 0+CMD_INIT_DELAY, 100,
          0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }

    uint8_t getMadCtl(uint8_t r) const override
    {
      static constexpr uint8_t madctl_table[] = 
      {
               MAD_HF       ,
        MAD_MV              ,
                      MAD_VF,
        MAD_MV|MAD_HF|MAD_VF,
               MAD_HF|MAD_VF,
        MAD_MV|MAD_HF       ,
                           0,
        MAD_MV|       MAD_VF,
      };
      return madctl_table[r];
    }
  };

//----------------------------------------------------------------------------

  struct Panel_ILI9486 : public Panel_ILI948x
  {
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
    static constexpr uint8_t CMD_VMCTR1  = 0xC5;
    static constexpr uint8_t CMD_VMCTR2  = 0xC7;
    static constexpr uint8_t CMD_GMCTRP1 = 0xE0; // Positive Gamma Correction
    static constexpr uint8_t CMD_GMCTRN1 = 0xE1; // Negative Gamma Correction
    static constexpr uint8_t CMD_ADJCTL3 = 0xF7;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
          CMD_PWCTR1,  2, 0x17,  // VRH1
                          0x15,  // VRH2
          CMD_PWCTR2,  1, 0x41,  // VGH, VGL
          CMD_VMCTR1,  3, 0x00,  // nVM
                          0x12,  // VCM_REG
                          0x80,  // VCM_REG_EN
          CMD_FRMCTR1, 1, 0xA0,  // Frame rate = 60Hz
          CMD_INVCTR,  1, 0x02,  // Display Inversion Control = 2dot
          CMD_DFUNCTR, 3, 0x02,  // Nomal scan
                          0x22,  // 5 frames
                          0x3B,
          CMD_ETMOD,   1, 0xC6,  // 
          CMD_ADJCTL3, 4, 0xA9,  // Adjust Control 3 
                          0x51,
                          0x2C,
                          0x82,

          CMD_GMCTRP1,15, 0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98,
                          0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00,

          CMD_GMCTRN1,15, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
                          0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,

          CMD_SLPOUT , 0+CMD_INIT_DELAY, 120,    // Exit sleep mode
          CMD_IDMOFF , 0,
          CMD_DISPON , 0+CMD_INIT_DELAY, 100,
          0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------

  struct Panel_ILI9488 : public Panel_ILI948x
  {
    void setColorDepth_impl(color_depth_t depth) override 
    {
      _write_depth = (((int)depth & color_depth_t::bit_mask) > 16
                    || (_bus && _bus->busType() == bus_spi))
                    ? rgb888_3Byte
                    : rgb565_2Byte;

      _read_depth = rgb888_3Byte;
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
    static constexpr uint8_t CMD_ADJCTL3 = 0xF7;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
          CMD_PWCTR1,  2, 0x17,  // VRH1
                          0x15,  // VRH2
          CMD_PWCTR2,  1, 0x41,  // VGH, VGL
          CMD_VMCTR ,  3, 0x00,  // nVM
                          0x12,  // VCM_REG
                          0x80,  // VCM_REG_EN
          CMD_FRMCTR1, 1, 0xA0,  // Frame rate = 60Hz
          CMD_INVCTR,  1, 0x02,  // Display Inversion Control = 2dot
          CMD_DFUNCTR, 3, 0x02,  // Nomal scan
                          0x22,  // 5 frames
                          0x3B,
          CMD_ETMOD,   1, 0xC6,  // 
          CMD_ADJCTL3, 4, 0xA9,  // Adjust Control 3 
                          0x51,
                          0x2C,
                          0x82,
          CMD_SLPOUT , 0+CMD_INIT_DELAY, 120,    // Exit sleep mode
          CMD_IDMOFF , 0,
          CMD_DISPON , 0+CMD_INIT_DELAY, 100,
          0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------

  struct Panel_HX8357B : public Panel_ILI948x
  {
  protected:

    static constexpr uint8_t CMD_SETEXTC           = 0xB0;
    static constexpr uint8_t CMD_SETDISPMODE       = 0xB4;
    static constexpr uint8_t CMD_SET_PANEL_DRIVING = 0xC0;
    static constexpr uint8_t CMD_SETDISPLAYFRAME   = 0xC5;
    static constexpr uint8_t CMD_SETGAMMA          = 0xC8;
    static constexpr uint8_t CMD_SETPOWER          = 0xD0;
    static constexpr uint8_t CMD_SETVCOM           = 0xD1;
    static constexpr uint8_t CMD_SETPWRNORMAL      = 0xD2;
    static constexpr uint8_t CMD_SETPANELRELATED   = 0xE9;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
          CMD_SETEXTC,          1, 0x00,
          CMD_SETPOWER,         3, 0x44, 0x41, 0x06,
          CMD_SETVCOM,          2, 0x40, 0x10,
          CMD_SETPWRNORMAL,     2, 0x05, 0x12,
          CMD_SET_PANEL_DRIVING,5, 0x14, 0x3b, 0x00, 0x02, 0x11,
          CMD_SETDISPLAYFRAME,  1, 0x0c, // 6.8mhz
          CMD_SETPANELRELATED,  1, 0x09, // BGR and shift direction reverse.
          0xEA,                 3, 0x03, 0x00, 0x00,
          0xEB,                 4, 0x40, 0x54, 0x26, 0xdb,
          CMD_SETGAMMA,        12, 0x00, 0x15, 0x00, 0x22, 0x00, 0x08, 0x77, 0x26, 0x66, 0x22, 0x04, 0x00,
          CMD_SETDISPMODE,      1, 0x00, // CPU (DBI) and internal oscillation ??
          CMD_SLPOUT , 0+CMD_INIT_DELAY, 120,    // Exit sleep mode
          CMD_IDMOFF , 0,
          CMD_DISPON , 0+CMD_INIT_DELAY, 100,
          0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------

  struct Panel_HX8357D : public Panel_ILI948x
  {
  protected:

    static constexpr uint8_t CMD_TEON    = 0x35;
    static constexpr uint8_t CMD_TEARLINE= 0x44;
    static constexpr uint8_t CMD_SETOSC  = 0xB0;
    static constexpr uint8_t CMD_SETPWR1 = 0xB1;
    static constexpr uint8_t CMD_SETRGB  = 0xB3;
    static constexpr uint8_t CMD_SETCYC  = 0xB4;
    static constexpr uint8_t CMD_SETCOM  = 0xB6;
    static constexpr uint8_t CMD_SETC    = 0xB9;
    static constexpr uint8_t CMD_SETSTBA = 0xC0;
    static constexpr uint8_t CMD_SETPANEL= 0xCC;
    static constexpr uint8_t CMD_SETGAMMA= 0xE0;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] =
      {
          CMD_SWRESET , CMD_INIT_DELAY, 20, 
          CMD_SETC    , 3+CMD_INIT_DELAY, 0xFF, 0x83, 0x57, 100, 
          CMD_SETRGB  , 4, 0x80, 0x00, 0x06, 0x06,
          CMD_SETCOM  , 1, 0x25,
          CMD_SETOSC  , 1, 0x68,
          CMD_SETPANEL, 1, 0x08,
          CMD_SETPWR1 , 6, 0x00, 0x15, 0x1C, 0x1C, 0x83, 0xAA,
          CMD_SETSTBA , 6, 0x50, 0x50, 0x01, 0x3C, 0x1E, 0x08,
          CMD_SETCYC  , 7, 0x02, 0x40, 0x00, 0x2A, 0x2A, 0x0D, 0x78,

          CMD_SETGAMMA,34,0x02, 0x0A, 0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b,
                          0x4b, 0x42, 0x3A, 0x27, 0x1B, 0x08, 0x09, 0x03,
                          0x02, 0x0A, 0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b,
                          0x4b, 0x42, 0x3A, 0x27, 0x1B, 0x08, 0x09, 0x03,
                          0x00, 0x01,

          CMD_TEON    , 1, 0x00,
          CMD_TEARLINE, 2, 0x00, 0x02,

          CMD_SLPOUT  , 0+CMD_INIT_DELAY, 120,    // Exit sleep mode
          CMD_IDMOFF  , 0,
          CMD_DISPON  , 0+CMD_INIT_DELAY, 100,
          0xFF,0xFF, // end
      };
      switch (listno)
      {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------
 }
}
