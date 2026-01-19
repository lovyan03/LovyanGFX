/*----------------------------------------------------------------------------/
 * Lovyan GFX - Graphics library for embedded devices.
 *
 O *riginal Source:
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

#include "Panel_LCD.hpp"

namespace lgfx
{
    inline namespace v1
    {
        //----------------------------------------------------------------------------

        struct Panel_ST7789P3  : public Panel_LCD
        {
            Panel_ST7789P3(void)
            {
                _cfg.panel_height = _cfg.memory_height = 320;
                _cfg.dummy_read_pixel = 16;
            }

        protected:

            const uint8_t* getInitCommands(uint8_t listno) const override {

                static constexpr uint8_t list0[] = {
                    0x11, 0+CMD_INIT_DELAY, 120,
                    0x36, 1, 0x00,
                    0x3A, 1, 0x05,
                    0xB2, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33,
                    0xb7, 1, 0x56,
                    0xbb, 1, 0x1d,
                    0xc0, 1, 0x2c,
                    0xc2, 1, 0x01,
                    0xc3, 1, 0x0f,
                    0xc6, 1, 0x0f,
                    0xd0, 1, 0xa7,
                    0xd0, 2, 0xa4, 0xa1,
                    0xd6, 1, 0xa1,
                    0xe0, 14, 0xF0, 0x02, 0x07, 0x05, 0x06, 0x14, 0x2F, 0x54, 0x46, 0x38, 0x13, 0x11, 0x2E, 0x35,
                    0xe1, 14, 0xF0, 0x08, 0x0C, 0x0C, 0x09, 0x05, 0x2F, 0x43, 0x46, 0x36, 0x10, 0x12, 0x2C, 0x32,
                    0x21, 0,
                    0x29, 0,
                    0x2c, 0+CMD_INIT_DELAY, 100,
                    0x2a, 4, 0x00, 0x00, 0x01, 0x3f,
                    0x2b, 4, 0x00, 0x00, 0x00, 0xef,
                    0x2c, 0,
                    0xFF,0xFF, // end
                };
                switch (listno) {
                    case 0: return list0;
                    default: return nullptr;
                }
            }
        };

        //----------------------------------------------------------------------------
    }
}
