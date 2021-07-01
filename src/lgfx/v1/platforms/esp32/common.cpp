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
#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (CONFIG_IDF_TARGET_ESP32S2) || defined (ESP_PLATFORM)

#include "../common.hpp"

#include <algorithm>
#include <cstring>

#include <driver/i2c.h>
#include <driver/spi_common.h>
#include <driver/rtc_io.h>
#include <driver/periph_ctrl.h>
#include <soc/rtc.h>
#include <soc/i2c_reg.h>
#include <soc/i2c_struct.h>

#if defined ( ARDUINO )
 #include <SPI.h>
 #include <Wire.h>
 #include <soc/dport_reg.h>
#else
 #include <driver/spi_master.h>
 #include <esp_log.h>
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  std::uint32_t getApbFrequency(void)
  {
    rtc_cpu_freq_config_t conf;
    rtc_clk_cpu_freq_get_config(&conf);
    if (conf.freq_mhz >= 80){
      return 80 * 1000000;
    }
    return (conf.source_freq_mhz * 1000000) / conf.div;
  }

  std::uint32_t FreqToClockDiv(std::uint32_t fapb, std::uint32_t hz)
  {
    if (fapb <= hz) return SPI_CLK_EQU_SYSCLK;
    std::uint32_t div_num = fapb / (1 + hz);
    std::uint32_t pre = div_num / 64u;
    div_num = div_num / (pre+1);
    return div_num << 12 | ((div_num-1)>>1) << 6 | div_num | pre << 18;
  }

//----------------------------------------------------------------------------

  void pinMode(std::int_fast16_t pin, pin_mode_t mode)
  {
    if (pin < 0) return;
    if (pin < 6 || pin > 11) {
      gpio_set_direction((gpio_num_t)pin, GPIO_MODE_DISABLE);
    }
#if defined (ARDUINO)
    int m;
    switch (mode)
    {
    case pin_mode_t::output:         m = OUTPUT;         break;
    default:
    case pin_mode_t::input:          m = INPUT;          break;
    case pin_mode_t::input_pullup:   m = INPUT_PULLUP;   break;
    case pin_mode_t::input_pulldown: m = INPUT_PULLDOWN; break;
    }
    ::pinMode(pin, m);
#else
    if (rtc_gpio_is_valid_gpio((gpio_num_t)pin)) rtc_gpio_deinit((gpio_num_t)pin);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (std::uint64_t)1 << pin;
    switch (mode)
    {
    case pin_mode_t::output:
      io_conf.mode = GPIO_MODE_OUTPUT;
      break;
    default:
      io_conf.mode = GPIO_MODE_INPUT;
      break;
    }
    io_conf.mode         = (mode == pin_mode_t::output) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;
    io_conf.pull_down_en = (mode == pin_mode_t::input_pulldown) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en   = (mode == pin_mode_t::input_pullup  ) ? GPIO_PULLUP_ENABLE   : GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
#endif
  }

