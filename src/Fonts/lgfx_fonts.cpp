#include "lgfx_fonts.hpp"

#include <cstdint>
#include <cstddef>

#ifndef PROGMEM
#define PROGMEM
#endif

namespace lgfx {
  void BaseFont::getDefaultMetric(FontMetrics *metrics) const
  {
    metrics->width    = width;
    metrics->x_advance = width;
    metrics->x_offset  = 0;
    metrics->height    = height;
    metrics->y_advance = height;
    metrics->y_offset  = 0;
    metrics->baseline  = baseline;
  }
  void BDFfont::getDefaultMetric(FontMetrics *metrics) const
  {
    BaseFont::getDefaultMetric(metrics);
    metrics->y_advance = y_advance;
  }

  bool GLCDfont::updateFontMetric(FontMetrics*, std::uint16_t uniCode) const {
    return uniCode < 256;
  }

  bool BMPfont::updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const {
    if ((uniCode -= 32) >= 96) return false;
    metrics->x_advance = metrics->width = widthtbl[uniCode];
    return true;
  }

  bool BDFfont::updateFontMetric(FontMetrics *metrics, std::uint16_t uniCode) const {
    metrics->x_advance = metrics->width = (uniCode < 0x0100) ? halfwidth : width;
    return true;
  }
}

//----------------------------------------------------------------------------

bool GFXfont::updateFontMetric(lgfx::FontMetrics *metrics, std::uint16_t uniCode) const {
  auto glyph = getGlyph(uniCode);
  if (!glyph) return false;
  metrics->x_offset  = glyph->xOffset;
  metrics->width     = glyph->width;
  metrics->x_advance = glyph->xAdvance;
  return true;
}

GFXglyph* GFXfont::getGlyph(std::uint16_t uniCode) const {
  if (uniCode > last 
  ||  uniCode < first) return nullptr;
  std::uint16_t custom_range_num = range_num;
  if (custom_range_num == 0) {
    uniCode -= first;
    return &glyph[uniCode];
  }
  auto range_pst = range;
  size_t i = 0;
  while ((uniCode > range_pst[i].end) 
      || (uniCode < range_pst[i].start)) {
    if (++i == custom_range_num) return nullptr;
  }
  uniCode -= range_pst[i].start - range_pst[i].base;
  return &glyph[uniCode];
}

void GFXfont::getDefaultMetric(lgfx::FontMetrics *metrics) const {
  std::int_fast8_t glyph_ab = 0;   // glyph delta Y (height) above baseline
  std::int_fast8_t glyph_bb = 0;   // glyph delta Y (height) below baseline
  size_t numChars = last - first;

  size_t custom_range_num = range_num;
  if (custom_range_num != 0) {
    EncodeRange *range_pst = range;
    size_t i = 0;
    numChars = custom_range_num;
    do {
      numChars += range_pst[i].end - range_pst[i].start;
    } while (++i < custom_range_num);
  }

  // Find the biggest above and below baseline offsets
  for (size_t c = 0; c < numChars; c++)
  {
    GFXglyph *glyph1 = &glyph[c];
    std::int_fast8_t ab = -glyph1->yOffset;
    if (ab > glyph_ab) glyph_ab = ab;
    std::int_fast8_t bb = glyph1->height - ab;
    if (bb > glyph_bb) glyph_bb = bb;
  }

  metrics->baseline = glyph_ab;
  metrics->y_offset = - glyph_ab;
  metrics->height   = glyph_bb + glyph_ab;
  metrics->y_advance = yAdvance;
}

//----------------------------------------------------------------------------

namespace fonts {
  using namespace lgfx;

  // Original Adafruit_GFX "Free Fonts"
  #include "GFXFF/TomThumb.h"  // TT1

  #include "GFXFF/FreeMono9pt7b.h"  // FF1 or FM9
  #include "GFXFF/FreeMono12pt7b.h" // FF2 or FM12
  #include "GFXFF/FreeMono18pt7b.h" // FF3 or FM18
  #include "GFXFF/FreeMono24pt7b.h" // FF4 or FM24

  #include "GFXFF/FreeMonoOblique9pt7b.h"  // FF5 or FMO9
  #include "GFXFF/FreeMonoOblique12pt7b.h" // FF6 or FMO12
  #include "GFXFF/FreeMonoOblique18pt7b.h" // FF7 or FMO18
  #include "GFXFF/FreeMonoOblique24pt7b.h" // FF8 or FMO24

  #include "GFXFF/FreeMonoBold9pt7b.h"  // FF9  or FMB9
  #include "GFXFF/FreeMonoBold12pt7b.h" // FF10 or FMB12
  #include "GFXFF/FreeMonoBold18pt7b.h" // FF11 or FMB18
  #include "GFXFF/FreeMonoBold24pt7b.h" // FF12 or FMB24

  #include "GFXFF/FreeMonoBoldOblique9pt7b.h"  // FF13 or FMBO9
  #include "GFXFF/FreeMonoBoldOblique12pt7b.h" // FF14 or FMBO12
  #include "GFXFF/FreeMonoBoldOblique18pt7b.h" // FF15 or FMBO18
  #include "GFXFF/FreeMonoBoldOblique24pt7b.h" // FF16 or FMBO24

