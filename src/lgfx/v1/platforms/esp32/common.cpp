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
#if defined (ESP_PLATFORM)
#include <sdkconfig.h>

/// ESP32-S3をターゲットにした際にREG_SPI_BASEが定義されていなかったので応急処置 ;
#if defined ( CONFIG_IDF_TARGET_ESP32S3 )
 #define REG_SPI_BASE(i)   (DR_REG_SPI1_BASE + (((i)>1) ? (((i)* 0x1000) + 0x20000) : (((~(i)) & 1)* 0x1000 )))
#endif

#include "../common.hpp"

#include <algorithm>
#include <string.h>
#include <math.h>

#include <driver/i2c.h>
#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <driver/rtc_io.h>
#include <soc/rtc.h>
#include <soc/soc.h>
#include <soc/i2c_reg.h>
#include <soc/i2c_struct.h>
#include <esp_log.h>

#include <soc/apb_ctrl_reg.h>
#include <soc/efuse_reg.h>

#if __has_include (<esp_private/periph_ctrl.h>)
 #include <esp_private/periph_ctrl.h>
#else
 #include <driver/periph_ctrl.h>
#endif

#if __has_include(<esp_arduino_version.h>)
 #include <esp_arduino_version.h>
#endif

#if defined (ESP_IDF_VERSION_VAL)
 #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(3, 4, 0)

  #if defined (ESP_ARDUINO_VERSION_VAL)
   #if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(2, 0, 3)
     #define LGFX_EFUSE_WORKAROUND
   #endif
  #else
   #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 4, 0)
    #define LGFX_EFUSE_WORKAROUND
   #endif
  #endif

  #if defined ( LGFX_EFUSE_WORKAROUND )
// include <esp_efuse.h> でエラーが出るバージョンが存在するため、エラー回避用の記述を行ってからincludeする。;
   #define _ROM_SECURE_BOOT_H_
   #define MAX_KEY_DIGESTS 3
   struct ets_secure_boot_key_digests
   {
     const void *key_digests[MAX_KEY_DIGESTS];
     bool allow_key_revoke;
   };
   typedef struct ets_secure_boot_key_digests ets_secure_boot_key_digests_t;
  #endif
  #include <esp_efuse.h>
  #define USE_ESP_EFUSE_GET_PKG_VER
 #endif
#endif

#if __has_include(<soc/i2c_periph.h>)
 #include <soc/i2c_periph.h>
#endif

#if defined ( ARDUINO )
 #include <SPI.h>
 #include <Wire.h>
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  static __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }

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
    if (fapb <= hz) return SPI_CLK_EQU_SYSCLK;
    uint32_t div_num = fapb / (1 + hz);
    uint32_t pre = div_num / 64u;
    div_num = div_num / (pre+1);
    return div_num << 12 | ((div_num-1)>>1) << 6 | div_num | pre << 18;
  }

  void calcClockDiv(uint32_t* div_a, uint32_t* div_b, uint32_t* div_n, uint32_t* clkcnt, uint32_t baseClock, uint32_t targetFreq)
  {
    uint32_t diff = INT32_MAX;
    *div_n = 256;
    *div_a = 63;
    *div_b = 62;
    *clkcnt = 64;
    uint32_t start_cnt = std::min<uint32_t>(64u, (baseClock / (targetFreq * 2) + 1));
    uint32_t end_cnt = std::max<uint32_t>(2u, baseClock / 256u / targetFreq);
    if (start_cnt <= 2) { end_cnt = 1; }
    for (uint32_t cnt = start_cnt; diff && cnt >= end_cnt; --cnt)
    {
      float fdiv = (float)baseClock / cnt / targetFreq;
      uint32_t n = std::max<uint32_t>(2u, (uint32_t)fdiv);
      fdiv -= n;

      for (uint32_t a = 63; diff && a > 0; --a)
      {
        uint32_t b = roundf(fdiv * a);
        if (a == b && n == 256) {
          break;
        }
        uint32_t freq = baseClock / ((n * cnt) + (float)(b * cnt) / (float)a);
        uint32_t d = abs((int)targetFreq - (int)freq);
        if (diff <= d) { continue; }
        diff = d;
        *clkcnt = cnt;
        *div_n = n;
        *div_b = b;
        *div_a = a;
        if (b == 0 || a == b) {
          break;
        }
      }
    }
    if (*div_a == *div_b)
    {
        *div_b = 0;
        *div_n += 1;
    }
  }

  uint32_t get_pkg_ver(void)
  {
#if defined ( USE_ESP_EFUSE_GET_PKG_VER )
    return esp_efuse_get_pkg_ver();
#else
    uint32_t pkg_ver = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
    if (pkg_ver == EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4)
    {
      if (REG_READ(APB_CTRL_DATE_REG) & 0x80000000)
      { // ESP32PICOV302
        return 6;
      }
    }
    return pkg_ver;
#endif
  }

//----------------------------------------------------------------------------

  void pinMode(int_fast16_t pin, pin_mode_t mode)
  {
    if (pin < 0) return;

    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_DISABLE);
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

#if !defined ( SPI_MOSI_DLEN_REG )
    static constexpr uint32_t SPI_EXECUTE = SPI_USR | SPI_UPDATE;
    #define SPI_MOSI_DLEN_REG(i) (REG_SPI_BASE(i) + 0x1C)
    #define SPI_MISO_DLEN_REG(i) (REG_SPI_BASE(i) + 0x1C)
