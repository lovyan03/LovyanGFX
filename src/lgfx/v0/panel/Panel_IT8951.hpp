#ifndef LGFX_PANEL_IT8951_HPP_
#define LGFX_PANEL_IT8951_HPP_

#include "PanelCommon.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  struct Panel_IT8951 : public PanelCommon
  {
    Panel_IT8951()
    {
      memory_width  = 2048;
      memory_height = 2048;
      panel_width  = 960;
      panel_height = 540;

      freq_write = 16000000;
      freq_read  = 16000000;
      freq_fill  = 16000000;

      spi_3wire = false;
      write_depth = rgb888_3Byte;
      read_depth = rgb888_3Byte;
      len_dummy_read_pixel = 8;
      len_dummy_read_rddid = 0;

      //cmd_caset  = 1;
      //cmd_raset  = 2;
      //cmd_ramwr  = 0x13;
      //cmd_ramwr  = 0x00;
      //cmd_ramrd  = 4;
      //cmd_rddid  = 0x70;

      //fp_begin       = beginTransaction;
      //fp_end         = endTransaction;
      fp_display     = display;
      fp_displayBusy = displayBusy;
      fp_waitDisplay = waitDisplay;
      fp_fillRect    = fillRect;
      fp_pushImage   = pushImage;
      fp_pushBlock   = pushBlock;
      fp_writePixels = writePixels;
      fp_readRect    = readRect;
    }

    bool isEPD(void) const override { return true; }

  protected:

    void post_init(LGFX_Device* gfx, bool use_reset) override;

    const uint8_t* getWindowCommands1(uint8_t* buf, uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override
    {
      (void)buf;
      _xpos = xs;
      _xs = xs;
      _ypos = ys;
      _ys = ys;
      _xe = xe;
      _ye = ye;
      return nullptr;
    }
    const uint8_t* getWindowCommands2(uint8_t*, uint_fast16_t, uint_fast16_t, uint_fast16_t, uint_fast16_t) override
    {
      return nullptr;
    }

    const uint8_t* getSleepInCommands(uint8_t* buf) override
    {
      reinterpret_cast<uint16_t*>(buf)[0] = 0xFFFF;
      return buf;
    }

    const uint8_t* getSleepOutCommands(uint8_t* buf) override
    {
      reinterpret_cast<uint16_t*>(buf)[0] = 0xFFFF;
      return buf;
    }

    const uint8_t* getPowerSaveOnCommands(uint8_t* buf) override
    {
      reinterpret_cast<uint16_t*>(buf)[0] = 0xFFFF;
      return buf;
    }

    const uint8_t* getPowerSaveOffCommands(uint8_t* buf) override
    {
      reinterpret_cast<uint16_t*>(buf)[0] = 0xFFFF;
      return buf;
    }

    const uint8_t* getInvertDisplayCommands(uint8_t* buf, bool invert) override
    {
      (void)invert;
      reinterpret_cast<uint16_t*>(buf)[0] = 0xFFFF;
      return buf;
    }

    const uint8_t* getRotationCommands(uint8_t* buf, int_fast8_t r) override
    {
      reinterpret_cast<uint16_t*>(buf)[0] = 0xFFFF;
      PanelCommon::getRotationCommands(buf, r);
      if (_internal_rotation & 1) // IT8951の回転方向は左回りなので右回りになるよう変更
      {
        _internal_rotation ^= 2;
      }
      return buf;
    }

    const uint8_t* getColorDepthCommands(uint8_t* buf, color_depth_t depth) override
    {
      (void)depth;
      reinterpret_cast<uint16_t*>(buf)[0] = 0xFFFF;
      return buf;
    }

    color_depth_t getAdjustBpp(color_depth_t bpp) const override { (void)bpp; return rgb888_3Byte; }

    const uint8_t* getInitCommands(uint8_t) const override { return nullptr; }

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

//  static constexpr int8_t Bayer[16] = { 0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5 };
//  static constexpr int8_t Bayer[16] = { -7, 1, -5, 3, 5, -3, 7, -1, -4, 4, -6, 2, 8, 0, 6, -2 };
//  static constexpr int8_t Bayer[16] = { -15, 1, -11, 5, 9, -7, 13, -3, -9, 7, -13, 3, 15, -1, 11, -5 };  
//  static constexpr int8_t Bayer[16] = {-22, 2, -16, 8, 14, -10, 20, -4, -13, 11, -19, 5, 23, -1, 17, -7};
  static constexpr int8_t Bayer[16] = {-30, 2, -22, 10, 18, -14, 26, -6, -18, 14, -26, 6, 30, -2, 22, -10};
//    static constexpr int8_t Bayer[16] = {-45, 3, -33, 15, 27, -21, 39, -9, -27, 21, -39, 9, 45, -3, 33, -15};

    range_rect_t _range_new;
    range_rect_t _range_old;

    uint_fast16_t _xpos = 0;
    uint_fast16_t _ypos = 0;
    bool _fastmode = true;

    static void display(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h);
    static bool displayBusy(PanelCommon* panel, LGFX_Device* gfx);
    static void waitDisplay(PanelCommon* panel, LGFX_Device* gfx);
    static void fillRect(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t rawcolor);
    static void pushImage(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param);
    static void pushBlock(PanelCommon* panel, LGFX_Device* gfx, int32_t length, uint32_t rawcolor);
    static void writePixels(PanelCommon* panel, LGFX_Device* gfx, int32_t len, pixelcopy_t* param);
    static void readRect(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param);

    bool WaitBusy(LGFX_Device* gfx, uint32_t timeout = 1000);
    bool WriteCommand(LGFX_Device* gfx, uint16_t cmd);
    bool WriteWord(LGFX_Device* gfx, uint16_t data);
    bool WriteArgs(LGFX_Device* gfx, uint16_t cmd, uint16_t *args, int32_t length);
    bool WriteReg(LGFX_Device* gfx, uint16_t addr, uint16_t data);
    bool ReadWords(LGFX_Device* gfx, uint16_t *buf, uint32_t length);
    bool CheckAFSR(LGFX_Device* gfx);
    bool SetTargetMemoryAddr(LGFX_Device* gfx, uint32_t tar_addr);
    bool SetArea(LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h);
    bool UpdateRawArea(LGFX_Device* gfx, epd_update_mode_t mode);
    bool ReadRawLine(LGFX_Device* gfx, int32_t raw_x, int32_t raw_y, int32_t len, uint16_t* buf);
  };

//----------------------------------------------------------------------------
 }
}

#endif
