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


    void Panel_SH8601Z::write_data(uint8_t data)
    {

    }


    /* Panel init */
    bool Panel_SH8601Z::init(bool use_reset)
    {
        if (!Panel_Device::init(use_reset)) {
            return false;
        }

        /* Init command */
        printf("666\n");

        





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








        

        return false;
    }


    void Panel_SH8601Z::beginTransaction(void)
    {
        if (_in_transaction) return;
        _in_transaction = true;
        _bus->beginTransaction();
        // cs_control(false);
    }


    void Panel_SH8601Z::endTransaction(void)
    {
        if (!_in_transaction) return;
        _in_transaction = false;
        _bus->endTransaction();
        // cs_control(true);
    }


    color_depth_t Panel_SH8601Z::setColorDepth(color_depth_t depth)
    {

        return lgfx::rgb565_2Byte;
    }


    void Panel_SH8601Z::setRotation(uint_fast8_t r)
    {

    }

    void Panel_SH8601Z::setInvert(bool invert)
    {

    }

    void Panel_SH8601Z::setSleep(bool flg)
    {

    }
    
    void Panel_SH8601Z::setPowerSave(bool flg)
    {

    }

    void Panel_SH8601Z::waitDisplay(void)
    {

    }

    bool Panel_SH8601Z::displayBusy(void)
    {

        return false;
    }

    void Panel_SH8601Z::writePixels(pixelcopy_t* param, uint32_t len, bool use_dma)
    {

    }

    void Panel_SH8601Z::writeBlock(uint32_t rawcolor, uint32_t len)
    {

    }

    void Panel_SH8601Z::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
    {

    }

    void Panel_SH8601Z::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
    {

    }

    void Panel_SH8601Z::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
    {

    }

    void Panel_SH8601Z::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
    {

    }

    uint32_t Panel_SH8601Z::readCommand(uint_fast16_t cmd, uint_fast8_t index, uint_fast8_t len)
    {
        return 0;
    }

    uint32_t Panel_SH8601Z::readData(uint_fast8_t index, uint_fast8_t len)
    {
        return 0;
    }

    void Panel_SH8601Z::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
    {

    }


//----------------------------------------------------------------------------
 }
}
