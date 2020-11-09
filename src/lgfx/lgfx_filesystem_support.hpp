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

#include <cmath>
#include <cstring>
#include <string>

#include "../Fonts/lgfx_fonts.hpp"

namespace lgfx
{
  template <class Base>
  class LGFX_FILESYSTEM_Support : public Base
  {
    FileWrapper _font_file;
  public:

    using Base::drawBmp;
    using Base::drawJpg;
    using Base::drawPng;
    using Base::loadFont;

    bool loadFont(const char *path)
    {
      this->unloadFont();

      this->prepareTmpTransaction(&this->_font_file);
      this->_font_file.preRead();

      bool result = this->_font_file.open(path, "r");
      if (!result) {
        std::string filename = "/";
        if (path[0] == '/') filename = path;
        else filename += path;
        int len = strlen(path);
        if (memcmp(&path[len - 4], ".vlw", 4)) {
          filename += ".vlw";
        }
        result = this->_font_file.open(filename.c_str(), "r");
      }
      auto font = new VLWfont();
      this->_runtime_font.reset(font);
      if (result) {
        result = font->loadFont(&this->_font_file);
      }
      if (result) {
        this->_font = font;
        this->_font->getDefaultMetric(&this->_font_metrics);
      } else {
        this->unloadFont();
      }
      this->_font_file.postRead();
      return result;
    }

#if defined (ARDUINO)
 #if defined (FS_H) || defined (__SEEED_FS__)

    void loadFont(const char *path, fs::FS &fs)
    {
      this->_font_file.setFS(fs);
      loadFont(path);
    }

    inline bool drawBmp(fs::FS &fs, const char *path, std::int32_t x=0, std::int32_t y=0) { return drawBmpFile(fs, path, x, y); }
    inline bool drawBmpFile(fs::FS &fs, const char *path, std::int32_t x=0, std::int32_t y=0) {
      FileWrapper file(fs);
      return this->drawBmpFile(&file, path, x, y);
    }

