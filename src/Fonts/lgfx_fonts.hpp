#include <stdint.h>

namespace lgfx
{
  enum textdatum_t : uint8_t
  //  0:left   1:centre   2:right
  //  0:top    4:middle   8:bottom   16:baseline
  { top_left        =  0  // Top left (default)
  , top_center      =  1  // Top center
  , top_centre      =  1  // Top center
  , top_right       =  2  // Top right
  , middle_left     =  4  // Middle left
  , middle_center   =  5  // Middle center
  , middle_centre   =  5  // Middle center
  , middle_right    =  6  // Middle right
  , bottom_left     =  8  // Bottom left
  , bottom_center   =  9  // Bottom center
  , bottom_centre   =  9  // Bottom center
  , bottom_right    = 10  // Bottom right
  , baseline_left   = 16  // Baseline left (Line the 'A' character would sit on)
  , baseline_center = 17  // Baseline center
  , baseline_centre = 17  // Baseline center
  , baseline_right  = 18  // Baseline right
  };

  enum font_type_t
  { ft_unknown
  , ft_glcd
  , ft_bmp
  , ft_rle
  , ft_gfx
  , ft_bdf
  , ft_vlw
  };

  struct FontStyle {
    uint32_t back_rgb888 = 0;
    uint32_t fore_rgb888 = 0xFFFFFFU;
    int_fast8_t size_x = 1;
    int_fast8_t size_y = 1;
    textdatum_t datum = textdatum_t::top_left;
    bool utf8 = true;
    bool cp437 = false;
  };

  struct font_size_t {
    int16_t x_size;
    int16_t x_advance;
    int16_t x_offset;
    int16_t y_size;
    int16_t y_advance;
    int16_t y_offset;
    int16_t baseline;
  };

  struct IFont
  {
    virtual ~IFont() {}
    virtual font_type_t getType(void) const { return ft_unknown; }
    virtual void getDefaultMetric(font_size_t *sizedata) const = 0;
    virtual bool updateFontMetric(font_size_t *sizedata, uint16_t uniCode) const = 0;
    virtual bool unloadFont(void) { return false; }
  };

