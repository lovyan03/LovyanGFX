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

Porting for RP2040:
 [yasuhirok](https://github.com/yasuhirok-git)
/----------------------------------------------------------------------------*/
#if defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)

#include "common.hpp"
#include <array>
#include <unordered_map>

#include <hardware/structs/resets.h>
#include <hardware/structs/padsbank0.h>
#include <hardware/structs/sio.h>
#include <hardware/structs/iobank0.h>
#include <hardware/sync.h>
#include <hardware/resets.h>
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/i2c.h>

//#include <xprintf.h>
//#define DBGPRINT(fmt, ...)  xprintf("%s %d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
// static char dbg_buf[256];
// #define DBGPRINT(fmt, ...) snprintf(dbg_buf, 256, "%s %d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); Serial.print(dbg_buf);
#define DBGPRINT(fmt, ...)

// 参考:
// https://os.mbed.com/docs/mbed-os/v6.15/mbed-os-api-doxy/group__hal__gpio.html
// https://github.com/arduino/ArduinoCore-mbed/releases
// https://github.com/raspberrypi/pico-sdk

namespace lgfx
{
 inline namespace v1
 {
  namespace rp2040
  {
    static bool pin_check(int pin, const uint8_t *pinlist)
    {
      int idx = 0;
      while (pinlist[idx] != static_cast<int>(UINT8_MAX))
      {
        if (static_cast<int>(pinlist[idx]) == pin)
        {
          return true;
        }
        idx++;
      }
      return false;
    }
  }

  namespace {

    bool lgfx_gpio_set_function(int_fast16_t pin, enum gpio_function fn)
    {
      if (pin < 0 || pin >= static_cast<int_fast16_t>(NUM_BANK0_GPIOS))
      {
        return false;
      }
      if ((((uint32_t)fn << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB) & ~IO_BANK0_GPIO0_CTRL_FUNCSEL_BITS) != 0)
      {
        return false;
      }
      uint32_t temp = padsbank0_hw->io[pin];
      temp &= ~(PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);
      temp |= PADS_BANK0_GPIO0_IE_BITS;
      padsbank0_hw->io[pin] = temp;
      volatile iobank0_hw_t *iobank0_regs = reinterpret_cast<volatile iobank0_hw_t *>(IO_BANK0_BASE);
      iobank0_regs->io[pin].ctrl = fn << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
      return true;
    }

    bool lgfx_gpio_init(int_fast16_t pin)
    {
      if (pin < 0 || pin >= static_cast<int_fast16_t>(NUM_BANK0_GPIOS))
      {
        return false;
      }
      return lgfx_gpio_set_function(pin, GPIO_FUNC_SIO);
    }

    __attribute__ ((always_inline)) inline void lgfx_gpio_set_pulls(int_fast16_t pin, bool up, bool down)
    {
      uint32_t temp = padsbank0_hw->io[pin];
      temp &= ~(PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_PDE_BITS);
      temp |= (((up ? 1U : 0U) << PADS_BANK0_GPIO0_PUE_LSB) | ((down ? 1U : 0U) << PADS_BANK0_GPIO0_PDE_LSB));
    }

    __attribute__ ((always_inline)) inline void lgfx_gpio_pull_up(int_fast16_t pin)
    {
      lgfx_gpio_set_pulls(pin, true, false);
    }

    __attribute__ ((always_inline)) inline void lgfx_gpio_pull_down(int_fast16_t pin)
    {
      lgfx_gpio_set_pulls(pin, false, true);
    }

    __attribute__ ((always_inline)) inline void lgfx_gpio_disable_pulls(int_fast16_t pin)
    {
      lgfx_gpio_set_pulls(pin, false, false);
    }

    bool lgfx_gpio_mode(int_fast16_t pin, pin_mode_t mode)
    {
      if (pin < 0 || pin >= static_cast<int_fast16_t>(NUM_BANK0_GPIOS))
      {
        return false;
      }
      bool rc = false;
      switch (mode)
      {
      case pin_mode_t::output:
        [[fallthrough]];
      case pin_mode_t::input:
        lgfx_gpio_disable_pulls(pin);
        rc = true;
        break;
      case pin_mode_t::input_pullup:
        lgfx_gpio_pull_up(pin);
        rc = true;
        break;
      case pin_mode_t::input_pulldown:
        lgfx_gpio_pull_down(pin);
        rc = true;
        break;
      default:
        break;
      }
      return rc;
    }

    bool lgfx_gpio_set_dir(int_fast16_t pin, bool out)
    {
      if (pin < 0 || pin >= static_cast<int_fast16_t>(NUM_BANK0_GPIOS))
      {
        return false;
      }
      const uint32_t mask = 1UL << pin;
      if (out)
      {
        sio_hw->gpio_oe_set = mask;
      }
      else
      {
        sio_hw->gpio_oe_clr = mask;
      }
      return true;
    }

    __attribute__ ((always_inline)) inline bool lgfx_gpio_get(int_fast16_t pin)
    {
      return (((1UL << pin) & sio_hw->gpio_in) != 0);
    }

    __attribute__ ((always_inline)) inline void lgfx_gpio_put(int_fast16_t pin, bool value)
    {
      const uint32_t mask = 1ul << pin;
      if (value)
      {
        sio_hw->gpio_set = mask;
      }
      else
      {
        sio_hw->gpio_clr = mask;
      }
    }

    __attribute__((always_inline)) inline void lgfx_reset_block(uint32_t bits)
    {
      reset_block(bits);
    }

    __attribute__((always_inline)) inline void lgfx_unreset_block_wait(uint32_t bits)
    {
      unreset_block_wait(bits);
    }
  }

  void pinMode(int_fast16_t pin, pin_mode_t mode)
  {
    if (pin < 0 || pin >= static_cast<int_fast16_t>(NUM_BANK0_GPIOS))
    {
      return;
    }
    lgfx_gpio_init(pin);
    lgfx_gpio_mode(pin, mode);
    lgfx_gpio_set_dir(pin, mode == pin_mode_t::output);
  }

  void digitalWrite(int_fast16_t pin, PinStatus val)
  {
    lgfx_gpio_put(pin, val == HIGH);
  }

  bool digitalRead(int_fast16_t pin)
  {
    return lgfx_gpio_get(static_cast<uint>(pin));
  }

//----------------------------------------------------------------------------

  namespace spi
  {
    namespace
    {
      struct spi_info_str {
        uint32_t ref_count { 0 };
        int pin_sclk { -1 };
        int pin_miso { -1 };
        int pin_mosi { -1 };
      };

      volatile spi_hw_t * const spi_dev[] = {
        reinterpret_cast<volatile spi_hw_t *>(SPI0_BASE),
        reinterpret_cast<volatile spi_hw_t *>(SPI1_BASE),
      };
      constexpr int n_spi = std::extent<decltype(spi_dev), 0>::value;

      // RP2040 Dataheetの 1.4.3. GPIO Functions Table 2を参照
      constexpr uint8_t spi0_sclk_pinlist[] = {  2,  6, 18, 23, UINT8_MAX };
      constexpr uint8_t spi0_miso_pinlist[] = {  0,  4, 16, 20, UINT8_MAX };
      constexpr uint8_t spi0_mosi_pinlist[] = {  3,  7, 19, 23, UINT8_MAX };
      constexpr uint8_t spi1_sclk_pinlist[] = { 10, 14, 26,     UINT8_MAX };
      constexpr uint8_t spi1_miso_pinlist[] = {  8, 12, 24, 28, UINT8_MAX };
      constexpr uint8_t spi1_mosi_pinlist[] = { 11, 15, 27,     UINT8_MAX };

      constexpr uint32_t PRESCALE_ERROR = UINT32_MAX;

      //
      constexpr struct spi_pinlist_str {
        const uint8_t *sclk_pinlist;
        const uint8_t *miso_pinlist;
        const uint8_t *mosi_pinlist;
      } spi_pinlist[] = {
        {
          .sclk_pinlist = spi0_sclk_pinlist,
          .miso_pinlist = spi0_miso_pinlist,
          .mosi_pinlist = spi0_mosi_pinlist,
        },
        {
          .sclk_pinlist = spi1_sclk_pinlist,
          .miso_pinlist = spi1_miso_pinlist,
          .mosi_pinlist = spi1_mosi_pinlist,
        },
      };

      spi_info_str spi_info[n_spi];
    }

//----------------------------------------------------------------------------

    uint32_t FreqToClockDiv(uint32_t hz)
    {
      uint32_t fapb = clock_get_hz(clk_peri) >> 1;
      if (fapb <= hz) return 0;
      uint32_t div_num = fapb / (1 + hz);
      if (div_num > 255) div_num = 255;
      return div_num;
    }
    namespace
    {

      __attribute__ ((always_inline)) inline void lgfx_spi_reset(volatile spi_hw_t * spi_regs)
      {
        lgfx_reset_block((spi_regs == spi_dev[0]) ? RESETS_RESET_SPI0_BITS : RESETS_RESET_SPI1_BITS);
      }

      __attribute__ ((always_inline)) inline void lgfx_spi_unreset(volatile spi_hw_t * spi_regs)
      {
        lgfx_unreset_block_wait((spi_regs == spi_dev[0]) ? RESETS_RESET_SPI0_BITS : RESETS_RESET_SPI1_BITS);
      }

      bool lgfx_spi_set_baudrate(volatile spi_hw_t * spi_regs, uint32_t baudrate)
      {
        spi_regs->cpsr = 2;  // prescale
        uint32_t div = FreqToClockDiv(baudrate);
        uint32_t temp = spi_regs->cr0;
        temp &= ~SPI_SSPCR0_SCR_BITS;
        temp |= (div << SPI_SSPCR0_SCR_LSB);
        spi_regs->cr0 = temp;
        return true;
      }

      __attribute__ ((always_inline)) inline void lgfx_spi_set_format(volatile spi_hw_t * spi_regs, uint data_bits, spi_cpol_t cpol, spi_cpha_t cpha, [[maybe_unused]]spi_order_t order)
      {
        uint32_t temp = spi_regs->cr0;
        temp &= ~(SPI_SSPCR0_DSS_BITS | SPI_SSPCR0_SPO_BITS | SPI_SSPCR0_SPH_BITS);
        temp |= (data_bits - 1) << SPI_SSPCR0_DSS_LSB |
          cpol << SPI_SSPCR0_SPO_LSB |
          cpha << SPI_SSPCR0_SPH_LSB;
        spi_regs->cr0 = temp;
      }

      bool lgfx_spi_set_pin_function(int mosi, int miso, int sclk)
      {
        if ((mosi != -1) && (!lgfx_gpio_set_function(mosi, GPIO_FUNC_SPI))) { return false; }
        if ((sclk != -1) && (!lgfx_gpio_set_function(sclk, GPIO_FUNC_SPI))) { return false; }
        if ((miso != -1) && (!lgfx_gpio_set_function(miso, GPIO_FUNC_SPI))) { return false; }
        return true;
      }

      bool lgfx_spi_init(volatile spi_hw_t * spi_regs)
      {
        lgfx_spi_reset(spi_regs);
        lgfx_spi_unreset(spi_regs);
        // SPIのクロック周波数を設定
        if (!lgfx_spi_set_baudrate(spi_regs, 1000000))
        {
          return false;
        }
        lgfx_spi_set_format(spi_regs, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

        spi_regs->dmacr |= (SPI_SSPDMACR_TXDMAE_BITS | SPI_SSPDMACR_RXDMAE_BITS);
        // SPI有効化
        spi_regs->cr1 |= SPI_SSPCR1_SSE_BITS;
        DBGPRINT("cr1 %08x\n", spi_regs->cr1);
        return true;
      }

      void lgfx_spi_deinit(volatile spi_hw_t * spi_regs)
      {
        spi_regs->cr1 &= ~SPI_SSPCR1_SSE_BITS;
        spi_regs->dmacr &= ~(SPI_SSPDMACR_TXDMAE_BITS | SPI_SSPDMACR_RXDMAE_BITS);
        lgfx_spi_reset(spi_regs);
      }
    }

    static std::pair<cpp::result<void, error_t>, bool> check_pin_update(int current_pin, int new_pin)
    {
      bool need_update = false;
      cpp::result<void, error_t> result = {};
      if (new_pin != -1)
      {
        if (current_pin == -1)
        {
          need_update = true;
        }
        else if (current_pin != new_pin)
        {
          result = cpp::fail(error_t::invalid_arg);
        }
      }
      return { result, need_update };
    }

//----------------------------------------------------------------------------

    cpp::result<void, error_t> init(int spi_port, int pin_sclk, int pin_miso, int pin_mosi)
    {
      DBGPRINT("enter %s\n", __func__);
      if (spi_port < 0 || spi_port >= n_spi)
      {
        return cpp::fail(error_t::invalid_arg);
      }
      // DBGPRINT("ref_count %d\n", spi_info[spi_port].ref_count); 
      if (spi_info[spi_port].ref_count == 0)
      {
        const spi_pinlist_str pinlist = spi_pinlist[spi_port];
        if (!lgfx::v1::rp2040::pin_check(pin_sclk, pinlist.sclk_pinlist))
        {
          return cpp::fail(error_t::invalid_arg);
        }
        // pin_misoは未使用(-1)でもOKとする
        if (pin_miso != -1 && !lgfx::v1::rp2040::pin_check(pin_miso, pinlist.miso_pinlist))
        {
          return cpp::fail(error_t::invalid_arg);
        }
        // pin_mosiは未使用(-1)でもOKとする
        if (pin_mosi != -1 && !lgfx::v1::rp2040::pin_check(pin_mosi, pinlist.mosi_pinlist))
        {
          return cpp::fail(error_t::invalid_arg);
        }
        lgfx_spi_set_pin_function(pin_mosi, pin_miso, pin_sclk);
        spi_info[spi_port].pin_sclk = pin_sclk;
        spi_info[spi_port].pin_miso = pin_miso;
        spi_info[spi_port].pin_mosi = pin_mosi;
        if (!lgfx_spi_init(spi_dev[spi_port]))
        {
          return cpp::fail(error_t::invalid_arg);
        }
        spi_info[spi_port].ref_count = 1;
        DBGPRINT("return %s\n", __func__);
        return {};
      }
      // 
      if (spi_info[spi_port].pin_sclk != pin_sclk)
      {
        return cpp::fail(error_t::invalid_arg);
      }
      //
      auto rc = check_pin_update(spi_info[spi_port].pin_miso, pin_miso);
      auto result_miso = rc.first;
      auto need_update_miso = rc.second;
      if (result_miso.has_error())
      {
        return result_miso;
      }
      if (need_update_miso)
      {
        spi_info[spi_port].pin_miso = pin_miso;
      }
      //
      rc = check_pin_update(spi_info[spi_port].pin_mosi, pin_mosi);
      auto result_mosi = rc.first;
      auto need_update_mosi = rc.second;
      if (result_mosi.has_error())
      {
        return result_mosi;
      }
      if (need_update_mosi)
      {
        spi_info[spi_port].pin_mosi = pin_mosi;
      }
      if (need_update_miso || need_update_mosi)
      {
        lgfx_spi_set_pin_function(spi_info[spi_port].pin_mosi,
          spi_info[spi_port].pin_miso, spi_info[spi_port].pin_sclk);
      }
      spi_info[spi_port].ref_count++;
      DBGPRINT("return %s\n", __func__);
      return {};
    }

    void release(int spi_port)
    {
      if (spi_info[spi_port].ref_count == 0)
      {
        return;
      }
      spi_info[spi_port].ref_count--;
      if (spi_info[spi_port].ref_count == 0)
      {
        spi_info[spi_port].pin_sclk = -1;
        spi_info[spi_port].pin_miso = -1;
        spi_info[spi_port].pin_mosi = -1;
        lgfx_spi_deinit(spi_dev[spi_port]);
      }
    }

    bool lgfx_spi_set_frequency(int spi_port, uint32_t baudrate)
    {
      return lgfx_spi_set_baudrate(spi_dev[spi_port], baudrate);
    }

    void beginTransaction(int spi_port, uint32_t freq, int spi_mode)
    {
      static constexpr spi_cpha_t cpha_table[] = { SPI_CPHA_0, SPI_CPHA_1, SPI_CPHA_0, SPI_CPHA_1 };
      static constexpr spi_cpol_t cpol_table[] = { SPI_CPOL_0, SPI_CPOL_0, SPI_CPOL_1, SPI_CPOL_1 };
      if (spi_mode < 0 || spi_mode > 3)
      {
        spi_mode = 0;
      }
      lgfx_spi_set_format(spi_dev[spi_port], 8, cpol_table[spi_mode], cpha_table[spi_mode], SPI_MSB_FIRST);
      lgfx_spi_set_baudrate(spi_dev[spi_port], freq);
    }

    void endTransaction([[maybe_unused]]int spi_port)
    {

    }
  }

//----------------------------------------------------------------------------

  // 実装中
  namespace i2c
  {
    namespace
    {
      struct i2c_info_str {
        uint32_t ref_count { 0 };
        int pin_sda { -1 };
        int pin_scl { -1 };
        uint8_t last_data;
        bool last_data_valid { false };
        bool is_restart { false };
      };

      volatile i2c_hw_t * const i2c_dev[] = {
        reinterpret_cast<volatile i2c_hw_t *>(I2C0_BASE),
        reinterpret_cast<volatile i2c_hw_t *>(I2C1_BASE),
      };
      constexpr int n_i2c = std::extent<decltype(i2c_dev), 0>::value;

      // RP2040 Dataheetの 1.4.3. GPIO Functions Table 2を参照
      constexpr uint8_t i2c0_sda_pinlist[] = {  0,  4,  8, 12, 16, 20, 24, 28, UINT8_MAX };
      constexpr uint8_t i2c0_sck_pinlist[] = {  1,  5,  9, 13, 17, 21, 25, 29, UINT8_MAX };
      constexpr uint8_t i2c1_sda_pinlist[] = {  2,  6, 10, 14, 18, 22, 26,     UINT8_MAX };
      constexpr uint8_t i2c1_sck_pinlist[] = {  3,  7, 11, 15, 19, 23, 27,     UINT8_MAX };

      constexpr struct i2c_pinlist_str {
        const uint8_t *sda_pinlist;
        const uint8_t *scl_pinlist;
      } i2c_pinlist[] = {
        {
          .sda_pinlist = i2c0_sda_pinlist,
          .scl_pinlist = i2c0_sck_pinlist,
        },
        {
          .sda_pinlist = i2c1_sda_pinlist,
          .scl_pinlist = i2c1_sck_pinlist,
        },
      };

      i2c_info_str i2c_info[n_i2c];
      std::unordered_map<uint32_t, std::array<uint32_t, 3>> prescale_map;
    }

    //
    namespace
    {
      __attribute__ ((always_inline)) inline bool is_master_not_idle(volatile i2c_hw_t * i2c_regs)
      {
        return ((i2c_regs->status & I2C_IC_STATUS_MST_ACTIVITY_BITS) != 0);
      }

      __attribute__ ((always_inline)) inline bool is_rx_fifo_full(volatile i2c_hw_t * i2c_regs)
      {
        return ((i2c_regs->status & I2C_IC_STATUS_RFF_BITS) != 0);
      }

      __attribute__ ((always_inline)) inline bool is_rx_fifo_not_empty(volatile i2c_hw_t * i2c_regs)
      {
        return ((i2c_regs->status & I2C_IC_STATUS_RFNE_BITS) != 0);
      }

      __attribute__ ((always_inline)) inline bool is_tx_fifo_empty(volatile i2c_hw_t * i2c_regs)
      {
        return ((i2c_regs->status & I2C_IC_STATUS_TFE_BITS) != 0);
      }

      __attribute__ ((always_inline)) inline bool is_tx_fifo_not_full(volatile i2c_hw_t * i2c_regs)
      {
        return ((i2c_regs->status & I2C_IC_STATUS_TFNF_BITS) != 0);
      }

      __attribute__ ((always_inline)) inline void wait_complete(volatile i2c_hw_t * i2c_regs)
      {
        while (is_master_not_idle(i2c_regs))
        {
          ;
        }
      }

      __attribute__ ((always_inline)) inline void send_data(volatile i2c_hw_t * i2c_regs, uint8_t data, bool is_restart, bool is_stop)
      {
        i2c_regs->data_cmd = (is_restart ? I2C_IC_DATA_CMD_RESTART_BITS : 0x0) |
                             (is_stop ? I2C_IC_DATA_CMD_STOP_BITS : 0x0) |
                             static_cast<uint32_t>(data);
      }      

      __attribute__ ((always_inline)) inline uint8_t recv_data(volatile i2c_hw_t * i2c_regs, bool is_restart, bool is_stop)
      {
        i2c_regs->data_cmd = (is_restart ? I2C_IC_DATA_CMD_RESTART_BITS : 0x0) |
                             (is_stop ? I2C_IC_DATA_CMD_STOP_BITS : 0x0) |
                             I2C_IC_DATA_CMD_CMD_BITS;
        return static_cast<uint8_t>(i2c_regs->data_cmd);
      }
      // 送信先のアドレスを指定する。
      __attribute__ ((always_inline)) inline void set_target_address(volatile i2c_hw_t * i2c_regs, int addr)
      {
        i2c_regs->enable = I2C_IC_ENABLE_ENABLE_VALUE_DISABLED;
        i2c_regs->tar = addr & I2C_IC_TAR_IC_TAR_BITS;
        i2c_regs->enable = I2C_IC_ENABLE_ENABLE_VALUE_ENABLED;
      }

      //

      __attribute__ ((always_inline)) inline void lgfx_i2c_reset(volatile i2c_hw_t *i2c_regs) {
        lgfx_reset_block(i2c_regs == i2c_dev[0] ? RESETS_RESET_I2C0_BITS : RESETS_RESET_I2C1_BITS);
      }

      __attribute__ ((always_inline)) inline void lgfx_i2c_unreset(volatile i2c_hw_t *i2c_regs) {
        lgfx_unreset_block_wait(i2c_regs == i2c_dev[0] ? RESETS_RESET_I2C0_BITS : RESETS_RESET_I2C1_BITS);
      }

      bool lgfx_i2c_set_baudrate(volatile i2c_hw_t * i2c_regs, uint32_t baudrate)
      {
        uint32_t hcnt;
        uint32_t lcnt;
        uint32_t sda_tx_hold_count;
        auto val = prescale_map.find(baudrate);
        if (val != prescale_map.end())
        {
          hcnt = val->second[0];
          lcnt = val->second[1];
          sda_tx_hold_count = val->second[2];
        }
        else
        {
          if (baudrate == 0)
          {
            return false;
          }
          uint32_t freq_in = clock_get_hz(clk_sys);
          uint32_t period = (freq_in + baudrate / 2) / baudrate;
          lcnt = period * 3 / 5;
          hcnt = period - lcnt;
          if (hcnt > I2C_IC_FS_SCL_HCNT_IC_FS_SCL_HCNT_BITS)
          {
            return false;
          }
          if (lcnt > I2C_IC_FS_SCL_LCNT_IC_FS_SCL_LCNT_BITS)
          {
            return false;
          }
          if (hcnt < 8)
          {
            return false;
          }
          if (lcnt < 8)
          {
            return false;
          }
          if (baudrate < 1000000)
          {
            sda_tx_hold_count = ((freq_in * 3) / 10000000) + 1;
          }
          else
          {
            sda_tx_hold_count = ((freq_in * 3) / 25000000) + 1;
          }
          if (sda_tx_hold_count <= lcnt - 2)
          {
            return false;
          }
          prescale_map.emplace(baudrate, decltype(prescale_map)::mapped_type{ hcnt, lcnt, sda_tx_hold_count });
        }
        i2c_regs->enable = I2C_IC_ENABLE_ENABLE_VALUE_DISABLED;
        uint32_t temp = i2c_regs->con;
        temp &= ~(I2C_IC_CON_SPEED_BITS);
        temp |= (I2C_IC_CON_SPEED_VALUE_FAST << I2C_IC_CON_SPEED_LSB);
        i2c_regs->con = temp;
        i2c_regs->fs_scl_hcnt = hcnt;
        i2c_regs->fs_scl_lcnt = lcnt;
        i2c_regs->fs_spklen = (lcnt < 16) ? 1 : lcnt / 16;
        temp = i2c_regs->sda_hold;
        temp &= ~(I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_BITS);
        temp |= (sda_tx_hold_count << I2C_IC_SDA_HOLD_IC_SDA_TX_HOLD_LSB);
        i2c_regs->sda_hold = temp;
        i2c_regs->enable = I2C_IC_ENABLE_ENABLE_VALUE_ENABLED;
        return true;
      }

      bool lgfx_i2c_init(volatile i2c_hw_t * i2c_regs, uint32_t baudrate)
      {
        lgfx_i2c_reset(i2c_regs);
        lgfx_i2c_unreset(i2c_regs);

        i2c_regs->enable = I2C_IC_ENABLE_ENABLE_VALUE_DISABLED;

        i2c_regs->con = 
          I2C_IC_CON_SPEED_VALUE_FAST << I2C_IC_CON_SPEED_LSB |
          I2C_IC_CON_MASTER_MODE_BITS |
          I2C_IC_CON_IC_SLAVE_DISABLE_BITS |
          I2C_IC_CON_IC_RESTART_EN_BITS |
          I2C_IC_CON_TX_EMPTY_CTRL_BITS;

        i2c_regs->tx_tl = 0;
        i2c_regs->rx_tl = 0;
        i2c_regs->dma_cr = I2C_IC_DMA_CR_TDMAE_BITS | I2C_IC_DMA_CR_RDMAE_BITS;
        return lgfx_i2c_set_baudrate(i2c_regs, baudrate);
      }

      void lgfx_i2c_deinit(volatile i2c_hw_t * i2c_regs)
      {
        i2c_regs->enable = I2C_IC_ENABLE_ENABLE_VALUE_DISABLED;
        lgfx_i2c_reset(i2c_regs);
      }
    }

    cpp::result<void, error_t> init(int i2c_port, int pin_sda, int pin_scl)
    {
      if (i2c_port < 0 || i2c_port >= n_i2c)
      {
        return cpp::fail(error_t::invalid_arg);
      }
      if (i2c_info[i2c_port].ref_count == 0)
      {
        const struct i2c_pinlist_str pinlist = i2c_pinlist[i2c_port];
        if (!lgfx::v1::rp2040::pin_check(pin_sda, pinlist.sda_pinlist))
        {
          return cpp::fail(error_t::invalid_arg);
        }
        if (!lgfx::v1::rp2040::pin_check(pin_scl, pinlist.scl_pinlist))
        {
          return cpp::fail(error_t::invalid_arg);
        }
        lgfx_gpio_set_function(pin_sda, GPIO_FUNC_I2C);
        lgfx_gpio_set_function(pin_scl, GPIO_FUNC_I2C);
        if (!lgfx_i2c_init(i2c_dev[i2c_port], 1000000))
        {
          return cpp::fail(error_t::invalid_arg);
        }
        i2c_info[i2c_port].pin_sda = pin_sda;
        i2c_info[i2c_port].pin_scl = pin_scl;
        i2c_info[i2c_port].ref_count = 1;
        return {};
      }
      if (i2c_info[i2c_port].pin_sda != pin_sda)
      {
        return cpp::fail(error_t::invalid_arg);
      }
      if (i2c_info[i2c_port].pin_scl != pin_scl)
      {
        return cpp::fail(error_t::invalid_arg);
      }
      i2c_info[i2c_port].ref_count++;
      return {};
    }

    cpp::result<void, error_t> release(int i2c_port)
    {
      if (i2c_info[i2c_port].ref_count == 0)
      {
        return {};
      }
      i2c_info[i2c_port].ref_count--;
      if (i2c_info[i2c_port].ref_count == 0)
      {
        i2c_info[i2c_port].pin_sda = -1;
        i2c_info[i2c_port].pin_scl = -1;
        lgfx_i2c_deinit(i2c_dev[i2c_port]);
      }
      return {};
    }

    cpp::result<void, error_t> restart(int i2c_port, int i2c_addr, uint32_t freq, [[maybe_unused]]bool read)
    {
       volatile i2c_hw_t * i2c_regs = i2c_dev[i2c_port];

      auto info = &i2c_info[i2c_port];
      // 直前のトランザクションの最後のデータを送信
      if (info->last_data_valid)
      {
        while (!is_tx_fifo_not_full(i2c_regs))
        {
        }
        send_data(i2c_regs, info->last_data, info->is_restart, false);
        info->last_data_valid = false;
        wait_complete(i2c_regs);
      }
      lgfx_i2c_set_baudrate(i2c_dev[i2c_port], freq);
      set_target_address(i2c_regs, i2c_addr);
      info->is_restart = true;
      return {};
    }

    cpp::result<void, error_t> beginTransaction(int i2c_port, int i2c_addr, uint32_t freq, [[maybe_unused]]bool read)
    {
      volatile i2c_hw_t * i2c_regs = i2c_dev[i2c_port];
      auto info = &i2c_info[i2c_port];
      if (info->last_data_valid)
      {
        send_data(i2c_regs, info->last_data, false, false);
        info->last_data_valid = false;
        wait_complete(i2c_regs);
      }
      lgfx_i2c_set_baudrate(i2c_dev[i2c_port], freq);
      set_target_address(i2c_regs, i2c_addr);
      info->is_restart = false;
      return {};
    }

    cpp::result<void, error_t> endTransaction(int i2c_port)
    {
      volatile i2c_hw_t * i2c_regs = i2c_dev[i2c_port];
      auto info = &i2c_info[i2c_port];
      if (info->last_data_valid)
      {
        while (!is_tx_fifo_not_full(i2c_regs))
        {
        }
        send_data(i2c_regs, info->last_data, false, true);
        info->last_data_valid = false;
      }
      return {};
    }

    cpp::result<void, error_t> writeBytes(int i2c_port, [[maybe_unused]]const uint8_t *data, size_t length)
    {
      volatile i2c_hw_t * i2c_regs = i2c_dev[i2c_port];
      auto info = &i2c_info[i2c_port];
      if (length == 0)
      {
        return {};
      }
      if (length == 1)
      {
        info->last_data = *data;
        info->last_data_valid = true;
        return {};
      }
      length--;
      for (decltype(length) i = 0; i < length; i++)
      {
        while (!is_tx_fifo_not_full(i2c_regs))
        {
        }
        send_data(i2c_regs, *data++, (i == 0 && info->is_restart), false);
      }
      // 最後のデータを保存
      info->is_restart = false;
      info->last_data = *data;
      info->last_data_valid = true;
      return {};
    }

    cpp::result<void, error_t> readBytes(int i2c_port, uint8_t *data, size_t length)
    {
      volatile i2c_hw_t * i2c_regs = i2c_dev[i2c_port];
      auto info = &i2c_info[i2c_port];
      if (length == 0)
      {
        return {};
      }
      for (decltype(length) i = 0; i < length; i++)
      {
        while (!is_rx_fifo_not_empty(i2c_regs))
        {
        }
        *data++ = recv_data(i2c_regs, ((i == 0) && info->is_restart), (i == length - 1));
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
      if (mask != 0)
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
