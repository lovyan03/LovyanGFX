#ifndef LGFX_FONTLOAD_HPP_
#define LGFX_FONTLOAD_HPP_


#include "Fonts/glcdfont.c"
#include "Fonts/gfxfont.h"

#include "Fonts/Font16.h"
#include "Fonts/Font32rle.h"
#include "Fonts/Font64rle.h"
#include "Fonts/Font7srle.h"
#include "Fonts/Font72rle.h"

const  uint8_t widtbl_null[1] = {0};
PROGMEM const uint8_t chr_null[1] = {0};
PROGMEM const uint8_t* const chrtbl_null[1] = {chr_null};

enum font_type_t
{ ft_unknown
, ft_glcd
, ft_bmp
, ft_rle
};

struct fontinfo {
  font_type_t type;
  const uint8_t *chartbl;
  const uint8_t *widthtbl;
  uint8_t height;
  uint8_t baseline;
};

const PROGMEM fontinfo fontdata [] = {
   { ft_unknown, (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },
   // GLCD font (Font 1) does not have all parameters
   { ft_glcd, (const uint8_t *)font, widtbl_null, 8, 6 },

   { ft_bmp,  (const uint8_t *)chrtbl_f16, widtbl_f16, chr_hgt_f16, baseline_f16},

   // Font 3 current unused
   { ft_unknown, (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },

   { ft_rle, (const uint8_t *)chrtbl_f32, widtbl_f32, chr_hgt_f32, baseline_f32},

   // Font 5 current unused
   { ft_unknown, (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },

   { ft_rle, (const uint8_t *)chrtbl_f64, widtbl_f64, chr_hgt_f64, baseline_f64},

   { ft_rle, (const uint8_t *)chrtbl_f7s, widtbl_f7s, chr_hgt_f7s, baseline_f7s},

   { ft_rle, (const uint8_t *)chrtbl_f72, widtbl_f72, chr_hgt_f72, baseline_f72}
};


#ifdef LOAD_GFXFF

  // Original Adafruit_GFX "Free Fonts"
  #include "Fonts/GFXFF/TomThumb.h"  // TT1

  #include "Fonts/GFXFF/FreeMono9pt7b.h"  // FF1 or FM9
  #include "Fonts/GFXFF/FreeMono12pt7b.h" // FF2 or FM12
  #include "Fonts/GFXFF/FreeMono18pt7b.h" // FF3 or FM18
  #include "Fonts/GFXFF/FreeMono24pt7b.h" // FF4 or FM24

  #include "Fonts/GFXFF/FreeMonoOblique9pt7b.h"  // FF5 or FMO9
  #include "Fonts/GFXFF/FreeMonoOblique12pt7b.h" // FF6 or FMO12
  #include "Fonts/GFXFF/FreeMonoOblique18pt7b.h" // FF7 or FMO18
  #include "Fonts/GFXFF/FreeMonoOblique24pt7b.h" // FF8 or FMO24

  #include "Fonts/GFXFF/FreeMonoBold9pt7b.h"  // FF9  or FMB9
  #include "Fonts/GFXFF/FreeMonoBold12pt7b.h" // FF10 or FMB12
  #include "Fonts/GFXFF/FreeMonoBold18pt7b.h" // FF11 or FMB18
  #include "Fonts/GFXFF/FreeMonoBold24pt7b.h" // FF12 or FMB24

  #include "Fonts/GFXFF/FreeMonoBoldOblique9pt7b.h"  // FF13 or FMBO9
  #include "Fonts/GFXFF/FreeMonoBoldOblique12pt7b.h" // FF14 or FMBO12
  #include "Fonts/GFXFF/FreeMonoBoldOblique18pt7b.h" // FF15 or FMBO18
  #include "Fonts/GFXFF/FreeMonoBoldOblique24pt7b.h" // FF16 or FMBO24

  // Sans serif fonts
  #include "Fonts/GFXFF/FreeSans9pt7b.h"  // FF17 or FSS9
  #include "Fonts/GFXFF/FreeSans12pt7b.h" // FF18 or FSS12
  #include "Fonts/GFXFF/FreeSans18pt7b.h" // FF19 or FSS18
  #include "Fonts/GFXFF/FreeSans24pt7b.h" // FF20 or FSS24

  #include "Fonts/GFXFF/FreeSansOblique9pt7b.h"  // FF21 or FSSO9
  #include "Fonts/GFXFF/FreeSansOblique12pt7b.h" // FF22 or FSSO12
  #include "Fonts/GFXFF/FreeSansOblique18pt7b.h" // FF23 or FSSO18
  #include "Fonts/GFXFF/FreeSansOblique24pt7b.h" // FF24 or FSSO24

  #include "Fonts/GFXFF/FreeSansBold9pt7b.h"  // FF25 or FSSB9
  #include "Fonts/GFXFF/FreeSansBold12pt7b.h" // FF26 or FSSB12
  #include "Fonts/GFXFF/FreeSansBold18pt7b.h" // FF27 or FSSB18
  #include "Fonts/GFXFF/FreeSansBold24pt7b.h" // FF28 or FSSB24

  #include "Fonts/GFXFF/FreeSansBoldOblique9pt7b.h"  // FF29 or FSSBO9
  #include "Fonts/GFXFF/FreeSansBoldOblique12pt7b.h" // FF30 or FSSBO12
  #include "Fonts/GFXFF/FreeSansBoldOblique18pt7b.h" // FF31 or FSSBO18
  #include "Fonts/GFXFF/FreeSansBoldOblique24pt7b.h" // FF32 or FSSBO24

  // Serif fonts
  #include "Fonts/GFXFF/FreeSerif9pt7b.h"  // FF33 or FS9
  #include "Fonts/GFXFF/FreeSerif12pt7b.h" // FF34 or FS12
  #include "Fonts/GFXFF/FreeSerif18pt7b.h" // FF35 or FS18
  #include "Fonts/GFXFF/FreeSerif24pt7b.h" // FF36 or FS24

  #include "Fonts/GFXFF/FreeSerifItalic9pt7b.h"  // FF37 or FSI9
  #include "Fonts/GFXFF/FreeSerifItalic12pt7b.h" // FF38 or FSI12
  #include "Fonts/GFXFF/FreeSerifItalic18pt7b.h" // FF39 or FSI18
  #include "Fonts/GFXFF/FreeSerifItalic24pt7b.h" // FF40 or FSI24

  #include "Fonts/GFXFF/FreeSerifBold9pt7b.h"  // FF41 or FSB9
  #include "Fonts/GFXFF/FreeSerifBold12pt7b.h" // FF42 or FSB12
  #include "Fonts/GFXFF/FreeSerifBold18pt7b.h" // FF43 or FSB18
  #include "Fonts/GFXFF/FreeSerifBold24pt7b.h" // FF44 or FSB24

  #include "Fonts/GFXFF/FreeSerifBoldItalic9pt7b.h"  // FF45 or FSBI9
  #include "Fonts/GFXFF/FreeSerifBoldItalic12pt7b.h" // FF46 or FSBI12
  #include "Fonts/GFXFF/FreeSerifBoldItalic18pt7b.h" // FF47 or FSBI18
  #include "Fonts/GFXFF/FreeSerifBoldItalic24pt7b.h" // FF48 or FSBI24

#endif

#endif
