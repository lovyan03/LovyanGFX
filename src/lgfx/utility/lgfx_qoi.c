#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "lgfx_qoi.h"


#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <Arduino.h>
#include "esp_log.h"


static const char* TAG = "[qoi]";


typedef enum {
  QOI_STATE_ERROR = -2,
  QOI_STATE_EOF = -1,
  //  QOI_STATE_INITIAL = 0,
  QOI_STATE_OP_INDEX  = 0x00, /* 00xxxxxx */
  QOI_STATE_OP_DIFF   = 0x40, /* 01xxxxxx */
  QOI_STATE_OP_LUMA   = 0x80, /* 10xxxxxx */
  QOI_STATE_OP_RUN    = 0xc0, /* 11xxxxxx */
  QOI_STATE_OP_RGB    = 0xfe, /* 11111110 */
  QOI_STATE_OP_RGBA   = 0xff, /* 11111111 */

  //QOI_STATE_FIND_CHUNK_HEADER,
  QOI_STATE_HANDLE_CHUNK,
  QOI_STATE_CRC,
} qoi_state_t;

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#define QOI_ERROR(s) (qoi->error = (s), qoi->state = QOI_STATE_ERROR, -1)

#define QOI_DEBUG true

#ifdef QOI_DEBUG
#define debug_printf(...) fprintf(stderr, __VA_ARGS__)
#else
#define debug_printf(...) ((void)0)
#endif


#define QOI_CALLOC(a, b, name) (debug_printf("[qoi] Allocating %zu bytes for %s\n", (size_t)(a) * (size_t)(b), (name)), calloc((size_t)(a), (size_t)(b)))


// typedef struct _qoi_t qoi_t; // declared in qoi.h
struct _qoi_t
{
  qoi_desc_t desc;

  uint_fast8_t channels;

  qoi_rgba_t index[64];
  qoi_rgba_t px;

  int b1;

  uint32_t px_pos;// = 0;
  uint32_t repeat;// = 0;

  qoi_state_t state;
  uint32_t chunk_type;
  uint32_t chunk_remain;
  uint32_t total_pixels;

  uint32_t drawing_y;

  const char *error;

  qoi_init_callback_t init_callback;
  qoi_draw_callback_t draw_callback;
  qoi_done_callback_t done_callback;

  void *user_data;
  uint32_t data_len;
};


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




static int qoi_handle_chunk(qoi_t *qoi, const uint8_t *buf, size_t len)
{
  size_t consume = 0;


  while( consume < len && qoi->px_pos < qoi->total_pixels ) {

    if (qoi->repeat > 0) {
      qoi->repeat--;

    } else {

      if( consume+4 >= len ) return consume;

      qoi->b1 = read_uint8(buf++);
      consume++;

      if (qoi->b1 == QOI_OP_RGB) {
        qoi->px.rgba.r = read_uint8(buf++);
        qoi->px.rgba.g = read_uint8(buf++);
        qoi->px.rgba.b = read_uint8(buf++);
        consume += 3;
      } else if (qoi->b1 == QOI_OP_RGBA) {
        qoi->px.rgba.r = read_uint8(buf++);
        qoi->px.rgba.g = read_uint8(buf++);
        qoi->px.rgba.b = read_uint8(buf++);
        qoi->px.rgba.a = read_uint8(buf++);
        consume += 4;
      } else if ((qoi->b1 & QOI_MASK_2) == QOI_OP_INDEX) {
        qoi->px = qoi->index[qoi->b1];
      } else if ((qoi->b1 & QOI_MASK_2) == QOI_OP_DIFF) {
        qoi->px.rgba.r += ((qoi->b1 >> 4) & 0x03) - 2;
        qoi->px.rgba.g += ((qoi->b1 >> 2) & 0x03) - 2;
        qoi->px.rgba.b += ( qoi->b1       & 0x03) - 2;
      } else if ((qoi->b1 & QOI_MASK_2) == QOI_OP_LUMA) {
        int b2 = read_uint8(buf++);
        consume ++;
        int vg = (qoi->b1 & 0x3f) - 32;
        qoi->px.rgba.r += vg - 8 + ((b2 >> 4) & 0x0f);
        qoi->px.rgba.g += vg;
        qoi->px.rgba.b += vg - 8 +  (b2       & 0x0f);
      } else if ((qoi->b1 & QOI_MASK_2) == QOI_OP_RUN) {
        qoi->repeat = (qoi->b1 & 0x3f);
      }
      qoi->index[QOI_COLOR_HASH(qoi->px) % 64] = qoi->px;
    }

    if (qoi->desc.channels != 4) {
      qoi->px.rgba.a = 255;
    }

    qoi->px_pos++;

    if (qoi->draw_callback) {
      qoi->draw_callback(qoi, qoi->px_pos%qoi->desc.width, qoi->px_pos/qoi->desc.width, 1/*interlace_div_x[qoi->interlace_pass]*/, 1/*qoi->scanline_pixels*/, (uint8_t*)&qoi->px.rgba);
    }
  }
  return consume;
}



