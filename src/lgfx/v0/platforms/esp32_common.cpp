#include "../lgfx_common.hpp"
#if defined (LGFX_ENABLE_V0) && defined (ESP_PLATFORM)

#include "esp32_common.hpp"

#include <driver/i2c.h>
#include <driver/spi_common.h>
#include <driver/rtc_io.h>
#include <soc/rtc.h>

#if __has_include (<esp_private/periph_ctrl.h>)
 #include <esp_private/periph_ctrl.h>
#else
 #include <driver/periph_ctrl.h>
#endif

#if defined ( ARDUINO )
 #include <SPI.h>
 #include <Wire.h>
 #include <esp32-hal-ledc.h>
 #include <esp32-hal-i2c.h>
#else
 #include <driver/ledc.h>
 #include <driver/spi_master.h>
 #include <esp_log.h>
#endif

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  void lgfxPinMode(int_fast8_t pin, pin_mode_t mode)
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
    pinMode(pin, m);
#else
    if (rtc_gpio_is_valid_gpio((gpio_num_t)pin)) rtc_gpio_deinit((gpio_num_t)pin);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (uint64_t)1 << pin;
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

  void initPWM(int_fast8_t pin, uint32_t pwm_ch, uint32_t freq, uint8_t duty)
  {
#ifdef ARDUINO

    ledcSetup(pwm_ch, freq, 8);
    ledcAttachPin(pin, pwm_ch);
    ledcWrite(pwm_ch, duty);

#else

    static ledc_channel_config_t ledc_channel;
    {
     ledc_channel.gpio_num   = (gpio_num_t)pin;
#if SOC_LEDC_SUPPORT_HS_MODE
     ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
#else
     ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
#endif
     ledc_channel.channel    = (ledc_channel_t)pwm_ch;
     ledc_channel.intr_type  = LEDC_INTR_DISABLE;
     ledc_channel.timer_sel  = (ledc_timer_t)((pwm_ch >> 1) & 3);
     ledc_channel.duty       = duty; // duty;
     ledc_channel.hpoint     = 0;
    };
    ledc_channel_config(&ledc_channel);
    static ledc_timer_config_t ledc_timer;
    {
#if SOC_LEDC_SUPPORT_HS_MODE
      ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;     // timer mode
#else
      ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
#endif
      ledc_timer.duty_resolution = (ledc_timer_bit_t)8; // resolution of PWM duty
      ledc_timer.freq_hz = freq;                        // frequency of PWM signal
      ledc_timer.timer_num = ledc_channel.timer_sel;    // timer index
    };
    ledc_timer_config(&ledc_timer);

#endif
  }

  void setPWMDuty(uint32_t pwm_ch, uint8_t duty)
  {
#ifdef ARDUINO
    ledcWrite(pwm_ch, duty);
#elif SOC_LEDC_SUPPORT_HS_MODE
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)pwm_ch, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)pwm_ch);
#else
    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)pwm_ch, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)pwm_ch);
#endif
  }

  uint32_t getApbFrequency(void)
  {
    rtc_cpu_freq_config_t conf;
    rtc_clk_cpu_freq_get_config(&conf);
    if (conf.freq_mhz >= 80){
      return 80 * 1000000;
    }
    return (conf.source_freq_mhz * 1000000) / conf.div;
  }

  uint32_t FreqToClockDiv(uint32_t fapb, uint32_t hz)
  {
    if (hz > ((fapb >> 2) * 3)) {
      return SPI_CLK_EQU_SYSCLK;
    }
    uint32_t besterr = fapb;
    uint32_t halfhz = hz >> 1;
    uint32_t bestn = 0;
    uint32_t bestpre = 0;
    for (uint32_t n = 2; n <= 64; n++) {
      uint32_t pre = ((fapb / n) + halfhz) / hz;
      if (pre == 0) pre = 1;
      else if (pre > 8192) pre = 8192;

      int errval = abs((int32_t)(fapb / (pre * n) - hz));
      if (errval < besterr) {
        besterr = errval;
        bestn = n - 1;
        bestpre = pre - 1;
        if (!besterr) break;
      }
    }
    return bestpre << 18 | bestn << 12 | ((bestn-1)>>1) << 6 | bestn;
  }

