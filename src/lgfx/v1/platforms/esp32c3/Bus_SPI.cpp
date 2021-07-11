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
#if defined (CONFIG_IDF_TARGET_ESP32C3)

#include "Bus_SPI.hpp"

#include "../../misc/pixelcopy.hpp"

#include <driver/periph_ctrl.h>
#include <driver/rtc_io.h>
#include <soc/spi_reg.h>
#include <esp_heap_caps.h>
#include <esp_log.h>

#if defined (ARDUINO) // Arduino ESP32
 #include <soc/periph_defs.h>
 #include <esp32-hal-cpu.h>
#else
 #include <driver/spi_master.h>
#endif

#if defined (CONFIG_IDF_TARGET_ESP32S2) || defined (CONFIG_IDF_TARGET_ESP32C3)
 #ifndef SPI_PIN_REG
 #define SPI_PIN_REG SPI_MISC_REG
 #endif
#endif

#include "common.hpp"

#include <algorithm>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Bus_SPI::config(const config_t& cfg)
  {
    _cfg = cfg;

    auto spi_port = (uint32_t)(cfg.spi_host) + 1;  // FSPI=1  HSPI=2  VSPI=3;
    _spi_port = spi_port;
    _spi_w0_reg           = reg(SPI_W0_REG(          spi_port));
    _spi_cmd_reg          = reg(SPI_CMD_REG(         spi_port));
    _spi_user_reg         = reg(SPI_USER_REG(        spi_port));
    _spi_mosi_dlen_reg    = reg(SPI_MOSI_DLEN_REG(   spi_port));
  //_spi_dma_out_link_reg = reg(SPI_DMA_OUT_LINK_REG(spi_port));
    _mask_reg_dc = (cfg.pin_dc < 0) ? 0 : (1ul << (cfg.pin_dc & 31));
    _gpio_reg_dc[0] = get_gpio_lo_reg(cfg.pin_dc);
    _gpio_reg_dc[1] = get_gpio_hi_reg(cfg.pin_dc);
    _last_freq_apb = 0;

    auto spi_mode = cfg.spi_mode;
    _user_reg = (spi_mode == 1 || spi_mode == 2) ? SPI_CK_OUT_EDGE | SPI_USR_MOSI : SPI_USR_MOSI;

//ESP_LOGI("LGFX","Bus_SPI::config  spi_port:%d  dc:%0d %02x", spi_port, _cfg.pin_dc, _mask_reg_dc);
  }

  bool Bus_SPI::init(void)
  {
//ESP_LOGI("LGFX","Bus_SPI::init");
    dc_control(true);
    pinMode(_cfg.pin_dc, pin_mode_t::output);
    _inited = spi::init(_cfg.spi_host, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi, _cfg.dma_channel).has_value();
    return _inited;
  }

  void Bus_SPI::release(void)
  {
//ESP_LOGI("LGFX","Bus_SPI::release");
    pinMode(_cfg.pin_dc  , pin_mode_t::input);
    pinMode(_cfg.pin_mosi, pin_mode_t::input);
    pinMode(_cfg.pin_miso, pin_mode_t::input);
    pinMode(_cfg.pin_sclk, pin_mode_t::input);
    if (!_inited) return;
    _inited = false;
    spi::release(_cfg.spi_host);
  }

  void Bus_SPI::beginTransaction(void)
  {
//ESP_LOGI("LGFX","Bus_SPI::beginTransaction 1");
    std::uint32_t freq_apb = getApbFrequency();
    std::uint32_t clkdiv_write = _clkdiv_write;
    if (_last_freq_apb != freq_apb)
    {
      _last_freq_apb = freq_apb;
      _clkdiv_read = FreqToClockDiv(freq_apb, _cfg.freq_read);
      clkdiv_write = FreqToClockDiv(freq_apb, _cfg.freq_write);
      _clkdiv_write = clkdiv_write;
      dc_control(true);
      pinMode(_cfg.pin_dc, pin_mode_t::output);
    }
    _next_dma_reset = true;

    auto spi_mode = _cfg.spi_mode;
    std::uint32_t pin  = (spi_mode & 2) ? SPI_CK_IDLE_EDGE : 0;

    if (_cfg.use_lock) spi::beginTransaction(_cfg.spi_host);

    *_spi_user_reg = _user_reg;
    auto spi_port = _spi_port;

    *reg(SPI_MISC_REG(spi_port)) = pin;
    *reg(SPI_CLOCK_REG(spi_port)) = clkdiv_write;
    *_spi_cmd_reg = SPI_UPDATE;
  }

  void Bus_SPI::endTransaction(void)
  {
    dc_control(true);
    if (_cfg.use_lock) spi::endTransaction(_cfg.spi_host);
#if defined (ARDUINO) // Arduino ESP32
    *_spi_user_reg = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN; // for other SPI device (e.g. SD card)
    *_spi_cmd_reg = SPI_UPDATE;
#endif
  }

  void Bus_SPI::wait(void)
  {
    auto spi_cmd_reg = _spi_cmd_reg;
    while (*spi_cmd_reg & SPI_USR);
  }

  bool Bus_SPI::busy(void) const
  {
    return (*_spi_cmd_reg & SPI_USR);
  }

  bool Bus_SPI::writeCommand(std::uint32_t data, std::uint_fast8_t bit_length)
  {
//ESP_LOGI("LGFX","writeCmd: %02x  len:%d   dc:%02x", data, bit_length, _mask_reg_dc);
    auto spi_mosi_dlen_reg = _spi_mosi_dlen_reg;
    auto spi_w0_reg = _spi_w0_reg;
    auto spi_cmd_reg = _spi_cmd_reg;
    auto gpio_reg_dc = _gpio_reg_dc[0];
    auto mask_reg_dc = _mask_reg_dc;
    --bit_length;
    while (*spi_cmd_reg & SPI_USR);    // wait SPI
    *spi_mosi_dlen_reg = bit_length;   // set bitlength
//    *spi_cmd_reg = SPI_UPDATE;
    *spi_w0_reg = data;                // set data
    *gpio_reg_dc = mask_reg_dc;        // D/C
    //while (*spi_cmd_reg & SPI_UPDATE);
    *spi_cmd_reg = SPI_EXECUTE;            // exec SPI
    return true;
  }

  void Bus_SPI::writeData(std::uint32_t data, std::uint_fast8_t bit_length)
  {
//ESP_LOGI("LGFX","writeData: %02x  len:%d", data, bit_length);
    auto spi_mosi_dlen_reg = _spi_mosi_dlen_reg;
    auto spi_w0_reg = _spi_w0_reg;
    auto spi_cmd_reg = _spi_cmd_reg;
    auto gpio_reg_dc = _gpio_reg_dc[1];
    auto mask_reg_dc = _mask_reg_dc;
    --bit_length;
    while (*spi_cmd_reg & SPI_USR);    // wait SPI
    *spi_mosi_dlen_reg = bit_length;   // set bitlength
//    *spi_cmd_reg = SPI_UPDATE;
    *spi_w0_reg = data;                // set data
    *gpio_reg_dc = mask_reg_dc;        // D/C
    //while (*spi_cmd_reg & SPI_UPDATE);
    *spi_cmd_reg = SPI_EXECUTE;            // exec SPI
  }

  void Bus_SPI::writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t count)
  {
    auto spi_mosi_dlen_reg = _spi_mosi_dlen_reg;
    auto spi_w0_reg = _spi_w0_reg;
    auto spi_cmd_reg = _spi_cmd_reg;
    auto gpio_reg_dc = _gpio_reg_dc[1];
    auto mask_reg_dc = _mask_reg_dc;
    if (1 == count)
    {
      --bit_length;
      while (*spi_cmd_reg & SPI_USR);    // wait SPI
      *spi_mosi_dlen_reg = bit_length;   // set bitlength
      *spi_w0_reg = data;                // set data
      *gpio_reg_dc = mask_reg_dc;        // D/C high (data)
      *spi_cmd_reg = SPI_EXECUTE;            // exec SPI
      return;
    }

    std::uint32_t regbuf0 = data | data << bit_length;
    std::uint32_t regbuf1;
    std::uint32_t regbuf2;
    // make 12Bytes data.
    bool bits24 = (bit_length == 24);
    if (bits24) {
      regbuf1 = regbuf0 >> 8 | regbuf0 << 16;
      regbuf2 = regbuf0 >>16 | regbuf0 <<  8;
    } else {
      regbuf1 = regbuf0;
      regbuf2 = regbuf0;
    }

    std::uint32_t length = bit_length * count;          // convert to bitlength.
    std::uint32_t len = (length <= 96u) ? length : (length <= 144u) ? 48u : 96u; // 1st send length = max 12Byte (96bit).

    length -= len;
    --len;

    while (*spi_cmd_reg & SPI_USR);  // wait SPI
    *spi_mosi_dlen_reg = len;
    // copy to SPI buffer register
    spi_w0_reg[0] = regbuf0;
    spi_w0_reg[1] = regbuf1;
    spi_w0_reg[2] = regbuf2;
    *gpio_reg_dc = mask_reg_dc;      // D/C high (data)
    *spi_cmd_reg = SPI_EXECUTE;          // exec SPI
    if (0 == length) return;

    std::uint32_t regbuf[7] = { regbuf0, regbuf1, regbuf2, regbuf0, regbuf1, regbuf2, regbuf0 } ;

    // copy to SPI buffer register
    memcpy((void*)&spi_w0_reg[3], regbuf, 24);
    memcpy((void*)&spi_w0_reg[9], regbuf, 28);

    // limit = 64Byte / depth_bytes;
    // When 24bit color, 504 bits out of 512bit buffer are used.
    // When 16bit color, it uses exactly 512 bytes. but, it behaves like a ring buffer, can specify a larger size.
    std::uint32_t limit;
    if (bits24)
    {
      limit = 504;
      len = length % limit;
    }
    else
    {
#if defined( CONFIG_IDF_TARGET_ESP32 )
      limit = (1 << 11);
#else
      limit = (1 << 9);
#endif
      len = length & (limit - 1);
    }

    if (len)
    {
      length -= len;
      --len;
      while (*spi_cmd_reg & SPI_USR);
      *spi_mosi_dlen_reg = len;
      *spi_cmd_reg = SPI_EXECUTE;
      if (0 == length) return;
    }

    while (*spi_cmd_reg & SPI_USR);
    *spi_mosi_dlen_reg = limit - 1;
    *spi_cmd_reg = SPI_EXECUTE;
    while (length -= limit)
    {
      while (*spi_cmd_reg & SPI_USR);
      *spi_cmd_reg = SPI_USR;
    }
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, std::uint32_t length)
  {
    const std::uint8_t bytes = param->dst_bits >> 3;

    const std::uint32_t limit = (bytes == 2) ? 16 : 10; //  limit = 32/bytes (bytes==2 is 16   bytes==3 is 10)
    std::uint32_t len = (length - 1) / limit;
    std::uint32_t highpart = (len & 1) << 3;
    len = length - (len * limit);
    std::uint32_t regbuf[8];
    param->fp_copy(regbuf, 0, len, param);

    auto spi_w0_reg = _spi_w0_reg;

    std::uint32_t user_reg = *_spi_user_reg;

    dc_control(true);
    set_write_len(len * bytes << 3);

    memcpy((void*)&spi_w0_reg[highpart], regbuf, (len * bytes + 3) & (~3));
    if (highpart) *_spi_user_reg = user_reg | SPI_USR_MOSI_HIGHPART;
    exec_spi();
    if (0 == (length -= len)) return;

    for (; length; length -= limit)
    {
      param->fp_copy(regbuf, 0, limit, param);
      memcpy((void*)&spi_w0_reg[highpart ^= 0x08], regbuf, limit * bytes);
      std::uint32_t user = user_reg;
      if (highpart) user |= SPI_USR_MOSI_HIGHPART;
      if (len != limit)
      {
        len = limit;
        wait_spi();
        set_write_len(limit * bytes << 3);
        *_spi_user_reg = user;
        exec_spi();
      }
      else
      {
        wait_spi();
        *_spi_user_reg = user;
        exec_spi();
      }
    }
  }

  void Bus_SPI::writeBytes(const std::uint8_t* data, std::uint32_t length, bool dc, bool use_dma)
  {
    std::uint32_t buf[16];
    constexpr std::uint32_t limit = 64;
    std::uint32_t len = ((length - 1) & 0x3F) + 1;

    auto spi_w0_reg = _spi_w0_reg;

    memcpy(buf, data, len);
    dc_control(dc);
    set_write_len(len << 3);

    memcpy((void*)spi_w0_reg, buf, (len + 3) & (~3));
    exec_spi();
    if (0 == (length -= len)) return;

    for (; length; length -= limit)
    {
      data += len;
      memcpy(buf, data, limit);
      if (len != limit)
      {
        len = limit;
        wait_spi();
        set_write_len(limit << 3);
        memcpy((void*)spi_w0_reg, buf, limit);
        exec_spi();
      }
      else
      {
        wait_spi();
        memcpy((void*)spi_w0_reg, buf, limit);
        exec_spi();
      }
    }
  }

  void Bus_SPI::initDMA(void)
  {
  }

  void Bus_SPI::addDMAQueue(const std::uint8_t* data, std::uint32_t length)
  {
    writeBytes(data, length, true, true);
  }

  void Bus_SPI::execDMAQueue(void) {}

  void Bus_SPI::beginRead(void)
  {
    std::uint32_t pin = (_cfg.spi_mode & 2) ? SPI_CK_IDLE_EDGE : 0;
    std::uint32_t user = ((_cfg.spi_mode == 1 || _cfg.spi_mode == 2) ? SPI_CK_OUT_EDGE | SPI_USR_MISO : SPI_USR_MISO)
                        | (_cfg.spi_3wire ? SPI_SIO : 0);
    dc_control(true);
    *_spi_user_reg = user;
    *reg(SPI_MISC_REG(_spi_port)) = pin;
    *reg(SPI_CLOCK_REG(_spi_port)) = _clkdiv_read;
    *_spi_cmd_reg = SPI_UPDATE;
  }

  void Bus_SPI::endRead(void)
  {
    std::uint32_t pin = (_cfg.spi_mode & 2) ? SPI_CK_IDLE_EDGE : 0;
    *reg(SPI_MISC_REG(_spi_port)) = pin;
    *reg(SPI_CLOCK_REG(_spi_port)) = _clkdiv_write;
    *_spi_user_reg = _user_reg;
    *_spi_cmd_reg = SPI_UPDATE;
  }

  std::uint32_t Bus_SPI::readData(std::uint_fast8_t bit_length)
  {
    set_read_len(bit_length);
    auto spi_cmd_reg = _spi_cmd_reg;
    *spi_cmd_reg = SPI_EXECUTE;
    auto spi_w0_reg = _spi_w0_reg;
    while (*spi_cmd_reg & SPI_USR);
    return *spi_w0_reg;
  }

  bool Bus_SPI::readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma)
  {
ESP_LOGI("LGFX","Bus_SPI::readBytes");
    {
      std::int32_t len1 = std::min<std::uint32_t>(length, 32);  // 32 Byte read.
      std::int32_t len2 = len1;
      wait_spi();
      set_read_len(len1 << 3);
      exec_spi();
      std::uint32_t highpart = 8;
      std::uint32_t userreg = *_spi_user_reg;
      auto spi_w0_reg = _spi_w0_reg;
      do {
        if (0 == (length -= len1)) {
          len2 = len1;
          wait_spi();
          *_spi_user_reg = userreg;
        } else {
          std::uint32_t user = userreg;
          if (highpart) user = userreg | SPI_USR_MISO_HIGHPART;
          if (length < len1) {
            len1 = length;
            wait_spi();
            set_read_len(len1 << 3);
          } else {
            wait_spi();
          }
          *_spi_user_reg = user;
          exec_spi();
        }
        memcpy(dst, (void*)&spi_w0_reg[highpart ^= 8], len2);
        dst += len2;
      } while (length);
    }
    return true;
  }

  void Bus_SPI::readPixels(void* dst, pixelcopy_t* param, std::uint32_t length)
  {
    std::uint32_t len1 = std::min<std::uint32_t>(length, 10); // 10 pixel read
    std::uint32_t len2 = len1;
    auto len_read_pixel = param->src_bits;
    std::uint32_t regbuf[8];
    wait_spi();
    set_read_len(len_read_pixel * len1);
    exec_spi();
    param->src_data = regbuf;
    std::int32_t dstindex = 0;
    auto spi_w0_reg = _spi_w0_reg;
    do {
      if (0 == (length -= len1)) {
        len2 = len1;
        wait_spi();
        memcpy(regbuf, (void*)spi_w0_reg, len2 * len_read_pixel >> 3);
      } else {
        if (length < len1) {
          len1 = length;
          wait_spi();
          set_read_len(len_read_pixel * len1);
        } else {
          wait_spi();
        }
        memcpy(regbuf, (void*)spi_w0_reg, len2 * len_read_pixel >> 3);
        exec_spi();
      }
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len2, param);
    } while (length);
  }

  void Bus_SPI::_alloc_dmadesc(size_t len)
  {
    if (_dmadesc) heap_caps_free(_dmadesc);
    _dmadesc_size = len;
    _dmadesc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t) * len, MALLOC_CAP_DMA);
  }

  void Bus_SPI::_spi_dma_reset(void)
  {
    _next_dma_reset = false;
  }

  void Bus_SPI::_setup_dma_desc_links(const std::uint8_t *data, std::int32_t len)
  {          //spicommon_setup_dma_desc_links
  }

//----------------------------------------------------------------------------
 }
}

#endif
#endif
