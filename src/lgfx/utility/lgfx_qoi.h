#ifndef __LGFX_QOI_H__
#define __LGFX_QOI_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Main Qoi object
typedef struct _qoi_t qoi_t;
// QOI image description header
typedef struct _qoi_desc_t qoi_desc_t;

// Callback signatures
typedef void (*qoi_init_callback_t)(qoi_t *qoi, uint32_t w, uint32_t h);
typedef void (*qoi_draw_callback_t)(qoi_t *qoi, uint32_t x, uint32_t y, uint_fast8_t div_x, size_t len, const uint8_t* rgba);
typedef uint8_t *(*lgfx_qoi_encoder_get_row_func)(uint8_t *lineBuffer, int flip, int w, int h, int y, void *qoienc);

// ---------------------
// Basic read interfaces
// ---------------------
qoi_t *lgfx_qoi_new();
void lgfx_qoi_destroy(qoi_t *qoi);
void lgfx_qoi_reset(qoi_t *qoi); // clear its internal state (not applied to qoi_set_* functions)
int lgfx_qoi_feed(qoi_t *qoi, const uint8_t *buf, size_t len); // returns -2: On error, -1: Done, 0: Need more data, n: n bytes eaten

void lgfx_qoi_set_init_callback(qoi_t *qoi, qoi_init_callback_t callback);
void lgfx_qoi_set_draw_callback(qoi_t *qoi, qoi_draw_callback_t callback);

void lgfx_qoi_set_user_data(qoi_t *qoi, void *user_data);
void *lgfx_qoi_get_user_data(qoi_t *qoi);

qoi_desc_t *lgfx_qoi_get_desc(qoi_t *qoi);

// ----------------------
// Basic write interfaces
// ----------------------
void *lgfx_qoi_encoder_write_framebuffer_to_file(const void *lineBuffer, int w, int h, int num_chans, size_t *out_len, int flip, lgfx_qoi_encoder_get_row_func cb, void *qoienc);
void *lgfx_qoi_encode(const void *lineBuffer, const qoi_desc_t *desc, int flip, lgfx_qoi_encoder_get_row_func cb, size_t *out_len, void *qoienc);


#ifdef __cplusplus
}
#endif

#endif /* __LGFX_QOI_H__ */


