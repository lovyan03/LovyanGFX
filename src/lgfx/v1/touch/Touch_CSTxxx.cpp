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

// CST816 info from here...
// https://www.waveshare.com/w/upload/c/c2/CST816S_register_declaration.pdf

#include "Touch_CSTxxx.hpp"

#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr uint8_t CST816S_TOUCH_REG  = 0x01;
  static constexpr uint8_t CST816S_SLEEP_REG  = 0xA5;
  static constexpr uint8_t CST816S_CHIPID_REG = 0xA7;

  static constexpr uint8_t CST816S_SLEEP_IN   = 0x03;

  bool Touch_CST816S::_write_reg(uint8_t reg, uint8_t val)
  {
    return i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, reg, val, 0, _cfg.freq).has_value();
  }

  bool Touch_CST816S::_write_regs(uint8_t* val, size_t length)
  {
    return i2c::transactionWrite(_cfg.i2c_port, _cfg.i2c_addr, val, length, _cfg.freq).has_value();
  }

  bool Touch_CST816S::_read_reg(uint8_t reg, uint8_t *data, size_t length)
  {
    return lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, &reg, 1, data, length, _cfg.freq).has_value();
  }

  bool Touch_CST816S::_check_init(void)
  {
    if (_inited) return true;

    uint8_t tmp[3] = { 0 };
    _inited = _write_reg(0x00, 0x00)
          && _read_reg(CST816S_CHIPID_REG, tmp, 3);

// CST816SのINT機能には問題があり、LOWパルス出力期間中のタッチ精度が顕著に劣化する。
// 0xFAレジスタを0x40とすると接触中に定期的にINTパルスを発生できる。
// また、0xEDを操作するとパルスの長さを変更できるが、これらを併用するとタッチ性能が顕著に劣化する。
//  _write_reg(0xFA, 0x40); // 0x40 = 触れている間、連続的にINTパルスを発生する。

    _write_reg(0xFA, 0x20); // 0x20 = 変化を検出したときINTパルスを発生する。
    _write_reg(0xED, 20); // INT LOW パルス長さ 20 == 2 msec
// ESP_LOGV("LGFX","CST816S id:%02x %02x %02x", tmp[0], tmp[1], tmp[2]);
    return _inited;
  }

//----------------------------------------------------------------------------
  bool Touch_CST816S::init(void)
  {
    _inited = false;

    if (_cfg.pin_rst >= 0)
    {
      lgfx::pinMode(_cfg.pin_rst, pin_mode_t::output);
      lgfx::gpio_lo(_cfg.pin_rst);
      lgfx::delay(10);
      lgfx::gpio_hi(_cfg.pin_rst);
      lgfx::delay(10);
    }

    if (_cfg.pin_int >= 0)
    { // intピンのプルアップ処理を行うが、「触れている間 LOW」の設定方法がないため使用していない。
      // GPIO割込みを使用すれば良いが、Arduinoに依存せず解決する必要があるため、対応を保留する。
      lgfx::pinMode(_cfg.pin_int, pin_mode_t::input_pullup);
    }
    lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value();

    // 画面に触れていない時、I2C通信でCST816が見つからない事がある。
    // CST816S は恐らく省電力モード中はI2Cでの通信に応答しないものと思われる。
    // 通信の成否による初期化の成否の判定ができないため、ひとまず true を返す。
    return true;
  }

  size_t Touch_CST816S::_read_data(uint8_t* readdata)
  {
    size_t res = 0;
    if (lgfx::i2c::beginTransaction(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, false))
    {
      readdata[0] = 0x02;
      if (lgfx::i2c::writeBytes(_cfg.i2c_port, readdata, 1)
      && lgfx::i2c::restart(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, true)
      && lgfx::i2c::readBytes(_cfg.i2c_port, readdata, 1))
      {
        uint_fast8_t points = std::min<uint_fast8_t>(max_touch_points, readdata[0] & 0x0Fu);
        if (points)
        {
          if (lgfx::i2c::readBytes(_cfg.i2c_port, &readdata[1], points * 6 - 2))
          {
            res = points * 6 - 1;
          }
        }
        else
        {
          res = 1;
        }
      }
      lgfx::i2c::endTransaction(_cfg.i2c_port).has_value();
    }
    return res;
  }

  uint_fast8_t Touch_CST816S::getTouchRaw(touch_point_t *tp, uint_fast8_t count)
  {
    if (!_inited && !_check_init()) return 0;

    // if (count > max_touch_points) { count = max_touch_points; }

    uint32_t msec = lgfx::millis();
    uint32_t diff_msec = msec - _last_update;

    if (diff_msec < 10 && _wait_cycle)
    {
      --_wait_cycle;
      if (_cfg.pin_int < 0 || gpio_in(_cfg.pin_int)) {
        return 0;
      }
    }
    _last_update = msec;

    uint8_t readdata[6];
    int retry = 3;
    do
    {
      // I2Cでの応答が得られない場合は触れていないものとして扱う。
      if (!_read_reg(0x02, readdata, 6))
      {
        count = 0;
        break;
      }
      count = readdata[0];
    } while (count > 1 && --retry);
    if (!retry)
    {
      count = 0;
    }
    _wait_cycle = count ? 0 : 16;

    if (count)
    {
      // ESP_LOGV("CST816S", "%02x %02x %02x %02x %02x %02x %02x %02x %d %d",
      //                     readdata[0], readdata[1], readdata[2], readdata[3],
      //                     readdata[4], readdata[5], readdata[6], readdata[7], _inited, lgfx::gpio_in(_cfg.pin_int));
      tp[0].id = 0;
      tp[0].size = 1;
      tp[0].x = readdata[2] | (readdata[1] & 0x0F) << 8;
      tp[0].y = readdata[4] | (readdata[3] & 0x0F) << 8;
    }
    return count;
  }

