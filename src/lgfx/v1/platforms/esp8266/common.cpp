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
#if defined ( ESP8266 )

#include "../common.hpp"

#if defined ( ARDUINO )
 #include <SPI.h>
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

#if defined ( ARDUINO )

  void pinMode(int_fast16_t pin, pin_mode_t mode)
  {
    switch (mode)
    {
    case pin_mode_t::output:
      ::pinMode(pin, OUTPUT);
      break;

#if defined (INPUT_PULLUP)
    case pin_mode_t::input_pullup:
      ::pinMode(pin, INPUT_PULLUP);
      break;
#endif
#if defined (INPUT_PULLDOWN)
    case pin_mode_t::input_pulldown:
      ::pinMode(pin, INPUT_PULLDOWN);
      break;
#endif
#if defined (INPUT_PULLDOWN_16)
    case pin_mode_t::input_pulldown:
      ::pinMode(pin, INPUT_PULLDOWN_16);
      break;
#endif

    default:
      ::pinMode(pin, INPUT);
      break;
    }
  }

#else

  #define  MAGIC_1E3_wLO  0x4bc6a7f0    // LS part
  #define  MAGIC_1E3_wHI  0x00418937    // MS part, magic multiplier

  unsigned long IRAM_ATTR millis()
  {
    union {
      uint64_t  q;     // Accumulator, 64-bit, little endian
      uint32_t  a[2];  // ..........., 32-bit  segments
    } acc;
    acc.a[1] = 0;       // Zero high-acc
    
    // Get usec system time, usec overflow counter
    uint32_t  m = system_get_time();
    uint32_t  c = micros_overflow_count +
                    ((m < micros_at_last_overflow_tick) ? 1 : 0);

    // (a) Init. low-acc with high-word of 1st product. The right-shift
    //     falls on a byte boundary, hence is relatively quick.
    
    acc.q  = ( (uint64_t)( m * (uint64_t)MAGIC_1E3_wLO ) >> 32 );

    // (b) Offset sum, low-acc
    acc.q += ( m * (uint64_t)MAGIC_1E3_wHI );

    // (c) Offset sum, low-acc
    acc.q += ( c * (uint64_t)MAGIC_1E3_wLO );

    // (d) Truncated sum, high-acc
    acc.a[1] += (uint32_t)( c * (uint64_t)MAGIC_1E3_wHI );

    return ( acc.a[1] );  // Extract result, high-acc

  } //millis

  unsigned long IRAM_ATTR micros() {
      return system_get_time();
  }

  void delay(unsigned long ms)
  {
    if(ms) {
        os_timer_setfn(&delay_timer, (os_timer_func_t*) &delay_end, 0);
        os_timer_arm(&delay_timer, ms, ONCE);
    } else {
        esp_schedule();
    }
    esp_yield();
    if(ms) {
        os_timer_disarm(&delay_timer);
    }
  }

#endif

//----------------------------------------------------------------------------

  /// unimplemented.
  namespace spi
  {
    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi)
    {
#if defined ( ARDUINO )
      SPI.pins(spi_sclk, spi_miso, spi_mosi, -1);
      SPI.begin();
#endif
/*
    if (spi_sclk == 6
     && spi_miso == 7
     && spi_mosi == 8)
    {
     // pinSet = SPI_PINS_HSPI_OVERLAP;
    } else
    if (spi_sclk == 14
     && spi_miso == 12
     && spi_mosi == 13)
    {
        pinSet = SPI_PINS_HSPI;
    }
    else
    {
      return false;
    }
    //*/

      return {};
    }
    void release(int spi_host)
    {
#if defined ( ARDUINO )
      SPI.end();
#endif
    }
    void beginTransaction(int spi_host, uint32_t freq, int spi_mode)
    {
#if defined ( ARDUINO )
      SPI.setFrequency(freq);
      SPI.setBitOrder(MSBFIRST);
      SPI.setDataMode(spi_mode);
#endif
      SPI1U &= ~(SPIUMISO | SPIUDUPLEX);
    }
    void beginTransaction(int spi_host)
    {
      SPI1U &= ~(SPIUMISO | SPIUDUPLEX);
    }
    void endTransaction(int spi_host)
    {
      while (SPI1CMD & SPIBUSY);
#if defined ( ARDUINO )
      SPI.endTransaction();
#endif
      SPI1U |= SPIUDUPLEX;
    }
    void writeBytes(int spi_host, const uint8_t* data, size_t length)
    {
      const uint32_t u1 = ((length << 3) - 1) << SPILMOSI;
      while (SPI1CMD & SPIBUSY);
      SPI1U1 = u1;
      memcpy_P(reinterpret_cast<void*>(SPI1W0), data, length);
      SPI1CMD = SPIBUSY;
    }
    void readBytes(int spi_host, uint8_t* data, size_t length)
    {
      const uint32_t u1 = ((length << 3) - 1) << SPILMISO;
      while (SPI1CMD & SPIBUSY);
      SPI1U1 = u1;
      memcpy(reinterpret_cast<void*>(SPI1W0), data, length);
      SPI1CMD = SPIBUSY;
      while (SPI1CMD & SPIBUSY) {}
      memcpy(data, reinterpret_cast<void*>(SPI1W0), length);
    }
  }

