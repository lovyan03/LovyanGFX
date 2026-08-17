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
// SPI on plain GPIO, selected by a negative host number: host -1 is slot 0,
// host -2 is slot 1. It reaches a bus without claiming an SPI peripheral, and
// the convention matches the touch driver configs, where a negative host has
// always meant bit banged GPIO.
//
// This file is not a header but a code fragment: a platform implementation
// includes it inside its spi namespace, and delegates to the soft_spi_*()
// functions at the top of each public function when the host is negative.
// Everything here has internal linkage, so nothing beyond the negative host
// convention becomes part of the library interface. Only the portable pin
// functions are used, which keeps the fragment platform independent.
#if !defined ( LGFX_INTERNAL_SOFT_SPI )
 #error "soft_spi.inl is an implementation fragment of the spi namespace; it cannot be included directly."
#endif

//----------------------------------------------------------------------------

    static constexpr int soft_spi_host_count = 2;

    struct soft_spi_context_t
    {
      int pin_sclk = -1;
      int pin_miso = -1;
      int pin_mosi = -1;
      uint32_t half_us = 0;  // 0 = unthrottled
      uint8_t mode = 0;
    };
    static soft_spi_context_t soft_spi_context[soft_spi_host_count];

    static inline bool soft_spi_valid_host(int spi_host) { return -soft_spi_host_count <= spi_host && spi_host < 0; }
    static inline soft_spi_context_t& soft_spi_ctx(int spi_host) { return soft_spi_context[~spi_host]; }

    static inline void soft_spi_half_wait(uint32_t half_us)
    {
      if (half_us) { delayMicroseconds(half_us); }
    }

    static inline void soft_spi_clock_idle(int pin_sclk, uint8_t spi_mode)
    {
      if (pin_sclk < 0) { return; }
      if (spi_mode & 2) { gpio_hi(pin_sclk); } else { gpio_lo(pin_sclk); }
    }

    /// Full duplex byte shift, MSB first. read_data may be the write_data
    /// buffer (in place), or nullptr to discard the incoming bits.
    /// half_us throttles the clock (0 = unthrottled); the clock is left at its
    /// idle level, which the caller has to establish before the first call
    /// ( see soft_spi_clock_idle ).
    static inline void soft_spi_transfer(uint8_t* read_data, const uint8_t* write_data, size_t len, const soft_spi_context_t& ctx)
    {
      if (!len) { return; }
      const int pin_sclk = ctx.pin_sclk;
      const int pin_miso = ctx.pin_miso;
      const int pin_mosi = ctx.pin_mosi;
      const uint32_t half_us = ctx.half_us;
      const bool cpha = ctx.mode & 1;
      const bool lead = !(ctx.mode & 2);  // the clock level of the leading edge
      do
      {
        uint_fast8_t w = *write_data++;
        uint_fast8_t r = 0;
        uint_fast8_t mask = 0x80;
        do
        {
          if (!cpha)
          { // The data is set while the clock idles and sampled on the leading edge.
            if (pin_mosi >= 0) { if (w & mask) { gpio_hi(pin_mosi); } else { gpio_lo(pin_mosi); } }
            soft_spi_half_wait(half_us);
            if (lead) { gpio_hi(pin_sclk); } else { gpio_lo(pin_sclk); }
            if (pin_miso >= 0 && gpio_in(pin_miso)) { r += mask; }
            soft_spi_half_wait(half_us);
            if (lead) { gpio_lo(pin_sclk); } else { gpio_hi(pin_sclk); }
          }
          else
          { // The data is set on the leading edge and sampled on the trailing edge.
            if (lead) { gpio_hi(pin_sclk); } else { gpio_lo(pin_sclk); }
            if (pin_mosi >= 0) { if (w & mask) { gpio_hi(pin_mosi); } else { gpio_lo(pin_mosi); } }
            soft_spi_half_wait(half_us);
            if (lead) { gpio_lo(pin_sclk); } else { gpio_hi(pin_sclk); }
            if (pin_miso >= 0 && gpio_in(pin_miso)) { r += mask; }
            soft_spi_half_wait(half_us);
          }
        } while (mask >>= 1);
        if (read_data) { *read_data++ = r; }
      } while (--len);
    }

// ------------------------------------------------------------------------
// Mirrors of the public functions, for the negative hosts.

    static inline cpp::result<void, error_t> soft_spi_init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi)
    {
      if (!soft_spi_valid_host(spi_host)) { return cpp::fail(error_t::invalid_arg); }
      if (spi_sclk < 0) { return cpp::fail(error_t::invalid_arg); }  // the clock is not optional
      auto& ctx = soft_spi_ctx(spi_host);
      ctx.pin_sclk = spi_sclk;
      ctx.pin_miso = spi_miso;
      ctx.pin_mosi = spi_mosi;
      ctx.mode = 0;
      ctx.half_us = 0;
      if (spi_sclk >= 0)
      {
        gpio_lo(spi_sclk); // low first, so that no HIGH pulse leaves the pin (panels without CS)
        pinMode(spi_sclk, pin_mode_t::output);
      }
      if (spi_mosi >= 0) { pinMode(spi_mosi, pin_mode_t::output); }
      if (spi_miso >= 0) { pinMode(spi_miso, pin_mode_t::input_pullup); }
      return {};
    }

    static inline void soft_spi_release(int spi_host)
    {
      if (!soft_spi_valid_host(spi_host)) { return; }
      auto& ctx = soft_spi_ctx(spi_host);
      if (ctx.pin_sclk >= 0) { pinMode(ctx.pin_sclk, pin_mode_t::input); }
      if (ctx.pin_miso >= 0) { pinMode(ctx.pin_miso, pin_mode_t::input); }
      if (ctx.pin_mosi >= 0) { pinMode(ctx.pin_mosi, pin_mode_t::input); }
      ctx = soft_spi_context_t();
    }

    static inline void soft_spi_beginTransaction(int spi_host)
    { // Nothing to acquire; just make sure the clock rests at its idle level.
      if (!soft_spi_valid_host(spi_host)) { return; }
      auto& ctx = soft_spi_ctx(spi_host);
      soft_spi_clock_idle(ctx.pin_sclk, ctx.mode);
    }

    static inline void soft_spi_beginTransaction(int spi_host, uint32_t freq, int spi_mode)
    {
      if (!soft_spi_valid_host(spi_host)) { return; }
      auto& ctx = soft_spi_ctx(spi_host);
      ctx.mode = spi_mode & 3;
      if (freq == 0) { freq = 100000; }  // an unset frequency falls back to a safe default
      // Best effort, rounded up: the throttled clock must not come out above the request.
      ctx.half_us = (freq >= 500000) ? 0 : (500000 + freq - 1) / freq;
      soft_spi_beginTransaction(spi_host);
    }

    static inline void soft_spi_writeBytes(int spi_host, const uint8_t* data, size_t length)
    {
      if (!soft_spi_valid_host(spi_host)) { return; }
      soft_spi_transfer(nullptr, data, length, soft_spi_ctx(spi_host));
    }

    static inline void soft_spi_readBytes(int spi_host, uint8_t* data, size_t length)
    { // Full duplex, in place: the same semantics as the peripheral paths.
      if (!soft_spi_valid_host(spi_host)) { return; }
      soft_spi_transfer(data, data, length, soft_spi_ctx(spi_host));
    }

//----------------------------------------------------------------------------
