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
#include "driver/spi_master.h"



namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

    void Panel_SH8601Z::write_cmd(uint8_t cmd)
    {
        uint8_t cmd_buffer[4] = {0x02, 0x00, 0x00, 0x00};
        cmd_buffer[2] = cmd;
        // _bus->writeBytes(cmd_buffer, 4, 0, false);
        for (int i = 0; i < 4; i++) {
            _bus->writeCommand(cmd_buffer[i], 8);
        }
    }


    void Panel_SH8601Z::start_qspi()
    {
        /* Begin QSPI */
        cs_control(false);
        _bus->writeCommand(0x32, 8);
        _bus->writeCommand(0x00, 8);
        _bus->writeCommand(0x2C, 8);
        _bus->writeCommand(0x00, 8);
        _bus->wait();
    }

    void Panel_SH8601Z::end_qspi()
    {
        /* Stop QSPI */
        _bus->writeCommand(0x32, 8);
        _bus->writeCommand(0x00, 8);
        _bus->writeCommand(0x00, 8);
        _bus->writeCommand(0x00, 8);
        _bus->wait();
        cs_control(true);
    }


    void Panel_SH8601Z::write_bytes(const uint8_t* data, uint32_t len, bool use_dma)
    {
        start_qspi();
        _bus->writeBytesQuad(data, len, true, use_dma);
        _bus->wait();
        end_qspi();
    }


    /* Panel init */
    bool Panel_SH8601Z::init(bool use_reset)
    {
        printf("init\n");

        
        if (!Panel_Device::init(use_reset)) {
            return false;
        }

        
        /* Store pannel resolution */
        _width = _cfg.panel_width;
        _height = _cfg.panel_height;


        startWrite();

        /* Sleep out */
        cs_control(false);
        write_cmd(0x11);
        _bus->wait();
        cs_control(true);
        delay(120);

        cs_control(false);
        write_cmd(0x44);
        _bus->writeCommand(0x01, 8);
        _bus->writeCommand(0x66, 8);
        _bus->wait();
        cs_control(true);
        delay(1);

        /* TE on */
        cs_control(false);
        write_cmd(0x35);
        _bus->writeCommand(0x00, 8);
        _bus->wait();
        cs_control(true);
        delay(1);

        /* Interface Pixel Format 16bit/pixel */
        cs_control(false);
        write_cmd(0x3A);
        _bus->writeCommand(0x55, 8);
        _bus->wait();
        cs_control(true);
        delay(1);

        cs_control(false);
        write_cmd(0x53);
        _bus->writeCommand(0x20, 8);
        _bus->wait();
        cs_control(true);
        delay(10);

        /* Write Display Brightness	MAX_VAL=0XFF */
        cs_control(false);
        write_cmd(0x51);
        _bus->writeCommand(0x00, 8);
        _bus->wait();
        cs_control(true);
        delay(10);

        /* Display on */
        cs_control(false);
        write_cmd(0x29);
        _bus->wait();
        cs_control(true);
        delay(10);

        /* Write Display Brightness	MAX_VAL=0XFF */
        cs_control(false);
        write_cmd(0x51);
        _bus->writeCommand(0xFF, 8);
        _bus->wait();
        cs_control(true);
        delay(1);

        endWrite();

        return true;
    }


    void Panel_SH8601Z::beginTransaction(void)
    {
        printf("beginTransaction\n");
        if (_in_transaction) return;
        _in_transaction = true;
        _bus->beginTransaction();
    }


    void Panel_SH8601Z::endTransaction(void)
    {
        printf("endTransaction\n");
        if (!_in_transaction) return;
        _in_transaction = false;
        _bus->endTransaction();
    }


    void Panel_SH8601Z::setBrightness(uint8_t brightness)
    {
        startWrite();

        /* Write Display Brightness	MAX_VAL=0XFF */
        cs_control(false);
        write_cmd(0x51);
        _bus->writeCommand(brightness, 8);
        _bus->wait();
        cs_control(true);

        endWrite();
    }


    color_depth_t Panel_SH8601Z::setColorDepth(color_depth_t depth)
    {
        printf("setColorDepth\n");
        return lgfx::rgb565_2Byte;
    }


    void Panel_SH8601Z::setRotation(uint_fast8_t r)
    {
        printf("setRotation\n");
    }

    void Panel_SH8601Z::setInvert(bool invert)
    {
        printf("setInvert\n");
    }

    void Panel_SH8601Z::setSleep(bool flg)
    {
        printf("setSleep\n");
    }
    
    void Panel_SH8601Z::setPowerSave(bool flg)
    {
        printf("setPowerSave\n");
    }

    void Panel_SH8601Z::waitDisplay(void)
    {
        printf("waitDisplay\n");
    }

    bool Panel_SH8601Z::displayBusy(void)
    {
        printf("displayBusy\n");
        return false;
    }

    

    void Panel_SH8601Z::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
    {
        // printf("setWindow\n");
        // printf("%d %d %d %d\n", xs, ys, xe, ye);

        
        /* Set limit */
        if ((xe - xs) >= _width) { xs = 0; xe = _width - 1; }
        if ((ye - ys) >= _height) { ys = 0; ye = _height - 1; }


        /* Set Column Start Address */
        cs_control(false);
        write_cmd(0x2A);
        _bus->writeCommand(xs >> 8, 8);
        _bus->writeCommand(xs & 0xFF, 8);
        _bus->writeCommand(xe >> 8, 8);
        _bus->writeCommand(xe & 0xFF, 8);
        _bus->wait();
        cs_control(true);
        
        /* Set Row Start Address */
        cs_control(false);
        write_cmd(0x2B);
        _bus->writeCommand(ys >> 8, 8);
        _bus->writeCommand(ys & 0xFF, 8);
        _bus->writeCommand(ye >> 8, 8);
        _bus->writeCommand(ye & 0xFF, 8);
        _bus->wait();
        cs_control(true);

        /* Memory Write */
        cs_control(false);
        write_cmd(0x2C);
        _bus->wait();
        cs_control(true);
    }


    



    void Panel_SH8601Z::writeBlock(uint32_t rawcolor, uint32_t len)
    {
        printf("writeBlock\n");
        printf("0x%lx %ld\n", rawcolor, len);


        /* Begin QSPI */
        cs_control(false);
        _bus->writeCommand(0x32, 8);
        _bus->writeCommand(0x00, 8);
        _bus->writeCommand(0x2C, 8);
        _bus->writeCommand(0x00, 8);
        _bus->wait();


        /* Push color */
        _bus->writeDataRepeatQuad(rawcolor, _write_bits, len);
        _bus->wait();


        /* Stop QSPI */
        _bus->writeCommand(0x32, 8);
        _bus->writeCommand(0x00, 8);
        _bus->writeCommand(0x00, 8);
        _bus->writeCommand(0x00, 8);
        _bus->wait();
        cs_control(true);

    }




    void Panel_SH8601Z::writePixels(pixelcopy_t* param, uint32_t len, bool use_dma)
    {
        
        printf("writePixels\n");
    }



    void Panel_SH8601Z::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
    {
        printf("drawPixelPreclipped\n");

    }

    void Panel_SH8601Z::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
    {
        printf("writeFillRectPreclipped\n");

        uint32_t len = w * h;
        uint_fast16_t xe = w + x - 1;
        uint_fast16_t ye = y + h - 1;

        setWindow(x,y,xe,ye);



    }



  void Panel_SH8601Z::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    printf("writeImage\n");
    printf("%d %d %d %d %d\n", x, y, w, h, use_dma);
    use_dma = false;

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




    uint32_t Panel_SH8601Z::readCommand(uint_fast16_t cmd, uint_fast8_t index, uint_fast8_t len)
    {
        printf("readCommand\n");
        return 0;
    }

    uint32_t Panel_SH8601Z::readData(uint_fast8_t index, uint_fast8_t len)
    {
        printf("readData\n");
        return 0;
    }

    void Panel_SH8601Z::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
    {
        printf("readRect\n");
    }


//----------------------------------------------------------------------------
 }
}