//----------------------------------------------------------------------------

  namespace spi
  {
#if defined ( ARDUINO )
    static spi_t* _spi_handle[VSPI_HOST + 1] = {nullptr};
#else // ESP-IDF
    static spi_device_handle_t _spi_handle[SOC_SPI_PERIPH_NUM] = {nullptr};
#endif

    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi)
    {
      return init(spi_host, spi_sclk, spi_miso, spi_mosi, 0);
    }

    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi, int dma_channel)
    {
//ESP_LOGI("LGFX","spi::init host:%d, sclk:%d, miso:%d, mosi:%d, dma:%d", spi_host, spi_sclk, spi_miso, spi_mosi, dma_channel);
      std::uint32_t spi_port = (spi_host + 1);

#if defined (ARDUINO) // Arduino ESP32
      if (spi_host == VSPI_HOST)
      {
        SPI.end();
        SPI.begin(spi_sclk, spi_miso, spi_mosi);
        _spi_handle[spi_host] = SPI.bus();
      }

      if (_spi_handle[spi_host] == nullptr) {
        _spi_handle[spi_host] = spiStartBus(spi_port, SPI_CLK_EQU_SYSCLK, 0, 0);
      }
      periph_module_enable(spi_periph_signal[spi_host].module);
      if (spi_mosi >= 0) {
        gpio_lo(spi_mosi);
        pinMode(spi_mosi, pin_mode_t::output);
        gpio_matrix_out(spi_mosi, spi_periph_signal[spi_host].spid_out, false, false);
        gpio_matrix_in(spi_mosi, spi_periph_signal[spi_host].spid_in, false);
      }
      if (spi_miso >= 0) {
        pinMode(spi_miso, pin_mode_t::input);
      //gpio_matrix_out(spi_miso, spi_periph_signal[spi_host].spiq_out, false, false);
        gpio_matrix_in(spi_miso, spi_periph_signal[spi_host].spiq_in, false);
      }
      if (spi_sclk >= 0) {
        gpio_lo(spi_sclk); // ここでLOWにしておくことで、pinMode変更によるHIGHパルスが出力されるのを防止する (CSなしパネル対策)
        pinMode(spi_sclk, pin_mode_t::output);
        //gpio_set_direction((gpio_num_t)_spi_sclk, GPIO_MODE_INPUT_OUTPUT);
        gpio_matrix_out(spi_sclk, spi_periph_signal[spi_host].spiclk_out, false, false);
        gpio_matrix_in(spi_sclk, spi_periph_signal[spi_host].spiclk_in, false);
      }
      if (dma_channel) {
        periph_module_enable( PERIPH_SPI_DMA_MODULE );
    //Select DMA channel.
        DPORT_SET_PERI_REG_BITS(DPORT_SPI_DMA_CHAN_SEL_REG, 3, dma_channel, (spi_host * 2));
      //Reset DMA
        WRITE_PERI_REG(SPI_DMA_CONF_REG(spi_port), READ_PERI_REG(SPI_DMA_CONF_REG(spi_port)) | SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST);
        WRITE_PERI_REG(SPI_DMA_IN_LINK_REG(spi_port), 0);
        WRITE_PERI_REG(SPI_DMA_OUT_LINK_REG(spi_port), 0);
        WRITE_PERI_REG(SPI_DMA_CONF_REG(spi_port), READ_PERI_REG(SPI_DMA_CONF_REG(spi_port)) & ~(SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST));
      }

      WRITE_PERI_REG(SPI_USER_REG (spi_port), SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN);  // need SD card access (full duplex setting)
      WRITE_PERI_REG(SPI_CTRL_REG( spi_port), 0);
      WRITE_PERI_REG(SPI_CTRL2_REG(spi_port), 0);
      WRITE_PERI_REG(SPI_SLAVE_REG(spi_port), READ_PERI_REG(SPI_SLAVE_REG(spi_port)) & ~(SPI_SLAVE_MODE | SPI_TRANS_DONE));

#else // ESP-IDF

      spi_bus_config_t buscfg = {
          .mosi_io_num = spi_mosi,
          .miso_io_num = spi_miso,
          .sclk_io_num = spi_sclk,
          .quadwp_io_num = -1,
          .quadhd_io_num = -1,
          .max_transfer_sz = 1,
          .flags = SPICOMMON_BUSFLAG_MASTER,
          .intr_flags = 0,
      };

      if (_spi_handle[spi_host] == nullptr)
      {
        spi_bus_initialize(static_cast<spi_host_device_t>(spi_host), &buscfg, dma_channel);

        spi_device_interface_config_t devcfg = {
            .command_bits = 0,
            .address_bits = 0,
            .dummy_bits = 0,
            .mode = 0,
            .duty_cycle_pos = 0,
            .cs_ena_pretrans = 0,
            .cs_ena_posttrans = 0,
            .clock_speed_hz = (int)getApbFrequency()>>1,
            .input_delay_ns = 0,
            .spics_io_num = -1,
            .flags = SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX,
            .queue_size = 1,
            .pre_cb = nullptr,
            .post_cb = nullptr};
        if (ESP_OK != spi_bus_add_device(static_cast<spi_host_device_t>(spi_host), &devcfg, &_spi_handle[spi_host])) {
          ESP_LOGE("LGFX", "Failed to spi_bus_add_device. ");
        }
      }

#endif
      WRITE_PERI_REG(SPI_CTRL1_REG(spi_port), 0);

      return {};
    }

    void release(int spi_host)
    {
//ESP_LOGI("LGFX","spi::release");
      if (_spi_handle[spi_host] != nullptr) {
#if defined (ARDUINO) // Arduino ESP32
        if (spi_host == VSPI_HOST)
        {
          SPI.end();
        }
        else
        {
          spiStopBus(_spi_handle[spi_host]);
        }
#else // ESP-IDF
        spi_bus_remove_device(_spi_handle[spi_host]);
        spi_bus_free(static_cast<spi_host_device_t>(spi_host));
#endif
        _spi_handle[spi_host] = nullptr;
      }
    }

    void beginTransaction(int spi_host)
    {
#if defined (ARDUINO) // Arduino ESP32
      spiSimpleTransaction(_spi_handle[spi_host]);
#else // ESP-IDF
      if (_spi_handle[spi_host]) {
        if (ESP_OK != spi_device_acquire_bus(_spi_handle[spi_host], portMAX_DELAY)) {
          ESP_LOGE("LGFX", "Failed to spi_device_acquire_bus. ");
        }
      }
#endif
    }

    void beginTransaction(int spi_host, std::uint32_t freq, int spi_mode)
    {
      std::uint32_t spi_port = (spi_host + 1);
      std::uint32_t clkdiv = FreqToClockDiv(getApbFrequency(), freq);

      std::uint32_t user = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN;
      if (spi_mode == 1 || spi_mode == 2) user |= SPI_CK_OUT_EDGE;
      std::uint32_t pin = 0;
      if (spi_mode & 2) pin = SPI_CK_IDLE_EDGE;

      beginTransaction(spi_host);

      WRITE_PERI_REG(SPI_USER_REG(spi_port), user);
#if defined (SPI_PIN_REG)
      WRITE_PERI_REG(SPI_PIN_REG( spi_port), pin);
#else
      WRITE_PERI_REG(SPI_MISC_REG( spi_port), pin);
#endif
      WRITE_PERI_REG(SPI_CLOCK_REG(spi_port), clkdiv);
      //gpio_lo(spi_cs);
    }

    void endTransaction(int spi_host)
    {
      if (_spi_handle[spi_host]) {
#if defined (ARDUINO) // Arduino ESP32
        spiEndTransaction(_spi_handle[spi_host]);
#else // ESP-IDF
        spi_device_release_bus(_spi_handle[spi_host]);
#endif
      }
    }

    void endTransaction(int spi_host, int spi_cs)
    {
      if (_spi_handle[spi_host]) {
#if defined (ARDUINO) // Arduino ESP32
        spiEndTransaction(_spi_handle[spi_host]);
#else // ESP-IDF
        spi_device_release_bus(_spi_handle[spi_host]);
#endif
      }
      gpio_hi(spi_cs);
    }

    void writeBytes(int spi_host, const std::uint8_t* data, std::size_t len)
    {
      std::uint32_t spi_port = (spi_host + 1);
      if (len > 64) len = 64;
      memcpy(reinterpret_cast<void*>(SPI_W0_REG(spi_port)), data, len);
      WRITE_PERI_REG(SPI_MOSI_DLEN_REG(spi_port), (len << 3) - 1);
      WRITE_PERI_REG(SPI_CMD_REG(      spi_port), SPI_USR);
      while (READ_PERI_REG(SPI_CMD_REG(spi_port)) & SPI_USR);
    }

    void readBytes(int spi_host, std::uint8_t* data, std::size_t len)
    {
      std::uint32_t spi_port = (spi_host + 1);
      if (len > 64) len = 64;
      memcpy(reinterpret_cast<void*>(SPI_W0_REG(spi_port)), data, len);
      WRITE_PERI_REG(SPI_MOSI_DLEN_REG(spi_port), (len << 3) - 1);
      WRITE_PERI_REG(SPI_CMD_REG(      spi_port), SPI_USR);
      while (READ_PERI_REG(SPI_CMD_REG(spi_port)) & SPI_USR);

      memcpy(data, reinterpret_cast<const void*>(SPI_W0_REG(spi_port)), len);
    }
  }

