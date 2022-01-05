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
#if defined (__SAMD51__)

#include "common.hpp"

#undef PORT_PINCFG_PULLEN
#undef PORT_PINCFG_PULLEN_Pos
#undef PORT_PINCFG_INEN
#undef PORT_PINCFG_INEN_Pos
#undef _Ul

#define _Ul(n) (static_cast<uint32_t>((n)))
#define PORT_PINCFG_INEN_Pos        1            /**< \brief (PORT_PINCFG) Input Enable */
#define PORT_PINCFG_INEN            (_Ul(0x1) << PORT_PINCFG_INEN_Pos)
#define PORT_PINCFG_PULLEN_Pos      2            /**< \brief (PORT_PINCFG) Pull Enable */
#define PORT_PINCFG_PULLEN          (_Ul(0x1) << PORT_PINCFG_PULLEN_Pos)

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  namespace samd51
  {
    static constexpr int8_t sercom_pin_list_end = 0;
    static constexpr int8_t sercom0_c_pin_list[] =
    { PORT_A |  8  // PAD 0  MUX C
    , PORT_B | 24  // PAD 0  MUX C
    , - 1
    , PORT_A |  9  // PAD 1  MUX C
    , PORT_B | 25  // PAD 1  MUX C
    , - 2
    , PORT_A | 10  // PAD 2  MUX C
    , PORT_C | 24  // PAD 2  MUX C
    , - 3 
    , PORT_A | 11  // PAD 3  MUX C
    , PORT_C | 25  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom1_c_pin_list[] =
    { PORT_A | 16  // PAD 0  MUX C
    , PORT_C | 22  // PAD 0  MUX C
    , PORT_C | 27  // PAD 0  MUX C
    , - 1
    , PORT_A | 17  // PAD 1  MUX C
    , PORT_C | 23  // PAD 1  MUX C
    , PORT_C | 28  // PAD 1  MUX C
    , - 2
    , PORT_A | 18  // PAD 2  MUX C
    , PORT_B | 22  // PAD 2  MUX C
    , PORT_D | 20  // PAD 2  MUX C
    , - 3
    , PORT_A | 19  // PAD 3  MUX C
    , PORT_B | 23  // PAD 3  MUX C
    , PORT_D | 21  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom2_c_pin_list[] =
    { PORT_A | 12  // PAD 0  MUX C
    , PORT_B | 26  // PAD 0  MUX C
    , - 1
    , PORT_A | 13  // PAD 1  MUX C
    , PORT_B | 27  // PAD 1  MUX C
    , - 2
    , PORT_A | 14  // PAD 2  MUX C
    , PORT_B | 28  // PAD 2  MUX C
    , - 3
    , PORT_A | 15  // PAD 3  MUX C
    , PORT_B | 29  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom3_c_pin_list[] =
    { PORT_A | 22  // PAD 0  MUX C
    , PORT_B | 20  // PAD 0  MUX C
    , - 1
    , PORT_A | 23  // PAD 1  MUX C
    , PORT_B | 21  // PAD 1  MUX C
    , - 2
    , PORT_A | 24  // PAD 2  MUX C
    , - 3
    , PORT_A | 25  // PAD 3  MUX C
    , sercom_pin_list_end
    };
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
    static constexpr int8_t sercom5_c_pin_list[] =
    { PORT_B | 16  // PAD 0  MUX C
    , - 1
    , PORT_B | 17  // PAD 1  MUX C
    , - 2
    , PORT_A | 20  // PAD 2  MUX C
    , PORT_B | 18  // PAD 2  MUX C
    , - 3
    , PORT_A | 21  // PAD 3  MUX C
    , PORT_B | 19  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom6_c_pin_list[] =
    { PORT_C |  4  // PAD 0  MUX C
    , PORT_C | 16  // PAD 0  MUX C
    , - 1
    , PORT_C |  5  // PAD 1  MUX C
    , PORT_C | 17  // PAD 1  MUX C
    , - 2
    , PORT_C |  6  // PAD 2  MUX C
    , PORT_C | 10  // PAD 2  MUX C
    , PORT_C | 18  // PAD 2  MUX C
    , - 3
    , PORT_C |  7  // PAD 3  MUX C
    , PORT_C | 11  // PAD 3  MUX C
    , PORT_C | 19  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom7_c_pin_list[] =
    { PORT_B | 30  // PAD 0  MUX C
    , PORT_C | 12  // PAD 0  MUX C
    , PORT_D |  8  // PAD 0  MUX C
    , - 1
    , PORT_B | 31  // PAD 1  MUX C
    , PORT_C | 13  // PAD 1  MUX C
    , PORT_D |  9  // PAD 1  MUX C
    , - 2
    , PORT_A | 30  // PAD 2  MUX C
    , PORT_C | 14  // PAD 2  MUX C
    , PORT_D | 10  // PAD 2  MUX C
    , - 3
    , PORT_A | 31  // PAD 3  MUX C
    , PORT_C | 15  // PAD 3  MUX C
    , PORT_D | 11  // PAD 3  MUX C
    , sercom_pin_list_end
    };

    static constexpr int8_t sercom0_d_pin_list[] =
    { PORT_A |  4  // PAD 0  MUX D
    , PORT_C | 17  // PAD 0  MUX D
    , - 1
    , PORT_A |  5  // PAD 1  MUX D
    , PORT_C | 16  // PAD 1  MUX D
    , - 2
    , PORT_A |  6  // PAD 2  MUX D
    , PORT_C | 18  // PAD 2  MUX D
    , - 3 
    , PORT_A |  7  // PAD 3  MUX D
    , PORT_C | 19  // PAD 3  MUX D
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
    static constexpr int8_t sercom2_d_pin_list[] =
    { PORT_A |  9  // PAD 0  MUX D
    , PORT_B | 25  // PAD 0  MUX D
    , - 1
    , PORT_A |  8  // PAD 1  MUX D
    , PORT_B | 24  // PAD 1  MUX D
    , - 2
    , PORT_A | 10  // PAD 2  MUX D
    , PORT_C | 24  // PAD 2  MUX D
    , - 3
    , PORT_A | 11  // PAD 3  MUX D
    , PORT_C | 25  // PAD 3  MUX D
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom3_d_pin_list[] =
    { PORT_A | 17  // PAD 0  MUX D
    , PORT_C | 23  // PAD 0  MUX D
    , - 1
    , PORT_A | 16  // PAD 1  MUX D
    , PORT_C | 22  // PAD 1  MUX D
    , - 2
    , PORT_A | 18  // PAD 2  MUX D
    , PORT_A | 20  // PAD 2  MUX D
    , PORT_D | 20  // PAD 2  MUX D
    , - 3
    , PORT_A | 19  // PAD 3  MUX D
    , PORT_A | 21  // PAD 3  MUX D
    , PORT_D | 21  // PAD 3  MUX D
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom4_d_pin_list[] =
    { PORT_A | 13  // PAD 0  MUX D
    , PORT_B |  8  // PAD 0  MUX D
    , PORT_B | 27  // PAD 0  MUX D
    , - 1
    , PORT_A | 12  // PAD 1  MUX D
    , PORT_B |  9  // PAD 1  MUX D
    , PORT_B | 26  // PAD 1  MUX D
    , - 2
    , PORT_A | 14  // PAD 2  MUX D
    , PORT_B | 10  // PAD 2  MUX D
    , PORT_B | 28  // PAD 2  MUX D
    , - 3
    , PORT_A | 15  // PAD 3  MUX D
    , PORT_B | 11  // PAD 3  MUX D
    , PORT_B | 29  // PAD 3  MUX D
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom5_d_pin_list[] =
    { PORT_A | 23  // PAD 0  MUX D
    , PORT_B |  2  // PAD 0  MUX D
    , PORT_B | 31  // PAD 0  MUX D
    , - 1
    , PORT_A | 22  // PAD 1  MUX D
    , PORT_B |  3  // PAD 1  MUX D
    , PORT_B | 30  // PAD 1  MUX D
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
    static constexpr int8_t sercom6_d_pin_list[] =
    { PORT_C | 13  // PAD 0  MUX D
    , PORT_D |  9  // PAD 0  MUX D
    , - 1
    , PORT_C | 12  // PAD 1  MUX D
    , PORT_D |  8  // PAD 1  MUX D
    , - 2
    , PORT_C | 14  // PAD 2  MUX D
    , PORT_D | 10  // PAD 2  MUX D
    , - 3
    , PORT_C | 15  // PAD 3  MUX D
    , PORT_D | 11  // PAD 3  MUX D
    , sercom_pin_list_end
    };
    static constexpr int8_t sercom7_d_pin_list[] =
    { PORT_B | 21  // PAD 0  MUX D
    , - 1
    , PORT_B | 20  // PAD 1  MUX D
    , - 2
    , PORT_B | 18  // PAD 2  MUX D
    , PORT_C | 10  // PAD 2  MUX D
    , - 3
    , PORT_B | 19  // PAD 3  MUX D
    , PORT_C | 11  // PAD 3  MUX D
    , sercom_pin_list_end
    };

    static constexpr const int8_t* sercom_mux_pin_list[2][SERCOM_INST_NUM] =
    {
      { sercom0_c_pin_list
      , sercom1_c_pin_list
      , sercom2_c_pin_list
      , sercom3_c_pin_list
      , sercom4_c_pin_list
      , sercom5_c_pin_list
    #if SERCOM_INST_NUM > 6
      , sercom6_c_pin_list
    #endif
    #if SERCOM_INST_NUM > 7
      , sercom7_c_pin_list
    #endif
      }
    ,
      { sercom0_d_pin_list
      , sercom1_d_pin_list
      , sercom2_d_pin_list
      , sercom3_d_pin_list
      , sercom4_d_pin_list
      , sercom5_d_pin_list
    #if SERCOM_INST_NUM > 6
      , sercom6_d_pin_list
    #endif
    #if SERCOM_INST_NUM > 7
      , sercom7_d_pin_list
    #endif
      }
    };

    static const sercom_data_t sercom_data[] = {
      { (uintptr_t)SERCOM0, SERCOM0_GCLK_ID_CORE, SERCOM0_GCLK_ID_SLOW, SERCOM0_DMAC_ID_TX, SERCOM0_DMAC_ID_RX },
      { (uintptr_t)SERCOM1, SERCOM1_GCLK_ID_CORE, SERCOM1_GCLK_ID_SLOW, SERCOM1_DMAC_ID_TX, SERCOM1_DMAC_ID_RX },
      { (uintptr_t)SERCOM2, SERCOM2_GCLK_ID_CORE, SERCOM2_GCLK_ID_SLOW, SERCOM2_DMAC_ID_TX, SERCOM2_DMAC_ID_RX },
      { (uintptr_t)SERCOM3, SERCOM3_GCLK_ID_CORE, SERCOM3_GCLK_ID_SLOW, SERCOM3_DMAC_ID_TX, SERCOM3_DMAC_ID_RX },
      { (uintptr_t)SERCOM4, SERCOM4_GCLK_ID_CORE, SERCOM4_GCLK_ID_SLOW, SERCOM4_DMAC_ID_TX, SERCOM4_DMAC_ID_RX },
      { (uintptr_t)SERCOM5, SERCOM5_GCLK_ID_CORE, SERCOM5_GCLK_ID_SLOW, SERCOM5_DMAC_ID_TX, SERCOM5_DMAC_ID_RX },
    #if SERCOM_INST_NUM > 6
      { (uintptr_t)SERCOM6, SERCOM6_GCLK_ID_CORE, SERCOM6_GCLK_ID_SLOW, SERCOM6_DMAC_ID_TX, SERCOM6_DMAC_ID_RX },
    #endif
    #if SERCOM_INST_NUM > 7
      { (uintptr_t)SERCOM7, SERCOM7_GCLK_ID_CORE, SERCOM7_GCLK_ID_SLOW, SERCOM7_DMAC_ID_TX, SERCOM7_DMAC_ID_RX },
    #endif
    };
    const sercom_data_t* getSercomData(size_t sercom_index)
    {
      return &sercom_data[sercom_index];
    }
  }

  static int getPadNumber(int sercom_index, int pin, bool alt)
  {
    auto spl = samd51::sercom_mux_pin_list[alt][sercom_index];
    int pad = 0;
    int tmp = spl[0];
    size_t idx = 0;
    do
    {
      if (pin == tmp) return pad;
      if (tmp < 0) { pad = - tmp; }
    } while (samd51::sercom_pin_list_end != (tmp = spl[++idx]));
    return -1;
  }

  void pinAssignPeriph(int pin_and_port, int type)
  {
    if (pin_and_port < 0) return;
    uint_fast8_t port = (pin_and_port >> samd51::PORT_SHIFT);
    uint_fast8_t pin  =  pin_and_port & (samd51::PIN_MASK);
    uint32_t temp = PORT->Group[port].PMUX[pin >> 1].reg;

    if (pin&1) temp = PORT_PMUX_PMUXO( type ) | (temp & PORT_PMUX_PMUXE( 0xF ));
    else       temp = PORT_PMUX_PMUXE( type ) | (temp & PORT_PMUX_PMUXO( 0xF ));
    PORT->Group[port].PMUX[pin >> 1].reg = temp ;
    PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
  }

//----------------------------------------------------------------------------

  void pinMode(int_fast16_t pin, pin_mode_t mode)
  {
    uint32_t port = pin >> samd51::PORT_SHIFT;
    pin &= samd51::PIN_MASK;
    uint32_t pinMask = (1ul << pin);

    // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
    switch ( mode )
    {
      case pin_mode_t::input:
        // Set pin to input mode
        PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;
      break ;

      case pin_mode_t::input_pullup:
        // Set pin to input mode with pull-up resistor enabled
        PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;

        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.7 Data Output Value Set')
        PORT->Group[port].OUTSET.reg = pinMask ;
      break ;

      case pin_mode_t::input_pulldown:
        // Set pin to input mode with pull-down resistor enabled
        PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;

        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.6 Data Output Value Clear')
        PORT->Group[port].OUTCLR.reg = pinMask ;
      break ;

      case pin_mode_t::output:
        // enable input, to support reading back values, with pullups disabled
        PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN) ;

        // Set pin to output mode
        PORT->Group[port].DIRSET.reg = pinMask ;
      break ;

      default:
        // do nothing
      break ;
    }
  }

//----------------------------------------------------------------------------

  namespace spi // TODO: implement.
  {
    static void resetSPI(SercomSpi* spi)
    {
      //Setting the Software Reset bit to 1
      spi->CTRLA.bit.SWRST = 1;

      //Wait both bits Software Reset from CTRLA and SYNCBUSY are equal to 0
      while (spi->CTRLA.bit.SWRST || spi->SYNCBUSY.bit.SWRST);
    }

    cpp::result<void, error_t> init(int sercom_index, int pin_sclk, int pin_miso, int pin_mosi)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }

      int dopo = -1;
      int dipo = -1;

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
        if (dopo >= 0)
        {
          pinAssignPeriph(pin_sclk, alt ? PIO_SERCOM_ALT : PIO_SERCOM);
          pinAssignPeriph(pin_mosi, alt ? PIO_SERCOM_ALT : PIO_SERCOM);
          if (dipo >= 0)
          {
            pinAssignPeriph(pin_miso, alt ? PIO_SERCOM_ALT : PIO_SERCOM);
          }
          break;
        }
// Serial.printf("pad sclk:%d / pad mosi:%d alt:%d\r\n", pad_sclk, pad_mosi, alt);
// Serial.printf("pad dipo:%d \r\n", dipo);
      }
      if (dopo < 0) { return cpp::fail(error_t::invalid_arg); }
      if (dipo < 0) dipo = 0;

      auto sercom_data = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercom_data->sercomPtr);

      auto *spi = &sercom->SPI;
      while (spi->SYNCBUSY.bit.ENABLE); //Waiting then enable bit from SYNCBUSY is equal to 0;
      spi->CTRLA.bit.ENABLE = 0;        //Setting the enable bit to 0

      resetSPI(spi);

#if defined(__SAMD51__)
auto mastermode = SERCOM_SPI_CTRLA_MODE(0x3);
#else
auto mastermode = SERCOM_SPI_CTRLA_MODE_SPI_MASTER;
#endif

      //Setting the CTRLA register
      spi->CTRLA.reg = mastermode
                     | SERCOM_SPI_CTRLA_DOPO(dopo)
                     | SERCOM_SPI_CTRLA_DIPO(dipo)
                     | MSB_FIRST << SERCOM_SPI_CTRLA_DORD_Pos;

      //Setting the CTRLB register
      spi->CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE(SPI_CHAR_SIZE_8_BITS)
                      | SERCOM_SPI_CTRLB_RXEN; //Active the SPI receiver.

      while ( spi->SYNCBUSY.bit.CTRLB == 1 );

      return {};
    }

    void release(int sercom_index)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return; }

      auto sercom_data = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercom_data->sercomPtr);

      auto *spi = &sercom->SPI;
      while (spi->SYNCBUSY.bit.ENABLE); //Waiting then enable bit from SYNCBUSY is equal to 0;
      spi->CTRLA.bit.ENABLE = 0;        //Setting the enable bit to 0

      resetSPI(spi);
    }

    static uint32_t FreqToClockDiv(uint32_t hz)
    {
#if defined ( F_CPU )
      return (F_CPU >> 1) / (1 + hz);
#else
      return (SERCOM_SPI_FREQ_REF >> 1) / (1 + hz);
#endif
    }

    void beginTransaction(int sercom_index, uint32_t freq, int spi_mode)
    {
      auto sercomData = samd51::getSercomData(sercom_index);
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

    void endTransaction(int sercom_index)
    {
    }

    void writeBytes(int sercom_index, const uint8_t* data, size_t length)
    {
      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto *spi = &sercom->SPI;
      do
      {
        spi->DATA.reg = *data++;
        while (spi->INTFLAG.bit.DRE == 0);
      } while (--length);
      while (!spi->INTFLAG.bit.TXC);
    }

    void readBytes(int sercom_index, uint8_t* data, size_t length)
    {
      auto sercom_data = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercom_data->sercomPtr);
      auto *spi = &sercom->SPI;
      
      while (spi->INTFLAG.bit.RXC) { uint32_t tmp = spi->DATA.reg; (void)tmp; }
      do
      {
        spi->DATA.reg = *data;
        while (!spi->INTFLAG.bit.RXC);
        *data++ = (spi->DATA.reg & 0xFF);
      } while (--length);
    }
  }

//----------------------------------------------------------------------------

  namespace i2c // TODO: implement.
  {
    struct i2c_context_t
    {
      enum state_t
      {
        state_disconnect,
        state_write,
        state_read
      };
      cpp::result<state_t, error_t> state;

      bool wait_ack = false;
      uint_fast16_t pin_scl = -1;
      uint_fast16_t pin_sda = -1;
      uint32_t freq = 0;

      void save_reg(SercomI2cm* dev)
      {
        baudrate = dev->BAUD.reg;
      }

      void load_reg(SercomI2cm* dev)
      {
        dev->BAUD.reg = baudrate;
      }

    private:
      uint32_t baudrate;
    };
    i2c_context_t i2c_context[SERCOM_INST_NUM];


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

      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto *i2cm = &sercom->I2CM;


      uint_fast8_t id_core = sercomData->id_core;
      uint_fast8_t id_slow = sercomData->id_slow;
      GCLK->PCHCTRL[id_core].bit.CHEN = 0;     // Disable timer
      GCLK->PCHCTRL[id_slow].bit.CHEN = 0;     // Disable timer
      uint32_t gclk_reg_value = GCLK_PCHCTRL_CHEN | SERCOM_CLOCK_SOURCE_48M << GCLK_PCHCTRL_GEN_Pos;
      while (GCLK->PCHCTRL[id_core].bit.CHEN || GCLK->PCHCTRL[id_slow].bit.CHEN);  // Wait for disable
      GCLK->PCHCTRL[id_core].reg = gclk_reg_value;
      GCLK->PCHCTRL[id_slow].reg = gclk_reg_value;


      // resetWIRE
      i2cm->CTRLA.bit.SWRST = 1;
      while (i2cm->CTRLA.bit.SWRST || i2cm->SYNCBUSY.bit.SWRST);

      // set master mode
      i2cm->CTRLA.reg =  SERCOM_I2CM_CTRLA_MODE( I2C_MASTER_OPERATION );

// I2C Master and Slave modes share the ENABLE bit function.

      // Enable the I2C master mode
      i2cm->CTRLA.bit.ENABLE = 1 ;
      while ( i2cm->SYNCBUSY.bit.ENABLE != 0 ) {}

      // Setting bus idle mode
      i2cm->STATUS.bit.BUSSTATE = 1 ;
      while ( i2cm->SYNCBUSY.bit.SYSOP != 0 ) {}
//*/
      pinAssignPeriph(pin_scl, alt ? PIO_SERCOM_ALT : PIO_SERCOM);
      pinAssignPeriph(pin_sda, alt ? PIO_SERCOM_ALT : PIO_SERCOM);

//Serial.println("lgfx i2c init success");
      return {};
    }

    cpp::result<void, error_t> release(int sercom_index)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }

      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);

      sercom->I2CM.CTRLA.bit.ENABLE = 0 ;
      while ( sercom->I2CM.SYNCBUSY.bit.ENABLE != 0 ) {}
