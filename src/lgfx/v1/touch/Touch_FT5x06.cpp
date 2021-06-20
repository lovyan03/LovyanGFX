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
#include "Touch_FT5x06.hpp"

#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr std::uint8_t FT5x06_VENDID_REG = 0xA8;
  static constexpr std::uint8_t FT5x06_POWER_REG  = 0x87;
  static constexpr std::uint8_t FT5x06_PERIODACTIVE = 0x88;
  static constexpr std::uint8_t FT5x06_INTMODE_REG= 0xA4;

  static constexpr std::uint8_t FT5x06_MONITOR  = 0x01;
  static constexpr std::uint8_t FT5x06_SLEEP_IN = 0x03;

  bool Touch_FT5x06::_write_reg(std::uint8_t reg, std::uint8_t val)
  {
    return i2c::registerWrite8(_cfg.i2c_port, _cfg.i2c_addr, reg, val, 0, _cfg.freq).has_value();

    // std::uint8_t data[] = { reg, val };
    // return lgfx::i2c::transactionWrite(_cfg.i2c_port, _cfg.i2c_addr, data, 2, _cfg.freq).has_value();
  }

  bool Touch_FT5x06::_read_reg(std::uint8_t reg, std::uint8_t *data, std::size_t length)
  {
    return lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, &reg, 1, data, length, _cfg.freq).has_value();
  }

  bool Touch_FT5x06::_check_init(void)
  {
    if (_inited) return true;

    std::uint8_t tmp[2] = { 0 };
    _inited = _write_reg(0x00, 0x00)
          && _read_reg(FT5x06_VENDID_REG, tmp, 1)
          && _write_reg(FT5x06_INTMODE_REG, 0x00) // INT Polling mode
          && tmp[0]
          ;

    return _inited;
  }

  bool Touch_FT5x06::init(void)
  {
    _inited = false;
    if (isSPI()) return false;

    if (_cfg.pin_int >= 0)
    {
      lgfx::pinMode(_cfg.pin_int, pin_mode_t::input_pullup);
    }
    return (lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value()) && _check_init();
  }

  void Touch_FT5x06::wakeup(void)
  {
    if (!_check_init()) return;
    _write_reg(FT5x06_POWER_REG, FT5x06_MONITOR);
  }

  void Touch_FT5x06::sleep(void)
  {
    if (!_check_init()) return;
    _write_reg(FT5x06_POWER_REG, FT5x06_SLEEP_IN);
  }

  std::uint_fast8_t Touch_FT5x06::getTouchRaw(touch_point_t *tp, std::uint_fast8_t number)
  {
    if (tp) tp->size = 0;
    if (!_check_init() || number > 4) return 0;
    if (_cfg.pin_int >= 0)
    {
      if (_flg_released != (bool)gpio_in(_cfg.pin_int))
      {
        _flg_released = !_flg_released;
        _write_reg(FT5x06_INTMODE_REG, 0x00); // INT Polling mode
      }
      if (_flg_released)
      {
        return 0;
      }
    }

    std::size_t base = number * 6;
    std::size_t len = base + 5;

    std::uint8_t tmp[2][len];
    _read_reg(0x02, tmp[0], len);
    std::int32_t retry = 5;
    do
    { // 読出し中に値が変わる事があるので、連続読出しして前回と同値でなければリトライする
      _read_reg(0x02, tmp[retry & 1], len);
    } while (memcmp(tmp[0], tmp[1], len) && --retry);

    if (number >= tmp[0][0]) return 0;
  
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
