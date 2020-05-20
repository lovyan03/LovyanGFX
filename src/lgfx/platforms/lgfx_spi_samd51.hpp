/*----------------------------------------------------------------------------/
  Lovyan GFX library - SAMD51 hardware SPI graphics library .  
  
    for Arduino  
  
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
#include "../lgfx_base.hpp"

#include <SERCOM.h>

#if defined (ARDUINO)
#include <Adafruit_ZeroDMA.h>
#include "utility/dma.h"
static void dmaCallback(Adafruit_ZeroDMA*) {}
#endif
/*
void _IRQhandler(std::uint8_t flags) {
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
    std::uint8_t channel = DMAC->INTPEND.bit.ID; // Channel # causing interrupt
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
    Sercom   *sercomPtr;
    std::uint8_t   id_core;
    std::uint8_t   id_slow;
    IRQn_Type irq[4];
    int       dmac_id_tx;
    int       dmac_id_rx;
  } sercomData[] = {
    { SERCOM0, SERCOM0_GCLK_ID_CORE, SERCOM0_GCLK_ID_SLOW, SERCOM0_0_IRQn, SERCOM0_1_IRQn, SERCOM0_2_IRQn, SERCOM0_3_IRQn, SERCOM0_DMAC_ID_TX, SERCOM0_DMAC_ID_RX },
    { SERCOM1, SERCOM1_GCLK_ID_CORE, SERCOM1_GCLK_ID_SLOW, SERCOM1_0_IRQn, SERCOM1_1_IRQn, SERCOM1_2_IRQn, SERCOM1_3_IRQn, SERCOM1_DMAC_ID_TX, SERCOM1_DMAC_ID_RX },
    { SERCOM2, SERCOM2_GCLK_ID_CORE, SERCOM2_GCLK_ID_SLOW, SERCOM2_0_IRQn, SERCOM2_1_IRQn, SERCOM2_2_IRQn, SERCOM2_3_IRQn, SERCOM2_DMAC_ID_TX, SERCOM2_DMAC_ID_RX },
    { SERCOM3, SERCOM3_GCLK_ID_CORE, SERCOM3_GCLK_ID_SLOW, SERCOM3_0_IRQn, SERCOM3_1_IRQn, SERCOM3_2_IRQn, SERCOM3_3_IRQn, SERCOM3_DMAC_ID_TX, SERCOM3_DMAC_ID_RX },
    { SERCOM4, SERCOM4_GCLK_ID_CORE, SERCOM4_GCLK_ID_SLOW, SERCOM4_0_IRQn, SERCOM4_1_IRQn, SERCOM4_2_IRQn, SERCOM4_3_IRQn, SERCOM4_DMAC_ID_TX, SERCOM4_DMAC_ID_RX },
    { SERCOM5, SERCOM5_GCLK_ID_CORE, SERCOM5_GCLK_ID_SLOW, SERCOM5_0_IRQn, SERCOM5_1_IRQn, SERCOM5_2_IRQn, SERCOM5_3_IRQn, SERCOM5_DMAC_ID_TX, SERCOM5_DMAC_ID_RX },
  #if defined(SERCOM6)
    { SERCOM6, SERCOM6_GCLK_ID_CORE, SERCOM6_GCLK_ID_SLOW, SERCOM6_0_IRQn, SERCOM6_1_IRQn, SERCOM6_2_IRQn, SERCOM6_3_IRQn, SERCOM6_DMAC_ID_TX, SERCOM6_DMAC_ID_RX },
  #endif
  #if defined(SERCOM7)
    { SERCOM7, SERCOM7_GCLK_ID_CORE, SERCOM7_GCLK_ID_SLOW, SERCOM7_0_IRQn, SERCOM7_1_IRQn, SERCOM7_2_IRQn, SERCOM7_3_IRQn, SERCOM7_DMAC_ID_TX, SERCOM7_DMAC_ID_RX },
  #endif
  };


  template <class CFG>
  class LGFX_SPI : public LovyanGFX
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

    LGFX_SPI() : LovyanGFX()
    {
      _panel = nullptr;

      _sercom = sercomData[CFG::sercom_index].sercomPtr;
    }

    void setPanel(PanelCommon* panel) { _panel = panel; postSetPanel(); }

    __attribute__ ((always_inline)) inline PanelCommon* getPanel(void) const { return _panel; }

    __attribute__ ((always_inline)) inline std::int_fast8_t getRotation(void) const { return _panel->rotation; }

    __attribute__ ((always_inline)) inline bool getInvert(void) const { return _panel->invert; }

    __attribute__ ((always_inline)) inline void dmaWait(void) const { wait_spi(); }

    __attribute__ ((always_inline)) inline void begin(void) { init(); }

    void init(void) { initBus(); initPanel(); }

    // Write single byte as COMMAND
    void writeCommand(std::uint_fast8_t cmd) { startWrite(); write_cmd(cmd); endWrite(); } // AdafruitGFX compatible
    void writecommand(std::uint_fast8_t cmd) { startWrite(); write_cmd(cmd); endWrite(); } // TFT_eSPI compatible

    // Write single bytes as DATA
    void spiWrite( std::uint_fast8_t data) { startWrite(); if (_spi_dlen == 16) { write_data(data << 8, _spi_dlen); } else { write_data(data, _spi_dlen); } endWrite(); } // AdafruitGFX compatible
    void writeData(std::uint_fast8_t data) { startWrite(); if (_spi_dlen == 16) { write_data(data << 8, _spi_dlen); } else { write_data(data, _spi_dlen); } endWrite(); } // TFT_eSPI compatible
    void writedata(std::uint_fast8_t data) { startWrite(); if (_spi_dlen == 16) { write_data(data << 8, _spi_dlen); } else { write_data(data, _spi_dlen); } endWrite(); } // TFT_eSPI compatible

    // Read data
    std::uint8_t  readCommand8( std::uint_fast8_t commandByte, std::uint_fast8_t index=0) { return read_command(commandByte, index << 3, 8); }
    std::uint8_t  readcommand8( std::uint_fast8_t commandByte, std::uint_fast8_t index=0) { return read_command(commandByte, index << 3, 8); }
    std::uint16_t readCommand16(std::uint_fast8_t commandByte, std::uint_fast8_t index=0) { return __builtin_bswap16(read_command(commandByte, index << 3, 16)); }
    std::uint16_t readcommand16(std::uint_fast8_t commandByte, std::uint_fast8_t index=0) { return __builtin_bswap16(read_command(commandByte, index << 3, 16)); }
    std::uint32_t readCommand32(std::uint_fast8_t commandByte, std::uint_fast8_t index=0) { return __builtin_bswap32(read_command(commandByte, index << 3, 32)); }
    std::uint32_t readcommand32(std::uint_fast8_t commandByte, std::uint_fast8_t index=0) { return __builtin_bswap32(read_command(commandByte, index << 3, 32)); }

    void setColorDepth(std::uint8_t bpp) { setColorDepth((color_depth_t)bpp); }

    void sleep()  { writeCommand(_panel->getCmdSlpin()); }

    void wakeup() { writeCommand(_panel->getCmdSlpout()); }

    void setColorDepth(color_depth_t depth)
    {
      std::uint8_t buf[32];
      commandList(_panel->getColorDepthCommands(buf, depth));
      postSetColorDepth();
    }

    void setRotation(std::int_fast8_t r)
    {
      std::uint8_t buf[32];
      commandList(_panel->getRotationCommands(buf, r));
      postSetRotation();
    }

    void invertDisplay(bool i)
    {
      std::uint8_t buf[32];
      commandList(_panel->getInvertDisplayCommands(buf, i));
    }

    void setBrightness(std::uint8_t brightness) {
      _panel->setBrightness(brightness);
    }

    std::uint32_t readPanelID(void)
    {
      return read_command(_panel->getCmdRddid(), _panel->len_dummy_read_rddid, 32);
    }


std::uint32_t freqRef; // Frequency corresponding to clockSource

void resetSPI()
{
  //Setting the Software Reset bit to 1
  _sercom->SPI.CTRLA.bit.SWRST = 1;

  //Wait both bits Software Reset from CTRLA and SYNCBUSY are equal to 0
  while (_sercom->SPI.CTRLA.bit.SWRST || _sercom->SPI.SYNCBUSY.bit.SWRST);
}

void enableSPI()
{
  //Setting the enable bit to 1
  _sercom->SPI.CTRLA.bit.ENABLE = 1;
  _need_wait = false;

    //Waiting then enable bit from SYNCBUSY is equal to 0;
  while (_sercom->SPI.SYNCBUSY.bit.ENABLE);
}

void disableSPI()
{
    //Waiting then enable bit from SYNCBUSY is equal to 0;
  while (_sercom->SPI.SYNCBUSY.bit.ENABLE);

  //Setting the enable bit to 0
  _sercom->SPI.CTRLA.bit.ENABLE = 0;
}


    void _set_freq(std::uint32_t freq)
    {
      static constexpr std::uint8_t id_core = sercomData[CFG::sercom_index].id_core;
      static constexpr std::uint8_t id_slow = sercomData[CFG::sercom_index].id_slow;

      GCLK->PCHCTRL[id_core].bit.CHEN = 0;     // Disable timer
      GCLK->PCHCTRL[id_slow].bit.CHEN = 0;     // Disable timer

      freq <<= 1;
      static constexpr std::uint32_t srcfreq[] = { F_CPU, 48000000ul, 100000000ul };
      std::uint32_t usefreq = 0;
      int useindex = 0;
      std::uint8_t usedivider = 0;
      for (int i = 0; i < 3; ++i) {
        std::uint32_t div = srcfreq[i] / freq;
        if (div == 0) div = 1;
        std::uint32_t tmp = srcfreq[i] / div;
        if (usefreq > tmp) continue;
        usefreq = tmp;
        useindex = i;
        usedivider = div - 1;
      }

      auto gclk_reg_value = GCLK_PCHCTRL_GEN_GCLK0_Val;
      if      (useindex == 1) { gclk_reg_value = GCLK_PCHCTRL_GEN_GCLK1_Val; }
      else if (useindex == 2) { gclk_reg_value = GCLK_PCHCTRL_GEN_GCLK2_Val; }

      gclk_reg_value |= (1 << GCLK_PCHCTRL_CHEN_Pos);

      while (GCLK->PCHCTRL[id_core].bit.CHEN || GCLK->PCHCTRL[id_slow].bit.CHEN);  // Wait for disable

      GCLK->PCHCTRL[id_core].reg = gclk_reg_value;
      GCLK->PCHCTRL[id_slow].reg = gclk_reg_value;

      _sercom->SPI.BAUD.reg = usedivider;

      while (!GCLK->PCHCTRL[id_core].bit.CHEN || !GCLK->PCHCTRL[id_slow].bit.CHEN);  // Wait for clock enable
    }

    std::uint32_t FreqToClockDiv(std::uint32_t freq)
    {
      std::uint32_t div = round((float)CFG::sercom_clkfreq / (freq<<1));
      if (div > 0) --div;
      return div;
    }

    void setFreqDiv(std::uint32_t div)
    {
      disableSPI();
      while( _sercom->SPI.SYNCBUSY.bit.CTRLB == 1 );
      _sercom->SPI.BAUD.reg = div;
      enableSPI();
    }

    void pinAssignSercom(int cfgport, int type = 3) {
      std::uint_fast8_t port = (cfgport >> 8) & 0xFF;
      std::uint_fast8_t pin = cfgport & 0xFF;
      std::uint32_t temp;

      if (pin&1) temp = PORT_PMUX_PMUXO( type ) | ((PORT->Group[port].PMUX[pin >> 1].reg) & PORT_PMUX_PMUXE( 0xF )) ;
      else       temp = PORT_PMUX_PMUXE( type ) | ((PORT->Group[port].PMUX[pin >> 1].reg) & PORT_PMUX_PMUXO( 0xF )) ;
      PORT->Group[port].PMUX[pin >> 1].reg = temp ;
      PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_PMUXEN | PORT_PINCFG_DRVSTR;
    }

    void initBus(void)
    {
      disableSPI();

      if (-1 != _spi_miso) pinAssignSercom(_spi_miso);
      if (-1 != _spi_mosi) pinAssignSercom(_spi_mosi);
      if (-1 != _spi_sclk) pinAssignSercom(_spi_sclk);

      resetSPI();

//      std::int8_t idx = CFG::sercom_index;
//      for(std::uint8_t i=0; i<4; i++) {
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
      _sercom->SPI.CTRLA.reg = mastermode
                             | SERCOM_SPI_CTRLA_DOPO(CFG::pad_mosi)
                             | SERCOM_SPI_CTRLA_DIPO(CFG::pad_miso)
                             | dataOrder << SERCOM_SPI_CTRLA_DORD_Pos;

      _sercom->SPI.CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable

      //Setting the CTRLB register
      _sercom->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE(SPI_CHAR_SIZE_8_BITS)
                             | SERCOM_SPI_CTRLB_RXEN; //Active the SPI receiver.

      while( _sercom->SPI.SYNCBUSY.bit.CTRLB == 1 );

      _clkdiv_read  = FreqToClockDiv(_panel->freq_read);
      _clkdiv_fill  = FreqToClockDiv(_panel->freq_fill);
      _clkdiv_write = FreqToClockDiv(_panel->freq_write);

      if (CFG::sercom_clksrc >= 0) {
        static constexpr std::uint8_t id_core = sercomData[CFG::sercom_index].id_core;
        static constexpr std::uint8_t id_slow = sercomData[CFG::sercom_index].id_slow;

        GCLK->PCHCTRL[id_core].bit.CHEN = 0;     // Disable timer
        GCLK->PCHCTRL[id_slow].bit.CHEN = 0;     // Disable timer

        std::uint32_t gclk_reg_value = GCLK_PCHCTRL_CHEN | CFG::sercom_clksrc << GCLK_PCHCTRL_GEN_Pos;

        while (GCLK->PCHCTRL[id_core].bit.CHEN || GCLK->PCHCTRL[id_slow].bit.CHEN);  // Wait for disable

        GCLK->PCHCTRL[id_core].reg = gclk_reg_value;
        GCLK->PCHCTRL[id_slow].reg = gclk_reg_value;

        while (!GCLK->PCHCTRL[id_core].bit.CHEN || !GCLK->PCHCTRL[id_slow].bit.CHEN);  // Wait for clock enable
      }
      _sercom->SPI.BAUD.reg = _clkdiv_write;

      enableSPI();


      _dma_adafruit.allocate();
      _dma_adafruit.setTrigger(sercomData[CFG::sercom_index].dmac_id_tx);
      _dma_adafruit.setAction(DMA_TRIGGER_ACTON_BEAT);
      _dma_adafruit.setCallback(dmaCallback);
      _dma_write_desc = _dma_adafruit.addDescriptor(nullptr, (void*)&_sercom->SPI.DATA.reg);
      _dma_write_desc->BTCTRL.bit.VALID  = true;
      _dma_write_desc->BTCTRL.bit.SRCINC = true;
      _dma_write_desc->BTCTRL.bit.DSTINC = false;

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

    virtual void initPanel(void)
    {
      if (!_panel) return;

      _panel->init();

      startWrite();

      const std::uint8_t *cmds;
      for (std::uint8_t i = 0; (cmds = _panel->getInitCommands(i)); i++) {
        delay(120);
        cs_l();
        commandList(cmds);
        wait_spi();
        cs_h();
      }
      cs_l();

      invertDisplay(getInvert());
      setColorDepth(getColorDepth());
      setRotation(getRotation());
      clear();

      endWrite();

      _sx = _sy = 0;
      _sw = _width;
      _sh = _height;
    }

    void pushPixelsDMA(const void* data, std::uint32_t length) {
      write_bytes((const std::uint8_t*)data, length, true);
    }


//----------------------------------------------------------------------------
  protected:

    void postSetPanel(void)
    {
      _last_apb_freq = -1;
      _cmd_ramwr      = _panel->getCmdRamwr();
      _len_setwindow  = _panel->len_setwindow;
      fpGetWindowAddr = _len_setwindow == 32 ? PanelCommon::getWindowAddr32 : PanelCommon::getWindowAddr16;

      std::int32_t spi_dc = _panel->spi_dc;
      _mask_reg_dc = (1ul << (spi_dc & 0xFF));
      std::uint32_t port = spi_dc >> 8;
      _gpio_reg_dc_h = &PORT->Group[port].OUTSET.reg;
      _gpio_reg_dc_l = &PORT->Group[port].OUTCLR.reg;
      dc_h();
      lgfxPinMode(spi_dc, pin_mode_t::output);

      cs_h();
      lgfxPinMode(_panel->spi_cs, pin_mode_t::output);

      postSetRotation();
      postSetColorDepth();
    }

    void postSetRotation(void)
    {
      bool fullscroll = (_sx == 0 && _sy == 0 && _sw == _width && _sh == _height);

      _cmd_caset = _panel->getCmdCaset();
      _cmd_raset = _panel->getCmdRaset();
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
      _xs = _xe = _ys = _ye = ~0;
      _clip_l = _clip_t = 0;
    }

    void postSetColorDepth(void)
    {
      _write_conv.setColorDepth(_panel->write_depth);
      _read_conv.setColorDepth(_panel->read_depth);
    }

    void beginTransaction_impl(void) override {
      if (_begun_tr) return;
      _begun_tr = true;
      begin_transaction();
    }

    void begin_transaction(void) {
      _need_wait = false;
      _fill_mode = false;
      set_clock_write();
      cs_l();
    }

    void endTransaction_impl(void) override {
      if (!_begun_tr) return;
      _begun_tr = false;
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

    void waitDMA_impl(void) override
    {
      wait_spi();
    }

    void setWindow_impl(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye) override
    {
      if (_fill_mode) {
        _fill_mode = false;
        wait_spi();
        set_clock_write();
      }
      set_window(xs, ys, xe, ye);
      write_cmd(_cmd_ramwr);
    }

    void drawPixel_impl(std::int32_t x, std::int32_t y) override
    {
      if (_begun_tr) {
        if (_fill_mode) {
          _fill_mode = false;
          wait_spi();
          set_clock_write();
        }
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

    void writeFillRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) override
    {
      if (_fill_mode) {
        _fill_mode = false;
        wait_spi();
        set_clock_write();
      }
      set_window(x, y, x+w-1, y+h-1);
      write_cmd(_cmd_ramwr);

      push_block(w*h, _clkdiv_write != _clkdiv_fill);
    }

    void pushBlock_impl(std::int32_t length) override
    {
      push_block(length);
    }

    void push_block(std::int32_t length, bool fillclock = false)
    {
      auto *reg = &_sercom->SPI.DATA.reg;
      std::uint32_t data = _color.raw;
      int bytes = _write_conv.bytes;
      if (bytes == 2 && length > 1) {
        if (length & 0x01) {
          --length;
          dc_h();
          _sercom->SPI.LENGTH.reg = 2 | SERCOM_SPI_LENGTH_LENEN;
          *reg = data;
          if (!length) return;
        }
        data |= _color.raw << 16;
        length >>= 1;
        bytes = 4;
      }
      if (fillclock && 2 <= length) { _fill_mode = true; dc_h(); set_clock_fill(); }
      else { dc_h(); }
      _sercom->SPI.LENGTH.reg = bytes | SERCOM_SPI_LENGTH_LENEN;
      *reg = data;
      _need_wait = true;
      while (--length) {
        wait_spi();
        *reg = data;
      };
    }

    bool commandList(const std::uint8_t *addr)
    {
      if (addr == nullptr) return false;
      std::uint8_t  cmd;
      std::uint8_t  numArgs;
      std::uint8_t  ms;

      _fill_mode = false;
      wait_spi();
      startWrite();
      set_clock_write();
      for (;;) {                // For each command...
        cmd     = *addr++;  // Read, issue command
        numArgs = *addr++;  // Number of args to follow
        if (0xFF == (cmd & numArgs)) break;
        write_cmd(cmd);
        ms = numArgs & CMD_INIT_DELAY;       // If hibit set, delay follows args
        numArgs &= ~CMD_INIT_DELAY;          // Mask out delay bit

        while (numArgs--) {                   // For each argument...
          writeData(*addr++);  // Read, issue argument
        }
        if (ms) {
          ms = *addr++;        // Read post-command delay time (ms)
          delay( (ms==255 ? 500 : ms) );
        }
      }
      endWrite();
      return true;
    }

    void write_cmd(std::uint_fast8_t cmd)
    {
      if (_spi_dlen == 16) { cmd <<= 8; }
      auto *datreg = &_sercom->SPI.DATA.reg;
      auto *lenreg = &_sercom->SPI.LENGTH.reg;
      dc_l();
      *lenreg = (_spi_dlen>>3) | SERCOM_SPI_LENGTH_LENEN;
      *datreg = cmd;
      _need_wait = true;
    }

    void write_data(std::uint32_t data, std::uint32_t bit_length)
    {
      auto *datreg = &_sercom->SPI.DATA.reg;
      auto *lenreg = &_sercom->SPI.LENGTH.reg;
      auto len = (bit_length>>3) | SERCOM_SPI_LENGTH_LENEN;
      if (len > 1 && !_sercom->SPI.CTRLC.bit.DATA32B) {
        disableSPI();
        _sercom->SPI.CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable
        enableSPI();
      }
      dc_h();
      *lenreg = len;
      *datreg = data;
      _need_wait = true;
    }

    void set_window(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye)
    {
      std::uint32_t len;
      if (_spi_dlen == 8) {
        len = _len_setwindow;
      } else {
        len = (_len_setwindow << 1);
      }
      auto fp = fpGetWindowAddr;

      if (_xs != xs || _xe != xe) {
        write_cmd(_cmd_caset);
        _xs = xs;
        _xe = xe;
        std::uint32_t tmp = _colstart;

        tmp = fp(xs + tmp, xe + tmp);
        if (_spi_dlen == 8) {
          write_data(tmp, len);
        } else if (_spi_dlen == 16) {
          write_data((tmp & 0xFF) << 8 | (tmp >> 8) << 24, 32);
          tmp >>= 16;
          write_data((tmp & 0xFF) << 8 | (tmp >> 8) << 24, 32);
        }
      }
      if (_ys != ys || _ye != ye) {
        write_cmd(_cmd_raset);
        _ys = ys;
        _ye = ye;
        std::uint32_t tmp = _rowstart;

        tmp = fp(ys + tmp, ye + tmp);
        if (_spi_dlen == 8) {
          write_data(tmp, len);
        } else if (_spi_dlen == 16) {
          write_data((tmp & 0xFF) << 8 | (tmp >> 8) << 24, 32);
          tmp >>= 16;
          write_data((tmp & 0xFF) << 8 | (tmp >> 8) << 24, 32);
        }
      }
    }

    void start_read(void) {
      _fill_mode = false;
      dc_h();
      set_clock_read();
    }

    void end_read(void)
    {
      wait_spi();
      cs_h();

      if (_panel->spi_cs < 0) {
        write_cmd(0); // NOP command
      }
      set_clock_write();
      _fill_mode = false;

      cs_l();
    }

    std::uint32_t read_data(std::uint32_t length)
    {
      dc_h();
      write_data(0, length);
      return _sercom->SPI.DATA.reg;
    }

    std::uint32_t read_command(std::uint_fast8_t command, std::uint32_t bitindex = 0, std::uint32_t bitlen = 8)
    {
      startWrite();
      write_cmd(command);
      start_read();
      if (bitindex) read_data(bitindex>>3);
      std::uint32_t res = read_data(bitlen>>3);
      end_read();
      endWrite();
      return res;
    }

    void pushImage_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param, bool use_dma) override
    {
      auto bytes = _write_conv.bytes;
      auto src_x = param->src_x;
      auto fp_copy = param->fp_copy;

      std::int32_t xr = (x + w) - 1;

      if (param->transp == ~0u) {
        if (param->no_convert) {
          setWindow_impl(x, y, xr, y + h - 1);
          std::uint32_t i = (src_x + param->src_y * param->src_width) * bytes;
          auto src = &((const std::uint8_t*)param->src_data)[i];

          if ((std::int32_t)param->src_width == w || h == 1) {
            std::int32_t len = w * h * bytes;
            write_bytes(src, len, use_dma);
          } else {
            auto add = param->src_width * bytes;
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
          write_bytes(buf, w * bytes, use_dma);
          while (--h) {
            param->src_x = src_x;
            param->src_y++;
            buf = get_dmabuffer(w * bytes);
            fp_copy(buf, 0, w, param);
            write_bytes(buf, w * bytes, use_dma);
          }
        } else {
          setWindow_impl(x, y, xr, y + h - 1);
          do {
            push_colors(w, param);
            param->src_x = src_x;
            param->src_y++;
          } while (--h);
//*/
        }
      } else {
        auto fp_skip = param->fp_skip;
        h += y;
        do {
          std::int32_t i = 0;
          while (w != (i = fp_skip(i, w, param))) {
            auto buf = get_dmabuffer(w * bytes);
            std::int32_t len = fp_copy(buf, 0, w - i, param);
            setWindow_impl(x + i, y, x + i + len - 1, y);
            write_bytes(buf, len * bytes, true);
            if (w == (i += len)) break;
          }
          param->src_x = src_x;
          param->src_y++;
        } while (++y != h);
      }
