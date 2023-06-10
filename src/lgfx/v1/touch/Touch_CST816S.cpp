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

// CST816 info from here...
// https://github.com/lupyuen/hynitron_i2c_cst0xxse/blob/master/cst0xx_common.h
// https://blog.csdn.net/Distance_98/article/details/126408819

#include "Touch_CST816S.hpp"

#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr uint8_t CST816S_TOUCH_REG  = 0x01;
  static constexpr uint8_t CST816S_SLEEP_REG  = 0xA5;
  static constexpr uint8_t CST816S_CHIPID_REG = 0xA7;

  static constexpr uint8_t CST816S_SLEEP_IN   = 0x03;

  bool Touch_CST816S::_write_reg(uint8_t reg, uint8_t val)
  {
    return i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, reg, val, 0, _cfg.freq).has_value();
  }

  bool Touch_CST816S::_write_regs(uint8_t* val, size_t length)
  {
    return i2c::transactionWrite(_cfg.i2c_port, _cfg.i2c_addr, val, length, _cfg.freq).has_value();
  }

  bool Touch_CST816S::_read_reg(uint8_t reg, uint8_t *data, size_t length)
  {
    return lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, &reg, 1, data, length, _cfg.freq).has_value();
  }

  bool Touch_CST816S::_check_init(void)
  {
    if (_inited) return true;

    uint8_t tmp[3] = { 0 };
    _inited = _write_reg(0x00, 0x00)
          && _read_reg(CST816S_CHIPID_REG, tmp, 3);
    // _write_reg(0xFA, 0b00000101); // レジスタ0xFAはINTピンの動作設定に関与すると思われる。
// ESP_LOGV("LGFX","CST816S id:%02x %02x %02x", tmp[0], tmp[1], tmp[2]);
    return _inited;
  }

//----------------------------------------------------------------------------
  bool Touch_CST816S::init(void)
  {
    _inited = false;

    if (_cfg.pin_rst >= 0)
    {
      lgfx::pinMode(_cfg.pin_rst, pin_mode_t::output);
      lgfx::gpio_lo(_cfg.pin_rst);
      lgfx::delay(10);
      lgfx::gpio_hi(_cfg.pin_rst);
      lgfx::delay(10);
    }

    if (_cfg.pin_int >= 0)
    { // intピンのプルアップ処理を行うが、「触れている間 LOW」の設定方法が不明のため、このピンを使用していない。
      // GPIO割込みを使用すれば良いが、Arduinoに依存せず解決する必要があるため、対応を保留する。
      lgfx::pinMode(_cfg.pin_int, pin_mode_t::input_pullup);
    }
    lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value();

    // 画面に触れていない時、I2C通信でCST816が見つからない事がある。
    // CST816S の正確な仕様を確認することができていないが恐らく省電力モード中は通信応答しないものと思われる。
    // 通信の成否による初期化の成否の判定ができないため、ひとまず true を返す。
    return true;
  }

  // void Touch_CST816S::wakeup(void)
  // {
  // }

  // void Touch_CST816S::sleep(void)
  // {
  //   if (!_check_init()) return;
  //   // _write_reg(CST816S_SLEEP_REG, CST816S_SLEEP_IN);
  // }

  size_t Touch_CST816S::_read_data(uint8_t* readdata)
  {
    size_t res = 0;
    if (lgfx::i2c::beginTransaction(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, false))
    {
      readdata[0] = 0x02;
      if (lgfx::i2c::writeBytes(_cfg.i2c_port, readdata, 1)
      && lgfx::i2c::restart(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, true)
      && lgfx::i2c::readBytes(_cfg.i2c_port, readdata, 1))
      {
        uint_fast8_t points = std::min<uint_fast8_t>(max_touch_points, readdata[0] & 0x0Fu);
        if (points)
        {
          if (lgfx::i2c::readBytes(_cfg.i2c_port, &readdata[1], points * 6 - 2))
          {
            res = points * 6 - 1;
          }
        }
        else
        {
          res = 1;
        }
      }
      lgfx::i2c::endTransaction(_cfg.i2c_port).has_value();
    }
    return res;
  }

  uint_fast8_t Touch_CST816S::getTouchRaw(touch_point_t *tp, uint_fast8_t count)
  {
    if (!_inited && !_check_init()) return 0;

    if (count > max_touch_points) { count = max_touch_points; }

    uint8_t readdata[8];
    int retry = 3;
    do
    {
      if (!_read_reg(0x02, readdata, 6))
      {
        return 0;
      }
      count = readdata[0];
    } while ((count > 1 || readdata[5] != 0x10) && --retry);
    if (retry && count)
    {
      // ESP_LOGV("CST816S", "%02x %02x %02x %02x %02x %02x %02x %02x %d %d",
      //                     readdata[0], readdata[1], readdata[2], readdata[3],
      //                     readdata[4], readdata[5], readdata[6], readdata[7], _inited, lgfx::gpio_in(_cfg.pin_int));
      tp[0].id = 0;
      tp[0].size = 1;
      tp[0].x = readdata[2] | (readdata[1] & 0x0F) << 8;
      tp[0].y = readdata[4] | (readdata[3] & 0x0F) << 8;
    }
    return count;
  }

//----------------------------------------------------------------------------
 }
}
