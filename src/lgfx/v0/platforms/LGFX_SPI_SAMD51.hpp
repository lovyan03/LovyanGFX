/*----------------------------------------------------------------------------/
  Lovyan GFX library - LCD graphics library .
  
  support platform:
    ESP32 (SPI/I2S) with Arduino/ESP-IDF
    ATSAMD51 (SPI) with Arduino
  
Original Source:  
 https://github.com/lovyan03/LovyanGFX/  

Licence:  
 [BSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)  

Author:  
 [lovyan03](https://twitter.com/lovyan03)  

Contributors:  
 [ciniml](https://github.com/ciniml)  
 [mongonta0716](https://github.com/mongonta0716)  
 [tobozo](https://github.com/tobozo)  
/----------------------------------------------------------------------------*/
#ifndef LGFX_SPI_SAMD51_HPP_
#define LGFX_SPI_SAMD51_HPP_

#include "samd51_common.hpp"
#include "../LGFX_Device.hpp"

#if defined (ARDUINO)
#include <SERCOM.h>
#include <Adafruit_ZeroDMA.h>
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

#define _Ul(n) (static_cast<uint32_t>((n)))
#define PORT_PINCFG_PMUXEN_Pos      0            /**< \brief (PORT_PINCFG) Peripheral Multiplexer Enable */
#define PORT_PINCFG_PMUXEN          (_Ul(0x1) << PORT_PINCFG_PMUXEN_Pos)
#define PORT_PINCFG_INEN_Pos        1            /**< \brief (PORT_PINCFG) Input Enable */
#define PORT_PINCFG_INEN            (_Ul(0x1) << PORT_PINCFG_INEN_Pos)
#define PORT_PINCFG_PULLEN_Pos      2            /**< \brief (PORT_PINCFG) Pull Enable */
#define PORT_PINCFG_PULLEN          (_Ul(0x1) << PORT_PINCFG_PULLEN_Pos)
#define PORT_PINCFG_DRVSTR_Pos      6            /**< \brief (PORT_PINCFG) Output Driver Strength Selection */
#define PORT_PINCFG_DRVSTR          (_Ul(0x1) << PORT_PINCFG_DRVSTR_Pos)
#define PORT_PINCFG_MASK            _Ul(0x47)     /**< \brief (PORT_PINCFG) MASK Register */

typedef uint8_t SercomDataOrder;
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
/*
void _IRQhandler(uint8_t flags) {
    int channel = flags;
#ifdef __SAMD51__
    flags = DMAC->Channel[flags].CHINTFLAG.reg;
#endif
    if(flags & DMAC_CHINTENCLR_TERR) {
        // Clear error flag
#ifdef __SAMD51__
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTENCLR_TERR;
#else
        DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TERR;
#endif
    } else if(flags & DMAC_CHINTENCLR_TCMPL) {
        // Clear transfer complete flag
#ifdef __SAMD51__
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTENCLR_TCMPL;
#else
        DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TCMPL;
#endif
    } else if(flags & DMAC_CHINTENCLR_SUSP) {
        // Clear channel suspend flag
#ifdef __SAMD51__
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTENCLR_SUSP;
#else
        DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_SUSP;
#endif
    }
}
#ifdef __SAMD51__
void DMAC_0_Handler(void)
#else
void DMAC_Handler(void)
#endif
{
while (!Serial);
    uint8_t channel = DMAC->INTPEND.bit.ID; // Channel # causing interrupt
    if(channel < DMAC_CH_NUM) {
            _IRQhandler(channel);
    }
}
#ifdef __SAMD51__
void DMAC_1_Handler(void) __attribute__((weak, alias("DMAC_0_Handler")));
void DMAC_2_Handler(void) __attribute__((weak, alias("DMAC_0_Handler")));
void DMAC_3_Handler(void) __attribute__((weak, alias("DMAC_0_Handler")));
void DMAC_4_Handler(void) __attribute__((weak, alias("DMAC_0_Handler")));
#endif
//*/


namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  #define MEMBER_DETECTOR(member, classname, classname_impl, valuetype) struct classname_impl { \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, T::member> check(decltype(T::member)*); \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, V> check(...); \
  };template<class T, valuetype V> class classname : public decltype(classname_impl::check<T, V>(nullptr)) {};
  MEMBER_DETECTOR(spi_host   , get_spi_host   , get_spi_host_impl   , int)
  MEMBER_DETECTOR(spi_mosi   , get_spi_mosi   , get_spi_mosi_impl   , int)
  MEMBER_DETECTOR(spi_miso   , get_spi_miso   , get_spi_miso_impl   , int)
  MEMBER_DETECTOR(spi_sclk   , get_spi_sclk   , get_spi_sclk_impl   , int)
  MEMBER_DETECTOR(spi_dlen   , get_spi_dlen   , get_spi_dlen_impl   , int)
  #undef MEMBER_DETECTOR


  static constexpr struct {
    uintptr_t sercomPtr;
    uint8_t   id_core;
    uint8_t   id_slow;
    IRQn_Type irq[4];
    int       dmac_id_tx;
    int       dmac_id_rx;
  } sercomData[] = {
    { 0x40003000UL, SERCOM0_GCLK_ID_CORE, SERCOM0_GCLK_ID_SLOW, SERCOM0_0_IRQn, SERCOM0_1_IRQn, SERCOM0_2_IRQn, SERCOM0_3_IRQn, SERCOM0_DMAC_ID_TX, SERCOM0_DMAC_ID_RX },
    { 0x40003400UL, SERCOM1_GCLK_ID_CORE, SERCOM1_GCLK_ID_SLOW, SERCOM1_0_IRQn, SERCOM1_1_IRQn, SERCOM1_2_IRQn, SERCOM1_3_IRQn, SERCOM1_DMAC_ID_TX, SERCOM1_DMAC_ID_RX },
    { 0x41012000UL, SERCOM2_GCLK_ID_CORE, SERCOM2_GCLK_ID_SLOW, SERCOM2_0_IRQn, SERCOM2_1_IRQn, SERCOM2_2_IRQn, SERCOM2_3_IRQn, SERCOM2_DMAC_ID_TX, SERCOM2_DMAC_ID_RX },
    { 0x41014000UL, SERCOM3_GCLK_ID_CORE, SERCOM3_GCLK_ID_SLOW, SERCOM3_0_IRQn, SERCOM3_1_IRQn, SERCOM3_2_IRQn, SERCOM3_3_IRQn, SERCOM3_DMAC_ID_TX, SERCOM3_DMAC_ID_RX },
    { 0x43000000UL, SERCOM4_GCLK_ID_CORE, SERCOM4_GCLK_ID_SLOW, SERCOM4_0_IRQn, SERCOM4_1_IRQn, SERCOM4_2_IRQn, SERCOM4_3_IRQn, SERCOM4_DMAC_ID_TX, SERCOM4_DMAC_ID_RX },
    { 0x43000400UL, SERCOM5_GCLK_ID_CORE, SERCOM5_GCLK_ID_SLOW, SERCOM5_0_IRQn, SERCOM5_1_IRQn, SERCOM5_2_IRQn, SERCOM5_3_IRQn, SERCOM5_DMAC_ID_TX, SERCOM5_DMAC_ID_RX },
  #if defined(SERCOM6)
    { 0x43000800UL, SERCOM6_GCLK_ID_CORE, SERCOM6_GCLK_ID_SLOW, SERCOM6_0_IRQn, SERCOM6_1_IRQn, SERCOM6_2_IRQn, SERCOM6_3_IRQn, SERCOM6_DMAC_ID_TX, SERCOM6_DMAC_ID_RX },
  #endif
  #if defined(SERCOM7)
    { 0x43000C00UL, SERCOM7_GCLK_ID_CORE, SERCOM7_GCLK_ID_SLOW, SERCOM7_0_IRQn, SERCOM7_1_IRQn, SERCOM7_2_IRQn, SERCOM7_3_IRQn, SERCOM7_DMAC_ID_TX, SERCOM7_DMAC_ID_RX },
  #endif
  };


  template <class CFG>
  class LGFX_SPI : public LGFX_Device
  {

  public:

    virtual ~LGFX_SPI() {
#if defined (ARDUINO)
      _dma_adafruit.free();
#endif
      //if (_dmadesc) {
      //  heap_free(_dmadesc);
      //  _dmadesc = nullptr;
      //  _dmadesc_len = 0;
      //}
      delete_dmabuffer();
    }

    LGFX_SPI() : LGFX_Device()
    {
      _need_wait = false;
      _sercom = reinterpret_cast<Sercom*>(sercomData[CFG::sercom_index].sercomPtr);
    }

    __attribute__ ((always_inline)) inline void begin(void) { init_impl(); }

    __attribute__ ((always_inline)) inline void init(void) { init_impl(); }

    void writeCommand(uint_fast8_t cmd) override { startWrite(); write_cmd(cmd); endWrite(); }

    void writeData(uint_fast8_t data) override { startWrite(); if (_spi_dlen == 16) { write_data(data << 8, _spi_dlen); } else { write_data(data, _spi_dlen); } endWrite(); }

    void writeData16(uint_fast16_t data) override { startWrite(); write_data(__builtin_bswap16(data), 16); endWrite(); }

    void writeData32(uint32_t data) override { startWrite(); write_data(__builtin_bswap32(data), 32); endWrite(); }

    uint32_t readCommand(uint_fast8_t commandByte, uint_fast8_t index=0, uint_fast8_t len=4) override { startWrite(); auto res = read_command(commandByte, index << 3, len << 3); endWrite(); return res; }

    uint32_t readData(uint_fast8_t index=0, uint_fast8_t len=4) override
    {
      startWrite();
      start_read();
      if (index) read_data(index << 3);
      uint32_t res = read_data(len << 3);
      end_read(false);
      endWrite();
      return res; 
    }

void resetSPI()
{
  auto *spi = &_sercom->SPI;

  //Setting the Software Reset bit to 1
  spi->CTRLA.bit.SWRST = 1;

  //Wait both bits Software Reset from CTRLA and SYNCBUSY are equal to 0
  while (spi->CTRLA.bit.SWRST || spi->SYNCBUSY.bit.SWRST);
}

void enableSPI()
{
  auto *spi = &_sercom->SPI;

  //Setting the enable bit to 1
  spi->CTRLA.bit.ENABLE = 1;
  _need_wait = false;

    //Waiting then enable bit from SYNCBUSY is equal to 0;
  while (spi->SYNCBUSY.bit.ENABLE);
}


    uint32_t FreqToClockDiv(uint32_t freq)
    {
      uint32_t div = CFG::sercom_clkfreq / (1+(freq<<1));
      return div;
    }

    void setFreqDiv(uint32_t div)
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

    void pinAssignSercom(int cfgport, int type = 3) {
      uint_fast8_t port = (cfgport >> 8) & 0xFF;
      uint_fast8_t pin = cfgport & 0xFF;
      uint32_t temp = PORT->Group[port].PMUX[pin >> 1].reg;

      if (pin&1) temp = PORT_PMUX_PMUXO( type ) | (temp & PORT_PMUX_PMUXE( 0xF ));
      else       temp = PORT_PMUX_PMUXE( type ) | (temp & PORT_PMUX_PMUXO( 0xF ));
      PORT->Group[port].PMUX[pin >> 1].reg = temp ;
      PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    }

    void initBus(void) override
    {
      preInit();

      auto *spi = &_sercom->SPI;
      while (spi->SYNCBUSY.bit.ENABLE); //Waiting then enable bit from SYNCBUSY is equal to 0;
      spi->CTRLA.bit.ENABLE = 0;        //Setting the enable bit to 0

      if (-1 != _spi_miso) pinAssignSercom(_spi_miso);
      if (-1 != _spi_mosi) pinAssignSercom(_spi_mosi);
      if (-1 != _spi_sclk) pinAssignSercom(_spi_sclk);

      resetSPI();

//      int8_t idx = CFG::sercom_index;
//      for(uint8_t i=0; i<4; i++) {
//        NVIC_ClearPendingIRQ(sercomData[idx].irq[i]);
//        NVIC_SetPriority(sercomData[idx].irq[i], SERCOM_NVIC_PRIORITY);
//        NVIC_EnableIRQ(sercomData[idx].irq[i]);
//      }

#if defined(__SAMD51__)
  auto mastermode = SERCOM_SPI_CTRLA_MODE(0x3);
#else
  auto mastermode = SERCOM_SPI_CTRLA_MODE_SPI_MASTER;
#endif
      SercomDataOrder dataOrder = MSB_FIRST;

      //Setting the CTRLA register
      spi->CTRLA.reg = mastermode
                     | SERCOM_SPI_CTRLA_DOPO(CFG::pad_mosi)
                     | SERCOM_SPI_CTRLA_DIPO(CFG::pad_miso)
                     | dataOrder << SERCOM_SPI_CTRLA_DORD_Pos;

      //Setting the CTRLB register
      spi->CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE(SPI_CHAR_SIZE_8_BITS)
                     | SERCOM_SPI_CTRLB_RXEN; //Active the SPI receiver.

      while( spi->SYNCBUSY.bit.CTRLB == 1 );

      _clkdiv_read  = FreqToClockDiv(_panel->freq_read);
      _clkdiv_fill  = FreqToClockDiv(_panel->freq_fill);
      _clkdiv_write = FreqToClockDiv(_panel->freq_write);

      if (CFG::sercom_clksrc >= 0) {
        static constexpr uint8_t id_core = sercomData[CFG::sercom_index].id_core;
        static constexpr uint8_t id_slow = sercomData[CFG::sercom_index].id_slow;

        GCLK->PCHCTRL[id_core].bit.CHEN = 0;     // Disable timer
        GCLK->PCHCTRL[id_slow].bit.CHEN = 0;     // Disable timer

        uint32_t gclk_reg_value = GCLK_PCHCTRL_CHEN | CFG::sercom_clksrc << GCLK_PCHCTRL_GEN_Pos;

        while (GCLK->PCHCTRL[id_core].bit.CHEN || GCLK->PCHCTRL[id_slow].bit.CHEN);  // Wait for disable

        GCLK->PCHCTRL[id_core].reg = gclk_reg_value;
        GCLK->PCHCTRL[id_slow].reg = gclk_reg_value;

        while (!GCLK->PCHCTRL[id_core].bit.CHEN || !GCLK->PCHCTRL[id_slow].bit.CHEN);  // Wait for clock enable
      }
      spi->BAUD.reg = _clkdiv_write;

      spi->CTRLA.bit.ENABLE = 1;         //Setting the enable bit to 1
      _need_wait = false;
      while (spi->SYNCBUSY.bit.ENABLE);  //Waiting then enable bit from SYNCBUSY is equal to 0;

#if defined (ARDUINO)
      _dma_adafruit.allocate();
      _dma_adafruit.setTrigger(sercomData[CFG::sercom_index].dmac_id_tx);
      _dma_adafruit.setAction(DMA_TRIGGER_ACTON_BEAT);
      _dma_adafruit.setCallback(dmaCallback);
      _dma_write_desc = _dma_adafruit.addDescriptor(nullptr, (void*)&spi->DATA.reg);
      _dma_write_desc->BTCTRL.bit.VALID  = true;
      _dma_write_desc->BTCTRL.bit.SRCINC = true;
      _dma_write_desc->BTCTRL.bit.DSTINC = false;
#endif

/*
      _alloc_dmadesc(4);

      uint8_t channel = _dma_channel;
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
      DMAC->BASEADDR.bit.BASEADDR = (uint32_t)&_dmadesc[1];
      DMAC->WRBADDR.bit.WRBADDR   = (uint32_t)&_dmadesc[0];

      // Re-enable DMA controller with all priority levels
      DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xF);

      // Enable DMA interrupt at lowest priority
#ifdef __SAMD51__
      IRQn_Type irqs[] = { DMAC_0_IRQn, DMAC_1_IRQn, DMAC_2_IRQn,
                           DMAC_3_IRQn, DMAC_4_IRQn };
      for(uint8_t i=0; i<(sizeof irqs / sizeof irqs[0]); i++) {
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

//----------------------------------------------------------------------------
  protected:

    //void preInit(void) override {}

    void preCommandList(void) override
    {
      wait_spi();
      if (!_fill_mode) return;
      _fill_mode = false;
      set_clock_write();
    }

    void postCommandList(void) override
    {
      wait_spi();
    }

    void postSetPanel(void) override
    {
      _last_apb_freq = -1;
      _cmd_ramwr      = _panel->getCmdRamwr();
      _len_setwindow  = _panel->len_setwindow;
      fpGetWindowAddr = _len_setwindow == 32 ? PanelCommon::getWindowAddr32 : PanelCommon::getWindowAddr16;

      int32_t spi_dc = _panel->spi_dc;
      _mask_reg_dc = (1ul << (spi_dc & 0xFF));
      uint32_t port = spi_dc >> 8;
      _gpio_reg_dc_h = &PORT->Group[port].OUTSET.reg;
      _gpio_reg_dc_l = &PORT->Group[port].OUTCLR.reg;
      dc_h();
      lgfxPinMode(spi_dc, pin_mode_t::output);

      cs_h();
      lgfxPinMode(_panel->spi_cs, pin_mode_t::output);

      postSetRotation();
      postSetColorDepth();
    }

    void postSetRotation(void) override
    {
      bool fullscroll = (_sx == 0 && _sy == 0 && _sw == _width && _sh == _height);

      //_cmd_caset = _panel->getCmdCaset();
      //_cmd_raset = _panel->getCmdRaset();
      _colstart  = _panel->getColStart();
      _rowstart  = _panel->getRowStart();
      _width     = _panel->getWidth();
      _height    = _panel->getHeight();
      _clip_r = _width - 1;
      _clip_b = _height - 1;

      if (fullscroll) {
        _sw = _width;
        _sh = _height;
      }
      //_xs = _xe = _ys = _ye = ~0;
      _clip_l = _clip_t = 0;
    }

    void beginTransaction_impl(void) override {
      if (_in_transaction) return;
      _in_transaction = true;
      begin_transaction();
    }

    void begin_transaction(void) {
      _need_wait = false;
      _fill_mode = false;
      set_clock_write();
      cs_l();
    }

    void endTransaction_impl(void) override {
      if (!_in_transaction) return;
      _in_transaction = false;
      end_transaction();
    }

    void end_transaction(void) {
      wait_spi();
      if (_panel->spi_cs < 0) {
        write_cmd(0); // NOP command
      }
      dc_h();
      cs_h();
    }

    void initDMA_impl(void) override {}

    void waitDMA_impl(void) override
    {
      wait_spi();
    }

    bool dmaBusy_impl(void) override
    {
      return _need_wait && (_sercom->SPI.INTFLAG.bit.TXC == 0);
    }

    void writePixelsDMA_impl(const void* data, int32_t length) override
    {
      write_bytes((const uint8_t*)data, length * _write_conv.bytes, true);
    }

    void writeBytes_impl(const uint8_t* data, int32_t length, bool use_dma) override
    {
      write_bytes((const uint8_t*)data, length, use_dma);
    }

    void readBytes_impl(uint8_t* dst, int32_t length) override
    {
      start_read();
      read_bytes(dst, length);
      end_read(false); // Don't use the CS operation.
    }

    void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
      write_cmd(_cmd_ramwr);
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      if (_in_transaction) {
        set_window(x, y, x, y);
        write_cmd(_cmd_ramwr);
        write_data(_color.raw, _write_conv.bits);
        return;
      }

      begin_transaction();
      set_window(x, y, x, y);
      write_cmd(_cmd_ramwr);
      write_data(_color.raw, _write_conv.bits);
      end_transaction();
    }

    void writeFillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
      set_window(x, y, x+w-1, y+h-1);
      write_cmd(_cmd_ramwr);

      push_block(w*h, _clkdiv_write != _clkdiv_fill);
    }

    void pushBlock_impl(int32_t length) override
    {
      push_block(length);
    }

    void push_block(int32_t length, bool fillclock = false)
    {
      auto *spi = &_sercom->SPI;
      fillclock &= (length > 4);
      bool d32b = spi->CTRLC.bit.DATA32B;
      dc_h();
      if (fillclock || !d32b) {
        while (spi->SYNCBUSY.reg);
        spi->CTRLA.bit.ENABLE = 0;
        if (!d32b) spi->CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
        if (fillclock) {
          _fill_mode = true; 
          spi->BAUD.reg = _clkdiv_fill;
        }
        spi->CTRLA.bit.ENABLE = 1;
        while (spi->SYNCBUSY.reg);
      }
      uint32_t data = _color.raw;
      if (_write_conv.bytes == 2) { // 16bit color
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

        uint32_t surplus = length & 3;
        if (surplus) {
          length -= surplus;
          do {
            while (spi->INTFLAG.bit.TXC == 0);
            spi->DATA.reg = data;
          } while (--surplus);
          if (!length) return;
        }

        uint32_t buf[3];
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

    void write_cmd(uint_fast8_t cmd)
    {
      auto *spi = &_sercom->SPI;
      dc_l();
      if (_spi_dlen != 16) {
        spi->LENGTH.reg = 1 | SERCOM_SPI_LENGTH_LENEN;
        spi->DATA.reg = cmd;
        _need_wait = true;
      } else {
        if (!_sercom->SPI.CTRLC.bit.DATA32B) {
          while (spi->SYNCBUSY.reg);
          spi->CTRLA.bit.ENABLE = 0;
          spi->CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
          spi->CTRLA.bit.ENABLE = 1;
          while (spi->SYNCBUSY.reg);
        }
        spi->LENGTH.reg = 2 | SERCOM_SPI_LENGTH_LENEN;
        spi->DATA.reg = cmd << 8;
        _need_wait = true;
      }
    }

    void write_data(uint32_t data, uint32_t bit_length)
    {
      auto len = bit_length >> 3 | SERCOM_SPI_LENGTH_LENEN;
      auto *spi = &_sercom->SPI;
      dc_h();
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

    __attribute__ ((always_inline)) inline 
    void write_cmd_data(const uint8_t *addr)
    {
      auto *spi = &_sercom->SPI;
      do {
        dc_l();
        bool d32b = !spi->CTRLC.bit.DATA32B;
        if (d32b || _fill_mode) {
          while (spi->SYNCBUSY.reg);
          spi->CTRLA.bit.ENABLE = 0;
          if (d32b) spi->CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
          if (_fill_mode) {
            _fill_mode = false;
            while (spi->SYNCBUSY.reg);
            spi->BAUD.reg = _clkdiv_write;
          }
          spi->CTRLA.bit.ENABLE = 1;
          while (spi->SYNCBUSY.reg);
        }

        if (_spi_dlen == 16) {
          spi->LENGTH.reg = 2 | SERCOM_SPI_LENGTH_LENEN;
          spi->DATA.reg = *addr++ << 8;
        }
        else
        {
          spi->LENGTH.reg = 1 | SERCOM_SPI_LENGTH_LENEN;
          spi->DATA.reg = *addr++;
        }
        _need_wait = true;
        uint_fast8_t numArgs = *addr++;
        if (numArgs)
        {
          auto mask_reg_dc = _mask_reg_dc;
          auto gpio_reg_dc_h = _gpio_reg_dc_h;
          while (spi->INTFLAG.bit.TXC == 0);
          *gpio_reg_dc_h = mask_reg_dc;   // dc_h();

          if (_spi_dlen == 16) {
            if (numArgs >= 2) {
              spi->LENGTH.reg = 0;
              spi->DATA.reg = addr[0] << 8 | addr[1] << 24;
              addr += 2;
              numArgs -= 2;
              if (numArgs >= 2) {
                do {
                  while (spi->INTFLAG.bit.DRE == 0);
                  spi->DATA.reg = addr[0] << 8 | addr[1] << 24;
                  addr += 2;
                } while (2 <= (numArgs -= 2));
              }
            }
            if (numArgs) {
              uint32_t tmp = addr[0] << 8;
              while (spi->INTFLAG.bit.TXC == 0);
              spi->LENGTH.reg = numArgs | SERCOM_SPI_LENGTH_LENEN;
              spi->DATA.reg = tmp;
              ++addr;
            }
          }
          else
          {
            if (numArgs >= 4) {
              spi->LENGTH.reg = 0;
              spi->DATA.reg = *(uint32_t*)addr;
              addr += 4;
              numArgs -= 4;
              if (numArgs >= 4) {
                do {
                  while (spi->INTFLAG.bit.DRE == 0);
                  spi->DATA.reg = *(uint32_t*)addr;
                  addr += 4;
                } while (4 <= (numArgs -= 4));
              }
            }
            if (numArgs) {
              uint32_t tmp = *(uint32_t*)addr;
              while (spi->INTFLAG.bit.TXC == 0);
              spi->LENGTH.reg = numArgs | SERCOM_SPI_LENGTH_LENEN;
              spi->DATA.reg = tmp;
              addr += numArgs;
            }
          }
        }
      } while (reinterpret_cast<const uint16_t*>(addr)[0] != 0xFFFF);
    }
//*/
    void set_window(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
    {
      uint8_t buf[16];
      if (auto b = _panel->getWindowCommands1(buf, xs, ys, xe, ye)) { write_cmd_data(b); }
      if (auto b = _panel->getWindowCommands2(buf, xs, ys, xe, ye)) { write_cmd_data(b); }
      return;
/*/
      uint32_t len;
      if (_spi_dlen == 8) {
        len = _len_setwindow;
      } else {
        len = (_len_setwindow << 1);
      }
      auto fp = fpGetWindowAddr;

      auto *spi = &_sercom->SPI;
      bool d32b = !spi->CTRLC.bit.DATA32B;
      if (d32b || _fill_mode) {
        wait_spi();
        while (spi->SYNCBUSY.reg);
        spi->CTRLA.bit.ENABLE = 0;
        if (d32b) spi->CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
        if (_fill_mode) {
          _fill_mode = false;
          while (spi->SYNCBUSY.reg);
          spi->BAUD.reg = _clkdiv_write;
        }
        spi->CTRLA.bit.ENABLE = 1;
        _need_wait = false;
        while (spi->SYNCBUSY.reg);
      }

      if (_xs != xs || _xe != xe) {
        dc_l();
        if (_spi_dlen != 16) {
          spi->LENGTH.reg = 1 | SERCOM_SPI_LENGTH_LENEN;
          spi->DATA.reg = _cmd_caset;
        } else {
          spi->LENGTH.reg = 2 | SERCOM_SPI_LENGTH_LENEN;
          spi->DATA.reg = _cmd_caset << 8;
        }
        _need_wait = true;

        uint32_t tmp = _colstart;

        tmp = fp(xs + tmp, xe + tmp);
        if (_spi_dlen == 8) {
          auto l = len >> 3 | SERCOM_SPI_LENGTH_LENEN;
          dc_h();
          spi->LENGTH.reg = l;
          spi->DATA.reg = tmp;
        } else if (_spi_dlen == 16) {
          auto t = (tmp & 0xFF) << 8 | (tmp >> 8) << 24;
          dc_h();
          spi->LENGTH.reg = 0;
          spi->DATA.reg = t;
          tmp >>= 16;
          t = (tmp & 0xFF) << 8 | (tmp >> 8) << 24;
          while (spi->INTFLAG.bit.TXC == 0);
          spi->DATA.reg = t;
        }
        _xs = xs;
        _xe = xe;
      }
      if (_ys != ys || _ye != ye) {
        dc_l();
        if (_spi_dlen != 16) {
          spi->LENGTH.reg = 1 | SERCOM_SPI_LENGTH_LENEN;
          spi->DATA.reg = _cmd_raset;
        } else {
          spi->LENGTH.reg = 2 | SERCOM_SPI_LENGTH_LENEN;
          spi->DATA.reg = _cmd_raset << 8;
        }
        _need_wait = true;

        uint32_t tmp = _rowstart;

        tmp = fp(ys + tmp, ye + tmp);
        if (_spi_dlen == 8) {
          auto l = len >> 3 | SERCOM_SPI_LENGTH_LENEN;
          dc_h();
          spi->LENGTH.reg = l;
          spi->DATA.reg = tmp;
        } else if (_spi_dlen == 16) {
          auto t = (tmp & 0xFF) << 8 | (tmp >> 8) << 24;
          dc_h();
          spi->LENGTH.reg = 0;
          spi->DATA.reg = t;
          tmp >>= 16;
          t = (tmp & 0xFF) << 8 | (tmp >> 8) << 24;
          while (spi->INTFLAG.bit.TXC == 0);
          spi->DATA.reg = t;
        }
        _ys = ys;
        _ye = ye;
      }
//*/
    }

    void start_read(void) {
      _fill_mode = false;
      dc_h();
      set_clock_read();
    }

    void end_read(bool cs_ctrl = true)
    {
      wait_spi();
      if (cs_ctrl)
      {
        cs_h();
        if (_panel->spi_cs < 0) {
          write_cmd(0); // NOP command
        }
      }
      set_clock_write();
      _fill_mode = false;
      if (cs_ctrl)
      {
        cs_l();
      }
    }

    uint32_t read_data(uint32_t length)
    {
      dc_h();
      write_data(0, length);
      return _sercom->SPI.DATA.reg;
    }

    uint32_t read_command(uint_fast8_t command, uint32_t bitindex = 0, uint32_t bitlen = 8)
    {
      startWrite();
      write_cmd(command);
      start_read();
      if (bitindex) read_data(bitindex>>3);
      uint32_t res = read_data(bitlen>>3);
      end_read();
      endWrite();
      return res;
    }

    void pushImage_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param, bool use_dma) override
    {
      auto bytes = _write_conv.bytes;
      auto src_x = param->src_x;
      auto fp_copy = param->fp_copy;

      int32_t xr = (x + w) - 1;

      if (param->transp == ~0u) {
        if (param->no_convert) {
          setWindow_impl(x, y, xr, y + h - 1);
          uint32_t i = (src_x + param->src_y * param->src_bitwidth) * bytes;
          auto src = &((const uint8_t*)param->src_data)[i];
          if (static_cast<int32_t>(param->src_bitwidth) == w || h == 1) {
            int32_t whb = w * h * bytes;
            if (!use_dma && (32 < whb) && (whb <= 1024)) {
              auto buf = get_dmabuffer(whb);
              memcpy(buf, src, whb);
              write_bytes(buf, whb, true);
            } else {
              write_bytes(src, whb, use_dma);
            }
          } else {
            auto add = param->src_bitwidth * bytes;
            do {
              write_bytes(src, w * bytes, use_dma);
              src += add;
            } while (--h);
          }
        } else
        if (use_dma)
        {
          auto buf = get_dmabuffer(w * bytes);
          fp_copy(buf, 0, w, param);
          setWindow_impl(x, y, xr, y + h - 1);
          write_bytes(buf, w * bytes, true);
          while (--h) {
            param->src_x = src_x;
            param->src_y++;
            buf = get_dmabuffer(w * bytes);
            fp_copy(buf, 0, w, param);
            write_bytes(buf, w * bytes, true);
          }
        } else {
          setWindow_impl(x, y, xr, y + h - 1);
          do {
            write_pixels(w, param);
            param->src_x = src_x;
            param->src_y++;
          } while (--h);
        }
      } else {
        auto fp_skip = param->fp_skip;
        h += y;
        do {
          int32_t i = 0;
          while (w != (i = fp_skip(i, w, param))) {
            auto buf = get_dmabuffer(w * bytes);
            int32_t len = fp_copy(buf, 0, w - i, param);
            setWindow_impl(x + i, y, x + i + len - 1, y);
            write_bytes(buf, len * bytes, true);
            if (w == (i += len)) break;
          }
          param->src_x = src_x;
          param->src_y++;
        } while (++y != h);
      }
    }

    void writePixels_impl(int32_t length, pixelcopy_t* param) override
    {
      write_pixels(length, param);
    }

    void write_pixels(int32_t length, pixelcopy_t* param)
    {
      const uint8_t dst_bytes = _write_conv.bytes;
      uint32_t limit = (dst_bytes == 2) ? 16 : 12;
      uint32_t len;
      do {
        len = ((length - 1) % limit) + 1;
        //if (limit <= 256) limit <<= 2;
        if (limit <= 512) limit <<= 1;
        auto dmabuf = get_dmabuffer(len * dst_bytes);
        param->fp_copy(dmabuf, 0, len, param);
        write_bytes(dmabuf, len * dst_bytes, true);
      } while (length -= len);
    }

    void write_bytes(const uint8_t* data, int32_t length, bool use_dma = false)
    {
      auto *spi = &_sercom->SPI;
#if defined (ARDUINO)
      if (use_dma && length > 31) {
        uint_fast8_t beatsize = spi->CTRLC.bit.DATA32B ? 2 : 0;
        // If the data is 4 bytes aligned, the DATA32B can be enabled.
        if ((bool)(beatsize) == ((length & 3) || ((uint32_t)data & 3))) {
          beatsize = 2 - beatsize;
          wait_spi();
          while (spi->SYNCBUSY.bit.ENABLE);
          spi->CTRLA.bit.ENABLE = 0;
          spi->CTRLC.bit.DATA32B = (bool)(beatsize);
          spi->CTRLA.bit.ENABLE = 1;
          _need_wait = false;
          while (spi->SYNCBUSY.bit.ENABLE);
        }
        dc_h();

        spi->LENGTH.reg = 0;

        auto desc = _dma_write_desc;
        desc->SRCADDR.reg = (uint32_t)data + length;
        desc->BTCNT.reg            = length >> beatsize;
        desc->BTCTRL.bit.BEATSIZE  = beatsize;

        _dma_adafruit.startJob();

/*
//        uint8_t channel = _dma_channel;
//        DMAC->Channel[channel].CHCTRLA.bit.ENABLE  = 0;
        //DMAC->CTRL.bit.DMAENABLE    = 0; // Disable DMA controller
        //DMAC->BASEADDR.bit.BASEADDR = (uint32_t)&_dmadesc[1];
        //DMAC->WRBADDR.bit.WRBADDR   = (uint32_t)&_dmadesc[0];
        //DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xF);
        uint8_t interruptMask = 2;
#ifdef __SAMD51__
        //DMAC->Channel[channel].CHCTRLA.bit.TRIGACT = DMAC_CHCTRLA_TRIGACT_BURST_Val; // triggerAction;
        //DMAC->Channel[channel].CHCTRLA.bit.BURSTLEN = DMAC_CHCTRLA_BURSTLEN_SINGLE_Val; // Single-beat burst length
        DMAC->Channel[channel].CHINTENSET.reg = DMAC_CHINTENSET_MASK &  interruptMask;
        DMAC->Channel[channel].CHINTENCLR.reg = DMAC_CHINTENCLR_MASK & ~interruptMask;
        DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 1;
#else
        DMAC->CHID.bit.ID    = channel;
        DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK &  interruptMask;
        DMAC->CHINTENCLR.reg = DMAC_CHINTENCLR_MASK & ~interruptMask;
        DMAC->CHCTRLA.bit.ENABLE = 1; // Enable the transfer channel
#endif
//*/
        _need_wait = true;
        return;
      }
#endif

      dc_h();
      if (!spi->CTRLC.bit.DATA32B) {
        while (spi->SYNCBUSY.reg);
        spi->CTRLA.bit.ENABLE = 0;
        spi->CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
        spi->CTRLA.bit.ENABLE = 1;
        while (spi->SYNCBUSY.reg);
      }

      if (length >= 4) {
        spi->LENGTH.reg = 0;
        spi->DATA.reg = *(uint32_t*)data;
        data += 4;
        length -= 4;
        if (4 <= length) {
          do {
            while (spi->INTFLAG.bit.DRE == 0);
            spi->DATA.reg = *(uint32_t*)data;
            data += 4;
          } while (4 <= (length -= 4));
        }
      }
      _need_wait = true;
      if (length) {
        uint32_t tmp = *(uint32_t*)data;
        while (spi->INTFLAG.bit.TXC == 0);
        spi->LENGTH.reg = length | SERCOM_SPI_LENGTH_LENEN;
        spi->DATA.reg = tmp;
      }
    }

    void readRect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) override
    {
      startWrite();
      set_window(x, y, x + w - 1, y + h - 1);
      auto len = w * h;
      if (!_panel->spi_read)
      {
        memset(dst, 0, len * param->dst_bits >> 3);
      }
      else
      {
        write_cmd(_panel->getCmdRamrd());
        uint32_t len_dummy_read_pixel = _panel->len_dummy_read_pixel;
        start_read();
        if (len_dummy_read_pixel) {;
          write_data(0, len_dummy_read_pixel);
        }

        if (param->no_convert) {
          read_bytes((uint8_t*)dst, len * _read_conv.bytes);
        } else {
          read_pixels(dst, len, param);
        }
        end_read();
      }
      endWrite();
    }

    void read_pixels(void* dst, int32_t length, pixelcopy_t* param)
    {
      int32_t bytes = _read_conv.bytes;
      int32_t dstindex = 0;
      int32_t len = 4;
      uint8_t buf[24];
      param->src_data = buf;
      do {
        if (len > length) len = length;
        read_bytes(buf, len * bytes);
        param->src_x = 0;
        dstindex = param->fp_copy(dst, dstindex, dstindex + len, param);
        length -= len;
      } while (length);
    }

    void read_bytes(uint8_t* dst, int32_t length)
    {
/*
      if (use_dma && length > 16) {
        _sercom->SPI.LENGTH.reg = 0;
        uint_fast8_t beatsize = _sercom->SPI.CTRLC.bit.DATA32B ? 2 : 0;
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
        desc->DSTADDR.reg          = (uint32_t)dst;
        desc->SRCADDR.reg          = (uint32_t)(&_sercom->SPI.DATA.reg);

        if (desc->BTCTRL.bit.DSTINC) {
          if (desc->BTCTRL.bit.STEPSEL) {
            desc->DSTADDR.reg += length * (1 << desc->BTCTRL.bit.STEPSIZE);
          } else {
            desc->DSTADDR.reg += length;
          }
        }

//        _dma_adafruit.setTrigger(sercomData[CFG::sercom_index].dmac_id_rx);
        //_dma_adafruit.changeDescriptor(desc, nullptr, const_cast<uint8_t*>(dst), length>>beatsize);
        _dma_adafruit.startJob();
        _need_wait = true;
        return;
      }
//*/

      auto *datreg = &_sercom->SPI.DATA.reg;
      auto *lenreg = &_sercom->SPI.LENGTH.reg;
      auto *intflag = &_sercom->SPI.INTFLAG.bit; 
      int32_t len1 = length > 3 ? 4 : length;
      int32_t len2 = len1;
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
        uint32_t d = *datreg;
        memcpy(dst, &d, len2);
        dst += len2;
      } while (length);
      _need_wait = false;
    }

    __attribute__ ((always_inline)) inline void set_clock_write(void) { setFreqDiv(_clkdiv_write); }
    __attribute__ ((always_inline)) inline void set_clock_read(void)  { setFreqDiv(_clkdiv_read ); }
    __attribute__ ((always_inline)) inline void set_clock_fill(void)  { setFreqDiv(_clkdiv_fill ); }
    __attribute__ ((always_inline)) inline void wait_spi(void) { if (_need_wait != true) return; auto *intflag = &_sercom->SPI.INTFLAG.bit; while (intflag->TXC == 0); }

    __attribute__ ((always_inline)) inline void dc_h(void) {
      auto mask_reg_dc = _mask_reg_dc;
      auto gpio_reg_dc_h = _gpio_reg_dc_h;
      wait_spi();
      *gpio_reg_dc_h = mask_reg_dc;
    }
    __attribute__ ((always_inline)) inline void dc_l(void) {
      auto mask_reg_dc = _mask_reg_dc;
      auto gpio_reg_dc_l = _gpio_reg_dc_l;
      wait_spi();
      *gpio_reg_dc_l = mask_reg_dc;
    }

    static constexpr int _spi_mosi = get_spi_mosi<CFG, -1>::value;
    static constexpr int _spi_miso = get_spi_miso<CFG, -1>::value;
    static constexpr int _spi_sclk = get_spi_sclk<CFG, -1>::value;
    static constexpr int _spi_dlen = get_spi_dlen<CFG,  8>::value;

    uint32_t(*fpGetWindowAddr)(uint_fast16_t, uint_fast16_t);
    uint_fast16_t _colstart;
    uint_fast16_t _rowstart;
