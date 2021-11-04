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
#if defined (ESP_PLATFORM)
#elif defined (ESP8266)
#elif defined (__SAMD21__) || defined(__SAMD21G18A__) || defined(__SAMD21J18A__) || defined(__SAMD21E17A__) || defined(__SAMD21E18A__)
#elif defined (__SAMD51__)
#elif defined (STM32F2xx) || defined (STM32F4xx) || defined (STM32F7xx)
#elif defined (ARDUINO_ARCH_SPRESENSE)
#elif defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
#elif defined (ARDUINO)

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

  bool Bus_SPI::init(void)
  {
    dc_h();
    pinMode(_cfg.pin_dc, pin_mode_t::output);
  //SPI.pins(_cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi, -1);
    SPI.begin();
    return true;
  }

  void Bus_SPI::release(void)
  {
    SPI.end();
  }

  void Bus_SPI::beginTransaction(void)
  {
    dc_h();
    //SPISettings setting(_cfg.freq_write, BitOrder::MSBFIRST, _cfg.spi_mode, true);
    SPISettings setting(_cfg.freq_write, MSBFIRST, _cfg.spi_mode);
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
    //SPISettings setting(_cfg.freq_read, BitOrder::MSBFIRST, _cfg.spi_mode, false);
    SPISettings setting(_cfg.freq_read, MSBFIRST, _cfg.spi_mode);
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

  bool Bus_SPI::writeCommand(uint32_t data, uint_fast8_t bit_length)
  {
    dc_l();
    SPI.transfer((uint8_t*)&data, bit_length >> 3);
    dc_h();
    return true;
  }

  void Bus_SPI::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    SPI.transfer((uint8_t*)&data, bit_length >> 3);
  }

  void Bus_SPI::writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t length)
  {
/*
    auto bytes = bit_length >> 3;
    do
    {
      SPI.send(reinterpret_cast<uint8_t*>(&data), bytes);
    } while (--length);
/*/
    const uint8_t dst_bytes = bit_length >> 3;
    uint32_t limit = (dst_bytes == 3) ? 12 : 16;
    auto buf = _flip_buffer.getBuffer(512);
    size_t fillpos = 0;
    reinterpret_cast<uint32_t*>(buf)[0] = data;
    fillpos += dst_bytes;
    uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      if (limit <= 64) limit <<= 1;

      while (fillpos < len * dst_bytes)
      {
        memcpy(&buf[fillpos], buf, fillpos);
        fillpos += fillpos;
      }

      SPI.transfer(buf, len * dst_bytes);
    } while (length -= len);
//*/
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, uint32_t length)
  {
    const uint8_t dst_bytes = param->dst_bits >> 3;
    uint32_t limit = (dst_bytes == 3) ? 12 : 16;
    uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      if (limit <= 32) limit <<= 1;
      auto buf = _flip_buffer.getBuffer(len * dst_bytes);
      param->fp_copy(buf, 0, len, param);
      SPI.transfer(buf, len * dst_bytes);
    } while (length -= len);
  }

  void Bus_SPI::writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma)
  {
    if (dc) dc_h();
    else dc_l();
    SPI.transfer(const_cast<uint8_t*>(data), length);
    if (!dc) dc_h();
  }

  uint32_t Bus_SPI::readData(uint_fast8_t bit_length)
  {
    uint32_t res = 0;
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

  bool Bus_SPI::readBytes(uint8_t* dst, uint32_t length, bool use_dma)
  {
    do
    {
      dst[0] = SPI.transfer(0);
      ++dst;
    } while (--length);
    return true;
  }

  void Bus_SPI::readPixels(void* dst, pixelcopy_t* param, uint32_t length)
  {
    uint32_t bytes = param->src_bits >> 3;
    uint32_t dstindex = 0;
    uint32_t len = 4;
    uint8_t buf[24];
    param->src_data = buf;
    do {
      if (len > length) len = length;
      readBytes((uint8_t*)buf, len * bytes, true);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len, param);
      length -= len;
    } while (length);
  }

//----------------------------------------------------------------------------
 }
}

#endif
