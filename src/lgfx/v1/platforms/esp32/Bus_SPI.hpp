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
#pragma once

#include <cstring>

#if __has_include(<esp32/rom/lldesc.h>)
 #include <esp32/rom/lldesc.h>
#else
 #include <rom/lldesc.h>
#endif

#if __has_include(<driver/spi_common_internal.h>)
 #include <driver/spi_common_internal.h>
#endif

#include <driver/spi_common.h>
#include <soc/spi_reg.h>

#include "../../Bus.hpp"
#include "../common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Bus_SPI : public IBus
  {
  public:
    struct config_t
    {
      // max80MHz , 40MHz , 26.67MHz , 20MHz , 16MHz , and more ...
      std::uint32_t freq_write = 16000000;
      std::uint32_t freq_read  =  8000000;
      std::int16_t pin_sclk = -1;
      std::int16_t pin_miso = -1;
      std::int16_t pin_mosi = -1;
      std::int16_t pin_dc   = -1;
      std::uint8_t spi_mode = 0;
      bool spi_3wire = true;
      bool use_lock = true;
#if defined (CONFIG_IDF_TARGET_ESP32S2)
      spi_host_device_t spi_host = SPI3_HOST;
#else
      spi_host_device_t spi_host = VSPI_HOST;
      std::uint8_t dma_channel = 0;
#endif
    };

    constexpr Bus_SPI(void) = default;

    const config_t& config(void) const { return _cfg; }

    void config(const config_t& config);

    bus_type_t busType(void) const override { return bus_type_t::bus_spi; }

    void init(void) override;
    void release(void) override;

    void beginTransaction(void) override;
    void endTransaction(void) override;
    void wait(void) override;
    bool busy(void) const override;

    void flush(void) override {}
    bool writeCommand(std::uint32_t data, std::uint_fast8_t bit_length) override;
    void writeData(std::uint32_t data, std::uint_fast8_t bit_length) override;
    void writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t count) override;
    void writePixels(pixelcopy_t* pc, std::uint32_t length) override;
    void writeBytes(const std::uint8_t* data, std::uint32_t length, bool dc, bool use_dma) override;

    void initDMA(void) override;
    void addDMAQueue(const std::uint8_t* data, std::uint32_t length) override;
    void execDMAQueue(void) override;
    std::uint8_t* getDMABuffer(std::uint32_t length) override { return _flip_buffer.getBuffer(length); }

    void beginRead(void) override;
    void endRead(void) override;
    std::uint32_t readData(std::uint_fast8_t bit_length) override;
    bool readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma) override;
    void readPixels(void* dst, pixelcopy_t* pc, std::uint32_t length) override;

  private:

    static __attribute__ ((always_inline)) inline volatile std::uint32_t* reg(std::uint32_t addr) { return (volatile std::uint32_t *)ETS_UNCACHED_ADDR(addr); }
    __attribute__ ((always_inline)) inline void dc_control(bool flg) { auto reg = _gpio_reg_dc[flg]; auto mask = _mask_reg_dc; auto cmd = _spi_cmd_reg; while (*cmd & SPI_USR); *reg = mask; }
    __attribute__ ((always_inline)) inline void exec_spi(void) {        *_spi_cmd_reg = SPI_USR; }
    __attribute__ ((always_inline)) inline void wait_spi(void) { while (*_spi_cmd_reg & SPI_USR); }
    __attribute__ ((always_inline)) inline void set_write_len(std::uint32_t bitlen) { *_spi_mosi_dlen_reg = bitlen - 1; }
    __attribute__ ((always_inline)) inline void set_read_len( std::uint32_t bitlen) { *reg(SPI_MISO_DLEN_REG(_spi_port)) = bitlen - 1; }

    void _alloc_dmadesc(size_t len);
    void _spi_dma_reset(void);
    void _setup_dma_desc_links(const std::uint8_t *data, std::int32_t len);

    config_t _cfg;
    FlipBuffer _flip_buffer;
    volatile std::uint32_t* _gpio_reg_dc[2] = { nullptr, nullptr };
    volatile std::uint32_t* _spi_mosi_dlen_reg = nullptr;
    volatile std::uint32_t* _spi_w0_reg = nullptr;
    volatile std::uint32_t* _spi_cmd_reg = nullptr;
    volatile std::uint32_t* _spi_user_reg = nullptr;
    volatile std::uint32_t* _spi_dma_out_link_reg = nullptr;
    std::uint32_t _last_freq_apb = 0;
    std::uint32_t _clkdiv_write = 0;
    std::uint32_t _clkdiv_read = 0;
    std::uint32_t _user_reg = 0;
    std::uint32_t _mask_reg_dc = 0;
    std::uint32_t _dma_queue_bytes = 0;
    lldesc_t* _dmadesc = nullptr;
    std::uint32_t _dmadesc_size = 0;
    lldesc_t* _dma_queue = nullptr;
    std::uint32_t _dma_queue_size = 0;
    std::uint32_t _dma_queue_capacity = 0;
    std::uint8_t _spi_port = 0;
    bool _next_dma_reset = false;
    bool _inited = false;
  };

//----------------------------------------------------------------------------
 }
}
