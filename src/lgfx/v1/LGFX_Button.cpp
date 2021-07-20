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

#include "LGFX_Button.hpp"

#include "LGFXBase.hpp"
#include <limits>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void LGFX_Button::_init_button( LovyanGFX *gfx, std::int16_t x, std::int16_t y, std::uint16_t w, std::uint16_t h
                    , const char *label, float textsize_x, float textsize_y)
  {
    _gfx = gfx;
    _x   = x;
    _y   = y;
    _w   = w;
    _h   = h;
    _textsize_x = textsize_x;
    _textsize_y = textsize_y <= std::numeric_limits<float>::epsilon() ? textsize_x : textsize_y;
    strncpy(_label, label, 12);
  }

  // Adjust text datum and x, y deltas
  void LGFX_Button::setLabelDatum(int16_t x_delta, int16_t y_delta, textdatum_t datum)
  {
    _xd = x_delta;
    _yd = y_delta;
    _textdatum = datum;
  }

  void LGFX_Button::drawButton(bool inverted, const char* long_name)
  {
    auto style = _gfx->getTextStyle();

    _gfx->setTextSize(_textsize_x, _textsize_y);
    _gfx->setTextDatum(_textdatum);
    _gfx->setTextPadding(0);

    auto fill = inverted ? _textcolor : _fillcolor;
    auto text = inverted ? _fillcolor : _textcolor;
    _gfx->setTextColor(text, fill);

    std::uint_fast8_t r = std::min(_w, _h) >> 2; // Corner radius
    _gfx->startWrite();
    _gfx->fillRoundRect(_x, _y, _w, _h, r, fill);
    _gfx->drawRoundRect(_x, _y, _w, _h, r, _outlinecolor);
    _gfx->drawString(long_name ? long_name : _label, _x + (_w >> 1) + _xd, _y + (_h >> 1) + _yd);
    _gfx->endWrite();

    _gfx->setTextStyle(style);
  }

//----------------------------------------------------------------------------
 }
}
