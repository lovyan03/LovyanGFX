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
#ifndef LGFX_SPI_ESP32_HPP_
#define LGFX_SPI_ESP32_HPP_

#include <string.h>
#include <type_traits>

#include <driver/periph_ctrl.h>
#include <driver/rtc_io.h>
#include <driver/spi_common.h>
#include <esp_heap_caps.h>
#include <freertos/task.h>
#include <soc/rtc.h>
#include <soc/spi_reg.h>
#include <soc/spi_struct.h>

#if defined (ARDUINO) // Arduino ESP32
 #include <SPI.h>
 #include <soc/periph_defs.h>
 #include <esp32-hal-cpu.h>
#else
 #include <esp_log.h>
 #include <driver/spi_master.h>
#endif

#if __has_include(<driver/spi_common_internal.h>)
 #include <driver/spi_common_internal.h>
#endif

#if defined (CONFIG_IDF_TARGET_ESP32S2)
 #define SPI_PIN_REG SPI_MISC_REG
 #define LGFX_SPI_DEFAULT SPI3_HOST
#else
 #define LGFX_SPI_DEFAULT VSPI_HOST
#endif

#include "esp32_common.hpp"
#include "../LGFX_Device.hpp"

#ifdef min
#undef min
#endif

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  inline static void spi_dma_transfer_active(int dmachan)
  {
    spicommon_dmaworkaround_transfer_active(dmachan);
  }

  #define MEMBER_DETECTOR(member, classname, classname_impl, valuetype) struct classname_impl { \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, T::member> check(decltype(T::member)*); \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, V> check(...); \
  };template<class T, valuetype V> class classname : public decltype(classname_impl::check<T, V>(nullptr)) {};
  MEMBER_DETECTOR(spi_host   , get_spi_host   , get_spi_host_impl   , spi_host_device_t)
  MEMBER_DETECTOR(spi_mosi   , get_spi_mosi   , get_spi_mosi_impl   , int)
  MEMBER_DETECTOR(spi_miso   , get_spi_miso   , get_spi_miso_impl   , int)
  MEMBER_DETECTOR(spi_sclk   , get_spi_sclk   , get_spi_sclk_impl   , int)
  MEMBER_DETECTOR(spi_dlen   , get_spi_dlen   , get_spi_dlen_impl   , int)
  MEMBER_DETECTOR(dma_channel, get_dma_channel, get_dma_channel_impl, int)
  #undef MEMBER_DETECTOR

  template <class CFG>
  class LGFX_SPI : public LGFX_Device
  {
  public:

    virtual ~LGFX_SPI() {
      if ((0 != _dma_channel) && _dmadesc) {
        heap_free(_dmadesc);
        _dmadesc = nullptr;
        _dmadesc_len = 0;
      }
      delete_dmabuffer();
    }

    LGFX_SPI() : LGFX_Device()
    {
    }

    void init(int sclk, int miso, int mosi, spi_host_device_t host = LGFX_SPI_DEFAULT)
    {
      _spi_sclk = sclk;
      _spi_miso = miso;
      _spi_mosi = mosi;
      _spi_host = host;

      init_impl();
    }

    __attribute__ ((always_inline)) inline void begin(int sclk, int miso, int mosi, spi_host_device_t host = LGFX_SPI_DEFAULT) { init(sclk, miso, mosi, host); }

    __attribute__ ((always_inline)) inline void begin(void) { init_impl(); }

    __attribute__ ((always_inline)) inline void init(void) { init_impl(); }

    void writeCommand(uint_fast8_t cmd) override { write_cmd(cmd); }

    void writeData(uint_fast8_t data) override { if (_spi_dlen == 16) { write_data(data << 8, _spi_dlen); } else { write_data(data, _spi_dlen); } }

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
      spi::init(_spi_host, _spi_sclk, _spi_miso, _spi_mosi, _dma_channel);
    }

    void releaseBus(void)
    {
      lgfxPinMode(_spi_mosi, pin_mode_t::output);
      lgfxPinMode(_spi_miso, pin_mode_t::output);
      lgfxPinMode(_spi_sclk, pin_mode_t::output);
      spi::release(_spi_host);
    }

