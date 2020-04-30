#ifndef LGFX_GFXFONT_HPP_
#define LGFX_GFXFONT_HPP_

struct GFXglyph { // Data stored PER GLYPH
  uint32_t bitmapOffset;     // Pointer into GFXfont->bitmap
  uint8_t  width, height;    // Bitmap dimensions in pixels
  uint8_t  xAdvance;         // Distance to advance cursor (x axis)
  int8_t   xOffset, yOffset; // Dist from cursor pos to UL corner
};

struct EncodeRange {
  uint16_t start;
  uint16_t end;
  uint16_t base;
};

struct GFXfont : public lgfx::IFont { // Data stored for FONT AS A WHOLE:
  uint8_t  *bitmap;      // Glyph bitmaps, concatenated
  GFXglyph *glyph;       // Glyph array
  uint16_t  first, last; // ASCII extents
  uint8_t   yAdvance;    // Newline distance (y axis)

  uint16_t range_num;    // Number of EncodeRange
  EncodeRange *range;    // Array ofEncodeRange

  GFXfont ( uint8_t *bitmap = nullptr
          , GFXglyph *glyph = nullptr
          , uint16_t first  = 0
          , uint16_t last   = 0
          , uint8_t yAdvance = 0
          , uint16_t range_num = 0
          , EncodeRange *range = nullptr)
  : bitmap   (bitmap   )
  , glyph    (glyph    )
  , first    (first    )
  , last     (last     )
  , yAdvance (yAdvance )
  , range_num(range_num)
  , range    (range    )
  {}

  GFXglyph* getGlyph(uint16_t uniCode) const {
    if (uniCode > pgm_read_word(&last )
    ||  uniCode < pgm_read_word(&first)) return nullptr;
    uint16_t custom_range_num = pgm_read_word(&range_num);
    if (custom_range_num == 0) {
      uniCode -= pgm_read_word(&first);
      return &(((GFXglyph *)pgm_read_dword(&glyph))[uniCode]);
    }
    auto range_pst = (EncodeRange*)pgm_read_dword(&range);
    size_t i = 0;
    while ((uniCode > pgm_read_word(&range_pst[i].end)) 
        || (uniCode < pgm_read_word(&range_pst[i].start))) {
      if (++i == custom_range_num) return nullptr;
    }
    uniCode -= pgm_read_word(&range_pst[i].start) - pgm_read_word(&range_pst[i].base);
    return &(((GFXglyph *)pgm_read_dword(&glyph))[uniCode]);
  }

  lgfx::font_type_t getType(void) const override { return lgfx::font_type_t::ft_gfx; }

  void getDefaultMetric(lgfx::font_size_t *sizedata) const override {
    int_fast8_t glyph_ab = 0;   // glyph delta Y (height) above baseline
    int_fast8_t glyph_bb = 0;   // glyph delta Y (height) below baseline
    size_t numChars = pgm_read_word(&last) - pgm_read_word(&first);

    size_t custom_range_num = pgm_read_word(&range_num);
    if (custom_range_num != 0) {
      EncodeRange *range_pst = (EncodeRange *)pgm_read_dword(&range);
      size_t i = 0;
      numChars = custom_range_num;
      do {
        numChars += pgm_read_word(&range_pst[i].end) - pgm_read_word(&range_pst[i].start);
      } while (++i < custom_range_num);
    }

    // Find the biggest above and below baseline offsets
    for (size_t c = 0; c < numChars; c++)
    {
      GFXglyph *glyph1 = &(((GFXglyph *)pgm_read_dword(&glyph))[c]);
      int8_t ab = -pgm_read_byte(&glyph1->yOffset);
      if (ab > glyph_ab) glyph_ab = ab;
      int8_t bb = pgm_read_byte(&glyph1->height) - ab;
      if (bb > glyph_bb) glyph_bb = bb;
    }

    sizedata->baseline = glyph_ab;
    sizedata->y_offset = - glyph_ab;
    sizedata->y_size = glyph_bb + glyph_ab;
    sizedata->y_advance = (uint8_t)pgm_read_byte(&yAdvance);
  }

  bool updateFontMetric(lgfx::font_size_t *sizedata, uint16_t uniCode) const override {
    auto glyph = getGlyph(uniCode);
    if (!glyph) return false;
    sizedata->x_offset  = (int8_t)pgm_read_byte(&glyph->xOffset);
    sizedata->x_size    = pgm_read_byte(&glyph->width);
    sizedata->x_advance = pgm_read_byte(&glyph->xAdvance);
    return true;
  }
};


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


#endif
