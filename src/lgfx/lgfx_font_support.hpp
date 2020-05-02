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

namespace lgfx
{
  enum attribute_t
  { cp437_switch = 1
  , utf8_switch  = 2
  };

  // deprecated array.
  static PROGMEM const IFont* fontdata [] = {
    &fonts::Font0,  // GLCD font (Font 0)
    &fonts::Font0,  // GLCD font (or GFX font)
    &fonts::Font2,
    &fonts::Font0,  // Font 3 current unused
    &fonts::Font4,
    &fonts::Font0,  // Font 5 current unused
    &fonts::Font6,
    &fonts::Font7,
    &fonts::Font8,
  };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

  template <class Base>
  class LGFX_Font_Support : public Base {
  public:
    virtual ~LGFX_Font_Support() { unloadFont(); }

//----------------------------------------------------------------------------
// print & text support
//----------------------------------------------------------------------------
// Arduino Print.h compatible
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

  #if defined (ARDUINO)
    size_t print(const String &s) { return write(s.c_str(), s.length()); }
    size_t print(const __FlashStringHelper *s)   { return print(reinterpret_cast<const char *>(s)); }
    size_t println(const String &s)              { size_t t = print(s); return println() + t; }
    size_t println(const __FlashStringHelper *s) { size_t t = print(s); return println() + t; }
  #endif

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
      len = write((uint8_t*)temp, len);
      if (temp != loc_buf){
        free(temp);
      }
      return len;
    }

    size_t write(const char* str)                 { return (!str) ? 0 : write((const uint8_t*)str, strlen(str)); }
    size_t write(const char *buf, size_t size)    { return write((const uint8_t *) buf, size); }
    size_t write(const uint8_t *buf, size_t size) { size_t n = 0; this->startWrite(); while (size--) { n += write(*buf++); } this->endWrite(); return n; }
    size_t write(uint8_t utf8)
    {
      if (utf8 == '\r') return 1;
      if (utf8 == '\n') {
        _filled_x = 0;
        _cursor_x = 0;
        _cursor_y += _font_size.y_advance * _font_style.size_y;
      } else {
        uint16_t uniCode = utf8;
        if (_font_style.utf8) {
          uniCode = decodeUTF8(utf8);
          if (uniCode < 32) return 1;
        }
        //if (!(fpUpdateFontSize)(this, uniCode)) return 1;
        if (!_font->updateFontMetric(&_font_size, uniCode)) return 1;

        if (0 == _font_size.x_size) return 1;

        int16_t w  = _font_size.x_size    * _font_style.size_x;
        int16_t xo = _font_size.x_offset  * _font_style.size_x;
        int16_t h  = _font_size.y_size    * _font_style.size_y;
        int16_t yo = _font_size.y_offset  * _font_style.size_y;
        if (_textscroll || _textwrap_x) {
          int32_t left = _textscroll ? this->_sx : 0;
          if (_cursor_x < left - xo) _cursor_x = left - xo;
          else {
            int32_t right = _textscroll ? this->_sx + this->_sw : this->_width;
            if (_cursor_x + xo + w > right) {
              _filled_x = 0;
              _cursor_x = left - xo;
              _cursor_y += _font_size.y_advance * _font_style.size_y;
            }
          }
        }
        if (_textscroll) {
          if (_cursor_y < this->_sy - yo) _cursor_y = this->_sy - yo;
          else {
            int yshift = (this->_sy + this->_sh) - (_cursor_y + yo + h);
            if (yshift < 0) {
              this->scroll(0, yshift);
              _cursor_y += yshift;
            }
          }
        } else if (_textwrap_y) {
          if (_cursor_y + yo + h > this->_height) {
            _filled_x = 0;
            _cursor_x = - xo;
            _cursor_y = - yo;
          } else
          if (_cursor_y < - yo) _cursor_y = - yo;
        }
        _cursor_x += (fpDrawChar)(this, _cursor_x, _cursor_y, uniCode, &_font_style, _font);
      }

      return 1;
    }

    int32_t textWidth(const char *string)
    {
      if (!string) return 0;

      int32_t left = 0;
      int32_t right = 0;
      do {
        uint16_t uniCode = *string;
        if (_font_style.utf8) {
          do {
            uniCode = decodeUTF8(*string);
          } while (uniCode < 32 && *(++string));
          if (uniCode < 32) break;
        }
        //if (!(fpUpdateFontSize)(this, uniCode)) continue;
        if (!_font->updateFontMetric(&_font_size, uniCode)) continue;
        if (left == 0 && right == 0 && _font_size.x_offset < 0) left = right = -_font_size.x_offset;
        right = left + std::max((int32_t)_font_size.x_advance, _font_size.x_size + _font_size.x_offset);
        left += _font_size.x_advance;
      } while (*(++string));

      return right * _font_style.size_x;
    }