//----------------------------------------------------------------------------

  namespace i2c
  {
    static constexpr int I2C_7BIT_ADDR_MIN = 0x08;
    static constexpr int I2C_7BIT_ADDR_MAX = 0x77;
    static constexpr int I2C_10BIT_ADDR_MAX = 1023;
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
      gpio_num_t pin_scl = (gpio_num_t)-1;
      gpio_num_t pin_sda = (gpio_num_t)-1;
      std::uint32_t freq = 0;

      void save_reg(i2c_dev_t* dev)
      {
        scl_high_period  = dev->scl_high_period.val ;
        scl_low_period   = dev->scl_low_period.val  ;
        scl_start_hold   = dev->scl_start_hold.val  ;
        scl_rstart_setup = dev->scl_rstart_setup.val;
        scl_stop_hold    = dev->scl_stop_hold.val   ;
        scl_stop_setup   = dev->scl_stop_setup.val  ;
        sda_hold         = dev->sda_hold.val        ;
        sda_sample       = dev->sda_sample.val      ;
        scl_filter       = dev->scl_filter_cfg.val  ;
        sda_filter       = dev->sda_filter_cfg.val  ;
        fifo_conf        = dev->fifo_conf.val       ;
        timeout          = dev->timeout.val         ;
      }

      void load_reg(i2c_dev_t* dev)
      {
        dev->scl_high_period.val  = scl_high_period ;
        dev->scl_low_period.val   = scl_low_period  ;
        dev->scl_start_hold.val   = scl_start_hold  ;
        dev->scl_rstart_setup.val = scl_rstart_setup;
        dev->scl_stop_hold.val    = scl_stop_hold   ;
        dev->scl_stop_setup.val   = scl_stop_setup  ;
        dev->sda_hold.val         = sda_hold        ;
        dev->sda_sample.val       = sda_sample      ;
        dev->scl_filter_cfg.val   = scl_filter      ;
        dev->sda_filter_cfg.val   = sda_filter      ;
        dev->fifo_conf.val        = fifo_conf       ;
        dev->timeout.val          = timeout         ;
      }

    private:
      std::uint32_t scl_high_period;
      std::uint32_t scl_low_period;
      std::uint32_t scl_start_hold;
      std::uint32_t scl_rstart_setup;
      std::uint32_t scl_stop_hold;
      std::uint32_t scl_stop_setup;
      std::uint32_t sda_hold;
      std::uint32_t sda_sample;
      std::uint32_t scl_filter;
      std::uint32_t sda_filter;
      std::uint32_t fifo_conf;
      std::uint32_t timeout;
    };
    i2c_context_t i2c_context[I2C_NUM_MAX];

    static void i2c_set_cmd(i2c_dev_t* dev, uint8_t index, uint8_t op_code, uint8_t byte_num)
    {
      typeof(dev->command[0]) cmd;
      cmd.val = 0;
      cmd.ack_en = (op_code == I2C_CMD_WRITE || op_code == I2C_CMD_STOP);
      cmd.byte_num = byte_num;
      cmd.op_code = op_code;
      dev->command[index].val = cmd.val;
    }

    static void i2c_stop(int i2c_port)
    {
      static constexpr int I2C_CLR_BUS_HALF_PERIOD_US = 5; // use standard 100kHz data rate
      static constexpr int I2C_CLR_BUS_SCL_NUM        = 9;

      gpio_num_t sda_io = i2c_context[i2c_port].pin_sda;
      gpio_set_level(sda_io, 1);
      gpio_set_direction(sda_io, GPIO_MODE_INPUT_OUTPUT_OD);

      gpio_num_t scl_io = i2c_context[i2c_port].pin_scl;
      gpio_set_level(scl_io, 1);
      gpio_set_direction(scl_io, GPIO_MODE_OUTPUT_OD);

      auto mod = i2c_port == 0 ? PERIPH_I2C0_MODULE : PERIPH_I2C1_MODULE;
      // ESP-IDF環境でperiph_module_disableを使うと、後でenableできなくなる問題が起きたためコメントアウト
      //periph_module_disable(mod);
      gpio_set_level(scl_io, 0);

      // SDAがHIGHになるまでクロック送出しながら待機する。
      int i = 0;
      while (!gpio_get_level(sda_io) && (i++ < I2C_CLR_BUS_SCL_NUM))
      {
        ets_delay_us(I2C_CLR_BUS_HALF_PERIOD_US);
        gpio_set_level(scl_io, 1);
        ets_delay_us(I2C_CLR_BUS_HALF_PERIOD_US);
        gpio_set_level(scl_io, 0);
      }
      gpio_set_level(sda_io, 0); // setup for STOP
      periph_module_enable(mod);
      gpio_set_level(scl_io, 1);
      periph_module_reset(mod);
      //gpio_set_level(sda_io, 1); // STOP, SDA low -> high while SCL is HIGH
      i2c_set_pin((i2c_port_t)i2c_port, sda_io, scl_io, gpio_pullup_t::GPIO_PULLUP_ENABLE, gpio_pullup_t::GPIO_PULLUP_ENABLE, I2C_MODE_MASTER);
    }

    static cpp::result<void, error_t> i2c_wait(int i2c_port, bool flg_stop = false)
    {
      if (i2c_context[i2c_port].state.has_error()) { return cpp::fail(i2c_context[i2c_port].state.error()); }
      cpp::result<void, error_t> res = {};
      if (i2c_context[i2c_port].state == i2c_context_t::state_disconnect) { return res; }
      auto dev = (i2c_port == 0) ? &I2C0 : &I2C1;
      typeof(dev->int_raw) int_raw;
//      static constexpr std::uint32_t intmask = I2C_ACK_ERR_INT_RAW_M | I2C_TIME_OUT_INT_RAW_M | I2C_END_DETECT_INT_RAW_M | I2C_ARBITRATION_LOST_INT_RAW_M;
      static constexpr std::uint32_t intmask = I2C_ACK_ERR_INT_RAW_M | I2C_END_DETECT_INT_RAW_M | I2C_ARBITRATION_LOST_INT_RAW_M;
      if (i2c_context[i2c_port].wait_ack)
      {
        int_raw.val = dev->int_raw.val;
        if (!(int_raw.val & intmask))
        {
          std::uint32_t us = lgfx::micros();
          std::uint32_t us_limit = (dev->scl_high_period.period + dev->scl_low_period.period + 16 ) * (1 + dev->status_reg.tx_fifo_cnt);
          do
          {
            int_raw.val = dev->int_raw.val;
          } while (!(int_raw.val & intmask) && ((lgfx::micros() - us) <= us_limit));
        }
        dev->int_clr.val = int_raw.val;

        if (!int_raw.end_detect || int_raw.ack_err)
        {
          res = cpp::fail(error_t::connection_lost);
          i2c_context[i2c_port].state = cpp::fail(error_t::connection_lost);
        }
      }

      if (flg_stop || res.has_error())
      {
        if (i2c_context[i2c_port].state == i2c_context_t::state_read || !int_raw.end_detect)
        { // force stop
          i2c_stop(i2c_port);
        }
        else
        {
          i2c_set_cmd(dev, 0, I2C_CMD_STOP, 0);
          dev->ctr.trans_start = 1;
          static constexpr std::uint32_t intmask = I2C_ACK_ERR_INT_RAW_M | I2C_TIME_OUT_INT_RAW_M | I2C_END_DETECT_INT_RAW_M | I2C_ARBITRATION_LOST_INT_RAW_M | I2C_TRANS_COMPLETE_INT_RAW_M;
          std::uint32_t ms = lgfx::millis();
          taskYIELD();
          while (!(dev->int_raw.val & intmask) && ((millis() - ms) < 14));
          if (res.has_value() && dev->int_raw.ack_err)
          {
            res = cpp::fail(error_t::connection_lost);
          }
          //ESP_LOGI("LGFX", "I2C stop");
        }
        i2c_context[i2c_port].load_reg(dev);
        if (res)
        {
          i2c_context[i2c_port].state = i2c_context_t::state_t::state_disconnect;
        }
      }
      i2c_context[i2c_port].wait_ack = false;
      return res;
    }

    cpp::result<void, error_t> init(int i2c_port, int pin_sda, int pin_scl)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      auto dev = (i2c_port == 0) ? &I2C0 : &I2C1;
      i2c_context[i2c_port].save_reg(dev);
      release(i2c_port);
      i2c_context[i2c_port].pin_scl = (gpio_num_t)pin_scl;
      i2c_context[i2c_port].pin_sda = (gpio_num_t)pin_sda;
      i2c_stop(i2c_port);
      i2c_context[i2c_port].load_reg(dev);

      return {};
    }

    cpp::result<void, error_t> release(int i2c_port)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }

      if (i2c_context[i2c_port].pin_scl >= 0 || i2c_context[i2c_port].pin_sda >= 0)
      {
      // ESP-IDF環境でperiph_module_disableを使うと、後でenableできなくなる問題が起きたためコメントアウト
//        periph_module_disable(i2c_port == 0 ? PERIPH_I2C0_MODULE : PERIPH_I2C1_MODULE);
        pinMode(i2c_context[i2c_port].pin_scl, pin_mode_t::input);
        pinMode(i2c_context[i2c_port].pin_sda, pin_mode_t::input);
        i2c_context[i2c_port].pin_scl = (gpio_num_t)-1;
        i2c_context[i2c_port].pin_sda = (gpio_num_t)-1;
      }

      return {};
    }

    cpp::result<void, error_t> restart(int i2c_port, int i2c_addr, std::uint32_t freq, bool read)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      if (i2c_addr < I2C_7BIT_ADDR_MIN || i2c_addr > I2C_10BIT_ADDR_MAX) return cpp::fail(error_t::invalid_arg);

      auto res = i2c_wait(i2c_port);
      if (res.has_error()) return res;

