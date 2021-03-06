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
#if defined (__SAMD51__)

#include "common.hpp"

#undef PORT_PINCFG_PULLEN
#undef PORT_PINCFG_PULLEN_Pos
#undef PORT_PINCFG_INEN
#undef PORT_PINCFG_INEN_Pos
#undef _Ul

#define _Ul(n) (static_cast<std::uint32_t>((n)))
#define PORT_PINCFG_INEN_Pos        1            /**< \brief (PORT_PINCFG) Input Enable */
#define PORT_PINCFG_INEN            (_Ul(0x1) << PORT_PINCFG_INEN_Pos)
#define PORT_PINCFG_PULLEN_Pos      2            /**< \brief (PORT_PINCFG) Pull Enable */
#define PORT_PINCFG_PULLEN          (_Ul(0x1) << PORT_PINCFG_PULLEN_Pos)

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void pinMode(std::int_fast16_t pin, pin_mode_t mode)
  {
    std::uint32_t port = pin >> samd51::PORT_SHIFT;
    pin &= 0xFF;
    std::uint32_t pinMask = (1ul << pin);

    // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
    switch ( mode )
    {
      case pin_mode_t::input:
        // Set pin to input mode
        PORT->Group[port].PINCFG[pin].reg=(std::uint8_t)(PORT_PINCFG_INEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;
      break ;

      case pin_mode_t::input_pullup:
        // Set pin to input mode with pull-up resistor enabled
        PORT->Group[port].PINCFG[pin].reg=(std::uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;

        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.7 Data Output Value Set')
        PORT->Group[port].OUTSET.reg = pinMask ;
      break ;

      case pin_mode_t::input_pulldown:
        // Set pin to input mode with pull-down resistor enabled
        PORT->Group[port].PINCFG[pin].reg=(std::uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;

        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.6 Data Output Value Clear')
        PORT->Group[port].OUTCLR.reg = pinMask ;
      break ;

      case pin_mode_t::output:
        // enable input, to support reading back values, with pullups disabled
        PORT->Group[port].PINCFG[pin].reg=(std::uint8_t)(PORT_PINCFG_INEN) ;

        // Set pin to output mode
        PORT->Group[port].DIRSET.reg = pinMask ;
      break ;

      default:
        // do nothing
      break ;
    }
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

    //void writeData(int spi_host, const std::uint8_t* data, std::uint32_t len)
    void writeData(int, const std::uint8_t*, std::uint32_t) {}

    //void readData(int spi_host, std::uint8_t* data, std::uint32_t len)
    void readData(int, std::uint8_t*, std::uint32_t) {}
  }

//----------------------------------------------------------------------------

  namespace i2c // TODO: implement.
  {
    //void init(int i2c_port, int sda, int scl, int freq) { }
    void init(int, int, int, int) {}

    //bool writeBytes(int i2c_port, std::uint16_t addr, const std::uint8_t *data, std::uint8_t len)
    bool writeBytes(int, std::uint16_t, const std::uint8_t*, std::uint8_t)
    {
      return false;
    }

    //bool writeReadBytes(int i2c_port, std::uint16_t addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint8_t *readdata, std::uint8_t readlen)
    bool writeReadBytes(int, std::uint16_t, const std::uint8_t*, std::uint8_t, std::uint8_t*, std::uint8_t)
    {
      return false;
    }

    //bool readRegister(int i2c_port, std::uint16_t addr, std::uint8_t reg, std::uint8_t *data, uint8_t len)
    bool readRegister(int, std::uint16_t, std::uint8_t, std::uint8_t*, uint8_t)
    {
      return false;
    }

    bool writeRegister8(int i2c_port, std::uint16_t addr, std::uint8_t reg, std::uint8_t data, std::uint8_t mask)
    {
      std::uint8_t tmp[2] = { reg, data };
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
