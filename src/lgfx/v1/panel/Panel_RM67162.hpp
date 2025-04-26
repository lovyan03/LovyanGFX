/*----------------------------------------------------------------------------/
 *  Lovyan GFX - Graphics library for embedded devices.
 *
 * Original Source:
 * https://github.com/lovyan03/LovyanGFX/
 *
 * Licence:
 * [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)
 *
 * Author:
 * [lovyan03](https://twitter.com/lovyan03)
 *
 * Contributors:
 * [ciniml](https://github.com/ciniml)
 * [mongonta0716](https://github.com/mongonta0716)
 * [tobozo](https://github.com/tobozo)
 * /----------------------------------------------------------------------------*/
#pragma once

#if defined (ESP_PLATFORM)

#include "Panel_AMOLED.hpp"
#include "Panel_FrameBufferBase.hpp"
#include "../platforms/common.hpp"
#include "../platforms/device.hpp"

#if defined LGFX_USE_QSPI

namespace lgfx
{
    inline namespace v1
    {
        //----------------------------------------------------------------------------

        // Panel used by LilyGO T-Display-S3-AMOLED

        struct Panel_RM67162 : public Panel_AMOLED
        {
        public:

            Panel_RM67162(void)
            {
              _cfg.memory_width  = _cfg.panel_width  = 240;
              _cfg.memory_height = _cfg.panel_height = 536;
              _write_depth = color_depth_t::rgb565_2Byte;
              _read_depth = color_depth_t::rgb565_2Byte;
              _cfg.dummy_read_pixel = 1;
            }

            const uint8_t* getInitCommands(uint8_t listno) const override
            {
              static constexpr uint8_t list0[] = {
                0xFE, 1, 0x00, // {0xFE, {0x00}, 0x01}, //SET PAGE 00H
                0x11, 0+CMD_INIT_DELAY, 150, // Sleep Out
                0xfe, 1, 0x05, // {0xFE, {0x05}, 0x01}, //SET PAGE 05H
                0x05, 1, 0x05, // {0x05, {0x05}, 0x01}, //OVSS control set elvss -3.95v
                0xFE, 1, 0x01, // {0xFE, {0x01}, 0x01}, //SET PAGE 01H
                0x73, 1, 0x25, // {0x73, {0x25}, 0x01}, //set OVSS voltage level.= -4.0V
                0xFE, 1, 0x00, // {0xFE, {0x00}, 0x01}, //SET PAGE 00H
                // 0x44, 2, 0x01, 0x66, // Set_Tear_Scanline
                // 0x35, 1, 0x00, // TE ON
                // 0x34, 1, 0x00, // TE OFF
                // 0x36, 1, 0x00, // Scan Direction Control
                0x36, 1, 0x60, //
                0x3a, 1, 0x55, // Interface Pixel Format: 16bit/pixel
                // 0x3a, 1, 0x66, // Interface Pixel Format: 18bit/pixel
                // 0x3a, 1, 0x77, // Interface Pixel Format: 24bit/pixel
                0x51, 1, 0x00, // display brightness dark (max = 0xff)
                // 0x51, 1, 0x01, // display brightness dark (max = 0xff)
                0x29, 0+CMD_INIT_DELAY, 150, // {0x29, {0x00}, 0x80}, // Display on
                0x51, 1, 0xAF, // display brightness on (max = 0xff)
                0xff, 0xff // end
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


#endif
#endif
