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
#include "Panel_LCD.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Panel_LCD::init(bool use_reset)
  {
    Panel_Device::init(use_reset);

    startWrite(true);

    for (std::uint8_t i = 0; auto cmds = getInitCommands(i); i++)
    {
      command_list(cmds);
    }

    setInvert(_invert);

    setColorDepth(_write_depth);

    setRotation(_cfg.rotation);

    if (use_reset)
    {
      writeFillRectPreclipped(0, 0, _width, _height, 0);
      _bus->wait();
    }

    endWrite();
  }

  void Panel_LCD::beginTransaction(void)
  {
    begin_transaction();
  }
  void Panel_LCD::begin_transaction(void)
  {
    if (_in_transaction) return;
    _in_transaction = true;
    _bus->beginTransaction();
    cs_control(false);
  }

  void Panel_LCD::endTransaction(void)
  {
    end_transaction();
  }
  void Panel_LCD::end_transaction(void)
  {
    if (!_in_transaction) return;
    _in_transaction = false;

    if (_align_data)
    {
      _align_data = false;
      _bus->writeData(0, 8);
    }

    if (_cfg.pin_cs < 0)
    {
      write_command(0x00); // NOP command
    }
    _bus->endTransaction();
    cs_control(true);
  }

  void Panel_LCD::setInvert(bool invert)
  {
    _invert = invert;
    startWrite();
    write_command((invert ^ _cfg.invert) ? CMD_INVON : CMD_INVOFF);
    endWrite();
  }

  void Panel_LCD::setSleep(bool flg)
  {
    startWrite();
    write_command(flg ? CMD_SLPIN : CMD_SLPOUT);
    endWrite();
  }

  void Panel_LCD::setIdol(bool flg)
  {
    startWrite();
    write_command(flg ? CMD_IDMON : CMD_IDMOFF);
    endWrite();
  }

  color_depth_t Panel_LCD::setColorDepth(color_depth_t depth)
  {
    setColorDepth_impl(depth);
    _write_bits = _write_depth & color_depth_t::bit_mask;
    _read_bits = _read_depth & color_depth_t::bit_mask;

    startWrite();
    write_command(CMD_COLMOD);
    writeData(getColMod(_write_depth), 1);
    endWrite();
    return _write_depth;
  }
  void Panel_LCD::setRotation(std::uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

    _width  = (_internal_rotation & 1) ? _cfg.panel_height : _cfg.panel_width;
    _height = (_internal_rotation & 1) ? _cfg.panel_width : _cfg.panel_height;

    switch (_internal_rotation & 3) {
    default:  _colstart = _cfg.offset_x; break;
    case 1:   _colstart = _cfg.offset_y; break;
    case 2:   _colstart = _cfg.memory_width  - (_cfg.panel_width  + _cfg.offset_x); break;
    case 3:   _colstart = _cfg.memory_height - (_cfg.panel_height + _cfg.offset_y); break;
    }

    switch ((_internal_rotation & 3) | (_internal_rotation & 4)) {
    default:        _rowstart = _cfg.offset_y; break;
    case 1: case 7: _rowstart = _cfg.memory_width  - (_cfg.panel_width  + _cfg.offset_x); break;
    case 2: case 4: _rowstart = _cfg.memory_height - (_cfg.panel_height + _cfg.offset_y); break;
    case 3: case 5: _rowstart = _cfg.offset_x; break;
    }

    startWrite();
    write_command(CMD_MADCTL);
    writeData(getMadCtl(_internal_rotation) | (_cfg.rgb_order ? MAD_RGB : MAD_BGR), 1);
    endWrite();
  }

  void Panel_LCD::write_command(std::uint32_t data)
  {
    if (!_cfg.dlen_16bit)
    {
      _bus->writeCommand(data, 8);
    }
    else
    {
      if (_align_data)
      {
        _bus->writeData(0, 8);
        _align_data = false;
      }
      _bus->writeCommand(data << 8, 16);
    }
  }

  std::uint32_t Panel_LCD::readCommand(std::uint_fast8_t cmd, std::uint_fast8_t index, std::uint_fast8_t len)
  {
    startWrite();
    write_command(cmd);
    index = (index << 3) + _cfg.dummy_read_bits;
    auto res = read_bits(index, len << 3);
    endWrite();
    if (_in_transaction) { cs_control(false); }
    return res;
  }

  std::uint32_t Panel_LCD::readData(std::uint_fast8_t index, std::uint_fast8_t len)
  {
    startWrite();
    auto res = read_bits(index << 3, len << 3);
    endWrite();
    if (_in_transaction) { cs_control(false); }
    return res;
  }
