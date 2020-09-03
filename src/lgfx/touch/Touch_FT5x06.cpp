#include "Touch_FT5x06.hpp"

namespace lgfx
{
  static constexpr std::uint8_t FT5x06_VENDID_REG = 0xA8;
  static constexpr std::uint8_t FT5x06_POWER_REG  = 0x87;

  static constexpr std::uint8_t FT5x06_MONITOR  = 0x01;
  static constexpr std::uint8_t FT5x06_SLEEP_IN = 0x03;

  bool Touch_FT5x06::init(void)
  {
    _inited = false;
    if (isSPI()) return false;

    lgfx::i2c::init(i2c_port, i2c_sda, i2c_scl, freq);

    std::uint8_t tmp[32];
    if (!lgfx::i2c::readRegister(i2c_port, i2c_addr, FT5x06_VENDID_REG, tmp, 1)) {
      return false;
    }

    tmp[0] = 0x00; // OperatingMode
    lgfx::i2c::writeRegister(i2c_port, i2c_addr, 0x00, tmp, 1);

    _inited = true;
    return true;
  }

  void Touch_FT5x06::wakeup(void)
  {
    if (!_inited) return;
    lgfx::i2c::writeRegister(i2c_port, i2c_addr, FT5x06_POWER_REG, &FT5x06_MONITOR, 1);
  }

  void Touch_FT5x06::sleep(void)
  {
    if (!_inited) return;
    lgfx::i2c::writeRegister(i2c_port, i2c_addr, FT5x06_POWER_REG, &FT5x06_SLEEP_IN, 1);
  }

  std::uint_fast8_t Touch_FT5x06::getTouch(std::int32_t* x, std::int32_t* y, std::int_fast8_t number)
  {
    if (!_inited) return 0;
    std::uint8_t tmp[16];
    std::uint32_t base = (number == 0) ? 0 : 6;
    lgfx::i2c::readRegister(i2c_port, i2c_addr, 2, tmp, 5 + base);
    if (number >= tmp[0]) return 0;
    if (x) *x = (tmp[base + 1] & 0x0F) << 8 | tmp[base + 2];
    if (y) *y = (tmp[base + 3] & 0x0F) << 8 | tmp[base + 4];
    return tmp[0];
  }

//----------------------------------------------------------------------------

}
