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

#if defined (ESP_PLATFORM)

#include "Panel_NV3041A.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"
#include "../misc/colortype.hpp"
#include "driver/spi_master.h"
#include "esp_log.h"


/**
 * @brief Bug list (inherited from Panel_SH8601Z)
 *
 *  > Write image (pushSprite) works fine, bugs down below are from writing directly
 *
 *  1> Write function is block even with DMA (manual CS wait data)
 *  2> In spi 40MHz draw vertical line incomplete, but 10MHz OK (Likely because my dupont line connection)
 *  3> After implement write/draw pixel funcs, "testFilledRects" stucks sometime, acts differently to the different sck freq
 *  4> Haven't found the way to set rotation by reg
 */


namespace lgfx
{
    inline namespace v1
    {
        //----------------------------------------------------------------------------

        /* Panel init */
        bool Panel_NV3041A::init(bool use_reset)
        {
            // ESP_LOGD("NV3041A","pannel init %d", use_reset);

            if (!Panel_Device::init(use_reset)) {
                return false;
            }

            startWrite();
            cs_control(false);
            write_cmd(CMD_SWRESET);
            delay(150);
            _bus->wait();
            cs_control(true);
            endWrite();

            startWrite();
            for(int i=0;i<sizeof(init_cmds);i+=2)
            {
                cs_control(false);
                this->write_cmd(init_cmds[i]);
                _bus->writeCommand(init_cmds[i+1], 8);
                _bus->wait();
                cs_control(true);
                delay(1);
            }
            endWrite();
            delay(120);

            startWrite();
            cs_control(false);
            this->write_cmd(CMD_DISPON);
            _bus->writeCommand(0x00, 8);
            _bus->wait();
            cs_control(true);
            endWrite();

            return true;
        }


        void Panel_NV3041A::update_madctl(void)
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



        void Panel_NV3041A::setInvert(bool invert)
        {
            // ESP_LOGD("NV3041A","setInvert %d", invert);

            cs_control(false);

            if (invert) {
                /* Inversion On */
                write_cmd(CMD_INVON);
            }
            else {
                /* Inversion Off */
                write_cmd(CMD_INVOFF);
            }
            _bus->wait();

            cs_control(true);
        }


        void Panel_NV3041A::setSleep(bool flg)
        {
            // ESP_LOGD("NV3041A","setSleep %d", flg);

            cs_control(false);

            if (flg) {
                /* Sleep in */
                write_cmd(CMD_SLPIN);
            }
            else {
                /* Sleep out */
                write_cmd(CMD_SLPOUT);
                delay(150);
            }
            _bus->wait();

            cs_control(true);
        }


        void Panel_NV3041A::setPowerSave(bool flg)
        {
            // ESP_LOGD("NV3041A","setPowerSave");
        }


        void Panel_NV3041A::waitDisplay(void)
        {
            // ESP_LOGD("NV3041A","waitDisplay");
        }


        bool Panel_NV3041A::displayBusy(void)
        {
            // ESP_LOGD("NV3041A","displayBusy");
            return false;
        }


        color_depth_t Panel_NV3041A::setColorDepth(color_depth_t depth)
        {
            // ESP_LOGD("NV3041A","setColorDepth %d", depth);

            /* 0x00: 16bit/pixel */
            /* 0x01: 18bit/pixel */
            uint8_t cmd_send = 0;
            if (depth == rgb565_2Byte) {
                cmd_send = 0x00;
            }
            else if (depth == rgb666_3Byte) {
                cmd_send = 0x01;
            }
            else {
                return _write_depth;
            }
            _write_depth = depth;

            /* Set interface Pixel Format */
            startWrite();

            cs_control(false);
            write_cmd(CMD_COLMOD);
            _bus->writeCommand(cmd_send, 8);
            _bus->wait();
            cs_control(true);

            endWrite();

            return _write_depth;
        }


        void Panel_NV3041A::write_cmd(uint8_t cmd)
        {
            uint8_t cmd_buffer[4] = {0x02, 0x00, 0x00, 0x00};
            cmd_buffer[2] = cmd;
            // _bus->writeBytes(cmd_buffer, 4, 0, false);
            for (int i = 0; i < 4; i++) {
                _bus->writeCommand(cmd_buffer[i], 8);
            }
        }


        void Panel_NV3041A::start_qspi()
        {
            /* Begin QSPI */
            cs_control(false);
            _bus->writeCommand(0x32, 8);
            _bus->writeCommand(0x00, 8);
            _bus->writeCommand(0x2C, 8);
            _bus->writeCommand(0x00, 8);
            _bus->wait();
        }

        void Panel_NV3041A::end_qspi()
        {
            /* Stop QSPI */
            _bus->writeCommand(0x32, 8);
            _bus->writeCommand(0x00, 8);
            _bus->writeCommand(0x00, 8);
            _bus->writeCommand(0x00, 8);
            _bus->wait();
            cs_control(true);
        }


        void Panel_NV3041A::beginTransaction(void)
        {
            // ESP_LOGD("NV3041A","beginTransaction");
            if (_in_transaction) return;
            _in_transaction = true;
            _bus->beginTransaction();
        }


        void Panel_NV3041A::endTransaction(void)
        {
            // ESP_LOGD("NV3041A","endTransaction");
            // if (!_in_transaction) return;
            // _in_transaction = false;
            // _bus->endTransaction();

            if (!_in_transaction) return;
            _in_transaction = false;

            if (_has_align_data)
            {
                _has_align_data = false;
                _bus->writeData(0, 8);
            }

            _bus->endTransaction();
        }


        void Panel_NV3041A::write_bytes(const uint8_t* data, uint32_t len, bool use_dma)
        {
            start_qspi();
            _bus->writeBytes(data, len, true, use_dma);
            _bus->wait();
            end_qspi();
        }


        void Panel_NV3041A::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
        {
            // ESP_LOGD("NV3041A","setWindow %d %d %d %d", xs, ys, xe, ye);

            /* Set limit */
            if ((xe - xs) >= _width) { xs = 0; xe = _width - 1; }
            if ((ye - ys) >= _height) { ys = 0; ye = _height - 1; }

            /* Set Column Start Address */
            cs_control(false);
            write_cmd(CMD_CASET);
            _bus->writeCommand(xs >> 8, 8);
            _bus->writeCommand(xs & 0xFF, 8);
            _bus->writeCommand(xe >> 8, 8);
            _bus->writeCommand(xe & 0xFF, 8);
            _bus->wait();
            cs_control(true);

            /* Set Row Start Address */
            cs_control(false);
            write_cmd(CMD_RASET);
            _bus->writeCommand(ys >> 8, 8);
            _bus->writeCommand(ys & 0xFF, 8);
            _bus->writeCommand(ye >> 8, 8);
            _bus->writeCommand(ye & 0xFF, 8);
            _bus->wait();
            cs_control(true);

            /* Memory Write */
            cs_control(false);
            write_cmd(CMD_RAMWR);
            _bus->wait();
            cs_control(true);
        }


        void Panel_NV3041A::writeBlock(uint32_t rawcolor, uint32_t len)
        {
            // ESP_LOGD("NV3041A","writeBlock 0x%lx %ld", rawcolor, len);

            /* Push color */
            start_qspi();
            _bus->writeDataRepeat(rawcolor, _write_bits, len);
            _bus->wait();
            end_qspi();
        }




        void Panel_NV3041A::writePixels(pixelcopy_t* param, uint32_t len, bool use_dma)
        {
            // ESP_LOGD("NV3041A","writePixels %ld %d", len, use_dma);

            start_qspi();

            if (param->no_convert) {
                _bus->writeBytes(reinterpret_cast<const uint8_t*>(param->src_data), len * _write_bits >> 3, true, use_dma);
            }
            else {
                _bus->writePixels(param, len);
            }
            if (_cfg.dlen_16bit && (_write_bits & 15) && (len & 1)) {
                _has_align_data = !_has_align_data;
            }

            _bus->wait();
            end_qspi();
        }


        void Panel_NV3041A::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
        {
            // ESP_LOGD("NV3041A","drawPixelPreclipped %d %d 0x%lX", x, y, rawcolor);

            setWindow(x,y,x,y);
            if (_cfg.dlen_16bit) { _has_align_data = (_write_bits & 15); }

            start_qspi();

            _bus->writeData(rawcolor, _write_bits);

            _bus->wait();
            end_qspi();
        }


        void Panel_NV3041A::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
        {
            // ESP_LOGD("NV3041A","writeFillRectPreclipped %d %d %d %d 0x%lX", x, y, w, h, rawcolor);

            uint32_t len = w * h;
            uint_fast16_t xe = w + x - 1;
            uint_fast16_t ye = y + h - 1;

            setWindow(x,y,xe,ye);
            // if (_cfg.dlen_16bit) { _has_align_data = (_write_bits & 15) && (len & 1); }

            start_qspi();
            _bus->writeDataRepeat(rawcolor, _write_bits, len);
            _bus->wait();
            end_qspi();
        }



        void Panel_NV3041A::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
        {
            // ESP_LOGD("NV3041A","writeImage %d %d %d %d %d", x, y, w, h, use_dma);
            // use_dma = false;

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
                        // bool nogap = (param->src_bitwidth == w || h == 1);
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
                        auto buf = _bus->getDMABuffer(wb);
                        param->fp_copy(buf, 0, w, param);
                        setWindow(x, y, x + w - 1, y + h - 1);
                        write_bytes(buf, wb, true);
                        _has_align_data = (_cfg.dlen_16bit && (_write_bits & 15) && (w & h & 1));
                        while (--h)
                        {
                            param->src_x = src_x;
                            param->src_y++;
                            buf = _bus->getDMABuffer(wb);
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
                        auto buf = _bus->getDMABuffer(wb);
                        int32_t len = param->fp_copy(buf, 0, w - i, param);
                        setWindow(x + i, y, x + i + len - 1, y);
                        write_bytes(buf, len * bytes, true);
                        if (w == (i += len)) break;
                    }
                    param->src_x = src_x;
                    param->src_y++;
                } while (++y != h);
            }
        }




        uint32_t Panel_NV3041A::readCommand(uint_fast16_t cmd, uint_fast8_t index, uint_fast8_t len)
        {
            // ESP_LOGD("NV3041A","readCommand");
            return 0;
        }

        uint32_t Panel_NV3041A::readData(uint_fast8_t index, uint_fast8_t len)
        {
            // ESP_LOGD("NV3041A","readData");
            return 0;
        }

        void Panel_NV3041A::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
        {
            // ESP_LOGD("NV3041A","readRect");
        }


        //----------------------------------------------------------------------------
    }
}


#endif
