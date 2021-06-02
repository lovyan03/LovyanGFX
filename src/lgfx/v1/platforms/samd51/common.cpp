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

#define _Ul(n) (static_cast<std::uint32_t>((n)))
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
    static constexpr std::int8_t sercom_pin_list_end = 0;
    static constexpr std::int8_t sercom0_c_pin_list[] =
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
    static constexpr std::int8_t sercom1_c_pin_list[] =
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
    static constexpr std::int8_t sercom2_c_pin_list[] =
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
    static constexpr std::int8_t sercom3_c_pin_list[] =
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
    static constexpr std::int8_t sercom4_c_pin_list[] =
    { PORT_B | 12  // PAD 0  MUX C
    , - 1
    , PORT_B | 13  // PAD 1  MUX C
    , - 2
    , PORT_B | 14  // PAD 2  MUX C
    , - 3
    , PORT_B | 15  // PAD 3  MUX C
    , sercom_pin_list_end
    };
    static constexpr std::int8_t sercom5_c_pin_list[] =
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
    static constexpr std::int8_t sercom6_c_pin_list[] =
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
    static constexpr std::int8_t sercom7_c_pin_list[] =
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

    static constexpr std::int8_t sercom0_d_pin_list[] =
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
    static constexpr std::int8_t sercom1_d_pin_list[] =
    { PORT_A |  0  // PAD 0  MUX D
    , - 1
    , PORT_A |  1  // PAD 1  MUX D
    , - 2
    , PORT_A | 30  // PAD 2  MUX D
    , - 3
    , PORT_A | 31  // PAD 3  MUX D
    , sercom_pin_list_end
    };
    static constexpr std::int8_t sercom2_d_pin_list[] =
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
    static constexpr std::int8_t sercom3_d_pin_list[] =
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
    static constexpr std::int8_t sercom4_d_pin_list[] =
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
    static constexpr std::int8_t sercom5_d_pin_list[] =
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
    static constexpr std::int8_t sercom6_d_pin_list[] =
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
    static constexpr std::int8_t sercom7_d_pin_list[] =
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

    static constexpr const std::size_t sercom_max = SERCOM_INST_NUM;
    static constexpr const std::int8_t* sercom_mux_pin_list[2][sercom_max] =
    {
      { sercom0_c_pin_list
      , sercom1_c_pin_list
      , sercom2_c_pin_list
      , sercom3_c_pin_list
      , sercom4_c_pin_list
      , sercom5_c_pin_list
    #if defined(SERCOM6)
      , sercom6_c_pin_list
    #endif
    #if defined(SERCOM7)
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
    #if defined(SERCOM6)
      , sercom6_d_pin_list
    #endif
    #if defined(SERCOM7)
      , sercom7_d_pin_list
    #endif
      }
    };

    static constexpr sercom_data_t sercom_data[] = {
      { 0x40003000UL, SERCOM0_GCLK_ID_CORE, SERCOM0_GCLK_ID_SLOW, SERCOM0_0_IRQn, SERCOM0_1_IRQn, SERCOM0_2_IRQn, SERCOM0_3_IRQn, SERCOM0_DMAC_ID_TX, SERCOM0_DMAC_ID_RX },
      { 0x40003400UL, SERCOM1_GCLK_ID_CORE, SERCOM1_GCLK_ID_SLOW, SERCOM1_0_IRQn, SERCOM1_1_IRQn, SERCOM1_2_IRQn, SERCOM1_3_IRQn, SERCOM1_DMAC_ID_TX, SERCOM1_DMAC_ID_RX },
      { 0x41012000UL, SERCOM2_GCLK_ID_CORE, SERCOM2_GCLK_ID_SLOW, SERCOM2_0_IRQn, SERCOM2_1_IRQn, SERCOM2_2_IRQn, SERCOM2_3_IRQn, SERCOM2_DMAC_ID_TX, SERCOM2_DMAC_ID_RX },
      { 0x41014000UL, SERCOM3_GCLK_ID_CORE, SERCOM3_GCLK_ID_SLOW, SERCOM3_0_IRQn, SERCOM3_1_IRQn, SERCOM3_2_IRQn, SERCOM3_3_IRQn, SERCOM3_DMAC_ID_TX, SERCOM3_DMAC_ID_RX },
      { 0x43000000UL, SERCOM4_GCLK_ID_CORE, SERCOM4_GCLK_ID_SLOW, SERCOM4_0_IRQn, SERCOM4_1_IRQn, SERCOM4_2_IRQn, SERCOM4_3_IRQn, SERCOM4_DMAC_ID_TX, SERCOM4_DMAC_ID_RX },
      { 0x43000400UL, SERCOM5_GCLK_ID_CORE, SERCOM5_GCLK_ID_SLOW, SERCOM5_0_IRQn, SERCOM5_1_IRQn, SERCOM5_2_IRQn, SERCOM5_3_IRQn, SERCOM5_DMAC_ID_TX, SERCOM5_DMAC_ID_RX },
    #if defined(SERCOM6)
      { 0x43000800UL, SERCOM6_GCLK_ID_CORE, SERCOM6_GCLK_ID_SLOW, SERCOM6_0_IRQn, SERCOM6_1_IRQn, SERCOM6_2_IRQn, SERCOM6_3_IRQn, SERCOM6_DMAC_ID_TX, SERCOM6_DMAC_ID_RX },
    #endif
    #if defined(SERCOM7)
      { 0x43000C00UL, SERCOM7_GCLK_ID_CORE, SERCOM7_GCLK_ID_SLOW, SERCOM7_0_IRQn, SERCOM7_1_IRQn, SERCOM7_2_IRQn, SERCOM7_3_IRQn, SERCOM7_DMAC_ID_TX, SERCOM7_DMAC_ID_RX },
    #endif
    };
    const sercom_data_t* getSercomData(std::size_t sercom_index)
    {
      return &sercom_data[sercom_index];
    }
  }

  static int getPadNumber(int sercom_index, int pin, bool alt)
  {
    auto spl = samd51::sercom_mux_pin_list[alt][sercom_index];
    int pad = 0;
    int tmp = spl[0];
    std::size_t idx = 0;
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
    std::uint_fast8_t port = (pin_and_port >> samd51::PORT_SHIFT);
    std::uint_fast8_t pin  =  pin_and_port & (samd51::PIN_MASK);
    std::uint32_t temp = PORT->Group[port].PMUX[pin >> 1].reg;

    if (pin&1) temp = PORT_PMUX_PMUXO( type ) | (temp & PORT_PMUX_PMUXE( 0xF ));
    else       temp = PORT_PMUX_PMUXE( type ) | (temp & PORT_PMUX_PMUXO( 0xF ));
    PORT->Group[port].PMUX[pin >> 1].reg = temp ;
    PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
  }

