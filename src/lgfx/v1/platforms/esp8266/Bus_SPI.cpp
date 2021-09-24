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
#if defined ( ESP8266 )

#include "esp8266_peri.h"

#include "Bus_SPI.hpp"

#include "../../misc/pixelcopy.hpp"

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
    pinMode(_cfg.pin_dc, pin_mode_t::output);
    dc_control(true);
    return spi::init(0, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi).has_value();
  }

  void Bus_SPI::release(void)
  {
    spi::release(1);
  }

  void Bus_SPI::beginTransaction(void)
  {
    spi::beginTransaction(0, _cfg.freq_write, _cfg.spi_mode);
  }

  void Bus_SPI::endTransaction(void)
  {
    spi::endTransaction(0);
    dc_control(true);
  }

  void Bus_SPI::beginRead(void)
  {
    uint32_t user = (SPI1U & ~(SPIUMOSI | SPIUSIO)) | SPIUMISO | (_cfg.spi_3wire ? SPIUSIO : 0);

    dc_control(true);
    spi::endTransaction(0);
    spi::beginTransaction(0, _cfg.freq_read, _cfg.spi_mode);
    SPI1U = user;
  }

  void Bus_SPI::endRead(void)
  {
    spi::endTransaction(0);
    spi::beginTransaction(0, _cfg.freq_write, _cfg.spi_mode);
    SPI1U = (SPI1U & ~(SPIUMISO | SPIUSIO)) | SPIUMOSI;
  }

  void Bus_SPI::wait(void)
  {
    wait_spi();
  }

  bool Bus_SPI::busy(void) const
  {
    return SPI1CMD & SPIBUSY;
  }

  bool Bus_SPI::writeCommand(uint32_t data, uint_fast8_t bit_length)
  {
    const uint32_t u1 = --bit_length << SPILMOSI;
    dc_control(false);
    SPI1W0 = data;
    SPI1U1 = u1;
    SPI1CMD = SPIBUSY;
    return true;
  }

  void Bus_SPI::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    const uint32_t u1 = --bit_length << SPILMOSI;
    dc_control(true);
    SPI1W0 = data;
    SPI1U1 = u1;
    SPI1CMD = SPIBUSY;
  }

  void Bus_SPI::writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t count)
  {
    uint32_t regbuf0 = data | data << bit_length;
    uint32_t regbuf1;
    uint32_t regbuf2;
    // make 12Bytes data.
    bool bits24 = (bit_length == 24);
    if (bits24) {
      regbuf1 = regbuf0 >> 8 | regbuf0 << 16;
      regbuf2 = regbuf0 >>16 | regbuf0 <<  8;
    } else {
      regbuf1 = regbuf0;
      regbuf2 = regbuf0;
    }

    uint32_t length = bit_length * count;          // convert to bitlength.
    uint32_t len = (length <= 96u) ? length : (length <= 144u) ? 48u : 96u; // 1st send length = max 12Byte (96bit).

    length -= len;
    --len;

    uint32_t u1 = len << SPILMOSI;

    dc_control(true);

    SPI1U1 = u1;
    SPI1W0 = regbuf0;
    SPI1W1 = regbuf1;
    SPI1W2 = regbuf2;
    SPI1CMD = SPIBUSY;
    if (0 == length) return;

    uint32_t regbuf[7] = { regbuf0, regbuf1, regbuf2, regbuf0, regbuf1, regbuf2, regbuf0 } ;
    memcpy((void*)&SPI1W3, regbuf, 24);
    memcpy((void*)&SPI1W9, regbuf, 28);
    uint32_t limit;
    if (bits24)
    {
      limit = 504;
      len = length % limit;
    }
    else
    {
      limit = 512;
      len = length & (limit - 1);
    }

    if (len)
    {
      length -= len;
      --len;
      u1 = len << SPILMOSI;
      wait_spi();
      SPI1U1 = u1;
      SPI1CMD = SPIBUSY;
      if (0 == length) return;
    }

    u1 = (limit-1) << SPILMOSI;
    while (SPI1CMD & SPIBUSY);
    SPI1CMD = SPIBUSY;
    SPI1U1 = u1;
    while (length -= limit)
    {
      while (SPI1CMD & SPIBUSY);
      SPI1CMD = SPIBUSY;
    }
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, uint32_t length)
  {
    const uint8_t bytes = param->dst_bits >> 3;
    const uint32_t limit = (bytes == 2) ? 32 : 20; //  limit = 64/bytes (4Byte aligned) (bytes==2 is 32   bytes==3 is 20)
    uint32_t len = (length - 1) / limit;
    len = length - (len * limit);
    uint32_t regbuf[16];
    param->fp_copy(regbuf, 0, len, param);

    auto spi_w0_reg = (void*)reinterpret_cast<volatile uint32_t*>(&SPI1W0);

    dc_control(true);
    set_write_len(len * bytes << 3);

    memcpy(spi_w0_reg, regbuf, (len * bytes + 3) & (~3));
    exec_spi();
    if (0 == (length -= len)) return;

    if (len != limit)
    {
      param->fp_copy(regbuf, 0, limit, param);
      wait_spi();
      memcpy(spi_w0_reg, regbuf, limit * bytes);
      set_write_len(limit * bytes << 3);
      exec_spi();
      if (0 == (length -= limit)) return;
    }

    for (; length; length -= limit)
    {
      param->fp_copy(regbuf, 0, limit, param);
      wait_spi();
      memcpy(spi_w0_reg, regbuf, limit * bytes);
      exec_spi();
    }
  }

  void Bus_SPI::writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma)
  {
    if (length <= 64)
    {
      auto aligned_len = (length + 3) & (~3);
      length <<= 3;
      dc_control(dc);
      set_write_len(length);
      memcpy((void*)&SPI1W0, data, aligned_len);
      exec_spi();
      return;
    }

    static constexpr uint32_t limit = 32;
    uint32_t len = ((length - 1) & 0x1F) + 1;
    uint32_t highpart = ((length - 1) & limit) ? 8 : 0;

    auto spi_w0_reg = reinterpret_cast<volatile uint32_t*>(&SPI1W0);

    uint32_t user_reg_0 = SPI1U;
    uint32_t user_reg_1 = user_reg_0 | SPIUMOSIH;
    dc_control(dc);
    set_write_len(len << 3);
    memcpy((void*)&spi_w0_reg[highpart], data, (len + 3) & (~3));
    if (highpart) SPI1U = user_reg_1;
    exec_spi();
    length -= len;

    if (len != limit)
    {
      data += len;
      memcpy((void*)&spi_w0_reg[highpart ^= 0x08], data, limit);
      uint32_t user = highpart ? user_reg_1 : user_reg_0;
      wait_spi();
      set_write_len(limit << 3);
      SPI1U = user;
      exec_spi();
      if (0 == (length -= limit)) return;
    }

    length >>= 5;
    do
    {
      data += limit;
      memcpy((void*)&spi_w0_reg[highpart ^= 0x08], data, limit);
      uint32_t user = highpart ? user_reg_1 : user_reg_0;
      wait_spi();
      SPI1U = user;
      exec_spi();
    } while (--length);
  }

  uint32_t Bus_SPI::readData(uint_fast8_t bit_length)
  {
    set_read_len(bit_length);
    exec_spi();
    wait_spi();
    return SPI1W0;
  }

  bool Bus_SPI::readBytes(uint8_t* dst, uint32_t length, bool use_dma)
  {
    uint32_t len1 = std::min(length, 32u);  // 32 Byte read.
    uint32_t len2 = len1;
    wait_spi();
    set_read_len(len1 << 3);
    exec_spi();
    uint32_t highpart = 8;
    uint32_t userreg = SPI1U;
    auto spi_w0_reg = reinterpret_cast<volatile uint32_t*>(&SPI1W0);
    do {
      if (0 == (length -= len1)) {
        len2 = len1;
        wait_spi();
        SPI1U = userreg;
      } else {
        uint32_t user = userreg;
        if (highpart) user = userreg | SPIUMISOH;
        if (length < len1) {
          len1 = length;
          wait_spi();
          set_read_len(len1 << 3);
        } else {
          wait_spi();
        }
        SPI1U = user;
        exec_spi();
      }
      memcpy(dst, (void*)&spi_w0_reg[highpart ^= 8], len2);

      dst += len2;
    } while (length);

    return true;
  }

  void Bus_SPI::readPixels(void* dst, pixelcopy_t* param, uint32_t length)
  {
    uint32_t bytes = param->src_bits >> 3;
    uint32_t dstindex = 0;
    uint32_t len = 4;
    uint32_t buf[6];
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