/*
    uint_fast16_t _xs;
    uint_fast16_t _xe;
    uint_fast16_t _ys;
    uint_fast16_t _ye;
    uint32_t _cmd_caset;
    uint32_t _cmd_raset;
//*/
    uint32_t _cmd_ramwr;
    uint32_t _last_apb_freq;
    uint32_t _clkdiv_write;
    uint32_t _clkdiv_read;
    uint32_t _clkdiv_fill;
    uint32_t _len_setwindow;
    bool _fill_mode;
    uint32_t _mask_reg_dc;
    volatile uint32_t* _gpio_reg_dc_h;
    volatile uint32_t* _gpio_reg_dc_l;

#if defined (ARDUINO)
    Adafruit_ZeroDMA _dma_adafruit;
    DmacDescriptor* _dma_write_desc;
    static DmacDescriptor* _dmadesc;
    static uint32_t _dmadesc_len;
#endif
    static bool _need_wait;
    static Sercom* _sercom;
  };

#if defined (ARDUINO)
  template <class T> DmacDescriptor* LGFX_SPI<T>::_dmadesc = nullptr;
  template <class T> uint32_t LGFX_SPI<T>::_dmadesc_len = 0;
#endif

  template <class T> bool LGFX_SPI<T>::_need_wait;
  template <class T> Sercom* LGFX_SPI<T>::_sercom;

//----------------------------------------------------------------------------
 }
}

using lgfx::LGFX_SPI;

#if !defined(ARDUINO)
#undef _Ul

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

#endif

#endif