/*
  void Panel_LCD::readBytes(std::uint8_t* dst, std::uint32_t len, bool use_dma)
  {
    startWrite();
    _bus->beginRead();
    _bus->readBytes(dst, len, use_dma);
    cs_control(true);
    _bus->endRead();
    endWrite();
    if (_in_transaction) { cs_control(false); }
  }
*/

  std::uint32_t Panel_LCD::read_bits(std::uint_fast8_t bit_index, std::uint_fast8_t bit_len)
  {
    _bus->beginRead();
    if (bit_index) { _bus->readData(bit_index); } // dummy read
    auto res = _bus->readData(bit_len);
    cs_control(true);
    _bus->endRead();
    return res;
  }

  void Panel_LCD::setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye)
  {
    if (!_cfg.dlen_16bit)
    {
      set_window_8(xs, ys, xe, ye, CMD_RAMWR);
    }
    else
    {
      set_window_16(xs, ys, xe, ye, CMD_RAMWR);
    }
  }

  void Panel_LCD::drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor)
  {
    bool tr = _in_transaction;
    if (!tr) begin_transaction();

    if (!_cfg.dlen_16bit)
    {
      set_window_8(x, y, x, y, CMD_RAMWR);
    }
    else
    {
      set_window_16(x, y, x, y, CMD_RAMWR);
      _align_data = _write_bits & 15;
    }
    _bus->writeData(rawcolor, _write_bits);

    if (!tr) end_transaction();
  }

  void Panel_LCD::writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor)
  {
    std::uint32_t len = w * h;
    std::uint_fast16_t xe = w + x - 1;
    std::uint_fast16_t ye = y + h - 1;
    if (!_cfg.dlen_16bit)
    {
      set_window_8(x, y, xe, ye, CMD_RAMWR);
    }
    else
    {
      set_window_16(x, y, xe, ye, CMD_RAMWR);
      _align_data = (_write_bits & 15) && (len & 1);
    }
    _bus->writeDataRepeat(rawcolor, _write_bits, len);
  }

  void Panel_LCD::writeBlock(std::uint32_t rawcolor, std::uint32_t len)
  {
    _bus->writeDataRepeat(rawcolor, _write_bits, len);
    if (_cfg.dlen_16bit && (_write_bits & 15) && (len & 1))
    {
      _align_data = !_align_data;
    }
  }

  void Panel_LCD::writePixels(pixelcopy_t* param, std::uint32_t len)
  {
    _bus->writePixels(param, len);
    if (_cfg.dlen_16bit && (_write_bits & 15) && (len & 1))
    {
      _align_data = !_align_data;
    }
  }

  void Panel_LCD::writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    auto bytes = param->dst_bits >> 3;
    auto src_x = param->src_x;

    if (param->transp == ~0u)
    {
      if (param->no_convert)
      {
        auto wb = w * bytes;
        std::uint32_t i = (src_x + param->src_y * param->src_bitwidth) * bytes;
        auto src = &((const std::uint8_t*)param->src_data)[i];
        setWindow(x, y, x + w - 1, y + h - 1);
        if (param->src_bitwidth == w || h == 1)
        {
          write_bytes(src, wb * h, use_dma);
        }
        else
        {
          auto add = param->src_bitwidth * bytes;
          if (use_dma)
          {
            if (_cfg.dlen_16bit && ((wb * h) & 1))
            {
              _align_data = !_align_data;
            }
            do
            {
              _bus->addDMAQueue(src, wb);
              src += add;
            } while (--h);
            _bus->execDMAQueue();
          }
          else
          {
            do
            {
              write_bytes(src, wb, false);
              src += add;
            } while (--h);
          }
        }
      }
      else
      {
        if (!_bus->busy() && (h == 1 || (param->src_bitwidth == w && w * h <= INT16_MAX)))
        {
          setWindow(x, y, x + w - 1, y + h - 1);
          writePixels(param, w * h);
        }
        else
        {
          std::int32_t wb = w * bytes;
          auto buf = _bus->getDMABuffer(wb);
          param->fp_copy(buf, 0, w, param);
          setWindow(x, y, x + w - 1, y + h - 1);
          write_bytes(buf, wb, true);
          while (--h)
          {
            param->src_x = src_x;
            param->src_y++;
            buf = _bus->getDMABuffer(wb);
            param->fp_copy(buf, 0, w, param);
            write_bytes(buf, wb, true);
          }
        }
      }
    }
    else
    {
      h += y;
      std::uint32_t wb = w * bytes;
      do
      {
        std::uint32_t i = 0;
        while (w != (i = param->fp_skip(i, w, param)))
        {
          auto buf = _bus->getDMABuffer(wb);
          std::int32_t len = param->fp_copy(buf, 0, w - i, param);
          setWindow(x + i, y, x + i + len - 1, y);
          write_bytes(buf, len * bytes, true);
          if (w == (i += len)) break;
        }
        param->src_x = src_x;
        param->src_y++;
      } while (++y != h);
    }
  }

  void Panel_LCD::write_bytes(const std::uint8_t* data, std::uint32_t len, bool use_dma)
  {
    _bus->writeBytes(data, len, use_dma);
    if (_cfg.dlen_16bit && (_write_bits & 15) && (len & 1))
    {
      _align_data = !_align_data;
    }
  }

  void Panel_LCD::readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    std::uint_fast16_t bytes = param->dst_bits >> 3;
    auto len = w * h;
    if (!_cfg.readable)
    {
      memset(dst, 0, len * bytes);
      return;
    }

    startWrite();
    setWindow(x, y, x + w - 1, y + h - 1);
