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
#include "Panel_Device.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  Panel_Device::Panel_Device(void)
  {
    bus(nullptr);
  }

  void Panel_Device::bus(IBus* bus)
  {
    static Bus_NULL nullobj;
    _bus = bus ? bus : &nullobj;
  }

//----------------------------------------------------------------------------

  void Panel_Device::init(bool use_reset)
  {
    init_rst();
    if (use_reset)
    {
      reset();
      delay(10);
    }
    init_cs();
    _bus->init();
  }

  void Panel_Device::initDMA(void)
  {
    _bus->initDMA();
  }
  void Panel_Device::waitDMA(void)
  {
    _bus->wait();
  }
  bool Panel_Device::dmaBusy(void)
  {
    return _bus->busy();
  }

  void Panel_Device::writeCommand(std::uint32_t data, std::uint_fast8_t length)
  {
    if (_cfg.dlen_16bit)
    {
      if (_align_data)
      {
        _align_data = false;
        _bus->writeData(0, 8);
      }
      if (length == 1) { length = 2; data <<= 8; }
    }
    _bus->writeCommand(data, length << 3);
  }

  void Panel_Device::writeData(std::uint32_t data, std::uint_fast8_t length)
  {
    if (!_cfg.dlen_16bit)
    {
      _bus->writeData(data, length << 3);
    }
    else
    {
      if (length == 1)
      {
        _bus->writeData(data << 8, 16);
      }
      else
      {
        _bus->writeData(data, length << 3);
      }
    }
  }
/*
  void Panel_Device::writeBytes(const std::uint8_t* data, std::uint32_t len, bool use_dma)
  {
    _bus->writeBytes(data, len, use_dma);
    if (_cfg.dlen_16bit && (_write_bits & 15) && (len & 1))
    {
      _align_data = !_align_data;
    }
  }
*/

