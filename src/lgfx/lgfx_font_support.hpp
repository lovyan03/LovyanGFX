/*----------------------------------------------------------------------------/
  Lovyan GFX library - ESP32 hardware SPI graphics library .  
  
    for Arduino and ESP-IDF  
  
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
#ifndef LGFX_FONT_SUPPORT_HPP_
#define LGFX_FONT_SUPPORT_HPP_

#include "../Fonts/lgfx_fonts.hpp"
#include <string>
#include <cstring>
#include <cmath>
#include <cstdarg>

#if defined (ARDUINO)
#include <Print.h>
#endif

namespace lgfx
{
  struct TextStyle {
    std::uint32_t fore_rgb888 = 0xFFFFFFU;
    std::uint32_t back_rgb888 = 0;
    float size_x = 1;
    float size_y = 1;
    textdatum_t datum = textdatum_t::top_left;
    bool utf8 = true;
    bool cp437 = false;
  };

//----------------------------------------------------------------------------

  struct VLWfont : public RunTimeFont {
    std::uint16_t gCount;     // Total number of characters
    std::uint16_t yAdvance;   // Line advance
    std::uint16_t spaceWidth; // Width of a space character
    std::int16_t  ascent;     // Height of top of 'd' above baseline, other characters may be taller
    std::int16_t  descent;    // Offset to bottom of 'p', other characters may have a larger descent
    std::uint16_t maxAscent;  // Maximum ascent found in font
    std::uint16_t maxDescent; // Maximum descent found in font

    // These are for the metrics for each individual glyph (so we don't need to seek this in file and waste time)
    std::uint16_t* gUnicode  = nullptr;  //UTF-16 code, the codes are searched so do not need to be sequential
    std::uint8_t*  gWidth    = nullptr;  //cwidth
    std::uint8_t*  gxAdvance = nullptr;  //setWidth
    std::int8_t*   gdX       = nullptr;  //leftExtent
    std::uint32_t* gBitmap   = nullptr;  //file pointer to greyscale bitmap

    DataWrapper* _fontData = nullptr;
    bool _fontLoaded = false; // Flags when a anti-aliased font is loaded

    font_type_t getType(void) const override { return ft_vlw;  } 

    void getDefaultMetric(FontMetrics *metrics) const override {
      metrics->x_offset  = 0;
      metrics->y_offset  = 0;
      metrics->baseline  = maxAscent;
      metrics->y_advance = yAdvance;
      metrics->height    = yAdvance;
    }

    virtual ~VLWfont() {
      unloadFont();
    }

    bool unloadFont(void) override {
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

    bool getUnicodeIndex(std::uint16_t unicode, std::uint16_t *index) const
    {
      auto poi = std::lower_bound(gUnicode, &gUnicode[gCount], unicode);
      *index = std::distance(gUnicode, poi);
      return (*poi == unicode);
    }

    bool updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const override {
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


    bool loadFont(DataWrapper* data) {
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
  };

//----------------------------------------------------------------------------

  template <class Base>
  class LGFX_Font_Support : public Base
#if defined (ARDUINO)
  , public Print
#endif
  {
  public:
    virtual ~LGFX_Font_Support() { unloadFont(); }

    std::int16_t getCursorX(void) const { return _cursor_x; }
    std::int16_t getCursorY(void) const { return _cursor_y; }
    float getTextSizeX(void) const { return _text_style.size_x; }
    float getTextSizeY(void) const { return _text_style.size_y; }
    textdatum_t getTextDatum(void) const { return _text_style.datum; }
    std::int16_t fontHeight(void) const { return _font_metrics.height * _text_style.size_y; }
    std::int16_t fontHeight(std::uint8_t font) const { return ((const BaseFont*)fontdata[font])->height * _text_style.size_y; }

    void setCursor( std::int16_t x, std::int16_t y)               { _filled_x = 0; _cursor_x = x; _cursor_y = y; }
    void setCursor( std::int16_t x, std::int16_t y, std::uint8_t font) { _filled_x = 0; _cursor_x = x; _cursor_y = y; _font = fontdata[font]; }
    void setTextStyle(const TextStyle& text_style) { _text_style = text_style; }
    const TextStyle& getTextStyle(void) const { return _text_style; }
    void setTextSize(float size) { setTextSize(size, size); }
    void setTextSize(float sx, float sy) { _text_style.size_x = (sx > 0) ? sx : 1; _text_style.size_y = (sy > 0) ? sy : 1; }
    void setTextDatum(textdatum_t datum) { _text_style.datum = datum; }
    [[deprecated("use textdatum_t")]]
    void setTextDatum(std::uint8_t datum) { _text_style.datum = (textdatum_t)datum; }
    void setTextPadding(std::uint32_t padding_x) { _padding_x = padding_x; }
    void setTextWrap( bool wrapX, bool wrapY = false) { _textwrap_x = wrapX; _textwrap_y = wrapY; }
    void setTextScroll(bool scroll) { _textscroll = scroll; if (_cursor_x < this->_sx) { _cursor_x = this->_sx; } if (_cursor_y < this->_sy) { _cursor_y = this->_sy; } }

    template<typename T>
    void setTextColor(T color) {
      if (this->hasPalette()) {
        _text_style.fore_rgb888 = _text_style.back_rgb888 = color;
      } else {
        _text_style.fore_rgb888 = _text_style.back_rgb888 = convert_to_rgb888(color);
      }
    }
    template<typename T1, typename T2>
    void setTextColor(T1 fgcolor, T2 bgcolor) {
      if (this->hasPalette()) {
        _text_style.fore_rgb888 = fgcolor;
        _text_style.back_rgb888 = bgcolor;
      } else {
        _text_style.fore_rgb888 = convert_to_rgb888(fgcolor);
        _text_style.back_rgb888 = convert_to_rgb888(bgcolor);
      }
    }

    std::int32_t textWidth(const char *string) {
      if (!string) return 0;

      auto sx = _text_style.size_x;

      std::int32_t left = 0;
      std::int32_t right = 0;
      do {
        std::uint16_t uniCode = *string;
        if (!uniCode) break;
        if (_text_style.utf8) {
          do {
            uniCode = decodeUTF8(*string);
          } while (uniCode < 0x20 && *(++string));
          if (uniCode < 0x20) break;
        }

        if (!_font->updateFontMetric(&_font_metrics, uniCode)) continue;
        if (left == 0 && right == 0 && _font_metrics.x_offset < 0) left = right = - (int)(_font_metrics.x_offset * sx);
        right = left + std::max<int>(_font_metrics.x_advance*sx, int(_font_metrics.width*sx) + int(_font_metrics.x_offset * sx));
        //right = left + (int)(std::max<int>(_font_metrics.x_advance, _font_metrics.width + _font_metrics.x_offset) * sx);
        left += (int)(_font_metrics.x_advance * sx);
      } while (*(++string));
      return right;
    }

  #if defined (ARDUINO)
    inline std::int32_t textWidth(const String& string) { return textWidth(string.c_str()); }

    inline size_t drawString(const String& string, std::int32_t x, std::int32_t y) { return draw_string(string.c_str(), x, y, _text_style.datum); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawCentreString(const String& string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawCenterString(const String& string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawRightString( const String& string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_right); }

    inline size_t drawString(const String& string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, _text_style.datum); }

  #endif
    inline size_t drawString(const char *string, std::int32_t x, std::int32_t y) { return draw_string(string, x, y, _text_style.datum); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawCentreString(const char *string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawCenterString(const char *string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawRightString( const char *string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_right); }

    inline size_t drawString(const char *string, std::int32_t x, std::int32_t y, std::uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, _text_style.datum); }

    inline size_t drawNumber(long long_num, std::int32_t poX, std::int32_t poY, std::uint8_t font) { setFont(fontdata[font]); return drawNumber(long_num, poX, poY); }

    inline size_t drawFloat(float floatNumber, std::uint8_t dp, std::int32_t poX, std::int32_t poY, std::uint8_t font) { setFont(fontdata[font]); return drawFloat(floatNumber, dp, poX, poY); }

    size_t drawNumber(long long_num, std::int32_t poX, std::int32_t poY)
    {
      constexpr size_t len = 8 * sizeof(long) + 1;
      char buf[len];
      return drawString(numberToStr(long_num, buf, len, 10), poX, poY);
    }

    size_t drawFloat(float floatNumber, std::uint8_t dp, std::int32_t poX, std::int32_t poY)
    {
      size_t len = 14 + dp;
      char buf[len];
      return drawString(floatToStr(floatNumber, buf, len, dp), poX, poY);
    }

    size_t drawChar(std::uint16_t uniCode, std::int32_t x, std::int32_t y, std::uint8_t font) {
      if (_font == fontdata[font]) return drawChar(uniCode, x, y);
      _filled_x = 0;
      switch (fontdata[font]->getType()) {
      default:
      case IFont::font_type_t::ft_glcd: return drawCharGLCD(this, x, y, uniCode, &_text_style, fontdata[font]);
      case IFont::font_type_t::ft_bmp:  return drawCharBMP( this, x, y, uniCode, &_text_style, fontdata[font]);
      case IFont::font_type_t::ft_rle:  return drawCharRLE( this, x, y, uniCode, &_text_style, fontdata[font]);
      case IFont::font_type_t::ft_bdf:  return drawCharBDF( this, x, y, uniCode, &_text_style, fontdata[font]);
      }
    }

    inline size_t drawChar(std::uint16_t uniCode, std::int32_t x, std::int32_t y) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, &_text_style, _font); }

    template<typename T>
    inline size_t drawChar(std::int32_t x, std::int32_t y, std::uint16_t uniCode, T color, T bg, float size) { return drawChar(x, y, uniCode, color, bg, size, size); }
    template<typename T>
    inline size_t drawChar(std::int32_t x, std::int32_t y, std::uint16_t uniCode, T color, T bg, float size_x, float size_y) {
      TextStyle style = _text_style;
      style.back_rgb888 = convert_to_rgb888(color);
      style.fore_rgb888 = convert_to_rgb888(bg);
      style.size_x = size_x;
      style.size_y = size_y;
      _filled_x = 0;
      return (fpDrawChar)(this, x, y, uniCode, &style, _font);
    }

#ifdef ARDUINO
    [[deprecated("use getFont()")]]
    std::uint8_t getTextFont(void) const {
      size_t ie = sizeof(fontdata) / sizeof(fontdata[0]);
      for (size_t i = 0; i < ie; ++i)
        if (fontdata[i] == _font) return i;
      return 0;
    }
#endif

#ifdef __EFONT_FONT_DATA_H__
    [[deprecated("use setFont(&fonts::efont)")]]
    void setTextEFont() { setFont(&fonts::efont); }
#endif

    [[deprecated("use setFont(&fonts::Font0)")]]
    void setTextFont(int f) {
      if (f == 1 && _font && _font->getType() == IFont::font_type_t::ft_gfx) return;

      setFont(fontdata[f]);
    }

    [[deprecated("use setFont(&fonts::Font0)")]]
    void setTextFont(const IFont* font = nullptr) { setFont(font); }

    [[deprecated("use setFont()")]]
    void setFreeFont(const IFont* font = nullptr) { setFont(font); }

    void unloadFont(void) {
      if (_dynamic_font) {
        delete _dynamic_font;
        _dynamic_font = nullptr;
      }
      setFont(&fonts::Font0);
    }

    __attribute__ ((always_inline)) inline const IFont* getFont (void) const { return _font; }

    void setFont(const IFont* font) {
      if (_dynamic_font) {
        delete _dynamic_font;
        _dynamic_font = nullptr;
      }
      if (font == nullptr) font = &fonts::Font0;
      _font = font;
      //_decoderState = utf8_decode_state_t::utf8_state0;

      font->getDefaultMetric(&_font_metrics);

      switch (font->getType()) {
      default:
      case IFont::font_type_t::ft_glcd: fpDrawChar = drawCharGLCD;  break;
      case IFont::font_type_t::ft_bmp:  fpDrawChar = drawCharBMP;   break;
      case IFont::font_type_t::ft_rle:  fpDrawChar = drawCharRLE;   break;
      case IFont::font_type_t::ft_bdf:  fpDrawChar = drawCharBDF;   break;
      case IFont::font_type_t::ft_gfx:  fpDrawChar = drawCharGFXFF; break;
      }
    }

    void cp437(bool enable = true) { _text_style.cp437 = enable; }  // AdafruitGFX compatible.

    void setAttribute(attribute_t attr_id, std::uint8_t param) {
      switch (attr_id) {
        case cp437_switch:
            _text_style.cp437 = param;
            break;
        case utf8_switch:
            _text_style.utf8  = param;
            _decoderState = utf8_decode_state_t::utf8_state0;
            break;
        default: break;
      }
    }

    std::uint8_t getAttribute(std::uint8_t attr_id) { return getAttribute((attribute_t)attr_id); }
    std::uint8_t getAttribute(attribute_t attr_id) {
      switch (attr_id) {
        case cp437_switch: return _text_style.cp437;
        case utf8_switch: return _text_style.utf8;
        default: return 0;
      }
    }


#if defined (ARDUINO)
 #if defined (FS_H) || defined (__SEEED_FS__)
    void loadFont(const char *path, fs::FS &fs) {
      _font_file.setFS(fs);
      loadFont(path);
    }
 #endif
#endif

    void loadFont(const std::uint8_t* array) {
      this->unloadFont();
      _font_data.set(array);
      auto font = new VLWfont();
      this->_dynamic_font = font;
      if (font->loadFont(&_font_data)) {
        this->_font = font;
        this->fpDrawChar = drawCharVLW;
        this->_font->getDefaultMetric(&this->_font_metrics);
      } else {
        this->unloadFont();
      }
    }

    void loadFont(const char *path) {
      this->unloadFont();

      this->prepareTmpTransaction(&_font_file);
      _font_file.preRead();

      bool result = _font_file.open(path, "r");
      if (!result) {
        std::string filename = "/";
        if (path[0] == '/') filename = path;
        else filename += path;
        int len = strlen(path);
        if (memcmp(&path[len - 4], ".vlw", 4)) {
          filename += ".vlw";
        }
        result = _font_file.open(filename.c_str(), "r");
      }
      auto font = new VLWfont();
      this->_dynamic_font = font;
      if (result) {
        result = font->loadFont(&_font_file);
      }
      if (result) {
        this->_font = font;
        this->_font->getDefaultMetric(&this->_font_metrics);
        this->fpDrawChar = drawCharVLW;
      } else {
        this->unloadFont();
      }
      _font_file.postRead();
    }

    void showFont(std::uint32_t td)
    {
      auto font = (const VLWfont*)this->_font;
      if (!font->_fontLoaded) return;

      std::int16_t x = this->width();
      std::int16_t y = this->height();
      std::uint32_t timeDelay = 0;    // No delay before first page

      this->fillScreen(this->_text_style.back_rgb888);

      for (std::uint16_t i = 0; i < font->gCount; i++)
      {
        // Check if this will need a new screen
        if (x + font->gdX[i] + font->gWidth[i] >= this->width())  {
          x = - font->gdX[i];

          y += font->yAdvance;
          if (y + font->maxAscent + font->descent >= this->height()) {
            x = - font->gdX[i];
            y = 0;
            //delay(timeDelay);
            timeDelay = td;
            this->fillScreen(this->_text_style.back_rgb888);
          }
        }

        this->drawChar(font->gUnicode[i], x, y);
        x += font->gxAdvance[i];
        //yield();
      }

      //delay(timeDelay);
      this->fillScreen(this->_text_style.back_rgb888);
      //fontFile.close();
    }


//----------------------------------------------------------------------------
// print & text support
//----------------------------------------------------------------------------
// Arduino Print.h compatible
  #if !defined (ARDUINO)
    size_t print(const char str[])      { return write(str); }
    size_t print(char c)                { return write(c); }
    size_t print(int  n, int base = 10) { return print((long)n, base); }
    size_t print(long n, int base = 10)
    {
      if (base == 0) { return write(n); }
      if (base == 10) {
        if (n < 0) {
          size_t t = print('-');
          return printNumber(-n, 10) + t;
        }
        return printNumber(n, 10);
      }
      return printNumber(n, base);
    }

    size_t print(unsigned char n, int base = 10) { return print((unsigned long)n, base); }
    size_t print(unsigned int  n, int base = 10) { return print((unsigned long)n, base); }
    size_t print(unsigned long n, int base = 10) { return (base) ? printNumber(n, base) : write(n); }
    size_t print(double        n, int digits= 2) { return printFloat(n, digits); }

    size_t println(void) { return print("\r\n"); }
    size_t println(const char c[])                 { size_t t = print(c); return println() + t; }
    size_t println(char c        )                 { size_t t = print(c); return println() + t; }
    size_t println(int  n, int base = 10)          { size_t t = print(n,base); return println() + t; }
    size_t println(long n, int base = 10)          { size_t t = print(n,base); return println() + t; }
    size_t println(unsigned char n, int base = 10) { size_t t = print(n,base); return println() + t; }
    size_t println(unsigned int  n, int base = 10) { size_t t = print(n,base); return println() + t; }
    size_t println(unsigned long n, int base = 10) { size_t t = print(n,base); return println() + t; }
    size_t println(double        n, int digits= 2) { size_t t = print(n, digits); return println() + t; }

  //size_t print(const String &s) { return write(s.c_str(), s.length()); }
  //size_t print(const __FlashStringHelper *s)   { return print(reinterpret_cast<const char *>(s)); }
  //size_t println(const String &s)              { size_t t = print(s); return println() + t; }
  //size_t println(const __FlashStringHelper *s) { size_t t = print(s); return println() + t; }

    size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)))
    {
      char loc_buf[64];
      char * temp = loc_buf;
      va_list arg;
      va_list copy;
      va_start(arg, format);
      va_copy(copy, arg);
      size_t len = vsnprintf(temp, sizeof(loc_buf), format, copy);
      va_end(copy);

      if (len >= sizeof(loc_buf)){
        temp = (char*) malloc(len+1);
        if (temp == nullptr) {
          va_end(arg);
          return 0;
        }
        len = vsnprintf(temp, len+1, format, arg);
      }
      va_end(arg);
      len = write((std::uint8_t*)temp, len);
      if (temp != loc_buf){
        free(temp);
      }
      return len;
    }

    size_t write(const char* str)                 { return (!str) ? 0 : write((const std::uint8_t*)str, strlen(str)); }
    size_t write(const char *buf, size_t size)    { return write((const std::uint8_t *) buf, size); }
  #endif
    size_t write(const std::uint8_t *buf, size_t size) { size_t n = 0; this->startWrite(); while (size--) { n += write(*buf++); } this->endWrite(); return n; }
    size_t write(std::uint8_t utf8)
    {
      if (utf8 == '\r') return 1;
      if (utf8 == '\n') {
        _filled_x = (_textscroll) ? this->_sx : 0;
        _cursor_x = _filled_x;
        _cursor_y += _font_metrics.y_advance * _text_style.size_y;
      } else {
        std::uint16_t uniCode = utf8;
        if (_text_style.utf8) {
          uniCode = decodeUTF8(utf8);
          if (uniCode < 0x20) return 1;
        }
        //if (!(fpUpdateFontSize)(this, uniCode)) return 1;
        if (!_font->updateFontMetric(&_font_metrics, uniCode)) return 1;

        std::int_fast16_t xo = _font_metrics.x_offset  * _text_style.size_x;
        std::int_fast16_t w  = std::max(xo + _font_metrics.width * _text_style.size_x, _font_metrics.x_advance * _text_style.size_x);
        if (_textscroll || _textwrap_x) {
          std::int32_t llimit = _textscroll ? this->_sx : this->_clip_l;
          if (_cursor_x < llimit - xo) _cursor_x = llimit - xo;
          else {
            std::int32_t rlimit = _textscroll ? this->_sx + this->_sw : (this->_clip_r + 1);
            if (_cursor_x + w > rlimit) {
              _filled_x = llimit;
              _cursor_x = llimit - xo;
              _cursor_y += _font_metrics.y_advance * _text_style.size_y;
            }
          }
        }

        std::int_fast16_t h  = _font_metrics.height * _text_style.size_y;

        std::int_fast16_t ydiff = 0;
        if (_text_style.datum & middle_left) {          // vertical: middle
          ydiff -= h >> 1;
        } else if (_text_style.datum & bottom_left) {   // vertical: bottom
          ydiff -= h;
        } else if (_text_style.datum & baseline_left) { // vertical: baseline
          ydiff -= (int)(_font_metrics.baseline * _text_style.size_y);
        }
        std::int_fast16_t y = _cursor_y + ydiff;

        if (_textscroll) {
          if (y < this->_sy) y = this->_sy;
          else {
            int yshift = (this->_sy + this->_sh) - (y + h);
            if (yshift < 0) {
              this->scroll(0, yshift);
              y += yshift;
            }
          }
        } else if (_textwrap_y) {
          if (y + h > (this->_clip_b + 1)) {
            _filled_x = 0;
            _cursor_x = - xo;
            y = 0;
          } else
          if (y < this->_clip_t) y = this->_clip_t;
        }
        _cursor_y = y - ydiff;
        y -= int(_font_metrics.y_offset  * _text_style.size_y);
        _cursor_x += (fpDrawChar)(this, _cursor_x, y, uniCode, &_text_style, _font);
      }

      return 1;
    }

    std::uint16_t decodeUTF8(std::uint8_t c)
    {
      // 7 bit Unicode Code Point
      if (!(c & 0x80)) {
        _decoderState = utf8_decode_state_t::utf8_state0;
        return c;
      }

      if (_decoderState == utf8_decode_state_t::utf8_state0)
      {
        // 11 bit Unicode Code Point
        if ((c & 0xE0) == 0xC0)
        {
          _unicode_buffer = ((c & 0x1F)<<6);
          _decoderState = utf8_decode_state_t::utf8_state1;
          return 0;
        }

        // 16 bit Unicode Code Point
        if ((c & 0xF0) == 0xE0)
        {
          _unicode_buffer = ((c & 0x0F)<<12);
          _decoderState = utf8_decode_state_t::utf8_state2;
          return 0;
        }
        // 21 bit Unicode  Code Point not supported so fall-back to extended ASCII
        //if ((c & 0xF8) == 0xF0) return (std::uint16_t)c;
      }
      else
      {
        if (_decoderState == utf8_decode_state_t::utf8_state2)
        {
          _unicode_buffer |= ((c & 0x3F)<<6);
          _decoderState = utf8_decode_state_t::utf8_state1;
          return 0;
        }
        _unicode_buffer |= (c & 0x3F);
        _decoderState = utf8_decode_state_t::utf8_state0;
        return _unicode_buffer;
      }

      _decoderState = utf8_decode_state_t::utf8_state0;

      return (std::uint16_t)c; // fall-back to extended ASCII
    }

  protected:

    enum utf8_decode_state_t
    { utf8_state0 = 0
    , utf8_state1 = 1
    , utf8_state2 = 2
    };
    utf8_decode_state_t _decoderState = utf8_state0;   // UTF8 decoder state
    std::uint16_t _unicode_buffer = 0;   // Unicode code-point buffer

    std::int32_t _cursor_x = 0;
    std::int32_t _cursor_y = 0;
    std::int32_t _filled_x = 0;
    std::int32_t _padding_x = 0;

    TextStyle _text_style;
    FontMetrics _font_metrics = { 6, 6, 0, 8, 8, 0, 7 }; // Font0 Metric
    const IFont* _font = &fonts::Font0;

    RunTimeFont* _dynamic_font = nullptr;  // run-time generated font
    FileWrapper _font_file;
    PointerWrapper _font_data;

    bool _textwrap_x = true;
    bool _textwrap_y = false;
    bool _textscroll = false;



    size_t printNumber(unsigned long n, std::uint8_t base)
    {
      size_t len = 8 * sizeof(long) + 1;
      char buf[len];
      return write(numberToStr(n, buf, len, base));
    }

    size_t printFloat(double number, std::uint8_t digits)
    {
      size_t len = 14 + digits;
      char buf[len];
      return write(floatToStr(number, buf, len, digits));
    }

    char* numberToStr(long n, char* buf, size_t buflen, std::uint8_t base)
    {
      if (n >= 0) return numberToStr((unsigned long) n, buf, buflen, base);
      auto res = numberToStr(- n, buf, buflen, 10) - 1;
      res[0] = '-';
      return res;
    }

    char* numberToStr(unsigned long n, char* buf, size_t buflen, std::uint8_t base)
    {
      char *str = &buf[buflen - 1];

      *str = '\0';

      if (base < 2) { base = 10; }  // prevent crash if called with base == 1
      do {
        unsigned long m = n;
        n /= base;
        char c = m - base * n;
        *--str = c < 10 ? c + '0' : c + 'A' - 10;
      } while (n);

      return str;
    }

    char* floatToStr(double number, char* buf, size_t buflen, std::uint8_t digits)
    {
      if (std::isnan(number))    { return strcpy(buf, "nan"); }
      if (std::isinf(number))    { return strcpy(buf, "inf"); }
      if (number > 4294967040.0) { return strcpy(buf, "ovf"); } // constant determined empirically
      if (number <-4294967040.0) { return strcpy(buf, "ovf"); } // constant determined empirically

      char* dst = buf;
      // Handle negative numbers
      //bool negative = (number < 0.0);
      if (number < 0.0) {
        number = -number;
        *dst++ = '-';
      }

      // Round correctly so that print(1.999, 2) prints as "2.00"
      double rounding = 0.5;
      for(std::uint8_t i = 0; i < digits; ++i) {
        rounding /= 10.0;
      }

      number += rounding;

      // Extract the integer part of the number and print it
      unsigned long int_part = (unsigned long) number;
      double remainder = number - (double) int_part;

      {
        constexpr size_t len = 14;
        char numstr[len];
        auto tmp = numberToStr(int_part, numstr, len, 10);
        auto slen = strlen(tmp);
        memcpy(dst, tmp, slen);
        dst += slen;
      }

      // Print the decimal point, but only if there are digits beyond
      if (digits > 0) {
        dst[0] = '.';
        ++dst;
      }
      // Extract digits from the remainder one at a time
      while (digits-- > 0) {
        remainder *= 10.0;
        unsigned int toPrint = (unsigned int)(remainder);
        dst[0] = '0' + toPrint;
        ++dst;
        remainder -= toPrint;
      }
      dst[0] = 0;
      return buf;
    }

    size_t draw_string(const char *string, std::int32_t x, std::int32_t y, textdatum_t datum)
    {
      std::int16_t sumX = 0;
      std::int32_t cwidth = textWidth(string); // Find the pixel width of the string in the font
      std::int32_t cheight = _font_metrics.height * _text_style.size_y;

      {
        auto tmp = string;
        do {
          std::uint16_t uniCode = *tmp;
          if (!tmp) break;
          if (_text_style.utf8) {
            do {
              uniCode = decodeUTF8(*tmp); 
            } while (uniCode < 0x20 && *++tmp);
            if (uniCode < 0x20) break;
          }
          if (_font->updateFontMetric(&_font_metrics, uniCode)) {
            if (_font_metrics.x_offset < 0) sumX = - _font_metrics.x_offset * _text_style.size_x;
            break;
          }
        } while (*++tmp);
      }
      if (datum & middle_left) {          // vertical: middle
        y -= cheight >> 1;
      } else if (datum & bottom_left) {   // vertical: bottom
        y -= cheight;
      } else if (datum & baseline_left) { // vertical: baseline
        y -= (int)(_font_metrics.baseline * _text_style.size_y);
      }

      this->startWrite();
      std::int32_t padx = _padding_x;
      if ((_text_style.fore_rgb888 != _text_style.back_rgb888) && (padx > cwidth)) {
        this->setColor(_text_style.back_rgb888);
        if (datum & top_center) {
          auto halfcwidth = cwidth >> 1;
          auto halfpadx = (padx >> 1);
          this->writeFillRect(x - halfpadx, y, halfpadx - halfcwidth, cheight);
          halfcwidth = cwidth - halfcwidth;
          halfpadx = padx - halfpadx;
          this->writeFillRect(x + halfcwidth, y, halfpadx - halfcwidth, cheight);
        } else if (datum & top_right) {
          this->writeFillRect(x - padx, y, padx - cwidth, cheight);
        } else {
          this->writeFillRect(x + cwidth, y, padx - cwidth, cheight);
        }
      }

      if (datum & top_center) {           // Horizontal: middle
        x -= cwidth >> 1;
      } else if (datum & top_right) {     // Horizontal: right
        x -= cwidth;
      }

      y -= int(_font_metrics.y_offset * _text_style.size_y);

      _filled_x = 0;
      do {
        std::uint16_t uniCode = *string;
        if (!uniCode) break;
        if (_text_style.utf8) {
          do {
            uniCode = decodeUTF8(*string);
          } while (uniCode < 0x20 && *++string);
          if (uniCode < 0x20) break;
        }
        sumX += (fpDrawChar)(this, x + sumX, y, uniCode, &_text_style, _font);
      } while (*(++string));
      this->endWrite();

      return sumX;
    }

    size_t (*fpDrawChar)(LGFX_Font_Support* me, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style, const IFont* font) = &LGFX_Font_Support::drawCharGLCD;

    static size_t drawCharGLCD(LGFX_Font_Support* me, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style, const IFont* ifont)
    { // glcd font
      auto font = (const GLCDfont*)ifont;
      if (c > 255) return 0;

      if (!style->cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

      const std::int32_t fontWidth  = font->width;
      const std::int32_t fontHeight = font->height;

      auto font_addr = font->chartbl + (c * 5);
      std::uint32_t colortbl[2] = {me->getColorConverter()->convert(style->back_rgb888), me->getColorConverter()->convert(style->fore_rgb888)};
      bool fillbg = (style->back_rgb888 != style->fore_rgb888);

      std::int32_t clip_left   = me->_clip_l;
      std::int32_t clip_right  = me->_clip_r;
      std::int32_t clip_top    = me->_clip_t;
      std::int32_t clip_bottom = me->_clip_b;

      float sy = style->size_y;
      float sx = style->size_x;
      if ((x <= clip_right) && (clip_left < (x + fontWidth * sx ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * sy ))) {
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

    static size_t drawCharBMP(LGFX_Font_Support* me, std::int32_t x, std::int32_t y, std::uint16_t uniCode, const TextStyle* style, const IFont* ifont)
    { // BMP font
      auto font = (const BMPfont*)ifont;

      if ((uniCode -= 0x20) >= 96) return 0;
      const std::int_fast8_t fontWidth = font->widthtbl[uniCode];
      const std::int_fast8_t fontHeight = font->height;

      auto font_addr = ((const std::uint8_t**)font->chartbl)[uniCode];
      return draw_char_bmp(me, x, y, style, font_addr, fontWidth, fontHeight, (fontWidth + 6) >> 3, 1);
    }

    static size_t drawCharBDF(LGFX_Font_Support* me, std::int32_t x, std::int32_t y, std::uint16_t c, const TextStyle* style, const IFont* ifont)
    {
      auto font = (BDFfont*)ifont;
      const std::int_fast8_t bytesize = (font->width + 7) >> 3;
      const std::int_fast8_t fontHeight = font->height;
      const std::int_fast8_t fontWidth = (c < 0x0100) ? font->halfwidth : font->width;
      auto it = std::lower_bound(font->indextbl, &font->indextbl[font->indexsize], c);
      if (*it != c) {
        if (style->fore_rgb888 != style->back_rgb888) {
          me->fillRect(x, y, fontWidth * style->size_x, fontHeight * style->size_y, style->back_rgb888);
        }
        return fontWidth * style->size_x;
      }
      const std::uint8_t* font_addr = &font->chartbl[std::distance(font->indextbl, it) * fontHeight * bytesize];
      return LGFX_Font_Support::draw_char_bmp(me, x, y, style, font_addr, fontWidth, fontHeight, bytesize, 0);
    }

    static size_t draw_char_bmp(LGFX_Font_Support* me, std::int32_t x, std::int32_t y, const TextStyle* style, const std::uint8_t* font_addr, std::int_fast8_t fontWidth, std::int_fast8_t fontHeight, std::int_fast8_t w, std::int_fast8_t margin )
    {
      std::uint32_t colortbl[2] = {me->getColorConverter()->convert(style->back_rgb888), me->getColorConverter()->convert(style->fore_rgb888)};
      bool fillbg = (style->back_rgb888 != style->fore_rgb888);

      std::int32_t clip_left   = me->_clip_l;
      std::int32_t clip_right  = me->_clip_r;
      std::int32_t clip_top    = me->_clip_t;
      std::int32_t clip_bottom = me->_clip_b;

      float sx = style->size_x;
      float sy = style->size_y;

      if ((x <= clip_right) && (clip_left < (x + fontWidth * sx ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * sy ))) {
//      if (!fillbg || sy != 1 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * sx > clip_right) {
          me->startWrite();
          if (fillbg) {
            me->setRawColor(colortbl[0]);
            if (margin) {
              std::int32_t x0 = (fontWidth - margin) * sx;
              std::int32_t x1 = (fontWidth         ) * sx;
              if (x0 < x1) {
                me->writeFillRect(x + x0, y, x1 - x0, fontHeight * sy);
              }
            }
          }
          std::int_fast8_t i = 0;
          std::int32_t y1 = 0;
          std::int32_t y0 = - 1;
          do {
            bool fill = y0 != y1;
            y0 = y1;
            y1 = ++i * sy;
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

    static size_t drawCharRLE(LGFX_Font_Support* me, std::int32_t x, std::int32_t y, std::uint16_t code, const TextStyle* style, const IFont* ifont)
    { // RLE font
      auto font = (RLEfont*)ifont;
      if ((code -= 0x20) >= 96) return 0;

      const int fontWidth = font->widthtbl[code];
      const int fontHeight = font->height;

      auto font_addr = ((const std::uint8_t**)font->chartbl)[code];

      std::uint32_t colortbl[2] = {me->getColorConverter()->convert(style->back_rgb888), me->getColorConverter()->convert(style->fore_rgb888)};
      bool fillbg = (style->back_rgb888 != style->fore_rgb888);

      std::int32_t clip_left   = me->_clip_l;
      std::int32_t clip_right  = me->_clip_r;
      std::int32_t clip_top    = me->_clip_t;
      std::int32_t clip_bottom = me->_clip_b;

      float sx = style->size_x;
      float sy = style->size_y;

      if ((x <= clip_right) && (clip_left < (x + fontWidth  * sx ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * sy ))) {
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

    static size_t drawCharGFXFF(LGFX_Font_Support* me, std::int32_t x, std::int32_t y, std::uint16_t uniCode, const TextStyle* style, const IFont* ifont)
    {
      auto font = (const GFXfont*)ifont;
      auto glyph = font->getGlyph(uniCode);
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

      std::int32_t clip_left   = me->_clip_l;
      std::int32_t clip_right  = me->_clip_r;
      std::int32_t clip_top    = me->_clip_t;
      std::int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + w * sx ))
       && (y <= clip_bottom) && (clip_top < (y + h * sy ))) {

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

        std::uint8_t *bitmap = &font->bitmap[glyph->bitmapOffset];
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
      } else {
        if (left < right) {
          me->writeFillRect(left, y, right - left, (me->_font_metrics.height) * sy);
        }
      }
      me->_filled_x = right;
      me->endWrite();
      return xAdvance;
    }

    static size_t drawCharVLW(LGFX_Font_Support* lgfxbase, std::int32_t x, std::int32_t y, std::uint16_t code, const TextStyle* style, const IFont*)
    {
      auto me = (LGFX_Font_Support*)lgfxbase;
      auto font = (const VLWfont*)me->_font;
      auto file = font->_fontData;

      std::uint32_t buffer[6] = {0};
      std::uint16_t gNum = 0;

      if (code == 0x20) {
        gNum = 0xFFFF;
        buffer[2] = __builtin_bswap32(font->spaceWidth);
      } else if (!font->getUnicodeIndex(code, &gNum)) {
        return 0;
      } else {
        file->preRead();
        file->seek(28 + gNum * 28);
        file->read((std::uint8_t*)buffer, 24);
        file->seek(font->gBitmap[gNum]);
      }


      std::int32_t h        = __builtin_bswap32(buffer[0]); // Height of glyph
      std::int32_t w        = __builtin_bswap32(buffer[1]); // Width of glyph
      float sx = style->size_x;
      std::int32_t xAdvance = __builtin_bswap32(buffer[2]) * sx; // xAdvance - to move x cursor
      std::int32_t xoffset   = (std::int32_t)((std::int8_t)__builtin_bswap32(buffer[4])) * sx; // x delta from cursor
      std::int32_t dY        = (std::int16_t)__builtin_bswap32(buffer[3]); // y delta from baseline
      float sy = style->size_y;
      std::int32_t yoffset = (font->maxAscent - dY);
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
                  me->push_image(x + rx, by, rw, bh, &p);
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
  };
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
}
//----------------------------------------------------------------------------

#endif
