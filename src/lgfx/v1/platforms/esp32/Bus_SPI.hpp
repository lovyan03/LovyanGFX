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

#include <string.h>

#include <hal/dma_types.h>
#if __has_include(<soc/gdma_channel.h>)
 #include <soc/gdma_channel.h>
#elif __has_include(<hal/gdma_channel.h>)
 #include <hal/gdma_channel.h>
#endif
// On chips whose SPI DMA goes through AXI (ESP32-P4) a descriptor is 16 bytes (8-byte aligned), as IDF's spi_dma_desc_t.
// MALLOC_CAP_DMA allocations satisfy the alignment (IDF aligns them to the cache line).
#if defined ( SOC_GDMA_BUS_AXI ) && defined ( SOC_GDMA_TRIG_PERIPH_SPI2_BUS ) && ( SOC_GDMA_TRIG_PERIPH_SPI2_BUS == SOC_GDMA_BUS_AXI )
 #define LGFX_SPI_DMA_DESC_ALIGN8
#endif

#if __has_include(<esp_private/spi_common_internal.h>)
 // ESP-IDF v5
 #pragma GCC diagnostic push
 #if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic ignored "-Wliteral-suffix"
 #endif
 #include <esp_private/spi_common_internal.h>
 #pragma GCC diagnostic pop
#elif __has_include(<driver/spi_common_internal.h>)
 // ESP-IDF v4
 #include <driver/spi_common_internal.h>
#endif

#include <driver/spi_common.h>
#include <soc/spi_reg.h>

#if __has_include(<esp_idf_version.h>)
 #include <esp_idf_version.h>
 #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
  #define LGFX_ESP32_SPI_DMA_CH SPI_DMA_CH_AUTO
 #endif
#endif

#ifndef LGFX_ESP32_SPI_DMA_CH
 #define LGFX_ESP32_SPI_DMA_CH 0
#endif


// Whether the chip has Segmented-Configure-Transfer (SCT).
// One SPI transaction is limited to an 18-bit length (32 KB); longer transfers otherwise need the CPU to wait for
// and restart each segment. With SCT a CONF buffer placed in the DMA chain lets the hardware move to the next
// segment by itself, so the CPU starts the transfer only once.
// Targets chips with both SCT and GDMA (S3 / C3 / C6 / C2 / H2). S2 has SCT but a different DMA, so it is excluded.
// This decides the class layout, so only SDK headers are used (a user macro could make sizeof(Bus_SPI) differ
// between translation units). To disable SCT, define LGFX_SPI_SCT_DISABLE for the translation unit that includes
// Bus_SPI.inl (only the runtime flag _sct_ok becomes false; the layout does not change).
#if __has_include(<soc/soc_caps.h>)
 #include <soc/soc_caps.h>
#endif
#if __has_include(<esp_idf_version.h>)
 #include <esp_idf_version.h>
#endif
#undef LGFX_SPI_SCT          // Internal. Overridden by the SDK check even if given from outside (avoids a redefinition warning)
#undef LGFX_SPI_SCT_PERIPH   // (host): whether that SPI host supports SCT
#if defined ( SOC_GDMA_SUPPORTED )
 #if defined ( SOC_SPI_SCT_SUPPORTED ) && SOC_SPI_SCT_SUPPORTED && defined ( SOC_SPI_SCT_SUPPORTED_PERIPH )
  #define LGFX_SPI_SCT_PERIPH(host) SOC_SPI_SCT_SUPPORTED_PERIPH(host)
 #elif !defined ( SOC_SPI_SCT_SUPPORTED ) && defined ( ESP_IDF_VERSION_VAL ) \
    && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 1, 0) && __has_include(<hal/spi_ll.h>)
  // ESP-IDF 6.1 moved the SCT capability from soc_caps.h to hal/spi_ll.h (SPI_LL_PERIPH_HAS_SCT).
  #include <hal/spi_ll.h>
  #if defined ( SPI_LL_PERIPH_HAS_SCT )
   #define LGFX_SPI_SCT_PERIPH(host) SPI_LL_PERIPH_HAS_SCT(host)
  #endif
 #endif
 #if defined ( LGFX_SPI_SCT_PERIPH )
  #define LGFX_SPI_SCT
 #endif
#endif

