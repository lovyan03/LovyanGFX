#include "Panel_IT8951.hpp"
#include "../LGFX_Device.hpp"

namespace lgfx
{
  constexpr std::int8_t Panel_IT8951::Bayer[16];

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


  static constexpr std::uint32_t IT8951_ROTATE_0    = 0;
  static constexpr std::uint32_t IT8951_ROTATE_90   = 1;
  static constexpr std::uint32_t IT8951_ROTATE_180  = 2;
  static constexpr std::uint32_t IT8951_ROTATE_270  = 3;

  static constexpr std::uint32_t IT8951_4BPP           = 2;
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


  void Panel_IT8951::post_init(LGFX_Device* gfx, bool use_reset)
  {
    if ((int)gfx->getPanel() != (int)this)
    {
    //Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); 
      gfx->setPanel(this); // workaround; _panelのアドレスが変わってしまっているので再設定する。原因未解明。 
    }
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
//*/

    gfx->startWrite();
    WriteCommand(gfx, IT8951_TCON_SYS_RUN);
    WriteReg(gfx, IT8951_I80CPCR, 0x0001); //enable pack write

    WriteCommand(gfx, 0x0039); //tcon vcom set command
    WriteWord(gfx, 0x0001);
    WriteWord(gfx, 2300); // 0x08fc
    SetTargetMemoryAddr(gfx, _tar_memaddr);

    if (use_reset) {
      _tr_top = INT32_MAX;
      _tr_left = INT32_MAX;
      _tr_right = 0;
      _tr_bottom = 0;
      UpdateArea(gfx, 0, 0, panel_width, panel_height, UPDATE_MODE_INIT);
    }
    gfx->endWrite();
  }

