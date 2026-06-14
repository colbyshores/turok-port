/*
 * Headless OSMesa window-manager backend + PNG capture for Fast3D.
 * Renders gfx_pc output into a host RGBA8 buffer with no display server, then
 * dumps a top-down PNG (self-contained encoder, stored-deflate — no libpng/zlib).
 */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "gfx_osmesa.h"

#define OSMESA_DEPTH_BITS   24
#define OSMESA_STENCIL_BITS 8
#define GL_UNSIGNED_BYTE    0x1401

static OSMesaContext s_ctx = nullptr;
static uint8_t      *s_buf = nullptr;
static int           s_w = 0, s_h = 0;

/* ------------------------------------------------------------------ WM impl */

static void osm_init(const struct GfxWindowInitSettings *set) {
    s_w = (int)set->width ? (int)set->width : 320;
    s_h = (int)set->height ? (int)set->height : 240;
    s_ctx = OSMesaCreateContextExt(OSMESA_RGBA, OSMESA_DEPTH_BITS, OSMESA_STENCIL_BITS, 0, nullptr);
    if (!s_ctx) { fprintf(stderr, "gfx_osmesa: OSMesaCreateContextExt failed\n"); abort(); }
    s_buf = (uint8_t *)calloc((size_t)s_w * s_h, 4);
    if (!OSMesaMakeCurrent(s_ctx, s_buf, GL_UNSIGNED_BYTE, s_w, s_h)) {
        fprintf(stderr, "gfx_osmesa: OSMesaMakeCurrent failed\n"); abort();
    }
    fprintf(stderr, "gfx_osmesa: %dx%d context current\n", s_w, s_h);
}

static void osm_close(void) {
    if (s_ctx) { OSMesaDestroyContext(s_ctx); s_ctx = nullptr; }
    free(s_buf); s_buf = nullptr;
}

static int  osm_get_display_mode(int n, int *w, int *h) { (void)n; *w = s_w; *h = s_h; return 1; }
static int  osm_get_current_display_mode(int *w, int *h) { *w = s_w; *h = s_h; return 1; }
static int  osm_get_num_display_modes(void) { return 1; }
static int32_t osm_zero32(void) { return 0; }
static void osm_set_fs_changed_cb(void (*cb)(bool)) { (void)cb; }
static void osm_set_bool(bool b) { (void)b; }
static void osm_set_i32(int32_t m) { (void)m; }
static int32_t osm_get_fs_flag_mode(void) { return 0; }
static void osm_get_refresh_rate(uint32_t *r) { *r = 30; }
static void osm_set_closest_res(int32_t w, int32_t h, bool c) { (void)w; (void)h; (void)c; }
static void osm_set_dimensions(uint32_t w, uint32_t h, int32_t x, int32_t y) { (void)w; (void)h; (void)x; (void)y; }
static void osm_get_dimensions(uint32_t *w, uint32_t *h, int32_t *x, int32_t *y) {
    *w = (uint32_t)s_w; *h = (uint32_t)s_h; if (x) *x = 0; if (y) *y = 0;
}
static void osm_get_centered(int32_t w, int32_t h, int32_t *x, int32_t *y) { (void)w; (void)h; *x = 0; *y = 0; }
static void osm_handle_events(void) {}
static bool osm_start_frame(void) { return true; }
static void osm_noop(void) {}
static double osm_get_time(void) { return 0.0; }
static int32_t osm_get_target_fps(void) { return 30; }
static void osm_set_target_fps(int f) { (void)f; }
static bool osm_true(void) { return true; }
static void *osm_get_window_handle(void) { return nullptr; }
static void osm_set_title(const char *t) { (void)t; }
static int  osm_get_swap_interval(void) { return 0; }
static bool osm_set_swap_interval(int i) { (void)i; return false; }

extern "C" struct GfxWindowManagerAPI gfx_osmesa_wm = {
    osm_init,
    osm_close,
    osm_get_display_mode,
    osm_get_current_display_mode,
    osm_get_num_display_modes,
    osm_zero32,                 /* get_fullscreen_state */
    osm_set_fs_changed_cb,
    osm_set_bool,               /* set_fullscreen */
    osm_set_bool,               /* set_fullscreen_exclusive */
    osm_set_i32,                /* set_fullscreen_flag */
    osm_get_fs_flag_mode,
    osm_zero32,                 /* get_maximized_state */
    osm_set_bool,               /* set_maximize */
    osm_get_refresh_rate,
    osm_set_bool,               /* set_cursor_visibility */
    osm_set_closest_res,
    osm_set_dimensions,
    osm_get_dimensions,
    osm_get_centered,
    osm_handle_events,
    osm_start_frame,
    osm_noop,                   /* swap_buffers_begin */
    osm_noop,                   /* swap_buffers_end */
    osm_get_time,
    osm_get_target_fps,
    osm_set_target_fps,
    osm_true,                   /* can_disable_vsync */
    osm_get_window_handle,
    osm_set_title,
    osm_get_swap_interval,
    osm_set_swap_interval,
};

