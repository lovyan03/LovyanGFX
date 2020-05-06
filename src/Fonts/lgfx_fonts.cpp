#include "lgfx_fonts.hpp"

#include "glcdfont.h"
#include "Font16.h"
#include "Font32rle.h"
#include "Font64rle.h"
#include "Font7srle.h"
#include "Font72rle.h"

namespace fonts {
  const lgfx::GLCDfont Font0 = { (const uint8_t *)font, nullptr, 6, 8, 7};
  const lgfx::BMPfont  Font2 = { (const uint8_t *)chrtbl_f16, widtbl_f16, 0, chr_hgt_f16, baseline_f16 };
  const lgfx::RLEfont  Font4 = { (const uint8_t *)chrtbl_f32, widtbl_f32, 0, chr_hgt_f32, baseline_f32 };
  const lgfx::RLEfont  Font6 = { (const uint8_t *)chrtbl_f64, widtbl_f64, 0, chr_hgt_f64, baseline_f64 };
  const lgfx::RLEfont  Font7 = { (const uint8_t *)chrtbl_f7s, widtbl_f7s, 0, chr_hgt_f7s, baseline_f7s };
  const lgfx::RLEfont  Font8 = { (const uint8_t *)chrtbl_f72, widtbl_f72, 0, chr_hgt_f72, baseline_f72 };
}

