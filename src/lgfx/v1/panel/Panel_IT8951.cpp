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
#include "Panel_IT8951.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"

#include <Arduino.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr std::int8_t Bayer[16] = {-30, 2, -22, 10, 18, -14, 26, -6, -18, 14, -26, 6, 30, -2, 22, -10};

  static constexpr std::uint32_t _tar_memaddr = 0x001236E0;

//Built in I80 Command Code
  static constexpr std::uint32_t IT8951_TCON_SYS_RUN         = 0x0001;
  static constexpr std::uint32_t IT8951_TCON_STANDBY         = 0x0002;
  static constexpr std::uint32_t IT8951_TCON_SLEEP           = 0x0003;
  static constexpr std::uint32_t IT8951_TCON_REG_RD          = 0x0010;
  static constexpr std::uint32_t IT8951_TCON_REG_WR          = 0x0011;
  static constexpr std::uint32_t IT8951_TCON_MEM_BST_RD_T    = 0x0012;
  static constexpr std::uint32_t IT8951_TCON_MEM_BST_RD_S    = 0x0013;
  static constexpr std::uint32_t IT8951_TCON_MEM_BST_WR      = 0x0014;
  static constexpr std::uint32_t IT8951_TCON_MEM_BST_END     = 0x0015;
  static constexpr std::uint32_t IT8951_TCON_LD_IMG          = 0x0020;
  static constexpr std::uint32_t IT8951_TCON_LD_IMG_AREA     = 0x0021;
  static constexpr std::uint32_t IT8951_TCON_LD_IMG_END      = 0x0022;

//I80 User defined command code
  static constexpr std::uint32_t IT8951_I80_CMD_DPY_AREA     = 0x0034;
  static constexpr std::uint32_t IT8951_I80_CMD_GET_DEV_INFO = 0x0302;
  static constexpr std::uint32_t IT8951_I80_CMD_DPY_BUF_AREA = 0x0037;
  static constexpr std::uint32_t IT8951_I80_CMD_VCOM         = 0x0039;
  static constexpr std::uint32_t IT8951_I80_CMD_FILLRECT     = 0x003A;

  static constexpr std::uint32_t IT8951_ROTATE_0    = 0;
  static constexpr std::uint32_t IT8951_ROTATE_90   = 1;
  static constexpr std::uint32_t IT8951_ROTATE_180  = 2;
  static constexpr std::uint32_t IT8951_ROTATE_270  = 3;

  static constexpr std::uint32_t IT8951_2BPP           = 0;
  static constexpr std::uint32_t IT8951_3BPP           = 1;
  static constexpr std::uint32_t IT8951_4BPP           = 2;
  static constexpr std::uint32_t IT8951_8BPP           = 3;
  static constexpr std::uint32_t IT8951_LDIMG_B_ENDIAN = 1;

/*-----------------------------------------------------------------------
IT8951 Registers defines
------------------------------------------------------------------------*/
//Register Base Address
  static constexpr std::uint32_t IT8951_DISPLAY_REG_BASE    = 0x1000; //Register RW access

//Base Address of Basic LUT Registers
  static constexpr std::uint32_t IT8951_LUT0EWHR   = (IT8951_DISPLAY_REG_BASE + 0x0000); //LUT0 Engine Width Height Reg
  static constexpr std::uint32_t IT8951_LUT0XYR    = (IT8951_DISPLAY_REG_BASE + 0x0040); //LUT0 XY Reg
  static constexpr std::uint32_t IT8951_LUT0BADDR  = (IT8951_DISPLAY_REG_BASE + 0x0080); //LUT0 Base Address Reg
  static constexpr std::uint32_t IT8951_LUT0MFN    = (IT8951_DISPLAY_REG_BASE + 0x00C0); //LUT0 Mode and Frame number Reg
  static constexpr std::uint32_t IT8951_LUT01AF    = (IT8951_DISPLAY_REG_BASE + 0x0114); //LUT0 and LUT1 Active Flag Reg