  #if defined (ARDUINO)
    inline int32_t textWidth(const String& string) { return textWidth(string.c_str()); }

    inline size_t drawString(const String& string, int32_t x, int32_t y) { return draw_string(string.c_str(), x, y, _font_style.datum); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawCentreString(const String& string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawCenterString(const String& string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawRightString( const String& string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, textdatum_t::top_right); }

    inline size_t drawString(const String& string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string.c_str(), x, y, _font_style.datum); }

  #endif
    inline size_t drawString(const char *string, int32_t x, int32_t y) { return draw_string(string, x, y, _font_style.datum); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawCentreString(const char *string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawCenterString(const char *string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_center); }

    [[deprecated("use setTextDatum() and drawString()")]]
    inline size_t drawRightString( const char *string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, textdatum_t::top_right); }

    inline size_t drawString(const char *string, int32_t x, int32_t y, uint8_t font) { setFont(fontdata[font]); return draw_string(string, x, y, _font_style.datum); }

    inline size_t drawNumber(long long_num, int32_t poX, int32_t poY, uint8_t font) { setFont(fontdata[font]); return drawNumber(long_num, poX, poY); }

    inline size_t drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY, uint8_t font) { setFont(fontdata[font]); return drawFloat(floatNumber, dp, poX, poY); }

    size_t drawNumber(long long_num, int32_t poX, int32_t poY)
    {
      constexpr size_t len = 8 * sizeof(long) + 1;
      char buf[len];
      return drawString(numberToStr(long_num, buf, len, 10), poX, poY);
    }

    size_t drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY)
    {
      size_t len = 14 + dp;
      char buf[len];
      return drawString(floatToStr(floatNumber, buf, len, dp), poX, poY);
    }

    uint16_t decodeUTF8(uint8_t c)
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
        //if ((c & 0xF8) == 0xF0) return (uint16_t)c;
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

      return (uint16_t)c; // fall-back to extended ASCII
    }

    template<typename T>
    void setTextColor(T color) {
      if (this->hasPalette()) {
        _font_style.fore_rgb888 = _font_style.back_rgb888 = color;
      } else {
        _font_style.fore_rgb888 = _font_style.back_rgb888 = convert_to_rgb888(color);
      }
    }
    template<typename T1, typename T2>
    void setTextColor(T1 fgcolor, T2 bgcolor) {
      if (this->hasPalette()) {
        _font_style.fore_rgb888 = fgcolor;
        _font_style.back_rgb888 = bgcolor;
      } else {
        _font_style.fore_rgb888 = convert_to_rgb888(fgcolor);
        _font_style.back_rgb888 = convert_to_rgb888(bgcolor);
      }
    }

    size_t drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font) {
      if (_font == fontdata[font]) return drawChar(uniCode, x, y);
      _filled_x = 0;
      switch (fontdata[font]->getType()) {
      default:
      case font_type_t::ft_glcd: return drawCharGLCD(this, x, y, uniCode, &_font_style, fontdata[font]);
      case font_type_t::ft_bmp:  return drawCharBMP( this, x, y, uniCode, &_font_style, fontdata[font]);
      case font_type_t::ft_rle:  return drawCharRLE( this, x, y, uniCode, &_font_style, fontdata[font]);
      case font_type_t::ft_bdf:  return drawCharBDF( this, x, y, uniCode, &_font_style, fontdata[font]);
      }
    }

    inline size_t drawChar(uint16_t uniCode, int32_t x, int32_t y) { _filled_x = 0; return (fpDrawChar)(this, x, y, uniCode, &_font_style, _font); }

    template<typename T>
    inline size_t drawChar(int32_t x, int32_t y, uint16_t uniCode, T color, T bg, int_fast8_t size) { return drawChar(x, y, uniCode, color, bg, size, size); }
    template<typename T>
    inline size_t drawChar(int32_t x, int32_t y, uint16_t uniCode, T color, T bg, int_fast8_t size_x, int_fast8_t size_y) {
      TextStyle style = _font_style;
      style.back_rgb888 = convert_to_rgb888(color);
      style.fore_rgb888 = convert_to_rgb888(bg);
      style.size_x = size_x;
      style.size_y = size_y;
      _filled_x = 0;
      return (fpDrawChar)(this, x, y, uniCode, &style, _font);
    }

    int16_t getCursorX(void) const { return _cursor_x; }
    int16_t getCursorY(void) const { return _cursor_y; }
    int16_t getTextSizeX(void) const { return _font_style.size_x; }
    int16_t getTextSizeY(void) const { return _font_style.size_y; }
    textdatum_t getTextDatum(void) const { return _font_style.datum; }
    int16_t fontHeight(void) const { return _font_size.y_size * _font_style.size_y; }
    int16_t fontHeight(uint8_t font) const { return pgm_read_byte( &((const BaseFont*)fontdata[font])->height ) * _font_style.size_y; }

    void setCursor( int16_t x, int16_t y)               { _filled_x = 0; _cursor_x = x; _cursor_y = y; }
    void setCursor( int16_t x, int16_t y, uint8_t font) { _filled_x = 0; _cursor_x = x; _cursor_y = y; _font = fontdata[font]; }
    void setTextSize(uint8_t s) { setTextSize(s,s); }
    void setTextSize(uint8_t sx, uint8_t sy) { _font_style.size_x = (sx > 0) ? sx : 1; _font_style.size_y = (sy > 0) ? sy : 1; }
    void setTextDatum(uint8_t datum) { _font_style.datum = (textdatum_t)datum; }
    void setTextDatum(textdatum_t datum) { _font_style.datum = datum; }
    void setTextPadding(uint16_t padding_x) { _padding_x = padding_x; }
    void setTextWrap( bool wrapX, bool wrapY = false) { _textwrap_x = wrapX; _textwrap_y = wrapY; }
    void setTextScroll(bool scroll) { _textscroll = scroll; if (_cursor_x < this->_sx) { _cursor_x = this->_sx; } if (_cursor_y < this->_sy) { _cursor_y = this->_sy; } }

    [[deprecated("use getFont()")]]
    uint8_t getTextFont(void) const {
      size_t ie = sizeof(fontdata) / sizeof(fontdata[0]);
      for (size_t i = 0; i < ie; ++i)
        if (fontdata[i] == _font) return i;
      return 0;
    }

