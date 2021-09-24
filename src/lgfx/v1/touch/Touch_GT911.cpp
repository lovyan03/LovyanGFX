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
#include "Touch_GT911.hpp"

#include "../../internal/algorithm.h"
#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  static constexpr uint8_t gt911cmd_getdata[] = { 0x81, 0x4E, 0x00 };

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

  bool Touch_GT911::writeBytes(const uint8_t* data, size_t len)
  {
    return lgfx::i2c::transactionWrite(_cfg.i2c_port, _cfg.i2c_addr, data, len, _cfg.freq).has_value();
  }

  bool Touch_GT911::writeReadBytes(const uint8_t* write_data, size_t write_len, uint8_t* read_data, size_t read_len)
  {
    return lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, write_data, write_len, read_data, read_len, _cfg.freq).has_value();
  }

  bool Touch_GT911::init(void)
  {
    if (_inited) return true;

    _readdata[0] = 0;

    if (isSPI()) return false;

    if (_cfg.pin_int >= 0)
    {
      lgfx::lgfxPinMode(_cfg.pin_int, pin_mode_t::input);
    }

    _inited = lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value() && writeBytes(gt911cmd_getdata, 3);

    if (_inited)
    {
      uint8_t buf[] = { 0x80, 0x56 };
      writeReadBytes(buf, 2, buf, 1);

      _refresh_rate = 5 + (buf[0] & 0x0F);
/*
      {
        uint8_t writedata[4] = { 0x80, 0x40 };

        uint8_t readdata[193] = {0};
        writeReadBytes(writedata, 2, readdata, 193);
        uint32_t addr = 0x8040;
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
    if (_cfg.pin_int < 0) return;
    lgfx::gpio_hi(_cfg.pin_int);
    lgfx::lgfxPinMode(_cfg.pin_int, pin_mode_t::output);
    delay(5);
    lgfx::lgfxPinMode(_cfg.pin_int, pin_mode_t::input);
  }

  void Touch_GT911::sleep(void)
  {
    if (!_inited) return;
    static constexpr uint8_t writedata[] = { 0x80, 0x40, 0x05 };
    writeBytes(writedata, 3);
  }

  uint_fast8_t Touch_GT911::getTouchRaw(touch_point_t* __restrict tp, uint_fast8_t count)
  {
    if (!_inited || count == 0) return 0;
    if (count > 5) { count = 5; }

    if ((_cfg.pin_int < 0 || !gpio_in(_cfg.pin_int)))
    {
      if (lgfx::i2c::beginTransaction(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, false))
      {
        uint8_t buf;
        if (lgfx::i2c::writeBytes(_cfg.i2c_port, gt911cmd_getdata, 2)
         && lgfx::i2c::restart(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, true)
         && lgfx::i2c::readBytes(_cfg.i2c_port, &buf, 1)
         && (buf & 0x80))
        {
          uint32_t points = std::min<uint_fast8_t>(count, buf & 0x0F);
          if (lgfx::i2c::readBytes(_cfg.i2c_port, &_readdata[1], points * 8))
          {
            _readdata[0] = buf;
          }
          if (lgfx::i2c::endTransaction(_cfg.i2c_port)
           && lgfx::i2c::beginTransaction(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, false)
           && lgfx::i2c::writeBytes(_cfg.i2c_port, gt911cmd_getdata, 3))
          {}
        }
        if (lgfx::i2c::endTransaction(_cfg.i2c_port)) {}
      }
    }
    uint32_t points = std::min<uint_fast8_t>(count, _readdata[0] & 0x0F);
    for (size_t idx = 0; idx < points; ++idx)
    {
      auto data = reinterpret_cast<uint16_t*>(&_readdata[idx * 8]);
      tp[idx].id   = data[0] >> 8;
      tp[idx].x    = data[1];
      tp[idx].y    = data[2];
      tp[idx].size = data[3];
    }
    return points;
  }

  void Touch_GT911::setTouchNums(int_fast8_t nums)
  {
    nums = std::max<int_fast8_t>(1, std::min<int_fast8_t>(5, nums));

    uint8_t buf[] = { 0x80, 0x4c, 0x00 };
    writeReadBytes(buf, 2, &buf[2], 1);
    if (buf[2] != nums)
    {
      buf[2] = nums;
      writeBytes(buf, 3);

      freshConfig();
    }
  }

  void Touch_GT911::freshConfig(void)
  {
    // 設定レジスタ全体を読取り
    uint8_t writedata[188] = { 0x80, 0x47 };
    if (writeReadBytes(writedata, 2, &writedata[2], 184))
    {
      // チェックサムを計算し、設定値の更新指示を行う
      writedata[0xBA] = calcChecksum(&writedata[2], 184); // 0x80FF checksum
      writedata[0xBB] = 0x01;                             // 0x8100 config fresh
      writeBytes(writedata, 188);
    }
  }

//----------------------------------------------------------------------------
 }
}
