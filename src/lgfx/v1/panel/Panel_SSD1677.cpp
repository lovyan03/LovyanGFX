/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#include "Panel_SSD1677.hpp"
#include "lgfx/v1/Bus.hpp"
#include "lgfx/v1/platforms/common.hpp"
#include "lgfx/v1/misc/pixelcopy.hpp"
#include "lgfx/v1/misc/colortype.hpp"

#include <string.h>

#ifdef min
#undef min
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr int8_t Bayer[16] = { -30, 18, -22, 26, -14, 2, -6, 10, -18, 30, -26, 22, -2, 14, -10, 6 };

  // SSD1677 commands
  static constexpr uint8_t CMD_DEEP_SLEEP        = 0x10;
  static constexpr uint8_t CMD_DATA_ENTRY        = 0x11;
  static constexpr uint8_t CMD_MASTER_ACTIVATION = 0x20;
  static constexpr uint8_t CMD_DISP_UPDATE_CTRL1 = 0x21;
  static constexpr uint8_t CMD_DISP_UPDATE_CTRL2 = 0x22;
  static constexpr uint8_t CMD_WRITE_RAM_BW      = 0x24; // current frame / LSB plane
  static constexpr uint8_t CMD_WRITE_RAM_RED     = 0x26; // previous frame / MSB plane
  static constexpr uint8_t CMD_WRITE_TEMP        = 0x1A;
  static constexpr uint8_t CMD_WRITE_LUT         = 0x32;
  static constexpr uint8_t CMD_GATE_VOLT         = 0x03; // VGH
  static constexpr uint8_t CMD_SOURCE_VOLT       = 0x04; // VSH1, VSH2, VSL
  static constexpr uint8_t CMD_WRITE_VCOM        = 0x2C;
  static constexpr uint8_t CMD_SET_RAM_X         = 0x44;
  static constexpr uint8_t CMD_SET_RAM_Y         = 0x45;
  static constexpr uint8_t CMD_SET_RAM_X_CNT     = 0x4E;
  static constexpr uint8_t CMD_SET_RAM_Y_CNT     = 0x4F;

  static constexpr uint8_t CTRL1_NORMAL     = 0x00;
  static constexpr uint8_t CTRL1_BYPASS_RED = 0x40;

  //--------------------------------------------------------------------------
  // SSD1677 LUT layout.
  // Layout: VS patterns (5 groups x 10 bytes) + TP/RP timing (10 groups x 5 bytes)
  //         + frame rate (5 bytes) = 105 bytes -> command 0x32.
  // Then voltages [VGH, VSH1, VSH2, VSL, VCOM] (bytes 105..109) -> 0x03/0x04/0x2C.
  //--------------------------------------------------------------------------

  // VS codes: 0 = VSS (no drive), 1 = VSH1 (darken), 2 = VSL (lighten),
  // 3 = VSH2 (weak darken).
  // For the absolute waveforms (quality/text/fast) the RAM group selects the
  // target level: group 0 = white, 1 = light gray, 2 = dark gray, 3 = black.

  // Quality: an oscillation prefix erases the previous image, then the
  // four-gray tail forms the target.
  // Keep |VSH1| == |VSL|: the net drive stays identical for all groups,
  // which prevents image-correlated ghosting over repeated refreshes.
  // The timing groups repeat the tail to stabilize the black and white
  // endpoints.
  static constexpr uint8_t lut_quality[110] = {
    0x66, 0x66, 0x00, 0x4A, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, // white
    0x66, 0x66, 0x80, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // light
    0x66, 0x66, 0x88, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // dark
    0x66, 0x66, 0xA8, 0x44, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, // black
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // VCOM
    0x05, 0x05, 0x05, 0x05, 0x00,
    0x05, 0x05, 0x05, 0x05, 0x01,
    0x08, 0x0B, 0x02, 0x03, 0x01,
    0x0C, 0x02, 0x07, 0x02, 0x01,
    0x01, 0x00, 0x02, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x22, 0x22, 0x22, 0x22, 0x22,       // frame rate
    0x17, 0x46, 0xA8, 0x36, 0x30,       // VGH, VSH1(+16V), VSH2, VSL(-16V), VCOM(-1.2V)
  };

  // Text: 64-frame absolute four-gray waveform for Mode 1.
  static constexpr uint8_t lut_text[110] = {
    0x55, 0x55, 0x55, 0x55, 0x55, 0x5A, 0xAA, 0xAA, 0x00, 0x00, // white
    0xAA, 0x95, 0x55, 0x55, 0x55, 0x5A, 0x82, 0xA0, 0x00, 0x00, // light
    0xAA, 0xA5, 0x55, 0x55, 0x55, 0x5A, 0xA0, 0x00, 0x00, 0x00, // dark
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x50, 0x00, 0x00, // black
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // VCOM
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x8F, 0x8F, 0x8F, 0x8F, 0x8F,       // frame rate
    0x17, 0x41, 0xA8, 0x32, 0x30,       // VGH, VSH1(+15V), VSH2, VSL(-15V), VCOM(-1.2V)
  };

  // Fast: 48-frame absolute four-gray waveform for Mode 2.
  // The first 16 phases run for one frame and the remaining phases for two,
  // while the final HOLD phase of each middle-gray row uses VSH2 for a weak
  // two-frame darkening trim.
  static constexpr uint8_t lut_fast[110] = {
    0x55, 0x55, 0x55, 0x55, 0x55, 0x5A, 0xAA, 0xAA, 0x00, 0x00, // white
    0xAA, 0x95, 0x55, 0x55, 0x55, 0x5A, 0x82, 0xAC, 0x00, 0x00, // light
    0xAA, 0xA5, 0x55, 0x55, 0x55, 0x5A, 0xAC, 0x00, 0x00, 0x00, // dark
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x50, 0x00, 0x00, // black
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // VCOM
    0x01, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x8F, 0x8F, 0x8F, 0x8F, 0x8F,       // frame rate
    0x17, 0x41, 0xA8, 0x32, 0x30,       // VGH, VSH1(+15V), VSH2, VSL(-15V), VCOM(-1.2V)
  };

  // Fastest: differential monochrome update (Mode 2). Here the RAM group is
  // the transition class computed in _send_transition_planes: group 0 holds
  // dark pixels, 3 holds light pixels, 1 drives black-to-white and 2 drives
  // white-to-black. A 2-frame reverse-polarity prepulse precedes the
  // 8-frame dose to curb ghosting from repeated partial updates.
  static constexpr uint8_t lut_fastest[110] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hold dark
    0x6A, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // black -> white
    0x95, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // white -> black
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hold light
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // VCOM
    0x02, 0x02, 0x02, 0x02, 0x00,
    0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x8F, 0x8F, 0x8F, 0x8F, 0x8F,       // frame rate
    // VCOM -1.2V keeps the hold groups near field-free.
    0x17, 0x46, 0xA8, 0x32, 0x30,       // VGH, VSH1(+16V), VSH2, VSL(-15V), VCOM(-1.2V)
  };

  // Write a 110-byte LUT: 105 waveform bytes -> 0x32, voltages -> 0x03/0x04/0x2C.
  static void send_lut(IBus* bus, const uint8_t* lut)
  {
    bus->writeCommand(CMD_WRITE_LUT, 8);
    bus->writeBytes(lut, 105, true, false);
    bus->writeCommand(CMD_GATE_VOLT, 8);
    bus->writeData(lut[105], 8);                 // VGH
    bus->writeCommand(CMD_SOURCE_VOLT, 8);
    bus->writeData(lut[106], 8);                 // VSH1
    bus->writeData(lut[107], 8);                 // VSH2
    bus->writeData(lut[108], 8);                 // VSL
    bus->writeCommand(CMD_WRITE_VCOM, 8);
    bus->writeData(lut[109], 8);                 // VCOM
  }

  //--------------------------------------------------------------------------

  Panel_SSD1677::Panel_SSD1677(void)
  {
    _cfg.dummy_read_bits = 0;
    _epd_mode = epd_mode_t::epd_quality;
  }

  Panel_SSD1677::~Panel_SSD1677(void) = default;

  color_depth_t Panel_SSD1677::setColorDepth(color_depth_t depth)
  {
    (void)depth;
    _write_depth = color_depth_t::grayscale_8bit;
    _read_depth = color_depth_t::grayscale_8bit;
    return color_depth_t::grayscale_8bit;
  }

  uint32_t Panel_SSD1677::_get_plane_length(void) const
  {
    // EPD native: row indexed by Y(gate), packed along X(source).
    // row_bytes = (panel_width+7)/8 ; rows = panel_height.
    return (((_cfg.panel_width + 7) & ~7) >> 3) * _cfg.panel_height;
  }

  size_t Panel_SSD1677::_get_buffer_length(void) const
  {
    return _get_plane_length() * 2; // planeL + planeM
  }

  bool Panel_SSD1677::init(bool use_reset)
  {
    pinMode(_cfg.pin_busy, pin_mode_t::input_pullup);

    if (!Panel_HasBuffer::init(use_reset))
    {
      return false;
    }
    _buf_x1_len = _get_plane_length();
    memset(_buf, 0xFF, _get_buffer_length()); // white
    _after_wake();
    return true;
  }

  void Panel_SSD1677::_after_wake(void)
  {
    startWrite(true);
    for (uint8_t i = 0; auto cmds = getInitCommands(i); i++)
    {
      _wait_busy();
      command_list(cmds);
    }

    // Full-screen RAM window + clear both RAM banks to white.
    _set_ram_area(0, 0, _cfg.panel_width, _cfg.panel_height);
    _wait_busy();
    _bus->writeCommand(0x46, 8); _bus->writeData(0xF7, 8); // auto write BW RAM
    _wait_busy();
    _bus->writeCommand(0x47, 8); _bus->writeData(0xF7, 8); // auto write RED RAM
    _wait_busy();

    _screen_on = false;
    _last_epd_mode = (epd_mode_t)~0u;
    _initialize_seq = true;

    setRotation(_rotation);

    _range_old.top = 0;
    _range_old.left = 0;
    _range_old.right = _cfg.panel_width - 1;
    _range_old.bottom = _cfg.panel_height - 1;
    _range_mod.top = INT16_MAX;
    _range_mod.left = INT16_MAX;
    _range_mod.right = 0;
    _range_mod.bottom = 0;

    endWrite();
  }

  void Panel_SSD1677::_power_on(void)
  {
    if (_screen_on) { return; }
    _bus->writeCommand(CMD_DISP_UPDATE_CTRL2, 8);
    _bus->writeData(0xC0, 8);
    _bus->writeCommand(CMD_MASTER_ACTIVATION, 8);
    _send_msec = millis();
    _wait_busy();
    _screen_on = true;
  }

  void Panel_SSD1677::waitDisplay(void)
  {
    _wait_busy();
  }

  bool Panel_SSD1677::displayBusy(void)
  {
    return _cfg.pin_busy >= 0 && gpio_in(_cfg.pin_busy);
  }

  void Panel_SSD1677::_set_ram_area(int32_t x, int32_t y, int32_t w, int32_t h)
  {
    // Native coords: X = source (0..799), Y = gate (0..479).
    // Gates are reversed on this panel -> reverse Y and use Y-decrement.
    int32_t yrev = _cfg.panel_height - y - h;

    _bus->writeCommand(CMD_DATA_ENTRY, 8);
    _bus->writeData(0x01, 8); // X increment, Y decrement

    _bus->writeCommand(CMD_SET_RAM_X, 8);
    _bus->writeData(x & 0xFF, 8);
    _bus->writeData((x >> 8) & 0xFF, 8);
    _bus->writeData((x + w - 1) & 0xFF, 8);
    _bus->writeData(((x + w - 1) >> 8) & 0xFF, 8);

    _bus->writeCommand(CMD_SET_RAM_Y, 8);
    _bus->writeData((yrev + h - 1) & 0xFF, 8);
    _bus->writeData(((yrev + h - 1) >> 8) & 0xFF, 8);
    _bus->writeData(yrev & 0xFF, 8);
    _bus->writeData((yrev >> 8) & 0xFF, 8);

    _bus->writeCommand(CMD_SET_RAM_X_CNT, 8);
    _bus->writeData(x & 0xFF, 8);
    _bus->writeData((x >> 8) & 0xFF, 8);

    _bus->writeCommand(CMD_SET_RAM_Y_CNT, 8);
    _bus->writeData((yrev + h - 1) & 0xFF, 8);
    _bus->writeData(((yrev + h - 1) >> 8) & 0xFF, 8);
  }

  void Panel_SSD1677::_send_plane(uint32_t cmd, const uint8_t* plane, const range_rect_t& range, bool extra_invert)
  {
    int32_t xs = range.left & ~7;
    int32_t xe = range.right | 7;
    if (xe >= (int32_t)_cfg.panel_width) { xe = _cfg.panel_width - 1; }
    int32_t ys = range.top;
    int32_t ye = range.bottom;
    if (ye >= (int32_t)_cfg.panel_height) { ye = _cfg.panel_height - 1; }

    _set_ram_area(xs, ys, xe - xs + 1, ye - ys + 1);
    _wait_busy();
    _bus->writeCommand(cmd, 8);

    int32_t row_bytes = ((_cfg.panel_width + 7) & ~7) >> 3;
    int32_t xbytes = (xe - xs + 1) >> 3;
    int32_t rows = ye - ys + 1;
    const uint8_t* b = &plane[ys * row_bytes + (xs >> 3)];

    bool inv = extra_invert ^ _invert ^ _cfg.invert;
    if (inv)
    {
      uint8_t tmp[128];
      for (int32_t row = 0; row < rows; row++)
      {
        for (int32_t i = 0; i < xbytes; i++) { tmp[i] = ~b[i]; }
        _bus->writeBytes(tmp, xbytes, true, false);
        b += row_bytes;
      }
    }
    else if (xbytes == row_bytes)
    {
      _bus->writeBytes(b, xbytes * rows, true, true);
    }
    else
    {
      for (int32_t row = 0; row < rows; row++)
      {
        _bus->writeBytes(b, xbytes, true, true);
        b += row_bytes;
      }
    }
  }

  void Panel_SSD1677::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      uint_fast16_t xs = x, ys = y, xe = x + w - 1, ye = y + h - 1;
      _rotate_pos(xs, ys, xe, ye);
      _range_mod.left   = std::min<int32_t>(_range_mod.left  , std::min(xs, xe));
      _range_mod.right  = std::max<int32_t>(_range_mod.right , std::max(xs, xe));
      _range_mod.top    = std::min<int32_t>(_range_mod.top   , std::min(ys, ye));
      _range_mod.bottom = std::max<int32_t>(_range_mod.bottom, std::max(ys, ye));
    }
    if (_range_mod.empty()) { return; }

    auto mode = getEpdMode();
    bool is_full = _initialize_seq || (_last_epd_mode != mode)
                || (mode == epd_mode_t::epd_quality) || (mode == epd_mode_t::epd_text);

    if (is_full)
    {
      _range_mod.left = 0;
      _range_mod.top = 0;
      _range_mod.right = _cfg.panel_width - 1;
      _range_mod.bottom = _cfg.panel_height - 1;
    }

    // For fast diff, include the previously-changed region too.
    range_rect_t tr = _range_mod;
    if (tr.top > _range_old.top) { tr.top = _range_old.top; }
    if (tr.left > _range_old.left) { tr.left = _range_old.left; }
    if (tr.right < _range_old.right) { tr.right = _range_old.right; }
    if (tr.bottom < _range_old.bottom) { tr.bottom = _range_old.bottom; }
    _range_old = _range_mod;

    startWrite();

    // Base panel renders B/W: use planeM (MSB) as the monochrome bit (white if v>=2).
    const uint8_t* img = &_buf[_buf_x1_len];

    _send_plane(CMD_WRITE_RAM_BW, img, tr);
    if (is_full)
    {
      _send_plane(CMD_WRITE_RAM_RED, img, tr); // full/half: same image to both
    }

    // Built-in monochrome refresh sequence.
    _wait_busy();
    _bus->writeCommand(CMD_DISP_UPDATE_CTRL1, 8);
    _bus->writeData(is_full ? CTRL1_BYPASS_RED : CTRL1_NORMAL, 8);

    uint8_t dm = 0;
    if (!_screen_on) { _screen_on = true; dm |= 0xC0; }
    if (mode == epd_mode_t::epd_quality) { dm |= 0x34; }       // FULL
    else if (mode == epd_mode_t::epd_text)                     // HALF
    {
      _bus->writeCommand(CMD_WRITE_TEMP, 8);
      _bus->writeData(0x5A, 8);
      dm |= 0xD4;
    }
    else { dm |= 0x1C; }                                        // FAST (built-in LUT)

    _bus->writeCommand(CMD_DISP_UPDATE_CTRL2, 8);
    _bus->writeData(dm, 8);
    _bus->writeCommand(CMD_MASTER_ACTIVATION, 8);
    _send_msec = millis();
    _wait_busy();

    if (!is_full)
    {
      // Sync RED RAM with current frame so it serves as "previous" next time.
      _send_plane(CMD_WRITE_RAM_RED, img, tr);
    }

    _initialize_seq = false;
    _last_epd_mode = mode;
    _range_mod.top = INT16_MAX;
    _range_mod.left = INT16_MAX;
    _range_mod.right = 0;
    _range_mod.bottom = 0;

    endWrite();
  }

  void Panel_SSD1677::setInvert(bool invert)
  {
    _invert = invert;
    _range_mod.top = 0;
    _range_mod.left = 0;
    _range_mod.right = _cfg.panel_width - 1;
    _range_mod.bottom = _cfg.panel_height - 1;
  }

  void Panel_SSD1677::setSleep(bool flg)
  {
    if (flg)
    {
      startWrite();
      _wait_busy();
      _bus->writeCommand(CMD_DISP_UPDATE_CTRL2, 8);
      _bus->writeData(0x03, 8); // analog off + clock off
      _bus->writeCommand(CMD_MASTER_ACTIVATION, 8);
      _wait_busy();
      _bus->writeCommand(CMD_DEEP_SLEEP, 8);
      _bus->writeData(0x01, 8);
      _screen_on = false;
      endWrite();
    }
    else
    {
      // Deep-sleep wake requires a hardware reset (RST may be external -> rst_control).
      rst_control(false);
      delay(10);
      rst_control(true);
      delay(10);
      _after_wake();
    }
  }

  void Panel_SSD1677::setPowerSave(bool flg)
  {
    startWrite();
    _wait_busy();
    _bus->writeCommand(CMD_DISP_UPDATE_CTRL2, 8);
    _bus->writeData(flg ? 0x03 : 0xC0, 8);
    _bus->writeCommand(CMD_MASTER_ACTIVATION, 8);
    _wait_busy();
    _screen_on = !flg;
    endWrite();
  }

  void Panel_SSD1677::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _xs = xs; _ys = ys; _xe = xe; _ye = ye;
    _update_transferred_rect(xs, ys, xe, ye);

    int32_t value = rawcolor;
    int32_t row_bytes = ((_cfg.panel_width + 7) & ~7) >> 3;

    y = ys;
    do
    {
      x = xs;
      auto btbl = &Bayer[(y & 3) << 2];
      do
      {
        uint32_t byte_idx = y * row_bytes + (x >> 3);
        uint8_t bit_mask = 0x80 >> (x & 7);
        int_fast8_t v = (value + btbl[x & 3]) >> 6;
        v = (v < 0) ? 0 : (v > 3 ? 3 : v);
        if (v & 1) _buf[byte_idx] |=  bit_mask; else _buf[byte_idx] &= ~bit_mask;
        if (v & 2) _buf[byte_idx + _buf_x1_len] |=  bit_mask; else _buf[byte_idx + _buf_x1_len] &= ~bit_mask;
      } while (++x <= xe);
    } while (++y <= ye);
  }

  void Panel_SSD1677::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    (void)use_dma;
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _update_transferred_rect(xs, ys, xe, ye);

    auto readbuf = (grayscale_t*)alloca(w * sizeof(grayscale_t));
    auto sx = param->src_x32;
    h += y;
    do
    {
      uint32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          do
          {
            auto color = readbuf[prev_pos];
            _draw_pixel(x + prev_pos, y, color.raw);
          } while (new_pos != ++prev_pos);
        }
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
    } while (++y < h);
  }

  void Panel_SSD1677::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    (void)use_dma;
    {
      uint_fast16_t xs = _xs, xe = _xe, ys = _ys, ye = _ye;
      _update_transferred_rect(xs, ys, xe, ye);
    }
    uint_fast16_t xs = _xs, ys = _ys, xe = _xe, ye = _ye;
    uint_fast16_t xpos = _xpos, ypos = _ypos;

    static constexpr uint32_t buflen = 16;
    grayscale_t colors[buflen];
    int bufpos = buflen;
    do
    {
      if (bufpos == (int)buflen) {
        param->fp_copy(colors, 0, std::min(length, buflen), param);
        bufpos = 0;
      }
      auto color = colors[bufpos++];
      _draw_pixel(xpos, ypos, color.raw);
      if (++xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye) { ypos = ys; }
      }
    } while (--length);
    _xpos = xpos;
    _ypos = ypos;
  }

  void Panel_SSD1677::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    auto readbuf = (grayscale_t*)alloca(w * sizeof(grayscale_t));
    param->src_data = readbuf;
    int32_t readpos = 0;
    h += y;
    do
    {
      uint32_t idx = 0;
      do
      {
        readbuf[idx] = _read_pixel(x + idx, y) * 0x55;
      } while (++idx != w);
      param->src_x32 = 0;
      readpos = param->fp_copy(dst, readpos, readpos + w, param);
    } while (++y < h);
  }

  bool Panel_SSD1677::_wait_busy(uint32_t timeout, bool enforce_refresh_minimum)
  {
    _bus->wait();
    // BUSY is not guaranteed to assert in the same instant that the command
    // transfer completes.  Sampling it immediately can therefore mistake the
    // pre-activation LOW level for completion and return before the waveform
    // has even started.
    delay(2);
    if (_cfg.pin_busy >= 0 && gpio_in(_cfg.pin_busy))
    {
      uint32_t start_time = millis();
      if (enforce_refresh_minimum)
      {
        uint32_t delay_msec = _refresh_msec - (start_time - _send_msec);
        if (delay_msec && delay_msec < timeout) { delay(delay_msec); }
      }
      do
      {
        if (millis() - start_time > timeout) { return false; }
        delay(1);
      } while (gpio_in(_cfg.pin_busy));
    }
    return true;
  }

  void Panel_SSD1677::_draw_pixel(uint_fast16_t x, uint_fast16_t y, uint32_t value)
  {
    _rotate_pos(x, y);
    int32_t row_bytes = ((_cfg.panel_width + 7) & ~7) >> 3;
    uint32_t byte_idx = y * row_bytes + (x >> 3);
    uint8_t bit_mask = 0x80 >> (x & 7);
    int_fast8_t v = ((int32_t)value + (Bayer[(x & 3) + ((y & 3) << 2)])) >> 6;
    v = (v < 0) ? 0 : (v > 3 ? 3 : v);
    if (v & 1) _buf[byte_idx] |=  bit_mask; else _buf[byte_idx] &= ~bit_mask;
    if (v & 2) _buf[byte_idx + _buf_x1_len] |=  bit_mask; else _buf[byte_idx + _buf_x1_len] &= ~bit_mask;
  }

  uint8_t Panel_SSD1677::_read_pixel(uint_fast16_t x, uint_fast16_t y)
  {
    _rotate_pos(x, y);
    int32_t row_bytes = ((_cfg.panel_width + 7) & ~7) >> 3;
    uint32_t byte_idx = y * row_bytes + (x >> 3);
    uint8_t bit_mask = 0x80 >> (x & 7);
    uint_fast8_t result = (_buf[byte_idx] & bit_mask) ? 1 : 0;
    result += (_buf[byte_idx + _buf_x1_len] & bit_mask) ? 2 : 0;
    return result;
  }

  void Panel_SSD1677::_update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye)
  {
    _rotate_pos(xs, ys, xe, ye);

    // X (source) direction is byte-packed -> align to 8.
    int32_t x1 = xs & ~7;
    int32_t x2 = (xe & ~7) + 7;
    if (x2 >= (int32_t)_cfg.panel_width) { x2 = _cfg.panel_width - 1; }
    if (ye >= _cfg.panel_height) { ye = _cfg.panel_height - 1; }

    _range_mod.left   = std::min<int32_t>(x1, _range_mod.left);
    _range_mod.right  = std::max<int32_t>(x2, _range_mod.right);
    _range_mod.top    = std::min<int32_t>(ys, _range_mod.top);
    _range_mod.bottom = std::max<int32_t>(ye, _range_mod.bottom);
  }

  //==========================================================================
  // Panel_SSD1677_4Gray
  //==========================================================================

  Panel_SSD1677_4Gray::~Panel_SSD1677_4Gray(void)
  {
    if (_displayed_buf) { heap_free(_displayed_buf); }
  }

  bool Panel_SSD1677_4Gray::init(bool use_reset)
  {
    bool result = Panel_SSD1677::init(use_reset);
    _invalidate_gray_state();
    if (result)
    {
      if (_displayed_buf) { heap_free(_displayed_buf); }
      _displayed_buf = static_cast<uint8_t*>(heap_alloc_psram(_get_buffer_length()));
      if (!_displayed_buf) { return false; }
      memset(_displayed_buf, 0xFF, _get_buffer_length());
      _displayed_valid = false;
      // _after_wake() performs a software reset and establishes the first
      // (non-swapped) SSD1677 RAM face.
      _mode2_face_odd = false;
      _mode2_face_known = true;
    }
    return result;
  }

  void Panel_SSD1677_4Gray::setSleep(bool flg)
  {
    Panel_SSD1677::setSleep(flg);
    _invalidate_gray_state();
    if (!flg)
    {
      // The hardware reset in the wake path establishes the even RAM face.
      _mode2_face_odd = false;
      _mode2_face_known = true;
    }
  }

  void Panel_SSD1677_4Gray::setPowerSave(bool flg)
  {
    Panel_SSD1677::setPowerSave(flg);
    // Analog power transitions make the retained Mode 2 face/history unsafe.
    // The next Fast/Fastest update will reset and rebuild them.
    _invalidate_gray_state();
  }

  void Panel_SSD1677_4Gray::_invalidate_gray_state(void)
  {
    _optical_state = optical_state_t::unknown;
    _mode2_face_odd = false;
    _mode2_face_known = false;
    _displayed_valid = false;
  }

  void Panel_SSD1677_4Gray::_clear_modified_range(void)
  {
    _range_mod.top = INT16_MAX;
    _range_mod.left = INT16_MAX;
    _range_mod.right = 0;
    _range_mod.bottom = 0;
  }

  range_rect_t Panel_SSD1677_4Gray::_full_range(void) const
  {
    range_rect_t result;
    result.left = 0;
    result.top = 0;
    result.right = _cfg.panel_width - 1;
    result.bottom = _cfg.panel_height - 1;
    return result;
  }

  void Panel_SSD1677_4Gray::_send_gray_lut(const uint8_t* lut)
  {
    send_lut(_bus, lut);
  }

  void Panel_SSD1677_4Gray::_remember_displayed(const uint8_t* lsb,
                                                const uint8_t* msb)
  {
    if (!_displayed_buf) { return; }
    memcpy(_displayed_buf, lsb, _buf_x1_len);
    memcpy(_displayed_buf + _buf_x1_len, msb, _buf_x1_len);
    _displayed_valid = true;
  }

  static uint8_t dirty_byte_mask(int32_t byte_index,
                                 const range_rect_t& dirty)
  {
    uint8_t mask = 0xFF;
    if (byte_index == (dirty.left >> 3))
    {
      mask &= uint8_t(0xFFu >> (dirty.left & 7));
    }
    if (byte_index == (dirty.right >> 3))
    {
      mask &= uint8_t(0xFFu << (7 - (dirty.right & 7)));
    }
    return mask;
  }

  void Panel_SSD1677_4Gray::_send_transition_planes(
      const uint8_t* new_msb,
      const range_rect_t& dirty)
  {
    const auto full = _full_range();
    const uint32_t row_bytes = ((_cfg.panel_width + 7) & ~7) >> 3;
    const uint8_t* old_lsb = _displayed_buf;
    const uint8_t* old_msb = _displayed_buf + _buf_x1_len;
    uint8_t row[128];

    // New levels are thresholded by new_msb, while an old middle gray must be
    // driven all the way to the requested black/white endpoint. The group
    // bits can still be formed bytewise without decoding individual pixels:
    //   BW  = new_msb
    //   RED = new_msb ? (old_lsb & old_msb) : (old_lsb | old_msb)
    const bool full_dirty = dirty.left == full.left
                         && dirty.top == full.top
                         && dirty.right == full.right
                         && dirty.bottom == full.bottom;
    if (full_dirty)
    {
      _send_plane(CMD_WRITE_RAM_BW, new_msb, full);
    }
    else
    {
      _set_ram_area(full.left, full.top, full.right + 1, full.bottom + 1);
      _bus->writeCommand(CMD_WRITE_RAM_BW, 8);
      const int32_t first_byte = dirty.left >> 3;
      const int32_t last_byte = dirty.right >> 3;
      for (int32_t y = 0; y < (int32_t)_cfg.panel_height; ++y)
      {
        const size_t row_offset = size_t(y) * row_bytes;
        memcpy(row, old_msb + row_offset, row_bytes);
        if (y >= dirty.top && y <= dirty.bottom)
        {
          for (int32_t b = first_byte; b <= last_byte; ++b)
          {
            const uint8_t mask = dirty_byte_mask(b, dirty);
            row[b] = (row[b] & uint8_t(~mask))
                   | (new_msb[row_offset + b] & mask);
          }
        }
        _bus->writeBytes(row, row_bytes, true, false);
      }
    }

    _set_ram_area(full.left, full.top, full.right + 1, full.bottom + 1);
    _bus->writeCommand(CMD_WRITE_RAM_RED, 8);
    const int32_t first_byte = dirty.left >> 3;
    const int32_t last_byte = dirty.right >> 3;
    for (int32_t y = 0; y < (int32_t)_cfg.panel_height; ++y)
    {
      const size_t row_offset = size_t(y) * row_bytes;
      memcpy(row, old_msb + row_offset, row_bytes);
      if (y >= dirty.top && y <= dirty.bottom)
      {
        for (int32_t b = first_byte; b <= last_byte; ++b)
        {
          const size_t index = row_offset + b;
          const uint8_t mask = dirty_byte_mask(b, dirty);
          const uint8_t new_side = new_msb[index];
          const uint8_t red = (new_side & (old_lsb[index] & old_msb[index]))
                            | (uint8_t(~new_side)
                               & (old_lsb[index] | old_msb[index]));
          row[b] = (row[b] & uint8_t(~mask)) | (red & mask);
        }
      }
      _bus->writeBytes(row, row_bytes, true, false);
    }
  }

  bool Panel_SSD1677_4Gray::_activate(uint8_t ctrl1, uint8_t ctrl2,
      bool powers_down, bool mode2_activation, bool enforce_refresh_minimum)
  {
    _bus->writeCommand(CMD_DISP_UPDATE_CTRL1, 8);
    _bus->writeData(ctrl1, 8);
    if (!_screen_on) { ctrl2 |= 0xC0; }
    _bus->writeCommand(CMD_DISP_UPDATE_CTRL2, 8);
    _bus->writeData(ctrl2, 8);
    _bus->writeCommand(CMD_MASTER_ACTIVATION, 8);
    _send_msec = millis();
    if (!_wait_busy(10000, enforce_refresh_minimum))
    {
      _invalidate_gray_state();
      return false;
    }
    _screen_on = !powers_down;
    if (mode2_activation)
    {
      if (_mode2_face_known) { _mode2_face_odd = !_mode2_face_odd; }
    }
    else
    {
      _mode2_face_odd = false;
      _mode2_face_known = true;
    }
    return true;
  }

  bool Panel_SSD1677_4Gray::_reset_controller_and_face(void)
  {
    _bus->writeCommand(0x12, 8);
    _send_msec = millis();
    if (!_wait_busy(5000)) { return false; }
    for (uint8_t i = 0; auto cmds = getInitCommands(i); ++i)
    {
      if (!_wait_busy(5000)) { return false; }
      command_list(cmds);
    }
    const auto full = _full_range();
    _set_ram_area(full.left, full.top, full.right + 1, full.bottom + 1);
    _bus->writeCommand(0x46, 8);
    _bus->writeData(0xF7, 8);
    if (!_wait_busy(5000)) { return false; }
    _bus->writeCommand(0x47, 8);
    _bus->writeData(0xF7, 8);
    if (!_wait_busy(5000)) { return false; }
    _screen_on = false;
    _mode2_face_odd = false;
    _mode2_face_known = true;
    return true;
  }

  bool Panel_SSD1677_4Gray::_ensure_known_face(void)
  {
    if (_mode2_face_known || _reset_controller_and_face()) { return true; }
    _invalidate_gray_state();
    return false;
  }

  bool Panel_SSD1677_4Gray::_refresh_mode1_absolute(
      const uint8_t* lsb, const uint8_t* msb, const uint8_t* lut)
  {
    // Mode 1 needs a known RAM face to map the two middle gray levels.
    if (!_ensure_known_face())
    {
      return false;
    }

    // The ping-pong face exchanges the two middle codes in Mode 1. Swap the
    // plane destinations on the odd face; 00 black and 11 white are symmetric.
    const auto full = _full_range();
    const uint8_t* bw = _mode2_face_odd ? msb : lsb;
    const uint8_t* red = _mode2_face_odd ? lsb : msb;
    _send_plane(CMD_WRITE_RAM_BW, bw, full, true);
    _send_plane(CMD_WRITE_RAM_RED, red, full, true);
    _send_gray_lut(lut);
    // Wait on the BUSY line itself, without the 400 ms refresh-minimum floor.
    if (!_activate(CTRL1_NORMAL, 0x07, true, false, false)) { return false; }
    _optical_state = optical_state_t::gray4;
    return true;
  }

  bool Panel_SSD1677_4Gray::_refresh_mode2_absolute(
      const uint8_t* lsb, const uint8_t* msb, const uint8_t* lut)
  {
    if (!lut || !_ensure_known_face()) { return false; }

    // Mode 2 consumes the group bits written to 0x24/0x26 directly on every
    // activation. Both RAMs are overwritten, so their command mapping stays
    // fixed even though the controller advances its ping-pong face. Swapping
    // these destinations on odd faces would exchange the two middle grays.
    const auto full = _full_range();
    _send_plane(CMD_WRITE_RAM_BW, lsb, full, true);
    _send_plane(CMD_WRITE_RAM_RED, msb, full, true);
    _send_gray_lut(lut);
    if (!_activate(CTRL1_NORMAL, 0x0C, false, true, false)) { return false; }

    _optical_state = optical_state_t::gray4;
    return true;
  }

  void Panel_SSD1677_4Gray::_remember_mono_dirty(
      const uint8_t* msb, const range_rect_t& dirty)
  {
    const uint32_t row_bytes = ((_cfg.panel_width + 7) & ~7) >> 3;
    uint8_t* old_lsb = _displayed_buf;
    uint8_t* old_msb = _displayed_buf + _buf_x1_len;
    const int32_t first_byte = dirty.left >> 3;
    const int32_t last_byte = dirty.right >> 3;
    for (int32_t y = dirty.top; y <= dirty.bottom; ++y)
    {
      const size_t row_offset = size_t(y) * row_bytes;
      for (int32_t b = first_byte; b <= last_byte; ++b)
      {
        const size_t index = row_offset + b;
        const uint8_t mask = dirty_byte_mask(b, dirty);
        const uint8_t mono = msb[index] & mask;
        old_lsb[index] = (old_lsb[index] & uint8_t(~mask)) | mono;
        old_msb[index] = (old_msb[index] & uint8_t(~mask)) | mono;
      }
    }
  }

  bool Panel_SSD1677_4Gray::_refresh_mode2_fastest(
      const uint8_t* lsb, const uint8_t* msb,
      const range_rect_t& current_dirty)
  {
    if (!_displayed_valid)
    {
      // Establish a complete optical/history baseline without leaving Mode 2.
      return _refresh_mode2_absolute(lsb, msb, lut_fast);
    }
    if (!_ensure_known_face()) { return false; }

    _send_transition_planes(msb, current_dirty);
    _send_gray_lut(lut_fastest);
    if (!_activate(CTRL1_NORMAL, 0x0C, false, true, false)) { return false; }

    _optical_state = optical_state_t::mono_synchronized;
    _remember_mono_dirty(msb, current_dirty);
    return true;
  }

  void Panel_SSD1677_4Gray::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      uint_fast16_t xs = x, ys = y, xe = x + w - 1, ye = y + h - 1;
      _rotate_pos(xs, ys, xe, ye);
      _range_mod.left   = std::min<int32_t>(_range_mod.left  , std::min(xs, xe));
      _range_mod.right  = std::max<int32_t>(_range_mod.right , std::max(xs, xe));
      _range_mod.top    = std::min<int32_t>(_range_mod.top   , std::min(ys, ye));
      _range_mod.bottom = std::max<int32_t>(_range_mod.bottom, std::max(ys, ye));
    }
    if (_range_mod.empty()) { return; }

    auto mode = getEpdMode();
    const auto current_dirty = _range_mod;

    const uint8_t* planeL = _buf;
    const uint8_t* planeM = &_buf[_buf_x1_len];

    startWrite();

    bool success = false;
    switch (mode)
    {
    case epd_mode_t::epd_quality:
      success = _refresh_mode1_absolute(planeL, planeM, lut_quality);
      break;
    case epd_mode_t::epd_text:
      success = _refresh_mode1_absolute(planeL, planeM, lut_text);
      break;
    case epd_mode_t::epd_fast:
      success = _refresh_mode2_absolute(planeL, planeM, lut_fast);
      break;
    case epd_mode_t::epd_fastest:
      success = _refresh_mode2_fastest(planeL, planeM, current_dirty);
      break;
    default:
      success = false;
      break;
    }

    if (success)
    {
      // Every absolute refresh (including Fastest's fallback) leaves the full
      // 2-bit planes on glass, so record them as the differential baseline.
      if (_optical_state == optical_state_t::gray4)
      {
        _remember_displayed(planeL, planeM);
      }
      _initialize_seq = false;
      _last_epd_mode = mode;
      _range_old = current_dirty;
      _clear_modified_range();
    }

    endWrite();
  }

//----------------------------------------------------------------------------
 }
}
