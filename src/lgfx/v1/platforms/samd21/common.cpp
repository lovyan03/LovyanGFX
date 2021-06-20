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

#include "common.hpp"

#include <Arduino.h>
#include <Wire.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

  void pinMode(std::int_fast16_t pin, pin_mode_t mode)
  {
    if (pin > 255) return;
    switch (mode)
    {
    case pin_mode_t::input:          ::pinMode(pin, INPUT);          break;
    case pin_mode_t::input_pulldown: ::pinMode(pin, INPUT_PULLDOWN); break;
    case pin_mode_t::input_pullup:   ::pinMode(pin, INPUT_PULLUP);   break;
    case pin_mode_t::output:         ::pinMode(pin, OUTPUT);         break;
    }
  }

//----------------------------------------------------------------------------

  namespace spi // TODO: implement.
  {
    cpp::result<void, error_t> init(int sercom_index, int pin_sclk, int pin_miso, int pin_mosi)
    {
      return {};
    }

    //void release(int spi_host) {}
    void release(int) {}

    //void beginTransaction(int spi_host, int spi_cs, int freq, int spi_mode)
    void beginTransaction(int, int, int, int) {}

    //void beginTransaction(int spi_host)
    void beginTransaction(int) {}

    //void endTransaction(int spi_host, int spi_cs)
    void endTransaction(int, int) {}

    //void writeData(int spi_host, const std::uint8_t* data, std::uint32_t len)
    void writeBytes(int, const std::uint8_t*, std::uint32_t) {}

    //void readData(int spi_host, std::uint8_t* data, std::uint32_t len)
    void readBytes(int, std::uint8_t*, std::uint32_t) {}
  }

//----------------------------------------------------------------------------

  namespace i2c // TODO: implement.
  {
    cpp::result<void, error_t> init(int sercom_index, int pin_sda, int pin_scl)
    {
      Wire.begin();
      return {};
    }

    cpp::result<void, error_t> release(int sercom_index)
    {
      Wire.end();
      return {};
    }

    cpp::result<void, error_t> restart(int sercom_index, int i2c_addr, std::uint32_t freq, bool read)
    {
      return {};
    }

    cpp::result<void, error_t> beginTransaction(int sercom_index, int i2c_addr, std::uint32_t freq, bool read)
    {
      if (!read)
      {
        Wire.beginTransmission(i2c_addr);
      }
      Wire.setClock(freq);
      return {};
    }

    cpp::result<void, error_t> endTransaction(int sercom_index)
    {
      Wire.endTransmission();
      return {};
    }

    cpp::result<void, error_t> writeBytes(int sercom_index, const std::uint8_t *data, std::size_t length)
    {
      Wire.write(data, length);
      return {};
    }

    cpp::result<void, error_t> readBytes(int sercom_index, std::uint8_t *data, std::size_t length)
    {
      return {};
    }

    cpp::result<void, error_t> transactionWrite(int sercom_index, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(sercom_index, addr, freq, false)).has_value()
       && (res = writeBytes(sercom_index, writedata, writelen)).has_value()
      )
      {
        res = endTransaction(sercom_index);
      }
      return res;
    }

    cpp::result<void, error_t> transactionRead(int sercom_index, int addr, std::uint8_t *readdata, std::uint8_t readlen, std::uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(sercom_index, addr, freq, true)).has_value()
       && (res = readBytes(sercom_index, readdata, readlen)).has_value()
      )
      {
        res = endTransaction(sercom_index);
      }
      return res;
    }

    cpp::result<void, error_t> transactionWriteRead(int sercom_index, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint8_t *readdata, std::size_t readlen, std::uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(sercom_index, addr, freq, false)).has_value()
       && (res = writeBytes(sercom_index, writedata, writelen)).has_value()
       && (res = restart(sercom_index, addr, freq, true)).has_value()
       && (res = readBytes(sercom_index, readdata, readlen)).has_value()
      )
      {
        res = endTransaction(sercom_index);
      }
      return res;
    }

    cpp::result<std::uint8_t, error_t> registerRead8(int sercom_index, int addr, std::uint8_t reg, std::uint32_t freq)
    {
      auto res = transactionWriteRead(sercom_index, addr, &reg, 1, &reg, 1, freq);
      if (res.has_value()) { return reg; }
      return cpp::fail( res.error() );
    }

    cpp::result<void, error_t> registerWrite8(int sercom_index, int addr, std::uint8_t reg, std::uint8_t data, std::uint8_t mask, std::uint32_t freq)
    {
      std::uint8_t tmp[2] = { reg, data };
      if (mask)
      {
        auto res = transactionWriteRead(sercom_index, addr, &reg, 1, &tmp[1], 1, freq);
        if (res.has_error()) { return res; }
        tmp[1] = (tmp[1] & mask) | data;
      }
      return transactionWrite(sercom_index, addr, tmp, 2, freq);
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