//----------------------------------------------------------------------------
  protected:

    void preInit(void) override
    {
      _spi_host = get_spi_host<CFG, LGFX_SPI_DEFAULT>::value;
      //_spi_port = (_spi_host == HSPI_HOST) ? 2 : 3;  // FSPI=1  HSPI=2  VSPI=3;
      _spi_port = (_spi_host + 1);  // FSPI=1  HSPI=2  VSPI=3;
      _spi_w0_reg = reg(SPI_W0_REG(_spi_port));
      _spi_cmd_reg = reg(SPI_CMD_REG(_spi_port));
      _spi_user_reg = reg(SPI_USER_REG(_spi_port));
      _spi_mosi_dlen_reg = reg(SPI_MOSI_DLEN_REG(_spi_port));
      _spi_dma_out_link_reg = reg(SPI_DMA_OUT_LINK_REG(_spi_port));
    }

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
      _last_apb_freq = ~0u;
      _cmd_ramwr      = _panel->getCmdRamwr();
      _len_setwindow  = _panel->len_setwindow;

      if (_panel->spi_dlen >> 3) _spi_dlen = _panel->spi_dlen & ~7;
      //fpGetWindowAddr = _len_setwindow == 32 ? PanelCommon::getWindowAddr32 : PanelCommon::getWindowAddr16;

      int32_t spi_dc = _panel->spi_dc;
      _mask_reg_dc = (spi_dc < 0) ? 0 : (1 << (spi_dc & 31));

      _gpio_reg_dc_l = get_gpio_lo_reg(spi_dc);
      _gpio_reg_dc_h = get_gpio_hi_reg(spi_dc);
      *_gpio_reg_dc_h = _mask_reg_dc;
      lgfxPinMode(spi_dc, pin_mode_t::output);

      gpio_hi(_panel->spi_cs);
      lgfxPinMode(_panel->spi_cs, pin_mode_t::output);

      postSetRotation();
      postSetColorDepth();
    }

    void postSetRotation(void) override
    {
      bool fullscroll = (_sx == 0 && _sy == 0 && _sw == _width && _sh == _height);
      /*
      _cmd_caset = _panel->getCmdCaset();
      _cmd_raset = _panel->getCmdRaset();
      _colstart  = _panel->getColStart();
      _rowstart  = _panel->getRowStart();
      //*/
      _width     = _panel->getWidth();
      _height    = _panel->getHeight();
      _clip_r = _width - 1;
      _clip_b = _height - 1;

      if (fullscroll) {
        _sw = _width;
        _sh = _height;
      }
      _clip_l = _clip_t = 0;
    }

    void beginTransaction_impl(void) override {
      if (_in_transaction) return;
      _in_transaction = true;
      begin_transaction();
    }

    void begin_transaction(void) {
      _fill_mode = false;
      uint32_t apb_freq = getApbFrequency();
      if (_last_apb_freq != apb_freq) {
        _last_apb_freq = apb_freq;
        _clkdiv_read  = FreqToClockDiv(apb_freq, _panel->freq_read);
        _clkdiv_fill  = FreqToClockDiv(apb_freq, _panel->freq_fill);
        _clkdiv_write = FreqToClockDiv(apb_freq, _panel->freq_write);
      }

      auto spi_mode = _panel->spi_mode;
      _user = (spi_mode == 1 || spi_mode == 2) ? SPI_CK_OUT_EDGE | SPI_USR_MOSI : SPI_USR_MOSI;
      uint32_t pin = (spi_mode & 2) ? SPI_CK_IDLE_EDGE : 0;

      spi::beginTransaction(_spi_host);

      if (_dma_channel) {
        _next_dma_reset = true;
      }

      *_spi_user_reg = _user;
      *reg(SPI_PIN_REG(_spi_port))  = pin;
      set_clock_write();

      cs_l();
      if (nullptr != _panel->fp_begin) { _panel->fp_begin(_panel, this); }
    }

    void endTransaction_impl(void) override
    {
      if (!_in_transaction) return;
      end_transaction();
      _in_transaction = false;
    }

    void end_transaction(void) {
      if (_auto_display && nullptr != _panel->fp_display) { _panel->fp_display(_panel, this, 0, 0, 0, 0); }
      if (nullptr != _panel->fp_end) { _panel->fp_end(_panel, this); }
      if (_spi_dlen == 16 && (_align_data)) write_data(0, 8);
      if (_panel->spi_cs < 0) {
        write_cmd(0); // NOP command
      }
      dc_h();
      spi::endTransaction(_spi_host, _panel->spi_cs);
#if defined (ARDUINO) // Arduino ESP32
      *_spi_user_reg = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN; // for other SPI device (SD)
#endif
    }

    void initDMA_impl(void) override
    {
      if (_dma_channel) {
        spi_dma_reset();
      }
    }

    void waitDMA_impl(void) override
    {
      wait_spi();
    }

    bool dmaBusy_impl(void) override
    {
      return *_spi_cmd_reg & SPI_USR;
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
      if (!_panel->fp_fillRect) {
        if (_in_transaction) {
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
      else
      {
        startWrite();
        _panel->fp_fillRect(_panel, this, x, y, 1, 1, _color.raw);
        endWrite();
      }
    }

    void writeFillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
      if (!_panel->fp_fillRect) {
        if (_fill_mode) {
          _fill_mode = false;
          wait_spi();
          set_clock_write();
        }
        set_window(x, y, x+w-1, y+h-1);
        write_cmd(_cmd_ramwr);
        push_block(w*h, _clkdiv_write != _clkdiv_fill);
      }
      else
      {
        _panel->fp_fillRect(_panel, this, x, y, w, h, _color.raw);
      }
    }

    void pushBlock_impl(int32_t length) override
    {
      if (_panel->fp_pushBlock)
      {
        _panel->fp_pushBlock(_panel, this, length, _color.raw);
      }
      else
      {
        push_block(length);
      }
    }

    void push_block(int32_t length, bool fillclock = false)
    {
      auto bits = _write_conv.bits;
      uint32_t regbuf0 = _color.raw;
      if (length == 1) { write_data(regbuf0, bits); return; }

      length *= bits;          // convert to bitlength.
      uint32_t len = std::min(96, length); // 1st send length = max 12Byte (96bit). 
      bool bits16 = (bits == 16);

      uint32_t regbuf1;
      uint32_t regbuf2;
      // make 12Bytes data.
      if (bits16) {
        regbuf0 = regbuf0 | regbuf0 << 16;
        regbuf1 = regbuf0;
        regbuf2 = regbuf0;
      } else {
        if (_spi_dlen == 16 && length & 8) _align_data = !_align_data;
        regbuf0 = regbuf0      | regbuf0 << 24;
        regbuf1 = regbuf0 >> 8 | regbuf0 << 16;
        regbuf2 = regbuf0 >>16 | regbuf0 <<  8;
      }

      auto spi_w0_reg = _spi_w0_reg;
      dc_h();

      // copy to SPI buffer register
      spi_w0_reg[0] = regbuf0;
      spi_w0_reg[1] = regbuf1;
      spi_w0_reg[2] = regbuf2;

      set_write_len(len);

      if (fillclock) {
        set_clock_fill();  // fillmode clockup
        _fill_mode = true;
      }

      exec_spi();   // 1st send.
      if (0 == (length -= len)) return;

      uint32_t regbuf[7];
      regbuf[0] = regbuf0;
      regbuf[1] = regbuf1;
      regbuf[2] = regbuf2;
      regbuf[3] = regbuf0;
      regbuf[4] = regbuf1;
      regbuf[5] = regbuf2;
      regbuf[6] = regbuf0;

      // copy to SPI buffer register
      memcpy((void*)&spi_w0_reg[3], regbuf, 24);
      memcpy((void*)&spi_w0_reg[9], regbuf, 28);

      // limit = 64Byte / depth_bytes;
      // When 24bit color, 504 bits out of 512bit buffer are used.
      // When 16bit color, it uses exactly 512 bytes. but, it behaves like a ring buffer, can specify a larger size.
      uint32_t limit;
      if (bits16) {
#if defined( CONFIG_IDF_TARGET_ESP32S2 )
        limit = (1 << 9);
#else
        limit = (1 << 11);
#endif
        len = length & (limit - 1);
      } else {
        limit = 504;
        len = length % limit;
      }
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
    }

    void write_cmd(uint_fast8_t cmd)
    {
      if (_spi_dlen == 16) {
        if (_align_data) write_data(0, 8);
        cmd <<= 8;
      }
      uint32_t len = _spi_dlen - 1;
      auto spi_mosi_dlen_reg = _spi_mosi_dlen_reg;
      auto spi_w0_reg        = _spi_w0_reg;
      dc_l();
      *spi_mosi_dlen_reg = len;
      *spi_w0_reg = cmd;
      exec_spi();
    }

    void write_data(uint32_t data, uint32_t bit_length)
    {
      auto spi_w0_reg        = _spi_w0_reg;
      auto spi_mosi_dlen_reg = _spi_mosi_dlen_reg;
      dc_h();
      *spi_mosi_dlen_reg = bit_length - 1;
      *spi_w0_reg = data;
      exec_spi();
      if (_spi_dlen == 16 && (bit_length & 8)) _align_data = !_align_data;
    }

    __attribute__ ((always_inline)) inline 
    void write_cmd_data(const uint8_t* addr)
    {
      auto spi_mosi_dlen_reg = _spi_mosi_dlen_reg;
      auto spi_w0_reg        = _spi_w0_reg;
      if (_spi_dlen == 16 && _align_data)
      {
        _align_data = false;
        dc_h();
        *spi_mosi_dlen_reg = 8 - 1;
        *spi_w0_reg = 0;
        exec_spi();
      }

      do {
        uint32_t data = *addr++;
        if (_spi_dlen == 16) {
          data <<= 8;
        }
        uint32_t len = _spi_dlen - 1;
        dc_l();
        *spi_mosi_dlen_reg = len;
        *spi_w0_reg = data;
        exec_spi();
//        write_cmd(*addr++);
        uint_fast8_t numArgs = *addr++;
        if (numArgs)
        {
          data = *reinterpret_cast<const uint32_t*>(addr);
          if (_spi_dlen == 16)
          {
            if (numArgs > 2)
            {
              uint_fast8_t len = ((numArgs + 1) >> 1) + 1;
              uint_fast8_t i = 1;
              do
              {
                _spi_w0_reg[i] = addr[i * 2] << 8 | addr[i * 2 + 1] << 24;
              } while (++i != len);
            }
            data = (data & 0xFF) << 8 | (data >> 8) << 24;
            //write_data(addr[0] << 8 | addr[1] << 24, _spi_dlen * numArgs);
          }
          else
          {
            if (numArgs > 4)
            {
              memcpy((void*)&_spi_w0_reg[1], addr + 4, numArgs - 4);
            }
            //write_data(*reinterpret_cast<const uint32_t*>(addr), _spi_dlen * numArgs);
          }
          addr += numArgs;
          len = _spi_dlen * numArgs - 1;
          dc_h();
          *spi_mosi_dlen_reg = len;
          *spi_w0_reg = data;
          exec_spi();
        }
      } while (reinterpret_cast<const uint16_t*>(addr)[0] != 0xFFFF);
    }

    void set_window(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
    {
//*
      uint8_t buf[16];
      if (auto b = _panel->getWindowCommands1(buf, xs, ys, xe, ye)) { write_cmd_data(b); }
      if (auto b = _panel->getWindowCommands2(buf, xs, ys, xe, ye)) { write_cmd_data(b); }
      return;
/*/
      uint32_t len;
      if (_spi_dlen == 8) {
        len = _len_setwindow - 1;
      } else {
        len = (_len_setwindow << 1) - 1;
        if (_align_data) write_data(0, 8);
      }
      auto spi_mosi_dlen_reg = _spi_mosi_dlen_reg;
      auto fp = fpGetWindowAddr;

      if (_xs != xs || _xe != xe) {
        auto cmd = _cmd_caset;
        if (_spi_dlen == 16) cmd <<= 8;
        uint32_t l  = _spi_dlen - 1;
        auto spi_w0_reg  = _spi_w0_reg;
        auto spi_cmd_reg = _spi_cmd_reg;
        dc_l();
        *spi_mosi_dlen_reg = l;
        *spi_w0_reg = cmd;
        *spi_cmd_reg = SPI_USR;

        uint32_t tmp = _colstart;

        tmp = fp(xs + tmp, xe + tmp);
        if (_spi_dlen == 16) {
          auto t = tmp >> 16;
          spi_w0_reg[1] = (t & 0xFF) << 8 | (t >> 8) << 24;
          tmp = (tmp & 0xFF) << 8 | (tmp >> 8) << 24;
        }
        dc_h();
        *spi_w0_reg = tmp;
        *spi_mosi_dlen_reg = len;
        *spi_cmd_reg = SPI_USR;
        _xs = xs;
        _xe = xe;
      }
      if (_ys != ys || _ye != ye) {
        auto cmd = _cmd_raset;
        if (_spi_dlen == 16) cmd <<= 8;
        uint32_t l  = _spi_dlen - 1;
        auto spi_w0_reg  = _spi_w0_reg;
        auto spi_cmd_reg = _spi_cmd_reg;
        dc_l();
        *spi_mosi_dlen_reg = l;
        *spi_w0_reg = cmd;
        *spi_cmd_reg = SPI_USR;

        uint32_t tmp = _rowstart;

        tmp = fp(ys + tmp, ye + tmp);
        if (_spi_dlen == 16) {
          auto t = tmp >> 16;
          spi_w0_reg[1] = (t & 0xFF) << 8 | (t >> 8) << 24;
          tmp = (tmp & 0xFF) << 8 | (tmp >> 8) << 24;
        }
        dc_h();
        *spi_w0_reg = tmp;
        *spi_mosi_dlen_reg = len;
        *spi_cmd_reg = SPI_USR;
        _ys = ys;
        _ye = ye;
      }
//*/
    }

    void start_read(void) {
      if (_spi_dlen == 16 && (_align_data)) write_data(0, 8);

      _fill_mode = false;
      uint32_t user = ((_panel->spi_mode_read == 1 || _panel->spi_mode_read == 2) ? SPI_CK_OUT_EDGE | SPI_USR_MISO : SPI_USR_MISO)
                    | (_panel->spi_3wire ? SPI_SIO : 0);
      uint32_t pin = (_panel->spi_mode_read & 2) ? SPI_CK_IDLE_EDGE : 0;
      dc_h();
      *_spi_user_reg = user;
      *reg(SPI_PIN_REG(_spi_port)) = pin;
      set_clock_read();
    }

    void end_read(bool cs_ctrl = true)
    {
      uint32_t pin = (_panel->spi_mode & 2) ? SPI_CK_IDLE_EDGE : 0;
      wait_spi();
      if (cs_ctrl) { cs_h(); }
      *_spi_user_reg = _user;
      *reg(SPI_PIN_REG(_spi_port)) = pin;
      if (cs_ctrl && _panel->spi_cs < 0) {
        write_cmd(0); // NOP command
      }
      set_clock_write();
      _fill_mode = false;

      if (cs_ctrl) { cs_l(); }
    }

    uint32_t read_data(uint32_t length)
    {
      set_read_len(length);
      exec_spi();
      wait_spi();
      return *_spi_w0_reg;

    }

    uint32_t read_command(uint_fast8_t command, uint32_t bitindex = 0, uint32_t bitlen = 8)
    {
      bitindex += _panel->len_dummy_read_rddid;
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
      if (_panel->fp_pushImage != nullptr)
      {
        _panel->fp_pushImage(_panel, this, x, y, w, h, param);
      }
      else
      {
        push_image(x, y, w, h, param, use_dma);
      }
    }

    void push_image(int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param, bool use_dma)
    {
      auto bits = _write_conv.bits;
      auto src_x = param->src_x;
      auto fp_copy = param->fp_copy;

      int32_t xr = (x + w) - 1;
      int32_t whb = w * h * bits >> 3;
      if (param->transp == ~0) {
        if (param->no_convert) {
          setWindow_impl(x, y, xr, y + h - 1);
          uint32_t i = (src_x + param->src_y * param->src_bitwidth) * bits >> 3;
          auto src = &((const uint8_t*)param->src_data)[i];
          if (_dma_channel && use_dma) {
            if (param->src_bitwidth == w) {
              _setup_dma_desc_links(src, w * h * bits >> 3);
            } else {
              _setup_dma_desc_links(src, w * bits >> 3, h, param->src_bitwidth * bits >> 3);
            }
            dc_h();
            set_write_len(whb << 3);
            *_spi_dma_out_link_reg = SPI_OUTLINK_START | ((int)(&_dmadesc[0]) & 0xFFFFF);
            spi_dma_transfer_active(_dma_channel);
            exec_spi();
            return;
          }
          if (param->src_bitwidth == w || h == 1) {
            if (_dma_channel && !use_dma && (64 < whb) && (whb <= 1024)) {
              auto buf = get_dmabuffer(whb);
              memcpy(buf, src, whb);
              write_bytes(buf, whb, true);
            } else {
              write_bytes(src, whb, use_dma);
            }
          } else {
            auto add = param->src_bitwidth * bits >> 3;
            do {
              write_bytes(src, w * bits >> 3, use_dma);
              src += add;
            } while (--h);
          }
        } else
        if (_dma_channel && (64 < whb)) {
          if (param->src_bitwidth == w && (whb <= 1024)) {
            auto buf = get_dmabuffer(whb);
            fp_copy(buf, 0, w * h, param);
            setWindow_impl(x, y, xr, y + h - 1);
            write_bytes(buf, whb, true);
          } else {
            int32_t wb = w * bits >> 3;
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
            auto buf = get_dmabuffer(w * bits >> 3);
            int32_t len = fp_copy(buf, 0, w - i, param);
            setWindow_impl(x + i, y, x + i + len - 1, y);
            write_bytes(buf, len * bits >> 3, true);
            if (w == (i += len)) break;
          }
          param->src_x = src_x;
          param->src_y++;
        } while (++y != h);
      }
    }

    void writePixels_impl(int32_t length, pixelcopy_t* param) override
    {
      if (!_panel->fp_writePixels)
      {
        if (_dma_channel)
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
        else
        {
          write_pixels(length, param);
        }
      }
      else
      {
        _panel->fp_writePixels(_panel, this, length, param);
      }
    }

    void write_pixels(int32_t length, pixelcopy_t* param)
    {
      const uint8_t bytes = _write_conv.bytes;
      const uint32_t limit = (bytes == 2) ? 16 : 10; //  limit = 32/bytes (bytes==2 is 16   bytes==3 is 10)
      uint32_t len = (length - 1) / limit;
      uint32_t highpart = (len & 1) << 3;
      len = length - (len * limit);
      uint32_t regbuf[8];
      param->fp_copy(regbuf, 0, len, param);

      auto spi_w0_reg = _spi_w0_reg;

      uint32_t user_reg = *_spi_user_reg;

      dc_h();
      set_write_len(len * bytes << 3);

      memcpy((void*)&spi_w0_reg[highpart], regbuf, (len * bytes + 3) & (~3));
      if (highpart) *_spi_user_reg = user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      for (; length; length -= limit) {
        param->fp_copy(regbuf, 0, limit, param);
        memcpy((void*)&spi_w0_reg[highpart ^= 0x08], regbuf, limit * bytes);
        uint32_t user = user_reg;
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
        auto spi_w0_reg = _spi_w0_reg;
        dc_h();
        set_write_len(length << 3);
        memcpy((void*)spi_w0_reg, data, (length + 3) & (~3));
        exec_spi();
        return;
      } else if (_dma_channel && use_dma) {
        dc_h();
        set_write_len(length << 3);
        _setup_dma_desc_links(data, length);
        *_spi_dma_out_link_reg = SPI_OUTLINK_START | ((int)(&_dmadesc[0]) & 0xFFFFF);
        spi_dma_transfer_active(_dma_channel);
        exec_spi();
        return;
      }
      constexpr uint32_t limit = 32;
      uint32_t len = ((length - 1) & 0x1F) + 1;
      uint32_t highpart = ((length - 1) & limit) >> 2; // 8 or 0

      auto spi_w0_reg = _spi_w0_reg;

      uint32_t user_reg = *_spi_user_reg;
      dc_h();
      set_write_len(len << 3);

      memcpy((void*)&spi_w0_reg[highpart], data, (len + 3) & (~3));
      if (highpart) *_spi_user_reg = user_reg | SPI_USR_MOSI_HIGHPART;
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
      if (!_panel->fp_readRect)
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
            set_read_len(len_dummy_read_pixel);
            exec_spi();
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
      else
      {
        _panel->fp_readRect(_panel, this, x, y, w, h, dst, param);
      }
    }

    void read_pixels(void* dst, int32_t length, pixelcopy_t* param)
    {
      int32_t len1 = std::min(length, 10); // 10 pixel read
      int32_t len2 = len1;
      auto len_read_pixel  = _read_conv.bits;
      uint32_t regbuf[8];
      wait_spi();
      set_read_len(len_read_pixel * len1);
      exec_spi();
      param->src_data = regbuf;
      int32_t dstindex = 0;
      uint32_t highpart = 8;
      uint32_t userreg = *_spi_user_reg;
      auto spi_w0_reg = _spi_w0_reg;
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
        memcpy(regbuf, (void*)&spi_w0_reg[highpart ^= 8], len2 * len_read_pixel >> 3);
        param->src_x = 0;
        dstindex = param->fp_copy(dst, dstindex, dstindex + len2, param);
      } while (length);
    }

    void read_bytes(uint8_t* dst, int32_t length, bool use_dma = false)
    {
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
        uint32_t userreg = *_spi_user_reg;
        auto spi_w0_reg = _spi_w0_reg;
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
          memcpy(dst, (void*)&spi_w0_reg[highpart ^= 8], len2);
          dst += len2;
        } while (length);
      }
