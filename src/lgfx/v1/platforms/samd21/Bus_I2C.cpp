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
#if defined (__SAMD21__)

#include "Bus_I2C.hpp"

#include "../../misc/pixelcopy.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Bus_I2C::config(const config_t& config)
  {
    _cfg = config;
  }

  bool Bus_I2C::init(void)
  {
    _state = state_t::state_none;
    return lgfx::i2c::init(_cfg.sercom_index, _cfg.pin_sda, _cfg.pin_scl).has_value();
  }

  void Bus_I2C::release(void)
  {
    lgfx::i2c::release(_cfg.sercom_index);
    _state = state_t::state_none;
  }

  void Bus_I2C::beginTransaction(void)
  {
    // 既に開始直後の場合は終了
    if (_state == state_t::state_write_none)
    {
      return;
    }

    if (_state != state_none)
    {
      lgfx::i2c::endTransaction(_cfg.sercom_index);
    }
    lgfx::i2c::beginTransaction(_cfg.sercom_index, _cfg.i2c_addr, _cfg.freq_write, false);
    _state = state_t::state_write_none;
  }

  void Bus_I2C::beginRead(void)
  {
    if (_state == state_t::state_read)
    {
      return;
    }
    
    if (_state != state_t::state_none)
    {
      lgfx::i2c::restart(_cfg.sercom_index, _cfg.i2c_addr, _cfg.freq_read, true);
    }
    else
    {
      lgfx::i2c::beginTransaction(_cfg.sercom_index, _cfg.i2c_addr, _cfg.freq_read, true);
    }
    _state = state_t::state_read;
  }

  void Bus_I2C::endTransaction(void)
  {
    if (_state == state_t::state_none)
    {
      return;
    }

    _state = state_t::state_none;
    lgfx::i2c::endTransaction(_cfg.sercom_index);
  }

  void Bus_I2C::endRead(void)
  {
    endTransaction();
  }

  void Bus_I2C::wait(void)
  {
    // auto dev = (_cfg.sercom_index == 0) ? &I2C0 : &I2C1;
    // while (dev->status_reg.bus_busy) { taskYIELD(); }
  }

  bool Bus_I2C::busy(void) const
  {
    // auto dev = (_cfg.sercom_index == 0) ? &I2C0 : &I2C1;
    // return dev->status_reg.bus_busy;
    return false;
  }

  void Bus_I2C::dc_control(bool dc)
  {
    // リード中の場合はトランザクションを終了しておく
    if (_state == state_t::state_read)
    {
      _state = state_t::state_none;
      lgfx::i2c::endTransaction(_cfg.sercom_index);
    }

    // まだトランザクションが開始されていない場合は開始しておく
    if (_state == state_t::state_none)
    {
      _state = state_t::state_write_none;
      lgfx::i2c::beginTransaction(_cfg.sercom_index, _cfg.i2c_addr, _cfg.freq_write, false);
    }

    // DCプリフィクスなしの場合は後の処理は不要
    if (_cfg.prefix_len == 0) return;

    state_t st = dc ? state_t::state_write_data : state_t::state_write_cmd;
    // 既に送信済みのDCプリフィクスが要求と一致している場合は終了
    if (_state == st) return;

    // DCプリフィクスが送信済みの場合、送信済みのDCプリフィクスと要求が不一致なのでトランザクションをやり直す。
    if (_state != state_t::state_write_none)
    {
      lgfx::i2c::endTransaction(_cfg.sercom_index);
      lgfx::i2c::beginTransaction(_cfg.sercom_index, _cfg.i2c_addr, _cfg.freq_write, false);
    }
    lgfx::i2c::writeBytes(_cfg.sercom_index, (std::uint8_t*)(dc ? &_cfg.prefix_data : &_cfg.prefix_cmd), _cfg.prefix_len);
    _state = st;
  }

  bool Bus_I2C::writeCommand(std::uint32_t data, std::uint_fast8_t bit_length)
  {
    dc_control(false);
    return lgfx::i2c::writeBytes(_cfg.sercom_index, (std::uint8_t*)&data, (bit_length >> 3)).has_value();
  }

  void Bus_I2C::writeData(std::uint32_t data, std::uint_fast8_t bit_length)
  {
    dc_control(true);
    lgfx::i2c::writeBytes(_cfg.sercom_index, (std::uint8_t*)&data, (bit_length >> 3));
  }

  void Bus_I2C::writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t length)
  {
    dc_control(true);
    const std::uint8_t dst_bytes = bit_length >> 3;
    std::uint32_t buf0 = data | data << bit_length;
    std::uint32_t buf1;
    std::uint32_t buf2;
    // make 12Bytes data.
    if (dst_bytes != 3)
    {
      if (dst_bytes == 1)
      {
        buf0 |= buf0 << 16;
      }
      buf1 = buf0;
      buf2 = buf0;
    }
    else
    {
      buf1 = buf0 >>  8 | buf0 << 16;
      buf2 = buf0 >> 16 | buf0 <<  8;
    }
    std::uint32_t src[8] = { buf0, buf1, buf2, buf0, buf1, buf2, buf0, buf1 };
    auto buf = reinterpret_cast<std::uint8_t*>(src);
    std::uint32_t limit = 32 / dst_bytes;
    std::uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      i2c::writeBytes(_cfg.sercom_index, buf, len * dst_bytes);
    } while (length -= len);
  }

  void Bus_I2C::writePixels(pixelcopy_t* param, std::uint32_t length)
  {
    dc_control(true);
    const std::uint8_t dst_bytes = param->dst_bits >> 3;
    std::uint32_t limit = 32 / dst_bytes;
    std::uint32_t len;
    std::uint8_t buf[32];
    do
    {
      len = ((length - 1) % limit) + 1;
      param->fp_copy(buf, 0, len, param);
      i2c::writeBytes(_cfg.sercom_index, buf, len * dst_bytes);
    } while (length -= len);
  }

  void Bus_I2C::writeBytes(const std::uint8_t* data, std::uint32_t length, bool dc, bool use_dma)
  {
    if (length < 64)
    {
      dc_control(dc);
      i2c::writeBytes(_cfg.sercom_index, data, length);
    }
    else
    {
      std::uint32_t len = ((length - 1) & 63) + 1;
      do
      {
        dc_control(dc);
        i2c::writeBytes(_cfg.sercom_index, data, len);
        data += len;
        length -= len;
        endTransaction();
        beginTransaction();
        len = 64;
      } while (length);
    }
  }

  std::uint32_t Bus_I2C::readData(std::uint_fast8_t bit_length)
  {
    beginRead();
    std::uint32_t res;
    i2c::readBytes(_cfg.sercom_index, reinterpret_cast<std::uint8_t*>(&res), bit_length >> 3);
    return res;
  }

  bool Bus_I2C::readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma)
  {
    beginRead();
    return i2c::readBytes(_cfg.sercom_index, dst, length).has_value();
  }

  void Bus_I2C::readPixels(void* dst, pixelcopy_t* param, std::uint32_t length)
  {
    beginRead();
    const auto bytes = param->src_bits >> 3;
    std::uint32_t regbuf[8];
    std::uint32_t limit = 32 / bytes;

    param->src_data = regbuf;
    std::int32_t dstindex = 0;
    do {
      std::uint32_t len = (limit > length) ? length : limit;
      length -= len;
      i2c::readBytes(_cfg.sercom_index, (std::uint8_t*)regbuf, len * bytes);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len, param);
    } while (length);
  }

//----------------------------------------------------------------------------
 }
}

#endif
