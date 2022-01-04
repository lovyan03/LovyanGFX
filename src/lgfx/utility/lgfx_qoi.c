#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "lgfx_qoi.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// color spaces
#define QOI_SRGB   0
#define QOI_LINEAR 1

#define QOI_HEADER_SIZE 14
#define QOI_PIXELS_MAX ((unsigned int)400000000)

// QOI header
static const uint32_t qoi_sig = 0x716F6966; // {'q','o','i','f'};
// QOI footer
static const uint8_t qoi_padding[8] = {0,0,0,0,0,0,0,1};


typedef union
{
  struct { unsigned char r, g, b, a; } rgba;
  unsigned int v;
} qoi_rgba_t;


typedef struct __attribute__((packed)) _qoi_desc_t
{
  unsigned int width;
  unsigned int height;
  unsigned char channels;
  unsigned char colorspace;
} qoi_desc_t;


typedef enum
{
  QOI_OP_INDEX  = 0x00, /* 00xxxxxx */
  QOI_OP_DIFF   = 0x40, /* 01xxxxxx */
  QOI_OP_LUMA   = 0x80, /* 10xxxxxx */
  QOI_OP_RUN    = 0xc0, /* 11xxxxxx */
  QOI_OP_RGB    = 0xfe, /* 11111110 */
  QOI_OP_RGBA   = 0xff, /* 11111111 */
  QOI_MASK_2    = 0xc0  /* 11000000 */
} qoi_flag_t;


typedef enum
{
  QOI_STATE_OP_INDEX  = 0x00, /* 00xxxxxx */
  QOI_STATE_OP_DIFF   = 0x40, /* 01xxxxxx */
  QOI_STATE_OP_LUMA   = 0x80, /* 10xxxxxx */
  QOI_STATE_OP_RUN    = 0xc0, /* 11xxxxxx */
  QOI_STATE_OP_RGB    = 0xfe, /* 11111110 */
  QOI_STATE_OP_RGBA   = 0xff, /* 11111111 */
} qoi_state_t;


// typedef struct _qoi_t qoi_t; // declared in qoi.h
struct _qoi_t
{
  qoi_rgba_t* pixelBuffer;
  qoi_rgba_t px;

  uint32_t drawing_x;
  uint32_t drawing_y;

  void *user_data;
  qoi_draw_callback_t draw_callback;
  qoi_init_callback_t init_callback;

  qoi_desc_t desc;
  uint8_t repeat;
  qoi_rgba_t index[64];
};

#ifdef QOI_DEBUG
#define debug_printf(...) fprintf(stderr, __VA_ARGS__)
#define QOI_ERROR(...) (fprintf(stderr, __VA_ARGS__), -2)
#else
#define debug_printf(...) ((void)0)
#define QOI_ERROR(...) ( -2 )
#endif


static inline uint8_t QOI_COLOR_HASH( const qoi_rgba_t *c )
{
  return 0x3F & (c->rgba.r*3 + c->rgba.g*5 + c->rgba.b*7 + c->rgba.a*11);
}


static uint8_t read_uint8(const uint8_t *p)
{
  return *p;
}

static uint32_t read_uint32(const uint8_t *p)
{
  return (p[0] << 24)
       | (p[1] << 16)
       | (p[2] <<  8)
       | (p[3] <<  0)
  ;
}

static void write_uint32(uint8_t *bytes, int *p, uint32_t v)
{
  bytes[(*p)++] = (uint8_t)(v >> 24);
  bytes[(*p)++] = (uint8_t)(v >> 16);
  bytes[(*p)++] = (uint8_t)(v >>  8);
  bytes[(*p)++] = (uint8_t)v;
}