//----------------------------------------------------------------------------

  void pinMode(std::int_fast16_t pin, pin_mode_t mode)
  {
    std::uint32_t port = pin >> samd51::PORT_SHIFT;
    pin &= samd51::PIN_MASK;
    std::uint32_t pinMask = (1ul << pin);

    // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
    switch ( mode )
    {
      case pin_mode_t::input:
        // Set pin to input mode
        PORT->Group[port].PINCFG[pin].reg=(std::uint8_t)(PORT_PINCFG_INEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;
      break ;

      case pin_mode_t::input_pullup:
        // Set pin to input mode with pull-up resistor enabled
        PORT->Group[port].PINCFG[pin].reg=(std::uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;

        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.7 Data Output Value Set')
        PORT->Group[port].OUTSET.reg = pinMask ;
      break ;

      case pin_mode_t::input_pulldown:
        // Set pin to input mode with pull-down resistor enabled
        PORT->Group[port].PINCFG[pin].reg=(std::uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;

        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.6 Data Output Value Clear')
        PORT->Group[port].OUTCLR.reg = pinMask ;
      break ;

      case pin_mode_t::output:
        // enable input, to support reading back values, with pullups disabled
        PORT->Group[port].PINCFG[pin].reg=(std::uint8_t)(PORT_PINCFG_INEN) ;

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
      if ((std::size_t)sercom_index >= samd51::sercom_max) { return cpp::fail(error_t::invalid_arg); }

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

//      std::int8_t idx = CFG::sercom_index;
//      for(std::uint8_t i=0; i<4; i++) {
//        NVIC_ClearPendingIRQ(sercom_data->irq[i]);
//        NVIC_SetPriority(sercom_data->irq[i], SERCOM_NVIC_PRIORITY);
//        NVIC_EnableIRQ(sercom_data->irq[i]);
//      }

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

      while( spi->SYNCBUSY.bit.CTRLB == 1 );

      return {};
    }

    //void release(int spi_host) {}
    void release(int) {}

    //void beginTransaction(int spi_host, int spi_cs, int freq, int spi_mode)
    void beginTransaction(int, int, int, int) {}

    //void beginTransaction(int spi_host)
    void beginTransaction(int) {}

    //void endTransaction(int spi_host, int spi_cs)
    void endTransaction(int, int) {}

    //void writeData(int spi_host, const std::uint8_t* data, std::uint32_t len)
    void writeBytes(int, const std::uint8_t*, std::uint32_t) {}

    //void readData(int spi_host, std::uint8_t* data, std::uint32_t len)
    void readBytes(int, std::uint8_t*, std::uint32_t) {}
  }

//----------------------------------------------------------------------------

  namespace i2c // TODO: implement.
  {
    cpp::result<void, error_t> init(int sercom_index, int pin_sda, int pin_scl)
    {
      if ((std::size_t)sercom_index >= samd51::sercom_max) { return cpp::fail(error_t::invalid_arg); }

      for (int alt = 0; alt < 2; ++alt)
      {
        int pad_sda = getPadNumber(sercom_index, pin_sda, alt);
        int pad_scl = getPadNumber(sercom_index, pin_scl, alt);
// Serial.printf("sercom:%d scl:%d  sda:%d \r\n", sercom_index, pin_scl, pin_sda);
// Serial.printf("pad scl:%d / pad sda:%d \r\n", pad_scl, pad_sda);
        if (pad_sda != 0 || pad_scl != 1) continue;

        auto sercom = reinterpret_cast<Sercom*>(samd51::getSercomData(sercom_index)->sercomPtr);

        auto *i2c = &sercom->I2CM;
        while (i2c->SYNCBUSY.bit.ENABLE); //Waiting then enable bit from SYNCBUSY is equal to 0;
        i2c->CTRLA.bit.ENABLE = 0;        //Setting the enable bit to 0

        pinAssignPeriph(pin_scl, alt ? PIO_SERCOM_ALT : PIO_SERCOM);
        pinAssignPeriph(pin_sda, alt ? PIO_SERCOM_ALT : PIO_SERCOM);

        return {};
      }
      return cpp::fail(error_t::invalid_arg);
    }

    cpp::result<void, error_t> release(int sercom_index)
    {
      if ((std::size_t)sercom_index >= samd51::sercom_max) { return cpp::fail(error_t::invalid_arg); }

      auto sercom = reinterpret_cast<Sercom*>(samd51::getSercomData(sercom_index)->sercomPtr);
      sercom->I2CM.CTRLA.bit.ENABLE = 0;
      while ( sercom->I2CM.SYNCBUSY.bit.ENABLE != 0 );

      return {};
    }

    cpp::result<void, error_t> restart(int sercom_index, int i2c_addr, std::uint32_t freq, bool read)
    {
      if ((std::size_t)sercom_index >= samd51::sercom_max) { return cpp::fail(error_t::invalid_arg); }

      return {};
    }

    cpp::result<void, error_t> beginTransaction(int sercom_index, int i2c_addr, std::uint32_t freq, bool read)
    {
      if ((std::size_t)sercom_index >= samd51::sercom_max) { return cpp::fail(error_t::invalid_arg); }

      auto sercomData = samd51::getSercomData(sercom_index);
      auto sercom = reinterpret_cast<Sercom*>(sercomData->sercomPtr);


  // Initialize the peripheral clock and interruption
#if defined(__SAMD51__)

      for (std::size_t i = 0; i < 4; ++i) {
        NVIC_ClearPendingIRQ(sercomData->irq[i]);
        NVIC_SetPriority(sercomData->irq[i], SERCOM_NVIC_PRIORITY);
        NVIC_EnableIRQ(sercomData->irq[i]);
      }

#endif // end !SAMD51



      sercom->I2CM.CTRLA.bit.SWRST = 1;
      while (sercom->I2CM.CTRLA.bit.SWRST || sercom->I2CM.SYNCBUSY.bit.SWRST);


  // Set master mode and enable SCL Clock Stretch mode (stretch after ACK bit)
      sercom->I2CM.CTRLA.reg =  SERCOM_I2CM_CTRLA_MODE( I2C_MASTER_OPERATION )/* |
                                SERCOM_I2CM_CTRLA_SCLSM*/ ;

  // Enable Smart mode and Quick Command
  //sercom->I2CM.CTRLB.reg =  SERCOM_I2CM_CTRLB_SMEN /*| SERCOM_I2CM_CTRLB_QCEN*/ ;


  // Enable all interrupts
//  sercom->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB | SERCOM_I2CM_INTENSET_ERROR ;

  // Synchronous arithmetic baudrate
#if defined(__SAMD51__)
      sercom->I2CM.BAUD.bit.BAUD = SERCOM_FREQ_REF / ( 2 * freq ) - 1 ;
#else
      sercom->I2CM.BAUD.bit.BAUD = SystemCoreClock / ( 2 * baudrate) - 5 - (((SystemCoreClock / 1000000) * WIRE_RISE_TIME_NANOSECONDS) / (2 * 1000));
#endif


      sercom->I2CM.CTRLA.bit.ENABLE = 1 ;
      while ( sercom->I2CM.SYNCBUSY.bit.ENABLE != 0 );

      sercom->I2CM.STATUS.bit.BUSSTATE = 1 ;
      while ( sercom->I2CM.SYNCBUSY.bit.SYSOP != 0 );

// ---------------------

      // Wait idle or owner bus mode
      while ( sercom->I2CM.STATUS.bit.BUSSTATE != WIRE_IDLE_STATE
           && sercom->I2CM.STATUS.bit.BUSSTATE != WIRE_OWNER_STATE);

      // Send start and address
      sercom->I2CM.ADDR.bit.ADDR = i2c_addr << 1 | read;

      return {};
    }

    cpp::result<void, error_t> endTransaction(int sercom_index)
    {
      if ((std::size_t)sercom_index >= samd51::sercom_max) { return cpp::fail(error_t::invalid_arg); }

      return {};
    }

    cpp::result<void, error_t> writeBytes(int sercom_index, const std::uint8_t *data, std::size_t length)
    {
      if ((std::size_t)sercom_index >= samd51::sercom_max) { return cpp::fail(error_t::invalid_arg); }

      return {};
    }

    cpp::result<void, error_t> readBytes(int sercom_index, std::uint8_t *data, std::size_t length)
    {
      if ((std::size_t)sercom_index >= samd51::sercom_max) { return cpp::fail(error_t::invalid_arg); }

      return {};
    }


    cpp::result<void, error_t> transactionWrite(int sercom_index, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint32_t freq)
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

    cpp::result<void, error_t> transactionRead(int sercom_index, int addr, std::uint8_t *readdata, std::uint8_t readlen, std::uint32_t freq)
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

    cpp::result<void, error_t> transactionWriteRead(int sercom_index, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint8_t *readdata, std::size_t readlen, std::uint32_t freq)
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

    cpp::result<std::uint8_t, error_t> registerRead8(int sercom_index, int addr, std::uint8_t reg, std::uint32_t freq)
    {
      auto res = transactionWriteRead(sercom_index, addr, &reg, 1, &reg, 1, freq);
      if (res.has_value()) { return reg; }
      return cpp::fail( res.error() );
    }

    cpp::result<void, error_t> registerWrite8(int sercom_index, int addr, std::uint8_t reg, std::uint8_t data, std::uint8_t mask, std::uint32_t freq)
    {
      std::uint8_t tmp[2] = { reg, data };
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
