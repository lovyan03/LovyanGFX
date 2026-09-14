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

#include "Panel_SSD1306.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  // Titan Micro TM1680 LED matrix driver (I2C, 24ROW x 16COM mode).
  // The frame buffer keeps the Panel_1bitOLED page layout and is converted
  // to the TM1680 RAM layout when pushed by display().
  // The protocol has no command/data prefix byte: configure Bus_I2C with prefix_len = 0.
  struct Panel_TM1680 : public Panel_1bitOLED
  {
    Panel_TM1680(void)
    {
      _cfg.memory_width  = _cfg.panel_width  = 16;
      _cfg.memory_height = _cfg.panel_height = 16;
      _auto_display = true;
    }

    bool init(bool use_reset) override;
    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setBrightness(uint8_t brightness) override;
    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

  protected:
    static constexpr uint8_t CMD_SYS_DIS    = 0x80;
    static constexpr uint8_t CMD_SYS_EN     = 0x81;
    static constexpr uint8_t CMD_LED_OFF    = 0x82;
    static constexpr uint8_t CMD_LED_ON     = 0x83;
    static constexpr uint8_t CMD_RC_MASTER  = 0x98; // internal RC oscillator, master mode
    static constexpr uint8_t CMD_COM_16NMOS = 0xA4; // 24ROW x 16COM (NMOS)
    static constexpr uint8_t CMD_PWM_DUTY   = 0xB0; // +0..15 = duty 1/16..16/16

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] = {
        CMD_SYS_DIS,
        CMD_COM_16NMOS,
        CMD_RC_MASTER,
        CMD_SYS_EN,
        CMD_PWM_DUTY + 1,
        CMD_LED_ON,
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
