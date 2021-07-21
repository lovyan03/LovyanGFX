#include "../lgfx_common.hpp"
#if defined (LGFX_ENABLE_V0)

#include "Touch_FT5x06.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  static constexpr uint8_t FT5x06_VENDID_REG = 0xA8;
  static constexpr uint8_t FT5x06_POWER_REG  = 0x87;
  static constexpr uint8_t FT5x06_INTMODE_REG= 0xA4;

  static constexpr uint8_t FT5x06_MONITOR  = 0x01;
  static constexpr uint8_t FT5x06_SLEEP_IN = 0x03;

  bool Touch_FT5x06::init(void)
  {
    _inited = false;
    if (isSPI()) return false;

    lgfx::i2c::init(i2c_port, i2c_sda, i2c_scl, freq);

    lgfx::i2c::writeRegister8(i2c_port, i2c_addr, 0x00, 0x00); // OperatingMode

    uint8_t tmp[2];
    if (!lgfx::i2c::readRegister(i2c_port, i2c_addr, FT5x06_VENDID_REG, tmp, 1)) {
      return false;
    }

    if (gpio_int >= 0)
    {
      lgfx::i2c::writeRegister8(i2c_port, i2c_addr, FT5x06_INTMODE_REG, 0x00); // INT Polling mode
      lgfx::lgfxPinMode(gpio_int, pin_mode_t::input);
    }
    _inited = true;
    return true;
  }

  void Touch_FT5x06::wakeup(void)
  {
    if (!_inited) return;
    lgfx::i2c::writeRegister8(i2c_port, i2c_addr, FT5x06_POWER_REG, FT5x06_MONITOR);
  }

  void Touch_FT5x06::sleep(void)
  {
    if (!_inited) return;
    lgfx::i2c::writeRegister8(i2c_port, i2c_addr, FT5x06_POWER_REG, FT5x06_SLEEP_IN);
  }

  uint_fast8_t Touch_FT5x06::getTouch(touch_point_t* tp, int_fast8_t number)
  {
    if (!_inited || number > 4) return 0;
    if (gpio_int >= 0 && gpio_in(gpio_int)) return 0;

    size_t base = number * 6;
    size_t len = base + 5;

    uint8_t tmp[2][len];
    lgfx::i2c::readRegister(i2c_port, i2c_addr, 2, tmp[0], len);
    int32_t retry = 5;
    do {
      lgfx::i2c::readRegister(i2c_port, i2c_addr, 2, tmp[retry & 1], len);
    } while (memcmp(tmp[0], tmp[1], len) && --retry);

    if ((uint8_t)number >= tmp[0][0]) return 0;

    if (tp)
    {
      auto data = &tmp[0][base];
      tp->size = 1;
      tp->x = (data[1] & 0x0F) << 8 | data[2];
      tp->y = (data[3] & 0x0F) << 8 | data[4];
      tp->id = data[3] >> 4;
    }
    return tmp[0][0];
  }

//----------------------------------------------------------------------------
 }
}
#endif