  bool Panel_IT8951::WaitBusy(LGFX_Device* gfx, std::uint32_t timeout)
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
//Serial.println("IT8951 timeout error");
        return false;
      }
    }
    gfx->cs_l();
    return true;
  }
  
  bool Panel_IT8951::CheckAFSR(LGFX_Device* gfx)
  {
//if ((int)gfx->getPanel() != (int)this) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); }
    uint32_t start_time = millis();
    uint16_t infobuf[1];
    do
    {
      if (WriteCommand(gfx, IT8951_TCON_REG_RD)
       && WriteWord(gfx, IT8951_LUTAFSR)
       && ReadWords(gfx, infobuf, 1)
       && infobuf[0] == 0)
      {
        return true;
      }
      delay(1);
    } while (millis() - start_time < 3000);

    return false;
  }

  bool Panel_IT8951::WriteCommand(LGFX_Device* gfx, uint16_t cmd)
  {
//if ((int)gfx->getPanel() != (int)this) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); }
    if (WaitBusy(gfx))
    {
      gfx->writeData16(0x6000);
        std::uint32_t retry = 0xFFFF;
        while (gpio_busy >= 0 && !lgfx::gpio_in(gpio_busy) && --retry);
      gfx->writeData16(cmd);
      //gfx->writeData32(0x6000 << 16 | cmd);
      return true;
    }
//    Serial.printf("WriteCmd timeout : %04x ", cmd);
    return false;
  }

  bool Panel_IT8951::WriteWord(LGFX_Device* gfx, uint16_t data)
  {
//if ((int)gfx->getPanel() != (int)this) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); }
    if (WaitBusy(gfx))
    {
      gfx->writeData16(0);
        std::uint32_t retry = 0xFFFF;
        while (gpio_busy >= 0 && !lgfx::gpio_in(gpio_busy) && --retry);
      gfx->writeData16(data);
      return true;
    }
//    Serial.printf("WriteDat timeout : %04x ", data);
    return false;
  }

  bool Panel_IT8951::WriteArgs(LGFX_Device* gfx, uint16_t cmd, uint16_t *args, int32_t length)
  {
//if ((int)gfx->getPanel() != (int)this) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); }
    if (WriteCommand(gfx, cmd)
     && WaitBusy(gfx))
    {
      gfx->writeData16(0);
      for (std::int32_t i = 0; i < length; i++)
      {
        std::uint32_t retry = 0xFFFF;
        while (gpio_busy >= 0 && !lgfx::gpio_in(gpio_busy) && --retry);
        gfx->writeData16(args[i]);
      }
      return true;
    }
    return false;
  }

  bool Panel_IT8951::ReadWords(LGFX_Device* gfx, uint16_t *buf, uint32_t length)
  {
//if ((int)gfx->getPanel() != (int)this) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); }
    if (!WaitBusy(gfx)) return false;
    gfx->writeData16(0x1000);
    gfx->writeData16(0); // dummy read
    gfx->readBytes(reinterpret_cast<std::uint8_t*>(buf), length << 1);
    gfx->cs_h();
    for (size_t i = 0; i < length; i++)
    {
      buf[i] = __builtin_bswap16(buf[i]);
    }
    return true;
  }

  bool Panel_IT8951::WriteReg(LGFX_Device* gfx, uint16_t addr, uint16_t data)
  {
//if ((int)gfx->getPanel() != (int)this) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); }
    return WriteCommand(gfx, 0x0011)
        && WriteWord(gfx, addr)
        && WriteWord(gfx, data);
  }

  bool Panel_IT8951::SetTargetMemoryAddr(LGFX_Device* gfx, uint32_t tar_addr)
  {
//if ((int)gfx->getPanel() != (int)this) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); }
    return WriteReg(gfx, IT8951_LISAR + 2, tar_addr >> 16)
        && WriteReg(gfx, IT8951_LISAR    , tar_addr      );
  }

  bool Panel_IT8951::SetArea(LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
  {
//if ((int)gfx->getPanel() != (int)this) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); }
    if (!WriteCommand(gfx, IT8951_TCON_LD_IMG_AREA))
    {
      // Serial.println("error: SetArea 1");
      return false;
    }

    if (!WaitBusy(gfx))
    {
      // Serial.println("error: SetArea 2");
      return false;
    }
    gfx->writeData32(IT8951_LDIMG_B_ENDIAN << 8 | IT8951_4BPP << 4 | _internal_rotation);
    _tr_left  =  std::min(x, _tr_left);
    _tr_top    = std::min(y, _tr_top);
    gfx->writeData32(x << 16 | y);
    _tr_right  = std::max(x+w-1, _tr_right);
    _tr_bottom = std::max(y+h-1, _tr_bottom);
    gfx->writeData32(w << 16 | h);

//      Serial.println(" Args ok");
    return true;
//    Serial.println(" Args timeout");
//    return false;

/*
    uint16_t args[5];
    args[0] = (IT8951_LDIMG_B_ENDIAN << 8 | IT8951_4BPP << 4 | _internal_rotation);
    args[1] = x;
    args[2] = y;
    args[3] = w;
    args[4] = h;
    return WriteArgs(gfx, IT8951_TCON_LD_IMG_AREA, args, 5);
    //*/
  }

  bool Panel_IT8951::UpdateArea(LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, m5epd_update_mode_t mode)
  {
//if ((int)gfx->getPanel() != (int)this) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)this); }
    int retry = 5;
    bool res = true;
    do {
      if (CheckAFSR(gfx)
       && WriteCommand(gfx, IT8951_I80_CMD_DPY_BUF_AREA)
       && WaitBusy(gfx, 50)) break;
      res = false;
//      Serial.println("error: UpdateArea 1");
      if (gpio_rst >= 0)
      {
        gfx->cs_h();
        gpio_lo(gpio_rst);
        delay(2);
        gpio_hi(gpio_rst);
        delay(1);
        post_init(gfx, false);
      }
    } while (--retry);
    gfx->writeData16(0);

    switch(_internal_rotation & 3)
    {
    case IT8951_ROTATE_0:
      gfx->writeData32(x << 16 | y);
      gfx->writeData32(w << 16 | h);
      break;
    case IT8951_ROTATE_90:
      gfx->writeData32(y << 16 | (panel_height - w - x));
      gfx->writeData32(h << 16 | w);
      break;
    case IT8951_ROTATE_180:
      gfx->writeData32((panel_width - w - x) << 16 | (panel_height - h - y));
      gfx->writeData32(w << 16 | h);
      break;
    case IT8951_ROTATE_270:
      gfx->writeData32((panel_width - h - y) << 16 | x);
      gfx->writeData32(h << 16 | w);
      break;
    }
    gfx->writeData16(mode);
    gfx->writeData16((std::uint16_t)_tar_memaddr);
    gfx->writeData16(_tar_memaddr >> 16);
    return res;
/*/
    uint16_t args[7];
    args[0] = x;
    args[1] = y;
    args[2] = w;
    args[3] = h;
    args[4] = mode;
    args[5] = (std::uint16_t)_tar_memaddr;
    args[6] = _tar_memaddr >> 16;

    switch(_internal_rotation & 3)
    {
        case IT8951_ROTATE_0:
        {
            args[0] = x;
            args[1] = y;
            args[2] = w;
            args[3] = h;
            break;
        }
        case IT8951_ROTATE_90:
        {
            args[0] = y;
            args[1] = panel_height - w -x;
            args[2] = h;
            args[3] = w;
            break;
        }
        case IT8951_ROTATE_180:
        {
            args[0] = panel_width - w - x;
            args[1] = panel_height - h - y;
            args[2] = w;
            args[3] = h;
            break;
        }
        case IT8951_ROTATE_270:
        {
            args[0] = panel_width - h - y;
            args[1] = x;
            args[2] = h;
            args[3] = w;
            break;
        }
    }

    return WriteArgs(gfx, IT8951_I80_CMD_DPY_BUF_AREA, args, 7);
//*/
  }

  void Panel_IT8951::fillRect(PanelCommon* panel, LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint32_t rawcolor)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
//if ((int)gfx->getPanel() != (int)me) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)me); }
    if (me->_internal_rotation & 4)
    {
      y = gfx->height() - y - h;
    }
    me->SetArea(gfx, x, y, w, h);
    bgr888_t color;
    color = rawcolor;
    std::int32_t sum = color.R8() + (color.G8() << 1) + color.B8();
    me->WaitBusy(gfx);
    gfx->writeData16(0);
    uint32_t wid = (((x + w + 3) >> 2) - (x >> 2));
    do
    {
      auto btbl = &me->Bayer[(y & 3) << 2];
/*
      std::uint32_t value = (sum >> 6) << 12
                          | (sum >> 6) <<  8
                          | (sum >> 6) <<  4
                          | (sum >> 6) ;
/*/
      std::uint32_t value = std::min<std::int32_t>(15, (std::max<std::int32_t>(0, sum + btbl[0])) >> 6) << 12
                          | std::min<std::int32_t>(15, (std::max<std::int32_t>(0, sum + btbl[1])) >> 6) <<  8
                          | std::min<std::int32_t>(15, (std::max<std::int32_t>(0, sum + btbl[2])) >> 6) <<  4
                          | std::min<std::int32_t>(15, (std::max<std::int32_t>(0, sum + btbl[3])) >> 6) ;
//*/
      auto len = wid;
      if (len & 1) gfx->writeData16(value);
      if (len >>= 1)
      {
        value |= value << 16;
        do {
          //std::uint32_t retry = 0xFFFF;
          //while (me->gpio_busy >= 0 && !lgfx::gpio_in(me->gpio_busy) && --retry);
          gfx->writeData32(value);
        } while (--len);
      }
      ++y;
    } while (--h);
    me->WriteCommand(gfx, IT8951_TCON_LD_IMG_END);
    gfx->cs_h();
  }

  void Panel_IT8951::pushImage(PanelCommon* panel, LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
//if ((int)gfx->getPanel() != (int)me) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)me); }
    std::int32_t add_y = 1;
    if (me->_internal_rotation & 4)
    {
      y = gfx->height() - y - 1;
      add_y = -1;
    }
    bgr888_t readbuf[w];
    auto sx = param->src_x32;
    do
    {
      std::int32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          me->SetArea(gfx, x + prev_pos, y, new_pos - prev_pos, 1);
          me->WaitBusy(gfx);
          gfx->writeData16(0);
          std::uint32_t shift = (3 - ((x + prev_pos) & 3)) << 2;
          std::uint16_t buf = 0;
          auto btbl = &me->Bayer[(y & 3) << 2];
          do
          {
            auto color = readbuf[prev_pos];
            auto pixel = std::min(15, std::max(0, (color.R8() + (color.G8() << 1) + color.B8() + btbl[(x + prev_pos) & 3])) >> 6);
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
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
      y += add_y;
    } while (--h);

    gfx->cs_h();
  }

  void Panel_IT8951::pushBlock(PanelCommon* panel, LGFX_Device* gfx, std::int32_t length, std::uint32_t rawcolor)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
    auto xpos = me->_xpos;
    auto ypos = me->_ypos;
    std::int32_t len;
    do {
      len = std::min<std::int32_t>(length, me->_xe + 1 - me->_xpos);
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

  void Panel_IT8951::writePixels(PanelCommon* panel, LGFX_Device* gfx, std::int32_t length, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
//if ((int)gfx->getPanel() != (int)me) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)me); }
    std::int32_t xs   = me->_xs  ;
    std::int32_t ys   = me->_ys  ;
    std::int32_t xe   = me->_xe  ;
    std::int32_t ye   = me->_ye  ;
    std::int32_t xpos = me->_xpos;
    std::int32_t ypos = me->_ypos;
    std::uint32_t w;

    std::int32_t maxw = std::min(length, xe - xs + 1);
    bgr888_t readbuf[maxw];
    do
    {
      w = std::min(length, xe - xs + 1);
      auto y = me->_internal_rotation & 4 ? gfx->height() - ypos - 1 : ypos;
      std::int32_t prev_pos = 0, new_pos = 0;
      //do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        //if (new_pos != prev_pos)
        {
          me->SetArea(gfx, xpos + prev_pos, y, new_pos - prev_pos, 1);
          me->WaitBusy(gfx);
          gfx->writeData16(0);
          std::uint32_t shift = (3 - ((xpos + prev_pos) & 3)) << 2;
          std::uint16_t buf = 0;
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

    gfx->cs_h();
  }

  void Panel_IT8951::readRect(PanelCommon* panel, LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param)
  {
    (void)panel;
    (void)gfx;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)dst;
    (void)param;    
  }

  void Panel_IT8951::flush(PanelCommon* panel, LGFX_Device* gfx)
  {
/*
    gfx->startWrite();
    endTransaction(panel, gfx);
    beginTransaction(panel, gfx);
    gfx->endWrite();
/*/
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
    bool res = false;

    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t w = gfx->width();
    std::int32_t h = gfx->height();
    if (me->_tr_left <= me->_tr_right && me->_tr_top <= me->_tr_bottom)
    {
      x = me->_tr_left;
      y = me->_tr_top;
      w = me->_tr_right - me->_tr_left + 1;
      h = me->_tr_bottom - me->_tr_top + 1;
    }
    do
    {
      res = me->UpdateArea(gfx, x, y, w, h, UPDATE_MODE_GC16);
      //if (!res) Serial.println("flush error : restart");
    } while (!res);
    me->_tr_top = INT32_MAX;
    me->_tr_left = INT32_MAX;
    me->_tr_right = 0;
    me->_tr_bottom = 0;
  }

  void Panel_IT8951::beginTransaction(PanelCommon* panel, LGFX_Device* gfx)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
//if ((int)gfx->getPanel() != (int)me) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)me); }
//    Serial.print("begintr");
    if (me->_tr_left <= me->_tr_right && me->_tr_top <= me->_tr_bottom)
    {
      me->_tr_top = INT32_MAX;
      me->_tr_left = INT32_MAX;
      me->_tr_right = 0;
      me->_tr_bottom = 0;
      me->CheckAFSR(gfx);
    }
    //me->SetTargetMemoryAddr(gfx, _tar_memaddr);
  }

  void Panel_IT8951::endTransaction(PanelCommon* panel, LGFX_Device* gfx)
  {
    auto me = reinterpret_cast<Panel_IT8951*>(panel);
//Serial.printf("e\r\n");
//if ((int)gfx->getPanel() != (int)me) { Serial.printf("error: %08x:%08x \r\n", (int)gfx->getPanel() , (int)me); }
    if (me->_tr_left > me->_tr_right || me->_tr_top > me->_tr_bottom) return;
    std::int32_t x = me->_tr_left;
    std::int32_t y = me->_tr_top;
    std::int32_t w = me->_tr_right - me->_tr_left + 1;
    std::int32_t h = me->_tr_bottom - me->_tr_top + 1;

    //Serial.printf("etr: %d %d %d %d \r\n" , x, y, w, h);
    //while (!me->UpdateArea(gfx, x, y, w, h, UPDATE_MODE_GC16)) delay(1000);
    if (!me->UpdateArea(gfx, x, y, w, h, UPDATE_MODE_GC16))
    {
      me->UpdateArea(gfx, x, y, w, h, UPDATE_MODE_GC16);
      me->UpdateArea(gfx, x, y, w, h, UPDATE_MODE_GC16);
    }
  }
}