#else
    static constexpr uint32_t SPI_EXECUTE = SPI_USR;
#endif

#if !defined (CONFIG_IDF_TARGET) || defined (CONFIG_IDF_TARGET_ESP32)
    static constexpr int default_spi_host = VSPI_HOST;
    static constexpr int spi_periph_num = 3;
#else
    static constexpr int default_spi_host = SPI2_HOST;
    static constexpr int spi_periph_num = SOC_SPI_PERIPH_NUM;
#endif

#if defined ( ARDUINO )
    static spi_t* _spi_handle[spi_periph_num] = {nullptr};
#endif
    static spi_device_handle_t _spi_dev_handle[spi_periph_num] = {nullptr};

    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi)
    {
      return init(spi_host, spi_sclk, spi_miso, spi_mosi, 0); // SPI_DMA_CH_AUTO;
    }

    cpp::result<void, error_t> init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi, int dma_channel)
    {
//ESP_LOGI("LGFX","spi::init host:%d, sclk:%d, miso:%d, mosi:%d, dma:%d", spi_host, spi_sclk, spi_miso, spi_mosi, dma_channel);
      uint32_t spi_port = (spi_host + 1);
      (void)spi_port;

      if (spi_sclk >= 0) {
        gpio_lo(spi_sclk); // ここでLOWにしておくことで、pinMode変更によるHIGHパルスが出力されるのを防止する (CSなしパネル対策);
      }
#if defined (ARDUINO) // Arduino ESP32
      if (spi_host == default_spi_host)
      {
        SPI.end();
        SPI.begin(spi_sclk, spi_miso, spi_mosi);
        _spi_handle[spi_host] = SPI.bus();
      }
      if (_spi_handle[spi_host] == nullptr)
      {
        _spi_handle[spi_host] = spiStartBus(spi_port, SPI_CLK_EQU_SYSCLK, 0, 0);
      }

#endif

 // バスの設定にはESP-IDFのSPIドライバを使用する。;
      if (_spi_dev_handle[spi_host] == nullptr)
      {
        spi_bus_config_t buscfg;
        memset(&buscfg, ~0u, sizeof(spi_bus_config_t));
        buscfg.mosi_io_num = spi_mosi;
        buscfg.miso_io_num = spi_miso;
        buscfg.sclk_io_num = spi_sclk;
        buscfg.max_transfer_sz = 1;
        buscfg.flags = SPICOMMON_BUSFLAG_MASTER;
        buscfg.intr_flags = 0;

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
        if (ESP_OK != spi_bus_add_device(static_cast<spi_host_device_t>(spi_host), &devcfg, &_spi_dev_handle[spi_host])) {
          ESP_LOGE("LGFX", "Failed to spi_bus_add_device. ");
        }
      }

      *reg(SPI_USER_REG(spi_port)) = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN;  // need SD card access (full duplex setting)
      *reg(SPI_CTRL_REG(spi_port)) = 0;
#if defined ( SPI_CTRL1_REG )
      *reg(SPI_CTRL1_REG(spi_port)) = 0;
#endif
#if defined ( SPI_CTRL2_REG )
      *reg(SPI_CTRL2_REG(spi_port)) = 0;
#endif

      return {};
    }

    void release(int spi_host)
    {
//ESP_LOGI("LGFX","spi::release");
#if defined (ARDUINO) // Arduino ESP32
      if (_spi_handle[spi_host] != nullptr)
      {
        if (spi_host == default_spi_host)
        {
          SPI.end();
        }
        else
        {
          spiStopBus(_spi_handle[spi_host]);
        }
        _spi_handle[spi_host] = nullptr;
      }
#endif
      if (_spi_dev_handle[spi_host] != nullptr)
      {
        spi_bus_remove_device(_spi_dev_handle[spi_host]);
        spi_bus_free(static_cast<spi_host_device_t>(spi_host));
        _spi_dev_handle[spi_host] = nullptr;
      }
    }

    void beginTransaction(int spi_host)
    {
#if defined (ARDUINO) // Arduino ESP32
      spiSimpleTransaction(_spi_handle[spi_host]);
#else // ESP-IDF
      if (_spi_dev_handle[spi_host]) {
        if (ESP_OK != spi_device_acquire_bus(_spi_dev_handle[spi_host], portMAX_DELAY)) {
          ESP_LOGE("LGFX", "Failed to spi_device_acquire_bus. ");
        }
      }
#endif
    }

    void beginTransaction(int spi_host, uint32_t freq, int spi_mode)
    {
      uint32_t spi_port = (spi_host + 1);
      (void)spi_port;
      uint32_t clkdiv = FreqToClockDiv(getApbFrequency(), freq);

      uint32_t user = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN;
      if (spi_mode == 1 || spi_mode == 2) user |= SPI_CK_OUT_EDGE;
      uint32_t pin = 0;
      if (spi_mode & 2) pin = SPI_CK_IDLE_EDGE;

      beginTransaction(spi_host);

      *reg(SPI_USER_REG(spi_port)) = user;
#if defined (SPI_PIN_REG)
      *reg(SPI_PIN_REG(spi_port)) = pin;
#else
      *reg(SPI_MISC_REG( spi_port)) = pin;
#endif
      *reg(SPI_CLOCK_REG(spi_port)) = clkdiv;

#if defined ( SPI_UPDATE )
      *reg(SPI_CMD_REG(spi_port)) |= SPI_UPDATE;
#endif
    }

    void endTransaction(int spi_host)
    {
      if (_spi_dev_handle[spi_host]) {
#if defined (ARDUINO) // Arduino ESP32
        spiEndTransaction(_spi_handle[spi_host]);
#else // ESP-IDF
        spi_device_release_bus(_spi_dev_handle[spi_host]);
#endif
      }
    }

    void endTransaction(int spi_host, int spi_cs)
    {
      if (_spi_dev_handle[spi_host]) {
#if defined (ARDUINO) // Arduino ESP32
        spiEndTransaction(_spi_handle[spi_host]);
#else // ESP-IDF
        spi_device_release_bus(_spi_dev_handle[spi_host]);
#endif
      }
      gpio_hi(spi_cs);
    }

    void writeBytes(int spi_host, const uint8_t* data, size_t len)
    {
      uint32_t spi_port = (spi_host + 1);
      (void)spi_port;
      if (len > 64) len = 64;
      memcpy(reinterpret_cast<void*>(SPI_W0_REG(spi_port)), data, (len + 3) & ~3);
      *reg(SPI_MOSI_DLEN_REG(spi_port)) = (len << 3) - 1;
      *reg(SPI_CMD_REG(      spi_port)) = SPI_EXECUTE;
      while (*reg(SPI_CMD_REG(spi_port)) & SPI_USR);
    }

    void readBytes(int spi_host, uint8_t* data, size_t len)
    {
      uint32_t spi_port = (spi_host + 1);
      (void)spi_port;
      if (len > 64) len = 64;
      memcpy(reinterpret_cast<void*>(SPI_W0_REG(spi_port)), data, (len + 3) & ~3);
      *reg(SPI_MOSI_DLEN_REG(spi_port)) = (len << 3) - 1;
      *reg(SPI_CMD_REG(      spi_port)) = SPI_EXECUTE;
      while (*reg(SPI_CMD_REG(spi_port)) & SPI_USR);

      memcpy(data, reinterpret_cast<const void*>(SPI_W0_REG(spi_port)), len);
    }
  }

