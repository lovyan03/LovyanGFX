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
#ifndef LGFX_PARALLEL_ESP32_HPP_
#define LGFX_PARALLEL_ESP32_HPP_

#include <string.h>
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

#endif

#include "esp32_common.hpp"
#include "../LGFX_Device.hpp"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  #define SAFE_I2S_FIFO_WR_REG(i) (0x6000F000 + ((i)*0x1E000))
  #define SAFE_I2S_FIFO_RD_REG(i) (0x6000F004 + ((i)*0x1E000))

  #define MEMBER_DETECTOR(member, classname, classname_impl, valuetype) struct classname_impl { \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, T::member> check(decltype(T::member)*); \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, V> check(...); \
  };template<class T, valuetype V> class classname : public decltype(classname_impl::check<T, V>(nullptr)) {};
  MEMBER_DETECTOR(i2s_port, get_i2s_port, get_i2s_port_impl, i2s_port_t)
  MEMBER_DETECTOR(gpio_d0  , get_gpio_d0  , get_gpio_d0_impl  , int)
  MEMBER_DETECTOR(gpio_d1  , get_gpio_d1  , get_gpio_d1_impl  , int)
  MEMBER_DETECTOR(gpio_d2  , get_gpio_d2  , get_gpio_d2_impl  , int)
  MEMBER_DETECTOR(gpio_d3  , get_gpio_d3  , get_gpio_d3_impl  , int)
  MEMBER_DETECTOR(gpio_d4  , get_gpio_d4  , get_gpio_d4_impl  , int)
  MEMBER_DETECTOR(gpio_d5  , get_gpio_d5  , get_gpio_d5_impl  , int)
  MEMBER_DETECTOR(gpio_d6  , get_gpio_d6  , get_gpio_d6_impl  , int)
  MEMBER_DETECTOR(gpio_d7  , get_gpio_d7  , get_gpio_d7_impl  , int)
  //MEMBER_DETECTOR(gpio_d8  , get_gpio_d8  , get_gpio_d8_impl  , int)
  //MEMBER_DETECTOR(gpio_d9  , get_gpio_d9  , get_gpio_d9_impl  , int)
  //MEMBER_DETECTOR(gpio_d10 , get_gpio_d10 , get_gpio_d10_impl , int)
  //MEMBER_DETECTOR(gpio_d11 , get_gpio_d11 , get_gpio_d11_impl , int)
  //MEMBER_DETECTOR(gpio_d12 , get_gpio_d12 , get_gpio_d12_impl , int)
  //MEMBER_DETECTOR(gpio_d13 , get_gpio_d13 , get_gpio_d13_impl , int)
  //MEMBER_DETECTOR(gpio_d14 , get_gpio_d14 , get_gpio_d14_impl , int)
  //MEMBER_DETECTOR(gpio_d15 , get_gpio_d15 , get_gpio_d15_impl , int)
  MEMBER_DETECTOR(gpio_wr  , get_gpio_wr  , get_gpio_wr_impl  , int)
  MEMBER_DETECTOR(gpio_rd  , get_gpio_rd  , get_gpio_rd_impl  , int)
  MEMBER_DETECTOR(gpio_rs  , get_gpio_rs  , get_gpio_rs_impl  , int)
  MEMBER_DETECTOR(bus_dlen, get_bus_dlen, get_bus_dlen_impl, int)
  #undef MEMBER_DETECTOR

  template <class CFG>
  class LGFX_PARALLEL : public LGFX_Device
  {
  public:

    virtual ~LGFX_PARALLEL() {
      if (_dmadesc) {
        heap_free(_dmadesc);
        _dmadesc = nullptr;
        _dmadesc_len = 0;
      }
      delete_dmabuffer();
    }

    LGFX_PARALLEL() : LGFX_Device()
    {
    }

    __attribute__ ((always_inline)) inline void begin(void) { init_impl(); }

    __attribute__ ((always_inline)) inline void init(void) { init_impl(); }

    void writeCommand(uint_fast8_t cmd) override { startWrite(); write_cmd(cmd); endWrite(); }

    void writeData(uint_fast8_t data) override { startWrite(); write_data(data, 8); endWrite(); }

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

    void initBus(void) override
    {
      preInit();

      //Reset I2S subsystem
      *reg(I2S_CONF_REG(_i2s_port)) = I2S_TX_RESET | I2S_RX_RESET | I2S_TX_FIFO_RESET | I2S_RX_FIFO_RESET;
      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_default;

      //Reset DMA
      *reg(I2S_LC_CONF_REG(_i2s_port)) = I2S_IN_RST | I2S_OUT_RST | I2S_AHBM_RST | I2S_AHBM_FIFO_RST;
      *reg(I2S_LC_CONF_REG(_i2s_port)) = I2S_OUT_EOF_MODE;

      *reg(I2S_CONF2_REG(_i2s_port)) = I2S_LCD_EN;

      *reg(I2S_CONF1_REG(_i2s_port))
            = I2S_TX_PCM_BYPASS
            | I2S_TX_STOP_EN
            ;

      *reg(I2S_CONF_CHAN_REG(_i2s_port))
            = 1 << I2S_TX_CHAN_MOD_S
            | 1 << I2S_RX_CHAN_MOD_S
            ;

      *reg(I2S_INT_ENA_REG(_i2s_port)) = 0;

      *reg(I2S_OUT_LINK_REG(_i2s_port)) = 0;
      *reg(I2S_IN_LINK_REG(_i2s_port)) = 0;
      *reg(I2S_TIMING_REG(_i2s_port)) = 0;

      _alloc_dmadesc(1);
      memset(_dmadesc, 0, sizeof(lldesc_t));
    }

//----------------------------------------------------------------------------
  protected:

    void preCommandList(void) override
    {
      wait_i2s();
    }

    void postCommandList(void) override
    {
      wait_i2s();
    }

    void postSetPanel(void) override
    {
      _last_apb_freq = ~0u;
      _cmd_ramwr      = _panel->getCmdRamwr();
      _len_setwindow  = _panel->len_setwindow;
      fpGetWindowAddr = _len_setwindow == 32 ? PanelCommon::getWindowAddr32 : PanelCommon::getWindowAddr16;
/*
      int32_t spi_dc = _panel->spi_dc;
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

    void beginTransaction_impl(void) override {
      if (_in_transaction) return;
      _in_transaction = true;
      begin_transaction();
    }

    void begin_transaction(void)
    {
      uint32_t apb_freq = getApbFrequency();
      if (_last_apb_freq != apb_freq) {
        _last_apb_freq = apb_freq;
        // clock = 80MHz(apb_freq) / I2S_CLKM_DIV_NUM
        // I2S_CLKM_DIV_NUM 4=20MHz  /  5=16MHz  /  8=10MHz  /  10=8MHz
        uint32_t div_num = std::min(32u, std::max(4u, 1 + (apb_freq / (1 + _panel->freq_write))));
        _clkdiv_write =            I2S_CLKA_ENA
                      |            I2S_CLK_EN
                      |       1 << I2S_CLKM_DIV_A_S
                      |       0 << I2S_CLKM_DIV_B_S
                      | div_num << I2S_CLKM_DIV_NUM_S
                      ;
      }
      *reg(I2S_CLKM_CONF_REG(_i2s_port)) = _clkdiv_write;

      cs_l();
    }

    void endTransaction_impl(void) override {
      if (!_in_transaction) return;
      _in_transaction = false;
      end_transaction();
    }

    void end_transaction(void) {
      if (_panel->spi_cs < 0) {
        write_cmd(0); // NOP command
      }
      wait_i2s();
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
      set_window(xs, ys, xe, ye, _cmd_ramwr);
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      if (_in_transaction) {
        set_window(x, y, x, y, _cmd_ramwr);
        push_block(1);
        return;
      }

      begin_transaction();
      set_window(x, y, x, y, _cmd_ramwr);
      push_block(1);
      end_transaction();
    }

    void writeFillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
      set_window(x, y, x+w-1, y+h-1, _cmd_ramwr);
      push_block(w*h);
    }

    void pushBlock_impl(int32_t length) override
    {
      push_block(length);
    }

    void push_block(int32_t length, bool fillclock = false)
    {
      if (_write_conv.bits == 16) {
        *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
        auto conf_start = _conf_reg_start;
        uint32_t data = 0x01000100;
        data |= _color.raw << 16 | _color.raw >> 8;
        int32_t limit = ((length - 1) & 31) + 1;
        do {
          length -= limit;

          wait();
          *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;

          while (limit--) {
            *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = data;
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
        static constexpr uint32_t data_wr = 0x01000100;
        uint32_t data0 = _color.raw << 16 | _color.raw >>  8 | data_wr;
        uint32_t data1 = _color.raw                          | data_wr;
        uint32_t data2 = _color.raw <<  8 | _color.raw >> 16 | data_wr;
        *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
        length >>= 1;
        uint32_t limit = ((length - 1) % 10) + 1;
        do {
          length -= limit;

          wait();
          *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
          while (limit--) {
            *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = data0;
            *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = data1;
            *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = data2;
          }
          *reg(I2S_CONF_REG(_i2s_port)) = conf_start;
          limit = 10;
        } while (length);
      }
    }

    void write_cmd(uint_fast8_t cmd)
    {
      wait();
      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_32bit;
      *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
      *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = cmd << 16;
      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
//auto dummy = *reg(I2S_CONF_REG(_i2s_port));
    }

    void write_data(uint32_t data, uint32_t bit_length)
    {
      wait();
      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_32bit;
      *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;
      *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = (0x100 | (data & 0xFF)) << 16;
      while (bit_length -= 8) {
        data >>= 8;
        *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = (0x100 | (data & 0xFF)) << 16;
      }
      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
//auto dummy = *reg(I2S_CONF_REG(_i2s_port));
    }

    void set_window(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye, uint32_t cmd)
    {
/*
      uint32_t len32 = _len_setwindow >> 5;
      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_32bit;
      uint32_t data_wr = 0x1000000;
      wait_i2s();

      if (_xs != xs || _xe != xe) {
        _xs = xs;
        _xe = xe;
        uint32_t tmp = _colstart;
        xs += tmp;
        xe += tmp;
        *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = _cmd_caset << 16;
        if (!len32) {
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = xs<<16|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = xe<<16|data_wr;
        } else {
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = xs<< 8|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = xs<<16|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = xe<< 8|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = xe<<16|data_wr;
        }
      }
      if (_ys != ys || _ye != ye) {
        _ys = ys;
        _ye = ye;
        uint32_t tmp = _rowstart;
        ys += tmp;
        ye += tmp;
        *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = _cmd_raset << 16;
        if (!len32) {
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ys<<16|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ye<<16|data_wr;
        } else {
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ys<< 8|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ys<<16|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ye<< 8|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ye<<16|data_wr;
        }
      }
      *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = cmd << 16;

      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
/*/
      uint32_t len32 = _len_setwindow >> 5;
      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
      static constexpr uint32_t data_wr = 0x01000100;
      wait_i2s();
      *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;

      if (_xs != xs || _xe != xe) {
        _xs = xs;
        _xe = xe;
        uint32_t tmp = _colstart;
        xs += tmp;
        xe += tmp;
        *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = _cmd_caset;
        if (!len32) {
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = xs|xe<<16|data_wr;
        } else {
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = xs|xs<<8|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = xe|xe<<8|data_wr;
        }
      }
      if (_ys != ys || _ye != ye) {
        _ys = ys;
        _ye = ye;
        uint32_t tmp = _rowstart;
        ys += tmp;
        ye += tmp;
        *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = _cmd_raset;
        if (!len32) {
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ys|ye<<16|data_wr;
        } else {
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ys|ys<<8|data_wr;
          *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = ye|ye<<8|data_wr;
        }
      }
      *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = cmd;

      *reg(I2S_CONF_REG(_i2s_port)) = _conf_reg_start;
//*/
    }

    void start_read(void)
    {
      wait();
      gpio_lo(_gpio_rd);
//      gpio_pad_select_gpio(_gpio_rd);
//      gpio_set_direction(_gpio_rd, GPIO_MODE_OUTPUT);
      gpio_pad_select_gpio(_gpio_wr);
      gpio_hi(_gpio_wr);
      gpio_set_direction(_gpio_wr, GPIO_MODE_OUTPUT);
      gpio_pad_select_gpio(_gpio_rs);
      gpio_hi(_gpio_rs);
      gpio_set_direction(_gpio_rs, GPIO_MODE_OUTPUT);
//      if (_i2s_port == I2S_NUM_0) {
////        gpio_matrix_out(_gpio_rd, I2S0O_WS_OUT_IDX    ,1,0);
//        gpio_matrix_out(_gpio_rd, I2S0O_BCK_OUT_IDX    ,1,0);
//      } else {
////        gpio_matrix_out(_gpio_rd, I2S1O_WS_OUT_IDX    ,1,0);
//        gpio_matrix_out(_gpio_rd, I2S1O_BCK_OUT_IDX    ,1,0);
//      }
//*
//      auto idx_base = (_i2s_port == I2S_NUM_0) ? I2S0O_DATA_OUT8_IDX : I2S1O_DATA_OUT8_IDX;
//      gpio_matrix_in(_gpio_d7, idx_base + 7, 0); // MSB
//      gpio_matrix_in(_gpio_d6, idx_base + 6, 0);
//      gpio_matrix_in(_gpio_d5, idx_base + 5, 0);
//      gpio_matrix_in(_gpio_d4, idx_base + 4, 0);
//      gpio_matrix_in(_gpio_d3, idx_base + 3, 0);
//      gpio_matrix_in(_gpio_d2, idx_base + 2, 0);
//      gpio_matrix_in(_gpio_d1, idx_base + 1, 0);
//      gpio_matrix_in(_gpio_d0, idx_base    , 0); // LSB
//*/
/*
      gpio_pad_select_gpio(_gpio_d7); gpio_set_direction(_gpio_d7, GPIO_MODE_INPUT);
      gpio_pad_select_gpio(_gpio_d6); gpio_set_direction(_gpio_d6, GPIO_MODE_INPUT);
      gpio_pad_select_gpio(_gpio_d5); gpio_set_direction(_gpio_d5, GPIO_MODE_INPUT);
      gpio_pad_select_gpio(_gpio_d4); gpio_set_direction(_gpio_d4, GPIO_MODE_INPUT);
      gpio_pad_select_gpio(_gpio_d3); gpio_set_direction(_gpio_d3, GPIO_MODE_INPUT);
      gpio_pad_select_gpio(_gpio_d2); gpio_set_direction(_gpio_d2, GPIO_MODE_INPUT);
      gpio_pad_select_gpio(_gpio_d1); gpio_set_direction(_gpio_d1, GPIO_MODE_INPUT);
      gpio_pad_select_gpio(_gpio_d0); gpio_set_direction(_gpio_d0, GPIO_MODE_INPUT);
      set_clock_read();
/*/
      gpio_matrix_out(_gpio_d7, 0x100, 0, 0); // MSB
      gpio_matrix_out(_gpio_d6, 0x100, 0, 0);
      gpio_matrix_out(_gpio_d5, 0x100, 0, 0);
      gpio_matrix_out(_gpio_d4, 0x100, 0, 0);
      gpio_matrix_out(_gpio_d3, 0x100, 0, 0);
      gpio_matrix_out(_gpio_d2, 0x100, 0, 0);
      gpio_matrix_out(_gpio_d1, 0x100, 0, 0);
      gpio_matrix_out(_gpio_d0, 0x100, 0, 0); // LSB

      lgfxPinMode(_gpio_d7, pin_mode_t::input);
      lgfxPinMode(_gpio_d6, pin_mode_t::input);
      lgfxPinMode(_gpio_d5, pin_mode_t::input);
      lgfxPinMode(_gpio_d4, pin_mode_t::input);
      lgfxPinMode(_gpio_d3, pin_mode_t::input);
      lgfxPinMode(_gpio_d2, pin_mode_t::input);
      lgfxPinMode(_gpio_d1, pin_mode_t::input);
      lgfxPinMode(_gpio_d0, pin_mode_t::input);
//*/
    }

    void end_read(bool cs_ctrl = true)
    {
      wait();
      if (cs_ctrl) { cs_h(); }
      preInit();
/*
      gpio_pad_select_gpio(_gpio_rd);
      gpio_set_level(_gpio_rd, 1);
      gpio_set_direction(_gpio_rd, GPIO_MODE_OUTPUT);
      if (_i2s_port == I2S_NUM_0) {
        gpio_matrix_out(_gpio_wr, I2S0O_WS_OUT_IDX    ,1,0); // WR (Write-strobe in 8080 mode, Active-low)
      } else {
        gpio_matrix_out(_gpio_wr, I2S1O_WS_OUT_IDX    ,1,0); // WR (Write-strobe in 8080 mode, Active-low)
      }
      auto idx_base = (_i2s_port == I2S_NUM_0) ? I2S0O_DATA_OUT8_IDX : I2S1O_DATA_OUT8_IDX;
      gpio_matrix_out(_gpio_rs, idx_base + 8, 0, 0); // RS (Command/Data select: 0=CMD, 1=DATA)
*/
      if (cs_ctrl) {
        if (_panel->spi_cs < 0) {
          write_cmd(0); // NOP command
        }
        cs_l();
      }
    }

    uint32_t read_data(uint32_t length)
    {
      union {
        uint32_t res;
        uint8_t raw[4];
      };
      length = (length + 7) & ~7;

      auto buf = raw;
      do {
        uint32_t tmp = GPIO.in;   // dummy read speed tweak.
        tmp = GPIO.in;
        gpio_hi(_gpio_rd);
        gpio_lo(_gpio_rd);
        *buf++ = reg_to_value(tmp);
      } while (length -= 8);
      return res;
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
      int32_t whb = w * h * bytes;
      if (param->transp == ~0) {
        if (param->no_convert) {
          uint32_t i = (src_x + param->src_y * param->src_bitwidth) * bytes;
          auto src = &((const uint8_t*)param->src_data)[i];
          setWindow_impl(x, y, xr, y + h - 1);
          if (param->src_bitwidth == w) {
            write_bytes(src, whb, use_dma);
          } else {
            auto add = param->src_bitwidth * bytes;
            do {
              write_bytes(src, w * bytes, use_dma);
              src += add;
            } while (--h);
          }
        } else
        if (param->src_bitwidth == w && (whb <= 1024)) {
          uint8_t buf[whb];
          fp_copy(buf, 0, w * h, param);
          setWindow_impl(x, y, xr, y + h - 1);
          write_bytes(buf, whb, true);
        } else {
          int32_t wb = w * bytes;
          uint8_t buf[wb];
          fp_copy(buf, 0, w, param);
          setWindow_impl(x, y, xr, y + h - 1);
          write_bytes(buf, wb, true);
          while (--h) {
            param->src_x = src_x;
            param->src_y++;
            fp_copy(buf, 0, w, param);
            write_bytes(buf, wb, true);
          }
        }
      } else {
        auto fp_skip = param->fp_skip;
        h += y;
        uint8_t buf[w * bytes];
        do {
          int32_t i = 0;
          while (w != (i = fp_skip(i, w, param))) {
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
      uint8_t buf[512];
      const uint32_t bytes = _write_conv.bytes;
      auto fp_copy = param->fp_copy;
      const uint32_t limit = (bytes == 2) ? 256 : 170;
      uint8_t len = length % limit;
      if (len) {
        fp_copy(buf, 0, len, param);
        write_bytes(buf, len * bytes);
        if (0 == (length -= len)) return;
      }
      do {
        fp_copy(buf, 0, limit, param);
        write_bytes(buf, limit * bytes);
      } while (length -= limit);
    }

    void write_bytes(const uint8_t* data, int32_t length, bool use_dma = false)
    {
      auto conf_start = _conf_reg_start;
      static constexpr uint32_t data_wr = 0x01000100;

      wait();
      if (length & 1) {
        write_data(data[0], 8);
        if (!--length) return;
        ++data;
      }

      *reg(I2S_SAMPLE_RATE_CONF_REG(_i2s_port)) = _sample_rate_conf_reg_16bit;
      *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_default;

      int32_t limit = (((length>>1)-1)&(31))+1;
      length -= limit << 1;
      do {
        *reg(SAFE_I2S_FIFO_WR_REG(_i2s_port)) = data[0] << 16 | data[1] | data_wr;
        data += 2;
      } while (--limit);
      *reg(I2S_CONF_REG(_i2s_port)) = conf_start;

      if (!length) return;

      uint32_t lbase = 64;
      do {
        auto buf = (uint32_t*)get_dmabuffer(512);
        int32_t limit = ((length - 1) & (lbase - 1)) + 1;
        length -= limit;
        _dmadesc->buf = (uint8_t*)buf;
        int32_t i = 0;
        limit>>=1;
        do {
          buf[i] = data[0]<<16 | data[1] | data_wr;
          data += 2;
        } while (++i != limit);
        *(uint32_t*)_dmadesc = (((i<<2) + 3) &  ~3 ) | i << 14 | 0xC0000000;
        wait();
        *reg(I2S_FIFO_CONF_REG(_i2s_port)) = _fifo_conf_dma;
        *reg(I2S_OUT_LINK_REG(_i2s_port)) = I2S_OUTLINK_START | ((uint32_t)_dmadesc & I2S_OUTLINK_ADDR);
        size_t wait = 48;
        do { __asm__ __volatile__ ("nop"); } while (--wait);
        *reg(I2S_CONF_REG(_i2s_port)) = conf_start;
        if (lbase != 256) lbase <<= 1;
      } while (length);
    }

    void readRect_impl(int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) override
    {
      startWrite();
      set_window(x, y, x + w - 1, y + h - 1, _panel->getCmdRamrd());
      auto len = w * h;
      start_read();
      GPIO.in;
      gpio_hi(_gpio_rd);
      GPIO.in;
      gpio_lo(_gpio_rd);
      taskYIELD();
      if (param->no_convert) {
        read_bytes((uint8_t*)dst, len * _read_conv.bytes);
      } else {
        read_pixels(dst, len, param);
      }
      end_read();
      endWrite();
    }

    static uint_fast8_t reg_to_value(uint32_t raw_value)
    {
      return ((raw_value >> _gpio_d7) & 1) << 7
           | ((raw_value >> _gpio_d6) & 1) << 6
           | ((raw_value >> _gpio_d5) & 1) << 5
           | ((raw_value >> _gpio_d4) & 1) << 4
           | ((raw_value >> _gpio_d3) & 1) << 3
           | ((raw_value >> _gpio_d2) & 1) << 2
           | ((raw_value >> _gpio_d1) & 1) << 1
           | ((raw_value >> _gpio_d0) & 1) ;
    }

    void read_pixels(void* dst, int32_t length, pixelcopy_t* param)
    {
      const auto bytes = _read_conv.bytes;
      uint32_t limit = (bytes == 2) ? 16 : 10;
      param->src_data = _regbuf;
      int32_t dstindex = 0;
      do {
        uint32_t len2 = (limit > length) ? length : limit;
        length -= len2;
        uint32_t i = len2 * bytes;
        auto d = (uint8_t*)_regbuf;
        do {
          uint32_t tmp = GPIO.in;
          gpio_hi(_gpio_rd);
          gpio_lo(_gpio_rd);
          *d++ = reg_to_value(tmp);
        } while (--i);
        param->src_x = 0;
        dstindex = param->fp_copy(dst, dstindex, dstindex + len2, param);
      } while (length);
    }

    void read_bytes(uint8_t* dst, int32_t length)
    {
      do {
        uint32_t tmp = GPIO.in;   // dummy read speed tweak.
        tmp = GPIO.in;
        gpio_hi(_gpio_rd);
        gpio_lo(_gpio_rd);
        *dst++ = reg_to_value(tmp);
      } while (--length);
    }

    static void _alloc_dmadesc(size_t len)
    {
      if (_dmadesc) heap_caps_free(_dmadesc);
      _dmadesc_len = len;
      _dmadesc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t) * len, MALLOC_CAP_DMA);
    }

    __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) const { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }

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
      auto conf_reg1 = _conf_reg_default | I2S_TX_RESET | I2S_RX_RESET | I2S_RX_FIFO_RESET;
      while (!(*reg(I2S_STATE_REG(_i2s_port)) & I2S_TX_IDLE));
      *reg(I2S_CONF_REG(_i2s_port)) = conf_reg1;
    }

    void wait(void) const {
      auto conf_reg1 = _conf_reg_default | I2S_TX_RESET | I2S_RX_RESET | I2S_RX_FIFO_RESET;

      *reg(I2S_INT_CLR_REG(_i2s_port)) = I2S_TX_REMPTY_INT_CLR;
      while (!(*reg(I2S_INT_RAW_REG(_i2s_port)) & I2S_TX_REMPTY_INT_RAW));

      while (!(*reg(I2S_STATE_REG(_i2s_port)) & I2S_TX_IDLE));
      *reg(I2S_CONF_REG(_i2s_port)) = conf_reg1;
    }

    void preInit(void)
    {
      gpio_pad_select_gpio(_gpio_d0);
      gpio_pad_select_gpio(_gpio_d1);
      gpio_pad_select_gpio(_gpio_d2);
      gpio_pad_select_gpio(_gpio_d3);
      gpio_pad_select_gpio(_gpio_d4);
      gpio_pad_select_gpio(_gpio_d5);
      gpio_pad_select_gpio(_gpio_d6);
      gpio_pad_select_gpio(_gpio_d7);

      gpio_pad_select_gpio(_gpio_rd);
      gpio_pad_select_gpio(_gpio_wr);
      gpio_pad_select_gpio(_gpio_rs);

      gpio_hi(_gpio_rd);
      gpio_hi(_gpio_wr);
      gpio_hi(_gpio_rs);

      gpio_set_direction(_gpio_rd, GPIO_MODE_OUTPUT);
      gpio_set_direction(_gpio_wr, GPIO_MODE_OUTPUT);
      gpio_set_direction(_gpio_rs, GPIO_MODE_OUTPUT);

      auto idx_base = (_i2s_port == I2S_NUM_0) ? I2S0O_DATA_OUT8_IDX : I2S1O_DATA_OUT8_IDX;
      gpio_matrix_out(_gpio_rs, idx_base + 8, 0, 0);
      gpio_matrix_out(_gpio_d7, idx_base + 7, 0, 0);
      gpio_matrix_out(_gpio_d6, idx_base + 6, 0, 0);
      gpio_matrix_out(_gpio_d5, idx_base + 5, 0, 0);
      gpio_matrix_out(_gpio_d4, idx_base + 4, 0, 0);
      gpio_matrix_out(_gpio_d3, idx_base + 3, 0, 0);
      gpio_matrix_out(_gpio_d2, idx_base + 2, 0, 0);
      gpio_matrix_out(_gpio_d1, idx_base + 1, 0, 0);
      gpio_matrix_out(_gpio_d0, idx_base    , 0, 0);

      uint32_t dport_clk_en;
      uint32_t dport_rst;

      if (_i2s_port == I2S_NUM_0) {
        idx_base = I2S0O_WS_OUT_IDX;
        dport_clk_en = DPORT_I2S0_CLK_EN;
        dport_rst = DPORT_I2S0_RST;
      } else {
        idx_base = I2S1O_WS_OUT_IDX;
        dport_clk_en = DPORT_I2S1_CLK_EN;
        dport_rst = DPORT_I2S1_RST;
      }
      gpio_matrix_out(_gpio_wr, idx_base, 1, 0); // WR (Write-strobe in 8080 mode, Active-low)

      DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, dport_clk_en);
      DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, dport_rst);
    }

    static constexpr uint32_t _conf_reg_default = I2S_TX_MSB_RIGHT | I2S_TX_RIGHT_FIRST | I2S_RX_RIGHT_FIRST;
    static constexpr uint32_t _conf_reg_start   = _conf_reg_default | I2S_TX_START;
    static constexpr uint32_t _sample_rate_conf_reg_32bit = 32 << I2S_TX_BITS_MOD_S | 32 << I2S_RX_BITS_MOD_S | 1 << I2S_TX_BCK_DIV_NUM_S | 1 << I2S_RX_BCK_DIV_NUM_S;
    static constexpr uint32_t _sample_rate_conf_reg_16bit = 16 << I2S_TX_BITS_MOD_S | 16 << I2S_RX_BITS_MOD_S | 1 << I2S_TX_BCK_DIV_NUM_S | 1 << I2S_RX_BCK_DIV_NUM_S;
    static constexpr uint32_t _fifo_conf_default = 1 << I2S_TX_FIFO_MOD | 1 << I2S_RX_FIFO_MOD | 32 << I2S_TX_DATA_NUM_S | 32 << I2S_RX_DATA_NUM_S;
    static constexpr uint32_t _fifo_conf_dma     = 1 << I2S_TX_FIFO_MOD | 1 << I2S_RX_FIFO_MOD | 32 << I2S_TX_DATA_NUM_S | 32 << I2S_RX_DATA_NUM_S | I2S_DSCR_EN;

    static constexpr gpio_num_t _gpio_wr  = (gpio_num_t)get_gpio_wr <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_rd  = (gpio_num_t)get_gpio_rd <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_rs  = (gpio_num_t)get_gpio_rs <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_d0  = (gpio_num_t)get_gpio_d0 <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_d1  = (gpio_num_t)get_gpio_d1 <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_d2  = (gpio_num_t)get_gpio_d2 <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_d3  = (gpio_num_t)get_gpio_d3 <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_d4  = (gpio_num_t)get_gpio_d4 <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_d5  = (gpio_num_t)get_gpio_d5 <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_d6  = (gpio_num_t)get_gpio_d6 <CFG, -1>::value;
    static constexpr gpio_num_t _gpio_d7  = (gpio_num_t)get_gpio_d7 <CFG, -1>::value;
    //static constexpr gpio_num_t _gpio_d8  = (gpio_num_t)get_gpio_d8 <CFG, -1>::value;
    //static constexpr gpio_num_t _gpio_d9  = (gpio_num_t)get_gpio_d9 <CFG, -1>::value;
    //static constexpr gpio_num_t _gpio_d10 = (gpio_num_t)get_gpio_d10<CFG, -1>::value;
    //static constexpr gpio_num_t _gpio_d11 = (gpio_num_t)get_gpio_d11<CFG, -1>::value;
    //static constexpr gpio_num_t _gpio_d12 = (gpio_num_t)get_gpio_d12<CFG, -1>::value;
    //static constexpr gpio_num_t _gpio_d13 = (gpio_num_t)get_gpio_d13<CFG, -1>::value;
    //static constexpr gpio_num_t _gpio_d14 = (gpio_num_t)get_gpio_d14<CFG, -1>::value;
    //static constexpr gpio_num_t _gpio_d15 = (gpio_num_t)get_gpio_d15<CFG, -1>::value;

    static constexpr int _bus_dlen = (get_bus_dlen<CFG, 8>::value + 7) & ~7;

    static constexpr i2s_port_t _i2s_port = get_i2s_port<CFG, I2S_NUM_0>::value;

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
  private:
    uint32_t _last_apb_freq;
    uint32_t _clkdiv_write;
    uint32_t _len_setwindow;
    uint32_t _mask_reg_dc;
    volatile uint32_t* _gpio_reg_dc_h;
    volatile uint32_t* _gpio_reg_dc_l;
    static uint32_t _regbuf[8];
    static lldesc_t* _dmadesc;
    static uint32_t _dmadesc_len;
  };
  template <class T> uint32_t LGFX_PARALLEL<T>::_regbuf[];
  template <class T> lldesc_t* LGFX_PARALLEL<T>::_dmadesc = nullptr;
  template <class T> uint32_t LGFX_PARALLEL<T>::_dmadesc_len = 0;

//----------------------------------------------------------------------------
 }
}

using lgfx::LGFX_PARALLEL;

#endif
