#ifndef LGFX_PANEL_ILITEK_COMMON_HPP_
#define LGFX_PANEL_ILITEK_COMMON_HPP_

#include "panel_device.hpp"

namespace lgfx
{
  template <class CFG>
  class PanelIlitekCommon : public LovyanGFXDevice<CFG>
  {
  public:
    PanelIlitekCommon() : LovyanGFXDevice<CFG>() 
    , _panel_x     (get_panel_x     <CFG, 0>::value)
    , _panel_y     (get_panel_y     <CFG, 0>::value)
    , _ram_width   (get_ram_width   <CFG, 0>::value)
    , _ram_height  (get_ram_height  <CFG, 0>::value)
    , _panel_width (get_panel_width <CFG, 0>::value)
    , _panel_height(get_panel_height<CFG, 0>::value)
    {
      this->_cmd_caset = CommandCommon::CASET;
      this->_cmd_raset = CommandCommon::RASET;
      this->_cmd_ramwr = CommandCommon::RAMWR;
      this->_cmd_ramrd = CommandCommon::RAMRD;
      this->_len_command = 8;
      this->_len_read_pixel = 24;
      this->_len_dummy_read_pixel = 8;
      this->_len_dummy_read_rddid = 0;
    }
    void setRotation(uint8_t r) override
    {
      this->startWrite();
      this->write_cmd(this->_cmd_madctl);
      r = r & 7;
      this->_rotation = r;
//Serial.printf("rotation:%d ", _rotation);

      switch (r & 3) {
      default:
        this->_colstart = _panel_x;
        this->_rowstart = _panel_y;
        break;
      case 1:
        this->_colstart = _panel_y;
        this->_rowstart = _ram_width - (_panel_width + _panel_x);
        break;
      case 2:
        this->_colstart = _ram_width  - (_panel_width  + _panel_x);
        this->_rowstart = _ram_height - (_panel_height + _panel_y);
        break;
      case 3:
        this->_colstart = _ram_height - (_panel_height + _panel_y);
        this->_rowstart = _panel_x;
        break;
      }
      if (r & 1) {
        this->_width  = _panel_height;
        this->_height = _panel_width;
      } else {
        this->_width  = _panel_width;
        this->_height = _panel_height;
      }
      uint8_t madctl = getMadCtl(r) | (this->_rgb_order ? _madctl_rgb : _madctl_bgr);
      this->write_data(madctl, 8);
      this->_last_xs = this->_last_xe = this->_last_ys = this->_last_ye = 0xFFFF;
      this->endWrite();
    }

    void* setColorDepth(uint8_t bpp) override  // 16 or 24
    {
      this->startWrite();
      this->write_cmd(_cmd_colmod);
      this->_bpp = getAdjustBpp(bpp);
      this->write_data(getColMod(this->_bpp), 8);
      this->endWrite();
      return nullptr;
    }

    void invertDisplay(bool i) override
    { // Send the command twice as otherwise it does not always work!
      this->startWrite();
      this->_invert = i;
      this->write_cmd(i ? _cmd_invon : _cmd_invoff);
      this->write_cmd(i ? _cmd_invon : _cmd_invoff);
      this->endWrite();
    }

    uint32_t readPanelID(void) override
    {
      //write_cmd(0xD9);
      //write_data(0x10, 8);
      this->write_cmd(_cmd_rddid);
      this->start_read();
      if (this->_len_dummy_read_rddid) this->read_data(this->_len_dummy_read_rddid);
      uint32_t res = this->read_data(32);
      this->end_read();
      return res;
    }

    uint32_t readPanelIDSub(uint8_t cmd) override
    {
      //write_cmd(0xD9);
      //write_data(0x10, 8);
      this->write_cmd(cmd);
      this->start_read();
      uint32_t res = this->read_data(32);
      this->end_read();
      return res;
    }

  protected:
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

    uint32_t _cmd_rddid  = CommandCommon::RDDID;
    uint32_t _cmd_invoff = CommandCommon::INVOFF;
    uint32_t _cmd_invon  = CommandCommon::INVON;
    uint32_t _cmd_madctl = CommandCommon::MADCTL;
    uint32_t _cmd_colmod = CommandCommon::COLMOD;

    uint16_t _panel_x = 0;
    uint16_t _panel_y = 0;
    uint16_t _ram_width  = 0;
    uint16_t _ram_height = 0;
    uint16_t _panel_width  = 0;
    uint16_t _panel_height = 0;

    uint8_t _madctl_rgb = 0x00;
    uint8_t _madctl_bgr = 0x08;

    virtual uint8_t getMadCtl(uint8_t r) const { return 0; }
    virtual uint8_t getColMod(uint8_t bpp) const { return 0; }
    virtual uint8_t getAdjustBpp(uint8_t bpp) const { return (bpp > 16) ? 24 : 16; }


    //inline static uint16_t getColor16(uint8_t r, uint8_t g, uint8_t b) { return color565(r,g,b); }
    //inline static uint32_t getColor24(uint8_t r, uint8_t g, uint8_t b) { return color888(r,g,b); }
    //inline static uint16_t getColor16FromRead(uint32_t raw) { return getColor565FromSwap888(raw); }
    //inline static uint32_t getColor24FromRead(uint32_t raw) { return getSwap24(raw); }
    //inline static uint16_t getWriteColor16FromRead(uint32_t raw) { return getSwapColor565FromSwap888(raw); }
    //inline static uint32_t getWriteColor24FromRead(uint32_t raw) { return raw & 0xFFFFFF; }
    //inline static uint16_t getWriteColor16(uint16_t color) { return getSwap16(color); }
    //inline static uint32_t getWriteColor24(uint32_t color) { return getSwap24(color) & 0xFFFFFF; }
  };
}

#endif