  struct BaseFont : public IFont {
    const uint8_t *chartbl;
    const uint8_t *widthtbl;
    const uint8_t width;
    const uint8_t height;
    const uint8_t baseline;
    BaseFont() = default;
    BaseFont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline)
     : chartbl  (chartbl  )
     , widthtbl (widthtbl )
     , width    (width    )
     , height   (height   )
     , baseline (baseline )
    {}
    void getDefaultMetric(font_size_t *sizedata) const override {
      sizedata->x_size    = width;
      sizedata->x_advance = width;
      sizedata->x_offset  = 0;
      sizedata->y_size    = height;
      sizedata->y_advance = height;
      sizedata->y_offset  = 0;
      sizedata->baseline  = baseline;
    }
  };

  struct GLCDfont : public BaseFont {
    GLCDfont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_glcd; }

    bool updateFontMetric(font_size_t *sizedata, uint16_t uniCode) const override {
      return uniCode < 256;
    }
  };

  struct BMPfont : public BaseFont {
    BMPfont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_bmp;  } 

    bool updateFontMetric(font_size_t *sizedata, uint16_t uniCode) const override {
      if ((uniCode -= 32) >= 96) return false;
      sizedata->x_advance = sizedata->x_size = pgm_read_byte( (uint8_t *)pgm_read_dword( &widthtbl ) + uniCode );
      return true;
    }
  };

  struct RLEfont : public BMPfont {
    RLEfont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline) : BMPfont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_rle;  }
  };

  struct BDFfont : public BaseFont {
    const uint16_t *indextbl;
    uint16_t indexsize;
    uint8_t halfwidth;
    BDFfont() = default;
    BDFfont(const uint8_t *chartbl, const uint16_t *indextbl, uint16_t indexsize, uint8_t width, uint8_t halfwidth, uint8_t height, uint8_t baseline) 
     : BaseFont(chartbl, nullptr, width, height, baseline )
     , indextbl(indextbl)
     , indexsize(indexsize)
     , halfwidth(halfwidth)
     {}
    font_type_t getType(void) const override { return ft_bdf;  }

    bool updateFontMetric(font_size_t *sizedata, uint16_t uniCode) const override {
      sizedata->x_advance = sizedata->x_size = pgm_read_byte( (uniCode < 0x0100) ? &halfwidth : &width );
      return true;
    }
  };

  struct VLWfont : public IFont {
    uint16_t gCount;     // Total number of characters
    uint16_t yAdvance;   // Line advance
    uint16_t spaceWidth; // Width of a space character
    int16_t  ascent;     // Height of top of 'd' above baseline, other characters may be taller
    int16_t  descent;    // Offset to bottom of 'p', other characters may have a larger descent
    uint16_t maxAscent;  // Maximum ascent found in font
    uint16_t maxDescent; // Maximum descent found in font

    // These are for the metrics for each individual glyph (so we don't need to seek this in file and waste time)
    uint16_t* gUnicode  = nullptr;  //UTF-16 code, the codes are searched so do not need to be sequential
    uint8_t*  gWidth    = nullptr;  //cwidth
    uint8_t*  gxAdvance = nullptr;  //setWidth
    int8_t*   gdX       = nullptr;  //leftExtent
    uint32_t* gBitmap   = nullptr;  //file pointer to greyscale bitmap

    DataWrapper* _fontData;
    bool _fontLoaded = false; // Flags when a anti-aliased font is loaded

    font_type_t getType(void) const override { return ft_vlw;  } 

    void getDefaultMetric(font_size_t *sizedata) const override {
      sizedata->x_offset  = 0;
      sizedata->y_offset  = 0;
      sizedata->baseline  = maxAscent;
      sizedata->y_advance = yAdvance;
      sizedata->y_size    = yAdvance;
    }

    virtual ~VLWfont() {
      unloadFont();
    }

    bool unloadFont(void) override {
      _fontLoaded = false;
      if (_fontData) { _fontData->close();   _fontData = nullptr; }
      if (gUnicode)  { heap_free(gUnicode);  gUnicode  = nullptr; }
      if (gWidth)    { heap_free(gWidth);    gWidth    = nullptr; }
      if (gxAdvance) { heap_free(gxAdvance); gxAdvance = nullptr; }
      if (gdX)       { heap_free(gdX);       gdX       = nullptr; }
      if (gBitmap)   { heap_free(gBitmap);   gBitmap   = nullptr; }
      return true;
    }

    bool getUnicodeIndex(uint16_t unicode, uint16_t *index) const
    {
      auto poi = std::lower_bound(gUnicode, &gUnicode[gCount], unicode);
      *index = std::distance(gUnicode, poi);
      return (*poi == unicode);
    }

    bool updateFontMetric(font_size_t *sizedata, uint16_t uniCode) const override {
      uint16_t gNum = 0;
      if (getUnicodeIndex(uniCode, &gNum)) {
        if (gWidth && gxAdvance && gdX[gNum]) {
          sizedata->x_size    = gWidth[gNum];
          sizedata->x_advance = gxAdvance[gNum];
          sizedata->x_offset  = gdX[gNum];
        } else {
          auto file = _fontData;

          file->preRead();
          //if (file->need_transaction && me->_transaction_count) me->endTransaction();

          file->seek(28 + gNum * 28);  // headerPtr
          uint32_t buffer[6];
          file->read((uint8_t*)buffer, 24);
          sizedata->x_size    = __builtin_bswap32(buffer[1]); // Width of glyph
          sizedata->x_advance = __builtin_bswap32(buffer[2]); // xAdvance - to move x cursor
          sizedata->x_offset  = (int32_t)((int8_t)__builtin_bswap32(buffer[4])); // x delta from cursor

          file->postRead();
          //if (file->need_transaction && me->_transaction_count) me->beginTransaction();
        }
        return true;
      }
      return false;
    }


    bool loadFont(DataWrapper* data) {
      _fontData = data;
      {
        uint32_t buf[6];
        data->read((uint8_t*)buf, 6 * 4); // 24 Byte read

        gCount   = __builtin_bswap32(buf[0]); // glyph count in file
                 //__builtin_bswap32(buf[1]); // vlw encoder version - discard
        yAdvance = __builtin_bswap32(buf[2]); // Font size in points, not pixels
                 //__builtin_bswap32(buf[3]); // discard
        ascent   = __builtin_bswap32(buf[4]); // top of "d"
        descent  = __builtin_bswap32(buf[5]); // bottom of "p"
      }

      // These next gFont values might be updated when the Metrics are fetched
      maxAscent  = ascent;   // Determined from metrics
      maxDescent = descent;  // Determined from metrics
      yAdvance   = std::max((int)yAdvance, ascent + descent);
      spaceWidth = yAdvance * 2 / 7;  // Guess at space width

//ESP_LOGI("LGFX", "ascent:%d  descent:%d", gFont.ascent, gFont.descent);

      if (!gCount) return false;

//ESP_LOGI("LGFX", "font count:%d", gCount);

      uint32_t bitmapPtr = 24 + (uint32_t)gCount * 28;

      gBitmap   = (uint32_t*)heap_alloc_psram( gCount * 4); // seek pointer to glyph bitmap in the file
      gUnicode  = (uint16_t*)heap_alloc_psram( gCount * 2); // Unicode 16 bit Basic Multilingual Plane (0-FFFF)
      gWidth    =  (uint8_t*)heap_alloc_psram( gCount );    // Width of glyph
      gxAdvance =  (uint8_t*)heap_alloc_psram( gCount );    // xAdvance - to move x cursor
      gdX       =   (int8_t*)heap_alloc_psram( gCount );    // offset for bitmap left edge relative to cursor X

      if (!gUnicode
       || !gBitmap
       || !gWidth
       || !gxAdvance
       || !gdX) {
//ESP_LOGE("LGFX", "can not alloc font table");
        return false;
      }

      _fontLoaded = true;

      size_t gNum = 0;
      _fontData->seek(24);  // headerPtr
      uint32_t buffer[7];
      do {
        _fontData->read((uint8_t*)buffer, 7 * 4); // 28 Byte read
        uint16_t unicode = __builtin_bswap32(buffer[0]); // Unicode code point value
        uint32_t w = (uint8_t)__builtin_bswap32(buffer[2]); // Width of glyph
        if (gUnicode)   gUnicode[gNum]  = unicode;
        if (gWidth)     gWidth[gNum]    = w;
        if (gxAdvance)  gxAdvance[gNum] = (uint8_t)__builtin_bswap32(buffer[3]); // xAdvance - to move x cursor
        if (gdX)        gdX[gNum]       =  (int8_t)__builtin_bswap32(buffer[5]); // x delta from cursor

        uint16_t height = __builtin_bswap32(buffer[1]); // Height of glyph
        if ((unicode > 0xFF) || ((unicode > 0x20) && (unicode < 0xA0) && (unicode != 0x7F))) {
          int16_t dY =  (int16_t)__builtin_bswap32(buffer[4]); // y delta from baseline
//ESP_LOGI("LGFX", "unicode:%x  dY:%d", unicode, dY);
          if (maxAscent < dY) {
            maxAscent = dY;
          }
          if (maxDescent < (height - dY)) {
//ESP_LOGI("LGFX", "maxDescent:%d", maxDescent);
            maxDescent = height - dY;
          }
        }

        if (gBitmap)  gBitmap[gNum] = bitmapPtr;
        bitmapPtr += w * height;
      } while (++gNum < gCount);

      yAdvance = maxAscent + maxDescent;

//ESP_LOGI("LGFX", "maxDescent:%d", maxDescent);
      return true;
    }
  };
}

