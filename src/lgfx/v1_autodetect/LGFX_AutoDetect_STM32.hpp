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
#pragma once

#include "../v1_init.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_F746DISCO : public Panel_ILI9341
  {
    Panel_F746DISCO(void) : Panel_ILI9341()
    {
      {
        auto cfg = _bus_instance.config();    // Get config params of bus.  バスの設定値を取得

        cfg.spi_mode = 0;             // Set the SPI mode. (0 ~ 3)      SPI通信モードを設定
        cfg.freq_write = 80000000;    // Set the SPI clock for write.   送信時のSPIクロックを設定
        cfg.freq_read  = 10000000;    // Set the SPI clock for read.    受信時のSPIクロックを設定
        cfg.pin_sclk = lgfx::stm32::PORT_B|13; // Set the SPI SCLK pin number   SPIのSCLKピン番号を設定
        cfg.pin_mosi = lgfx::stm32::PORT_B|15; // Set the SPI MOSI pin number   SPIのMOSIピン番号を設定
        cfg.pin_miso = lgfx::stm32::PORT_B|14; // Set the SPI MISO pin number   SPIのMISOピン番号を設定 (-1 = disable)
        cfg.pin_dc   = lgfx::stm32::PORT_B| 9;  // Set the SPI D/C pin number    SPIのD/Cピン番号を設定 (-1 = disable)
        cfg.spi_port = SPI2;
        cfg.dma_port = DMA1;
        _bus_instance.config(cfg);    // Set config params of bus.  バスの設定値を更新
      }

      bus(&_bus_instance);
    }

  private:
    lgfx::Bus_SPI _bus_instance;
  };

  struct LGFX : public LGFX_Device
  {
    LGFX(void) // コンストラクタ内で定義を行う
    {
      panel(&_panel_instance);      // 使用するパネルを指定する
    }

  private:
    Panel_F746DISCO _panel_instance;
  };

//----------------------------------------------------------------------------
 }
}

using LGFX = lgfx::LGFX;