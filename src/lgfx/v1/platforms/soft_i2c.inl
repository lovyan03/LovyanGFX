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
// I2C on plain GPIO, selected by a negative port number: port -1 is slot 0,
// port -2 is slot 1. It reaches a bus without claiming a peripheral unit, for
// when every unit is taken or when touching one is undesirable.
//
// This file is not a header but a code fragment: a platform implementation
// includes it inside its i2c namespace, and delegates to the soft_i2c_*()
// functions at the top of each public function when the port is negative.
// Everything here has internal linkage, so nothing beyond the negative port
// convention becomes part of the library interface.
//
// The transaction semantics mirror the peripheral paths: an unacknowledged
// address surfaces at the next operation or at endTransaction, not at
// beginTransaction. On a failure the bus is stopped and the lock released
// right away, and the stored error is reported (and cleared) later.
// Clock stretching is honored wherever SCL is released.
#if !defined ( LGFX_INTERNAL_SOFT_I2C )
 #error "soft_i2c.inl is an implementation fragment of the i2c namespace; it cannot be included directly."
#endif

// An I2C line is only ever driven low or released, never driven high. The
// default implementation releases by turning the pin back into an input and
// drives by turning it into an output whose latch was parked low, which works
// on any platform. A platform whose pinMode() gives an input mode an open
// drain output stage (esp32) overrides these with plain latch toggles and
// skips the direction changes.
#if !defined ( SOFT_I2C_LINE_LO )
 #define SOFT_I2C_LINE_LO(pin)   do { gpio_lo(pin); pinMode(pin, pin_mode_t::output); } while (0)
 #define SOFT_I2C_LINE_HI(pin)   pinMode(pin, pin_mode_t::input_pullup)
#endif

// Serialization of the transactions on a slot. A platform with an RTOS wraps
// a mutex over the lock_handle member; the default is single threaded.
#if !defined ( SOFT_I2C_LOCK )
 #define SOFT_I2C_LOCK(ctx)
 #define SOFT_I2C_UNLOCK(ctx)
#endif

// Called while waiting out a stretched clock.
#if !defined ( SOFT_I2C_YIELD )
 #if defined ( ARDUINO )
  #define SOFT_I2C_YIELD() yield()
 #else
  #define SOFT_I2C_YIELD()
 #endif
#endif

