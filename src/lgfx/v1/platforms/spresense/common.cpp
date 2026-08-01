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

#include "common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void pinMode(int_fast16_t pin, pin_mode_t mode)
  {
    switch (mode)
    {
    case pin_mode_t::output:
      ::pinMode(pin, OUTPUT);
      break;

#if defined (INPUT_PULLUP)
    case pin_mode_t::input_pullup:
      ::pinMode(pin, INPUT_PULLUP);
      break;
#endif
#if defined (INPUT_PULLDOWN)
    case pin_mode_t::input_pulldown:
      ::pinMode(pin, INPUT_PULLDOWN);
      break;
#endif
#if defined (INPUT_PULLDOWN_16)
    case pin_mode_t::input_pulldown:
      ::pinMode(pin, INPUT_PULLDOWN_16);
      break;
#endif

    default:
      ::pinMode(pin, INPUT);
      break;
    }
  }

//----------------------------------------------------------------------------

  /// unimplemented.
  namespace spi
  {
// ------------------------------------------------------------------------
// Software SPI ( soft_spi.inl ), reached through the negative host numbers.

#define LGFX_INTERNAL_SOFT_SPI
#include "../soft_spi.inl"

// ------------------------------------------------------------------------

    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi)
    {
      if (spi_host < 0) { return soft_spi_init(spi_host, spi_sclk, spi_miso, spi_mosi); }
      return cpp::fail(error_t::unknown_err);
    }
    void release(int spi_host)
    {
      if (spi_host < 0) { soft_spi_release(spi_host); return; }
    }
    void beginTransaction(int spi_host, uint32_t freq, int spi_mode)
    {
      if (spi_host < 0) { soft_spi_beginTransaction(spi_host, freq, spi_mode); return; }
    }
    void endTransaction(int spi_host)
    {
      if (spi_host < 0) { return; }  // a software host holds nothing to release
    }
    void writeBytes(int spi_host, const uint8_t* data, size_t length)
    {
      if (spi_host < 0) { soft_spi_writeBytes(spi_host, data, length); return; }
    }
    void readBytes(int spi_host, uint8_t* data, size_t length)
    {
      if (spi_host < 0) { soft_spi_readBytes(spi_host, data, length); return; }
    }
  }

//----------------------------------------------------------------------------

  /// unimplemented.
  namespace i2c
  {
// ------------------------------------------------------------------------
// Software I2C ( soft_i2c.inl ), reached through the negative port numbers.

#define LGFX_INTERNAL_SOFT_I2C
#include "../soft_i2c.inl"

// ------------------------------------------------------------------------

    cpp::result<void, error_t> init(int i2c_port, int pin_sda, int pin_scl)
    {
      if (i2c_port < 0)
      {
        auto res = soft_i2c_setPins(i2c_port, pin_sda, pin_scl);
        return res.has_error() ? res : soft_i2c_init(i2c_port);
      }
      return cpp::fail(error_t::unknown_err);
    }
    cpp::result<void, error_t> release(int i2c_port)
    {
      if (i2c_port < 0) { return soft_i2c_release(i2c_port); }
      return cpp::fail(error_t::unknown_err);
    }
    cpp::result<void, error_t> restart(int i2c_port, int i2c_addr, uint32_t freq, bool read)
    {
      if (i2c_port < 0) { return soft_i2c_restart(i2c_port, i2c_addr, freq, read); }
      return cpp::fail(error_t::unknown_err);
    }
    cpp::result<void, error_t> beginTransaction(int i2c_port, int i2c_addr, uint32_t freq, bool read)
    {
      if (i2c_port < 0) { return soft_i2c_beginTransaction(i2c_port, i2c_addr, freq, read); }
      return cpp::fail(error_t::unknown_err);
    }
    cpp::result<void, error_t> endTransaction(int i2c_port)
    {
      if (i2c_port < 0) { return soft_i2c_endTransaction(i2c_port); }
      return cpp::fail(error_t::unknown_err);
    }
    cpp::result<void, error_t> writeBytes(int i2c_port, const uint8_t *data, size_t length)
    {
      if (i2c_port < 0) { return soft_i2c_writeBytes(i2c_port, data, length); }
      return cpp::fail(error_t::unknown_err);
    }
    cpp::result<void, error_t> readBytes(int i2c_port, uint8_t *data, size_t length, bool last_nack)
    {
      if (i2c_port < 0) { return soft_i2c_readBytes(i2c_port, data, length, last_nack); }
      return cpp::fail(error_t::unknown_err);
    }

//--------
// The negative (software) ports reach these through the primitives above, the
// same way a hardware port does; a fail from beginTransaction on a hardware
// port keeps the external behavior unchanged.

    cpp::result<void, error_t> transactionWrite(int i2c_port, int addr, const uint8_t *writedata, uint8_t writelen, uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(i2c_port, addr, freq, false)).has_value())
      {
        res = writeBytes(i2c_port, writedata, writelen);
      }
      auto last = endTransaction(i2c_port);
      return res.has_error() ? res : last;
    }

    cpp::result<void, error_t> transactionRead(int i2c_port, int addr, uint8_t *readdata, uint8_t readlen, uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(i2c_port, addr, freq, true)).has_value())
      {
        res = readBytes(i2c_port, readdata, readlen, true);
      }
      auto last = endTransaction(i2c_port);
      return res.has_error() ? res : last;
    }

    cpp::result<void, error_t> transactionWriteRead(int i2c_port, int addr, const uint8_t *writedata, uint8_t writelen, uint8_t *readdata, size_t readlen, uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(i2c_port, addr, freq, false)).has_value()
       && (res = writeBytes(i2c_port, writedata, writelen)).has_value()
       && (res = restart(i2c_port, addr, freq, true)).has_value()
      )
      {
        res = readBytes(i2c_port, readdata, readlen, true);
      }
      auto last = endTransaction(i2c_port);
      return res.has_error() ? res : last;
    }

    cpp::result<uint8_t, error_t> readRegister8(int i2c_port, int addr, uint8_t reg, uint32_t freq)
    {
      auto res = transactionWriteRead(i2c_port, addr, &reg, 1, &reg, 1, freq);
      if (res.has_value()) { return reg; }
      return cpp::fail( res.error() );
    }

    cpp::result<void, error_t> writeRegister8(int i2c_port, int addr, uint8_t reg, uint8_t data, uint8_t mask, uint32_t freq)
    {
      uint8_t tmp[2] = { reg, data };
      if (mask)
      {
        auto res = transactionWriteRead(i2c_port, addr, &reg, 1, &tmp[1], 1, freq);
        if (res.has_error()) { return res; }
        tmp[1] = (tmp[1] & mask) | data;
      }
      return transactionWrite(i2c_port, addr, tmp, 2, freq);
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
