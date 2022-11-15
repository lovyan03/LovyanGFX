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

#include "Panel_CoordinateConvertFB.hpp"

#include "../misc/DividedFrameBuffer.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_HUB75 : public Panel_CoordinateConvertFB
  {
  public:
    bool init(bool use_reset) override;

    virtual ~Panel_HUB75(void);

    // config での指定について
    // memory_width,panel_width   は物理配置における幅を指定する
    // memory_height,panel_height は物理配置における高さを指定する
    // 例えば４枚の64x32 パネルを縦２枚,横２枚の配置にする場合、widthには128を、heightには64を指定する

    struct config_detail_t
    {
      uint8_t x_panel_count = 1; // 幅方向のパネル枚数
      uint8_t y_panel_count = 1; // 高さ方向のパネル枚数
      uint8_t panel_count = 1;  // 総パネル枚数
      // uint8_t* panel_pos_list;
      // DividedFrameBuffer::psram_setting_t use_psram = DividedFrameBuffer::psram_setting_t::no_psram;
    };

    const config_detail_t& config_detail(void) const { return _config_detail; }
    void config_detail(const config_detail_t& config_detail) { _config_detail = config_detail; }

    void setBrightness(uint8_t brightness) override;
    color_depth_t setColorDepth(color_depth_t depth) override;

  protected:
    config_detail_t _config_detail;
    DividedFrameBuffer _frame_buffer;

    uint16_t _single_width;
    uint16_t _single_height;
    bool _initialized = false;

    bool _init_frame_buffer(void);

    uint32_t _read_pixel_inner(uint_fast16_t x, uint_fast16_t y) override;
    void _draw_pixel_inner(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
  };

//----------------------------------------------------------------------------
 }
}
