#include "../lgfx_common.hpp"
#if defined (LGFX_ENABLE_V0)

#include "Touch_XPT2046.hpp"

#include <algorithm>

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  bool Touch_XPT2046::init(void)
  {
    _inited = false;
    if (!isSPI()) return false;

    lgfx::spi::init(spi_host, spi_sclk, spi_miso, spi_mosi);

    _inited = true;
    return true;
  }

  uint_fast8_t Touch_XPT2046::getTouch(touch_point_t* tp, int_fast8_t number)
  {
    if (!_inited || number != 0) return 0;
    if (!isSPI()) return 0;

    int xt[24], yt[24];
    uint8_t data[61];
    for (int i = 0; i < 3; ++i) {
      memset(data, 0, 61);
      data[ 0] = 0xD1;
      data[ 2] = 0x91;
      data[ 4] = 0xB1;
      data[ 6] = 0xC1;
      memcpy(&data[ 8], data,  8);
      memcpy(&data[16], data, 16);
      memcpy(&data[32], data, 28);

      spi::beginTransaction(spi_host, spi_cs, freq, 0);
      spi::readData(spi_host, data, 61);
      spi::endTransaction(spi_host, spi_cs);

      for (int j = 0; j < 7; ++j) {
        int tmp = 0xFFF
             + (data[5 + j * 8] << 5 | data[6 + j * 8] >> 3)
             - (data[7 + j * 8] << 5 | data[8 + j * 8] >> 3);
        if (tmp < threshold) return 0;
      }
      for (int j = 0; j < 8; ++j) {
        xt[i * 8 + j] = data[1 + j * 8] << 5 | data[2 + j * 8] >> 3;
        yt[i * 8 + j] = data[3 + j * 8] << 5 | data[4 + j * 8] >> 3;
      }
    }

    if (tp) {
      std::sort(xt, xt+24);
      tp->x = (xt[10]+xt[11]+xt[12]+xt[13]) >> 2;
      std::sort(yt, yt+24);
      tp->y = (yt[10]+yt[11]+yt[12]+yt[13]) >> 2;
    }
    return 1;
  }

//----------------------------------------------------------------------------
 }
}
#endif
