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
#if defined (ARDUINO_ARCH_SPRESENSE)

#include "Bus_SPI.hpp"
#include "../../misc/pixelcopy.hpp"

#include <sdk/config.h>
#include <nuttx/config.h>
#include <nuttx/spi/spi.h>
#include <Arduino.h>
#include <SPI.h>

#include <chip/hardware/cxd5602_memorymap.h>
#include <chip/hardware/cxd5602_topreg.h>

#include <wiring_private.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static uintptr_t get_gpio_regaddr(uint32_t pin)
  {
    pin = pin_convert(pin);
    uint32_t base = (pin < PIN_IS_CLK) ? 1 : 7;
    return CXD56_TOPREG_GP_I2C4_BCK + ((pin - base) * 4);
  }

//----------------------------------------------------------------------------

  void Bus_SPI::config(const config_t& config)
  {
    _cfg = config;

    switch (_cfg.spi_port)
    {
    case 3:  _spibase = CXD56_SCU_SPI_BASE;  break;
    case 4:  _spibase = CXD56_IMG_SPI_BASE;  break;
    case 5:  _spibase = CXD56_IMG_WSPI_BASE; break;
    default: _spibase = CXD56_SPIM_BASE;     break; // SPI 0
    }
    _spi_reg_sr = (volatile uint32_t*)(_spibase + CXD56_SPI_SR_OFFSET);
    _spi_reg_dr = (volatile uint32_t*)(_spibase + CXD56_SPI_DR_OFFSET);

    _gpio_reg_dc = nullptr;
    if (_cfg.pin_dc >= 0)
    {
      _gpio_reg_dc = get_gpio_regaddr(_cfg.pin_dc);
      pinMode(_cfg.pin_dc, pin_mode_t::output);
      gpio_hi(_cfg.pin_dc);
    }
  }

  bool Bus_SPI::init(void)
  {
    dc_h();
    pinMode(_cfg.pin_dc, pin_mode_t::output);
    SPI.begin();

    _spi_dev = cxd56_spibus_initialize(_cfg.spi_port);

    return true;
  }

  void Bus_SPI::release(void)
  {
    SPI.end();
  }

  void Bus_SPI::beginTransaction(void)
  {
    dc_h();
    SPISettings setting(_cfg.freq_write, MSBFIRST, _cfg.spi_mode);
    SPI.beginTransaction(setting);

    _spi_dev->ops->setbits(_spi_dev, 8);
  }

  void Bus_SPI::endTransaction(void)
  {
    auto sr = _spi_reg_sr;
    auto dr = _spi_reg_dr;
    while (*sr & (SPI_SR_BSY | SPI_SR_RNE)) { volatile uint32_t dummy = *dr; };
    dc_h();
    SPI.endTransaction();
  }

  void Bus_SPI::beginRead(void)
  {
    SPI.endTransaction();
    SPISettings setting(_cfg.freq_read, MSBFIRST, _cfg.spi_mode);
    SPI.beginTransaction(setting);

    _spi_dev->ops->setbits(_spi_dev, 8);
  }

  void Bus_SPI::endRead(void)
  {
    SPI.endTransaction();
    SPISettings setting(_cfg.freq_write, MSBFIRST, _cfg.spi_mode);
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
    auto bytes = bit_length >> 3;
    dc_l();
    //SPI.send((uint8_t*)&data, bit_length >> 3);
    //SPI_EXCHANGE(_spi_dev, (uint8_t*)&data, nullptr, bit_length >> 3);
    //_spi_dev->ops->exchange(_spi_dev, (uint8_t*)&data, nullptr, bit_length >> 3);

//while ((spi_getreg(priv, CXD56_SPI_SR_OFFSET) & SPI_SR_TNF)
    //while (!(*_spi_reg_sr & SPI_SR_TNF)) {} // 送信完了待ち
    *_spi_reg_dr = data;
    while (--bytes)
    {
      data >>= 8;
      *_spi_reg_dr = data;
    }

    return true;
  }

  void Bus_SPI::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    auto bytes = bit_length >> 3;
    dc_h();
    //SPI.send((uint8_t*)&data, bit_length >> 3);
    //_spi_dev->ops->exchange(_spi_dev, (uint8_t*)&data, nullptr, bit_length >> 3);
    //while (!(*_spi_reg_sr & SPI_SR_TNF)) {} // 送信完了待ち
    *_spi_reg_dr = data;
    while (--bytes)
    {
      data >>= 8;
      *_spi_reg_dr = data;
    }
  }

  void Bus_SPI::writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t length)
  {
    auto bytes = bit_length >> 3;
    auto buf = (uint8_t*)&data;
    auto dr = _spi_reg_dr;
    auto sr = _spi_reg_sr;

    if (bytes == 2)
    {
      data = __builtin_bswap16(data);
      auto cr0_reg = (volatile uint32_t*)(_spibase + CXD56_SPI_CR0_OFFSET);
      uint32_t cr0 = *cr0_reg;
      cr0 |= 0x0F;  // 16bit len setting
      dc_h();
      *cr0_reg = cr0;
      do
      {
        while (!(*sr & SPI_SR_TNF)) {}
        *dr = data;
      } while (--length);
      cr0 &= ~0x08; // 8bit len setting
      while (*sr & SPI_SR_BSY) {}
      *cr0_reg = cr0;
      return;
    }

    dc_h();
    do
    {
      for (size_t b = 0; b < bytes; ++b)
      {
        while (!(*sr & SPI_SR_TNF)) {}
        *dr = buf[b];
      }
/*
      writeData(data, bit_length);
//*/
      //SPI.send(reinterpret_cast<uint8_t*>(&data), bytes);
    } while (--length);
/*/
    dc_h();
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

      SPI.send(buf, len * dst_bytes);
    } while (length -= len);
//*/
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, uint32_t length)
  {
    auto dr = _spi_reg_dr;
    auto sr = _spi_reg_sr;
    const uint8_t dst_bytes = param->dst_bits >> 3;
    uint8_t buf[18];
    uint32_t limit = 18 / dst_bytes;

auto cr0_reg = (volatile uint32_t*)(_spibase + CXD56_SPI_CR0_OFFSET);
uint32_t cr0 = *cr0_reg;
bool en16 = false;

    dc_h();
    uint32_t len;
    do
    {
//    len = ((length - 1) % limit) + 1;
      len = std::min(length, limit);
      param->fp_copy(buf, 0, len, param);
      auto b = buf;
      auto l = len * dst_bytes;
      if ((l & 1) == en16)
      {
        en16 = !en16;
        while (!(*sr & SPI_SR_TFE)) {}
        *cr0_reg = cr0 | (en16 ? 8 : 0);
      }
      if (en16)
      {
        do
        {
          while (!(*sr & SPI_SR_TNF)) {}
          *dr = b[0]<<8 | b[1];
          b += 2;
        } while (l -= 2);
      }
      else
      {
        do
        {
          while (!(*sr & SPI_SR_TNF)) {}
          *dr = *b++;
        } while (--l);
      }
    } while (length -= len);

    if (en16)
    {
      while (!(*sr & SPI_SR_TFE)) {}
      *cr0_reg = cr0;
    }
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
/*/
    SPI.send(const_cast<uint8_t*>(data), length);
//*/
/*
    auto reg_dma = (volatile uint32_t*)(_spibase + CXD56_SPI_DMACR_OFFSET);
    *reg_dma |= SPI_DMACR_TXDMAE;


    uint32_t dst = (_spibase + CXD56_SPI_DR_OFFSET) & 0x03ffffffu;
    cxd56_txdmasetup(priv->txdmach, (uintptr_t)dst, (uintptr_t)txbuffer,
                     nwords, priv->txconfig);
//*/
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