//----------------------------------------------------------------------------


    bool Touch_CST226::init(void)
    {
      _inited = false;

      if (_cfg.pin_rst >= 0) {
        lgfx::pinMode(_cfg.pin_rst, pin_mode_t::output);
        lgfx::gpio_lo(_cfg.pin_rst);
        lgfx::delay(10);
        lgfx::gpio_hi(_cfg.pin_rst);
        lgfx::delay(10);
      }

      if (_cfg.pin_int >= 0) {
        lgfx::pinMode(_cfg.pin_int, pin_mode_t::input_pullup);
      }

      lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value();

      return true;
    }


    bool Touch_CST226::_check_init(void)
    {
      if (_inited) return true;

      { // enter command mode
        if( ! _write_reg16(0xd101)) // 0xd101 = ENUM_MODE_DEBUG_INFO
          return false;
      }

      lgfx::delay(10);

      { // verify chip integrity
        uint32_t chip_ic_checkcode;
        if( ! _read_reg16(0xd1fc, (uint8_t*)&chip_ic_checkcode, 4) ) // 0xd1fc = read chip_ic_checkcode
          return false;
        // printf("Touch Chip_ic_checkcode: 0x%08lx (expecting 0xcacaxxxx)\n", chip_ic_checkcode); // e.g. Chip_ic_checkcode: 0xcaca5fdc
        if( (chip_ic_checkcode & 0xffff0000) != 0xcaca0000 )
          return false;
      }

      lgfx::delay(10);

      { // read touch resolution
        uint16_t resolution[2];
        if( ! _read_reg16(0xd1f8, (uint8_t*)&resolution, 4) ) // 0xd1f8 = read chip resolution
          return false;
        // printf("Touch Resolution: %d x %d\n", resolution[1], resolution[0]); // e.g. Resolution: 600 x 450
      }

      lgfx::delay(10);

      { // identify the chip id and project id (NOTE: this can be used in autodetect routine)
        uint16_t c_p_info[2];
        if( ! _read_reg16(0xd204, (uint8_t*)&c_p_info, 4) ) // 0xd204 = read chip type and project id
          return false;
        if( c_p_info[1] != 0xa8 ) // 0xa8 is chip ID for CST226 and CST226SE
          return false;
        // printf("Touch Chip id: 0x%04x, project id: 0x%04x\n", c_p_info[1], c_p_info[0]); // e.g. Chip id: 0x00a8, project id: 0x465f
      }

      lgfx::delay(10);

      { // check if firmware is installed
        uint32_t fw_chksum[2];
        if( ! _read_reg16(0xd208, (uint8_t*)fw_chksum, 8) ) // 0xd208 = read fw version + checksum
          return false;
        if( fw_chksum[0]==0xA5A5A5A5 ) // the chip has no firmware !!
          return false;
        // printf("Touch FW Version: 0x%08lx, Checksum: 0x%08lx\n", fw_chksum[0], fw_chksum[1]); // e.g. FW version 0x01000001, Checksum: 0x140f47c8
      }

      lgfx::delay(10);

      { // exit command mode
        if( ! _write_reg16(0xd109) ) // 0xd109 = ENUM_MODE_NORMAL, 0xd109 = ENUM_MODE_FACTORY
          return false;
      }

      lgfx::delay(5);
      _inited = true;
      return true;
    }


    void Touch_CST226::wakeup(void)
    {
      init();
    }


    void Touch_CST226::sleep(void)
    {
      _write_reg16(0xd105); // 0xD105 = deep sleep
    }


    uint_fast8_t Touch_CST226::getTouchRaw(touch_point_t* tp, uint_fast8_t count)
    {
      if (!_inited && !_check_init()) return 0;
      if (count > max_touch_points || count == 0) return 0;
      auto requested_count = count;

      uint32_t msec = lgfx::millis();
      uint32_t diff_msec = msec - _last_update;

      if (diff_msec < 10 && _wait_cycle)
      {
        --_wait_cycle;
        if (_cfg.pin_int < 0 || gpio_in(_cfg.pin_int)) {
          return 0;
        }
      }
      _last_update = msec;

      uint8_t readdata[28];
      uint8_t bytes_to_read = count==1 ? 8 : 28;

      if( ! _read_reg8(0, readdata, bytes_to_read) )
      {
        return 0;
      }

      if (readdata[6] != 0xab) return 0;
      if (readdata[0] == 0xab) return 0;
      if (readdata[5] == 0x80) return 0;
      count = readdata[5] & 0x7f;
      if (count > max_touch_points || count == 0) {
        _write_reg16(0x00ab); // Publish the synchronization command to complete the coordinate read
        return 0;
      }
      _wait_cycle = count ? 0 : 16;

      if (count)
      {
        if( requested_count > 1 ) // an array of touch_point_t was provided
        {
          int pIdx = 0; // points index with corrected offset
          for (int i=0; i<count; i++) {
            tp[i].id = i;
            tp[i].size = readdata[pIdx+4]; // pressure
            tp[i].x = (uint16_t)((readdata[pIdx+1] << 4) | ((readdata[pIdx+3] >> 4) & 0xf));
            tp[i].y = (uint16_t)((readdata[pIdx+2] << 4) | (readdata[pIdx+3] & 0xf));
            pIdx += i==0 ? 7 : 5; // first point is followed by 2 extra bytes, next points are consecutive
          }
        }
        else // a single touch_point_t was provided
        {
          tp[0].id = 0;
          tp[0].size = readdata[4]; // pressure
          tp[0].x = (uint16_t)((readdata[1] << 4) | ((readdata[3] >> 4) & 0xf));
          tp[0].y = (uint16_t)((readdata[2] << 4) | (readdata[3] & 0xf));
        }
      }
      return count;
    }


    bool Touch_CST226::_write_reg16(uint16_t reg)
    {
      reg = (reg<<8)+(reg>>8); // swap
      return lgfx::i2c::transactionWrite(_cfg.i2c_port, _cfg.i2c_addr, (uint8_t*)&reg, 2, _cfg.freq).has_value();
    }


    bool Touch_CST226::_read_reg16(uint16_t reg, uint8_t *data, size_t length)
    {
      reg = (reg<<8)+(reg>>8); // swap
      return lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, (uint8_t*)&reg, 2, data, length, _cfg.freq).has_value();
    }


    bool Touch_CST226::_read_reg8(uint8_t reg, uint8_t *data, size_t length)
    {
      return lgfx::i2c::transactionWriteRead(_cfg.i2c_port, _cfg.i2c_addr, &reg, 1, data, length, _cfg.freq).has_value();
    }

//----------------------------------------------------------------------------

 }
}
