#include "Touch_FT5x06.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  static constexpr std::uint8_t FT5x06_VENDID_REG = 0xA8;
  static constexpr std::uint8_t FT5x06_POWER_REG  = 0x87;
  static constexpr std::uint8_t FT5x06_INTMODE_REG= 0xA4;

  static constexpr std::uint8_t FT5x06_MONITOR  = 0x01;
  static constexpr std::uint8_t FT5x06_SLEEP_IN = 0x03;

  bool Touch_FT5x06::init(void)
  {
    _inited = false;
    if (isSPI()) return false;

    lgfx::i2c::init(i2c_port, i2c_sda, i2c_scl, freq);

    lgfx::i2c::writeRegister8(i2c_port, i2c_addr, 0x00, 0x00); // OperatingMode

    std::uint8_t tmp[2];
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

  std::uint_fast8_t Touch_FT5x06::getTouch(touch_point_t* tp, std::int_fast8_t number)
  {
    if (!_inited || number > 4) return 0;
    if (gpio_int >= 0 && gpio_in(gpio_int)) return 0;

    std::uint_fast16_t tx, ty;
    std::int32_t retry = 3;
    std::uint32_t base = number * 6;
    std::uint8_t tmp[base + 5];
    do {
      lgfx::i2c::readRegister(i2c_port, i2c_addr, 2, tmp, 5 + base);
      if (number >= tmp[0]) return 0;
      tx = (tmp[base + 1] & 0x0F) << 8 | tmp[base + 2];
      ty = (tmp[base + 3] & 0x0F) << 8 | tmp[base + 4];
    } while ((tx > x_max || ty > y_max) && --retry);
    if (tp)
    {
      tp->x = tx;
      tp->y = ty;
      tp->id = tmp[base + 3] >> 4;
    }
    return tmp[0];
  }

//----------------------------------------------------------------------------
 }
}
