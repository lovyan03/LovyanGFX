#include "lgfx_fonts.hpp"

#include "../lgfx/lgfx_common.hpp"
#include "../lgfx/LGFXBase.hpp"

#include <algorithm>
#include <cstdint>
#include <cstddef>

#ifndef PROGMEM
#define PROGMEM
#endif

namespace lgfx {
  void BaseFont::getDefaultMetric(FontMetrics *metrics) const
  {
    metrics->width    = width;
    metrics->x_advance = width;
    metrics->x_offset  = 0;
    metrics->height    = height;
    metrics->y_advance = height;
    metrics->y_offset  = 0;
    metrics->baseline  = baseline;
  }
  void BDFfont::getDefaultMetric(FontMetrics *metrics) const
  {
    BaseFont::getDefaultMetric(metrics);
    metrics->y_advance = y_advance;
  }

  bool GLCDfont::updateFontMetric(FontMetrics*, std::uint16_t uniCode) const {
    return uniCode < 256;
  }

  std::size_t GLCDfont::drawChar(LGFXBase* me, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const
  {
    if (c > 255) return 0;

    if (!style->cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

    const std::int32_t fontWidth  = this->width;
    const std::int32_t fontHeight = this->height;

    auto font_addr = this->chartbl + (c * 5);
    std::uint32_t colortbl[2] = {me->getColorConverter()->convert(style->back_rgb888), me->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);

    //std::int32_t clip_left   = me->_clip_l;
    //std::int32_t clip_right  = me->_clip_r;
    //std::int32_t clip_top    = me->_clip_t;
    //std::int32_t clip_bottom = me->_clip_b;

    float sy = style->size_y;
    float sx = style->size_x;
    //if ((x <= clip_right) && (clip_left < (x + fontWidth * sx ))
    // && (y <= clip_bottom) && (clip_top < (y + fontHeight * sy )))
    {
//      if (!fillbg || style->size_y != 1.0 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * sx > clip_right) {
        std::int32_t x1 = sx;
        std::int32_t x0 = 0;
        me->startWrite();

        std::int_fast8_t i = 0;
        do {
          std::uint8_t line = font_addr[i];
          std::uint8_t flg = (line & 0x01);
          std::int_fast8_t j = 1;
          std::int_fast16_t y0 = 0;
          std::int_fast16_t y1 = 0;
          do {
            while (flg == ((line >> j) & 0x01) && ++j < fontHeight);
            y1 = j * sy;
            if (flg || fillbg) {
              me->setRawColor(colortbl[flg]);
              me->writeFillRect(x + x0, y + y0, x1 - x0, y1 - y0);
            }
            y0 = y1;
            flg = !flg;
          } while (j < fontHeight);
          x0 = x1;
          x1 = (++i + 1) * sx;
        } while (i < fontWidth - 1);

        if (fillbg) {
          me->setRawColor(colortbl[0]);
          me->writeFillRect(x + x0, y, x1 - x0, fontHeight * style->size_y); 
        }
        me->endWrite();
/*
      } else {
        std::uint8_t col[fontWidth];
        std::int_fast8_t i = 0;
        do {
          col[i] = font_addr[i];
        } while (++i < 5);
        col[5] = 0;
        me->startWrite();
        me->setAddrWindow(x, y, fontWidth * style->size_x, fontHeight);
        std::uint8_t flg = col[0] & 1;
        std::uint32_t len = 0;
        i = 0;
        do {
          std::int_fast8_t j = 0;
          do {
            if (flg != ((col[j] >> i) & 1)) {
              me->writeRawColor(colortbl[flg], len);
              len = 0;
              flg = !flg;
            }
            len += style->size_x;
          } while (++j < fontWidth);
        } while (++i < fontHeight);
        me->writeRawColor(colortbl[0], len);
        me->endWrite();
      }
//*/
    }
    return fontWidth * sx;
  }

  static std::size_t draw_char_bmp(LGFXBase* me, std::int32_t x, std::int32_t y, const TextStyle* style, const std::uint8_t* font_addr, std::int_fast8_t fontWidth, std::int_fast8_t fontHeight, std::int_fast8_t w, std::int_fast8_t margin )
  {
    std::uint32_t colortbl[2] = {me->getColorConverter()->convert(style->back_rgb888), me->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);

    //std::int32_t clip_left   = me->_clip_l;
    //std::int32_t clip_right  = me->_clip_r;
    //std::int32_t clip_top    = me->_clip_t;
    //std::int32_t clip_bottom = me->_clip_b;

    float sx = style->size_x;
    std::int32_t sh = fontHeight * style->size_y;

    //if ((x <= clip_right) && (clip_left < (x + fontWidth * sx ))
    // && (y <= clip_bottom) && (clip_top < (y + sh )))
    {
//      if (!fillbg || sy != 1 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * sx > clip_right) {
        me->startWrite();
        if (fillbg) {
          me->setRawColor(colortbl[0]);
          if (margin) {
            std::int32_t x0 = (fontWidth - margin) * sx;
            std::int32_t x1 = (fontWidth         ) * sx;
            if (x0 < x1) {
              me->writeFillRect(x + x0, y, x1 - x0, sh);
            }
          }
        }
        std::int32_t i = 0;
        std::int32_t y1 = 0;
        std::int32_t y0 = - 1;
        do {
          bool fill = y0 != y1;
          y0 = y1;
          y1 = ++i * sh / fontHeight;
          std::uint8_t line = font_addr[0];
          bool flg = line & 0x80;
          std::int_fast8_t j = 1;
          std::int_fast8_t je = fontWidth - margin;
          std::int32_t x0 = 0;
          do {
            do {
              if (0 == (j & 7)) line = font_addr[j >> 3];
            } while (flg == (bool)(line & (0x80) >> (j&7)) && ++j < je);
            std::int32_t x1 = j * sx;
            if (flg || (fillbg && fill)) {
              me->setRawColor(colortbl[flg]);
              if (flg && x1 == std::int32_t((j-1)*sx)) ++x1;
              me->writeFillRect(x + x0, y + y0, x1 - x0, std::max<std::int32_t>(1, y1 - y0));
            }
            x0 = x1;
            flg = !flg;
          } while (j < je);
          font_addr += w;
        } while (i < fontHeight);
        me->endWrite();
/*
      } else {
        std::int_fast8_t len = 0;
        std::uint8_t line = 0;
        bool flg = false;
        me->startWrite();
        me->setAddrWindow(x, y, fontWidth * style->size_x, fontHeight);
        std::int_fast8_t i = 0;
        std::int_fast8_t je = fontWidth - margin;
        do {
          std::int_fast8_t j = 0;
          do {
            if (j & 7) {
              line <<= 1;
            } else {
              line = (j == je) ? 0 : font_addr[j >> 3];
            }
            if (flg != (bool)(line & 0x80)) {
              me->writeRawColor(colortbl[flg], len);
              flg = !flg;
              len = 0;
            }
            len += style->size_x;
          } while (++j < fontWidth);
          font_addr += w;
        } while (++i < fontHeight);
        me->writeRawColor(colortbl[flg], len);
        me->endWrite();
      }
//*/
    }

    return fontWidth * sx;
  }


  bool BMPfont::updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const {
    if ((uniCode -= 32) >= 96) return false;
    metrics->x_advance = metrics->width = widthtbl[uniCode];
    return true;
  }

  bool BDFfont::updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const {
    metrics->x_advance = metrics->width = (uniCode < 0x0100) ? halfwidth : width;
    return true;
  }

  std::size_t BMPfont::drawChar(LGFXBase* me, std::int32_t x, std::int32_t y, std::uint16_t uniCode, const TextStyle* style) const
  { // BMP font
    if ((uniCode -= 0x20) >= 96) return 0;
    const std::int_fast8_t fontWidth = this->widthtbl[uniCode];
    const std::int_fast8_t fontHeight = this->height;

    auto font_addr = ((const std::uint8_t**)this->chartbl)[uniCode];
    return draw_char_bmp(me, x, y, style, font_addr, fontWidth, fontHeight, (fontWidth + 6) >> 3, 1);
  }

  std::size_t BDFfont::drawChar(LGFXBase* me, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style) const
  {
    const std::int_fast8_t bytesize = (this->width + 7) >> 3;
    const std::int_fast8_t fontHeight = this->height;
    const std::int_fast8_t fontWidth = (c < 0x0100) ? this->halfwidth : this->width;
    auto it = std::lower_bound(this->indextbl, &this->indextbl[this->indexsize], c);
    if (*it != c) {
      if (style->fore_rgb888 != style->back_rgb888) {
        me->fillRect(x, y, fontWidth * style->size_x, fontHeight * style->size_y, style->back_rgb888);
      }
      return fontWidth * style->size_x;
    }
    const std::uint8_t* font_addr = &this->chartbl[std::distance(this->indextbl, it) * fontHeight * bytesize];
    return draw_char_bmp(me, x, y, style, font_addr, fontWidth, fontHeight, bytesize, 0);
  }

  std::size_t RLEfont::drawChar(LGFXBase* me, std::int32_t x, std::int32_t y, std::uint16_t code, const TextStyle* style) const
  { // RLE font
    if ((code -= 0x20) >= 96) return 0;

    const int fontWidth = this->widthtbl[code];
    const int fontHeight = this->height;

    auto font_addr = ((const std::uint8_t**)this->chartbl)[code];

    std::uint32_t colortbl[2] = {me->getColorConverter()->convert(style->back_rgb888), me->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);

    //std::int32_t clip_left   = me->_clip_l;
    //std::int32_t clip_right  = me->_clip_r;
    //std::int32_t clip_top    = me->_clip_t;
    //std::int32_t clip_bottom = me->_clip_b;

    float sx = style->size_x;
    float sy = style->size_y;

    //if ((x <= clip_right) && (clip_left < (x + fontWidth  * sx ))
    // && (y <= clip_bottom) && (clip_top < (y + fontHeight * sy )))
    {
//      if (!fillbg || sy != 1.0 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * sx > clip_right) {
        bool flg = false;
        std::uint8_t line = 0, i = 0, j = 0;
        std::int32_t len;
        std::int32_t y0 = 0;
        std::int32_t y1 = sy;
        std::int32_t x0 = 0;
        me->startWrite();
        do {
          line = *font_addr++;
          flg = line & 0x80;
          line = (line & 0x7F)+1;
          do {
            len = (j + line > fontWidth) ? fontWidth - j : line;
            line -= len;
            j += len;
            std::int32_t x1 = j * sx;
            if (fillbg || flg) {
              me->setRawColor(colortbl[flg]);
              me->writeFillRect( x + x0, y + y0, x1 - x0, y1 - y0);
            }
            x0 = x1;
            if (j == fontWidth) {
              j = 0;
              x0 = 0;
              y0 = y1;
              y1 = (++i + 1) * sy;
            }
          } while (line);
        } while (i < fontHeight);
        me->endWrite();
/*
      } else {
        std::uint32_t line = 0;
        me->startWrite();
        me->setAddrWindow(x, y, fontWidth * style->size_x, fontHeight);
        std::uint32_t len = fontWidth * style->size_x * fontHeight;
        do {
          line = *font_addr++;
          bool flg = line & 0x80;
          line = ((line & 0x7F) + 1) * style->size_x;
          me->writeRawColor(colortbl[flg], line);
        } while (len -= line);
        me->endWrite();
      }
//*/
    }

    return fontWidth * sx;
  }


//----------------------------------------------------------------------------

  bool GFXfont::updateFontMetric(lgfx::FontMetrics *metrics, std::uint16_t uniCode) const {
    auto glyph = getGlyph(uniCode);
    if (!glyph) return false;
    metrics->x_offset  = glyph->xOffset;
    metrics->width     = glyph->width;
    metrics->x_advance = glyph->xAdvance;
    return true;
  }

  GFXglyph* GFXfont::getGlyph(std::uint16_t uniCode) const {
    if (uniCode > last 
    ||  uniCode < first) return nullptr;
    std::uint16_t custom_range_num = range_num;
    if (custom_range_num == 0) {
      uniCode -= first;
      return &glyph[uniCode];
    }
    auto range_pst = range;
    size_t i = 0;
    while ((uniCode > range_pst[i].end) 
        || (uniCode < range_pst[i].start)) {
      if (++i == custom_range_num) return nullptr;
    }
    uniCode -= range_pst[i].start - range_pst[i].base;
    return &glyph[uniCode];
  }

  void GFXfont::getDefaultMetric(lgfx::FontMetrics *metrics) const {
    std::int_fast8_t glyph_ab = 0;   // glyph delta Y (height) above baseline
    std::int_fast8_t glyph_bb = 0;   // glyph delta Y (height) below baseline
    size_t numChars = last - first;

    size_t custom_range_num = range_num;
    if (custom_range_num != 0) {
      EncodeRange *range_pst = range;
      size_t i = 0;
      numChars = custom_range_num;
      do {
        numChars += range_pst[i].end - range_pst[i].start;
      } while (++i < custom_range_num);
    }

    // Find the biggest above and below baseline offsets
    for (size_t c = 0; c < numChars; c++)
    {
      GFXglyph *glyph1 = &glyph[c];
      std::int_fast8_t ab = -glyph1->yOffset;
      if (ab > glyph_ab) glyph_ab = ab;
      std::int_fast8_t bb = glyph1->height - ab;
      if (bb > glyph_bb) glyph_bb = bb;
    }

    metrics->baseline = glyph_ab;
    metrics->y_offset = - glyph_ab;
    metrics->height   = glyph_bb + glyph_ab;
    metrics->y_advance = yAdvance;
  }

//----------------------------------------------------------------------------

  void VLWfont::getDefaultMetric(FontMetrics *metrics) const {
    metrics->x_offset  = 0;
    metrics->y_offset  = 0;
    metrics->baseline  = maxAscent;
    metrics->y_advance = yAdvance;
    metrics->height    = yAdvance;
  }

  VLWfont::~VLWfont() {
    unloadFont();
  }

  bool VLWfont::unloadFont(void) {
    _fontLoaded = false;
    if (gUnicode)  { heap_free(gUnicode);  gUnicode  = nullptr; }
    if (gWidth)    { heap_free(gWidth);    gWidth    = nullptr; }
    if (gxAdvance) { heap_free(gxAdvance); gxAdvance = nullptr; }
    if (gdX)       { heap_free(gdX);       gdX       = nullptr; }
    if (gBitmap)   { heap_free(gBitmap);   gBitmap   = nullptr; }
    if (_fontData) {
      _fontData->preRead();
      _fontData->close();
      _fontData->postRead();
      _fontData = nullptr;
    }
    return true;
  }

  bool VLWfont::getUnicodeIndex(std::uint16_t unicode, std::uint16_t *index) const
  {
    auto poi = std::lower_bound(gUnicode, &gUnicode[gCount], unicode);
    *index = std::distance(gUnicode, poi);
    return (*poi == unicode);
  }

  bool VLWfont::updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const {
    std::uint16_t gNum = 0;
    if (getUnicodeIndex(uniCode, &gNum)) {
      if (gWidth && gxAdvance && gdX[gNum]) {
        metrics->width     = gWidth[gNum];
        metrics->x_advance = gxAdvance[gNum];
        metrics->x_offset  = gdX[gNum];
      } else {
        auto file = _fontData;

        file->preRead();

        file->seek(28 + gNum * 28);  // headerPtr
        std::uint32_t buffer[6];
        file->read((std::uint8_t*)buffer, 24);
        metrics->width    = __builtin_bswap32(buffer[1]); // Width of glyph
        metrics->x_advance = __builtin_bswap32(buffer[2]); // xAdvance - to move x cursor
        metrics->x_offset  = (std::int32_t)((std::int8_t)__builtin_bswap32(buffer[4])); // x delta from cursor

        file->postRead();
      }
      return true;
    }
    if (uniCode == 0x20) {
      metrics->width = metrics->x_advance = metrics->y_advance * 2 / 7;
      metrics->x_offset = 0;
      return true;
    }
    return false;
  }


  bool VLWfont::loadFont(DataWrapper* data) {
    _fontData = data;
    {
      std::uint32_t buf[6];
      data->read((std::uint8_t*)buf, 6 * 4); // 24 Byte read

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

    std::uint32_t bitmapPtr = 24 + (std::uint32_t)gCount * 28;

    gBitmap   = (std::uint32_t*)heap_alloc_psram( gCount * 4); // seek pointer to glyph bitmap in the file
    gUnicode  = (std::uint16_t*)heap_alloc_psram( gCount * 2); // Unicode 16 bit Basic Multilingual Plane (0-FFFF)
    gWidth    =  (std::uint8_t*)heap_alloc_psram( gCount );    // Width of glyph
    gxAdvance =  (std::uint8_t*)heap_alloc_psram( gCount );    // xAdvance - to move x cursor
    gdX       =   (std::int8_t*)heap_alloc_psram( gCount );    // offset for bitmap left edge relative to cursor X

    if (nullptr == gBitmap  ) gBitmap   = (std::uint32_t*)heap_alloc( gCount * 4); // seek pointer to glyph bitmap in the file
    if (nullptr == gUnicode ) gUnicode  = (std::uint16_t*)heap_alloc( gCount * 2); // Unicode 16 bit Basic Multilingual Plane (0-FFFF)
    if (nullptr == gWidth   ) gWidth    =  (std::uint8_t*)heap_alloc( gCount );    // Width of glyph
    if (nullptr == gxAdvance) gxAdvance =  (std::uint8_t*)heap_alloc( gCount );    // xAdvance - to move x cursor
    if (nullptr == gdX      ) gdX       =   (std::int8_t*)heap_alloc( gCount );    // offset for bitmap left edge relative to cursor X

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
    std::uint32_t buffer[7];
    do {
      _fontData->read((std::uint8_t*)buffer, 7 * 4); // 28 Byte read
      std::uint16_t unicode = __builtin_bswap32(buffer[0]); // Unicode code point value
      std::uint32_t w = (std::uint8_t)__builtin_bswap32(buffer[2]); // Width of glyph
      if (gUnicode)   gUnicode[gNum]  = unicode;
      if (gWidth)     gWidth[gNum]    = w;
      if (gxAdvance)  gxAdvance[gNum] = (std::uint8_t)__builtin_bswap32(buffer[3]); // xAdvance - to move x cursor
      if (gdX)        gdX[gNum]       =  (std::int8_t)__builtin_bswap32(buffer[5]); // x delta from cursor

      std::uint16_t height = __builtin_bswap32(buffer[1]); // Height of glyph
      if ((unicode > 0xFF) || ((unicode > 0x20) && (unicode < 0xA0) && (unicode != 0x7F))) {
        std::int16_t dY =  (std::int16_t)__builtin_bswap32(buffer[4]); // y delta from baseline
//Serial.printf("LGFX:unicode:%x  dY:%d\r\n", unicode, dY);
        if (maxAscent < dY && unicode != 0x3000) {
          maxAscent = dY;
        }
        if (maxDescent < (height - dY) && unicode != 0x3000) {
//Serial.printf("LGFX:maxDescent:%d\r\n", maxDescent);
          maxDescent = height - dY;
        }
      }

      if (gBitmap)  gBitmap[gNum] = bitmapPtr;
      bitmapPtr += w * height;
    } while (++gNum < gCount);

    yAdvance = maxAscent + maxDescent;

//Serial.printf("LGFX:maxDescent:%d\r\n", maxDescent);
    return true;
  }

//----------------------------------------------------------------------------

  std::size_t GFXfont::drawChar(LGFXBase* me, std::int32_t x, std::int32_t y, std::uint16_t uniCode, const TextStyle* style) const
  {
    auto glyph = this->getGlyph(uniCode);
    if (!glyph) return 0;

    std::int32_t w = glyph->width,
                  h = glyph->height;

    float sx = style->size_x;
    float sy = style->size_y;

    std::int32_t xAdvance = sx * glyph->xAdvance;
    std::int32_t xoffset  = sx * glyph->xOffset;

    me->startWrite();
    std::uint32_t colortbl[2] = {me->getColorConverter()->convert(style->back_rgb888), me->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);
    std::int32_t left  = 0;
    std::int32_t right = 0;
    if (fillbg) {
      left  = std::max<int>(me->_filled_x, x + (xoffset < 0 ? xoffset : 0));
      right = x + std::max<int>(w * sx + xoffset, xAdvance);
      me->setRawColor(colortbl[0]);
    }

    x += xoffset;
    y += int(me->_font_metrics.y_offset * sy);
    std::int32_t yoffset = (- me->_font_metrics.y_offset) + glyph->yOffset;

    //std::int32_t clip_left   = me->_clip_l;
    //std::int32_t clip_right  = me->_clip_r;
    //std::int32_t clip_top    = me->_clip_t;
    //std::int32_t clip_bottom = me->_clip_b;

    //if ((x <= clip_right) && (clip_left < (x + w * sx ))
    // && (y <= clip_bottom) && (clip_top < (y + h * sy )))
    {

      if (left < right) {
        if (yoffset > 0) {
          me->writeFillRect(left, y, right - left, yoffset * sy);
        }
        std::int32_t y0 = (yoffset + h) * sy;
        std::int32_t y1 = me->_font_metrics.height * sy;
        if (y0 < y1) {
          me->writeFillRect(left, y + y0, right - left, y1 - y0);
        }
      }

      std::uint8_t *bitmap = &this->bitmap[glyph->bitmapOffset];
      std::uint8_t mask=0x80;

      me->setRawColor(colortbl[1]);
      std::int_fast8_t i = 0;
      std::int32_t y1 = yoffset * sy;
      std::int32_t y0 = y1 - 1;
      do {
        bool fill = y0 != y1;
        y0 = y1;
        y1 = (++i + yoffset) * sy;
        std::int32_t fh = y1 - y0;
        if (!fh) fh = 1;
        if (left < right && fill) {
          me->setRawColor(colortbl[0]);
          me->writeFillRect(left, y + y0, right - left, fh);
          me->setRawColor(colortbl[1]);
        }

        std::int32_t j = 0;
        std::int32_t x0 = 0;
        bool flg = false;
        do {
          do {
            if (flg != (bool)(*bitmap & mask)) break;
            if (! (mask >>= 1)) {
              mask = 0x80;
              ++bitmap;
            }
          } while (++j < w);
          std::int32_t x1 = j * sx;
          if (flg) {
            std::int32_t fw = (x0 < x1) ? x1 - x0 : 1;
            me->writeFillRect(x + x0, y + y0, fw, fh);
          }
          x0 = x1;
          flg = !flg;
        } while (j < w);
      } while (i < h);
    //} else {
    //  if (left < right) {
    //    me->writeFillRect(left, y, right - left, (me->_font_metrics.height) * sy);
    //  }
    }
    me->_filled_x = right;
    me->endWrite();
    return xAdvance;
  }

  std::size_t VLWfont::drawChar(LGFXBase* lgfxbase, std::int32_t x, std::int32_t y, std::uint16_t code, const TextStyle* style) const
  {
    auto me = (LGFXBase*)lgfxbase;
    auto file = this->_fontData;

    std::uint32_t buffer[6] = {0};
    std::uint16_t gNum = 0;

    if (code == 0x20) {
      gNum = 0xFFFF;
      buffer[2] = __builtin_bswap32(this->spaceWidth);
    } else if (!this->getUnicodeIndex(code, &gNum)) {
      return 0;
    } else {
      file->preRead();
      file->seek(28 + gNum * 28);
      file->read((std::uint8_t*)buffer, 24);
      file->seek(this->gBitmap[gNum]);
    }


    std::int32_t h        = __builtin_bswap32(buffer[0]); // Height of glyph
    std::int32_t w        = __builtin_bswap32(buffer[1]); // Width of glyph
    float sx = style->size_x;
    std::int32_t xAdvance = __builtin_bswap32(buffer[2]) * sx; // xAdvance - to move x cursor
    std::int32_t xoffset   = (std::int32_t)((std::int8_t)__builtin_bswap32(buffer[4])) * sx; // x delta from cursor
    std::int32_t dY        = (std::int16_t)__builtin_bswap32(buffer[3]); // y delta from baseline
    float sy = style->size_y;
    std::int32_t yoffset = (this->maxAscent - dY);
//      std::int32_t yoffset = (me->_font_metrics.y_offset) - dY;

    std::uint8_t pbuffer[w * h];
    std::uint8_t* pixel = pbuffer;
    if (gNum != 0xFFFF) {
      file->read(pixel, w * h);
      file->postRead();
    }

    me->startWrite();

    std::uint32_t colortbl[2] = {me->getColorConverter()->convert(style->back_rgb888), me->getColorConverter()->convert(style->fore_rgb888)};
    bool fillbg = (style->back_rgb888 != style->fore_rgb888);
    std::int32_t left  = 0;
    std::int32_t right = 0;
    if (fillbg) {
      left  = std::max(me->_filled_x, x + (xoffset < 0 ? xoffset : 0));
      right = x + std::max<int>(w * sx + xoffset, xAdvance);
    }
    me->_filled_x = right;

    x += xoffset;
    y += int(me->_font_metrics.y_offset * sy);

    std::int32_t bx = x;
    std::int32_t bw = w * sx;
    std::int32_t clip_left = me->_clip_l;
    if (x < clip_left) { bw += (x - clip_left); bx = clip_left; }

    std::int32_t clip_right = me->_clip_r;
    if (bw > clip_right+1 - bx) bw = clip_right+1 - bx;

    if (bw >= 0)
    {
      std::int32_t fore_r = ((style->fore_rgb888>>16)&0xFF);
      std::int32_t fore_g = ((style->fore_rgb888>> 8)&0xFF);
      std::int32_t fore_b = ((style->fore_rgb888)    &0xFF);

      if (fillbg || !me->isReadable() || me->hasPalette())
      { // fill background mode  or unreadable panel  or palette sprite mode
        if (left < right && fillbg) {
          me->setRawColor(colortbl[0]);
          if (yoffset > 0) {
            me->writeFillRect(left, y, right - left, yoffset * sy);
          }
          std::int32_t y0 = (yoffset + h) * sy;
          std::int32_t y1 = me->_font_metrics.height * sy;
          if (y0 < y1) {
            me->writeFillRect(left, y + y0, right - left, y1 - y0);
          }
        }

        if (0 < w) {
          uint32_t back = fillbg ? style->back_rgb888 : me->_base_rgb888;
          std::int32_t back_r = ((back>>16)&0xFF);
          std::int32_t back_g = ((back>> 8)&0xFF);
          std::int32_t back_b = ( back     &0xFF);
          std::int32_t i = 0;
          std::int32_t y0, y1 = yoffset * sy;
          do {
            y0 = y1;
            if (y0 > me->_clip_b) break;
            y1 = (yoffset + i + 1) * sy;
            if (left < right) {
              me->setRawColor(colortbl[0]);
              me->writeFillRect(left, y + y0, right - left, y1 - y0);
            }
            std::int32_t j = 0;
            do {
              std::int32_t x0 = j * sx;
              while (pixel[j] != 0xFF) {
                std::int32_t x1 =(j+1)* sx;
                if (pixel[j] != 0 && x0 < x1) {
                  std::int32_t p = 1 + (std::uint32_t)pixel[j];
                  me->setColor(color888( ( fore_r * p + back_r * (257 - p)) >> 8
                                        , ( fore_g * p + back_g * (257 - p)) >> 8
                                        , ( fore_b * p + back_b * (257 - p)) >> 8 ));
                  me->writeFillRect(x + x0, y + y0, x1 - x0, y1 - y0);
                }
                x0 = x1;
                if (++j == w || clip_right < x0) break;
              }
              if (j == w || clip_right < x0) break;
              me->setRawColor(colortbl[1]);
              do { ++j; } while (j != w && pixel[j] == 0xFF);
              me->writeFillRect(x + x0, y + y0, (j * sx) - x0, y1 - y0);
            } while (j != w);
            pixel += w;
          } while (++i < h);
        }
      }
      else // alpha blend mode
      {
        bgr888_t buf[bw * (int)ceil(sy)];
        pixelcopy_t p(buf, me->getColorConverter()->depth, rgb888_3Byte, me->hasPalette());
        std::int32_t y0, y1 = yoffset * sy;
        std::int32_t i = 0;
        do {
          y0 = y1;
          if (y0 > me->_clip_b) break;
          y1 = (yoffset + i + 1) * sy;
          std::int32_t by = y + y0;
          std::int32_t bh = y1 - y0;

          auto ct = me->_clip_t;
          if (by < ct) { bh += by - ct; by = ct; }
          if (bh > 0) {
            std::int32_t j0 = 0;
            std::int32_t j1 = w;

            // search first and last pixel
            while (j0 != j1 && !pixel[j0    ]) { ++j0; }
            while (j0 != j1 && !pixel[j1 - 1]) { --j1; }

            if (j0 != j1) {
              std::int32_t rx = j0  * sx;
              std::int32_t rw = j1 * sx;
              if (rx < bx    -x) rx = bx    -x;
              if (rw > bx+bw -x) rw = bx+bw -x;
              rw -= rx;

              if (0 < rw) {
                me->readRectRGB(x + rx, by, rw, bh, (std::uint8_t*)buf);

                std::int32_t x0, x1 = (j0 * sx) - rx;
                do {
                  x0 = x1;
                  if (x0 < 0) x0 = 0;
                  x1 = (int)((j0+1) * sx) - rx;
                  if (x1 > rw) x1 = rw;
                  if (pixel[j0] && x0 < x1) {
                    std::int32_t p = 1 + pixel[j0];
                    do {
                      std::int32_t yy = 0;
                      do {
                        auto bgr = &buf[x0 + yy * rw];
                        bgr->r = ( fore_r * p + bgr->r * (257 - p)) >> 8;
                        bgr->g = ( fore_g * p + bgr->g * (257 - p)) >> 8;
                        bgr->b = ( fore_b * p + bgr->b * (257 - p)) >> 8;
                      } while (++yy != bh);
                    } while (++x0 != x1);
                  }
                } while (++j0 < j1);
                me->pushImage(x + rx, by, rw, bh, &p);
              }
            }
          }
          pixel += w;
        } while (++i < h);
      }
    }
    me->endWrite();
    return xAdvance;
  }

}

//----------------------------------------------------------------------------

namespace fonts {
  using namespace lgfx;

  // Original Adafruit_GFX "Free Fonts"
  #include "GFXFF/TomThumb.h"  // TT1

  #include "GFXFF/FreeMono9pt7b.h"  // FF1 or FM9
  #include "GFXFF/FreeMono12pt7b.h" // FF2 or FM12
  #include "GFXFF/FreeMono18pt7b.h" // FF3 or FM18
  #include "GFXFF/FreeMono24pt7b.h" // FF4 or FM24

  #include "GFXFF/FreeMonoOblique9pt7b.h"  // FF5 or FMO9
  #include "GFXFF/FreeMonoOblique12pt7b.h" // FF6 or FMO12
  #include "GFXFF/FreeMonoOblique18pt7b.h" // FF7 or FMO18
  #include "GFXFF/FreeMonoOblique24pt7b.h" // FF8 or FMO24

  #include "GFXFF/FreeMonoBold9pt7b.h"  // FF9  or FMB9
  #include "GFXFF/FreeMonoBold12pt7b.h" // FF10 or FMB12
  #include "GFXFF/FreeMonoBold18pt7b.h" // FF11 or FMB18
  #include "GFXFF/FreeMonoBold24pt7b.h" // FF12 or FMB24

  #include "GFXFF/FreeMonoBoldOblique9pt7b.h"  // FF13 or FMBO9
  #include "GFXFF/FreeMonoBoldOblique12pt7b.h" // FF14 or FMBO12
  #include "GFXFF/FreeMonoBoldOblique18pt7b.h" // FF15 or FMBO18
  #include "GFXFF/FreeMonoBoldOblique24pt7b.h" // FF16 or FMBO24

  // Sans serif fonts
  #include "GFXFF/FreeSans9pt7b.h"  // FF17 or FSS9
  #include "GFXFF/FreeSans12pt7b.h" // FF18 or FSS12
  #include "GFXFF/FreeSans18pt7b.h" // FF19 or FSS18
  #include "GFXFF/FreeSans24pt7b.h" // FF20 or FSS24

  #include "GFXFF/FreeSansOblique9pt7b.h"  // FF21 or FSSO9
  #include "GFXFF/FreeSansOblique12pt7b.h" // FF22 or FSSO12
  #include "GFXFF/FreeSansOblique18pt7b.h" // FF23 or FSSO18
  #include "GFXFF/FreeSansOblique24pt7b.h" // FF24 or FSSO24

  #include "GFXFF/FreeSansBold9pt7b.h"  // FF25 or FSSB9
  #include "GFXFF/FreeSansBold12pt7b.h" // FF26 or FSSB12
  #include "GFXFF/FreeSansBold18pt7b.h" // FF27 or FSSB18
  #include "GFXFF/FreeSansBold24pt7b.h" // FF28 or FSSB24

  #include "GFXFF/FreeSansBoldOblique9pt7b.h"  // FF29 or FSSBO9
  #include "GFXFF/FreeSansBoldOblique12pt7b.h" // FF30 or FSSBO12
  #include "GFXFF/FreeSansBoldOblique18pt7b.h" // FF31 or FSSBO18
  #include "GFXFF/FreeSansBoldOblique24pt7b.h" // FF32 or FSSBO24

  // Serif fonts
  #include "GFXFF/FreeSerif9pt7b.h"  // FF33 or FS9
  #include "GFXFF/FreeSerif12pt7b.h" // FF34 or FS12
  #include "GFXFF/FreeSerif18pt7b.h" // FF35 or FS18
  #include "GFXFF/FreeSerif24pt7b.h" // FF36 or FS24

  #include "GFXFF/FreeSerifItalic9pt7b.h"  // FF37 or FSI9
  #include "GFXFF/FreeSerifItalic12pt7b.h" // FF38 or FSI12
  #include "GFXFF/FreeSerifItalic18pt7b.h" // FF39 or FSI18
  #include "GFXFF/FreeSerifItalic24pt7b.h" // FF40 or FSI24

  #include "GFXFF/FreeSerifBold9pt7b.h"  // FF41 or FSB9
  #include "GFXFF/FreeSerifBold12pt7b.h" // FF42 or FSB12
  #include "GFXFF/FreeSerifBold18pt7b.h" // FF43 or FSB18
  #include "GFXFF/FreeSerifBold24pt7b.h" // FF44 or FSB24

  #include "GFXFF/FreeSerifBoldItalic9pt7b.h"  // FF45 or FSBI9
  #include "GFXFF/FreeSerifBoldItalic12pt7b.h" // FF46 or FSBI12
  #include "GFXFF/FreeSerifBoldItalic18pt7b.h" // FF47 or FSBI18
  #include "GFXFF/FreeSerifBoldItalic24pt7b.h" // FF48 or FSBI24

  // Custom fonts
  #include "Custom/Orbitron_Light_24.h"
  #include "Custom/Orbitron_Light_32.h"
  #include "Custom/Roboto_Thin_24.h"
  #include "Custom/Satisfy_24.h"
  #include "Custom/Yellowtail_32.h"


  #include "glcdfont.h"
  #include "Font16.h"
  #include "Font32rle.h"
  #include "Font64rle.h"
  #include "Font7srle.h"
  #include "Font72rle.h"

  const GLCDfont Font0 = { (const uint8_t *)font, nullptr, 6, 8, 7};
  const BMPfont  Font2 = { (const uint8_t *)chrtbl_f16, widtbl_f16, 0, chr_hgt_f16, baseline_f16 };
  const RLEfont  Font4 = { (const uint8_t *)chrtbl_f32, widtbl_f32, 0, chr_hgt_f32, baseline_f32 };
  const RLEfont  Font6 = { (const uint8_t *)chrtbl_f64, widtbl_f64, 0, chr_hgt_f64, baseline_f64 };
  const RLEfont  Font7 = { (const uint8_t *)chrtbl_f7s, widtbl_f7s, 0, chr_hgt_f7s, baseline_f7s };
  const RLEfont  Font8 = { (const uint8_t *)chrtbl_f72, widtbl_f72, 0, chr_hgt_f72, baseline_f72 };
}

//----------------------------------------------------------------------------

namespace lgfx
{
  // deprecated array.
  const IFont* fontdata [] = {
    &fonts::Font0,  // GLCD font (Font 0)
    &fonts::Font0,  // Font 1 current unused
    &fonts::Font2,
    &fonts::Font0,  // Font 3 current unused
    &fonts::Font4,
    &fonts::Font0,  // Font 5 current unused
    &fonts::Font6,
    &fonts::Font7,
    &fonts::Font8,
    nullptr,
  };
}

