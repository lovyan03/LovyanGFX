/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)
/----------------------------------------------------------------------------*/
#pragma once

#include "lgfx/v1/panel/Panel_FrameBufferBase.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_ED2208 : public Panel_FrameBufferBase
  {
    Panel_ED2208(void);
    ~Panel_ED2208(void);

    bool init(bool use_reset) override;

    color_depth_t setColorDepth(color_depth_t depth) override;

    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void waitDisplay(void) override;
    bool displayBusy(void) override;
    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

  private:

    uint8_t* _framebuffer = nullptr;

    // A full refresh takes about 15 to 30 s (depends on the image).
    static constexpr uint32_t REFRESH_TIMEOUT_MS = 40000;
    // Without a BUSY pin a refresh is taken to last this long.
    static constexpr uint32_t REFRESH_TIME_NO_BUSY_MS = 30000;
    // The controller is in deep sleep and needs a reset before the next command.
    bool _asleep = false;
    // This instance has started a refresh (it may still be running), and when.
    bool _refresh_started = false;
    uint32_t _refresh_ms = 0;

    bool _wait_busy(uint32_t timeout = 20000);   // the default suits commands; a refresh uses REFRESH_TIMEOUT_MS
    bool _exec_transfer(void);
    void _start_refresh(void);
    void _send_command(uint8_t cmd);
    void _send_data(uint8_t data);
    void _init_sequence(void);
    void _after_wake(void);

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] = {
        0xAA,  6, 0x49, 0x55, 0x20, 0x08, 0x09, 0x18,  // CMDH
        0x01,  1, 0x3F,
        0x00,  2, 0x5F, 0x69,
        0x05,  4, 0x40, 0x1F, 0x1F, 0x2C,
        0x08,  4, 0x6F, 0x1F, 0x1F, 0x22,
        0x06,  4, 0x6F, 0x1F, 0x17, 0x17,
        0x03,  4, 0x03, 0x54, 0x00, 0x44,

        0x60,  2, 0x02, 0x00,
        0x30,  1, 0x08,
        0x50,  1, 0x3F,

        0xE3,  1, 0x2F,
        0x84,  1, 0x01,
        0xFF, 0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------
 }
}
