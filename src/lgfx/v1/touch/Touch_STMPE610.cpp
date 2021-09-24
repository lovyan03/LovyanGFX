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
#include "Touch_STMPE610.hpp"

#include "../../internal/algorithm.h"
#include "../platforms/common.hpp"

/** STMPE610 Address **/
static constexpr uint8_t STMPE_ADDR = 0x41;

/** Reset Control **/
static constexpr uint8_t STMPE_SYS_CTRL1 = 0x03;
static constexpr uint8_t STMPE_SYS_CTRL1_RESET = 0x02;

/** Clock Contrl **/
static constexpr uint8_t STMPE_SYS_CTRL2 = 0x04;

/** Touchscreen controller setup **/
static constexpr uint8_t STMPE_TSC_CTRL = 0x40;
static constexpr uint8_t STMPE_TSC_CTRL_EN = 0x01;
static constexpr uint8_t STMPE_TSC_CTRL_XYZ = 0x00;
static constexpr uint8_t STMPE_TSC_CTRL_XY = 0x02;

/** Interrupt control **/
static constexpr uint8_t STMPE_INT_CTRL = 0x09;
static constexpr uint8_t STMPE_INT_CTRL_POL_HIGH = 0x04;
static constexpr uint8_t STMPE_INT_CTRL_POL_LOW = 0x00;
static constexpr uint8_t STMPE_INT_CTRL_EDGE = 0x02;
static constexpr uint8_t STMPE_INT_CTRL_LEVEL = 0x00;
static constexpr uint8_t STMPE_INT_CTRL_ENABLE = 0x01;
static constexpr uint8_t STMPE_INT_CTRL_DISABLE = 0x00;

/** Interrupt enable **/
static constexpr uint8_t STMPE_INT_EN = 0x0A;
static constexpr uint8_t STMPE_INT_EN_TOUCHDET = 0x01;
static constexpr uint8_t STMPE_INT_EN_FIFOTH = 0x02;
static constexpr uint8_t STMPE_INT_EN_FIFOOF = 0x04;
static constexpr uint8_t STMPE_INT_EN_FIFOFULL = 0x08;
static constexpr uint8_t STMPE_INT_EN_FIFOEMPTY = 0x10;
static constexpr uint8_t STMPE_INT_EN_ADC = 0x40;
static constexpr uint8_t STMPE_INT_EN_GPIO = 0x80;

/** Interrupt status **/
static constexpr uint8_t STMPE_INT_STA = 0x0B;
static constexpr uint8_t STMPE_INT_STA_TOUCHDET = 0x01;

/** ADC control **/
static constexpr uint8_t STMPE_ADC_CTRL1 = 0x20;
static constexpr uint8_t STMPE_ADC_CTRL1_12BIT = 0x08;
static constexpr uint8_t STMPE_ADC_CTRL1_10BIT = 0x00;

/** ADC control **/
static constexpr uint8_t STMPE_ADC_CTRL2 = 0x21;
static constexpr uint8_t STMPE_ADC_CTRL2_1_625MHZ = 0x00;
static constexpr uint8_t STMPE_ADC_CTRL2_3_25MHZ = 0x01;
static constexpr uint8_t STMPE_ADC_CTRL2_6_5MHZ = 0x02;

/** Touchscreen controller configuration **/
static constexpr uint8_t STMPE_TSC_CFG = 0x41;
static constexpr uint8_t STMPE_TSC_CFG_1SAMPLE = 0x00;
static constexpr uint8_t STMPE_TSC_CFG_2SAMPLE = 0x40;
static constexpr uint8_t STMPE_TSC_CFG_4SAMPLE = 0x80;
static constexpr uint8_t STMPE_TSC_CFG_8SAMPLE = 0xC0;
static constexpr uint8_t STMPE_TSC_CFG_DELAY_10US = 0x00;
static constexpr uint8_t STMPE_TSC_CFG_DELAY_50US = 0x08;
static constexpr uint8_t STMPE_TSC_CFG_DELAY_100US = 0x10;
static constexpr uint8_t STMPE_TSC_CFG_DELAY_500US = 0x18;
static constexpr uint8_t STMPE_TSC_CFG_DELAY_1MS = 0x20;
static constexpr uint8_t STMPE_TSC_CFG_DELAY_5MS = 0x28;
static constexpr uint8_t STMPE_TSC_CFG_DELAY_10MS = 0x30;
static constexpr uint8_t STMPE_TSC_CFG_DELAY_50MS = 0x38;
static constexpr uint8_t STMPE_TSC_CFG_SETTLE_10US = 0x00;
static constexpr uint8_t STMPE_TSC_CFG_SETTLE_100US = 0x01;
static constexpr uint8_t STMPE_TSC_CFG_SETTLE_500US = 0x02;
static constexpr uint8_t STMPE_TSC_CFG_SETTLE_1MS = 0x03;
static constexpr uint8_t STMPE_TSC_CFG_SETTLE_5MS = 0x04;
static constexpr uint8_t STMPE_TSC_CFG_SETTLE_10MS = 0x05;
static constexpr uint8_t STMPE_TSC_CFG_SETTLE_50MS = 0x06;
static constexpr uint8_t STMPE_TSC_CFG_SETTLE_100MS = 0x07;