//Update Parameter Setting Register
  static constexpr std::uint32_t IT8951_UP0SR      = (IT8951_DISPLAY_REG_BASE + 0x134); //Update Parameter0 Setting Reg
  static constexpr std::uint32_t IT8951_UP1SR      = (IT8951_DISPLAY_REG_BASE + 0x138); //Update Parameter1 Setting Reg
  static constexpr std::uint32_t IT8951_LUT0ABFRV  = (IT8951_DISPLAY_REG_BASE + 0x13C); //LUT0 Alpha blend and Fill rectangle Value
  static constexpr std::uint32_t IT8951_UPBBADDR   = (IT8951_DISPLAY_REG_BASE + 0x17C); //Update Buffer Base Address
  static constexpr std::uint32_t IT8951_LUT0IMXY   = (IT8951_DISPLAY_REG_BASE + 0x180); //LUT0 Image buffer X/Y offset Reg
  static constexpr std::uint32_t IT8951_LUTAFSR    = (IT8951_DISPLAY_REG_BASE + 0x224); //LUT Status Reg (status of All LUT Engines)
  static constexpr std::uint32_t IT8951_BGVR       = (IT8951_DISPLAY_REG_BASE + 0x250); //Bitmap (1bpp) image color table

//System Registers
  static constexpr std::uint32_t IT8951_SYS_REG_BASE        = 0x0000;

//Address of System Registers
  static constexpr std::uint32_t IT8951_I80CPCR             = (IT8951_SYS_REG_BASE + 0x04);

