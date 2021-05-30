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
#if defined (__SAMD51__)

#include "Bus_SPI.hpp"

#include "../../misc/pixelcopy.hpp"

#if defined (ARDUINO)
  #include <SERCOM.h>
  #include "utility/dma.h"
  static void dmaCallback(Adafruit_ZeroDMA*) {}
#else
  #include "samd51_arduino_compat.hpp"

  #undef PORT_PINCFG_PMUXEN_Pos
  #undef PORT_PINCFG_PMUXEN
  #undef PORT_PINCFG_INEN_Pos
  #undef PORT_PINCFG_INEN
  #undef PORT_PINCFG_PULLEN_Pos
  #undef PORT_PINCFG_PULLEN
  #undef PORT_PINCFG_DRVSTR_Pos
  #undef PORT_PINCFG_DRVSTR
  #undef PORT_PINCFG_MASK

  #undef SERCOM_SPI_CTRLA_MODE_Pos
  #undef SERCOM_SPI_CTRLA_MODE_Msk
  #undef SERCOM_SPI_CTRLA_MODE
  #undef SERCOM_SPI_CTRLA_DORD_Pos
  #undef SERCOM_SPI_CTRLA_DORD
  #undef SERCOM_SPI_CTRLB_RXEN_Pos
  #undef SERCOM_SPI_CTRLB_RXEN
  #undef SERCOM_SPI_CTRLB_CHSIZE_Pos
  #undef SERCOM_SPI_CTRLB_CHSIZE_Msk
  #undef SERCOM_SPI_CTRLB_CHSIZE
  #undef SERCOM_SPI_LENGTH_LEN_Pos
  #undef SERCOM_SPI_LENGTH_LEN_Msk
  #undef SERCOM_SPI_LENGTH_LEN
  #undef SERCOM_SPI_LENGTH_LENEN_Pos
  #undef SERCOM_SPI_LENGTH_LENEN
  #undef SERCOM_SPI_CTRLA_DIPO_Pos
  #undef SERCOM_SPI_CTRLA_DIPO_Msk
  #undef SERCOM_SPI_CTRLA_DIPO
  #undef SERCOM_SPI_CTRLA_DOPO_Pos
  #undef SERCOM_SPI_CTRLA_DOPO_Msk
  #undef SERCOM_SPI_CTRLA_DOPO
  #undef SPI_CHAR_SIZE_8_BITS
  #undef MSB_FIRST
  #undef GCLK_PCHCTRL_CHEN_Pos
  #undef GCLK_PCHCTRL_CHEN

  #define _Ul(n) (static_cast<std::uint32_t>((n)))
  #define PORT_PINCFG_PMUXEN_Pos      0            /**< \brief (PORT_PINCFG) Peripheral Multiplexer Enable */
  #define PORT_PINCFG_PMUXEN          (_Ul(0x1) << PORT_PINCFG_PMUXEN_Pos)
  #define PORT_PINCFG_INEN_Pos        1            /**< \brief (PORT_PINCFG) Input Enable */
  #define PORT_PINCFG_INEN            (_Ul(0x1) << PORT_PINCFG_INEN_Pos)
  #define PORT_PINCFG_PULLEN_Pos      2            /**< \brief (PORT_PINCFG) Pull Enable */
  #define PORT_PINCFG_PULLEN          (_Ul(0x1) << PORT_PINCFG_PULLEN_Pos)
  #define PORT_PINCFG_DRVSTR_Pos      6            /**< \brief (PORT_PINCFG) Output Driver Strength Selection */
  #define PORT_PINCFG_DRVSTR          (_Ul(0x1) << PORT_PINCFG_DRVSTR_Pos)
  #define PORT_PINCFG_MASK            _Ul(0x47)     /**< \brief (PORT_PINCFG) MASK Register */

  typedef std::uint8_t SercomDataOrder;
  #define SERCOM_SPI_CTRLA_MODE_Pos   2            /**< \brief (SERCOM_SPI_CTRLA) Operating Mode */
  #define SERCOM_SPI_CTRLA_MODE_Msk   (_Ul(0x7) << SERCOM_SPI_CTRLA_MODE_Pos)
  #define SERCOM_SPI_CTRLA_MODE(value) (SERCOM_SPI_CTRLA_MODE_Msk & ((value) << SERCOM_SPI_CTRLA_MODE_Pos))
  #define SERCOM_SPI_CTRLA_DORD_Pos   30           /**< \brief (SERCOM_SPI_CTRLA) Data Order */
  #define SERCOM_SPI_CTRLA_DORD       (_Ul(0x1) << SERCOM_SPI_CTRLA_DORD_Pos)
  #define SERCOM_SPI_CTRLB_RXEN_Pos   17           /**< \brief (SERCOM_SPI_CTRLB) Receiver Enable */
  #define SERCOM_SPI_CTRLB_RXEN       (_Ul(0x1) << SERCOM_SPI_CTRLB_RXEN_Pos)
  #define SERCOM_SPI_CTRLB_CHSIZE_Pos 0            /**< \brief (SERCOM_SPI_CTRLB) Character Size */
  #define SERCOM_SPI_CTRLB_CHSIZE_Msk (_Ul(0x7) << SERCOM_SPI_CTRLB_CHSIZE_Pos)
  #define SERCOM_SPI_CTRLB_CHSIZE(value) (SERCOM_SPI_CTRLB_CHSIZE_Msk & ((value) << SERCOM_SPI_CTRLB_CHSIZE_Pos))
  #define SERCOM_SPI_LENGTH_LEN_Pos   0            /**< \brief (SERCOM_SPI_LENGTH) Data Length */
  #define SERCOM_SPI_LENGTH_LEN_Msk   (_Ul(0xFF) << SERCOM_SPI_LENGTH_LEN_Pos)
  #define SERCOM_SPI_LENGTH_LEN(value) (SERCOM_SPI_LENGTH_LEN_Msk & ((value) << SERCOM_SPI_LENGTH_LEN_Pos))
  #define SERCOM_SPI_LENGTH_LENEN_Pos 8            /**< \brief (SERCOM_SPI_LENGTH) Data Length Enable */
  #define SERCOM_SPI_LENGTH_LENEN     (_Ul(0x1) << SERCOM_SPI_LENGTH_LENEN_Pos)
  #define SERCOM_SPI_CTRLA_DIPO_Pos   20           /**< \brief (SERCOM_SPI_CTRLA) Data In Pinout */
  #define SERCOM_SPI_CTRLA_DIPO_Msk   (_Ul(0x3) << SERCOM_SPI_CTRLA_DIPO_Pos)
  #define SERCOM_SPI_CTRLA_DIPO(value) (SERCOM_SPI_CTRLA_DIPO_Msk & ((value) << SERCOM_SPI_CTRLA_DIPO_Pos))
  #define SERCOM_SPI_CTRLA_DOPO_Pos   16           /**< \brief (SERCOM_SPI_CTRLA) Data Out Pinout */
  #define SERCOM_SPI_CTRLA_DOPO_Msk   (_Ul(0x3) << SERCOM_SPI_CTRLA_DOPO_Pos)
  #define SERCOM_SPI_CTRLA_DOPO(value) (SERCOM_SPI_CTRLA_DOPO_Msk & ((value) << SERCOM_SPI_CTRLA_DOPO_Pos))

  #define SPI_CHAR_SIZE_8_BITS 0
  #define MSB_FIRST 0

  #define GCLK_PCHCTRL_CHEN_Pos       6            /**< \brief (GCLK_PCHCTRL) Channel Enable */
  #define GCLK_PCHCTRL_CHEN           (_Ul(0x1) << GCLK_PCHCTRL_CHEN_Pos)

