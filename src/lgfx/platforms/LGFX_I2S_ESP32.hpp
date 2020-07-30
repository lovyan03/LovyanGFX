/*----------------------------------------------------------------------------/
  Lovyan GFX library - ESP32 hardware SPI graphics library .  
  
    for Arduino and ESP-IDF  
  
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
#ifndef LGFX_I2S_ESP32_HPP_
#define LGFX_I2S_ESP32_HPP_

#include <cstring>
#include <type_traits>

#include <driver/periph_ctrl.h>
#include <driver/rtc_io.h>
#include <driver/i2s.h>
#include <esp_heap_caps.h>
#include <freertos/task.h>
#include <soc/dport_reg.h>
#include <soc/rtc.h>
#include <soc/i2s_reg.h>

#if defined (ARDUINO) // Arduino ESP32
 #include <SPI.h>
 #include <driver/periph_ctrl.h>
 #include <soc/periph_defs.h>
 #include <esp32-hal-cpu.h>
#else
 #include <esp_log.h>

  static std::uint32_t getApbFrequency()
  {
    rtc_cpu_freq_config_t conf;
    rtc_clk_cpu_freq_get_config(&conf);
    if (conf.freq_mhz >= 80){
      return 80 * 1000000;
    }
    return (conf.source_freq_mhz * 1000000) / conf.div;
  }

#endif

#include "esp32_common.hpp"
#include "../LGFXBase.hpp"

namespace lgfx
{
  #ifndef I2S_FIFO_WR_REG
  #define I2S_FIFO_WR_REG(i) (REG_I2S_BASE(i))
  #endif

/*
  inline static void spi_dma_transfer_active(int dmachan)
  {
    spicommon_dmaworkaround_transfer_active(dmachan);
  }

  static void spi_dma_reset(void)
  {
    periph_module_reset( PERIPH_SPI_DMA_MODULE );
  }
//*/
  #define MEMBER_DETECTOR(member, classname, classname_impl, valuetype) struct classname_impl { \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, T::member> check(decltype(T::member)*); \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, V> check(...); \
  };template<class T, valuetype V> class classname : public decltype(classname_impl::check<T, V>(nullptr)) {};
  MEMBER_DETECTOR(i2s_port   , get_i2s_port   , get_i2s_port_impl   , i2s_port_t)
  MEMBER_DETECTOR(i2s_d0     , get_i2s_d0     , get_i2s_d0_impl     , int)
  MEMBER_DETECTOR(i2s_d1     , get_i2s_d1     , get_i2s_d1_impl     , int)
  MEMBER_DETECTOR(i2s_d2     , get_i2s_d2     , get_i2s_d2_impl     , int)
  MEMBER_DETECTOR(i2s_d3     , get_i2s_d3     , get_i2s_d3_impl     , int)
  MEMBER_DETECTOR(i2s_d4     , get_i2s_d4     , get_i2s_d4_impl     , int)
  MEMBER_DETECTOR(i2s_d5     , get_i2s_d5     , get_i2s_d5_impl     , int)
  MEMBER_DETECTOR(i2s_d6     , get_i2s_d6     , get_i2s_d6_impl     , int)
  MEMBER_DETECTOR(i2s_d7     , get_i2s_d7     , get_i2s_d7_impl     , int)
  MEMBER_DETECTOR(i2s_d8     , get_i2s_d8     , get_i2s_d8_impl     , int)
  MEMBER_DETECTOR(i2s_d9     , get_i2s_d9     , get_i2s_d9_impl     , int)
  MEMBER_DETECTOR(i2s_d10    , get_i2s_d10    , get_i2s_d10_impl    , int)
  MEMBER_DETECTOR(i2s_d11    , get_i2s_d11    , get_i2s_d11_impl    , int)
  MEMBER_DETECTOR(i2s_d12    , get_i2s_d12    , get_i2s_d12_impl    , int)
  MEMBER_DETECTOR(i2s_d13    , get_i2s_d13    , get_i2s_d13_impl    , int)
  MEMBER_DETECTOR(i2s_d14    , get_i2s_d14    , get_i2s_d14_impl    , int)
  MEMBER_DETECTOR(i2s_d15    , get_i2s_d15    , get_i2s_d15_impl    , int)

  MEMBER_DETECTOR(i2s_wr     , get_i2s_wr     , get_i2s_wr_impl     , int)
  MEMBER_DETECTOR(i2s_rd     , get_i2s_rd     , get_i2s_rd_impl     , int)
  MEMBER_DETECTOR(i2s_rs     , get_i2s_rs     , get_i2s_rs_impl     , int)
  MEMBER_DETECTOR(i2s_dlen   , get_i2s_dlen   , get_i2s_dlen_impl   , int)
  #undef MEMBER_DETECTOR

  template <class CFG>
  class LGFX_I2S : public LovyanGFX
  {
  public:

    virtual ~LGFX_I2S() {
      if (_dmadesc) {
        heap_caps_free(_dmadesc);
        _dmadesc = nullptr;
        _dmadesc_len = 0;
      }
      delete_dmabuffer();
    }

    LGFX_I2S() : LovyanGFX()
    {
      _panel = nullptr;
    }

    void setPanel(PanelCommon* panel)
    {
      _panel = panel; 
      _cmd_ramwr      = _panel->getCmdRamwr();
      _len_setwindow  = _panel->len_setwindow;
      fpGetWindowAddr = _len_setwindow == 32 ? PanelCommon::getWindowAddr32 : PanelCommon::getWindowAddr16;
/*
      std::int32_t spi_dc = _panel->spi_dc;
      _mask_reg_dc = (spi_dc < 0) ? 0 : (1 << (spi_dc & 31));
      _gpio_reg_dc_h = get_gpio_hi_reg(spi_dc);
      _gpio_reg_dc_l = get_gpio_lo_reg(spi_dc);
      dc_h();
      lgfxPinMode(spi_dc, pin_mode_t::output);
//*/
      cs_h();
      lgfxPinMode(_panel->spi_cs, pin_mode_t::output);

      postSetRotation();
      postSetColorDepth();
    }

    __attribute__ ((always_inline)) inline PanelCommon* getPanel(void) const { return _panel; }

    __attribute__ ((always_inline)) inline bool getInvert(void) const { return _panel->invert; }

    __attribute__ ((always_inline)) inline void dmaWait(void) const { wait_i2s(); }

    __attribute__ ((always_inline)) inline void begin(void) { init(); }

    void init(void) { initBus(); initPanel(); }

    // Write single byte as COMMAND
    void writeCommand(std::uint_fast8_t cmd) { startWrite(); write_cmd(cmd); endWrite(); } // AdafruitGFX compatible
    void writecommand(std::uint_fast8_t cmd) { startWrite(); write_cmd(cmd); endWrite(); } // TFT_eSPI compatible

    // Write single bytes as DATA
    void spiWrite( std::uint_fast8_t data) { startWrite(); write_data(data, 8); endWrite(); } // AdafruitGFX compatible
    void writeData(std::uint_fast8_t data) { startWrite(); write_data(data, 8); endWrite(); } // TFT_eSPI compatible
    void writedata(std::uint_fast8_t data) { startWrite(); write_data(data, 8); endWrite(); } // TFT_eSPI compatible

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
      commandList(_panel->getColorDepthCommands((std::uint8_t*)_regbuf, depth));
      postSetColorDepth();
    }

    void setRotation(std::int_fast8_t r)
    {
      commandList(_panel->getRotationCommands((std::uint8_t*)_regbuf, r));
      postSetRotation();
    }

    void invertDisplay(bool i)
    {
      commandList(_panel->getInvertDisplayCommands((std::uint8_t*)_regbuf, i));
    }

    void setBrightness(std::uint8_t brightness) {
      _panel->setBrightness(brightness);
    }

    std::uint32_t readPanelID(void)
    {
      return read_command(_panel->getCmdRddid(), _panel->len_dummy_read_rddid, 32);
    }

    void initBus(void)
    {
      gpio_pad_select_gpio(_i2s_d0);
      gpio_pad_select_gpio(_i2s_d1);
      gpio_pad_select_gpio(_i2s_d2);
      gpio_pad_select_gpio(_i2s_d3);
      gpio_pad_select_gpio(_i2s_d4);
      gpio_pad_select_gpio(_i2s_d5);
      gpio_pad_select_gpio(_i2s_d6);
      gpio_pad_select_gpio(_i2s_d7);

      gpio_pad_select_gpio(_i2s_rd);
      gpio_pad_select_gpio(_i2s_wr);
      gpio_pad_select_gpio(_i2s_rs);

      gpio_set_level(_i2s_rd, 1);
      gpio_set_level(_i2s_wr, 1);
      gpio_set_level(_i2s_rs, 1);

      gpio_set_direction(_i2s_rd, GPIO_MODE_OUTPUT);
      gpio_set_direction(_i2s_wr, GPIO_MODE_OUTPUT);
      gpio_set_direction(_i2s_rs, GPIO_MODE_OUTPUT);

      /* Route I2S peripheral outputs -> GPIO_MUXs */
      auto idx_base = (_i2s_port == I2S_NUM_0) ? I2S0O_DATA_OUT8_IDX : I2S1O_DATA_OUT8_IDX;

      gpio_matrix_out(_i2s_d7, idx_base + 7, 0, 0); // MSB
      gpio_matrix_out(_i2s_d6, idx_base + 6, 0, 0);
      gpio_matrix_out(_i2s_d5, idx_base + 5, 0, 0);
      gpio_matrix_out(_i2s_d4, idx_base + 4, 0, 0);
      gpio_matrix_out(_i2s_d3, idx_base + 3, 0, 0);
      gpio_matrix_out(_i2s_d2, idx_base + 2, 0, 0);
      gpio_matrix_out(_i2s_d1, idx_base + 1, 0, 0);
      gpio_matrix_out(_i2s_d0, idx_base    , 0, 0); // LSB
      gpio_matrix_out(_i2s_rs, idx_base + 8, 0, 0); // RS (Command/Data select: 0=CMD, 1=DATA)

      if (_i2s_port == I2S_NUM_0) {
        gpio_matrix_out(_i2s_wr, I2S0O_WS_OUT_IDX    ,1,0); // WR (Write-strobe in 8080 mode, Active-low)
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG,DPORT_I2S0_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG,DPORT_I2S0_RST);
      } else {
        gpio_matrix_out(_i2s_wr, I2S1O_WS_OUT_IDX    ,1,0); // WR (Write-strobe in 8080 mode, Active-low)
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_I2S1_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_I2S1_RST);
      }

      //Reset I2S subsystem
      *reg(I2S_CONF_REG(_i2s_port)) = I2S_TX_RESET | I2S_RX_RESET | I2S_TX_FIFO_RESET | I2S_RX_FIFO_RESET;
      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_default;

      //Reset DMA
      *reg(I2S_LC_CONF_REG(_i2s_port)) = I2S_IN_RST | I2S_OUT_RST | I2S_AHBM_RST | I2S_AHBM_FIFO_RST;
      *reg(I2S_LC_CONF_REG(_i2s_port)) = I2S_OUT_EOF_MODE;

      *reg(I2S_CONF2_REG(_i2s_port))
            = I2S_LCD_EN
  //          | I2S_LCD_TX_WRX2_EN
            ;

      *reg(I2S_CONF1_REG(_i2s_port))
            = I2S_TX_PCM_BYPASS
            | I2S_TX_STOP_EN
            ;

      *reg(I2S_CONF_CHAN_REG(_i2s_port))
            = 1 << I2S_TX_CHAN_MOD_S
            | 1 << I2S_RX_CHAN_MOD_S
            ;

      *reg(I2S_INT_ENA_REG(_i2s_port)) |= I2S_TX_REMPTY_INT_ENA;

      *reg(I2S_OUT_LINK_REG(_i2s_port)) = 0;
      *reg(I2S_IN_LINK_REG(_i2s_port)) = 0;
      *reg(I2S_TIMING_REG(_i2s_port)) = 0;

      _alloc_dmadesc(1);
      memset(_dmadesc, 0, sizeof(lldesc_t));
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
        wait_i2s();
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


    void pushPixelsDMA_impl(const void* data, std::int32_t length) override {
      write_bytes((const std::uint8_t*)data, length * _write_conv.bytes, true);
    }


