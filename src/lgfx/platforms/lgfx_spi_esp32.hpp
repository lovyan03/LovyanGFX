#ifndef LGFX_SPI_ESP32_HPP_
#define LGFX_SPI_ESP32_HPP_

#include <type_traits>
#include <esp_heap_caps.h>
#include <freertos/task.h>
#include <driver/spi_common.h>

#if defined (ARDUINO) // Arduino ESP32
 #include <SPI.h>
#endif

#include "../lgfx_base.hpp"

namespace lgfx
{
  static portMUX_TYPE periph_spinlock = portMUX_INITIALIZER_UNLOCKED;
  static void spi_dma_reset(void) //periph_module_reset( PERIPH_SPI_DMA_MODULE );
  {
    vTaskEnterCritical(&periph_spinlock);
    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI_DMA_CLK_EN);
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI_DMA_CLK_EN);
    vTaskExitCritical(&periph_spinlock);
  }

  #define MEMBER_DETECTOR(member, classname, classname_impl, valuetype) struct classname_impl { \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, T::member> check(decltype(T::member)*); \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, V> check(...); \
  };template<class T, valuetype V> class classname : public decltype(classname_impl::check<T, V>(nullptr)) {};
  MEMBER_DETECTOR(spi_host   , get_spi_host   , get_spi_host_impl   , spi_host_device_t)
  MEMBER_DETECTOR(spi_mosi   , get_spi_mosi   , get_spi_mosi_impl   , int)
  MEMBER_DETECTOR(spi_miso   , get_spi_miso   , get_spi_miso_impl   , int)
  MEMBER_DETECTOR(spi_sclk   , get_spi_sclk   , get_spi_sclk_impl   , int)
  MEMBER_DETECTOR(dma_channel, get_dma_channel, get_dma_channel_impl, int)
  #undef MEMBER_DETECTOR

  template <class CFG>
  class LGFX_SPI : public LovyanGFX
  {
  public:

    virtual ~LGFX_SPI() {
      if (_panel) delete _panel;
      _panel = nullptr;

      if ((0 != _dma_channel) && _dmadesc) {
        heap_caps_free(_dmadesc);
        _dmadesc = nullptr;
      }
      delete_dmabuffer();
    }

    LGFX_SPI() : LovyanGFX()
    {
      if (nullptr != (_panel = createPanelFromConfig<CFG>(nullptr))) { postSetPanel(); }
    }

    template <class T> inline void setPanel(const T& panel) { setPanel<T>(); }
    template <class T> inline void setPanel(void) { if (_panel) { delete _panel; } _panel = new T; postSetPanel(); }

    __attribute__ ((always_inline)) inline void begin(void) { init(); }
    void init(void) { initBus(); initPanel(); }

    void initBus(void)
    {
#if defined (ARDUINO) // Arduino ESP32
//*
      if (_spi_host == HSPI_HOST) {
        SPIClass spi = SPIClass(HSPI);
        spi.begin(_spi_sclk, _spi_miso, _spi_mosi, -1);
      } else {
        SPI.begin(_spi_sclk, _spi_miso, _spi_mosi, -1);
      }
/*/
      bool use_gpio_matrix = (
         (_spi_sclk>=0 &&
          _spi_sclk != spi_periph_signal[_spi_host].spiclk_iomux_pin)
       || (_spi_mosi >= 0 &&
           _spi_mosi != spi_periph_signal[_spi_host].spid_iomux_pin)
       || (_spi_miso>=0 &&
           _spi_miso != spi_periph_signal[_spi_host].spiq_iomux_pin));

      if (use_gpio_matrix) {
        ESP_LOGI("LGFX", "bus init: use gpio_matrix.");
        if (_spi_mosi >= 0) {
          TPin<_spi_mosi>::init(GPIO_MODE_INPUT_OUTPUT);
          gpio_matrix_out(_spi_mosi, spi_periph_signal[_spi_host].spid_out, false, false);
          gpio_matrix_in(_spi_mosi, spi_periph_signal[_spi_host].spid_in, false);
        }
        if (_spi_miso >= 0) {
          TPin<_spi_miso>::init(GPIO_MODE_INPUT_OUTPUT);
        //gpio_matrix_out(_spi_miso, spi_periph_signal[_spi_host].spiq_out, false, false);
          gpio_matrix_in(_spi_miso, spi_periph_signal[_spi_host].spiq_in, false);
        }
        if (_spi_sclk >= 0) {
          TPin<_spi_sclk>::init(GPIO_MODE_INPUT_OUTPUT);
          //gpio_set_direction((gpio_num_t)_spi_sclk, GPIO_MODE_INPUT_OUTPUT);
          gpio_matrix_out(_spi_sclk, spi_periph_signal[_spi_host].spiclk_out, false, false);
          gpio_matrix_in(_spi_sclk, spi_periph_signal[_spi_host].spiclk_in, false);
        }
      } else {
        ESP_LOGI("LGFX", "bus init: use iomux.");
        if (_spi_mosi >= 0) {
          gpio_iomux_in(_spi_mosi, spi_periph_signal[_spi_host].spid_in);
          gpio_iomux_out(_spi_mosi, 1, false);
        }
        if (_spi_miso >= 0) {
          gpio_iomux_in(_spi_miso, spi_periph_signal[_spi_host].spiq_in);
          gpio_iomux_out(_spi_miso, 1, false);
        }
        if (_spi_sclk >= 0) {
          gpio_iomux_in(_spi_sclk, spi_periph_signal[_spi_host].spiclk_in);
          gpio_iomux_out(_spi_sclk, 1, false);
        }
      }
//*/
      periph_module_enable(spi_periph_signal[_spi_host].module);
      if (_dma_channel) {
        periph_module_enable( PERIPH_SPI_DMA_MODULE );
    //Select DMA channel.
        DPORT_SET_PERI_REG_BITS(DPORT_SPI_DMA_CHAN_SEL_REG, 3, _dma_channel, (_spi_host * 2));
      //Reset DMA
        *reg(SPI_DMA_CONF_REG(_spi_port)) |= SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST;
        *reg(SPI_DMA_OUT_LINK_REG(_spi_port)) = 0;
        *reg(SPI_DMA_IN_LINK_REG(_spi_port)) = 0;
        *reg(SPI_DMA_CONF_REG(_spi_port)) &= ~(SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST);
      }

      *reg(SPI_USER_REG (_spi_port)) = SPI_USR_MISO | SPI_USR_MOSI | SPI_DOUTDIN;  // need SD card access (full duplex setting)
      *reg(SPI_CTRL2_REG(_spi_port)) = 0;
      *reg(SPI_SLAVE_REG(_spi_port)) &= ~(SPI_SLAVE_MODE | SPI_TRANS_DONE);
/*
      *reg(SPI_CTRL_REG (_spi_port)) = 0;
      *reg(SPI_USER1_REG(_spi_port)) = 0;
      *reg(SPI_USER2_REG(_spi_port)) = 0;
      *reg(SPI_PIN_REG  (_spi_port)) = 0;
//*/
#elif defined (CONFIG_IDF_TARGET_ESP32) // ESP-IDF

      spi_bus_config_t buscfg = {
          .mosi_io_num = _spi_mosi,
          .miso_io_num = _spi_miso,
          .sclk_io_num = _spi_sclk,
          .quadwp_io_num = -1,
          .quadhd_io_num = -1,
          .max_transfer_sz = 1,
          .flags = SPICOMMON_BUSFLAG_MASTER,
          .intr_flags = 0,
      };

      if (ESP_OK != spi_bus_initialize(_spi_host, &buscfg, _dma_channel)) {
        ESP_LOGE("LGFX", "Failed to spi_bus_initialize. ");
      }

      if (_spi_handle == nullptr) {
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
        if (ESP_OK != spi_bus_add_device(_spi_host, &devcfg, &_spi_handle)) {
          ESP_LOGE("LGFX", "Failed to spi_bus_add_device. ");
        }
      }

#endif
      *reg(SPI_CTRL1_REG(_spi_port)) = 0;
    }

    void initPanel(void)
    {
      if (!_panel) return;
      auto gpio_rst = _panel->gpio_rst;
      if (gpio_rst >= 0) {
        initGPIO((gpio_num_t)gpio_rst);
        auto tmp = get_gpio_lo_reg(gpio_rst);
        auto mask = (1 << (gpio_rst & 31));
        *tmp = mask;
        delay(1);
        tmp = get_gpio_hi_reg(gpio_rst);
        *tmp = mask;
      }
      if ( (0 != _dma_channel) && _dmadesc == nullptr ) {
        int dma_desc_ct=(320 * 240 * 2 + SPI_MAX_DMA_LEN-1)/SPI_MAX_DMA_LEN;
        _dmadesc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t)*dma_desc_ct, MALLOC_CAP_DMA);
      }

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

    __attribute__ ((always_inline)) inline void setColorDepth(uint8_t bpp) { setColorDepth((color_depth_t)bpp); }
    void setColorDepth(color_depth_t depth)
    {
      commandList(_panel->getColorDepthCommands((uint8_t*)_regbuf, depth));
      postSetColorDepth();
    }

    void setRotation(uint8_t r)
    {
      commandList(_panel->getRotationCommands((uint8_t*)_regbuf, r));
      postSetRotation();
    }

    void invertDisplay(bool i)
    {
      _invert = i;
      commandList(_panel->getInvertDisplayCommands((uint8_t*)_regbuf, i));
    }

    void display() {
      wait_spi();
    }

    void setBrightness(uint8_t brightness) {
      _panel->setBrightness(brightness);
    }

    void writecommand(uint_fast8_t cmd)
    {
      startWrite();
      write_cmd(cmd);
      endWrite();
    }

    void writedata(uint32_t data, uint32_t len = 8)
    {
      startWrite();
      write_data(data, len);
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
//*
    uint32_t readPanelIDSub(uint8_t cmd)
    {
      startWrite();
      //write_cmd(0xD9);
      //write_data(0x10, 8);
      write_cmd(cmd);
      start_read();
      if (_panel->len_dummy_read_rddid) read_data(_panel->len_dummy_read_rddid);
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

    static volatile uint32_t* get_gpio_hi_reg(int16_t pin) { return (pin & 32) ? &GPIO.out1_w1ts.val : &GPIO.out_w1ts; }
    static volatile uint32_t* get_gpio_lo_reg(int16_t pin) { return (pin & 32) ? &GPIO.out1_w1tc.val : &GPIO.out_w1tc; }

    void postSetPanel(void)
    {
      _last_apb_freq = -1;
      _cmd_ramwr      = _panel->cmd_ramwr;
      _invert         = _panel->invert   ;
      _rotation       = _panel->rotation ;
      _len_setwindow  = _panel->len_setwindow;
      fpGetWindowAddr = _len_setwindow == 32 ? PanelCommon::getWindowAddr32 : PanelCommon::getWindowAddr16;

      int32_t spi_dc = _panel->spi_dc;
      _gpio_reg_dc_h = get_gpio_hi_reg(spi_dc);
      _gpio_reg_dc_l = get_gpio_lo_reg(spi_dc);
      _mask_reg_dc = (spi_dc < 0) ? 0 : (1 << (spi_dc & 31));
      dc_h();
      initGPIO((gpio_num_t)spi_dc);

      cs_h();
      initGPIO(_panel->spi_cs);

      postSetRotation();
      postSetColorDepth();
    }

    void postSetRotation(void)
    {
      bool fullscroll = (_sx == 0 && _sy == 0 && _sw == _width && _sh == _height);

      _rotation  = _panel->rotation ;
      _colstart  = _panel->colstart ;
      _rowstart  = _panel->rowstart ;
      _cmd_caset = _panel->cmd_caset;
      _cmd_raset = _panel->cmd_raset;
      _width     = _panel->width    ;
      _height    = _panel->height   ;
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
      uint32_t apb_freq = getApbFrequency();
      if (_last_apb_freq != apb_freq) {
        _last_apb_freq = apb_freq;
        _clkdiv_read  = FreqToClockDiv(apb_freq, _panel->freq_read);
        _clkdiv_fill  = FreqToClockDiv(apb_freq, _panel->freq_fill);
        _clkdiv_write = FreqToClockDiv(apb_freq, _panel->freq_write);
      }
      _fill_mode = false;

      _user_reg = (*_spi_user_reg & ~(SPI_USR_MISO | SPI_DOUTDIN | SPI_SIO | SPI_CK_OUT_EDGE | SPI_USR_COMMAND))
                | ((_panel->spi_mode == 1 || _panel->spi_mode == 2) ? SPI_CK_OUT_EDGE : 0)
                | SPI_USR_MOSI;

      _pin_reg = (*reg(SPI_PIN_REG(_spi_port)) & ~(SPI_CK_IDLE_EDGE | SPI_CS0_DIS | SPI_CS1_DIS | SPI_CS2_DIS))
               | ((_panel->spi_mode & 2) ? SPI_CK_IDLE_EDGE : 0);

      wait_spi();

      *_spi_user_reg = _user_reg;
      *_spi_pin_reg = _pin_reg;

#if defined (ARDUINO) // Arduino ESP32
      if (_dma_channel) {
        _next_dma_reset = true;
//      *reg(SPI_DMA_CONF_REG(_spi_port)) |= SPI_OUT_DATA_BURST_EN | SPI_INDSCR_BURST_EN | SPI_OUTDSCR_BURST_EN;
      }
/*
*reg(SPI_CTRL1_REG(_spi_port)) = 0;
*reg(SPI_USER1_REG(_spi_port)) = 0;
*reg(SPI_USER2_REG(_spi_port)) = 0;
*reg(SPI_PIN_REG  (_spi_port)) = 0;
//*/
#elif defined (CONFIG_IDF_TARGET_ESP32) // ESP-IDF
      if (_spi_handle) {
        if (ESP_OK != spi_device_acquire_bus(_spi_handle, portMAX_DELAY)) {
          ESP_LOGE("LGFX", "Failed to spi_device_acquire_bus. ");
        }
      }
#endif
      *reg(SPI_CTRL_REG(_spi_port)) &= ~(SPI_WR_BIT_ORDER | SPI_RD_BIT_ORDER);

      set_clock_write();

      cs_l();

      // MSB first
    }

    void endTransaction_impl(void) override {
      if (!_begun_tr) return;
      _begun_tr = false;
      end_transaction();
    }

    void end_transaction(void) {
      if (_panel->spi_cs < 0) {
        write_cmd(_panel->cmd_nop);
      }
      dc_h();
      cs_h();
      delete_dmabuffer();
#if defined (ARDUINO) // Arduino ESP32
      if (_dma_channel) {
        if (_next_dma_reset) spi_dma_reset();
      }

      *_spi_user_reg = _user_reg
                     | ((_spi_miso == -1) ? 0 : (SPI_USR_MISO | SPI_DOUTDIN)); // for other SPI device (SD)
#elif defined (CONFIG_IDF_TARGET_ESP32) // ESP-IDF
      if (_spi_handle) {
        spi_device_release_bus(_spi_handle);
      }
#endif
    }

    void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
      if (_clkdiv_write != _clkdiv_fill && _fill_mode) {
        _fill_mode = false;
        wait_spi();
        set_clock_write();
      }
      write_cmd(_cmd_ramwr);
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      if (!_begun_tr) begin_transaction();
      set_window(x, y, x, y);
      if (_clkdiv_write != _clkdiv_fill && !_fill_mode) {
        _fill_mode = true;
        wait_spi();
        set_clock_fill();
      }
      write_cmd(_cmd_ramwr);
      write_data(_color.raw, _write_conv.bits);
      if (!_begun_tr) end_transaction();
    }

    void fillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
      set_window(x, y, x+w-1, y+h-1);
      if (_clkdiv_write != _clkdiv_fill && !_fill_mode) {
        _fill_mode = true;
        wait_spi();
        set_clock_fill();
      }
      write_cmd(_cmd_ramwr);
      write_color(w*h);
    }

    void writeColor_impl(int32_t length) override
    {
      write_color(length);
    }

    void write_color(int32_t length)
    {
      if (length == 1) { write_data(_color.raw, _write_conv.bits); return; }

      // make 12Bytes data.
      auto bytes = _write_conv.bytes;
      if (bytes == 2) {
        _regbuf[0] = _color.raw | _color.raw << 16;
        memcpy(&_regbuf[1], _regbuf, 4);
        memcpy(&_regbuf[2], _regbuf, 4);
      } else { // bytes == 3
        _regbuf[0] = _color.raw;
        memcpy(&((uint8_t*)_regbuf)[3], _regbuf, 3);
        memcpy(&((uint8_t*)_regbuf)[6], _regbuf, 6);
      }

      length *= _write_conv.bits;          // convert to bitlength.
      uint32_t len = std::min(96, length); // 1st send length = max (96bit) 12Byte. 
      dc_h();
      set_write_len(len);

      // copy to SPI buffer register
      memcpy((void*)_spi_w0_reg, _regbuf, 12);

      exec_spi();   // 1st send.
      if (0 == (length -= len)) return;

      // make 28Byte data from 12Byte data.
      memcpy((void*)&_regbuf[ 3], _regbuf, 12);
      memcpy((void*)&_regbuf[ 6], _regbuf, 4);
      // copy to SPI buffer register
      memcpy((void*)&_spi_w0_reg[ 3], _regbuf, 24);
      memcpy((void*)&_spi_w0_reg[ 9], _regbuf, 28);

      // limit = 64Byte / depth_bytes;
      // When 3Byte color, 504 bits out of 512bit buffer are used.
      // When 2Byte color, it uses exactly 512 bytes. but, it behaves like a ring buffer, can specify a larger size.
      const uint32_t limit = (bytes == 3) ? 504 : (1 << 11);

      len = (bytes == 3)           // 2nd send length = Surplus of buffer size.
          ? (length % limit)
          : length & (limit - 1);
      if (len) {
        wait_spi();
        set_write_len(len);
        exec_spi();                // 2nd send.
        if (0 == (length -= len)) return;
      }
      wait_spi();
      set_write_len(limit);
      exec_spi();
      while (length -= limit) {
        taskYIELD();
        wait_spi();
        exec_spi();
      }
//*/
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
          write_data(pgm_read_byte(addr++), 8);  // Read, issue argument
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
      auto spi_w0_reg = _spi_w0_reg;
      auto spi_mosi_dlen_reg = _spi_mosi_dlen_reg;
      dc_l();
      *spi_mosi_dlen_reg = 7;
      *spi_w0_reg = cmd;
      exec_spi();
    }

    void write_data(uint32_t data, uint32_t bit_length)
    {
      auto spi_w0_reg = _spi_w0_reg;
      auto spi_mosi_dlen_reg = _spi_mosi_dlen_reg;
      dc_h();
      *spi_mosi_dlen_reg = bit_length - 1;
      *spi_w0_reg = data;
      exec_spi();
    }

    void set_window(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
    {
      if (_xs != xs || _xe != xe) {
        write_cmd(_cmd_caset);
        _xs = xs;
        _xe = xe;
        write_data(fpGetWindowAddr(xs += _colstart, xe += _colstart), _len_setwindow);
      }
      if (_ys != ys || _ye != ye) {
        write_cmd(_cmd_raset);
        _ys = ys;
        _ye = ye;
        write_data(fpGetWindowAddr(ys += _rowstart, ye += _rowstart), _len_setwindow);
      }
    }

    void start_read(void) {
      _fill_mode = false;
      uint32_t user = (_user_reg & ~(SPI_USR_MOSI | SPI_CK_OUT_EDGE))
                    | ((_panel->spi_mode_read == 1 || _panel->spi_mode_read == 2) ? SPI_CK_OUT_EDGE : 0)
                    | (_panel->spi_3wire ? SPI_SIO : 0)
                    | SPI_USR_MISO;
      uint32_t pin = (_pin_reg & ~SPI_CK_IDLE_EDGE)
                   | ((_panel->spi_mode_read & 2) ? SPI_CK_IDLE_EDGE : 0);
      dc_h();
      *_spi_user_reg = user;
      *_spi_pin_reg = pin;
      set_clock_read();
    }

    void end_read(void)
    {
      wait_spi();
      cs_h();
      *_spi_user_reg = _user_reg;
      *_spi_pin_reg = _pin_reg;
      if (_panel->spi_cs < 0) {
        write_cmd(_panel->cmd_nop);
      }
      set_clock_write();
      _fill_mode = false;

      cs_l();
    }

    uint32_t read_data(uint32_t length)
    {
      set_read_len(length);
      exec_spi();
      wait_spi();
      return *_spi_w0_reg;
    }

    void pushImage_impl(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param, bool use_dma) override
    {
      auto bytes = _write_conv.bytes;
      auto src_x = param->src_x;
      auto fp_copy = param->fp_copy;

      if (param->transp == ~0) {
        int32_t xr = ((h == 1) ? _width : (x + w)) - 1;
        if (param->no_convert) {
          setWindow_impl(x, y, xr, y + h - 1);
          uint32_t i = (src_x + param->src_y * param->src_width) * bytes;
          auto src = &((const uint8_t*)param->src_data)[i];
          if (param->src_width == w) {
            int32_t len = w * h * bytes;
            if (!use_dma && (64 < len) && (len <= 1024)) {
              auto buf = get_dmabuffer(len);
              memcpy(buf, src, len);
              write_bytes(buf, len, true);
            } else {
              write_bytes(src, len, false);
            }
          } else {
            auto add = param->src_width * bytes;
            do {
              write_bytes(src, w * bytes, use_dma);
              src += add;
            } while (--h);
          }
        } else
        if (_dma_channel && use_dma) {
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
        }
      } else {
        int32_t xr = _width - 1;
        auto fp_skip = param->fp_skip;
        h += y;
        do {
          int32_t i = 0;
          while (w != (i = fp_skip(i, w, param))) {
            auto buf = get_dmabuffer(w * bytes);
            int32_t len = fp_copy(buf, 0, w - i, param);
            setWindow_impl(x + i, y, xr, y);
            write_bytes(buf, len * bytes, use_dma);
            if (w == (i += len)) break;
          }
          param->src_x = src_x;
          param->src_y++;
        } while (++y != h);
      }
    }

    void pushColors_impl(int32_t length, pixelcopy_t* param) override
    {
      push_colors(length, param);
    }

    void push_colors(int32_t length, pixelcopy_t* param)
    {
      const uint8_t bytes = _write_conv.bytes;
      const uint32_t limit = (bytes == 2) ? 16 : 10; //  limit = 32/bytes (bytes==2 is 16   bytes==3 is 10)
      uint32_t len = (length - 1) / limit;
      uint32_t highpart = (len & 1) << 3;
      len = length - (len * limit);
      param->fp_copy(_regbuf, 0, len, param);

      dc_h();
      set_write_len(len * bytes << 3);

      memcpy((void*)&_spi_w0_reg[highpart], _regbuf, (len * bytes + 3) & (~3));
      if (highpart) *_spi_user_reg = _user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      for (; length; length -= limit) {
        param->fp_copy(_regbuf, 0, limit, param);
        memcpy((void*)&_spi_w0_reg[highpart ^= 0x08], _regbuf, limit * bytes);
        uint32_t user = _user_reg;
        if (highpart) user |= SPI_USR_MOSI_HIGHPART;
        if (len != limit) {
          len = limit;
          wait_spi();
          set_write_len(limit * bytes << 3);
          *_spi_user_reg = user;
          exec_spi();
        } else {
          wait_spi();
          *_spi_user_reg = user;
          exec_spi();
        }
      }
    }

    void write_bytes(const uint8_t* data, int32_t length, bool use_dma = false)
    {
      if (length <= 64) {
        dc_h();
        set_write_len(length << 3);
        memcpy((void*)_spi_w0_reg, data, (length + 3) & (~3));
        exec_spi();
        return;
      } else if (_dma_channel && use_dma) {
        dc_h();
        set_write_len(length << 3);
        _setup_dma_desc_links(_dmadesc, length, data, _clkdiv_write == SPI_CLK_EQU_SYSCLK);
        *reg(SPI_DMA_OUT_LINK_REG(_spi_port)) = SPI_OUTLINK_START | ((int)(&_dmadesc[0]) & 0xFFFFF);
        spicommon_dmaworkaround_transfer_active(_dma_channel);
        exec_spi();
        return;
      }
      constexpr uint32_t limit = 32;
      uint32_t len = ((length - 1) & 0x1F) + 1;
      uint32_t highpart = ((length - 1) & limit) >> 2; // 8 or 0

      dc_h();
      set_write_len(len << 3);

      memcpy((void*)&_spi_w0_reg[highpart], data, (len + 3) & (~3));
      if (highpart) *_spi_user_reg = _user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      for (; length; length -= limit) {
        data += len;
        memcpy((void*)&_spi_w0_reg[highpart ^= 0x08], data, limit);
        uint32_t user = _user_reg;
        if (highpart) user |= SPI_USR_MOSI_HIGHPART;
        if (len != limit) {
          len = limit;
          wait_spi();
          set_write_len(limit << 3);
          *_spi_user_reg = user;
          exec_spi();
        } else {
          wait_spi();
          *_spi_user_reg = user;
          exec_spi();
        }
      }
    }

    void readRect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) override
    {
      set_window(x, y, x + w - 1, y + h - 1);
      write_cmd(_panel->cmd_ramrd);
      auto len_dummy_read_pixel = _panel->len_dummy_read_pixel;
      start_read();
      if (len_dummy_read_pixel) {;
        set_read_len(len_dummy_read_pixel);
        exec_spi();
      }

      if (param->no_convert) {
        read_bytes((uint8_t*)dst, w * h * _read_conv.bytes);
      } else {
        read_pixels(dst, w * h, param);
      }
      end_read();
    }

    void read_pixels(void* dst, int32_t length, pixelcopy_t* param)
    {
      int32_t len1 = std::min(length, 10); // 10 pixel read
      int32_t len2 = len1;
      auto len_read_pixel  = _read_conv.bits;
      wait_spi();
      set_read_len(len_read_pixel * len1);
      exec_spi();
      param->src_data = _regbuf;
      int32_t dstindex = 0;
      uint32_t highpart = 8;
      uint32_t userreg = *_spi_user_reg;
      do {
        if (0 == (length -= len1)) {
          len2 = len1;
          wait_spi();
          *_spi_user_reg = userreg;
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
          *_spi_user_reg = user;
          exec_spi();
        }
        memcpy(_regbuf, (void*)&_spi_w0_reg[highpart ^= 8], len2 * len_read_pixel >> 3);
        param->src_x = 0;
        dstindex = param->fp_copy(dst, dstindex, dstindex + len2, param);
      } while (length);
    }

    void read_bytes(uint8_t* dst, int32_t length)
    {
      if (_dma_channel) {
        wait_spi();
        set_read_len(length << 3);
        _setup_dma_desc_links(_dmadesc, length, dst);
        *reg(SPI_DMA_IN_LINK_REG(_spi_port)) = SPI_INLINK_START | ((int)(&_dmadesc[0]) & 0xFFFFF);
        spicommon_dmaworkaround_transfer_active(_dma_channel);
        exec_spi();
      } else {
        int32_t len1 = std::min(length, 32);  // 32 Byte read.
        int32_t len2 = len1;
        wait_spi();
        set_read_len(len1 << 3);
        exec_spi();
        uint32_t highpart = 8;
        uint32_t userreg = *_spi_user_reg;
        do {
          if (0 == (length -= len1)) {
            len2 = len1;
            wait_spi();
            *_spi_user_reg = userreg;
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
            *_spi_user_reg = user;
            exec_spi();
          }
          memcpy(dst, (void*)&_spi_w0_reg[highpart ^= 8], len2);
          dst += len2;
        } while (length);
      }
//*/
    }

    void copyRect_impl(int32_t dst_x, int32_t dst_y, int32_t w, int32_t h, int32_t src_x, int32_t src_y) override
    {
      pixelcopy_t p(nullptr, _write_conv.depth, _read_conv.depth);
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
    }

    struct _dmabufs_t {
      uint8_t* buffer = nullptr;
      uint32_t length = 0;
      void free(void) {
        if (buffer) {
          heap_caps_free(buffer);
          buffer = nullptr;
          length = 0;
        }
      }
    };

    uint8_t* get_dmabuffer(uint32_t length)
    {
      _dma_flip = !_dma_flip;
      length = (length + 3) & ~3;
      if (_dmabufs[_dma_flip].length < length) {
        _dmabufs[_dma_flip].free();
        _dmabufs[_dma_flip].buffer = (uint8_t*)heap_caps_malloc(length, MALLOC_CAP_DMA);
        _dmabufs[_dma_flip].length = _dmabufs[_dma_flip].buffer ? length : 0;
      }
      return _dmabufs[_dma_flip].buffer;
    }

    void delete_dmabuffer(void)
    {
      _dmabufs[0].free();
      _dmabufs[1].free();
    }

    static void _setup_dma_desc_links(lldesc_t *dmadesc, int len, const uint8_t *data, bool align = true)
    {          //spicommon_setup_dma_desc_links
      if (_next_dma_reset) spi_dma_reset();
      // length is 4Byte align, and need reset with next dma.
      _next_dma_reset = align && (len & 3);
      if (_next_dma_reset) len = ( len + 3 ) & ( ~3 );
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

    __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }
    __attribute__ ((always_inline)) inline void set_clock_write(void) { *_spi_clock_reg = _clkdiv_write; }
    __attribute__ ((always_inline)) inline void set_clock_read(void)  { *_spi_clock_reg = _clkdiv_read;  }
    __attribute__ ((always_inline)) inline void set_clock_fill(void)  { *_spi_clock_reg = _clkdiv_fill;  }
    __attribute__ ((always_inline)) inline void exec_spi(void) {        *_spi_cmd_reg = SPI_USR; }
    __attribute__ ((always_inline)) inline void wait_spi(void) { while (*_spi_cmd_reg & SPI_USR); }
    __attribute__ ((always_inline)) inline void set_write_len(uint32_t len) { *_spi_mosi_dlen_reg = len - 1; }
    __attribute__ ((always_inline)) inline void set_read_len( uint32_t len) { *_spi_miso_dlen_reg = len - 1; }

    void cs_h(void) {
      int32_t spi_cs = _panel->spi_cs;
      if (spi_cs >= 0) *get_gpio_hi_reg(spi_cs) = (1 << (spi_cs & 31));
    }
    void cs_l(void) {
      int32_t spi_cs = _panel->spi_cs;
      if (spi_cs >= 0) *get_gpio_lo_reg(spi_cs) = (1 << (spi_cs & 31));
    }