//Memory Converter Registers
  static constexpr std::uint32_t IT8951_MCSR_BASE_ADDR      = 0x0200;
  static constexpr std::uint32_t IT8951_MCSR                = (IT8951_MCSR_BASE_ADDR + 0x0000);
  static constexpr std::uint32_t IT8951_LISAR               = (IT8951_MCSR_BASE_ADDR + 0x0008);


  Panel_IT8951::Panel_IT8951(void)
  {
    _cfg.memory_width  = 2048;
    _cfg.memory_height = 2048;
    _cfg.panel_width  = 960;
    _cfg.panel_height = 540;
    _cfg.dummy_read_bits = 0;
    _epd_mode = epd_mode_t::epd_quality;
    _auto_display = true;
  }

  Panel_IT8951::~Panel_IT8951(void)
  {
  }

  color_depth_t Panel_IT8951::setColorDepth(color_depth_t depth)
  {
    _write_bits = 24;
    _read_bits = 24;
    _write_depth = color_depth_t::rgb888_3Byte;
    _read_depth = color_depth_t::rgb888_3Byte;
    return color_depth_t::rgb888_3Byte;
  }

  void Panel_IT8951::init(bool use_reset)
  {
    Panel_Device::init(use_reset);
    pinMode(_cfg.pin_busy, pin_mode_t::input);

    _wait_busy();

    startWrite(true);

    setInvert(_invert);
    setRotation(_cfg.rotation);

    _write_command(IT8951_TCON_SYS_RUN);
    _write_reg(IT8951_I80CPCR, 0x0001); //enable pack write

    _write_command(0x0039); //tcon vcom set command
    _write_word(0x0001);
    _write_word(2300); // 0x08fc

    _set_target_memory_addr(_tar_memaddr);

    _range_new.top = INT16_MAX;
    _range_new.left = INT16_MAX;
    _range_new.right = 0;
    _range_new.bottom = 0;
//    gfx->setBaseColor(TFT_WHITE);
//    gfx->setTextColor(TFT_BLACK, TFT_WHITE);

    if (use_reset) {
      auto mode = _epd_mode;
      writeFillRectPreclipped(0, 0, width(), height(), 0);
      display(0, 0, 0, 0);
      writeFillRectPreclipped(0, 0, width(), height(), ~0u);
      _epd_mode = epd_mode_t::epd_fastest;
      display(0, 0, 0, 0);
      _epd_mode = mode;
      _check_afsr();
    }

    endWrite();
  }

  void Panel_IT8951::beginTransaction(void)
  {
    if (_in_transaction) return;
    _in_transaction = true;
    _bus->beginTransaction();
  }

  void Panel_IT8951::endTransaction(void)
  {
    if (!_in_transaction) return;
    _in_transaction = false;
    _bus->endTransaction();
    cs_control(true);
  }

  bool Panel_IT8951::_wait_busy(std::uint32_t timeout)
  {
    _bus->wait();
    cs_control(true);
    if (_cfg.pin_busy >= 0 && !lgfx::gpio_in(_cfg.pin_busy))
    {
      auto time = millis();
      do
      {
        if (millis() - time > timeout)
        {
          return false;
        }
      } while (!lgfx::gpio_in(_cfg.pin_busy));
    }
    cs_control(false);
    return true;
  }

  void Panel_IT8951::waitDisplay(void)
  {
    _check_afsr();
  }

  bool Panel_IT8951::displayBusy(void)
  {
    std::uint16_t infobuf[1] = { 1 };
    if (_write_command(IT8951_TCON_REG_RD)
      && _write_word(IT8951_LUTAFSR)
      && _read_words(infobuf, 1))
    {
      return 0 != infobuf[0];
    }
    cs_control(true);
    return true;
  }

  bool Panel_IT8951::_check_afsr(void)
  {
    std::uint32_t start_time = millis();
    std::uint16_t infobuf[1] = { 1 };
    do
    {
      if (_write_command(IT8951_TCON_REG_RD)
       && _write_word(IT8951_LUTAFSR)
       && _read_words(infobuf, 1)
       && infobuf[0] == 0)
      {
        break;
      }
      delay(1);
    } while (millis() - start_time < 3000);

    cs_control(true);
    return infobuf[0] != 0;
  }

  bool Panel_IT8951::_write_command(std::uint16_t cmd)
  {
    std::uint32_t buf = __builtin_bswap16(0x6000) | __builtin_bswap16(cmd) << 16;
    if (!_wait_busy()) return false;
    _bus->writeData(buf, 32);
    return true;
  }

  bool Panel_IT8951::_write_word(std::uint16_t data)
  {
    std::uint32_t buf = __builtin_bswap16(data) << 16;
    if (!_wait_busy()) return false;
    _bus->writeData(buf, 32);
    return true;
  }

  bool Panel_IT8951::_write_args(std::uint16_t cmd, std::uint16_t *args, std::int32_t length)
  {
    if (_write_command(cmd)
     && _wait_busy())
    {
      _bus->writeData(0, 16);
      std::int32_t i = 0;
      do
      {
        std::uint32_t buf = __builtin_bswap16(args[i]);
        _bus->wait();
        while (!lgfx::gpio_in(_cfg.pin_busy));
        _bus->writeData(buf, 16);
      } while ( ++i < length );
      return true;
    }
    return false;
  }

  bool Panel_IT8951::_read_words(std::uint16_t *buf, std::uint32_t length)
  {
    if (!_wait_busy()) return false;
    _bus->writeData(__builtin_bswap16(0x1000), 16 + 16); // +16 dummy read
//    _bus->writeData(0, 16); // dummy read
    _bus->beginRead();
    _bus->readBytes(reinterpret_cast<std::uint8_t*>(buf), length << 1);
    _bus->endRead();
    cs_control(true);
    for (size_t i = 0; i < length; i++)
    {
      buf[i] = __builtin_bswap16(buf[i]);
    }
    return true;
  }

  bool Panel_IT8951::_write_reg(std::uint16_t addr, std::uint16_t data)
  {
    return _write_command(0x0011)
        && _write_word(addr)
        && _write_word(data);
  }

  bool Panel_IT8951::_set_target_memory_addr(std::uint32_t tar_addr)
  {
    return _write_reg(IT8951_LISAR + 2, tar_addr >> 16)
        && _write_reg(IT8951_LISAR    , tar_addr      );
  }

  bool Panel_IT8951::_set_area(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
  {
    std::int32_t rx, ry, rw, rh;
    switch(_internal_rotation & 3)
    {
    default:
    case IT8951_ROTATE_0:
      rx = x;
      ry = y;
      rw = w;
      rh = h;
      break;
    case IT8951_ROTATE_90:
      rx = y;
      ry = _cfg.panel_height - w - x;
      rw = h;
      rh = w;
      break;
    case IT8951_ROTATE_180:
      rx = _cfg.panel_width - w - x;
      ry = _cfg.panel_height - h - y;
      rw = w;
      rh = h;
      break;
    case IT8951_ROTATE_270:
      rx = _cfg.panel_width - h - y;
      ry = x;
      rw = h;
      rh = w;
      break;
    }

    _range_new.left   = std::min<std::int32_t>(_range_new.left  , rx         );
    _range_new.right  = std::max<std::int32_t>(_range_new.right , rx + rw - 1);
    _range_new.top    = std::min<std::int32_t>(_range_new.top   , ry         );
    _range_new.bottom = std::max<std::int32_t>(_range_new.bottom, ry + rh - 1);

    if (_epd_mode != epd_mode_t::epd_fastest
     && _range_old.horizon.intersectsWith(rx, rx + rw - 1)
     && _range_old.vertical.intersectsWith(ry, ry + rh - 1))
    {
      _check_afsr();
      _range_old.left = INT16_MAX;
      _range_old.top = INT16_MAX;
      _range_old.right = 0;
      _range_old.bottom = 0;
    }

    std::uint16_t params[5];
    params[0] = IT8951_LDIMG_B_ENDIAN << 8 | IT8951_4BPP << 4 | _internal_rotation;
    params[1] = x;
    params[2] = y;
    params[3] = w;
    params[4] = h;
    return _write_args(IT8951_TCON_LD_IMG_AREA, params, 5);
  }

  bool Panel_IT8951::_update_raw_area(epd_update_mode_t mode)
  {
    if (_range_new.empty()) return false;
    std::uint16_t params[7];
    params[0] = _range_new.left;
    params[1] = _range_new.top;
    params[2] = _range_new.right - _range_new.left + 1;
    params[3] = _range_new.bottom - _range_new.top + 1;
    params[4] = mode;
    params[5] = (std::uint16_t)_tar_memaddr;
    params[6] = (std::uint16_t)(_tar_memaddr >> 16);
    return _write_args(IT8951_I80_CMD_DPY_BUF_AREA, params, 7);
  }

  void Panel_IT8951::display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      if (_internal_rotation & 4)
      {
        y = height() - y - h;
      }
      _set_area(x, y, w, h);
    }
    if (_range_new.empty()) return;

    _range_old = _range_new;
    epd_update_mode_t mode;
    switch (_epd_mode)
    {
    case epd_mode_t::epd_fastest:  mode = UPDATE_MODE_DU4;  break;
    case epd_mode_t::epd_fast:     mode = UPDATE_MODE_DU;   break;
    case epd_mode_t::epd_text:     mode = UPDATE_MODE_GL16; break;
    default:                       mode = UPDATE_MODE_GC16; break;
    }

    _update_raw_area(mode);

    _range_new.top = INT16_MAX;
    _range_new.left = INT16_MAX;
    _range_new.right = 0;
    _range_new.bottom = 0;
  }

  void Panel_IT8951::setInvert(bool invert)
  {
    // unimplemented
    _invert = invert;
  }

  void Panel_IT8951::setSleep(bool flg_sleep)
  {
    _write_command(flg_sleep ? IT8951_TCON_SLEEP : IT8951_TCON_STANDBY);
  }

  void Panel_IT8951::setIdol(bool flg_partial)
  {
    _write_command(flg_partial ? IT8951_TCON_SLEEP : IT8951_TCON_STANDBY);
  }

  void Panel_IT8951::setRotation(std::uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));
    if (_internal_rotation & 1) // IT8951の回転方向は左回りなので右回りになるよう変更
    {
      _internal_rotation ^= 2;
    }

    _width  = (_internal_rotation & 1) ? _cfg.panel_height : _cfg.panel_width;
    _height = (_internal_rotation & 1) ? _cfg.panel_width : _cfg.panel_height;
  }

  void Panel_IT8951::setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye)
  {
    _xpos = xs;
    _ypos = ys;
    _xs = xs;
    _ys = ys;
    _xe = xe;
    _ye = ye;
  }

  void Panel_IT8951::drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor)
  {
    startWrite();
    writeFillRectPreclipped(x, y, 1, 1, rawcolor);
    endWrite();
  }

  void Panel_IT8951::writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor)
  {
    if (_internal_rotation & 4)
    {
      y = height() - y - h;
    }
    _set_area(x, y, w, h);
    bgr888_t color = rawcolor;
    std::int32_t sum = color.R8() + (color.G8() << 1) + color.B8();
    _wait_busy();
    _bus->writeData(0, 16);
    bool fast = _epd_mode == epd_mode_t::epd_fast || _epd_mode == epd_mode_t::epd_fastest;
    std::uint32_t wid = (((x + w + 3) >> 2) - (x >> 2));
    do
    {
      auto btbl = &Bayer[(y & 3) << 2];
      std::uint32_t value;
      if (fast)
      {
        value = (sum + btbl[0]*16 < 512 ? 0 : 0xF000)
              | (sum + btbl[1]*16 < 512 ? 0 : 0x0F00)
              | (sum + btbl[2]*16 < 512 ? 0 : 0x00F0)
              | (sum + btbl[3]*16 < 512 ? 0 : 0x000F);
      }
      else
      {
        value = std::min<std::int32_t>(15, (std::max<std::int32_t>(0, sum + btbl[0])) >> 6) << 12
              | std::min<std::int32_t>(15, (std::max<std::int32_t>(0, sum + btbl[1])) >> 6) <<  8
              | std::min<std::int32_t>(15, (std::max<std::int32_t>(0, sum + btbl[2])) >> 6) <<  4
              | std::min<std::int32_t>(15, (std::max<std::int32_t>(0, sum + btbl[3])) >> 6) ;
      }
      _bus->writeDataRepeat(__builtin_bswap16(value), 16, wid);
      ++y;
    } while (--h);
    _write_command(IT8951_TCON_LD_IMG_END);
  }

  void Panel_IT8951::writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    bgr888_t* readbuf = static_cast<bgr888_t*>(heap_alloc(w * sizeof(bgr888_t) + 3));
    std::uint16_t* writebuf = reinterpret_cast<std::uint16_t*>(readbuf);
    if (readbuf == nullptr) return;

    std::int32_t add_y = 1;
    bool flg_setarea = false;
    if (_internal_rotation & 4)
    {
      y = height() - y - 1;
      add_y = -1;
    }
    bool fast = _epd_mode == epd_mode_t::epd_fast || _epd_mode == epd_mode_t::epd_fastest;
    auto sx = param->src_x32;

    bool fastdraw = (param->transp == ~0u)
                 && ( _internal_rotation == 0
                  || (_internal_rotation == 1 && w == 1));
    if (fastdraw)
    {
      _set_area(x, y, w, h);
      flg_setarea = true;
      _wait_busy();
      _bus->writeData(0, 16);
    }

    do
    {
      std::uint32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          if (!fastdraw)
          {
            if (flg_setarea)
            {
              _write_command(IT8951_TCON_LD_IMG_END);
            }
            flg_setarea = true;
            _set_area(x + prev_pos, y, new_pos - prev_pos, 1);
            //_wait_busy();
            //_bus->writeData(0, 16);
          }
          std::uint32_t writepos = 1;
          std::int32_t shift = (3 - ((x + prev_pos) & 3)) << 2;
          auto btbl = &Bayer[(y & 3) << 2];
          do
          {
            std::uint16_t buf = 0;
            do
            {
              auto color = readbuf[prev_pos];
              std::int32_t pixel = color.R8() + (color.G8() << 1) + color.B8();
              if (fast)
              {
                pixel = (pixel + btbl[(x + prev_pos) & 3] * 16 < 512) ? 0 : 15;
              }
              else
              {
                pixel = std::min<std::int32_t>(15, std::max<std::int32_t>(0, pixel + btbl[(x + prev_pos) & 3]) >> 6);
              }
              buf |= pixel << shift;
              shift -= 4;
            } while (new_pos != ++ prev_pos && shift >= 0);
            writebuf[writepos] = __builtin_bswap16(buf);
            writepos++;
            shift = 12;
            //_bus->writeData(__builtin_bswap16(buf), 16);
          } while (new_pos != prev_pos);
          writebuf[0] = 0;
          _wait_busy();
          _bus->writeBytes((std::uint8_t*)writebuf, writepos << 1, false);
        }
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
      y += add_y;
    } while (--h);
    heap_free(readbuf);
    if (flg_setarea)
    {
      _write_command(IT8951_TCON_LD_IMG_END);
    }
  }

  void Panel_IT8951::writeBlock(std::uint32_t rawcolor, std::uint32_t length)
  {
    auto xpos = _xpos;
    auto ypos = _ypos;
    std::int32_t len;
    do {
      len = std::min<std::int32_t>(length, _xe + 1 - _xpos);
      writeFillRectPreclipped(xpos, ypos, len, 1, rawcolor);
      xpos += len;
      if (xpos > _xe)
      {
        xpos = _xs;
        if (++ypos > _ye)
        {
          ypos = _ys;
        }
        _ypos = ypos;
      }
    } while (length -= len);
    _xpos = xpos;
  }

  void Panel_IT8951::writePixels(pixelcopy_t* param, std::uint32_t length)
  {
    std::uint32_t xs   = _xs  ;
    std::uint32_t ys   = _ys  ;
    std::uint32_t xe   = _xe  ;
    std::uint32_t ye   = _ye  ;
    std::uint32_t xpos = _xpos;
    std::uint32_t ypos = _ypos;
    std::uint32_t w;

    std::uint32_t maxw = std::min(length, xe - xs + 1);
    bgr888_t* readbuf = static_cast<bgr888_t*>(heap_alloc(maxw * sizeof(bgr888_t)));
    if (readbuf == nullptr) return;
    do
    {
      w = std::min(length, xe - xs + 1);
      auto y = _internal_rotation & 4 ? height() - ypos - 1 : ypos;
      std::int32_t prev_pos = 0, new_pos = 0;
      //do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        //if (new_pos != prev_pos)
        {
          _set_area(xpos + prev_pos, y, new_pos - prev_pos, 1);
          _wait_busy();
          _bus->writeData(0, 16);
          std::uint32_t shift = (3 - ((xpos + prev_pos) & 3)) << 2;
          std::uint16_t buf = 0;
          auto btbl = &Bayer[(y & 3) << 2];
          do
          {
            auto color = readbuf[prev_pos];
            std::uint16_t pixel = std::min(15, std::max(0, (color.R8() + (color.G8() << 1) + color.B8() + btbl[(xpos + prev_pos) & 3])) >> 6);
            buf |= pixel << shift;
            if (shift)
            {
              shift -= 4;
            }
            else
            {
              _bus->writeData(__builtin_bswap16(buf), 16);
              buf = 0;
              shift = 12;
            }
          } while (new_pos != ++prev_pos);
          if (shift != 12)
          {
            _bus->writeData(__builtin_bswap16(buf), 16);
          }
          _write_command(IT8951_TCON_LD_IMG_END);
        }
      }// while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      xpos += w;
      if (xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye)
        {
          ypos = ys;
        }
      }
    } while (length -= w);
    _xpos = xpos;
    _ypos = ypos;

    heap_free(readbuf);
  }

  bool Panel_IT8951::_read_raw_line(std::int32_t raw_x, std::int32_t raw_y, std::int32_t len, std::uint16_t* buf)
  {
    std::uint16_t params[4];
    auto addr = _tar_memaddr + raw_x + raw_y * _cfg.panel_width;
    params[0] = (std::uint16_t)addr;
    params[1] = (std::uint16_t)(addr >> 16);
    params[2] = len; // (len + 15) & ~15;
    params[3] = 0;
    return _write_args(IT8951_TCON_MEM_BST_RD_T, params, 4)
        && _write_command(IT8951_TCON_MEM_BST_RD_S)
        && _read_words(buf, len);
  }

  void Panel_IT8951::readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
