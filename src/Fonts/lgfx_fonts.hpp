#include <stdint.h>

namespace lgfx
{
  struct LGFXBase;

  enum font_type_t
  { ft_unknown
  , ft_glcd
  , ft_bmp
  , ft_rle
  , ft_gfx
  , ft_bdf
  , ft_vlw
  };

  struct font_size_t {
    int16_t x_size;
    int16_t x_advance;
    int16_t x_offset;
    int16_t y_size;
    int16_t y_advance;
    int16_t y_offset;
    int16_t baseline;
  };

  struct IFont
  {
    virtual font_type_t getType(void) const { return ft_unknown; }
    virtual void getDefaultMetric(font_size_t *sizedata) const {}
    virtual bool updateFontMetric(font_size_t *sizedata, uint16_t uniCode) const { return false; }
    virtual bool unloadFont(void) { return false; }
    virtual ~IFont() {}
  };

  struct BaseFont : public IFont {
    const uint8_t *chartbl;
    const uint8_t *widthtbl;
    const uint8_t width;
    const uint8_t height;
    const uint8_t baseline;
    BaseFont() = default;
    BaseFont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline)
     : chartbl  (chartbl  )
     , widthtbl (widthtbl )
     , width    (width    )
     , height   (height   )
     , baseline (baseline )
    {}
    void getDefaultMetric(font_size_t *sizedata) const override {
      sizedata->x_size    = width;
      sizedata->x_advance = width;
      sizedata->x_offset  = 0;
      sizedata->y_size    = height;
      sizedata->y_advance = height;
      sizedata->y_offset  = 0;
      sizedata->baseline  = baseline;
    }
  };

  struct GLCDfont : public BaseFont {
    GLCDfont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_glcd; }

    bool updateFontMetric(font_size_t *sizedata, uint16_t uniCode) const override {
      return uniCode < 256;
    }
  };

  struct BMPfont : public BaseFont {
    BMPfont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline) : BaseFont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_bmp;  } 

    bool updateFontMetric(font_size_t *sizedata, uint16_t uniCode) const override {
      if ((uniCode -= 32) >= 96) return false;
      sizedata->x_advance = sizedata->x_size = pgm_read_byte( (uint8_t *)pgm_read_dword( &widthtbl ) + uniCode );
      return true;
    }
  };

  struct RLEfont : public BMPfont {
    RLEfont(const uint8_t *chartbl, const uint8_t *widthtbl, uint8_t width, uint8_t height, uint8_t baseline) : BMPfont(chartbl, widthtbl, width, height, baseline ) {}
    font_type_t getType(void) const override { return ft_rle;  }
  };

  struct BDFfont : public BaseFont {
    const uint16_t *indextbl;
    uint16_t indexsize;
    uint8_t halfwidth;
    BDFfont() = default;
    BDFfont(const uint8_t *chartbl, const uint16_t *indextbl, uint16_t indexsize, uint8_t width, uint8_t halfwidth, uint8_t height, uint8_t baseline) 
     : BaseFont(chartbl, nullptr, width, height, baseline )
     , indextbl(indextbl)
     , indexsize(indexsize)
     , halfwidth(halfwidth)
     {}
    font_type_t getType(void) const override { return ft_bdf;  }

    bool updateFontMetric(font_size_t *sizedata, uint16_t uniCode) const override {
      sizedata->x_advance = sizedata->x_size = pgm_read_byte( (uniCode < 0x0100) ? &halfwidth : &width );
      return true;
    }
  };
}

#if !defined LOAD_GFXFF
#include "Fonts/gfxfont.hpp"
#endif


#include "Fonts/glcdfont.h"

#include "Fonts/Font16.h"
#include "Fonts/Font32rle.h"
#include "Fonts/Font64rle.h"
#include "Fonts/Font7srle.h"
#include "Fonts/Font72rle.h"

namespace fonts {
  static PROGMEM const lgfx::GLCDfont font0 = { (const uint8_t *)font, nullptr, 6, 8, 6};
  static PROGMEM const lgfx::BMPfont  font2 = { (const uint8_t *)chrtbl_f16, widtbl_f16, 0, chr_hgt_f16, baseline_f16 };
  static PROGMEM const lgfx::RLEfont  font4 = { (const uint8_t *)chrtbl_f32, widtbl_f32, 0, chr_hgt_f32, baseline_f32 };
  static PROGMEM const lgfx::RLEfont  font6 = { (const uint8_t *)chrtbl_f64, widtbl_f64, 0, chr_hgt_f64, baseline_f64 };
  static PROGMEM const lgfx::RLEfont  font7 = { (const uint8_t *)chrtbl_f7s, widtbl_f7s, 0, chr_hgt_f7s, baseline_f7s };
  static PROGMEM const lgfx::RLEfont  font8 = { (const uint8_t *)chrtbl_f72, widtbl_f72, 0, chr_hgt_f72, baseline_f72 };

#ifdef __EFONT_FONT_DATA_H__
  static PROGMEM const lgfx::BDFfont efont = { (const uint8_t *)efontFontData, efontFontList, sizeof(efontFontList)>>1, 16, 8, 16, 14 };
#endif
}


