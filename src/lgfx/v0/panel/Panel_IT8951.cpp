#include "../lgfx_common.hpp"
#if defined (LGFX_ENABLE_V0)

#include "Panel_IT8951.hpp"
#include "../LGFX_Device.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  constexpr int8_t Panel_IT8951::Bayer[16];

  static constexpr uint32_t _tar_memaddr = 0x001236E0;

//Built in I80 Command Code
  static constexpr uint32_t IT8951_TCON_SYS_RUN         = 0x0001;
  static constexpr uint32_t IT8951_TCON_STANDBY         = 0x0002;
  static constexpr uint32_t IT8951_TCON_SLEEP           = 0x0003;
  static constexpr uint32_t IT8951_TCON_REG_RD          = 0x0010;
  static constexpr uint32_t IT8951_TCON_REG_WR          = 0x0011;
  static constexpr uint32_t IT8951_TCON_MEM_BST_RD_T    = 0x0012;
  static constexpr uint32_t IT8951_TCON_MEM_BST_RD_S    = 0x0013;
  static constexpr uint32_t IT8951_TCON_MEM_BST_WR      = 0x0014;
  static constexpr uint32_t IT8951_TCON_MEM_BST_END     = 0x0015;
  static constexpr uint32_t IT8951_TCON_LD_IMG          = 0x0020;
  static constexpr uint32_t IT8951_TCON_LD_IMG_AREA     = 0x0021;
  static constexpr uint32_t IT8951_TCON_LD_IMG_END      = 0x0022;

//I80 User defined command code
  static constexpr uint32_t IT8951_I80_CMD_DPY_AREA     = 0x0034;
  static constexpr uint32_t IT8951_I80_CMD_GET_DEV_INFO = 0x0302;
  static constexpr uint32_t IT8951_I80_CMD_DPY_BUF_AREA = 0x0037;
  static constexpr uint32_t IT8951_I80_CMD_VCOM         = 0x0039;
  static constexpr uint32_t IT8951_I80_CMD_FILLRECT     = 0x003A;

  static constexpr uint32_t IT8951_ROTATE_0    = 0;
  static constexpr uint32_t IT8951_ROTATE_90   = 1;
  static constexpr uint32_t IT8951_ROTATE_180  = 2;
  static constexpr uint32_t IT8951_ROTATE_270  = 3;

  static constexpr uint32_t IT8951_2BPP           = 0;
  static constexpr uint32_t IT8951_3BPP           = 1;
  static constexpr uint32_t IT8951_4BPP           = 2;
  static constexpr uint32_t IT8951_8BPP           = 3;
  static constexpr uint32_t IT8951_LDIMG_B_ENDIAN = 1;

/*-----------------------------------------------------------------------
IT8951 Registers defines
------------------------------------------------------------------------*/
//Register Base Address
  static constexpr uint32_t IT8951_DISPLAY_REG_BASE    = 0x1000; //Register RW access

//Base Address of Basic LUT Registers
  static constexpr uint32_t IT8951_LUT0EWHR   = (IT8951_DISPLAY_REG_BASE + 0x0000); //LUT0 Engine Width Height Reg
  static constexpr uint32_t IT8951_LUT0XYR    = (IT8951_DISPLAY_REG_BASE + 0x0040); //LUT0 XY Reg
  static constexpr uint32_t IT8951_LUT0BADDR  = (IT8951_DISPLAY_REG_BASE + 0x0080); //LUT0 Base Address Reg
  static constexpr uint32_t IT8951_LUT0MFN    = (IT8951_DISPLAY_REG_BASE + 0x00C0); //LUT0 Mode and Frame number Reg
  static constexpr uint32_t IT8951_LUT01AF    = (IT8951_DISPLAY_REG_BASE + 0x0114); //LUT0 and LUT1 Active Flag Reg

