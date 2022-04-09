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
#if defined (CONFIG_IDF_TARGET_ESP32S3)
#if __has_include (<esp_lcd_panel_io.h>)
#include <esp_lcd_panel_io.h>

#include "Bus_Parallel8.hpp"
#include "../../misc/pixelcopy.hpp"

#include <hal/gpio_ll.h>
#include <hal/lcd_hal.h>
#include <soc/lcd_cam_reg.h>
#include <soc/lcd_cam_struct.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

/// from esp-idf/components/esp_lcd/src/esp_lcd_panel_io_i80.c
struct esp_lcd_i80_bus_t {
    int bus_id;            // Bus ID, index from 0
    portMUX_TYPE spinlock; // spinlock used to protect i80 bus members(hal, device_list, cur_trans)
    lcd_hal_context_t hal; // Hal object
    size_t bus_width;      // Number of data lines
    intr_handle_t intr;    // LCD peripheral interrupt handle
    void* pm_lock; // Power management lock
    size_t num_dma_nodes;  // Number of DMA descriptors
    uint8_t *format_buffer;  // The driver allocates an internal buffer for DMA to do data format transformer
    size_t resolution_hz;    // LCD_CLK resolution, determined by selected clock source
    gdma_channel_handle_t dma_chan; // DMA channel handle
};


  static __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }

  static lcd_cam_dev_t* getDev(int port)
  {
    return &LCD_CAM;
  }

  void Bus_Parallel8::config(const config_t& cfg)
  {
    _cfg = cfg;
    auto port = cfg.port;
    _dev = getDev(port);
  }

  static void _gpio_pin_init(int pin)
  {
    if (pin >= 0)
    {
      gpio_pad_select_gpio(pin);
      gpio_hi(pin);
      gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    }
  }

  bool Bus_Parallel8::init(void)
  {
    _init_pin();

    _gpio_pin_init(_cfg.pin_rd);
    _gpio_pin_init(_cfg.pin_wr);
    _gpio_pin_init(_cfg.pin_rs);

    gpio_matrix_out(_cfg.pin_rs, LCD_DC_IDX, 0, 0);
    gpio_matrix_out(_cfg.pin_wr, LCD_PCLK_IDX, 0, 0);

    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = _cfg.pin_rs,
        .wr_gpio_num = _cfg.pin_wr,
        .data_gpio_nums = {
            _cfg.pin_d0,
            _cfg.pin_d1,
            _cfg.pin_d2,
            _cfg.pin_d3,
            _cfg.pin_d4,
            _cfg.pin_d5,
            _cfg.pin_d6,
            _cfg.pin_d7,
        },
        .bus_width = 8,
        .max_transfer_bytes = 32768
    };
    esp_lcd_new_i80_bus(&bus_config, &_i80_bus);
    _dma_chan = ((esp_lcd_i80_bus_t*)_i80_bus)->dma_chan;

    for (size_t i = 0; i < 8; ++i)
    {
      gpio_ll_input_enable(&GPIO, (gpio_num_t)_cfg.pin_data[i]);
    }

    _alloc_dmadesc(1);
    return true;
  }

  void Bus_Parallel8::_init_pin(bool read)
  {
    int8_t* pins = _cfg.pin_data;
    if (read)
    {
      for (size_t i = 0; i < 8; ++i)
      {
        gpio_ll_output_disable(&GPIO, (gpio_num_t)pins[i]);
      }
    }
    else
    {
      auto idx_base = LCD_DATA_OUT0_IDX;
      for (size_t i = 0; i < 8; ++i)
      {
        gpio_matrix_out(pins[i], idx_base + i, 0, 0);
      }
    }
  }

  void Bus_Parallel8::release(void)
  {
    if (_dmadesc)
    {
      heap_caps_free(_dmadesc);
      _dmadesc = nullptr;
      _dmadesc_size = 0;
    }
  }

  void Bus_Parallel8::beginTransaction(void)
  {
    auto dev = getDev(_cfg.port);
    int clk_div = std::max(1u, 80*1000*1000 / (_cfg.freq_write+1));

    dev->lcd_clock.lcd_clkcnt_n = clk_div;
    dev->lcd_clock.lcd_clk_equ_sysclk = 0;
    dev->lcd_clock.lcd_ck_idle_edge = true;
    dev->lcd_clock.lcd_ck_out_edge = false;

    dev->lcd_misc.val = LCD_CAM_LCD_CD_IDLE_EDGE;
    // dev->lcd_misc.lcd_cd_idle_edge = 1;
    // dev->lcd_misc.lcd_cd_cmd_set = 0;
    // dev->lcd_misc.lcd_cd_dummy_set = 0;
    // dev->lcd_misc.lcd_cd_data_set = 0;

    dev->lcd_user.val = 0;
    // dev->lcd_user.lcd_byte_order = false;
    // dev->lcd_user.lcd_bit_order = false;
    // dev->lcd_user.lcd_8bits_order = false;

    _cache_flip = _cache[0];
  }

  void Bus_Parallel8::endTransaction(void)
  {
    auto dev = getDev(_cfg.port);
    while (dev->lcd_user.val & LCD_CAM_LCD_START) {}
  }

  void Bus_Parallel8::wait(void)
  {
    auto dev = getDev(_cfg.port);
    while (dev->lcd_user.val & LCD_CAM_LCD_START) {}
  }

  bool Bus_Parallel8::busy(void) const
  {
    auto dev = getDev(_cfg.port);
    return (dev->lcd_user.val & LCD_CAM_LCD_START);
  }

  bool Bus_Parallel8::writeCommand(uint32_t data, uint_fast8_t bit_length)
  {
    auto bytes = bit_length >> 3;
    auto dev = getDev(_cfg.port);
    auto reg_lcd_user = &(dev->lcd_user.val);
    dev->lcd_misc.lcd_cd_cmd_set = 1;
    do
    {
      dev->lcd_cmd_val.lcd_cmd_value = data;
      data >>= 8;
      while (*reg_lcd_user & LCD_CAM_LCD_START) {}
      *reg_lcd_user = LCD_CAM_LCD_CMD | LCD_CAM_LCD_UPDATE_REG | LCD_CAM_LCD_START;
    } while (--bytes);
    return true;
  }

  void Bus_Parallel8::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    auto bytes = bit_length >> 3;
    auto dev = getDev(_cfg.port);
    auto reg_lcd_user = &(dev->lcd_user.val);
    dev->lcd_misc.lcd_cd_cmd_set = 0;

    if (bytes & 1)
    {
      dev->lcd_cmd_val.lcd_cmd_value = data;
      data >>= 8;
      while (*reg_lcd_user & LCD_CAM_LCD_START) {}
      *reg_lcd_user = LCD_CAM_LCD_CMD | LCD_CAM_LCD_UPDATE_REG | LCD_CAM_LCD_START;
      if (0 == --bytes) { return; }
    }

    bytes >>= 1;
    do
    {
      dev->lcd_cmd_val.lcd_cmd_value = (data & 0xFF) | (data << 8);
      data >>= 16;
      while (*reg_lcd_user & LCD_CAM_LCD_START) {}
      *reg_lcd_user = LCD_CAM_LCD_CMD | LCD_CAM_LCD_CMD_2_CYCLE_EN | LCD_CAM_LCD_UPDATE_REG | LCD_CAM_LCD_START;
    } while (--bytes);
  }

  void Bus_Parallel8::writeDataRepeat(uint32_t color_raw, uint_fast8_t bit_length, uint32_t count)
  {
// do
// {
//   writeData(color_raw, bit_length);
// } while (--count);
// return;

    uint32_t regbuf0 = color_raw | color_raw << bit_length;
    uint32_t regbuf1;
    uint32_t regbuf2;
    // make 12Bytes data.
    bool bits24 = (bit_length == 24);
    if (bits24) {
      regbuf1 = regbuf0 >> 8 | regbuf0 << 16;
      regbuf2 = regbuf0 >>16 | regbuf0 <<  8;
    } else {
      if (bit_length == 8) { regbuf0 |= regbuf0 << 16; }
      regbuf1 = regbuf0;
      regbuf2 = regbuf0;
    }

    uint32_t byte_length = bit_length >> 3;
    uint32_t length = byte_length * count;
    uint32_t len = (length <= 12u) ? length : 12u;
    uint32_t* dst = (uint32_t*)_cache_flip;

    dst[0] = regbuf0;
    dst[1] = regbuf1;
    dst[2] = regbuf2;

    _cache_flip = _cache[(dst == _cache[0])];
    writeBytes((const uint8_t*)dst, len, true, true);
    if (0 == (length -= len)) return;

    if (length > 12)
    {
      memcpy((void*)&dst[ 3], dst, 12);
      if (length > 24)
      {
        memcpy((void*)&dst[ 6], dst, 24);
        if (length > 48)
        {
          memcpy((void*)&dst[12], dst, 48);
          if (length > 96)
          {
            memcpy((void*)&dst[24], dst, 96);
          }
        }
      }
    }
    do
    {
      len = (length < 192u) ? length : 192u;
      writeBytes((const uint8_t*)dst, len, true, true);
    } while (0 != (length -= len));
  }

  void Bus_Parallel8::writePixels(pixelcopy_t* param, uint32_t length)
  {
    const uint32_t bytes = param->dst_bits >> 3;
    auto fp_copy = param->fp_copy;
    const uint32_t limit = CACHE_SIZE / bytes;
    uint8_t len = length % limit;
    if (len)
    {
      auto c = _cache_flip;
      fp_copy(c, 0, len, param);
      _cache_flip = _cache[(c == _cache[0])];
      writeBytes((uint8_t*)c, len * bytes, true, true);
      if (0 == (length -= len)) return;
    }
    do
    {
      auto c = _cache_flip;
      fp_copy(_cache_flip, 0, limit, param);
      _cache_flip = _cache[(c == _cache[0])];
      writeBytes((uint8_t*)c, limit * bytes, true, true);
    } while (length -= limit);
  }

  void Bus_Parallel8::writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma)
  {
    if (length <= 4)
    {
      if (dc)
      {
        writeData(*(uint32_t*)data, length << 3);
        return;
      }
      else
      {
        writeCommand(*(uint32_t*)data, length << 3);
        return;
      }
    }
    auto dev = getDev(_cfg.port);

    // if (use_dma)
    {
      auto reg_lcd_user = &(dev->lcd_user.val);
      dev->lcd_misc.lcd_cd_cmd_set  = !dc;
      dev->lcd_cmd_val.lcd_cmd_value = data[0] | data[1] << 16;
      while (*reg_lcd_user & LCD_CAM_LCD_START) {}
      *reg_lcd_user = LCD_CAM_LCD_CMD | LCD_CAM_LCD_CMD_2_CYCLE_EN | LCD_CAM_LCD_UPDATE_REG | LCD_CAM_LCD_START;
      _setup_dma_desc_links(&data[4], length - 4);
      gdma_start(_dma_chan, (intptr_t)(_dmadesc));
      dev->lcd_misc.lcd_cd_data_set = !dc;
      dev->lcd_cmd_val.lcd_cmd_value = data[2] | data[3] << 16;
      while (*reg_lcd_user & LCD_CAM_LCD_START) {}
      *reg_lcd_user = 1 | LCD_CAM_LCD_ALWAYS_OUT_EN | LCD_CAM_LCD_DOUT | LCD_CAM_LCD_CMD | LCD_CAM_LCD_CMD_2_CYCLE_EN | LCD_CAM_LCD_UPDATE_REG;
      *reg_lcd_user = 1 | LCD_CAM_LCD_ALWAYS_OUT_EN | LCD_CAM_LCD_DOUT | LCD_CAM_LCD_CMD | LCD_CAM_LCD_CMD_2_CYCLE_EN | LCD_CAM_LCD_START;
      return;
    }
  }

  void Bus_Parallel8::beginRead(void)
  {
    wait();
    _init_pin(true);
    gpio_lo(_cfg.pin_rd);
    gpio_matrix_out(_cfg.pin_rs, 0x100, 0, 0);
  }

  void Bus_Parallel8::endRead(void)
  {
    gpio_hi(_cfg.pin_rd);
    _init_pin();
    gpio_matrix_out(_cfg.pin_rs, LCD_DC_IDX, 0, 0);
  }

  void Bus_Parallel8::_read_bytes(uint8_t* dst, uint32_t length)
  {
    union
    {
      uint32_t in32[2];
      uint8_t in[8];
    };

    uint32_t mask = (((((((((((((((
                    (_cfg.pin_d0 & 7)) << 3)
                  + (_cfg.pin_d1 & 7)) << 3)
                  + (_cfg.pin_d2 & 7)) << 3)
                  + (_cfg.pin_d3 & 7)) << 3)
                  + (_cfg.pin_d4 & 7)) << 3)
                  + (_cfg.pin_d5 & 7)) << 3)
                  + (_cfg.pin_d6 & 7)) << 3)
                  + (_cfg.pin_d7 & 7))
                  ;

    uint32_t idx = (((((((((((((((
                    (_cfg.pin_d0 >> 3)) << 3)
                  + (_cfg.pin_d1 >> 3)) << 3)
                  + (_cfg.pin_d2 >> 3)) << 3)
                  + (_cfg.pin_d3 >> 3)) << 3)
                  + (_cfg.pin_d4 >> 3)) << 3)
                  + (_cfg.pin_d5 >> 3)) << 3)
                  + (_cfg.pin_d6 >> 3)) << 3)
                  + (_cfg.pin_d7 >> 3))
                  ;

    auto reg_rd_h = get_gpio_hi_reg(_cfg.pin_rd);
    auto reg_rd_l = get_gpio_lo_reg(_cfg.pin_rd);
    uint32_t mask_rd = 1ul << (_cfg.pin_rd & 31);
    uint_fast8_t val;
    do
    {
      in32[0] = GPIO.in;
      in32[1] = GPIO.in1.val;
      *reg_rd_h = mask_rd;
      val =              (1 & (in[(idx >>  0) & 7] >> ((mask >>  0) & 7)));
      val = (val << 1) + (1 & (in[(idx >>  3) & 7] >> ((mask >>  3) & 7)));
      val = (val << 1) + (1 & (in[(idx >>  6) & 7] >> ((mask >>  6) & 7)));
      val = (val << 1) + (1 & (in[(idx >>  9) & 7] >> ((mask >>  9) & 7)));
      val = (val << 1) + (1 & (in[(idx >> 12) & 7] >> ((mask >> 12) & 7)));
      *reg_rd_l = mask_rd;
      val = (val << 1) + (1 & (in[(idx >> 15) & 7] >> ((mask >> 15) & 7)));
      val = (val << 1) + (1 & (in[(idx >> 18) & 7] >> ((mask >> 18) & 7)));
      val = (val << 1) + (1 & (in[(idx >> 21) & 7] >> ((mask >> 21) & 7)));
      *dst++ = val;
    } while (--length);
  }

  uint32_t Bus_Parallel8::readData(uint_fast8_t bit_length)
  {
    union {
      uint32_t res;
      uint8_t raw[4];
    };
    _read_bytes(raw, (bit_length + 7) >> 3);
    return res;
  }

  bool Bus_Parallel8::readBytes(uint8_t* dst, uint32_t length, bool use_dma)
  {
    _read_bytes(dst, length);
    return true;
  }

  void Bus_Parallel8::readPixels(void* dst, pixelcopy_t* param, uint32_t length)
  {
    uint32_t _regbuf[8];
    const auto bytes = param->src_bits >> 3;
    uint32_t limit = (bytes == 2) ? 16 : 10;
    param->src_data = _regbuf;
    int32_t dstindex = 0;
    do {
      uint32_t len2 = (limit > length) ? length : limit;
      length -= len2;

      _read_bytes((uint8_t*)_regbuf, len2 * bytes);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len2, param);
    } while (length);
  }

  void Bus_Parallel8::_alloc_dmadesc(size_t len)
  {
    if (_dmadesc) heap_caps_free(_dmadesc);
    _dmadesc_size = len;
    _dmadesc = (dma_descriptor_t*)heap_caps_malloc(sizeof(dma_descriptor_t) * len, MALLOC_CAP_DMA);
  }

  void Bus_Parallel8::_setup_dma_desc_links(const uint8_t *data, int32_t len)
  {
    static constexpr size_t MAX_DMA_LEN = (4096-4);

    if (_dmadesc_size * MAX_DMA_LEN < len)
    {
      _alloc_dmadesc(len / MAX_DMA_LEN + 1);
    }
    dma_descriptor_t *dmadesc = _dmadesc;

    while (len > MAX_DMA_LEN)
    {
      len -= MAX_DMA_LEN;
      dmadesc->buffer = (uint8_t *)data;
      data += MAX_DMA_LEN;
      *(uint32_t*)dmadesc = MAX_DMA_LEN | MAX_DMA_LEN << 12 | 0x80000000;
      dmadesc->next = dmadesc + 1;
      dmadesc++;
    }
    *(uint32_t*)dmadesc = ((len + 3) & ( ~3 )) | len << 12 | 0xC0000000;
    dmadesc->buffer = (uint8_t *)data;
    dmadesc->next = nullptr;
  }

//----------------------------------------------------------------------------
 }
}

#endif
#endif
#endif
