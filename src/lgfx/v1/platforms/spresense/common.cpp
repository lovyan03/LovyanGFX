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
    cpp::result<void, error_t> init(int , int , int , int )  { return cpp::fail(error_t::unknown_err); }
    void release(int ) {}
    void beginTransaction(int , uint32_t , int ) {}
    void endTransaction(int ) {}
    void writeBytes(int , const uint8_t* , size_t ) {}
    void readBytes(int , uint8_t* , size_t ) {}
  }

//----------------------------------------------------------------------------

  /// unimplemented.
  namespace i2c
  {
    cpp::result<void, error_t> init(int , int , int ) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> release(int ) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> restart(int , int , uint32_t , bool ) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> beginTransaction(int , int , uint32_t , bool ) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> endTransaction(int ) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> writeBytes(int , const uint8_t *, size_t ) { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> readBytes(int , uint8_t *, size_t ) { return cpp::fail(error_t::unknown_err); }

//--------

    cpp::result<void, error_t> transactionWrite(int , int , const uint8_t *, uint8_t , uint32_t )  { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> transactionRead(int , int , uint8_t *, uint8_t , uint32_t )  { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> transactionWriteRead(int , int , const uint8_t *, uint8_t , uint8_t *, size_t , uint32_t )  { return cpp::fail(error_t::unknown_err); }

    cpp::result<uint8_t, error_t> readRegister8(int , int , uint8_t , uint32_t )  { return cpp::fail(error_t::unknown_err); }
    cpp::result<void, error_t> writeRegister8(int , int , uint8_t , uint8_t , uint8_t , uint32_t )  { return cpp::fail(error_t::unknown_err); }
  }

//----------------------------------------------------------------------------
 }
}

#endif
