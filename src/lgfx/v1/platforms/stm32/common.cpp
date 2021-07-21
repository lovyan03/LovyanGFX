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
#if defined (STM32F2xx) || defined (STM32F4xx) || defined (STM32F7xx)

#include "common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void pinMode(int_fast16_t pin, pin_mode_t mode)
  {
    if (pin < 0) return;
/*
    switch (mode)
    {
    case pin_mode_t::output:
      ::pinMode(pin, OUTPUT);
      break;
    
    default:
      ::pinMode(pin, INPUT);
      break;
    }
//*/
    unsigned int pd = 0;

    switch (mode) {
    case pin_mode_t::input: /* INPUT_FLOATING */
      pd = STM_PIN_DATA(STM_MODE_INPUT, GPIO_NOPULL, 0);
      break;
    case pin_mode_t::input_pullup:
      pd = STM_PIN_DATA(STM_MODE_INPUT, GPIO_PULLUP, 0);
      break;
    case pin_mode_t::input_pulldown:
      pd = STM_PIN_DATA(STM_MODE_INPUT, GPIO_PULLDOWN, 0);
      break;
    case pin_mode_t::output:
      pd = STM_PIN_DATA(STM_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
      break;
    }

    pin_function((PinName)pin, pd);
  }

//----------------------------------------------------------------------------

  namespace spi // TODO: implement.
  {
    //void init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi)
    void init(int, int, int, int) {}

    //void release(int spi_host) {}
    void release(int) {}

    //void beginTransaction(int spi_host, int spi_cs, int freq, int spi_mode)
    void beginTransaction(int, int, int, int) {}

    //void beginTransaction(int spi_host)
    void beginTransaction(int) {}

    //void endTransaction(int spi_host, int spi_cs)
    void endTransaction(int, int) {}

    //void writeData(int spi_host, const uint8_t* data, uint32_t len)
    void writeData(int, const uint8_t*, uint32_t) {}

    //void readData(int spi_host, uint8_t* data, uint32_t len)
    void readData(int, uint8_t*, uint32_t) {}
  }

//----------------------------------------------------------------------------

  namespace i2c // TODO: implement.
  {
    //void init(int i2c_port, int sda, int scl, int freq) { }
    void init(int, int, int, int) {}

    //bool writeBytes(int i2c_port, uint16_t addr, const uint8_t *data, uint8_t len)
    bool writeBytes(int, uint16_t, const uint8_t*, uint8_t)
    {
      return false;
    }

    //bool writeReadBytes(int i2c_port, uint16_t addr, const uint8_t *writedata, uint8_t writelen, uint8_t *readdata, uint8_t readlen)
    bool writeReadBytes(int, uint16_t, const uint8_t*, uint8_t, uint8_t*, uint8_t)
    {
      return false;
    }

    //bool readRegister(int i2c_port, uint16_t addr, uint8_t reg, uint8_t *data, uint8_t len)
    bool readRegister(int, uint16_t, uint8_t, uint8_t*, uint8_t)
    {
      return false;
    }

    bool writeRegister8(int i2c_port, uint16_t addr, uint8_t reg, uint8_t data, uint8_t mask)
    {
      uint8_t tmp[2] = { reg, data };
      if (mask) {
        if (!readRegister(i2c_port, addr, reg, &tmp[1], 1)) return false;
        tmp[1] = (tmp[1] & mask) | data;
      }
      return writeBytes(i2c_port, addr, tmp, 2);
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
