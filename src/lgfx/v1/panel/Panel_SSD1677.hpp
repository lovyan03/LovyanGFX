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
#pragma once

#include "lgfx/v1/panel/Panel_HasBuffer.hpp"
#include "lgfx/v1/misc/range.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  /*
    SSD1677 (GDEQ0426T82 / 800x480) E-Paper panel driver.

    Buffer / coordinate model (EPD native):
      - Panel native orientation = landscape 800(X=source) x 480(Y=gate).
      - 8 pixels per byte are packed along X (source) direction, matching the
        controller's source-shift scan order. byte = 8 consecutive X pixels.
      - Buffer rows are indexed by Y (gate). row_bytes = (panel_width+7)/8 = 100.
        byte_idx = y * row_bytes + (x >> 3) ; bit = 0x80 >> (x & 7).
      - Portrait usage is provided via setRotation (handled by _rotate_pos).

    Grayscale (4-level) model:
      - _draw_pixel stores an abstract gray level v(0=black..3=white) split into
        two bit planes: planeL = v&1 (-> BW RAM 0x24), planeM = v&2 (-> RED RAM 0x26).
      - The hardware RAM encoding is generated at send time per epd_mode:
          quality/text : Mode 1 absolute four-gray update
          fast         : Mode 2 absolute four-gray update
          fastest      : Mode 2 monochrome differential update vs history
  */
  struct Panel_SSD1677 : public Panel_HasBuffer
  {
    Panel_SSD1677(void);
    virtual ~Panel_SSD1677(void);

    bool init(bool use_reset) override;

    color_depth_t setColorDepth(color_depth_t depth) override;

    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void waitDisplay(void) override;
    bool displayBusy(void) override;
    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
    void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override;

    uint32_t readCommand(uint_fast16_t, uint_fast8_t, uint_fast8_t) override { return 0; }
    uint32_t readData(uint_fast8_t, uint_fast8_t) override { return 0; }

    void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override;

  protected:

    // Minimum wait used by the base monochrome refresh path.
    static constexpr unsigned long _refresh_msec = 400;

    range_rect_t _range_old;
    unsigned long _send_msec = 0;
    epd_mode_t _last_epd_mode;
    uint32_t _buf_x1_len;       // one plane length in bytes
    bool _initialize_seq;
    bool _screen_on = false;

    size_t _get_buffer_length(void) const override;
    uint32_t _get_plane_length(void) const;

    bool _wait_busy(uint32_t timeout = 4096, bool enforce_refresh_minimum = true);
    void _draw_pixel(uint_fast16_t x, uint_fast16_t y, uint32_t value);
    uint8_t _read_pixel(uint_fast16_t x, uint_fast16_t y);
    void _update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye);

    // Set the controller RAM window (native coords; Y is gate, reversed in HW).
    void _set_ram_area(int32_t x, int32_t y, int32_t w, int32_t h);
    // Send one bit-plane region (native coords) to RAM command `cmd`.
    void _send_plane(uint32_t cmd, const uint8_t* plane, const range_rect_t& range, bool invert = false);

    void _power_on(void);
    void _after_wake(void);

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      // SSD1677 power-up sequence for the native 800x480 panel.
      // 0x01 configures 480 gates; RAM setup and power-on follow in _after_wake.
      static constexpr uint8_t list0[] = {
          0x12, 0 + CMD_INIT_DELAY, 10,             // SW Reset + 10 msec
          0x18, 1, 0x80,                            // Temp sensor: internal
          0x0C, 5, 0xAE, 0xC7, 0xC3, 0xC0, 0x40,    // Booster soft start
          0x01, 3, (480-1) & 0xFF, ((480-1) >> 8) & 0xFF, 0x02, // Driver output: 480 gates, SM=1/TB=0
          0x3C, 1, 0x01,                            // Border waveform
          0xFF, 0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

  struct Panel_SSD1677_4Gray : public Panel_SSD1677
  {
    ~Panel_SSD1677_4Gray(void) override;
    bool init(bool use_reset) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;
    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

  protected:
    enum class optical_state_t : uint8_t
    {
      unknown,
      gray4,
      mono_synchronized,
    };

    optical_state_t _optical_state = optical_state_t::unknown;
    bool _mode2_face_odd = false;
    bool _mode2_face_known = false;
    uint8_t* _displayed_buf = nullptr;
    bool _displayed_valid = false;

    void _invalidate_gray_state(void);
    void _clear_modified_range(void);
    range_rect_t _full_range(void) const;
    void _send_gray_lut(const uint8_t* lut);
    void _remember_displayed(const uint8_t* lsb, const uint8_t* msb);
    void _send_transition_planes(const uint8_t* new_msb,
                                 const range_rect_t& dirty);
    void _remember_mono_dirty(const uint8_t* msb, const range_rect_t& dirty);
    bool _activate(uint8_t ctrl1, uint8_t ctrl2, bool powers_down,
                   bool mode2_activation, bool enforce_refresh_minimum = true);
    bool _reset_controller_and_face(void);
    bool _ensure_known_face(void);
    bool _refresh_mode1_absolute(const uint8_t* lsb, const uint8_t* msb,
                                 const uint8_t* lut);
    bool _refresh_mode2_absolute(const uint8_t* lsb, const uint8_t* msb,
                                 const uint8_t* lut);
    bool _refresh_mode2_fastest(const uint8_t* lsb, const uint8_t* msb,
                                const range_rect_t& dirty);
  };

//----------------------------------------------------------------------------
 }
}
