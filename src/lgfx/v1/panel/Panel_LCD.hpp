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

#include "Panel_Device.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Panel_LCD : public Panel_Device
  {
  public:
    bool init(bool use_reset) override;
    void beginTransaction(void) override;
    void endTransaction(void) override;

    color_depth_t setColorDepth(color_depth_t depth) override;
    void setRotation(std::uint_fast8_t r) override;
    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void waitDisplay(void) override {}
    bool displayBusy(void) override { return false; }

    void writePixels(pixelcopy_t* param, std::uint32_t len, bool use_dma) override;
    void writeBlock(std::uint32_t rawcolor, std::uint32_t len) override;

    void setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) override;
    void drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor) override;
    void writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor) override;
    void writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;

    std::uint32_t readCommand(std::uint_fast8_t cmd, std::uint_fast8_t index, std::uint_fast8_t len) override;
    std::uint32_t readData(std::uint_fast8_t index, std::uint_fast8_t len) override;
    void readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param) override;

  protected:

    std::uint16_t _colstart = 0;
    std::uint16_t _rowstart = 0;
    bool _in_transaction = false;
    std::uint8_t _cmd_nop = CMD_NOP;
    std::uint8_t _cmd_ramrd = CMD_RAMRD;

    enum mad_t
    { MAD_MY  = 0x80
    , MAD_MX  = 0x40
    , MAD_MV  = 0x20
    , MAD_ML  = 0x10
    , MAD_BGR = 0x08
    , MAD_MH  = 0x04
    , MAD_HF  = 0x02
    , MAD_VF  = 0x01
    , MAD_RGB = 0x00
    };

    enum colmod_t
    { RGB444_12bit = 0x33
    , RGB565_2BYTE = 0x55
    , RGB888_3BYTE = 0x66
    };

    virtual std::uint8_t getMadCtl(std::uint8_t r) const
    {
      static constexpr std::uint8_t madctl_table[] =
      {
                                         0,
        MAD_MV|MAD_MX|MAD_MH              ,
               MAD_MX|MAD_MH|MAD_MY|MAD_ML,
        MAD_MV|              MAD_MY|MAD_ML,
                             MAD_MY|MAD_ML,
        MAD_MV                            ,
               MAD_MX|MAD_MH              ,
        MAD_MV|MAD_MX|MAD_MY|MAD_MH|MAD_ML,
      };
      return madctl_table[r];
    }

    static constexpr std::uint8_t CMD_NOP     = 0x00;
    static constexpr std::uint8_t CMD_SWRESET = 0x01;
    static constexpr std::uint8_t CMD_RDDID   = 0x04;
    static constexpr std::uint8_t CMD_RDDST   = 0x09;
    static constexpr std::uint8_t CMD_SLPIN   = 0x10;
    static constexpr std::uint8_t CMD_SLPOUT  = 0x11;
    static constexpr std::uint8_t CMD_PTLON   = 0x12;
    static constexpr std::uint8_t CMD_NORON   = 0x13;
    static constexpr std::uint8_t CMD_INVOFF  = 0x20;
    static constexpr std::uint8_t CMD_INVON   = 0x21;
    static constexpr std::uint8_t CMD_GAMMASET= 0x26;
    static constexpr std::uint8_t CMD_DISPOFF = 0x28;
    static constexpr std::uint8_t CMD_DISPON  = 0x29;
    static constexpr std::uint8_t CMD_CASET   = 0x2A;
    static constexpr std::uint8_t CMD_RASET   = 0x2B;
    static constexpr std::uint8_t CMD_PASET   = 0x2B;
    static constexpr std::uint8_t CMD_RAMWR   = 0x2C;
    static constexpr std::uint8_t CMD_RAMRD   = 0x2E;
    static constexpr std::uint8_t CMD_MADCTL  = 0x36;
    static constexpr std::uint8_t CMD_IDMOFF  = 0x38;
    static constexpr std::uint8_t CMD_IDMON   = 0x39;
    static constexpr std::uint8_t CMD_COLMOD  = 0x3A;
    static constexpr std::uint8_t CMD_PIXSET  = 0x3A;

    void begin_transaction(void);
    void end_transaction(void);
    std::uint32_t read_bits(std::uint_fast8_t bit_index, std::uint_fast8_t bit_len);

    void write_command(std::uint32_t data);
    void write_bytes(const std::uint8_t* data, std::uint32_t len, bool use_dma);
    void set_window_8(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye, std::uint32_t cmd);
    void set_window_16(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye, std::uint32_t cmd);

    virtual void update_madctl(void);

    virtual std::uint8_t getColMod(std::uint8_t bpp) const { return (bpp > 16) ? RGB888_3BYTE : RGB565_2BYTE; }

    /// 引数に応じて _write_depth と _read_depth を設定する。_read_depthはドライバによってrgb888_3Byte固定のものや_write_depthと同じなるものがある。違いに注意が必要。
    virtual void setColorDepth_impl(color_depth_t depth) { _write_depth = ((int)depth & color_depth_t::bit_mask) > 16 ? rgb888_3Byte : rgb565_2Byte; _read_depth = rgb888_3Byte; }
//  virtual void setColorDepth_impl(color_depth_t depth) { _write_depth = ((int)depth & color_depth_t::bit_mask) > 16 ? rgb888_3Byte : rgb565_2Byte; _read_depth = _write_depth; }
  };

//----------------------------------------------------------------------------
 }
}
