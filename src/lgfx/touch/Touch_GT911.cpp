#include "Touch_GT911.hpp"

namespace lgfx
{
  static constexpr std::uint8_t GT911_VENDID_REG = 0xA8;
  static constexpr std::uint8_t GT911_POWER_REG  = 0x87;
  static constexpr std::uint8_t GT911_INTMODE_REG= 0xA4;

  static constexpr std::uint8_t GT911_MONITOR  = 0x01;
  static constexpr std::uint8_t GT911_SLEEP_IN = 0x03;

  bool Touch_GT911::init(void)
  {
    _readdata[0] = 0;

    _inited = false;
    if (isSPI()) return false;

    lgfx::i2c::init(i2c_port, i2c_sda, i2c_scl, freq);
    /*
    std::uint8_t writedata[] = { 0x80, 0x47 };
    std::uint8_t readdata[64] = {0};
    lgfx::i2c::writeReadBytes(i2c_port, i2c_addr, writedata, 2, readdata, 64);
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 16; ++j) {
        int k = i * 16 + j;
        Serial.printf("%02x ", readdata[k]);
      }
      Serial.println();
    }
    //*/
    _inited = true;
    return true;
  }

  void Touch_GT911::wakeup(void)
  {
    if (!_inited) return;
    lgfx::i2c::writeRegister8(i2c_port, i2c_addr, GT911_POWER_REG, GT911_MONITOR);
  }

  void Touch_GT911::sleep(void)
  {
    if (!_inited) return;
    lgfx::i2c::writeRegister8(i2c_port, i2c_addr, GT911_POWER_REG, GT911_SLEEP_IN);
  }

  std::uint_fast8_t Touch_GT911::getTouch(std::int32_t* x, std::int32_t* y, std::int_fast8_t number)
  {
    if (!_inited) return 0;

//  Serial.printf("%d \r\n", gpio_in(gpio_int));
    static constexpr std::uint8_t writedata[] = { 0x81, 0x4E, 0x00 };

    std::uint_fast8_t res = 0;

    if (gpio_int < 0 || !gpio_in(gpio_int))
    {
      lgfx::i2c::writeReadBytes(i2c_port, i2c_addr, writedata, 2, _readdata, 16);
      /*
      for (int i = 0; i < 1; ++i) {
        for (int j = 0; j < 16; ++j) {
          int k = i * 16 + j;
          Serial.printf("%02x ", readdata[k]);
        }
        Serial.println();
      }
      //*/
      if (_readdata[0] & 0x80)
      {
        lgfx::i2c::writeBytes(i2c_port, i2c_addr, writedata, 3);
      }
    }
    std::uint_fast8_t points = _readdata[0] & 0x0F;
    if (number < points)
    {
      res = points;
      auto data = reinterpret_cast<std::uint16_t*>(&_readdata[number * 8 + 2]);
      if (x) *x = data[0];
      if (y) *y = data[1];
    }
    return res;
  }
//----------------------------------------------------------------------------
}
