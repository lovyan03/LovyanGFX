/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [Christian Erhardt](https://github.com/MoJo2600)

 ST77916 QSPI panel support (e.g. Guition JC3636W518C, 360x360 round).

 Bus plumbing follows the QSPI pattern from Panel_SH8601Z; the init
 register sequence follows the Sitronix ST779xx family (Panel_ST77961)
 with values taken from the JC3636W518 manufacturer init code.
/----------------------------------------------------------------------------*/
#pragma once

#if defined (ESP_PLATFORM)

#include "Panel_Device.hpp"
#include "../platforms/common.hpp"
#include "../platforms/device.hpp"

#if defined LGFX_USE_QSPI

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_ST77916 : public Panel_Device
  {
  public:
    Panel_ST77916(void)
    {
      _cfg.panel_width  = _cfg.memory_width  = 360;
      _cfg.panel_height = _cfg.memory_height = 360;

      _cfg.dummy_read_pixel = 8;
    }

    bool init(bool use_reset) override;
    void beginTransaction(void) override;
    void endTransaction(void) override;

    color_depth_t setColorDepth(color_depth_t depth) override;
    void setRotation(uint_fast8_t r) override;
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

    // Panel_Device (unlike Panel_LCD) does not provide these; declare our own.
    uint16_t _colstart = 0;
    uint16_t _rowstart = 0;

    void command_list(const uint8_t *addr);
    void update_madctl(void);

    void write_cmd(uint8_t cmd);
    void start_qspi(void);
    void end_qspi(void);
    void write_bytes(const uint8_t* data, uint32_t len, bool use_dma);

    const uint8_t* getInitCommands(uint8_t listno) const;
  };

//----------------------------------------------------------------------------
 }
}

#endif // LGFX_USE_QSPI
#endif // ESP_PLATFORM