//----------------------------------------------------------------------------

  /// unimplemented.
  namespace i2c
  {
    static constexpr int I2C_NUM_MAX = 2;

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
      uint8_t pin_scl = -1;
      uint8_t pin_sda = -1;
      uint32_t freq = 0;

      int nopwait = 26;

      bool WAIT_CLOCK_STRETCH(uint32_t msec = 5000)
      {
        uint32_t scl = 1 << pin_scl;
        int i = 256;
        while (!(GPI & scl) && --i);
        if (i) return true;

        auto start = millis();
        do
        {
          yield();
          if (millis() - start > msec) return false;
        } while (!(GPI & scl));

        return true;
      }

      bool write_start(void)
      {
        uint32_t sda = 1 << pin_sda;
        uint32_t scl = 1 << pin_scl;

        int retry = 9;
        while (!(GPI & sda) && --retry)
        {
          GPES = scl; // lo
          GPEC = sda; // hi
          for (int i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }
          GPEC = scl; // hi
          for (int i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }
        }
        GPES = sda; // lo
        for (int i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }
        return retry;
      }

      bool write_stop(void)
      {
        uint32_t scl = 1 << pin_scl;
        GPES = scl; // lo

        uint32_t sda = 1 << pin_sda;
        GPEC = sda; // hi

        for (int i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }
        // SDAがHIGHになるまでクロック送出しながら待機する。;
        int retry = 9;
        while (!(GPI & sda) && (--retry))
        {
          GPEC = scl; // hi
          for (int i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }
          GPES = scl; // lo
          for (int i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }
        }
        GPES = sda; // lo
        GPEC = scl; // hi
        for (int i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }

        GPEC = sda; // hi

        return retry;
      }

      bool write_byte(uint8_t byte)
      {
        uint32_t sda = 1 << pin_sda;
        uint32_t scl = 1 << pin_scl;
        uint32_t mask = 0x80;
        volatile uint32_t* outreg[2] = { &GPES, &GPEC };
        int i;
        do
        {
          *outreg[0] = scl; // lo
          *outreg[(bool)(byte & mask)] = sda;
          mask >>= 1;
          for (i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }
          *outreg[1] = scl; // hi
          for (i = (nopwait+1)>>1; i; --i) { __asm__ __volatile__("nop"); }
          i = 256;
          while (!(GPI & scl) && --i);
        } while (i && mask);
        *outreg[0] = scl; // lo
        *outreg[1] = sda; // hi
        for (i = (nopwait+1)>>1; i; --i) { __asm__ __volatile__("nop"); }
        *outreg[1] = scl; // hi
        for (i = (nopwait>>1)-16; i > 0; --i) { __asm__ __volatile__("nop"); }

        i = 256;
        while (!(GPI & scl) && --i);
        return ((i || WAIT_CLOCK_STRETCH())) && !(GPI & sda); //NACK/ACK
      }

      uint8_t read_byte(bool nack)
      {
        uint32_t sda = 1 << pin_sda;
        uint32_t scl = 1 << pin_scl;
        uint8_t byte = 0;
        int i;
        for (int idx = 0; idx < 8; idx++)
        {
          GPES = scl; // lo
          GPEC = sda; // hi
          for (i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }
          GPEC = scl; // hi
          for (i = (nopwait+1)>>1; i; --i) { __asm__ __volatile__("nop"); }

          i = 256;
          while (!(GPI & scl) && --i);

          byte = (byte << 1) + (bool)(GPI & sda);
        }
        GPES = scl; // lo
        (nack ? GPEC : GPES) = sda;
        for (i = nopwait>>1; i; --i) { __asm__ __volatile__("nop"); }
        GPEC = scl; // hi
        for (i = (nopwait+1)>>1; i; --i) { __asm__ __volatile__("nop"); }

        return byte;
      }
    };
    i2c_context_t i2c_context[I2C_NUM_MAX];

    cpp::result<void, error_t> init(int i2c_port, int pin_sda, int pin_scl)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      auto i2c = &i2c_context[i2c_port];

      pinMode(pin_sda, pin_mode_t::input_pullup);
      pinMode(pin_scl, pin_mode_t::input_pullup);
      i2c->nopwait = 32;
      i2c->pin_scl = pin_scl;
      i2c->pin_sda = pin_sda;
      i2c->write_stop();

      return {};
    }
    cpp::result<void, error_t> release(int i2c_port)
    {
      return {};
    }
    cpp::result<void, error_t> restart(int i2c_port, int i2c_addr, uint32_t freq, bool read)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      auto i2c = &i2c_context[i2c_port];

      if (i2c->freq != freq)
      {
        i2c->freq = freq;
        int tmp = (500000000 / freq);  // half-cycle period in ns
        int overhead = 280;
        int div = 40630;
        if (esp_get_cpu_freq_mhz() >= 160) // 160MHz
        {
          div >>= 1;
          overhead >>= 1;
        }
        i2c->nopwait = (1000 * std::max(0, tmp - overhead)) / div; // (half cycle - overhead) / busywait loop time
      }

      if (i2c->write_start()
       && i2c->write_byte((i2c_addr << 1 ) + read))
      {
        return {};
      }
      return cpp::fail(error_t::connection_lost);
    }
    cpp::result<void, error_t> beginTransaction(int i2c_port, int i2c_addr, uint32_t freq, bool read)
    {
      return restart(i2c_port, i2c_addr, freq, read);
    }
    cpp::result<void, error_t> endTransaction(int i2c_port)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      auto i2c = &i2c_context[i2c_port];

      if (i2c->write_stop())
      {
        return {};
      }
      return cpp::fail(error_t::connection_lost);
    }
    cpp::result<void, error_t> writeBytes(int i2c_port, const uint8_t *data, size_t length)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      auto i2c = &i2c_context[i2c_port];
      if (length)
      {
        do
        {
          if (!i2c->write_byte(*data++))
          {
            return cpp::fail(error_t::connection_lost);
          }
        } while (--length);
      }
      return {};
    }
    cpp::result<void, error_t> readBytes(int i2c_port, uint8_t *data, size_t length)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      auto i2c = &i2c_context[i2c_port];
      while (length--)
      {
        *data++ = i2c->read_byte(false);
      }
      return {};
    }

