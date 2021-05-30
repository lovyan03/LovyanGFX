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
    lgfx::spi::init(_cfg.spi_host, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi);

    _inited = true;
    return true;
  }

  void Touch_XPT2046::wakeup(void)
  {
    if (!_inited) return;
  }

  void Touch_XPT2046::sleep(void)
  {
    if (!_inited) return;
  }

  std::uint_fast8_t Touch_XPT2046::getTouchRaw(touch_point_t *tp, std::uint_fast8_t number)
  {
    tp->size = 0;

    if (!_inited || number != 0) return 0;
    if (!isSPI()) return 0;

    int xt[24], yt[24], size[21];
    uint8_t data[61];
    for (int i = 0; i < 3; ++i)
    {
      memset(data, 0, 61);
      data[ 0] = 0xD1;
      data[ 2] = 0x91;
      data[ 4] = 0xB1;
      data[ 6] = 0xC1;
      memcpy(&data[ 8], data,  8);
      memcpy(&data[16], data, 16);
      memcpy(&data[32], data, 28);

      spi::beginTransaction(_cfg.spi_host, _cfg.freq, 0);
      lgfx::gpio_lo(_cfg.pin_cs);
      spi::readBytes(_cfg.spi_host, data, 61);
      spi::endTransaction(_cfg.spi_host);
      lgfx::gpio_hi(_cfg.pin_cs);

      for (std::size_t j = 0; j < 7; ++j)
      {
        int tmp = 0xFFF
                + (data[5 + j * 8] << 5 | data[6 + j * 8] >> 3)
                - (data[7 + j * 8] << 5 | data[8 + j * 8] >> 3);
        size[i * 7 + j] = std::max(0, tmp);
      }
      for (std::size_t j = 0; j < 8; ++j)
      {
        xt[i * 8 + j] = data[1 + j * 8] << 5 | data[2 + j * 8] >> 3;
        yt[i * 8 + j] = data[3 + j * 8] << 5 | data[4 + j * 8] >> 3;
      }
    }

    std::sort(xt, xt+24);
    tp->x = (xt[10]+xt[11]+xt[12]+xt[13]) >> 2;

    std::sort(yt, yt+24);
    tp->y = (yt[10]+yt[11]+yt[12]+yt[13]) >> 2;

    std::sort(size, size+21);
    tp->size = std::max<std::uint16_t>(0,
                        0
                        + size[10]
                        + (tp->y * ((_cfg.x_max - tp->x) >> 4) >> 9) // 座標による感度の差を補正(LoLIN D32 Proで調整)
                        - threshold
                        ) >> 8;

    return tp->size ? 1 : 0;
  }

//----------------------------------------------------------------------------
 }
}
