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

#include "Panel_FlexibleFrameBuffer.hpp"

#include "../misc/DividedFrameBuffer.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_HUB75 : public Panel_FlexibleFrameBuffer
  {
  public:
    bool init(bool use_reset) override;

    Panel_HUB75(void)
    {
      _write_depth = color_depth_t::rgb332_1Byte;
      _read_depth = color_depth_t::rgb332_1Byte;
    }

    virtual ~Panel_HUB75(void);

    void setBrightness(uint8_t brightness) override;

    // Panel_HUB75では RGB332 8bitモード と RGB565 16bitモードが使用できる
    color_depth_t setColorDepth(color_depth_t depth) override;

    void (*convertCoordinate)(uint_fast16_t &x, uint_fast16_t &y) = nullptr;

  protected:
    DividedFrameBuffer _frame_buffer;

    bool _initialized = false;

    bool _init_impl(uint_fast16_t width, uint_fast16_t height);

    bool _init_frame_buffer(uint_fast16_t total_width, uint_fast16_t single_height);

    uint32_t _read_pixel_inner(uint_fast16_t x, uint_fast16_t y) override;
    void _draw_pixel_inner(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
  };


  // 複数パネルを繋げて使用する場合のクラス
  // config_detailを設定した後、setPanelPositionで各パネルの座標を設定して使用する
  struct Panel_HUB75_Multi : public Panel_HUB75
  {
  public:

    virtual ~Panel_HUB75_Multi(void);

    bool init(bool use_reset) override;

    struct config_detail_t
    {
      // 総パネル枚数
      uint8_t panel_count = 1;

      // パネル1枚の幅方向ピクセル数
      uint16_t single_width = 64;

      // パネル1枚の高さ方向のピクセル数
      uint16_t single_height = 32;
    };

    const config_detail_t& config_detail(void) const { return _config_detail; }
    void config_detail(const config_detail_t& config_detail) { _config_detail = config_detail; }

    // 各パネルの表示座標の設定。configおよびconfig_detailを設定した後に使用すること。
    bool setPanelPosition(uint_fast8_t index, uint_fast16_t x, uint_fast16_t y, uint_fast8_t rotation = 0);

  protected:
    struct panel_position_t
    {
      uint16_t x;
      uint16_t y;
      uint8_t rotation;
    };

    config_detail_t _config_detail;

// 座標の上位3ビットを使用し、座標範囲に該当するパネルを判定するためのテーブル
    uint32_t _x_hitcheck_mask[8];
    uint32_t _y_hitcheck_mask[8];
    uint8_t _x_hitcheck_shift = 0;
    uint8_t _y_hitcheck_shift = 0;

    panel_position_t* _panel_position = nullptr;
    size_t  _panel_position_count = 0;

    uint16_t _single_width;
    uint16_t _single_height;

    bool _init_hitcheck(void);
    uint32_t _read_pixel_inner(uint_fast16_t x, uint_fast16_t y) override;
    void _draw_pixel_inner(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
  };

//----------------------------------------------------------------------------
 }
}
