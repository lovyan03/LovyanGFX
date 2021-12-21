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
#if defined (ARDUINO)

#include "Bus_Stream.hpp"
#include "../../misc/pixelcopy.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Bus_Stream::config(const config_t& config)
  {
    _cfg = config;
  }

  void Bus_Stream::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    const uint8_t dst_bytes = bit_length >> 3;
    writeBytes((uint8_t*)&data, dst_bytes);
  }

  void Bus_Stream::writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t length)
  {
    const uint8_t dst_bytes = bit_length >> 3;
    uint32_t buf0 = data | data << bit_length;
    uint32_t buf1;
    uint32_t buf2;
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
    uint32_t src[8] = { buf0, buf1, buf2, buf0, buf1, buf2, buf0, buf1 };
    auto buf = reinterpret_cast<uint8_t*>(src);
    uint32_t limit = 32 / dst_bytes;
    uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      writeBytes(buf, len * dst_bytes);
    } while (length -= len);
  }

  void Bus_Stream::writePixels(pixelcopy_t* param, uint32_t length)
  {
    const uint8_t dst_bytes = param->dst_bits >> 3;
    uint32_t limit = 32 / dst_bytes;
    uint32_t len;
    uint8_t buf[32];
    do
    {
      len = ((length - 1) % limit) + 1;
      param->fp_copy(buf, 0, len, param);
      writeBytes(buf, len * dst_bytes);
    } while (length -= len);
  }

  void Bus_Stream::writeBytes(const uint8_t* data, uint32_t length)
  {
    _cfg.stream->write(data, length);
  }

  uint32_t Bus_Stream::readData(uint_fast8_t bit_length)
  {
    const uint8_t dst_bytes = bit_length >> 3;
    uint32_t res;
    _cfg.stream->readBytes(reinterpret_cast<uint8_t*>(&res), dst_bytes);
    return res;
  }

  bool Bus_Stream::readBytes(uint8_t* dst, uint32_t length, bool use_dma)
  {
    _cfg.stream->readBytes(dst, length);
    return true;
  }

  void Bus_Stream::readPixels(void* dst, pixelcopy_t* param, uint32_t length)
  {
    beginRead();
    const auto bytes = param->src_bits >> 3;
    uint32_t regbuf[8];
    uint32_t limit = 32 / bytes;

    param->src_data = regbuf;
    int32_t dstindex = 0;
    do {
      uint32_t len = (limit > length) ? length : limit;
      length -= len;
      _cfg.stream->readBytes((uint8_t*)regbuf, len * bytes);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len, param);
    } while (length);
  }

//----------------------------------------------------------------------------
 }
}

#endif