//ESP_LOGI("LGFX", "i2c::beginTransaction : port:%d / addr:%02x / freq:%d / rw:%d", i2c_port, i2c_addr, freq, read);

      auto dev = (i2c_port == 0) ? &I2C0 : &I2C1;

      uint32_t fifo_addr = (i2c_port == 0) ? 0x6001301c : 0x6002701c;
      i2c_set_cmd(dev, 0, I2C_CMD_RESTART, 0);
      i2c_set_cmd(dev, 2, I2C_CMD_END, 0);
      if (i2c_addr <= I2C_7BIT_ADDR_MAX)
      { // 7bitアドレスの場合
        WRITE_PERI_REG(fifo_addr, i2c_addr << 1 | (read ? I2C_MASTER_READ : I2C_MASTER_WRITE));
        i2c_set_cmd(dev, 1, I2C_CMD_WRITE, 1);
      }
      else
      { // 10bitアドレスの場合
        WRITE_PERI_REG(fifo_addr, 0xF0 | (i2c_addr>>8)<<1 | I2C_MASTER_WRITE);
        WRITE_PERI_REG(fifo_addr,         i2c_addr);
        i2c_set_cmd(dev, 1, I2C_CMD_WRITE, 2);
        if (read)
        { // 10bitアドレスのread要求の場合
          WRITE_PERI_REG(fifo_addr, 0xF0 | (i2c_addr>>8)<<1 | I2C_MASTER_READ);
          i2c_set_cmd(dev, 2, I2C_CMD_RESTART, 0);
          i2c_set_cmd(dev, 3, I2C_CMD_READ, 1);
          i2c_set_cmd(dev, 4, I2C_CMD_END, 0);
        }
      }

      if (i2c_context[i2c_port].state == i2c_context_t::state_disconnect || i2c_context[i2c_port].freq != freq)
      {
        i2c_context[i2c_port].freq = freq;
        static constexpr std::uint32_t MIN_I2C_CYCLE = 35;
        rtc_cpu_freq_config_t cpu_freq_conf;
        rtc_clk_cpu_freq_get_config(&cpu_freq_conf);
        std::uint32_t apb = 80 * 1000000;
        if (cpu_freq_conf.freq_mhz < 80)
        {
          apb = (cpu_freq_conf.source_freq_mhz * 1000000) / cpu_freq_conf.div;
        }
        std::uint32_t cycle = std::min(32767u, std::max(MIN_I2C_CYCLE, (apb / (freq + 1) + 1)));
        freq = apb / cycle;
        dev->scl_filter_cfg.en = cycle > 64;
        dev->scl_filter_cfg.thres = 0;
        dev->sda_filter_cfg.en = cycle > 64;
        dev->sda_filter_cfg.thres = 0;

  /// ESP32 TRM page 286  Table 57: SCL Frequency Configuration
        std::uint32_t scl_high_offset = ( dev->scl_filter_cfg.en
                                        ? ( dev->scl_filter_cfg.thres <= 2
                                          ? 8 : (6 + dev->scl_filter_cfg.thres)
                                          )
                                        : 7
                                        );

        std::uint32_t period_total = cycle - scl_high_offset - 1;
        std::uint32_t scl_high_period = std::max(18u, (period_total-10) >> 1);
        std::uint32_t scl_low_period  = period_total - scl_high_period;

        dev->scl_high_period.period = scl_high_period;
        dev->scl_low_period .period = scl_low_period ;

        dev->sda_hold.time   = std::min(1023, (dev->scl_high_period.period >> 1));
        dev->sda_sample.time = std::min(1023, (dev->scl_low_period .period >> 1));

        if (freq > 400000)
        {
          cycle = cycle * freq / 400000;
        }
        else
        if (cycle > ((1<<10)-1))
        {
          cycle = (1<<10)-1;
        }
        dev->scl_stop_hold.time = cycle << 1;     //the clock num after the STOP bit's posedge
        dev->scl_stop_setup.time = cycle;    //the clock num between the posedge of SCL and the posedge of SDA
        dev->scl_start_hold.time = cycle;    //the clock num between the negedge of SDA and negedge of SCL for start mark
        dev->scl_rstart_setup.time = cycle;  //the clock num between the posedge of SCL and the negedge of SDA for restart mark
      }

      dev->int_clr.val = 0x1FFF;
      dev->ctr.trans_start = 1;
      i2c_context[i2c_port].state = read ? i2c_context_t::state_t::state_read : i2c_context_t::state_t::state_write;
      i2c_context[i2c_port].wait_ack = true;

      return res;
    }

    cpp::result<void, error_t> beginTransaction(int i2c_port, int i2c_addr, std::uint32_t freq, bool read)
    {
      if (i2c_port >= I2C_NUM_MAX) return cpp::fail(error_t::invalid_arg);

//ESP_LOGI("LGFX", "i2c::beginTransaction : port:%d / addr:%02x / freq:%d / rw:%d", i2c_port, i2c_addr, freq, read);

      auto dev = (i2c_port == 0) ? &I2C0 : &I2C1;
      i2c_context[i2c_port].save_reg(dev);

      if (dev->status_reg.bus_busy)
      {
        //ESP_LOGI("LGFX", "i2c::begin wait");
        auto ms = micros();
        do
        {
          taskYIELD();
        } while (dev->status_reg.bus_busy && micros() - ms < 128);
      }

      dev->int_ena.val = 0;
      dev->timeout.tout = 0xFFFFF; // max 13ms
// ---------- i2c_ll_master_init
      typeof(dev->ctr) ctrl_reg;
      ctrl_reg.val = 0;
      ctrl_reg.ms_mode = 1;       // master mode
      ctrl_reg.sda_force_out = 1;
      ctrl_reg.scl_force_out = 1;
      dev->ctr.val = ctrl_reg.val;
// ---------- i2c_ll_master_init
      typeof(dev->fifo_conf) fifo_conf_reg;
      fifo_conf_reg.val = 0;
      fifo_conf_reg.tx_fifo_rst = 1;
      fifo_conf_reg.rx_fifo_rst = 1;
      dev->fifo_conf.val = fifo_conf_reg.val;

      fifo_conf_reg.val = 0;
      dev->fifo_conf.val = fifo_conf_reg.val;

      i2c_context[i2c_port].state = i2c_context_t::state_t::state_disconnect;

      return restart(i2c_port, i2c_addr, freq, read);
    }

    cpp::result<void, error_t> endTransaction(int i2c_port)
    {
      if (i2c_port >= I2C_NUM_MAX) return cpp::fail(error_t::invalid_arg);
      return i2c_wait(i2c_port, true);
    }