//*/
    }

    static void _alloc_dmadesc(size_t len)
    {
      if (_dmadesc) heap_caps_free(_dmadesc);
      _dmadesc_len = len;
      _dmadesc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t) * len, MALLOC_CAP_DMA);
    }

    void spi_dma_reset()
    {
#if defined( CONFIG_IDF_TARGET_ESP32S2 )
      if (_spi_host == SPI2_HOST)
      {
        periph_module_reset( PERIPH_SPI2_DMA_MODULE );
      }
      else if (_spi_host == SPI3_HOST)
      {
        periph_module_reset( PERIPH_SPI3_DMA_MODULE );
      }
#else
      periph_module_reset( PERIPH_SPI_DMA_MODULE );
#endif
      _next_dma_reset = false;
    }

    void _setup_dma_desc_links(const uint8_t *data, int32_t len)
    {          //spicommon_setup_dma_desc_links
      if (!_dma_channel) return;

      if (_next_dma_reset) {
        spi_dma_reset();
      }
      if (_dmadesc_len * SPI_MAX_DMA_LEN < len) {
        _alloc_dmadesc(len / SPI_MAX_DMA_LEN + 1);
      }
      lldesc_t *dmadesc = _dmadesc;

      while (len > SPI_MAX_DMA_LEN) {
        len -= SPI_MAX_DMA_LEN;
        dmadesc->buf = (uint8_t *)data;
        data += SPI_MAX_DMA_LEN;
        *(uint32_t*)dmadesc = SPI_MAX_DMA_LEN | SPI_MAX_DMA_LEN<<12 | 0x80000000;
        dmadesc->qe.stqe_next = dmadesc + 1;
        dmadesc++;
      }
      *(uint32_t*)dmadesc = ((len + 3) & ( ~3 )) | len << 12 | 0xC0000000;
      dmadesc->buf = (uint8_t *)data;
      dmadesc->qe.stqe_next = nullptr;
    }

    void _setup_dma_desc_links(const uint8_t *data, int32_t w, int32_t h, int32_t width)
    {          //spicommon_setup_dma_desc_links
      if (!_dma_channel) return;

      if (_next_dma_reset) {
        spi_dma_reset();
      }
      if (_dmadesc_len < h) {
        _alloc_dmadesc(h);
      }
      lldesc_t *dmadesc = _dmadesc;
      int32_t idx = 0;
      do {
        dmadesc[idx].buf = (uint8_t *)data;
        data += width;
        *(uint32_t*)(&dmadesc[idx]) = ((w + 3) & (~3)) | w<<12 | 0x80000000;
        dmadesc[idx].qe.stqe_next = &dmadesc[idx + 1];
      } while (++idx < h);
      --idx;
      dmadesc[idx].eof = 1;
//    *(uint32_t*)(&dmadesc[idx]) |= 0xC0000000;
      dmadesc[idx].qe.stqe_next = 0;
    }

    void _setup_dma_desc_links(uint8_t** data, int32_t w, int32_t h, bool endless)
    {          //spicommon_setup_dma_desc_links
      if (!_dma_channel) return;

      if (_next_dma_reset) {
        spi_dma_reset();
      }

      if (_dmadesc_len < h) {
        _alloc_dmadesc(h);
      }

      lldesc_t *dmadesc = _dmadesc;
      int32_t idx = 0;
      do {
        dmadesc[idx].buf = (uint8_t *)data[idx];
        *(uint32_t*)(&dmadesc[idx]) = w | w<<12 | 0x80000000;
        dmadesc[idx].qe.stqe_next = &dmadesc[idx + 1];
      } while (++idx < h);
      --idx;
      if (endless) {
        dmadesc[idx].qe.stqe_next = &dmadesc[0];
      } else {
        dmadesc[idx].eof = 1;
//        *(uint32_t*)(&dmadesc[idx]) |= 0xC0000000;
        dmadesc[idx].qe.stqe_next = 0;
      }
    }

    __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }
    __attribute__ ((always_inline)) inline void set_clock_write(void) { *reg(SPI_CLOCK_REG(_spi_port)) = _clkdiv_write; }
    __attribute__ ((always_inline)) inline void set_clock_read(void)  { *reg(SPI_CLOCK_REG(_spi_port)) = _clkdiv_read;  }
    __attribute__ ((always_inline)) inline void set_clock_fill(void)  { *reg(SPI_CLOCK_REG(_spi_port)) = _clkdiv_fill;  }
    __attribute__ ((always_inline)) inline void exec_spi(void) {        *_spi_cmd_reg = SPI_USR; }
    __attribute__ ((always_inline)) inline void wait_spi(void) { while (*_spi_cmd_reg & SPI_USR); }
    __attribute__ ((always_inline)) inline void set_write_len(uint32_t bitlen) { *_spi_mosi_dlen_reg = bitlen - 1; }
    __attribute__ ((always_inline)) inline void set_read_len( uint32_t bitlen) { *reg(SPI_MISO_DLEN_REG(_spi_port)) = bitlen - 1; }

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

