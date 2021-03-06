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

#include "../Panel.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  struct IBus;

  class Panel_Device : public IPanel
  {
  public:
    Panel_Device(void);

    void bus(IBus* bus);
    inline IBus* bus(void) const { return _bus; }

    struct config_t
    {
      std::int16_t  pin_cs          =    -1;  /// CSが接続されているピン番号
      std::int16_t  pin_rst         =    -1;  /// RSTが接続されているピン番号
      std::int16_t  pin_busy        =    -1;  /// BUSYが接続されているピン番号
      std::int16_t  memory_width    =   240;  /// ドライバがサポートしている最大の幅
      std::int16_t  memory_height   =   240;  /// ドライバがサポートしている最大の高さ
      std::int16_t  panel_width     =   240;  /// 実際に表示可能な幅
      std::int16_t  panel_height    =   240;  /// 実際に表示可能な高さ
      std::uint16_t offset_x        =     0;  /// パネルのX方向オフセット量
      std::uint16_t offset_y        =     0;  /// パネルのY方向オフセット量
      std::uint8_t rotation         =     0;  /// 回転方向の初期値 0~7 (4~7は上下反転)
      std::uint8_t offset_rotation  =     0;  /// 回転方向の値のオフセット 0~7 (4~7は上下反転)
      std::uint8_t dummy_read_pixel =     8;  /// ピクセル読出し前のダミーリードのビット数
      std::uint8_t dummy_read_bits  =     1;  /// ピクセル以外のデータ読出し前のダミーリードのビット数
      bool         readable         =  true;  /// データ読出しが可能な場合 trueに設定
      bool         invert           = false;  /// パネルの明暗が反転してしまう場合 trueに設定
      bool         rgb_order        = false;  /// パネルの赤と青が入れ替わってしまう場合 trueに設定
      bool         dlen_16bit       = false;  /// データ長を16bit単位で送信するパネルの場合 trueに設定
      bool         bus_shared       =  true;  /// SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
    };

    const config_t& config(void) const { return _cfg; }
    void config(const config_t& cfg) { _cfg = cfg; }


    bool isReadable(void) const override { return _cfg.readable; }
    bool isBusShared(void) const override { return _cfg.bus_shared; }

    void init(bool use_reset = true) override;
    void initDMA(void) override;
    void waitDMA(void) override;
    bool dmaBusy(void) override;

    void writeCommand(std::uint32_t data, std::uint_fast8_t length) override;
    void writeData(std::uint32_t data, std::uint_fast8_t length) override;
//    void writeBytes(const std::uint8_t* data, std::uint32_t len, bool use_dma) override;
    void writeImageARGB(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param) override;
    void copyRect(std::uint_fast16_t dst_x, std::uint_fast16_t dst_y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint_fast16_t src_x, std::uint_fast16_t src_y) override;

  protected:

    static constexpr std::uint8_t CMD_INIT_DELAY = 0x80;

    config_t _cfg;

    IBus* _bus = nullptr;
    bool _align_data = false;
    std::uint8_t _internal_rotation = 0;

    /// CSピンの準備処理を行う。CSピンを独自に制御したい場合はこの関数をoverrideして実装すること。
    virtual void init_cs(void);
    /// 引数levelがtrueの時にCSピンがHIGHになるよう制御する。CSピンを独自に制御したい場合はこの関数をoverrideして実装すること。
    virtual void cs_control(bool level);
    /// RSTピンの準備処理を行う。RSTピンを独自に制御したい場合はこの関数をoverrideして実装すること。
    virtual void init_rst(void);
    /// RSTピンを一度LOWにし、HIGHに戻して終了する。RSTピンを独自に制御したい場合はこの関数をoverrideして実装すること。
    virtual void reset(void);
    /// パネルの初期化コマンド列を返す。
    virtual const std::uint8_t* getInitCommands(std::uint8_t listno) const { (void)listno; return nullptr; }

    enum fastread_dir_t
    {
      fastread_nothing,
      fastread_horizontal,
      fastread_vertical,
    };
    virtual fastread_dir_t get_fastread_dir(void) const { return fastread_nothing; }

    void command_list(const std::uint8_t *addr);

  };

//----------------------------------------------------------------------------
 }
}
