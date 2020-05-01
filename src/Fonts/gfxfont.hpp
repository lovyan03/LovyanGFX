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

#include "Custom/Orbitron_Light_24.h"
#include "Custom/Orbitron_Light_32.h"
#include "Custom/Roboto_Thin_24.h"
#include "Custom/Satisfy_24.h"
#include "Custom/Yellowtail_32.h"

namespace fonts {

  static PROGMEM const GFXfont& TomThumb                  = TomThumb                 ;
  static PROGMEM const GFXfont& FreeMono9pt7b             = FreeMono9pt7b            ;
  static PROGMEM const GFXfont& FreeMono12pt7b            = FreeMono12pt7b           ;
  static PROGMEM const GFXfont& FreeMono18pt7b            = FreeMono18pt7b           ;
  static PROGMEM const GFXfont& FreeMono24pt7b            = FreeMono24pt7b           ;
  static PROGMEM const GFXfont& FreeMonoBold9pt7b         = FreeMonoBold9pt7b        ;
  static PROGMEM const GFXfont& FreeMonoBold12pt7b        = FreeMonoBold12pt7b       ;
  static PROGMEM const GFXfont& FreeMonoBold18pt7b        = FreeMonoBold18pt7b       ;
  static PROGMEM const GFXfont& FreeMonoBold24pt7b        = FreeMonoBold24pt7b       ;
  static PROGMEM const GFXfont& FreeMonoOblique9pt7b      = FreeMonoOblique9pt7b     ;
  static PROGMEM const GFXfont& FreeMonoOblique12pt7b     = FreeMonoOblique12pt7b    ;
  static PROGMEM const GFXfont& FreeMonoOblique18pt7b     = FreeMonoOblique18pt7b    ;
  static PROGMEM const GFXfont& FreeMonoOblique24pt7b     = FreeMonoOblique24pt7b    ;
  static PROGMEM const GFXfont& FreeMonoBoldOblique9pt7b  = FreeMonoBoldOblique9pt7b ;
  static PROGMEM const GFXfont& FreeMonoBoldOblique12pt7b = FreeMonoBoldOblique12pt7b;
  static PROGMEM const GFXfont& FreeMonoBoldOblique18pt7b = FreeMonoBoldOblique18pt7b;
  static PROGMEM const GFXfont& FreeMonoBoldOblique24pt7b = FreeMonoBoldOblique24pt7b;
  static PROGMEM const GFXfont& FreeSans9pt7b             = FreeSans9pt7b            ;
  static PROGMEM const GFXfont& FreeSans12pt7b            = FreeSans12pt7b           ;
  static PROGMEM const GFXfont& FreeSans18pt7b            = FreeSans18pt7b           ;
  static PROGMEM const GFXfont& FreeSans24pt7b            = FreeSans24pt7b           ;
  static PROGMEM const GFXfont& FreeSansBold9pt7b         = FreeSansBold9pt7b        ;
  static PROGMEM const GFXfont& FreeSansBold12pt7b        = FreeSansBold12pt7b       ;
  static PROGMEM const GFXfont& FreeSansBold18pt7b        = FreeSansBold18pt7b       ;
  static PROGMEM const GFXfont& FreeSansBold24pt7b        = FreeSansBold24pt7b       ;
  static PROGMEM const GFXfont& FreeSansOblique9pt7b      = FreeSansOblique9pt7b     ;
  static PROGMEM const GFXfont& FreeSansOblique12pt7b     = FreeSansOblique12pt7b    ;
  static PROGMEM const GFXfont& FreeSansOblique18pt7b     = FreeSansOblique18pt7b    ;
  static PROGMEM const GFXfont& FreeSansOblique24pt7b     = FreeSansOblique24pt7b    ;
  static PROGMEM const GFXfont& FreeSansBoldOblique9pt7b  = FreeSansBoldOblique9pt7b ;
  static PROGMEM const GFXfont& FreeSansBoldOblique12pt7b = FreeSansBoldOblique12pt7b;
  static PROGMEM const GFXfont& FreeSansBoldOblique18pt7b = FreeSansBoldOblique18pt7b;
  static PROGMEM const GFXfont& FreeSansBoldOblique24pt7b = FreeSansBoldOblique24pt7b;
  static PROGMEM const GFXfont& FreeSerif9pt7b            = FreeSerif9pt7b           ;
  static PROGMEM const GFXfont& FreeSerif12pt7b           = FreeSerif12pt7b          ;
  static PROGMEM const GFXfont& FreeSerif18pt7b           = FreeSerif18pt7b          ;
  static PROGMEM const GFXfont& FreeSerif24pt7b           = FreeSerif24pt7b          ;
  static PROGMEM const GFXfont& FreeSerifItalic9pt7b      = FreeSerifItalic9pt7b     ;
  static PROGMEM const GFXfont& FreeSerifItalic12pt7b     = FreeSerifItalic12pt7b    ;
  static PROGMEM const GFXfont& FreeSerifItalic18pt7b     = FreeSerifItalic18pt7b    ;
  static PROGMEM const GFXfont& FreeSerifItalic24pt7b     = FreeSerifItalic24pt7b    ;
  static PROGMEM const GFXfont& FreeSerifBold9pt7b        = FreeSerifBold9pt7b       ;
  static PROGMEM const GFXfont& FreeSerifBold12pt7b       = FreeSerifBold12pt7b      ;
  static PROGMEM const GFXfont& FreeSerifBold18pt7b       = FreeSerifBold18pt7b      ;
  static PROGMEM const GFXfont& FreeSerifBold24pt7b       = FreeSerifBold24pt7b      ;
  static PROGMEM const GFXfont& FreeSerifBoldItalic9pt7b  = FreeSerifBoldItalic9pt7b ;
  static PROGMEM const GFXfont& FreeSerifBoldItalic12pt7b = FreeSerifBoldItalic12pt7b;
  static PROGMEM const GFXfont& FreeSerifBoldItalic18pt7b = FreeSerifBoldItalic18pt7b;
  static PROGMEM const GFXfont& FreeSerifBoldItalic24pt7b = FreeSerifBoldItalic24pt7b;

  static PROGMEM const GFXfont& Orbitron_Light_24         = Orbitron_Light_24        ;
  static PROGMEM const GFXfont& Orbitron_Light_32         = Orbitron_Light_32        ;
  static PROGMEM const GFXfont& Roboto_Thin_24            = Roboto_Thin_24           ;
  static PROGMEM const GFXfont& Satisfy_24                = Satisfy_24               ;
  static PROGMEM const GFXfont& Yellowtail_32             = Yellowtail_32            ;

}

#endif
