#ifndef LGFX_SPI_ESP32_HPP_
#define LGFX_SPI_ESP32_HPP_

#include <type_traits>
#include <esp_heap_caps.h>

#include "esp32_common.hpp"
#include "lgfx_common.hpp"
#include "lgfx_sprite.hpp"
#include "panel_common.hpp"

namespace lgfx
{
  #define MEMBER_DETECTOR(member, classname, classname_impl, valuetype) struct classname_impl { \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, T::member> check(decltype(T::member)*); \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, V> check(...); \
  };template<class T, valuetype V> class classname : public decltype(classname_impl::check<T, V>(nullptr)) {};
  MEMBER_DETECTOR(spi_host  , get_spi_host  , get_spi_host_impl  , spi_host_device_t)
  MEMBER_DETECTOR(spi_mosi  , get_spi_mosi  , get_spi_mosi_impl  , int)
  MEMBER_DETECTOR(spi_miso  , get_spi_miso  , get_spi_miso_impl  , int)
  MEMBER_DETECTOR(spi_sclk  , get_spi_sclk  , get_spi_sclk_impl  , int)
  MEMBER_DETECTOR(dma_ch    , get_dma_ch    , get_dma_ch_impl    , int)
  MEMBER_DETECTOR(panel_rst , get_panel_rst , get_panel_rst_impl , int)
  #undef MEMBER_DETECTOR

  template <class CFG>
  class LGFX_SPI : public LovyanGFX
  {
  public:

    virtual ~LGFX_SPI() {
      if (_panel) delete _panel;
      _panel = nullptr;

      if (dmadesc_tx) heap_caps_free(dmadesc_tx);
      dmadesc_tx = nullptr;
    }

    LGFX_SPI() : LovyanGFX() {
//      _hw = (volatile spi_dev_t *)REG_SPI_BASE(_spi_port);

      if (nullptr != (_panel = createPanelFromConfig<CFG>(nullptr))) { postSetPanel(); }

      _dma_chan_claimed = false;
      if (0 != _dma_ch) {
        _dma_chan_claimed = spicommon_dma_chan_claim(_dma_ch);
        if ( _dma_chan_claimed ) {
          int dma_desc_ct=(320 * 80 * 2 + SPI_MAX_DMA_LEN-1)/SPI_MAX_DMA_LEN;
          dmadesc_tx = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t)*dma_desc_ct, MALLOC_CAP_DMA);
          for (int i = 0; i < dma_desc_ct; i++) {
            dmadesc_tx[i].sosf = 0;
            dmadesc_tx[i].owner = 1;
            dmadesc_tx[i].qe.stqe_next = &dmadesc_tx[i + 1];
          }
        }
      }
    }

    template <class T> inline void setPanel(const T& panel) { setPanel<T>(); }
    template <class T> inline void setPanel(void) { if (_panel) delete _panel; _panel = new T; postSetPanel(); }

    __attribute__ ((always_inline)) inline void begin(void) { init(); }
    __attribute__ ((always_inline)) inline void init(void) { initBus(); initPanel(); }

    void initBus(void)
    {
      TPin<_spi_mosi>::init(GPIO_MODE_INPUT_OUTPUT);
      TPin<_spi_miso>::init(GPIO_MODE_INPUT_OUTPUT);
      TPin<_spi_sclk>::init();
      TPin<_panel_rst>::init();
      TPin<_panel_rst>::lo();

      if (_spi_host == HSPI_HOST) {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI_RST);
      } else {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI_CLK_EN_2);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI_RST_2);
      }

      {//spiStopBus;
        *reg(SPI_CTRL_REG (_spi_port)) = 0;
        *reg(SPI_CTRL1_REG(_spi_port)) = 0;
        *reg(SPI_CTRL2_REG(_spi_port)) = 0;
        *reg(SPI_CLOCK_REG(_spi_port)) = 0;
        *reg(SPI_USER_REG (_spi_port)) = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN;
        *reg(SPI_USER1_REG(_spi_port)) = 0;
        *reg(SPI_PIN_REG  (_spi_port)) = 0;
        *reg(SPI_SLAVE_REG(_spi_port)) = 0; //&= ~(SPI_SLAVE_MODE | SPI_TRANS_DONE);

  //Reset DMA
        *reg(SPI_DMA_CONF_REG(_spi_port)) |= SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST;
        *reg(SPI_DMA_OUT_LINK_REG(_spi_port)) = 0;
        *reg(SPI_DMA_IN_LINK_REG(_spi_port)) = 0;
        *reg(SPI_DMA_CONF_REG(_spi_port)) &= ~(SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST);
//*/
      }

