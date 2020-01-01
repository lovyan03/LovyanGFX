#ifndef LGFX_PANEL_COMMON_HPP_
#define LGFX_PANEL_COMMON_HPP_

#include "lgfx_common.hpp"

namespace lgfx
{
  class PanelCommon
  {
  protected:
    dev_color_t _color;
    bool _invert;
    uint8_t _bpp;
    uint8_t _rotation;
    int32_t _width;
    int32_t _height;

  public:
    PanelCommon() 
    : _color(0)
    , _invert(false)
    , _bpp(16)
    , _rotation(0)
    , _width(0)
    , _height(0)
    {}

    inline bool getInvert() const { return _invert; }
    inline uint8_t getColorDepth() const { return _bpp; }
    inline uint8_t getRotation() const { return _rotation; }
    inline int32_t width() const { return _width; }
    inline int32_t height() const { return _height; }

    inline const dev_color_t& getColor() const { return _color; }
    inline void setColor(const dev_color_t& c) { _color = c; }
#define TYPECHECK(dType) template < typename tType, typename std::enable_if < (sizeof(tType) == sizeof(dType)) && (std::is_signed<tType>::value == std::is_signed<dType>::value), std::nullptr_t >::type=nullptr >
    TYPECHECK(uint8_t ) __attribute__ ((always_inline)) inline void setColor(tType c) { setColor(*(rgb332_t*)&c); }
    TYPECHECK(uint16_t) __attribute__ ((always_inline)) inline void setColor(tType c) { setColor(*(rgb565_t*)&c); }
    TYPECHECK(uint32_t) __attribute__ ((always_inline)) inline void setColor(tType c) { setColor(*(rgb888_t*)&c); }
    TYPECHECK(int     ) __attribute__ ((always_inline)) inline void setColor(tType c) { setColor(*(rgb565_t*)&c); }
#undef TYPECHECK
    inline void setColor(const rgb888_t& c) {
      _color.raw = (_bpp == 16) ? swap565( c.r, c.g, c.b)
                 : (_bpp == 24) ? c.r | c.g<<8 | c.b<<16
                 : (_bpp ==  8) ? color332(c.r, c.g, c.b)
                 : (c.r|c.g|c.b);
    }
    inline void setColor(const rgb565_t& c) {
      _color.raw = (_bpp == 16) ? c.raw << 8 | c.raw >> 8
                 : (_bpp == 24) ? c.R8() | (c.G8()<<8) | (c.B8()<<16)
                 : (_bpp ==  8) ? ((c.r<<3)&0xE0) | ((c.g>>1)&0x1C) | (c.b>>3)
                 : c.raw;
    }
    inline void setColor(const rgb332_t& c) {
      _color.raw = (_bpp == 16) ? swap565(c.R8(), c.G8(), c.B8())
                 : (_bpp == 24) ? swap888(c.R8(), c.G8(), c.B8())
                 : (_bpp ==  8) ? c.raw
                 : c.raw;
    }

    virtual void init() {}
    virtual void beginTransaction() {}
    virtual void endTransaction() {}

    virtual void* setColorDepth(uint8_t bpp) { return nullptr; }
    virtual void setRotation(uint8_t rotation) {}
    virtual void invertDisplay(bool invert) {}

    virtual void setWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) {}
    virtual void fillWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) {}
    virtual void readWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye) {}
    virtual void endRead(void) {}

    virtual void writeColor(uint32_t length = 1) {}

    virtual void writeBytes(      uint8_t* buf, uint32_t length) {}
    virtual void writeBytes(const uint8_t* buf, uint32_t length) {}

    virtual uint32_t readPanelID(void) { return 0; }
    virtual uint32_t readPanelIDSub(uint8_t cmd) { return 0; }

    virtual rgb565_t readPixel(void) { return 0; }
    virtual void readPixels(rgb332_t*   buf, uint32_t length) {}
    virtual void readPixels(rgb565_t*   buf, uint32_t length) {}
    virtual void readPixels(rgb888_t*   buf, uint32_t length) {}
    virtual void readPixels(swap565_t*  buf, uint32_t length) {}
    virtual void readPixels(swap888_t*  buf, uint32_t length) {}
    virtual void readPixels(argb8888_t* buf, uint32_t length) {}
/*
    virtual void writePixels(rgb332_t*   src, uint32_t length) {}
    virtual void writePixels(rgb565_t*   src, uint32_t length) {}
    virtual void writePixels(rgb888_t*   src, uint32_t length) {}
    virtual void writePixels(swap565_t*  src, uint32_t length) {}
    virtual void writePixels(swap888_t*  src, uint32_t length) {}
    virtual void writePixels(argb8888_t* src, uint32_t length) {}
*/
    virtual void writePixels(const rgb332_t*   src, uint32_t length) {}
    virtual void writePixels(const rgb565_t*   src, uint32_t length) {}
    virtual void writePixels(const rgb888_t*   src, uint32_t length) {}
    virtual void writePixels(const swap565_t*  src, uint32_t length) {}
    virtual void writePixels(const swap888_t*  src, uint32_t length) {}
    virtual void writePixels(const argb8888_t* src, uint32_t length) {}
  };
}

#endif