/*
    __attribute__ ((always_inline)) inline void cs_h(void) { *_gpio_reg_cs_h = _mask_reg_cs; }
    __attribute__ ((always_inline)) inline void cs_l(void) { *_gpio_reg_cs_l = _mask_reg_cs; }
//
    __attribute__ ((always_inline)) inline void cs_h(void) { if (_mask_reg_cs) *_gpio_reg_cs_h = _mask_reg_cs; else cs_h_impl(); }
    __attribute__ ((always_inline)) inline void cs_l(void) { if (_mask_reg_cs) *_gpio_reg_cs_l = _mask_reg_cs; else cs_l_impl(); }
    virtual void cs_h_impl(void) {}
    virtual void cs_l_impl(void) {}
//*/

    int _spi_mosi = get_spi_mosi<CFG, -1>::value;
    int _spi_miso = get_spi_miso<CFG, -1>::value;
    int _spi_sclk = get_spi_sclk<CFG, -1>::value;
    int _spi_dlen = get_spi_dlen<CFG,  8>::value;
    spi_host_device_t _spi_host;
    static constexpr int _dma_channel= get_dma_channel<CFG,  0>::value;
    /*
    uint32_t(*fpGetWindowAddr)(uint_fast16_t, uint_fast16_t);
    uint_fast16_t _colstart;
    uint_fast16_t _rowstart;
    uint_fast16_t _xs;
    uint_fast16_t _xe;
    uint_fast16_t _ys;
    uint_fast16_t _ye;
    uint32_t _cmd_caset;
    uint32_t _cmd_raset;
    //*/
    uint32_t _cmd_ramwr;

  private:
    uint32_t _last_apb_freq;
    uint32_t _clkdiv_write;
    uint32_t _clkdiv_read;
    uint32_t _clkdiv_fill;
    uint32_t _len_setwindow;
    bool _fill_mode;
    bool _align_data = false;
    uint32_t _mask_reg_dc;
    volatile uint32_t* _gpio_reg_dc_h;
    volatile uint32_t* _gpio_reg_dc_l;
    volatile uint32_t* _spi_w0_reg;
    volatile uint32_t* _spi_cmd_reg;
    volatile uint32_t* _spi_user_reg;
    volatile uint32_t* _spi_mosi_dlen_reg;
    volatile uint32_t* _spi_dma_out_link_reg;
    uint32_t _user;
    uint8_t _spi_port;
    static lldesc_t* _dmadesc;
    static uint32_t _dmadesc_len;
    static bool _next_dma_reset;

  };
  template <class T> lldesc_t* LGFX_SPI<T>::_dmadesc = nullptr;
  template <class T> uint32_t LGFX_SPI<T>::_dmadesc_len = 0;
  template <class T> bool LGFX_SPI<T>::_next_dma_reset;

//----------------------------------------------------------------------------
 }
}

using lgfx::LGFX_SPI;

#endif
