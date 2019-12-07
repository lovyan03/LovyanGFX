#ifndef LGFX_PANEL_ST7789_HPP_
#define LGFX_PANEL_ST7789_HPP_

#include "panel_lcd_common.hpp"

namespace lgfx
{
  struct Panel_ST7789_COMMON : public PanelLcdCommon
  {
    static constexpr uint8_t SPI_MODE = 0;

    static constexpr uint8_t LEN_CMD = 8;
    static constexpr uint8_t LEN_PIXEL_READ = 24;
    static constexpr uint8_t LEN_DUMMY_READ_PIXEL = 16;
    static constexpr uint8_t LEN_DUMMY_READ_RDDID = 1;

    static constexpr int16_t RAM_WIDTH  = 240;
    static constexpr int16_t RAM_HEIGHT = 320;

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
    { RGB565_2BYTE = 0x55
    , RGB666_3BYTE = 0x66
    };

    static const uint8_t* getInitCommands(uint8_t listno = 0) {
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

    inline static uint8_t getAdjustBpp(uint8_t bpp) { return (bpp > 17 ) ? 24 : 16; }
    inline static PIX getPixset(uint8_t bpp) { return (bpp > 16) ? RGB666_3BYTE : RGB565_2BYTE; }
  };

  struct Panel_ST7789_240x240 : public Panel_ST7789_COMMON
  {
    static constexpr int16_t PANEL_WIDTH  = 240;
    static constexpr int16_t PANEL_HEIGHT = 240;
    static constexpr int16_t OFFSET_X = 0;
    static constexpr int16_t OFFSET_Y = 0;
    static constexpr bool HAS_OFFSET = true;

    static rotation_data_t* getRotationData(uint8_t r) {
      static rotation_data_t res {0,0,0};
      r = r & 3;
      //res.colstart = (r == 3) ? 80 : 0;
      //res.rowstart = (r == 2) ? 80 : 0;
      switch (r) {
      default:
        res.colstart = OFFSET_X;
        res.rowstart = OFFSET_Y;
        break;
      case 1:
        res.colstart = OFFSET_Y;
        res.rowstart = RAM_WIDTH - (PANEL_WIDTH + OFFSET_X);
        break;
      case 2:
        res.colstart = RAM_WIDTH - (PANEL_WIDTH + OFFSET_X);
        res.rowstart = RAM_HEIGHT - (PANEL_HEIGHT + OFFSET_Y);
        break;
      case 3:
        res.colstart = RAM_HEIGHT - (PANEL_HEIGHT + OFFSET_Y);
        res.rowstart = OFFSET_X;
        break;
      }
      static constexpr uint8_t madctl_table[] = {
                                        MAD::BGR,
        MAD::MX|MAD::MV|                MAD::BGR,
        MAD::MX|        MAD::MY|MAD::MH|MAD::BGR,
                MAD::MV|MAD::MY|        MAD::BGR,
      };
      res.madctl = madctl_table[r];
      return &res;
    }
  };

  struct Panel_ST7789_240x320 : public Panel_ST7789_COMMON
  {
    static constexpr int16_t PANEL_WIDTH  = 240;
    static constexpr int16_t PANEL_HEIGHT = 320;
    static constexpr bool HAS_OFFSET = true;

    static rotation_data_t* getRotationData(uint8_t r) {
      static rotation_data_t res {0,0,0};
      r = r & 3;
      static constexpr uint8_t madctl_table[] = {
                                        MAD::BGR,
        MAD::MX|MAD::MV|                MAD::BGR,
        MAD::MX|        MAD::MY|MAD::MH|MAD::BGR,
                MAD::MV|MAD::MY|        MAD::BGR,
      };
      res.madctl = madctl_table[r];
      return &res;
    }
  };
}

#endif
