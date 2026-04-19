/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)
 [mverch67](https://github.com/mverch67) - EK79007 support

Info: 
  Panel_EK79007 - LovyanGFX Panel_DSI subclass for the EK79007AD MIPI-DSI controller
  Initialization sequence extracted from the Espressif esp_lcd_ek79007 component (Apache-2.0)
  https://github.com/espressif/esp-iot-solution/blob/297c8a5d0c1d50fb92726b3998724630bcba3a57/components/display/lcd/esp_lcd_ek79007/esp_lcd_ek79007.c#L114

/----------------------------------------------------------------------------*/
#pragma once

#include "Panel_DSI.hpp"
#if SOC_MIPI_DSI_SUPPORTED

namespace lgfx
{
inline namespace v1
{
//----------------------------------------------------------------------------

struct Panel_EK79007 : public Panel_DSI {
  protected:
    const uint8_t *getInitParams(size_t listno) const override
    {
        static constexpr uint8_t list0[] = {
            2,    0xB2, 0x10, // PAD_CONTROL: 2-lane DSI
            2,    0x80, 0x8B, // Vendor analogue trim
            2,    0x81, 0x78,
            2,    0x82, 0x84,
            2,    0x83, 0x88,
            2,    0x84, 0xA8,
            2,    0x85, 0xE3,
            2,    0x86, 0x88,
            1,    0x11, // SLEEP OUT (no parameters)
            0,    // end-of-list
        };

        switch (listno) {
        case 0:
            return list0;
        default:
            return nullptr;
        }
    }

    size_t getInitDelay(size_t listno) const override
    {
        switch (listno) {
        case 0:
            return 120; // 120 ms after Sleep Out
        default:
            return 0;
        }
    }
};

//----------------------------------------------------------------------------
} // namespace v1
} // namespace lgfx

#endif // SOC_MIPI_DSI_SUPPORTED
