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

#if __has_include (<epdiy.h>)
extern "C" {
  #include <epdiy.h>
};
#endif

#include "lgfx/v1/panel/Panel_HasBuffer.hpp"
#include "lgfx/v1/misc/range.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_EPDiy : public Panel_HasBuffer
  {
    Panel_EPDiy(void);
    virtual ~Panel_EPDiy(void);

    struct config_detail_t
    {
#if __has_include (<epdiy.h>)
      EpdBoardDefinition* epd_board;
      EpdiyHighlevelState* epd_hl;
#endif
    };

    const config_detail_t& config_detail(void) const { return _config_detail; }
    void config_detail(const config_detail_t& config_detail) { _config_detail = config_detail; }

    void beginTransaction(void) override;
    void endTransaction(void) override;

    bool init(bool use_reset) override;

    void waitDisplay(void) override;
    bool displayBusy(void) override;
    color_depth_t setColorDepth(color_depth_t depth) override;

    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
    void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override;

    uint32_t readCommand(uint_fast16_t, uint_fast8_t, uint_fast8_t) override { return 0; }
    uint32_t readData(uint_fast8_t, uint_fast8_t) override { return 0; }

    void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override;

  private:
    config_detail_t _config_detail;

    // フレームバッファはEPDiyに用意してもらうのでここでの戻り値は0とする
    size_t _get_buffer_length(void) const override { return 0; }
    uint8_t _read_pixel(uint_fast16_t x, uint_fast16_t y);
    void _draw_pixel(uint_fast16_t x, uint_fast16_t y, uint32_t value);
    void _update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye);
  };

//----------------------------------------------------------------------------
 }
}