/** FIFO level to generate interrupt **/
static constexpr uint8_t STMPE_FIFO_TH = 0x4A;

/** Current filled level of FIFO **/
static constexpr uint8_t STMPE_FIFO_SIZE = 0x4C;

/** Current status of FIFO **/
static constexpr uint8_t STMPE_FIFO_STA = 0x4B;
static constexpr uint8_t STMPE_FIFO_STA_RESET = 0x01;
static constexpr uint8_t STMPE_FIFO_STA_OFLOW = 0x80;
static constexpr uint8_t STMPE_FIFO_STA_FULL = 0x40;
static constexpr uint8_t STMPE_FIFO_STA_EMPTY = 0x20;
static constexpr uint8_t STMPE_FIFO_STA_THTRIG = 0x10;

/** Touchscreen controller drive I **/
static constexpr uint8_t STMPE_TSC_I_DRIVE = 0x58;
static constexpr uint8_t STMPE_TSC_I_DRIVE_20MA = 0x00;
static constexpr uint8_t STMPE_TSC_I_DRIVE_50MA = 0x01;

/** Data port for TSC data address **/
static constexpr uint8_t STMPE_TSC_DATA_X = 0x4D;
static constexpr uint8_t STMPE_TSC_DATA_Y = 0x4F;
static constexpr uint8_t STMPE_TSC_FRACTION_Z = 0x56;

