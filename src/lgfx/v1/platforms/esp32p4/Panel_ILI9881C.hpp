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

#include "Panel_DSI.hpp"
#if SOC_MIPI_DSI_SUPPORTED

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

struct Panel_ILI9881C : public Panel_DSI
  {
  public:

  protected:
  };

//----------------------------------------------------------------------------
 }
}

#endif
