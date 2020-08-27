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
#ifndef LGFX_DEVICE_HPP_
#define LGFX_DEVICE_HPP_

#include "LGFXBase.hpp"

namespace lgfx
{

//----------------------------------------------------------------------------

  class LGFX_Device : public LovyanGFX
  {
  public:
    // Write single byte as COMMAND
    virtual void writeCommand(std::uint_fast8_t cmd) = 0; // AdafruitGFX compatible
    __attribute__ ((always_inline))
    inline  void writecommand(std::uint_fast8_t cmd) { writeCommand(cmd); } // TFT_eSPI compatible

    // Write single bytes as DATA
    virtual void writeData(std::uint_fast8_t data) = 0; // TFT_eSPI compatible
    __attribute__ ((always_inline))
    inline  void spiWrite( std::uint_fast8_t data) { writeData(data); } // AdafruitGFX compatible
    __attribute__ ((always_inline))
    inline  void writedata(std::uint_fast8_t data) { writeData(data); } // TFT_eSPI compatible

    virtual std::uint32_t readCommand(std::uint_fast8_t cmd, std::uint_fast8_t index=0, std::uint_fast8_t len=4) = 0;

    std::uint8_t  readCommand8( std::uint_fast8_t cmd, std::uint_fast8_t index=0) { return readCommand(cmd, index, 1); }
    std::uint8_t  readcommand8( std::uint_fast8_t cmd, std::uint_fast8_t index=0) { return readCommand(cmd, index, 1); }
    std::uint16_t readCommand16(std::uint_fast8_t cmd, std::uint_fast8_t index=0) { return __builtin_bswap16(readCommand(cmd, index, 2)); }
    std::uint16_t readcommand16(std::uint_fast8_t cmd, std::uint_fast8_t index=0) { return __builtin_bswap16(readCommand(cmd, index, 2)); }
    std::uint32_t readCommand32(std::uint_fast8_t cmd, std::uint_fast8_t index=0) { return __builtin_bswap32(readCommand(cmd, index, 4)); }
    std::uint32_t readcommand32(std::uint_fast8_t cmd, std::uint_fast8_t index=0) { return __builtin_bswap32(readCommand(cmd, index, 4)); }
    std::uint32_t readPanelID(void) { return readCommand(_panel->getCmdRddid()); }

    __attribute__ ((always_inline)) inline bool getInvert(void) const { return _panel->invert; }

    __attribute__ ((always_inline)) inline PanelCommon* getPanel(void) const { return _panel; }
    __attribute__ ((always_inline)) inline PanelCommon* panel(void) const { return _panel; }

    __attribute__ ((always_inline)) inline void setPanel(PanelCommon* panel) { _panel = panel; postSetPanel(); }
    __attribute__ ((always_inline)) inline void panel(PanelCommon* panel) { _panel = panel; postSetPanel(); }

    __attribute__ ((always_inline)) inline TouchCommon* touch(void) const { return _touch; }
    __attribute__ ((always_inline)) inline void touch(TouchCommon* touch_) { _touch = touch_; postSetTouch(); }

    bool isReadable_impl(void) const override { return _panel->spi_read; }
    std::int_fast8_t getRotation_impl(void) const override { return _panel->rotation; }

    void sleep()  { writeCommand(_panel->getCmdSlpin()); _panel->sleep(); }

    void wakeup() { writeCommand(_panel->getCmdSlpout()); _panel->wakeup(); }

    void setColorDepth(std::uint8_t bpp) { setColorDepth((color_depth_t)bpp); }
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

    virtual void initBus(void) = 0;
    
    void initPanel(void)
    {
      preInit();
      if (!_panel) return;

      _panel->init();

      _sx = _sy = 0;
      _sw = _width;
      _sh = _height;

      startWrite();

      const std::uint8_t *cmds;
      for (std::uint8_t i = 0; (cmds = _panel->getInitCommands(i)); i++) {
        delay(120);
        cs_l();
        commandList(cmds);
        cs_h();
      }
      cs_l();

      invertDisplay(getInvert());
      setColorDepth(getColorDepth());
      setRotation(getRotation());

      endWrite();
    }

    void initTouch(void)
    {
      if (!_touch) return;

      _touch->init();
      _touch->wakeup();
    }

