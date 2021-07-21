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
#if defined (__SAMD21__)

#include "common.hpp"

#if defined ( ARDUINO )
 #include <SPI.h>
 #include <Wire.h>
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  namespace samd21
  {
    static constexpr int8_t sercom_pin_list_end = 0;
    static constexpr int8_t sercom0_c_pin_list[] =
    { PORT_A |  8  // PAD 0  MUX C
    , - 1
    , PORT_A |  9  // PAD 1  MUX C
    , - 2
    , PORT_A | 10  // PAD 2  MUX C
    , - 3
    , PORT_A | 11  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom0_d_pin_list[] =
    { PORT_A |  4  // PAD 0  MUX D
    , - 1
    , PORT_A |  5  // PAD 1  MUX D
    , - 2
    , PORT_A |  6  // PAD 2  MUX D
    , - 3
    , PORT_A |  7  // PAD 3  MUX D
    , sercom_pin_list_end
    };

    static constexpr int8_t sercom1_c_pin_list[] =
    { PORT_A | 16  // PAD 0  MUX C
    , - 1
    , PORT_A | 17  // PAD 1  MUX C
    , - 2
    , PORT_A | 18  // PAD 2  MUX C
    , - 3
    , PORT_A | 19  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom1_d_pin_list[] =
    { PORT_A |  0  // PAD 0  MUX D
    , - 1
    , PORT_A |  1  // PAD 1  MUX D
    , - 2
    , PORT_A | 30  // PAD 2  MUX D
    , - 3
    , PORT_A | 31  // PAD 3  MUX D
    , sercom_pin_list_end
    };

    static constexpr int8_t sercom2_c_pin_list[] =
    { PORT_A | 12  // PAD 0  MUX C
    , - 1
    , PORT_A | 13  // PAD 1  MUX C
    , - 2
    , PORT_A | 14  // PAD 2  MUX C
    , - 3
    , PORT_A | 15  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom2_d_pin_list[] =
    { PORT_A |  8  // PAD 0  MUX D
    , - 1
    , PORT_A |  9  // PAD 1  MUX D
    , - 2
    , PORT_A | 10  // PAD 2  MUX D
    , - 3
    , PORT_A | 11  // PAD 3  MUX D
    , sercom_pin_list_end
    };

    static constexpr int8_t sercom3_c_pin_list[] =
    { PORT_A | 22  // PAD 0  MUX C
    , - 1
    , PORT_A | 23  // PAD 1  MUX C
    , - 2
    , PORT_A | 24  // PAD 2  MUX C
    , - 3
    , PORT_A | 25  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom3_d_pin_list[] =
    { PORT_A | 16  // PAD 0  MUX D
    , - 1
    , PORT_A | 17  // PAD 1  MUX D
    , - 2
    , PORT_A | 18  // PAD 2  MUX D
    , PORT_A | 20  // PAD 2  MUX D
    , - 3
    , PORT_A | 19  // PAD 3  MUX D
    , PORT_A | 21  // PAD 3  MUX D
    , sercom_pin_list_end
    };
#if SERCOM_INST_NUM > 4
    static constexpr int8_t sercom4_c_pin_list[] =
    { PORT_B | 12  // PAD 0  MUX C
    , - 1
    , PORT_B | 13  // PAD 1  MUX C
    , - 2
    , PORT_B | 14  // PAD 2  MUX C
    , - 3
    , PORT_B | 15  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom4_d_pin_list[] =
    { PORT_A | 12  // PAD 0  MUX D
    , PORT_B |  8  // PAD 0  MUX D
    , - 1
    , PORT_A | 13  // PAD 1  MUX D
    , PORT_B |  9  // PAD 1  MUX D
    , - 2
    , PORT_A | 14  // PAD 2  MUX D
    , PORT_B | 10  // PAD 2  MUX D
    , - 3
    , PORT_A | 15  // PAD 3  MUX D
    , PORT_B | 11  // PAD 3  MUX D
    , sercom_pin_list_end
    };
#endif
#if SERCOM_INST_NUM > 5
    static constexpr int8_t sercom5_c_pin_list[] =
    { PORT_B | 16  // PAD 0  MUX C
    , - 1
    , PORT_B | 17  // PAD 1  MUX C
    , - 2
    , PORT_A | 20  // PAD 2  MUX C
    , - 3
    , PORT_A | 21  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom5_d_pin_list[] =
    { PORT_A | 22  // PAD 0  MUX D
    , PORT_B |  2  // PAD 0  MUX D
    , PORT_B | 30  // PAD 0  MUX D
    , - 1
    , PORT_A | 23  // PAD 1  MUX D
    , PORT_B |  3  // PAD 1  MUX D
    , PORT_B | 31  // PAD 1  MUX D
    , - 2
    , PORT_A | 24  // PAD 2  MUX D
    , PORT_B |  0  // PAD 2  MUX D
    , PORT_B | 22  // PAD 2  MUX D
    , - 3
    , PORT_A | 25  // PAD 3  MUX D
    , PORT_B |  1  // PAD 3  MUX D
    , PORT_B | 23  // PAD 3  MUX D
    , sercom_pin_list_end
    };
#endif
    static constexpr const int8_t* sercom_mux_pin_list[2][SERCOM_INST_NUM] =
    {
      { sercom0_c_pin_list
      , sercom1_c_pin_list
      , sercom2_c_pin_list
      , sercom3_c_pin_list
#if SERCOM_INST_NUM > 4
      , sercom4_c_pin_list
#endif
#if SERCOM_INST_NUM > 5
      , sercom5_c_pin_list
#endif
      }
    ,
      { sercom0_d_pin_list
      , sercom1_d_pin_list
      , sercom2_d_pin_list
      , sercom3_d_pin_list
#if SERCOM_INST_NUM > 4
      , sercom4_d_pin_list
#endif
#if SERCOM_INST_NUM > 5
      , sercom5_d_pin_list
#endif
      }
    };

    static constexpr sercom_data_t sercom_data[SERCOM_INST_NUM] = {
      { (uintptr_t)SERCOM0, GCM_SERCOM0_CORE, SERCOM0_IRQn },
      { (uintptr_t)SERCOM1, GCM_SERCOM1_CORE, SERCOM1_IRQn },
      { (uintptr_t)SERCOM2, GCM_SERCOM2_CORE, SERCOM2_IRQn },
      { (uintptr_t)SERCOM3, GCM_SERCOM3_CORE, SERCOM3_IRQn },
#if SERCOM_INST_NUM > 4
      { (uintptr_t)SERCOM4, GCM_SERCOM4_CORE, SERCOM4_IRQn },
#endif
#if SERCOM_INST_NUM > 5
      { (uintptr_t)SERCOM5, GCM_SERCOM5_CORE, SERCOM5_IRQn },
#endif
    };
    const sercom_data_t* getSercomData(size_t sercom_index)
    {
      return &sercom_data[sercom_index];
    }

  }

  static int getPadNumber(int sercom_index, int pin, bool alt)
  {
    auto spl = samd21::sercom_mux_pin_list[alt][sercom_index];
    int pad = 0;
    int tmp = spl[0];
    size_t idx = 0;
    do
    {
      if (pin == tmp) return pad;
      if (tmp < 0) { pad = - tmp; }
    } while (samd21::sercom_pin_list_end != (tmp = spl[++idx]));
    return -1;
  }

  void pinAssignPeriph(int pin_and_port, int type)
  {
    if (pin_and_port < 0) return;
    uint_fast8_t port = (pin_and_port >> samd21::PORT_SHIFT);
    uint_fast8_t pin  =  pin_and_port & (samd21::PIN_MASK);
    uint32_t temp = PORT->Group[port].PMUX[pin >> 1].reg;

    if (pin&1) temp = PORT_PMUX_PMUXO( type ) | (temp & PORT_PMUX_PMUXE( 0xF ));
    else       temp = PORT_PMUX_PMUXE( type ) | (temp & PORT_PMUX_PMUXO( 0xF ));
    PORT->Group[port].PMUX[pin >> 1].reg = temp ;
    PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
  }

//----------------------------------------------------------------------------

  void pinMode(int_fast16_t pin, pin_mode_t mode)
  {
    size_t port = pin >> samd21::PORT_SHIFT;
    if (port >= 2) return;

    pin &= samd21::PIN_MASK;
    uint32_t pinMask = (1ul << pin);

    switch (mode)
    {
    case pin_mode_t::input:
      PORT->Group[port].PINCFG[pin].reg = (uint8_t)(PORT_PINCFG_INEN) ;
      PORT->Group[port].DIRCLR.reg = pinMask ;
      break;

    case pin_mode_t::input_pulldown:
      PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
      PORT->Group[port].DIRCLR.reg = pinMask ;
      PORT->Group[port].OUTCLR.reg = pinMask ;
      break;

    case pin_mode_t::input_pullup:
      PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
      PORT->Group[port].DIRCLR.reg = pinMask ;
      PORT->Group[port].OUTSET.reg = pinMask ;
      break;

    case pin_mode_t::output:
      PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN) ;
      PORT->Group[port].DIRSET.reg = pinMask ;
    }
  }

//----------------------------------------------------------------------------

  namespace spi
  {
    cpp::result<void, error_t> init(int sercom_index, int pin_sclk, int pin_miso, int pin_mosi)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }

      int dipo = -1;
      int dopo = -1;

// Serial.printf("sercom:%d sclk:%d  mosi:%d  miso:%d \r\n", sercom_index, pin_sclk, pin_mosi, pin_miso);
      for (int alt = 0; alt < 2; ++alt)
      {
        int pad_sclk = getPadNumber(sercom_index, pin_sclk, alt);
        int pad_mosi = getPadNumber(sercom_index, pin_mosi, alt);
        dipo         = getPadNumber(sercom_index, pin_miso, alt);
        if (pad_sclk == 1)
        {
          if (     pad_mosi == 0) dopo = 0;
          else if (pad_mosi == 3) dopo = 2;
        }
        else if (pad_sclk == 3)
        {
          if (     pad_mosi == 0) dopo = 3;
          else if (pad_mosi == 2) dopo = 1;
        }
        if (dopo >= 0)
        {
// Serial.printf("pad sclk:%d / pad mosi:%d alt:%d\r\n", pad_sclk, pad_mosi, alt);
// Serial.printf("pad dipo:%d \r\n", dipo);
          pinAssignPeriph(pin_sclk, alt ? PIO_SERCOM_ALT : PIO_SERCOM);
          pinAssignPeriph(pin_mosi, alt ? PIO_SERCOM_ALT : PIO_SERCOM);
          if (dipo >= 0)
          {
            pinAssignPeriph(pin_miso, alt ? PIO_SERCOM_ALT : PIO_SERCOM);
          }
          break;
        }
      }
      if (dopo < 0) { return cpp::fail(error_t::invalid_arg); }
      if (dipo < 0) dipo = 0;

      auto sercomData = samd21::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);

 //SPI.begin();
// void SPIClass::begin()
// {
  // void SPIClass::init()
  // {
  //   if (initialized)
  //     return;
  //   interruptMode = SPI_IMODE_NONE;
  //   interruptSave = 0;
  //   interruptMask = 0;
  //   initialized = true;
  // }
//   pinPeripheral(_uc_pinMiso, g_APinDescription[_uc_pinMiso].ulPinType);
//   pinPeripheral(_uc_pinSCK, g_APinDescription[_uc_pinSCK].ulPinType);
//   pinPeripheral(_uc_pinMosi, g_APinDescription[_uc_pinMosi].ulPinType);
  // void SPIClass::config(SPISettings settings)
  // {
  //   _p_sercom->disableSPI();
      /// spi disable
      while (sercom->SPI.SYNCBUSY.bit.ENABLE) {}
      sercom->SPI.CTRLA.bit.ENABLE = 0;
    // void SERCOM::initSPI(SercomSpiTXPad mosi, SercomRXPad miso, SercomSpiCharSize charSize, SercomDataOrder dataOrder)
    // {
    //   resetSPI();
      /// spi reset
      sercom->SPI.CTRLA.bit.SWRST = 1;
      while(sercom->SPI.CTRLA.bit.SWRST || sercom->SPI.SYNCBUSY.bit.SWRST);

    //   initClockNVIC();
      uint8_t   clockId = sercomData->clock;
      IRQn_Type IdNvic  = sercomData->irqn;

      // Setting NVIC
      NVIC_ClearPendingIRQ(IdNvic);
      NVIC_SetPriority(IdNvic, SERCOM_NVIC_PRIORITY);
      NVIC_EnableIRQ(IdNvic);

      // Setting clock
      GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( clockId ) // Generic Clock 0 (SERCOMx)
                        | GCLK_CLKCTRL_GEN_GCLK0     // Generic Clock Generator 0 is source
                        | GCLK_CLKCTRL_CLKEN;

      while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); // Wait for synchronization


      //Setting the CTRLA register
      sercom->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_MASTER
                            | SERCOM_SPI_CTRLA_DOPO(dopo)
                            | SERCOM_SPI_CTRLA_DIPO(dipo)
                            | SercomDataOrder::MSB_FIRST << SERCOM_SPI_CTRLA_DORD_Pos;

      //Setting the CTRLB register
      sercom->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE(SPI_CHAR_SIZE_8_BITS)
                            | SERCOM_SPI_CTRLB_RXEN; //Active the SPI receiver.

      while( sercom->SPI.SYNCBUSY.bit.CTRLB == 1 );
    // }
  //   _p_sercom->initSPIClock(settings.dataMode, settings.clockFreq);
  //   _p_sercom->enableSPI();
  // }
// }
      return {};
    }

    //void release(int spi_host) {}
    void release(int sercom_index)
    {
      auto sercomData = samd21::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);

      while (sercom->SPI.SYNCBUSY.bit.ENABLE) {}
      sercom->SPI.CTRLA.bit.ENABLE = 0;

      sercom->SPI.CTRLA.bit.SWRST = 1;
      while (sercom->SPI.CTRLA.bit.SWRST || sercom->SPI.SYNCBUSY.bit.SWRST);
    }

    static uint32_t FreqToClockDiv(uint32_t hz)
    {
      return (SERCOM_SPI_FREQ_REF >> 1) / (1 + hz);
    }

    void beginTransaction(int sercom_index, uint32_t freq, int spi_mode)
    {
      auto sercomData = samd21::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);

      while (sercom->SPI.SYNCBUSY.bit.ENABLE) {}
      sercom->SPI.CTRLA.bit.ENABLE = 0;

      //Extract data from clockMode
      sercom->SPI.CTRLA.bit.CPHA = spi_mode & 1;
      sercom->SPI.CTRLA.bit.CPOL = spi_mode & 2;
      sercom->SPI.BAUD.reg = FreqToClockDiv(freq);

      sercom->SPI.CTRLA.bit.ENABLE = 1;
      while (sercom->SPI.SYNCBUSY.bit.ENABLE) {}
    }

    void endTransaction(int spi_host)
    {

    }
    void writeBytes(int spi_host, const uint8_t* data, size_t length)
    {

    }
    void readBytes(int spi_host, uint8_t* data, size_t length)
    {

    }
  }