#include "../../Bus.hpp"
#include "../common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Bus_SPI : public IBus
  {
#if defined ( LGFX_SPI_DMA_DESC_ALIGN8 )
    using dma_desc_t = dma_descriptor_align8_t;
#else
    using dma_desc_t = dma_descriptor_t;
#endif
#if defined ( CONFIG_IDF_TARGET_ESP32P4 )
    static_assert(sizeof(dma_desc_t) == 16, "ESP32-P4 SPI DMA descriptor must be 16 bytes");
#endif
#if defined ( SPI_UPDATE )
    static constexpr uint32_t SPI_EXECUTE = SPI_USR | SPI_UPDATE;
    #define SPI_MOSI_DLEN_REG(i) (REG_SPI_BASE(i) + 0x1C)
    #define SPI_MISO_DLEN_REG(i) (REG_SPI_BASE(i) + 0x1C)
#else
    static constexpr uint32_t SPI_EXECUTE = SPI_USR;
#endif
  public:
    struct config_t
    {
      // max80MHz , 40MHz , 26.67MHz , 20MHz , 16MHz , and more ...
      uint32_t freq_write = 16000000;
      uint32_t freq_read  =  8000000;
      int16_t pin_sclk = -1;
      int16_t pin_miso = -1;
      int16_t pin_mosi = -1;
      int16_t pin_dc   = -1;
      int16_t pin_io0  = -1;// Quad spi
      int16_t pin_io1  = -1;// Quad spi
      int16_t pin_io2  = -1;// Quad spi
      int16_t pin_io3  = -1;// Quad spi
      uint8_t spi_mode = 0;
      bool spi_3wire = true;
      bool use_lock = true;
      uint8_t dma_channel = LGFX_ESP32_SPI_DMA_CH;
#if !defined (CONFIG_IDF_TARGET) || defined (CONFIG_IDF_TARGET_ESP32)
     #if defined (VSPI_HOST)
       spi_host_device_t spi_host = VSPI_HOST;
     #elif defined (SPI3_HOST)
       spi_host_device_t spi_host = SPI3_HOST;
     #else
       spi_host_device_t spi_host = SPI2_HOST;
     #endif
#else
      spi_host_device_t spi_host = SPI2_HOST;
#endif
    };

    constexpr Bus_SPI(void) = default;
#if defined (ARDUINO) && (defined (CONFIG_IDF_TARGET_ESP32P4) \
 || defined (CONFIG_IDF_TARGET_ESP32C5) || defined (CONFIG_IDF_TARGET_ESP32C6) \
 || defined (CONFIG_IDF_TARGET_ESP32C61))
    ~Bus_SPI(void) override;
#endif

    const config_t& config(void) const { return _cfg; }

    void config(const config_t& config);

    bus_type_t busType(void) const override { return bus_type_t::bus_spi; }

    bool init(void) override;
    void release(void) override;

    void beginTransaction(void) override;
    void endTransaction(void) override;
    void wait(void) override;
    bool busy(void) const override;
    uint32_t getClock(void) const override { return _cfg.freq_write; }
    void setClock(uint32_t freq) override { if (_cfg.freq_write != freq) { _cfg.freq_write = freq; _last_freq_apb = 0; } }
    uint32_t getReadClock(void) const override { return _cfg.freq_read; }
    void setReadClock(uint32_t freq) override { if (_cfg.freq_read != freq) { _cfg.freq_read = freq; _last_freq_apb = 0; } }

    void flush(void) override {}
    bool writeCommand(uint32_t data, uint_fast8_t bit_length) override;
    void writeData(uint32_t data, uint_fast8_t bit_length) override;
    void writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t count) override;
    void writePixels(pixelcopy_t* pc, uint32_t length) override;
    void writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma) override;

    void initDMA(void) override {}
    void addDMAQueue(const uint8_t* data, uint32_t length) override;
    void execDMAQueue(void) override;
    uint8_t* getDMABuffer(uint32_t length) override { return _flip_buffer.getBuffer(length); }
    bool reserveDMABuffer(uint32_t length) override { return _flip_buffer.reserve(length); }

    void beginRead(uint_fast8_t dummy_bits) override;
    void beginRead(void) override;
    void endRead(void) override;
    uint32_t readData(uint_fast8_t bit_length) override;
    bool readBytes(uint8_t* dst, uint32_t length, bool use_dma) override;
    void readPixels(void* dst, pixelcopy_t* pc, uint32_t length) override;

  private:

    bool _is_quad_spi = false;

    static __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }
    __attribute__ ((always_inline)) inline void exec_spi(void) {        *_spi_cmd_reg = SPI_EXECUTE; }
    __attribute__ ((always_inline)) inline void wait_spi(void) { while (*_spi_cmd_reg & SPI_USR); }
    __attribute__ ((always_inline)) inline void set_write_len(uint32_t bitlen) { *_spi_mosi_dlen_reg = bitlen - 1; }
    __attribute__ ((always_inline)) inline void set_read_len( uint32_t bitlen) { *reg(SPI_MISO_DLEN_REG(_spi_port)) = bitlen - 1; }

    void dc_control(bool flg)
    {
      auto reg = _gpio_reg_dc[flg];
      auto mask = _mask_reg_dc;
      auto spi_cmd_reg = _spi_cmd_reg;
#if !defined ( CONFIG_IDF_TARGET ) || defined ( CONFIG_IDF_TARGET_ESP32 )
      while (*spi_cmd_reg & SPI_USR) {}    // wait SPI
#else
 #if defined ( SOC_GDMA_SUPPORTED )
      auto dma = _clear_dma_reg;
      if (dma)
      {
        _clear_dma_reg = nullptr;
        while (*spi_cmd_reg & SPI_USR) {}    // wait SPI
        *dma = 0;
#if defined ( LGFX_SPI_SCT )
        if (_sct_active) { _sct_end(); }
        if (_sct_ok) { *_spi_user_reg &= ~(uint32_t)SPI_CS_SETUP; }
#endif
      }
      else
      {
        while (*spi_cmd_reg & SPI_USR) {}    // wait SPI
      }
 #else
      auto dma = _spi_dma_out_link_reg;
      while (*spi_cmd_reg & SPI_USR) {}    // wait SPI
      *dma = 0;
 #endif
#endif
      *reg = mask;
    }

    void _alloc_dmadesc(size_t len);
    void _spi_dma_reset(void);
    void _setup_dma_desc_links(const uint8_t *data, int32_t len);

