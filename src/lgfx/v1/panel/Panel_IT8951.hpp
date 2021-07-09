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

#include "Panel_Device.hpp"
#include "../misc/range.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_IT8951 : public Panel_Device
  {
    Panel_IT8951(void);
    virtual ~Panel_IT8951(void);

    void beginTransaction(void) override;
    void endTransaction(void) override;
    bool init(bool use_reset) override;

    color_depth_t setColorDepth(color_depth_t depth) override;

    void setInvert(bool invert) override;
    void setRotation(std::uint_fast8_t r) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void waitDisplay(void) override;
    bool displayBusy(void) override;
    void display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h) override;

    void writeBlock(std::uint32_t rawcolor, std::uint32_t len) override;
    void setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) override;
    void drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor) override;
    void writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor) override;
    void writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writePixels(pixelcopy_t* param, std::uint32_t len, bool use_dma) override;

    std::uint32_t readCommand(std::uint_fast8_t, std::uint_fast8_t, std::uint_fast8_t) override { return 0; }
    std::uint32_t readData(std::uint_fast8_t, std::uint_fast8_t) override { return 0; }

    void readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param) override;

  private:

    enum epd_update_mode_t
    {                           //   Ghosting  Update Time  Usage
      UPDATE_MODE_INIT    = 0,  // * N/A       2000ms       Display initialization, 
      UPDATE_MODE_DU      = 1,  //   Low       260ms        Monochrome menu, text input, and touch screen input 
      UPDATE_MODE_GC16    = 2,  // * Very Low  450ms        High quality images
      UPDATE_MODE_GL16    = 3,  // * Medium    450ms        Text with white background 
      UPDATE_MODE_GLR16   = 4,  //   Low       450ms        Text with white background
      UPDATE_MODE_GLD16   = 5,  //   Low       450ms        Text and graphics with white background 
      UPDATE_MODE_DU4     = 6,  // * Medium    120ms        Fast page flipping at reduced contrast
      UPDATE_MODE_A2      = 7,  //   Medium    290ms        Anti-aliased text in menus / touch and screen input 
      UPDATE_MODE_NONE    = 8
    };        // The ones marked with * are more commonly used

    range_rect_t _range_new;
    range_rect_t _range_old;

    std::uint_fast16_t _xpos = 0;
    std::uint_fast16_t _ypos = 0;
    std::uint_fast8_t _it8951_rotation = 0;
    bool _in_transaction = false;

    bool _wait_busy( std::uint32_t timeout = 1000);
    bool _write_command( std::uint16_t cmd);
    bool _write_word( std::uint16_t data);
    bool _write_args( std::uint16_t cmd, std::uint16_t *args, std::int32_t length);
    bool _write_reg( std::uint16_t addr, std::uint16_t data);
    bool _read_words( std::uint16_t *buf, std::uint32_t length);
    bool _check_afsr( void );
    bool _set_target_memory_addr( std::uint32_t tar_addr);
    bool _set_area( std::uint32_t x, std::uint32_t y, std::uint32_t w, std::uint32_t h);
    bool _update_raw_area( epd_update_mode_t mode);
    bool _read_raw_line( std::int32_t raw_x, std::int32_t raw_y, std::int32_t len, std::uint16_t* buf);

    fastread_dir_t get_fastread_dir(void) const override { return _it8951_rotation & 1 ? fastread_vertical : fastread_horizontal; }
  };

//----------------------------------------------------------------------------
 }
}
