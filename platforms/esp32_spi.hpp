#ifndef LGFX_ESP32_SPI_HPP_
#define LGFX_ESP32_SPI_HPP_

// #pragma GCC optimize ("O3")

#include <type_traits>

#include "lgfx_common.hpp"
#include "esp32_common.hpp"
#include "panel_lcd_common.hpp"

/*
struct LGFX_CONFIG_SAMPLE
{
  static constexpr spi_host_device_t spi_host = VSPI_HOST; //  VSPI_HOST or HSPI_HOST
  static constexpr int spi_mode =  0;
  static constexpr int spi_mosi = 23;
  static constexpr int spi_miso = 19;
  static constexpr int spi_sclk = 18;
  static constexpr int spi_cs   = -1;
  static constexpr int spi_dc   = -1;
  static constexpr bool spi_half_duplex = false;
  static constexpr int panel_rst = -1;
  static constexpr bool rgb_order = true;   // true=RGB / false=BGR
  static constexpr int freq_write = 20000000;
  static constexpr int freq_read  = 10000000;
  static constexpr int freq_fill  = 20000000;
};
*/
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
  #undef MEMBER_DETECTOR

  template <class CFG>
  class Esp32Spi
  {
  public:
#define TYPECHECK(dType) template < typename tType, typename std::enable_if < (sizeof(tType) == sizeof(dType)) && (std::is_signed<tType>::value == std::is_signed<dType>::value), std::nullptr_t >::type=nullptr >
    TYPECHECK(uint8_t ) __attribute__ ((always_inline)) inline dev_color_t getDevColor(tType c) { return getDevColor(*(rgb332_t*)&c); }
    TYPECHECK(uint16_t) __attribute__ ((always_inline)) inline dev_color_t getDevColor(tType c) { return getDevColor(*(rgb565_t*)&c); }
    TYPECHECK(uint32_t) __attribute__ ((always_inline)) inline dev_color_t getDevColor(tType c) { return getDevColor(*(rgb888_t*)&c); }
    TYPECHECK(int     ) __attribute__ ((always_inline)) inline dev_color_t getDevColor(tType c) { return getDevColor(*(rgb565_t*)&c); }
#undef TYPECHECK
    __attribute__ ((always_inline)) inline dev_color_t getDevColor(const rgb888_t& c) { return (_bpp == 16) ? swap565( c.r, c.g, c.b) : (c.r | c.g<<8 | c.b<<16); }
    __attribute__ ((always_inline)) inline dev_color_t getDevColor(const rgb565_t& c) { return (_bpp == 16) ? (c.raw << 8 | c.raw >> 8) : (c.R8() | (c.G8()<<8) | (c.B8()<<16)); }
    __attribute__ ((always_inline)) inline dev_color_t getDevColor(const rgb332_t& c) { return (_bpp == 16) ? swap565(c.R8(), c.G8(), c.B8()) : swap888(c.R8(), c.G8(), c.B8()); }

    Esp32Spi() {
      _panel = &_cfg.panel;

      _bpp      = get_bpp     <CFG,   16>::value;
      _invert   = get_invert  <CFG,false>::value;
      _rotation = get_rotation<CFG,    0>::value;
    }

    virtual ~Esp32Spi() {}

    static void setPanel(const PanelLcdCommon* panel)
    {
      _panel = panel;
      _panel_cmd_caset = panel->cmd_caset;
      _panel_cmd_raset = panel->cmd_raset;
      _panel_len_command = panel->len_command;
    }

    static void init(void)
    {
      init_bus();

      setPanel(_panel);

      init_panel();
    }

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

    static void init_panel(void)
    {
      const uint8_t *cmds;
      for (uint8_t i = 0; (cmds = _panel->getInitCommands(i)); i++) {
        delay(120);
        beginTransaction();
        commandList(cmds);
        invertDisplay(_invert);
        setRotation(_rotation);
        endTransaction();
      }
      _last_xs = _last_xe = _last_ys = _last_ye = 0xFFFF;
    }

    static bool commandList(const uint8_t *addr)
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
        ms = numArgs & _panel->CMD_INIT_DELAY;       // If hibit set, delay follows args
        numArgs &= ~_panel->CMD_INIT_DELAY;          // Mask out delay bit

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

    static void beginTransaction(void) {
      uint32_t apb_freq = getApbFrequency();
      if (_last_apb_freq != apb_freq) {
        _last_apb_freq = apb_freq;
        _clkdiv_write = spiFrequencyToClockDiv(_freq_write);
        _clkdiv_read  = spiFrequencyToClockDiv(_freq_read);
        _clkdiv_fill  = spiFrequencyToClockDiv(_freq_fill);
      }
      _fillmode = false;
      wait_spi();
      cs_l();
      *_spi_miso_dlen_reg = 0;
      *_spi_clock_reg = _clkdiv_write;
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

    static void endTransaction(void) {
      wait_spi();
      cs_h();
      *_spi_user_reg = _user_reg | SPI_USR_MISO | SPI_DOUTDIN;
    }

    inline static int32_t width(void) { return _width; }
    inline static int32_t height(void) { return _height; }
    inline static bool    getInvert(void) { return _invert; }
    inline static uint8_t getRotation(void) { return _rotation; }
    inline static uint8_t getColorDepth(void) { return _bpp; }

    static void setRotation(uint8_t r)
    {
      write_cmd(_panel->cmd_madctl);
      r = r & 7;
      _rotation = r;
//Serial.printf("rotation:%d ", _rotation);

      auto rotation_data = _panel->getRotationData(r);
      _colstart = rotation_data->colstart;
      _rowstart = rotation_data->rowstart;
//Serial.printf("colstart:%d / rowstart:%d ", _colstart, _rowstart);
      uint8_t madctl = rotation_data->madctl | (_rgb_order ? _panel->madctl_rgb : _panel->madctl_bgr);
      write_data(madctl, 8);
      if (r & 1) {
        _width  = _panel->panel_height;
        _height = _panel->panel_width;
      } else {
        _width  = _panel->panel_width;
        _height = _panel->panel_height;
      }
//Serial.printf("width:%d / height:%d \r\n", _width, _height);
      _last_xs = _last_xe = _last_ys = _last_ye = 0xFFFF;
    }

    static void* setColorDepth(uint8_t bpp)  // 16 or 24
    {
      write_cmd(_panel->cmd_colmod);
      _bpp = _panel->getAdjustBpp(bpp);
      write_data(_panel->getColMod(_bpp), 8);
      return nullptr;
    }

    static void invertDisplay(bool i)
    { // Send the command twice as otherwise it does not always work!
      _invert = i;
      write_cmd(i ? _panel->cmd_invon : _panel->cmd_invoff);
      write_cmd(i ? _panel->cmd_invon : _panel->cmd_invoff);
    }

    static uint32_t readPanelID(void)
    {
      if (0 == _panel->cmd_rddid) return 0;
      //write_cmd(0xD9);
      //write_data(0x10, 8);
      write_cmd(_panel->cmd_rddid);
      startRead();
      if (_panel->len_dummy_read_rddid) read_data(_panel->len_dummy_read_rddid);
      uint32_t res = read_data(32);
      endRead();
      return res;
    }

    static uint32_t readPanelIDSub(uint8_t cmd)
    {
      //write_cmd(0xD9);
      //write_data(0x10, 8);
      write_cmd(cmd);
      startRead();
      uint32_t res = read_data(32);
      endRead();
      return res;
    }

    static void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      set_window(xs, ys, xe, ye);
      if (_freq_write != _freq_fill && _fillmode) {
        _fillmode = false;
        wait_spi();
        set_clock_write();
      }
      write_cmd(_panel->cmd_ramwr);
    }

    static void readWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      set_window(xs, ys, xe, ye);
      write_cmd(_panel->cmd_ramrd);
      if (_freq_write != _freq_fill) _fillmode = false;
      startRead();
      if (_panel->len_dummy_read_pixel) read_data(_panel->len_dummy_read_pixel);
    }

    static void startRead() {
      *_spi_user_reg = _user_reg | SPI_USR_MISO | SPI_DOUTDIN;
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

    static void endRead() {
      cs_h();
      set_clock_write();
      *_spi_user_reg = _user_reg;
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

    static void fillWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye, const dev_color_t& color)
    {
      set_window(xs, ys, xe, ye);
      if (_freq_write != _freq_fill && !_fillmode) {
        _fillmode = true;
        wait_spi();
        set_clock_fill();
      }
      write_cmd(_panel->cmd_ramwr);
      writeColor(color, (xe-xs+1) * (ye-ys+1));
    }

    static void writeColor(const dev_color_t& color, uint32_t length = 1)
    {
      if (length == 1) { write_data(color.raw, _bpp); return; }

      uint32_t len;
      if (_bpp == 16) {
        _regbuf[0] = color.raw | color.raw << 16;
        memcpy(&_regbuf[1], _regbuf, 4);
        memcpy(&_regbuf[2], _regbuf, 4);
        len = 6;
      } else { // _bpp == 24
        _regbuf[0] = color.raw;
        memcpy(&((uint8_t*)_regbuf)[3], _regbuf, 3);
        memcpy(&((uint8_t*)_regbuf)[6], _regbuf, 6);
        len = 4;
      }

      if (length < len) len = length;
      wait_spi();
      set_len(len * _bpp);
      dc_h();
      memcpy((void*)_spi_w0_reg, _regbuf, 12);
      exec_spi();
      if (0 == (length -= len)) return;

      memcpy((void*)&_regbuf[ 3], _regbuf, 12);
      memcpy((void*)&_regbuf[ 6], _regbuf,  4);
      memcpy((void*)&_spi_w0_reg[ 3], _regbuf, 24);
      memcpy((void*)&_spi_w0_reg[ 9], _regbuf, 28);

      const uint32_t limit = (_bpp == 16) ? 32 : 21; // limit = 512 / bpp;
      len = length % limit;
      if (len) {
        wait_spi();
        set_len(len * _bpp);
        exec_spi();
        if (0 == (length -= len)) return;
      }
      wait_spi();
      set_len(limit * _bpp);
      exec_spi();
      while (length -= limit) {
        wait_spi();
        exec_spi();
      }
    }

    __attribute__ ((always_inline)) inline static void writePixels(const rgb332_t*   src, uint32_t length) { writePixelsTemplate(src, length); }
    __attribute__ ((always_inline)) inline static void writePixels(const rgb565_t*   src, uint32_t length) { writePixelsTemplate(src, length); }
    __attribute__ ((always_inline)) inline static void writePixels(const rgb888_t*   src, uint32_t length) { writePixelsTemplate(src, length); }
    __attribute__ ((always_inline)) inline static void writePixels(const argb8888_t* src, uint32_t length) { writePixelsTemplate(src, length); }

    __attribute__ ((always_inline)) inline static void writePixels(const swap565_t*  src, uint32_t length)
    {
      if      (_bpp == 16) { writeBytes((const uint8_t*)src, length * 2); }
      else if (_bpp == 24) { write_pixels<swap888_t>(src, length); }
    }

    __attribute__ ((always_inline)) inline static void writePixels(const swap888_t* src, uint32_t length)
    {
      if      (_bpp == 16) { write_pixels<swap565_t>(src, length); }
      else if (_bpp == 24) { writeBytes((const uint8_t*)src, length * 3); }
    }

    // need data size = length ( no use bpp )
    static void writeBytes(const uint8_t* data, uint32_t length)
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

    static rgb565_t readPixel(void)
    {
      wait_spi();
      if (_panel->len_read_pixel == 24) return swap888_t(read_data(24));
    }

    __attribute__ ((always_inline)) inline static void readPixels(rgb332_t*   buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline static void readPixels(rgb565_t*   buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline static void readPixels(rgb888_t*   buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline static void readPixels(swap565_t*  buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline static void readPixels(swap888_t*  buf, uint32_t length) { readPixelsTemplate(buf, length); }
    __attribute__ ((always_inline)) inline static void readPixels(argb8888_t* buf, uint32_t length) { readPixelsTemplate(buf, length); }


//----------------------------------------------------------------------------
  private:

    static void set_window(uint32_t xs, uint32_t ys, uint32_t xe, uint32_t ye)
    {
      if (_last_xs != xs || _last_xe != xe) {
        write_cmd(_panel_cmd_caset);
        _last_xs = xs;
        _last_xe = xe;
        write_data(_panel->getWindowAddr(xs += _colstart, xe += _colstart), 32);
      }
      if (_last_ys != ys || _last_ye != ye) {
        write_cmd(_panel_cmd_raset);
        _last_ys = ys;
        _last_ye = ye;
        write_data(_panel->getWindowAddr(ys += _rowstart, ye += _rowstart), 32);
      }
    }

    inline static void write_cmd(uint32_t cmd)
    {
      wait_spi();
      *_spi_w0_reg = cmd;
      set_len(_panel_len_command);
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

    template<class T>
    __attribute__ ((always_inline)) inline static void writePixelsTemplate(const T* src, uint32_t length)
    {
      if (     _bpp == 16) { write_pixels<swap565_t>(src, length); }
      else if (_bpp == 24) { write_pixels<swap888_t>(src, length); }
    }
    template <class TDst, class TSrc>
    static void write_pixels(const TSrc* src, uint32_t length)
    {
      if (!length) return;
      constexpr uint32_t limit = (sizeof(TDst) == 2) ? 16 : 10;
      uint32_t len = (length - 1) / limit;
      uint8_t highpart = (len & 1) << 3;
      len = length - (len * limit);
      auto dst = (TDst*)_regbuf;
      for (uint32_t i = 0; i < len; i++) { *dst++ = *src++; }
      wait_spi();
      dc_h();
      set_len(len * sizeof(TDst) << 3);
      memcpy((void*)&_spi_w0_reg[highpart], _regbuf, (len * sizeof(TDst) + 3) & 0xFC);
      if (highpart) *_spi_user_reg = _user_reg | SPI_USR_MOSI_HIGHPART;
      exec_spi();
      if (0 == (length -= len)) return;

      for (; length; length -= limit) {
        dst = (TDst*)_regbuf;
        for (uint32_t i = 0; i < limit; i++) { *dst++ = *src++; }
        memcpy((void*)&_spi_w0_reg[highpart ^= 0x08], _regbuf, limit * sizeof(TDst));
        uint32_t user = _user_reg;
        if (highpart) user |= SPI_USR_MOSI_HIGHPART;
        if (len != limit) {
          wait_spi();
          set_len(limit * sizeof(TDst) << 3);
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

    template<class T>
    __attribute__ ((always_inline)) inline static void readPixelsTemplate(T* buf, uint32_t length)
    {
      if (_panel->len_read_pixel == 24) {
        read_pixels(buf, length, copy_to_userbuf_template<T, swap888_t>);
      }
    }

    template <class TDst, class TSrc>
    static void* copy_to_userbuf_template(void* dst, uint32_t len) {
      auto s = (TSrc*)_regbuf;
      auto d = (TDst*)dst;
      while (len--) { *d++ = *s++; }
      return d;
    }
    static void read_pixels(void* dst, uint32_t length, void*(*copy_to_userbuf)(void*, uint32_t))
    {
      if (!length) return;
      uint32_t len(length & 7);
      length >>= 3;
      wait_spi();
      if (len) {
        set_len(_panel->len_read_pixel * len);
        exec_spi();
        wait_spi();
        memcpy(_regbuf, (void*)_spi_w0_reg, 3 * len);
        if (length) {
          set_len(_panel->len_read_pixel << 3); // 8pixel read
          exec_spi();
        }
        dst = copy_to_userbuf(dst, len);
        if (!length) { return; }
      } else {
        set_len(_panel->len_read_pixel << 3); // 8pixel read
        exec_spi();
      }
      while (--length) {
        wait_spi();
        memcpy(_regbuf, (void*)_spi_w0_reg, 24);
        exec_spi();
        dst = copy_to_userbuf(dst, 8);
      }
      wait_spi();
      memcpy(_regbuf, (void*)_spi_w0_reg, 24);
      copy_to_userbuf(dst, 8);
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

    static const CFG _cfg;
    static const PanelLcdCommon* _panel;
    static uint32_t _panel_len_command;
    static uint32_t _panel_cmd_caset;
    static uint32_t _panel_cmd_raset;
    //static uint8_t _panel_len_read_pixel;
    //static uint8_t _panel_len_dummy_read_pixel;
    //static uint8_t _panel_len_dummy_read_rddid;
    static uint8_t _bpp;
    static uint8_t _invert;
    static uint8_t _rotation;
    static int32_t _width;
    static int32_t _height;
    static uint32_t _colstart;
    static uint32_t _rowstart;
    static uint32_t _last_xs;
    static uint32_t _last_xe;
    static uint32_t _last_ys;
    static uint32_t _last_ye;
    static uint32_t _last_apb_freq;
    static uint32_t _clkdiv_write;
    static uint32_t _clkdiv_read;
    static uint32_t _clkdiv_fill;
    static bool _fillmode;
    static uint32_t _user_reg;
    static uint32_t _regbuf[8];
  };
  template <class T> const T Esp32Spi<T>::_cfg;
  template <class T> const PanelLcdCommon* Esp32Spi<T>::_panel;
  template <class T> uint32_t Esp32Spi<T>::_panel_len_command;
  template <class T> uint32_t Esp32Spi<T>::_panel_cmd_caset;
  template <class T> uint32_t Esp32Spi<T>::_panel_cmd_raset;
//template <class T> uint8_t Esp32Spi<T>::_panel_len_read_pixel;
//template <class T> uint8_t Esp32Spi<T>::_panel_len_dummy_read_pixel;
//template <class T> uint8_t Esp32Spi<T>::_panel_len_dummy_read_rddid;
  template <class T> uint8_t Esp32Spi<T>::_bpp;
  template <class T> uint8_t Esp32Spi<T>::_invert;
  template <class T> uint8_t Esp32Spi<T>::_rotation;
  template <class T> int32_t Esp32Spi<T>::_width;
  template <class T> int32_t Esp32Spi<T>::_height;
  template <class T> uint32_t Esp32Spi<T>::_colstart;
  template <class T> uint32_t Esp32Spi<T>::_rowstart;
  template <class T> uint32_t Esp32Spi<T>::_last_xs;
  template <class T> uint32_t Esp32Spi<T>::_last_xe;
  template <class T> uint32_t Esp32Spi<T>::_last_ys;
  template <class T> uint32_t Esp32Spi<T>::_last_ye;
  template <class T> uint32_t Esp32Spi<T>::_last_apb_freq;
  template <class T> uint32_t Esp32Spi<T>::_clkdiv_write;
  template <class T> uint32_t Esp32Spi<T>::_clkdiv_read;
  template <class T> uint32_t Esp32Spi<T>::_clkdiv_fill;
  template <class T> bool Esp32Spi<T>::_fillmode;
  template <class T> uint32_t Esp32Spi<T>::_user_reg;
  template <class T> uint32_t Esp32Spi<T>::_regbuf[];
}
#endif
