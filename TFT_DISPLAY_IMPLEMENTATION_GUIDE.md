# LovyanGFX TFT Display Implementation Guide

## Overview

This document provides a comprehensive code review and implementation guide for TFT display drivers in the LovyanGFX library. It focuses on three specific display controllers:

| Display | Resolution | Interface | Notes |
|---------|------------|-----------|-------|
| **GC9A01** | 240x240 | SPI | Round displays, no NOP command support |
| **ST7789** | 240x320 | SPI | Common rectangular displays |
| **ST77961** | 360x390 | SPI | Actually ST77916 (typo in library: 16→61) |

> **Important Note on ST77916/ST77961**: The library uses `ST77961` due to a historical typo. The actual chip is ST77916 (note: 16 vs 61). Users should reference `Panel_ST77961` when implementing ST77916 displays.

---

## Architecture Overview

### Class Hierarchy

```
IPanel (Interface)
    └── Panel_Device (Base with config, bus, touch support)
            └── Panel_LCD (LCD-specific operations)
                    ├── Panel_ST7789
                    ├── Panel_ST77961 (ST77916)
                    └── Panel_GC9xxx
                            └── Panel_GC9A01
```

### Key Source Files

| File | Purpose |
|------|---------|
| `src/lgfx/v1/Panel.hpp` | Base IPanel interface |
| `src/lgfx/v1/panel/Panel_Device.hpp` | Device configuration structure |
| `src/lgfx/v1/panel/Panel_LCD.hpp` | LCD command constants and base LCD operations |
| `src/lgfx/v1/panel/Panel_LCD.cpp` | Core LCD implementation (setWindow, writePixels, etc.) |
| `src/lgfx/v1/panel/Panel_GC9A01.hpp` | GC9A01 driver |
| `src/lgfx/v1/panel/Panel_ST7789.hpp` | ST7789 driver |
| `src/lgfx/v1/panel/Panel_ST77961.hpp` | ST77916/ST77961 driver |
| `src/lgfx/v1/Bus.hpp` | Bus interface (SPI, I2C, Parallel) |
| `src/lgfx/v1/platforms/esp32/Bus_SPI.cpp` | ESP32 SPI implementation |
| `src/lgfx/v1/platforms/rp2040/Bus_SPI.cpp` | RP2040 SPI implementation |
| `src/lgfx/v1/misc/pixelcopy.hpp` | Color conversion and pixel copying |

---

## Display Controller Specifics

### GC9A01 (Panel_GC9A01)

**Location**: `src/lgfx/v1/panel/Panel_GC9A01.hpp:78-151`

**Characteristics**:
- 240x240 pixels (circular display)
- **Critical**: Malfunctions when receiving NOP command (`_nop_closing = false`)
- Uses non-standard MADCTL (Memory Access Control) mapping
- 16-bit dummy read before pixel readout

**Initialization Sequence** (key registers):
```cpp
// Enable internal oscillator
0xEF, 0xEB (0x14), 0xFE, 0xEF

// Power settings
0x84-0x8F  // Internal power/voltage settings

// Display function control
0xB6 (0x00, 0x20)  // Display function control

// Frame rate control
0x90-0xBE  // Various frame rate and timing registers

// Gamma correction
0xF0, 0xF1, 0xF2, 0xF3  // Positive/negative gamma

// Tearing effect
0x35 (0x00)  // TE line ON

// Sleep out + Display on
0x11 (delay 120ms), 0x29
```

**Custom setWindow Implementation**:
```cpp
void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override
{
  // GC9A01 requires special handling for rotated modes
  if (_internal_rotation & 1)
  {
    _bus->writeCommand(CMD_RASET, 8);
    _bus->writeData(~0u, 32);  // Write dummy data for row set
  }
  // Standard CASET/RASET sequence follows...
}
```

**MADCTL Mapping** (differs from standard):
```cpp
static constexpr uint8_t madctl_table[] = {
  0,                        // 0: Normal
  MAD_MV|MAD_MX,           // 1: 90° rotation
  MAD_MX|MAD_MY,           // 2: 180° rotation
  MAD_MV|MAD_MY,           // 3: 270° rotation
  MAD_MY,                  // 4: Vertical flip
  MAD_MV,                  // 5: 90° + flip
  MAD_MX,                  // 6: 180° + flip
  MAD_MV|MAD_MX|MAD_MY,    // 7: 270° + flip
};
```

