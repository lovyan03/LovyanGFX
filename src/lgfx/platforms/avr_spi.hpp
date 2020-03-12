#ifndef LGFX_AVR_SPI_HPP_
#define LGFX_AVR_SPI_HPP_

// #pragma GCC optimize ("O3")

#include <SPI.h>

#include "lgfx_common.hpp"
#include "avr_common.hpp"

namespace lgfx
{
  template <typename Panel, typename CFG>
  class AvrSpi
  {
    inline static volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }
  public:
    inline uint32_t getColorFromRGB(uint8_t r, uint8_t g, uint8_t b) { return (_bpp == 16) ? color565(r,g,b) : color888(r,g,b); }

    ArduinoSpi()
    {
      //_spi_dev = spicommon_hw_for_host(_spi_host);
      _caset = _paset = 0xffffffff;
      _width = Panel::PANEL_WIDTH;
      _height = Panel::PANEL_HEIGHT;
    }

    inline void init(void)
    {
      _spisetting = SPISettings(8000000, MSBFIRST, SPI_MODE0);

      const uint8_t *cmds;
      for (uint8_t i = 0; cmds = Panel::getInitCommands(i); i++) {
        delay(120);
        startWrite();
        commandList(cmds);
        endWrite();
      }
    }

    static bool commandList(const uint8_t *addr)
    {
      if (addr == nullptr) return false;
      uint8_t  cmd;
      uint8_t  numArgs;
      uint8_t  ms;

      for (;;) {                // For each command...
        cmd     = pgm_read_byte(addr++);  // Read, issue command
        numArgs = pgm_read_byte(addr++);  // Number of args to follow
        if (0xFF == cmd && 0xFF == numArgs) break;
        write_cmd(cmd);
        ms = numArgs & Panel::CMD_INIT_DELAY;       // If hibit set, delay follows args
        numArgs &= ~Panel::CMD_INIT_DELAY;          // Mask out delay bit

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

    inline void startWrite(void) {
      SPI.begin();
      set_clock_write();
      TPin<_spi_cs>::lo();
    }

    inline static void endWrite(void) {
      SPI.end();
      wait_spi();
      TPin<_spi_cs>::hi();
    }

    inline void startFill(void) {
    }

    inline void endFill(void) {
    }

    void startRead()  {
      write_cmd(Panel::CMD::RAMRD);
      wait_spi();
//      TPin<_spi_dc>::hi();
      set_clock_read();
      if (Panel::LEN_DUMMY_READ_PIXEL) read_data(Panel::LEN_DUMMY_READ_PIXEL);
    }
    void endRead() {
      set_clock_write();
      TPin<_spi_cs>::hi();
      { delay(1); }
      TPin<_spi_cs>::lo();
    }

    inline uint16_t width(void) const { return _width; }
    inline uint16_t height(void) const { return _height; }
    inline bool    getInvert(void) const { return _invert; }
    inline uint8_t getRotation(void) const { return _rotation; }
    inline uint8_t getColorDepth(void) const { return _bpp; }

    void setRotation(uint8_t r)
    {
      write_cmd(Panel::CMD::MADCTL);
      r = r & 7;
      _rotation = r;

      auto rotation_data = Panel::getRotationData(r);
      _colstart = rotation_data->colstart;
      _rowstart = rotation_data->rowstart;
      write_data(rotation_data->madctl, 8);
      if (r & 1) {
        _width  = Panel::PANEL_HEIGHT;
        _height = Panel::PANEL_WIDTH;
      } else {
        _width  = Panel::PANEL_WIDTH;
        _height = Panel::PANEL_HEIGHT;
      }
      _caset = _paset = 0xffffffff;
    }

    void* setColorDepth(uint8_t bpp)  // b : 16 or 24
    {
      write_cmd(Panel::CMD::PIXSET);
      _bpp = Panel::getAdjustBpp(bpp);
      write_data(Panel::getPixset(_bpp), 8);
      return nullptr;
    }

    void invertDisplay(bool i)
    { // Send the command twice as otherwise it does not always work!
      _invert = i;
      write_cmd(i ? Panel::CMD::INVON : Panel::CMD::INVOFF);
      write_cmd(i ? Panel::CMD::INVON : Panel::CMD::INVOFF);
    }

    uint32_t readPanelID(void)
    {
      if (0 == Panel::CMD::RDDID) return 0;
      //write_cmd(0xD9);
      //write_data(0x10, 8);
      write_cmd(Panel::CMD::RDDID);
      wait_spi();
      TPin<_spi_dc>::hi();
      set_clock_read();
      if (Panel::LEN_DUMMY_READ_RDDID) {
        read_data(Panel::LEN_DUMMY_READ_RDDID);
        wait_spi();
      }
      uint32_t res = read_data(32);
      endRead();
      return res;
    }

    uint32_t readPanelIDSub(uint8_t cmd)
    {
      //write_cmd(0xD9);
      //write_data(0x10, 8);
      write_cmd(cmd);
      wait_spi();
      TPin<_spi_dc>::hi();
      set_clock_read();
      uint32_t res = read_data(32);
      endRead();
      return res;
    }


    void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
    {
      { xs += _colstart; xe += _colstart; }
      uint32_t caset = Panel::getWindowAddr(xs, xe);
      if (_caset != caset) {
        write_cmd(Panel::CMD::CASET);
        write_data(caset, 32);
        _caset = caset;
      }
      { ys += _rowstart; ye += _rowstart; }
      uint32_t paset = Panel::getWindowAddr(ys, ye);
      if (_paset != paset) {
        write_cmd(Panel::CMD::PASET);
        write_data(paset, 32);
        _paset = paset;
      }
    }

    inline void setWindowFill(int32_t xs, int32_t ys, int32_t xe, int32_t ye, uint32_t color) {
      setWindow(xs, ys, xe, ye);
      write_cmd(Panel::CMD::RAMWR);
      writeColor(color, (xe-xs+1) * (ye-ys+1));
    }

    inline void writeMode()  {
      write_cmd(Panel::CMD::RAMWR);
    }

    inline void writeColor(uint32_t color)
    {
      write_data( (_bpp == 16)
                ? Panel::getWriteColor16(color) 
                : Panel::getWriteColor24(color), _bpp);
    }

    void writeColor(uint32_t color, uint32_t length)
    {
      uint32_t buf[17];
      if (_bpp == 16) {
        buf[0] = Panel::getWriteColor16(color);
      } else {
        buf[0] = Panel::getWriteColor24(color);
      }
      const uint32_t limit = (_bpp == 24) ? 21 : 32; // limit = 512 / bpp;
      uint16_t len = length % limit;
      if (!len) len = limit;
      length -= len;
      uint16_t ie;
      if (_bpp == 24) {
        uint8_t* dst = ((uint8_t*)buf) + 3;
        *(uint32_t*)dst = buf[0]; dst += 3;
        *(uint32_t*)dst = buf[0]; dst += 3;
        *(uint32_t*)dst = buf[0];
        ie = (len + 1) * 3 >> 2;
      } else {
        ((uint16_t*)buf)[1] = buf[0];
        ie = (len + 1) >> 1;
      }
      wait_spi();
      TPin<_spi_dc>::hi();
      if (_bpp == 24) { for (uint16_t i = 0; i < ie; i++) { SPI.transfer(buf[i%3]); } } 
      else {            for (uint16_t i = 0; i < ie; i++) { SPI.transfer(buf[  0]); } }
      set_len(len * _bpp);
      exec_spi();
      for (; length; length -= limit) {
        wait_spi();
        if (_bpp == 24) { for (uint16_t i = 0; i < 16; i++) { SPI.transfer(buf[i%3]); } } 
        else {            for (uint16_t i = 0; i < 16; i++) { SPI.transfer(buf[  0]); } }
        set_len(limit * _bpp);
        exec_spi();
      }
    }

    // need data size = length * (24bpp=3 : 16bpp=2)
    void writePixels(const void* src, uint32_t length, bool swap)
    {
      if (swap) writePixels(src, length);
      else      writeBytes((const uint8_t*)src, length * getColorDepth() >> 3);
    }

    // need data size = length * (24bpp=3 : 16bpp=2)
    void writePixels(const void* src, uint32_t length)
    {
      const uint8_t* data = (uint8_t*)src;
      uint32_t buf[17];
      uint8_t* dst = (uint8_t*)buf;
      const uint32_t step = _bpp >> 3;
      const uint16_t limit = (_bpp == 24) ? 21 : 32; // limit = 512 / bpp;
      uint16_t len = length % limit;
      if (!len) len = limit;
      length -= len;

      uint16_t ie;
      if (_bpp == 24) {
        ie = (len + 1) * 3 >> 2;
        for (uint16_t i = 0; i < len; i++) {
          *(uint32_t*)dst = Panel::getWriteColor24(*(const uint32_t*)data);
          dst += step;
          data += step;
        }
      } else {
        ie = (len + 1) >> 1;
        for (uint16_t i = 0; i < len; i++) {
          *(uint16_t*)dst = Panel::getWriteColor16(*(const uint32_t*)data);
          dst += step;
          data += step;
        }
      }
      wait_spi();
      TPin<_spi_dc>::hi();
      for (uint16_t i = 0; i < ie; i++) SPI.transfer(buf[i]);
      set_len(len * _bpp);
      exec_spi();
      for (; length; length -= limit) {
        dst = (uint8_t*)buf;
        if (_bpp == 24) {
          for (uint16_t i = 0; i < limit; i++) {
            *(uint32_t*)dst = Panel::getWriteColor24(*(const uint32_t*)data);
            dst += step;
            data += step;
          }
        } else {
          for (uint16_t i = 0; i < limit; i++) {
            *(uint16_t*)dst = Panel::getWriteColor16(*(const uint32_t*)data);
            dst += step;
            data += step;
          }
        }
        wait_spi();
        for (uint16_t i = 0; i < 16; i++) SPI.transfer(buf[i]);
        set_len(limit * _bpp);
        exec_spi();
      }
    }

    // need data size = length ( no use bpp )
    void writeBytes(const uint8_t* data, uint32_t length)
    {
      const uint32_t* data32 = (const uint32_t*)data;
      const uint16_t limit = 64;
      uint16_t len = length % limit;
      if (!len) len = limit;
      length -= len;
      uint16_t ie = (len+3)>>2;
      wait_spi();
      TPin<_spi_dc>::hi();
      set_len(len << 3);
      for (uint16_t i = 0; i < ie; i++) SPI.transfer(*data32++);
      exec_spi();
      data32 = (const uint32_t*)(data + len);
      for (; length; length -= limit) {
        wait_spi();
        for (uint16_t i = 0; i < 16; i++) SPI.transfer(*data32++);
        set_len(limit << 3);
        exec_spi();
      }
    }

    uint32_t readPixel(void)
    {
      return (_bpp == 16) ? Panel::getColor16FromRead(read_data(Panel::LEN_READ_PIXEL))
                          : Panel::getColor24FromRead(read_data(Panel::LEN_READ_PIXEL));
    }

    void readPixels(uint32_t len, uint8_t* buf, bool swapBytes)
    {
/*
#ifdef taskDISABLE_INTERRUPTS
      taskDISABLE_INTERRUPTS();
#endif
      if (_bpp == 16) {
        readPixels16( len, (uint16_t*)buf
                    , swapBytes ? Panel::getColor16FromRead
                                : Panel::getWriteColor16FromRead
                    );
      } else if (_bpp == 24) {
        readPixels24( len, buf
                    , swapBytes ? Panel::getColor24FromRead
                                : Panel::getWriteColor24FromRead
                    );
      }
#ifdef taskENABLE_INTERRUPTS
      taskENABLE_INTERRUPTS();
#endif
*/
    }

  private:
    uint8_t _bpp      = 16;
    uint8_t _invert   = false;
    uint8_t _rotation = 0;

    int16_t _colstart = 0, _rowstart = 0;
    int16_t _width, _height;
    uint32_t _caset, _paset;

    static constexpr int _spi_mosi = 11;
    static constexpr int _spi_miso = 12;
    static constexpr int _spi_sclk = 13;
    static constexpr int _spi_cs   = 10;
    static constexpr int _spi_dc   =  9;
    static constexpr int _gpio_rst= -1;
    static SPISettings _spisetting;
/*
    uint32_t _clkdiv_write;
    uint32_t _clkdiv_read ;
    uint32_t _clkdiv_fill ;
    //static volatile spi_dev_t* _spi_dev;
    static constexpr uint8_t _spi_port = (_spi_host == HSPI_HOST) ? 2 : 3;  // FSPI=1  HSPI=2  VSPI=3;
    static constexpr volatile uint32_t *_spi_w0_reg        = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_W0_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_cmd_reg       = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_CMD_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_clock_reg     = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_CLOCK_REG(_spi_port));
    static constexpr volatile uint32_t *_spi_mosi_dlen_reg = (volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_MOSI_DLEN_REG(_spi_port));
*/
    inline void set_clock_write(void) { SPI.setClockDivider(SPI_CLOCK_DIV2); }
    inline void set_clock_read(void)  { SPI.setClockDivider(SPI_CLOCK_DIV2); }
    inline void set_clock_fill(void)  { SPI.setClockDivider(SPI_CLOCK_DIV2); }
    inline static void exec_spi(void) {  }
    inline static void wait_spi(void) {  }
    inline static void set_len(uint32_t len) {  }

    inline static void write_cmd(uint8_t cmd) __attribute__ ((always_inline))
    {
      wait_spi();
      //taskDISABLE_INTERRUPTS();
      TPin<_spi_dc>::lo();
      SPI.transfer(cmd);
      exec_spi();
      //taskENABLE_INTERRUPTS();
    }
/*
    static void readPixels16(uint32_t len, uint16_t* buf, uint16_t (*func)(uint32_t))
    {
      if (!len) return;
      set_len(Panel::LEN_READ_PIXEL);
      exec_spi();
      while (len--) {
        wait_spi();
        if (len) exec_spi();
        *buf++ = func(*_spi_w0_reg);
      }
    }

    static void readPixels24(uint32_t len, uint8_t* buf, uint32_t (*func)(uint32_t))
    {
      if (!len) return;
      set_len(Panel::LEN_READ_PIXEL);
      exec_spi();
      while (--len) {
        wait_spi();
        exec_spi();
        *(uint32_t*)buf = func(*_spi_w0_reg);
        buf += 3;
      }
      wait_spi();
      uint32_t c = func(*_spi_w0_reg);
      *buf++ = c;
      *buf++ = c >> 8;
      *buf   = c >> 16;
    }
*/
    inline static void write_data(uint32_t data, uint8_t len) __attribute__ ((always_inline))
    {
      TPin<_spi_dc>::hi();
      while (len >= 8) {
        SPI.transfer(data);
        len -= 8;
        data >>= 8;
      }
    }

    inline static uint32_t read_data(uint8_t len) __attribute__ ((always_inline))
    {
      uint8_t res[4];
      for (int i = 0; i < len; i += 8) {
        res[i >> 3] = SPI.transfer(0);
      }
      return *(uint32_t*)res;
    }
  };
}
#endif
