#ifndef __LGFX_QOI_H__
#define __LGFX_QOI_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QOI_SRGB   0
#define QOI_LINEAR 1

#define QOI_OP_INDEX  0x00 /* 00xxxxxx */
#define QOI_OP_DIFF   0x40 /* 01xxxxxx */
#define QOI_OP_LUMA   0x80 /* 10xxxxxx */
#define QOI_OP_RUN    0xc0 /* 11xxxxxx */
#define QOI_OP_RGB    0xfe /* 11111110 */
#define QOI_OP_RGBA   0xff /* 11111111 */

#define QOI_MASK_2    0xc0 /* 11000000 */

#define QOI_HEADER_SIZE 14
#define QOI_PIXELS_MAX ((unsigned int)400000000)

#define QOI_COLOR_HASH(C) (C.rgba.r*3 + C.rgba.g*5 + C.rgba.b*7 + C.rgba.a*11)

static const uint8_t qoi_sig[4]     = {'q','o','i','f'};
static const uint8_t qoi_padding[8] = {0,0,0,0,0,0,0,1};

// Main Qoi object
typedef struct _qoi_t qoi_t;
// QOI image description header
typedef struct _qoi_desc_t qoi_desc_t;

// Callback signatures
typedef void (*qoi_init_callback_t)(qoi_t *qoi, uint32_t w, uint32_t h);
typedef void (*qoi_draw_callback_t)(qoi_t *qoi, uint32_t x, uint32_t y, uint_fast8_t div_x, size_t len, const uint8_t* rgba);
typedef void (*qoi_done_callback_t)(qoi_t *qoi);
typedef uint8_t *(*lgfx_qoi_encoder_get_row_func)(uint8_t *lineBuffer, int flip, int w, int h, int y, void *qoienc);

// ---------------------
// Basic read interfaces
// ---------------------
qoi_t *lgfx_qoi_new();
void lgfx_qoi_destroy(qoi_t *qoi);
void lgfx_qoi_reset(qoi_t *qoi); // clear its internal state (not applied to qoi_set_* functions)
const char *lgfx_qoi_error(qoi_t *qoi);
int lgfx_qoi_feed(qoi_t *qoi, const void *buf, size_t len); // returns -1: On error, 0: Need more data, n: n bytes eaten

uint32_t lgfx_qoi_get_width(qoi_t *qoi);
uint32_t lgfx_qoi_get_height(qoi_t *qoi);

void lgfx_qoi_set_init_callback(qoi_t *qoi, qoi_init_callback_t callback);
void lgfx_qoi_set_draw_callback(qoi_t *qoi, qoi_draw_callback_t callback);
void lgfx_qoi_set_done_callback(qoi_t *qoi, qoi_done_callback_t callback);

void lgfx_qoi_set_user_data(qoi_t *qoi, void *user_data);
void *lgfx_qoi_get_user_data(qoi_t *qoi);

qoi_desc_t *lgfx_qoi_get_desc(qoi_t *qoi);

// ----------------------
// Basic write interfaces
// ----------------------
void *lgfx_qoi_encoder_write_framebuffer_to_file(const void *lineBuffer, int w, int h, int num_chans, size_t *out_len, int flip, lgfx_qoi_encoder_get_row_func cb, void *qoienc);
void *lgfx_qoi_encode(const void *lineBuffer, const qoi_desc_t *desc, int flip, lgfx_qoi_encoder_get_row_func cb, int *out_len, void *qoienc);


#ifdef __cplusplus
}
#endif

#endif /* __LGFX_QOI_H__ */


