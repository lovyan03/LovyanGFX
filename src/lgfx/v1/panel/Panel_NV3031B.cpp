/*----------------------------------------------------------------------------/
 *  Lovyan GFX - Graphics library for embedded devices.
 *
 * Original Source:
 * https://github.com/lovyan03/LovyanGFX/
 *
 * Licence:
 * [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)
 *
 * Authors:
 * [lovyan03](https://twitter.com/lovyan03)
 *
 * Contributors:
 * [ciniml](https://github.com/ciniml)
 * [mongonta0716](https://github.com/mongonta0716)
 * [tobozo](https://github.com/tobozo)
 * [mverch67](https://github.com/mverch67)
 * /----------------------------------------------------------------------------*/

#if defined (ESP_PLATFORM)

#include "Panel_NV3031B.hpp"
#include <lgfx/v1/Bus.hpp>
#include <lgfx/v1/platforms/common.hpp>
#include <lgfx/v1/misc/pixelcopy.hpp>
#include <lgfx/v1/misc/colortype.hpp>
#include "driver/spi_master.h"
#include "esp_log.h"


namespace lgfx
{
    inline namespace v1
    {
        //----------------------------------------------------------------------------

        bool Panel_NV3031B::init(bool use_reset)
        {
            //ESP_LOGD("NV3031B","panel init %d", use_reset);
            if (!Panel_Device::init(use_reset)) {
                return false;
            }

            startWrite();

            // Flush QSPI bus with a NOP before sending real commands.
            cs_control(false);
            write_cmd(0x00);  // {0x02, 0x00, 0x00, 0x00} — 4-byte NOP frame only
            _bus->wait();
            cs_control(true);

            endWrite();
            run_init_cmds();

            return true;
        }

        /**
         * NV3031B initialization commands are sent in a custom way instead of using Panel_Device::command_list()
         * because NV3031B requires the 4-byte QSPI framing that our write_cmd() implements.
         * If Panel_Device::command_list() would have been virtual we could have overwritten it here.
         */
        void Panel_NV3031B::run_init_cmds(void)
        {
            // Send the full init table (flat: cmd, len[|CMD_INIT_DELAY], data...[, delay_ms], 0xFF 0xFF),
            // then DISPON with dummy parameter byte as required by NV303x.
            startWrite();
            const uint8_t* p = init_cmds;
            while (p[0] != 0xFF || p[1] != 0xFF)
            {
                uint8_t cmd       = p[0];
                uint8_t len       = p[1] & ~CMD_INIT_DELAY;
                bool    has_delay = (p[1] & CMD_INIT_DELAY) != 0;
                p += 2;
                cs_control(false);
                write_cmd(cmd);
                for (uint8_t i = 0; i < len; i++) {
                    _bus->writeCommand(p[i], 8);
                }
                _bus->wait();
                cs_control(true);
                p += len;
                if (has_delay) {
                    delay(*p++);
                }
            }
            endWrite();

            startWrite();
            cs_control(false);
            write_cmd(CMD_DISPON);
            _bus->writeCommand(0x00, 8);
            _bus->wait();
            cs_control(true);
            endWrite();
        }


        void Panel_NV3031B::update_madctl(void)
        {
            uint8_t r = _internal_rotation;
            uint8_t rgb_order = (_cfg.rgb_order ? CMD_MADCTL_RGB : CMD_MADCTL_BGR);
            switch (r)
            {
                case 1:
                    r = CMD_MADCTL_MY | CMD_MADCTL_MV | rgb_order;
                    break;
                case 2:
                    r = rgb_order;
                    break;
                case 3:
                    r = CMD_MADCTL_MX | CMD_MADCTL_MV | rgb_order;
                    break;
                default: // case 0:
                    r = CMD_MADCTL_MX | CMD_MADCTL_MY | rgb_order;
                    break;
            }

            startWrite();
            cs_control(false);
            this->write_cmd(CMD_MADCTL);
            _bus->writeCommand(r, 8);
            _bus->wait();
            cs_control(true);
            endWrite();
        }


        void Panel_NV3031B::setInvert(bool invert)
        {
            startWrite();
            cs_control(false);
            write_cmd(invert ? CMD_INVON : CMD_INVOFF);
            _bus->wait();
            cs_control(true);
            endWrite();
        }


        void Panel_NV3031B::setSleep(bool flg)
        {
            if (flg) {
                startWrite();
                cs_control(false);
                write_cmd(CMD_SLPIN);
                _bus->wait();
                cs_control(true);
                endWrite();
            } else {
                // Software reset: resets all NV3031B registers to power-on defaults,
                // equivalent to a hardware RST pulse.  This is the only reliable way
                // to eliminate the 1-pixel column shift that appears after SLPOUT —
                // no combination of delays, NOP flushes, or register restores fixes it.
                startWrite();
                cs_control(false);
                write_cmd(CMD_SWRESET);
                _bus->wait();
                cs_control(true);
                endWrite();
                delay(CMD_RST_DELAY);
                run_init_cmds();
                _has_align_data = false;
                update_madctl();
            }
        }


        void Panel_NV3031B::setPowerSave(bool flg)
        {
            startWrite();
            cs_control(false);
            write_cmd(flg ? CMD_DISPOFF : CMD_DISPON);
            if (!flg) {
                // NV303x requires a dummy 0x00 parameter byte after CMD_DISPON.
                _bus->writeCommand(0x00, 8);
            }
            _bus->wait();
            cs_control(true);
            endWrite();
        }


        void Panel_NV3031B::waitDisplay(void)
        {
        }


        bool Panel_NV3031B::displayBusy(void)
        {
            return false;
        }


        color_depth_t Panel_NV3031B::setColorDepth(color_depth_t depth)
        {
            // MIPI DCS COLMOD values: 0x55 = RGB565 (16bpp), 0x66 = RGB666 (18bpp)
            uint8_t cmd_send = 0;
            if (depth == rgb565_2Byte) {
                cmd_send = 0x55;
            } else if (depth == rgb666_3Byte) {
                cmd_send = 0x66;
            } else {
                return _write_depth;
            }
            _write_depth = depth;

            startWrite();
            cs_control(false);
            write_cmd(CMD_COLMOD);
            _bus->writeCommand(cmd_send, 8);
            _bus->wait();
            cs_control(true);
            endWrite();

            return _write_depth;
        }


        void Panel_NV3031B::write_cmd(uint8_t cmd)
        {
            uint8_t cmd_buffer[4] = {0x02, 0x00, 0x00, 0x00};
            cmd_buffer[2] = cmd;
            for (int i = 0; i < 4; i++) {
                _bus->writeCommand(cmd_buffer[i], 8);
            }
        }


        void Panel_NV3031B::start_qspi()
        {
            cs_control(false);
            _bus->writeCommand(0x32, 8);
            _bus->writeCommand(0x00, 8);
            _bus->writeCommand(0x2C, 8);
            _bus->writeCommand(0x00, 8);
            _bus->wait();
        }

        void Panel_NV3031B::end_qspi()
        {
            cs_control(true);
        }


        void Panel_NV3031B::beginTransaction(void)
        {
            //ESP_LOGD("NV3031B","beginTransaction");
            if (_in_transaction) return;
            _in_transaction = true;
            _bus->beginTransaction();
        }


        void Panel_NV3031B::endTransaction(void)
        {
            //ESP_LOGD("NV3031B","endTransaction");
            if (!_in_transaction) return;
            _in_transaction = false;

            if (_has_align_data)
            {
                _has_align_data = false;
                _bus->writeData(0, 8);
            }

            _bus->wait();
            cs_control(true);
            _bus->endTransaction();
        }


        void Panel_NV3031B::write_bytes(const uint8_t* data, uint32_t len, bool use_dma)
        {
            start_qspi();
            _bus->writeBytes(data, len, true, use_dma);
            _bus->wait();
            end_qspi();
        }


        void Panel_NV3031B::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
        {
            //ESP_LOGD("NV3031B","setWindow %d %d %d %d", xs, ys, xe, ye);
            if ((xe - xs) >= _width)  { xs = 0; xe = _width  - 1; }
            if ((ye - ys) >= _height) { ys = 0; ye = _height - 1; }

            cs_control(false);
            write_cmd(CMD_CASET);
            _bus->writeCommand(xs >> 8, 8);
            _bus->writeCommand(xs & 0xFF, 8);
            _bus->writeCommand(xe >> 8, 8);
            _bus->writeCommand(xe & 0xFF, 8);
            _bus->wait();
            cs_control(true);

            cs_control(false);
            write_cmd(CMD_RASET);
            _bus->writeCommand(ys >> 8, 8);
            _bus->writeCommand(ys & 0xFF, 8);
            _bus->writeCommand(ye >> 8, 8);
            _bus->writeCommand(ye & 0xFF, 8);
            _bus->wait();
            cs_control(true);

            cs_control(false);
            write_cmd(CMD_RAMWR);
            _bus->wait();
            cs_control(true);
        }


        void Panel_NV3031B::writeBlock(uint32_t rawcolor, uint32_t len)
        {
            start_qspi();
            _bus->writeDataRepeat(rawcolor, _write_bits, len);
            _bus->wait();
            end_qspi();
        }


        void Panel_NV3031B::writePixels(pixelcopy_t* param, uint32_t len, bool use_dma)
        {
            //ESP_LOGD("NV3031B","writePixels %ld %d", len, use_dma);
            start_qspi();

            if (param->no_convert) {
                _bus->writeBytes(reinterpret_cast<const uint8_t*>(param->src_data), len * _write_bits >> 3, true, use_dma);
            } else {
                _bus->writePixels(param, len);
            }
            if (_cfg.dlen_16bit && (_write_bits & 15) && (len & 1)) {
                _has_align_data = !_has_align_data;
            }

            _bus->wait();
            end_qspi();
        }


        void Panel_NV3031B::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
        {
            //ESP_LOGD("NV3031B","drawPixelPreclipped %d %d 0x%lX", x, y, rawcolor);
            setWindow(x, y, x, y);
            if (_cfg.dlen_16bit) { _has_align_data = (_write_bits & 15); }

            start_qspi();
            _bus->writeData(rawcolor, _write_bits);
            _bus->wait();
            end_qspi();
        }


        void Panel_NV3031B::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
        {
            //ESP_LOGD("NV3031B","writeFillRectPreclipped %d %d %d %d 0x%lX", x, y, w, h, rawcolor);
            uint32_t len = w * h;
            uint_fast16_t xe = w + x - 1;
            uint_fast16_t ye = y + h - 1;

            setWindow(x, y, xe, ye);

            start_qspi();
            _bus->writeDataRepeat(rawcolor, _write_bits, len);
            _bus->wait();
            end_qspi();
        }


        void Panel_NV3031B::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
        {
            //ESP_LOGD("NV3031B","writeImage %d %d %d %d %d", x, y, w, h, use_dma);
            auto bytes = param->dst_bits >> 3;
            auto src_x = param->src_x;

            if (param->transp == pixelcopy_t::NON_TRANSP)
            {
                if (param->no_convert)
                {
                    auto wb = w * bytes;
                    uint32_t i = (src_x + param->src_y * param->src_bitwidth) * bytes;
                    auto src = &((const uint8_t*)param->src_data)[i];
                    setWindow(x, y, x + w - 1, y + h - 1);
                    if (param->src_bitwidth == w || h == 1)
                    {
                        write_bytes(src, wb * h, use_dma);
                    }
                    else
                    {
                        auto add = param->src_bitwidth * bytes;
                        if (use_dma)
                        {
                            if (_cfg.dlen_16bit && ((wb * h) & 1))
                            {
                                _has_align_data = !_has_align_data;
                            }
                            do
                            {
                                _bus->addDMAQueue(src, wb);
                                src += add;
                            } while (--h);
                            _bus->execDMAQueue();
                        }
                        else
                        {
                            do
                            {
                                write_bytes(src, wb, false);
                                src += add;
                            } while (--h);
                        }
                    }
                }
                else
                {
                    if (!_bus->busy())
                    {
                        static constexpr uint32_t WRITEPIXELS_MAXLEN = 32767;

                        setWindow(x, y, x + w - 1, y + h - 1);
                        bool nogap = (h == 1) || (param->src_y32_add == 0 && ((param->src_bitwidth << pixelcopy_t::FP_SCALE) == (w * param->src_x32_add)));
                        if (nogap && (w * h <= WRITEPIXELS_MAXLEN))
                        {
                            writePixels(param, w * h, use_dma);
                        }
                        else
                        {
                            uint_fast16_t h_step = nogap ? WRITEPIXELS_MAXLEN / w : 1;
                            uint_fast16_t h_len = (h_step > 1) ? ((h - 1) % h_step) + 1 : 1;
                            writePixels(param, w * h_len, use_dma);
                            if (h -= h_len)
                            {
                                param->src_y += h_len;
                                do
                                {
                                    param->src_x = src_x;
                                    writePixels(param, w * h_step, use_dma);
                                    param->src_y += h_step;
                                } while (h -= h_step);
                            }
                        }
                    }
                    else
                    {
                        size_t wb = w * bytes;
                        auto buf = get_dma_buffer_checked(wb);
                        if (!buf) { return; }
                        param->fp_copy(buf, 0, w, param);
                        setWindow(x, y, x + w - 1, y + h - 1);
                        write_bytes(buf, wb, true);
                        _has_align_data = (_cfg.dlen_16bit && (_write_bits & 15) && (w & h & 1));
                        while (--h)
                        {
                            param->src_x = src_x;
                            param->src_y++;
                            buf = get_dma_buffer_checked(wb);
                            if (!buf) { return; }
                            param->fp_copy(buf, 0, w, param);
                            write_bytes(buf, wb, true);
                        }
                    }
                }
            }
            else
            {
                h += y;
                uint32_t wb = w * bytes;
                do
                {
                    uint32_t i = 0;
                    while (w != (i = param->fp_skip(i, w, param)))
                    {
                        auto buf = get_dma_buffer_checked(wb);
                        if (!buf) { return; }
                        int32_t len = param->fp_copy(buf, 0, w - i, param);
                        setWindow(x + i, y, x + i + len - 1, y);
                        write_bytes(buf, len * bytes, true);
                        if (w == (i += len)) break;
                    }
                    param->src_x = src_x;
                    param->src_y++;
                } while (++y != (int)h);
            }
        }


        uint32_t Panel_NV3031B::readCommand(uint_fast16_t cmd, uint_fast8_t index, uint_fast8_t len)
        {
            (void)cmd; (void)index; (void)len;
            return 0;
        }

        uint32_t Panel_NV3031B::readData(uint_fast8_t index, uint_fast8_t len)
        {
            (void)index; (void)len;
            return 0;
        }

        void Panel_NV3031B::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
        {
            (void)x; (void)y; (void)w; (void)h; (void)dst; (void)param;
        }


        //----------------------------------------------------------------------------
    }
}


#endif
