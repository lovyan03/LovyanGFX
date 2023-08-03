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
#include "Panel_RA8875.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"

#include "../platforms/device.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  const uint8_t* Panel_RA8875::getInitCommands(uint8_t listno) const
  {
    static constexpr uint8_t list0[] PROGMEM =
    {
      0x88      , 1+CMD_INIT_DELAY, 0x0B, 1, // PLL ini
      0x89      , 1+CMD_INIT_DELAY, 0x01, 1,
      0x04      , 1+CMD_INIT_DELAY, 0x82, 1,    //PCLK
    // 0x14      , 1, 0x63, //HDWR//Horizontal Display Width Setting Bit[6:0]  //Horizontal display width(pixels) = (HDWR + 1)*8       0x27
      0x15      , 1, 0x02, //HNDFCR//Horizontal Non-Display Period fine tune Bit[3:0]  //(HNDR + 1)*8 +HNDFCR
      0x16      , 1, 0x03, //HNDR//Horizontal Non-Display Period Bit[4:0] //Horizontal Non-Display Period (pixels) = (HNDR + 1)*8
      0x17      , 1, 0x01, //HSTR//HSYNC Start Position[4:0]  //HSYNC Start Position(PCLK) = (HSTR + 1)*8
      0x18      , 1, 0x03, //HPWR//HSYNC Polarity ,The period width of HSYNC.  //HSYNC Width [4:0]   HSYNC Pulse width(PCLK) = (HPWR + 1)*8

//Vertical set
    // 0x19      , 1, 0xDF, //VDHR0 //Vertical Display Height Bit [7:0] //Vertical pixels = VDHR + 1	0xef
    // 0x1a      , 1, 0x01, //VDHR1 //Vertical Display Height Bit [8]  //Vertical pixels = VDHR + 1 	0x00
      0x1b      , 1, 0x0F, //VNDR0 //Vertical Non-Display Period Bit [7:0]  //Vertical Non-Display area = (VNDR + 1)
      0x1c      , 1, 0x00, //VNDR1 //Vertical Non-Display Period Bit [8] //Vertical Non-Display area = (VNDR + 1)
      0x1d      , 1, 0x0e, //VSTR0 //VSYNC Start Position[7:0]  //VSYNC Start Position(PCLK) = (VSTR + 1)
      0x1e      , 1, 0x06, //VSTR1 //VSYNC Start Position[8]  //VSYNC Start Position(PCLK) = (VSTR + 1)
      0x1f      , 1, 0x01, //VPWR //VSYNC Polarity ,VSYNC Pulse Width[6:0]  //VSYNC Pulse Width(PCLK) = (VPWR + 1)

      0x8a      , 1, 0x80, //PWM setting
      0x8a      , 1, 0x81, //PWM setting //open PWM
      0x8b      , 1, 0x7F, //Backlight brightness setting //Brightness parameter 0xff-0x00

      0x01      , 1, 0x80, //display on

      0xFF,0xFF, // end
    };
    switch (listno)
    {
    case 0: return list0;
    default: return nullptr;
    }
  }

  bool Panel_RA8875::init(bool use_reset)
  {
    _flg_serialbus = _bus->busType() == bus_spi;

    if (!Panel_Device::init(use_reset))
    {
      return false;
    }

    if (_cfg.pin_busy >= 0)
    {
      pinMode(_cfg.pin_busy, pin_mode_t::input);
    }

    uint32_t freq_write = _bus->getClock();
    if (freq_write > 5000000)
    {
      /// 初期化処理は低めのクロックで実施する。;
      /// RA8875は起動直後は動作クロックが低く、送信クロックが速すぎると処理されないため。;
      /// 初期化時に動作クロックを上げるコマンドが実行されるので、;
      /// 初期化後にユーザーが設定した本来のクロックに戻す。;
      _bus->setClock(5000000);
    }

    startWrite(true);

    for (uint8_t i = 0; auto cmds = getInitCommands(i); i++)
    {
      command_list(cmds);
    }

    {
      //HDWR  // Horizontal Display Width Setting
      _write_reg( 0x14, (_cfg.offset_x + _cfg.panel_width +7) >> 3);

      uint_fast16_t height = _cfg.offset_y + _cfg.panel_height - 1;
      //VDHR0  // Vertical Display Height Setting
      _write_reg( 0x19, height);
      _write_reg( 0x1a, height >> 8);
    }

    endWrite();

    /// 初期化後にクロックをユーザー設定値に戻す;
    _bus->setClock(freq_write);

    _latestcolor = 0;

    uint16_t data16[6] = { (uint16_t)_cfg.offset_x
                         , (uint16_t)_cfg.offset_y
                         , (uint16_t)(_cfg.offset_x + _cfg.panel_width)
                         , (uint16_t)(_cfg.offset_y + _cfg.panel_height)
                         };
    auto data = (uint8_t*)data16;
    for (size_t idx = 0; idx < 8; ++idx)
    {
      _write_reg(0x30 + idx, data[idx]);
    }

    for (size_t reg = 0x51; reg <= 0x65; ++reg)
    {
      _write_reg(reg, 0);
    }

    return true;
  }

  void Panel_RA8875::beginTransaction(void)
  {
    begin_transaction();
  }
  void Panel_RA8875::begin_transaction(void)
  {
    if (_in_transaction) return;
    _in_transaction = true;
    _flg_memorywrite = false;
    _bus->beginTransaction();

    if (!_flg_serialbus) { cs_control(false); }
  }

  void Panel_RA8875::endTransaction(void)
  {
    end_transaction();
  }
  void Panel_RA8875::end_transaction(void)
  {
    if (!_in_transaction) return;
    _in_transaction = false;

    _bus->endTransaction();
    cs_control(true);
  }

  color_depth_t Panel_RA8875::setColorDepth(color_depth_t depth)
  {
    depth = ((int)depth & color_depth_t::bit_mask) >= 16 ? rgb565_2Byte : rgb332_1Byte;
    _write_depth = depth;
    _read_depth = depth;

    update_madctl();

    return depth;
  }
  void Panel_RA8875::setRotation(uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

    _width  = _cfg.panel_width;
    _height = _cfg.panel_height;
    if (_internal_rotation & 1) { std::swap(_width, _height); }

    _colstart = _cfg.offset_x;
    _rowstart = _cfg.offset_y;

    _xs = _xe = _ys = _ye = INT16_MAX;

    update_madctl();
  }

  void Panel_RA8875::update_madctl(void)
  {
    //SYSR   bit[4:3]=00 256 color  bit[2:1]=  00 8bit MPU interface    1x 64k color  1x 16bit
    _write_reg(0x10, _write_depth == rgb565_2Byte ? 0x0C : 0x00);

    uint_fast8_t data = 0;
    switch (_internal_rotation & 3)
    {
    default:
    case 0: case 4:              break; // left to right 0x00
    case 1: case 5: data = 0x02; break; // top to bottom 0x08
    case 2: case 6: data = 0x01; break; // right to left 0x04
    case 3: case 7: data = 0x03; break; // bottom to top 0x0C
    }
    _write_reg(0x40, data << 2); // write direction
    _write_reg(0x45, data); // read direction

    _latestcolor = ~0u;

    return;
  }

  void Panel_RA8875::waitDisplay(void)
  {
    _wait_busy();
  }

  bool Panel_RA8875::displayBusy(void)
  {
    if (_bus->busy()) return true;
    if (_cfg.pin_busy >= 0 && !lgfx::gpio_in(_cfg.pin_busy)) return true;
    return false;
  }

  bool Panel_RA8875::_wait_busy(uint32_t timeout)
  {
    _bus->wait();
    cs_control(true);
    int_fast16_t pin = _cfg.pin_busy;
    if (pin >= 0 && !lgfx::gpio_in(pin))
    {
      auto time = millis();
      do
      {
        if (millis() - time > timeout)
        {
          return false;
        }
      } while (!lgfx::gpio_in(pin));
    }
    cs_control(false);
    return true;
  }

  void Panel_RA8875::_write_reg(uint_fast16_t reg, uint_fast16_t data)
  {
    _flg_memorywrite = false;
    if (_flg_serialbus)
    {
      uint32_t value = (data << 24) + (reg << 8) + 0x80;
      _wait_busy();
      _bus->writeCommand(value, 32);
    }
    else
    {
      _bus->flush();
      uint_fast8_t len = 8;
      if (_cfg.dlen_16bit)
      {
        len <<= 1;
        reg <<= 8;
        data <<= 8;
      }
      _wait_busy();
      _bus->writeCommand(reg, len);
      _bus->writeData(data, len);
    }
  }

  void Panel_RA8875::_write_reg_0x51(uint8_t reg, uint8_t data)
  {
    size_t index = reg - 0x51;
    if (index < sizeof(_reg_0x51))
    {
      if (_reg_0x51[index] == data) { return; }
      _reg_0x51[index] = data;
    }
    _write_reg(reg, data);
  }

  void Panel_RA8875::_start_memorywrite(void)
  {
    if (_flg_memorywrite) { return; }
    _flg_memorywrite = true;
    if (_flg_serialbus)
    {
      _wait_busy();
      _bus->writeCommand((0x02 << 8) + 0x80, 24);
    }
    else
    {
      _bus->flush();
      uint32_t reg = 0x02;
      uint_fast8_t len = 8;
      if (_cfg.dlen_16bit)
      {
        len <<= 1;
        reg <<= 8;
      }
      _wait_busy();
      _bus->writeCommand(reg, len);
    }
  }

  void Panel_RA8875::writeCommand(uint32_t cmd, uint_fast8_t length)
  {
    if (_flg_serialbus)
    {
      cmd = (cmd << 8) + 0x80;
      _wait_busy();
      _bus->writeCommand(cmd, 16);
    }
    else
    {
      Panel_Device::writeCommand(cmd, length);
    }
  }
  void Panel_RA8875::writeData(uint32_t data, uint_fast8_t length)
  {
    if (_flg_serialbus)
    {
      data <<= 8;
      _wait_busy();
      _bus->writeData(data, 16);
    }
    else
    {
      Panel_Device::writeData(data, length);
    }
  }

  void Panel_RA8875::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
  {
    _xs = xs;
    _xe = xe;
    _ys = ys;
    _ye = ye;
    _xpos = _ypos = INT16_MAX;
  }

  void Panel_RA8875::_set_write_pos(uint_fast16_t x, uint_fast16_t y)
  {
    auto flg_x = (_xpos != x);
    _xpos = x;
    auto flg_y = (_ypos != y);
    _ypos = y;
    if (flg_x || flg_y)
    {
      uint_fast8_t r = _internal_rotation;
      if (r)
      {
        if ((1u << r) & 0b10010110) { y = _height - (y + 1); }
        if (r & 2)                  { x = _width  - (x + 1); }
        if (r & 1) { std::swap(x, y); std::swap(flg_x, flg_y); }
      }
      if (flg_x)
      {
        x += _colstart;
        _write_reg(0x46, x);
        _write_reg(0x47, x >> 8);
      }
      if (flg_y)
      {
        y += _rowstart;
        _write_reg(0x48, y);
        _write_reg(0x49, y >> 8);
      }
    }
  }

  void Panel_RA8875::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    bool tr = _in_transaction;
    if (!tr) begin_transaction();

    writeFillRectPreclipped(x, y, 1, 1, rawcolor);

    if (!tr) end_transaction();
  }

  void Panel_RA8875::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    _xs = x;
    _xe = x + w - 1;
    _ys = y;
    _ye = y + h - 1;
    if (h == 1 && w <= 8)
    {
      _set_write_pos(x, y);
      writeBlock(rawcolor, w);
    }
    else
    {
      if (_latestcolor != rawcolor)
      {
        _latestcolor = rawcolor;
        if (_write_depth == rgb565_2Byte)
        {
          rawcolor = getSwap16(rawcolor);
          _write_reg(0x63, rawcolor >>11);
          _write_reg(0x64, rawcolor >> 5);
          _write_reg(0x65, rawcolor     );
        }
        else
        {
          _write_reg(0x63, rawcolor >> 5);
          _write_reg(0x64, rawcolor >> 2);
          _write_reg(0x65, rawcolor     );
        }
      }

      uint_fast8_t r = _internal_rotation;
      if (r)
      {
        if ((1u << r) & 0b10010110) { y = _height - (y + h); }
        if (r & 2)                  { x = _width  - (x + w); }
        if (r & 1) { std::swap(x, y);  std::swap(w, h); }
      }

      _write_reg_0x51(0x51, 0x0C); // Solid Fill.

      x += _colstart;
      y += _rowstart;
      uint16_t data16[6] = { (uint16_t)x
                           , (uint16_t)y
                           , (uint16_t)w
                           , (uint16_t)h
                           };
      auto data = (uint8_t*)data16;
      for (size_t idx = 0; idx < 8; ++idx)
      {
        _write_reg_0x51(0x58 + idx, data[idx]);
      }
      _write_reg(0x50, 0x80);
    }
  }

  void Panel_RA8875::writeBlock(uint32_t rawcolor, uint32_t length)
  {
    uint32_t xpos = _xpos;
    uint32_t ypos = _ypos;
    do
    {
      bool flg_setpos = false;
      if (xpos > _xe) { flg_setpos = true; xpos = _xs; ++ypos; }
      if (ypos > _ye) { flg_setpos = true; ypos = _ys;         }
      if (flg_setpos)
      {
        _set_write_pos(xpos, ypos);
      }

      _start_memorywrite();

      auto w = std::min<uint32_t>(length, _xe + 1 - xpos);
      xpos += w;
      _xpos = xpos;
      _bus->writeDataRepeat(rawcolor, _write_bits, w);
      length -= w;
    } while (length);
  }

  void Panel_RA8875::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    uint32_t xpos = _xpos;
    uint32_t ypos = _ypos;
    do
    {
      bool flg_setpos = false;
      if (xpos > _xe) { flg_setpos = true; xpos = _xs; ++ypos; }
      if (ypos > _ye) { flg_setpos = true; ypos = _ys;         }
      if (flg_setpos)
      {
        _set_write_pos(xpos, ypos);
      }

      _start_memorywrite();

      auto w = std::min<uint32_t>(length, _xe + 1 - xpos);
      xpos += w;
      _xpos = xpos;

      if (param->no_convert)
      {
        _bus->writeBytes(reinterpret_cast<const uint8_t*>(param->src_data), w * _write_bits >> 3, true, use_dma);
      }
      else
      {
        _bus->writePixels(param, w);
      }

      length -= w;
    } while (length);
  }

  void Panel_RA8875::write_bytes(const uint8_t* data, uint32_t length, bool use_dma)
  {
    uint32_t xpos = _xpos;
    uint32_t ypos = _ypos;
    do
    {
      bool flg_setpos = false;
      if (xpos > _xe) { flg_setpos = true; xpos = _xs; ++ypos; }
      if (ypos > _ye) { flg_setpos = true; ypos = _ys;         }
      if (flg_setpos)
      {
        _set_write_pos(xpos, ypos);
      }

      _start_memorywrite();

      auto w = std::min<uint32_t>(length >> (_write_bits >> 4), _xe + 1 - xpos);
      xpos += w;
      _xpos = xpos;
      w <<= (_write_bits >> 4);
      _bus->writeBytes(data, w, true, use_dma);
      data += w;
      length -= w;
    } while (length);
  }

  void Panel_RA8875::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    auto bytes = param->dst_bits >> 3;
    auto src_x = param->src_x;

    if (param->transp == pixelcopy_t::NON_TRANSP)
    {
      setWindow(x, y, x + w - 1, y + h - 1);
      if (param->no_convert)
      {
        auto wb = w * bytes;
        uint32_t i = (src_x + param->src_y * param->src_bitwidth) * bytes;
        auto src = &((const uint8_t*)param->src_data)[i];
        auto add = param->src_bitwidth * bytes;
        do
        {
          write_bytes(src, wb, use_dma);
          src += add;
        } while (--h);
      }
      else
      {
        do
        {
          writePixels(param, w, use_dma);
          param->src_x = src_x;
          param->src_y++;
        } while (--h);
      }
    }
    else
    {
      h += y;
      uint32_t wb = w * bytes;
      do
      {
        uint32_t i = 0;
        while (w != (i = param->fp_skip(i, w, param)))
        {
          auto buf = _bus->getDMABuffer(wb);
          int32_t len = param->fp_copy(buf, 0, w - i, param);
          setWindow(x + i, y, x + i + len - 1, y);
          write_bytes(buf, len * bytes, true);
          if (w == (i += len)) break;
        }
        param->src_x = src_x;
        param->src_y++;
      } while (++y != h);
    }
  }

  void Panel_RA8875::copyRect(uint_fast16_t dst_x, uint_fast16_t dst_y, uint_fast16_t w, uint_fast16_t h, uint_fast16_t src_x, uint_fast16_t src_y)
  {
    uint_fast8_t r = _internal_rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { src_y = _height - (src_y + h); dst_y = _height - (dst_y + h); }
      if (r & 2)                  { src_x = _width  - (src_x + w); dst_x = _width  - (dst_x + w); }
      if (r & 1) { std::swap(src_x, src_y); std::swap(dst_x, dst_y); std::swap(w, h); }
    }

    bool positive = (dst_y < src_y || (dst_y == src_y && dst_x < src_x));

    _write_reg_0x51( 0x51, positive ? 0xC2 : 0xC3 );
    if (!positive)
    {
      src_x += w - 1;
      dst_x += w - 1;
      src_y += h - 1;
      dst_y += h - 1;
    }
    src_x += _colstart;
    dst_x += _colstart;
    src_y += _rowstart;
    dst_y += _rowstart;

    uint16_t data16[6] = { (uint16_t)src_x
                         , (uint16_t)src_y
                         , (uint16_t)dst_x
                         , (uint16_t)dst_y
                         , (uint16_t)w
                         , (uint16_t)h
                         };
    auto data = (uint8_t*)data16;
    for (size_t idx = 0; idx < 12; ++idx)
    {
      _write_reg_0x51(0x54 + idx, data[idx]);
    }
    _write_reg(0x50, 0x80);
  }

  void Panel_RA8875::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    uint_fast16_t bytes = param->dst_bits >> 3;
    auto len = w * h;
    if (!_cfg.readable)
    {
      memset(dst, 0, len * bytes);
      return;
    }

    auto dst8 = (uint8_t*)dst;

    int xadd = 0;
    int yadd = 1;
    uint_fast8_t r = _internal_rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { y = _height - (y + 1); yadd = -1; }
      if (r & 2)                  { x = _width  - (x + 1); }
      if (r & 1) { std::swap(x, y); std::swap(xadd, yadd); } // ここでは wとhは入れ替えない;
    }

    x += _colstart;
    y += _rowstart;

    startWrite();
    do
    {
      _write_reg(0x4A, x);
      _write_reg(0x4B, x >> 8);
      _write_reg(0x4C, y);
      _write_reg(0x4D, y >> 8);

      if (_flg_serialbus)
      {
        _wait_busy();
        _bus->writeCommand(0x80 + (0x02 << 8) + (0x40 << 16), 24);
      }
      else
      {
        _bus->flush();
        uint32_t reg = 0x02;
        uint_fast8_t len_ = 8;
        if (_cfg.dlen_16bit)
        {
          len_ <<= 1;
          reg <<= 8;
        }
        _wait_busy();
        _bus->writeCommand(reg, len_);
      }

      _bus->beginRead(_cfg.dummy_read_pixel);
      if (param->no_convert)
      {
        _bus->readBytes(dst8, w * bytes);
      }
      else
      {
        _bus->readPixels(dst8, param, w);
      }
      dst8 += w * bytes;
      _bus->endRead();
      x += xadd;
      y += yadd;
    } while (--h);

    endWrite();

    if (_in_transaction) { cs_control(false); }
  }

//----------------------------------------------------------------------------
 }
}
