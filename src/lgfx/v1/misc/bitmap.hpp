/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

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
#pragma once

#include "DataWrapper.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct bitmap_header_t
  {
#pragma pack(push)
#pragma pack(1)
    union
    {
      uint8_t raw[54];
      struct {
        uint16_t bfType;
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;

        uint32_t biSize;
        int32_t  biWidth;
        int32_t  biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t  biXPelsPerMeter;
        int32_t  biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
      };
    };
#pragma pack(pop)

    bool load_bmp_header(DataWrapper* data)
    {
      data->read((uint8_t*)this, sizeof(bitmap_header_t));
      return ( (bfType == 0x4D42)   // bmp header "BM"
            && (biPlanes == 1)  // bcPlanes always 1
            && (biWidth > 0)
            && (biHeight != 0)
            && (biBitCount <= 32)
            && (biBitCount != 0)
            && (biCompression != 1 || biBitCount == 8)   // BI_RLE8 is 8 bpp only
            && (biCompression != 2 || biBitCount == 4)); // BI_RLE4 is 4 bpp only
    }

    // The callers size the line buffer from biBitCount, so an RLE mode that does not
    // match it (see load_bmp_header) would be written past the end of that buffer.
    static bool load_bmp_rle8(DataWrapper* data, uint8_t* linebuf, uint_fast16_t width)
    {
      width = (width + 3) & ~3;
      uint8_t code[2];
      uint_fast16_t xidx = 0;
      bool eol = false;
      do {
        if (data->read(code, 2) != 2) return false; // truncated file
        if (code[0] == 0) {
          switch (code[1]) {
          case 0x00: // EOL
          case 0x01: // EOB
            eol = true;
            break;

          case 0x02: // move info  (not support)
            return false;

          default:
            if (xidx + code[1] > width) return false;
            if (data->read(&linebuf[xidx], code[1]) != code[1]) return false;
            if (code[1] & 1) data->skip(1); // word align
            xidx += code[1];
            break;
          }
        } else if (xidx + code[0] <= width) {
          memset(&linebuf[xidx], code[1], code[0]);
          xidx += code[0];
        } else {
          return false;
        }
      } while (!eol);
      return true;
    }

    // Store the 4-bit pixel number x of a row, leaving its neighbour in the same byte alone.
    static void put_nibble(uint8_t* linebuf, uint_fast16_t x, uint8_t v)
    {
      uint8_t* p = &linebuf[x >> 1];
      *p = (x & 1) ? ((*p & 0xF0) | v) : ((*p & 0x0F) | (v << 4));
    }

    static bool load_bmp_rle4(DataWrapper* data, uint8_t* linebuf, uint_fast16_t width)
    {
      width = (width + 3) & ~3;
      uint8_t code[2];
      uint_fast16_t xidx = 0;
      bool eol = false;
      do {
        if (data->read(code, 2) != 2) return false; // truncated file
        if (code[0] == 0) {
          switch (code[1]) {
          case 0x00: // EOL
          case 0x01: // EOB
            eol = true;
            break;

          case 0x02: // move info  (not support)
            return false;

          default:  // 絶対モードデータ;
            {
              uint_fast16_t len = code[1];
              if (xidx + len > width) return false;
              uint_fast16_t dbyte = (len + 1) >> 1;
              uint8_t src[128]; // an absolute run is at most 255 pixels
              if (data->read(src, dbyte) != (int)dbyte) return false;
              if (dbyte & 1) data->skip(1); // word align
              for (uint_fast16_t i = 0; i < len; ++i) {
                put_nibble(linebuf, xidx + i, (i & 1) ? (src[i >> 1] & 0x0F) : (src[i >> 1] >> 4));
              }
              xidx += len;
            }
            break;
          }
        } else if (xidx + code[0] <= width) {
          for (uint_fast16_t i = 0; i < code[0]; ++i) {
            put_nibble(linebuf, xidx + i, (i & 1) ? (code[1] & 0x0F) : (code[1] >> 4));
          }
          xidx += code[0];
        } else {
          return false;
        }
      } while (!eol);
      return true;
    }
  };

//----------------------------------------------------------------------------
 }
}
