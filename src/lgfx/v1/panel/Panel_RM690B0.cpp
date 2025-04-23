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

#include "Panel_RM690B0.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"
#include "../misc/colortype.hpp"
#include "driver/spi_master.h"
#include "esp_log.h"

#if defined LGFX_USE_QSPI


namespace lgfx
{
    inline namespace v1
    {

        //----------------------------------------------------------------------------


        void Panel_RM690B0::update_madctl()
        {
            uint8_t madctl = 0;
            switch (_rotation) {
                case 0:                break;
                case 1: madctl = 0x60; break;
                case 2: madctl = 0xc0; break;
                case 3: madctl = 0xa0; break;
            }

            startWrite();
            cs_control(false);
            write_cmd(0x36);
            _bus->writeCommand(madctl, 8);
            _bus->wait();
            cs_control(true);
            endWrite();
        }


        void Panel_RM690B0::command_list(const uint8_t *addr)
        {
            startWrite();
            for (;;)
            {                // For each command...
                uint8_t cmd = *addr++;
                uint8_t num = *addr++;   // Number of args to follow
                if (cmd == 0xff && num == 0xff) break;

                cs_control(false);

                this->write_cmd(cmd);  // Read, issue command
                uint_fast8_t ms = num & CMD_INIT_DELAY;       // If hibit set, delay follows args
                num &= ~CMD_INIT_DELAY;          // Mask out delay bit
                if (num) {
                    do
                    {   // For each argument...
                        _bus->writeCommand(*addr++, 8);
                    } while (--num);
                }

                _bus->wait();
                cs_control(true);

                if (ms) {
                    ms = *addr++;        // Read post-command delay time (ms)
                    delay( (ms==255 ? 500 : ms) );
                }
            }
            endWrite();
        }


        void Panel_RM690B0::write_cmd(uint8_t cmd)
        {
            uint8_t cmd_buffer[4] = {0x02, 0x00, 0x00, 0x00};
            cmd_buffer[2] = cmd;
            for (int i = 0; i < 4; i++) {
                _bus->writeCommand(cmd_buffer[i], 8);
            }
        }


        void Panel_RM690B0::start_qspi()
        {
            /* Begin QSPI */
            cs_control(false);
            _bus->writeCommand(0x32, 8);
            _bus->writeCommand(0x00, 8);
            _bus->writeCommand(0x2c, 8); // WRITE_MEMORY_START
            _bus->writeCommand(0x00, 8);
            _bus->wait();
        }

        void Panel_RM690B0::end_qspi()
        {
            /* Stop QSPI */
            _bus->writeCommand(0x32, 8);
            _bus->writeCommand(0x00, 8);
            _bus->writeCommand(0x00, 8); // NOP
            _bus->writeCommand(0x00, 8);
            _bus->wait();
            cs_control(true);
        }

        void Panel_RM690B0::write_bytes(const uint8_t* data, uint32_t len, bool use_dma)
        {
            start_qspi();
            _bus->writeBytes(data, len, true, use_dma);
            _bus->wait();
            end_qspi();
        }



        //----------------------------------------------------------------------------



        /* Panel init */
        bool Panel_RM690B0::init(bool use_reset)
        {
            // ESP_LOGD("RM690B0","pannel init %d", use_reset);

            if (!Panel_Device::init(use_reset)) {
                return false;
            }

            uint8_t cmds[] =
            {
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
                0xff, 0xff
            };

            this->command_list(cmds);

            return true;
        }



        void Panel_RM690B0::setBrightness(uint8_t brightness)
        {
            // ESP_LOGD("RM690B0","setBrightness %d", brightness);

            startWrite();
            /* Write Display Brightness	MAX_VAL=0XFF */
            cs_control(false);
            write_cmd(0x51);
            _bus->writeCommand(brightness, 8);
            _bus->wait();
            cs_control(true);
            endWrite();
        }


        void Panel_RM690B0::setRotation(uint_fast8_t r)
        {
            // ESP_LOGD("RM690B0","setRotation %d", r);

            r &= 7;
            _rotation = r;
            // offset_rotationを加算 (0~3:回転方向、 4:上下反転フラグ);
            _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

            auto ox = _cfg.offset_x;
            auto oy = _cfg.offset_y;
            auto pw = _cfg.panel_width;
            auto ph = _cfg.panel_height;
            auto mw = _cfg.memory_width;
            auto mh = _cfg.memory_height;
            if (_internal_rotation & 1)
            {
                std::swap(ox, oy);
                std::swap(pw, ph);
                std::swap(mw, mh);
            }
            _width  = pw;
            _height = ph;

            _colstart = ox;
            _rowstart = oy;

            // _colstart = (_internal_rotation & 2)
            //         ? mw - (pw + ox) : ox;

            // _rowstart = ((1 << _internal_rotation) & 0b10010110) // case 1:2:4:7
            //         ? mh - (ph + oy) : oy;

            _xs = _xe = _ys = _ye = INT16_MAX;

            update_madctl();
        }


        void Panel_RM690B0::setInvert(bool invert)
        {
            // ESP_LOGD("RM690B0","setInvert %d", invert);

            cs_control(false);

            if (invert) {
                /* Inversion On */
                write_cmd(0x21);
            }
            else {
                /* Inversion Off */
                write_cmd(0x20);
            }
            _bus->wait();

            cs_control(true);
        }


        void Panel_RM690B0::setSleep(bool flg)
        {
            // ESP_LOGD("RM690B0","setSleep %d", flg);

            cs_control(false);

            if (flg) {
                /* Sleep in */
                write_cmd(0x10);
            }
            else {
                /* Sleep out */
                write_cmd(0x11);
                delay(150);
            }
            _bus->wait();

            cs_control(true);
        }


        void Panel_RM690B0::setPowerSave(bool flg)
        {
            // ESP_LOGD("RM690B0","setPowerSave");
        }


        void Panel_RM690B0::waitDisplay(void)
        {
            // ESP_LOGD("RM690B0","waitDisplay");
        }


        bool Panel_RM690B0::displayBusy(void)
        {
            // ESP_LOGD("RM690B0","displayBusy");
            return false;
        }


        color_depth_t Panel_RM690B0::setColorDepth(color_depth_t depth)
        {
            // ESP_LOGD("RM690B0","setColorDepth %d", depth);
            // NOTE: this probably needs revisiting, supported formats are RGB888/RGB666/RGB565/RGB332/RGB111/Gray 256

            /* 0x55: 16bit/pixel */
            /* 0x66: 18bit/pixel */
            /* 0x77: 24bit/pixel */
            uint8_t cmd_send = 0;
            if (depth == rgb565_2Byte) {
                cmd_send = 0x55;
            }
            else if (depth == rgb666_3Byte) {
                cmd_send = 0x66;
            }
            else if (depth == rgb888_3Byte) {
                cmd_send = 0x77;
            }
            else {
                return _write_depth;
            }
            _write_depth = depth;

            /* Set interface Pixel Format */
            startWrite();

            cs_control(false);
            write_cmd(0x3a);
            _bus->writeCommand(cmd_send, 8);
            _bus->wait();
            cs_control(true);

            endWrite();

            return _write_depth;
        }


        void Panel_RM690B0::beginTransaction(void)
        {
            // ESP_LOGD("RM690B0","beginTransaction");
            if (_in_transaction) return;
            _in_transaction = true;
            _bus->beginTransaction();
        }


        void Panel_RM690B0::endTransaction(void)
        {
            // ESP_LOGD("RM690B0","endTransaction");
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


        void Panel_RM690B0::setPartialArea(uint_fast16_t ys, uint_fast16_t ye)
        {
            if( ys==0 && ys==_height-1 ) {
                cs_control(false);
                write_cmd(0x13); // Set Partial Display Mode to Normal (off)
                cs_control(true);
                return;
            }

            cs_control(false);
            write_cmd(0x12); // Set Partial Display Mode On
            cs_control(true);

            cs_control(false);
            write_cmd(0x30); // set Partial Area
            _bus->writeCommand(ys >> 8, 8);
            _bus->writeCommand(ys & 0xFF, 8);
            _bus->writeCommand(ye >> 8, 8);
            _bus->writeCommand(ye & 0xFF, 8);
            _bus->wait();
            cs_control(true);
        }


        void Panel_RM690B0::setVerticalPartialArea(uint_fast16_t xs, uint_fast16_t xe)
        {
            if( xs==0 && xe==_width-1 ) {
                cs_control(false);
                write_cmd(0x13); // Set Partial Display Mode to Normal (off)
                cs_control(true);
                return;
            }

            cs_control(false);
            write_cmd(0x12); // Set Partial Display Mode On
            cs_control(true);

            cs_control(false);
            write_cmd(0x31);// set Vertical Partial Area
            _bus->writeCommand(xs >> 8, 8);
            _bus->writeCommand(xs & 0xFF, 8);
            _bus->writeCommand(xe >> 8, 8);
            _bus->writeCommand(xe & 0xFF, 8);
            _bus->wait();
            cs_control(true);
        }


        void Panel_RM690B0::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
        {
            // ESP_LOGD("RM690B0","setWindow %d %d %d %d", xs, ys, xe, ye);
            uint16_t w = (xe-xs)+1;
            if(xs%2!=0 || w%2!=0) { // RM690B0 restriction: x and w must be divisible by 2
                // ESP_LOGD("LGFX", "clip coords aren't aligned");
                return;
            }

            if (xs > xe || xe > _width-1) { return; }
            if (ys > ye || ye > _height-1) { return; }

            /* As RM69090 driver need offset (see ORIENTATIONS_GENERAL) then the memory area needs to follow offsets*/

            xs += _colstart;
            ys += _rowstart;
            xe += _colstart;
            ye += _rowstart;

            /* Set limit */
            if ((xe - xs) >= _width) { xs = 0; xe = _width - 1; }
            if ((ye - ys) >= _height) { ys = 0; ye = _height - 1; }

            static uint_fast16_t _oldxs = -1, _oldys = -1, _oldxe = -1, _oldye = -1;

            if( xs != _oldxs || xe != _oldxe ) {
                /* Set Column Start Address */
                cs_control(false);
                write_cmd(0x2A);
                //write_cmd(0x30);
                _bus->writeCommand(xs >> 8, 8);
                _bus->writeCommand(xs & 0xFF, 8);
                _bus->writeCommand(xe >> 8, 8);
                _bus->writeCommand(xe & 0xFF, 8);
                _bus->wait();
                cs_control(true);
            }

            if( ys != _oldys || ye != _oldye ) {
                /* Set Row Start Address */
                cs_control(false);
                write_cmd(0x2B);
                //write_cmd(0x31);
                _bus->writeCommand(ys >> 8, 8);
                _bus->writeCommand(ys & 0xFF, 8);
                _bus->writeCommand(ye >> 8, 8);
                _bus->writeCommand(ye & 0xFF, 8);
                _bus->wait();
                cs_control(true);
            }

            _oldxs = xs; _oldys = ys;
            _oldxe = xe; _oldye = ye;

            // /* Memory Write */
            // cs_control(false);
            // write_cmd(0x2C);
            // _bus->wait();
            // cs_control(true);
        }


        void Panel_RM690B0::writeBlock(uint32_t rawcolor, uint32_t len)
        {
            // ESP_LOGD("RM690B0","writeBlock 0x%lx %ld", rawcolor, len);
            /* Push color */
            start_qspi();
            _bus->writeDataRepeat(rawcolor, _write_bits, len);
            _bus->wait();
            end_qspi();
        }


        void Panel_RM690B0::writePixels(pixelcopy_t* param, uint32_t len, bool use_dma)
        {
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


        void Panel_RM690B0::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
        {
            // ESP_LOGD("RM690B0","drawPixelPreclipped %d %d 0x%lX", x, y, rawcolor);
            if(x%2!=0) { // RM690B0 restriction: x and w must be divisible by 2
                // ESP_LOGD("LGFX", "clip coords aren't aligned");
                return;
            }
            setWindow(x,y,x,y);
            if (_cfg.dlen_16bit) { _has_align_data = (_write_bits & 15); }

            start_qspi();

            _bus->writeData(rawcolor, _write_bits);

            _bus->wait();
            end_qspi();
        }


        void Panel_RM690B0::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
        {
            // ESP_LOGD("RM690B0","writeFillRectPreclipped %d %d %d %d 0x%lX", x, y, w, h, rawcolor);
            if(x%2!=0 || w%2!=0) { // RM690B0 restriction: x and w must be divisible by 2
                // ESP_LOGD("LGFX", "clip coords aren't aligned");
                return;
            }
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


        void Panel_RM690B0::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
        {
            // ESP_LOGD("RM690B0","writeImage %d %d %d %d %d", x, y, w, h, use_dma);
            if(x%2!=0 || w%2!=0) { // RM690B0 restriction: x and w must be divisible by 2
                ESP_LOGD("LGFX", "clip coords aren't aligned x(%d) y(%d) w(%d) h(%d) use_dma(%d)", x, y, w, h, use_dma);
                return;
            }
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



        //----------------------------------------------------------------------------



        bool Panel_RM690B0_Framebuffer::init(bool use_reset)
        {
            if( _frame_buffer )
              return true;
            // setRotation(getRotation());
            ESP_LOGD("RM690B0","Panel_RM690B0_Framebuffer init [%d x %d] %d", _width, _height, use_reset);
            if( !initFramebuffer(_width, _height) )
              return false;
            _internal_rotation = 0;
            return Panel_Device/*Panel_FrameBufferBase*/::init(false);
        }


        void Panel_RM690B0_Framebuffer::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
        {
            if( !_frame_buffer)
                return;

            if( x == 0 && y == 0 && w == 0 && h == 0 )
            {
                w = _width;
                h = _height;
            }

            if( w == 0 || h == 0 )
                return;

            uint16_t x0 = x & 0xfe; // Make even
            uint16_t y0 = y & 0xfe; // Make even
            uint16_t x1 = x + w - 1;
            uint16_t y1 = y + h - 1;
            size_t p_buf_idx;
            size_t buf_idx;

            if( x1%2==0 ) x1++; // Make odd if not
            if( y1%2==0 ) y1++; // Make odd if not

            uint16_t w1 = x1 - x0 + 1 ; // Corrected width
            uint16_t h1 = y1 - y0 + 1;  // Corrected height

            _panel->setWindow(x0, y0, x1, y1);
            // ESP_LOGD("LGFX", "x(%d=>%d), y(%d=>%d), w(%d=>%d), h(%d=>%d)", x, x0, y, y0, w, w1, h, h1);

            uint8_t bpp = _write_bits >> 3; // bytes per pixel
            size_t wb = w1 * bpp; // bytes per line
            uint8_t* buf = _panel->getBus()->getDMABuffer(wb*2);

            _panel->start_qspi();
            for( int i=y0;i<y1;i+=2)
            {
                int fbpos0 = ((i*_width)+(x0))*bpp;
                int fbpos1 = fbpos0+(_width*bpp);
                memcpy(&buf[0],  &_frame_buffer[fbpos0], wb);
                memcpy(&buf[wb], &_frame_buffer[fbpos1], wb);
                _panel->getBus()->writeBytes(buf, wb*2, false, true);
            }
            _panel->end_qspi();

            return;
        }


        bool Panel_RM690B0_Framebuffer::initFramebuffer(uint_fast16_t w, uint_fast16_t h)
        {
            size_t lineArray_size = h * sizeof(void*);
            // ESP_LOGE("DEBUG","height:%d", h);
            uint8_t** lineArray = (uint8_t**)heap_alloc_dma(lineArray_size);
            if (lineArray)
            {
                memset(lineArray, 0, lineArray_size);
                uint8_t bits = (_write_depth & color_depth_t::bit_mask);
                ESP_LOGD("LGFX", "color has %d bits (%d bytes)", bits, bits/8);
                w = (w + 3) & ~3; // round up to nearest multiple of 4
                // 暫定実装。画面全体のバッファを一括で確保する。
                // ToDo : 分割確保
                int framebuffersize = (w * (bits >> 3)) * h;
                //log_d("framebuffersize: %d bytes (expected=%d)", framebuffersize, 600*452*2);
                _frame_buffer = (uint8_t*)heap_alloc_psram(framebuffersize);
                if (_frame_buffer) {
                    _lines_buffer = lineArray;
                    auto fb = _frame_buffer;
                    for (int i = 0; i < h; ++i) {
                        lineArray[i] = fb;
                        fb += w * bits >> 3;
                    }
                    ESP_LOGD("LGFX", "Framebuffer allocated [%d x %d] (%d) bytes", w, h, framebuffersize);
                    return true;
                }
                heap_free(lineArray);
            }
            ESP_LOGE("LGFX", "Framebuffer allocation failed [%d x %d]", w, h);
            return false;
        }


        void Panel_RM690B0_Framebuffer::deinitFramebuffer(void)
        {
            if (_frame_buffer)
            {
                heap_free(_frame_buffer);
                _frame_buffer = nullptr;
            }

            if (_lines_buffer)
            {
                heap_free(_lines_buffer);
                _lines_buffer = nullptr;
            }
        }


        //----------------------------------------------------------------------------
    }
}

#endif
#endif
