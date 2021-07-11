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

#include <cstdint>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct pixelcopy_t;

  enum bus_type_t
  {
    bus_unknown,
    bus_spi,
    bus_i2c,
    bus_parallel8,
    bus_parallel16,
  };

  struct IBus
  {
    virtual ~IBus(void) = default;

    virtual bus_type_t busType(void) const = 0;

    /// ペリフェラルの準備を行う。
    virtual bool init(void) = 0;

    /// ペリフェラルを解放する。
    virtual void release(void) = 0;

    /// 通信トランザクションを開始する。（ペリフェラルを占有する）
    virtual void beginTransaction(void) = 0;

    /// 通信トランザクションを終了する。（ペリフェラルの占有を終了する）
    virtual void endTransaction(void) = 0;

    /// 通信が完了するのを待機する
    virtual void wait(void) = 0;

    /// 現在通信中か否かを返す。true:通信中
    virtual bool busy(void) const = 0;

    /// DMA転送に必要なペリフェラルの準備を行う。
    virtual void initDMA(void) = 0;

    /// DMA転送キューを追加する。
    virtual void addDMAQueue(const std::uint8_t* data, std::uint32_t length) = 0; // { writeBytes(data, length, true); }

    /// 蓄積したDMA転送キューの送信を実行する。
    virtual void execDMAQueue(void) = 0;

    /// DMA用のバッファを取得する。バスの実装によっては内部的には2個のバッファを交互に使用する。
    /// 繰返し実行した場合は前回と異なるポインタを得るが、前々回と同じになる場合がある点に注意すること。
    virtual std::uint8_t* getDMABuffer(std::uint32_t length) = 0;

    /// 未送信のデータがあれば送信を開始する。
    virtual void flush(void) = 0;

    /// D/Cピンをlowにしてデータを送信する。
    virtual bool writeCommand(std::uint32_t data, std::uint_fast8_t bit_length) = 0;

    /// D/Cピンをhighにしてデータを送信する。
    virtual void writeData(std::uint32_t data, std::uint_fast8_t bit_length) = 0;

    /// D/Cピンをhighにして指定回数繰り返しデータを送信する。
    virtual void writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t count) = 0;

    /// pixelcopy構造体を利用してピクセルデータを送信する。
    virtual void writePixels(pixelcopy_t* pc, std::uint32_t length) = 0;

    /// 引数のバイト列を送信する。
    virtual void writeBytes(const std::uint8_t* data, std::uint32_t length, bool dc, bool use_dma) = 0;

    virtual void beginRead(void) = 0;
    virtual void endRead(void) = 0;
    virtual std::uint32_t readData(std::uint_fast8_t bit_length) = 0;
    virtual bool readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma = false) = 0;
    virtual void readPixels(void* dst, pixelcopy_t* pc, std::uint32_t length) = 0;
  };

  struct Bus_NULL : public IBus
  {
    bus_type_t busType(void) const override { return bus_type_t::bus_unknown; }
    bool init(void) override { return false; }
    void release(void) override {}
    void beginTransaction(void) override {}
    void endTransaction(void) override {}
    void wait(void) override {}
    bool busy(void) const override { return false; }

    void initDMA(void) override {}
    void addDMAQueue(const std::uint8_t*, std::uint32_t) override {}
    void execDMAQueue(void) override {}
    std::uint8_t* getDMABuffer(std::uint32_t) override { return nullptr; }

    void flush(void) override {}
    bool writeCommand(std::uint32_t, std::uint_fast8_t) override { return false; }
    void writeData(std::uint32_t, std::uint_fast8_t) override {}
    void writeDataRepeat(std::uint32_t, std::uint_fast8_t, std::uint32_t) override {}
    void writePixels(pixelcopy_t*, std::uint32_t) override {}
    void writeBytes(const std::uint8_t*, std::uint32_t, bool, bool) override {}

    void beginRead(void) override {}
    void endRead(void) override {}
    std::uint32_t readData(std::uint_fast8_t) override { return 0; }
    bool readBytes(std::uint8_t*, std::uint32_t, bool) override { return false; }
    void readPixels(void*, pixelcopy_t*, std::uint32_t) override {}
  };

//----------------------------------------------------------------------------
 }
}