/// IT8951には画素読出しコマンドが存在せず、画像メモリを直接読むコマンドが提供されている。
/// 画像メモリを直接読み出す場合、ビットシフトや回転方向の解決などは自前で行う必要がある。

    std::int32_t rx, ry, rw, rh;
    if (_internal_rotation & 4)
    {
      y = height() - y - h;
    }
    switch(_internal_rotation & 3)
    {
    default:
    case IT8951_ROTATE_0:
      rx = x;
      ry = y;
      rw = w;
      rh = h;
      break;
    case IT8951_ROTATE_90:
      rx = y;
      ry = _cfg.panel_height - w - x;
      rw = h;
      rh = w;
      break;
    case IT8951_ROTATE_180:
      rx = _cfg.panel_width - w - x;
      ry = _cfg.panel_height - h - y;
      rw = w;
      rh = h;
      break;
    case IT8951_ROTATE_270:
      rx = _cfg.panel_width - h - y;
      ry = x;
      rw = h;
      rh = w;
      break;
    }

    std::int32_t adjust_left = (rx & 3);
    std::uint32_t padding_len = (adjust_left + rw + 31) & ~31;
    auto readbuf = static_cast<std::uint8_t*>(heap_alloc(std::max(padding_len, rw * param->dst_bits >> 3)));
    auto colorbuf = static_cast<bgr888_t*>(heap_alloc(rw * sizeof(bgr888_t)));

    param->src_data = colorbuf;

    for (std::int32_t j = 0; j < rh; ++j)
    {
      _read_raw_line(rx & ~3, ry, padding_len >> 1, reinterpret_cast<std::uint16_t*>(readbuf));
      for (std::int32_t i = 0; i < rw; ++i)
      {
        auto l = readbuf[adjust_left + i];
        l = l + 8;
        colorbuf[i].set(l,l,l);
      }
      param->src_x = 0;

      if (0 == (_internal_rotation & 1))
      {
        if (_internal_rotation & 2)
        {
          param->src_x32_add = (~0 << 16);
          param->src_x = rw - 1;
        }
        std::uint32_t dstpos = rw * ((_internal_rotation & 4) ? (rh - j - 1) : j);
        param->fp_copy(dst, dstpos, dstpos + rw, param);
      }
      else
      {
        for (std::int32_t i = 0; i < rw; ++i) {
          std::int32_t dstpos;
          switch (_internal_rotation)
          {
          default:
        //case 0:   dstpos =       i           +       j      * rw; break;
          case 1:   dstpos =       i      * rh + (rh - j - 1)     ; break;
        //case 2:   dstpos = (rw - i - 1)      + (rh - j - 1) * rw; break;
          case 3:   dstpos = (rw - i - 1) * rh +       j          ; break;
        //case 4:   dstpos =       i           + (rh - j - 1) * rw; break;
          case 5:   dstpos = (rw - i - 1) * rh + (rh - j - 1)     ; break;
        //case 6:   dstpos = (rw - i - 1)      +       j      * rw; break;
          case 7:   dstpos =       i      * rh +       j          ; break;
          }
          param->fp_copy(dst, dstpos, dstpos + 1, param);
        }
      }
      ++ry;
    } while (--rh);
    cs_control(true);

    heap_free(colorbuf);
    heap_free(readbuf);
  }

//----------------------------------------------------------------------------
 }
}