/* --------------------------------------------------------------- capture */

const unsigned char *gfx_osmesa_get_buffer(int *out_w, int *out_h) {
    if (out_w) *out_w = s_w;
    if (out_h) *out_h = s_h;
    return s_buf;
}

/* ----- minimal PNG (RGBA8, stored-deflate; no external deps) ----- */
static uint32_t crc32_buf(const uint8_t *p, size_t n, uint32_t crc) {
    static uint32_t tab[256]; static int init = 0;
    if (!init) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            tab[i] = c;
        }
        init = 1;
    }
    crc ^= 0xFFFFFFFFu;
    for (size_t i = 0; i < n; i++) crc = tab[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

static void put_be32(FILE *f, uint32_t v) {
    uint8_t b[4] = { (uint8_t)(v >> 24), (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)v };
    fwrite(b, 1, 4, f);
}

static void png_chunk(FILE *f, const char *type, const uint8_t *data, size_t len) {
    put_be32(f, (uint32_t)len);
    fwrite(type, 1, 4, f);
    if (len) fwrite(data, 1, len, f);
    /* CRC over (type + data), computed over a contiguous temp buffer */
    uint8_t *tmp = (uint8_t *)malloc(4 + len);
    memcpy(tmp, type, 4);
    if (len) memcpy(tmp + 4, data, len);
    uint32_t crc = crc32_buf(tmp, 4 + len, 0);
    free(tmp);
    put_be32(f, crc);
}

int gfx_osmesa_save_png(const char *path) {
    if (!s_buf || s_w <= 0 || s_h <= 0) return -1;
    FILE *f = fopen(path, "wb");
    if (!f) return -1;

    /* signature */
    static const uint8_t sig[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    fwrite(sig, 1, 8, f);

    /* IHDR */
    uint8_t ihdr[13];
    ihdr[0] = (uint8_t)(s_w >> 24); ihdr[1] = (uint8_t)(s_w >> 16);
    ihdr[2] = (uint8_t)(s_w >> 8);  ihdr[3] = (uint8_t)s_w;
    ihdr[4] = (uint8_t)(s_h >> 24); ihdr[5] = (uint8_t)(s_h >> 16);
    ihdr[6] = (uint8_t)(s_h >> 8);  ihdr[7] = (uint8_t)s_h;
    ihdr[8] = 8;   /* bit depth */
    ihdr[9] = 6;   /* color type RGBA */
    ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;
    png_chunk(f, "IHDR", ihdr, 13);

    /* raw filtered scanlines: filter byte 0 + RGBA row, flipped top-down */
    size_t raw_len = (size_t)s_h * (1 + (size_t)s_w * 4);
    uint8_t *raw = (uint8_t *)malloc(raw_len);
    size_t o = 0;
    for (int y = 0; y < s_h; y++) {
        const uint8_t *src = s_buf + (size_t)(s_h - 1 - y) * s_w * 4; /* OSMesa row0=bottom */
        raw[o++] = 0;
        memcpy(raw + o, src, (size_t)s_w * 4);
        o += (size_t)s_w * 4;
    }

    /* zlib stream: header + stored deflate blocks + adler32 */
    size_t max_z = 2 + raw_len + (raw_len / 65535 + 1) * 5 + 4;
    uint8_t *z = (uint8_t *)malloc(max_z);
    size_t zo = 0;
    z[zo++] = 0x78; z[zo++] = 0x01;               /* zlib header */
    size_t pos = 0;
    while (pos < raw_len) {
        size_t blk = raw_len - pos; if (blk > 65535) blk = 65535;
        int final = (pos + blk >= raw_len) ? 1 : 0;
        z[zo++] = (uint8_t)final;                  /* BFINAL + BTYPE=00 */
        z[zo++] = (uint8_t)(blk & 0xFF); z[zo++] = (uint8_t)(blk >> 8);
        uint16_t nlen = (uint16_t)~blk;
        z[zo++] = (uint8_t)(nlen & 0xFF); z[zo++] = (uint8_t)(nlen >> 8);
        memcpy(z + zo, raw + pos, blk); zo += blk; pos += blk;
    }
    /* adler32 of raw */
    uint32_t a = 1, b = 0;
    for (size_t i = 0; i < raw_len; i++) { a = (a + raw[i]) % 65521; b = (b + a) % 65521; }
    uint32_t adler = (b << 16) | a;
    z[zo++] = (uint8_t)(adler >> 24); z[zo++] = (uint8_t)(adler >> 16);
    z[zo++] = (uint8_t)(adler >> 8);  z[zo++] = (uint8_t)adler;

    png_chunk(f, "IDAT", z, zo);
    png_chunk(f, "IEND", nullptr, 0);

    free(z); free(raw);
    fclose(f);
    fprintf(stderr, "gfx_osmesa: wrote %s (%dx%d)\n", path, s_w, s_h);
    return 0;
}
