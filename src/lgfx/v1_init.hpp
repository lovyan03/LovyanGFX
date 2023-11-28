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

#include "v1/gitTagVersion.h"

#include "v1/platforms/device.hpp"
#include "v1/platforms/common.hpp"
#include "v1/lgfx_filesystem_support.hpp"
#include "v1/LGFXBase.hpp"
#include "v1/LGFX_Sprite.hpp"
#include "v1/LGFX_Button.hpp"
#include "v1/Light.hpp"

// LCD / OLED
#include "v1/panel/Panel_GC9A01.hpp"
#include "v1/panel/Panel_ILI9163.hpp"
#include "v1/panel/Panel_ILI9225.hpp"
#include "v1/panel/Panel_ILI9341.hpp"
#include "v1/panel/Panel_ILI9806.hpp"
#include "v1/panel/Panel_ILI9342.hpp"
#include "v1/panel/Panel_ILI948x.hpp"
#include "v1/panel/Panel_NT35510.hpp"
#include "v1/panel/Panel_R61529.hpp"
#include "v1/panel/Panel_RA8875.hpp"
#include "v1/panel/Panel_RM68120.hpp"
#include "v1/panel/Panel_S6D04K1.hpp"
#include "v1/panel/Panel_SSD1306.hpp"
#include "v1/panel/Panel_SSD1327.hpp"
#include "v1/panel/Panel_SSD1331.hpp"
#include "v1/panel/Panel_SSD1351.hpp"
#include "v1/panel/Panel_SSD1963.hpp"
#include "v1/panel/Panel_ST7735.hpp"
#include "v1/panel/Panel_ST7789.hpp"
#include "v1/panel/Panel_ST7796.hpp"

// EPD
#include "v1/panel/Panel_GDEW0154M09.hpp"
#include "v1/panel/Panel_IT8951.hpp"

// other
#include "v1/panel/Panel_HUB75.hpp"
#include "v1/panel/Panel_M5UnitLCD.hpp"

// TouchScreen
#include "v1/touch/Touch_CST816S.hpp"
#include "v1/touch/Touch_FT5x06.hpp"
#include "v1/touch/Touch_GSLx680.hpp"
#include "v1/touch/Touch_GT911.hpp"
#include "v1/touch/Touch_NS2009.hpp"
#include "v1/touch/Touch_STMPE610.hpp"
#include "v1/touch/Touch_TT21xxx.hpp"
#include "v1/touch/Touch_XPT2046.hpp"

