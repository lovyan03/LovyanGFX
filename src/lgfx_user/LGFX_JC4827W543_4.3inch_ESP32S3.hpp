/**
 * @file LGFX_JC4827W543_4.3inch_ESP32S3.hpp
 * @author tobozo
 * @brief
 * @version 0.1
 * @date 2024-08-21
 *
 * @copyleft Copyleft (c+) tobozo 2024
 *
 */
#pragma once
#include <LovyanGFX.hpp>
#include <lgfx/v1/panel/Panel_NV3041A.hpp>

class LGFX : public lgfx::LGFX_Device {

    lgfx::Touch_GT911   _touch_instance;
    lgfx::Panel_NV3041A _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;

public:

    LGFX(void)
    {
        {
            auto cfg = _bus_instance.config();

            cfg.spi_host    = SPI3_HOST;
            cfg.spi_mode    = 1;
            cfg.freq_write  = 32000000UL; // NV3041A Maximum supported speed is 32MHz
            cfg.freq_read   = 16000000;
            cfg.spi_3wire   = true;
            cfg.use_lock    = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;

            cfg.pin_sclk = GPIO_NUM_47;
            cfg.pin_io0  = GPIO_NUM_21;
            cfg.pin_io1  = GPIO_NUM_48;
            cfg.pin_io2  = GPIO_NUM_40;
            cfg.pin_io3  = GPIO_NUM_39;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }


        {
            auto cfg = _panel_instance.config();

            cfg.pin_cs   = GPIO_NUM_45; // Chip select pin
            cfg.pin_rst  = GPIO_NUM_4 ; // Reset pin
            cfg.pin_busy = -1;          // Busy pin (not used)

            cfg.panel_width   = 480;
            cfg.panel_height  = 272;

            cfg.memory_width  = 480;
            cfg.memory_height = 272;

            cfg.offset_x      = 0;
            cfg.offset_y      = 0;

            cfg.offset_rotation = 0;

            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;

            cfg.readable   = true;
            cfg.invert     = true;
            cfg.rgb_order  = true;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;

            _panel_instance.config(cfg);
        }

        {  // Configure settings for touch screen control.  (delete if not necessary)
            auto cfg = _touch_instance.config();

            cfg.x_min = 0;
            cfg.x_max = 480 - 1;
            cfg.y_min = 0;
            cfg.y_max = 272 - 1;
            cfg.offset_rotation = 6;  // 6 fits but double lines. why?

            // For SPI connection
            cfg.bus_shared = false;     // Set true when using a common bus with the screen
            cfg.spi_host = SPI2_HOST;   // Select SPI to use (HSPI_HOST or VSPI_HOST)
            cfg.freq     = 2500000;     // SPI Clock frequency 1000000 -> 2500000
            cfg.pin_int  = -1;          // Pin number to which INT is connected (-1 = not connected)
            cfg.pin_sda  = GPIO_NUM_8 ; // SCLK
            cfg.pin_scl  = GPIO_NUM_4 ; // CS

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        {
            auto cfg = _light_instance.config();

            cfg.pin_bl = GPIO_NUM_1 ;  // Backlight pin
            cfg.invert = false;

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }

};
