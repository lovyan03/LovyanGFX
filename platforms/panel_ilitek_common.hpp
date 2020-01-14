#ifndef LGFX_PANEL_ILITEK_COMMON_HPP_
#define LGFX_PANEL_ILITEK_COMMON_HPP_

#include "panel_common.hpp"

namespace lgfx
{
  class PanelIlitekCommon : public PanelCommon
  {
  public:

    PanelIlitekCommon() : PanelCommon()
    {
      cmd_caset  = CommandCommon::CASET;
      cmd_raset  = CommandCommon::RASET;
      cmd_ramwr  = CommandCommon::RAMWR;
      cmd_ramrd  = CommandCommon::RAMRD;
      cmd_invon  = CommandCommon::INVON;
      cmd_invoff = CommandCommon::INVOFF;
      cmd_rddid  = CommandCommon::RDDID;
      read_depth = rgb888_3Byte;
      len_dummy_read_pixel = 8;
      len_dummy_read_rddid = 0;
      len_setwindow = 32;
    }

  protected:
    const uint8_t* getRotationCommands(uint8_t* buf, uint8_t r) override
    {
      PanelCommon::getRotationCommands(buf, r);
      buf[0] = CommandCommon::MADCTL;
      buf[1] = 1;
      buf[2] = getMadCtl(rotation) | (_rgb_order ? _madctl_rgb : _madctl_bgr);
      buf[3] = buf[4] = 0xFF;
      return buf;
    }

    const uint8_t* getColorDepthCommands(uint8_t* buf, color_depth_t depth) override
    {
      PanelCommon::getColorDepthCommands(buf, depth);
      buf[0] = CommandCommon::COLMOD;
      buf[1] = 1;
      buf[2] = getColMod(write_depth);
      buf[3] = buf[4] = 0xFF;
      return buf;
    }

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

    uint8_t _madctl_rgb = 0x00;
    uint8_t _madctl_bgr = 0x08;

    virtual uint8_t getMadCtl(uint8_t r) const = 0;
    virtual uint8_t getColMod(uint8_t bpp) const = 0;
  };
}

#endif
