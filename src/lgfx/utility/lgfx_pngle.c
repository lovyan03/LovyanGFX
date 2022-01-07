/*-
 * MIT License
 *
 * Copyright (c) 2019 kikuchan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*----------------------------------------------------------------------------/
/ original source is here : https://github.com/kikuchan/pngle/
/
/ Modified for LGFX  by lovyan03, 2020
/ tweak for 32bit processor
/----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "miniz.h"
#include "lgfx_pngle.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifdef PNGLE_DEBUG
#define debug_printf(...) fprintf(stderr, __VA_ARGS__)
#else
#define debug_printf(...) ((void)0)
#endif

#define PNGLE_ERROR(s) (pngle->error = (s), pngle->state = PNGLE_STATE_ERROR, -1)
#define PNGLE_MALLOC(a, b, name) (debug_printf("[pngle] Allocating %zu bytes for %s\n", (size_t)(a) * (size_t)(b), (name)), malloc((size_t)(a) * (size_t)(b)))

#define PNGLE_UNUSED(x) (void)(x)

typedef enum {
  PNGLE_STATE_ERROR = -2,
  PNGLE_STATE_EOF = -1,
  PNGLE_STATE_INITIAL = 0,

  PNGLE_STATE_FIND_CHUNK_HEADER,
  PNGLE_STATE_HANDLE_CHUNK,
  PNGLE_STATE_CRC,
} pngle_state_t;

typedef enum {
// Supported chunks
//   Filter chunk names by following command to (re)generate hex constants;
//     % perl -ne 'chomp; s/.*\s*\/\/\s*//; print "\tPNGLE_CHUNK_$_ = 0x" . unpack("H*") . "UL, // $_\n";'
  PNGLE_CHUNK_IHDR = 0x49484452UL, // IHDR
  PNGLE_CHUNK_PLTE = 0x504c5445UL, // PLTE
  PNGLE_CHUNK_IDAT = 0x49444154UL, // IDAT
  PNGLE_CHUNK_IEND = 0x49454e44UL, // IEND
  PNGLE_CHUNK_tRNS = 0x74524e53UL, // tRNS
  PNGLE_CHUNK_gAMA = 0x67414d41UL, // gAMA
} pngle_chunk_t;

// typedef struct _pngle_t pngle_t; // declared in pngle.h
struct _pngle_t {
  // PLTE chunk
  uint8_t *palette;
  size_t n_palettes;

  // tRNS chunk
  uint8_t *trans_palette;
  size_t n_trans_palettes;

  // scanline decoder (reset on every set_interlace_pass() call)
  uint8_t *scanline_flipbuf[2];
  uint8_t *scanline_buf;
  uint32_t drawing_y;
  size_t scanline_pixels;
  size_t scanline_stride;
  size_t scanline_remain_bytes_to_render;

  void *user_data;
  pngle_draw_callback_t draw_callback;
  pngle_init_callback_t init_callback;
  pngle_done_callback_t done_callback;

  // parser state (reset on every chunk header)
  uint32_t chunk_type;
  uint32_t chunk_remain;

  // for pngle_draw_pixels
  uint32_t magni;

  pngle_ihdr_t hdr;

  pngle_state_t state;

  int8_t filter_type;
  // interlace
  uint8_t interlace_pass;

  // 0 indicates IHDR hasn't been processed yet
  uint8_t channels;

#ifndef PNGLE_NO_GAMMA_CORRECTION
  uint8_t *gamma_table;
  double display_gamma;
#endif

  const char *error;
  // decompression state (reset on IHDR)
  uint8_t *next_out; // NULL indicates IDAT hasn't been processed yet
  size_t  avail_out;
  tinfl_decompressor inflator; // 11000 bytes
  uint8_t lz_buf[TINFL_LZ_DICT_SIZE]; // 32768 bytes
};

