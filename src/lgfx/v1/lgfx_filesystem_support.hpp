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

#include <cmath>
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

    inline bool drawBmp(fs::FS &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawBmpFile(fs, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawBmpFile(fs::FS &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      FileWrapper file(fs);
      return this->drawBmpFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawBmpFile(fs::FS &fs, const String& path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawBmpFile(fs, path.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    inline bool drawJpgFile(fs::FS &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      FileWrapper file(fs);
      return this->drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawJpgFile(fs::FS &fs, const String& path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawJpgFile(fs, path.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]]
    inline bool drawJpgFile(fs::FS &fs, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(fs, path, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

    inline bool drawPngFile(fs::FS &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      FileWrapper file(fs);
      return this->drawPngFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawPngFile(fs::FS &fs, const String& path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawPngFile(fs, path.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }


    inline bool drawBmpFile(fs::FS &fs, fs::File *file, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      FileWrapper data(fs, file);
      this->prepareTmpTransaction(&data);
      return this->draw_bmp(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    inline bool drawJpgFile(fs::FS &fs, fs::File *file, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      FileWrapper data(fs, file);
      this->prepareTmpTransaction(&data);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]]
    inline bool drawJpgFile(fs::FS &fs, fs::File *file, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(fs, file, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

    inline bool drawPngFile(fs::FS &fs, fs::File *file, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      FileWrapper data(fs, file);
      this->prepareTmpTransaction(&data);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }


    inline bool drawBmp(fs::File *dataSource, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      StreamWrapper data;
      data.set(dataSource);
      data.need_transaction = true;
      this->prepareTmpTransaction(&data);
      return this->draw_bmp(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    inline bool drawJpg(fs::File *dataSource, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      StreamWrapper data;
      data.set(dataSource);
      data.need_transaction = true;
      this->prepareTmpTransaction(&data);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]]
    inline bool drawJpg(fs::File *dataSource, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpg(dataSource, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

    inline bool drawPng(fs::File *dataSource, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      StreamWrapper data;
      data.set(dataSource);
      data.need_transaction = true;
      this->prepareTmpTransaction(&data);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

 #endif

 #if defined (SdFat_h)

    void loadFont(const char *path, SdBase<FsVolume> &fs)
    {
      init_font_file<SdFatWrapper>(fs);
      load_font_with_path(path);
    }

    void loadFont(SdBase<FsVolume> &fs, const char *path)
    {
      init_font_file<SdFatWrapper>(fs);
      load_font_with_path(path);
    }

    inline bool drawBmp(SdBase<FsVolume> &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawBmpFile(fs, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawBmpFile(SdBase<FsVolume> &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      SdFatWrapper file(fs);
      return this->drawBmpFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawBmpFile(SdBase<FsVolume> &fs, const String& path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawBmpFile(fs, path.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    inline bool drawJpgFile(SdBase<FsVolume> &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      SdFatWrapper file(fs);
      return this->drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawJpgFile(SdBase<FsVolume> &fs, const String& path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawJpgFile(fs, path.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]]
    inline bool drawJpgFile(SdBase<FsVolume> &fs, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(fs, path, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

    inline bool drawPngFile(SdBase<FsVolume> &fs, const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      SdFatWrapper file(fs);
      return this->drawPngFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawPngFile(SdBase<FsVolume> &fs, const String& path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawPngFile(fs, path.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }


    inline bool drawBmpFile(SdBase<FsVolume> &fs, FsFile *file, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      SdFatWrapper data(fs, file);
      this->prepareTmpTransaction(&data);
      return this->draw_bmp(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    inline bool drawJpgFile(SdBase<FsVolume> &fs, FsFile *file, int32_t x=0, int32_t y=0, int32_t maxWidth=0, int32_t maxHeight=0, int32_t offX=0, int32_t offY=0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      SdFatWrapper data(fs, file);
      this->prepareTmpTransaction(&data);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]]
    inline bool drawJpgFile(SdBase<FsVolume> &fs, FsFile *file, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(fs, file, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

    inline bool drawPngFile(SdBase<FsVolume> &fs, FsFile *file, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      SdFatWrapper data(fs, file);
      this->prepareTmpTransaction(&data);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

 #endif

 #if defined (Stream_h)

    inline bool drawBmp(Stream *dataSource, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      StreamWrapper data;
      data.set(dataSource);
      return this->draw_bmp(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    inline bool drawJpg(Stream *dataSource, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      StreamWrapper data;
      data.set(dataSource);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]]
    inline bool drawJpg(Stream *dataSource, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpg(dataSource, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

    inline bool drawPng(Stream *dataSource, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      StreamWrapper data;
      data.set(dataSource);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
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


    inline bool drawBmpUrl(const char* url, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      HttpWrapper http;
      return http.open(url) && drawBmp(&http, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawBmpUrl(const String& url, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawBmpUrl(url.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

    inline bool drawJpgUrl(const char* url, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      HttpWrapper http;
      return http.open(url) && drawJpg(&http, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawJpgUrl(const String& url, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawJpgUrl(url.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]]
    inline bool drawJpgUrl(const char* url, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgUrl(url, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }

    inline bool drawPngUrl(const char* url, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      HttpWrapper http;
      return http.open(url) && drawPng(&http, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawPngUrl(const String& url, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      return drawPngUrl(url.c_str(), x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }

  #endif
 #endif

#elif defined (CONFIG_IDF_TARGET_ESP32) || defined(__SAMD51_HARMONY__) || defined(_INC_STDIO) // ESP-IDF or Harmony

    inline bool drawBmpFile(const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      FileWrapper file;
      return drawBmpFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    inline bool drawJpgFile(const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      FileWrapper file;
      return drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
    }
    [[deprecated("use float scale")]]
    inline bool drawJpgFile(const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, jpeg_div::jpeg_div_t scale)
    {
      return drawJpgFile(path, x, y, maxWidth, maxHeight, offX, offY, 1.0f / (1 << scale));
    }
    inline bool drawPngFile(const char *path, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left)
    {
      FileWrapper file;
      return drawPngFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
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

    bool drawBmpFile(DataWrapper* file, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float scale_x, float scale_y, datum_t datum)
    {
      bool res = false;
      this->prepareTmpTransaction(file);
      file->preRead();
      if (file->open(path))
      {
        res = this->draw_bmp(file, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
        file->close();
      }
      file->postRead();
      return res;
    }

    bool drawJpgFile(DataWrapper* file, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float scale_x, float scale_y, datum_t datum)
    {
      bool res = false;
      this->prepareTmpTransaction(file);
      file->preRead();
      if (file->open(path))
      {
        res = this->draw_jpg(file, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
        file->close();
      }
      file->postRead();
      return res;
    }

    bool drawPngFile(DataWrapper* file, const char *path, int32_t x, int32_t y, int32_t maxWidth, int32_t maxHeight, int32_t offX, int32_t offY, float scale_x, float scale_y, datum_t datum)
    {
      bool res = false;
      this->prepareTmpTransaction(file);
      file->preRead();
      if (file->open(path))
      {
        res = this->draw_png(file, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum);
        file->close();
      }
      file->postRead();
      return res;
    }
  };

//----------------------------------------------------------------------------
 }
}

#endif