/** GPIO **/
static constexpr uint8_t STMPE_GPIO_SET_PIN = 0x10;
static constexpr uint8_t STMPE_GPIO_CLR_PIN = 0x11;
static constexpr uint8_t STMPE_GPIO_DIR = 0x13;
static constexpr uint8_t STMPE_GPIO_ALT_FUNCT = 0x17;

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  uint8_t Touch_STMPE610::readRegister8(uint8_t reg)
  {
    if (isSPI()) {
      uint8_t tmp[2] = { (uint8_t)(0x80 | reg), 0 };
      spi::beginTransaction(_cfg.spi_host, _cfg.freq, _spi_mode);
      lgfx::gpio_lo(_cfg.pin_cs);
      spi::writeBytes(_cfg.spi_host, tmp, 1);
      spi::readBytes(_cfg.spi_host, &tmp[1], 1);
      spi::endTransaction(_cfg.spi_host);
      lgfx::gpio_hi(_cfg.pin_cs);
      return tmp[1];
    }
    return 0;
  }

  void Touch_STMPE610::writeRegister8(uint8_t reg, uint8_t val)
  {
    if (isSPI()) {
      uint8_t tmp[2] = { reg, val };
      spi::beginTransaction(_cfg.spi_host, _cfg.freq, _spi_mode);
      lgfx::gpio_lo(_cfg.pin_cs);
      spi::writeBytes(_cfg.spi_host, tmp, 1);
      spi::writeBytes(_cfg.spi_host, &tmp[1], 1);
      spi::endTransaction(_cfg.spi_host);
      lgfx::gpio_hi(_cfg.pin_cs);
    }
  }

  int Touch_STMPE610::getVersion(void)
  {
    if (isSPI()) {
      lgfx::pinMode(_cfg.pin_cs, lgfx::pin_mode_t::output);
      uint16_t v = readRegister8(0) << 8;
      v |= readRegister8(1);
//Serial.printf("getVersion:%04x \r\n", v);
      return v;
    }
    return 0;
  }

  bool Touch_STMPE610::init(void)
  {
    _inited = false;
    if (!isSPI()) return false;

    lgfx::gpio_hi(_cfg.pin_cs);
    lgfx::pinMode(_cfg.pin_cs, lgfx::pin_mode_t::output);
    if (!lgfx::spi::init(_cfg.spi_host, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi).has_value()) return false;

    _spi_mode = 0;
    if (0x811 != getVersion()) {
      _spi_mode = 1;
      if (0x811 != getVersion()) {
        return false;
      }
    }

    writeRegister8(STMPE_SYS_CTRL1, STMPE_SYS_CTRL1_RESET);
    delay(10);

    for (uint8_t i = 0; i < 65; i++) {
      readRegister8(i);
    }

    writeRegister8(STMPE_SYS_CTRL2, 0x0); // turn on clocks!
    writeRegister8(STMPE_TSC_CTRL,
                   STMPE_TSC_CTRL_XYZ | STMPE_TSC_CTRL_EN); // XYZ and enable!
    // Serial.println(readRegister8(STMPE_TSC_CTRL), HEX);
    writeRegister8(STMPE_INT_EN, STMPE_INT_EN_TOUCHDET);
    writeRegister8(STMPE_ADC_CTRL1, STMPE_ADC_CTRL1_10BIT |
                                        (0x6 << 4)); // 96 clocks per conversion
    writeRegister8(STMPE_ADC_CTRL2, STMPE_ADC_CTRL2_6_5MHZ);
    writeRegister8(STMPE_TSC_CFG, STMPE_TSC_CFG_8SAMPLE |
                                      STMPE_TSC_CFG_DELAY_100US |
                                      STMPE_TSC_CFG_SETTLE_500US);
//    writeRegister8(STMPE_TSC_CFG, STMPE_TSC_CFG_4SAMPLE |
//                                      STMPE_TSC_CFG_DELAY_1MS |
//                                      STMPE_TSC_CFG_SETTLE_5MS);
    writeRegister8(STMPE_TSC_FRACTION_Z, 0x6);
    writeRegister8(STMPE_FIFO_TH, 1);
    writeRegister8(STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);
    writeRegister8(STMPE_FIFO_STA, 0); // unreset
    writeRegister8(STMPE_TSC_I_DRIVE, STMPE_TSC_I_DRIVE_50MA);
    writeRegister8(STMPE_INT_STA, 0xFF); // reset all ints
    writeRegister8(STMPE_INT_CTRL,
                   STMPE_INT_CTRL_POL_HIGH | STMPE_INT_CTRL_ENABLE);

    _inited = true;
    return true;
  }

  void Touch_STMPE610::wakeup(void)
  {
    if (!_inited) return;
  }

  void Touch_STMPE610::sleep(void)
  {
    if (!_inited) return;
  }

  uint_fast8_t Touch_STMPE610::getTouchRaw(touch_point_t* __restrict tp, uint_fast8_t count)
  {
    if (!_inited || count == 0) return 0;

    if (isSPI()) {
      uint8_t data[8];
      spi::beginTransaction(_cfg.spi_host, _cfg.freq, _spi_mode);
      lgfx::gpio_lo(_cfg.pin_cs);
      data[0] = 0x80 | STMPE_FIFO_STA; // check buffer empty
      data[1] = 0;
      data[2] = 0x80 | STMPE_TSC_CTRL; // check touch
      data[3] = 0;
      data[4] = STMPE_INT_STA;
      data[5] = 0xFF;
      spi::readBytes(_cfg.spi_host, data, 6);
      bool empty = data[1] & STMPE_FIFO_STA_EMPTY;
      bool press = data[3] & 0x80;
      if (press != _last_press) {
        _last_press = press & !empty;
        press = false;
      }
      do {
        memset(data, 0xD7, 4);
        data[4] = 0x80 | STMPE_FIFO_STA;
        data[5] = 0;
        spi::readBytes(_cfg.spi_host, data, 6);
      } while (!(data[5] & STMPE_FIFO_STA_EMPTY));
      spi::endTransaction(_cfg.spi_host);
      lgfx::gpio_hi(_cfg.pin_cs);
      if (tp)
      {
        tp->x =  data[1] << 4 | data[2] >> 4;
        tp->y = (data[2] & 0x0F) << 8 | data[3];
        tp->size = press ? 1 : 0;
//Serial.printf("x:%03d  y:%03d  pr:%03d \r\n", tp->x, tp->y, press);
      }

      return press ? 1 : 0;
    }

    return 0;
  }

//----------------------------------------------------------------------------
 }
}