//--------

    cpp::result<void, error_t> transactionWrite(int i2c_port, int addr, const uint8_t *writedata, uint8_t writelen, uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(i2c_port, addr, freq, false)).has_value()
       && (res = writeBytes(i2c_port, writedata, writelen)).has_value()
      )
      {
        res = endTransaction(i2c_port);
      }
      return res;
    }

    cpp::result<void, error_t> transactionRead(int i2c_port, int addr, uint8_t *readdata, uint8_t readlen, uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(i2c_port, addr, freq, true)).has_value()
       && (res = readBytes(i2c_port, readdata, readlen)).has_value()
      )
      {
        res = endTransaction(i2c_port);
      }
      return res;
    }

    cpp::result<void, error_t> transactionWriteRead(int i2c_port, int addr, const uint8_t *writedata, uint8_t writelen, uint8_t *readdata, size_t readlen, uint32_t freq)
    {
      cpp::result<void, error_t> res;
      if ((res = beginTransaction(i2c_port, addr, freq, false)).has_value()
       && (res = writeBytes(i2c_port, writedata, writelen)).has_value()
       && (res = restart(i2c_port, addr, freq, true)).has_value()
       && (res = readBytes(i2c_port, readdata, readlen)).has_value()
      )
      {
        res = endTransaction(i2c_port);
      }
      return res;
    }

    cpp::result<uint8_t, error_t> readRegister8(int i2c_port, int addr, uint8_t reg, uint32_t freq)
    {
      auto res = transactionWriteRead(i2c_port, addr, &reg, 1, &reg, 1, freq);
      if (res.has_value()) { return reg; }
      return cpp::fail( res.error() );
    }

    cpp::result<void, error_t> writeRegister8(int i2c_port, int addr, uint8_t reg, uint8_t data, uint8_t mask, uint32_t freq)
    {
      uint8_t tmp[2] = { reg, data };
      if (mask)
      {
        auto res = transactionWriteRead(i2c_port, addr, &reg, 1, &tmp[1], 1, freq);
        if (res.has_error()) { return res; }
        tmp[1] = (tmp[1] & mask) | data;
      }
      return transactionWrite(i2c_port, addr, tmp, 2, freq);
    }

  }

//----------------------------------------------------------------------------
 }
}

#endif
