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
#if defined ( _WINSOCK2API_ )

    struct HttpWrapper : public DataWrapper
    {
      bool open(const char* url) override
      {
        if (strlen(url) > 900) { return false; }
        const char* urlpart_host = strstr(url, "//");
        if (urlpart_host == nullptr) { return false; }
        urlpart_host += 2;
        const char* urlpart_path = strstr(urlpart_host, "/");
        if (urlpart_path == nullptr)
        {
          urlpart_path = &url[strlen(url)];
        }

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
        {
          _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
          struct hostent* Host;
          char* hostname = (char*)alloca(urlpart_path - urlpart_host + 1);
          memcpy(hostname, urlpart_host, urlpart_path - urlpart_host);
          hostname[urlpart_path - urlpart_host] = 0;
          Host = gethostbyname(hostname);

          SOCKADDR_IN SockAddr;
          SockAddr.sin_port = htons(80);
          SockAddr.sin_family = AF_INET;
          SockAddr.sin_addr.s_addr = *((unsigned long*)Host->h_addr);

          if (connect(_socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) == 0)
          {
            char* get_http = (char*)alloca(1024);

            memset(get_http, ' ', sizeof(get_http));
            strcpy(get_http, "GET ");
            if (urlpart_path[0] == 0)
            {
              strcat(get_http, "/");
            }
            else
            {
              strcat(get_http, urlpart_path);
            }
            strcat(get_http, " HTTP/1.1\r\nHost: ");
            strcat(get_http, hostname);
            strcat(get_http, "\r\nConnection: close\r\n\r\n");

            send(_socket, get_http, strlen(get_http), 0);

            parseHttpHeader();

            if (_http_code == 200)
            {
              return true;
            }
          }
          closesocket(_socket);
        }
//      WSACleanup();
        return false;
      }

      void close(void) override
      {
        closesocket(_socket);
//      WSACleanup();
      }

      int read(uint8_t* buf, uint32_t len) override
      {
        int32_t res = recv(_socket, (char*)buf, (int)len, 0);
        _index += res;
        return res;
      }
      void skip(int32_t offset) override
      {
        if (0 >= offset) { return; }
        _index += offset;
        uint8_t dummy[64];
        size_t len = ((offset - 1) & 63) + 1;
        do
        {
          read(dummy, len);
          offset -= len;
          len = 64;
        } while (offset);
      }

      bool seek(uint32_t offset) { return false; }
      int32_t tell(void) { return _index; }

      protected:
      SOCKET _socket;
      int32_t _index = 0;
      int32_t _content_length = ~0u;
      int32_t _http_code = 0;
      void checkHeaderString(const char* str)
      {
        if (_http_code == 0        && memcmp(str, "HTTP/1.1 "        , 9) == 0) { _http_code      = atoi(&str[ 9]); }
        if (_content_length == ~0u && memcmp(str, "Content-Length: ", 16) == 0) { _content_length = atoi(&str[16]); }
      }

      void parseHttpHeader(void)
      {
        char buffer[257];

        int index = 0;
        int readlen;
        int limit = -1;
        while ((readlen = recv(_socket, &buffer[index], 1, 0)) > 0)
        {
          if (buffer[index] == '\r')
          {
            if (limit == -1)
            {
              buffer[index] = 0;
              checkHeaderString(buffer);
              limit = index;
            }
            index = 0;
          }
          else if (buffer[index] == '\n')
          {
            if (limit == 0)
            {
              return;
            }
            limit = -1;
          }
          else
          {
            index = (index + readlen) & 0xFF;
          }
        }
      }
    };

  #define LGFX_FUNCTION_GENERATOR(drawImg) \
    inline bool drawImg##Url(const char* url, int32_t x = 0, int32_t y = 0, int32_t maxWidth = 0, int32_t maxHeight = 0, int32_t offX = 0, int32_t offY = 0, float scale_x = 1.0f, float scale_y = 0.0f, datum_t datum = datum_t::top_left) \
    { \
      HttpWrapper http; \
      return http.open(url) && drawImg(&http, x, y, maxWidth, maxHeight, offX, offY, scale_x, scale_y, datum); \
    } \

    LGFX_FUNCTION_GENERATOR(drawBmp)
    LGFX_FUNCTION_GENERATOR(drawJpg)
    LGFX_FUNCTION_GENERATOR(drawPng)
    LGFX_FUNCTION_GENERATOR(drawQoi)

  #undef LGFX_FUNCTION_GENERATOR

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
