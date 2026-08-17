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
#include "Touch_CHSC6540.hpp"

#include "../platforms/common.hpp"

#include <cstring>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Touch_CHSC6540::init(void)
  {
    _inited = false;
    if (isSPI()) return false;

    if (_cfg.pin_int >= 0)
    {
      lgfx::pinMode(_cfg.pin_int, lgfx::pin_mode_t::input);
    }
    _inited = lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value();
    static constexpr uint8_t irq_modechange_cmd[] = { 0x5a, 0x5a };
    lgfx::i2c::transactionWrite(_cfg.i2c_port, _cfg.i2c_addr, irq_modechange_cmd, 2);

    return _inited;
  }

  uint_fast8_t Touch_CHSC6540::getTouchRaw(touch_point_t *tp, uint_fast8_t count)
  {
    if (tp) tp->size = 0;
    if (!_inited || count == 0) return 0;
    if (count > 2) count = 2;
    if (_cfg.pin_int >= 0)
    {
      if (gpio_in(_cfg.pin_int)) return 0;
    }

    size_t len = 3 + count * 6;
    uint8_t buf[2][15];
    int32_t retry = 5;
    bool flip = false;
    uint8_t* tmp;
    for (;;)
    {
      tmp = buf[flip];
      memset(tmp, 0, len);
      if (lgfx::i2c::beginTransaction(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, false))
      {
        static constexpr uint8_t reg_number = 2;
        if (lgfx::i2c::writeBytes(_cfg.i2c_port, &reg_number, 1)
        && lgfx::i2c::restart(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, true)
        && lgfx::i2c::readBytes(_cfg.i2c_port, tmp, 1)
        && (tmp[0] != 0))
        {
          flip = !flip;
          size_t points = std::min<uint_fast8_t>(count, tmp[0]);
          if (points && !lgfx::i2c::readBytes(_cfg.i2c_port, &tmp[1], points * 6 - 2))
          { // 座標を読み切れなかったフレームは破棄する (ゼロ埋めの座標を有効タッチとして返さないため);
            tmp[0] = 0;
          }
        }
        if (lgfx::i2c::endTransaction(_cfg.i2c_port)) {}
        if (tmp[0] == 0 || memcmp(buf[0], buf[1], len) == 0) break;
      }
      // リトライを使い切った場合はタッチ無しとせず直近の読み値を採用する。
      // (指の移動中は連続読出しが一致しないため、return 0 だとフリックが取れない)
      if (0 == --retry) { break; }
    }
    if (count > tmp[0]) count = tmp[0];

    for (size_t idx = 0; idx < count; ++idx)
    {
      auto data = &tmp[1 + idx * 6];
      tp[idx].size = 1;
      tp[idx].x = (data[0] & 0x0F) << 8 | data[1];
      tp[idx].y = (data[2] & 0x0F) << 8 | data[3];
      tp[idx].id = idx;
    }
    return count;
  }

//----------------------------------------------------------------------------
 }
}