---

### ST7789 (Panel_ST7789)

**Location**: `src/lgfx/v1/panel/Panel_ST7789.hpp:28-87`

**Characteristics**:
- 240x320 pixels (typical)
- Standard SPI interface
- Uses standard Panel_LCD MADCTL mapping
- 16-bit dummy read before pixel readout

**Key Register Commands**:
```cpp
static constexpr uint8_t CMD_RAMCTRL  = 0xB0;  // RAM Control
static constexpr uint8_t CMD_PORCTRL  = 0xB2;  // Porch control
static constexpr uint8_t CMD_GCTRL    = 0xB7;  // Gate control
static constexpr uint8_t CMD_VCOMS    = 0xBB;  // VCOMS setting
static constexpr uint8_t CMD_LCMCTRL  = 0xC0;  // LCM control
static constexpr uint8_t CMD_VDVVRHEN = 0xC2;  // VDV and VRH enable
static constexpr uint8_t CMD_VRHS     = 0xC3;  // VRH set
static constexpr uint8_t CMD_VDVSET   = 0xC4;  // VDV setting
static constexpr uint8_t CMD_FRCTR2   = 0xC6;  // Frame rate control (0x0f = 60Hz)
static constexpr uint8_t CMD_PWCTRL1  = 0xD0;  // Power control 1
static constexpr uint8_t CMD_PVGAMCTRL= 0xE0;  // Positive gamma
static constexpr uint8_t CMD_NVGAMCTRL= 0xE1;  // Negative gamma
```

**Initialization Sequence**:
```cpp
CMD_PORCTRL, 5, 0x0c, 0x0c, 0x00, 0x33, 0x33,  // Porch timing
CMD_GCTRL,   1, 0x35,                           // Gate control
CMD_VCOMS,   1, 0x28,                           // VCOM = 1.1V
CMD_LCMCTRL, 1, 0x0C,                           // LCM control
CMD_VDVVRHEN,2, 0x01, 0xFF,                     // VDV/VRH enable
CMD_VRHS,    1, 0x10,                           // VRH = 4.1V
CMD_VDVSET,  1, 0x20,                           // VDV = 0V
CMD_FRCTR2,  1, 0x0f,                           // 60Hz frame rate
CMD_PWCTRL1, 2, 0xa4, 0xa1,                     // Power control
CMD_RAMCTRL, 2, 0x00, 0xC0,                     // RAM control
// 14-byte gamma curves for PVGAMCTRL and NVGAMCTRL
CMD_SLPOUT, delay 130ms,
CMD_IDMOFF,
CMD_DISPON,
```

---

### ST77961/ST77916 (Panel_ST77961)

**Location**: `src/lgfx/v1/panel/Panel_ST77961.hpp:28-279`

**Characteristics**:
- 360x390 pixels (AMOLED typical)
- Complex initialization with multiple command pages
- 8-bit dummy read (less than other displays)
- Custom color depth handling (RGB888 read/write)
- OTP (One-Time Programmable) memory access during init

**Key Register Commands**:
```cpp
static constexpr uint8_t CMD_GAMCTRP1 = 0xE0;  // Positive Gamma
static constexpr uint8_t CMD_GAMCTRN1 = 0xE1;  // Negative Gamma
static constexpr uint8_t CMD_CSC1     = 0xF0;  // Command Set Control 1
static constexpr uint8_t CMD_CSC2     = 0xF1;  // Command Set Control 2
static constexpr uint8_t CMD_CSC3     = 0xF2;  // Command Set Control 3
static constexpr uint8_t CMD_CSC4     = 0xF3;  // Command Set Control 4
```

**Command Page System**:
```cpp
// Enable test command page
CMD_CSC1, 1, 0x08
CMD_CSC3, 1, 0x08

// Switch between command pages
CMD_CSC1, 1, 0x01  // Command2 enable
CMD_CSC2, 1, 0x01  // Command2 enable

// Enter gamma correction mode
CMD_CSC1, 1, 0x02

// Enter GIP (Gate-In-Panel) mode
CMD_CSC4, 1, 0x10
```

