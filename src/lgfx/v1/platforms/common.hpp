/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (CONFIG_IDF_TARGET_ESP32S2) || defined (ESP_PLATFORM)

#include "esp32/common.hpp"

#endif

#if defined (__SAMD51__)

#include "samd51/common.hpp"

#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class FlipBuffer
  {
  public:
    virtual ~FlipBuffer(void)
    {
      deleteBuffer();
    }

    void deleteBuffer(void)
    {
      for (int i = 0; i < 2; i++)
      {
        _length[i] = 0;
        if (_buffer[i])
        {
          heap_free(_buffer[i]);
          _buffer[i] = nullptr;
        }
      }
    }

    std::uint8_t* getBuffer(std::uint32_t length)
    {
      length = (length + 3) & ~3;
      _flip = !_flip;

      if (_length[_flip] != length)
      {
        if (_buffer[_flip]) heap_free(_buffer[_flip]);
        _buffer[_flip] = (std::uint8_t*)heap_alloc_dma(length);
        _length[_flip] = _buffer[_flip] ? length : 0;
      }
      return _buffer[_flip];
    }

  private:
    std::uint8_t* _buffer[2] = { nullptr, nullptr };
    std::uint32_t _length[2] = { 0, 0 };
    bool _flip = false;
  };

//----------------------------------------------------------------------------
 }
}
