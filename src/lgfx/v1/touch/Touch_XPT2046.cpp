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
#include "Touch_XPT2046.hpp"

#include "../platforms/common.hpp"

#include <algorithm>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Touch_XPT2046::init(void)
  {
    _inited = false;
    if (!isSPI()) return false;

    lgfx::gpio_hi(_cfg.pin_cs);
    lgfx::pinMode(_cfg.pin_cs, lgfx::pin_mode_t::output);
    if (lgfx::spi::init(_cfg.spi_host, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi).has_error())
    {
      return false;
    }

    if (_cfg.pin_int >= 0)
    {
      lgfx::pinMode(_cfg.pin_int, pin_mode_t::input);
    }

    _inited = true;
    return true;
  }

  std::uint_fast8_t Touch_XPT2046::getTouchRaw(touch_point_t *tp, std::uint_fast8_t number)
  {
    tp->size = 0;

    if (!_inited || number != 0) return 0;

    if (_cfg.pin_int >= 0)
    {
      if (lgfx::gpio_in(_cfg.pin_int)) return 0;
    }

    std::uint_fast16_t xt[21], yt[21], zt[21];
    std::uint8_t data[57];
    for (std::size_t i = 0; i < 3; ++i)
    {
      memset(data, 0, 8);
      data[ 0] = 0xB1;
      data[ 2] = 0xC1;
      data[ 4] = 0x91;
      data[ 6] = 0xD1;
      memcpy(&data[ 8], data,  8);
      memcpy(&data[16], data, 16);
      memcpy(&data[32], data, 24);
      data[54] = 0xD0; // last power off.
      data[56] = 0;

      spi::beginTransaction(_cfg.spi_host, _cfg.freq, 0);
      lgfx::gpio_lo(_cfg.pin_cs);
      spi::readBytes(_cfg.spi_host, data, 57);
      spi::endTransaction(_cfg.spi_host);
      lgfx::gpio_hi(_cfg.pin_cs);

      for (std::size_t j = 0; j < 7; ++j)
      {
        auto d = &data[j * 8];
        int x = (d[7] << 8 | d[8]) >> 3;
        int y = (d[5] << 8 | d[6]) >> 3;
        int z = 0x3000 + y - x
              + (((d[1] << 8 | d[2])
                - (d[3] << 8 | d[4])) >> 1);
        if (z <= 0) return 0;
        xt[i * 7 + j] = x;
        yt[i * 7 + j] = y;
        zt[i * 7 + j] = z;
      }
    }

    std::partial_sort(xt, xt+12, xt+21);
    tp->x = (xt[8]+xt[9]+xt[10]+xt[11]) >> 2;

    std::partial_sort(yt, yt+12, yt+21);
    tp->y = (yt[8]+yt[9]+yt[10]+yt[11]) >> 2;

    std::partial_sort(zt, zt+11, zt+21);
    tp->size = zt[10] >> 8;

    return tp->size ? 1 : 0;
  }

//----------------------------------------------------------------------------
 }
}