//*/
    cpp::result<void, error_t> writeBytes(int i2c_port, const std::uint8_t *data, std::size_t length)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      if (i2c_context[i2c_port].state.has_error()) { return cpp::fail(i2c_context[i2c_port].state.error()); }
      if (i2c_context[i2c_port].state != i2c_context_t::state_write) { return cpp::fail(error_t::mode_mismatch); }
      cpp::result<void, error_t> res {};
      if (!length) return res;

      static constexpr int txfifo_limit = 32;
      auto dev = (i2c_port == 0) ? &I2C0 : &I2C1;
      uint32_t fifo_addr = (i2c_port == 0) ? 0x6001301c : 0x6002701c;
      std::size_t len = ((length - 1) & (txfifo_limit-1)) + 1;
      do
      {
        res = i2c_wait(i2c_port);
        if (res.has_error())
        {
          ESP_LOGE("LGFX", "i2c write error : ack wait");
          break;
        }
        std::size_t idx = 0;
        do
        {
          WRITE_PERI_REG(fifo_addr, data[idx]);
        } while (++idx != len);
        i2c_set_cmd(dev, 0, I2C_CMD_WRITE, len);
        i2c_set_cmd(dev, 1, I2C_CMD_END, 0);
        dev->ctr.trans_start = 1;
        i2c_context[i2c_port].wait_ack = true;
        data += len;
        length -= len;
        len = txfifo_limit;
      } while (length);
      return res;
    }

    cpp::result<void, error_t> readBytes(int i2c_port, std::uint8_t *readdata, std::size_t length)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      if (i2c_context[i2c_port].state.has_error()) { return cpp::fail(i2c_context[i2c_port].state.error()); }
      if (i2c_context[i2c_port].state != i2c_context_t::state_read) { return cpp::fail(error_t::mode_mismatch); }
      cpp::result<void, error_t> res {};
      if (!length) return res;

      static constexpr std::uint32_t intmask = I2C_ACK_ERR_INT_RAW_M | I2C_TIME_OUT_INT_RAW_M | I2C_END_DETECT_INT_RAW_M | I2C_ARBITRATION_LOST_INT_RAW_M;

      auto dev = (i2c_port == 0) ? &I2C0 : &I2C1;
      std::size_t len = 0;

      std::uint32_t us_limit = (dev->scl_high_period.period + dev->scl_low_period.period + 16);
      do
      {
        len = ((length-1) & 63) + 1;
        length -= len;
        res = i2c_wait(i2c_port);
        if (res.has_error())
        {
          ESP_LOGE("LGFX", "i2c read error : ack wait");
          break;
        }
        i2c_set_cmd(dev, 0, I2C_CMD_READ, len);
        i2c_set_cmd(dev, 1, I2C_CMD_END, 0);
        dev->int_clr.val = intmask;
        dev->ctr.trans_start = 1;
        //taskYIELD();
        do
        {
          std::uint32_t us = lgfx::micros();
          while (0 == dev->status_reg.rx_fifo_cnt && !(dev->int_raw.val & intmask) && ((micros() - us) <= us_limit));
          if (0 != dev->status_reg.rx_fifo_cnt)
          {
            *readdata++ = dev->fifo_data.data;
          }
          else
          {
            i2c_stop(i2c_port);
            ESP_LOGE("LGFX", "i2c read error : read timeout");
            res = cpp::fail(error_t::connection_lost);
            i2c_context[i2c_port].state = cpp::fail(error_t::connection_lost);
          }
        } while (--len);
      } while (length);

      return res;
    }

    cpp::result<void, error_t> transactionWrite(int i2c_port, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint32_t freq)
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

    cpp::result<void, error_t> transactionRead(int i2c_port, int addr, std::uint8_t *readdata, std::uint8_t readlen, std::uint32_t freq)
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

    cpp::result<void, error_t> transactionWriteRead(int i2c_port, int addr, const std::uint8_t *writedata, std::uint8_t writelen, std::uint8_t *readdata, std::size_t readlen, std::uint32_t freq)
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

    cpp::result<std::uint8_t, error_t> readRegister8(int i2c_port, int addr, std::uint8_t reg, std::uint32_t freq)
    {
      auto res = transactionWriteRead(i2c_port, addr, &reg, 1, &reg, 1, freq);
      if (res.has_value()) { return reg; }
      return cpp::fail( res.error() );
    }

    cpp::result<void, error_t> writeRegister8(int i2c_port, int addr, std::uint8_t reg, std::uint8_t data, std::uint8_t mask, std::uint32_t freq)
    {
      std::uint8_t tmp[2] = { reg, data };
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