/*
      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto i2cm = &(sercom->I2CM);

      i2cm->CTRLA.bit.ENABLE = 0;
      while ( i2cm->SYNCBUSY.bit.ENABLE != 0 );
//*/
      return {};
    }

    cpp::result<void, error_t> restart(int sercom_index, int i2c_addr, uint32_t freq, bool read)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }

      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto i2cm = &(sercom->I2CM);
/*
Serial.println("restart");
  i2cm->CTRLA.bit.ENABLE = 0 ;
  while ( i2cm->SYNCBUSY.bit.ENABLE ) {}
Serial.println("restart 1");
sercom3.initMasterWIRE(freq);
Serial.println("restart 2");

  i2cm->CTRLA.bit.ENABLE = 1 ;

  while ( i2cm->SYNCBUSY.bit.ENABLE ) {}

  // Setting bus idle mode
  i2cm->STATUS.bit.BUSSTATE = 1 ;

  while ( i2cm->SYNCBUSY.bit.SYSOP ) {}
Serial.println("restart 3");

if (sercom3.startTransmissionWIRE(i2c_addr, read ? WIRE_READ_FLAG : WIRE_WRITE_FLAG))
{
Serial.println("restart:ok");
  return {};
}
Serial.println("restart:fail");
return cpp::fail(error_t::connection_lost);
/*/

      if (i2c_context[sercom_index].freq != freq
       || i2c_context[sercom_index].state == i2c_context_t::state_t::state_disconnect)
      {
        i2c_context[sercom_index].freq = freq;

        // disable
        i2cm->CTRLA.bit.ENABLE = 0;
        while ( i2cm->SYNCBUSY.bit.ENABLE ) {}

        // reset
        i2cm->CTRLA.bit.SWRST = 1;
        while (i2cm->CTRLA.bit.SWRST || i2cm->SYNCBUSY.bit.SWRST) {}

  // Synchronous arithmetic baudrate
#if defined(__SAMD51__)
        i2cm->BAUD.bit.BAUD = SERCOM_FREQ_REF / ( 2 * freq ) - 1 ;
#else
        i2cm->BAUD.bit.BAUD = SystemCoreClock / ( 2 * baudrate) - 5 - (((SystemCoreClock / 1000000) * WIRE_RISE_TIME_NANOSECONDS) / (2 * 1000));
#endif
        // enable
      // set master mode
        i2cm->CTRLA.reg = SERCOM_I2CM_CTRLA_MODE( I2C_MASTER_OPERATION ) | SERCOM_I2CM_CTRLA_ENABLE;

        //i2cm->CTRLA.bit.ENABLE = 1;
        while ( i2cm->SYNCBUSY.bit.ENABLE ) {}

        // Setting bus idle mode
        i2cm->STATUS.bit.BUSSTATE = 1 ;

        while ( i2cm->SYNCBUSY.bit.SYSOP ) {}
      }

      while (i2cm->STATUS.bit.BUSSTATE != WIRE_IDLE_STATE
          && i2cm->STATUS.bit.BUSSTATE != WIRE_OWNER_STATE) {}

      // Send start and 7-bits address + 1-bits R/W
      i2cm->ADDR.bit.ADDR = (i2c_addr << 0x1ul) | read;

      i2c_context[sercom_index].wait_ack = !read;
      i2c_context[sercom_index].state = read ? i2c_context_t::state_t::state_read : i2c_context_t::state_t::state_write;

