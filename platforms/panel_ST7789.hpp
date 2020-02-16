#ifndef LGFX_PANEL_ST7789_HPP_
#define LGFX_PANEL_ST7789_HPP_

#include "panel_ilitek_common.hpp"

namespace lgfx
{
  class Panel_ST7789_Common  : public PanelIlitekCommon
  {
  protected:

    void setConfig_impl(void) override
    {
      if (!_ram_width   ) _ram_width = 240;
      if (!_ram_height  ) _ram_height = 320;
      if (!_panel_width ) _panel_width = 240;
      if (!_panel_height) _panel_height = 320;
      if (!freq_write) freq_write = 80000000;
      if (!freq_read ) freq_read  = 20000000;
      if (!freq_fill ) freq_fill  = 80000000;

      len_dummy_read_pixel = 16;
    }

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
                                      0,
        MAD::MX|MAD::MV                ,
        MAD::MX|        MAD::MY|MAD::MH,
                MAD::MV|MAD::MY        ,
      };
      r = r & 3;
      return madctl_table[r];
    }


    struct CMD : public CommandCommon
    {
      static constexpr uint8_t PORCTRL  = 0xB2;      // Porch control
      static constexpr uint8_t GCTRL    = 0xB7;      // Gate control
      static constexpr uint8_t VCOMS    = 0xBB;      // VCOMS setting
      static constexpr uint8_t LCMCTRL  = 0xC0;      // LCM control
      static constexpr uint8_t VDVVRHEN = 0xC2;      // VDV and VRH command enable
      static constexpr uint8_t VRHS     = 0xC3;      // VRH set
      static constexpr uint8_t VDVSET   = 0xC4;      // VDV setting
      static constexpr uint8_t FRCTR2   = 0xC6;      // FR Control 2
      static constexpr uint8_t PWCTRL1  = 0xD0;      // Power control 1
      static constexpr uint8_t PVGAMCTRL= 0xE0;      // Positive voltage gamma control
      static constexpr uint8_t NVGAMCTRL= 0xE1;      // Negative voltage gamma control
    };
    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CMD::SLPOUT , CMD_INIT_DELAY, 120,
          CMD::NORON  , CMD_INIT_DELAY, 0,
          0xB6        , 2, 0x0A,0x82,
          CMD::PORCTRL, 5, 0x0c, 0x0c, 0x00, 0x33, 0x33,
          CMD::GCTRL  , 1, 0x35,

          CMD::VCOMS  , 1, 0x28,  // JLX240 display datasheet
          CMD::LCMCTRL, 1, 0x0C,
          CMD::VDVVRHEN,2, 0x01, 0xFF,
          CMD::VRHS   , 1, 0x10,      // voltage VRHS
          CMD::VDVSET , 1, 0x20,
          CMD::FRCTR2 , 1, 0x0f,
          CMD::PWCTRL1, 2, 0xa4, 0xa1,
          //--------------------------------ST7789V gamma setting---------------------------------------//
          CMD::PVGAMCTRL,14, 0xd0,0x00,0x02,0x07,
                             0x0a,0x28,0x32,0x44,
                             0x42,0x06,0x0e,0x12,
                             0x14,0x17,
          CMD::NVGAMCTRL,14, 0xd0,0x00,0x02,0x07,
                             0x0a,0x28,0x31,0x54,
                             0x47,0x0e,0x1c,0x17,
                             0x1b,0x1e,
          CMD::SLPOUT, 1, 255,
          CMD::DISPON, 1, 255,
          0xFF,0xFF, // end
      };
      static constexpr uint8_t list1[] = {
          CMD::DISPON, 1, 255,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      case 1: return list1;
      default: return nullptr;
      }
    }
  };


  template <typename CFG>
  class Panel_ST7789 : public Panel_ST7789_Common
  {
  public:
    Panel_ST7789() : Panel_ST7789_Common() { setConfig<CFG>(); }
  };


  class Panel_TTGO_TWatch : public Panel_ST7789_Common
  {
  public:
    Panel_TTGO_TWatch() : Panel_ST7789_Common() {
      setConfig<cfg_t>(); 
    }

  private:
    struct cfg_t {
      static constexpr int spi_mode_read = 1;
      static constexpr int spi_cs =  5;
      static constexpr int spi_dc = 27;
      static constexpr uint32_t freq_write = 80000000;
      static constexpr uint32_t freq_read  = 20000000;
      static constexpr uint32_t freq_fill  = 80000000;
      static constexpr bool spi_3wire   = true;
      static constexpr bool invert      = true;
      static constexpr int panel_width  = 240;
      static constexpr int panel_height = 240;
    };
  };
}

#endif
