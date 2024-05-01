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

#include "../../panel/Panel_FrameBufferBase.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_RGB : public Panel_FrameBufferBase
  {
  public:

    Panel_RGB(void);
    virtual ~Panel_RGB(void);

    struct config_detail_t
    {
      int8_t pin_cs = -1;
      int8_t pin_sclk = -1;
      int8_t pin_mosi = -1;

      // unimplemented... : TODO : 0=SRAM only (no use PSRAM) / 1=both(half PSRAM and half SRAM) / 2=PSRAM only (no use SRAM)
      uint8_t use_psram = 2;
    };
    const config_detail_t& config_detail(void) const { return _config_detail; }
    void config_detail(const config_detail_t& config_detail) { _config_detail = config_detail; }

    color_depth_t setColorDepth(color_depth_t) override { return _write_depth; }

    void setPsram( bool use_psram ) { _config_detail.use_psram = use_psram; }

    bool init(bool) override;

    void writeCommand(uint32_t, uint_fast8_t) override;
    void writeData(uint32_t, uint_fast8_t) override;

  protected:

    config_detail_t _config_detail;

    bool initFrameBuffer(uint_fast16_t w, uint_fast16_t h, color_depth_t depth, uint8_t chunk_lines, uint8_t use_psram);
    void deinitFrameBuffer(void);

    uint8_t _lines_per_chunk = 4;

    uint8_t* _frame_buffer = nullptr;
/*
    uint16_t* _allocated_list = nullptr;
    struct config_detail_t
    {
      // 0=SRAM only (no use PSRAM) / 1=both(half PSRAM and half SRAM) / 2=PSRAM only (no use SRAM)
      uint8_t use_psram = 0;
    };

    void setPsram( bool use_psram ) { _config_detail.use_psram = use_psram; config_detail(_config_detail); }

    int32_t getScanLine(void) override;

    const config_detail_t& config_detail(void) const { return _config_detail; }
    void config_detail(const config_detail_t& config_detail);

    void* getLineBuffer(uint_fast16_t y);

  protected:

    config_detail_t _config_detail;

    void deinit(void);

  private:

    bool _started = false;
//*/
  };


  struct Panel_ST7701_Base : public Panel_RGB
  {
	bool init(bool) override;
  };

  struct Panel_ST7701 : public Panel_ST7701_Base
  {
  protected:
    const uint8_t* getInitCommands(uint8_t listno) const override;
  };

  struct Panel_ST7701_guition_esp32_4848S040: public Panel_ST7701_Base
  {
  protected:
    const uint8_t* getInitCommands(uint8_t listno) const override;
  };

  struct Panel_GC9503 : public Panel_RGB
  {
    bool init(bool) override;
  protected:
    const uint8_t* getInitCommands(uint8_t listno) const override;
  };

//----------------------------------------------------------------------------
 }
}
