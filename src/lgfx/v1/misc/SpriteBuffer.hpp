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

#include <algorithm>
#include <cassert>

#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct bgr888_t;

  enum AllocationSource
  {
    Normal,
    Dma,
    Psram,
    Preallocated,
  };

  class SpriteBuffer
  {
  private:
    std::uint8_t* _buffer;
    std::size_t _length;
    AllocationSource _source;

  public:
    SpriteBuffer(void) : _buffer(nullptr), _length(0), _source(Dma) {}

    SpriteBuffer(std::size_t length, AllocationSource source = AllocationSource::Dma) : _buffer(nullptr), _length(0), _source(source)
    {
      if (length)
      {
        assert (source != AllocationSource::Preallocated);
        this->reset(length, source);
      }
    }

    SpriteBuffer(std::uint8_t* buffer, std::size_t length) : _buffer(buffer), _length(length), _source(AllocationSource::Preallocated)
    {
    }

    SpriteBuffer(const SpriteBuffer& rhs) : _buffer(nullptr)
    {
      if ( rhs._source == AllocationSource::Preallocated )
      {
        this->_buffer = rhs._buffer;
        this->_length = rhs._length;
        this->_source = rhs._source;
      }
      else
      {
        this->reset(rhs._length, rhs._source);
        if( _buffer != nullptr && rhs._buffer != nullptr )
        {
          std::copy(rhs._buffer, rhs._buffer + _length, _buffer);
        }
      }
    }

    SpriteBuffer(SpriteBuffer&& rhs) : _buffer(nullptr)
    {
      if ( rhs._source == AllocationSource::Preallocated ) {
        this->_buffer = rhs._buffer;
        this->_length = rhs._length;
        this->_source = rhs._source;
      }
      else {
        this->reset(rhs._length, rhs._source);
        if( _buffer != nullptr && rhs._buffer != nullptr ) {
          std::copy(rhs._buffer, rhs._buffer + _length, _buffer);
          rhs.release();
        }
      }
    }

    SpriteBuffer& operator=(const SpriteBuffer& rhs)
    {
      if ( rhs._source == AllocationSource::Preallocated ) {
        this->_buffer = rhs._buffer;
        this->_length = rhs._length;
        this->_source = rhs._source;
      }
      else {
        this->reset(rhs._length, rhs._source);
        if ( _buffer != nullptr && rhs._buffer != nullptr ) {
          std::copy(rhs._buffer, rhs._buffer + _length, _buffer);
        }
      }
      return *this;
    }

    SpriteBuffer& operator=(SpriteBuffer&& rhs)
    {
      if( rhs._source == AllocationSource::Preallocated ) {
        this->_buffer = rhs._buffer;
        this->_length = rhs._length;
        this->_source = rhs._source;
      }
      else {
        this->reset(rhs._length, rhs._source);
        if( _buffer != nullptr && rhs._buffer != nullptr ) {
          std::copy(rhs._buffer, rhs._buffer + _length, _buffer);
          rhs.release();
        }
      }
      return *this;
    }

    operator std::uint8_t*() const { return _buffer; }
    operator bool() const { return _buffer != nullptr; }

    std::uint8_t* get() const { return _buffer; }
    std::uint8_t* img8() const { return _buffer; }
    std::uint16_t* img16() const { return reinterpret_cast<std::uint16_t*>(_buffer); }
    bgr888_t* img24() const { return reinterpret_cast<bgr888_t*>(_buffer); }

    void reset(void* buffer)
    {
      this->release();
      _source = AllocationSource::Preallocated;
      _buffer = reinterpret_cast<std::uint8_t*>(buffer);
      _length = 0;
    }

    void reset(std::size_t length, AllocationSource source)
    {
      this->release();
      void* buffer = nullptr;
      _source = source;
      switch (source)
      {
        default:
        case AllocationSource::Normal:
          buffer = heap_alloc(length);
          break;
        case AllocationSource::Dma:
          buffer = heap_alloc_dma(length);
          break;
        case AllocationSource::Psram:
          buffer = heap_alloc_psram(length);
          if (!buffer)
          {
            _source = AllocationSource::Dma;
            buffer = heap_alloc_dma(length);
          }
          break;
      }
      _buffer = reinterpret_cast<std::uint8_t*>(buffer);
      if ( _buffer != nullptr ) {
        _length = length;
      }
    }

    void release() {
      _length = 0;
      if ( _buffer != nullptr ) {
        if (_source != AllocationSource::Preallocated)
        {
          heap_free(_buffer);
        }
        _buffer = nullptr;
      }
    }

    bool use_dma() const { return _source == AllocationSource::Dma; }
    bool use_memcpy() const { return _source != AllocationSource::Psram; }
  };

//----------------------------------------------------------------------------
 }
}
