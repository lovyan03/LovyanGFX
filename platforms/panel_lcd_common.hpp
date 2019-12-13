#ifndef LGFX_PANEL_LCD_COMMON_HPP_
#define LGFX_PANEL_LCD_COMMON_HPP_

#include "lgfx_common.hpp"

namespace lgfx
{
  struct PanelLcdCommon
  {
    struct rotation_data_t {
      uint8_t madctl;
      uint8_t colstart;
      uint8_t rowstart;
    };
    static constexpr uint8_t CMD_INIT_DELAY = 0x80;

    struct CommandCommon {
    static constexpr uint8_t NOP     = 0x00;
    static constexpr uint8_t SWRESET = 0x01;
    static constexpr uint8_t RDDID   = 0x04;
    static constexpr uint8_t RDDST   = 0x09;
    static constexpr uint8_t SLPIN   = 0x10;
    static constexpr uint8_t SLPOUT  = 0x11;
    static constexpr uint8_t PTLON   = 0x12;
    static constexpr uint8_t NORON   = 0x13;
    static constexpr uint8_t INVOFF  = 0x20;
    static constexpr uint8_t INVON   = 0x21;
    static constexpr uint8_t GAMMASET= 0x26;
    static constexpr uint8_t DISPOFF = 0x28;
    static constexpr uint8_t DISPON  = 0x29;
    static constexpr uint8_t CASET   = 0x2A;
    static constexpr uint8_t RASET   = 0x2B; static constexpr uint8_t PASET = 0x2B;
    static constexpr uint8_t RAMWR   = 0x2C;
    static constexpr uint8_t RAMRD   = 0x2E;
    static constexpr uint8_t MADCTL  = 0x36;
    static constexpr uint8_t COLMOD  = 0x3A; static constexpr uint8_t PIXSET = 0x3A;
    };
    uint32_t cmd_rddid  = CommandCommon::RDDID;
    uint32_t cmd_invoff = CommandCommon::INVOFF;
    uint32_t cmd_invon  = CommandCommon::INVON;
    uint32_t cmd_caset  = CommandCommon::CASET;
    uint32_t cmd_raset  = CommandCommon::RASET;
    uint32_t cmd_ramwr  = CommandCommon::RAMWR;
    uint32_t cmd_ramrd  = CommandCommon::RAMRD;
    uint32_t cmd_madctl = CommandCommon::MADCTL;
    uint32_t cmd_colmod = CommandCommon::COLMOD;

    uint16_t panel_width  = 240;
    uint16_t panel_height = 320;
    uint16_t ram_width  = 240;
    uint16_t ram_height = 320;
    uint16_t offset_x = 0;
    uint16_t offset_y = 0;
    uint8_t spi_mode  = 0;
    uint8_t len_command = 8;
    uint8_t len_read_pixel = 24;
    uint8_t len_dummy_read_pixel = 8;
    uint8_t len_dummy_read_rddid = 0;

    virtual uint8_t getColMod(uint8_t bpp) const { return 0; }
    virtual uint8_t getAdjustBpp(uint8_t bpp) const { return (bpp > 16 ) ? 24 : 16; }
    virtual const uint8_t* getInitCommands(uint8_t listno = 0) const { return nullptr; }
    virtual const rotation_data_t* getRotationData(uint8_t r) const
    {
      static rotation_data_t res {0,0,0};
      switch (r & 3) {
      default:
        res.colstart = offset_x;
        res.rowstart = offset_y;
        break;
      case 1:
        res.colstart = offset_y;
        res.rowstart = ram_width - (panel_width + offset_x);
        break;
      case 2:
        res.colstart = ram_width - (panel_width + offset_x);
        res.rowstart = ram_height - (panel_height + offset_y);
        break;
      case 3:
        res.colstart = ram_height - (panel_height + offset_y);
        res.rowstart = offset_x;
        break;
      }
      return &res;
    }


    inline static uint16_t getColor16(uint8_t r, uint8_t g, uint8_t b) { return getColor565(r,g,b); }
    inline static uint32_t getColor24(uint8_t r, uint8_t g, uint8_t b) { return getColor888(r,g,b); }

    inline static uint16_t getColor16FromRead(uint32_t raw) { return getColor565FromSwap888(raw); }
    inline static uint32_t getColor24FromRead(uint32_t raw) { return getSwap24(raw); }

    inline static uint16_t getWriteColor16FromRead(uint32_t raw) { return getSwapColor565FromSwap888(raw); }
    inline static uint32_t getWriteColor24FromRead(uint32_t raw) { return raw & 0xFFFFFF; }

    inline static uint16_t getWriteColor16(uint16_t color) { return getSwap16(color); }
    inline static uint32_t getWriteColor24(uint32_t color) { return getSwap24(color) & 0xFFFFFF; }

    inline static uint32_t getWindowAddr(uint16_t H, uint16_t L) { return ((H)<<8 | (H)>>8) | (((L)<<8 | (L)>>8)<<16 ); }
  };
}

#endif
