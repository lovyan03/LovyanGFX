#include "Touch_GT911.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  static constexpr std::uint8_t gt911cmd_getdata[] = { 0x81, 0x4E, 0x00 };

  static uint8_t calcChecksum(const uint8_t *buf, uint8_t len)
  {
    uint8_t ccsum = 0;
    for (int i = 0; i < len; i++)
    {
        ccsum += buf[i];
    }
    ccsum = (~ccsum) + 1;
    return ccsum;
  }

  bool Touch_GT911::init(void)
  {
    if (_inited) return true;

    _readdata[0] = 0;

    if (isSPI()) return false;

    lgfx::i2c::init(i2c_port, i2c_sda, i2c_scl, freq);

    if (gpio_int >= 0)
    {
      lgfx::lgfxPinMode(gpio_int, pin_mode_t::input);
    }

    _inited = lgfx::i2c::writeBytes(i2c_port, i2c_addr, gt911cmd_getdata, 3);

    if (_inited)
    {
      std::uint8_t buf[] = { 0x80, 0x56 };
      lgfx::i2c::writeReadBytes(i2c_port, i2c_addr, buf, 2, buf, 1);

      _refresh_rate = 5 + (buf[0] & 0x0F);
/*
      {
        std::uint8_t writedata[4] = { 0x80, 0x40 };

        std::uint8_t readdata[193] = {0};
        lgfx::i2c::writeReadBytes(i2c_port, i2c_addr, writedata, 2, readdata, 193);
        std::uint32_t addr = 0x8040;
        for (int i = 0; i < 12; ++i) {
          Serial.printf("%04x:" , addr);
          for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
              int l = i * 16 + j * 4 + k;
              Serial.printf("%02x ", readdata[l]);
            }
            Serial.print(" ");
          }
          Serial.println();
          addr += 16;
        }
      }
//*/
    }
    return _inited;
  }

  void Touch_GT911::wakeup(void)
  {
    if (!_inited) return;
    if (gpio_int < 0) return;
    lgfx::gpio_hi(gpio_int);
    lgfx::lgfxPinMode(gpio_int, pin_mode_t::output);
    delay(5);
    lgfx::lgfxPinMode(gpio_int, pin_mode_t::input);
  }

  void Touch_GT911::sleep(void)
  {
    if (!_inited) return;
    static constexpr std::uint8_t writedata[] = { 0x80, 0x40, 0x05 };
    lgfx::i2c::writeBytes(i2c_port, i2c_addr, writedata, 3);
  }

  std::uint_fast8_t Touch_GT911::getTouch(touch_point_t* tp, std::int_fast8_t number)
  {
    if (!_inited) return 0;

    std::uint_fast8_t res = 0;

    std::uint32_t nowtime = millis();
    if ((gpio_int < 0 || !gpio_in(gpio_int)) && (std::uint32_t)(nowtime - _lasttime) > _refresh_rate)
    {
      std::uint8_t buf;
      lgfx::i2c::writeReadBytes(i2c_port, i2c_addr, gt911cmd_getdata, 2, &buf, 1);
      if (buf & 0x80)
      {
        _lasttime = nowtime;
        std::int32_t points = std::min(5, buf & 0x0F);
        lgfx::i2c::writeReadBytes(i2c_port, i2c_addr, gt911cmd_getdata, 2, _readdata, 2 + points * 8);
        lgfx::i2c::writeBytes(i2c_port, i2c_addr, gt911cmd_getdata, 3);
      }
    }
    std::int32_t points = std::min(5, _readdata[0] & 0x0F);
    if (number < points && tp != nullptr)
    {
      res = points;
      auto data = reinterpret_cast<std::uint16_t*>(&_readdata[number * 8 + 2]);
      tp->x    = data[0];
      tp->y    = data[1];
      tp->size = data[2];
      tp->id   = data[3] >> 8;
    }
    return res;
  }

  void Touch_GT911::setTouchNums(std::int_fast8_t nums)
  {
    nums = std::max(1, std::min(5, nums));

    std::uint8_t buf[] = { 0x80, 0x4c, 0x00 };
    lgfx::i2c::writeReadBytes(i2c_port, i2c_addr, buf, 2, &buf[2], 1);
    if (buf[2] != nums)
    {
      buf[2] = nums;
      lgfx::i2c::writeBytes(i2c_port, i2c_addr, buf, 3);

      freshConfig();
    }
  }

  void Touch_GT911::freshConfig(void)
  {
    std::uint8_t writedata[188] = { 0x80, 0x47 };
    lgfx::i2c::writeReadBytes(i2c_port, i2c_addr, writedata, 2, &writedata[2], 184);

    writedata[0xBA] = calcChecksum(&writedata[2], 184); // 0x80FF checksum
    writedata[0xBB] = 0x01;                             // 0x8100 config fresh
    lgfx::i2c::writeBytes(i2c_port, i2c_addr, writedata, 188);
  }

//----------------------------------------------------------------------------
 }
}
