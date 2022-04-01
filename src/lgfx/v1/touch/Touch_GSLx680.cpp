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
#include "Touch_GSLx680.hpp"
#include "gslx680/Touch_GSL1680E_800x480_FW.hpp"
#include "gslx680/Touch_GSL1680F_480x272_FW.hpp"
#include "gslx680/Touch_GSL1680F_800x480_FW.hpp"
#include "gslx680/Touch_GSLx680_320x320_FW.hpp"

#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {

  Touch_GSL1680F_800x480::Touch_GSL1680F_800x480(void)
  {
    _cfg.x_max = 800;
    _cfg.y_max = 480;
    setFirmWare(GSL1680F_800x480_FW, sizeof(GSL1680F_800x480_FW) / sizeof(gsl_fw_data));
  }

  Touch_GSL1680F_480x272::Touch_GSL1680F_480x272(void)
  {
    _cfg.x_max = 480;
    _cfg.y_max = 272;
    setFirmWare(GSL1680F_480x272_FW, sizeof(GSL1680F_480x272_FW) / sizeof(gsl_fw_data));
  }

  Touch_GSL1680E_800x480::Touch_GSL1680E_800x480(void)
  {
    _cfg.x_max = 800;
    _cfg.y_max = 480;
    setFirmWare(GSL1680E_800x480_FW, sizeof(GSL1680E_800x480_FW) / sizeof(gsl_fw_data));
  }

  Touch_GSLx680_320x320::Touch_GSLx680_320x320(void)
  {
    _cfg.x_max = 320;
    _cfg.y_max = 320;
    setFirmWare(GSLx680_320x320_FW, sizeof(GSLx680_320x320_FW) / sizeof(gsl_fw_data));
  }

//----------------------------------------------------------------------------

  bool Touch_GSLx680::_init(void)
  {
    if (_inited) return true;
    _inited = false;

    static constexpr const uint8_t buf[] = {0xBC, 0,0,0,0 };
    lgfx::i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, 0xE0, 0x88);  lgfx::delay(20);
    lgfx::i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, 0x80, 0x01);  lgfx::delay(5);
    lgfx::i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, 0xE4, 0x04);  lgfx::delay(5);
    lgfx::i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, 0xE0, 0x00);  lgfx::delay(20);
    lgfx::i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, 0xE0, 0x88);  lgfx::delay(20);
    lgfx::i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, 0xE4, 0x04);  lgfx::delay(10);
    lgfx::i2c::transactionWrite(_cfg.i2c_port, _cfg.i2c_addr, buf, sizeof(buf));  lgfx::delay(10);

    size_t length = _fw_size;
    auto ptr_fw = _fw_data;

    for (size_t i = 0; i < length; ++i) 
    {
      int retry = 3;
      while (lgfx::i2c::transactionWrite( _cfg.i2c_port
                                        , _cfg.i2c_addr
                                        , (uint8_t*)&ptr_fw[i]
                                        , (0xF0 == ptr_fw[i].offset) ? 2 : 5
                                        , _cfg.freq
                                        ).has_error()
        && --retry);
      if (retry == 0)
      {
        return false;  
      }
    }

    lgfx::i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, 0xE0, 0x88);  lgfx::delay(20);
    lgfx::i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, 0xE4, 0x04);  lgfx::delay(10);
    lgfx::i2c::writeRegister8(_cfg.i2c_port, _cfg.i2c_addr, 0xE0, 0x00);  lgfx::delay(20);

    _inited = true;
    return true;
  }

  bool Touch_GSLx680::init(void)
  {
    _inited = false;
    if (isSPI()) return false;

    if (_cfg.pin_int >= 0)
    {
      lgfx::pinMode(_cfg.pin_int, pin_mode_t::input_pullup);
    }
    return (lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value()) && _init();
  }

  uint_fast8_t Touch_GSLx680::getTouchRaw(touch_point_t *tp, uint_fast8_t count)
  {
    if (!_inited || count == 0) { return 0; }

    if (_cfg.pin_int < 0 || !gpio_in(_cfg.pin_int))
    {
      if (lgfx::i2c::beginTransaction(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, false).has_value())
      {
        static constexpr const uint8_t DATA_REG = 0x80;
        if (lgfx::i2c::writeBytes(_cfg.i2c_port, &DATA_REG, 1).has_error()
         || lgfx::i2c::restart(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, true).has_error()
         || lgfx::i2c::readBytes(_cfg.i2c_port, _readdata, 4).has_error()
         || (_readdata[0] == 0)
         || (_readdata[0] > 10)
         || lgfx::i2c::readBytes(_cfg.i2c_port, &_readdata[4], _readdata[0] * 4).has_error())
        {
          memset(_readdata, 0, sizeof(_readdata));
        }
      }
      lgfx::i2c::endTransaction(_cfg.i2c_port).has_value();
    }

    uint32_t points = std::min<uint_fast8_t>(count, _readdata[0]);
    if (points > 10) { points = 10; }
    for (size_t idx = 0; idx < points; ++idx)
    {
      auto data = &_readdata[4 + idx * 4];
      tp[idx].id = data[3] >> 4;
      tp[idx].x  = data[0] + ((data[1] & 0x0F) << 8);
      tp[idx].y  = data[2] + ((data[3] & 0x0F) << 8);
      tp[idx].size = 1;
    }
    return points;
  }

//----------------------------------------------------------------------------
 }
}
