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
#include "Touch_RA8875.hpp"

#include "../../internal/algorithm.h"
#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Touch_RA8875::init(void)
  {
    _inited = false;

    lgfx::gpio_hi(_cfg.pin_cs);
    lgfx::pinMode(_cfg.pin_cs, lgfx::pin_mode_t::output);
    if (!lgfx::spi::init(_cfg.spi_host, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi).has_value()) return false;

    // enable Touch Panel
    // RA8875_TPCR0: RA8875_TPCR0_ENABLE | RA8875_TPCR0_WAIT_4096CLK | RA8875_TPCR0_WAKEENABLE | adcClk
    writeRegister8(0x70, 0x80 | 0x30 | 0x08 | 0x04); // 10mhz max!

    // Set Auto Mode
    // RA8875_TPCR1, RA8875_TPCR1_AUTO | RA8875_TPCR1_DEBOUNCE
    writeRegister8(0x71, 0x00 | 0x04);

    // Enable TP INT
    // ToDo: you might want to mask the other bits of the register
    writeRegister8(0xF0, 0x04);

    delay(2);
    writeRegister8(0xF1, 0x04); // clear interrupt

    _inited = true;
    return true;
  }

  uint_fast8_t Touch_RA8875::getTouchRaw(touch_point_t* tp, uint_fast8_t count)
  {
    if (!_inited || count == 0) return 0;   // ToDo: figure what count does

    // what does it do??
    // if (_cfg.pin_int >= 0 && lgfx::gpio_in(_cfg.pin_int))
    // {
    //   return 0;
    // }

    uint16_t tx, ty;
    uint8_t temp, interrupt_reg;

    tx = readRegister8(0x72); // RA8875_TPXH
    ty = readRegister8(0x73); // RA8875_TPYH 
    temp = readRegister8(0x74); // RA8875_TPXYL
    interrupt_reg = readRegister8(0xF1);  // INTC2

    tx <<= 2;
    ty <<= 2;
    tx |= temp & 0x03;        // get the bottom x bits
    ty |= (temp >> 2) & 0x03; // get the bottom y bits

    if(interrupt_reg & (1 << 2)){   // touch interrupt has been triggered
      tp->x = tx;
      tp->y = ty;
      tp->size = 1;
    }else{
      tp->x = 0;
      tp->y = 0;
      tp->size = 0;
    }    

    writeRegister8(0xF1, 0x04); // clear interrupt

    return tp->size ? 1 : 0;
  }

  uint8_t Touch_RA8875::readRegister8(uint8_t reg)
  {
    if (isSPI()) {
      uint8_t tmp[3] = {0x80, reg, 0x40};
      uint8_t res = 0;

      spi::beginTransaction(_cfg.spi_host, _cfg.freq);
      lgfx::gpio_lo(_cfg.pin_cs);

      spi::writeBytes(_cfg.spi_host, tmp, 3);
      spi::readBytes(_cfg.spi_host, &res, 1);

      spi::endTransaction(_cfg.spi_host);
      lgfx::gpio_hi(_cfg.pin_cs);

      return res;
    }
    return 0;
  }

  void Touch_RA8875::writeRegister8(uint8_t reg, uint8_t data)
  {
    if (isSPI()) {
      uint8_t tmp[4] = {0x80, reg, 0x00, data };

      spi::beginTransaction(_cfg.spi_host, _cfg.freq);
      lgfx::gpio_lo(_cfg.pin_cs);

      spi::writeBytes(_cfg.spi_host, tmp, 4);

      spi::endTransaction(_cfg.spi_host);
      lgfx::gpio_hi(_cfg.pin_cs);
    }
  }
//----------------------------------------------------------------------------
 }
}