//----------------------------------------------------------------------------

  void Panel_Device::writeImageARGB(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param)
  {
    auto src_x = param->src_x;
    auto buffer = reinterpret_cast<argb8888_t*>(const_cast<void*>(param->src_data));
    auto bytes = param->dst_bits >> 3;
// ESP_LOGI("LGFX","DEBUG: %d %d", param->dst_bits, bytes);
    // std::uint8_t* dmabuf = _bus->getFlipBuffer(w * bytes);
    // memset(dmabuf, 0, w * bytes);
    // param->fp_copy(dmabuf, 0, w, param);
    // setWindow(x, y, x + w - 1, y);
    // writeBytes(dmabuf, w * bytes, true);
    // return;
    pixelcopy_t pc_read(nullptr, _write_depth, _read_depth);
    pixelcopy_t pc_write(nullptr, _write_depth, _write_depth);
    for (;;)
    {
      std::uint8_t* dmabuf = _bus->getDMABuffer((w+1) * bytes);
      pc_write.src_data = dmabuf;
      std::uint32_t xstart = 0, drawed_x = 0;
      do
      {
        std::uint_fast8_t a = buffer[xstart].a;
        if (!a)
        {
          if (drawed_x < xstart)
          {
            param->src_x = drawed_x;
            param->fp_copy(dmabuf, drawed_x, xstart, param);

            pc_write.src_x = drawed_x;
            writeImage(x + drawed_x, y, xstart - drawed_x, 1, &pc_write, true);
          }
          drawed_x = xstart + 1;
        }
        else
        {
          while (255 == buffer[xstart].a && ++xstart != w);
          if (xstart == w) break;
          std::uint32_t j = xstart;
          while (++j != w && buffer[j].a && buffer[j].a != 255);
          readRect(x + xstart, y, j - xstart + 1, 1, &dmabuf[xstart * bytes], &pc_read);
          if (w == (xstart = j)) break;
        }
      } while (++xstart != w);
      if (drawed_x < xstart)
      {
        param->src_x = drawed_x;
        param->fp_copy(dmabuf, drawed_x, xstart, param);

        pc_write.src_x = drawed_x;
        writeImage(x + drawed_x, y, xstart - drawed_x, 1, &pc_write, true);
      }
      if (!--h) return;
      param->src_x = src_x;
      param->src_y++;
      ++y;
    }
  }

  void Panel_Device::copyRect(std::uint_fast16_t dst_x, std::uint_fast16_t dst_y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint_fast16_t src_x, std::uint_fast16_t src_y)
  {
    pixelcopy_t pc_read( (void*)nullptr, _write_depth, _read_depth);
    pixelcopy_t pc_write((void*)nullptr, _write_depth, _write_depth);
    std::size_t write_bytes = (_write_depth + 7) >> 3;
    startWrite();

    auto dir = get_fastread_dir();
    if (dir == fastread_dir_t::fastread_vertical
    || (dir == fastread_dir_t::fastread_nothing && (w < h)))
    {
      const std::uint32_t buflen = h * write_bytes;
      std::uint8_t buf[buflen];
      pc_write.src_data = buf;
      pc_write.src_width = 1;
      pc_write.src_bitwidth = 1;
      std::int32_t add = (src_x < dst_x) ?   - 1 : 1;
      std::int32_t pos = (src_x < dst_x) ? w - 1 : 0;
      do {
        readRect(src_x + pos, src_y, 1, h, buf, &pc_read);
        pc_write.src_x = 0;
        pc_write.src_y = 0;
        writeImage(dst_x + pos, dst_y, 1, h, &pc_write, true);
        //setWindow(dst_x + pos, dst_y, dst_x + pos, dst_y);
        //writePixels(&pc_write, h);
        pos += add;
      } while (--w);
      waitDMA();
    }
    else
    {
      const std::uint32_t buflen = w * write_bytes;
      std::uint8_t buf[buflen];
      pc_write.src_data = buf;
      std::int32_t add = (src_y < dst_y) ?   - 1 : 1;
      std::int32_t pos = (src_y < dst_y) ? h - 1 : 0;
      do {
        readRect(src_x, src_y + pos, w, 1, buf, &pc_read);
        pc_write.src_x = 0;
        pc_write.src_y = 0;
        writeImage(dst_x, dst_y + pos, w, 1, &pc_write, true);
        pos += add;
      } while (--h);
      waitDMA();
    }
    endWrite();
  }

//----------------------------------------------------------------------------

  void Panel_Device::init_cs(void)
  {
    gpio_hi(_cfg.pin_cs);
    pinMode(_cfg.pin_cs , pin_mode_t::output);
  }

  void Panel_Device::init_rst(void)
  {
    gpio_hi(_cfg.pin_rst);
    pinMode(_cfg.pin_rst, pin_mode_t::output);
  }

  void Panel_Device::cs_control(bool level)
  {
    auto pin = _cfg.pin_cs;
    if (level)
    {
      gpio_hi(pin);
    }
    else
    {
      gpio_lo(pin);
    }
  }

  void Panel_Device::reset(void)
  {
    gpio_lo(_cfg.pin_rst);
    delay(4);
    gpio_hi(_cfg.pin_rst);
  }

//----------------------------------------------------------------------------

  void Panel_Device::command_list(const std::uint8_t *addr)
  {
    for (;;)
    {                // For each command...
      if (*reinterpret_cast<const std::uint16_t*>(addr) == 0xFFFF) break;
      writeCommand(*addr++, 1);  // Read, issue command
      std::uint_fast8_t numArgs = *addr++;  // Number of args to follow
      std::uint_fast8_t ms = numArgs & CMD_INIT_DELAY;       // If hibit set, delay follows args
      numArgs &= ~CMD_INIT_DELAY;          // Mask out delay bit
      if (numArgs)
      {
        do
        {                   // For each argument...
          writeData(*addr++, 1);  // Read, issue argument
        } while (--numArgs);
      }
      if (ms)
      {
        ms = *addr++;        // Read post-command delay time (ms)
        delay( (ms==255 ? 500 : ms) );
      }
    }
  }

//----------------------------------------------------------------------------
 }
}
