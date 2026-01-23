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

#include "Touch_ST7123.hpp"

#include "../../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr uint16_t ST7123_FW_VERSION_REG     = 0x0000;
  static constexpr uint16_t ST7123_FW_REVISION_REG    = 0x000C;
  static constexpr uint16_t ST7123_MAX_X_COORD_H_REG  = 0x0005;
  static constexpr uint16_t ST7123_MAX_X_COORD_L_REG  = 0x0006;
  static constexpr uint16_t ST7123_MAX_Y_COORD_H_REG  = 0x0007;
  static constexpr uint16_t ST7123_MAX_Y_COORD_L_REG  = 0x0008;
  static constexpr uint16_t ST7123_MAX_TOUCHES_REG    = 0x0009;
  static constexpr uint16_t ST7123_REPORT_COORD_0_REG = 0x0014;

  bool Touch_ST7123::_readParams(uint16_t reg, uint8_t* read_data, size_t read_len)
  {
    uint8_t write_data[2] = { (uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF) };
    return lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, write_data, sizeof(write_data), read_data, read_len, _cfg.freq).has_value();
  }

  bool Touch_ST7123::_read_fw_info(void)
  {
    uint8_t version = 0;
    uint8_t revision[4] = { 0 };
    struct {
        uint8_t max_x_h;
        uint8_t max_x_l;
        uint8_t max_y_h;
        uint8_t max_y_l;
        uint8_t max_touches;
    } info;

    if (_readParams(ST7123_FW_VERSION_REG, &version, 1)
     && _readParams(ST7123_FW_REVISION_REG, revision, 4)
     && _readParams(ST7123_MAX_X_COORD_H_REG, (uint8_t*)&info, sizeof(info)))
    {
      uint32_t sum = version + revision[0] + revision[1] + revision[2] + revision[3]
                  + info.max_x_h + info.max_x_l + info.max_y_h + info.max_y_l + info.max_touches;
      if (sum == 0)
      {
        return false;
      }
    }
    // printf("Firmware version: %d(%d.%d.%d.%d), Max.X: %d, Max.Y: %d, Max.Touchs: %d",
    //          version, revision[0], revision[1], revision[2], revision[3], ((uint16_t)info.max_x_h << 8) | info.max_x_l,
    //          ((uint16_t)info.max_y_h << 8) | info.max_y_l, info.max_touches);
// fflush(stdout);
// delay(1000);
    return true;
  }

  bool Touch_ST7123::init(void)
  {
    if (_inited) return true;

    if (isSPI()) return false;

    if (_cfg.pin_rst >= 0)
    {
      lgfx::pinMode(_cfg.pin_rst, pin_mode_t::output);
      lgfx::gpio_lo(_cfg.pin_rst);
      lgfx::delay(5);
      lgfx::gpio_hi(_cfg.pin_rst);
    }

    if (lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value())
    {
      for (int retry = 6; retry; --retry)
      {
  // printf("st7123 touch retry:%d\n", retry);
  // lgfx::delay(1000);
        if (_read_fw_info())
        {
          _inited = true;
          wakeup();
          return true;
        }
      }
    }
    return false;
  }

  uint_fast8_t Touch_ST7123::getTouchRaw(touch_point_t *tp, uint_fast8_t count)
  {
    struct touch_report_t {
        uint8_t x_h: 6;
        uint8_t reserved_6: 1;
        uint8_t valid: 1;
        uint8_t x_l;
        uint8_t y_h;
        uint8_t y_l;
        uint8_t area;
        uint8_t intensity;
        uint8_t reserved_49_55;
    };
    struct adv_info_t {
        uint8_t reserved_0_1: 2;
        uint8_t with_prox: 1;
        uint8_t with_coord: 1;
        uint8_t prox_status: 3;
        uint8_t rst_chip: 1;
    };

    size_t valid_count = 0;
    
    adv_info_t adv_info;
    _readParams(0x0010, (uint8_t *)&adv_info, 1);
    if (adv_info.with_coord) {
      uint8_t max_touches = 0;
      _readParams(ST7123_MAX_TOUCHES_REG, &max_touches, 1);
      if (max_touches > max_touch_points) {
        max_touches = max_touch_points;
      }
      touch_report_t touch_report[max_touch_points];
      _readParams(ST7123_REPORT_COORD_0_REG, (uint8_t *)&touch_report[0], sizeof(touch_report_t) * max_touches);

      for (size_t i = 0; i < max_touches; i++) {
        if (!touch_report[i].valid) { continue; }
        tp[valid_count].id = i;
        tp[valid_count].x = touch_report[i].x_h << 8 | touch_report[i].x_l;
        tp[valid_count].y = touch_report[i].y_h << 8 | touch_report[i].y_l;
        tp[valid_count].size = touch_report[i].area;
  // printf("id:%d x:%d y:%d size:%d\n", tp[valid_count].id, tp[valid_count].x, tp[valid_count].y, tp[valid_count].size);
        valid_count++;
        if (valid_count >= count) break;
      }
    }
// printf("count:%d\n", valid_count);

    return valid_count;
  }

//----------------------------------------------------------------------------
 }
}