**Power Configuration** (key voltage registers):
```cpp
0xB0: VRHPS  (Positive power: 5.750V)
0xB1: VRHNS  (Negative power: -3.450V)
0xB2: VCOMS  (VCOM GND: 1.150V)
0xB4: GAMOPPS (Gamma OP power)
0xB5: STEP14S (AVCL=-4.05V, AVDD=6.42V)
0xB6: STEP23S (VGL=-12.00V, VGH=12.0V)
```

**Custom Color Depth Handling**:
```cpp
void setColorDepth_impl(color_depth_t depth) override
{
  // ST77916 supports both RGB565 and RGB888
  _write_depth = ((int)depth & color_depth_t::bit_mask) > 16
                 ? rgb888_3Byte : rgb565_2Byte;
  _read_depth = _write_depth;  // Read matches write (unlike other displays)
}
```

**Memory Clear Sequence** (unique to ST77916):
```cpp
// Clear RAM to black
0x4D, 1, 0x00,  // RAMCLSETR - Clear RED
0x4E, 1, 0x00,  // RAMCLSETG - Clear GREEN
0x4F, 1, 0x00,  // RAMCLSETB - Clear BLUE
0x4C, 1+delay, 0x01, 10ms  // RAMCLACT - Trigger memory clear
```

---

## Performance Analysis & Bottlenecks

### 1. Window Setting Optimization

**Location**: `src/lgfx/v1/panel/Panel_LCD.cpp:450-470`

The `set_window_8()` function implements change detection to avoid redundant commands:

```cpp
void Panel_LCD::set_window_8(uint_fast16_t xs, uint_fast16_t ys,
                             uint_fast16_t xe, uint_fast16_t ye, uint32_t cmd)
{
  static constexpr uint32_t mask = 0xFF00FF;
  uint32_t x = xs + (xe << 16);

  // Only send CASET if X coordinates changed
  if (_xsxe != x) {
    _xsxe = x;
    _bus->writeCommand(CMD_CASET, 8);
    x += _colstart + (_colstart << 16);
    _bus->writeData(((x >> 8) & mask) + ((x & mask) << 8), 32);
  }

  // Only send RASET if Y coordinates changed
  uint32_t y = ys + (ye << 16);
  if (_ysye != y) {
    _ysye = y;
    _bus->writeCommand(CMD_RASET, 8);
    y += _rowstart + (_rowstart << 16);
    _bus->writeData(((y >> 8) & mask) + ((y & mask) << 8), 32);
  }

  _bus->writeCommand(cmd, 8);
}
```

**Performance Impact**: This caching mechanism reduces SPI traffic by ~50% for sequential pixel operations in the same row/column.

### 2. DMA Transfer Optimization

**Location**: `src/lgfx/v1/platforms/esp32/Bus_SPI.cpp:549-636`

**Key Thresholds**:
```cpp
// Data ≤64 bytes: Direct register write (faster for small transfers)
if (length <= 64) {
  memcpy((void*)spi_w0_reg, data, aligned_len);
  exec_spi();
  return;
}

// Data >64 bytes with DMA: Use DMA transfer
if (_cfg.dma_channel && use_dma) {
  // Setup DMA descriptor chain
  _setup_dma_desc_links(data, length);
  // ...
}
```

**DMA Burst Mode Consideration**:
```cpp
// Burst mode only safe when length is 4-byte aligned
// Bug: DMA corruption at 80MHz with unaligned data
dma_conf |= (length & 3)
  ? (SPI_OUTDSCR_BURST_EN)                          // No data burst
  : (SPI_OUTDSCR_BURST_EN | SPI_OUT_DATA_BURST_EN); // Full burst
```

### 3. Pixel Copy Performance

**Location**: `src/lgfx/v1/misc/pixelcopy.hpp:160-177`

Optimized fast path for same-format copies:
```cpp
template <typename TDst, typename TSrc>
static uint32_t copy_rgb_fast(void* dst, uint32_t index,
                               uint32_t last, pixelcopy_t* param)
{
  if (std::is_same<TDst, TSrc>::value) {
    // Direct memcpy when source and destination formats match
    memcpy(reinterpret_cast<void*>(&d[index]),
           reinterpret_cast<const void*>(&s[index]),
           (last - index) * sizeof(TSrc));
  }
  // ... color conversion fallback
}
```

### 4. writeDataRepeat Optimization

**Location**: `src/lgfx/v1/platforms/esp32/Bus_SPI.cpp:342-443`

