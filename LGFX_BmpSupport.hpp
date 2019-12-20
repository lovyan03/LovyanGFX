#ifndef LGFX_BMPSUPPORT_HPP_
#define LGFX_BMPSUPPORT_HPP_

#include "platforms/lgfx_common.hpp"

#if defined (ARDUINO)

#else

#endif

template <class Base>
struct LGFX_BmpSupport : public Base { // テンプレート継承
public:

  uint16_t read16(fs::File &f) {
    uint16_t result;
    f.read(reinterpret_cast<uint8_t*>(&result), 2);
    return result;
  }

  uint32_t read32(fs::File &f) {
    uint32_t result;
    f.read(reinterpret_cast<uint8_t*>(&result), 4);
    return result;
  }

  void skip(fs::File &f, uint16_t len) {
    f.seek(len, SeekCur);
  }

  inline void drawBmp(fs::FS &fs, const char *path, int16_t x, int16_t y) { drawBmpFile(fs, path, x, y); }
  void drawBmpFile(fs::FS &fs, const char *path, int16_t x, int16_t y) {
    if ((x >= this->_width) || (y >= this->_height)) return;

    this->_dev.endTransaction();
    // Open requested file on SD card
    File bmpFS = fs.open(path, "r");

    if (!bmpFS) {
      if (this->_start_write_count) { this->_dev.beginTransaction(); }
      return;
    }

    uint32_t startTime = millis();

    if (read16(bmpFS) == 0x4D42) {  // bmp header "BM"
      skip(bmpFS, 8);
      uint32_t seekOffset = read32(bmpFS);
      skip(bmpFS, 4);
      int16_t row, col;
      int16_t w = read32(bmpFS);
      int16_t h = read32(bmpFS);  // bcHeight Image height (pixels)
      //If the value of bcHeight is positive, the image data is from bottom to top
      //If the value of bcHeight is negative, the image data is from top to bottom.
      int16_t flow = (h > 0) ? -1 : 1;
      if (h < 0) h = -h;
      if (flow < 0) y += h - 1;

      if (read16(bmpFS) == 1) {  // bcPlanes always 1
        uint16_t bpp = read16(bmpFS); // 24 bcBitCount 24=RGB24bit
        uint32_t biComp = read32(bmpFS); // biCompression 0=BI_RGB
        if ((bpp == 24 || bpp == 16 || bpp == 32)
         && (biComp == 0 || biComp == 3)) {
          bmpFS.seek(seekOffset);
          this->setSwapBytes(true);
          uint16_t padding = (4 - (w & 3)) & 3;
          uint8_t lineBuffer[w * (bpp >> 3) + padding];
          while (h--) {
            this->_dev.endTransaction();
            bmpFS.read(lineBuffer, sizeof(lineBuffer));
            this->_dev.beginTransaction();
            if (bpp == 24) {      this->pushImage(x, y, w, 1, reinterpret_cast<lgfx::rgb888_t*>(lineBuffer));  }
            else if (bpp == 16) { this->pushImage(x, y, w, 1, reinterpret_cast<lgfx::rgb565_t*>(lineBuffer));  }
            else if (bpp == 32) { this->pushImage(x, y, w, 1, reinterpret_cast<lgfx::argb8888_t*>(lineBuffer)); }
            y += flow;
          }
        } else
        if ((bpp == 8 || bpp == 4 || bpp == 1) && biComp == 0) {
          skip(bmpFS, 12);
          uint32_t biClrUsed = read32(bmpFS);
          skip(bmpFS, 4);
          lgfx::argb8888_t palette[1<<bpp];
          bmpFS.read((uint8_t*)palette, (1<<bpp)*4); // load palette
          bmpFS.seek(seekOffset);
          if (bpp == 1) {
            uint8_t lineBuffer[((w+31) >> 5) << 2];
            while (h--) {
              this->_dev.endTransaction();
              bmpFS.read(lineBuffer, sizeof(lineBuffer));
              this->_dev.beginTransaction();
              uint8_t* src = lineBuffer;
              bool flg = lineBuffer[0] & 0x80;
              int32_t len = 1;
              for (int32_t i = 1; i < w; i++) {
                if (0 == (i & 7)) src++;
                if (flg != (bool)(*src & (0x80 >> (i&7)))) {
                  this->drawFastHLine(x + i - len, y, len, palette[flg].raw);
                  flg = !flg;
                  len = 0;
                }
                len++;
              }
              this->drawFastHLine(x + w - len, y, len, palette[flg].raw);
              y += flow;
            }
          } else {
            this->setSwapBytes(true);
            uint16_t readlen = (bpp == 4)
                             ? ((w+1)>>1) + ((4 - ((w+1)>>1) & 3) & 3)
                             : (w + (4 - (w & 3) & 3));
            uint8_t lineBuffer[w * 3];
            while (h--) {
              this->_dev.endTransaction();
              bmpFS.read(lineBuffer, readlen);
              this->_dev.beginTransaction();
              if (bpp == 8) {
                for (int16_t i = 1; i <= w; i++) {
                  reinterpret_cast<lgfx::rgb888_t*>(lineBuffer)[w - i] = palette[lineBuffer[w - i]];
                }
              } else {
                for (int16_t i = 1; i <= w; i++) {
                  reinterpret_cast<lgfx::rgb888_t*>(lineBuffer)[w - i] = palette[(lineBuffer[(w - i)>>1]>>((i&1)?0:4))&0x0F];
                }
              }
              this->pushImage(x, y, w, 1, reinterpret_cast<lgfx::rgb888_t*>(lineBuffer));
              y += flow;
            }
          }
        }
//        Serial.print("Loaded in "); Serial.print(millis() - startTime);   Serial.println(" ms");
      }
      else Serial.println("BMP format not recognized.");
    }
    bmpFS.close();
    if (this->_start_write_count) { this->_dev.beginTransaction(); }
  }

/*
  void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint16_t *data);
  void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint8_t *data);
  void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t *data);
  void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint8_t *data);
  void drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint16_t *data, uint16_t transparent);
*/
};

#endif
