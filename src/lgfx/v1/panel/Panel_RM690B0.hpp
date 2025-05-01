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

        // AMOLED Panel used by LilyGO T4-S3

        struct Panel_RM690B0 : public Panel_AMOLED
        {
        public:

            Panel_RM690B0(void)
            {
              _cfg.memory_width  = _cfg.panel_width  = 452;
              _cfg.memory_height = _cfg.panel_height = 600;
              _write_depth = color_depth_t::rgb565_2Byte;
              _read_depth = color_depth_t::rgb565_2Byte;
            }

            const uint8_t* getInitCommands(uint8_t listno) const override
            {
              static constexpr uint8_t list0[] = {
                0x11, 0+CMD_INIT_DELAY, 150, // Sleep out
                0xfe, 1, 0x20, // SET PAGE
                0x26, 1, 0x0a, // MIPI OFF
                0x24, 1, 0x80, // SPI write RAM
                0x5a, 1, 0x51, //! 230918:SWIRE FOR BV6804
                0x5b, 1, 0x2e, //! 230918:SWIRE FOR BV6804
                0xfe, 1, 0x00, // SET PAGE

                0x2a, 4, 0x00, 0x10, 0x01, 0xd1, // SET COLUMN START ADRESS SC = 0x0010 = 16 and EC = 0x01D1 = 465 (450 columns but an 16 offset)
                0x2b, 4, 0x00, 0x00, 0x02, 0x57, // SET ROW START ADRESS SP = 0 and EP = 0x256 = 599 (600 lines)

                0xc2, 1, 0xA1, // Set DSI Mode; 0x00 = Internal Timmings, 0xA1 = 1010 0001, first bit = SPI interface write RAM enable

                0x3a, 1+CMD_INIT_DELAY, 0x55, 20, // Interface Pixel Format, 0x55=16bit/pixel
                0x51, 1, 0x01, // display brightness dark (max = 0xff)
                0x29, 0+CMD_INIT_DELAY, 200, // display on
                0x51, 1, 0xd0, // display brightness (max = 0xff)
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