//----------------------------------------------------------------------------

    static constexpr int soft_i2c_port_count = 2;

    static constexpr int soft_i2c_7bit_addr_min = 0x08;
    static constexpr int soft_i2c_7bit_addr_max = 0x77;
    static constexpr int soft_i2c_10bit_addr_max = 1023;

    struct soft_i2c_context_t
    {
      enum state_t
      {
        state_disconnect,
        state_write,
        state_read
      };
      cpp::result<state_t, error_t> state;
      void* lock_handle = nullptr;  // storage for the SOFT_I2C_LOCK hooks
      int pin_sda = -1;
      int pin_scl = -1;
      uint32_t freq = 0;
      uint32_t half_us = 0;  // 0 = unthrottled
      bool initialized = false;
    };
    static soft_i2c_context_t soft_i2c_context[soft_i2c_port_count];

    static inline bool soft_i2c_valid_port(int i2c_port) { return -soft_i2c_port_count <= i2c_port && i2c_port < 0; }
    static inline soft_i2c_context_t& soft_i2c_ctx(int i2c_port) { return soft_i2c_context[~i2c_port]; }

    static inline void soft_i2c_half_wait(uint32_t half_us)
    { if (half_us) { delayMicroseconds(half_us); } }

    static inline void soft_i2c_set_freq(soft_i2c_context_t& ctx, uint32_t freq)
    {
      if (freq == 0) { freq = 100000; }  // an unset frequency falls back to the usual default
      ctx.freq = freq;
      // Rounded up: the throttled clock must not come out above the request.
      ctx.half_us = (freq >= 500000) ? 0 : (500000 + freq - 1) / freq;
    }

    /// Release SCL and wait for it to actually rise, honoring clock stretching.
    static inline bool soft_i2c_scl_hi(const soft_i2c_context_t& ctx)
    {
      SOFT_I2C_LINE_HI(ctx.pin_scl);
      soft_i2c_half_wait(ctx.half_us);
      if (gpio_in(ctx.pin_scl)) { return true; }
      auto us = micros();
      do
      {
        SOFT_I2C_YIELD();
        if (gpio_in(ctx.pin_scl)) { return true; }
      } while ((micros() - us) < 13000); // the same order as a peripheral timeout
      return false;
    }

    /// Returns true when the byte was acknowledged.
    static inline bool soft_i2c_write_byte(const soft_i2c_context_t& ctx, uint8_t data)
    {
      size_t i = 0;
      do
      {
        SOFT_I2C_LINE_LO(ctx.pin_scl);
        if (data & 0x80) { SOFT_I2C_LINE_HI(ctx.pin_sda); } else { SOFT_I2C_LINE_LO(ctx.pin_sda); }
        data <<= 1;
        soft_i2c_half_wait(ctx.half_us);
        if (!soft_i2c_scl_hi(ctx)) { return false; }
      } while (++i < 8);
      SOFT_I2C_LINE_LO(ctx.pin_scl);
      SOFT_I2C_LINE_HI(ctx.pin_sda);  // release the data line for the acknowledge
      soft_i2c_half_wait(ctx.half_us);
      if (!soft_i2c_scl_hi(ctx)) { return false; }
      bool ack = !gpio_in(ctx.pin_sda);
      SOFT_I2C_LINE_LO(ctx.pin_scl);
      return ack;
    }

    static inline bool soft_i2c_read_byte(const soft_i2c_context_t& ctx, uint8_t* data, bool ack)
    {
      uint_fast8_t byte = 0;
      SOFT_I2C_LINE_HI(ctx.pin_sda);  // released: the device drives the data line
      size_t i = 0;
      do
      {
        SOFT_I2C_LINE_LO(ctx.pin_scl);
        soft_i2c_half_wait(ctx.half_us);
        if (!soft_i2c_scl_hi(ctx)) { return false; }
        byte = (byte << 1) + (gpio_in(ctx.pin_sda) ? 1 : 0);
      } while (++i < 8);
      SOFT_I2C_LINE_LO(ctx.pin_scl);
      if (ack) { SOFT_I2C_LINE_LO(ctx.pin_sda); } else { SOFT_I2C_LINE_HI(ctx.pin_sda); }
      soft_i2c_half_wait(ctx.half_us);
      if (!soft_i2c_scl_hi(ctx)) { return false; }
      SOFT_I2C_LINE_LO(ctx.pin_scl);
      SOFT_I2C_LINE_HI(ctx.pin_sda);
      *data = byte;
      return true;
    }

    /// Returns false when the clock could not be released for the stop condition.
    static inline bool soft_i2c_stop_cond(const soft_i2c_context_t& ctx)
    {
      SOFT_I2C_LINE_LO(ctx.pin_scl);
      SOFT_I2C_LINE_LO(ctx.pin_sda);
      soft_i2c_half_wait(ctx.half_us);
      bool ok = soft_i2c_scl_hi(ctx);
      soft_i2c_half_wait(ctx.half_us);
      SOFT_I2C_LINE_HI(ctx.pin_sda);
      soft_i2c_half_wait(ctx.half_us);
      return ok;
    }

    /// Free a bus whose device was left mid transfer (e.g. by a reset) and holds
    /// the data line, by clocking the leftover bits out.
    static inline void soft_i2c_recover(const soft_i2c_context_t& ctx)
    {
      SOFT_I2C_LINE_HI(ctx.pin_sda);
      size_t i = 0;
      while (!gpio_in(ctx.pin_sda) && ++i <= 9)
      {
        SOFT_I2C_LINE_LO(ctx.pin_scl);
        soft_i2c_half_wait(ctx.half_us);
        soft_i2c_scl_hi(ctx);
      }
      soft_i2c_stop_cond(ctx);
    }

    /// Ends a transfer on a failure. The bus is stopped and the lock released
    /// right away; the error is stored so that it surfaces from the following
    /// operation or from endTransaction, as it does on a peripheral path.
    static inline void soft_i2c_abort(soft_i2c_context_t& ctx)
    {
      soft_i2c_stop_cond(ctx);
      ctx.state = cpp::fail(error_t::connection_lost);
      SOFT_I2C_UNLOCK(ctx);
    }

    static inline cpp::result<void, error_t> soft_i2c_send_address(soft_i2c_context_t& ctx, int i2c_addr, bool read)
    {
      bool ack;
      if (i2c_addr <= soft_i2c_7bit_addr_max)
      {
        ack = soft_i2c_write_byte(ctx, i2c_addr << 1 | (read ? 1 : 0));
      }
      else
      {
        ack = soft_i2c_write_byte(ctx, 0xF0 | (i2c_addr >> 8) << 1)
           && soft_i2c_write_byte(ctx, i2c_addr & 0xFF);
        if (ack && read)
        { // A 10 bit read re-addresses the high byte in read mode.
          SOFT_I2C_LINE_HI(ctx.pin_sda);
          soft_i2c_half_wait(ctx.half_us);
          ack = soft_i2c_scl_hi(ctx);
          if (ack)
          {
            SOFT_I2C_LINE_LO(ctx.pin_sda);
            soft_i2c_half_wait(ctx.half_us);
            SOFT_I2C_LINE_LO(ctx.pin_scl);
            ack = soft_i2c_write_byte(ctx, 0xF0 | (i2c_addr >> 8) << 1 | 1);
          }
        }
      }
      if (!ack)
      {
        soft_i2c_abort(ctx);
        return {};
      }
      ctx.state = read ? soft_i2c_context_t::state_t::state_read : soft_i2c_context_t::state_t::state_write;
      return {};
    }