#ifdef __EFONT_FONT_DATA_H__
    [[deprecated("use setFont(&fonts::efont)")]]
    void setTextEFont() { setFont(&fonts::efont); }
#endif

    [[deprecated("use setFont(&fonts::Font0)")]]
    void setTextFont(int f) {
      if (f == 1 && _font && _font->getType() == ft_gfx) return;

      setFont(fontdata[f]);
    }

    [[deprecated("use setFont(&fonts::Font0)")]]
    void setTextFont(const IFont* font = nullptr) { setFont(font); }

    [[deprecated("use setFont(&fonts::Font0)")]]
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
      _filled_x = 0;
      //_decoderState = utf8_decode_state_t::utf8_state0;

      font->getDefaultMetric(&_font_size);

      switch (font->getType()) {
      default:
      case font_type_t::ft_glcd: fpDrawChar = drawCharGLCD;  break;
      case font_type_t::ft_bmp:  fpDrawChar = drawCharBMP;   break;
      case font_type_t::ft_rle:  fpDrawChar = drawCharRLE;   break;
      case font_type_t::ft_bdf:  fpDrawChar = drawCharBDF;   break;
      case font_type_t::ft_gfx:  fpDrawChar = drawCharGFXFF; break;
      }
    }

    void cp437(bool enable = true) { _font_style.cp437 = enable; }  // AdafruitGFX compatible.

    void setAttribute(uint8_t attr_id, uint8_t param) { setAttribute((attribute_t)attr_id, param); }
    void setAttribute(attribute_t attr_id, uint8_t param) {
      switch (attr_id) {
        case cp437_switch:
            _font_style.cp437 = param;
            break;
        case utf8_switch:
            _font_style.utf8  = param;
            _decoderState = utf8_decode_state_t::utf8_state0;
            break;
        default: break;
      }
    }

    uint8_t getAttribute(uint8_t attr_id) { return getAttribute((attribute_t)attr_id); }
    uint8_t getAttribute(attribute_t attr_id) {
      switch (attr_id) {
        case cp437_switch: return _font_style.cp437;
        case utf8_switch: return _font_style.utf8;
        default: return 0;
      }
    }


#if defined (ARDUINO) && defined (FS_H)
    void loadFont(const char *path, fs::FS &fs) {
      _fontFile.setFS(fs);
      loadFont(path);
    }
