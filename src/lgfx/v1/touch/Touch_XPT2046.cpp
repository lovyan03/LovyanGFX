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

#include "../../internal/algorithm.h"
#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  bool Touch_XPT2046::init(void)
  {
    _inited = false;

    if (_cfg.pin_cs > -1) {
      lgfx::gpio_hi(_cfg.pin_cs);
      lgfx::pinMode(_cfg.pin_cs, lgfx::pin_mode_t::output);
    }
    if (_cfg.spi_host < 0)
    {
      pinMode(_cfg.pin_sclk, lgfx::pin_mode_t::output);
      pinMode(_cfg.pin_mosi, lgfx::pin_mode_t::output);
      pinMode(_cfg.pin_miso, lgfx::pin_mode_t::input_pullup);
    }
    else
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

  static void transfer(uint8_t* read_data, const uint8_t* write_data, size_t len, const Touch_XPT2046::config_t *cfg)
  {
    int pin_sclk = cfg->pin_sclk;
    int pin_miso = cfg->pin_miso;
    int pin_mosi = cfg->pin_mosi;

    do
    {
      uint_fast8_t r = 0;
      uint_fast8_t mask = 0x80;
      uint_fast8_t d = *write_data++;
      do
      {
        gpio_lo(pin_sclk);
        if (d & mask) { gpio_hi(pin_mosi); } else { gpio_lo(pin_mosi); }
        gpio_hi(pin_sclk);
        if (gpio_in(pin_miso)) { r += mask; }
      } while (mask >>= 1);

      *read_data++ = (uint8_t)r;
    } while (--len);
    gpio_lo(pin_sclk);
  }

  uint_fast8_t Touch_XPT2046::getTouchRaw(touch_point_t* __restrict tp, uint_fast8_t count)
  {
    if (!_inited || count == 0) return 0;

    tp->size = 0;

    if (_cfg.pin_int >= 0 && lgfx::gpio_in(_cfg.pin_int))
    {
      return 0;
    }

    uint8_t data[57];

    memset(data, 0, 8);
    data[ 0] = 0x91;
    data[ 2] = 0xB1;
    data[ 4] = 0xD1;
    data[ 6] = 0xC1;
    data[56] = 0x80; // last power off.
    memcpy(&data[ 8], data,  8);
    memcpy(&data[16], data, 16);
    memcpy(&data[32], data, 24);

    if (_cfg.spi_host < 0)
    {
      lgfx::gpio_lo(_cfg.pin_cs);
      transfer(data, data, 57, &_cfg);
      lgfx::gpio_hi(_cfg.pin_cs);
    }
    else
    {
      spi::beginTransaction(_cfg.spi_host, _cfg.freq, 0);
      if (_cfg.pin_cs > -1)
        lgfx::gpio_lo(_cfg.pin_cs);
      spi::readBytes(_cfg.spi_host, data, 57);
      if (_cfg.pin_cs > -1)
        lgfx::gpio_hi(_cfg.pin_cs);
      spi::endTransaction(_cfg.spi_host);
    }

    size_t ix = 0, iy = 0, iz = 0;
    uint_fast16_t xt[7], yt[7], zt[7];
    for (size_t j = 0; j < 7; ++j)
    {
      auto d = &data[j * 8];
      int x = (d[5] << 8 | d[6]) >> 3;
      int y = (d[1] << 8 | d[2]) >> 3;
      int z = 0x3200 + y - x
            + (((d[3] << 8 | d[4])
              - (d[7] << 8 | d[8])) >> 1);
      if (x > 128 && x <= 3968)
      {
        xt[ix++] = x;
      }
      if (y > 128 && y <= 3968)
      {
        yt[iy++] = y;
      }
      if (z > 0)
      {
        zt[iz++] = z;
      }
    }
    if (ix < 3 || iy < 3 || iz < 3)
    {
      return 0;
    }

    std::sort(xt, xt + ix);
    std::sort(yt, yt + iy);
    std::sort(zt, zt + iz);

    tp->x    = xt[ix>>1];
    tp->y    = yt[iy>>1];
    tp->size = zt[iz>>1] >> 8;

    return tp->size ? 1 : 0;
  }

//----------------------------------------------------------------------------
 }
}