//----------------------------------------------------------------------------

  namespace i2c // TODO: implement.
  {
    cpp::result<void, error_t> init(int sercom_index, int pin_sda, int pin_scl)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }

      int pad_sda;
      int pad_scl;
      int alt = 0;
      do
      {
        pad_sda = getPadNumber(sercom_index, pin_sda, alt);
        pad_scl = getPadNumber(sercom_index, pin_scl, alt);
        if (pad_sda == 0 && pad_scl == 1) break;
      } while (++alt < 2);
      if (alt == 2) { return cpp::fail(error_t::invalid_arg); }

// Serial.printf("sercom:%d scl:%d  sda:%d \r\n", sercom_index, pin_scl, pin_sda);
// Serial.printf("pad scl:%d / pad sda:%d \r\n", pad_scl, pad_sda);

      auto sercomData = samd21::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto *i2cm = &sercom->I2CM;


      Wire.begin();
      return {};
    }

    cpp::result<void, error_t> release(int sercom_index)
    {
      Wire.end();
      return {};
    }

    cpp::result<void, error_t> restart(int sercom_index, int i2c_addr, uint32_t freq, bool read)
    {
      return {};
    }

    cpp::result<void, error_t> beginTransaction(int sercom_index, int i2c_addr, uint32_t freq, bool read)
    {
      if (!read)
      {
        Wire.beginTransmission(i2c_addr);
      }
      Wire.setClock(freq);
      return {};
    }

    cpp::result<void, error_t> endTransaction(int sercom_index)
    {
      Wire.endTransmission();
      return {};
    }

    cpp::result<void, error_t> writeBytes(int sercom_index, const uint8_t *data, size_t length)
    {
      Wire.write(data, length);
      return {};
    }

    cpp::result<void, error_t> readBytes(int sercom_index, uint8_t *data, size_t length)
    {
      return {};
    }

    cpp::result<void, error_t> transactionWrite(int sercom_index, int addr, const uint8_t *writedata, uint8_t writelen, uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(sercom_index, addr, freq, false)).has_value()
       && (res = writeBytes(sercom_index, writedata, writelen)).has_value()
      )
      {
        res = endTransaction(sercom_index);
      }
      return res;
    }

    cpp::result<void, error_t> transactionRead(int sercom_index, int addr, uint8_t *readdata, uint8_t readlen, uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(sercom_index, addr, freq, true)).has_value()
       && (res = readBytes(sercom_index, readdata, readlen)).has_value()
      )
      {
        res = endTransaction(sercom_index);
      }
      return res;
    }

    cpp::result<void, error_t> transactionWriteRead(int sercom_index, int addr, const uint8_t *writedata, uint8_t writelen, uint8_t *readdata, size_t readlen, uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(sercom_index, addr, freq, false)).has_value()
       && (res = writeBytes(sercom_index, writedata, writelen)).has_value()
       && (res = restart(sercom_index, addr, freq, true)).has_value()
       && (res = readBytes(sercom_index, readdata, readlen)).has_value()
      )
      {
        res = endTransaction(sercom_index);
      }
      return res;
    }

    cpp::result<uint8_t, error_t> readRegister8(int sercom_index, int addr, uint8_t reg, uint32_t freq)
    {
      auto res = transactionWriteRead(sercom_index, addr, &reg, 1, &reg, 1, freq);
      if (res.has_value()) { return reg; }
      return cpp::fail( res.error() );
    }

    cpp::result<void, error_t> writeRegister8(int sercom_index, int addr, uint8_t reg, uint8_t data, uint8_t mask, uint32_t freq)
    {
      uint8_t tmp[2] = { reg, data };
      if (mask)
      {
        auto res = transactionWriteRead(sercom_index, addr, &reg, 1, &tmp[1], 1, freq);
        if (res.has_error()) { return res; }
        tmp[1] = (tmp[1] & mask) | data;
      }
      return transactionWrite(sercom_index, addr, tmp, 2, freq);
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
