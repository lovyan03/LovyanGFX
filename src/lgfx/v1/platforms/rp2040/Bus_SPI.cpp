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
#if defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)

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


    #define spi0_hw ((spi_hw_t *const)SPI0_BASE)

    //_spibase = SPI0_BASE;
    _spibase = SPI1_BASE;
    _spi_reg_dr = &((spi_hw_t *const)_spibase)->dr;
    _spi_reg_sr = &((spi_hw_t *const)_spibase)->sr;


    pinMode(_cfg.pin_dc, pin_mode_t::output);
  //SPI.pins(_cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi, -1);

gpio_set_function(12, gpio_function::GPIO_FUNC_SIO);
gpio_hi(12);
//pinMode(12, pin_mode_t::output);

    return true;
  }

  void Bus_SPI::release(void)
  {
  }

  void Bus_SPI::beginTransaction(void)
  {
    dc_h();
    _spi.format(8, _cfg.spi_mode);
    _spi.frequency(_cfg.freq_write);

    auto cr0_reg = &((spi_hw_t *const)_spibase)->cr0;
//*cr0_reg |= 0x10;
    _cr0 = *cr0_reg & ~0x08;
gpio_set_function(12, gpio_function::GPIO_FUNC_SIO);
  }

  void Bus_SPI::endTransaction(void)
  {
    dc_h();
  }

  void Bus_SPI::beginRead(void)
  {
    _spi.format(8, _cfg.spi_mode);
    _spi.frequency(_cfg.freq_read);
  }

  void Bus_SPI::endRead(void)
  {
    _spi.format(8, _cfg.spi_mode);
    _spi.frequency(_cfg.freq_write);
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
    auto bytes = bit_length >> 3;
    auto dr = _spi_reg_dr;

    if (_cr0)
    {
      auto cr0 = _cr0;
      _cr0 = 0;
      auto cr0_reg = &((spi_hw_t *const)_spibase)->cr0;
      dc_l();
      *cr0_reg = cr0;
    }
    else
    {
      dc_l();
    }
    *dr = data;
    while (--bytes)
    {
      data >>= 8;
      *dr = data;
    }

    return true;
  }

  void Bus_SPI::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    // auto bytes = bit_length >> 3;
    // char dummy[8];
    // _spi.write((const char*)&data, bytes, dummy, bytes);
    auto bytes = bit_length >> 3;
    auto dr = _spi_reg_dr;
    dc_h();

    *dr = data;
    while (--bytes)
    {
      data >>= 8;
      *dr = data;
    }
  }

  void Bus_SPI::writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t length)
  {
    // writeData(data, bit_length);
    // if (! --length) { return; }
    size_t bytes = bit_length >> 3;
    auto dr = _spi_reg_dr;
    auto sr = _spi_reg_sr;

    if (bytes == 2)
    {
      data = __builtin_bswap16(data);
      auto cr0_reg = &((spi_hw_t *const)_spibase)->cr0;
      uint32_t cr0 = *cr0_reg;
      _cr0 = cr0;
      cr0 |= 0x0F;
      dc_h();
      *cr0_reg = cr0;
      do
      {
        while (!(*sr & SPI_SR_TNF)) {}
        *dr = data;
      } while (--length);
      return;
    }
//*/
    auto buf = (uint8_t*)&data;
    dc_h();
    do
    {
      size_t b = 0;
      do
      {
        while (!(*sr & SPI_SR_TNF)) {}
        *dr = buf[b];
      } while (++b != bytes);
    } while (--length);
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, uint32_t length)
  {
    const uint8_t dst_bytes = param->dst_bits >> 3;
    uint8_t buf[12];
    uint32_t limit = 12 / dst_bytes;
    uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      auto buf = _flip_buffer.getBuffer(len * dst_bytes);
      param->fp_copy(buf, 0, len, param);
      writeBytes(buf, len * dst_bytes, true, true);
    } while (length -= len);
  }

  void Bus_SPI::writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma)
  {
    auto dr = _spi_reg_dr;
    auto sr = _spi_reg_sr;
    if (!dc) dc_l();
    else dc_h();
    do
    {
      uint32_t tmp = *data++;
      while (!(*sr & SPI_SR_TNF)) {}
      *dr = tmp;
    } while (--length);
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
