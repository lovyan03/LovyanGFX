#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>


/*\
 *

  Description: Display from the popular JYETech DSO138 Oscilloscope
  Platform:    ESP32-S2
  Panel:       ILI9341
  Resolution:  320x240
  Bus Mode:    8 bits Parallel

  Resources:
  - http://www.jyetech.com/Products/LcdScope/UserManual_138.pdf
  - https://www.elecrow.com/download/Schematic%20of%20the%20LCD%20board.pdf
  - https://dl.espressif.com/dl/schematics/ESP32-S2-Kaluga-1_V1.3-Pin-Mapping-v0.1.xlsx
  - https://tomeko.net/projects/dso138/index.php?lang=en
  - https://docs.rs-online.com/784d/0900766b8120b293.pdf
  - https://www.iot-experiments.com/dso138-assembly/
  - https://www.reddit.com/r/arduino/comments/3hpxov/is_it_possible_to_use_this_tft_lcd_with_an/

  Notes:
  - IM0_JP5 is HIGH and IM3_JP6 is LOW to select parallel 8 bits mode, your mileage may vary.
  - LED_K* pins can be spread over 4 pulled down pins for brightness control.


   ESP32S2  |           DSO138 LCD HEADER          |  ESP32S2
  ----------+--------------------------------------+----------
            |                                      |
            |           /--------------\           |
            |        x  |  1        2  |  x        |
            |        x  |  3        4  |  x        |
      GND  <->     GND  |  5        6  |  +3.3v   <->  3.3v
     IO37  <->  TFT_CS  |  7        8  |  TFT_RS  <->  IO36
     IO35  <->  TFT_WR  |  9       10  |  TFT_RD  <->  IO34
     3.3v  <-> IM0_JP5  |  11      12  |  x        |
            |        x  |  13      14  |  x        |
            |        x  |  15      16  |  LED_A   <->  5v
      GND  <->  LED_K1  |  17      18  |  LED_K2  <->  GND
      GND  <->  LED_K3  |  19      20  |  LED_K4  <->  GND
      GND  <-> IM3_JP6  |  21      22  |  x        |
      IO7  <->      D0  |  23      24  |  D1      <->  IO8
      IO6  <->      D2  |  25      26  |  D3      <->  IO9
      IO5  <->      D4  |  27      28  |  D5      <->  IO10
      IO4  <->      D6  |  29      30  |  D7      <->  IO11
     IO38  <-> TFT_RST  |  31      32  |  +3.3v   <->  3.3v
     3.3v  <->   +3.3v  |  33      34  |  GND     <->  GND
            |        x  |  35      36  |  x        |
            |        x  |  37      38  |  x        |
            |        x  |  39      40  |  x        |
            |           \--------------/           |
            |                                      |
  ----------+--------------------------------------+----------

 *
\*/




class LGFX : public lgfx::LGFX_Device {

  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_Parallel8 _bus_instance;

  public:

    LGFX(void)
    {
      {
        auto cfg = _bus_instance.config();
        cfg.freq_write = 30000000; // used 16MHz and 20MHz during testing
        cfg.pin_wr = 35;
        cfg.pin_rd = 34;
        cfg.pin_rs = 36;

        cfg.pin_d0 = 7;  // D0
        cfg.pin_d1 = 8;  // D1
        cfg.pin_d2 = 6;  // D2
        cfg.pin_d3 = 9;  // D3
        cfg.pin_d4 = 5;  // D4
        cfg.pin_d5 = 10; // D5
        cfg.pin_d6 = 4;  // D6
        cfg.pin_d7 = 11; // D7
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance.config();
        cfg.pin_cs           =    37;
        cfg.pin_rst          =    38;
        cfg.pin_busy         =    -1;

        cfg.panel_width      =   240;
        cfg.panel_height     =   320;

        cfg.readable         = true;
        cfg.invert           = false;
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;
        cfg.bus_shared       = true; // SD = true
        _panel_instance.config(cfg);
      }
      setPanel(&_panel_instance);
    }
};


