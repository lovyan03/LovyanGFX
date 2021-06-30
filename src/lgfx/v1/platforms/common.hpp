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
#pragma once

#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (CONFIG_IDF_TARGET_ESP32S2) || defined (ESP_PLATFORM)

#include "esp32/common.hpp"

#elif defined ( ESP8266 )

#include "esp8266/common.hpp"

#elif defined (__SAMD21__)

#include "samd21/common.hpp"

#elif defined (__SAMD51__)

#include "samd51/common.hpp"

#elif defined (STM32F2xx) || defined (STM32F4xx) || defined (STM32F7xx)

#include "stm32/common.hpp"

#elif defined ( ARDUINO )

#include "arduino_default/common.hpp"

#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class SimpleBuffer
  {
  public:
    virtual ~SimpleBuffer(void)
    {
      deleteBuffer();
    }

    void deleteBuffer(void)
    {
      _length = 0;
      if (_buffer)
      {
        heap_free(_buffer);
        _buffer = nullptr;
      }
    }

    std::uint8_t* getBuffer(std::size_t length)
    {
      length = (length + 3) & ~3;

      if (_length != length)
      {
        if (_buffer) heap_free(_buffer);
        _buffer = (std::uint8_t*)heap_alloc_dma(length);
        _length = _buffer ? length : 0;
      }
      return _buffer;
    }

  private:
    std::uint8_t* _buffer = nullptr;
    std::size_t _length = 0;
  };

//----------------------------------------------------------------------------

  class FlipBuffer
  {
  public:
    virtual ~FlipBuffer(void)
    {
      deleteBuffer();
    }

    void deleteBuffer(void)
    {
      for (std::size_t i = 0; i < 2; i++)
      {
        _length[i] = 0;
        if (_buffer[i])
        {
          heap_free(_buffer[i]);
          _buffer[i] = nullptr;
        }
      }
    }

    std::uint8_t* getBuffer(std::size_t length)
    {
      length = (length + 3) & ~3;
      _flip = !_flip;

      if (_length[_flip] != length)
      {
        if (_buffer[_flip]) heap_free(_buffer[_flip]);
        _buffer[_flip] = (std::uint8_t*)heap_alloc_dma(length);
        _length[_flip] = _buffer[_flip] ? length : 0;
      }
      return _buffer[_flip];
    }

  private:
    std::uint8_t* _buffer[2] = { nullptr, nullptr };
    std::size_t _length[2] = { 0, 0 };
    bool _flip = false;
  };

//----------------------------------------------------------------------------

  namespace spi
  {
    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi);
    void release(int spi_host);
    void beginTransaction(int spi_host, std::uint32_t freq, int spi_mode = 0);
    void endTransaction(int spi_host);
    void writeBytes(int spi_host, const std::uint8_t* data, std::size_t length);
    void readBytes(int spi_host, std::uint8_t* data, std::size_t length);
  }

  namespace i2c
  {
    static constexpr std::uint32_t I2C_DEFAULT_FREQ = 400000;

    cpp::result<void, error_t> init(int i2c_port, int pin_sda, int pin_scl);
    cpp::result<void, error_t> release(int i2c_port);
    cpp::result<void, error_t> restart(int i2c_port, int i2c_addr, std::uint32_t freq, bool read = false);
    cpp::result<void, error_t> beginTransaction(int i2c_port, int i2c_addr, std::uint32_t freq, bool read = false);
    cpp::result<void, error_t> endTransaction(int i2c_port);
    cpp::result<void, error_t> writeBytes(int i2c_port, const std::uint8_t *data, std::size_t length);
    cpp::result<void, error_t> readBytes(int i2c_port, std::uint8_t *data, std::size_t length);

//--------

    cpp::result<void, error_t> transactionWrite(int i2c_port, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint32_t freq = I2C_DEFAULT_FREQ);
    cpp::result<void, error_t> transactionRead(int i2c_port, int addr, std::uint8_t *readdata, std::uint8_t readlen, std::uint32_t freq = I2C_DEFAULT_FREQ);
    cpp::result<void, error_t> transactionWriteRead(int i2c_port, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint8_t *readdata, std::size_t readlen, std::uint32_t freq = I2C_DEFAULT_FREQ);

    cpp::result<std::uint8_t, error_t> readRegister8(int i2c_port, int addr, std::uint8_t reg, std::uint32_t freq = I2C_DEFAULT_FREQ);
    cpp::result<void, error_t> writeRegister8(int i2c_port, int addr, std::uint8_t reg, std::uint8_t data, std::uint8_t mask = 0, std::uint32_t freq = I2C_DEFAULT_FREQ);

    inline cpp::result<void, error_t> readRegister(int i2c_port, int addr, std::uint8_t reg, std::uint8_t* data, std::size_t len, std::uint32_t freq = I2C_DEFAULT_FREQ)
    {
      return transactionWriteRead(i2c_port, addr, &reg, 1, data, len, freq);
    }
    inline cpp::result<void, error_t> bitOn(int i2c_port, int addr, std::uint8_t reg, std::uint8_t bit, std::uint32_t freq = I2C_DEFAULT_FREQ)
    {
      return writeRegister8(i2c_port, addr, reg, bit, ~0, freq);
    }
    inline cpp::result<void, error_t> bitOff(int i2c_port, int addr, std::uint8_t reg, std::uint8_t bit, std::uint32_t freq = I2C_DEFAULT_FREQ)
    {
      return writeRegister8(i2c_port, addr, reg, 0, ~bit, freq);
    }
  }

//----------------------------------------------------------------------------
 }
}