**Ring Buffer Technique**:
```cpp
// For 16-bit colors: ESP32 SPI buffer acts as ring buffer
// Allows specifying larger transfer size than physical buffer
#if defined ( CONFIG_IDF_TARGET_ESP32 )
  limit = (1 << 11);  // 2048 bits = 256 bytes (128 pixels @ 16-bit)
#else
  limit = (1 << 9);   // 512 bits for other ESP32 variants
#endif
```

### 5. Transaction Management

**Location**: `src/lgfx/v1/panel/Panel_LCD.cpp:56-89`

**NOP Closing for Bus Sharing**:
```cpp
void Panel_LCD::end_transaction(void)
{
  // NOP command allows clean bus release for SD card sharing
  if (_nop_closing) {
    write_command(_cmd_nop);
  }
  _bus->endTransaction();
  cs_control(true);
}
```

**Warning**: GC9A01 sets `_nop_closing = false` because the chip malfunctions on NOP.

---

## Performance Bottlenecks Identified

### Critical Bottlenecks

1. **Color Format Conversion** (`pixelcopy.hpp:243-263`)
   - Every pixel through `color_convert<>` when formats don't match
   - **Solution**: Use `no_convert = true` when source matches display format

2. **Small Transfer Overhead** (`Bus_SPI.cpp:549-560`)
   - Transfers ≤64 bytes bypass DMA but require CPU wait
   - Optimal batch size: 256-4096 bytes for DMA efficiency

3. **Window Command Redundancy**
   - `setWindow()` called for every operation
   - Mitigated by internal caching but still overhead
   - **Best Practice**: Use `writePixels()` for bulk operations

### Platform-Specific Issues

**ESP32**:
- DMA data corruption at 80MHz with non-4-byte-aligned lengths
- HIGHPART register not available on ESP32-C3/S3 (different code path)

**RP2040**:
- No native DMA queue support (`addDMAQueue` falls back to `writeBytes`)
- FIFO limited to 8 entries, causing CPU stalls on large transfers

---

## Implementation Best Practices

### 1. Optimal Configuration

```cpp
// ESP32 optimal SPI config
Bus_SPI::config_t bus_cfg;
bus_cfg.freq_write = 80000000;  // 80MHz (max stable)
bus_cfg.freq_read  = 16000000;  // 16MHz for reliable reads
bus_cfg.dma_channel = SPI_DMA_CH_AUTO;  // Enable DMA
bus_cfg.spi_host = VSPI_HOST;

// Panel optimal config
Panel_Device::config_t panel_cfg;
panel_cfg.memory_width  = 240;  // Match actual display
panel_cfg.memory_height = 320;
panel_cfg.panel_width   = 240;
panel_cfg.panel_height  = 320;
panel_cfg.readable      = true;  // Enable if hardware supports
panel_cfg.bus_shared    = false; // Set true only if sharing with SD
```

### 2. High-Performance Drawing

```cpp
// BAD: Individual pixel writes
for (int y = 0; y < 100; y++) {
  for (int x = 0; x < 100; x++) {
    display.drawPixel(x, y, color);  // Calls setWindow() each time!
  }
}

// GOOD: Batch pixel writes
display.startWrite();
display.setAddrWindow(0, 0, 100, 100);
for (int i = 0; i < 10000; i++) {
  display.writeColor(color);  // Direct to frame buffer
}
display.endWrite();

// BEST: Use sprites for complex graphics
LGFX_Sprite sprite(&display);
sprite.createSprite(100, 100);
// Draw to sprite (memory operations)
sprite.fillRect(0, 0, 100, 100, color);
// Single DMA transfer to display
sprite.pushSprite(0, 0);
```

### 3. Color Format Optimization

```cpp
// Pre-convert colors to display format for fastest transfer
// Display uses swap565 (RGB565 byte-swapped)
pixelcopy_t pc(buffer, rgb565_2Byte, rgb565_2Byte);
pc.no_convert = true;  // Skip conversion

// Or use display's native format
auto color = display.color565(r, g, b);  // Returns swap565_t
```

### 4. DMA Buffer Management

```cpp
// Use getDMABuffer for zero-copy transfers
uint8_t* buf = bus.getDMABuffer(length);
// Fill buffer with pixel data
memcpy(buf, src, length);
// Transfer without additional copy
bus.writeBytes(buf, length, true, true);
```

---

## Command Reference

### Common LCD Commands (Panel_LCD.hpp:101-124)

