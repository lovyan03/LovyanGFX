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
#include <wiring_private.h>

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
  MEMBER_DETECTOR(dma_channel, get_dma_channel, get_dma_channel_impl, int)
  #undef MEMBER_DETECTOR


static const struct {
  Sercom   *sercomPtr;
  uint8_t   id_core;
  uint8_t   id_slow;
  IRQn_Type irq[4];
} sercomData[] = {
  { SERCOM0, SERCOM0_GCLK_ID_CORE, SERCOM0_GCLK_ID_SLOW,
    SERCOM0_0_IRQn, SERCOM0_1_IRQn, SERCOM0_2_IRQn, SERCOM0_3_IRQn },
  { SERCOM1, SERCOM1_GCLK_ID_CORE, SERCOM1_GCLK_ID_SLOW,
    SERCOM1_0_IRQn, SERCOM1_1_IRQn, SERCOM1_2_IRQn, SERCOM1_3_IRQn },
  { SERCOM2, SERCOM2_GCLK_ID_CORE, SERCOM2_GCLK_ID_SLOW,
    SERCOM2_0_IRQn, SERCOM2_1_IRQn, SERCOM2_2_IRQn, SERCOM2_3_IRQn },
  { SERCOM3, SERCOM3_GCLK_ID_CORE, SERCOM3_GCLK_ID_SLOW,
    SERCOM3_0_IRQn, SERCOM3_1_IRQn, SERCOM3_2_IRQn, SERCOM3_3_IRQn },
  { SERCOM4, SERCOM4_GCLK_ID_CORE, SERCOM4_GCLK_ID_SLOW,
    SERCOM4_0_IRQn, SERCOM4_1_IRQn, SERCOM4_2_IRQn, SERCOM4_3_IRQn },
  { SERCOM5, SERCOM5_GCLK_ID_CORE, SERCOM5_GCLK_ID_SLOW,
    SERCOM5_0_IRQn, SERCOM5_1_IRQn, SERCOM5_2_IRQn, SERCOM5_3_IRQn },
#if defined(SERCOM6)
  { SERCOM6, SERCOM6_GCLK_ID_CORE, SERCOM6_GCLK_ID_SLOW,
    SERCOM6_0_IRQn, SERCOM6_1_IRQn, SERCOM6_2_IRQn, SERCOM6_3_IRQn },
#endif
#if defined(SERCOM7)
  { SERCOM7, SERCOM7_GCLK_ID_CORE, SERCOM7_GCLK_ID_SLOW,
    SERCOM7_0_IRQn, SERCOM7_1_IRQn, SERCOM7_2_IRQn, SERCOM7_3_IRQn },
#endif
};


  template <class CFG>
  class LGFX_SPI : public LovyanGFX
  {

  public:

    virtual ~LGFX_SPI() {
    }

    LGFX_SPI() : LovyanGFX()
    {
      _panel = nullptr;

_sercom = SERCOM7;

    }

    void setPanel(PanelCommon* panel) { _panel = panel; postSetPanel(); }

    __attribute__ ((always_inline)) inline PanelCommon* getPanel(void) const { return _panel; }

    __attribute__ ((always_inline)) inline int_fast8_t getRotation(void) const { return _panel->rotation; }

    __attribute__ ((always_inline)) inline bool getInvert(void) const { return _panel->invert; }

    __attribute__ ((always_inline)) inline void dmaWait(void) const { wait_spi(); }

    __attribute__ ((always_inline)) inline void begin(void) { init(); }

    void init(void) { initBus(); initPanel(); }

    // Write single byte as COMMAND
    void writeCommand(uint_fast8_t cmd) { startWrite(); write_cmd(cmd); endWrite(); } // AdafruitGFX compatible
    void writecommand(uint_fast8_t cmd) { startWrite(); write_cmd(cmd); endWrite(); } // TFT_eSPI compatible

    // Write single bytes as DATA
    void spiWrite( uint_fast8_t data) { startWrite(); if (_spi_dlen == 16) { write_data(data << 8, _spi_dlen); } else { write_data(data, _spi_dlen); } endWrite(); } // AdafruitGFX compatible
    void writeData(uint_fast8_t data) { startWrite(); if (_spi_dlen == 16) { write_data(data << 8, _spi_dlen); } else { write_data(data, _spi_dlen); } endWrite(); } // TFT_eSPI compatible
    void writedata(uint_fast8_t data) { startWrite(); if (_spi_dlen == 16) { write_data(data << 8, _spi_dlen); } else { write_data(data, _spi_dlen); } endWrite(); } // TFT_eSPI compatible

    // Read data
    uint8_t  readCommand8( uint_fast8_t commandByte, uint_fast8_t index=0) { return read_command(commandByte, index << 3, 8); }
    uint8_t  readcommand8( uint_fast8_t commandByte, uint_fast8_t index=0) { return read_command(commandByte, index << 3, 8); }
    uint16_t readCommand16(uint_fast8_t commandByte, uint_fast8_t index=0) { return __builtin_bswap16(read_command(commandByte, index << 3, 16)); }
    uint16_t readcommand16(uint_fast8_t commandByte, uint_fast8_t index=0) { return __builtin_bswap16(read_command(commandByte, index << 3, 16)); }
    uint32_t readCommand32(uint_fast8_t commandByte, uint_fast8_t index=0) { return __builtin_bswap32(read_command(commandByte, index << 3, 32)); }
    uint32_t readcommand32(uint_fast8_t commandByte, uint_fast8_t index=0) { return __builtin_bswap32(read_command(commandByte, index << 3, 32)); }

    void setColorDepth(uint8_t bpp) { setColorDepth((color_depth_t)bpp); }

    void sleep()  { writeCommand(_panel->getCmdSlpin()); }

    void wakeup() { writeCommand(_panel->getCmdSlpout()); }

    void setColorDepth(color_depth_t depth)
    {
      commandList(_panel->getColorDepthCommands((uint8_t*)_regbuf, depth));
      postSetColorDepth();
    }

    void setRotation(int_fast8_t r)
    {
      commandList(_panel->getRotationCommands((uint8_t*)_regbuf, r));
      postSetRotation();
    }

    void invertDisplay(bool i)
    {
      commandList(_panel->getInvertDisplayCommands((uint8_t*)_regbuf, i));
    }

    void setBrightness(uint8_t brightness) {
      _panel->setBrightness(brightness);
    }

    uint32_t readPanelID(void)
    {
      return read_command(_panel->getCmdRddid(), _panel->len_dummy_read_rddid, 32);
    }


uint32_t freqRef; // Frequency corresponding to clockSource

void setClockSource(int8_t idx, SercomClockSource src, bool core) {

  if(src == SERCOM_CLOCK_SOURCE_NO_CHANGE) return;

  uint8_t clk_id = core ? sercomData[idx].id_core : sercomData[idx].id_slow;

  GCLK->PCHCTRL[clk_id].bit.CHEN = 0;     // Disable timer
  while(GCLK->PCHCTRL[clk_id].bit.CHEN);  // Wait for disable

//  if(core) clockSource = src; // Save SercomClockSource value

  // From cores/arduino/startup.c:
  // GCLK0 = F_CPU
  // GCLK1 = 48 MHz
  // GCLK2 = 100 MHz
  // GCLK3 = XOSC32K
  // GCLK4 = 12 MHz
  if(src == SERCOM_CLOCK_SOURCE_FCPU) {
    GCLK->PCHCTRL[clk_id].reg =
      GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
    if(core) freqRef = F_CPU; // Save clock frequency value
  } else if(src == SERCOM_CLOCK_SOURCE_48M) {
    GCLK->PCHCTRL[clk_id].reg =
      GCLK_PCHCTRL_GEN_GCLK1_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
    if(core) freqRef = 48000000;
  } else if(src == SERCOM_CLOCK_SOURCE_100M) {
    GCLK->PCHCTRL[clk_id].reg =
      GCLK_PCHCTRL_GEN_GCLK2_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
    if(core) freqRef = 100000000;
  } else if(src == SERCOM_CLOCK_SOURCE_32K) {
    GCLK->PCHCTRL[clk_id].reg =
      GCLK_PCHCTRL_GEN_GCLK3_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
    if(core) freqRef = 32768;
  } else if(src == SERCOM_CLOCK_SOURCE_12M) {
    GCLK->PCHCTRL[clk_id].reg =
      GCLK_PCHCTRL_GEN_GCLK4_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
    if(core) freqRef = 12000000;
  }

  while(!GCLK->PCHCTRL[clk_id].bit.CHEN); // Wait for clock enable
}

void resetSPI()
{
  //Setting the Software Reset bit to 1
  _sercom->SPI.CTRLA.bit.SWRST = 1;

  //Wait both bits Software Reset from CTRLA and SYNCBUSY are equal to 0
  while(_sercom->SPI.CTRLA.bit.SWRST || _sercom->SPI.SYNCBUSY.bit.SWRST);
}

void enableSPI()
{
  //Setting the enable bit to 1
  _sercom->SPI.CTRLA.bit.ENABLE = 1;

  while(_sercom->SPI.SYNCBUSY.bit.ENABLE)
  {
    //Waiting then enable bit from SYNCBUSY is equal to 0;
  }
}

void disableSPI()
{
  while(_sercom->SPI.SYNCBUSY.bit.ENABLE)
  {
    //Waiting then enable bit from SYNCBUSY is equal to 0;
  }

  //Setting the enable bit to 0
  _sercom->SPI.CTRLA.bit.ENABLE = 0;
}
    void initBus(void)
    {
      pinPeripheral(_spi_miso, g_APinDescription[_spi_miso].ulPinType);
      pinPeripheral(_spi_sclk, g_APinDescription[_spi_sclk].ulPinType);
      pinPeripheral(_spi_mosi, g_APinDescription[_spi_mosi].ulPinType);


      disableSPI();

//    initSPI(_padTx, _padRx, SPI_CHAR_SIZE_8_BITS, settings.bitOrder);
      resetSPI();

//      initClockNVIC();
  int8_t idx = CFG::sercom_index;
  for(uint8_t i=0; i<4; i++) {
    NVIC_ClearPendingIRQ(sercomData[idx].irq[i]);
    NVIC_SetPriority(sercomData[idx].irq[i], SERCOM_NVIC_PRIORITY);
    NVIC_EnableIRQ(sercomData[idx].irq[i]);
  }

  // SPI DMA speed is dictated by the "slow clock" (I think...maybe) so
  // BOTH are set to the same clock source (clk_slow isn't sourced from
  // XOSC32K as in prior versions of SAMD core).
  // This might have power implications for sleep code.
SercomClockSource clockSource;
  clockSource = SERCOM_CLOCK_SOURCE_FCPU;
//  clockSource = SERCOM_CLOCK_SOURCE_48M;
//  clockSource = SERCOM_CLOCK_SOURCE_100M;

  setClockSource(idx, clockSource, true);  // true  = core clock
  setClockSource(idx, clockSource, false); // false = slow clock
  setClockSource(idx, clockSource, true);  // true  = core clock

SercomSpiTXPad mosi = PAD_SPI3_TX;
SercomRXPad    miso = PAD_SPI3_RX;
SercomDataOrder dataOrder = MSB_FIRST;
SercomSpiCharSize charSize = SPI_CHAR_SIZE_8_BITS;

      _sercom->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE(0x3)  |  // master mode
                              SERCOM_SPI_CTRLA_DOPO(mosi) |
                              SERCOM_SPI_CTRLA_DIPO(miso) |
                              dataOrder << SERCOM_SPI_CTRLA_DORD_Pos;

      //Setting the CTRLB register
      _sercom->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE(charSize) |
                              SERCOM_SPI_CTRLB_RXEN; //Active the SPI receiver.

      _sercom->SPI.CTRLC.bit.DATA32B = 1;  // 4Byte transfer enable

      _sercom->SPI.BAUD.reg = 0;

      while( _sercom->SPI.SYNCBUSY.bit.CTRLB == 1 );


      enableSPI();

      _sercom->SPI.DATA.bit.DATA = 0;   // dummy send data
    }

    virtual void initPanel(void)
    {
      if (!_panel) return;

      _panel->init();

      startWrite();

      const uint8_t *cmds;
      for (uint8_t i = 0; (cmds = _panel->getInitCommands(i)); i++) {
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

    void pushPixelsDMA(const void* data, uint32_t length) {
      write_bytes((const uint8_t*)data, length, true);
    }


//----------------------------------------------------------------------------
  protected:

    void postSetPanel(void)
    {
      _last_apb_freq = -1;
      _cmd_ramwr      = _panel->getCmdRamwr();
      _len_setwindow  = _panel->len_setwindow;
      fpGetWindowAddr = _len_setwindow == 32 ? PanelCommon::getWindowAddr32 : PanelCommon::getWindowAddr16;

      int32_t spi_dc = _panel->spi_dc;

      EPortType port = g_APinDescription[spi_dc].ulPort;
      uint32_t pin = g_APinDescription[spi_dc].ulPin;
      _mask_reg_dc = (1ul << pin);
      _gpio_reg_dc_h = &PORT->Group[port].OUTSET.reg;
      _gpio_reg_dc_l = &PORT->Group[port].OUTCLR.reg;
      pinMode(spi_dc, OUTPUT);

      cs_h();
      pinMode(_panel->spi_cs, OUTPUT);

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
//      _sercom->SPI.BAUD.reg = 5;

//      initSPIClock(settings.dataMode, settings.clockFreq);

/*
      int cpha = _panel->spi_mode & 1;
      int cpol = (_panel->spi_mode & 2) >> 1;

      //Setting the CTRLA register
      _sercom->SPI.CTRLA.reg |= ( cpha << SERCOM_SPI_CTRLA_CPHA_Pos ) |
                                    ( cpol << SERCOM_SPI_CTRLA_CPOL_Pos );

      //Synchronous arithmetic
      _sercom->SPI.BAUD.reg = 4; //calculateBaudrateSynchronous(_panel->freq_write);

/*
      _fill_mode = false;
      uint32_t apb_freq = getApbFrequency();
      if (_last_apb_freq != apb_freq) {
        _last_apb_freq = apb_freq;
        _clkdiv_read  = FreqToClockDiv(apb_freq, _panel->freq_read);
        _clkdiv_fill  = FreqToClockDiv(apb_freq, _panel->freq_fill);
        _clkdiv_write = FreqToClockDiv(apb_freq, _panel->freq_write);
      }

      auto spi_mode = _panel->spi_mode;
      uint32_t user = (spi_mode == 1 || spi_mode == 2) ? SPI_CK_OUT_EDGE | SPI_USR_MOSI : SPI_USR_MOSI;
      uint32_t pin = (spi_mode & 2) ? SPI_CK_IDLE_EDGE : 0;

//    wait_spi();

#if defined (ARDUINO) // Arduino ESP32
      spiSimpleTransaction(_sercom);

      if (_dma_channel) {
        _next_dma_reset = true;
      }
#elif defined (CONFIG_IDF_TARGET_ESP32) // ESP-IDF
      if (_sercom) {
        if (ESP_OK != spi_device_acquire_bus(_sercom, portMAX_DELAY)) {
          ESP_LOGE("LGFX", "Failed to spi_device_acquire_bus. ");
        }
      }
#endif

      *reg(SPI_USER_REG(_spi_port)) = user;
      *reg(SPI_PIN_REG(_spi_port))  = pin;
      set_clock_write();

//*/
      cs_l();
    }

    void endTransaction_impl(void) override {
      if (!_begun_tr) return;
      _begun_tr = false;
      end_transaction();
    }

    void end_transaction(void) {
      while (_sercom->SPI.INTFLAG.bit.TXC == 0); // Waiting Complete Reception
      if (_panel->spi_cs < 0) {
        write_cmd(0); // NOP command
      }
      dc_h();
      cs_h();
/*
#if defined (ARDUINO) // Arduino ESP32
      *reg(SPI_USER_REG(_spi_port)) = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN; // for other SPI device (SD)
      spiEndTransaction(_sercom);
#elif defined (CONFIG_IDF_TARGET_ESP32) // ESP-IDF
      if (_sercom) {
        spi_device_release_bus(_sercom);
      }
#endif
//*/
    }

    void waitDMA_impl(void) override
    {
      wait_spi();
    }

    void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      if (_fill_mode) {
        _fill_mode = false;
        wait_spi();
        set_clock_write();
      }
      set_window(xs, ys, xe, ye);
      write_cmd(_cmd_ramwr);
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      if (_begun_tr) {
//        if (_fill_mode) {
//          _fill_mode = false;
//          wait_spi();
//          set_clock_write();
//        }
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
//      if (_fill_mode) {
//        _fill_mode = false;
//        wait_spi();
//        set_clock_write();
//      }
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
      auto *reg = &_sercom->SPI.DATA.reg;
      uint32_t data = _color.raw;
      int bytes = _write_conv.bytes;
      if (bytes == 2 && length > 1) {
        if (length & 0x01) {
          --length;
          dc_h();
          _sercom->SPI.LENGTH.reg = 2 | 256;
          *reg = data;
          if (!length) return;
        }
        data |= _color.raw << 16;
        length >>= 1;
        bytes = 4;
      }
      dc_h();
      _sercom->SPI.LENGTH.reg = bytes|256;
      *reg = data;
      while (--length) {
        wait_spi();
        *reg = data;
      };
    }

    bool commandList(const uint8_t *addr)
    {
      if (addr == nullptr) return false;
      uint8_t  cmd;
      uint8_t  numArgs;
      uint8_t  ms;

      _fill_mode = false;
      wait_spi();
      startWrite();
      set_clock_write();
      for (;;) {                // For each command...
        cmd     = pgm_read_byte(addr++);  // Read, issue command
        numArgs = pgm_read_byte(addr++);  // Number of args to follow
        if (0xFF == (cmd & numArgs)) break;
        write_cmd(cmd);
        ms = numArgs & CMD_INIT_DELAY;       // If hibit set, delay follows args
        numArgs &= ~CMD_INIT_DELAY;          // Mask out delay bit

        while (numArgs--) {                   // For each argument...
          writeData(pgm_read_byte(addr++));  // Read, issue argument
        }
        if (ms) {
          ms = pgm_read_byte(addr++);        // Read post-command delay time (ms)
          delay( (ms==255 ? 500 : ms) );
        }
      }
      endWrite();
      return true;
    }

    void write_cmd(uint_fast8_t cmd)
    {
      auto *reg = &_sercom->SPI.DATA.reg;
//      auto *intflag = &_sercom->SPI.INTFLAG;
//      while (intflag->bit.TXC == 0); // Waiting Complete Reception
      dc_l();
      _sercom->SPI.LENGTH.reg = 1|256;
      *reg = cmd; // Writing data into Data register
/*
      if (_spi_dlen == 16) { cmd <<= 8; }
      auto spi_w0_reg        = reg(SPI_W0_REG(_spi_port));
      auto spi_mosi_dlen_reg = reg(SPI_MOSI_DLEN_REG(_spi_port));
      dc_l();
      *spi_mosi_dlen_reg = _spi_dlen - 1;
      *spi_w0_reg = cmd;
      exec_spi();
//*/
    }

    void write_data(uint32_t data, uint32_t bit_length)
    {
      auto *reg = &_sercom->SPI.DATA.reg;
      dc_h();
      _sercom->SPI.LENGTH.reg = (bit_length>>3)|256;
      *reg = data;
/*
      while (bit_length > 8) {
        bit_length -= 8;
        data >>= 8;
        wait_spi();
        *reg = data;
//        _sercom->SPI.DATA.bit.DATA = data; // Writing data into Data register
      };
/*
      auto spi_w0_reg        = reg(SPI_W0_REG(_spi_port));
      auto spi_mosi_dlen_reg = reg(SPI_MOSI_DLEN_REG(_spi_port));
      dc_h();
      *spi_mosi_dlen_reg = bit_length - 1;
      *spi_w0_reg = data;
      exec_spi();
//*/
    }

    void set_window(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
    {
      uint32_t len;
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
        uint32_t tmp = _colstart;

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
        uint32_t tmp = _rowstart;

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
/*
      _fill_mode = false;
      uint32_t user = ((_panel->spi_mode_read == 1 || _panel->spi_mode_read == 2) ? SPI_CK_OUT_EDGE | SPI_USR_MISO : SPI_USR_MISO)
                    | (_panel->spi_3wire ? SPI_SIO : 0);
      uint32_t pin = (_panel->spi_mode_read & 2) ? SPI_CK_IDLE_EDGE : 0;
      dc_h();
      *reg(SPI_USER_REG(_spi_port)) = user;
      *reg(SPI_PIN_REG(_spi_port)) = pin;
      set_clock_read();
//*/
    }

    void end_read(void)
    {
/*
      uint32_t user = (_panel->spi_mode == 1 || _panel->spi_mode == 2) ? SPI_CK_OUT_EDGE | SPI_USR_MOSI : SPI_USR_MOSI;
      uint32_t pin = (_panel->spi_mode & 2) ? SPI_CK_IDLE_EDGE : 0;
      wait_spi();
      cs_h();
      *reg(SPI_USER_REG(_spi_port)) = user;
      *reg(SPI_PIN_REG(_spi_port)) = pin;
      if (_panel->spi_cs < 0) {
        write_cmd(0); // NOP command
      }
      set_clock_write();
      _fill_mode = false;

      cs_l();
//*/
    }

    uint32_t read_data(uint32_t length)
    {
/*
      set_read_len(length);
      exec_spi();
      wait_spi();
      return *reg(SPI_W0_REG(_spi_port));
//*/
    }

    uint32_t read_command(uint_fast8_t command, uint32_t bitindex = 0, uint32_t bitlen = 8)
    {
      startWrite();
      write_cmd(command);
      start_read();
      if (bitindex) read_data(bitindex);
      uint32_t res = read_data(bitlen);
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

      if (param->transp == ~0) {
        if (param->no_convert) {
          setWindow_impl(x, y, xr, y + h - 1);
          uint32_t i = (src_x + param->src_y * param->src_width) * bytes;
          auto src = &((const uint8_t*)param->src_data)[i];
/*
          if (_dma_channel && use_dma) {
            if (param->src_width == w) {
              _setup_dma_desc_links(src, w * h * bytes);
            } else {
              _setup_dma_desc_links(src, w * bytes, h, param->src_width * bytes);
            }
            dc_h();
            set_write_len(w * h * bytes << 3);
            *reg(SPI_DMA_OUT_LINK_REG(_spi_port)) = SPI_OUTLINK_START | ((int)(&_dmadesc[0]) & 0xFFFFF);
            spi_dma_transfer_active(_dma_channel);
            exec_spi();
            return;
          }
          if (param->src_width == w) {
            int32_t len = w * h * bytes;
            if (_dma_channel && !use_dma && (64 < len) && (len <= 1024)) {
              auto buf = get_dmabuffer(len);
              memcpy(buf, src, len);
              write_bytes(buf, len, true);
            } else {
              write_bytes(src, len, false);
            }
          } else
*/
          {
            auto add = param->src_width * bytes;
            do {
              write_bytes(src, w * bytes, use_dma);
              src += add;
            } while (--h);
          }
        } else
//        if (_dma_channel && use_dma)
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
/*
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
          int32_t i = 0;
          while (w != (i = fp_skip(i, w, param))) {
            auto buf = get_dmabuffer(w * bytes);
            int32_t len = fp_copy(buf, 0, w - i, param);
            setWindow_impl(x + i, y, x + i + len - 1, y);
            write_bytes(buf, len * bytes, use_dma);
            if (w == (i += len)) break;
          }
          param->src_x = src_x;
          param->src_y++;
        } while (++y != h);
      }
//*/
    }

    void pushColors_impl(int32_t length, pixelcopy_t* param) override
    {
      push_colors(length, param);
    }

    void push_colors(int32_t length, pixelcopy_t* param)
    {
/*
      const uint8_t bytes = _write_conv.bytes;
      const uint32_t limit = (bytes == 2) ? 16 : 10; //  limit = 32/bytes (bytes==2 is 16   bytes==3 is 10)
      uint32_t len = (length - 1) / limit;
      uint32_t highpart = (len & 1) << 3;
      len = length - (len * limit);
      param->fp_copy(_regbuf, 0, len, param);

      auto spi_w0_reg = reg(SPI_W0_REG(_spi_port));

      uint32_t user_reg = *reg(SPI_USER_REG(_spi_port));

      dc_h();
      set_write_len(len * bytes << 3);

      memcpy((void*)&spi_w0_reg[highpart], _regbuf, (len * bytes + 3) & (~3));
      if (highpart) *reg(SPI_USER_REG(_spi_port)) = user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      for (; length; length -= limit) {
        param->fp_copy(_regbuf, 0, limit, param);
        memcpy((void*)&spi_w0_reg[highpart ^= 0x08], _regbuf, limit * bytes);
        uint32_t user = user_reg;
        if (highpart) user |= SPI_USR_MOSI_HIGHPART;
        if (len != limit) {
          len = limit;
          wait_spi();
          set_write_len(limit * bytes << 3);
          *reg(SPI_USER_REG(_spi_port)) = user;
          exec_spi();
        } else {
          wait_spi();
          *reg(SPI_USER_REG(_spi_port)) = user;
          exec_spi();
        }
      }
//*/
    }

    void write_bytes(const uint8_t* data, int32_t length, bool use_dma = false)
    {
      auto *reg = &_sercom->SPI.DATA.reg;
      int32_t idx = length & 0x03;
      if (idx) {
        dc_h();
        _sercom->SPI.LENGTH.reg = (idx) | 256;
        *reg = *(uint32_t*)data;
        length -= idx;
        if (!length) return;
      }
      dc_h();
      _sercom->SPI.LENGTH.reg = 4|256;
      *reg = *(uint32_t*)&data[idx];
      while (length != (idx += 4)) {
        wait_spi();
        *reg = *(uint32_t*)&data[idx];
      };

/*
      if (length <= 64) {
        auto spi_w0_reg = reg(SPI_W0_REG(_spi_port));
        dc_h();
        set_write_len(length << 3);
        memcpy((void*)spi_w0_reg, data, (length + 3) & (~3));
        exec_spi();
        return;
      } else if (_dma_channel && use_dma) {
        dc_h();
        set_write_len(length << 3);
        _setup_dma_desc_links(data, length);
        *reg(SPI_DMA_OUT_LINK_REG(_spi_port)) = SPI_OUTLINK_START | ((int)(&_dmadesc[0]) & 0xFFFFF);
        spi_dma_transfer_active(_dma_channel);
        exec_spi();
        return;
      }
      constexpr uint32_t limit = 32;
      uint32_t len = ((length - 1) & 0x1F) + 1;
      uint32_t highpart = ((length - 1) & limit) >> 2; // 8 or 0

      auto spi_w0_reg = reg(SPI_W0_REG(_spi_port));

      uint32_t user_reg = *reg(SPI_USER_REG(_spi_port));
      dc_h();
      set_write_len(len << 3);

      memcpy((void*)&spi_w0_reg[highpart], data, (len + 3) & (~3));
      if (highpart) *reg(SPI_USER_REG(_spi_port)) = user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      for (; length; length -= limit) {
        data += len;
        memcpy((void*)&spi_w0_reg[highpart ^= 0x08], data, limit);
        uint32_t user = user_reg;
        if (highpart) user |= SPI_USR_MOSI_HIGHPART;
        if (len != limit) {
          len = limit;
          wait_spi();
          set_write_len(limit << 3);
          *reg(SPI_USER_REG(_spi_port)) = user;
          exec_spi();
        } else {
          wait_spi();
          *reg(SPI_USER_REG(_spi_port)) = user;
          exec_spi();
        }
      }
//*/
    }

    void readRect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) override
    {
/*
      set_window(x, y, x + w - 1, y + h - 1);
      auto len = w * h;
      if (!_panel->spi_read) {
        memset(dst, 0, len * _read_conv.bytes);
        return;
      }
      write_cmd(_panel->getCmdRamrd());
      uint32_t len_dummy_read_pixel = _panel->len_dummy_read_pixel;
      start_read();
      if (len_dummy_read_pixel) {;
        set_read_len(len_dummy_read_pixel);
        exec_spi();
      }

      if (param->no_convert) {
        read_bytes((uint8_t*)dst, len * _read_conv.bytes);
      } else {
        read_pixels(dst, len, param);
      }
      end_read();
//*/
    }

    void read_pixels(void* dst, int32_t length, pixelcopy_t* param)
    {
/*
      int32_t len1 = std::min(length, 10); // 10 pixel read
      int32_t len2 = len1;
      auto len_read_pixel  = _read_conv.bits;
      wait_spi();
      set_read_len(len_read_pixel * len1);
      exec_spi();
      param->src_data = _regbuf;
      int32_t dstindex = 0;
      uint32_t highpart = 8;
      uint32_t userreg = *reg(SPI_USER_REG(_spi_port));
      auto spi_w0_reg = reg(SPI_W0_REG(_spi_port));
      do {
        if (0 == (length -= len1)) {
          len2 = len1;
          wait_spi();
          *reg(SPI_USER_REG(_spi_port)) = userreg;
        } else {
          uint32_t user = userreg;
          if (highpart) user = userreg | SPI_USR_MISO_HIGHPART;
          if (length < len1) {
            len1 = length;
            wait_spi();
            set_read_len(len_read_pixel * len1);
          } else {
            wait_spi();
          }
          *reg(SPI_USER_REG(_spi_port)) = user;
          exec_spi();
        }
        memcpy(_regbuf, (void*)&spi_w0_reg[highpart ^= 8], len2 * len_read_pixel >> 3);
        param->src_x = 0;
        dstindex = param->fp_copy(dst, dstindex, dstindex + len2, param);
      } while (length);
//*/
    }

    void read_bytes(uint8_t* dst, int32_t length, bool use_dma = false)
    {
/*
      if (_dma_channel && use_dma) {
        wait_spi();
        set_read_len(length << 3);
        _setup_dma_desc_links(dst, length);
        *reg(SPI_DMA_IN_LINK_REG(_spi_port)) = SPI_INLINK_START | ((int)(&_dmadesc[0]) & 0xFFFFF);
        spi_dma_transfer_active(_dma_channel);
        exec_spi();
      } else {
        int32_t len1 = std::min(length, 32);  // 32 Byte read.
        int32_t len2 = len1;
        wait_spi();
        set_read_len(len1 << 3);
        exec_spi();
        uint32_t highpart = 8;
        uint32_t userreg = *reg(SPI_USER_REG(_spi_port));
        auto spi_w0_reg = reg(SPI_W0_REG(_spi_port));
        do {
          if (0 == (length -= len1)) {
            len2 = len1;
            wait_spi();
            *reg(SPI_USER_REG(_spi_port)) = userreg;
          } else {
            uint32_t user = userreg;
            if (highpart) user = userreg | SPI_USR_MISO_HIGHPART;
            if (length < len1) {
              len1 = length;
              wait_spi();
              set_read_len(len1 << 3);
            } else {
              wait_spi();
            }
            *reg(SPI_USER_REG(_spi_port)) = user;
            exec_spi();
          }
          memcpy(dst, (void*)&spi_w0_reg[highpart ^= 8], len2);
          dst += len2;
        } while (length);
      }
//*/
    }

    void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) override
    {
/*
      pixelcopy_t p((void*)nullptr, _write_conv.depth, _read_conv.depth);
      if (w < h) {
        const uint32_t buflen = h * _write_conv.bytes;
        auto buf = get_dmabuffer(buflen);
        int32_t add = (src_x < dst_x) ?   - 1 : 1;
        int32_t pos = (src_x < dst_x) ? w - 1 : 0;
        do {
          readRect_impl(src_x + pos, src_y, 1, h, buf, &p);
          setWindow_impl(dst_x + pos, dst_y, dst_x + pos, dst_y + h - 1);
          write_bytes(buf, buflen);
          pos += add;
        } while (--w);
      } else {
        const uint32_t buflen = w * _write_conv.bytes;
        auto buf = get_dmabuffer(buflen);
        int32_t add = (src_y < dst_y) ?   - 1 : 1;
        int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        do {
          readRect_impl(src_x, src_y + pos, w, 1, buf, &p);
          setWindow_impl(dst_x, dst_y + pos, dst_x + w - 1, dst_y + pos);
          write_bytes(buf, buflen);
          pos += add;
        } while (--h);
      }
//*/
    }

    struct _dmabufs_t {
      uint8_t* buffer = nullptr;
      uint32_t length = 0;
      void free(void) {
        if (buffer) {
          heap_free(buffer);
          buffer = nullptr;
          length = 0;
        }
//*/
      }
    };

    uint8_t* get_dmabuffer(uint32_t length)
    {
      _dma_flip = !_dma_flip;
      length = (length + 3) & ~3;
      if (_dmabufs[_dma_flip].length < length) {
        _dmabufs[_dma_flip].free();
        _dmabufs[_dma_flip].buffer = (uint8_t*)heap_alloc_dma(length);
        _dmabufs[_dma_flip].length = _dmabufs[_dma_flip].buffer ? length : 0;
      }
      return _dmabufs[_dma_flip].buffer;
//*/
      return nullptr;
    }


//    __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }
    __attribute__ ((always_inline)) inline void set_clock_write(void) { /* *reg(SPI_CLOCK_REG(_spi_port)) = _clkdiv_write; */ }
    __attribute__ ((always_inline)) inline void set_clock_read(void)  { /* *reg(SPI_CLOCK_REG(_spi_port)) = _clkdiv_read;  */ }
    __attribute__ ((always_inline)) inline void set_clock_fill(void)  { /* *reg(SPI_CLOCK_REG(_spi_port)) = _clkdiv_fill;  */ }
    __attribute__ ((always_inline)) inline void exec_spi(void) { /*        *reg(SPI_CMD_REG(_spi_port)) = SPI_USR;  */ }
    __attribute__ ((always_inline)) inline void wait_spi(void) { auto *intflag = &_sercom->SPI.INTFLAG.bit; while (intflag->TXC == 0); }
    __attribute__ ((always_inline)) inline void set_write_len(uint32_t bitlen) { /* *reg(SPI_MOSI_DLEN_REG(_spi_port)) = bitlen - 1; */ }
    __attribute__ ((always_inline)) inline void set_read_len( uint32_t bitlen) { /* *reg(SPI_MISO_DLEN_REG(_spi_port)) = bitlen - 1; */ }

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
      digitalWrite(_panel->spi_cs, HIGH);
/*
      int32_t spi_cs = _panel->spi_cs;
      if (spi_cs >= 0) *get_gpio_hi_reg(spi_cs) = (1 << (spi_cs & 31));
*/
    }
    void cs_l(void) {
      digitalWrite(_panel->spi_cs, LOW);
/*
      int32_t spi_cs = _panel->spi_cs;
      if (spi_cs >= 0) *get_gpio_lo_reg(spi_cs) = (1 << (spi_cs & 31));
*/
    }

    static constexpr int _dma_channel= get_dma_channel<CFG,  0>::value;
    static constexpr int _spi_mosi = get_spi_mosi<CFG, -1>::value;
    static constexpr int _spi_miso = get_spi_miso<CFG, -1>::value;
    static constexpr int _spi_sclk = get_spi_sclk<CFG, -1>::value;
    static constexpr int _spi_dlen = get_spi_dlen<CFG,  8>::value;

//    static constexpr spi_host_device_t _spi_host = get_spi_host<CFG, VSPI_HOST>::value;
//    static constexpr uint8_t _spi_port = (_spi_host == HSPI_HOST) ? 2 : 3;  // FSPI=1  HSPI=2  VSPI=3;

    PanelCommon* _panel = nullptr;
    uint32_t(*fpGetWindowAddr)(uint_fast16_t, uint_fast16_t);
    uint_fast16_t _colstart;
    uint_fast16_t _rowstart;
    uint_fast16_t _xs;
    uint_fast16_t _xe;
    uint_fast16_t _ys;
    uint_fast16_t _ye;
    uint32_t _cmd_caset;
    uint32_t _cmd_raset;
    uint32_t _cmd_ramwr;
    uint32_t _last_apb_freq;
    uint32_t _clkdiv_write;
    uint32_t _clkdiv_read;
    uint32_t _clkdiv_fill;
    uint32_t _len_setwindow;
    _dmabufs_t _dmabufs[2];
    bool _begun_tr = false;
    bool _dma_flip = false;
    bool _fill_mode;
    uint32_t _mask_reg_dc;
    volatile uint32_t* _gpio_reg_dc_h;
    volatile uint32_t* _gpio_reg_dc_l;
    static uint32_t _regbuf[8];
//    static lldesc_t* _dmadesc;
//    static uint32_t _dmadesc_len;
    static bool _next_dma_reset;

//    static volatile spi_dev_t *_hw;
    static Sercom* _sercom;
  };
  template <class T> uint32_t LGFX_SPI<T>::_regbuf[];
//  template <class T> lldesc_t* LGFX_SPI<T>::_dmadesc = nullptr;
//  template <class T> uint32_t LGFX_SPI<T>::_dmadesc_len = 0;
  template <class T> bool LGFX_SPI<T>::_next_dma_reset;

  template <class T> Sercom* LGFX_SPI<T>::_sercom;

//----------------------------------------------------------------------------

}
#endif
