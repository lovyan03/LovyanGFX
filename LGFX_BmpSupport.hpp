#ifndef LGFX_BMPSUPPORT_HPP_
#define LGFX_BMPSUPPORT_HPP_

template <class Base>
struct LGFX_BmpSupport : public Base {
public:

  uint16_t read16(fs::File &f) {
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read(); // MSB
    return result;
  }

  uint32_t read32(fs::File &f) {
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read(); // MSB
    return result;
  }

  inline void drawBmp(fs::FS &fs, const char *path, int16_t x, int16_t y) { drawBmpFile(fs, path, x, y); }
  void drawBmpFile(fs::FS &fs, const char *path, int16_t x, int16_t y) {
    if ((x >= this->_width) || (y >= this->_height)) return;

    this->_dev.endTransaction();
    // Open requested file on SD card
    File bmpFS = fs.open(path, "r");

    if (!bmpFS) {
      Serial.print("File not found");
      if (this->_start_write_count) { this->_dev.beginTransaction(); }
      return;
    }

    uint32_t seekOffset;
    uint16_t w, h, row, col;
    uint8_t  r, g, b;

    uint32_t startTime = millis();

    if (read16(bmpFS) == 0x4D42) {
      read32(bmpFS);
      read32(bmpFS);
      seekOffset = read32(bmpFS);
      read32(bmpFS);
      w = read32(bmpFS);
      h = read32(bmpFS);

      if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0)) {
        y += h - 1;

        this->setSwapBytes(true);
        bmpFS.seek(seekOffset);

        uint16_t padding = (4 - ((w * 3) & 3)) & 3;
        uint8_t lineBuffer[w * 3 + padding];

        for (row = 0; row < h; row++) {
          this->_dev.endTransaction();
          bmpFS.read(lineBuffer, sizeof(lineBuffer));
          uint8_t*  bptr = lineBuffer;
          uint16_t* tptr = (uint16_t*)lineBuffer;
          // Convert 24 to 16 bit colours
          for (col = 0; col < w; col++) {
            b = *bptr++;
            g = *bptr++;
            r = *bptr++;
            *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
          }

          // Push the pixel row to screen, pushImage will crop the line if needed
          // y is decremented as the BMP image is drawn bottom up
          this->_dev.beginTransaction();
          this->pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
        }
        Serial.print("Loaded in "); Serial.print(millis() - startTime);
        Serial.println(" ms");
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