int lgfx_qoi_feed(qoi_t *qoi, const uint8_t *buf, size_t len)
{
  size_t consume = 0;

  if ( len < sizeof(qoi_padding) ) { return -2; }
  len -= sizeof(qoi_padding);

  if (qoi->desc.width == 0)
  {
    if ( len < QOI_HEADER_SIZE ) { return -2; }

    if (qoi_sig != read_uint32(buf)) { return QOI_ERROR("Incorrect QOI signature"); }

    qoi->desc.width       = read_uint32(buf +  4);
    qoi->desc.height      = read_uint32(buf +  8);
    qoi->desc.channels    = read_uint8(buf + 12);
    qoi->desc.colorspace  = read_uint8(buf + 13);
    qoi->px.rgba.a = 255;

    if( qoi->desc.width == 0 || qoi->desc.height == 0 || qoi->desc.colorspace > 1 ) return QOI_ERROR("Incorrect QOI signature");
    if (qoi->desc.channels != 0 && qoi->desc.channels != 3 && qoi->desc.channels != 4) return QOI_ERROR("Bad channels count");
    // if( qoi->desc.height >= QOI_PIXELS_MAX / qoi->desc.width ) return QOI_ERROR("Image too big");

    qoi->pixelBuffer = (qoi_rgba_t*)malloc(qoi->desc.width * sizeof(qoi_rgba_t));
    if (qoi->pixelBuffer == NULL) { return QOI_ERROR("Insufficient memory"); }

    if (qoi->init_callback) qoi->init_callback(qoi, qoi->desc.width, qoi->desc.height);

    consume = QOI_HEADER_SIZE;
  }

  uint32_t x = qoi->drawing_x;

  while ( consume < len )
  {
    if (qoi->repeat > 0)
    {
      uint32_t xe = x + qoi->repeat;
      if (xe > qoi->desc.width) { xe = qoi->desc.width; }
      qoi->repeat -= xe - x;
      do
      {
        qoi->pixelBuffer[x] = qoi->px;
      } while (++x != xe);
    }
    else
    {
      uint8_t b1 = buf[consume];

      if (b1 == QOI_OP_RGB)
      {
        qoi->px.rgba.r = buf[++consume];
        qoi->px.rgba.g = buf[++consume];
        qoi->px.rgba.b = buf[++consume];
      }
      else if (b1 == QOI_OP_RGBA)
      {
        qoi->px.rgba.r = buf[++consume];
        qoi->px.rgba.g = buf[++consume];
        qoi->px.rgba.b = buf[++consume];
        qoi->px.rgba.a = buf[++consume];
      }
      else if ((b1 & QOI_MASK_2) == QOI_OP_INDEX)
      {
        qoi->px = qoi->index[b1];
      }
      else if ((b1 & QOI_MASK_2) == QOI_OP_DIFF)
      {
        qoi->px.rgba.r += ((b1 >> 4) & 0x03) - 2;
        qoi->px.rgba.g += ((b1 >> 2) & 0x03) - 2;
        qoi->px.rgba.b += ( b1       & 0x03) - 2;
      }
      else if ((b1 & QOI_MASK_2) == QOI_OP_LUMA)
      {
        uint8_t b2 = buf[++consume];
        int vg = (b1 & 0x3f) - 32;
        qoi->px.rgba.r += vg - 8 + ((b2 >> 4));
        qoi->px.rgba.g += vg;
        qoi->px.rgba.b += vg - 8 +  (b2 & 0x0f);
      }
      else // if ((b1 & QOI_MASK_2) == QOI_OP_RUN)
      {
        qoi->repeat = (b1 & 0x3f);
      }

      ++consume;
      qoi->index[QOI_COLOR_HASH(&qoi->px)] = qoi->px;

      if (qoi->desc.channels != 4) {
        qoi->px.rgba.a = 255;
      }
      qoi->pixelBuffer[x++] = qoi->px;
    }

    if (x < qoi->desc.width) { continue; }
    x = 0;

    if (qoi->draw_callback)
    {
      qoi->draw_callback(qoi, 0, qoi->drawing_y, 1, qoi->desc.width, (const uint8_t*)qoi->pixelBuffer);
    }
    if (++qoi->drawing_y >= qoi->desc.height)
    {
      return -1;
    }
  }
  qoi->drawing_x = x;
  return consume;
}


void lgfx_qoi_reset(qoi_t *qoi)
{
  if (!qoi) return;
  if (qoi->pixelBuffer != NULL) { free(qoi->pixelBuffer);  qoi->pixelBuffer = NULL; }
}


qoi_t *lgfx_qoi_new()
{
  qoi_t *qoi = (qoi_t *)calloc(1, sizeof(qoi_t) );
  if (!qoi) return NULL;
  return qoi;
}


void lgfx_qoi_destroy(qoi_t *qoi)
{
  if (qoi) {
    lgfx_qoi_reset(qoi);
    free(qoi);
  }
}

void lgfx_qoi_set_init_callback(qoi_t *qoi, qoi_init_callback_t callback)
{
  if (!qoi) return;
  qoi->init_callback = callback;
}


void lgfx_qoi_set_draw_callback(qoi_t *qoi, qoi_draw_callback_t callback)
{
  if (!qoi) return;
  qoi->draw_callback = callback;
}


void lgfx_qoi_set_user_data(qoi_t *qoi, void *user_data)
{
  if (!qoi) return;
  qoi->user_data = user_data;
}


void *lgfx_qoi_get_user_data(qoi_t *qoi)
{
  if (!qoi) return NULL;
  return qoi->user_data;
}


// Qoi Encoder

void *lgfx_qoi_encoder_write_framebuffer_to_file(const void *lineBuffer, int w, int h, int num_chans, size_t *out_len, int flip, lgfx_qoi_encoder_get_row_func get_row, void *qoienc)
{
  qoi_desc_t desc;
  desc.width      = w;
  desc.height     = h;
  desc.channels   = num_chans;
  desc.colorspace = QOI_SRGB; // QOI_SRGB=0, QOI_LINEAR=1
  void * res = lgfx_qoi_encode(lineBuffer, &desc, flip, get_row, out_len, qoienc);
  return res;
}

