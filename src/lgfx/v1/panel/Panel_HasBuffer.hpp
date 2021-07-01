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

#include "Panel_Device.hpp"
#include "../misc/range.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Panel_HasBuffer : public Panel_Device
  {

  public:

    Panel_HasBuffer(void);
    virtual ~Panel_HasBuffer(void);

    bool init(bool use_reset) override;
    void beginTransaction(void) override;
    void endTransaction(void) override;

    void setRotation(std::uint_fast8_t r) override;

    void setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) override;
    void drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor) override;
    void writeBlock(std::uint32_t rawcolor, std::uint32_t length) override;

  protected:
    std::uint8_t* _buf = nullptr;
    range_rect_t _range_mod;
    std::int32_t _xpos = 0;
    std::int32_t _ypos = 0;

    virtual std::size_t _get_buffer_length(void) const = 0;
    void _rotate_pos(std::uint_fast16_t &x, std::uint_fast16_t &y);
    void _rotate_pos(std::uint_fast16_t &xs, std::uint_fast16_t &ys, std::uint_fast16_t &xe, std::uint_fast16_t &ye);
  };

//----------------------------------------------------------------------------
 }
}