/*
    _bus->beginRead();
    if (bit_index) { _bus->readData(bit_index); } // dummy read
    auto res = _bus->readData(bit_len);
    gpio_hi(spi_cs);
    _bus->endRead();
    return res;
*/
    write_command(CMD_RAMRD);
    _bus->beginRead();

    if (_cfg.dummy_read_pixel)
    {
      _bus->readData(_cfg.dummy_read_pixel); // dummy read
    }

    if (param->no_convert)
    {
      _bus->readBytes((std::uint8_t*)dst, len * bytes);
    }
    else
    {
      _bus->readPixels(dst, param, len);
    }
    cs_control(true);
    _bus->endRead();

    endWrite();

    if (_in_transaction) { cs_control(false); }
  }

  void Panel_LCD::set_window_8(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye, std::uint32_t cmd)
  {
    if (xs != _xs || xe != _xe)
    {
      _xs = xs;
      _xe = xe;
      _bus->writeCommand(CMD_CASET, 8);
      xs += _colstart;
      xe += _colstart;
      _bus->writeData(xs >> 8 | (xs & 0xFF) << 8 | (xe << 8 | xe >> 8) << 16, 32);
    }
    if (ys != _ys || ye != _ye)
    {
      _ys = ys;
      _ye = ye;
      _bus->writeCommand(CMD_RASET, 8);
      ys += _rowstart;
      ye += _rowstart;
      _bus->writeData(ys >> 8 | (ys & 0xFF) << 8 | (ye << 8 | ye >> 8) << 16, 32);
    }
    _bus->writeCommand(cmd, 8);
  }

  void Panel_LCD::set_window_16(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye, std::uint32_t cmd)
  {
    if (_align_data)
    {
      _bus->writeData(0, 8);
      _align_data = false;
    }
    if (xs != _xs || xe != _xe)
    {
      _bus->writeCommand(CMD_CASET << 8, 16);
      _xs = xs;
      xs += _colstart;
      _bus->writeData((xs >> 8) << 8 | xs << 24, 32);
      _xe = xe;
      xe += _colstart;
      _bus->writeData((xe >> 8) << 8 | xe << 24, 32);
    }
    if (ys != _ys || ye != _ye)
    {
      _bus->writeCommand(CMD_RASET << 8, 16);
      _ys = ys;
      ys += _rowstart;
      _bus->writeData((ys >> 8) << 8 | ys << 24, 32);
      _ye = ye;
      ye += _rowstart;
      _bus->writeData((ye >> 8) << 8 | ye << 24, 32);
    }
    _bus->writeCommand(cmd << 8, 16);
  }

//----------------------------------------------------------------------------
 }
}