//*/
    }

    void pushColors_impl(std::int32_t length, pixelcopy_t* param) override
    {
      push_colors(length, param);
    }

    void push_colors(std::int32_t length, pixelcopy_t* param)
    {
      const std::uint8_t bytes = _write_conv.bytes;
      const std::uint32_t limit = (bytes == 2) ? 2 : 1;
      std::uint32_t buf;
      auto *reg = &_sercom->SPI.DATA.reg;
      auto *lenreg = &_sercom->SPI.LENGTH.reg;

      if (bytes == 2 && (length & 0x01)) {
        param->fp_copy(&buf, 0, 1, param);
        dc_h();
        *lenreg = 2 | SERCOM_SPI_LENGTH_LENEN;
        *reg = buf;
        --length;
        if (!length) return;
      }
      param->fp_copy(&buf, 0, limit, param);
      dc_h();
      *lenreg = (limit*bytes) | SERCOM_SPI_LENGTH_LENEN;
      *reg = buf;
      while (0 != (length -= limit)) {
        param->fp_copy(&buf, 0, limit, param);
        wait_spi();
        *reg = buf;
      };
    }

    void write_bytes(const std::uint8_t* data, std::int32_t length, bool use_dma = false)
    {
      if (use_dma && length > 31) {
        std::uint_fast8_t beatsize = _sercom->SPI.CTRLC.bit.DATA32B ? 2 : 0;
        if ((bool)(beatsize) == ((length & 3) || ((std::uint32_t)data & 3))) {
          beatsize = 2 - beatsize;
          disableSPI();
          _sercom->SPI.CTRLC.bit.DATA32B = (bool)(beatsize);
          enableSPI();
        }
        dc_h();

        _sercom->SPI.LENGTH.reg = 0;

        auto desc = _dma_write_desc;
        desc->SRCADDR.reg = (std::uint32_t)data + length;
        desc->BTCNT.reg            = length >> beatsize;
        desc->BTCTRL.bit.BEATSIZE  = beatsize;

        _dma_adafruit.startJob();

/*
//        std::uint8_t channel = _dma_channel;
//        DMAC->Channel[channel].CHCTRLA.bit.ENABLE  = 0;
        //DMAC->CTRL.bit.DMAENABLE    = 0; // Disable DMA controller
        //DMAC->BASEADDR.bit.BASEADDR = (std::uint32_t)&_dmadesc[1];
        //DMAC->WRBADDR.bit.WRBADDR   = (std::uint32_t)&_dmadesc[0];
        //DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xF);
        std::uint8_t interruptMask = 2;
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

      auto *reg = &_sercom->SPI.DATA.reg;
      std::int32_t idx = length & 0x03;
      std::uint32_t buf = *(std::uint32_t*)data;
      if (idx) {
        dc_h();
        _sercom->SPI.LENGTH.reg = idx | SERCOM_SPI_LENGTH_LENEN;
        *reg = buf;
        _need_wait = true;
        if (length == idx) return;
        buf = *(std::uint32_t*)&data[idx];
      }

      dc_h();
      _sercom->SPI.LENGTH.reg = 4 | SERCOM_SPI_LENGTH_LENEN;
      *reg = buf;
      _need_wait = true;
      while (length != (idx += 4)) {
        buf = *(std::uint32_t*)&data[idx];
        wait_spi();
        *reg = buf;
      };
    }

    void readRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param) override
    {
      set_window(x, y, x + w - 1, y + h - 1);
      auto len = w * h;
      if (!_panel->spi_read) {
        memset(dst, 0, len * _read_conv.bytes);
        return;
      }
      write_cmd(_panel->getCmdRamrd());
      std::uint32_t len_dummy_read_pixel = _panel->len_dummy_read_pixel;
      start_read();
      if (len_dummy_read_pixel) {;
        write_data(0, len_dummy_read_pixel);
      }

      if (param->no_convert) {
        read_bytes((std::uint8_t*)dst, len * _read_conv.bytes);
      } else {
        read_pixels(dst, len, param);
      }
//*/
      end_read();
    }

    void read_pixels(void* dst, std::int32_t length, pixelcopy_t* param)
    {
      std::int32_t bytes = _read_conv.bytes;
      std::int32_t dstindex = 0;
      std::int32_t len = 4;
      std::uint8_t buf[24];
      param->src_data = buf;
      do {
        if (len > length) len = length;
        read_bytes(buf, len * bytes);
        param->src_x = 0;
        dstindex = param->fp_copy(dst, dstindex, dstindex + len, param);
        length -= len;
      } while (length);
    }

    void read_bytes(std::uint8_t* dst, std::int32_t length)
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
      std::int32_t len1 = length > 3 ? 4 : length;
      std::int32_t len2 = len1;
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
    }

    void copyRect_impl(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y) override
    {
      pixelcopy_t p((void*)nullptr, _write_conv.depth, _read_conv.depth);
      if (w < h) {
        const std::uint32_t buflen = h * _write_conv.bytes;
        auto buf = get_dmabuffer(buflen);
        std::int32_t add = (src_x < dst_x) ?   - 1 : 1;
        std::int32_t pos = (src_x < dst_x) ? w - 1 : 0;
        do {
          readRect_impl(src_x + pos, src_y, 1, h, buf, &p);
          setWindow_impl(dst_x + pos, dst_y, dst_x + pos, dst_y + h - 1);
          write_bytes(buf, buflen, true);
          pos += add;
        } while (--w);
      } else {
        const std::uint32_t buflen = w * _write_conv.bytes;
        auto buf = get_dmabuffer(buflen);
        std::int32_t add = (src_y < dst_y) ?   - 1 : 1;
        std::int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        do {
          readRect_impl(src_x, src_y + pos, w, 1, buf, &p);
          setWindow_impl(dst_x, dst_y + pos, dst_x + w - 1, dst_y + pos);
          write_bytes(buf, buflen, true);
          pos += add;
        } while (--h);
      }
    }

    struct _dmabufs_t {
      std::uint8_t* buffer = nullptr;
      std::uint32_t length = 0;
      void free(void) {
        if (buffer) {
          heap_free(buffer);
          buffer = nullptr;
          length = 0;
        }
      }
    };

    std::uint8_t* get_dmabuffer(std::uint32_t length)
    {
      _dma_flip = !_dma_flip;
      length = (length + 3) & ~3;
      if (_dmabufs[_dma_flip].length < length) {
        _dmabufs[_dma_flip].free();
        _dmabufs[_dma_flip].buffer = (std::uint8_t*)heap_alloc_dma(length);
        _dmabufs[_dma_flip].length = _dmabufs[_dma_flip].buffer ? length : 0;
      }
      return _dmabufs[_dma_flip].buffer;
      return nullptr;
    }

    void delete_dmabuffer(void)
    {
      _dmabufs[0].free();
      _dmabufs[1].free();
    }

    //static void _alloc_dmadesc(size_t len)
    //{
    //  if (_dmadesc) heap_free(_dmadesc);
    //  _dmadesc_len = len;
    //  _dmadesc = (DmacDescriptor*)memalign(16, sizeof(DmacDescriptor) * len);
    //  if (_dmadesc) memset(_dmadesc, 0, sizeof(DmacDescriptor) * len);
    //}

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

    void cs_h(void) {
      gpio_hi(_panel->spi_cs);
    }
    void cs_l(void) {
      gpio_lo(_panel->spi_cs);
    }

    static constexpr int _spi_mosi = get_spi_mosi<CFG, -1>::value;
    static constexpr int _spi_miso = get_spi_miso<CFG, -1>::value;
    static constexpr int _spi_sclk = get_spi_sclk<CFG, -1>::value;
    static constexpr int _spi_dlen = get_spi_dlen<CFG,  8>::value;

    PanelCommon* _panel = nullptr;
    std::uint32_t(*fpGetWindowAddr)(std::uint_fast16_t, std::uint_fast16_t);
    std::uint_fast16_t _colstart;
    std::uint_fast16_t _rowstart;
    std::uint_fast16_t _xs;
    std::uint_fast16_t _xe;
    std::uint_fast16_t _ys;
    std::uint_fast16_t _ye;
    std::uint32_t _cmd_caset;
    std::uint32_t _cmd_raset;
    std::uint32_t _cmd_ramwr;
    std::uint32_t _last_apb_freq;
    std::uint32_t _clkdiv_write;
    std::uint32_t _clkdiv_read;
    std::uint32_t _clkdiv_fill;
    std::uint32_t _len_setwindow;
    _dmabufs_t _dmabufs[2];
    bool _begun_tr = false;
    bool _dma_flip = false;
    bool _fill_mode;
    std::uint32_t _mask_reg_dc;
    volatile std::uint32_t* _gpio_reg_dc_h;
    volatile std::uint32_t* _gpio_reg_dc_l;

#if defined (ARDUINO)
    Adafruit_ZeroDMA _dma_adafruit;
    DmacDescriptor* _dma_write_desc;
#endif

    static DmacDescriptor* _dmadesc;
    static std::uint32_t _dmadesc_len;
    static bool _need_wait;

    static Sercom* _sercom;
  };
  template <class T> DmacDescriptor* LGFX_SPI<T>::_dmadesc = nullptr;
  template <class T> std::uint32_t LGFX_SPI<T>::_dmadesc_len = 0;
  template <class T> bool LGFX_SPI<T>::_need_wait;

  template <class T> Sercom* LGFX_SPI<T>::_sercom;

//----------------------------------------------------------------------------

}
#endif
