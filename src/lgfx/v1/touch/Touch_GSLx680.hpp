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
#pragma once

#include "../Touch.hpp"

#include "gslx680/Touch_GSLx680_FW.h"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Touch_GSLx680 : public ITouch
  {
    bool init(void) override;
    void wakeup(void) override {}
    void sleep(void) override {}

    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override;

    void setFirmWare(const gsl_fw_data* fw_data, size_t fw_size) { _fw_data = fw_data; _fw_size = fw_size; };

  protected:
    const gsl_fw_data* _fw_data = nullptr;
    size_t _fw_size = 0;

  private:
    uint8_t _readdata[44] = {0}; // 10point * 4byte + 4byte

    bool _init(void);
  };

//----------------------------------------------------------------------------

  struct Touch_GSL1680F_800x480 : public Touch_GSLx680
  {
    Touch_GSL1680F_800x480(void);
  };

//----------------------------------------------------------------------------

  struct Touch_GSL1680F_480x272 : public Touch_GSLx680
  {
    Touch_GSL1680F_480x272(void);
  };

//----------------------------------------------------------------------------

  struct Touch_GSL1680E_800x480 : public Touch_GSLx680
  {
    Touch_GSL1680E_800x480(void);
  };

//----------------------------------------------------------------------------

  struct Touch_GSLx680_320x320 : public Touch_GSLx680
  {
    Touch_GSLx680_320x320(void);
  };

//----------------------------------------------------------------------------
 }
}