//*/
      if (_spi_sclk >= 0) { pinMatrixOutAttach(_spi_sclk, (_spi_host == HSPI_HOST) ? HSPICLK_OUT_IDX : VSPICLK_OUT_IDX, false, false); }
      if (_spi_mosi >= 0) { pinMatrixOutAttach(_spi_mosi, (_spi_host == HSPI_HOST) ? HSPID_IN_IDX : VSPID_IN_IDX, false, false); }
      if (_spi_miso >= 0) { pinMatrixInAttach( _spi_miso, (_spi_host == HSPI_HOST) ? HSPIQ_OUT_IDX : VSPIQ_OUT_IDX, false); }

      TPin<_panel_rst>::hi();
    }

    void initPanel(void)
    {
      const uint8_t *cmds;
      for (uint8_t i = 0; (cmds = _panel->getInitCommands(i)); i++) {
        delay(120);
        if (!_start_write_count) beginTransaction_impl();
        commandList(cmds);
        if (!_start_write_count) endTransaction_impl();
      }

      startWrite();
      invertDisplay(getInvert());
      setColorDepth(getColorDepth());
      setRotation(getRotation());
      clear();
      endWrite();
    }

    uint32_t readPanelID(void)
    {
      startWrite();
      //write_cmd(0xD9);
      //write_data(0x10, 8);
      write_cmd(_panel->cmd_rddid);
      start_read();
      if (_panel->len_dummy_read_rddid) read_data(_panel->len_dummy_read_rddid);
      uint32_t res = read_data(32);
      end_read();
      endWrite();
      return res;
    }
