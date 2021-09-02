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

#include "Bus_SPI.hpp"

#include "../../misc/pixelcopy.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  uint32_t Bus_SPI::FreqToClockDiv(uint32_t freq)
  {
    uint32_t div = std::min<uint32_t>(255, _cfg.sercom_clkfreq / (1+(freq<<1)));
    return div;
  }

  void Bus_SPI::setFreqDiv(uint32_t div)
  {
    auto *spi = &_sercom->SPI;
    while (spi->SYNCBUSY.reg);
    spi->CTRLA.bit.ENABLE = 0;
    while (spi->SYNCBUSY.reg);
    spi->BAUD.reg = div;
    spi->CTRLA.bit.ENABLE = 1;
    _need_wait = false;
    while (spi->SYNCBUSY.reg);
  }

  void Bus_SPI::config(const config_t& config)
  {
    _cfg = config;

    _need_wait = false;
    _sercom = reinterpret_cast<Sercom*>(samd21::getSercomData(_cfg.sercom_index)->sercomPtr);
    _last_apb_freq = -1;
    _mask_reg_dc = 0;
    uint32_t port = 0;
    if (_cfg.pin_dc >= 0)
    {
      _mask_reg_dc = (1ul << (_cfg.pin_dc & (samd21::PIN_MASK)));
      port = _cfg.pin_dc >> samd21::PORT_SHIFT;
    }

    _gpio_reg_dc_h = &PORT->Group[port].OUTSET.reg;
    _gpio_reg_dc_l = &PORT->Group[port].OUTCLR.reg;

    _clkdiv_read  = FreqToClockDiv(_cfg.freq_read);
    _clkdiv_write = FreqToClockDiv(_cfg.freq_write);
  }

  bool Bus_SPI::init(void)
  {
    dc_control(true);
    pinMode(_cfg.pin_dc, pin_mode_t::output);

    if (lgfx::spi::init(_cfg.sercom_index, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi).has_error())
    {
      return false;
    }

    return true;
  }

  void Bus_SPI::release(void)
  {
    lgfx::spi::release(_cfg.sercom_index);
  }

  void Bus_SPI::beginTransaction(void)
  {
    dc_control(true);
    spi::beginTransaction(_cfg.sercom_index, _cfg.freq_write, _cfg.spi_mode);
    _need_wait = false;
  }

  void Bus_SPI::endTransaction(void)
  {
    dc_control(true);
    spi::endTransaction(_cfg.sercom_index);
  }

  void Bus_SPI::beginRead(void)
  {
    wait_spi();
    _need_wait = false;
    spi::endTransaction(_cfg.sercom_index);
    spi::beginTransaction(_cfg.sercom_index, _cfg.freq_read, _cfg.spi_mode);
  }

  void Bus_SPI::endRead(void)
  {
    spi::endTransaction(_cfg.sercom_index);
    spi::beginTransaction(_cfg.sercom_index, _cfg.freq_write, _cfg.spi_mode);
  }

  void Bus_SPI::wait(void)
  {
    wait_spi();
  }

  bool Bus_SPI::busy(void) const
  {
    return _need_wait && (_sercom->SPI.INTFLAG.bit.TXC == 0);
  }

  bool Bus_SPI::writeCommand(uint32_t data, uint_fast8_t bit_length)
  {
    bit_length >>= 3;
    auto *spi = &_sercom->SPI;

    auto gpio_reg_dc = _gpio_reg_dc_l;
    auto mask_reg_dc = _mask_reg_dc;
    wait_spi();
    *gpio_reg_dc = mask_reg_dc;
    spi->DATA.reg = data;
    _need_wait = true;
    while (--bit_length)
    {
      data >>= 8;
      while (spi->INTFLAG.bit.DRE == 0);
      spi->DATA.reg = data;
    }
    return true;
  }

  void Bus_SPI::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    bit_length >>= 3;
    auto *spi = &_sercom->SPI;

    auto gpio_reg_dc = _gpio_reg_dc_h;
    auto mask_reg_dc = _mask_reg_dc;
    wait_spi();
    *gpio_reg_dc = mask_reg_dc;
    spi->DATA.reg = data;
    _need_wait = true;
    while (--bit_length)
    {
      data >>= 8;
      while (spi->INTFLAG.bit.DRE == 0);
      spi->DATA.reg = data;
    }
  }

  void Bus_SPI::writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t length)
  {
    size_t bits = bit_length & ~7;
    auto *spi = &_sercom->SPI;
    dc_control(true);
    _need_wait = true;
    size_t i = 0;
    for (;;)
    {
      size_t tmp = data >> i;
      i += 8;
      while (spi->INTFLAG.bit.DRE == 0);
      spi->DATA.reg = tmp;
      if (i != bits) continue;
      i = 0;
      if (0 == --length) return;
    }
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, uint32_t length)
  {
    const uint8_t bytes = param->dst_bits >> 3;
    auto *spi = &_sercom->SPI;
    uint32_t data;
    param->fp_copy((uint8_t*)&data, 0, 1, param);
    dc_control(true);
    spi->DATA.reg = data;
    _need_wait = true;
    uint32_t i = 0;
    while (++i != bytes)
    {
      while (spi->INTFLAG.bit.DRE == 0);
      spi->DATA.reg = data >> (i << 3);
    }
    if (--length)
    {
      do
      {
        param->fp_copy((uint8_t*)&data, 0, 1, param);
        i = 0;
        do
        {
          uint32_t tmp = data >> (i << 3);
          while (spi->INTFLAG.bit.DRE == 0);
          spi->DATA.reg = tmp;
        } while (++i != bytes);
      } while (--length);
    }
  }

  void Bus_SPI::writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma)
  {
    auto *spi = &_sercom->SPI;
    dc_control(dc);
    spi->DATA.reg = *data;
    _need_wait = true;
    while (--length)
    {
      uint32_t tmp = *++data;
      while (spi->INTFLAG.bit.DRE == 0);
      spi->DATA.reg = tmp;
    }
  }

  uint32_t Bus_SPI::readData(uint_fast8_t bit_length)
  {
    auto *spi = &_sercom->SPI;
    uint32_t res = 0;
    bit_length >>= 3;
    if (!bit_length) return res;
    int idx = 0;
    do
    {
      spi->DATA.reg = 0;
      while (spi->INTFLAG.bit.RXC == 0);
      res |= (spi->DATA.reg & 0xFF) << idx;
      idx += 8;
    } while (--bit_length);
    return res;
  }

  bool Bus_SPI::readBytes(uint8_t* dst, uint32_t length, bool use_dma)
  {
    auto *spi = &_sercom->SPI;
    spi->DATA.reg = 0;
    while (--length)
    {
      while (spi->INTFLAG.bit.RXC == 0);
      uint_fast8_t tmp = spi->DATA.reg;
      spi->DATA.reg = 0;
      *dst++ = tmp;
    }
    while (spi->INTFLAG.bit.RXC == 0);
    dst[0] = spi->DATA.reg;
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
