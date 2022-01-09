/*----------------------------------------------------------------------------/
  Lovyan GFX library - LCD graphics library .

  support platform:
    ESP32 (SPI/I2S) with Arduino/ESP-IDF
    ATSAMD51 (SPI) with Arduino

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
#ifndef LGFX_FILESYSTEM_SUPPORT_HPP_
#define LGFX_FILESYSTEM_SUPPORT_HPP_

#include "misc/enum.hpp"

#include <math.h>
#include <string.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  /// Warning : Do not add any data members to this class.
  /// This class only allows include order dependent extension methods.
  /// ( e.g. file system related, HTTP related )
  /// このクラスにメンバ変数を追加してはならない。(コンパイル単位間でLovyanGFXクラスに不整合が生じる);
  /// このクラスにはinclude順依存の機能拡張メソッドのみ追加を許可する。;
  /// ( 例：ファイルシステム関連、HTTP関連 );
  template <class Base>
  class LGFX_FILESYSTEM_Support : public Base
  {
  public:

    using Base::drawBmp;
    using Base::drawJpg;
    using Base::drawPng;
    using Base::drawQoi;
    using Base::loadFont;

    virtual ~LGFX_FILESYSTEM_Support<Base>()
    {
      if (this->_font_file != nullptr)
      {
        delete this->_font_file;
        this->_font_file = nullptr;
      }
    }

#if defined (ARDUINO)
 #if defined (FS_H) || defined (__SEEED_FS__)

    /// load vlw fontdata from filesystem.
    void loadFont(const char *path, fs::FS &fs
#if defined (_SD_H_)
 = SD
#elif defined (_SPIFFS_H_)
 = SPIFFS
#endif
    )
    {
      init_font_file<FileWrapper>(fs);
      load_font_with_path(path);
    }

    void loadFont(fs::FS &fs, const char *path)
    {
      init_font_file<FileWrapper>(fs);
      load_font_with_path(path);
    }

  #define LGFX_FUNCTION_GENERATOR(drawImg, draw_img) \
    inline bool drawImg##File(fs::FS &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    {  FileWrapper file(fs); \
       return this->drawImg##File(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \
    inline bool drawImg##File(fs::FS &fs, fs::File *file, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      FileWrapper data(fs, file); \
      return this->draw_img(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \
    inline bool drawImg##File(fs::FS &fs, const String& path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      return drawImg##File(fs, path.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \
    inline bool drawImg(fs::File *dataSource, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      StreamWrapper data; \
      data.set(dataSource); \
      data.need_transaction = true; \
      return this->draw_img(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \

    LGFX_FUNCTION_GENERATOR(drawBmp, draw_bmp)
    LGFX_FUNCTION_GENERATOR(drawJpg, draw_jpg)
    LGFX_FUNCTION_GENERATOR(drawPng, draw_png)
    LGFX_FUNCTION_GENERATOR(drawQoi, draw_qoi)

  #undef LGFX_FUNCTION_GENERATOR

    inline bool drawBmp(fs::FS &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawBmpFile(fs, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    [[deprecated("use float scale")]]
    inline bool drawJpgFile(fs::FS &fs, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(fs, path, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }
    [[deprecated("use float scale")]]
    inline bool drawJpgFile(fs::FS &fs, fs::File *file, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(fs, file, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }
    [[deprecated("use float scale")]]
    inline bool drawJpg(fs::File *dataSource, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpg(dataSource, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }


 #endif

 #if defined (SdFat_h)
  #if SD_FAT_VERSION >= 20102
   #define LGFX_SDFAT_TYPE SdBase<FsVolume,FsFormatter>
  #else
   #define LGFX_SDFAT_TYPE SdBase<FsVolume>
  #endif

    void loadFont(const char *path, LGFX_SDFAT_TYPE &fs)
    {
      init_font_file<SdFatWrapper>(fs);
      load_font_with_path(path);
    }

    void loadFont(LGFX_SDFAT_TYPE &fs, const char *path)
    {
      init_font_file<SdFatWrapper>(fs);
      load_font_with_path(path);
    }

  #define LGFX_FUNCTION_GENERATOR(drawImg, draw_img) \
    inline bool drawImg##File(LGFX_SDFAT_TYPE &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      SdFatWrapper file(fs); \
      return this->drawImg##File(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \
    inline bool drawImg##File(LGFX_SDFAT_TYPE &fs, FsFile *file, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      SdFatWrapper data(fs, file); \
      return this->draw_img(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \
    inline bool drawImg##File(LGFX_SDFAT_TYPE &fs, const String& path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      return drawImg##File(fs, path.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \

    LGFX_FUNCTION_GENERATOR(drawBmp, draw_bmp)
    LGFX_FUNCTION_GENERATOR(drawJpg, draw_jpg)
    LGFX_FUNCTION_GENERATOR(drawPng, draw_png)
    LGFX_FUNCTION_GENERATOR(drawQoi, draw_qoi)

  #undef LGFX_FUNCTION_GENERATOR

    inline bool drawBmp(LGFX_SDFAT_TYPE &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawBmpFile(fs, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]]
    inline bool drawJpgFile(LGFX_SDFAT_TYPE &fs, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(fs, path, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }
    [[deprecated("use float scale")]]
    inline bool drawJpgFile(LGFX_SDFAT_TYPE &fs, FsFile *file, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(fs, file, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

  #undef LGFX_SDFAT_TYPE
 #endif

 #if defined (Stream_h)

  #define LGFX_FUNCTION_GENERATOR(drawImg, draw_img) \
    inline bool drawImg(Stream *dataSource, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      StreamWrapper data; \
      data.set(dataSource); \
      data.need_transaction = this->isBusShared(); \
      return this->draw_img(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \

    LGFX_FUNCTION_GENERATOR(drawBmp, draw_bmp)
    LGFX_FUNCTION_GENERATOR(drawJpg, draw_jpg)
    LGFX_FUNCTION_GENERATOR(drawPng, draw_png)
    LGFX_FUNCTION_GENERATOR(drawQoi, draw_qoi)

  #undef LGFX_FUNCTION_GENERATOR

    [[deprecated("use float scale")]]
    inline bool drawJpg(Stream *dataSource, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpg(dataSource, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

  #if defined (HTTPClient_H_)

    struct HttpWrapper : public StreamWrapper
    {
      int read(uint8_t *buf, uint32_t len) override {
        while (_http.connected() && !_stream->available() && _index < _length) delay(1);
        return StreamWrapper::read(buf, len);
      }

      bool open(const char* url) {
        _http.begin(url);
        int httpCode = _http.GET();
        set(_http.getStreamPtr(), _http.getSize());
        if (httpCode == HTTP_CODE_OK) return true;

        log_e("HTTP ERROR: %d\n", httpCode);
        return false;
      }

      void close(void) override { _http.end(); }

    private:
      HTTPClient _http;
    };

  #define LGFX_FUNCTION_GENERATOR(drawImg) \
    inline bool drawImg##Url(const char* url, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      HttpWrapper http; \
      return http.open(url) && drawImg(&http, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \
    inline bool drawImg##Url(const String& url, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      return drawImg##Url(url.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \

    LGFX_FUNCTION_GENERATOR(drawBmp)
    LGFX_FUNCTION_GENERATOR(drawJpg)
    LGFX_FUNCTION_GENERATOR(drawPng)
    LGFX_FUNCTION_GENERATOR(drawQoi)

  #undef LGFX_FUNCTION_GENERATOR

    [[deprecated("use float scale")]]
    inline bool drawJpgUrl(const char* url, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgUrl(url, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

  #endif
 #endif

#elif defined (CONFIG_IDF_TARGET_ESP32) || defined(__SAMD51_HARMONY__) || defined(_INC_STDIO) // ESP-IDF or Harmony

  #define LGFX_FUNCTION_GENERATOR(drawImg) \
    inline bool drawImg##File(const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      FileWrapper file; \
      return drawImg##File(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \

    LGFX_FUNCTION_GENERATOR(drawBmp)
    LGFX_FUNCTION_GENERATOR(drawJpg)
    LGFX_FUNCTION_GENERATOR(drawPng)
    LGFX_FUNCTION_GENERATOR(drawQoi)

  #undef LGFX_FUNCTION_GENERATOR

    [[deprecated("use float scale")]]
    inline bool drawJpgFile(const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(path, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

#endif

  private:

    template<typename T, typename Tfs>
    void init_font_file(Tfs &fs)
    {
      this->unloadFont();
      if (this->_font_file != nullptr)
      {
        delete this->_font_file;
      }
      auto wrapper = new T(fs);
      //wrapper->setFS(fs);
      this->_font_file = wrapper;
    }

    bool load_font_with_path(const char *path)
    {
      this->unloadFont();

      if (this->_font_file == nullptr) return false;
      //if (this->_font_file == nullptr) { init_font_file<FileWrapper>(SD); }

      this->prepareTmpTransaction(this->_font_file);
      this->_font_file->preRead();

      bool result = this->_font_file->open(path);
      if (!result)
      {
        char filename[strlen(path) + 8] = {'/', 0 };
        strcpy(&filename[1], &path[(path[0] == '/') ? 1 : 0]);
        int len = strlen(filename);
        if (memcmp(&filename[len - 4], ".vlw", 4))
        {
          strcpy(&filename[len], ".vlw");
        }
        result = this->_font_file->open(filename);
      }

      if (result) {
        result = this->load_font(this->_font_file);
      }
      this->_font_file->postRead();
      return result;
    }

  #define LGFX_FUNCTION_GENERATOR(drawImg, draw_img) \
    bool drawImg##File(DataWrapper* file, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float scale_x, float scale_y, datum_t datum) \
    { \
      bool res = false; \
      this->prepareTmpTransaction(file); \
      file->preRead(); \
      if (file->open(path)) \
      { \
        res = this->draw_img(file, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
        file->close(); \
      } \
      file->postRead(); \
      return res; \
    } \

    LGFX_FUNCTION_GENERATOR(drawBmp, draw_bmp)
    LGFX_FUNCTION_GENERATOR(drawJpg, draw_jpg)
    LGFX_FUNCTION_GENERATOR(drawPng, draw_png)
    LGFX_FUNCTION_GENERATOR(drawQoi, draw_qoi)

  #undef LGFX_FUNCTION_GENERATOR
  };

//----------------------------------------------------------------------------
 }
}

#endif