  // Sans serif fonts
  #include "GFXFF/FreeSans9pt7b.h"  // FF17 or FSS9
  #include "GFXFF/FreeSans12pt7b.h" // FF18 or FSS12
  #include "GFXFF/FreeSans18pt7b.h" // FF19 or FSS18
  #include "GFXFF/FreeSans24pt7b.h" // FF20 or FSS24

  #include "GFXFF/FreeSansOblique9pt7b.h"  // FF21 or FSSO9
  #include "GFXFF/FreeSansOblique12pt7b.h" // FF22 or FSSO12
  #include "GFXFF/FreeSansOblique18pt7b.h" // FF23 or FSSO18
  #include "GFXFF/FreeSansOblique24pt7b.h" // FF24 or FSSO24

  #include "GFXFF/FreeSansBold9pt7b.h"  // FF25 or FSSB9
  #include "GFXFF/FreeSansBold12pt7b.h" // FF26 or FSSB12
  #include "GFXFF/FreeSansBold18pt7b.h" // FF27 or FSSB18
  #include "GFXFF/FreeSansBold24pt7b.h" // FF28 or FSSB24

  #include "GFXFF/FreeSansBoldOblique9pt7b.h"  // FF29 or FSSBO9
  #include "GFXFF/FreeSansBoldOblique12pt7b.h" // FF30 or FSSBO12
  #include "GFXFF/FreeSansBoldOblique18pt7b.h" // FF31 or FSSBO18
  #include "GFXFF/FreeSansBoldOblique24pt7b.h" // FF32 or FSSBO24

  // Serif fonts
  #include "GFXFF/FreeSerif9pt7b.h"  // FF33 or FS9
  #include "GFXFF/FreeSerif12pt7b.h" // FF34 or FS12
  #include "GFXFF/FreeSerif18pt7b.h" // FF35 or FS18
  #include "GFXFF/FreeSerif24pt7b.h" // FF36 or FS24

  #include "GFXFF/FreeSerifItalic9pt7b.h"  // FF37 or FSI9
  #include "GFXFF/FreeSerifItalic12pt7b.h" // FF38 or FSI12
  #include "GFXFF/FreeSerifItalic18pt7b.h" // FF39 or FSI18
  #include "GFXFF/FreeSerifItalic24pt7b.h" // FF40 or FSI24

  #include "GFXFF/FreeSerifBold9pt7b.h"  // FF41 or FSB9
  #include "GFXFF/FreeSerifBold12pt7b.h" // FF42 or FSB12
  #include "GFXFF/FreeSerifBold18pt7b.h" // FF43 or FSB18
  #include "GFXFF/FreeSerifBold24pt7b.h" // FF44 or FSB24

  #include "GFXFF/FreeSerifBoldItalic9pt7b.h"  // FF45 or FSBI9
  #include "GFXFF/FreeSerifBoldItalic12pt7b.h" // FF46 or FSBI12
  #include "GFXFF/FreeSerifBoldItalic18pt7b.h" // FF47 or FSBI18
  #include "GFXFF/FreeSerifBoldItalic24pt7b.h" // FF48 or FSBI24

  // Custom fonts
  #include "Custom/Orbitron_Light_24.h"
  #include "Custom/Orbitron_Light_32.h"
  #include "Custom/Roboto_Thin_24.h"
  #include "Custom/Satisfy_24.h"
  #include "Custom/Yellowtail_32.h"


  #include "glcdfont.h"
  #include "Font16.h"
  #include "Font32rle.h"
  #include "Font64rle.h"
  #include "Font7srle.h"
  #include "Font72rle.h"

  const GLCDfont Font0 = { (const uint8_t *)font, nullptr, 6, 8, 7};
  const BMPfont  Font2 = { (const uint8_t *)chrtbl_f16, widtbl_f16, 0, chr_hgt_f16, baseline_f16 };
  const RLEfont  Font4 = { (const uint8_t *)chrtbl_f32, widtbl_f32, 0, chr_hgt_f32, baseline_f32 };
  const RLEfont  Font6 = { (const uint8_t *)chrtbl_f64, widtbl_f64, 0, chr_hgt_f64, baseline_f64 };
  const RLEfont  Font7 = { (const uint8_t *)chrtbl_f7s, widtbl_f7s, 0, chr_hgt_f7s, baseline_f7s };
  const RLEfont  Font8 = { (const uint8_t *)chrtbl_f72, widtbl_f72, 0, chr_hgt_f72, baseline_f72 };
}

//----------------------------------------------------------------------------

namespace lgfx
{
  // deprecated array.
  const IFont* fontdata [] = {
    &fonts::Font0,  // GLCD font (Font 0)
    &fonts::Font0,  // Font 1 current unused
    &fonts::Font2,
    &fonts::Font0,  // Font 3 current unused
    &fonts::Font4,
    &fonts::Font0,  // Font 5 current unused
    &fonts::Font6,
    &fonts::Font7,
    &fonts::Font8,
  };
}

