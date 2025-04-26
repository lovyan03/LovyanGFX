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

#include "../Touch.hpp"

namespace lgfx
{
 inline namespace v1
 {

  // Hynitron CSTxxx family touch devices

  struct Touch_CST226; // CST226/CST226SE
  struct Touch_CST816S;


//----------------------------------------------------------------------------

  struct Touch_CST816S : public ITouch
  {
    enum CS816S_GESTURE {
      NONE         = 0x00,
      SWIPE_UP     = 0x01,
      SWIPE_DOWN   = 0x02,
      SWIPE_LEFT   = 0x03,
      SWIPE_RIGHT  = 0x04,
      SINGLE_CLICK = 0x05,
      DOUBLE_CLICK = 0x0B,
      LONG_PRESS   = 0x0C
    };

    Touch_CST816S(void)
    {
      _cfg.i2c_addr = 0x15;
      _cfg.x_min    = 0;
      _cfg.x_max    = 320;
      _cfg.y_min    = 0;
      _cfg.y_max    = 320;
    }

    bool init(void) override;

    void wakeup(void) override {};
    void sleep(void) override {};

    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override;

  private:
    enum
    {
      max_touch_points = 1
    };
    uint32_t _last_update = 0;
    uint8_t _wait_cycle = 0;

    bool _check_init(void);
    bool _write_reg(uint8_t reg, uint8_t val);
    bool _write_regs(uint8_t* val, size_t length);
    bool _read_reg(uint8_t reg, uint8_t *data, size_t length);
    size_t _read_data(uint8_t* data);
  };


//----------------------------------------------------------------------------

  // Datasheet: https://github.com/lewisxhe/SensorLib/blob/master/datasheet/海栎创触摸芯片移植手册-v3.5-20220701(1).pdf

  struct Touch_CST226 : public ITouch
  {
    enum CST226_GESTURE { // D3h
      NONE         = 0x28,
      SWIPE_LEFT   = 0x20,
      SWIPE_RIGHT  = 0x21,
      SWIPE_UP     = 0x22,
      SWIPE_DOWN   = 0x23,
      DOUBLE_CLICK = 0x24,
      GESTURE_O    = 0x30,
      GESTURE_W    = 0x31,
      GESTURE_M    = 0x32,
      GESTURE_E    = 0x33,
      GESTURE_C    = 0x34,
      GESTURE_S    = 0x46,
      GESTURE_V    = 0x54,
      GESTURE_Z    = 0x65,
    };

    Touch_CST226(void)
    {
      _cfg.i2c_addr = 0x5A; // 0x5a (CST226SE) or 0x1A (CST226)
      _cfg.x_min    = 0;
      _cfg.x_max    = 720;
      _cfg.y_min    = 0;
      _cfg.y_max    = 1280;
    }

    bool init(void) override;
    void wakeup(void) override;
    void sleep(void) override;
    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override;

  private:

    enum
    {
      max_touch_points = 5,
      max_keys = 3
    };

    uint32_t _last_update = 0;
    uint8_t _wait_cycle = 0;

    bool _check_init(void);
    bool _read_reg8(uint8_t reg, uint8_t *data, size_t length);
    bool _read_reg16(uint16_t reg, uint8_t *data, size_t length);
    bool _write_reg16(uint16_t reg);

  };



//----------------------------------------------------------------------------
 }
}