#include "glcdfont.h"
#include "Font16.h"
#include "Font32rle.h"
#include "Font64rle.h"
#include "Font7srle.h"
#include "Font72rle.h"
#include "gfxfont.hpp"

namespace fonts {
  static PROGMEM const lgfx::GLCDfont Font0 = { (const uint8_t *)font, nullptr, 6, 8, 7};
  static PROGMEM const lgfx::BMPfont  Font2 = { (const uint8_t *)chrtbl_f16, widtbl_f16, 0, chr_hgt_f16, baseline_f16 };
  static PROGMEM const lgfx::RLEfont  Font4 = { (const uint8_t *)chrtbl_f32, widtbl_f32, 0, chr_hgt_f32, baseline_f32 };
  static PROGMEM const lgfx::RLEfont  Font6 = { (const uint8_t *)chrtbl_f64, widtbl_f64, 0, chr_hgt_f64, baseline_f64 };
  static PROGMEM const lgfx::RLEfont  Font7 = { (const uint8_t *)chrtbl_f7s, widtbl_f7s, 0, chr_hgt_f7s, baseline_f7s };
  static PROGMEM const lgfx::RLEfont  Font8 = { (const uint8_t *)chrtbl_f72, widtbl_f72, 0, chr_hgt_f72, baseline_f72 };

#ifdef __EFONT_FONT_DATA_H__
  static PROGMEM const lgfx::BDFfont efont = { (const uint8_t *)efontFontData, efontFontList, sizeof(efontFontList)>>1, 16, 8, 16, 14 };
#endif

#ifdef misakiUTF16FontData_h
  static PROGMEM const lgfx::BDFfont misaki = { (const uint8_t *)fdata, ftable, sizeof(ftable)>>1, 8, 4, 7, 6 };
#endif
}


