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
#include "Panel_NT35510.hpp"

#include "../Bus.hpp"
#include "../misc/colortype.hpp"
#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
// ToDo : 初期化コマンド以外は RM68120と共通化する;

  void Panel_NT35510::writeRegister(uint16_t cmd, uint8_t data)
  {
    writeCommand(getSwap16(cmd), 2);
    writeData(data, 1);
  }

  void Panel_NT35510::writeCommandList(const uint8_t *addr)
  {
    for (;;)
    {                // For each command...
      uint16_t cmd = *addr++;
      uint8_t num = *addr++;   // Number of args to follow
      if (cmd == 0xFF && num == 0xFF) break;
      cmd <<= 8;
      uint_fast8_t ms = num & CMD_INIT_DELAY;       // If hibit set, delay follows args
      num &= ~CMD_INIT_DELAY;          // Mask out delay bit
      if (num)
      {
        do
        {
          writeCommand(getSwap16(cmd++), 2);
          writeData(*addr++, 1);
          _bus->flush();
        } while (--num);
      }
      else
      {
        writeCommand(getSwap16(cmd), 2);
        _bus->flush();
      }
      if (ms)
      {
        ms = *addr++;        // Read post-command delay time (ms)
        delay( (ms==255 ? 500 : ms) );
      }
    }
  }

  const uint8_t* Panel_NT35510::getInitCommands(uint8_t listno) const
  {
    if (listno != 0) { return nullptr; }
    static constexpr const uint8_t list0[] =
    {
      0xF0,  5, 0x55, 0xAA, 0x52, 0x08, 0x01,

      0xB6,  3, 0x34, 0x34, 0x34,
      0xB0,  3, 0x0D, 0x0D, 0x0D,   // AVDD Set AVDD 5.2V
      0xB7,  3, 0x34, 0x34, 0x34,   // AVEE ratio
      0xB1,  3, 0x0D, 0x0D, 0x0D,   // AVEE  -5.2V
      0xB8,  3, 0x24, 0x24, 0x24,   // VCL ratio
      0xB9,  3, 0x34, 0x34, 0x34,   // VGH  ratio
      0xB3,  3, 0x0F, 0x0F, 0x0F,
      0xBA,  3, 0x24, 0x24, 0x24,   // VGLX  ratio
      0xB5,  2, 0x08, 0x08,
      0xBC,  3, 0x00, 0x78, 0x00,   // VGMP/VGSP 4.5V/0V
      0xBD,  3, 0x00, 0x78, 0x00,   // VGMN/VGSN -4.5V/0V
      0xBE,  2, 0x00, 0x89,         // VCOM  -1.325V

  // Gamma Setting
      0xD1, 52, 0x00, 0x2D, 0x00, 0x2E, 0x00, 0x32, 0x00, 0x44, 0x00, 0x53,
                0x00, 0x88, 0x00, 0xB6, 0x00, 0xF3, 0x01, 0x22, 0x01, 0x64,
                0x01, 0x92, 0x01, 0xD4, 0x02, 0x07, 0x02, 0x08, 0x02, 0x34,
                0x02, 0x5F, 0x02, 0x78, 0x02, 0x94, 0x02, 0xA6, 0x02, 0xBB,
                0x02, 0xCA, 0x02, 0xDB, 0x02, 0xE8, 0x02, 0xF9, 0x03, 0x1F,
                0x03, 0x7F,
      0xD4, 52, 0x00, 0x2D, 0x00, 0x2E, 0x00, 0x32, 0x00, 0x44, 0x00, 0x53,
                0x00, 0x88, 0x00, 0xB6, 0x00, 0xF3, 0x01, 0x22, 0x01, 0x64,
                0x01, 0x92, 0x01, 0xD4, 0x02, 0x07, 0x02, 0x08, 0x02, 0x34,
                0x02, 0x5F, 0x02, 0x78, 0x02, 0x94, 0x02, 0xA6, 0x02, 0xBB,
                0x02, 0xCA, 0x02, 0xDB, 0x02, 0xE8, 0x02, 0xF9, 0x03, 0x1F,
                0x03, 0x7F,  // R+ R-

      0xD2, 52, 0x00, 0x2D, 0x00, 0x2E, 0x00, 0x32, 0x00, 0x44, 0x00, 0x53,
                0x00, 0x88, 0x00, 0xB6, 0x00, 0xF3, 0x01, 0x22, 0x01, 0x64,
                0x01, 0x92, 0x01, 0xD4, 0x02, 0x07, 0x02, 0x08, 0x02, 0x34,
                0x02, 0x5F, 0x02, 0x78, 0x02, 0x94, 0x02, 0xA6, 0x02, 0xBB,
                0x02, 0xCA, 0x02, 0xDB, 0x02, 0xE8, 0x02, 0xF9, 0x03, 0x1F,
                0x03, 0x7F,
      0xD5, 52, 0x00, 0x2D, 0x00, 0x2E, 0x00, 0x32, 0x00, 0x44, 0x00, 0x53,
                0x00, 0x88, 0x00, 0xB6, 0x00, 0xF3, 0x01, 0x22, 0x01, 0x64,
                0x01, 0x92, 0x01, 0xD4, 0x02, 0x07, 0x02, 0x08, 0x02, 0x34,
                0x02, 0x5F, 0x02, 0x78, 0x02, 0x94, 0x02, 0xA6, 0x02, 0xBB,
                0x02, 0xCA, 0x02, 0xDB, 0x02, 0xE8, 0x02, 0xF9, 0x03, 0x1F,
                0x03, 0x7F,  // G+ G-

      0xD3, 52, 0x00, 0x2D, 0x00, 0x2E, 0x00, 0x32, 0x00, 0x44, 0x00, 0x53,
                0x00, 0x88, 0x00, 0xB6, 0x00, 0xF3, 0x01, 0x22, 0x01, 0x64,
                0x01, 0x92, 0x01, 0xD4, 0x02, 0x07, 0x02, 0x08, 0x02, 0x34,
                0x02, 0x5F, 0x02, 0x78, 0x02, 0x94, 0x02, 0xA6, 0x02, 0xBB,
                0x02, 0xCA, 0x02, 0xDB, 0x02, 0xE8, 0x02, 0xF9, 0x03, 0x1F,
                0x03, 0x7F,
      0xD6, 52, 0x00, 0x2D, 0x00, 0x2E, 0x00, 0x32, 0x00, 0x44, 0x00, 0x53,
                0x00, 0x88, 0x00, 0xB6, 0x00, 0xF3, 0x01, 0x22, 0x01, 0x64,
                0x01, 0x92, 0x01, 0xD4, 0x02, 0x07, 0x02, 0x08, 0x02, 0x34,
                0x02, 0x5F, 0x02, 0x78, 0x02, 0x94, 0x02, 0xA6, 0x02, 0xBB,
                0x02, 0xCA, 0x02, 0xDB, 0x02, 0xE8, 0x02, 0xF9, 0x03, 0x1F,
                0x03, 0x7F,  // B+ B-

      0xF0,  5, 0x55, 0xAA, 0x52, 0x08, 0x00,   //#Enable Page0

      0xB0,  5, 0x08, 0x05, 0x02, 0x05, 0x02,   //# RGB I/F Setting

      0xB6,  1, 0x08,
      0xB5,  1, 0x50,         //## SDT: //0x6b ?? 480x854  0x50 ?? 480x800
      0xB7,  2, 0x00, 0x00,   //## Gate EQ:
      0xB8,  4, 0x01, 0x05, 0x05, 0x05,   //## Source EQ:
      0xBC,  3, 0x00, 0x00, 0x00,         //# Inversion: Column inversion (NVT)
      0xCC,  3, 0x03, 0x00, 0x00,         //# BOE's Setting(default)
      0xBD,  6, 0x01, 0x84, 0x07, 0x31, 0x00, 0x01,   //# Display Timing:
      0xFF,  4, 0xAA, 0x55, 0x25, 0x01,
      0x35,  1, 0x00,        // teon

      CMD_SLPOUT, CMD_INIT_DELAY, 120,
      CMD_DISPON, 0,
      0xFF, 0xFF,
    };
    return list0;
  }

  bool Panel_NT35510::init(bool use_reset)
  {
    if (!Panel_LCD::init(use_reset)) return false;
    startWrite();

    for (uint8_t i = 0; auto cmds = getInitCommands(i); i++)
    {
      writeCommandList(cmds);
    }

    endWrite();

    return true;
  }

  void Panel_NT35510::setInvert(bool invert)
  {
    _invert = invert;
    startWrite();
    writeCommand((invert ^ _cfg.invert) ? CMD_INVON : CMD_INVOFF, 2);
    _bus->flush();
    endWrite();
  }

  void Panel_NT35510::update_madctl(void)
  {
    if (_bus != nullptr)
    {
      startWrite();
      writeCommand(CMD_COLMOD, 2);
      writeData(getColMod(_write_bits), 1);
      writeCommand(CMD_MADCTL, 2);
      writeData(getMadCtl(_internal_rotation) | (_cfg.rgb_order ? MAD_RGB : MAD_BGR), 1);
      _bus->flush();
      endWrite();
    }
  }

  void Panel_NT35510::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
  {
    bool dlen_16bit = _cfg.dlen_16bit;
    if (dlen_16bit && _has_align_data)
    {
      _bus->writeData(0, 8);
      _has_align_data = false;
    }

    if (xs != _xs || xe != _xe)
    {
      _xs = xs;
      xs += _colstart;
      writeRegister((CMD_CASET << 8) + 0, xs >> 8);
      writeRegister((CMD_CASET << 8) + 1, xs     );
      _xe = xe;
      xe += _colstart;
      writeRegister((CMD_CASET << 8) + 2, xe >> 8);
      writeRegister((CMD_CASET << 8) + 3, xe     );
    }
    if (ys != _ys || ye != _ye)
    {
      _ys = ys;
      ys += _rowstart;
      writeRegister((CMD_RASET << 8) + 0, ys >> 8);
      writeRegister((CMD_RASET << 8) + 1, ys     );
      _ye = ye;
      ye += _rowstart;
      writeRegister((CMD_RASET << 8) + 2, ye >> 8);
      writeRegister((CMD_RASET << 8) + 3, ye     );
    }
    writeCommand(CMD_RAMWR, 2);
  }

  void Panel_NT35510::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    uint_fast16_t bytes = param->dst_bits >> 3;
    auto len = w * h;
    if (!_cfg.readable)
    {
      memset(dst, 0, len * bytes);
      return;
    }

    startWrite();
    setWindow(x, y, x + w - 1, y + h - 1);

    writeCommand(_cmd_ramrd, 2);
    _bus->beginRead(_cfg.dummy_read_pixel);

    if (_bus->busType() == bus_type_t::bus_parallel16)
    { /// 16bitパラレル時の読出しは、RGB3Byte+ダミー1Byteの4Byte単位構成となっているため、ダミーデータの除去処理が必要;
      uint32_t* tmp = (uint32_t*)alloca(len * 4);
      _bus->readBytes((uint8_t*)tmp, len * 4);
      for (size_t idx = 0; idx < len; ++idx)
      {
        param->src_data = &tmp[idx];
        param->src_x32 = 0;
        param->fp_copy(dst, idx, idx + 1, param);
      }
    }
    else
    {
      if (param->no_convert)
      {
        _bus->readBytes((uint8_t*)dst, len * bytes);
      }
      else
      {
        _bus->readPixels(dst, param, len);
      }
    }
    cs_control(true);
    _bus->endRead();

    endWrite();

    if (_in_transaction) { cs_control(false); }
  }

//----------------------------------------------------------------------------
 }
}