//Update Parameter Setting Register
  static constexpr uint32_t IT8951_UP0SR      = (IT8951_DISPLAY_REG_BASE + 0x134); //Update Parameter0 Setting Reg
  static constexpr uint32_t IT8951_UP1SR      = (IT8951_DISPLAY_REG_BASE + 0x138); //Update Parameter1 Setting Reg
  static constexpr uint32_t IT8951_LUT0ABFRV  = (IT8951_DISPLAY_REG_BASE + 0x13C); //LUT0 Alpha blend and Fill rectangle Value
  static constexpr uint32_t IT8951_UPBBADDR   = (IT8951_DISPLAY_REG_BASE + 0x17C); //Update Buffer Base Address
  static constexpr uint32_t IT8951_LUT0IMXY   = (IT8951_DISPLAY_REG_BASE + 0x180); //LUT0 Image buffer X/Y offset Reg
  static constexpr uint32_t IT8951_LUTAFSR    = (IT8951_DISPLAY_REG_BASE + 0x224); //LUT Status Reg (status of All LUT Engines)
  static constexpr uint32_t IT8951_BGVR       = (IT8951_DISPLAY_REG_BASE + 0x250); //Bitmap (1bpp) image color table

//System Registers
  static constexpr uint32_t IT8951_SYS_REG_BASE        = 0x0000;

//Address of System Registers
  static constexpr uint32_t IT8951_I80CPCR             = (IT8951_SYS_REG_BASE + 0x04);