#if defined ( LGFX_SPI_SCT )
    // Segmented-Configure-Transfer (SCT): runs a single DMA transfer over 32 KB in hardware by placing a CONF buffer
    // per segment in the DMA chain (the CPU starts it only once).
    bool _sct_start(const uint8_t* data, uint32_t length);
    bool _sct_start_chain(dma_desc_t* first, uint32_t total);
    void _sct_put_conf(dma_desc_t*& d, uint32_t*& conf, uint32_t user, uint32_t user1, uint32_t seg, bool last);
    bool _sct_exec(void);
    void _sct_end(void);
    volatile uint32_t* _spi_slave_reg = nullptr;
    dma_desc_t* _sct_desc = nullptr;
    uint32_t* _sct_conf = nullptr;
    uint32_t _sct_desc_capacity = 0;   // descriptors allocated for _sct_desc (_sct_conf holds half of that + 1)
    bool _sct_active = false;    // a transfer started with SCT is not cleaned up yet (usr_conf must be cleared)
    bool _sct_ok = false;        // whether this bus uses SCT (GP-SPI2 only; false with LGFX_SPI_SCT_DISABLE)
#endif

    config_t _cfg;
    FlipBuffer _flip_buffer;
    volatile uint32_t* _gpio_reg_dc[2] = { nullptr, nullptr };
    volatile uint32_t* _spi_mosi_dlen_reg = nullptr;
    volatile uint32_t* _spi_w0_reg = nullptr;
    volatile uint32_t* _spi_cmd_reg = nullptr;
    volatile uint32_t* _spi_user_reg = nullptr;
    volatile uint32_t* _spi_dma_out_link_reg = nullptr;
    volatile uint32_t* _spi_dma_out_conf0_reg = nullptr;
    // Alignment of the start of a transfer from external RAM: the largest GDMA external memory block size
    // (16/32/64 B, differs by IDF version). Only the first descriptor is aligned (an unaligned tail at later
    // descriptor boundaries was confirmed not to stall).
    static constexpr uint32_t dma_ext_align = 64;
    // Whether external RAM may be a DMA source (decided in init; if false, external RAM goes through the CPU).
    bool _psram_dma_ok = true;
    // Reset the GDMA channel before each transfer so leftover data from an earlier abort (underflow etc.) is not carried over.
    // Leftovers corrupt the next transfer whatever its source, so this is done on every GDMA chip and every transfer,
    // not only on chips that support external RAM.
    void dma_channel_reset(void);
    // AXI_DMA and the newer AHB_DMA take the descriptor address in a register of its own (unused elsewhere)
    volatile uint32_t* _spi_dma_out_link2_reg = nullptr;
    volatile uint32_t* _spi_dma_outstatus_reg = nullptr;
    volatile uint32_t* _clear_dma_reg = nullptr;
    uint32_t _last_freq_apb = 0;
    uint32_t _clkdiv_write = 0;
    uint32_t _clkdiv_read = 0;
    uint32_t _user_reg = 0;
    uint32_t _mask_reg_dc = 0;
    uint32_t _dma_queue_bytes = 0;
    dma_desc_t* _dmadesc = nullptr;
    uint32_t _dmadesc_size = 0;
    dma_desc_t* _dma_queue = nullptr;
    uint32_t _dma_queue_size = 0;
    uint32_t _dma_queue_capacity = 0;
    uint8_t _spi_port = 0;
    uint8_t _dma_ch = 0;
    bool _inited = false;
  };

//----------------------------------------------------------------------------
 }
}