// magic
static const uint8_t png_sig[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
static const uint8_t interlace_div_y[9] = { 8, 8, 8, 4, 4, 2, 2, 1, 1 };
static const uint8_t interlace_off_y[9] = { 0, 0, 4, 0, 2, 0, 1, 0, 0 };
static const uint8_t* interlace_off_x = &interlace_off_y[1];
static const uint8_t* interlace_div_x = &interlace_div_y[1];


static inline uint8_t  read_uint8(const uint8_t *p)
{
  return *p;
}

static inline uint32_t read_uint32(const uint8_t *p)
{
  return (p[0] << 24)
       | (p[1] << 16)
       | (p[2] <<  8)
       | (p[3] <<  0)
  ;
}

void lgfx_pngle_reset(pngle_t *pngle)
{
  if (!pngle) return ;

  pngle->state = PNGLE_STATE_INITIAL;
  pngle->error = "No error";

  if (pngle->scanline_buf ) { free(pngle->scanline_buf ); pngle->scanline_buf = NULL; }
  if (pngle->palette      ) { free(pngle->palette      ); pngle->palette = NULL; }
  if (pngle->trans_palette) { free(pngle->trans_palette); pngle->trans_palette = NULL; }

#ifndef PNGLE_NO_GAMMA_CORRECTION
  if (pngle->gamma_table     ) { free(pngle->gamma_table     ); pngle->gamma_table = NULL; }
#endif

  pngle->channels = 0; // indicates IHDR hasn't been processed yet
  pngle->next_out = NULL; // indicates IDAT hasn't been processed yet

  // clear them just in case...
  memset(&pngle->hdr, 0, sizeof(pngle->hdr));
  pngle->n_palettes = 0;
  pngle->n_trans_palettes = 0;

  tinfl_init(&pngle->inflator);
}

pngle_t *lgfx_pngle_new()
{
  pngle_t *pngle = (pngle_t *)PNGLE_MALLOC(1, sizeof(pngle_t), "pngle_t");
  if (!pngle) return NULL;

  memset(pngle, 0, 64);
  pngle->scanline_buf = NULL;
  pngle->palette = NULL;
  pngle->trans_palette = NULL;
  pngle->error = "No error";
  pngle->state = PNGLE_STATE_INITIAL;
  pngle->next_out = NULL;
  pngle->avail_out = 0;
  pngle->channels = 0;
  tinfl_init(&pngle->inflator);

  return pngle;
}

void lgfx_pngle_destroy(pngle_t *pngle)
{
  if (pngle) {
    if (pngle->scanline_buf ) { free(pngle->scanline_buf ); }
    if (pngle->palette      ) { free(pngle->palette      ); }
    if (pngle->trans_palette) { free(pngle->trans_palette); }
    free(pngle);
  }
}

const char *lgfx_pngle_error(pngle_t *pngle)
{
  if (!pngle) return "Uninitialized";
  return pngle->error;
}

uint32_t lgfx_pngle_get_width(pngle_t *pngle)
{
  if (!pngle) return 0;
  return pngle->hdr.width;
}

uint32_t lgfx_pngle_get_height(pngle_t *pngle)
{
  if (!pngle) return 0;
  return pngle->hdr.height;
}

pngle_ihdr_t *lgfx_pngle_get_ihdr(pngle_t *pngle)
{
  if (!pngle) return NULL;
  if (pngle->channels == 0) return NULL;
  return &pngle->hdr;
}

static int pngle_draw_pixels(pngle_t *pngle, uint8_t* buf, uint32_t* rgbbuf)
{
  size_t depth = pngle->hdr.depth;
  size_t pixels = pngle->scanline_pixels;
  uint32_t* rgba32 = rgbbuf - 1;
  uint32_t* last = &rgba32[pixels];
  if (depth >= 8) {
    size_t add = depth >> 3;
    switch (pngle->channels)
    {
    case 1: // grayscale or palette
      do {
        *++rgba32 = buf[0];
        buf += add;
      } while (rgba32 != last);
      break;

    case 2: // grayscale with alpha
      do {
        /// grayscale to color (* 0x010101) + alpha channel
        *++rgba32 = buf[0]*0x01010100 + buf[add];
        buf += add * 2;
      } while (rgba32 != last);
      break;
    
    case 3: // color
      do {
        *++rgba32 = 0xFF
                  + (buf[    0] <<  8)
                  + (buf[add  ] << 16)
                  + (buf[add*2] << 24);
        buf += add * 3;
      } while (rgba32 != last);
      break;

    default: //4 : color + alpha
      do {
        // *++rgba32 = *(uint32_t*)buf;
        *++rgba32 = (buf[    0] <<  8)
                  + (buf[add  ] << 16)
                  + (buf[add*2] << 24)
                  + (buf[add*3]      );
        buf += add * 4;
      } while (rgba32 != last);
      break;
    }
  } else {
    size_t mask = ((1 << depth) - 1);
    size_t shift = 8;
    size_t b = buf[0];
    do {
      shift -= depth;
      *++rgba32 = (b >> shift) & mask;
      if (shift == 0) {
        shift = 8;
        b = *++buf;
      }
    } while (rgba32 != last);
  }

  rgba32 = rgbbuf - 1;
  pixels = pngle->scanline_pixels;
  // color type: 0000 0111
  //                     ^-- indexed color (palette)
  //                    ^--- Color
  //                   ^---- Alpha channel
  switch (pngle->hdr.color_type)
  {
  case 0: // grayscale
    {
      uint32_t tp = (pngle->n_trans_palettes != 1)
                  ? ~0u
                  : pngle->trans_palette[0 * 2 + 1];
      uint32_t magni = pngle->magni;
      do {
        uint32_t tmp = *++rgba32;
        *rgba32 = ((tmp == tp) ? 0 : 0xFF) + (tmp * magni);
      } while (--pixels);
    }
    break;

  case 2: // true color
    if (pngle->n_trans_palettes == 1)
    {
      uint32_t tp = (pngle->trans_palette[0 * 2 + 1] <<  8)
                  + (pngle->trans_palette[1 * 2 + 1] << 16)
                  + (pngle->trans_palette[2 * 2 + 1] << 24)
                  + 0xFF;
      do {
        if (*++rgba32 == tp) { ((uint8_t*)rgba32)[0] = 0; }
      } while (--pixels);
    }
    break;

  case 3: // indexed color
    do
    {  // lookup palette info
      size_t pidx = *++rgba32;
      uint_fast32_t tmp = (pidx < pngle->n_trans_palettes ? pngle->trans_palette[pidx] : 0xFF);
      pidx *= 3;
      *rgba32 = ( pngle->palette[pidx + 0] <<  8 )
              + ( pngle->palette[pidx + 1] << 16 )
              + ( pngle->palette[pidx + 2] << 24 )
              + tmp;
    } while (--pixels);
    break;

//case 4: // grayscale with alpha ...nothing to do.

  default:
    break;
  }

#ifndef PNGLE_NO_GAMMA_CORRECTION
  if (pngle->gamma_table) {
    rgba = rgbbuf;
    pixels = pngle->scanline_pixels;
    do {
      for (int i = 0; i < 3; i++) {
        rgba[i] = pngle->gamma_table[rgba[i]];
      }
      rgba += 4;
    } while (--pixels);
  }
#endif

  if (pngle->draw_callback) {
    pngle->draw_callback(pngle, interlace_off_x[pngle->interlace_pass], pngle->drawing_y, interlace_div_x[pngle->interlace_pass], pngle->scanline_pixels, (const uint8_t*)rgbbuf);
  }
  return 0;
}
//*/

static inline int paeth(int a, int b, int c)
{
  int pa = (b - c);
  int pb = (a - c);
  int pc = abs(pa + pb);
  pa = abs(pa);
  pb = abs(pb);
  if (pb < pa) { pa = pb; a = b; }
  return (pc < pa) ? c : a;
}

static void set_interlace_pass(pngle_t *pngle, uint_fast8_t pass)
{
  while (pngle->hdr.width <= interlace_off_x[pass] || pngle->hdr.height <= interlace_off_y[pass])
  {
    ++pass;
  }

  pngle->interlace_pass = pass;
  pngle->drawing_y = interlace_off_y[pass];

  size_t scanline_pixels = (pngle->hdr.width - interlace_off_x[pass] + interlace_div_x[pass] - 1) / interlace_div_x[pass];
  pngle->scanline_pixels = scanline_pixels;
  size_t scanline_stride = (scanline_pixels * pngle->channels * pngle->hdr.depth + 7) >> 3;
  pngle->scanline_stride = scanline_stride;
  pngle->scanline_remain_bytes_to_render = scanline_stride;
}

static int setup_gamma_table(pngle_t *pngle, uint32_t png_gamma)
{
#ifndef PNGLE_NO_GAMMA_CORRECTION
  if (pngle->gamma_table) free(pngle->gamma_table);

  if (pngle->display_gamma <= 0) return 0; // disable gamma correction
  if (png_gamma == 0) return 0;

  uint_fast8_t pixel_depth = (pngle->hdr.color_type & 1) ? 8 : pngle->hdr.depth;
  uint_fast16_t maxval = (1UL << pixel_depth) - 1;

  pngle->gamma_table = PNGLE_MALLOC(1, maxval + 1, "gamma table");
  if (!pngle->gamma_table) return PNGLE_ERROR("Insufficient memory");

  for (int i = 0; i < maxval + 1; i++) {
    pngle->gamma_table[i] = (uint8_t)floor(pow(i / (double)maxval, 100000.0 / png_gamma / pngle->display_gamma) * 255.0 + 0.5);
  }
  debug_printf("[pngle] gamma value = %d\n", png_gamma);
#else
  PNGLE_UNUSED(pngle);
  PNGLE_UNUSED(png_gamma);
#endif
  return 0;
}


static int pngle_on_data(pngle_t *pngle, const uint8_t *p, int len)
{
  // const uint8_t *ep = p + len;

  uint_fast8_t bytes_per_pixel = (pngle->channels * pngle->hdr.depth + 7) >> 3; // 1 if depth <= 8
  int8_t filter_type = pngle->filter_type;
  size_t remain_bytes = pngle->scanline_remain_bytes_to_render;

  uint8_t* prev_buf    = pngle->scanline_flipbuf[0];
  uint8_t* current_buf = pngle->scanline_flipbuf[1];
  while (len) {
    if (filter_type < 0) {
      filter_type = (int8_t)*p++; // 0 - 4
      if (filter_type > 4) {
        debug_printf("[pngle] Invalid filter type is found; 0x%02x\n", *p);
        return PNGLE_ERROR("Invalid filter type is found");
      }
      if (--len == 0) break;
    }
    size_t cidx = pngle->scanline_stride - remain_bytes;

    size_t l = (len < remain_bytes) ? len : remain_bytes;
    remain_bytes -= l;
    len -= l;

    const uint8_t* newdata = p - cidx;
    p += l;
    size_t last = cidx + l;

    switch (filter_type) {
    default:
      memcpy(&current_buf[cidx], &newdata[cidx], l);
      cidx = last;
      break;

    case 1:
      do {
        current_buf[cidx] = newdata[cidx] + current_buf[cidx - bytes_per_pixel];
      } while (++cidx != last);
      break;

    case 2:
      do {
        current_buf[cidx] = newdata[cidx] + prev_buf[cidx];
      } while (++cidx != last);
      break;

    case 3:
      do {
        current_buf[cidx] = newdata[cidx] + ((current_buf[cidx - bytes_per_pixel] + prev_buf[cidx]) >> 1);
      } while (++cidx != last);
      break;

    case 4:
      do {
        current_buf[cidx] = newdata[cidx] + paeth(current_buf[cidx - bytes_per_pixel], prev_buf[cidx], prev_buf[cidx - bytes_per_pixel]);
      } while (++cidx != last);
      break;
    }

    if (remain_bytes)
    {
      break;
    }

    remain_bytes = pngle->scanline_stride; // reset

    if (pngle_draw_pixels(pngle, current_buf, (uint32_t*)prev_buf) < 0) return -1;

    uint8_t* tmp = prev_buf;
    prev_buf = current_buf;
    current_buf = tmp;
    filter_type = -1;

    pngle->drawing_y += interlace_div_y[pngle->interlace_pass];
    if (pngle->drawing_y >= pngle->hdr.height) {
      if (pngle->interlace_pass >= 6) return 0; // Do nothing further

      // Interlace: Next pass
      set_interlace_pass(pngle, pngle->interlace_pass + 1);
      debug_printf("[pngle] interlace pass changed to: %d\n", pngle->interlace_pass);
      remain_bytes = pngle->scanline_stride; // reset
      memset(prev_buf, 0, remain_bytes);
    }
  }
  pngle->scanline_flipbuf[0] = prev_buf;
  pngle->scanline_flipbuf[1] = current_buf;
  pngle->scanline_remain_bytes_to_render = remain_bytes;
  pngle->filter_type = filter_type;
  
  return 0;
}


static int pngle_handle_chunk(pngle_t *pngle, const uint8_t *buf, size_t len)
{
  size_t consume = 0;

  switch (pngle->chunk_type) {
  case PNGLE_CHUNK_IDAT:
    // parse & decode IDAT chunk
    if (len < 1) return 0;

    debug_printf("[pngle]   Reading IDAT (len %zd / chunk remain %u)\n", len, pngle->chunk_remain);

    size_t in_bytes  = len;
    size_t out_bytes = pngle->avail_out;

    //debug_printf("[pngle]     in_bytes %zd, out_bytes %zd, next_out %p\n", in_bytes, out_bytes, pngle->next_out);

    // XXX: tinfl_decompress always requires (next_out - lz_buf + avail_out) == TINFL_LZ_DICT_SIZE
    tinfl_status status = tinfl_decompress(&pngle->inflator, (const mz_uint8 *)buf, &in_bytes, pngle->lz_buf, (mz_uint8 *)pngle->next_out, &out_bytes, TINFL_FLAG_HAS_MORE_INPUT | TINFL_FLAG_PARSE_ZLIB_HEADER);

    //debug_printf("[pngle]       tinfl_decompress\n");
    //debug_printf("[pngle]       => in_bytes %zd, out_bytes %zd, next_out %p, status %d\n", in_bytes, out_bytes, pngle->next_out, status);

    if (status < TINFL_STATUS_DONE) {
      // Decompression failed.
      debug_printf("[pngle] tinfl_decompress() failed with status %d!\n", status);
      return PNGLE_ERROR("Failed to decompress the IDAT stream");
    }

    pngle->next_out   += out_bytes;
    pngle->avail_out  -= out_bytes;

    // debug_printf("[pngle]         => avail_out %zd, next_out %p\n", pngle->avail_out, pngle->next_out);

    if (status == TINFL_STATUS_DONE || pngle->avail_out == 0) {
      // Output buffer is full, or decompression is done, so write buffer to output file.
      // XXX: This is the only chance to process the buffer.
      uint8_t *read_ptr = pngle->lz_buf;
      size_t n = TINFL_LZ_DICT_SIZE - (size_t)pngle->avail_out;

      // pngle_on_data() usually returns n, otherwise -1 on error
      if (pngle_on_data(pngle, read_ptr, n) < 0) return -1;

      // XXX: tinfl_decompress always requires (next_out - lz_buf + avail_out) == TINFL_LZ_DICT_SIZE
      pngle->next_out = pngle->lz_buf;
      pngle->avail_out = TINFL_LZ_DICT_SIZE;
    }

    consume = in_bytes;
    break;

  case PNGLE_CHUNK_IHDR:
    // parse IHDR
    consume = 13;
    if (len < consume) return 0;

    debug_printf("[pngle]   Parse IHDR\n");

    pngle->hdr.width       = read_uint32(buf +  0);
    pngle->hdr.height      = read_uint32(buf +  4);
    pngle->hdr.depth       = read_uint8 (buf +  8);
    pngle->hdr.color_type  = read_uint8 (buf +  9);
    pngle->hdr.compression = read_uint8 (buf + 10);
    pngle->hdr.filter      = read_uint8 (buf + 11);
    pngle->hdr.interlace   = read_uint8 (buf + 12);

    debug_printf("[pngle]     width      : %d\n", pngle->hdr.width      );
    debug_printf("[pngle]     height     : %d\n", pngle->hdr.height     );
    debug_printf("[pngle]     depth      : %d\n", pngle->hdr.depth      );
    debug_printf("[pngle]     color_type : %d\n", pngle->hdr.color_type );
    debug_printf("[pngle]     compression: %d\n", pngle->hdr.compression);
    debug_printf("[pngle]     filter     : %d\n", pngle->hdr.filter     );
    debug_printf("[pngle]     interlace  : %d\n", pngle->hdr.interlace  );

    /*
            Color    Allowed    Interpretation                            channels
            Type    Bit Depths

            0       1,2,4,8,16  Each pixel is a grayscale sample.         1 channels (Brightness)

            2       8,16        Each pixel is an R,G,B triple.            3 channels (R, G, B)

            3       1,2,4,8     Each pixel is a palette index;            1 channels (palette info)
                                a PLTE chunk must appear.

            4       8,16        Each pixel is a grayscale sample,         2 channels (Brightness, Alpha)
                                followed by an alpha sample.

            6       8,16        Each pixel is an R,G,B triple,            4 channels (R, G, B, Alpha)
                                followed by an alpha sample.
    */
    //  111
    //    ^-- indexed color (palette)
    //   ^--- Color
    //  ^---- Alpha channel
    {
      uint_fast8_t ch_mask = 0;
      switch (pngle->hdr.color_type) {
      case 0: ch_mask = 1 << 5 | 0;       break; // 1 channel grayscale
      case 2: ch_mask = 3 << 5 | 0b00111; break; // 3 channel truecolor
      case 3: ch_mask = 1 << 5 | 0b10000; break; // 1 channel indexed color
      case 4: ch_mask = 2 << 5 | 0b00111; break; // 2 channel grayscale + alpha
      case 6: ch_mask = 4 << 5 | 0b00111; break; // 4 channel truecolor + alpha
      default:
        return PNGLE_ERROR("Incorrect IHDR info");
      }
      pngle->channels = ch_mask >> 5;
      int depth = pngle->hdr.depth;
      if ((depth & ch_mask) || (depth != (depth & (-depth)))) return PNGLE_ERROR("Invalid bit depth");
    }

    pngle->filter_type = -1;
    pngle->magni = ( (pngle->hdr.depth == 1) ? 0xFFFFFF00
                   : (pngle->hdr.depth == 2) ? 0x55555500
                   : (pngle->hdr.depth == 4) ? 0x11111100
                                             : 0x01010100
                   );

    if (pngle->hdr.compression != 0) return PNGLE_ERROR("Unsupported compression type in IHDR");
    if (pngle->hdr.filter      != 0) return PNGLE_ERROR("Unsupported filter type in IHDR");

    // interlace
    set_interlace_pass(pngle, pngle->hdr.interlace ? 0 : 7);

    {
      uint_fast8_t bytes_per_pixel = (pngle->channels * pngle->hdr.depth + 7) >> 3;
      if (bytes_per_pixel < 4) bytes_per_pixel = 4;
      size_t memlen = (pngle->hdr.width * bytes_per_pixel + 11) & ~3; // width x ARGB8888  4byte aligned
      if ((pngle->scanline_buf = PNGLE_MALLOC(memlen, 2, "scanline flipbuf")) == NULL) return PNGLE_ERROR("Insufficient memory");
      memset(pngle->scanline_buf, 0, memlen + 8);
      pngle->scanline_flipbuf[0] = &pngle->scanline_buf[8];
      pngle->scanline_flipbuf[1] = &pngle->scanline_buf[8 + memlen];
    }

    // callback
    if (pngle->init_callback) pngle->init_callback(pngle, pngle->hdr.width, pngle->hdr.height);

    break;

  case PNGLE_CHUNK_PLTE:
    {
      size_t num = (len / 3);
      if (num == 0) return 0;
      consume = num * 3;
      memcpy(pngle->palette + pngle->n_palettes * 3, buf, consume);
      pngle->n_palettes += num;
    }
    break;

  case PNGLE_CHUNK_IEND:
    consume = 0;
    break;

  case PNGLE_CHUNK_tRNS:
    switch (pngle->hdr.color_type) {
    case 3: consume =     1; break;
    case 0: consume = 2 * 1; break;
    case 2: consume = 2 * 3; break;
    default:
      return PNGLE_ERROR("tRNS chunk is prohibited on the color type");
    }
    if (len < consume) return 0;

    memcpy(pngle->trans_palette + pngle->n_trans_palettes, buf, consume);

    pngle->n_trans_palettes++;

    break;

  case PNGLE_CHUNK_gAMA:
    consume = 4;
    if (len < consume) return 0;

    if (setup_gamma_table(pngle, read_uint32(buf)) < 0) return -1;

    break;

  default:
    // unknown chunk
    consume = len;

    debug_printf("[pngle] Unknown chunk; %zd bytes discarded\n", consume);
    break;
  }

  return consume;
}

static int pngle_feed_internal(pngle_t *pngle, const uint8_t *buf, size_t len)
{
  switch (pngle->state) {
  case PNGLE_STATE_INITIAL:
    // find PNG header
    if (len < sizeof(png_sig)) return 0;

    if (memcmp(png_sig, buf, sizeof(png_sig))) return PNGLE_ERROR("Incorrect PNG signature");

    debug_printf("[pngle] PNG signature found\n");

    pngle->state = PNGLE_STATE_FIND_CHUNK_HEADER;
    return sizeof(png_sig);

  case PNGLE_STATE_FIND_CHUNK_HEADER:
    if (len < 8) return 0;

    pngle->chunk_remain = read_uint32(buf);
    pngle->chunk_type = read_uint32(buf + 4);

    debug_printf("[pngle] Chunk '%.4s' len %u\n", buf + 4, pngle->chunk_remain);

    pngle->state = PNGLE_STATE_HANDLE_CHUNK;

    // initialize & sanity check
    switch (pngle->chunk_type) {
    case PNGLE_CHUNK_IHDR:
      if (pngle->chunk_remain != 13) return PNGLE_ERROR("Invalid IHDR chunk size");
      if (pngle->channels != 0) return PNGLE_ERROR("Multiple IHDR chunks are not allowed");
      break;

    case PNGLE_CHUNK_IDAT:
      if (pngle->chunk_remain <= 0) return PNGLE_ERROR("Invalid IDAT chunk size");
      if (pngle->channels == 0) return PNGLE_ERROR("No IHDR chunk is found");
      if (pngle->hdr.color_type == 3 && pngle->palette == NULL) return PNGLE_ERROR("No PLTE chunk is found");

      if (pngle->next_out == NULL) {
        // Very first IDAT
        pngle->next_out = pngle->lz_buf;
        pngle->avail_out = TINFL_LZ_DICT_SIZE;
      }
      break;

    case PNGLE_CHUNK_PLTE:
      if (pngle->chunk_remain <= 0) return PNGLE_ERROR("Invalid PLTE chunk size");
      if (pngle->channels == 0) return PNGLE_ERROR("No IHDR chunk is found");
      if (pngle->palette) return PNGLE_ERROR("Too many PLTE chunk");

      switch (pngle->hdr.color_type) {
      case 3: // indexed color
        break;
      case 2: // truecolor
      case 6: // truecolor + alpha
        // suggested palettes
        break;
      default:
        return PNGLE_ERROR("PLTE chunk is prohibited on the color type");
      }
      uint32_t chunk_remain_3 = pngle->chunk_remain / 3;
      if (pngle->chunk_remain != chunk_remain_3 * 3) return PNGLE_ERROR("Invalid PLTE chunk size");
      if (chunk_remain_3 > MIN(256, (1UL << pngle->hdr.depth))) return PNGLE_ERROR("Too many palettes in PLTE");
      if ((pngle->palette = PNGLE_MALLOC(chunk_remain_3, 3, "palette")) == NULL) return PNGLE_ERROR("Insufficient memory");
      pngle->n_palettes = 0;
      break;

    case PNGLE_CHUNK_IEND:
      if (pngle->next_out == NULL) return PNGLE_ERROR("No IDAT chunk is found");
      if (pngle->chunk_remain > 0) return PNGLE_ERROR("Invalid IEND chunk size");
      break;

    case PNGLE_CHUNK_tRNS:
      if (pngle->chunk_remain <= 0) return PNGLE_ERROR("Invalid tRNS chunk size");
      if (pngle->channels == 0) return PNGLE_ERROR("No IHDR chunk is found");
      if (pngle->trans_palette) return PNGLE_ERROR("Too many tRNS chunk");

      switch (pngle->hdr.color_type) {
      case 3: // indexed color
        if (pngle->chunk_remain > (1UL << pngle->hdr.depth)) return PNGLE_ERROR("Too many palettes in tRNS");
        break;
      case 0: // grayscale
        if (pngle->chunk_remain != 2) return PNGLE_ERROR("Invalid tRNS chunk size");
        break;
      case 2: // truecolor
        if (pngle->chunk_remain != 6) return PNGLE_ERROR("Invalid tRNS chunk size");
        break;

      default:
        return PNGLE_ERROR("tRNS chunk is prohibited on the color type");
      }
      if ((pngle->trans_palette = PNGLE_MALLOC(pngle->chunk_remain, 1, "trans palette")) == NULL) return PNGLE_ERROR("Insufficient memory");
      pngle->n_trans_palettes = 0;
      break;

    default:
      break;
    }

    return 8;

  case PNGLE_STATE_HANDLE_CHUNK:
    len = MIN(len, pngle->chunk_remain);

    int consumed = pngle_handle_chunk(pngle, buf, len);

    if (consumed > 0) {
      if (pngle->chunk_remain < (uint32_t)consumed) return PNGLE_ERROR("Chunk data has been consumed too much");

      pngle->chunk_remain -= consumed;
    }
    if (pngle->chunk_remain <= 0) pngle->state = PNGLE_STATE_CRC;

    return consumed;

  case PNGLE_STATE_CRC:
    if (len < 4) return 0;
    pngle->state = PNGLE_STATE_FIND_CHUNK_HEADER;

    // XXX:
    if (pngle->chunk_type == PNGLE_CHUNK_IEND) {
      pngle->state = PNGLE_STATE_EOF;
      if (pngle->done_callback) pngle->done_callback(pngle);
      debug_printf("[pngle] DONE\n");
      return -1;
    }

    return 4;

  case PNGLE_STATE_EOF:
    return len;

  case PNGLE_STATE_ERROR:
    return -1;

  default:
    return PNGLE_ERROR("Invalid state");
  }

}

int lgfx_pngle_feed(pngle_t *pngle, const void *buf, size_t len)
{
  if (!pngle) return -1;
  size_t pos = 0;
  pngle_state_t last_state = pngle->state;

  int r;
  do {
    r = pngle_feed_internal(pngle, (const uint8_t *)buf + pos, len - pos);
    if (r < 0) return r; // error

    if (r == 0 && last_state == pngle->state) break;
    last_state = pngle->state;

  } while ((pos += r) < len);

  return pos;
}

void lgfx_pngle_set_display_gamma(pngle_t *pngle, double display_gamma)
{
  if (!pngle) return ;
#ifndef PNGLE_NO_GAMMA_CORRECTION
  pngle->display_gamma = display_gamma;
#else
  PNGLE_UNUSED(display_gamma);
#endif
}

void lgfx_pngle_set_init_callback(pngle_t *pngle, pngle_init_callback_t callback)
{
  if (!pngle) return ;
  pngle->init_callback = callback;
}

void lgfx_pngle_set_draw_callback(pngle_t *pngle, pngle_draw_callback_t callback)
{
  if (!pngle) return ;
  pngle->draw_callback = callback;
}

void lgfx_pngle_set_done_callback(pngle_t *pngle, pngle_done_callback_t callback)
{
  if (!pngle) return ;
  pngle->done_callback = callback;
}

void lgfx_pngle_set_user_data(pngle_t *pngle, void *user_data)
{
  if (!pngle) return ;
  pngle->user_data = user_data;
}

void *lgfx_pngle_get_user_data(pngle_t *pngle)
{
  if (!pngle) return NULL;
  return pngle->user_data;
}

/* vim: set ts=4 sw=4 noexpandtab: */
