#include <stdint.h>

namespace lgfx
{
  struct FontMetrics
  {
    uint16_t width;
    uint16_t height;
  };

  struct IFont
  {
    virtual bool isProportional() const = 0;
    virtual FontMetrics getCharMetrics(const char* utf8_char) const = 0;

    template <typename TLGFX>
    virtual void drawChar(&TLGFX lgfx, int32_t x, int32_t y, uint16_t unicode, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y) = 0;
  };

  class TextFontGLCD : public IFont
  {
  private:
    void* font_data;
    TextFontGLCD(void* font_data) : font_data(font_data) {}
  public:
    bool isProportional() const override { return false; }
    FontMetrics getCharMetrics(const char* utf8_char) const override { return FontMetrics{6,8}; }

    template <typename TLGFX>
    void drawChar(&TLGFX lgfx, int32_t x, int32_t y, uint16_t unicode, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y);
  };

  class TextFontBMP : public IFont
  {
  private:
    void* font_data;

    TextFont(void* font_data) : font_type(font_type), font_data(font_data) {}

    int16_t drawCharGLCD(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y);
    int16_t drawCharBMP(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y);
    int16_t drawCharRLE(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y);
  public:
    bool isProportional() const override { return false; }
    FontMetrics getCharMetrics(const char* utf8_char) const override { ... }
    FontMetrics getTextMetrics(const char* utf8_text) const override { ... }
    void drawText(const char* utf8_text, std::uint16_t x, std::uint16_t y, std::uint16_t text_color, std::uint16_t backgroud_color) override {}
  };

  class TextFontBMP : public IFont
  {
  private:
    enum class FontType {
      GLCD,
      BMP,
      RLE,
    };
    FontType font_type;
    void* font_data;

    TextFont(FontType font_type, void* font_data) : font_type(font_type), font_data(font_data) {}

    int16_t drawCharGLCD(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y);
    int16_t drawCharBMP(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y);
    int16_t drawCharRLE(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size_x, uint8_t size_y);
  public:
    bool isProportional() const override { return false; }
    FontMetrics getCharMetrics(const char* utf8_char) const override { ... }
    FontMetrics getTextMetrics(const char* utf8_text) const override { ... }
    void drawText(const char* utf8_text, std::uint16_t x, std::uint16_t y, std::uint16_t text_color, std::uint16_t backgroud_color) override {}
  };

  // 各フォントのソースで定義
  TextFont font7srle(TextFont::FontType::RLE, ...);  // Font7srle.cpp
  TextFont font16(TextFont::FontType::RLE, ...);    // Font16.cpp

  // システムのフォント定義ファイル (font_table.hpp)みたいな？

  enum class FontID 
  {
    // ここでマクロの有無でテーブルに登録するフォントのIDを追加･削除する
    FONT_7SRLE,
    FONT_16,
  };

  struct FontTableEntry
  {
    FontID id;
    IFont* font;
  };

  extern const FontTableEntry fonts_defined[];

  static IFont* getFontFromID(FontID id) {
    // fonts_definedからidで探索して返す
  }

  // font_table.cpp
  const FontTableEntry fonts_defined[] {
    // ここでマクロの有無でテーブルに登録するフォントを追加・削除する
    { FONT_7SRLE, &font7srle },
    { FONT_16, &font16 },
  };

};