    inline bool drawJpgFile(fs::FS &fs, const char *path, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div::jpeg_div_t scale=jpeg_div::jpeg_div_t::JPEG_DIV_NONE) {
      FileWrapper file(fs);
      return this->drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline bool drawPngFile(fs::FS &fs, const char *path, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, float scale = 1.0f)
    {
      FileWrapper file(fs);
      return this->drawPngFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }


    inline bool drawBmpFile(fs::FS &fs, fs::File *file, std::int32_t x=0, std::int32_t y=0) {
      FileWrapper data(fs, file);
      this->prepareTmpTransaction(&data);
      return this->draw_bmp(&data, x, y);
    }

    inline bool drawJpgFile(fs::FS &fs, fs::File *file, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div::jpeg_div_t scale=jpeg_div::jpeg_div_t::JPEG_DIV_NONE) {
      FileWrapper data(fs, file);
      this->prepareTmpTransaction(&data);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline bool drawPngFile(fs::FS &fs, fs::File *file, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, float scale = 1.0f)
    {
      FileWrapper data(fs, file);
      this->prepareTmpTransaction(&data);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }


    inline bool drawBmp(fs::File *dataSource, std::int32_t x=0, std::int32_t y=0) {
      StreamWrapper data;
      data.set(dataSource);
      data.need_transaction = true;
      this->prepareTmpTransaction(&data);
      return this->draw_bmp(&data, x, y);
    }

    inline bool drawJpg(fs::File *dataSource, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div::jpeg_div_t scale=jpeg_div::jpeg_div_t::JPEG_DIV_NONE) {
      StreamWrapper data;
      data.set(dataSource);
      data.need_transaction = true;
      this->prepareTmpTransaction(&data);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline bool drawPng(fs::File *dataSource, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, float scale = 1.0f) {
      StreamWrapper data;
      data.set(dataSource);
      data.need_transaction = true;
      this->prepareTmpTransaction(&data);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

 #endif
 #if defined (Stream_h)

    inline bool drawBmp(Stream *dataSource, std::int32_t x=0, std::int32_t y=0) {
      StreamWrapper data;
      data.set(dataSource);
      return this->draw_bmp(&data, x, y);
    }

    inline bool drawJpg(Stream *dataSource, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div::jpeg_div_t scale=jpeg_div::jpeg_div_t::JPEG_DIV_NONE) {
      StreamWrapper data;
      data.set(dataSource);
      return this->draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline bool drawPng(Stream *dataSource, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, float scale = 1.0f) {
      StreamWrapper data;
      data.set(dataSource);
      return this->draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

  #if defined (HTTPClient_H_)

    inline bool drawBmpUrl(const char* url, std::int32_t x=0, std::int32_t y=0)
    {
      if (WiFi.status() != WL_CONNECTED) {
        log_e("Not connected");
        return false;
      }

      HTTPClient http;
      http.begin(url);

      bool res = false;
      int httpCode = http.GET();
      if (httpCode != HTTP_CODE_OK)
      {
        log_e("HTTP ERROR: %d\n", httpCode);
      }
      else
      {
        res = drawBmp(http.getStreamPtr(), x, y);
      }
      http.end();
      return res;
    }

    inline bool drawJpgUrl(const char* url, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div::jpeg_div_t scale=jpeg_div::jpeg_div_t::JPEG_DIV_NONE)
    {
      if (WiFi.status() != WL_CONNECTED) {
        log_e("Not connected");
        return false;
      }

      HTTPClient http;
      http.begin(url);

      bool res = false;
      int httpCode = http.GET();
      if (httpCode != HTTP_CODE_OK)
      {
        log_e("HTTP ERROR: %d\n", httpCode);
      }
      else
      {
        res = drawJpg(http.getStreamPtr(), x, y, maxWidth, maxHeight, offX, offY, scale);
      }
      http.end();
      return res;
    }

    inline bool drawPngUrl(const char* url, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, float scale = 1.0f)
    {
      if (WiFi.status() != WL_CONNECTED) {
        log_e("Not connected");
        return false;
      }

      HTTPClient http;
      http.begin(url);

      bool res = false;
      int httpCode = http.GET();
      if (httpCode != HTTP_CODE_OK)
      {
        log_e("HTTP ERROR: %d\n", httpCode);
      }
      else
      {
        res = drawPng(http.getStreamPtr(), x, y, maxWidth, maxHeight, offX, offY, scale);
      }
      http.end();
      return res;
    }
    
  #endif
 #endif

#elif defined (CONFIG_IDF_TARGET_ESP32)  || defined(__SAMD51_HARMONY__) // ESP-IDF or Harmony

    inline bool drawBmpFile(const char *path, std::int32_t x, std::int32_t y) {
      FileWrapper file;
      return drawBmpFile(&file, path, x, y);
    }
    inline bool drawJpgFile(const char *path, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div::jpeg_div_t scale=jpeg_div::jpeg_div_t::JPEG_DIV_NONE) {
      FileWrapper file;
      return drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }
    inline bool drawPngFile(const char *path, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, float scale = 1.0f)
    {
      FileWrapper file;
      return drawPngFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

#endif

  private:

    bool drawBmpFile(FileWrapper* file, const char *path, std::int32_t x=0, std::int32_t y=0) {
      bool res = false;
      this->prepareTmpTransaction(file);
      file->preRead();
      if (file->open(path, "r")) {
        res = this->draw_bmp(file, x, y);
        file->close();
      }
      file->postRead();
      return res;
    }

    bool drawJpgFile(FileWrapper* file, const char *path, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, jpeg_div::jpeg_div_t scale) {
      bool res = false;
      this->prepareTmpTransaction(file);
      file->preRead();
      if (file->open(path, "r")) {
        res = this->draw_jpg(file, x, y, maxWidth, maxHeight, offX, offY, scale);
        file->close();
      }
      file->postRead();
      return res;
    }

    bool drawPngFile(FileWrapper* file, const char *path, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, float scale)
    {
      bool res = false;
      this->prepareTmpTransaction(file);
      file->preRead();
      if (file->open(path, "r")) {
        res = this->draw_png(file, x, y, maxWidth, maxHeight, offX, offY, scale);
        file->close();
      }
      file->postRead();
      return res;
    }
  };
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
}

#endif