//----------------------------------------------------------------------------
  protected:

    bool isReadable_impl(void) const override { return _panel->spi_read; }
    std::int_fast8_t getRotation_impl(void) const override { return _panel->rotation; }

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

      // I2S_CLKM_DIV_NUM 4=20MHz  /  5=16MHz  /  8=10MHz  /  10=8MHz
      // clock = 80MHz / I2S_CLKM_DIV_NUM

      _clkdiv_write = I2S_CLKA_ENA
                    | 63 << I2S_CLKM_DIV_A_S
                    |  0 << I2S_CLKM_DIV_B_S
                    |  4 << I2S_CLKM_DIV_NUM_S
                    ;
      _clkdiv_read = _clkdiv_write;

      set_clock_write();

      cs_l();
    }

    void endTransaction_impl(void) override {
      if (!_begun_tr) return;
      _begun_tr = false;
      end_transaction();
    }

    void end_transaction(void) {
      wait();
      if (_panel->spi_cs < 0) {
        write_cmd(0); // NOP command
      }
      //dc_h();
      cs_h();
    }


    void initDMA_impl(void) override
    {
    }

    void waitDMA_impl(void) override
    {
      wait_i2s();
    }

    bool dmaBusy_impl(void) override
    {
      return !(*reg(I2S_STATE_REG(_i2s_port)) & I2S_TX_IDLE);
    }

    void setWindow_impl(std::int32_t xs, std::int32_t ys, std::int32_t xe, std::int32_t ye) override
    {
      set_window(xs, ys, xe, ye, _cmd_ramwr);
    }

    void drawPixel_impl(std::int32_t x, std::int32_t y) override
    {
      if (_begun_tr) {
        set_window(x, y, x, y, _cmd_ramwr);
        push_block(1);
        return;
      }

      begin_transaction();
      set_window(x, y, x, y, _cmd_ramwr);
      push_block(1);
      end_transaction();
    }

    void writeFillRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) override
    {
      set_window(x, y, x+w-1, y+h-1, _cmd_ramwr);
      push_block(w*h);
    }

    void pushBlock_impl(std::int32_t length) override
    {
      push_block(length);
    }

    void push_block(std::int32_t length, bool fillclock = false)
    {
      if (_write_conv.bits == 16) {
        *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
        auto conf_start = _conf_reg_start;
        std::uint32_t data = 0x01000100;
        data |= _color.raw << 16 | _color.raw >> 8;
        std::int32_t limit = ((length - 1) & 31) + 1;
        do {
          length -= limit;

          wait();
          *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;

          while (limit--) {
            *reg(I2S_FIFO_WR_REG(_i2s_port)) = data;
          }
          *reg(I2S_CONF_REG(_i2s_port)) = conf_start;
          limit = 32;
        } while (length);
      } else {
        if (length & 1) {
          write_data(_color.raw, _write_conv.bits);
          if (!--length) return;
        }
        auto conf_start = _conf_reg_start;
        static constexpr std::uint32_t data_wr = 0x01000100;
        std::uint32_t data0 = _color.raw << 16 | _color.raw >>  8 | data_wr;
        std::uint32_t data1 = _color.raw                          | data_wr;
        std::uint32_t data2 = _color.raw <<  8 | _color.raw >> 16 | data_wr;
        *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
        length >>= 1;
        std::uint32_t limit = ((length - 1) % 10) + 1;
        do {
          length -= limit;

          wait();
          *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
          while (limit--) {
            *reg(I2S_FIFO_WR_REG(_i2s_port)) = data0;
            *reg(I2S_FIFO_WR_REG(_i2s_port)) = data1;
            *reg(I2S_FIFO_WR_REG(_i2s_port)) = data2;
          }
          *reg(I2S_CONF_REG(_i2s_port)) = conf_start;
          limit = 10;
        } while (length);
      }
    }

    bool commandList(const std::uint8_t *addr)
    {
      if (addr == nullptr) return false;
      std::uint8_t  cmd;
      std::uint8_t  numArgs;
      std::uint8_t  ms;

      _fill_mode = false;
      wait_i2s();
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
      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_32bit;
      wait();
      *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
      *reg(I2S_FIFO_WR_REG(_i2s_port)) = cmd << 16;
      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
//auto dummy = *reg(I2S_CONF_REG(_i2s_port));
    }

    void write_data(std::uint32_t data, std::uint32_t bit_length)
    {
      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_32bit;
      wait();
      *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
      *reg(I2S_FIFO_WR_REG(_i2s_port)) = (0x100 | (data & 0xFF)) << 16;
      while (bit_length -= 8) {
        data >>= 8;
        *reg(I2S_FIFO_WR_REG(_i2s_port)) = (0x100 | (data & 0xFF)) << 16;
      }
      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
//auto dummy = *reg(I2S_CONF_REG(_i2s_port));
    }

    void set_window(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye, std::uint32_t cmd)
    {
/*
      std::uint32_t len32 = _len_setwindow >> 5;
      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_32bit;
      std::uint32_t data_wr = 0x1000000;
      wait_i2s();

      if (_xs != xs || _xe != xe) {
        _xs = xs;
        _xe = xe;
        std::uint32_t tmp = _colstart;
        xs += tmp;
        xe += tmp;
        *reg(I2S_FIFO_WR_REG(_i2s_port)) = _cmd_caset << 16;
        if (!len32) {
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = xs<<16|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = xe<<16|data_wr;
        } else {
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = xs<< 8|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = xs<<16|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = xe<< 8|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = xe<<16|data_wr;
        }
      }
      if (_ys != ys || _ye != ye) {
        _ys = ys;
        _ye = ye;
        std::uint32_t tmp = _rowstart;
        ys += tmp;
        ye += tmp;
        *reg(I2S_FIFO_WR_REG(_i2s_port)) = _cmd_raset << 16;
        if (!len32) {
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = ys<<16|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = ye<<16|data_wr;
        } else {
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = ys<< 8|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = ys<<16|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = ye<< 8|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = ye<<16|data_wr;
        }
      }
      *reg(I2S_FIFO_WR_REG(_i2s_port)) = cmd << 16;

      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
/*/
      std::uint32_t len32 = _len_setwindow >> 5;
      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
      static constexpr std::uint32_t data_wr = 0x01000100;
      wait_i2s();
      *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;

      if (_xs != xs || _xe != xe) {
        _xs = xs;
        _xe = xe;
        std::uint32_t tmp = _colstart;
        xs += tmp;
        xe += tmp;
        *reg(I2S_FIFO_WR_REG(_i2s_port)) = _cmd_caset;
        if (!len32) {
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = xs|xe<<16|data_wr;
        } else {
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = xs|xs<<8|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = xe|xe<<8|data_wr;
        }
      }
      if (_ys != ys || _ye != ye) {
        _ys = ys;
        _ye = ye;
        std::uint32_t tmp = _rowstart;
        ys += tmp;
        ye += tmp;
        *reg(I2S_FIFO_WR_REG(_i2s_port)) = _cmd_raset;
        if (!len32) {
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = ys|ye<<16|data_wr;
        } else {
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = ys|ys<<8|data_wr;
          *reg(I2S_FIFO_WR_REG(_i2s_port)) = ye|ye<<8|data_wr;
        }
      }
      *reg(I2S_FIFO_WR_REG(_i2s_port)) = cmd;

      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
//*/
    }

    void start_read(void) {
/*
      _fill_mode = false;
      std::uint32_t user = ((_panel->spi_mode_read == 1 || _panel->spi_mode_read == 2) ? SPI_CK_OUT_EDGE | SPI_USR_MISO : SPI_USR_MISO)
                    | (_panel->spi_3wire ? SPI_SIO : 0);
      std::uint32_t pin = (_panel->spi_mode_read & 2) ? SPI_CK_IDLE_EDGE : 0;
      dc_h();
      *reg(SPI_USER_REG(_i2s_port)) = user;
      *reg(SPI_PIN_REG(_i2s_port)) = pin;
      set_clock_read();
//*/
    }

    void end_read(void)
    {
/*
      std::uint32_t user = (_panel->spi_mode == 1 || _panel->spi_mode == 2) ? SPI_CK_OUT_EDGE | SPI_USR_MOSI : SPI_USR_MOSI;
      std::uint32_t pin = (_panel->spi_mode & 2) ? SPI_CK_IDLE_EDGE : 0;
      wait_i2s();
      cs_h();
      *reg(SPI_USER_REG(_i2s_port)) = user;
      *reg(SPI_PIN_REG(_i2s_port)) = pin;
      if (_panel->spi_cs < 0) {
        write_cmd(0); // NOP command
      }
      set_clock_write();
      _fill_mode = false;

      cs_l();
//*/
    }

    std::uint32_t read_data(std::uint32_t length)
    {
      set_read_len(length);
      exec_spi();
      wait_i2s();
      return *reg(SPI_W0_REG(_i2s_port));

    }

    std::uint32_t read_command(std::uint_fast8_t command, std::uint32_t bitindex = 0, std::uint32_t bitlen = 8)
    {
      startWrite();
      write_cmd(command);
      start_read();
      if (bitindex) read_data(bitindex);
      std::uint32_t res = read_data(bitlen);
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
      std::int32_t whb = w * h * bytes;
      if (param->transp == ~0) {
        if (param->no_convert) {
          std::uint32_t i = (src_x + param->src_y * param->src_width) * bytes;
          auto src = &((const std::uint8_t*)param->src_data)[i];
          setWindow_impl(x, y, xr, y + h - 1);
          if (param->src_width == w) {
            write_bytes(src, whb, use_dma);
          } else {
            auto add = param->src_width * bytes;
            do {
              write_bytes(src, w * bytes, use_dma);
              src += add;
            } while (--h);
          }
        } else
        if (param->src_width == w && (whb <= 1024)) {
          auto buf = get_dmabuffer(whb);
          fp_copy(buf, 0, w * h, param);
          setWindow_impl(x, y, xr, y + h - 1);
          write_bytes(buf, whb, true);
        } else {
          std::int32_t wb = w * bytes;
          auto buf = get_dmabuffer(wb);
          fp_copy(buf, 0, w, param);
          setWindow_impl(x, y, xr, y + h - 1);
          write_bytes(buf, wb, true);
          while (--h) {
            param->src_x = src_x;
            param->src_y++;
            buf = get_dmabuffer(wb);
            fp_copy(buf, 0, w, param);
            write_bytes(buf, wb, true);
          }
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
    }

    void pushColors_impl(std::int32_t length, pixelcopy_t* param) override
    {
      push_colors(length, param);
    }

    void push_colors(std::int32_t length, pixelcopy_t* param)
    {
/*
      const std::uint8_t bytes = _write_conv.bytes;
      const std::uint32_t limit = (bytes == 2) ? 16 : 10; //  limit = 32/bytes (bytes==2 is 16   bytes==3 is 10)
      std::uint32_t len = (length - 1) / limit;
      std::uint32_t highpart = (len & 1) << 3;
      len = length - (len * limit);
      param->fp_copy(_regbuf, 0, len, param);

      auto spi_w0_reg = reg(SPI_W0_REG(_i2s_port));

      std::uint32_t user_reg = *reg(SPI_USER_REG(_i2s_port));

      dc_h();
      set_write_len(len * bytes << 3);

      memcpy((void*)&spi_w0_reg[highpart], _regbuf, (len * bytes + 3) & (~3));
      if (highpart) *reg(SPI_USER_REG(_i2s_port)) = user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      for (; length; length -= limit) {
        param->fp_copy(_regbuf, 0, limit, param);
        memcpy((void*)&spi_w0_reg[highpart ^= 0x08], _regbuf, limit * bytes);
        std::uint32_t user = user_reg;
        if (highpart) user |= SPI_USR_MOSI_HIGHPART;
        if (len != limit) {
          len = limit;
          wait_i2s();
          set_write_len(limit * bytes << 3);
          *reg(SPI_USER_REG(_i2s_port)) = user;
          exec_spi();
        } else {
          wait_i2s();
          *reg(SPI_USER_REG(_i2s_port)) = user;
          exec_spi();
        }
      }
//*/
    }

    void write_bytes(const std::uint8_t* data, std::int32_t length, bool use_dma = false)
    {
      auto conf_start = _conf_reg_start;
      static constexpr std::uint32_t data_wr = 0x01000100;

      if (length & 1) {
        write_data(data[0], 8);
        if (!--length) return;
        ++data;
      }

      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
      *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;

      std::int32_t limit = (((length>>1)-1)&(31))+1;
      length -= limit << 1;
      wait();
      do {
        *reg(I2S_FIFO_WR_REG(_i2s_port)) = data[0] << 16 | data[1] | data_wr;
        data += 2;
      } while (--limit);
      *reg(I2S_CONF_REG(_i2s_port)) = conf_start;

      if (!length) return;

      std::uint32_t lbase = 64;
      do {
        auto buf = (std::uint32_t*)get_dmabuffer(512);
        std::int32_t limit = ((length - 1) & (lbase - 1)) + 1;
        length -= limit;
        _dmadesc->buf = (std::uint8_t*)buf;
        std::int32_t i = 0;
        limit>>=1;
        do {
          buf[i] = data[0]<<16 | data[1] | data_wr;
          data += 2;
        } while (++i != limit);
        *(std::uint32_t*)_dmadesc = (((i<<2) + 3) &  ~3 ) | i << 14 | 0xC0000000;
        wait();
        *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_dma;
        *reg(I2S_OUT_LINK_REG(_i2s_port)) = I2S_OUTLINK_START | ((uint32_t)_dmadesc & I2S_OUTLINK_ADDR);
        *reg(I2S_CONF_REG(_i2s_port)) &= ~(I2S_TX_RESET | I2S_RX_RESET | I2S_TX_START);
        *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_default;
        *reg(I2S_INT_CLR_REG(_i2s_port)) = ~0;
        *reg(I2S_CONF_REG(_i2s_port)) |= conf_start;
        if (lbase != 256) lbase <<= 1;
      } while (length);
    }

    void readRect_impl(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param) override
    {
/*
      set_window(x, y, x + w - 1, y + h - 1, _panel->getCmdRamrd());
      auto len = w * h;
      if (!_panel->spi_read) {
        memset(dst, 0, len * _read_conv.bytes);
        return;
      }
      std::uint32_t len_dummy_read_pixel = _panel->len_dummy_read_pixel;
      start_read();
      if (len_dummy_read_pixel) {;
        set_read_len(len_dummy_read_pixel);
        exec_spi();
      }

      if (param->no_convert) {
        read_bytes((std::uint8_t*)dst, len * _read_conv.bytes);
      } else {
        read_pixels(dst, len, param);
      }
      end_read();
//*/
    }

    void read_pixels(void* dst, std::int32_t length, pixelcopy_t* param)
    {
/*
      std::int32_t len1 = std::min(length, 10); // 10 pixel read
      std::int32_t len2 = len1;
      auto len_read_pixel  = _read_conv.bits;
      wait_i2s();
      set_read_len(len_read_pixel * len1);
      exec_spi();
      param->src_data = _regbuf;
      std::int32_t dstindex = 0;
      std::uint32_t highpart = 8;
      std::uint32_t userreg = *reg(SPI_USER_REG(_i2s_port));
      auto spi_w0_reg = reg(SPI_W0_REG(_i2s_port));
      do {
        if (0 == (length -= len1)) {
          len2 = len1;
          wait_i2s();
          *reg(SPI_USER_REG(_i2s_port)) = userreg;
        } else {
          std::uint32_t user = userreg;
          if (highpart) user = userreg | SPI_USR_MISO_HIGHPART;
          if (length < len1) {
            len1 = length;
            wait_i2s();
            set_read_len(len_read_pixel * len1);
          } else {
            wait_i2s();
          }
          *reg(SPI_USER_REG(_i2s_port)) = user;
          exec_spi();
        }
        memcpy(_regbuf, (void*)&spi_w0_reg[highpart ^= 8], len2 * len_read_pixel >> 3);
        param->src_x = 0;
        dstindex = param->fp_copy(dst, dstindex, dstindex + len2, param);
      } while (length);
//*/
    }

    void read_bytes(std::uint8_t* dst, std::int32_t length, bool use_dma = false)
    {
/*
      if (_dma_channel && use_dma) {
        wait_i2s();
        set_read_len(length << 3);
        _setup_dma_desc_links(dst, length);
        *reg(SPI_DMA_IN_LINK_REG(_i2s_port)) = SPI_INLINK_START | ((int)(&_dmadesc[0]) & 0xFFFFF);
        spi_dma_transfer_active(_dma_channel);
        exec_spi();
      } else {
        std::int32_t len1 = std::min(length, 32);  // 32 Byte read.
        std::int32_t len2 = len1;
        wait_i2s();
        set_read_len(len1 << 3);
        exec_spi();
        std::uint32_t highpart = 8;
        std::uint32_t userreg = *reg(SPI_USER_REG(_i2s_port));
        auto spi_w0_reg = reg(SPI_W0_REG(_i2s_port));
        do {
          if (0 == (length -= len1)) {
            len2 = len1;
            wait_i2s();
            *reg(SPI_USER_REG(_i2s_port)) = userreg;
          } else {
            std::uint32_t user = userreg;
            if (highpart) user = userreg | SPI_USR_MISO_HIGHPART;
            if (length < len1) {
              len1 = length;
              wait_i2s();
              set_read_len(len1 << 3);
            } else {
              wait_i2s();
            }
            *reg(SPI_USER_REG(_i2s_port)) = user;
            exec_spi();
          }
          memcpy(dst, (void*)&spi_w0_reg[highpart ^= 8], len2);
          dst += len2;
        } while (length);
      }
//*/
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
          write_bytes(buf, buflen);
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
          write_bytes(buf, buflen);
          pos += add;
        } while (--h);
      }
    }

    struct _dmabufs_t {
      std::uint8_t* buffer = nullptr;
      std::uint32_t length = 0;
      void free(void) {
        if (buffer) {
          heap_caps_free(buffer);
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
        _dmabufs[_dma_flip].buffer = (std::uint8_t*)heap_caps_malloc(length, MALLOC_CAP_DMA);
        _dmabufs[_dma_flip].length = _dmabufs[_dma_flip].buffer ? length : 0;
      }
      return _dmabufs[_dma_flip].buffer;
    }

    void delete_dmabuffer(void)
    {
      _dmabufs[0].free();
      _dmabufs[1].free();
    }

    static void _alloc_dmadesc(size_t len)
    {
      if (_dmadesc) heap_caps_free(_dmadesc);
      _dmadesc_len = len;
      _dmadesc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t) * len, MALLOC_CAP_DMA);
    }

    static void _setup_dma_desc_links(const std::uint8_t *data, std::int32_t len)
    {          //spicommon_setup_dma_desc_links
/*
      if (_dmadesc_len * SPI_MAX_DMA_LEN < len) {
        _alloc_dmadesc(len / SPI_MAX_DMA_LEN + 1);
      }
      lldesc_t *dmadesc = _dmadesc;

      while (len > SPI_MAX_DMA_LEN) {
        len -= SPI_MAX_DMA_LEN;
        dmadesc->buf = (std::uint8_t *)data;
        data += SPI_MAX_DMA_LEN;
        *(std::uint32_t*)dmadesc = SPI_MAX_DMA_LEN | SPI_MAX_DMA_LEN<<12 | 0x80000000;
        dmadesc->qe.stqe_next = dmadesc + 1;
        dmadesc++;
      }
      *(std::uint32_t*)dmadesc = ((len + 3) & ( ~3 )) | len << 12 | 0xC0000000;
      dmadesc->buf = (std::uint8_t *)data;
      dmadesc->qe.stqe_next = nullptr;
//*/
    }

    static void _setup_dma_desc_links(const std::uint8_t *data, std::int32_t w, std::int32_t h, std::int32_t width)
    {          //spicommon_setup_dma_desc_links
/*
      if (_dmadesc_len < h) {
        _alloc_dmadesc(h);
      }
      lldesc_t *dmadesc = _dmadesc;
      std::int32_t idx = 0;
      do {
        dmadesc[idx].buf = (std::uint8_t *)data;
        data += width;
        *(std::uint32_t*)(&dmadesc[idx]) = ((w + 3) & (~3)) | w<<12 | 0x80000000;
        dmadesc[idx].qe.stqe_next = &dmadesc[idx + 1];
      } while (++idx < h);
      --idx;
      dmadesc[idx].eof = 1;
//    *(std::uint32_t*)(&dmadesc[idx]) |= 0xC0000000;
      dmadesc[idx].qe.stqe_next = 0;
//*/
    }

    static void _setup_dma_desc_links(std::uint8_t** data, std::int32_t w, std::int32_t h, bool endless)
    {          //spicommon_setup_dma_desc_links
/*
      if (_dmadesc_len < h) {
        _alloc_dmadesc(h);
      }

      lldesc_t *dmadesc = _dmadesc;
      std::int32_t idx = 0;
      do {
        dmadesc[idx].buf = (std::uint8_t *)data[idx];
        *(std::uint32_t*)(&dmadesc[idx]) = w | w<<12 | 0x80000000;
        dmadesc[idx].qe.stqe_next = &dmadesc[idx + 1];
      } while (++idx < h);
      --idx;
      if (endless) {
        dmadesc[idx].qe.stqe_next = &dmadesc[0];
      } else {
        dmadesc[idx].eof = 1;
//        *(std::uint32_t*)(&dmadesc[idx]) |= 0xC0000000;
        dmadesc[idx].qe.stqe_next = 0;
      }
//*/
    }

    __attribute__ ((always_inline)) inline volatile std::uint32_t* reg(std::uint32_t addr) const { return (volatile std::uint32_t *)ETS_UNCACHED_ADDR(addr); }
    __attribute__ ((always_inline)) inline void set_clock_write(void) { *reg(I2S_CLKM_CONF_REG(_i2s_port)) = _clkdiv_write; }
    __attribute__ ((always_inline)) inline void set_clock_read(void)  { *reg(I2S_CLKM_CONF_REG(_i2s_port)) = _clkdiv_read;  }
    __attribute__ ((always_inline)) inline void exec_spi(void) {        *reg(SPI_CMD_REG(_i2s_port)) = SPI_USR; }
    __attribute__ ((always_inline)) inline void set_write_len(std::uint32_t bitlen) { *reg(SPI_MOSI_DLEN_REG(_i2s_port)) = bitlen - 1; }
    __attribute__ ((always_inline)) inline void set_read_len( std::uint32_t bitlen) { *reg(SPI_MISO_DLEN_REG(_i2s_port)) = bitlen - 1; }

    __attribute__ ((always_inline)) inline void dc_h(void) {
      auto mask_reg_dc = _mask_reg_dc;
      auto gpio_reg_dc_h = _gpio_reg_dc_h;
      wait_i2s();
      *gpio_reg_dc_h = mask_reg_dc;
    }
    __attribute__ ((always_inline)) inline void dc_l(void) {
      auto mask_reg_dc = _mask_reg_dc;
      auto gpio_reg_dc_l = _gpio_reg_dc_l;
      wait_i2s();
      *gpio_reg_dc_l = mask_reg_dc;
    }

    void wait_i2s(void) const {
      auto conf_reg1 = _conf_reg_default | I2S_TX_RESET;
//      auto conf_reg1 = (*reg(I2S_CONF_REG(_i2s_port)) | I2S_TX_RESET) & ~I2S_TX_START;
      while (!(*reg(I2S_STATE_REG(_i2s_port)) & I2S_TX_IDLE));

//      while (!(*reg(I2S_INT_RAW_REG(_i2s_port)) & I2S_TX_REMPTY_INT_RAW && *reg(I2S_STATE_REG(_i2s_port)) & I2S_TX_IDLE));
      *reg(I2S_CONF_REG(_i2s_port)) = conf_reg1;
//      while (*reg(I2S_STATE_REG(_i2s_port)) & I2S_TX_FIFO_RESET_BACK);
    }

    void wait(void) const {
      auto conf_reg1 = _conf_reg_default | I2S_TX_RESET;

      while (!(*reg(I2S_INT_RAW_REG(_i2s_port)) & I2S_TX_REMPTY_INT_RAW));
      *reg(I2S_INT_CLR_REG(_i2s_port)) = I2S_TX_REMPTY_INT_CLR;

      while (!(*reg(I2S_STATE_REG(_i2s_port)) & I2S_TX_IDLE));
      *reg(I2S_CONF_REG(_i2s_port)) = conf_reg1;
    }

    void cs_h(void) {
      std::int32_t spi_cs = _panel->spi_cs;
      if (spi_cs >= 0) *get_gpio_hi_reg(spi_cs) = (1 << (spi_cs & 31));
    }
    void cs_l(void) {
      std::int32_t spi_cs = _panel->spi_cs;
      if (spi_cs >= 0) *get_gpio_lo_reg(spi_cs) = (1 << (spi_cs & 31));
    }
