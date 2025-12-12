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
 [UT2UH](https://github.com/UT2UH)
/----------------------------------------------------------------------------*/
#include "Panel_SharpLCD.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"
#include <cmath>


namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr uint8_t Bayer[] = { 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88, 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88 };

  static constexpr const uint8_t byte_rev_table[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
  };

  // 1<<n is a costly operation on AVR -- table usu. smaller & faster
  static const uint8_t set[] = {1, 2, 4, 8, 16, 32, 64, 128},
  clr[] = {(uint8_t)~1,  (uint8_t)~2,  (uint8_t)~4,
          (uint8_t)~8,  (uint8_t)~16, (uint8_t)~32,
          (uint8_t)~64, (uint8_t)~128};


  inline static uint8_t bitrev8(uint8_t byte)
  {
    return byte_rev_table[byte];
  }


  inline static uint32_t to_gray(uint8_t r, uint8_t g, uint8_t b)
  {
    return (uint32_t)          // gamma2.0 convert and ITU-R BT.601 RGB to Y convert
          ( (r * r * 19749)    // R 0.299
          + (g * g * 38771)    // G 0.587
          + (b * b *  7530)    // B 0.114
          ) >> 24;
  }


  void Panel_SharpLCD::setInvert(bool invert)
  {
    if( _invert != invert ) {
      _invert = invert;
    }
  }


  color_depth_t Panel_SharpLCD::setColorDepth(color_depth_t depth)
  {
    _write_depth = color_depth_t::rgb565_2Byte;
    _read_depth = color_depth_t::rgb565_2Byte;
    return _write_depth;
  }


  size_t Panel_SharpLCD::_get_buffer_length(void) const
  {
    return (_cfg.panel_width * _cfg.panel_height / 8);
  }


  void Panel_SharpLCD::setTilePattern(uint_fast8_t i)
  {
    _bayer_offset = Bayer[i & 15] >> 4;
  }


  bool Panel_SharpLCD::init(bool use_reset)
  {
    if (!Panel_HasBuffer::init(use_reset)) {
      return false;
    }

    if( _config_detail.pin_extmod > -1 )
    {
      lgfx::gpio_hi(_config_detail.pin_extmod);
      lgfx::pinMode(_config_detail.pin_extmod, pin_mode_t::output);
    }

    if(_config_detail.pin_dispon > -1 )
    {
      lgfx::gpio_hi(_config_detail.pin_dispon);
      lgfx::pinMode(_config_detail.pin_dispon, pin_mode_t::output);
    }
    
    _vcom = BIT_VCOM;
    memset(_buf, 0xff, _get_buffer_length());
    setRotation(_rotation);
    _pitch = std::ceil(float(_cfg.memory_width)/8.0f);
    setAutoDisplay(_config_detail.enable_autodisplay);
    return true;
  }


  void Panel_SharpLCD::init_cs(void)
  {
    auto pin = _cfg.pin_cs;
    if (pin < 0) return;
    lgfx::gpio_lo(pin);
    lgfx::pinMode(pin , pin_mode_t::output);
  }


  void Panel_SharpLCD::cs_control(bool level)
  {
    auto pin = _cfg.pin_cs;
    if (pin < 0) return;

    if (level) {
      lgfx::gpio_lo(pin);
    } else {
      lgfx::gpio_hi(pin);
    }
  }


  void Panel_SharpLCD::clearDisplay() {

    memset(_buf, 0xff, _get_buffer_length());

    beginTransaction();

    _bus->writeData(bitrev8(_vcom | CMD_CLEAR), 8);
    _bus->writeData(0x00, 8);
    _bus->wait();

    _vcom ^=1;

    endTransaction();
  }


  void Panel_SharpLCD::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    display(_buf, x, y, w, h);
  }


  void Panel_SharpLCD::display(uint8_t* dst, uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if( w==0 ) w = _cfg.panel_width;
    if( h==0 ) h = _cfg.panel_height;

    if (0 < w && 0 < h)
    {
      _range_mod.left   = std::min<int_fast16_t>(_range_mod.left  , x        );
      _range_mod.right  = std::max<int_fast16_t>(_range_mod.right , x + w - 1);
      _range_mod.top    = std::min<int_fast16_t>(_range_mod.top   , y        );
      _range_mod.bottom = std::max<int_fast16_t>(_range_mod.bottom, y + h - 1);
    }
    if (_range_mod.empty()) { return; }

    uint16_t i, lineAddr;
    uint8_t prefix_bytes = _config_detail.prefix_bytes;
    uint16_t bytes_per_line = prefix_bytes + _pitch + _config_detail.suffix_bytes;
    uint8_t line[bytes_per_line];
    
    beginTransaction();
    if(prefix_bytes == 1) {
      _bus->writeCommand(bitrev8(_vcom | CMD_UPDATE), 8);
      _vcom ^=1;
    }

    uint32_t buflen = _get_buffer_length();
    uint32_t bstart = std::min( uint32_t(_range_mod.top*_pitch), buflen );
    uint32_t bend   = std::max( uint32_t(_range_mod.bottom*_pitch), bstart );

    for (i = bstart; i <= bend; i += _pitch) {
      auto _linebuf = &dst[i];
      lineAddr = ((i + 1) / _pitch) + 1;

      if(prefix_bytes == 2) {
        if( _cfg.panel_height < 512 ) {
          // LS018B7DH02 230x303 requires 9 bits for line address
          // line number's 8 MSBs of 9 go to line[1]
          line[1] = bitrev8((lineAddr >> 1) & 0xFF);
          // line number's LSB is left in line[0]
          line[0] = uint8_t(lineAddr & 0x01) << 7;
        } else {
          // LS032B7DD02 336x536 requires 10 bits for line address
          // line number's 8 MSBs of 10 go to line[1]
          line[1] = bitrev8((lineAddr >> 2) & 0xFF);
          // line number's 2 LSBs are on left side in line[0]
          line[0] = uint8_t(lineAddr & 0x03) << 6;
        }
        if(i == bstart){
          //M0-M2 added to first line only
          line[0] = bitrev8(line[0] | _vcom | CMD_UPDATE);
          _vcom ^=1;
        } else {
          line[0] = bitrev8(line[0]);
        }
      } else {
        line[0] = bitrev8(lineAddr);
      }
      for(int j=0;j<=_pitch;j++) // apply LSB and invert if applicable
        line[j+prefix_bytes] = bitrev8( _invert ? ~_linebuf[j] : _linebuf[j] );
      line[bytes_per_line-1] = 0xff; // eol (value ignored)

      _bus->writeBytes(line, bytes_per_line, false, true);
    }

    _bus->writeCommand(bitrev8(VAL_TRAILER), 8);

    endTransaction();

  }


  void Panel_SharpLCD::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _xs = xs;
    _ys = ys;
    _xe = xe;
    _ye = ye;
    _update_transferred_rect(xs, ys, xe, ye);

    y = ys;

    do
    {
      x = xs;
      do
      {
        _draw_pixel(_buf, x, y, rawcolor);
      } while (++x <= xe);
    } while (++y <= ye);
  }


  void Panel_SharpLCD::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _update_transferred_rect(xs, ys, xe, ye);

    auto readbuf = (swap565_t*)alloca(w * sizeof(swap565_t));
    auto sx = param->src_x32;
    h += y;
    do
    {
      uint32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          do
          {
            auto color = readbuf[prev_pos];
            _draw_pixel(_buf, x + prev_pos, y, color.raw);
          } while (new_pos != ++prev_pos);
        }
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
    } while (++y < h);
  }


  void Panel_SharpLCD::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    {
      uint_fast16_t xs = _xs;
      uint_fast16_t xe = _xe;
      uint_fast16_t ys = _ys;
      uint_fast16_t ye = _ye;
      _update_transferred_rect(xs, ys, xe, ye);
    }
    uint_fast16_t xs   = _xs  ;
    uint_fast16_t ys   = _ys  ;
    uint_fast16_t xe   = _xe  ;
    uint_fast16_t ye   = _ye  ;
    uint_fast16_t xpos = _xpos;
    uint_fast16_t ypos = _ypos;

    static constexpr uint32_t buflen = 16;
    swap565_t colors[buflen];
    int bufpos = buflen;
    do
    {
      if (bufpos == buflen) {
        param->fp_copy(colors, 0, std::min(length, buflen), param);
        bufpos = 0;
      }
      auto color = colors[bufpos++];
      _draw_pixel(_buf, xpos, ypos, color.raw);
      if (++xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye)
        {
          ypos = ys;
        }
      }
    } while (--length);
    _xpos = xpos;
    _ypos = ypos;
  }


  void Panel_SharpLCD::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    auto readbuf = (swap565_t*)alloca(w * sizeof(swap565_t));
    param->src_data = readbuf;
    int32_t readpos = 0;
    h += y;
    do
    {
      uint32_t idx = 0;
      do
      {
        readbuf[idx] = _read_pixel(x + idx, y) ? -1 : 0;
      } while (++idx != w);
      param->src_x32 = 0;
      readpos = param->fp_copy(dst, readpos, readpos + w, param);
    } while (++y < h);
  }


  void Panel_SharpLCD::_draw_pixel(uint8_t* dst, uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    _rotate_pos(x, y);
    if( _config_detail.enable_dithering ) {

      swap565_t color;
      color.raw = rawcolor;
      uint32_t value = to_gray(color.R8(), color.G8(), color.B8());

      auto btbl = &Bayer[((y + (_bayer_offset >> 2)) & 3) << 2];
      uint32_t ms = y * _pitch;
      uint32_t idx = ms + (x >> 3);
      uint32_t mask = 1 << (x & 7);
      bool flg = 256 <= value + btbl[(x + _bayer_offset) & 3];
      if (flg) dst[idx] |=   mask;
      else     dst[idx] &= ~ mask;

    } else {

      if (rawcolor) {
        dst[(y * _cfg.memory_width + x) >> 3] |= set[x & 7];
      } else {
        dst[(y * _cfg.memory_width + x) >> 3] &= clr[x & 7];
      }

    }
  }


  bool Panel_SharpLCD::_read_pixel(uint_fast16_t x, uint_fast16_t y)
  {
    return _buf[(y * _cfg.memory_width + x) >> 3] & set[x & 7] ? 1 : 0;
  }


  void Panel_SharpLCD::_update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye)
  {
    _range_mod.left   = std::min<int32_t>(xs, _range_mod.left);
    _range_mod.right  = std::max<int32_t>(xe, _range_mod.right);
    _range_mod.top    = std::min<int32_t>(ys, _range_mod.top);
    _range_mod.bottom = std::max<int32_t>(ye, _range_mod.bottom);
  }


//----------------------------------------------------------------------------
}
}
