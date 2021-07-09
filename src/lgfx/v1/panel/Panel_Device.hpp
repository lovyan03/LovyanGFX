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
  struct ILight;
  struct ITouch;
  struct touch_point_t;

  class Panel_Device : public IPanel
  {
  public:
    Panel_Device(void);

    struct config_t
    {
      /// Number of CS pin
      /// CS ピン番号
      std::int16_t pin_cs = -1;

      /// Number of RST pin
      /// RST ピン番号
      std::int16_t pin_rst = -1;

      /// Number of BUSY pin
      /// BUSY ピン番号
      std::int16_t pin_busy = -1;

      /// The maximum width of an image that the LCD driver can handle.
      /// LCDドライバが扱える画像の最大幅
      std::uint16_t memory_width = 240;

      /// The maximum height of an image that the LCD driver can handle.
      /// LCDドライバが扱える画像の最大高さ
      std::uint16_t memory_height = 240;

      /// Actual width of the display.
      /// 実際に表示できる幅
      std::uint16_t panel_width = 240;

      /// Actual height of the display.
      /// 実際に表示できる高さ
      std::uint16_t panel_height = 240;

      /// Number of offset pixels in the X direction.
      /// パネルのX方向オフセット量
      std::uint16_t offset_x = 0;

      /// Number of offset pixels in the Y direction.
      /// パネルのY方向オフセット量
      std::uint16_t offset_y = 0;

      /// Offset value in the direction of rotation. 0~7 (4~7 is upside down)
      /// 回転方向のオフセット 0~7 (4~7は上下反転)
      std::uint8_t offset_rotation = 0;

      /// Number of bits in dummy read before pixel readout.
      /// ピクセル読出し前のダミーリードのビット数
      std::uint8_t dummy_read_pixel = 8;

      /// Number of bits in dummy read before data readout.
      /// データ読出し前のダミーリードのビット数
      std::uint8_t dummy_read_bits = 1;

      /// Whether the data is readable or not.
      /// データ読出しが可能か否か
      bool readable = true;

      /// brightness inversion (e.g. IPS panel)
      /// 明暗の反転 (IPSパネルはtrueに設定)
      bool invert = false;

      /// Set the RGB/BGR color order.
      /// RGB=true / BGR=false パネルの赤と青が入れ替わってしまう場合 trueに設定
      bool rgb_order = false;

      /// 16-bit alignment of transmitted data
      /// 送信データの16bitアライメント データ長を16bit単位で送信するパネルの場合 trueに設定
      bool dlen_16bit = false;

      /// Whether or not to share the bus with the file system (if set to true, drawJpgFile etc. will control the bus)
      /// SD等のファイルシステムとのバス共有の有無 (trueに設定するとdrawJpgFile等でバス制御が行われる)
      bool bus_shared = true;
    };

    const config_t& config(void) const { return _cfg; }
    void config(const config_t& cfg) { _cfg = cfg; }

    virtual bool init(bool use_reset);
    virtual bool initTouch(void);

    virtual void initBus(void);
    virtual void releaseBus(void);
    void setBus(IBus* bus);
    void bus(IBus* bus) { setBus(bus); };
    IBus* getBus(void) const { return _bus; }
    IBus* bus(void) const { return _bus; }

    void setLight(ILight* light) { _light = light; }
    void light(ILight* light) { _light = light; }
    ILight* getLight(void) const { return _light; }
    ILight* light(void) const { return _light; }
    void setBrightness(std::uint8_t brightness) override;


    void setTouch(ITouch* touch);
    void touch(ITouch* touch) { setTouch(touch); }
    ITouch* getTouch(void) const { return _touch; }
    ITouch* touch(void) const { return _touch; }
    std::uint_fast8_t getTouchRaw(touch_point_t* tp, std::uint_fast8_t count);
    std::uint_fast8_t getTouch(touch_point_t* tp, std::uint_fast8_t count);
    void convertRawXY(touch_point_t *tp, std::uint_fast8_t count);
    void touchCalibrate(void);
    void setCalibrateAffine(float affine[6]);
    void setCalibrate(std::uint16_t *parameters);


    bool isReadable(void) const override { return _cfg.readable; }
    bool isBusShared(void) const override { return _cfg.bus_shared; }

    void initDMA(void) override;
    void waitDMA(void) override;
    bool dmaBusy(void) override;
    void display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h) override;

    void writeCommand(std::uint32_t data, std::uint_fast8_t length) override;
    void writeData(std::uint32_t data, std::uint_fast8_t length) override;
    //void writePixelsDMA(const std::uint8_t* data, std::uint32_t length) override;
    void writeImageARGB(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param) override;
    void copyRect(std::uint_fast16_t dst_x, std::uint_fast16_t dst_y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint_fast16_t src_x, std::uint_fast16_t src_y) override;

  protected:

    static constexpr std::uint8_t CMD_INIT_DELAY = 0x80;

    config_t _cfg;

    IBus* _bus = nullptr;
    ILight* _light = nullptr;
    ITouch* _touch = nullptr;
    bool _align_data = false;
    std::uint8_t _internal_rotation = 0;

    float _affine[6] = {1,0,0,0,1,0};  /// touch affine parameter

    /// Performs preparation processing for the CS pin.
    /// If you want to control the CS pin on your own, override this function and implement it.
    /// CSピンの準備処理を行う。CSピンを自前で制御する場合、この関数をoverrideして実装すること。
    virtual void init_cs(void);

    /// Controls the CS pin to go HIGH when the argument is true.
    /// If you want to control the CS pin on your own, override this function and implement it.
    /// 引数に応じてCSピンを制御する。false=LOW / true=HIGH。CSピンを自前で制御する場合、この関数をoverrideして実装すること。
    virtual void cs_control(bool level);

    /// Performs preparation processing for the RST pin.
    /// If you want to control the RST pin on your own, override this function and implement it.
    /// RSTピンの準備処理を行う。RSTピンを自前で制御する場合、この関数をoverrideして実装すること。
    virtual void init_rst(void);

    /// Bring the RST pin low once and bring it back high.
    /// If you want to control the RST pin on your own, override this function and implement it.
    /// RSTピンを一度LOWにし、HIGHに戻す。RSTピンを自前で制御する場合、この関数をoverrideして実装すること。
    virtual void reset(void);

    /// Get the panel initialization command sequence.
    /// パネルの初期化コマンド列を得る。無い場合はnullptrを返す。
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

  struct Panel_NULL : public Panel_Device
  {
    Panel_NULL(void) = default;

    void initBus(void) override {}
    void releaseBus(void) override {}

    bool init(bool) override { return false; }

    void beginTransaction(void) override {}
    void endTransaction(void) override {}

    color_depth_t setColorDepth(color_depth_t depth) override { return depth; }

    void setInvert(bool) override {}
    void setRotation(std::uint_fast8_t) override {}
    void setSleep(bool) override {}
    void setPowerSave(bool) override {}

    void writeCommand(std::uint32_t, std::uint_fast8_t) override {}
    void writeData(std::uint32_t, std::uint_fast8_t) override {}

    void initDMA(void) override {}
    void waitDMA(void) override {}
    bool dmaBusy(void) override { return false; }
    void waitDisplay(void) override {}
    bool displayBusy(void) override { return false; }
    void display(std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t) override {}
    bool isReadable(void) const override { return false; }
    bool isBusShared(void) const override { return false; }

    void writeBlock(std::uint32_t, std::uint32_t) override {}
    void setWindow(std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t) override {}
    void drawPixelPreclipped(std::uint_fast16_t, std::uint_fast16_t, std::uint32_t) override {}
    void writeFillRectPreclipped(std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint32_t) override {}
    void writeImage(std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, pixelcopy_t*, bool) override {}
    void writeImageARGB(std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, pixelcopy_t*) override {}
    void writePixels(pixelcopy_t*, std::uint32_t, bool) override {}

    std::uint32_t readCommand(std::uint_fast8_t, std::uint_fast8_t, std::uint_fast8_t) override { return 0; }
    std::uint32_t readData(std::uint_fast8_t, std::uint_fast8_t) override { return 0; }
    void readRect(std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, void*, pixelcopy_t*) override {}
    void copyRect(std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t) override {}
  };

//----------------------------------------------------------------------------
 }
}
