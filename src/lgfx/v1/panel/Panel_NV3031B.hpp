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


            // Flat init table: cmd, len[|CMD_INIT_DELAY], data...[, delay_ms], ..., 0xFF, 0xFF
            // CMD_INIT_DELAY ORed into len means a delay_ms byte follows the data bytes.
            static constexpr uint8_t CMD_INIT_DELAY = 0x80;

            static constexpr uint8_t init_cmds[] =
            {
                0xFD, 2, 0x06, 0x08,                             // Cmd2 Enable: unlock factory registers
                0x60, 1, 0x0C,                                   // Source timing adjust
                0x61, 2, 0x07, 0x04,                             // Gate timing control
                0xB4, 1, 0x01,                                   // Display inversion: 1-dot column
                0xB1, 3, 0x0F, 0x02, 0x03,                       // Frame rate control
                0xB5, 4, 0x02, 0x02, 0x0A, 0x14,                 // Blanking porch control
                0xB6, 5, 0x44, 0x01, 0x9F, 0x00, 0x02,           // Display function control
                0xDF, 1, 0x11,                                   // Bias/oscillator trim
                0x67, 1, 0x21,                                   // Bias current control
                0x68, 4, 0x90, 0x4F, 0x27, 0x21,                 // VCOM / power control
                0xE1, 2, 0x20, 0x69,                             // Positive gamma voltage
                0xE4, 2, 0x69, 0x20,                             // Negative gamma voltage
                0xE2, 6, 0x10, 0x12, 0x12, 0x30, 0x39, 0x3F,     // Positive gamma mid-tones
                0xE5, 6, 0x3F, 0x33, 0x2D, 0x12, 0x12, 0x10,     // Negative gamma mid-tones
                0xE0, 8, 0x06, 0x06, 0x0B, 0x12, 0x11, 0x11, 0x0E, 0x19, // Positive gamma curve
                0xE3, 8, 0x19, 0x13, 0x14, 0x14, 0x14, 0x12, 0x08, 0x05, // Negative gamma curve
                0xE6, 2, 0x00, 0xFF,                             // AVDD/AVCL slope control
                0xE7, 6, 0x01, 0x04, 0x03, 0x03, 0x00, 0x12,     // Source output / EQ timing
                0xE8, 3, 0x00, 0x70, 0x00,                       // Source driver output level
                0xEC, 1, 0x54,                                   // Gate EQ / charge-pump timing
                0xFD, 2, 0xFA, 0xFC,                             // Cmd2 Enable: lock factory registers
                0x3A, 1, 0x55,                                   // COLMOD: 16 bpp RGB565
                0x11, 0|CMD_INIT_DELAY, 100,                     // Sleep Out + 100 ms delay
                0xFF, 0xFF                                       // end of table
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
            void run_init_cmds(void);
        };

        //----------------------------------------------------------------------------
    }
}


#endif
