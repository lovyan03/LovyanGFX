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
 * [mverch67](https://github.com/mverch67)
 * /----------------------------------------------------------------------------*/
#pragma once

#if defined (ESP_PLATFORM)

#include <lgfx/v1/panel/Panel_LCD.hpp>


namespace lgfx
{
    inline namespace v1
    {
        //----------------------------------------------------------------------------

        struct Panel_NV3031B : public Panel_LCD
        {
        protected:

            static constexpr uint8_t CMD_RST_DELAY    = 120 ;   ///< delay ms wait for reset finish
            static constexpr uint8_t CMD_SLPIN_DELAY  = 120 ;   ///< delay ms wait for sleep in finish
            static constexpr uint8_t CMD_SLPOUT_DELAY = 120 ;   ///< delay ms wait for sleep out finish
            static constexpr uint8_t CMD_NOP          = 0x00;
            static constexpr uint8_t CMD_SWRESET      = 0x01;
            static constexpr uint8_t CMD_SLPIN        = 0x10;
            static constexpr uint8_t CMD_SLPOUT       = 0x11;
            static constexpr uint8_t CMD_INVOFF       = 0x20;
            static constexpr uint8_t CMD_INVON        = 0x21;
            static constexpr uint8_t CMD_DISPOFF      = 0x28;
            static constexpr uint8_t CMD_DISPON       = 0x29;
            static constexpr uint8_t CMD_CASET        = 0x2A;
            static constexpr uint8_t CMD_RASET        = 0x2B;
            static constexpr uint8_t CMD_RAMWR        = 0x2C;
            static constexpr uint8_t CMD_MADCTL       = 0x36;
            static constexpr uint8_t CMD_COLMOD       = 0x3A;
            static constexpr uint8_t CMD_GATEON       = 0x51;
            static constexpr uint8_t CMD_MADCTL_MY    = 0x80;
            static constexpr uint8_t CMD_MADCTL_MX    = 0x40;
            static constexpr uint8_t CMD_MADCTL_MV    = 0x20;
            static constexpr uint8_t CMD_MADCTL_ML    = 0x10;
            static constexpr uint8_t CMD_MADCTL_RGB   = 0x0 ;
            static constexpr uint8_t CMD_MADCTL_BGR   = 0x08;


            // Init sequence derived from esp_lcd_nv3031b.c vendor_specific_init_default.
            // Commands have variable-length data; see Panel_NV3031B::init() for the send loop.
            struct InitCmd {
                uint8_t  cmd;
                uint8_t  data[8];
                uint8_t  len;       // number of data bytes (0 = command only)
                uint16_t delay_ms;
            };

            static constexpr InitCmd init_cmds[] =
            {
                // 0xFD — Cmd2 Enable: password bytes 0x06,0x08 unlock factory register set (Cmd2)
                { 0xFD, { 0x06, 0x08 },                         2, 0   },
                // 0x60 — Source timing adjust: SDNL timing control value 0x0C
                { 0x60, { 0x0C },                                1, 0   },
                // 0x61 — Gate timing control: gate on/off timing (0x07 = rise, 0x04 = fall)
                { 0x61, { 0x07, 0x04 },                         2, 0   },
                // 0xB4 — Display inversion control: 0x01 = 1-dot inversion (column)
                { 0xB4, { 0x01 },                                1, 0   },
                // 0xB1 — Frame rate control (normal mode): DIVA=0x0F (osc divisor), RTNA=0x02 (line period), FPA=0x03 (front porch)
                { 0xB1, { 0x0F, 0x02, 0x03 },                   3, 0   },
                // 0xB5 — Blanking porch control: VFP=0x02, VBP=0x02, HBP_even=0x0A, HBP_odd=0x14
                { 0xB5, { 0x02, 0x02, 0x0A, 0x14 },             4, 0   },
                // 0xB6 — Display function control: ISC=0x44, SM=0x01, SS=0x9F, GS=0x00, REV=0x02
                { 0xB6, { 0x44, 0x01, 0x9F, 0x00, 0x02 },       5, 0   },
                // 0xDF — Vendor-specific (bias/oscillator trim): value 0x11
                { 0xDF, { 0x11 },                                1, 0   },
                // 0x67 — Vendor-specific (bias current control): value 0x21
                { 0x67, { 0x21 },                                1, 0   },
                // 0x68 — VCOM / power control: VCOM_H=0x90, VDV=0x4F, VCM_offset=0x27, VCOM_L=0x21
                { 0x68, { 0x90, 0x4F, 0x27, 0x21 },             4, 0   },
                // 0xE1 — Positive gamma voltage control: V63P=0x20, V0P=0x69
                { 0xE1, { 0x20, 0x69 },                         2, 0   },
                // 0xE4 — Negative gamma voltage control: V63N=0x69, V0N=0x20
                { 0xE4, { 0x69, 0x20 },                         2, 0   },
                // 0xE2 — Positive gamma correction (mid-tones): VP20,VP36,VP44,VP52,VP59,VP63
                { 0xE2, { 0x10, 0x12, 0x12, 0x30, 0x39, 0x3F }, 6, 0   },
                // 0xE5 — Negative gamma correction (mid-tones): VN20,VN36,VN44,VN52,VN59,VN63
                { 0xE5, { 0x3F, 0x33, 0x2D, 0x12, 0x12, 0x10 }, 6, 0   },
                // 0xE0 — Positive gamma curve (shadow/highlight): VP1..VP8
                { 0xE0, { 0x06, 0x06, 0x0B, 0x12, 0x11, 0x11, 0x0E, 0x19 }, 8, 0 },
                // 0xE3 — Negative gamma curve (shadow/highlight): VN1..VN8
                { 0xE3, { 0x19, 0x13, 0x14, 0x14, 0x14, 0x12, 0x08, 0x05 }, 8, 0 },
                // 0xE6 — Vendor-specific (power / AVDD/AVCL slope control): 0x00, 0xFF
                { 0xE6, { 0x00, 0xFF },                         2, 0   },
                // 0xE7 — Vendor-specific (source output / EQ timing): 6 bytes
                { 0xE7, { 0x01, 0x04, 0x03, 0x03, 0x00, 0x12 }, 6, 0   },
                // 0xE8 — Source driver output level / pre-charge control: 3 bytes
                { 0xE8, { 0x00, 0x70, 0x00 },                   3, 0   },
                // 0xEC — Vendor-specific (gate EQ / charge-pump timing): value 0x54
                { 0xEC, { 0x54 },                                1, 0   },
                // 0xFD — Cmd2 Enable: password bytes 0xFA,0xFC lock factory register set (Cmd2)
                { 0xFD, { 0xFA, 0xFC },                         2, 0   },
                // 0x3A — COLMOD (interface pixel format): 0x55 = 16 bpp RGB565
                { 0x3A, { 0x55 },                                1, 0   },
                // 0x11 — Sleep Out: exits sleep mode; 100 ms delay required before display on
                { 0x11, { },                                     0, 100 },
            };

        public:
            Panel_NV3031B(void)
            {
                _cfg.memory_width  = _cfg.panel_width  = 240;
                _cfg.memory_height = _cfg.panel_height = 320;
            }

            bool init(bool use_reset) override;
            void beginTransaction(void) override;
            void endTransaction(void) override;

            color_depth_t setColorDepth(color_depth_t depth) override;
            void setInvert(bool invert) override;
            void setSleep(bool flg) override;
            void setPowerSave(bool flg) override;

            void waitDisplay(void) override;
            bool displayBusy(void) override;

            void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override;
            void writeBlock(uint32_t rawcolor, uint32_t len) override;

            void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override;
            void drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
            void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
            void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;

            uint32_t readCommand(uint_fast16_t cmd, uint_fast8_t index, uint_fast8_t len) override;
            uint32_t readData(uint_fast8_t index, uint_fast8_t len) override;
            void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override;

        protected:
            bool _in_transaction = false;

            void update_madctl(void) override;
            void write_cmd(uint8_t cmd);
            void start_qspi();
            void end_qspi();
            void write_bytes(const uint8_t* data, uint32_t len, bool use_dma);
        };

        //----------------------------------------------------------------------------
    }
}


#endif
