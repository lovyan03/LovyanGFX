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

#include "Panel_Device.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Panel_M5UnitLCD : public Panel_Device
  {
  public:
    Panel_M5UnitLCD(void)
    {
      _cfg.memory_width  = _cfg.panel_width = 135;
      _cfg.memory_height = _cfg.panel_height = 240;
    }

    bool init(bool use_reset) override;
    void beginTransaction(void) override;
    void endTransaction(void) override;

    color_depth_t setColorDepth(color_depth_t depth) override;
    void setRotation(std::uint_fast8_t r) override;
    void setBrightness(std::uint8_t brightness) override;
    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void waitDisplay(void) override {}
    bool displayBusy(void) override { return false; }
    void display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h) override {}

    void writePixels(pixelcopy_t* param, std::uint32_t len, bool use_dma) override;
    void writeBlock(std::uint32_t rawcolor, std::uint32_t len) override;

    void setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) override;
    void drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor) override;
    void writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor) override;
    void writeFillRectAlphaPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t argb8888) override;
    void writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writeImageARGB(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param) override;
    void copyRect(std::uint_fast16_t dst_x, std::uint_fast16_t dst_y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint_fast16_t src_x, std::uint_fast16_t src_y) override;

    std::uint32_t readCommand(std::uint_fast8_t cmd, std::uint_fast8_t index, std::uint_fast8_t len) override { return 0; }
    std::uint32_t readData(std::uint_fast8_t index, std::uint_fast8_t len) override { return 0; }
    void readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param) override;

    static constexpr std::uint8_t CMD_NOP          = 0x00; // 1Byte 何もしない
    static constexpr std::uint8_t CMD_READ_ID      = 0x04; // 1Byte ID読出し  スレーブからの回答は4Byte (0x77 0x89 0x00 0x?? (最後の1バイトはファームウェアバージョン))
    static constexpr std::uint8_t CMD_READ_BUFCOUNT= 0x09; // 1Byte コマンドバッファの空き取得。回答は1Byte、受信可能なコマンド数が返される。数字が小さいほどバッファの余裕がない。
    static constexpr std::uint8_t CMD_INVOFF       = 0x20; // 1Byte 色反転を解除
    static constexpr std::uint8_t CMD_INVON        = 0x21; // 1Byte 色反転を有効
    static constexpr std::uint8_t CMD_BRIGHTNESS   = 0x22; // 2Byte バックライト data[1]==明るさ 0~255
    static constexpr std::uint8_t CMD_COPYRECT     = 0x23; // 7Byte 矩形範囲コピー [1]==XS [2]==YS [3]==XE [4]==YE [5]==DST_X [6]==DST_Y
    static constexpr std::uint8_t CMD_CASET        = 0x2A; // 3Byte X方向の範囲選択 data[1]==XS  data[2]==XE
    static constexpr std::uint8_t CMD_RASET        = 0x2B; // 3Byte Y方向の範囲選択 data[1]==YS  data[2]==YE
    static constexpr std::uint8_t CMD_ROTATE       = 0x36; // 2Byte 回転処理 [1]==回転方向 0:通常 1:右90度 2:180度 3:270度 4~7は0~3の上下反転
    static constexpr std::uint8_t CMD_SET_POWER    = 0x38; // 2Byte data[1] 0:低速ローパワー / 1:通常 / 2:高速ハイパワー
    static constexpr std::uint8_t CMD_SET_SLEEP    = 0x39; // 2Byte data[1] 0:スリープ解除 / 1:スリープ開始

    static constexpr std::uint8_t CMD_WRITE_RAW    = 0x40;
    static constexpr std::uint8_t CMD_WRITE_RAW_8  = 0x41; // 不定長 RGB332   1Byteのピクセルデータを連続送信
    static constexpr std::uint8_t CMD_WRITE_RAW_16 = 0x42; // 不定長 RGB565   2Byteのピクセルデータを連続送信
    static constexpr std::uint8_t CMD_WRITE_RAW_24 = 0x43; // 不定長 RGB888   3Byteのピクセルデータを連続送信
    static constexpr std::uint8_t CMD_WRITE_RAW_32 = 0x44; // 不定長 ARGB8888 4Byteのピクセルデータを連続送信
    static constexpr std::uint8_t CMD_WRITE_RAW_A  = 0x45; // 不定長 A8       1Byteのピクセルデータを連続送信(アルファチャネルのみ、描画色は最後に使用したものを再利用する)

    static constexpr std::uint8_t CMD_WRITE_RLE    = 0x48;
    static constexpr std::uint8_t CMD_WRITE_RLE_8  = 0x49; // 不定長 RGB332   1Byteのピクセルデータを連続送信(RLE圧縮)
    static constexpr std::uint8_t CMD_WRITE_RLE_16 = 0x4A; // 不定長 RGB565   2Byteのピクセルデータを連続送信(RLE圧縮)
    static constexpr std::uint8_t CMD_WRITE_RLE_24 = 0x4B; // 不定長 RGB888   3Byteのピクセルデータを連続送信(RLE圧縮)
    static constexpr std::uint8_t CMD_WRITE_RLE_32 = 0x4C; // 不定長 ARGB8888 4Byteのピクセルデータを連続送信(RLE圧縮)
    static constexpr std::uint8_t CMD_WRITE_RLE_A  = 0x4D; // 不定長 A8       1Byteのピクセルデータを連続送信(RLE圧縮 アルファチャネルのみ、描画色は最後に使用したものを再利用する)

    static constexpr std::uint8_t CMD_RAM_FILL     = 0x50; // 1Byte 現在の描画色で選択範囲全塗り
    static constexpr std::uint8_t CMD_SET_COLOR    = 0x50;
    static constexpr std::uint8_t CMD_SET_COLOR_8  = 0x51; // 2Byte 描画色をRGB332で指定
    static constexpr std::uint8_t CMD_SET_COLOR_16 = 0x52; // 3Byte 描画色をRGB565で指定
    static constexpr std::uint8_t CMD_SET_COLOR_24 = 0x53; // 4Byte 描画色をRGB888で指定
    static constexpr std::uint8_t CMD_SET_COLOR_32 = 0x54; // 5Byte 描画色をARGB8888で指定

    static constexpr std::uint8_t CMD_DRAWPIXEL    = 0x60; // 3Byte ドット描画 [1]==X [2]==Y
    static constexpr std::uint8_t CMD_DRAWPIXEL_8  = 0x61; // 4Byte ドット描画 [1]==X [2]==Y [3  ]==RGB332
    static constexpr std::uint8_t CMD_DRAWPIXEL_16 = 0x62; // 5Byte ドット描画 [1]==X [2]==Y [3~4]==RGB565
    static constexpr std::uint8_t CMD_DRAWPIXEL_24 = 0x63; // 6Byte ドット描画 [1]==X [2]==Y [3~5]==RGB888
    static constexpr std::uint8_t CMD_DRAWPIXEL_32 = 0x64; // 7Byte ドット描画 [1]==X [2]==Y [3~6]==ARGB8888

    static constexpr std::uint8_t CMD_FILLRECT     = 0x68; // 5Byte 矩形塗潰 [1]==XS [2]==YS [3]==XE [4]==YE
    static constexpr std::uint8_t CMD_FILLRECT_8   = 0x69; // 6Byte 矩形塗潰 [1]==XS [2]==YS [3]==XE [4]==YE [5  ]==RGB332
    static constexpr std::uint8_t CMD_FILLRECT_16  = 0x6A; // 7Byte 矩形塗潰 [1]==XS [2]==YS [3]==XE [4]==YE [5~6]==RGB565
    static constexpr std::uint8_t CMD_FILLRECT_24  = 0x6B; // 8Byte 矩形塗潰 [1]==XS [2]==YS [3]==XE [4]==YE [5~7]==RGB888
    static constexpr std::uint8_t CMD_FILLRECT_32  = 0x6C; // 9Byte 矩形塗潰 [1]==XS [2]==YS [3]==XE [4]==YE [5~8]==ARGB8888

    static constexpr std::uint8_t CMD_READ_RAW     = 0x80;
    static constexpr std::uint8_t CMD_READ_RAW_8   = 0x81; // 1Byte RGB332のピクセルデータを読出し
    static constexpr std::uint8_t CMD_READ_RAW_16  = 0x82; // 1Byte RGB565のピクセルデータを読出し
    static constexpr std::uint8_t CMD_READ_RAW_24  = 0x83; // 1Byte RGB888のピクセルデータを読出し

    static constexpr std::uint8_t CMD_CHANGE_ADDR  = 0xA0; // 4Byte i2cアドレス変更 [0]=0xA0 [1]=変更後のI2Cアドレス [2]=変更後のI2Cアドレスのビット反転値 [3]=0xA0

    static constexpr std::uint8_t CMD_UPDATE_BEGIN = 0xF0; // 4Byte ファームウェア更新開始   [0]=0xF0 [1]=0x77 [2]=0x89 [3]=0xF0
    static constexpr std::uint8_t CMD_UPDATE_DATA  = 0xF1; // 4Byte ファームウェアデータ送信 [0]=0xF1 [1]=0x77 [2]=0x89 [3]=0xF1
    static constexpr std::uint8_t CMD_UPDATE_END   = 0xF2; // 4Byte ファームウェア更新完了   [0]=0xF2 [1]=0x77 [2]=0x89 [3]=0xF2
    static constexpr std::uint8_t CMD_RESET        = 0xFF; // 4Byte リセット(再起動)         [0]=0xFF [1]=0x77 [2]=0x89 [3]=0xFF

    static constexpr std::uint8_t UPDATE_RESULT_BROKEN = 0x01;
    static constexpr std::uint8_t UPDATE_RESULT_ERROR  = 0x00;
    static constexpr std::uint8_t UPDATE_RESULT_OK     = 0xF1;
    static constexpr std::uint8_t UPDATE_RESULT_BUSY   = 0xFF;

  protected:
  
    std::uint32_t _raw_color = ~0u;
    std::uint32_t _xpos;
    std::uint32_t _ypos;
    std::uint32_t _last_cmd;
    std::uint32_t _buff_free_count;

    void _set_window(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye);
    void _fill_rect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint_fast8_t bytes);
    bool _check_repeat(std::uint32_t cmd = 0, std::uint_fast8_t limit = 64);

  };

//----------------------------------------------------------------------------
 }
}