#endif

    void loadFont(const uint8_t* array) {
      this->unloadFont();
      _fontPointer.set(array);
      auto font = new VLWfont();
      this->_dynamic_font = font;
      if (font->loadFont(&_fontPointer)) {
        this->_font = font;
        this->fpDrawChar = drawCharVLW;
        this->_font->getDefaultMetric(&this->_font_size);
      } else {
        this->unloadFont();
      }
    }

    void loadFont(const char *path) {
      this->unloadFont();

      this->prepareTmpTransaction(&_fontFile);
      _fontFile.preRead();

      bool result = _fontFile.open(path, "rb");
      if (!result) {
        std::string filename = "/";
        filename += path;
        filename += ".vlw";
        result = _fontFile.open(filename.c_str(), "rb");
      }
      auto font = new VLWfont();
      this->_dynamic_font = font;
      if (result) {
        result = font->loadFont(&_fontFile);
      }
      if (result) {
        this->_font = font;
        this->_font->getDefaultMetric(&this->_font_size);
        this->fpDrawChar = drawCharVLW;
      } else {
        this->unloadFont();
      }
      _fontFile.postRead();
    }

    void showFont(uint32_t td)
    {
      auto font = (const VLWfont*)this->_font;
      if(!font->_fontLoaded) return;

      int16_t x = this->width();
      int16_t y = this->height();
      uint32_t timeDelay = 0;    // No delay before first page

      this->fillScreen(this->_font_style.back_rgb888);

      for (uint16_t i = 0; i < font->gCount; i++)
      {
        // Check if this will need a new screen
        if (x + font->gdX[i] + font->gWidth[i] >= this->width())  {
          x = - font->gdX[i];

          y += font->yAdvance;
          if (y + font->maxAscent + font->descent >= this->height()) {
            x = - font->gdX[i];
            y = 0;
            delay(timeDelay);
            timeDelay = td;
            this->fillScreen(this->_font_style.back_rgb888);
          }
        }

        this->drawChar(font->gUnicode[i], x, y);
        x += font->gxAdvance[i];
        //yield();
      }

      delay(timeDelay);
      this->fillScreen(this->_font_style.back_rgb888);
      //fontFile.close();
    }

  protected:

    enum utf8_decode_state_t
    { utf8_state0 = 0
    , utf8_state1 = 1
    , utf8_state2 = 2
    };
    utf8_decode_state_t _decoderState = utf8_state0;   // UTF8 decoder state
    uint16_t _unicode_buffer = 0;   // Unicode code-point buffer

    int32_t _cursor_x = 0;
    int32_t _cursor_y = 0;
    int32_t _filled_x = 0;

    TextStyle _font_style;
    FontMetrics _font_size = { 6, 6, 0, 8, 8, 0, 7 }; // Font0 Metric
    const IFont* _font = &fonts::Font0;

    IFont* _dynamic_font = nullptr;  // run-time generated font
    FileWrapper _fontFile;
    PointerWrapper _fontPointer;

    int16_t _padding_x = 0;

    bool _textwrap_x = true;
    bool _textwrap_y = false;
    bool _textscroll = false;