static int qoi_feed_internal(qoi_t *qoi, const uint8_t *buf, size_t len)
{
  switch (qoi->state) {

    case QOI_STATE_OP_INDEX:
    {
      if ( len < QOI_HEADER_SIZE + (int)sizeof(qoi_padding) ) return 0;

      uint8_t magic8[4] = { read_uint8(buf + 0), read_uint8(buf + 1), read_uint8(buf + 2), read_uint8(buf + 3) };

      qoi->desc.width       = read_uint32(buf +  4);
      qoi->desc.height      = read_uint32(buf +  8);
      qoi->desc.channels    = read_uint8(buf + 12);
      qoi->desc.colorspace  = read_uint8(buf + 13);

      int cmp = memcmp(qoi_sig, magic8, 4);

      if ( cmp ) return QOI_ERROR("Incorrect QOI signature");
      if( qoi->desc.width == 0 || qoi->desc.height == 0 || qoi->desc.colorspace > 1 ) return QOI_ERROR("Incorrect QOI signature");
      if (qoi->desc.channels != 0 && qoi->desc.channels != 3 && qoi->desc.channels != 4) return QOI_ERROR("Bad channels count");
      if( qoi->desc.height >= QOI_PIXELS_MAX / qoi->desc.width ) return QOI_ERROR("Image too big");

      // TODO: fix this
      qoi->total_pixels = qoi->desc.width*qoi->desc.height;
      qoi->chunk_remain = qoi->total_pixels;
      qoi->state = QOI_STATE_HANDLE_CHUNK;

      if (qoi->init_callback) qoi->init_callback(qoi, qoi->desc.width, qoi->desc.height);

      return QOI_HEADER_SIZE;
    }

    case QOI_STATE_HANDLE_CHUNK:
    {
      len = MIN(len, qoi->chunk_remain);

      int consumed = qoi_handle_chunk(qoi, buf, len);

      if (consumed > 0) {
        if (qoi->chunk_remain < (uint32_t)consumed) return QOI_ERROR("Chunk data has been consumed too much");
        qoi->chunk_remain -= consumed;
      }
      if (qoi->chunk_remain <= 0) qoi->state = QOI_STATE_CRC;

      return consumed;
    }

    case QOI_STATE_CRC:

      qoi->state = QOI_STATE_HANDLE_CHUNK;
      if (qoi->chunk_remain <= 0) {
        qoi->state = QOI_STATE_EOF;
        if (qoi->done_callback) qoi->done_callback(qoi);
        return -1;
      }
      return 0;

    case QOI_STATE_EOF:
      return len;

    case QOI_STATE_ERROR:
      return -1;

    default:
      return QOI_ERROR("Invalid state");
  }

  return -1;
}

int lgfx_qoi_feed(qoi_t *qoi, const void *buf, size_t len)
{
  if (!qoi) return -1;
  size_t pos = 0;
  qoi_state_t last_state = qoi->state;

  int r;
  do {
    r = qoi_feed_internal(qoi, (const uint8_t *)buf + pos, len - pos);
    if (r < 0) return r; // error
    if (r == 0 && last_state == qoi->state) break;
    last_state = qoi->state;

  } while ((pos += r) < len);

  return pos;
}



void lgfx_qoi_reset(qoi_t *qoi)
{
  if (!qoi) return;

  qoi->state = QOI_STATE_OP_INDEX;
  qoi->error = "No error";

  qoi->channels = 0;
  memset( qoi->index, 0, sizeof(qoi->index) );

  qoi->px.rgba.a = 255;
  qoi->px.rgba.r = 0;
  qoi->px.rgba.g = 0;
  qoi->px.rgba.b = 0;

  qoi->px_pos = 0;
  qoi->repeat = 0;

  // clear them just in case...
  memset(&qoi->desc, 0, sizeof(qoi->desc));
}


qoi_t *lgfx_qoi_new()
{
  qoi_t *qoi = (qoi_t *)calloc(1, sizeof(qoi_t) );
  if (!qoi) return NULL;
  lgfx_qoi_reset(qoi);
  return qoi;
}

void lgfx_qoi_destroy(qoi_t *qoi)
{
  if (qoi) {
    lgfx_qoi_reset(qoi);
    free(qoi);
  }
}

const char *lgfx_qoi_error(qoi_t *qoi)
{
  if (!qoi) return "Uninitialized";
  return qoi->error;
}

uint32_t lgfx_qoi_get_width(qoi_t *qoi)
{
  if (!qoi) return 0;
  return qoi->desc.width;
}

uint32_t lgfx_qoi_get_height(qoi_t *qoi)
{
  if (!qoi) return 0;
  return qoi->desc.height;
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

void lgfx_qoi_set_done_callback(qoi_t *qoi, qoi_done_callback_t callback)
{
  if (!qoi) return;
  qoi->done_callback = callback;
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