#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
/*
  void Bus_SPI::enableSPI(void)
  {
    auto *spi = &_sercom->SPI;

    //Setting the enable bit to 1
    spi->CTRLA.bit.ENABLE = 1;
    _need_wait = false;

      //Waiting then enable bit from SYNCBUSY is equal to 0;
    while (spi->SYNCBUSY.bit.ENABLE);
  }
//*/
  std::uint32_t Bus_SPI::FreqToClockDiv(std::uint32_t freq)
  {
    std::uint32_t div = _cfg.sercom_clkfreq / (1+(freq<<1));
    return div;
  }

  void Bus_SPI::setFreqDiv(std::uint32_t div)
  {
    auto *spi = &_sercom->SPI;
    while (spi->SYNCBUSY.reg);
    spi->CTRLA.bit.ENABLE = 0;
    while (spi->SYNCBUSY.reg);
    spi->BAUD.reg = div;
    spi->CTRLA.bit.ENABLE = 1;
    _need_wait = false;
    while (spi->SYNCBUSY.reg);
  }

  void Bus_SPI::config(const config_t& config)
  {
    _cfg = config;

    _need_wait = false;
    _sercom = reinterpret_cast<Sercom*>(samd51::getSercomData(_cfg.sercom_index)->sercomPtr);
    _last_apb_freq = -1;
    _mask_reg_dc = 0;
    std::uint32_t port = 0;
    if (_cfg.pin_dc >= 0)
    {
      _mask_reg_dc = (1ul << (_cfg.pin_dc & (samd51::PIN_MASK)));
      port = _cfg.pin_dc >> samd51::PORT_SHIFT;
    }

    _gpio_reg_dc_h = &PORT->Group[port].OUTSET.reg;
    _gpio_reg_dc_l = &PORT->Group[port].OUTCLR.reg;
  }

  void Bus_SPI::init(void)
  {
    dc_control(true);
    pinMode(_cfg.pin_dc, pin_mode_t::output);

    auto *spi = &_sercom->SPI;
    while (spi->SYNCBUSY.bit.ENABLE); //Waiting then enable bit from SYNCBUSY is equal to 0;
    spi->CTRLA.bit.ENABLE = 0;        //Setting the enable bit to 0

    lgfx::spi::init(_cfg.sercom_index, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi);

    auto sercom_data = samd51::getSercomData(_cfg.sercom_index);

    if (_cfg.sercom_clksrc >= 0)
    {
      std::uint8_t id_core = sercom_data->id_core;
      std::uint8_t id_slow = sercom_data->id_slow;

      GCLK->PCHCTRL[id_core].bit.CHEN = 0;     // Disable timer
      GCLK->PCHCTRL[id_slow].bit.CHEN = 0;     // Disable timer

      std::uint32_t gclk_reg_value = GCLK_PCHCTRL_CHEN | _cfg.sercom_clksrc << GCLK_PCHCTRL_GEN_Pos;

      while (GCLK->PCHCTRL[id_core].bit.CHEN || GCLK->PCHCTRL[id_slow].bit.CHEN);  // Wait for disable

      GCLK->PCHCTRL[id_core].reg = gclk_reg_value;
      GCLK->PCHCTRL[id_slow].reg = gclk_reg_value;

      while (!GCLK->PCHCTRL[id_core].bit.CHEN || !GCLK->PCHCTRL[id_slow].bit.CHEN);  // Wait for clock enable
    }
//    spi->BAUD.reg = _clkdiv_write;

    spi->CTRLA.bit.ENABLE = 1;         //Setting the enable bit to 1
    while (spi->SYNCBUSY.bit.ENABLE);  //Waiting then enable bit from SYNCBUSY is equal to 0;

    _need_wait = false;

    _clkdiv_read  = FreqToClockDiv(_cfg.freq_read);
    _clkdiv_write = FreqToClockDiv(_cfg.freq_write);

#if defined (ARDUINO)
    _dma_adafruit.allocate();
    _dma_adafruit.setTrigger(sercom_data->dmac_id_tx);
    _dma_adafruit.setAction(DMA_TRIGGER_ACTON_BEAT);
    _dma_adafruit.setCallback(dmaCallback);
    _dma_write_desc = _dma_adafruit.addDescriptor(nullptr, (void*)&spi->DATA.reg);
    _dma_write_desc->BTCTRL.bit.VALID  = true;
    _dma_write_desc->BTCTRL.bit.SRCINC = true;
    _dma_write_desc->BTCTRL.bit.DSTINC = false;
#endif

/*
    _alloc_dmadesc(4);

    std::uint8_t channel = _dma_channel;
#if (SAML21) || (SAML22) || (SAMC20) || (SAMC21)
    PM->AHBMASK.bit.DMAC_       = 1;
#elif defined(__SAMD51__)
    MCLK->AHBMASK.bit.DMAC_     = 1; // Initialize DMA clocks
#else
    PM->AHBMASK.bit.DMAC_       = 1; // Initialize DMA clocks
    PM->APBBMASK.bit.DMAC_      = 1;
#endif

    DMAC->CTRL.bit.DMAENABLE    = 0; // Disable DMA controller
    DMAC->CTRL.bit.SWRST        = 1; // Perform software reset

    // Initialize descriptor list addresses
    DMAC->BASEADDR.bit.BASEADDR = (std::uint32_t)&_dmadesc[1];
    DMAC->WRBADDR.bit.WRBADDR   = (std::uint32_t)&_dmadesc[0];

    // Re-enable DMA controller with all priority levels
    DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xF);

    // Enable DMA interrupt at lowest priority
#ifdef __SAMD51__
    IRQn_Type irqs[] = { DMAC_0_IRQn, DMAC_1_IRQn, DMAC_2_IRQn,
                          DMAC_3_IRQn, DMAC_4_IRQn };
    for(std::uint8_t i=0; i<(sizeof irqs / sizeof irqs[0]); i++) {
        NVIC_EnableIRQ(irqs[i]);
        NVIC_SetPriority(irqs[i], (1<<__NVIC_PRIO_BITS)-1);
    }
#else
    NVIC_EnableIRQ(DMAC_IRQn);
    NVIC_SetPriority(DMAC_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
#endif

// Reset the allocated channel
#ifdef __SAMD51__
    DMAC->Channel[channel].CHCTRLA.bit.ENABLE  = 0;
    DMAC->Channel[channel].CHCTRLA.bit.SWRST   = 1;
#else
    DMAC->CHID.bit.ID         = channel;
    DMAC->CHCTRLA.bit.ENABLE  = 0;
    DMAC->CHCTRLA.bit.SWRST   = 1;
#endif

// Clear software trigger
    DMAC->SWTRIGCTRL.reg     &= ~(1 << channel);

// Configure default behaviors
#ifdef __SAMD51__
    DMAC->Channel[channel].CHPRILVL.bit.PRILVL = 0;
    DMAC->Channel[channel].CHCTRLA.bit.TRIGSRC = sercomData[CFG::sercom_index].dmac_id_tx;
    DMAC->Channel[channel].CHCTRLA.bit.TRIGACT = DMAC_CHCTRLA_TRIGACT_BURST_Val;
    DMAC->Channel[channel].CHCTRLA.bit.BURSTLEN = DMAC_CHCTRLA_BURSTLEN_SINGLE_Val; // Single-beat burst length
#else
    DMAC->CHCTRLB.bit.LVL     = 0;
    DMAC->CHCTRLB.bit.TRIGSRC = sercomData[CFG::sercom_index].dmac_id_tx;
    DMAC->CHCTRLB.bit.TRIGACT = DMAC_CHCTRLB_TRIGACT_BEAT_Val;
#endif
//*/
  }

  void Bus_SPI::release(void)
  {
    lgfx::spi::release(_cfg.sercom_index);
  }

  void Bus_SPI::beginTransaction(void)
  {
    _need_wait = false;
    set_clock_write();
  }

  void Bus_SPI::endTransaction(void)
  {
    wait_spi();
    dc_control(true);
  }

  void Bus_SPI::wait(void)
  {
    wait_spi();
  }

  bool Bus_SPI::busy(void) const
  {
    return _need_wait && (_sercom->SPI.INTFLAG.bit.TXC == 0);
  }

  bool Bus_SPI::writeCommand(std::uint32_t data, std::uint_fast8_t bit_length)
  {
    auto *spi = &_sercom->SPI;
    dc_control(false);
    if (bit_length <= 8)
    {
      spi->LENGTH.reg = 1 | SERCOM_SPI_LENGTH_LENEN;
      spi->DATA.reg = data;
      _need_wait = true;
      return true;
    }
    if (!_sercom->SPI.CTRLC.bit.DATA32B) {
      while (spi->SYNCBUSY.reg);
      spi->CTRLA.bit.ENABLE = 0;
      spi->CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
      spi->CTRLA.bit.ENABLE = 1;
      while (spi->SYNCBUSY.reg);
    }
    spi->LENGTH.reg = 2 | SERCOM_SPI_LENGTH_LENEN;
    spi->DATA.reg = data << 8;
    _need_wait = true;
    return true;
  }

  void Bus_SPI::writeData(std::uint32_t data, std::uint_fast8_t bit_length)
  {
    auto len = bit_length >> 3 | SERCOM_SPI_LENGTH_LENEN;
    auto *spi = &_sercom->SPI;
    dc_control(true);
    if (!_sercom->SPI.CTRLC.bit.DATA32B) {
      while (spi->SYNCBUSY.reg);
      spi->CTRLA.bit.ENABLE = 0;
      spi->CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
      spi->CTRLA.bit.ENABLE = 1;
      while (spi->SYNCBUSY.reg);
    }
    spi->LENGTH.reg = len;
    spi->DATA.reg = data;
    _need_wait = true;
  }

  void Bus_SPI::writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t length)
  {
    std::size_t bytes = bit_length >> 3;
    auto *spi = &_sercom->SPI;
    bool d32b = spi->CTRLC.bit.DATA32B;
    dc_control(true);
    if (!d32b) {
      while (spi->SYNCBUSY.reg);
      spi->CTRLA.bit.ENABLE = 0;
      if (!d32b) spi->CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
      spi->CTRLA.bit.ENABLE = 1;
      while (spi->SYNCBUSY.reg);
    }
    if (bytes == 2) { // 16bit color
      if (length & 0x01) {
        spi->LENGTH.reg = 2 | SERCOM_SPI_LENGTH_LENEN;
        _need_wait = true;
        spi->DATA.reg = data;
        if (!--length) return;
        while (spi->INTFLAG.bit.TXC == 0); // LENEN有効の時はTXC待ち(DRE待ちを使うと挙動がおかしくなる)
      }
      spi->LENGTH.reg = 0;
      data |= data << 16;
      spi->DATA.reg = data;
      _need_wait = true;
      length >>= 1;
      while (--length) {
        while (spi->INTFLAG.bit.DRE == 0);
        spi->DATA.reg = data;
      }

    } else {  // 24bit color

      spi->LENGTH.reg = 3 | SERCOM_SPI_LENGTH_LENEN;
      spi->DATA.reg = data;
      _need_wait = true;
      if (!--length) return;

      std::uint32_t surplus = length & 3;
      if (surplus) {
        length -= surplus;
        do {
          while (spi->INTFLAG.bit.TXC == 0);
          spi->DATA.reg = data;
        } while (--surplus);
        if (!length) return;
      }

      std::uint32_t buf[3];
      buf[0] = data       | data << 24;
      buf[1] = data >>  8 | data << 16;
      buf[2] = data >> 16 | data <<  8;
      length = (length * 3) >> 2;
      while (spi->INTFLAG.bit.TXC == 0);
      spi->LENGTH.reg = 0;
      spi->DATA.reg = buf[0];
      if (!--length) return;
      do {
        while (spi->INTFLAG.bit.DRE == 0);
        spi->DATA.reg = buf[1];
        if (!--length) return;
        while (spi->INTFLAG.bit.DRE == 0);
        spi->DATA.reg = buf[2];
        if (!--length) return;
        while (spi->INTFLAG.bit.DRE == 0);
        spi->DATA.reg = buf[0];
      } while (--length);
    }
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, std::uint32_t length)
  {
    const std::uint8_t dst_bytes = param->dst_bits >> 3;
    std::uint32_t limit = (dst_bytes == 3) ? 12 : 16;
    std::uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      //if (limit <= 256) limit <<= 2;
      if (limit <= 512) limit <<= 1;
      auto dmabuf = _flip_buffer.getBuffer(len * dst_bytes);
      param->fp_copy(dmabuf, 0, len, param);
      writeBytes(dmabuf, len * dst_bytes, true, true);
    } while (length -= len);
  }

  void Bus_SPI::writeBytes(const std::uint8_t* data, std::uint32_t length, bool dc, bool use_dma)
  {
    auto *spi = &_sercom->SPI;
#if defined (ARDUINO)
    if (length > 31)
    {
      std::uint_fast8_t beatsize = spi->CTRLC.bit.DATA32B ? 2 : 0;
      // If the data is 4 bytes aligned, the DATA32B can be enabled.
      if ((bool)(beatsize) == ((length & 3) || ((std::uint32_t)data & 3)))
      {
        beatsize = 2 - beatsize;
        wait_spi();
        _need_wait = false;
        while (spi->SYNCBUSY.bit.ENABLE);
        spi->CTRLA.bit.ENABLE = 0;
        spi->CTRLC.bit.DATA32B = (bool)(beatsize);
        spi->CTRLA.bit.ENABLE = 1;
        while (spi->SYNCBUSY.bit.ENABLE);
      }

      auto desc = _dma_write_desc;
      std::uint32_t len = length;
      dc_control(dc);
      spi->LENGTH.reg = 0;
      desc->BTCTRL.bit.BEATSIZE  = beatsize;
      desc->SRCADDR.reg = (std::uint32_t)data + length;
      do
      {
        if (!use_dma)
        {
          len = ((length - 1) & 1023) + 1;
          auto dmabuf = _flip_buffer.getBuffer(len);
          memcpy(dmabuf, data, len);
          data += len;
          wait_spi();
          desc->SRCADDR.reg = (std::uint32_t)dmabuf + len;
        }
        desc->BTCNT.reg = len >> beatsize;
        _dma_adafruit.startJob();
        _need_wait = true;
        length -= len;
      } while (length);
      return;
    }
#endif

    dc_control(dc);
    if (!spi->CTRLC.bit.DATA32B) {
      while (spi->SYNCBUSY.reg);
      spi->CTRLA.bit.ENABLE = 0;
      spi->CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
      spi->CTRLA.bit.ENABLE = 1;
      while (spi->SYNCBUSY.reg);
    }

    if (length >= 4) {
      spi->LENGTH.reg = 0;
      spi->DATA.reg = *(std::uint32_t*)data;
      data += 4;
      length -= 4;
      if (4 <= length) {
        do {
          while (spi->INTFLAG.bit.DRE == 0);
          spi->DATA.reg = *(std::uint32_t*)data;
          data += 4;
        } while (4 <= (length -= 4));
      }
    }
    _need_wait = true;
    if (length) {
      std::uint32_t tmp = *(std::uint32_t*)data;
      while (spi->INTFLAG.bit.TXC == 0);
      spi->LENGTH.reg = length | SERCOM_SPI_LENGTH_LENEN;
      spi->DATA.reg = tmp;
    }
  }

  void Bus_SPI::beginRead(void)
  {
    set_clock_read();
  }

  void Bus_SPI::endRead(void)
  {
    set_clock_write();
  }

  std::uint32_t Bus_SPI::readData(std::uint_fast8_t bit_length)
  {
    writeData(0, bit_length);
    return _sercom->SPI.DATA.reg;
  }

  bool Bus_SPI::readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma)
  {
/*
      if (use_dma && length > 16) {
        _sercom->SPI.LENGTH.reg = 0;
        std::uint_fast8_t beatsize = _sercom->SPI.CTRLC.bit.DATA32B ? 2 : 0;
        if (beatsize && (length & 3)) {
          disableSPI();
          beatsize = 0;
          _sercom->SPI.CTRLC.bit.DATA32B = 0;  // 4Byte transfer enable
          enableSPI();
        }
        auto desc = _dma_write_desc;
        desc->BTCNT.reg            = length >> beatsize;   // count;
        desc->BTCTRL.bit.BEATSIZE  = beatsize; // DMA_BEAT_SIZE_BYTE;   // size;
        desc->BTCTRL.bit.VALID     = true;
        desc->BTCTRL.bit.EVOSEL    = 0; // DMA_EVENT_OUTPUT_DISABLE;
        desc->BTCTRL.bit.BLOCKACT  = 0; // DMA_BLOCK_ACTION_NOACT;
        desc->BTCTRL.bit.SRCINC    = false;                 // srcInc;
        desc->BTCTRL.bit.DSTINC    = true;                // dstInc;
        desc->BTCTRL.bit.STEPSEL   = 0; // DMA_STEPSEL_DST;      // stepSel;
        desc->BTCTRL.bit.STEPSIZE  = 0; // DMA_ADDRESS_INCREMENT_STEP_SIZE_1; // stepSize;
        desc->DSTADDR.reg          = (std::uint32_t)dst;
        desc->SRCADDR.reg          = (std::uint32_t)(&_sercom->SPI.DATA.reg);

        if (desc->BTCTRL.bit.DSTINC) {
          if (desc->BTCTRL.bit.STEPSEL) {
            desc->DSTADDR.reg += length * (1 << desc->BTCTRL.bit.STEPSIZE);
          } else {
            desc->DSTADDR.reg += length;
          }
        }

//        _dma_adafruit.setTrigger(sercomData[CFG::sercom_index].dmac_id_rx);
        //_dma_adafruit.changeDescriptor(desc, nullptr, const_cast<std::uint8_t*>(dst), length>>beatsize);
        _dma_adafruit.startJob();
        _need_wait = true;
        return;
      }
//*/
    auto *datreg = &_sercom->SPI.DATA.reg;
    auto *lenreg = &_sercom->SPI.LENGTH.reg;
    auto *intflag = &_sercom->SPI.INTFLAG.bit;
    std::uint32_t len1 = length > 3 ? 4 : length;
    std::uint32_t len2 = len1;
    wait_spi();
    dst[0] = *datreg;
    *lenreg = len1 | SERCOM_SPI_LENGTH_LENEN;
    *datreg = 0;
    _need_wait = true;
    do {
      if (0 == (length -= len1)) {
        len2 = len1;
        while (intflag->RXC == 0);
      } else {
        if (length < len1) {
          len1 = length;
          while (intflag->RXC == 0);
          *lenreg = len1 | SERCOM_SPI_LENGTH_LENEN;
        } else {
          while (intflag->RXC == 0);
        }
        *datreg = 0;
      }
      std::uint32_t d = *datreg;
      memcpy(dst, &d, len2);
      dst += len2;
    } while (length);
    _need_wait = false;
    return true;
  }

  void Bus_SPI::readPixels(void* dst, pixelcopy_t* param, std::uint32_t length)
  {
    std::uint32_t bytes = param->src_bits >> 3;
    std::uint32_t dstindex = 0;
    std::uint32_t len = 4;
    std::uint8_t buf[24];
    param->src_data = buf;
    do {
      if (len > length) len = length;
      readBytes((std::uint8_t*)buf, len * bytes, true);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len, param);
      length -= len;
    } while (length);
  }

//----------------------------------------------------------------------------
 }
}

#endif
