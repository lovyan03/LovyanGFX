/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)
/----------------------------------------------------------------------------*/
#include "Panel_ED2208.hpp"
#include "lgfx/v1/Bus.hpp"
#include "lgfx/v1/platforms/common.hpp"
#include "lgfx/v1/misc/pixelcopy.hpp"
#include "lgfx/v1/misc/colortype.hpp"

#include <cmath>

#ifdef min
#undef min
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  // EPD color indices (panel native 4bpp values)
  static constexpr uint8_t EPD_BLACK  = 0x0;
  static constexpr uint8_t EPD_WHITE  = 0x1;
  static constexpr uint8_t EPD_YELLOW = 0x2;
  static constexpr uint8_t EPD_RED    = 0x3;
  static constexpr uint8_t EPD_BLUE   = 0x5;
  static constexpr uint8_t EPD_GREEN  = 0x6;
  // 0x4 and 0x7 are unused

  static constexpr uint8_t EPD_PALETTE_COUNT = 6;

  // Ideal RGB palette for nearest-color lookup.
  struct palette_t { uint8_t r, g, b, idx; };

  static constexpr palette_t epd_palette[] = {
    {   0,   0,   0, EPD_BLACK  },
    { 255, 255, 255, EPD_WHITE  },
    { 255, 255,   0, EPD_YELLOW },
    { 255,   0,   0, EPD_RED    },
    {   0,   0, 255, EPD_BLUE   },
    {   0, 255,   0, EPD_GREEN  },
  };

  static constexpr uint8_t bayer256[256] = {
      0, 192,  48, 240,  12, 204,  60, 252,   3, 195,  51, 243,  15, 207,  63, 255,
    128,  64, 176, 112, 140,  76, 188, 124, 131,  67, 179, 115, 143,  79, 191, 127,
     32, 224,  16, 208,  44, 236,  28, 220,  35, 227,  19, 211,  47, 239,  31, 223,
    160,  96, 144,  80, 172, 108, 156,  92, 163,  99, 147,  83, 175, 111, 159,  95,
      8, 200,  56, 248,   4, 196,  52, 244,  11, 203,  59, 251,   7, 199,  55, 247,
    136,  72, 184, 120, 132,  68, 180, 116, 139,  75, 187, 123, 135,  71, 183, 119,
     40, 232,  24, 216,  36, 228,  20, 212,  43, 235,  27, 219,  39, 231,  23, 215,
    168, 104, 152,  88, 164, 100, 148,  84, 171, 107, 155,  91, 167, 103, 151,  87,
      2, 194,  50, 242,  14, 206,  62, 254,   1, 193,  49, 241,  13, 205,  61, 253,
    130,  66, 178, 114, 142,  78, 190, 126, 129,  65, 177, 113, 141,  77, 189, 125,
     34, 226,  18, 210,  46, 238,  30, 222,  33, 225,  17, 209,  45, 237,  29, 221,
    162,  98, 146,  82, 174, 110, 158,  94, 161,  97, 145,  81, 173, 109, 157,  93,
     10, 202,  58, 250,   6, 198,  54, 246,   9, 201,  57, 249,   5, 197,  53, 245,
    138,  74, 186, 122, 134,  70, 182, 118, 137,  73, 185, 121, 133,  69, 181, 117,
     42, 234,  26, 218,  38, 230,  22, 214,  41, 233,  25, 217,  37, 229,  21, 213,
    170, 106, 154,  90, 166, 102, 150,  86, 169, 105, 153,  89, 165, 101, 149,  85,
  };

  static uint8_t _rgb_to_epd_color(int32_t r, int32_t g, int32_t b, const palette_t* palette)
  {
    uint32_t min_dist = UINT32_MAX;
    uint8_t best = EPD_WHITE;
    for (size_t i = 0; i < EPD_PALETTE_COUNT; ++i) {
      const auto& p = palette[i];
      int32_t dr = r - (int32_t)p.r;
      int32_t dg = g - (int32_t)p.g;
      int32_t db = b - (int32_t)p.b;
      uint32_t dist = dr * dr + dg * dg + db * db;
      if (dist < min_dist) {
        min_dist = dist;
        best = p.idx;
      }
    }
    return best;
  }

  
  static uint8_t _rgb_to_epd_color_pair(int32_t r0, int32_t g0, int32_t b0,
                                       int32_t r1, int32_t g1, int32_t b1, const palette_t* palette)
  {
    uint32_t min_dist = UINT32_MAX;
    uint8_t best = EPD_WHITE;

    int32_t r0array[EPD_PALETTE_COUNT];
    int32_t g0array[EPD_PALETTE_COUNT];
    int32_t b0array[EPD_PALETTE_COUNT];
    int32_t r1array[EPD_PALETTE_COUNT];
    int32_t g1array[EPD_PALETTE_COUNT];
    int32_t b1array[EPD_PALETTE_COUNT];
    // Per-palette squared individual error for each pixel. Added to the block
    // metric to keep hue information when palette pairs tie on block/brightness
    // terms (e.g. prevents dusty-orange input collapsing to black+white).
    int32_t indiv0[EPD_PALETTE_COUNT];
    int32_t indiv1[EPD_PALETTE_COUNT];
    for (uint_fast8_t index = 0; index < EPD_PALETTE_COUNT; ++index) {
      const auto& p = palette[index];
      int32_t dr0 = r0 - (int32_t)p.r;
      int32_t dg0 = g0 - (int32_t)p.g;
      int32_t db0 = b0 - (int32_t)p.b;
      r0array[index] = dr0;
      g0array[index] = dg0;
      b0array[index] = db0;
      indiv0[index] = (dr0 * dr0 + dg0 * dg0 + db0 * db0);

      int32_t dr1 = r1 - (int32_t)p.r;
      int32_t dg1 = g1 - (int32_t)p.g;
      int32_t db1 = b1 - (int32_t)p.b;
      r1array[index] = dr1;
      g1array[index] = dg1;
      b1array[index] = db1;
      indiv1[index] = (dr1 * dr1 + dg1 * dg1 + db1 * db1);
    }

    for (uint_fast8_t i = 0; i < EPD_PALETTE_COUNT; ++i) {
      for (uint_fast8_t j = 0; j < EPD_PALETTE_COUNT; ++j) {
        int32_t dr = r0array[i] + r1array[j];
        int32_t dg = g0array[i] + g1array[j];
        int32_t db = b0array[i] + b1array[j];
        uint32_t dist = ((uint32_t)(dr * dr + dg * dg + db * db))
                      + ((uint32_t)(indiv0[i] + indiv1[j]));
        if (dist < min_dist) {
          min_dist = dist;
          best = palette[i].idx << 4 | palette[j].idx;
        }
      }
    }
    return best;
  }

  // --- Dither: Bayer RGB (uniform bias on all channels) ---

  static void _dither_row_bayer_simple(const lgfx::bgr888_t* src, uint8_t* dst, uint_fast16_t w, uint_fast16_t y, uint8_t dither)
  {
    auto palette = epd_palette;
    auto row = &bayer256[( y    & 15) << 4];
    for (uint_fast16_t x = 0; x < w; x += 2) {
      int32_t bias = row[x & 15];
      bias = bias * 2 - 255;
      bias = bias * dither >> 9;
      uint8_t c0 = _rgb_to_epd_color((int32_t)src[x].r + bias,
                                     (int32_t)src[x].g + bias,
                                     (int32_t)src[x].b + bias, palette);
      uint8_t c1 = EPD_WHITE;
      if (x + 1 < w) {
        bias = row[(x + 1) & 15];
        bias = bias * 2 - 255;
        bias = bias * dither >> 9;
        c1 = _rgb_to_epd_color((int32_t)src[x + 1].r + bias,
                               (int32_t)src[x + 1].g + bias,
                               (int32_t)src[x + 1].b + bias, palette);
      }
      dst[x >> 1] = (c0 << 4) | c1;
    }
  }

  static void _dither_row_bayer_simple_pair(const lgfx::bgr888_t* src, uint8_t* dst, uint_fast16_t w, uint_fast16_t y, uint8_t dither)
  {
    auto palette = epd_palette;
    auto row = &bayer256[( y    & 15) << 4];
    for (uint_fast16_t x = 0; x < w; x += 2) {
      int32_t bias = row[x & 15];
      bias = bias * 2 - 255;
      bias = bias * dither >> 8;
      int32_t r0 = (int32_t)src[x].r + bias;
      int32_t g0 = (int32_t)src[x].g + bias;
      int32_t b0 = (int32_t)src[x].b + bias;
      int32_t r1 = 0;
      int32_t g1 = 0;
      int32_t b1 = 0;
      if (x + 1 < w) {
        bias = row[(x + 1) & 15];
        bias = bias * 2 - 255;
        bias = bias * dither >> 8;
        r1 = (int32_t)src[x + 1].r + bias;
        g1 = (int32_t)src[x + 1].g + bias;
        b1 = (int32_t)src[x + 1].b + bias;
      }
      dst[x >> 1] = _rgb_to_epd_color_pair(r0, g0, b0, r1, g1, b1, palette);
    }
  }

  static void _dither_row_rgb_pair(const lgfx::bgr888_t* src, uint8_t* dst, uint_fast16_t w, uint_fast16_t y, uint8_t dither)
  {
    auto palette = epd_palette;

    const int32_t x_step = 127 * 29;
    const int32_t y_step = 129 * 48;

    static constexpr size_t step_value = 129*127;
    static constexpr size_t step_diff = step_value / 3;

    int32_t bias_base = y * y_step % step_value;
    // Original scale was /4.0f; keep that at dither_level=255 and scale down linearly.
    int32_t r0 = 0, g0 = 0, b0 = 0;
    for (uint_fast16_t x = 0; x <= w; x ++) {
      int32_t r = 128;
      int32_t g = 128;
      int32_t b = 128;
      if (x < w) {
        r = src[x].r;
        g = src[x].g;
        b = src[x].b;
      }
      bias_base -= x_step;
      if (bias_base < 0) bias_base += step_value;
      int32_t bias_r = bias_base;
      int32_t bias_g = bias_base - step_diff;
      if (bias_g < 0) bias_g += step_value;
      int32_t bias_b = bias_g - step_diff;
      if (bias_b < 0) bias_b += step_value;
      bias_b = bias_b * 2 - (step_value - 1);
      bias_g = bias_g * 2 - (step_value - 1);
      bias_r = bias_r * 2 - (step_value - 1);
      int32_t bias = (bias_r + bias_g + bias_b);
      bias = bias * dither >> 16;
      bias_r = bias_r * dither >> 16;
      bias_g = bias_g * dither >> 16;
      bias_b = bias_b * dither >> 16;
      r += bias + bias_r;
      g += bias + bias_g;
      b += bias + bias_b;
      if (x & 1) {
        dst[x >> 1] = _rgb_to_epd_color_pair(r0, g0, b0, r, g, b, palette);
      } else {
        r0 = r;
        g0 = g;
        b0 = b;
      }
    }
  }