| Command | Value | Description |
|---------|-------|-------------|
| CMD_NOP | 0x00 | No operation |
| CMD_SWRESET | 0x01 | Software reset |
| CMD_SLPIN | 0x10 | Sleep in |
| CMD_SLPOUT | 0x11 | Sleep out |
| CMD_INVOFF | 0x20 | Inversion off |
| CMD_INVON | 0x21 | Inversion on |
| CMD_DISPOFF | 0x28 | Display off |
| CMD_DISPON | 0x29 | Display on |
| CMD_CASET | 0x2A | Column address set |
| CMD_RASET | 0x2B | Row address set |
| CMD_RAMWR | 0x2C | Memory write |
| CMD_RAMRD | 0x2E | Memory read |
| CMD_MADCTL | 0x36 | Memory access control |
| CMD_COLMOD | 0x3A | Pixel format set |

### MADCTL Bits (Panel_LCD.hpp:67-77)

| Bit | Name | Description |
|-----|------|-------------|
| 0x80 | MAD_MY | Row address order |
| 0x40 | MAD_MX | Column address order |
| 0x20 | MAD_MV | Row/Column exchange |
| 0x10 | MAD_ML | Vertical refresh order |
| 0x08 | MAD_BGR | BGR color order |
| 0x04 | MAD_MH | Horizontal refresh order |

### COLMOD Values (Panel_LCD.hpp:79-83)

| Value | Name | Description |
|-------|------|-------------|
| 0x33 | RGB444_12bit | 12-bit color (4096 colors) |
| 0x55 | RGB565_2BYTE | 16-bit color (65K colors) |
| 0x66 | RGB888_3BYTE | 24-bit color (16M colors) |

---

## Initialization Timing Requirements

| Display | Reset Low | Reset High | Sleep Out Delay | Display On |
|---------|-----------|------------|-----------------|------------|
| GC9A01 | 10ms | 120ms | 120ms | immediate |
| ST7789 | 10ms | 120ms | 130ms | immediate |
| ST77961 | 10ms | 120ms | 120ms | immediate |

---

## Troubleshooting Guide

### Display Shows Garbage/Noise

1. Check SPI frequency (reduce to 20MHz for testing)
2. Verify MADCTL configuration matches panel orientation
3. Check color depth setting matches display capabilities
4. Ensure proper power sequencing

### Colors Inverted/Wrong

1. Toggle `_cfg.invert` setting
2. Check `_cfg.rgb_order` (RGB vs BGR)
3. Verify COLMOD setting

### Display Blank/White

1. Check backlight control (Light_PWM)
2. Verify SLPOUT command was sent
3. Check DISPON command was sent
4. Measure power supply voltages

### Partial Display/Offset

1. Check `offset_x` and `offset_y` settings
2. Verify `memory_width/height` matches driver capability
3. Check `panel_width/height` matches visible area

### Slow Performance

1. Enable DMA (`dma_channel = SPI_DMA_CH_AUTO`)
2. Increase SPI frequency
3. Use batch operations (`startWrite/endWrite`)
4. Use sprites for complex graphics
5. Match source color format to display format

---

## Platform Compatibility Matrix

| Platform | SPI DMA | Max SPI Freq | Parallel Bus | Notes |
|----------|---------|--------------|--------------|-------|
| ESP32 | Yes | 80MHz | 8/16-bit | Best performance |
| ESP32-S2 | Yes | 80MHz | 8/16-bit | No HIGHPART |
| ESP32-S3 | Yes | 80MHz | 8/16-bit + RGB | Best for RGB panels |
| ESP32-C3 | Yes | 80MHz | 8-bit | Limited GPIO |
| RP2040 | Limited | 62.5MHz | No | Simple DMA only |
| SAMD21 | No | 12MHz | No | Slow but stable |
| SAMD51 | Yes | 24MHz | No | Better than SAMD21 |

---

## Version Information

- Library Version: 1.2.7
- Last Updated: 2024
- Supported Arduino-ESP32: v2.x, v3.x
- Supported ESP-IDF: v4.4+, v5.x

---

## License

This documentation is based on the LovyanGFX library which is licensed under FreeBSD license.

Original Source: https://github.com/lovyan03/LovyanGFX

Authors:
- [lovyan03](https://twitter.com/lovyan03)
- Contributors: ciniml, mongonta0716, tobozo
