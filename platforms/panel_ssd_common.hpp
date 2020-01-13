#ifndef LGFX_PANEL_SSD_COMMON_HPP_
#define LGFX_PANEL_SSD_COMMON_HPP_

#include "LovyanGFX.hpp"

namespace lgfx
{
  template <class CFG>
  class PanelSsdCommon : public LGFX_SPI<CFG>
  {
  public:
    PanelSsdCommon() : LGFX_SPI<CFG>()
    , _panel_x     (get_panel_x     <CFG, 0>::value)
    , _panel_y     (get_panel_y     <CFG, 0>::value)
    , _ram_width   (get_ram_width   <CFG, 128>::value)
    , _ram_height  (get_ram_height  <CFG, 128>::value)
    , _panel_width (get_panel_width <CFG, 128>::value)
    , _panel_height(get_panel_height<CFG, 128>::value)
    {
      this->_cmd_caset = CommandCommon::CASET;
      this->_cmd_raset = CommandCommon::RASET;
      this->_cmd_ramwr = CommandCommon::RAMWR;
      this->_cmd_ramrd = CommandCommon::RAMRD;
      this->_len_command = 8;
      this->_len_read_pixel = 24;
      this->_len_dummy_read_pixel = 8;
      this->_len_dummy_read_rddid = 0;
      this->_len_setwindow = 16;
      this->getWindowAddr = this->getWindowAddr16;
    }

    void setRotation_impl(uint8_t r) override
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

      this->write_data(getMadCtl(r) | getColMod(this->getColorDepth()), 8);

      if (r & 1) {
        this->_width  = _panel_height;
        this->_height = _panel_width;
        this->_cmd_caset = CommandCommon::RASET;
        this->_cmd_raset = CommandCommon::CASET;
      } else {
        this->_width  = _panel_width;
        this->_height = _panel_height;
        this->_cmd_caset = CommandCommon::CASET;
        this->_cmd_raset = CommandCommon::RASET;
      }

      this->write_cmd(CommandCommon::STARTLINE);
      this->write_data((r < 2) ? _ram_height : 0, 8);

      this->_last_xs = this->_last_xe = this->_last_ys = this->_last_ye = 0xFFFF;
      this->endWrite();
    }

    void* setColorDepth_impl(color_depth_t bpp) override  // 16 or 24
    {
      this->_color.setColorDepth(getAdjustBpp(bpp));
      setRotation_impl(this->_rotation);
      return nullptr;
    }

    void invertDisplay_impl(bool i) override
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
//  static constexpr uint8_t RDDID   = 0x04;
//  static constexpr uint8_t RDDST   = 0x09;
    static constexpr uint8_t SLPIN   = 0xAE;
    static constexpr uint8_t SLPOUT  = 0xAF;
//  static constexpr uint8_t PTLON   = 0x12;
//  static constexpr uint8_t NORON   = 0x13;
    static constexpr uint8_t INVOFF  = 0xA6;
    static constexpr uint8_t INVON   = 0xA7;
//  static constexpr uint8_t GAMMASET= 0x26;
    static constexpr uint8_t DISPOFF = 0xA4;
    static constexpr uint8_t DISPON  = 0xA5;
    static constexpr uint8_t CASET   = 0x15;
    static constexpr uint8_t RASET   = 0x75; static constexpr uint8_t PASET = 0x75;
    static constexpr uint8_t RAMWR   = 0x5C;
    static constexpr uint8_t RAMRD   = 0x5D;
    static constexpr uint8_t MADCTL  = 0xA0;
//  static constexpr uint8_t COLMOD  = 0x3A; static constexpr uint8_t PIXSET = 0x3A;

    static constexpr uint8_t CMDLOCK = 0xFD;
    static constexpr uint8_t STARTLINE = 0xA1;
    };

    uint32_t _cmd_rddid  = 0;
    uint32_t _cmd_invoff = CommandCommon::INVOFF;
    uint32_t _cmd_invon  = CommandCommon::INVON;
    uint32_t _cmd_madctl = CommandCommon::MADCTL;
    uint32_t _cmd_colmod = 0;

    uint16_t _panel_x = 0;
    uint16_t _panel_y = 0;
    uint16_t _ram_width  = 0;
    uint16_t _ram_height = 0;
    uint16_t _panel_width  = 0;
    uint16_t _panel_height = 0;

    //virtual uint8_t getMadCtl(uint8_t r) const { return 0; }
    //virtual uint8_t getColMod(uint8_t bpp) const { return 0; }
    //virtual uint8_t getAdjustBpp(uint8_t bpp) const { return 16; }
    virtual color_depth_t getAdjustBpp(color_depth_t bpp) const { return (bpp > 16) ? rgb666_3Byte : rgb565_2Byte; }


    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list0[] = {
          CommandCommon::CMDLOCK, 1, 0x12,
          CommandCommon::CMDLOCK, 1, 0xB1,
          CommandCommon::SLPIN  , 0,
          //CommandCommon::DISPOFF, 0,
          0xB3                  , 1, 0xF1,  // CLOCKDIV
          0xCA                  , 1, 0x7F,  // MUXRATIO
          0xA2                  , 1, 0x00,  // DISPLAYOFFSET
          0xB5                  , 1, 0x00,  // SETGPIO
          0xAB                  , 1, 0x01,  // FUNCTIONSELECT
          0xB1                  , 1, 0x32,  // PRECHARGE

          0xBE                  , 1, 0x05,  // VCOMH
          CommandCommon::INVOFF , 0,
          0xC1                  , 3, 0xC8, 0x80, 0xC8, // CONTRASTABC
          0xC7                  , 1, 0x0F,  // CONTRASTMASTER
          0xB4                  , 3, 0xA0, 0xB5, 0x55, // SETVSL
          0xB6                  , 1, 0x01,  // PRECHARGE2
          CommandCommon::SLPOUT , 0,
          CommandCommon::DISPON , 0,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }

    virtual uint8_t getMadCtl(uint8_t r) const {
      static constexpr uint8_t madctl_table[] = {
        0b00110100,
        0b00110111,
        0b00100110,
        0b00100101,
      };
      r = r & 3;
      return madctl_table[r];
    }
    virtual uint8_t getColMod(uint8_t bpp) const {
      if (bpp == 16) return 0x40;
      return 0x80;
    }
  };
}

#endif