/*
    __attribute__ ((always_inline)) inline void cs_h(void) { *_gpio_reg_cs_h = _mask_reg_cs; }
    __attribute__ ((always_inline)) inline void cs_l(void) { *_gpio_reg_cs_l = _mask_reg_cs; }
//
    __attribute__ ((always_inline)) inline void cs_h(void) { if (_mask_reg_cs) *_gpio_reg_cs_h = _mask_reg_cs; else cs_h_impl(); }
    __attribute__ ((always_inline)) inline void cs_l(void) { if (_mask_reg_cs) *_gpio_reg_cs_l = _mask_reg_cs; else cs_l_impl(); }
    virtual void cs_h_impl(void) {}
    virtual void cs_l_impl(void) {}
//*/

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

    static constexpr spi_host_device_t _spi_host = get_spi_host<CFG, VSPI_HOST>::value;
    static constexpr int _dma_channel= get_dma_channel<CFG,  0>::value;
    static constexpr int _spi_mosi = get_spi_mosi<CFG, -1>::value;
    static constexpr int _spi_miso = get_spi_miso<CFG, -1>::value;
    static constexpr int _spi_sclk = get_spi_sclk<CFG, -1>::value;

    static constexpr uint8_t _spi_port = (_spi_host == HSPI_HOST) ? 2 : 3;  // FSPI=1  HSPI=2  VSPI=3;
    static constexpr volatile uint32_t *_spi_w0_reg        = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_W0_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_cmd_reg       = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_CMD_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_pin_reg       = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_PIN_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_user_reg      = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_USER_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_clock_reg     = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_CLOCK_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_mosi_dlen_reg = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_MOSI_DLEN_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_miso_dlen_reg = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_MISO_DLEN_REG(_spi_port));

    volatile uint32_t* _gpio_reg_dc_h;
    volatile uint32_t* _gpio_reg_dc_l;
    uint32_t _mask_reg_dc;

    PanelCommon* _panel;
    uint32_t(*fpGetWindowAddr)(uint_fast16_t, uint_fast16_t);
    bool _begun_tr = false;
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
    bool _dma_flip = false;
    bool _fill_mode;
    static uint32_t _user_reg;
    static uint32_t _pin_reg;
    static uint32_t _regbuf[8];
    static lldesc_t* _dmadesc;
    static bool _next_dma_reset;

//    static bool _dma_chan_claimed;
//    static volatile spi_dev_t *_hw;

#if defined (CONFIG_IDF_TARGET_ESP32) // ESP-IDF
    static spi_device_handle_t _spi_handle;
#endif
  };
  template <class T> uint32_t LGFX_SPI<T>::_user_reg;
  template <class T> uint32_t LGFX_SPI<T>::_pin_reg;
  template <class T> uint32_t LGFX_SPI<T>::_regbuf[];
  template <class T> lldesc_t* LGFX_SPI<T>::_dmadesc;
  template <class T> bool LGFX_SPI<T>::_next_dma_reset;
//  template <class T> bool LGFX_SPI<T>::_dma_chan_claimed;
//  template <class T> volatile spi_dev_t *LGFX_SPI<T>::_hw;

#if defined (CONFIG_IDF_TARGET_ESP32) // ESP-IDF
  template <class T> spi_device_handle_t LGFX_SPI<T>::_spi_handle;
#endif

//----------------------------------------------------------------------------

}
#endif