/*
      // Address Transmitted
      if ( !read ) // Write mode
      {
        while( !i2cm->INTFLAG.bit.MB ) {}
      }
      else  // Read mode
      {
        while( !i2cm->INTFLAG.bit.SB )
        {
          if (i2cm->INTFLAG.bit.MB) {
            i2cm->CTRLB.bit.CMD = WIRE_MASTER_ACT_STOP; // Stop condition
Serial.println("restart:fail");
            return cpp::fail(error_t::connection_lost);
          }
        }
        // Clean the 'Slave on Bus' flag, for further usage.
        //sercom->I2CM.INTFLAG.bit.SB = 0x1ul;
      }

      //ACK received (0: ACK, 1: NACK)
      if(sercom->I2CM.STATUS.bit.RXNACK)
      {
Serial.println("restart:fail");
        return cpp::fail(error_t::connection_lost);
      }
Serial.println("restart:ok");
//*/
      return {};
//*/
    }

    cpp::result<void, error_t> beginTransaction(int sercom_index, int i2c_addr, uint32_t freq, bool read)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }
/*
      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto i2cm = &(sercom->I2CM);
      // Wait idle or owner bus mode
      while ( i2cm->STATUS.bit.BUSSTATE != WIRE_IDLE_STATE
           && i2cm->STATUS.bit.BUSSTATE != WIRE_OWNER_STATE);
 Serial.println("beginTrans DEBUG 1");
//*/
      i2c_context[sercom_index].state = i2c_context_t::state_t::state_disconnect;

      return restart(sercom_index, i2c_addr, freq, read);
    }

    cpp::result<void, error_t> i2c_wait(int sercom_index, bool flg_stop = false)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }

      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto i2cm = &(sercom->I2CM);

      cpp::result<void, error_t> res = {};

      if (i2c_context[sercom_index].wait_ack)
      {
        i2c_context[sercom_index].wait_ack = false;
        //Wait transmission successful
        while (!i2cm->INTFLAG.bit.MB)
        {
          // If a bus error occurs, the MB bit may never be set.
          // Check the bus error bit and bail if it's set.
          if (i2cm->STATUS.bit.BUSERR)
          {
            res = cpp::fail(error_t::connection_lost);
            break;
          }
        }
        //Problems on line? nack received?
        if (i2cm->STATUS.bit.RXNACK)
        {
          res = cpp::fail(error_t::connection_lost);
        }
      }
      if (res.has_value() && flg_stop)
      {
        i2cm->CTRLB.bit.CMD = WIRE_MASTER_ACT_STOP; // Stop condition
        //res = cpp::fail(error_t::connection_lost);
        while (sercom->I2CM.SYNCBUSY.bit.SYSOP) {};
      }

      return res;
    }

    cpp::result<void, error_t> endTransaction(int sercom_index)
    {
      return i2c_wait(sercom_index, true);

/*
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }
      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto i2cm = &(sercom->I2CM);
      cpp::result<void, error_t> res = {};

      i2cm->CTRLB.bit.CMD = WIRE_MASTER_ACT_STOP; // Stop condition
      //res = cpp::fail(error_t::connection_lost);
      while (sercom->I2CM.SYNCBUSY.bit.SYSOP) {};
      return res;
//*/
    }

    cpp::result<void, error_t> writeBytes(int sercom_index, const uint8_t *data, size_t length)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }

      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto i2cm = &(sercom->I2CM);
      cpp::result<void, error_t> res = {};
      do
      {
        auto d = *data++;
        res = i2c_wait(sercom_index);
        if (res.has_error()) break;
        //Send data
        i2cm->DATA.bit.DATA = d;
        i2c_context[sercom_index].wait_ack = true;
      } while (--length);
      return res;
    }

    cpp::result<void, error_t> readBytes(int sercom_index, uint8_t *data, size_t length)
    {
      if ((size_t)sercom_index >= SERCOM_INST_NUM) { return cpp::fail(error_t::invalid_arg); }

      auto res = i2c_wait(sercom_index);
      if (res.has_error()) { return res; }

      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);
      auto i2cm = &(sercom->I2CM);
      do
      {
        while( i2cm->INTFLAG.bit.SB == 0 ) {};
        *data++ = i2cm->DATA.bit.DATA;

        i2cm->CTRLB.bit.ACKACT = 0;                     // Prepare Acknowledge

        i2cm->CTRLB.bit.CMD = WIRE_MASTER_ACT_READ;     // Prepare the ACK command for the slave
        while (i2cm->SYNCBUSY.bit.SYSOP) {}
      } while (--length);
      return {};
/*
Serial.println("i2c::readBytes");
      cpp::result<void, error_t> res = {};
      do
      {
        while ( !sercom->I2CM.INTFLAG.bit.SB )
        {
          if (sercom->I2CM.INTFLAG.bit.MB)
          {
Serial.println("read fail");
            sercom->I2CM.CTRLB.bit.CMD = WIRE_MASTER_ACT_STOP; // Stop condition
            res = cpp::fail(error_t::connection_lost);
            break;
          }
        }
        if (res.has_error()) break;

        // Receive data
        *data++ = i2cm->DATA.bit.DATA;

        // Send ACK
        i2cm->CTRLB.bit.ACKACT = 0;

        // Next Read
        i2cm->CTRLB.bit.CMD = WIRE_MASTER_ACT_READ; // read command

        while (i2cm->SYNCBUSY.bit.SYSOP)
        {
          // Waiting for synchronization
        }

      } while (--length);

      return res;
//*/
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