//----------------------------------------------------------------------------

  namespace i2c
  {
#if __has_include( <core_version.h> )
  #include <core_version.h>
#endif

#if !defined ( I2C_ACK_ERR_INT_RAW_M )
 #define I2C_ACK_ERR_INT_RAW_M I2C_NACK_INT_RAW_M
#endif

#if __has_include(<soc/i2c_periph.h>)
    static periph_module_t getPeriphModule(int num)
    {
      return i2c_periph_signal[num].module;
    }
#else
    static periph_module_t getPeriphModule(int num)
    {
      return num == 0 ? PERIPH_I2C0_MODULE : PERIPH_I2C1_MODULE;
    }
#endif

    static i2c_dev_t* getDev(int num)
    {
#if defined ( CONFIG_IDF_TARGET_ESP32C3 )
      return &I2C0;
#else
      return num == 0 ? &I2C0 : &I2C1;
#endif
    }

#if defined ( CONFIG_IDF_TARGET_ESP32C3 ) || defined ( CONFIG_IDF_TARGET_ESP32S3 )

    static void updateDev(i2c_dev_t* dev)
    {
      dev->ctr.conf_upgate = 1;
    }
    static volatile uint32_t* getFifoAddr(int num)
    {
#if defined ( CONFIG_IDF_TARGET_ESP32S3 )
      return &(getDev(num)->data.val);
#else
      return &(getDev(num)->fifo_data.val);
#endif
    }

    static constexpr int i2c_cmd_start = 6;
    static constexpr int i2c_cmd_write = 1;
    static constexpr int i2c_cmd_read  = 3;
    static constexpr int i2c_cmd_stop  = 2;
    static constexpr int i2c_cmd_end   = 4;

#else

    static void updateDev(i2c_dev_t* dev)
    {
    }
    static volatile uint32_t* getFifoAddr(int num)
    {
      return (volatile uint32_t*)((num == 0) ? 0x6001301c : 0x6002701c);
    }

    static constexpr int i2c_cmd_start = 0;
    static constexpr int i2c_cmd_write = 1;
    static constexpr int i2c_cmd_read  = 2;
    static constexpr int i2c_cmd_stop  = 3;
    static constexpr int i2c_cmd_end   = 4;

#endif

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

      gpio_num_t pin_scl = (gpio_num_t)-1;
      gpio_num_t pin_sda = (gpio_num_t)-1;
      bool initialized = false;
      bool wait_ack = false;
      uint32_t freq = 0;

      void save_reg(i2c_dev_t* dev)
      {
        auto reg = (volatile uint32_t*)dev;
#if defined ( CONFIG_IDF_TARGET_ESP32S3 )
        auto fifo_reg = (volatile uint32_t*)(&dev->data);
#else
        auto fifo_reg = (volatile uint32_t*)(&dev->fifo_data);
#endif
        for (size_t i = 0; i < sizeof(_reg_store) >> 2; ++i)
        {
          if (fifo_reg == &reg[i]) { continue; }
          _reg_store[i] = reg[i];
        }
      }

      void load_reg(i2c_dev_t* dev)
      {
        auto reg = (volatile uint32_t*)dev;
#if defined ( CONFIG_IDF_TARGET_ESP32S3 )
        auto fifo_reg = (volatile uint32_t*)(&dev->data);
#else
        auto fifo_reg = (volatile uint32_t*)(&dev->fifo_data);
#endif
        for (size_t i = 0; i < sizeof(_reg_store) >> 2; ++i)
        {
          if (fifo_reg == &reg[i]) { continue; }
          reg[i] = _reg_store[i];
        }
      }

      void setPins(i2c_dev_t* dev, gpio_num_t scl, gpio_num_t sda)
      {
        pin_sda = sda;
        pin_scl = scl;
#if defined ( ARDUINO )
 #if defined ( ESP_IDF_VERSION_VAL )
  #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
   #if defined ARDUINO_ESP32_GIT_VER
    #if ARDUINO_ESP32_GIT_VER != 0x44c11981
     #define USE_TWOWIRE_SETPINS
    #endif
   #endif
  #endif
 #endif
        auto twowire = ((dev == &I2C0) ? &Wire : &Wire1);
 #if defined ( USE_TWOWIRE_SETPINS )
        twowire->setPins(sda, scl);
 #else
        twowire->begin((int)sda, (int)scl);
 #endif
#endif
      }

    private:
      uint32_t _reg_store[22];
    };
    i2c_context_t i2c_context[I2C_NUM_MAX];



    static void i2c_set_cmd(i2c_dev_t* dev, uint8_t index, uint8_t op_code, uint8_t byte_num)
    {
/*
      typeof(dev->command[0]) cmd;
      cmd.val = 0;
      cmd.ack_en = (op_code == i2c_cmd_write || op_code == i2c_cmd_stop);
      cmd.byte_num = byte_num;
      cmd.op_code = op_code;
      dev->command[index].val = cmd.val;
*/
      uint32_t cmd_val = byte_num
                            | (( op_code == i2c_cmd_write
                              || op_code == i2c_cmd_stop)
                              ? 0x100 : 0)  // writeおよびstop時はACK_ENを有効にする;
                            | op_code << 11 ;
#if defined (CONFIG_IDF_TARGET_ESP32S3)
      (&dev->comd0)[index].val = cmd_val;
#else
      dev->command[index].val = cmd_val;
#endif
    }

    static void i2c_stop(int i2c_port)
    {
#if 1 // !defined (CONFIG_IDF_TARGET) || defined (CONFIG_IDF_TARGET_ESP32)
      static constexpr int I2C_CLR_BUS_HALF_PERIOD_US = 2;
      static constexpr int I2C_CLR_BUS_SCL_NUM        = 9;

      gpio_num_t sda_io = i2c_context[i2c_port].pin_sda;
      gpio_set_level(sda_io, 1);
      gpio_set_direction(sda_io, GPIO_MODE_INPUT_OUTPUT_OD);

      gpio_num_t scl_io = i2c_context[i2c_port].pin_scl;
      gpio_set_level(scl_io, 1);
      gpio_set_direction(scl_io, GPIO_MODE_OUTPUT_OD);
      ets_delay_us(I2C_CLR_BUS_HALF_PERIOD_US);

      auto mod = getPeriphModule(i2c_port);
      // ESP-IDF環境でperiph_module_disableを使うと、後でenableできなくなる問題が起きたためコメントアウト;
      //periph_module_disable(mod);

      // SDAがHIGHになるまでSTOP送出を繰り返す。;
      int i = 0;
      do
      {
        gpio_set_level(scl_io, 0);
        ets_delay_us(I2C_CLR_BUS_HALF_PERIOD_US);
        gpio_set_level(sda_io, 0);
        ets_delay_us(I2C_CLR_BUS_HALF_PERIOD_US);
        gpio_set_level(scl_io, 1);
        ets_delay_us(I2C_CLR_BUS_HALF_PERIOD_US);
        gpio_set_level(sda_io, 1);
        ets_delay_us(I2C_CLR_BUS_HALF_PERIOD_US);
      } while (!gpio_get_level(sda_io) && (i++ < I2C_CLR_BUS_SCL_NUM));
      periph_module_enable(mod);
#if !defined (CONFIG_IDF_TARGET_ESP32C3)
/// ESP32C3で periph_module_reset を使用すると以後通信不能になる問題が起きたため分岐;
      periph_module_reset(mod);
#endif
      i2c_set_pin((i2c_port_t)i2c_port, sda_io, scl_io, gpio_pullup_t::GPIO_PULLUP_ENABLE, gpio_pullup_t::GPIO_PULLUP_ENABLE, I2C_MODE_MASTER);
#else
      auto mod = getPeriphModule(i2c_port);
      periph_module_enable(mod);
      auto dev = getDev(i2c_port);
      dev->scl_sp_conf.scl_rst_slv_num = 9;
      dev->scl_sp_conf.scl_rst_slv_en = 0;
      updateDev(dev);
      dev->scl_sp_conf.scl_rst_slv_en = 1;
      gpio_num_t sda_io = i2c_context[i2c_port].pin_sda;
      gpio_num_t scl_io = i2c_context[i2c_port].pin_scl;
      periph_module_reset(mod);
      i2c_set_pin((i2c_port_t)i2c_port, sda_io, scl_io, gpio_pullup_t::GPIO_PULLUP_ENABLE, gpio_pullup_t::GPIO_PULLUP_ENABLE, I2C_MODE_MASTER);
#endif
    }

    static cpp::result<void, error_t> i2c_wait(int i2c_port, bool flg_stop = false)
    {
      if (i2c_context[i2c_port].state.has_error()) { return cpp::fail(i2c_context[i2c_port].state.error()); }
      cpp::result<void, error_t> res = {};
      if (i2c_context[i2c_port].state == i2c_context_t::state_disconnect) { return res; }
      auto dev = getDev(i2c_port);
      typeof(dev->int_raw) int_raw;
      static constexpr uint32_t intmask = I2C_ACK_ERR_INT_RAW_M | I2C_END_DETECT_INT_RAW_M | I2C_ARBITRATION_LOST_INT_RAW_M;
      if (i2c_context[i2c_port].wait_ack)
      {
        i2c_context[i2c_port].wait_ack = false;
        int_raw.val = dev->int_raw.val;
        if (!(int_raw.val & intmask))
        {
          uint32_t start_us = lgfx::micros();
          uint32_t us;
#if defined ( CONFIG_IDF_TARGET_ESP32C3 )
          uint32_t us_limit = (dev->scl_high_period.period + dev->scl_low_period.period + 16 ) * (1 + dev->sr.tx_fifo_cnt);
#elif defined ( CONFIG_IDF_TARGET_ESP32S3 )
          uint32_t us_limit = (dev->scl_high_period.scl_high_period + dev->scl_low_period.scl_low_period + 16 ) * (1 + dev->sr.txfifo_cnt);
#else
          uint32_t us_limit = (dev->scl_high_period.period + dev->scl_low_period.period + 16 ) * (1 + dev->status_reg.tx_fifo_cnt);
#endif
          do
          {
            taskYIELD();
            us = lgfx::micros() - start_us;
            int_raw.val = dev->int_raw.val;
          } while (!(int_raw.val & intmask) && (us <= us_limit));
        }
        dev->int_clr.val = int_raw.val;
#if !defined (CONFIG_IDF_TARGET) || defined (CONFIG_IDF_TARGET_ESP32)
        if (!int_raw.end_detect || int_raw.ack_err)
#elif defined ( CONFIG_IDF_TARGET_ESP32S3 )
        if (!int_raw.end_detect_int_raw || int_raw.nack_int_raw)
#else
        if (!int_raw.end_detect || int_raw.nack)
#endif
        {
          res = cpp::fail(error_t::connection_lost);
          i2c_context[i2c_port].state = cpp::fail(error_t::connection_lost);
        }
      }

      if (flg_stop || res.has_error())
      {
#if defined ( CONFIG_IDF_TARGET_ESP32S3 )
        if (i2c_context[i2c_port].state == i2c_context_t::state_read || !int_raw.end_detect_int_raw)
#else
        if (i2c_context[i2c_port].state == i2c_context_t::state_read || !int_raw.end_detect)
#endif
        { // force stop
          i2c_stop(i2c_port);
        }
        else
        {
          i2c_set_cmd(dev, 0, i2c_cmd_stop, 0);
          dev->ctr.trans_start = 1;
          static constexpr uint32_t intmask = I2C_ACK_ERR_INT_RAW_M | I2C_TIME_OUT_INT_RAW_M | I2C_END_DETECT_INT_RAW_M | I2C_ARBITRATION_LOST_INT_RAW_M | I2C_TRANS_COMPLETE_INT_RAW_M;
          uint32_t ms = lgfx::millis();
          taskYIELD();
          while (!(dev->int_raw.val & intmask) && ((millis() - ms) < 14));
#if !defined (CONFIG_IDF_TARGET) || defined (CONFIG_IDF_TARGET_ESP32)
          if (res.has_value() && dev->int_raw.ack_err)
#elif defined ( CONFIG_IDF_TARGET_ESP32S3 )
          if (res.has_value() && dev->int_raw.nack_int_raw)
#else
          if (res.has_value() && dev->int_raw.nack)
#endif
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
      return res;
    }

    cpp::result<void, error_t> setPins(int i2c_port, int pin_sda, int pin_scl)
    {
      if ((i2c_port >= I2C_NUM_MAX)
       || ((uint32_t)pin_scl >= GPIO_NUM_MAX)
       || ((uint32_t)pin_sda >= GPIO_NUM_MAX))
      {
        return cpp::fail(error_t::invalid_arg);
      }

      if (i2c_context[i2c_port].initialized
       && i2c_context[i2c_port].pin_scl == (gpio_num_t)pin_scl
       && i2c_context[i2c_port].pin_sda == (gpio_num_t)pin_sda
      )
      {
        return {};
      }

      release(i2c_port).has_value();
      i2c_context[i2c_port].pin_scl = (gpio_num_t)pin_scl;
      i2c_context[i2c_port].pin_sda = (gpio_num_t)pin_sda;
#if defined ( ARDUINO )
 #if defined ( ESP_IDF_VERSION_VAL )
  #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(3, 3, 0)
   #define USE_TWOWIRE_SETPINS
  #endif
 #endif
 #if defined ( USE_TWOWIRE_SETPINS )
      auto twowire = ((i2c_port == 1) ? &Wire1 : &Wire);
      twowire->setPins(pin_sda, pin_scl);
 #endif
#endif
      return {};
    }

    cpp::result<void, error_t> init(int i2c_port)
    {
      if ((i2c_port >= I2C_NUM_MAX)
       || ((uint32_t)i2c_context[i2c_port].pin_scl >= GPIO_NUM_MAX)
       || ((uint32_t)i2c_context[i2c_port].pin_sda >= GPIO_NUM_MAX))
      {
        return cpp::fail(error_t::invalid_arg);
      }
      if (!i2c_context[i2c_port].initialized)
      {
        i2c_context[i2c_port].initialized = true;
        auto dev = getDev(i2c_port);
        i2c_context[i2c_port].save_reg(dev);
        i2c_stop(i2c_port);
        i2c_context[i2c_port].load_reg(dev);
      }

#if defined ( ARDUINO )
      auto twowire = ((i2c_port == 1) ? &Wire1 : &Wire);
 #if defined ( USE_TWOWIRE_SETPINS )
      twowire->begin();
 #else
      twowire->begin((int)i2c_context[i2c_port].pin_sda, (int)i2c_context[i2c_port].pin_scl);
 #endif
#endif

      return {};
    }

    cpp::result<void, error_t> init(int i2c_port, int pin_sda, int pin_scl)
    {
      auto res = setPins(i2c_port, pin_sda, pin_scl);
      if (res.has_value())
      {
        return init(i2c_port);
      }
      return res;
    }

    cpp::result<void, error_t> release(int i2c_port)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      if (i2c_context[i2c_port].initialized)
      {
        i2c_context[i2c_port].initialized = false;
#if defined ( ARDUINO ) && defined ( ESP_IDF_VERSION_VAL )
 #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
  #if defined ARDUINO_ESP32_GIT_VER
    #if ARDUINO_ESP32_GIT_VER != 0x44c11981
        auto twowire = ((i2c_port == 1) ? &Wire1 : &Wire);
        twowire->end();
    #endif
  #endif
 #endif
#endif
        if (i2c_context[i2c_port].pin_scl >= 0)
        {
          pinMode(i2c_context[i2c_port].pin_scl, pin_mode_t::input_pullup);
          i2c_context[i2c_port].pin_scl = (gpio_num_t)-1;
        }
        if (i2c_context[i2c_port].pin_sda >= 0)
        {
          pinMode(i2c_context[i2c_port].pin_sda, pin_mode_t::input_pullup);
          i2c_context[i2c_port].pin_sda = (gpio_num_t)-1;
        }
      }

      return {};
    }

    cpp::result<void, error_t> restart(int i2c_port, int i2c_addr, uint32_t freq, bool read)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      if (i2c_addr < I2C_7BIT_ADDR_MIN || i2c_addr > I2C_10BIT_ADDR_MAX) return cpp::fail(error_t::invalid_arg);

      auto res = i2c_wait(i2c_port);
      if (res.has_error()) return res;

// ESP_LOGI("LGFX", "i2c::restart : port:%d / addr:%02x / freq:%d / rw:%d", i2c_port, i2c_addr, freq, read);

      auto dev = getDev(i2c_port);

      auto fifo_addr = getFifoAddr(i2c_port);
      i2c_set_cmd(dev, 0, i2c_cmd_start, 0);
      i2c_set_cmd(dev, 2, i2c_cmd_end, 0);
      if (i2c_addr <= I2C_7BIT_ADDR_MAX)
      { // 7bitアドレスの場合;
        *fifo_addr = i2c_addr << 1 | (read ? I2C_MASTER_READ : I2C_MASTER_WRITE);
        i2c_set_cmd(dev, 1, i2c_cmd_write, 1);
      }
      else
      { // 10bitアドレスの場合;
        *fifo_addr = 0xF0 | (i2c_addr>>8)<<1 | I2C_MASTER_WRITE;
        *fifo_addr =         i2c_addr;
        i2c_set_cmd(dev, 1, i2c_cmd_write, 2);
        if (read)
        { // 10bitアドレスのread要求の場合;
          *fifo_addr = 0xF0 | (i2c_addr>>8)<<1 | I2C_MASTER_READ;
          i2c_set_cmd(dev, 2, i2c_cmd_start, 0);
          i2c_set_cmd(dev, 3, i2c_cmd_read, 1);
          i2c_set_cmd(dev, 4, i2c_cmd_end, 0);
        }
      }

      if (i2c_context[i2c_port].state == i2c_context_t::state_disconnect || i2c_context[i2c_port].freq != freq)
      {
        i2c_context[i2c_port].freq = freq;
        static constexpr uint32_t MIN_I2C_CYCLE = 35;
#if defined (CONFIG_IDF_TARGET_ESP32C3) || defined (CONFIG_IDF_TARGET_ESP32S3)
        uint32_t src_clock = 40 * 1000 * 1000; // XTAL clock
#else
        rtc_cpu_freq_config_t cpu_freq_conf;
        rtc_clk_cpu_freq_get_config(&cpu_freq_conf);
        uint32_t src_clock = 80 * 1000 * 1000;
        if (cpu_freq_conf.freq_mhz < 80)
        {
          src_clock = (cpu_freq_conf.source_freq_mhz * 1000000) / cpu_freq_conf.div;
        }
// ESP_LOGI("LGFX", "i2c::restart : port:%d / addr:%02x / freq:%d / rw:%d", i2c_port, i2c_addr, freq, read);
// ESP_LOGI("LGFX", "cpu_freq_conf.div             :%d", cpu_freq_conf.div);
// ESP_LOGI("LGFX", "cpu_freq_conf.freq_mhz        :%d", cpu_freq_conf.freq_mhz);
// ESP_LOGI("LGFX", "cpu_freq_conf.source          :%d", cpu_freq_conf.source);
// ESP_LOGI("LGFX", "cpu_freq_conf.source_freq_mhz :%d", cpu_freq_conf.source_freq_mhz);
#endif

        auto cycle = std::min<uint32_t>(32767u, std::max(MIN_I2C_CYCLE, (src_clock / (freq + 1) + 1)));
        freq = src_clock / cycle;

#if defined (CONFIG_IDF_TARGET_ESP32S2)
        dev->ctr.ref_always_on = 1;
#endif

#if defined ( I2C_FILTER_CFG_REG )
        // dev->filter_cfg.scl_en = cycle > 64;
        // dev->filter_cfg.scl_thres = 0;
        // dev->filter_cfg.sda_en = cycle > 64;
        // dev->filter_cfg.sda_thres = 0;

        uint32_t val = (cycle > 64) ? (I2C_SCL_FILTER_EN | I2C_SDA_FILTER_EN) : 0;
        dev->filter_cfg.val = val;
        uint32_t scl_high_offset = ( val ? 8 : 7 );
        dev->clk_conf.sclk_sel = 0;
#else
        dev->scl_filter_cfg.en = cycle > 64;
        dev->scl_filter_cfg.thres = 0;
        dev->sda_filter_cfg.en = cycle > 64;
        dev->sda_filter_cfg.thres = 0;
  /// ESP32 TRM page 286  Table 57: SCL Frequency Configuration
        uint32_t scl_high_offset = ( dev->scl_filter_cfg.en
                                        ? ( dev->scl_filter_cfg.thres <= 2
                                          ? 8 : (6 + dev->scl_filter_cfg.thres)
                                          )
                                        : 7
                                        );

#endif

        uint32_t period_total = cycle - scl_high_offset - 1;
        uint32_t scl_high_period = std::max<uint32_t>(18, (period_total-10) >> 1);
        uint32_t scl_low_period  = period_total - scl_high_period;
        if (freq > 400000)
        {
          cycle = cycle * freq / 400000;
        }
        else
        if (cycle > ((1<<10)-1))
        {
          cycle = (1<<10)-1;
        }

#if defined (CONFIG_IDF_TARGET_ESP32S3)
        dev->scl_high_period.scl_high_period = scl_high_period;
        dev->scl_low_period .scl_low_period  = scl_low_period ;
        dev->sda_hold.sda_hold_time     = std::min<uint32_t>(1023u, (scl_high_period >> 1));
        dev->sda_sample.sda_sample_time = std::min<uint32_t>(1023u, (scl_low_period  >> 1));
        dev->scl_stop_hold.scl_stop_hold_time = cycle << 1;     //the clock num after the STOP bit's posedge
        dev->scl_stop_setup.scl_stop_setup_time = cycle;    //the clock num between the posedge of SCL and the posedge of SDA
        dev->scl_start_hold.scl_start_hold_time = cycle;    //the clock num between the negedge of SDA and negedge of SCL for start mark
        dev->scl_rstart_setup.scl_rstart_setup_time = cycle;  //the clock num between the posedge of SCL and the negedge of SDA for restart mark
#else
        dev->scl_high_period.period = scl_high_period;
        dev->scl_low_period .period = scl_low_period ;
        dev->sda_hold.time   = std::min<uint32_t>(1023u, (scl_high_period >> 1));
        dev->sda_sample.time = std::min<uint32_t>(1023u, (scl_low_period  >> 1));
 #if defined (CONFIG_IDF_TARGET_ESP32S2) || defined (CONFIG_IDF_TARGET_ESP32C3)
        dev->scl_high_period.scl_wait_high_period = scl_high_period >> 2;
 #endif
        dev->scl_stop_hold.time = cycle << 1;     //the clock num after the STOP bit's posedge
        dev->scl_stop_setup.time = cycle;    //the clock num between the posedge of SCL and the posedge of SDA
        dev->scl_start_hold.time = cycle;    //the clock num between the negedge of SDA and negedge of SCL for start mark
        dev->scl_rstart_setup.time = cycle;  //the clock num between the posedge of SCL and the negedge of SDA for restart mark
#endif
      }

      updateDev(dev);
      dev->int_clr.val = 0x1FFFF;
      dev->ctr.trans_start = 1;
      i2c_context[i2c_port].state = read ? i2c_context_t::state_t::state_read : i2c_context_t::state_t::state_write;
      i2c_context[i2c_port].wait_ack = true;
      return res;
    }

    cpp::result<void, error_t> beginTransaction(int i2c_port, int i2c_addr, uint32_t freq, bool read)
    {
      if (i2c_port >= I2C_NUM_MAX) return cpp::fail(error_t::invalid_arg);

//ESP_LOGI("LGFX", "i2c::beginTransaction : port:%d / addr:%02x / freq:%d / rw:%d", i2c_port, i2c_addr, freq, read);

      auto dev = getDev(i2c_port);
      i2c_context[i2c_port].save_reg(dev);

#if defined ( CONFIG_IDF_TARGET_ESP32C3 ) ||  defined ( CONFIG_IDF_TARGET_ESP32S3 )
      if (dev->sr.bus_busy)
#else
      if (dev->status_reg.bus_busy)
#endif
      {
        //ESP_LOGI("LGFX", "i2c::begin wait");
        auto ms = micros();
        do
        {
          taskYIELD();
        }
#if defined ( CONFIG_IDF_TARGET_ESP32C3 ) || defined ( CONFIG_IDF_TARGET_ESP32S3 )
        while (dev->sr.bus_busy && micros() - ms < 128);
#else
        while (dev->status_reg.bus_busy && micros() - ms < 128);
#endif
      }

#if SOC_I2C_SUPPORT_HW_FSM_RST
      dev->ctr.fsm_rst = 1;
#endif

#if defined ( CONFIG_IDF_TARGET_ESP32C3 )
      dev->timeout.time_out_value = 31;
      dev->timeout.time_out_en = 1;
#elif defined ( CONFIG_IDF_TARGET_ESP32S3 )
      dev->to.time_out_value = 31;
      dev->to.time_out_en = 1;
#else
      dev->timeout.tout = 0xFFFFF; // max 13ms
#endif
      dev->int_ena.val = 0;
// ---------- i2c_ll_master_init
      typeof(dev->ctr) ctrl_reg;
      ctrl_reg.val = 0;
      ctrl_reg.ms_mode = 1;       // master mode
      ctrl_reg.clk_en = 1;
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
#if defined (CONFIG_IDF_TARGET_ESP32S2) || defined (CONFIG_IDF_TARGET_ESP32C3) || defined (CONFIG_IDF_TARGET_ESP32S3)
      fifo_conf_reg.fifo_prt_en = 1;
#endif
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
    cpp::result<void, error_t> writeBytes(int i2c_port, const uint8_t *data, size_t length)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      if (i2c_context[i2c_port].state.has_error()) { return cpp::fail(i2c_context[i2c_port].state.error()); }
      if (i2c_context[i2c_port].state != i2c_context_t::state_write) { return cpp::fail(error_t::mode_mismatch); }
      cpp::result<void, error_t> res {};
      if (!length) return res;

      static constexpr int txfifo_limit = 32;
      auto dev = getDev(i2c_port);
      auto fifo_addr = getFifoAddr(i2c_port);
      size_t len = ((length - 1) & (txfifo_limit-1)) + 1;
      do
      {
        res = i2c_wait(i2c_port);
        if (res.has_error())
        {
          ESP_LOGE("LGFX", "i2c write error : ack wait");
          break;
        }
        size_t idx = 0;
        do
        {
          *fifo_addr = data[idx];
        } while (++idx != len);
        i2c_set_cmd(dev, 0, i2c_cmd_write, len);
        i2c_set_cmd(dev, 1, i2c_cmd_end, 0);
        updateDev(dev);
        dev->ctr.trans_start = 1;
        i2c_context[i2c_port].wait_ack = true;
        data += len;
        length -= len;
        len = txfifo_limit;
      } while (length);
      return res;
    }

    cpp::result<void, error_t> readBytes(int i2c_port, uint8_t *readdata, size_t length)
    {
      if (i2c_port >= I2C_NUM_MAX) { return cpp::fail(error_t::invalid_arg); }
      if (i2c_context[i2c_port].state.has_error()) { return cpp::fail(i2c_context[i2c_port].state.error()); }
      if (i2c_context[i2c_port].state != i2c_context_t::state_read) { return cpp::fail(error_t::mode_mismatch); }
      cpp::result<void, error_t> res {};
      if (!length) return res;

      static constexpr uint32_t intmask = I2C_ACK_ERR_INT_RAW_M | I2C_TIME_OUT_INT_RAW_M | I2C_END_DETECT_INT_RAW_M | I2C_ARBITRATION_LOST_INT_RAW_M;
      auto fifo_addr = getFifoAddr(i2c_port);
      auto dev = getDev(i2c_port);
      size_t len = 0;
#if defined ( CONFIG_IDF_TARGET_ESP32S3 )
      uint32_t us_limit = (dev->scl_high_period.scl_high_period + dev->scl_low_period.scl_low_period + 16);
#else
      uint32_t us_limit = (dev->scl_high_period.period + dev->scl_low_period.period + 16);
#endif
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
        i2c_set_cmd(dev, 0, i2c_cmd_read, len);
        i2c_set_cmd(dev, 1, i2c_cmd_end, 0);
        updateDev(dev);
        dev->ctr.trans_start = 1;
        taskYIELD();
        dev->int_clr.val = intmask;
        do
        {
          uint32_t us = lgfx::micros();
#if defined ( CONFIG_IDF_TARGET_ESP32C3 )
          while (0 == dev->sr.rx_fifo_cnt && !(dev->int_raw.val & intmask) && ((lgfx::micros() - us) <= us_limit));
          if (0 != dev->sr.rx_fifo_cnt)
#elif defined ( CONFIG_IDF_TARGET_ESP32S3 )
          while (0 == dev->sr.rxfifo_cnt && !(dev->int_raw.val & intmask) && ((lgfx::micros() - us) <= us_limit));
          if (0 != dev->sr.rxfifo_cnt)
#else
          while (0 == dev->status_reg.rx_fifo_cnt && !(dev->int_raw.val & intmask) && ((lgfx::micros() - us) <= us_limit));
          if (0 != dev->status_reg.rx_fifo_cnt)
#endif
          {
            *readdata++ = *fifo_addr; //dev->fifo_data.data;
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
