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
#ifndef LGFX_IMG_SUPPORT_HPP_
#define LGFX_IMG_SUPPORT_HPP_

#include <cmath>

namespace lgfx
{
  template <class Base>
  class LGFX_IMG_Support : public Base {
  public:

#if defined (ARDUINO)
 #if defined (FS_H) || defined (__SEEED_FS__)

    inline void drawBmp(fs::FS &fs, const char *path, std::int32_t x=0, std::int32_t y=0) { drawBmpFile(fs, path, x, y); }
    inline void drawBmpFile(fs::FS &fs, const char *path, std::int32_t x=0, std::int32_t y=0) {
      FileWrapper file(fs);
      drawBmpFile(&file, path, x, y);
    }

    inline bool drawJpgFile(fs::FS &fs, const char *path, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      FileWrapper file(fs);
      return drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline bool drawPngFile(fs::FS &fs, const char *path, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, double scale = 1.0)
    {
      FileWrapper file(fs);
      return drawPngFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

 #endif
 #if defined (Stream_h)

    inline void drawBmp(Stream *dataSource, std::int32_t x=0, std::int32_t y=0) {
      StreamWrapper data;
      data.set(dataSource);
      draw_bmp(&data, x, y);
    }

    inline bool drawJpg(Stream *dataSource, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      StreamWrapper data;
      data.set(dataSource);
      return draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline void drawPng(Stream *dataSource, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, double scale = 1.0) {
      StreamWrapper data;
      data.set(dataSource);
      return draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

 #endif

#elif defined (CONFIG_IDF_TARGET_ESP32)  // ESP-IDF

    inline void drawBmpFile(const char *path, std::int32_t x, std::int32_t y) {
      FileWrapper file;
      drawBmpFile(&file, path, x, y);
    }
    inline bool drawJpgFile(const char *path, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      FileWrapper file;
      return drawJpgFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }
    inline bool drawPngFile(const char *path, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, double scale = 1.0)
    {
      FileWrapper file;
      return drawPngFile(&file, path, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

#endif

    void drawBmp(const std::uint8_t *bmp_data, std::uint32_t bmp_len, std::int32_t x=0, std::int32_t y=0) {
      PointerWrapper data;
      data.set(bmp_data, bmp_len);
      draw_bmp(&data, x, y);
    }
    bool drawJpg(const std::uint8_t *jpg_data, std::uint32_t jpg_len, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      PointerWrapper data;
      data.set(jpg_data, jpg_len);
      return draw_jpg(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }
    bool drawPng(const std::uint8_t *png_data, std::uint32_t png_len, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, double scale = 1.0)
    {
      PointerWrapper data;
      data.set(png_data, png_len);
      return draw_png(&data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

    inline void drawBmp(DataWrapper *data, std::int32_t x=0, std::int32_t y=0) {
      draw_bmp(data, x, y);
    }
    inline bool drawJpg(DataWrapper *data, std::int32_t x=0, std::int32_t y=0, std::int32_t maxWidth=0, std::int32_t maxHeight=0, std::int32_t offX=0, std::int32_t offY=0, jpeg_div_t scale=JPEG_DIV_NONE) {
      return draw_jpg(data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }
    inline void drawPng(DataWrapper *data, std::int32_t x = 0, std::int32_t y = 0, std::int32_t maxWidth = 0, std::int32_t maxHeight = 0, std::int32_t offX = 0, std::int32_t offY = 0, double scale = 1.0) {
      return draw_png(data, x, y, maxWidth, maxHeight, offX, offY, scale);
    }

  protected:

    struct bitmap_header_t {
      union {
        std::uint8_t raw[54];
        struct {
          std::uint16_t bfType; 
          std::uint32_t bfSize;
          std::uint16_t bfReserved1;
          std::uint16_t bfReserved2;
          std::uint32_t bfOffBits;

          std::uint32_t biSize; 
          std::int32_t  biWidth;
          std::int32_t  biHeight;
          std::uint16_t biPlanes; 
          std::uint16_t biBitCount;
          std::uint32_t biCompression;
          std::uint32_t biSizeImage; 
          std::int32_t  biXPelsPerMeter;
          std::int32_t  biYPelsPerMeter;
          std::uint32_t biClrUsed; 
          std::uint32_t biClrImportant;
        } __attribute__((packed));
      };
    };

    bool load_bmp_header(DataWrapper* data, bitmap_header_t* result) {
      data->read((std::uint8_t*)result, sizeof(bitmap_header_t));
      return ((result->bfType == 0x4D42)   // bmp header "BM"
           && (result->biPlanes == 1)  // bcPlanes always 1
           && (result->biWidth > 0)
           && (result->biHeight > 0)
           && (result->biBitCount <= 32)
           && (result->biBitCount != 0));
    }

    bool load_bmp_rle8(DataWrapper* data, std::uint8_t* linebuf, uint_fast16_t width) {
      width = (width + 3) & ~3;
      std::uint8_t code[2];
      uint_fast16_t xidx = 0;
      bool eol = false;
      do {
        data->read(code, 2);
        if (code[0] == 0) {
          switch (code[1]) {
          case 0x00: // EOL
          case 0x01: // EOB
            eol = true;
            break;

          case 0x02: // move info  (not support)
            return false;

          default:
            data->read(&linebuf[xidx], (code[1] + 1) & ~1); // word align
            xidx += code[1];
            break;
          }
        } else if (xidx + code[0] <= width) {
          memset(&linebuf[xidx], code[1], code[0]);
          xidx += code[0];
        } else {
          return false;
        }
      } while (!eol);
      return true;
    }

    bool load_bmp_rle4(DataWrapper* data, std::uint8_t* linebuf, uint_fast16_t width) {
      width = (width + 3) & ~3;
      std::uint8_t code[2];
      uint_fast16_t xidx = 0;
      bool eol = false;
      do {
        data->read(code, 2);
        if (code[0] == 0) {
          switch (code[1]) {
          case 0x00: // EOL
          case 0x01: // EOB
            eol = true;
            break;

          case 0x02: // move info  (not support)
            return false;

          default:  // 絶対モードデータ
            {
              int_fast16_t len = code[1];
              int_fast16_t dbyte = ((int_fast16_t)code[1] + 1) >> 1;

              data->read(&linebuf[(xidx + 1) >> 1], (dbyte + 1) & ~1); // word align
              if (xidx & 1) {
                linebuf[xidx >> 1] |= linebuf[(xidx >> 1) + 1] >> 4;
                for (long i = 1; i < dbyte; ++i) {
                  linebuf[((xidx + i) >> 1)] = (linebuf[((xidx + i) >> 1)    ] << 4)
                                             |  linebuf[((xidx + i) >> 1) + 1] >> 4;
                }
              }
              xidx += len;
            }
            break;
          }
        } else if (xidx + code[0] <= width) {
          if (xidx & 1) {
            linebuf[xidx >> 1] |= code[1] >> 4;
            code[1] = (code[1] >> 4) | (code[1] << 4);
          }
          memset(&linebuf[(xidx + 1) >> 1], code[1], (code[0] + 1) >> 1);
          xidx += code[0];
          if (xidx & 1) linebuf[xidx >> 1] &= 0xF0;
        } else {
          return false;
        }
      } while (!eol);
      return true;
    }

  private:

    void drawBmpFile(FileWrapper* file, const char *path, std::int32_t x=0, std::int32_t y=0) {
      this->prepareTmpTransaction(file);
      file->preRead();
      if (file->open(path, "r")) {
        draw_bmp(file, x, y);
        file->close();
      }
      file->postRead();
    }

    bool drawJpgFile(FileWrapper* file, const char *path, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, jpeg_div_t scale) {
      bool res = false;
      this->prepareTmpTransaction(file);
      file->preRead();
      if (file->open(path, "r")) {
        res = draw_jpg(file, x, y, maxWidth, maxHeight, offX, offY, scale);
        file->close();
      }
      file->postRead();
      return res;
    }

    bool drawPngFile( FileWrapper* file, const char *path, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, double scale)
    {
      bool res = false;
      this->prepareTmpTransaction(file);
      file->preRead();
      if (file->open(path, "r")) {
        res = draw_png(file, x, y, maxWidth, maxHeight, offX, offY, scale);
        file->close();
      }
      file->postRead();
      return res;
    }

    void draw_bmp(DataWrapper* data, std::int32_t x, std::int32_t y) {
      if ((x >= this->_width) || (y >= this->_height)) return;

      bitmap_header_t bmpdata;
      if (!load_bmp_header(data, &bmpdata)
       || (bmpdata.biCompression > 3)) {
        return;
      }

      //std::uint32_t startTime = millis();
      std::uint32_t seekOffset = bmpdata.bfOffBits;
      std::int32_t w = bmpdata.biWidth;
      std::int32_t h = bmpdata.biHeight;  // bcHeight Image height (pixels)
      uint_fast16_t bpp = bmpdata.biBitCount; // 24 bcBitCount 24=RGB24bit

        //If the value of Height is positive, the image data is from bottom to top
        //If the value of Height is negative, the image data is from top to bottom.
      std::int32_t flow = (h < 0) ? 1 : -1;
      if (h < 0) h = -h;
      else y += h - 1;

      argb8888_t *palette = nullptr;
      if (bpp <= 8) {
        palette = new argb8888_t[1 << bpp];
        data->seek(bmpdata.biSize + 14);
        data->read((std::uint8_t*)palette, (1 << bpp)*sizeof(argb8888_t)); // load palette
      }

      data->seek(seekOffset);

      auto dst_depth = this->_write_conv.depth;
      std::uint32_t buffersize = ((w * bpp + 31) >> 5) << 2;  // readline 4Byte align.
      std::uint8_t lineBuffer[buffersize + 4];
      pixelcopy_t p(lineBuffer, dst_depth, (color_depth_t)bpp, this->_palette_count, palette);
      p.no_convert = false;
      if (8 >= bpp && !this->_palette_count) {
        p.fp_copy = pixelcopy_t::get_fp_palettecopy<argb8888_t>(dst_depth);
      } else {
        if (bpp == 16) {
          p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb565_t>(dst_depth);
        } else if (bpp == 24) {
          p.fp_copy = pixelcopy_t::get_fp_normalcopy<rgb888_t>(dst_depth);
        } else if (bpp == 32) {
          p.fp_copy = pixelcopy_t::get_fp_normalcopy<argb8888_t>(dst_depth);
        }
      }

      this->startWrite(false);
      if (bmpdata.biCompression == 1) {
        do {
          data->preRead();
          load_bmp_rle8(data, lineBuffer, w);
          data->postRead();
          this->push_image(x, y, w, 1, &p);
          y += flow;
        } while (--h);
      } else
      if (bmpdata.biCompression == 2) {
        do {
          data->preRead();
          load_bmp_rle4(data, lineBuffer, w);
          data->postRead();
          this->push_image(x, y, w, 1, &p);
          y += flow;
        } while (--h);
      } else {
        do {
          data->preRead();
          data->read(lineBuffer, buffersize);
          data->postRead();
          this->push_image(x, y, w, 1, &p);
          y += flow;
        } while (--h);
      }
      if (palette) delete[] palette;
      this->endWrite();
      //Serial.print("Loaded in "); Serial.print(millis() - startTime);   Serial.println(" ms");
    }


protected:

#if !defined (__LGFX_TJPGDEC_H__)

    bool draw_jpg(DataWrapper* data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, jpeg_div_t scale)
    {
//ESP_LOGI("LGFX","drawJpg need include utility/tjpgd.h");
      return false;
    }

#else

    bool draw_jpg(DataWrapper* data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, jpeg_div_t scale)
    {
      draw_jpg_info_t jpeg;
      pixelcopy_t pc(nullptr, this->getColorDepth(), bgr888_t::depth, this->hasPalette());
      jpeg.pc = &pc;
      jpeg.lgfx = this;
      jpeg.data = data;
      jpeg.x = x - offX;
      jpeg.y = y - offY;

      //TJpgD jpegdec;
      lgfxJdec jpegdec;

      static constexpr std::uint16_t sz_pool = 3100;
      std::uint8_t *pool = (std::uint8_t*)heap_alloc_dma(sz_pool);
      if (!pool) {
//        ESP_LOGE("LGFX","memory allocation failure");
        return false;
      }

      auto jres = lgfx_jd_prepare(&jpegdec, jpg_read_data, pool, sz_pool, &jpeg);

      if (jres != JDR_OK) {
//ESP_LOGE("LGFX","jpeg prepare error:%x", jres);
        heap_free(pool);
        return false;
      }

      if (!maxWidth) maxWidth = this->width();
      auto cl = this->_clip_l;
      if (0 > x - cl) { maxWidth += x - cl; x = cl; }
      auto cr = this->_clip_r + 1;
      if (maxWidth > (cr - x)) maxWidth = (cr - x);

      if (!maxHeight) maxHeight = this->height();
      auto ct = this->_clip_t;
      if (0 > y - ct) { maxHeight += y - ct; y = ct; }
      auto cb = this->_clip_b + 1;
      if (maxHeight > (cb - y)) maxHeight = (cb - y);

      if (maxWidth > 0 && maxHeight > 0) {
        this->setClipRect(x, y, maxWidth, maxHeight);
        this->startWrite();
        jres = lgfx_jd_decomp(&jpegdec, jpg_push_image, scale);

        this->_clip_l = cl;
        this->_clip_t = ct;
        this->_clip_r = cr-1;
        this->_clip_b = cb-1;
        this->endWrite();
      }
      heap_free(pool);

      if (jres != JDR_OK) {
//ESP_LOGE("LGFX","jpeg decomp error:%x", jres);
        return false;
      }
      return true;
    }

    struct draw_jpg_info_t {
      std::int32_t x;
      std::int32_t y;
      DataWrapper *data;
      LGFX_IMG_Support *lgfx;
      pixelcopy_t *pc;
    };

    static std::uint32_t jpg_read_data(lgfxJdec  *decoder, std::uint8_t *buf, std::uint32_t len) {
      auto jpeg = (draw_jpg_info_t *)decoder->device;
      auto data = (DataWrapper*)jpeg->data;
      auto res = len;
      data->preRead();
      if (buf) {
        res = data->read(buf, len);
      } else {
        data->skip(len);
      }
      return res;
    }

    static std::uint32_t jpg_push_image(lgfxJdec *decoder, void *bitmap, JRECT *rect) {
      draw_jpg_info_t *jpeg = (draw_jpg_info_t *)decoder->device;
      jpeg->pc->src_data = bitmap;
      auto data = (DataWrapper*)jpeg->data;
      data->postRead();
      jpeg->lgfx->push_image( jpeg->x + rect->left
                           , jpeg->y + rect->top 
                           , rect->right  - rect->left + 1
                           , rect->bottom - rect->top + 1
                           , jpeg->pc
                           , false);
      return 1;
    }

#endif


#ifndef __LGFX_PNGLE_H__

    bool draw_png(DataWrapper* data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, double scale)
    {
//ESP_LOGI("LGFX","drawPng need include utility/pngle.h");
      return false;
    }

#else

    struct png_file_decoder_t {
      std::int32_t x;
      std::int32_t y;
      std::int32_t offX;
      std::int32_t offY;
      std::int32_t maxWidth;
      std::int32_t maxHeight;
      double scale;
      bgr888_t* lineBuffer;
      pixelcopy_t *pc;
      LGFX_IMG_Support *lgfx;
      std::uint32_t last_y;
      std::int32_t scale_y0;
      std::int32_t scale_y1;
    };

    static bool png_ypos_update(png_file_decoder_t *p, std::uint32_t y)
    {
      p->scale_y0 = ceil( y      * p->scale) - p->offY;
      if (p->scale_y0 < 0) p->scale_y0 = 0;
      p->scale_y1 = ceil((y + 1) * p->scale) - p->offY;
      if (p->scale_y1 > p->maxHeight) p->scale_y1 = p->maxHeight;
      return (p->scale_y0 < p->scale_y1);
    }

    static void png_post_line(png_file_decoder_t *p)
    {
      std::int32_t h = p->scale_y1 - p->scale_y0;
      if (0 < h)
        p->lgfx->push_image(p->x, p->y + p->scale_y0, p->maxWidth, h, p->pc, true);
    }

    static void png_prepare_line(png_file_decoder_t *p, std::uint32_t y)
    {
      p->last_y = y;
      if (png_ypos_update(p, y))      // read next line
        p->lgfx->readRectRGB(p->x, p->y + p->scale_y0, p->maxWidth, p->scale_y1 - p->scale_y0, p->lineBuffer);
    }

    static void png_done_callback(pngle_t *pngle)
    {
      auto p = (png_file_decoder_t *)lgfx_pngle_get_user_data(pngle);
      png_post_line(p);
    }

    static void png_draw_normal_callback(pngle_t *pngle, std::uint32_t x, std::uint32_t y, std::uint8_t rgba[4])
    {
      auto p = (png_file_decoder_t*)lgfx_pngle_get_user_data(pngle);

      std::int32_t t = y - p->offY;
      if (t < 0 || t >= p->maxHeight) return;

      std::int32_t l = x - p->offX;
      if (l < 0 || l >= p->maxWidth) return;

      p->lgfx->setColor(color888(rgba[0], rgba[1], rgba[2]));
      p->lgfx->writeFillRectPreclipped(p->x + l, p->y + t, 1, 1);
    }

    static void png_draw_normal_scale_callback(pngle_t *pngle, std::uint32_t x, std::uint32_t y, std::uint8_t rgba[4])
    {
      auto p = (png_file_decoder_t*)lgfx_pngle_get_user_data(pngle);

      if (y != p->last_y) {
        p->last_y = y;
        png_ypos_update(p, y);
      }

      std::int32_t t = p->scale_y0;
      std::int32_t h = p->scale_y1 - t;
      if (h <= 0) return;

      std::int32_t l = ceil( x      * p->scale) - p->offX;
      if (l < 0) l = 0;
      std::int32_t r = ceil((x + 1) * p->scale) - p->offX;
      if (r > p->maxWidth) r = p->maxWidth;
      if (l >= r) return;

      p->lgfx->setColor(color888(rgba[0], rgba[1], rgba[2]));
      p->lgfx->writeFillRectPreclipped(p->x + l, p->y + t, r - l, h);
    }

    static void png_draw_alpha_callback(pngle_t *pngle, std::uint32_t x, std::uint32_t y, std::uint8_t rgba[4])
    {
      auto p = (png_file_decoder_t*)lgfx_pngle_get_user_data(pngle);
      if (y != p->last_y) {
        png_post_line(p);
        png_prepare_line(p, y);
      }

      if (p->scale_y0 >= p->scale_y1) return;

      std::int32_t l = ( x      ) - p->offX;
      if (l < 0) l = 0;
      std::int32_t r = ((x + 1) ) - p->offX;
      if (r > p->maxWidth) r = p->maxWidth;
      if (l >= r) return;

      if (rgba[3] == 255) {
        memcpy(&p->lineBuffer[l], rgba, 3);
      } else {
        auto data = &p->lineBuffer[l];
        uint_fast8_t alpha = rgba[3] + 1;
        data->r = (rgba[0] * alpha + data->r * (257 - alpha)) >> 8;
        data->g = (rgba[1] * alpha + data->g * (257 - alpha)) >> 8;
        data->b = (rgba[2] * alpha + data->b * (257 - alpha)) >> 8;
      }
    }

    static void png_draw_alpha_scale_callback(pngle_t *pngle, std::uint32_t x, std::uint32_t y, std::uint8_t rgba[4])
    {
      auto p = (png_file_decoder_t*)lgfx_pngle_get_user_data(pngle);
      if (y != p->last_y) {
        png_post_line(p);
        png_prepare_line(p, y);
      }

      std::int32_t b = p->scale_y1 - p->scale_y0;
      if (b <= 0) return;

      std::int32_t l = ceil( x      * p->scale) - p->offX;
      if (l < 0) l = 0;
      std::int32_t r = ceil((x + 1) * p->scale) - p->offX;
      if (r > p->maxWidth) r = p->maxWidth;
      if (l >= r) return;

      if (rgba[3] == 255) {
        std::int32_t i = l;
        do {
          for (std::int32_t j = 0; j < b; ++j) {
            auto data = &p->lineBuffer[i + j * p->maxWidth];
            memcpy(data, rgba, 3);
          }
        } while (++i < r);
      } else {
        uint_fast8_t alpha = rgba[3] + 1;
        std::int32_t i = l;
        do {
          for (std::int32_t j = 0; j < b; ++j) {
            auto data = &p->lineBuffer[i + j * p->maxWidth];
            data->r = (rgba[0] * alpha + data->r * (257 - alpha)) >> 8;
            data->g = (rgba[1] * alpha + data->g * (257 - alpha)) >> 8;
            data->b = (rgba[2] * alpha + data->b * (257 - alpha)) >> 8;
          }
        } while (++i < r);
      }
    }

    static void png_init_callback(pngle_t *pngle, std::uint32_t w, std::uint32_t h, uint_fast8_t hasTransparent)
    {
//    auto ihdr = lgfx_pngle_get_ihdr(pngle);

      auto p = (png_file_decoder_t*)lgfx_pngle_get_user_data(pngle);

      if (p->scale != 1.0) {
        w = ceil(w * p->scale);
        h = ceil(h * p->scale);
      }

      std::int32_t ww = w - abs(p->offX);
      if (p->maxWidth > ww) p->maxWidth = ww;
      if (p->maxWidth < 0) return;
      if (p->offX < 0) { p->offX = 0; }

      std::int32_t hh = h - abs(p->offY);
      if (p->maxHeight > hh) p->maxHeight = hh;
      if (p->maxHeight < 0) return;
      if (p->offY < 0) { p->offY = 0; }

      if (hasTransparent) { // need pixel read ?
        p->lineBuffer = (bgr888_t*)heap_alloc_dma(sizeof(bgr888_t) * p->maxWidth * ceil(p->scale));
        p->pc->src_data = p->lineBuffer;
        png_prepare_line(p, 0);
        lgfx_pngle_set_done_callback(pngle, png_done_callback);

        if (p->scale == 1.0) {
          lgfx_pngle_set_draw_callback(pngle, png_draw_alpha_callback);
        } else {
          lgfx_pngle_set_draw_callback(pngle, png_draw_alpha_scale_callback);
        }
      } else {
        if (p->scale == 1.0) {
          lgfx_pngle_set_draw_callback(pngle, png_draw_normal_callback);
        } else {
          p->last_y = 0;
          png_ypos_update(p, 0);
          lgfx_pngle_set_draw_callback(pngle, png_draw_normal_scale_callback);
        }
        return;
      }
    }

    bool draw_png(DataWrapper* data, std::int32_t x, std::int32_t y, std::int32_t maxWidth, std::int32_t maxHeight, std::int32_t offX, std::int32_t offY, double scale)
    {
      if (!maxHeight) maxHeight = INT32_MAX;
      auto ct = this->_clip_t;
      if (0 > y - ct) { maxHeight += y - ct; offY -= y - ct; y = ct; }
      if (0 > offY) { y -= offY; maxHeight += offY; offY = 0; }
      auto cb = this->_clip_b + 1;
      if (maxHeight > (cb - y)) maxHeight = (cb - y);
      if (maxHeight < 0) return true;

      if (!maxWidth) maxWidth = INT32_MAX;
      auto cl = this->_clip_l;
      if (0 > x - cl) { maxWidth += x - cl; offX -= x - cl; x = cl; }
      if (0 > offX) { x -= offX; maxWidth  += offX; offX = 0; }
      auto cr = this->_clip_r + 1;
      if (maxWidth > (cr - x)) maxWidth = (cr - x);
      if (maxWidth < 0) return true;

      png_file_decoder_t png;
      png.x = x;
      png.y = y;
      png.offX = offX;
      png.offY = offY;
      png.maxWidth = maxWidth;
      png.maxHeight = maxHeight;
      png.scale = scale;
      png.lgfx = this;
      png.lineBuffer = nullptr;

      pixelcopy_t pc(nullptr, this->getColorDepth(), bgr888_t::depth, this->_palette_count);
      png.pc = &pc;

      pngle_t *pngle = lgfx_pngle_new();

      lgfx_pngle_set_user_data(pngle, &png);

      lgfx_pngle_set_init_callback(pngle, png_init_callback);

      // Feed data to pngle
      std::uint8_t buf[512];
      int remain = 0;
      int len;
      bool res = true;

      this->startWrite(false);
      while (0 < (len = data->read(buf + remain, sizeof(buf) - remain))) {
        data->postRead();

        int fed = lgfx_pngle_feed(pngle, buf, remain + len);

        if (fed < 0) {
//ESP_LOGE("LGFX", "[pngle error] %s", lgfx_pngle_error(pngle));
          res = false;
          break;
        }

        remain = remain + len - fed;
        if (remain > 0) memmove(buf, buf + fed, remain);
        data->preRead();
      }
      this->endWrite();
      if (png.lineBuffer) {
        this->waitDMA();
        heap_free(png.lineBuffer);
      }
      lgfx_pngle_destroy(pngle);
      return res;
    }

#endif

  };
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
}

#endif