void *lgfx_qoi_encode(const void *lineBuffer, const qoi_desc_t *desc, int flip, lgfx_qoi_encoder_get_row_func get_row, size_t *out_len, void *qoienc)
{
  int i, max_size, p, repeat;
  int px_len, px_end, px_pos, channels;
  unsigned char *outbuff;
  uint8_t *pixels = (uint8_t*)lineBuffer;
  qoi_rgba_t index[64]; // TODO: calloc/free this, this is more than 256 bytes wasted
  qoi_rgba_t px, px_prev;

  if (lineBuffer == NULL)                            { /*ESP_LOGE("[qoi]", "Bad lineBuffer"); */ return NULL; }
  if (out_len == NULL )                              { /*ESP_LOGE("[qoi]", "No out_len");     */ return NULL; }
  if (desc == NULL )                                 { /*ESP_LOGE("[qoi]", "Bad desc");       */ return NULL; }
  if (desc->width == 0 || desc->height == 0 )        { /*ESP_LOGE("[qoi]", "Bad w/h");        */ return NULL; }
  if (desc->channels < 3 || desc->channels > 4 )     { /*ESP_LOGE("[qoi]", "Bad bpp");        */ return NULL; }
  if (desc->colorspace > 1 )                         { /*ESP_LOGE("[qoi]", "Bad colorspace"); */ return NULL; }
  if (desc->height >= QOI_PIXELS_MAX / desc->width ) { /*ESP_LOGE("[qoi]", "Too big");        */ return NULL; }

  max_size =
    desc->width * desc->height * (desc->channels + 1) +
    QOI_HEADER_SIZE + sizeof(qoi_padding);

  p = 0;
  outbuff = (unsigned char *) malloc(max_size);
  if (!outbuff) {
    //ESP_LOGE("[qoi]", "Can't malloc %d bytes", max_size);
    return NULL;
  }

  write_uint32(outbuff, &p, qoi_sig);
  write_uint32(outbuff, &p, desc->width);
  write_uint32(outbuff, &p, desc->height);
  outbuff[p++] = desc->channels;
  outbuff[p++] = desc->colorspace;

  uint32_t lineBufferLen = desc->width * desc->channels;

  memset( index, 0, sizeof(index) );

  repeat = 0;
  px_prev.rgba.r = 0;
  px_prev.rgba.g = 0;
  px_prev.rgba.b = 0;
  px_prev.rgba.a = 255;
  px = px_prev;

  px_len = desc->width * desc->height * desc->channels;
  px_end = px_len - desc->channels;
  channels = desc->channels;

  for (px_pos = 0; px_pos < px_len; px_pos += channels) {

    uint32_t bufferPos = px_pos%lineBufferLen;
    uint32_t ypos      = px_pos/lineBufferLen;
    if( get_row && bufferPos == 0 ) get_row( pixels, flip, desc->width, 1, ypos, qoienc );

    if (channels == 4) {
      px = *(qoi_rgba_t *)(pixels + bufferPos);
    } else {
      px.rgba.r = pixels[bufferPos + 0];
      px.rgba.g = pixels[bufferPos + 1];
      px.rgba.b = pixels[bufferPos + 2];
    }

    if (px.v == px_prev.v) {
      repeat++;
      if (repeat == 62 || px_pos == px_end) {
        outbuff[p++] = QOI_OP_RUN | (repeat - 1);
        repeat = 0;
      }
    } else {
      int index_pos;

      if (repeat > 0) {
        outbuff[p++] = QOI_OP_RUN | (repeat - 1);
        repeat = 0;
      }

      index_pos = QOI_COLOR_HASH(&px);

      if (index[index_pos].v == px.v) {
        outbuff[p++] = QOI_OP_INDEX | index_pos;
      } else {
        index[index_pos] = px;

        if (px.rgba.a == px_prev.rgba.a) {
          signed char vr = px.rgba.r - px_prev.rgba.r;
          signed char vg = px.rgba.g - px_prev.rgba.g;
          signed char vb = px.rgba.b - px_prev.rgba.b;

          signed char vg_r = vr - vg;
          signed char vg_b = vb - vg;

          if (
            vr > -3 && vr < 2 &&
            vg > -3 && vg < 2 &&
            vb > -3 && vb < 2
          ) {
            outbuff[p++] = QOI_OP_DIFF + ((vr + 2) << 4) + ((vg + 2) << 2) + (vb + 2);
          } else if (
            vg_r >  -9 && vg_r <  8 &&
            vg   > -33 && vg   < 32 &&
            vg_b >  -9 && vg_b <  8
          ) {
            outbuff[p++] = QOI_OP_LUMA     | (vg   + 32);
            outbuff[p++] = (vg_r + 8) << 4 | (vg_b +  8);
          } else {
            outbuff[p++] = QOI_OP_RGB;
            outbuff[p++] = px.rgba.r;
            outbuff[p++] = px.rgba.g;
            outbuff[p++] = px.rgba.b;
          }
        } else {
          outbuff[p++] = QOI_OP_RGBA;
          outbuff[p++] = px.rgba.r;
          outbuff[p++] = px.rgba.g;
          outbuff[p++] = px.rgba.b;
          outbuff[p++] = px.rgba.a;
        }
      }
    }
    px_prev = px;
  }

  for (i = 0; i < (int)sizeof(qoi_padding); i++) {
    outbuff[p++] = qoi_padding[i];
  }

  *out_len = p;
  return outbuff;
}