/*
    uint32_t readPanelIDSub(uint8_t cmd)
    {
      startWrite();
      //write_cmd(0xD9);
      //write_data(0x10, 8);
      write_cmd(cmd);
      start_read();
      uint32_t res = read_data(32);
      end_read();
      endWrite();
      return res;
    }
//*/
//----------------------------------------------------------------------------
  protected:
    template<class T> static PanelCommon* createPanel(const T&) { return new T; }
    template<class T> static PanelCommon* createPanelFromConfig(decltype(T::panel)*) { return createPanel(T::panel); }
    template<class T> static PanelCommon* createPanelFromConfig(...) { return nullptr; }

    static volatile uint32_t* _get_gpio_hi_reg(uint8_t pin) { return (pin == -1) ? nullptr : (pin & 32) ? &GPIO.out1_w1ts.val : &GPIO.out_w1ts; }
    static volatile uint32_t* _get_gpio_lo_reg(uint8_t pin) { return (pin == -1) ? nullptr : (pin & 32) ? &GPIO.out1_w1tc.val : &GPIO.out_w1tc; }

    void postSetPanel(void)
    {
      _cmd_ramrd      = _panel->cmd_ramrd;
      _cmd_ramwr      = _panel->cmd_ramwr;
      _invert         = _panel->invert   ;
      _rotation       = _panel->rotation ;
      _len_setwindow  = _panel->len_setwindow;
      fpGetWindowAddr = _len_setwindow == 32 ? PanelCommon::getWindowAddr32 : PanelCommon::getWindowAddr16;
      _last_apb_freq = -1;

      _gpio_reg_cs_h = _get_gpio_hi_reg(_panel->spi_cs);
      _gpio_reg_cs_l = _get_gpio_lo_reg(_panel->spi_cs);
      _gpio_reg_dc_h = _get_gpio_hi_reg(_panel->spi_dc);
      _gpio_reg_dc_l = _get_gpio_lo_reg(_panel->spi_dc);
      _mask_reg_cs = 1 << (_panel->spi_cs & 31);
      _mask_reg_dc = 1 << (_panel->spi_dc & 31);
      gpioInit((gpio_num_t)_panel->spi_cs);
      gpioInit((gpio_num_t)_panel->spi_dc);
      cs_h();

      postSetRotation();
      postSetColorDepth();
    }

    void postSetRotation(void)
    {
      _width     = _panel->width    ;
      _height    = _panel->height   ;
      _rotation  = _panel->rotation ;
      _colstart  = _panel->colstart ;
      _rowstart  = _panel->rowstart ;
      _cmd_caset = _panel->cmd_caset;
      _cmd_raset = _panel->cmd_raset;
      _last_xs = _last_xe = _last_ys = _last_ye = ~0;
    }

    void postSetColorDepth(void)
    {
      _color.setColorDepth(_panel->write_depth);
      _readColorDepth  = _panel->read_depth;
      _len_read_pixel  = _readColorDepth;
      _last_xs = _last_xe = _last_ys = _last_ye = ~0;
    }

    void invertDisplay_impl(bool i) override
    {
      startWrite();
      _invert = i;
      write_cmd(i ? _panel->cmd_invon : _panel->cmd_invoff);
      endWrite();
    }

    void setRotation_impl(uint8_t r) override
    {
      if (!_start_write_count) beginTransaction_impl();

      commandList(_panel->getRotationCommands((uint8_t*)_regbuf, r));
      postSetRotation();

      if (!_start_write_count) endTransaction_impl();
    }

    void* setColorDepth_impl(color_depth_t depth) override
    {
      if (!_start_write_count) beginTransaction_impl();

      commandList(_panel->getColorDepthCommands((uint8_t*)_regbuf, depth));
      postSetColorDepth();

      if (!_start_write_count) endTransaction_impl();
    }

    void beginTransaction_impl(void) override {
//memcpy(_spi_reg_backup, (void*)REG_SPI_BASE(_spi_port), 16*4);
      uint32_t apb_freq = getApbFrequency();
      if (_last_apb_freq != apb_freq) {
        _last_apb_freq = apb_freq;
        _clkdiv_read  = spiFrequencyToClockDiv(_panel->freq_read);
        _clkdiv_fill  = spiFrequencyToClockDiv(_panel->freq_fill);
        _clkdiv_write = spiFrequencyToClockDiv(_panel->freq_write);
      }
      _fill_mode = false;
      wait_spi();
      cs_l();
      *_spi_miso_dlen_reg = 0;
      set_clock_write();
      _user_reg = *_spi_user_reg & ~(SPI_USR_MISO | SPI_DOUTDIN);
      {
        if (_panel->spi_mode == 1 || _panel->spi_mode == 2) {
          _user_reg |= SPI_CK_OUT_EDGE;
        } else {
          _user_reg &= ~SPI_CK_OUT_EDGE;
        }
        if (_panel->spi_mode & 2) { // 2 or 3
          *reg(SPI_PIN_REG(_spi_port)) |= SPI_CK_IDLE_EDGE;
        } else {
          *reg(SPI_PIN_REG(_spi_port)) &= ~SPI_CK_IDLE_EDGE;
        }
      }
      *_spi_user_reg = _user_reg;
      // MSB first
      *reg(SPI_CTRL_REG(_spi_port)) &= ~(SPI_WR_BIT_ORDER | SPI_RD_BIT_ORDER);
      if (_dma_ch && _dma_chan_claimed) {
        DPORT_SET_PERI_REG_BITS(DPORT_SPI_DMA_CHAN_SEL_REG, 3, _dma_ch, (_spi_host * 2));
      }
    }

    void endTransaction_impl(void) override {
      wait_spi();
      cs_h();
      if (_dma_ch && _dma_chan_claimed) {
        spicommon_dmaworkaround_idle(_dma_ch);
      }
//memcpy((void*)REG_SPI_BASE(_spi_port), _spi_reg_backup, 16*4);
      *_spi_user_reg = _user_reg | SPI_USR_MISO | SPI_DOUTDIN;
    }

    void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
      if (_clkdiv_write != _clkdiv_fill && _fill_mode) {
        wait_spi();
        set_clock_write();
        _fill_mode = false;
      }
      write_cmd(_cmd_ramwr);
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      if (!_start_write_count) beginTransaction_impl();
      set_window(x, y, x, y);
      if (_clkdiv_write != _clkdiv_fill && !_fill_mode) {
        wait_spi();
        set_clock_fill();
        _fill_mode = true;
      }
      write_cmd(_cmd_ramwr);
      write_data(_color.raw, _color.bits);
      if (!_start_write_count) endTransaction_impl();
    }

    void fillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
      if (!_start_write_count) beginTransaction_impl();
      set_window(x, y, x+w-1, y+h-1);
      if (_clkdiv_write != _clkdiv_fill && !_fill_mode) {
        wait_spi();
        set_clock_fill();
        _fill_mode = true;
      }
      write_cmd(_cmd_ramwr);
      if (1 == (w|h)) write_data(_color.raw, _color.bits);
      else            writeColor_impl(w*h);
      if (!_start_write_count) endTransaction_impl();
    }

    void readWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
      write_cmd(_cmd_ramrd);
      start_read();
      if (_len_dummy_read_pixel) {;
        set_len(_len_dummy_read_pixel);
        exec_spi();
      }
    }

    void writeColor_impl(int32_t length) override
    {
      if (length == 1) { write_data(_color.raw, _color.bits); return; }

      uint32_t len;
      if (_color.bytes == 2) {
        _regbuf[0] = _color.raw | _color.raw << 16;
        memcpy(&_regbuf[1], _regbuf, 4);
        memcpy(&_regbuf[2], _regbuf, 4);
        len = 6;
      } else { // bytes == 3
        _regbuf[0] = _color.raw;
        memcpy(&((uint8_t*)_regbuf)[3], _regbuf, 3);
        memcpy(&((uint8_t*)_regbuf)[6], _regbuf, 6);
        len = 4;
      }

      if (length < len) len = length;
      wait_spi();
      set_len(len * _color.bits);
      dc_h();
      memcpy((void*)_spi_w0_reg, _regbuf, 12);
      exec_spi();
      if (0 == (length -= len)) return;

      memcpy((void*)&_regbuf[ 3], _regbuf, 12);
      memcpy((void*)&_regbuf[ 6], _regbuf,  4);
      memcpy((void*)&_spi_w0_reg[ 3], _regbuf, 24);
      memcpy((void*)&_spi_w0_reg[ 9], _regbuf, 28);

      const uint32_t limit = (_color.bytes == 2) ? 32 : 21; // limit = 512 / bpp;
      len = length % limit;
      if (len) {
        wait_spi();
        set_len(len * _color.bits);
        exec_spi();
        if (0 == (length -= len)) return;
      }
      wait_spi();
      set_len(limit * _color.bits);
      exec_spi();
      while (length -= limit) {
        wait_spi();
        exec_spi();
      }
    }

    rgb565_t readPixel16_impl(int32_t x, int32_t y) override
    {
      startWrite();
      readWindow_impl(x, y, x, y);
      //if (_len_read_pixel == 24) 
      rgb565_t res = (rgb565_t)swap888_t(read_data(_len_read_pixel));
      end_read();
      endWrite();
      return res;
    }

    bool commandList(const uint8_t *addr)
    {
      if (addr == nullptr) return false;
      uint8_t  cmd;
      uint8_t  numArgs;
      uint8_t  ms;

      _fill_mode = false;
      wait_spi();
      set_clock_write();
      for (;;) {                // For each command...
        cmd     = pgm_read_byte(addr++);  // Read, issue command
        numArgs = pgm_read_byte(addr++);  // Number of args to follow
        if (0xFF == (cmd & numArgs)) break;
        write_cmd(cmd);
        ms = numArgs & CMD_INIT_DELAY;       // If hibit set, delay follows args
        numArgs &= ~CMD_INIT_DELAY;          // Mask out delay bit

        while (numArgs--) {                   // For each argument...
          write_data(pgm_read_byte(addr++), 8);  // Read, issue argument
        }
        if (ms) {
          ms = pgm_read_byte(addr++);        // Read post-command delay time (ms)
          delay( (ms==255 ? 500 : ms) );
        }
      }
      return true;
    }

    void write_cmd(uint32_t cmd)
    {
      wait_spi();
      *_spi_w0_reg = cmd;
      set_len(8);
      dc_l();
      exec_spi();
    }

    void write_data(uint32_t data, uint32_t length)
    {
      wait_spi();
      *_spi_w0_reg = data;
      set_len(length);
      dc_h();
      exec_spi();
    }

    void set_window(uint32_t xs, uint32_t ys, uint32_t xe, uint32_t ye)
    {
      if (_last_xs != xs || _last_xe != xe) {
        write_cmd(_cmd_caset);
        _last_xs = xs;
        _last_xe = xe;
        write_data(fpGetWindowAddr(xs += _colstart, xe += _colstart), _len_setwindow);
      }
      if (_last_ys != ys || _last_ye != ye) {
        write_cmd(_cmd_raset);
        _last_ys = ys;
        _last_ye = ye;
        write_data(fpGetWindowAddr(ys += _rowstart, ye += _rowstart), _len_setwindow);
      }
    }

    void start_read(void) {
      *_spi_user_reg = _user_reg | SPI_USR_MISO | SPI_DOUTDIN;
      _fill_mode = false;
      wait_spi();
      set_clock_read();
      dc_h();
      if (_panel->spi_half_duplex) {
        pinMatrixOutDetach(_spi_mosi, false, false);
        pinMatrixInAttach(_spi_mosi, (_spi_host == HSPI_HOST) ? HSPIQ_OUT_IDX : VSPIQ_OUT_IDX, false);
        TPin<_spi_mosi>::disableOutput();
      } else if (_spi_miso != -1) {
        pinMatrixInAttach(_spi_miso, (_spi_host == HSPI_HOST) ? HSPIQ_OUT_IDX : VSPIQ_OUT_IDX, false);
        TPin<_spi_miso>::disableOutput();
      }
    }

    void endRead_impl(void) override
    {
      end_read();
    }
    void end_read(void) {
      cs_h();
      *_spi_user_reg = _user_reg;
      set_clock_write();
      _fill_mode = false;
      if (_panel->spi_half_duplex) {
        if (_spi_miso != -1) {
          pinMatrixInAttach(_spi_miso, (_spi_host == HSPI_HOST) ? HSPIQ_OUT_IDX : VSPIQ_OUT_IDX, false);
        }
        TPin<_spi_mosi>::enableOutput();
        pinMatrixOutAttach(_spi_mosi, (_spi_host == HSPI_HOST) ? HSPID_IN_IDX : VSPID_IN_IDX, false, false);
      }
      wait_spi();
      cs_l();
    }

    uint32_t read_data(uint32_t length)
    {
      set_len(length);
      exec_spi();
      wait_spi();
      return *_spi_w0_reg;
    }

    void write_pixels(const void* src, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) override
    {
      if (!length) return;
      const uint8_t bytes = _color.bytes;
      const uint32_t limit = (bytes == 2) ? 16 : 10; //  limit = 32/bytes (bytes==2 is 16   bytes==3 is 10)
      uint32_t len = (length - 1) / limit;
      uint8_t highpart = (len & 1) << 3;
      len = length - (len * limit);
      void* dst = _regbuf;
      fp_copy(dst, src, len, param);
      wait_spi();
      dc_h();
      set_len(len * bytes << 3);
      memcpy((void*)&_spi_w0_reg[highpart], _regbuf, (len * bytes + 3) & 0xFC);
      if (highpart) *_spi_user_reg = _user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      for (; length; length -= limit) {
        dst = _regbuf;
        fp_copy(dst, src, limit, param);
        memcpy((void*)&_spi_w0_reg[highpart ^= 0x08], _regbuf, limit * bytes);
        uint32_t user = _user_reg;
        if (highpart) user |= SPI_USR_MOSI_HIGHPART;
        if (len != limit) {
          wait_spi();
          set_len(limit * bytes << 3);
          *_spi_user_reg = user;
          exec_spi();
          len = limit;
        } else {
          wait_spi();
          *_spi_user_reg = user;
          exec_spi();
        }
      }
    }

    void write_bytes(const uint8_t* data, int32_t length) override
    {
      if (!length) return;
      constexpr uint32_t limit = 32;
      uint32_t len = ((length - 1) & 0x1F) + 1;
      uint8_t highpart = ((length - 1) & limit) >> 2; // 8 or 0

      wait_spi();
      dc_h();
      set_len(len << 3);

      memcpy((void*)&_spi_w0_reg[highpart], data, (len + 3) & 0xFC);
      if (highpart) *_spi_user_reg = _user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      if (len != limit) {
        memcpy((void*)&_spi_w0_reg[highpart ^= 0x08], data += len, limit);
        uint32_t user = _user_reg;
        if (highpart) user |= SPI_USR_MOSI_HIGHPART;
        wait_spi();
        *_spi_user_reg = user;
        set_len(limit << 3);
        exec_spi();
        length -= limit;
      }
      for (; length; length -= limit) {
        memcpy((void*)&_spi_w0_reg[highpart ^= 0x08], data += limit, limit);
        uint32_t user = _user_reg;
        if (highpart) user |= SPI_USR_MOSI_HIGHPART;
        wait_spi();
        *_spi_user_reg = user;
        exec_spi();
      }
    }

    void read_pixels(void* dst, int32_t length, pixelcopy_param_t* param, void(*fp_copy)(void*&, const void*&, int32_t, pixelcopy_param_t*)) override
    {
      if (!length) return;
      const void* src;
      uint32_t len(length & 7);
      length >>= 3;
      wait_spi();
      if (length) {
        set_len(_len_read_pixel << 3); // 8pixel read
        exec_spi();
        while (--length) {
          wait_spi();
          memcpy(_regbuf, (void*)_spi_w0_reg, _len_read_pixel);
          exec_spi();
          src = _regbuf;
          fp_copy(dst, src, 8, param);
        }
        wait_spi();
        memcpy(_regbuf, (void*)_spi_w0_reg, _len_read_pixel);
        if (len) {
          set_len(_len_read_pixel * len);
          exec_spi();
        }
        src = _regbuf;
        fp_copy(dst, src, 8, param);
        if (!len) return;
      } else {
        set_len(_len_read_pixel * len);
        exec_spi();
      }
      wait_spi();
      memcpy(_regbuf, (void*)_spi_w0_reg, 3 * len);
      src = _regbuf;
      fp_copy(dst, src, len, param);
    }

    void read_bytes(uint8_t* dst, int32_t length) override
    {
      if (!length) return;
      uint32_t len(length & 0x3F);
      length >>= 6;
      wait_spi();
      if (length) {
        set_len(64 << 3); // 64byte read
        exec_spi();
        while (--length) {
          wait_spi();
          memcpy(dst, (void*)_spi_w0_reg, 64);
          exec_spi();
          dst += 64;
        }
        wait_spi();
        memcpy(dst, (void*)_spi_w0_reg, 64);
        if (len) {
          set_len(len << 3);
          exec_spi();
        }
        dst += 64;
        if (!len) return;
      } else {
        set_len(len << 3);
        exec_spi();
      }
      wait_spi();
      memcpy(dst, (void*)_spi_w0_reg, len);
    }

    static void IRAM_ATTR _setup_dma_desc_links(lldesc_t *dmadesc, int len, const uint8_t *data)
    {          //spicommon_setup_dma_desc_links
      while (len > SPI_MAX_DMA_LEN) {
        dmadesc->buf = (uint8_t *)data;
        data += SPI_MAX_DMA_LEN;
        *(uint32_t*)dmadesc = SPI_MAX_DMA_LEN | SPI_MAX_DMA_LEN<<12 | 0x80000000;
        dmadesc->qe.stqe_next = dmadesc + 1;
        dmadesc++;
        len -= SPI_MAX_DMA_LEN;
      }
      *(uint32_t*)dmadesc = len | len<<12 | 0xC0000000;
      dmadesc->buf = (uint8_t *)data;
      dmadesc->qe.stqe_next = nullptr;
    }

    void pushDMA_impl(const uint8_t* data, int32_t length) override
    {
      if (!_dma_ch || !_dma_chan_claimed) { write_bytes(data, length); return; }
      wait_spi();
      dc_h();
      set_len(length << 3);
      //spicommon_dmaworkaround_idle(_dma_ch);
      _setup_dma_desc_links(dmadesc_tx, length, data);
      //spicommon_setup_dma_desc_links(dmadesc_tx, length, data, false);
      *reg(SPI_DMA_OUT_LINK_REG(_spi_port)) = SPI_OUTLINK_START | ((int)(&dmadesc_tx[0]) & 0xFFFFF);
      spicommon_dmaworkaround_transfer_active(_dma_ch);
      exec_spi();
    }


    __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }
    __attribute__ ((always_inline)) inline void set_clock_write(void) { *_spi_clock_reg = _clkdiv_write; }
    __attribute__ ((always_inline)) inline void set_clock_read(void)  { *_spi_clock_reg = _clkdiv_read;  }
    __attribute__ ((always_inline)) inline void set_clock_fill(void)  { *_spi_clock_reg = _clkdiv_fill;  }
    __attribute__ ((always_inline)) inline void exec_spi(void) { *_spi_cmd_reg = SPI_USR; }
    __attribute__ ((always_inline)) inline void wait_spi(void) { while (*_spi_cmd_reg & SPI_USR); }
    __attribute__ ((always_inline)) inline void set_len(uint32_t len) { *_spi_mosi_dlen_reg = len - 1; }

    __attribute__ ((always_inline)) inline void cs_h(void) { if (_gpio_reg_cs_h) *_gpio_reg_cs_h = _mask_reg_cs; else cs_h_impl(); }
    __attribute__ ((always_inline)) inline void cs_l(void) { if (_gpio_reg_cs_l) *_gpio_reg_cs_l = _mask_reg_cs; else cs_l_impl(); }
    __attribute__ ((always_inline)) inline void dc_h(void) { if (_gpio_reg_dc_h) *_gpio_reg_dc_h = _mask_reg_dc; else dc_h_impl(); }
    __attribute__ ((always_inline)) inline void dc_l(void) { if (_gpio_reg_dc_l) *_gpio_reg_dc_l = _mask_reg_dc; else dc_l_impl(); }

    virtual void cs_h_impl(void) {}
    virtual void cs_l_impl(void) {}
    virtual void dc_h_impl(void) {}
    virtual void dc_l_impl(void) {}

    static constexpr int _dma_ch   = get_dma_ch  <CFG,  0>::value;
    static constexpr int _spi_mosi = get_spi_mosi<CFG, 23>::value;
    static constexpr int _spi_miso = get_spi_miso<CFG, 19>::value;
    static constexpr int _spi_sclk = get_spi_sclk<CFG, 18>::value;
    static constexpr int _panel_rst= get_panel_rst<CFG, -1>::value;
    static constexpr spi_host_device_t _spi_host = get_spi_host<CFG, VSPI_HOST>::value;

    static constexpr uint8_t _spi_port = (_spi_host == HSPI_HOST) ? 2 : 3;  // FSPI=1  HSPI=2  VSPI=3;
    static constexpr volatile uint32_t *_spi_w0_reg        = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_W0_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_cmd_reg       = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_CMD_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_user_reg      = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_USER_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_clock_reg     = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_CLOCK_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_mosi_dlen_reg = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_MOSI_DLEN_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_miso_dlen_reg = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_MISO_DLEN_REG(_spi_port));
    static constexpr volatile spi_dev_t *_hw           = (volatile spi_dev_t *)REG_SPI_BASE(_spi_port);

    volatile uint32_t* _gpio_reg_cs_h;
    volatile uint32_t* _gpio_reg_cs_l;
    volatile uint32_t* _gpio_reg_dc_h;
    volatile uint32_t* _gpio_reg_dc_l;
    uint32_t _mask_reg_cs;
    uint32_t _mask_reg_dc;

    PanelCommon* _panel;
    uint32_t(*fpGetWindowAddr)(uint16_t, uint16_t);
    int32_t _colstart;
    int32_t _rowstart;
    uint32_t _cmd_caset;
    uint32_t _cmd_raset;
    uint32_t _cmd_ramrd;
    uint32_t _cmd_ramwr;
    uint32_t _last_xs;
    uint32_t _last_xe;
    uint32_t _last_ys;
    uint32_t _last_ye;
    uint32_t _len_setwindow;
    uint32_t _len_read_pixel;
    uint32_t _len_dummy_read_pixel;
    uint32_t _last_apb_freq;
    uint32_t _clkdiv_write;
    uint32_t _clkdiv_read;
    uint32_t _clkdiv_fill;
    bool _fill_mode;
    static uint32_t _user_reg;
    static uint32_t _regbuf[8];
    static lldesc_t* dmadesc_tx;
//    static volatile spi_dev_t *_hw;
    static bool _dma_chan_claimed;

    static uint32_t _spi_reg_backup[16];
  };
  template <class T> uint32_t LGFX_SPI<T>::_user_reg;
  template <class T> uint32_t LGFX_SPI<T>::_regbuf[];
  template <class T> lldesc_t* LGFX_SPI<T>::dmadesc_tx;
//  template <class T> volatile spi_dev_t *LGFX_SPI<T>::_hw;
  template <class T> bool LGFX_SPI<T>::_dma_chan_claimed;

  template <class T> uint32_t LGFX_SPI<T>::_spi_reg_backup[];

//----------------------------------------------------------------------------

}
#endif
