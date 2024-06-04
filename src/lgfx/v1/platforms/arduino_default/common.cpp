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
    HardwareSPI *spi;
    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi)  {
      cpp::result<void, error_t> res = {};
      spi = new HardwareSPI(spi_host);
      spi->begin();
      return res; }
    void release(int spi_host) {}
    void beginTransaction(int spi_host, uint32_t freq, int spi_mode) {
      SPISettings setting(freq, MSBFIRST, SPI_MODE0);
      spi->beginTransaction(setting);
    }
    void endTransaction(int spi_host) {spi->endTransaction();}
    void writeBytes(int spi_host, const uint8_t* data, size_t length) {}
    void readBytes(int spi_host, uint8_t* data, size_t length) {spi->transfer(data, length);}  }

//----------------------------------------------------------------------------


  namespace i2c
  {
#ifdef TwoWire_h
    cpp::result<void, error_t> init(int i2c_port, int pin_sda, int pin_scl) { Wire.begin(); return {};}
    cpp::result<void, error_t> release(int i2c_port) { Wire.end(); return {};}
    cpp::result<void, error_t> restart(int i2c_port, int i2c_addr, uint32_t freq, bool read) {  Wire.endTransmission(true); Wire.beginTransmission(i2c_addr); return {};}
    cpp::result<void, error_t> beginTransaction(int i2c_port, int i2c_addr, uint32_t freq, bool read) { Wire.beginTransmission(i2c_addr); return {};}
    cpp::result<void, error_t> endTransaction(int i2c_port) { Wire.endTransmission(true); return {};}
    cpp::result<void, error_t> writeBytes(int i2c_port, const uint8_t *data, size_t length) { Wire.write(data, length); return {};}
    cpp::result<void, error_t> readBytes(int i2c_port, uint8_t *data, size_t length)
      {
        Wire.readBytes((char *)data, (size_t)length);
        /*
        printf("0x");
        for (int i=0; i< length; i++) {
          printf("%02x ", data[i]);
        }
        printf("\n");
        */
        return {};
      }
    cpp::result<void, error_t> readBytes(int i2c_port, uint8_t *data, size_t length, bool last_nack)
    {
      Wire.readBytes((char *)data, (size_t)length);
      /*
      printf("0x");
      for (int i=0; i< length; i++) {
        printf("%02x ", data[i]);
      }
      printf("\n");
      */
      return {};
    }

    cpp::result<void, error_t> transactionWrite(int i2c_port, int addr, const uint8_t *writedata, uint8_t writelen, uint32_t freq)  {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(i2c_port, addr, freq, false)).has_value() && (res = writeBytes(i2c_port, writedata, writelen)).has_value())
      {
        res = endTransaction(i2c_port);
      }
      return res;
     }

    cpp::result<void, error_t> transactionRead(int i2c_port, int addr, uint8_t *readdata, uint8_t readlen, uint32_t freq)  {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(i2c_port, addr, freq, false)).has_value() && (res = readBytes(i2c_port, readdata, readlen)).has_value())
      {
        res = endTransaction(i2c_port);
      }
      return res;
       }

    cpp::result<void, error_t> transactionWriteRead(int i2c_port, int addr, const uint8_t *writedata, uint8_t writelen, uint8_t *readdata, size_t readlen, uint32_t freq)  {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(i2c_port, addr, freq, false)).has_value() && (res = writeBytes(i2c_port, writedata, writelen)).has_value() &&
      (res = restart(i2c_port, addr, freq, false)).has_value() && (res = readBytes(i2c_port, readdata, readlen)).has_value())
      {
        res = endTransaction(i2c_port);
      }
      return res;
       }

    cpp::result<uint8_t, error_t> readRegister8(int i2c_port, int addr, uint8_t reg, uint32_t freq)  {
            auto res = transactionWriteRead(i2c_port, addr, &reg, 1, &reg, 1, freq);
      if (res.has_value())
      {
        return reg;
      }
      return cpp::fail(res.error());
     }

    cpp::result<void, error_t> writeRegister8(int i2c_port, int addr, uint8_t reg, uint8_t data, uint8_t mask, uint32_t freq)  {
            uint8_t tmp[2] = {reg, data};
      if (mask != 0)
      {
        auto res = transactionWriteRead(i2c_port, addr, &reg, 1, &tmp[1], 1, freq);
        if (res.has_error())
        {
          return res;
        }
        tmp[1] = (tmp[1] & mask) | data;
      }
      return transactionWrite(i2c_port, addr, tmp, 2, freq);
     }

     #else
    cpp::result<void, error_t> init(int i2c_port, int pin_sda, int pin_scl) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> release(int i2c_port) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> restart(int i2c_port, int i2c_addr, uint32_t freq, bool read) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> beginTransaction(int i2c_port, int i2c_addr, uint32_t freq, bool read) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> endTransaction(int i2c_port) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> writeBytes(int i2c_port, const uint8_t *data, size_t length) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> readBytes(int i2c_port, uint8_t *data, size_t length) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> readBytes(int i2c_port, uint8_t *data, size_t length, bool last_nack) { return cpp::fail(error_t::unknown_err); }

//--------

    cpp::result<void, error_t> transactionWrite(int i2c_port, int addr, const uint8_t *writedata, uint8_t writelen, uint32_t freq)  { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> transactionRead(int i2c_port, int addr, uint8_t *readdata, uint8_t readlen, uint32_t freq)  { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> transactionWriteRead(int i2c_port, int addr, const uint8_t *writedata, uint8_t writelen, uint8_t *readdata, size_t readlen, uint32_t freq)  { return cpp::fail(error_t::unknown_err); }

    cpp::result<uint8_t, error_t> readRegister8(int i2c_port, int addr, uint8_t reg, uint32_t freq)  { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> writeRegister8(int i2c_port, int addr, uint8_t reg, uint8_t data, uint8_t mask, uint32_t freq)  { return cpp::fail(error_t::unknown_err); }
#endif
  }

//----------------------------------------------------------------------------
 }
}

#endif