//----------------------------------------------------------------------------

  namespace spi
  {
#if defined ( ARDUINO )
    static spi_t* _spi_handle[VSPI_HOST + 1] = {nullptr};
#else // ESP-IDF
    static spi_device_handle_t _spi_handle[SOC_SPI_PERIPH_NUM] = {nullptr};
#endif

    void init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi)
    {
      init(spi_host, spi_sclk, spi_miso, spi_mosi, 0);
    }

    void init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi, int dma_channel)
    {
      uint32_t spi_port = (spi_host + 1);

#if defined (ARDUINO) // Arduino ESP32
      if (spi_host == VSPI_HOST) {
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
        lgfxPinMode(spi_mosi, pin_mode_t::output);
        gpio_matrix_out(spi_mosi, spi_periph_signal[spi_host].spid_out, false, false);
        gpio_matrix_in(spi_mosi, spi_periph_signal[spi_host].spid_in, false);
      }
      if (spi_miso >= 0) {
        lgfxPinMode(spi_miso, pin_mode_t::input);
      //gpio_matrix_out(spi_miso, spi_periph_signal[spi_host].spiq_out, false, false);
        gpio_matrix_in(spi_miso, spi_periph_signal[spi_host].spiq_in, false);
      }
      if (spi_sclk >= 0) {
        gpio_lo(spi_sclk); // ここでLOWにしておくことで、pinMode変更によるHIGHパルスが出力されるのを防止する (CSなしパネル対策)
        lgfxPinMode(spi_sclk, pin_mode_t::output);
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

      spi_bus_config_t buscfg;
      buscfg.mosi_io_num = spi_mosi;
      buscfg.miso_io_num = spi_miso;
      buscfg.sclk_io_num = spi_sclk;
      buscfg.quadwp_io_num = -1;
      buscfg.quadhd_io_num = -1;
      buscfg.max_transfer_sz = 1;
      buscfg.flags = SPICOMMON_BUSFLAG_MASTER;
      buscfg.intr_flags = 0;

      if (ESP_OK != spi_bus_initialize(static_cast<spi_host_device_t>(spi_host), &buscfg, dma_channel)) {
        ESP_LOGE("LGFX", "Failed to spi_bus_initialize. ");
      }

      if (_spi_handle[spi_host] == nullptr) {
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
    }

    void release(int spi_host)
    {
      if (_spi_handle[spi_host] != nullptr) {
#if defined (ARDUINO) // Arduino ESP32
        if (spi_host == VSPI_HOST) {
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

    void beginTransaction(int spi_host, int spi_cs, int freq, int spi_mode)
    {
      uint32_t spi_port = (spi_host + 1);
      uint32_t clkdiv = FreqToClockDiv(getApbFrequency(), freq);

      uint32_t user = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN;
      if (spi_mode == 1 || spi_mode == 2) user |= SPI_CK_OUT_EDGE;
      uint32_t pin = 0;
      if (spi_mode & 2) pin = SPI_CK_IDLE_EDGE;

      beginTransaction(spi_host);

      WRITE_PERI_REG(SPI_USER_REG(spi_port), user);
#if defined (SPI_PIN_REG)
      WRITE_PERI_REG(SPI_PIN_REG( spi_port), pin);
#else
      WRITE_PERI_REG(SPI_MISC_REG( spi_port), pin);
#endif
      WRITE_PERI_REG(SPI_CLOCK_REG(spi_port), clkdiv);
      gpio_lo(spi_cs);
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

    void writeData(int spi_host, const uint8_t* data, uint32_t len)
    {
      uint32_t spi_port = (spi_host + 1);
      if (len > 64) len = 64;
      memcpy(reinterpret_cast<void*>(SPI_W0_REG(spi_port)), data, len);
      WRITE_PERI_REG(SPI_MOSI_DLEN_REG(spi_port), (len << 3) - 1);
      WRITE_PERI_REG(SPI_CMD_REG(      spi_port), SPI_USR);
      while (READ_PERI_REG(SPI_CMD_REG(spi_port)) & SPI_USR);
    }

    void readData(int spi_host, uint8_t* data, uint32_t len)
    {
      uint32_t spi_port = (spi_host + 1);
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
    void init(int i2c_port, int pin_sda, int pin_scl, int freq)
    {
#if defined (ARDUINO) // Arduino ESP32
      auto &twowire = (i2c_port) ? Wire1 : Wire;
      twowire.begin(pin_sda, pin_scl);
      twowire.setClock(freq);

#else // ESP-IDF
      i2c_config_t conf;
      conf.mode = I2C_MODE_MASTER;
      conf.sda_io_num = (gpio_num_t)pin_sda;
      conf.scl_io_num = (gpio_num_t)pin_scl;
      conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
      conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
      conf.master.clk_speed = freq;

      i2c_param_config(static_cast<i2c_port_t>(i2c_port), &conf);
      i2c_driver_install(static_cast<i2c_port_t>(i2c_port), I2C_MODE_MASTER, 0, 0, 0);
#endif
    }

    bool writeBytes(int i2c_port, uint16_t addr, const uint8_t *data, uint8_t len)
    {
#if defined (ARDUINO) // Arduino ESP32
      auto &twowire = (i2c_port) ? Wire1 : Wire;

      twowire.beginTransmission(addr);
      twowire.write(data, len);
      return 0 == twowire.endTransmission();

#else // ESP-IDF
      auto cmd = i2c_cmd_link_create();
      i2c_master_start(cmd);
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
      i2c_master_write(cmd, const_cast<uint8_t*>(data), len, true);
      i2c_master_stop(cmd);

      auto result = i2c_master_cmd_begin(static_cast<i2c_port_t>(i2c_port), cmd, 10/portTICK_PERIOD_MS);
      i2c_cmd_link_delete(cmd);

      return result == ESP_OK;
#endif
    }

    bool writeReadBytes(int i2c_port, uint16_t addr, const uint8_t *writedata, uint8_t writelen, uint8_t *readdata, uint8_t readlen)
    {
#if defined (ARDUINO) // Arduino ESP32
      auto &twowire = (i2c_port) ? Wire1 : Wire;
      twowire.beginTransmission(addr);
      twowire.write(writedata, writelen);
      if (0 != twowire.endTransmission(false))
      {
        return false;
      }
      twowire.requestFrom(addr, readlen, true);
      twowire.readBytes(readdata, readlen);
      return true;
#else // ESP-IDF
      auto cmd = i2c_cmd_link_create();
      i2c_master_start(cmd);
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
      i2c_master_write(cmd, const_cast<uint8_t*>(writedata), writelen, true);
      i2c_master_start(cmd);
      i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
      i2c_master_read(cmd, readdata, readlen, I2C_MASTER_LAST_NACK);
      i2c_master_stop(cmd);

      auto result = i2c_master_cmd_begin(static_cast<i2c_port_t>(i2c_port), cmd, 10/portTICK_PERIOD_MS);
      i2c_cmd_link_delete(cmd);

      return result == ESP_OK;
#endif
    }

    bool readRegister(int i2c_port, uint16_t addr, uint8_t reg, uint8_t *data, uint8_t len)
    {
      return writeReadBytes(i2c_port, addr, &reg, 1, data, len);
    }

    bool writeRegister8(int i2c_port, uint16_t addr, uint8_t reg, uint8_t data, uint8_t mask)
    {
      uint8_t tmp[2] = { reg, data };
      if (mask) {
        if (!readRegister(i2c_port, addr, reg, &tmp[1], 1)) return false;
        tmp[1] = (tmp[1] & mask) | data;
      }
      return writeBytes(i2c_port, addr, tmp, 2);
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
