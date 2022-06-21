#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

// LGFX for wywy ESP32-S3-HMI-DevKit
// https://github.com/W00ng/ESP32-S3-HMI-DevKit/

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_RM68120 _panel_instance;
    lgfx::Bus_Parallel16 _bus_instance;
    lgfx::Touch_FT5x06 _touch_instance;

public:

    LGFX(void)
    {
        {
            auto cfg = _bus_instance.config();

            cfg.freq_write = 30000000;

            cfg.pin_wr = 17;
            cfg.pin_rd = 21;
            cfg.pin_rs = 38;

            cfg.pin_d0  =  1;
            cfg.pin_d1  =  9;
            cfg.pin_d2  =  2;
            cfg.pin_d3  = 10;
            cfg.pin_d4  =  3;
            cfg.pin_d5  = 11;
            cfg.pin_d6  =  4;
            cfg.pin_d7  = 12;
            cfg.pin_d8  =  5;
            cfg.pin_d9  = 13;
            cfg.pin_d10 =  6;
            cfg.pin_d11 = 14;
            cfg.pin_d12 =  7;
            cfg.pin_d13 = 15;
            cfg.pin_d14 =  8;
            cfg.pin_d15 = 16;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();

            cfg.pin_cs           =    -1;
            cfg.pin_rst          =    18;
            cfg.pin_busy         =    -1;
            cfg.dlen_16bit = true;
            cfg.rgb_order  = true;
            cfg.memory_width  = 480;
            cfg.memory_height = 800;
            cfg.panel_width  = 480;
            cfg.panel_height = 800;
            cfg.readable = true;

            _panel_instance.config(cfg);
        }

        {
            auto cfg = _touch_instance.config();
            cfg.i2c_addr = 0x38;
            cfg.x_min = 0;
            cfg.y_min = 0;
            cfg.x_max = 479;
            cfg.y_max = 799;
            cfg.i2c_port = 0;
            cfg.pin_sda  = GPIO_NUM_40;
            cfg.pin_scl  = GPIO_NUM_39;
            cfg.pin_int  = -1;
            cfg.freq = 400000;
            cfg.bus_shared = false;

            _touch_instance.config(cfg);
            _panel_instance.touch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