//----------------------------------------------------------------------------
// print & text support
//----------------------------------------------------------------------------
// Arduino Print.h compatible
    size_t printNumber(unsigned long n, uint8_t base)
    {
      size_t len = 8 * sizeof(long) + 1;
      char buf[len];
      return write(numberToStr(n, buf, len, base));
    }

    size_t printFloat(double number, uint8_t digits)
    {
      size_t len = 14 + digits;
      char buf[len];
      return write(floatToStr(number, buf, len, digits));
    }

    char* numberToStr(long n, char* buf, size_t buflen, uint8_t base)
    {
      if (n >= 0) return numberToStr((unsigned long) n, buf, buflen, base);
      auto res = numberToStr(- n, buf, buflen, 10) - 1;
      res[0] = '-';
      return res;
    }

    char* numberToStr(unsigned long n, char* buf, size_t buflen, uint8_t base)
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

    char* floatToStr(double number, char* buf, size_t buflen, uint8_t digits)
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
      for(uint8_t i = 0; i < digits; ++i) {
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

    size_t draw_string(const char *string, int32_t x, int32_t y, textdatum_t datum)
    {
      int16_t sumX = 0;
      int32_t cwidth = textWidth(string); // Find the pixel width of the string in the font
      int32_t cheight = _font_size.y_size * _font_style.size_y;

      {
        auto tmp = string;
        do {
          uint16_t uniCode = *tmp;
          if (_font_style.utf8) {
            do {
              uniCode = decodeUTF8(*tmp); 
            } while (uniCode < 32 && *++tmp);
            if (uniCode < 32) break;
          }
          if (_font->updateFontMetric(&_font_size, uniCode)) {
            if (_font_size.x_offset < 0) sumX = - _font_size.x_offset * _font_style.size_x;
            break;
          }
        } while (*++tmp);
      }
      if (datum & middle_left) {          // vertical: middle
        y -= cheight >> 1;
      } else if (datum & bottom_left) {   // vertical: bottom
        y -= cheight;
      } else if (datum & baseline_left) { // vertical: baseline
        y -= _font_size.baseline * _font_style.size_y;
      }

      this->startWrite();
      int32_t padx = _padding_x;
      if ((_font_style.fore_rgb888 != _font_style.back_rgb888) && (padx > cwidth)) {
        this->setColor(_font_style.back_rgb888);
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

      y -= _font_size.y_offset * _font_style.size_y;

      _filled_x = 0;
      do {
        uint16_t uniCode = *string;
        if (_font_style.utf8) {
          do {
            uniCode = decodeUTF8(*string);
          } while (uniCode < 32 && *++string);
          if (uniCode < 32) break;
        }
        sumX += (fpDrawChar)(this, x + sumX, y, uniCode, &_font_style, _font);
      } while (*(++string));
      this->endWrite();

      return sumX;
    }

    size_t (*fpDrawChar)(LGFX_Font_Support* me, int32_t x, int32_t y, uint16_t c, const TextStyle* style, const IFont* font) = &LGFX_Font_Support::drawCharGLCD;

    static size_t drawCharGLCD(LGFX_Font_Support* me, int32_t x, int32_t y, uint16_t c, const TextStyle* style, const IFont* ifont)
    { // glcd font
      auto font = (const GLCDfont*)ifont;
      if (c > 255) return 0;

      if (!style->cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

      const int32_t fontWidth  = font->width;
      const int32_t fontHeight = font->height;

      auto font_addr = font->chartbl + (c * 5);
      uint32_t colortbl[2] = {me->_write_conv.convert(style->back_rgb888), me->_write_conv.convert(style->fore_rgb888)};
      bool fillbg = (style->back_rgb888 != style->fore_rgb888);

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + fontWidth * style->size_x ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * style->size_y ))) {
        if (!fillbg || style->size_y > 1 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * style->size_x > clip_right) {
          int32_t xpos = x;
          me->startWrite();

          int_fast8_t i = 0;
          do {
            int_fast16_t ypos = y;
            uint8_t line = pgm_read_byte(&font_addr[i]);
            uint8_t flg = (line & 0x01);
            int_fast8_t j = 1;
            int_fast8_t jp = 0;
            do {
              while (flg == ((line >> j) & 0x01) && ++j < fontHeight);
              jp = j - jp;
              if (flg || fillbg) {
                me->setRawColor(colortbl[flg]);
                me->writeFillRect(xpos, ypos, style->size_x, jp * style->size_y);
              }
              ypos += jp * style->size_y;
              flg = !flg;
              jp = j;
            } while (j < fontHeight);
            xpos += style->size_x;
          } while (++i < fontWidth - 1);

          if (fillbg) {
            me->setRawColor(colortbl[0]);
            me->writeFillRect(xpos, y, style->size_x, fontHeight * style->size_y); 
          }
          me->endWrite();
        } else {
          uint8_t col[fontWidth];
          int_fast8_t i = 0;
          do {
            col[i] = pgm_read_byte(&font_addr[i]);
          } while (++i < 5);
          col[5] = 0;
          me->startWrite();
          me->setAddrWindow(x, y, fontWidth * style->size_x, fontHeight);
          uint8_t flg = col[0] & 1;
          uint32_t len = 0;
          i = 0;
          do {
            int_fast8_t j = 0;
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
      }
      return fontWidth * style->size_x;
    }

    static size_t drawCharBMP(LGFX_Font_Support* me, int32_t x, int32_t y, uint16_t uniCode, const TextStyle* style, const IFont* ifont)
    { // BMP font
      auto font = (const BMPfont*)ifont;

      if ((uniCode -= 32) >= 96) return 0;
      const int_fast8_t fontWidth = pgm_read_byte(font->widthtbl + uniCode);
      const int_fast8_t fontHeight = pgm_read_byte(&font->height);

      auto font_addr = (const uint8_t*)pgm_read_dword(&((const uint8_t**)font->chartbl)[uniCode]);
      return draw_char_bmp(me, x, y, style, font_addr, fontWidth, fontHeight, (fontWidth + 6) >> 3, 1);
    }

    static size_t drawCharBDF(LGFX_Font_Support* me, int32_t x, int32_t y, uint16_t c, const TextStyle* style, const IFont* ifont)
    {
      auto font = (BDFfont*)ifont;
      const int_fast8_t bytesize = (font->width + 7) >> 3;
      const int_fast8_t fontHeight = font->height;
      const int_fast8_t fontWidth = (c < 0x0100) ? font->halfwidth : font->width;
      auto it = std::lower_bound(font->indextbl, &font->indextbl[font->indexsize], c);
      if (*it != c) {
        if (style->fore_rgb888 != style->back_rgb888) {
          me->fillRect(x, y, fontWidth * style->size_x, fontHeight * style->size_y, style->back_rgb888);
        }
        return fontWidth * style->size_x;
      }
      const uint8_t* font_addr = &font->chartbl[std::distance(font->indextbl, it) * fontHeight * bytesize];
      return LGFX_Font_Support::draw_char_bmp(me, x, y, style, font_addr, fontWidth, fontHeight, bytesize, 0);
    }

    static size_t draw_char_bmp(LGFX_Font_Support* me, int32_t x, int32_t y, const TextStyle* style, const uint8_t* font_addr, int_fast8_t fontWidth, int_fast8_t fontHeight, int_fast8_t w, int_fast8_t margin )
    {
      uint32_t colortbl[2] = {me->_write_conv.convert(style->back_rgb888), me->_write_conv.convert(style->fore_rgb888)};
      bool fillbg = (style->back_rgb888 != style->fore_rgb888);

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + fontWidth * style->size_x ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * style->size_y ))) {
        if (!fillbg || style->size_y > 1 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * style->size_x > clip_right) {
          me->startWrite();
          if (fillbg) {
            me->setRawColor(colortbl[0]);
            if (margin)
              me->writeFillRect(x + (fontWidth - margin) * style->size_x, y, style->size_x, fontHeight * style->size_y);
          }
          int_fast8_t i = 0;
          do {
            uint8_t line = pgm_read_byte(font_addr);
            bool flg = line & 0x80;
            int_fast8_t len = 1;
            int_fast8_t j = 1;
            int_fast8_t je = fontWidth - margin;
            do {
              if (j & 7) {
                line <<= 1;
              } else {
                line = pgm_read_byte(&font_addr[j >> 3]);
              }
              if (flg != (bool)(line & 0x80)) {
                if (flg || fillbg) {
                  me->setRawColor(colortbl[flg]);
                  me->writeFillRect( x + (j - len) * style->size_x, y, len * style->size_x, style->size_y); 
                }
                len = 1;
                flg = !flg;
              } else {
                ++len;
              }
            } while (++j < je);
            if (flg || fillbg) {
              me->setRawColor(colortbl[flg]);
              me->writeFillRect( x + (j - len) * style->size_x, y, len * style->size_x, style->size_y); 
            }
            y += style->size_y;
            font_addr += w;
          } while (++i < fontHeight);
          me->endWrite();
        } else {
          int_fast8_t len = 0;
          uint8_t line = 0;
          bool flg = false;
          me->startWrite();
          me->setAddrWindow(x, y, fontWidth * style->size_x, fontHeight);
          int_fast8_t i = 0;
          int_fast8_t je = fontWidth - margin;
          do {
            int_fast8_t j = 0;
            do {
              if (j & 7) {
                line <<= 1;
              } else {
                line = (j == je) ? 0 : pgm_read_byte(&font_addr[j >> 3]);
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
      }

      return fontWidth * style->size_x;
    }

    static size_t drawCharRLE(LGFX_Font_Support* me, int32_t x, int32_t y, uint16_t code, const TextStyle* style, const IFont* ifont)
    { // RLE font
      auto font = (RLEfont*)ifont;
      if ((code -= 32) >= 96) return 0;

      const int fontWidth = pgm_read_byte( (uint8_t *)pgm_read_dword( &(font->widthtbl ) ) + code );
      const int fontHeight = pgm_read_byte( &font->height );

      auto font_addr = (const uint8_t*)pgm_read_dword( (const void*)(pgm_read_dword( &(font->chartbl ) ) + code * sizeof(void *)) );

      uint32_t colortbl[2] = {me->_write_conv.convert(style->back_rgb888), me->_write_conv.convert(style->fore_rgb888)};
      bool fillbg = (style->back_rgb888 != style->fore_rgb888);

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + fontWidth * style->size_x ))
       && (y <= clip_bottom) && (clip_top < (y + fontHeight * style->size_y ))) {
        if (!fillbg || style->size_y > 1 || x < clip_left || y < clip_top || y + fontHeight > clip_bottom || x + fontWidth * style->size_x > clip_right) {
          bool flg = false;
          uint8_t line = 0, i = 0, j = 0;
          int32_t len;
          me->startWrite();
          do {
            line = pgm_read_byte(font_addr++);
            flg = line & 0x80;
            line = (line & 0x7F)+1;
            do {
              len = (j + line > fontWidth) ? fontWidth - j : line;
              line -= len;
              if (fillbg || flg) {
                me->setRawColor(colortbl[flg]);
                me->writeFillRect( x + j * style->size_x, y + (i * style->size_y), len * style->size_x, style->size_y);
              }
              j += len;
              if (j == fontWidth) {
                j = 0;
                i++;
              }
            } while (line);
          } while (i < fontHeight);
          me->endWrite();
        } else {
          uint32_t line = 0;
          me->startWrite();
          me->setAddrWindow(x, y, fontWidth * style->size_x, fontHeight);
          uint32_t len = fontWidth * style->size_x * fontHeight;
          do {
            line = pgm_read_byte(font_addr++);
            bool flg = line & 0x80;
            line = ((line & 0x7F) + 1) * style->size_x;
            me->writeRawColor(colortbl[flg], line);
          } while (len -= line);
          me->endWrite();
        }
      }

      return fontWidth * style->size_x;
    }

    static size_t drawCharGFXFF(LGFX_Font_Support* me, int32_t x, int32_t y, uint16_t uniCode, const TextStyle* style, const IFont* ifont)
    {
      auto font = (const GFXfont*)ifont;
      auto glyph = font->getGlyph(uniCode);
      if (!glyph) return 0;

      int32_t w = pgm_read_byte(&glyph->width),
              h = pgm_read_byte(&glyph->height);

      int32_t xAdvance = (int32_t)style->size_x * (int8_t)pgm_read_byte(&glyph->xAdvance);
      int32_t xoffset  = (int32_t)style->size_x * (int8_t)pgm_read_byte(&glyph->xOffset);
      int32_t yoffset  = (int32_t)style->size_y * (int8_t)pgm_read_byte(&glyph->yOffset);

      me->startWrite();
      uint32_t colortbl[2] = {me->_write_conv.convert(style->back_rgb888), me->_write_conv.convert(style->fore_rgb888)};
      bool fillbg = (style->back_rgb888 != style->fore_rgb888);
      int32_t left  = 0;
      int32_t right = 0;
      if (fillbg) {
        left  = std::max(me->_filled_x, x + (xoffset < 0 ? xoffset : 0));
        right = x + std::max((int32_t)(w * style->size_x + xoffset), (int32_t)(xAdvance));
      }

      x += xoffset;
      y += yoffset;

      int32_t clip_left   = me->_clip_l;
      int32_t clip_right  = me->_clip_r;
      int32_t clip_top    = me->_clip_t;
      int32_t clip_bottom = me->_clip_b;

      if ((x <= clip_right) && (clip_left < (x + w * style->size_x ))
       && (y <= clip_bottom) && (clip_top < (y + h * style->size_y ))) {

        if (right > left) {
          me->setRawColor(colortbl[0]);
          int tmp = yoffset - (me->_font_size.y_offset * style->size_y);
          if (tmp > 0)
            me->writeFillRect(left, y - yoffset + me->_font_size.y_offset * style->size_y, right - left, tmp);

          tmp = (me->_font_size.y_offset + me->_font_size.y_size - h) * style->size_y - yoffset;
          if (tmp > 0)
            me->writeFillRect(left, y + h * style->size_y, right - left, tmp);
        }

        uint8_t  *bitmap = (uint8_t *)pgm_read_dword(&font->bitmap)
                         + pgm_read_dword(&glyph->bitmapOffset);
        uint8_t bits=0, bit=0;

        me->setRawColor(colortbl[1]);
        while (h--) {
          if (right > left) {
            me->setRawColor(colortbl[0]);
            me->writeFillRect(left, y, right - left, style->size_y);
            me->setRawColor(colortbl[1]);
          }

          int32_t len = 0;
          int32_t i = 0;
          for (i = 0; i < w; i++) {
            if (bit == 0) {
              bit  = 0x80;
              bits = pgm_read_byte(bitmap++);
            }
            if (bits & bit) len++;
            else if (len) {
              me->writeFillRect(x + (i-len) * style->size_x, y, style->size_x * len, style->size_y);
              len=0;
            }
            bit >>= 1;
          }
          if (len) {
            me->writeFillRect(x + (i-len) * style->size_x, y, style->size_x * len, style->size_y);
          }
          y += style->size_y;
        }
      } else {
        if (right > left) {
          me->setRawColor(colortbl[0]);
          me->writeFillRect(left, y - yoffset + me->_font_size.y_offset * style->size_y, right - left, (me->_font_size.y_size) * style->size_y);
        }
      }
      me->_filled_x = right;
      me->endWrite();
      return xAdvance;
    }

    static size_t drawCharVLW(LGFX_Font_Support* lgfxbase, int32_t x, int32_t y, uint16_t code, const TextStyle* style, const IFont*)
    {
      auto me = (LGFX_Font_Support*)lgfxbase;
      auto font = (const VLWfont*)me->_font;

      uint16_t gNum = 0;
      if (!font->getUnicodeIndex(code, &gNum)) {
        return 0;
      }

      auto file = font->_fontData;

      file->preRead();

      file->seek(28 + gNum * 28);  // headerPtr
      uint32_t buffer[6];
      file->read((uint8_t*)buffer, 24);
      uint32_t h        = __builtin_bswap32(buffer[0]); // Height of glyph
      uint32_t w        = __builtin_bswap32(buffer[1]); // Width of glyph
      uint32_t xAdvance = __builtin_bswap32(buffer[2]) * style->size_x; // xAdvance - to move x cursor
      int32_t xoffset   = (int32_t)((int8_t)__builtin_bswap32(buffer[4])) * style->size_x; // x delta from cursor
      int32_t dY        = (int16_t)__builtin_bswap32(buffer[3]); // y delta from baseline
      int32_t yoffset = ((int32_t)font->maxAscent - dY) * (int32_t)style->size_y;

      uint8_t pbuffer[w * h];
      uint8_t* pixel = pbuffer;

      file->seek(font->gBitmap[gNum]);  // headerPtr
      file->read(pixel, w * h);

      file->postRead();

      me->startWrite();

      uint32_t colortbl[2] = {me->_write_conv.convert(style->back_rgb888), me->_write_conv.convert(style->fore_rgb888)};
      bool fillbg = (style->back_rgb888 != style->fore_rgb888);
      int32_t left  = 0;
      int32_t right = 0;
      if (fillbg) {
        left  = std::max(me->_filled_x, x + (xoffset < 0 ? xoffset : 0));
        right = x + std::max((int32_t)(w * style->size_x + xoffset), (int32_t)(xAdvance));
      }
      me->_filled_x = right;

      y += yoffset;
      x += xoffset;
      int32_t l = 0;
      int32_t bx = x;
      int32_t bw = w * style->size_x;
      int32_t clip_left = me->_clip_l;
      if (x < clip_left) { l = -((x - clip_left) / style->size_x); bw += (x - clip_left); bx = clip_left; }
      int32_t clip_right = me->_clip_r + 1;
      if (bw > clip_right - bx) bw = clip_right - bx;
      if (bw > 0 && (y <= me->_clip_b) && (me->_clip_t < (y + h * style->size_y))) {
        int32_t fore_r = ((style->fore_rgb888>>16)&0xFF);
        int32_t fore_g = ((style->fore_rgb888>> 8)&0xFF);
        int32_t fore_b = ((style->fore_rgb888)    &0xFF);

        if (fillbg) { // fill background mode
          if (right > left) {
            me->setRawColor(colortbl[0]);
            int tmp = yoffset - (me->_font_size.y_offset * style->size_y);
            if (tmp > 0)
              me->writeFillRect(left, y - yoffset + me->_font_size.y_offset * style->size_y, right - left, tmp);

            tmp = (me->_font_size.y_offset + me->_font_size.y_size - h) * style->size_y - yoffset;
            if (tmp > 0)
              me->writeFillRect(left, y + h * style->size_y, right - left, tmp);
          }

          int32_t back_r = ((style->back_rgb888>>16)&0xFF);
          int32_t back_g = ((style->back_rgb888>> 8)&0xFF);
          int32_t back_b = ((style->back_rgb888)    &0xFF);
          int32_t r = (clip_right - x + style->size_x - 1) / style->size_x;
          if (r > w) r = w;
          do {
            if (right > left) {
              me->setRawColor(colortbl[0]);
              me->writeFillRect(left, y, right - left, style->size_y);
            }
            int32_t i = l;
            do {
              while (pixel[i] != 0xFF) {
                if (pixel[i] != 0) {
                  int32_t p = 1 + (uint32_t)pixel[i];
                  me->setColor(color888( ( fore_r * p + back_r * (257 - p)) >> 8
                                       , ( fore_g * p + back_g * (257 - p)) >> 8
                                       , ( fore_b * p + back_b * (257 - p)) >> 8 ));
                  me->writeFillRect(i * style->size_x + x, y, style->size_x, style->size_y);
                }
                if (++i == r) break;
              }
              if (i == r) break;
              int32_t dl = 1;
              while (i + dl != r && pixel[i + dl] == 0xFF) { ++dl; }
              me->setRawColor(colortbl[1]);
              me->writeFillRect(x + i * style->size_x, y, dl * style->size_x, style->size_y);
              i += dl;
            } while (i != r);
            pixel += w;
            y += style->size_y;
          } while (--h);

        } else { // alpha blend mode

          int32_t xshift = (bx - x) % style->size_x;
          bgr888_t buf[bw * style->size_y];
          pixelcopy_t p(buf, me->_write_conv.depth, rgb888_3Byte, me->hasPalette());
          do {
            int32_t by = y;
            int32_t bh = style->size_y;
            if (by < 0) { bh += by; by = 0; }
            if (bh > 0) {
              me->readRectRGB(bx, by, bw, bh, (uint8_t*)buf);
              for (int32_t sx = 0; sx < bw; sx++) {
                int32_t p = 1 + pixel[left + (sx+xshift) / style->size_x];
                int32_t sy = 0;
                do {
                  auto bgr = &buf[sx + sy * bw];
                  bgr->r = ( fore_r * p + bgr->r * (257 - p)) >> 8;
                  bgr->g = ( fore_g * p + bgr->g * (257 - p)) >> 8;
                  bgr->b = ( fore_b * p + bgr->b * (257 - p)) >> 8;
                } while (++sy < bh);
              }
              me->push_image(bx, by, bw, bh, &p);
            }
            pixel += w;
            if ((y += style->size_y) >= me->height()) break;
          } while (--h);
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
