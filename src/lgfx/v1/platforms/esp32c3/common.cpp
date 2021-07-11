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
#if defined (ESP32) || defined (ESP_PLATFORM)
#include <sdkconfig.h>
#if defined (CONFIG_IDF_TARGET_ESP32C3)

#include "../common.hpp"

#include <algorithm>
#include <cstring>

#include <driver/i2c.h>
#include <driver/spi_common.h>
#include <driver/rtc_io.h>
#include <driver/periph_ctrl.h>
#include <soc/soc.h>
#include <soc/rtc.h>
#include <soc/i2c_reg.h>
#include <soc/i2c_struct.h>

#if defined ( ARDUINO )
 #include <SPI.h>
 #include <Wire.h>
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

#if SOC_RTCIO_INPUT_OUTPUT_SUPPORTED
    if (rtc_gpio_is_valid_gpio((gpio_num_t)pin)) rtc_gpio_deinit((gpio_num_t)pin);
#endif
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
 #if defined (SOC_SPI_PERIPH_NUM)
    static spi_t* _spi_handle[SOC_SPI_PERIPH_NUM] = {nullptr};
 #else
    static spi_t* _spi_handle[SPI3_HOST + 1] = {nullptr};
 #endif
#else // ESP-IDF
    static spi_device_handle_t _spi_handle[SOC_SPI_PERIPH_NUM] = {nullptr};
#endif

    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi)
    {
      return init(spi_host, spi_sclk, spi_miso, spi_mosi, SPI_DMA_CH_AUTO);
    }

    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi, int dma_channel)
    {
//ESP_LOGI("LGFX","spi::init host:%d, sclk:%d, miso:%d, mosi:%d, dma:%d", spi_host, spi_sclk, spi_miso, spi_mosi, dma_channel);
      std::uint32_t spi_port = (spi_host + 1);
      (void)spi_port;

#if defined (ARDUINO) // Arduino ESP32
      if (spi_host == SPI2_HOST)
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
/*
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
//*/
      WRITE_PERI_REG(SPI_USER_REG (spi_port), SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN);  // need SD card access (full duplex setting)
      WRITE_PERI_REG(SPI_CTRL_REG( spi_port), 0);
      WRITE_PERI_REG(SPI_SLAVE_REG(spi_port), 0);

#else // ESP-IDF

      if (_spi_handle[spi_host] == nullptr)
      {
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

        if (ESP_OK != spi_bus_initialize(static_cast<spi_host_device_t>(spi_host), &buscfg, dma_channel))
        {
          ESP_LOGE("LGFX", "Failed to spi_bus_initialize. ");
        }

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

#if defined ( SPI_CTRL1_REG )
      WRITE_PERI_REG(SPI_CTRL1_REG(spi_port), 0);
#endif

      return {};
    }

    void release(int spi_host)
    {
//ESP_LOGI("LGFX","spi::release");
      if (_spi_handle[spi_host] != nullptr) {
#if defined (ARDUINO) // Arduino ESP32
        if (spi_host == SPI2_HOST)
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
      (void)spi_port;
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
      (void)spi_port;
      if (len > 64) len = 64;
      memcpy(reinterpret_cast<void*>(SPI_W0_REG(spi_port)), data, len);
      WRITE_PERI_REG(SPI_MS_DLEN_REG(spi_port), (len << 3) - 1);
      WRITE_PERI_REG(SPI_CMD_REG(      spi_port), SPI_USR);
      while (READ_PERI_REG(SPI_CMD_REG(spi_port)) & SPI_USR);
    }

    void readBytes(int spi_host, std::uint8_t* data, std::size_t len)
    {
      std::uint32_t spi_port = (spi_host + 1);
      (void)spi_port;
      if (len > 64) len = 64;
      memcpy(reinterpret_cast<void*>(SPI_W0_REG(spi_port)), data, len);
      WRITE_PERI_REG(SPI_MS_DLEN_REG(spi_port), (len << 3) - 1);
      WRITE_PERI_REG(SPI_CMD_REG(      spi_port), SPI_USR);
      while (READ_PERI_REG(SPI_CMD_REG(spi_port)) & SPI_USR);

      memcpy(data, reinterpret_cast<const void*>(SPI_W0_REG(spi_port)), len);
    }
  }

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
 }
}

#endif
#endif
