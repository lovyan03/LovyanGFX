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
#include "Panel_SH8601Z.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"
#include "../misc/colortype.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

    /* Panel init */
    bool Panel_SH8601Z::init(bool use_reset)
    {
        if (!Panel_Device::init(use_reset)) {
            return false;
        }


        /* Init command */
        

        return false;
    }


    void Panel_SH8601Z::beginTransaction(void)
    {

    }


    void Panel_SH8601Z::endTransaction(void)
    {

    }


    color_depth_t Panel_SH8601Z::setColorDepth(color_depth_t depth)
    {

        return lgfx::rgb565_2Byte;
    }


    void Panel_SH8601Z::setRotation(uint_fast8_t r)
    {

    }

    void Panel_SH8601Z::setInvert(bool invert)
    {

    }

    void Panel_SH8601Z::setSleep(bool flg)
    {

    }
    
    void Panel_SH8601Z::setPowerSave(bool flg)
    {

    }

    void Panel_SH8601Z::waitDisplay(void)
    {

    }

    bool Panel_SH8601Z::displayBusy(void)
    {

        return false;
    }

    void Panel_SH8601Z::writePixels(pixelcopy_t* param, uint32_t len, bool use_dma)
    {

    }

    void Panel_SH8601Z::writeBlock(uint32_t rawcolor, uint32_t len)
    {

    }

    void Panel_SH8601Z::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
    {

    }

    void Panel_SH8601Z::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
    {

    }

    void Panel_SH8601Z::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
    {

    }

    void Panel_SH8601Z::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
    {

    }

    uint32_t Panel_SH8601Z::readCommand(uint_fast16_t cmd, uint_fast8_t index, uint_fast8_t len)
    {
        return 0;
    }

    uint32_t Panel_SH8601Z::readData(uint_fast8_t index, uint_fast8_t len)
    {
        return 0;
    }

    void Panel_SH8601Z::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
    {

    }


//----------------------------------------------------------------------------
 }
}