    bool getTouch(std::int32_t *px, std::int32_t *py, std::uint_fast8_t number = 0)
    {
      if (!_touch) {
        return false;
      }

      bool res = _touch->getTouch(px, py, number);

      auto touch_min = _touch->x_min;
      auto touch_max = _touch->x_max;
      std::int32_t diff = (touch_max - touch_min) + 1;
      std::int32_t x = (*px - touch_min) * _panel->panel_width / diff;

      touch_min = _touch->y_min;
      touch_max = _touch->y_max;
      diff = (touch_max - touch_min) + 1;
      std::int32_t y = (*py - touch_min) * _panel->panel_height / diff;

      int r = _panel->rotation & 7;
      if (r & 1) {
        std::swap(x, y);
      }

      std::int32_t w = _width-1;
      if (x < 0) x = 0;
      if (x > w) x = w;
      if (r & 2) x = w - x;
      *px = x;

      std::int32_t h = _height-1;
      if (y < 0) y = 0;
      if (y > h) y = h;
      if ((0 == ((r + 1) & 2)) != (0 == (r & 4))) y = h - y;
      *py = y;

      return res;
    }

    bool commandList(const std::uint8_t *addr)
    {
      if (addr == nullptr) return false;
      std::uint8_t  cmd;
      std::uint8_t  numArgs;
      std::uint8_t  ms;

      startWrite();
      preCommandList();
      for (;;) {                // For each command...
        cmd     = *addr++;  // Read, issue command
        numArgs = *addr++;  // Number of args to follow
        if (0xFF == (cmd & numArgs)) break;
        writeCommand(cmd);
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
      postCommandList();
      endWrite();
      return true;
    }

  protected:
    PanelCommon* _panel = nullptr;
    TouchCommon* _touch = nullptr;

    std::uint_fast16_t _touch_xmin;
    std::uint_fast16_t _touch_xmax;
    std::uint_fast16_t _touch_ymin;
    std::uint_fast16_t _touch_ymax;
    bool _touch_calibration = false;
    bool _last_touched = false;

    virtual void init_impl(void) {
      initBus(); 
      initPanel(); 
      initTouch(); 
      startWrite(); 
      clear(); 
      setWindow(0,0,0,0); 
      endWrite();
    }

    virtual void preInit(void) {}
    virtual void preCommandList(void) {}
    virtual void postCommandList(void) {}
    virtual void postSetPanel(void) {}
    virtual void postSetRotation(void) {}
    virtual void postSetTouch(void)
    {
      _touch_xmin = _touch->x_min;
      _touch_xmax = _touch->x_max;
      _touch_ymin = _touch->y_min;
      _touch_ymax = _touch->y_max;
    }

    void postSetColorDepth(void)
    {
      _write_conv.setColorDepth(_panel->write_depth);
      _read_conv.setColorDepth(_panel->read_depth);
    }

    void copyRect_impl(std::int32_t dst_x, std::int32_t dst_y, std::int32_t w, std::int32_t h, std::int32_t src_x, std::int32_t src_y) override
    {
      pixelcopy_t p((void*)nullptr, _write_conv.depth, _read_conv.depth);
      if (w < h) {
        const std::uint32_t buflen = h * _write_conv.bytes;
        std::uint8_t buf[buflen];
        std::int32_t add = (src_x < dst_x) ?   - 1 : 1;
        std::int32_t pos = (src_x < dst_x) ? w - 1 : 0;
        do {
          readRect_impl(src_x + pos, src_y, 1, h, buf, &p);
          setWindow_impl(dst_x + pos, dst_y, dst_x + pos, dst_y + h - 1);
          writePixelsDMA_impl(buf, h);
          pos += add;
        } while (--w);
        waitDMA_impl();
      } else {
        const std::uint32_t buflen = w * _write_conv.bytes;
        std::uint8_t buf[buflen];
        std::int32_t add = (src_y < dst_y) ?   - 1 : 1;
        std::int32_t pos = (src_y < dst_y) ? h - 1 : 0;
        do {
          readRect_impl(src_x, src_y + pos, w, 1, buf, &p);
          setWindow_impl(dst_x, dst_y + pos, dst_x + w - 1, dst_y + pos);
          writePixelsDMA_impl(buf, w);
          pos += add;
        } while (--h);
        waitDMA_impl();
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
    }

    void delete_dmabuffer(void)
    {
      _dmabufs[0].free();
      _dmabufs[1].free();
    }

    void cs_h(void) {
      gpio_hi(_panel->spi_cs);
    }
    void cs_l(void) {
      gpio_lo(_panel->spi_cs);
    }

  private:
    bool _dma_flip = false;
    _dmabufs_t _dmabufs[2];

  };

//----------------------------------------------------------------------------

}

using lgfx::LGFX_Device;

#endif
