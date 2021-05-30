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
#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (CONFIG_IDF_TARGET_ESP32S2) || defined (ESP_PLATFORM)
#elif defined (__SAMD51__)
#elif defined (STM32F2xx) || defined (STM32F4xx) || defined (STM32F7xx)
#elif defined ( ARDUINO )

#include "Bus_SPI.hpp"
#include "../../misc/pixelcopy.hpp"
#include <Arduino.h>
#include <SPI.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Bus_SPI::config(const config_t& config)
  {
    _cfg = config;

    if (_cfg.pin_dc >= 0)
    {
      pinMode(_cfg.pin_dc, pin_mode_t::output);
      gpio_hi(_cfg.pin_dc);
    }
  }

  void Bus_SPI::init(void)
  {
    dc_h();
    pinMode(_cfg.pin_dc, pin_mode_t::output);
  }

  void Bus_SPI::release(void)
  {
  }

  void Bus_SPI::beginTransaction(void)
  {
    dc_h();
    SPISettings setting(_cfg.freq_write, BitOrder::MSBFIRST, _cfg.spi_mode, true);
    SPI.beginTransaction(setting);
  }

  void Bus_SPI::endTransaction(void)
  {
    SPI.endTransaction();
    dc_h();
  }

  void Bus_SPI::beginRead(void)
  {
    SPI.endTransaction();
    SPISettings setting(_cfg.freq_read, BitOrder::MSBFIRST, _cfg.spi_mode, false);
    SPI.beginTransaction(setting);
  }

  void Bus_SPI::endRead(void)
  {
    SPI.endTransaction();
    beginTransaction();
  }

  void Bus_SPI::wait(void)
  {
  }

  bool Bus_SPI::busy(void) const
  {
    return false;
  }

  void Bus_SPI::writeCommand(std::uint32_t data, std::uint_fast8_t bit_length)
  {
    dc_l();
    do
    {
      SPI.transfer(data);
      data >>= 8;
    } while (bit_length -= 8);
    dc_h();
  }

  void Bus_SPI::writeData(std::uint32_t data, std::uint_fast8_t bit_length)
  {
    do
    {
      SPI.transfer(data);
      data >>= 8;
    } while (bit_length -= 8);
  }

  void Bus_SPI::writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t length)
  {
    const std::uint8_t dst_bytes = bit_length >> 3;
    std::uint32_t limit = (dst_bytes == 3) ? 12 : 16;
    auto dmabuf = _flip_buffer.getBuffer(1024);
    std::size_t fillpos = 0;
    reinterpret_cast<uint32_t*>(dmabuf)[0] = data;
    fillpos += dst_bytes;
    std::uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      if (limit <= 512) limit <<= 1;

      while (fillpos < len * dst_bytes)
      {
        memcpy(&dmabuf[fillpos], dmabuf, fillpos);
        fillpos += fillpos;
      }

      writeBytes(dmabuf, len * dst_bytes, true);
    } while (length -= len);
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, std::uint32_t length)
  {
    const std::uint8_t dst_bytes = param->dst_bits >> 3;
    std::uint32_t limit = (dst_bytes == 3) ? 12 : 16;
    std::uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      if (limit <= 512) limit <<= 1;
      auto dmabuf = _flip_buffer.getBuffer(len * dst_bytes);
      param->fp_copy(dmabuf, 0, len, param);
      writeBytes(dmabuf, len * dst_bytes, true);
    } while (length -= len);
  }

  void Bus_SPI::writeBytes(const std::uint8_t* data, std::uint32_t length, bool use_dma)
  {
    SPI.transfer(const_cast<std::uint8_t*>(data), length);
  }

  std::uint32_t Bus_SPI::readData(std::uint_fast8_t bit_length)
  {
    std::uint32_t res = 0;
    bit_length >>= 3;
    if (!bit_length) return res;
    int idx = 0;
    do
    {
      res |= SPI.transfer(0) << idx;
      idx += 8;
    } while (--bit_length);
    return res;
  }

  void Bus_SPI::readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma)
  {
    do
    {
      dst[0] = SPI.transfer(0);
      ++dst;
    } while (--length);
  }

  void Bus_SPI::readPixels(void* dst, pixelcopy_t* param, std::uint32_t length)
  {
    std::uint32_t bytes = param->src_bits >> 3;
    std::uint32_t dstindex = 0;
    std::uint32_t len = 4;
    std::uint8_t buf[24];
    param->src_data = buf;
    do {
      if (len > length) len = length;
      readBytes((std::uint8_t*)buf, len * bytes, true);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len, param);
      length -= len;
    } while (length);
  }

//----------------------------------------------------------------------------
 }
}

#endif