// ------------------------------------------------------------------------
// Mirrors of the public functions, for the negative ports.

    static inline cpp::result<void, error_t> soft_i2c_release(int i2c_port)
    {
      if (!soft_i2c_valid_port(i2c_port)) { return cpp::fail(error_t::invalid_arg); }
      auto& ctx = soft_i2c_ctx(i2c_port);
      if (ctx.initialized)
      {
        ctx.initialized = false;
        // Park the pins the same way the peripheral paths do.
        if (ctx.pin_scl >= 0) { pinMode(ctx.pin_scl, pin_mode_t::input_pullup); }
        if (ctx.pin_sda >= 0) { pinMode(ctx.pin_sda, pin_mode_t::input_pullup); }
      }
      return {};
    }

    static inline cpp::result<void, error_t> soft_i2c_setPins(int i2c_port, int pin_sda, int pin_scl)
    {
      if (!soft_i2c_valid_port(i2c_port) || pin_sda < 0 || pin_scl < 0)
      {
        return cpp::fail(error_t::invalid_arg);
      }
      auto& ctx = soft_i2c_ctx(i2c_port);
      if (ctx.initialized && ctx.pin_sda == pin_sda && ctx.pin_scl == pin_scl)
      {
        return {};
      }
      soft_i2c_release(i2c_port).has_value();
      ctx.pin_sda = pin_sda;
      ctx.pin_scl = pin_scl;
      return {};
    }

    static inline cpp::result<int, error_t> soft_i2c_getPinSDA(int i2c_port)
    {
      if (!soft_i2c_valid_port(i2c_port)) { return cpp::fail(error_t::invalid_arg); }
      return soft_i2c_ctx(i2c_port).pin_sda;
    }

    static inline cpp::result<int, error_t> soft_i2c_getPinSCL(int i2c_port)
    {
      if (!soft_i2c_valid_port(i2c_port)) { return cpp::fail(error_t::invalid_arg); }
      return soft_i2c_ctx(i2c_port).pin_scl;
    }

    static inline cpp::result<void, error_t> soft_i2c_init(int i2c_port)
    {
      if (!soft_i2c_valid_port(i2c_port)) { return cpp::fail(error_t::invalid_arg); }
      auto& ctx = soft_i2c_ctx(i2c_port);
      if (ctx.pin_sda < 0 || ctx.pin_scl < 0) { return cpp::fail(error_t::invalid_arg); }
      if (ctx.initialized)
      {
        soft_i2c_release(i2c_port).has_value();
      }
      soft_i2c_set_freq(ctx, 100000);  // until a transfer brings its own
      // Both lines are released; the pullup carries them high when the bus is
      // healthy. ( see SOFT_I2C_LINE_HI for how a line is driven afterwards )
      pinMode(ctx.pin_sda, pin_mode_t::input_pullup);
      pinMode(ctx.pin_scl, pin_mode_t::input_pullup);
      ctx.state = soft_i2c_context_t::state_t::state_disconnect;
      ctx.initialized = true;
      return {};
    }

    static inline cpp::result<void, error_t> soft_i2c_beginTransaction(int i2c_port, int i2c_addr, uint32_t freq, bool read)
    {
      if (!soft_i2c_valid_port(i2c_port)) { return cpp::fail(error_t::invalid_arg); }
      auto& ctx = soft_i2c_ctx(i2c_port);
      if (ctx.pin_sda < 0 || ctx.pin_scl < 0) { return cpp::fail(error_t::invalid_arg); }
      if (i2c_addr < soft_i2c_7bit_addr_min || i2c_addr > soft_i2c_10bit_addr_max) { return cpp::fail(error_t::invalid_arg); }
      SOFT_I2C_LOCK(ctx);
      ctx.state = soft_i2c_context_t::state_t::state_disconnect;
      soft_i2c_set_freq(ctx, freq);
      SOFT_I2C_LINE_HI(ctx.pin_sda);
      if (!soft_i2c_scl_hi(ctx))
      { // The clock never rose: no start condition can be made on this bus.
        soft_i2c_abort(ctx);
        return {};
      }
      if (!gpio_in(ctx.pin_sda))
      {
        soft_i2c_recover(ctx);
        if (!gpio_in(ctx.pin_sda))
        { // The data line is still held; without it no start condition can be
          // made, and the acknowledge bits would read as permanently asserted.
          soft_i2c_abort(ctx);
          return {};
        }
      }
      SOFT_I2C_LINE_LO(ctx.pin_sda);  // start condition
      soft_i2c_half_wait(ctx.half_us);
      SOFT_I2C_LINE_LO(ctx.pin_scl);
      return soft_i2c_send_address(ctx, i2c_addr, read);
    }

    static inline cpp::result<void, error_t> soft_i2c_restart(int i2c_port, int i2c_addr, uint32_t freq, bool read)
    {
      if (!soft_i2c_valid_port(i2c_port)) { return cpp::fail(error_t::invalid_arg); }
      if (i2c_addr < soft_i2c_7bit_addr_min || i2c_addr > soft_i2c_10bit_addr_max) { return cpp::fail(error_t::invalid_arg); }
      auto& ctx = soft_i2c_ctx(i2c_port);
      if (ctx.state.has_error()) { return cpp::fail(ctx.state.error()); }
      if (ctx.state == soft_i2c_context_t::state_disconnect)
      { // Only an open transaction can be restarted; the lock is not held here.
        return cpp::fail(error_t::mode_mismatch);
      }
      soft_i2c_set_freq(ctx, freq);
      SOFT_I2C_LINE_HI(ctx.pin_sda);  // repeated start
      soft_i2c_half_wait(ctx.half_us);
      if (!soft_i2c_scl_hi(ctx))
      {
        soft_i2c_abort(ctx);
        return {};
      }
      SOFT_I2C_LINE_LO(ctx.pin_sda);
      soft_i2c_half_wait(ctx.half_us);
      SOFT_I2C_LINE_LO(ctx.pin_scl);
      return soft_i2c_send_address(ctx, i2c_addr, read);
    }

    static inline cpp::result<void, error_t> soft_i2c_endTransaction(int i2c_port)
    {
      if (!soft_i2c_valid_port(i2c_port)) { return cpp::fail(error_t::invalid_arg); }
      auto& ctx = soft_i2c_ctx(i2c_port);
      if (ctx.state.has_error())
      { // The failing operation has already stopped the bus and released the lock;
        // report its error here and clear it.
        auto err = ctx.state.error();
        ctx.state = soft_i2c_context_t::state_t::state_disconnect;
        return cpp::fail(err);
      }
      if (ctx.state == soft_i2c_context_t::state_disconnect) { return {}; }
      bool stopped = soft_i2c_stop_cond(ctx);
      ctx.state = soft_i2c_context_t::state_t::state_disconnect;
      SOFT_I2C_UNLOCK(ctx);
      if (!stopped) { return cpp::fail(error_t::connection_lost); }
      return {};
    }

    static inline cpp::result<void, error_t> soft_i2c_writeBytes(int i2c_port, const uint8_t *data, size_t length)
    {
      if (!soft_i2c_valid_port(i2c_port)) { return cpp::fail(error_t::invalid_arg); }
      auto& ctx = soft_i2c_ctx(i2c_port);
      if (ctx.state.has_error()) { return cpp::fail(ctx.state.error()); }
      if (ctx.state != soft_i2c_context_t::state_write) { return cpp::fail(error_t::mode_mismatch); }
      if (!length) { return {}; }
      do
      {
        if (!soft_i2c_write_byte(ctx, *data++))
        {
          soft_i2c_abort(ctx);
          return cpp::fail(error_t::connection_lost);
        }
      } while (--length);
      return {};
    }

    static inline cpp::result<void, error_t> soft_i2c_readBytes(int i2c_port, uint8_t *readdata, size_t length, bool last_nack)
    {
      if (!soft_i2c_valid_port(i2c_port)) { return cpp::fail(error_t::invalid_arg); }
      auto& ctx = soft_i2c_ctx(i2c_port);
      if (ctx.state.has_error()) { return cpp::fail(ctx.state.error()); }
      if (ctx.state != soft_i2c_context_t::state_read) { return cpp::fail(error_t::mode_mismatch); }
      if (!length) { return {}; }
      do
      {
        if (!soft_i2c_read_byte(ctx, readdata++, !(last_nack && length == 1)))
        {
          soft_i2c_abort(ctx);
          return cpp::fail(error_t::connection_lost);
        }
      } while (--length);
      return {};
    }

//----------------------------------------------------------------------------
