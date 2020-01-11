#ifndef LGFX_SPI_ESP32_HPP_
#define LGFX_SPI_ESP32_HPP_

#include <type_traits>
#include <esp_heap_caps.h>

#include "lgfx_common.hpp"
#include "esp32_common.hpp"
#include "lgfx_sprite.hpp"

namespace lgfx
{
  #define MEMBER_DETECTOR(member, classname, classname_impl, valuetype) struct classname_impl { \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, T::member> check(decltype(T::member)*); \
  template<class T, valuetype V> static constexpr std::integral_constant<valuetype, V> check(...); \
  };template<class T, valuetype V> class classname : public decltype(classname_impl::check<T, V>(nullptr)) {};
  MEMBER_DETECTOR(freq_fill, get_freq_fill, get_freq_fill_impl, int)
  MEMBER_DETECTOR(freq_read, get_freq_read, get_freq_read_impl, int)
  MEMBER_DETECTOR(freq_write,get_freq_write,get_freq_write_impl,int)
  MEMBER_DETECTOR(spi_mode,  get_spi_mode,  get_spi_mode_impl,  int)
  MEMBER_DETECTOR(spi_mosi,  get_spi_mosi,  get_spi_mosi_impl,  int)
  MEMBER_DETECTOR(spi_miso,  get_spi_miso,  get_spi_miso_impl,  int)
  MEMBER_DETECTOR(spi_sclk,  get_spi_sclk,  get_spi_sclk_impl,  int)
  MEMBER_DETECTOR(spi_cs,    get_spi_cs,    get_spi_cs_impl,    int)
  MEMBER_DETECTOR(spi_dc,    get_spi_dc,    get_spi_dc_impl,    int)
  MEMBER_DETECTOR(spi_host,  get_spi_host,  get_spi_host_impl, spi_host_device_t)
  MEMBER_DETECTOR(spi_half_duplex, get_spi_half_duplex, get_spi_half_duplex_impl, bool)
  MEMBER_DETECTOR(panel_rst, get_panel_rst, get_panel_rst_impl, int)
  MEMBER_DETECTOR(rgb_order, get_rgb_order, get_rgb_order_impl, bool)
  MEMBER_DETECTOR(bpp,       get_bpp,       get_bpp_impl,       uint8_t)
  MEMBER_DETECTOR(invert,    get_invert,    get_invert_impl,    bool)
  MEMBER_DETECTOR(rotation,  get_rotation,  get_rotation_impl,  uint8_t)
  MEMBER_DETECTOR(ram_width,    get_ram_width,    get_ram_width_impl,    int)
  MEMBER_DETECTOR(ram_height,   get_ram_height,   get_ram_height_impl,   int)
  MEMBER_DETECTOR(panel_x,      get_panel_x,      get_panel_x_impl,     int)
  MEMBER_DETECTOR(panel_y,      get_panel_y,      get_panel_y_impl,     int)
  MEMBER_DETECTOR(panel_width,  get_panel_width,  get_panel_width_impl,  int)
  MEMBER_DETECTOR(panel_height, get_panel_height, get_panel_height_impl, int)
  #undef MEMBER_DETECTOR

  template <class CFG>
  class LGFX_SPI : public LovyanGFX
  {
  public:

    LGFX_SPI() : LovyanGFX() {
      _color.setColorDepth((color_depth_t)get_bpp<CFG, 16>::value);
      _readColorDepth = rgb888_3Byte;
      _invert   = get_invert  <CFG,false>::value;
      _rotation = get_rotation<CFG,    0>::value;
    }

    virtual ~LGFX_SPI() {}
/*
    static void setPanel(const PanelLcdCommon* panel)
    {
      _panel = panel;
      _panel_cmd_caset = panel->cmd_caset;
      _panel_cmd_raset = panel->cmd_raset;
      _panel_len_command = panel->len_command;
    }
*/
    void init_impl(void) override
    {
      init_bus();
      init_panel();
      LovyanGFX::init_impl();
    }

    void beginTransaction_impl(void) override {
      uint32_t apb_freq = getApbFrequency();
      if (_last_apb_freq != apb_freq) {
        _last_apb_freq = apb_freq;
        _clkdiv_write = spiFrequencyToClockDiv(_freq_write);
        _clkdiv_read  = spiFrequencyToClockDiv(_freq_read);
        _clkdiv_fill  = spiFrequencyToClockDiv(_freq_fill);
      }
      fill_mode = false;
      wait_spi();
      cs_l();
      *_spi_miso_dlen_reg = 0;
      set_clock_write();
      _user_reg = *_spi_user_reg & ~(SPI_USR_MISO | SPI_DOUTDIN);
      {//spiSetDataMode(spi, dataMode);
        if (_spi_mode == 1 || _spi_mode == 2) {
          _user_reg |= SPI_CK_OUT_EDGE;
        } else {
          _user_reg &= ~SPI_CK_OUT_EDGE;
        }
        if (_spi_mode == 2 || _spi_mode == 3) {
          *reg(SPI_PIN_REG(_spi_port)) |= SPI_CK_IDLE_EDGE;
        } else {
          *reg(SPI_PIN_REG(_spi_port)) &= ~SPI_CK_IDLE_EDGE;
        }
      }
      *_spi_user_reg = _user_reg;
      // MSB first
      *reg(SPI_CTRL_REG(_spi_port)) &= ~(SPI_WR_BIT_ORDER | SPI_RD_BIT_ORDER);
    }

    void endTransaction_impl(void) override {
      wait_spi();
      cs_h();
      *_spi_user_reg = _user_reg | SPI_USR_MISO | SPI_DOUTDIN;
    }

    void setWindow_impl(int32_t xs, int32_t ys, int32_t xe, int32_t ye) override
    {
      set_window(xs, ys, xe, ye);
      if (_freq_write != _freq_fill && fill_mode) {
        wait_spi();
        set_clock_write();
        fill_mode = false;
      }
      write_cmd(_cmd_ramwr);
    }

    void drawPixel_impl(int32_t x, int32_t y) override
    {
      if (!_start_write_count) beginTransaction_impl();
      set_window(x, y, x, y);
      if (_freq_write != _freq_fill && !fill_mode) {
        wait_spi();
        set_clock_fill();
        fill_mode = true;
      }
      write_cmd(_cmd_ramwr);
      write_data(_color.raw, _color.bits);
      if (!_start_write_count) endTransaction_impl();
    }

    void fillRect_impl(int32_t x, int32_t y, int32_t w, int32_t h) override
    {
      if (!_start_write_count) beginTransaction_impl();
      set_window(x, y, x+w-1, y+h-1);
      if (_freq_write != _freq_fill && !fill_mode) {
        wait_spi();
        set_clock_fill();
        fill_mode = true;
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
      rgb565_t res = (rgb565_t)swap888_t(read_data(24));
      end_read();
      endWrite();
      return res;
    }

//----------------------------------------------------------------------------
  protected:
    virtual const uint8_t* getInitCommands(uint8_t listno = 0) const { return nullptr; }
    __attribute__ ((always_inline)) inline static uint32_t getWindowAddr(uint16_t H, uint16_t L) { return ((H)<<8 | (H)>>8) | (((L)<<8 | (L)>>8)<<16 ); }

    static constexpr uint8_t CMD_INIT_DELAY = 0x80;

    bool fill_mode;
    uint32_t _cmd_caset;
    uint32_t _cmd_raset;
    uint32_t _cmd_ramrd;
    uint32_t _cmd_ramwr;
    uint32_t _len_command = 8;
    uint32_t _len_read_pixel = 24;
    uint32_t _len_dummy_read_pixel = 8;
    uint32_t _len_dummy_read_rddid = 0;
    uint32_t _colstart;
    uint32_t _rowstart;
    uint32_t _last_xs;
    uint32_t _last_xe;
    uint32_t _last_ys;
    uint32_t _last_ye;

    static void init_bus(void)
    {
      TPin<_spi_cs>::init();
      TPin<_spi_dc>::init();
      TPin<_spi_mosi>::init(GPIO_MODE_INPUT_OUTPUT);
      TPin<_spi_miso>::init(GPIO_MODE_INPUT_OUTPUT);
      TPin<_spi_sclk>::init();
      TPin<_panel_rst>::init();
      TPin<_panel_rst>::lo();
      cs_h();

      if (_spi_host == HSPI_HOST) {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI_CLK_EN);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI_RST);
      } else {
        DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_SPI_CLK_EN_2);
        DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_SPI_RST_2);
      }

      {//spiStopBus;
        *reg(SPI_SLAVE_REG(_spi_port)) &= ~(SPI_SLAVE_MODE | SPI_TRANS_DONE);
        *reg(SPI_PIN_REG  (_spi_port)) = 0;
        *reg(SPI_USER_REG (_spi_port)) = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN;
        *reg(SPI_USER1_REG(_spi_port)) = 0;
        *reg(SPI_CTRL_REG (_spi_port)) = 0;
        *reg(SPI_CTRL1_REG(_spi_port)) = 0;
        *reg(SPI_CTRL2_REG(_spi_port)) = 0;
        *reg(SPI_CLOCK_REG(_spi_port)) = 0;
      }

      if (_spi_sclk >= 0) { pinMatrixOutAttach(_spi_sclk, (_spi_host == HSPI_HOST) ? HSPICLK_OUT_IDX : VSPICLK_OUT_IDX, false, false); }
      if (_spi_mosi >= 0) { pinMatrixOutAttach(_spi_mosi, (_spi_host == HSPI_HOST) ? HSPID_IN_IDX : VSPID_IN_IDX, false, false); }
      if (_spi_miso >= 0) { pinMatrixInAttach( _spi_miso, (_spi_host == HSPI_HOST) ? HSPIQ_OUT_IDX : VSPIQ_OUT_IDX, false); }

      TPin<_panel_rst>::hi();
    }

    void init_panel(void)
    {
      const uint8_t *cmds;
      for (uint8_t i = 0; (cmds = getInitCommands(i)); i++) {
        delay(120);
        beginTransaction_impl();
        commandList(cmds);
        endTransaction_impl();
      }
      _last_xs = _last_xe = _last_ys = _last_ye = 0xFFFF;
    }

    bool commandList(const uint8_t *addr)
    {
      if (addr == nullptr) return false;
      uint8_t  cmd;
      uint8_t  numArgs;
      uint8_t  ms;

      wait_spi();
      set_clock_write();
      for (;;) {                // For each command...
        cmd     = pgm_read_byte(addr++);  // Read, issue command
        numArgs = pgm_read_byte(addr++);  // Number of args to follow
        if (0xFF == cmd && 0xFF == numArgs) break;
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

    virtual void set_window(uint32_t xs, uint32_t ys, uint32_t xe, uint32_t ye)
    {
      if (_last_xs != xs || _last_xe != xe) {
        write_cmd(_cmd_caset);
        _last_xs = xs;
        _last_xe = xe;
        write_data(getWindowAddr(xs += _colstart, xe += _colstart), 32);
      }
      if (_last_ys != ys || _last_ye != ye) {
        write_cmd(_cmd_raset);
        _last_ys = ys;
        _last_ye = ye;
        write_data(getWindowAddr(ys += _rowstart, ye += _rowstart), 32);
      }
    }

    void start_read(void) {
      *_spi_user_reg = _user_reg | SPI_USR_MISO | SPI_DOUTDIN;
      fill_mode = false;
      wait_spi();
      set_clock_read();
      dc_h();
      if (_spi_half_duplex) {
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
      fill_mode = false;
      if (_spi_half_duplex) {
        if (_spi_miso != -1) {
          pinMatrixInAttach(_spi_miso, (_spi_host == HSPI_HOST) ? HSPIQ_OUT_IDX : VSPIQ_OUT_IDX, false);
        }
        TPin<_spi_mosi>::enableOutput();
        pinMatrixOutAttach(_spi_mosi, (_spi_host == HSPI_HOST) ? HSPID_IN_IDX : VSPID_IN_IDX, false, false);
      }
      wait_spi();
      cs_l();
    }

    inline void write_cmd(uint32_t cmd)
    {
      wait_spi();
      *_spi_w0_reg = cmd;
      set_len(_len_command);
      dc_l();
      exec_spi();
    }

    inline static void write_data(uint32_t data, uint32_t length)
    {
      wait_spi();
      *_spi_w0_reg = data;
      set_len(length);
      dc_h();
      exec_spi();
    }

    inline static uint32_t read_data(uint32_t length)
    {
      set_len(length);
      exec_spi();
      wait_spi();
      return *_spi_w0_reg;
    }

    void write_pixels(const void* src, int32_t length, void(*fp_copy)(void*&, const void*&, uint32_t)) override
    {
      if (!length) return;
      const uint8_t bytes = _color.bytes;
      const uint32_t limit = (bytes == 2) ? 16 : 10; //  limit = 32/bytes (bytes==2 is 16   bytes==3 is 10)
      uint32_t len = (length - 1) / limit;
      uint8_t highpart = (len & 1) << 3;
      len = length - (len * limit);
      void* dst = _regbuf;
      fp_copy(dst, src, 0);
      fp_copy(dst, src, len);
      wait_spi();
      dc_h();
      set_len(len * bytes << 3);
      memcpy((void*)&_spi_w0_reg[highpart], _regbuf, (len * bytes + 3) & 0xFC);
      if (highpart) *_spi_user_reg = _user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      for (; length; length -= limit) {
        dst = _regbuf;
        fp_copy(dst, src, limit);
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

    void read_pixels(void* dst, int32_t length, void(*fp_copy)(void*&, const void* &src, uint32_t)) override
    {
      if (!length) return;
      const void* src;
      fp_copy(dst, src, 0);
      uint32_t len(length & 7);
      length >>= 3;
      wait_spi();
      if (length) {
        set_len(_len_read_pixel << 3); // 8pixel read
        exec_spi();
        while (--length) {
          wait_spi();
          memcpy(_regbuf, (void*)_spi_w0_reg, 24);
          exec_spi();
          src = _regbuf;
          fp_copy(dst, src, 8);
        }
        wait_spi();
        memcpy(_regbuf, (void*)_spi_w0_reg, 24);
        if (len) {
          set_len(_len_read_pixel * len);
          exec_spi();
        }
        src = _regbuf;
        fp_copy(dst, src, 8);
        if (!len) return;
      } else {
        set_len(_len_read_pixel * len);
        exec_spi();
      }
      wait_spi();
      memcpy(_regbuf, (void*)_spi_w0_reg, 3 * len);
      src = _regbuf;
      fp_copy(dst, src, len);
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

    __attribute__ ((always_inline)) inline static volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }
    __attribute__ ((always_inline)) inline static void set_clock_write(void) { *_spi_clock_reg = _clkdiv_write; }
    __attribute__ ((always_inline)) inline static void set_clock_read(void)  { *_spi_clock_reg = _clkdiv_read;  }
    __attribute__ ((always_inline)) inline static void set_clock_fill(void)  { *_spi_clock_reg = _clkdiv_fill;  }
    __attribute__ ((always_inline)) inline static void exec_spi(void) { *_spi_cmd_reg = SPI_USR; }
    __attribute__ ((always_inline)) inline static void wait_spi(void) { while (*_spi_cmd_reg & SPI_USR); }
    __attribute__ ((always_inline)) inline static void set_len(uint32_t len) { *_spi_mosi_dlen_reg = len - 1; }

    __attribute__ ((always_inline)) inline static void cs_h(void) { TPin<_spi_cs>::hi(); }
    __attribute__ ((always_inline)) inline static void cs_l(void) { TPin<_spi_cs>::lo(); }
    __attribute__ ((always_inline)) inline static void dc_h(void) { TPin<_spi_dc>::hi(); }
    __attribute__ ((always_inline)) inline static void dc_l(void) { TPin<_spi_dc>::lo(); }

    static constexpr int _spi_mode = get_spi_mode<CFG,  0>::value;
    static constexpr int _spi_mosi = get_spi_mosi<CFG, 23>::value;
    static constexpr int _spi_miso = get_spi_miso<CFG, 19>::value;
    static constexpr int _spi_sclk = get_spi_sclk<CFG, 18>::value;
    static constexpr int _spi_cs   = get_spi_cs<  CFG, -1>::value;
    static constexpr int _spi_dc   = get_spi_dc<  CFG, -1>::value;
    static constexpr int _panel_rst= get_panel_rst< CFG, -1>::value;
    static constexpr int _rgb_order= get_rgb_order< CFG, false>::value;
    static constexpr bool _spi_half_duplex = get_spi_half_duplex<CFG, false>::value;
    static constexpr int _freq_write = get_freq_write<CFG, 40000000>::value;
    static constexpr int _freq_read  = get_freq_read< CFG, 16000000>::value;
    static constexpr int _freq_fill  = get_freq_fill< CFG, _freq_write>::value;
    static constexpr spi_host_device_t _spi_host = get_spi_host<CFG, VSPI_HOST>::value;

    static constexpr uint8_t _spi_port = (_spi_host == HSPI_HOST) ? 2 : 3;  // FSPI=1  HSPI=2  VSPI=3;
    static constexpr volatile uint32_t *_spi_w0_reg        = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_W0_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_cmd_reg       = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_CMD_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_user_reg      = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_USER_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_clock_reg     = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_CLOCK_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_mosi_dlen_reg = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_MOSI_DLEN_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_miso_dlen_reg = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_MISO_DLEN_REG(_spi_port));

    static uint32_t _last_apb_freq;
    static uint32_t _clkdiv_write;
    static uint32_t _clkdiv_read;
    static uint32_t _clkdiv_fill;
    static uint32_t _user_reg;
    static uint32_t _regbuf[8];
    //static uint32_t _palette_bits;
    //static uint32_t _palette_mask;
  };
  template <class T> uint32_t LGFX_SPI<T>::_last_apb_freq;
  template <class T> uint32_t LGFX_SPI<T>::_clkdiv_write;
  template <class T> uint32_t LGFX_SPI<T>::_clkdiv_read;
  template <class T> uint32_t LGFX_SPI<T>::_clkdiv_fill;
  template <class T> uint32_t LGFX_SPI<T>::_user_reg;
  template <class T> uint32_t LGFX_SPI<T>::_regbuf[];
  //template <class T> uint32_t LGFX_SPI<T>::_palette_bits;
  //template <class T> uint32_t LGFX_SPI<T>::_palette_mask;

//----------------------------------------------------------------------------

}
#endif