//Memory Converter Registers
  static constexpr uint32_t IT8951_MCSR_BASE_ADDR      = 0x0200;
  static constexpr uint32_t IT8951_MCSR                = (IT8951_MCSR_BASE_ADDR + 0x0000);
  static constexpr uint32_t IT8951_LISAR               = (IT8951_MCSR_BASE_ADDR + 0x0008);


  void Panel_IT8951::post_init(LGFX_Device* gfx, bool use_reset)
  {
/*
    gfx->startWrite();
    uint16_t infobuf[20];
    WriteCommand(gfx, IT8951_I80_CMD_GET_DEV_INFO);
    ReadWords(gfx, infobuf, 20);
    for (int i = 0; i < 20; ++i)
    {
      Serial.printf(" %04x \r\n", infobuf[i]);
    }
    Serial.println();
    // _dev_memaddr_l = infobuf[2];
    // _dev_memaddr_h = infobuf[3];
    // _tar_memaddr = (_dev_memaddr_h << 16) | _dev_memaddr_l;
    log_d("memory addr = %04X%04X", infobuf[3], infobuf[2]);
    gfx->endWrite();

    WriteReg(gfx, IT8951_UP1SR + 2, 0x20);   // Set normal mode
    WriteReg(gfx, IT8951_BGVR, 0xFF);  //Set BitMap color table 0 and 1 , => Set Register[0x18001250]:
                                      //Bit[7:0]: ForeGround Color(G0~G15) for 1
                                      //Bit[15:8]:Background Color(G0~G15) for 0
//*/
    gfx->startWrite();
    WriteCommand(gfx, IT8951_TCON_SYS_RUN);
    WriteReg(gfx, IT8951_I80CPCR, 0x0001); //enable pack write

    WriteCommand(gfx, 0x0039); //tcon vcom set command
    WriteWord(gfx, 0x0001);
    WriteWord(gfx, 2300); // 0x08fc

    SetTargetMemoryAddr(gfx, _tar_memaddr);

    _range_new.top = INT16_MAX;
    _range_new.left = INT16_MAX;
    _range_new.right = 0;
    _range_new.bottom = 0;
    gfx->setBaseColor(TFT_WHITE);
    gfx->setTextColor(TFT_BLACK, TFT_WHITE);

    if (use_reset) {
      auto mode = gfx->getEpdMode();
      gfx->setEpdMode(epd_mode_t::epd_quality);
      fillRect(this, gfx, 0, 0, gfx->width(), gfx->height(), 0);
      display(this, gfx, 0, 0, 0, 0);
      fillRect(this, gfx, 0, 0, gfx->width(), gfx->height(), ~0u);
      gfx->setEpdMode(epd_mode_t::epd_fastest);
      display(this, gfx, 0, 0, 0, 0);
      gfx->setEpdMode(mode);
      CheckAFSR(gfx);
    }

    gfx->endWrite();
  }

  bool Panel_IT8951::WaitBusy(LGFX_Device* gfx, uint32_t timeout)
  {
    gfx->cs_h();
    if (gpio_busy >= 0 && !lgfx::gpio_in(gpio_busy))
    {
      bool res = true;
      auto time = millis();
      do {
        if (millis() - time > timeout)
        {
          res = false;
          break;
        }
      } while (!lgfx::gpio_in(gpio_busy));

      if (res == false)
      {
        return false;
      }
    }
    gfx->cs_l();
    return true;
  }
  
  bool Panel_IT8951::CheckAFSR(LGFX_Device* gfx)
  {
    uint32_t start_time = millis();
    uint16_t infobuf[1] = { 1 };
    do
    {
      delay(1);
      if (WriteCommand(gfx, IT8951_TCON_REG_RD)
       && WriteWord(gfx, IT8951_LUTAFSR)
       && ReadWords(gfx, infobuf, 1)
       && infobuf[0] == 0)
      {
        break;
      }
    } while (millis() - start_time < 3000);

    gfx->cs_h();
    return infobuf[0] != 0;
  }

  bool Panel_IT8951::WriteCommand(LGFX_Device* gfx, uint16_t cmd)
  {
    if (!WaitBusy(gfx)) return false;

    gfx->writeData16(0x6000);
    uint32_t retry = 0xFFFF;
    while (gpio_busy >= 0 && !lgfx::gpio_in(gpio_busy) && --retry);
    gfx->writeData16(cmd);
    return true;
  }

  bool Panel_IT8951::WriteWord(LGFX_Device* gfx, uint16_t data)
  {
    if (!WaitBusy(gfx)) return false;

    gfx->writeData16(0);
    uint32_t retry = 0xFFFF;
    while (gpio_busy >= 0 && !lgfx::gpio_in(gpio_busy) && --retry);
    gfx->writeData16(data);
    return true;
  }

  bool Panel_IT8951::WriteArgs(LGFX_Device* gfx, uint16_t cmd, uint16_t *args, int32_t length)
  {
    if (WriteCommand(gfx, cmd)
     && WaitBusy(gfx))
    {
      gfx->writeData16(0);
      for (int32_t i = 0; i < length; i++)
      {
        gfx->waitDMA();
        while (!lgfx::gpio_in(gpio_busy));
        gfx->writeData16(args[i]);
      }
      return true;
    }
    return false;
  }

  bool Panel_IT8951::ReadWords(LGFX_Device* gfx, uint16_t *buf, uint32_t length)
  {
    if (!WaitBusy(gfx)) return false;
    gfx->writeData16(0x1000);
    gfx->writeData16(0); // dummy read
    gfx->readBytes(reinterpret_cast<uint8_t*>(buf), length << 1);
    gfx->cs_h();
    for (size_t i = 0; i < length; i++)
    {
      buf[i] = __builtin_bswap16(buf[i]);
    }
    return true;
  }

  bool Panel_IT8951::WriteReg(LGFX_Device* gfx, uint16_t addr, uint16_t data)
  {
    return WriteCommand(gfx, 0x0011)
        && WriteWord(gfx, addr)
        && WriteWord(gfx, data);
  }

  bool Panel_IT8951::SetTargetMemoryAddr(LGFX_Device* gfx, uint32_t tar_addr)
  {
    return WriteReg(gfx, IT8951_LISAR + 2, tar_addr >> 16)
        && WriteReg(gfx, IT8951_LISAR    , tar_addr      );
  }

  bool Panel_IT8951::SetArea(LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h)
  {
    int32_t rx, ry, rw, rh;
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
      ry = panel_height - w - x;
      rw = h;
      rh = w;
      break;
    case IT8951_ROTATE_180:
      rx = panel_width - w - x;
      ry = panel_height - h - y;
      rw = w;
      rh = h;
      break;
    case IT8951_ROTATE_270:
      rx = panel_width - h - y;
      ry = x;
      rw = h;
      rh = w;
      break;
    }

    _range_new.left   = std::min(_range_new.left  , rx         );
    _range_new.right  = std::max(_range_new.right , rx + rw - 1);
    _range_new.top    = std::min(_range_new.top   , ry         );
    _range_new.bottom = std::max(_range_new.bottom, ry + rh - 1);

    if (gfx->getEpdMode() != epd_mode_t::epd_fastest
     && _range_old.horizon.intersectsWith(rx, rx + rw - 1)
     && _range_old.vertical.intersectsWith(ry, ry + rh - 1))
    {
      CheckAFSR(gfx);
      _range_old.left = INT_MAX;
      _range_old.top = INT_MAX;
      _range_old.right = 0;
      _range_old.bottom = 0;
    }

    uint16_t params[5];
    params[0] = IT8951_LDIMG_B_ENDIAN << 8 | IT8951_4BPP << 4 | _internal_rotation;
    params[1] = x;
    params[2] = y;
    params[3] = w;
    params[4] = h;
    return WriteArgs(gfx, IT8951_TCON_LD_IMG_AREA, params, 5);
  }

  bool Panel_IT8951::UpdateRawArea(LGFX_Device* gfx, epd_update_mode_t mode)
  {
    if (_range_new.empty()) return false;
    uint32_t l = _range_new.left;
    uint32_t r = _range_new.right;
    // 更新範囲の幅が小さすぎる場合、IT8951がクラッシュすることがある。
    // 厳密には、左右端の座標値の下2ビットを無視して同値になる場合で、
    // かつ以前の表示更新がまだ動作中の場合にクラッシュする事例がある。
    if ((l & ~3) == (r & ~3))
    {
      if (( l & 3 ) < (3-(r & 3)))
      {
        l = (l & ~3) - 1;
      }
      else
      {
        r = (r + 4) & ~3;
      }
    }
    uint32_t w = r - l + 1;
    uint16_t params[7];
    params[0] = l;//_range_new.left;
    params[1] = _range_new.top;
    params[2] = w;//_range_new.right - _range_new.left + 1;
    params[3] = _range_new.bottom - _range_new.top + 1;
    params[4] = mode;
    params[5] = (uint16_t)_tar_memaddr;
    params[6] = (uint16_t)(_tar_memaddr >> 16);
    return WriteArgs(gfx, IT8951_I80_CMD_DPY_BUF_AREA, params, 7);
  }

  void Panel_IT8951::fillRect(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t rawcolor)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
    if (me->_internal_rotation & 4)
    {
      y = gfx->height() - y - h;
    }
    me->SetArea(gfx, x, y, w, h);
    bgr888_t color;
    color = rawcolor;
    int32_t sum = color.R8() + (color.G8() << 1) + color.B8();
    me->WaitBusy(gfx);
    gfx->writeData16(0);
    auto mode = gfx->getEpdMode();
    bool fast = mode == epd_mode_t::epd_fast || mode == epd_mode_t::epd_fastest;
    uint32_t wid = (((x + w + 3) >> 2) - (x >> 2));
    do
    {
      auto btbl = &me->Bayer[(y & 3) << 2];
      uint32_t value;
      if (fast)
      {
        value = (sum + btbl[0]*16 < 512 ? 0 : 0xF000)
              | (sum + btbl[1]*16 < 512 ? 0 : 0x0F00)
              | (sum + btbl[2]*16 < 512 ? 0 : 0x00F0)
              | (sum + btbl[3]*16 < 512 ? 0 : 0x000F);
      }
      else
      {
        value = std::min<int32_t>(15, (std::max<int32_t>(0, sum + btbl[0])) >> 6) << 12
              | std::min<int32_t>(15, (std::max<int32_t>(0, sum + btbl[1])) >> 6) <<  8
              | std::min<int32_t>(15, (std::max<int32_t>(0, sum + btbl[2])) >> 6) <<  4
              | std::min<int32_t>(15, (std::max<int32_t>(0, sum + btbl[3])) >> 6) ;
      }
      auto len = wid;
      if (len & 1) gfx->writeData16(value);
      if (len >>= 1)
      {
        value |= value << 16;
        do {
          gfx->writeData32(value);
        } while (--len);
      }
      ++y;
    } while (--h);
    me->WriteCommand(gfx, IT8951_TCON_LD_IMG_END);
    gfx->cs_h();
  }

  void Panel_IT8951::pushImage(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param)
  {
    bgr888_t* readbuf = static_cast<bgr888_t*>(heap_alloc(w * sizeof(bgr888_t)));
    if (readbuf == nullptr) return;

    auto me = reinterpret_cast<Panel_IT8951*>(panel);
    int32_t add_y = 1;
    bool flg_setarea = false;
    if (me->_internal_rotation & 4)
    {
      y = gfx->height() - y - 1;
      add_y = -1;
    }
    auto mode = gfx->getEpdMode();
    bool fast = mode == epd_mode_t::epd_fast || mode == epd_mode_t::epd_fastest;
    auto sx = param->src_x32;

    do
    {
      int32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          if (flg_setarea)
          {
            me->WriteCommand(gfx, IT8951_TCON_LD_IMG_END);
          }
          flg_setarea = true;
          me->SetArea(gfx, x + prev_pos, y, new_pos - prev_pos, 1);
          me->WaitBusy(gfx);
          gfx->writeData16(0);

          int32_t shift = (3 - ((x + prev_pos) & 3)) << 2;
          auto btbl = &me->Bayer[(y & 3) << 2];
          do
          {
            uint16_t buf = 0;
            do
            {
              auto color = readbuf[prev_pos];
              int32_t pixel = color.R8() + (color.G8() << 1) + color.B8();
              if (fast)
              {
                pixel = (pixel + btbl[(x + prev_pos) & 3] * 16 < 512) ? 0 : 15;
              }
              else
              {
                pixel = std::min<int32_t>(15, std::max<int32_t>(0, pixel + btbl[(x + prev_pos) & 3]) >> 6);
              }
              buf |= pixel << shift;
              shift -= 4;
            } while (new_pos != ++ prev_pos && shift >= 0);
            gfx->writeData16(buf);
            shift = 12;
          } while (new_pos != prev_pos);
        }
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
      y += add_y;
    } while (--h);
    if (flg_setarea)
    {
      me->WriteCommand(gfx, IT8951_TCON_LD_IMG_END);
    }
    heap_free(readbuf);
    gfx->cs_h();
  }

  void Panel_IT8951::pushBlock(PanelCommon* panel, LGFX_Device* gfx, int32_t length, uint32_t rawcolor)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
    auto xpos = me->_xpos;
    auto ypos = me->_ypos;
    int32_t len;
    do {
      len = std::min<int32_t>(length, me->_xe + 1 - me->_xpos);
      fillRect(panel, gfx, xpos, ypos, len, 1, rawcolor);
      xpos += len;
      if (xpos > me->_xe)
      {
        xpos = me->_xs;
        if (++ypos > me->_ye)
        {
          ypos = me->_ys;
        }
        me->_ypos = ypos;
      }
    } while (length -= len);
    me->_xpos = xpos;
  }

  void Panel_IT8951::writePixels(PanelCommon* panel, LGFX_Device* gfx, int32_t length, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
    int32_t xs   = me->_xs  ;
    int32_t ys   = me->_ys  ;
    int32_t xe   = me->_xe  ;
    int32_t ye   = me->_ye  ;
    int32_t xpos = me->_xpos;
    int32_t ypos = me->_ypos;
    uint32_t w;

    int32_t maxw = std::min(length, xe - xs + 1);
    bgr888_t* readbuf = static_cast<bgr888_t*>(heap_alloc(maxw * sizeof(bgr888_t)));
    if (readbuf == nullptr) return;
    do
    {
      w = std::min(length, xe - xs + 1);
      auto y = me->_internal_rotation & 4 ? gfx->height() - ypos - 1 : ypos;
      int32_t prev_pos = 0, new_pos = 0;
      //do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        //if (new_pos != prev_pos)
        {
          me->SetArea(gfx, xpos + prev_pos, y, new_pos - prev_pos, 1);
          me->WaitBusy(gfx);
          gfx->writeData16(0);
          uint32_t shift = (3 - ((xpos + prev_pos) & 3)) << 2;
          uint16_t buf = 0;
          auto btbl = &me->Bayer[(y & 3) << 2];
          do
          {
            auto color = readbuf[prev_pos];
            auto pixel = std::min(15, std::max(0, (color.R8() + (color.G8() << 1) + color.B8() + btbl[(xpos + prev_pos) & 3])) >> 6);
            buf |= pixel << shift;
            if (shift)
            {
              shift -= 4;
            }
            else
            {
              gfx->writeData16(buf);
              buf = 0;
              shift = 12;
            }
          } while (new_pos != ++prev_pos);
          if (shift != 12)
          {
            gfx->writeData16(buf); 
          }
          me->WriteCommand(gfx, IT8951_TCON_LD_IMG_END);
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
    me->_xpos = xpos;
    me->_ypos = ypos;

    heap_free(readbuf);
    gfx->cs_h();
  }

  bool Panel_IT8951::ReadRawLine(LGFX_Device* gfx, int32_t raw_x, int32_t raw_y, int32_t len, uint16_t* buf)
  {
    uint16_t params[4];
    auto addr = _tar_memaddr + raw_x + raw_y * panel_width;
    params[0] = (uint16_t)addr;
    params[1] = (uint16_t)(addr >> 16);
    params[2] = len; // (len + 15) & ~15;
    params[3] = 0;
    return WriteArgs(gfx, IT8951_TCON_MEM_BST_RD_T, params, 4)
        && WriteCommand(gfx, IT8951_TCON_MEM_BST_RD_S)
        && ReadWords(gfx, buf, len);
  }

  void Panel_IT8951::readRect(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);

    int32_t rx, ry, rw, rh;
    if (me->_internal_rotation & 4)
    {
      y = gfx->height() - y - h;
    }
    switch(me->_internal_rotation & 3)
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
      ry = me->panel_height - w - x;
      rw = h;
      rh = w;
      break;
    case IT8951_ROTATE_180:
      rx = me->panel_width - w - x;
      ry = me->panel_height - h - y;
      rw = w;
      rh = h;
      break;
    case IT8951_ROTATE_270:
      rx = me->panel_width - h - y;
      ry = x;
      rw = h;
      rh = w;
      break;
    }

    int32_t adjust_left = (rx & 3);
    int32_t padding_len = (adjust_left + rw + 31) & ~31;
    auto readbuf = static_cast<uint8_t*>(heap_alloc(padding_len));
    auto colorbuf = static_cast<bgr888_t*>(heap_alloc(rw * sizeof(bgr888_t)));

    param->src_data = colorbuf;

    for (int32_t j = 0; j < rh; ++j) {
      me->ReadRawLine(gfx, rx & ~3, ry, padding_len >> 1, reinterpret_cast<uint16_t*>(readbuf));
      for (int32_t i = 0; i < rw; ++i)
      {
        auto l = readbuf[adjust_left + i];
        l = l + 8;
        colorbuf[i].set(l,l,l);
      }
      param->src_x = 0;
      for (int32_t i = 0; i < rw; ++i) {
        int32_t dstpos;
        switch (me->_internal_rotation)
        {
        default:
        case 0:   dstpos =       i           +       j      * rw; break;
        case 1:   dstpos =       i      * rh + (rh - j - 1)     ; break;
        case 2:   dstpos = (rw - i - 1)      + (rh - j - 1) * rw; break;
        case 3:   dstpos = (rw - i - 1) * rh +       j          ; break;
        case 4:   dstpos =       i           + (rh - j - 1) * rw; break;
        case 5:   dstpos = (rw - i - 1) * rh + (rh - j - 1)     ; break;
        case 6:   dstpos = (rw - i - 1)      +       j      * rw; break;
        case 7:   dstpos =       i      * rh +       j          ; break;
        }
        param->fp_copy(dst, dstpos, dstpos + 1, param);
      }
      ++ry;
    } while (--rh);

    heap_free(colorbuf);
    heap_free(readbuf);
  }

  void Panel_IT8951::waitDisplay(PanelCommon* panel, LGFX_Device* gfx)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
    me->CheckAFSR(gfx);
    return;
  }

  bool Panel_IT8951::displayBusy(PanelCommon* panel, LGFX_Device* gfx)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
    uint16_t infobuf[1] = { 1 };
    if (me->WriteCommand(gfx, IT8951_TCON_REG_RD)
      && me->WriteWord(gfx, IT8951_LUTAFSR)
      && me->ReadWords(gfx, infobuf, 1))
    {
      return 0 != infobuf[0];
    }
    gfx->cs_h();
    return true;
  }

  void Panel_IT8951::display(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
    if (0 < w && 0 < h)
    {
      me->_range_new.left   = std::min(me->_range_new.left  , x        );
      me->_range_new.right  = std::max(me->_range_new.right , x + w - 1);
      me->_range_new.top    = std::min(me->_range_new.top   , y        );
      me->_range_new.bottom = std::max(me->_range_new.bottom, y + h - 1);
    }
    if (me->_range_new.empty()) return;

    me->_range_old = me->_range_new;
    epd_update_mode_t mode;
    switch (gfx->getEpdMode())
    {
    case epd_mode_t::epd_fastest:  mode = UPDATE_MODE_DU4;  break;
    case epd_mode_t::epd_fast:     mode = UPDATE_MODE_DU;   break;
    case epd_mode_t::epd_text:     mode = UPDATE_MODE_GL16; break;
    default:                       mode = UPDATE_MODE_GC16; break;
    }

    me->UpdateRawArea(gfx, mode);

    me->_range_new.top = INT32_MAX;
    me->_range_new.left = INT32_MAX;
    me->_range_new.right = 0;
    me->_range_new.bottom = 0;
  }

//----------------------------------------------------------------------------
 }
}
#endif