/*

    void _set_write_dir(void)
    {
      // Set in-out modes of IO pads.
      gpio_set_direction(_i2s_d0, GPIO_MODE_OUTPUT);
      gpio_set_direction(_i2s_d1, GPIO_MODE_OUTPUT);
      gpio_set_direction(_i2s_d2, GPIO_MODE_OUTPUT);
      gpio_set_direction(_i2s_d3, GPIO_MODE_OUTPUT);
      gpio_set_direction(_i2s_d4, GPIO_MODE_OUTPUT);
      gpio_set_direction(_i2s_d5, GPIO_MODE_OUTPUT);
      gpio_set_direction(_i2s_d6, GPIO_MODE_OUTPUT);
      gpio_set_direction(_i2s_d7, GPIO_MODE_OUTPUT);
    }
//*/
    static constexpr std::uint32_t _conf_reg_default = I2S_TX_MSB_RIGHT | I2S_TX_RIGHT_FIRST | I2S_RX_RIGHT_FIRST;
    static constexpr std::uint32_t _conf_reg_start   = _conf_reg_default | I2S_TX_START;
    static constexpr std::uint32_t _sample_rate_conf_reg_32bit = 32 << I2S_TX_BITS_MOD_S | 32 << I2S_RX_BITS_MOD_S | 1 << I2S_TX_BCK_DIV_NUM_S | 1 << I2S_RX_BCK_DIV_NUM_S;
    static constexpr std::uint32_t _sample_rate_conf_reg_16bit = 16 << I2S_TX_BITS_MOD_S | 16 << I2S_RX_BITS_MOD_S | 1 << I2S_TX_BCK_DIV_NUM_S | 1 << I2S_RX_BCK_DIV_NUM_S;
    static constexpr std::uint32_t _fifo_conf_default = 1 << I2S_TX_FIFO_MOD | 1 << I2S_RX_FIFO_MOD | 32 << I2S_TX_DATA_NUM_S | 32 << I2S_RX_DATA_NUM_S;
    static constexpr std::uint32_t _fifo_conf_dma     = 1 << I2S_TX_FIFO_MOD | 1 << I2S_RX_FIFO_MOD | 32 << I2S_TX_DATA_NUM_S | 32 << I2S_RX_DATA_NUM_S | I2S_DSCR_EN;

    static constexpr gpio_num_t _i2s_wr  = (gpio_num_t)get_i2s_wr <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_rd  = (gpio_num_t)get_i2s_rd <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_rs  = (gpio_num_t)get_i2s_rs <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d0  = (gpio_num_t)get_i2s_d0 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d1  = (gpio_num_t)get_i2s_d1 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d2  = (gpio_num_t)get_i2s_d2 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d3  = (gpio_num_t)get_i2s_d3 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d4  = (gpio_num_t)get_i2s_d4 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d5  = (gpio_num_t)get_i2s_d5 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d6  = (gpio_num_t)get_i2s_d6 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d7  = (gpio_num_t)get_i2s_d7 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d8  = (gpio_num_t)get_i2s_d8 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d9  = (gpio_num_t)get_i2s_d9 <CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d10 = (gpio_num_t)get_i2s_d10<CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d11 = (gpio_num_t)get_i2s_d11<CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d12 = (gpio_num_t)get_i2s_d12<CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d13 = (gpio_num_t)get_i2s_d13<CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d14 = (gpio_num_t)get_i2s_d14<CFG, -1>::value;
    static constexpr gpio_num_t _i2s_d15 = (gpio_num_t)get_i2s_d15<CFG, -1>::value;

    static constexpr int _i2s_dlen = get_i2s_dlen<CFG, 8>::value;

    static constexpr i2s_port_t _i2s_port = get_i2s_port<CFG, I2S_NUM_0>::value;

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
    std::uint32_t _clkdiv_write;
    std::uint32_t _clkdiv_read;
    std::uint32_t _len_setwindow;
    _dmabufs_t _dmabufs[2];
    bool _begun_tr = false;
    bool _dma_flip = false;
    bool _fill_mode;
    std::uint32_t _mask_reg_dc;
    volatile std::uint32_t* _gpio_reg_dc_h;
    volatile std::uint32_t* _gpio_reg_dc_l;
    static std::uint32_t _regbuf[8];
    static lldesc_t* _dmadesc;
    static std::uint32_t _dmadesc_len;
    static bool _next_dma_reset;

//    static volatile spi_dev_t *_hw;
#if defined ( ARDUINO )
    static spi_t* _spi_handle;
#elif defined (CONFIG_IDF_TARGET_ESP32) // ESP-IDF
    static spi_device_handle_t _spi_handle;
#endif
  };
  template <class T> std::uint32_t LGFX_I2S<T>::_regbuf[];
  template <class T> lldesc_t* LGFX_I2S<T>::_dmadesc = nullptr;
  template <class T> std::uint32_t LGFX_I2S<T>::_dmadesc_len = 0;
  template <class T> bool LGFX_I2S<T>::_next_dma_reset;
//  template <class T> volatile spi_dev_t *LGFX_I2S<T>::_hw;

#if defined ( ARDUINO )
  template <class T> spi_t* LGFX_I2S<T>::_spi_handle;
#elif defined (CONFIG_IDF_TARGET_ESP32) // ESP-IDF
  template <class T> spi_device_handle_t LGFX_I2S<T>::_spi_handle;
#endif

//----------------------------------------------------------------------------

}
#endif