//---------------------------------------------------------------

  // --- Panel implementation ---

  Panel_ED2208::Panel_ED2208(void)
  {
    _cfg.dummy_read_bits = 0;
    _epd_mode = epd_mode_t::epd_quality;
  }

  Panel_ED2208::~Panel_ED2208(void)
  {
    if (_lines_buffer) {
      heap_free(_lines_buffer);
      _lines_buffer = nullptr;
    }
    if (_framebuffer) {
      heap_free(_framebuffer);
      _framebuffer = nullptr;
    }
  }

  color_depth_t Panel_ED2208::setColorDepth(color_depth_t depth)
  {
    (void)depth;
    _write_depth = color_depth_t::rgb888_3Byte;
    _read_depth = color_depth_t::rgb888_3Byte;
    return color_depth_t::rgb888_3Byte;
  }

  void Panel_ED2208::_send_command(uint8_t cmd)
  {
    _bus->writeCommand(cmd, 8);
  }

  void Panel_ED2208::_send_data(uint8_t data)
  {
    _bus->writeData(data, 8);
  }

  bool Panel_ED2208::_wait_busy(uint32_t timeout)
  {
    _bus->wait();
    if (_cfg.pin_busy >= 0 && !gpio_in(_cfg.pin_busy))
    {
      uint32_t start_time = millis();
      do
      {
        if (millis() - start_time > timeout) {
          return false;
        }
        lgfx::delay(10);
      } while (!gpio_in(_cfg.pin_busy));
      lgfx::delay(200);
    }
    return true;
  }

  void Panel_ED2208::_init_sequence(void)
  {
    _bus->beginTransaction();
    cs_control(false);

    for (uint8_t i = 0; auto cmds = getInitCommands(i); i++)
    {
      _wait_busy();
      command_list(cmds);
    }

    _wait_busy();
    _send_command(0x61);    // Resolution
    _send_data((_cfg.panel_width >> 8) & 0xFF);
    _send_data(_cfg.panel_width & 0xFF);
    _send_data((_cfg.panel_height >> 8) & 0xFF);
    _send_data(_cfg.panel_height & 0xFF);

    _bus->wait();
    cs_control(true);
    _bus->endTransaction();
  }

  void Panel_ED2208::_turn_on_display(void)
  {
    _bus->beginTransaction();
    cs_control(false);

    _send_command(0x04);    // POWER_ON
    _wait_busy();
    lgfx::delay(200);

    _send_command(0x06);
    _send_data(0x6F);
    _send_data(0x1F);
    _send_data(0x17);
    _send_data(0x27);
    lgfx::delay(200);

    _send_command(0x12);    // DISPLAY_REFRESH
    _send_data(0x00);
    _wait_busy();

    _send_command(0x02);    // POWER_OFF
    _send_data(0x00);
    _wait_busy();
    lgfx::delay(200);

    _bus->wait();
    cs_control(true);
    _bus->endTransaction();
  }

  bool Panel_ED2208::init(bool use_reset)
  {
    pinMode(_cfg.pin_busy, pin_mode_t::input_pullup);

    setColorDepth(color_depth_t::rgb888_3Byte);

    uint32_t pw = _cfg.panel_width;
    uint32_t ph = _cfg.panel_height;
    uint32_t bytes_per_line = pw * sizeof(bgr888_t);

    if (_framebuffer) { heap_free(_framebuffer); _framebuffer = nullptr; }
    if (_lines_buffer) { heap_free(_lines_buffer); _lines_buffer = nullptr; }

    _framebuffer = static_cast<uint8_t*>(heap_alloc_psram(bytes_per_line * ph));
    if (!_framebuffer) { return false; }

    _lines_buffer = static_cast<uint8_t**>(heap_alloc(ph * sizeof(uint8_t*)));
    if (!_lines_buffer) { heap_free(_framebuffer); _framebuffer = nullptr; return false; }

    for (uint32_t y = 0; y < ph; ++y) {
      _lines_buffer[y] = _framebuffer + y * bytes_per_line;
    }

    memset(_framebuffer, 0xFF, bytes_per_line * ph);

    if (!Panel_FrameBufferBase::init(false))
    {
      return false;
    }

    _after_wake();
    setRotation(_rotation);

    return true;
  }

  void Panel_ED2208::_after_wake(void)
  {
    _init_sequence();
  }

  void Panel_ED2208::waitDisplay(void)
  {
    _wait_busy();
  }

  bool Panel_ED2208::displayBusy(void)
  {
    return _cfg.pin_busy >= 0 && !gpio_in(_cfg.pin_busy);
  }

  void Panel_ED2208::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      uint_fast8_t r = _internal_rotation;
      if (r)
      {
        if ((1u << r) & 0b10010110) { y = _height - (y + h); }
        if (r & 2)                  { x = _width  - (x + w); }
        if (r & 1) { std::swap(x, y);  std::swap(w, h); }
      }
      _range_mod.left   = std::min<int_fast16_t>(_range_mod.left  , x        );
      _range_mod.right  = std::max<int_fast16_t>(_range_mod.right , x + w - 1);
      _range_mod.top    = std::min<int_fast16_t>(_range_mod.top   , y        );
      _range_mod.bottom = std::max<int_fast16_t>(_range_mod.bottom, y + h - 1);
    }
    if (!_range_mod.empty()) {
      _exec_transfer();
      _turn_on_display();
      _range_mod.top = INT16_MAX;
      _range_mod.left = INT16_MAX;
      _range_mod.right = 0;
      _range_mod.bottom = 0;
    }
  }

  // --- Transfer ---

  void Panel_ED2208::_exec_transfer(void)
  {
    uint_fast16_t w = _cfg.panel_width;
    uint_fast16_t h = _cfg.panel_height;
    uint32_t row_bytes = (w + 1) >> 1;

    // Select dither algorithm for the current _epd_mode.
    auto dither_fn = _dither_row_bayer_simple;
    uint8_t dither = 128;
    switch (_epd_mode) {
      case epd_mode_t::epd_fastest: dither_fn = _dither_row_bayer_simple;      dither = 248; break;
      case epd_mode_t::epd_fast:    dither_fn = _dither_row_bayer_simple_pair; dither = 160; break;
      case epd_mode_t::epd_text:    dither_fn = _dither_row_rgb_pair;    dither = 128; break;
      case epd_mode_t::epd_quality: dither_fn = _dither_row_rgb_pair;    dither = 248; break;
      default: break;
    }

    _bus->beginTransaction();
    cs_control(false);

    _send_command(0x10);    // Data start transmission

    for (uint_fast16_t y = 0; y < h; ++y) {
      uint8_t* dst = _bus->getDMABuffer(row_bytes);
      const bgr888_t* src = reinterpret_cast<const bgr888_t*>(_lines_buffer[y]);
      dither_fn(src, dst, w, y, dither);
      _bus->writeBytes(dst, row_bytes, true, true);
    }
    _bus->wait();

    cs_control(true);
    _bus->endTransaction();
  }

  void Panel_ED2208::setSleep(bool flg)
  {
    if (flg)
    {
      startWrite();
      _send_command(0x07);  // DEEP_SLEEP
      _send_data(0xA5);
      endWrite();
    }
    else
    {
      _after_wake();
    }
  }

  void Panel_ED2208::setPowerSave(bool flg)
  {
    if (flg) {
      setSleep(true);
    }
  }

//----------------------------------------------------------------------------
 }
}
