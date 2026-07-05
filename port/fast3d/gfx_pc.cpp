#define _GNU_SOURCE 1
#define NOMINMAX

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdio>

#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include <list>
#include <stack>
#include <string>
#include <iostream>
#include <memory>
#include <limits>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#include "platform.h"
#include "gbi_ext.h"
#include <sched.h>

#include "gfx_pc.h"
#include "gfx_cc.h"
#include "gfx_window_manager_api.h"
#include "gfx_rendering_api.h"
#include "gfx_screen_config.h"
#include "turok_trace.h"     /* gated pipeline trace (BK_TR / BK_TR_EVERY) */

uintptr_t gfxFramebuffer;

#define ALIGN(x, a) (((x) + (a - 1)) & ~(a - 1))

#define SUPPORT_CHECK(x) assert(x)

// SCALE_M_N: upscale/downscale M-bit integer to N-bit
#define SCALE_5_8(VAL_) (((VAL_)*0xFF) / 0x1F)
#define SCALE_8_5(VAL_) ((((VAL_) + 4) * 0x1F) / 0xFF)
#define SCALE_4_8(VAL_) ((VAL_)*0x11)
#define SCALE_8_4(VAL_) ((VAL_) / 0x11)
#define SCALE_3_8(VAL_) ((VAL_)*0x24)
#define SCALE_8_3(VAL_) ((VAL_) / 0x24)

// SCREEN_WIDTH and SCREEN_HEIGHT are defined in the headerfile
#define HALF_SCREEN_WIDTH (SCREEN_WIDTH / 2.f)
#define HALF_SCREEN_HEIGHT (SCREEN_HEIGHT / 2.f)

#define RATIO_X (gfx_current_dimensions.width / (float)SCREEN_WIDTH)
#define RATIO_Y (gfx_current_dimensions.height / (float)SCREEN_HEIGHT)

#define MAX_BUFFERED 256
#define MAX_LIGHTS 4
#define MAX_VERTICES 128
#define MAX_VERTEX_COLORS 64

#define TEXTURE_CACHE_MAX_SIZE 2048 // §33: doubled so a texture-heavy area holds its working set resident and
                                    // doesn't evict + RE-FREAD textures from the ROM under streaming (the slow
                                    // SD). Bounded by the backend TEX_POOL_USABLE (3584); watch memlog texOOM —
                                    // more resident textures = more VRAM, competes with the facade bakes.

#define C0(pos, width) ((cmd->words.w0 >> (pos)) & ((1U << width) - 1))
#define C1(pos, width) ((cmd->words.w1 >> (pos)) & ((1U << width) - 1))

struct RGBA {
    uint8_t r, g, b, a;
};

struct NormalColor {
    union {
        struct { uint8_t r, g, b, a; };
        struct { int8_t x, y, z, w; };
    };
};

struct LoadedVertex {
    float x, y, z, w;
    float u, v;
    struct RGBA color;
    uint8_t fog;
    uint8_t clip_rej;
};

static struct {
    TextureCacheMap map;
    std::list<TextureCacheMapIter> lru;
    std::vector<uint32_t> free_texture_ids;
} gfx_texture_cache;

struct ColorCombiner {
    uint64_t shader_id0;
    uint32_t shader_id1;
    bool used_textures[2];
    struct ShaderProgram* prg[16];
    uint8_t shader_input_mapping[2][7];
};

static std::map<ColorCombinerKey, struct ColorCombiner> color_combiner_pool;
static std::map<ColorCombinerKey, struct ColorCombiner>::iterator prev_combiner = color_combiner_pool.end();

static uint8_t* tex_upload_buffer = nullptr;
static uint32_t turok_tex_upload_capacity = 0; /* bytes; set when tex_upload_buffer is allocated (gfx_pc.cpp ~3081) */

/* The N64 RSP modelview matrix stack was 10 deep. Turok's DoDraw recurses the per-object skeletal
 * node hierarchy emitting a paired gSPMatrix(PUSH)/gSPPopMatrix per non-only-child node (romstruc.c
 * ~10390/10425), so a deep creature/boss hierarchy can nest past the original depth-11 cap. When the
 * cap was hit, the PUSH was dropped but its matching POP still executed -> the camera-base matrix was
 * popped away -> the whole frame (world + weapon + HUD) drew with a corrupted modelview. That is the
 * interactive-only "camera + HUD completely fucked up while walking" regression (a shallow headless
 * enemy never exceeded 11, so push_dropped read 0 in headless tests). Give generous headroom. */
#define MODELVIEW_STACK_DEPTH 64

static struct RSP {
    float modelview_matrix_stack[MODELVIEW_STACK_DEPTH][4][4];
    uint8_t modelview_matrix_stack_size;

    float MP_matrix[4][4];
    float P_matrix[4][4];

    Light_t lookat[2];
    bool lookat_enabled;

    Light_t current_lights[MAX_LIGHTS + 1];
    float current_lights_coeffs[MAX_LIGHTS][3];
    float current_lookat_coeffs[2][3]; // lookat_x, lookat_y
    uint8_t current_num_lights;        // includes ambient light
    bool lights_changed;

    uint32_t geometry_mode;
    int16_t fog_mul, fog_offset;

    uint32_t extra_geometry_mode;

    uint32_t aspect_mode;
    float aspect_ofs;
    float aspect_scale;

    struct {
        // U0.16
        uint16_t s, t;
    } texture_scaling_factor;

    /* +4 = shared scratch (texrect corners / 3DS near-clip inserts); +4 more = dedicated
     * G_LINE3D quad corners (slots MAX_VERTICES+4..7) so a line expansion can't collide
     * with either scratch user. */
    struct LoadedVertex loaded_vertices[MAX_VERTICES + 8];

    const struct NormalColor *vertex_colors; //[MAX_VERTEX_COLORS];
} rsp;

struct RawTexMetadata {
    uint16_t width, height;
    float h_byte_scale = 1, v_pixel_scale = 1;
};

struct LoadedTexture {
    const uint8_t* addr;
    uint32_t orig_size_bytes;
    uint32_t full_size_bytes; // full_image_line_size_bytes * height
    uint32_t size_bytes; // line_size_bytes * height
    uint32_t full_image_line_size_bytes;
    uint32_t line_size_bytes;
    uint32_t tex_flags;
    struct RawTexMetadata raw_tex_metadata;
};

static struct RDP {
    uint16_t palette[256];
    const uint8_t* palette_addrs[2];
    uint32_t palette_fmt;
    struct {
        const uint8_t* addr;
        uint8_t siz;
        uint32_t width;
        uint32_t tex_flags;
        struct RawTexMetadata raw_tex_metadata;
    } texture_to_load;
    struct {
        uint8_t fmt;
        uint8_t siz;
        uint8_t cms, cmt;
        uint8_t shifts, shiftt;
        uint16_t uls, ult, lrs, lrt; // U10.2
        uint16_t width, height;      // in texels
        uint16_t tmem;               // 0-511, in 64-bit word units
        uint32_t line_size_bytes;
        uint8_t palette;
    } texture_tile[8];
    LoadedTexture loaded_texture[512]; // for each tmem location
    bool textures_changed[2];

    uint8_t first_tile_index;
    uint8_t tex_min_lod;
    uint8_t tex_max_lod;

    uint32_t other_mode_l, other_mode_h;
    uint64_t combine_mode;
    bool grayscale;
    bool tex_lod;
    bool tex_detail;

    uint8_t prim_lod_fraction;
    struct RGBA env_color, prim_color, fog_color, fill_color, grayscale_color, blend_color;
    struct XYWidthHeight viewport, scissor;
    bool viewport_or_scissor_changed;
    void* z_buf_address;
    void* color_image_address;

    int16_t subpixel_ofs_x;
    int16_t subpixel_ofs_y;
} rdp;

static struct RenderingState {
    uint8_t depth_mode;
    bool alpha_blend;
    bool modulate;
    struct XYWidthHeight viewport, scissor;
    struct ShaderProgram* shader_program;
    TextureCacheNode* textures[SHADER_MAX_TEXTURES];
} rendering_state;

struct GfxDimensions gfx_current_window_dimensions;
int32_t gfx_current_window_position_x;
int32_t gfx_current_window_position_y;
struct GfxDimensions gfx_current_dimensions;
static struct GfxDimensions gfx_prev_dimensions;
struct XYWidthHeight gfx_current_game_window_viewport;
struct XYWidthHeight gfx_current_native_viewport;
float gfx_current_native_aspect = 4.f / 3.f;
#ifdef PLATFORM_PORT
static bool s_proj_is_2d = false;   /* widescreen: current projection is ortho (2D HUD/menu) vs perspective (3D) */
#endif
bool gfx_framebuffers_enabled = true;
bool gfx_detail_textures_enabled = true;

static bool game_renders_to_framebuffer;
static int game_framebuffer;
static int game_framebuffer_msaa_resolved;

uint32_t gfx_msaa_level = 1;

static bool dropped_frame;

static float buf_vbo[MAX_BUFFERED * (32 * 3)]; // 3 vertices in a triangle and 32 floats per vtx
static size_t buf_vbo_len;
static size_t buf_vbo_num_tris;
#ifdef BK_GFX_TRACE
size_t g_bk_tris_drawn = 0;
#endif

static struct GfxWindowManagerAPI* gfx_wapi;
static struct GfxRenderingAPI* gfx_rapi;

static uintptr_t segmentPointers[16];

struct FBInfo {
    uint32_t orig_width, orig_height;
    uint32_t applied_width, applied_height;
    bool upscale, autoresize;
};

static bool fbActive = 0;
static std::map<int, FBInfo>::iterator active_fb;
static std::map<int, FBInfo> framebuffers;

static constexpr float clampf(const float x, const float min, const float max) {
    return (x < min) ? min : (x > max) ? max : x;
}

size_t g_turok_tris = 0;   /* PORT: per-frame triangle count probe (turok_gfx.c reads+resets) */
static void gfx_flush(void) {
    if (buf_vbo_len > 0) {
        g_turok_tris += buf_vbo_num_tris;
        // FOG (backends with a fixed-function fog unit, i.e. 3DS — NULL on GL, which fogs in the
        // fragment shader from the buf_vbo fog floats). Pass this batch's N64 fog state; the backend
        // builds a depth FogLut from mul/offset. (CLAUDE.md §3.3/§20.12.)
        if (gfx_rapi->set_fog) {
            gfx_rapi->set_fog((rsp.geometry_mode & G_FOG) != 0, rsp.fog_mul, rsp.fog_offset,
                              rdp.fog_color.r, rdp.fog_color.g, rdp.fog_color.b);
        }
#ifdef BK_GFX_TRACE
        { extern size_t g_bk_tris_drawn; g_bk_tris_drawn += buf_vbo_num_tris; }
        /* TEMP tile-pair probe: for the BK character lerp combiner, dump the
         * render tile pair (lod/detail mode, tmem, dims, source addr) to see
         * whether the dark/light TEXEL0/TEXEL1 pair is really two textures. */
        static int sProbeLo = -2, sProbeHi = -2;
        if (sProbeLo == -2) {
            const char *e = getenv("BK_PROBE_LO"); sProbeLo = e ? atoi(e) : -1;
            e = getenv("BK_PROBE_HI"); sProbeHi = e ? atoi(e) : -1;
        }
        if (bkTraceFrame >= sProbeLo && bkTraceFrame <= sProbeHi) {
            const int ft = rdp.first_tile_index;
            fprintf(stderr, "[TILEPAIR] f=%d tris=%zu cm=%08lx.%08lx lod=%d det=%d ft=%d "
                    "t0(tmem=%u w=%u h=%u line=%u fmt=%d/%d) t1(tmem=%u w=%u h=%u line=%u fmt=%d/%d) "
                    "addr0=%p addr1=%p\n",
                    bkTraceFrame, (size_t)buf_vbo_num_tris,
                    (unsigned long)(rdp.combine_mode >> 32), (unsigned long)rdp.combine_mode,
                    (int)rdp.tex_lod, (int)rdp.tex_detail, ft,
                    rdp.texture_tile[ft].tmem, rdp.texture_tile[ft].width, rdp.texture_tile[ft].height,
                    rdp.texture_tile[ft].line_size_bytes, rdp.texture_tile[ft].fmt, rdp.texture_tile[ft].siz,
                    rdp.texture_tile[ft+1].tmem, rdp.texture_tile[ft+1].width, rdp.texture_tile[ft+1].height,
                    rdp.texture_tile[ft+1].line_size_bytes, rdp.texture_tile[ft+1].fmt, rdp.texture_tile[ft+1].siz,
                    (void *)rdp.loaded_texture[rdp.texture_tile[ft].tmem].addr,
                    (void *)rdp.loaded_texture[rdp.texture_tile[ft+1].tmem].addr);
            const size_t nverts = (size_t)buf_vbo_num_tris * 3;
            const size_t stride = buf_vbo_len / nverts;
            float mnx = 1e9f, mxx = -1e9f, mny = 1e9f, mxy = -1e9f, mnz = 1e9f, mxz = -1e9f;
            for (size_t k = 0; k < nverts; k++) {
                const float *v = &buf_vbo[k * stride];
                float w = v[3] != 0.f ? v[3] : 1.f;
                float x = v[0] / w, y = v[1] / w, z = v[2] / w;
                if (x < mnx) mnx = x; if (x > mxx) mxx = x;
                if (y < mny) mny = y; if (y > mxy) mxy = y;
                if (z < mnz) mnz = z; if (z > mxz) mxz = z;
            }
            fprintf(stderr, "[TILEPAIR+] X[%.2f,%.2f] Y[%.2f,%.2f] Z[%.2f,%.2f] stride=%zu "
                    "v0tail=%.2f,%.2f,%.2f,%.2f om_l=%08x prim=%02x%02x%02x%02x env=%02x%02x%02x%02x geo_fog=%d\n",
                    mnx, mxx, mny, mxy, mnz, mxz, stride,
                    buf_vbo[stride-4], buf_vbo[stride-3], buf_vbo[stride-2], buf_vbo[stride-1],
                    rdp.other_mode_l,
                    rdp.prim_color.r, rdp.prim_color.g, rdp.prim_color.b, rdp.prim_color.a,
                    rdp.env_color.r, rdp.env_color.g, rdp.env_color.b, rdp.env_color.a,
                    (rsp.geometry_mode & G_FOG) ? 1 : 0);
            fprintf(stderr, "[TILEPAIR vp] vp=%d,%d %ux%u sc=%d,%d %ux%u\n",
                    rdp.viewport.x, rdp.viewport.y, rdp.viewport.width, rdp.viewport.height,
                    rdp.scissor.x, rdp.scissor.y, rdp.scissor.width, rdp.scissor.height);
        }
#endif
#ifdef BK_TRACE
        /* SHARED sliver detector (PC + 3DS): flag draw batches whose NDC
         * bounding box collapses to a thin vertical strip — the signature of
         * the squashed character meshes ("faint lines"). Runs every frame so
         * the two targets' timelines don't need to align. */
        if (buf_vbo_num_tris >= 12) {
            static int sSliverN = 0;
            const size_t nverts = (size_t)buf_vbo_num_tris * 3;
            const size_t stride = buf_vbo_len / nverts;
            float mnx = 1e9f, mxx = -1e9f, mny = 1e9f, mxy = -1e9f;
            for (size_t k = 0; k < nverts; k++) {
                const float *v = &buf_vbo[k * stride];
                float w = v[3] != 0.f ? v[3] : 1.f, x = v[0] / w, y = v[1] / w;
                if (x < mnx) mnx = x;
                if (x > mxx) mxx = x;
                if (y < mny) mny = y;
                if (y > mxy) mxy = y;
            }
            if ((mxx - mnx) < 0.06f && (mxy - mny) > 0.25f && sSliverN < 40) {
                sSliverN++;
                BK_TR(BK_TR_DRAW, "SLIVER tris=%zu cm=%08lx.%08lx X[%.3f,%.3f] Y[%.2f,%.2f]",
                      (size_t)buf_vbo_num_tris,
                      (unsigned long)(rdp.combine_mode >> 32),
                      (unsigned long)(rdp.combine_mode & 0xffffffffu),
                      mnx, mxx, mny, mxy);
            }
        }
#endif
#ifdef BK_GFX_TRACE
        /* TEMP (logo-scene hunt): full per-flush dump for the black intro gap */
        if (bkTraceFrame >= 180 && bkTraceFrame <= 184) {
            const size_t nverts = (size_t)buf_vbo_num_tris * 3;
            const size_t stride = buf_vbo_len / nverts;
            float mnx = 1e9f, mxx = -1e9f, mny = 1e9f, mxy = -1e9f, mnz = 1e9f, mxz = -1e9f;
            for (size_t k = 0; k < nverts; k++) {
                const float *v = &buf_vbo[k * stride];
                float w = v[3] != 0.f ? v[3] : 1.f;
                float x = v[0] / w, y = v[1] / w, z = v[2] / w;
                if (x < mnx) mnx = x; if (x > mxx) mxx = x;
                if (y < mny) mny = y; if (y > mxy) mxy = y;
                if (z < mnz) mnz = z; if (z > mxz) mxz = z;
            }
            uint64_t _id0 = 0;
            bool _ut0 = false, _ut1 = false;
            if (prev_combiner != color_combiner_pool.end()) {
                _id0 = prev_combiner->second.shader_id0;
                _ut0 = prev_combiner->second.used_textures[0];
                _ut1 = prev_combiner->second.used_textures[1];
            }
            fprintf(stderr, "[LOGO] f=%d tris=%zu id0=%08lx.%08lx ut=%d%d oml=%08lx str=%zu "
                    "X[%.2f,%.2f] Y[%.2f,%.2f] Z[%.2f,%.2f]\n",
                    bkTraceFrame, (size_t)buf_vbo_num_tris,
                    (unsigned long)(_id0 >> 32), (unsigned long)(_id0 & 0xffffffffu),
                    (int)_ut0, (int)_ut1,
                    (unsigned long)rdp.other_mode_l, stride,
                    mnx, mxx, mny, mxy, mnz, mxz);
            for (size_t k = 0; k < nverts && k < 3; k++) {
                const float *v = &buf_vbo[k * stride];
                fprintf(stderr, "[LOGO]   v%zu floats:", k);
                for (size_t j = 0; j < stride; j++) fprintf(stderr, " %.3f", v[j]);
                fprintf(stderr, "\n");
            }
        }
#endif
        gfx_rapi->draw_triangles(buf_vbo, buf_vbo_len, buf_vbo_num_tris);
        buf_vbo_len = 0;
        buf_vbo_num_tris = 0;
    }
}

static struct ShaderProgram* gfx_lookup_or_create_shader_program(uint64_t shader_id0, uint32_t shader_id1) {
    struct ShaderProgram* prg = gfx_rapi->lookup_shader(shader_id0, shader_id1);
    if (prg == NULL) {
        gfx_rapi->unload_shader(rendering_state.shader_program);
        prg = gfx_rapi->create_and_load_new_shader(shader_id0, shader_id1);
        rendering_state.shader_program = prg;
    }
    return prg;
}

static const char* ccmux_to_string(uint32_t ccmux) {
    static const char* const tbl[] = {
        "G_CCMUX_COMBINED",
        "G_CCMUX_TEXEL0",
        "G_CCMUX_TEXEL1",
        "G_CCMUX_PRIMITIVE",
        "G_CCMUX_SHADE",
        "G_CCMUX_ENVIRONMENT",
        "G_CCMUX_1",
        "G_CCMUX_COMBINED_ALPHA",
        "G_CCMUX_TEXEL0_ALPHA",
        "G_CCMUX_TEXEL1_ALPHA",
        "G_CCMUX_PRIMITIVE_ALPHA",
        "G_CCMUX_SHADE_ALPHA",
        "G_CCMUX_ENV_ALPHA",
        "G_CCMUX_LOD_FRACTION",
        "G_CCMUX_PRIM_LOD_FRAC",
        "G_CCMUX_K5",
    };
    if (ccmux > 15) {
        return "G_CCMUX_0";

    } else {
        return tbl[ccmux];
    }
}

static const char* acmux_to_string(uint32_t acmux) {
    static const char* const tbl[] = {
        "G_ACMUX_COMBINED or G_ACMUX_LOD_FRACTION",
        "G_ACMUX_TEXEL0",
        "G_ACMUX_TEXEL1",
        "G_ACMUX_PRIMITIVE",
        "G_ACMUX_SHADE",
        "G_ACMUX_ENVIRONMENT",
        "G_ACMUX_1 or G_ACMUX_PRIM_LOD_FRAC",
        "G_ACMUX_0",
    };
    return tbl[acmux];
}

static void gfx_generate_cc(struct ColorCombiner* comb, const ColorCombinerKey& key) {
    bool is_2cyc = (key.options & (uint64_t)SHADER_OPT_2CYC) != 0;

    uint8_t c[2][2][4] = { { { 0 } } };
    uint64_t shader_id0 = 0;
    uint32_t shader_id1 = key.options;
    uint8_t shader_input_mapping[2][7] = { { 0 } };
    bool used_textures[2] = { false, false };
    for (int i = 0; i < 2 && (i == 0 || is_2cyc); i++) {
        uint32_t rgb_a = (key.combine_mode >> (i * 28)) & 0xf;
        uint32_t rgb_b = (key.combine_mode >> (i * 28 + 4)) & 0xf;
        uint32_t rgb_c = (key.combine_mode >> (i * 28 + 8)) & 0x1f;
        uint32_t rgb_d = (key.combine_mode >> (i * 28 + 13)) & 7;
        uint32_t alpha_a = (key.combine_mode >> (i * 28 + 16)) & 7;
        uint32_t alpha_b = (key.combine_mode >> (i * 28 + 16 + 3)) & 7;
        uint32_t alpha_c = (key.combine_mode >> (i * 28 + 16 + 6)) & 7;
        uint32_t alpha_d = (key.combine_mode >> (i * 28 + 16 + 9)) & 7;

        if (rgb_a >= 8) {
            rgb_a = G_CCMUX_0;
        }
        if (rgb_b >= 8) {
            rgb_b = G_CCMUX_0;
        }
        if (rgb_c >= 16) {
            rgb_c = G_CCMUX_0;
        }
        if (rgb_d == 7) {
            rgb_d = G_CCMUX_0;
        }

        if (rgb_a == rgb_b || rgb_c == G_CCMUX_0) {
            // Normalize
            rgb_a = G_CCMUX_0;
            rgb_b = G_CCMUX_0;
            rgb_c = G_CCMUX_0;
        }
        if (alpha_a == alpha_b || alpha_c == G_ACMUX_0) {
            // Normalize
            alpha_a = G_ACMUX_0;
            alpha_b = G_ACMUX_0;
            alpha_c = G_ACMUX_0;
        }
        if (i == 1) {
            if (rgb_a != G_CCMUX_COMBINED && rgb_b != G_CCMUX_COMBINED && rgb_c != G_CCMUX_COMBINED &&
                rgb_d != G_CCMUX_COMBINED) {
                // First cycle RGB not used, so clear it away
                c[0][0][0] = c[0][0][1] = c[0][0][2] = c[0][0][3] = G_CCMUX_0;
            }
            if (rgb_c != G_CCMUX_COMBINED_ALPHA && alpha_a != G_ACMUX_COMBINED && alpha_b != G_ACMUX_COMBINED &&
                alpha_d != G_ACMUX_COMBINED) {
                // First cycle ALPHA not used, so clear it away
                c[0][1][0] = c[0][1][1] = c[0][1][2] = c[0][1][3] = G_ACMUX_0;
            }
        }

        c[i][0][0] = rgb_a;
        c[i][0][1] = rgb_b;
        c[i][0][2] = rgb_c;
        c[i][0][3] = rgb_d;
        c[i][1][0] = alpha_a;
        c[i][1][1] = alpha_b;
        c[i][1][2] = alpha_c;
        c[i][1][3] = alpha_d;
    }
    if (!is_2cyc) {
        for (int i = 0; i < 2; i++) {
            for (int k = 0; k < 4; k++) {
                c[1][i][k] = i == 0 ? G_CCMUX_0 : G_ACMUX_0;
            }
        }
    }
    {
        uint8_t input_number[32] = { 0 };
        int next_input_number = SHADER_INPUT_1;
        for (int i = 0; i < 2 && (i == 0 || is_2cyc); i++) {
            for (int j = 0; j < 4; j++) {
                uint32_t val = 0;
                switch (c[i][0][j]) {
                    case G_CCMUX_0:
                        val = SHADER_0;
                        break;
                    case G_CCMUX_1:
                        val = SHADER_1;
                        break;
                    case G_CCMUX_TEXEL0:
                        val = SHADER_TEXEL0;
                        used_textures[0] = true;
                        break;
                    case G_CCMUX_TEXEL1:
                        val = SHADER_TEXEL1;
                        used_textures[1] = true;
                        break;
                    case G_CCMUX_TEXEL0_ALPHA:
                        val = SHADER_TEXEL0A;
                        used_textures[0] = true;
                        break;
                    case G_CCMUX_TEXEL1_ALPHA:
                        val = SHADER_TEXEL1A;
                        used_textures[1] = true;
                        break;
                    case G_CCMUX_NOISE:
                        val = SHADER_NOISE;
                        break;
                    case G_CCMUX_PRIMITIVE:
                    case G_CCMUX_PRIMITIVE_ALPHA:
                    case G_CCMUX_PRIM_LOD_FRAC:
                    case G_CCMUX_SHADE:
                    case G_CCMUX_SHADE_ALPHA:
                    case G_CCMUX_ENVIRONMENT:
                    case G_CCMUX_ENV_ALPHA:
                    case G_CCMUX_LOD_FRACTION:
                        if (input_number[c[i][0][j]] == 0) {
                            shader_input_mapping[0][next_input_number - 1] = c[i][0][j];
                            input_number[c[i][0][j]] = next_input_number++;
                        }
                        val = input_number[c[i][0][j]];
                        break;
                    case G_CCMUX_COMBINED:
                        val = SHADER_COMBINED;
                        break;
                    default:
                        sysLogPrintf(LOG_WARNING, "Unsupported ccmux: %d", c[i][0][j]);
                        break;
                }
                shader_id0 |= (uint64_t)val << (i * 32 + j * 4);
            }
        }
    }
    {
        uint8_t input_number[16] = { 0 };
        int next_input_number = SHADER_INPUT_1;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 4; j++) {
                uint32_t val = 0;
                switch (c[i][1][j]) {
                    case G_ACMUX_0:
                        val = SHADER_0;
                        break;
                    case G_ACMUX_TEXEL0:
                        val = SHADER_TEXEL0;
                        used_textures[0] = true;
                        break;
                    case G_ACMUX_TEXEL1:
                        val = SHADER_TEXEL1;
                        used_textures[1] = true;
                        break;
                    case G_ACMUX_LOD_FRACTION:
                        // case G_ACMUX_COMBINED: same numerical value
                        if (j != 2) {
                            val = SHADER_COMBINED;
                            break;
                        }
                        c[i][1][j] = G_CCMUX_LOD_FRACTION;
                        [[fallthrough]]; // for G_ACMUX_LOD_FRACTION
                    case G_ACMUX_1:
                        // case G_ACMUX_PRIM_LOD_FRAC: same numerical value
                        if (j != 2) {
                            val = SHADER_1;
                            break;
                        }
                        [[fallthrough]]; // for G_ACMUX_PRIM_LOD_FRAC
                    case G_ACMUX_PRIMITIVE:
                    case G_ACMUX_SHADE:
                    case G_ACMUX_ENVIRONMENT:
                        if (input_number[c[i][1][j]] == 0) {
                            shader_input_mapping[1][next_input_number - 1] = c[i][1][j];
                            input_number[c[i][1][j]] = next_input_number++;
                        }
                        val = input_number[c[i][1][j]];
                        break;
                }
                shader_id0 |= (uint64_t)val << (i * 32 + 16 + j * 4);
            }
        }
    }
    comb->shader_id0 = shader_id0;
    comb->shader_id1 = shader_id1;
    comb->used_textures[0] = used_textures[0];
    comb->used_textures[1] = used_textures[1];
    // comb->prg = gfx_lookup_or_create_shader_program(shader_id0, shader_id1);
    memcpy(comb->shader_input_mapping, shader_input_mapping, sizeof(shader_input_mapping));
}

static struct ColorCombiner* gfx_lookup_or_create_color_combiner(const ColorCombinerKey& key) {
    if (prev_combiner != color_combiner_pool.end() && prev_combiner->first == key) {
        return &prev_combiner->second;
    }

    prev_combiner = color_combiner_pool.find(key);
    if (prev_combiner != color_combiner_pool.end()) {
        return &prev_combiner->second;
    }
    gfx_flush();
    prev_combiner = color_combiner_pool.insert(std::make_pair(key, ColorCombiner())).first;
    gfx_generate_cc(&prev_combiner->second, key);
    return &prev_combiner->second;
}

void gfx_texture_cache_clear() {
    gfx_flush();
    for (const auto& entry : gfx_texture_cache.map) {
        gfx_texture_cache.free_texture_ids.push_back(entry.second.texture_id);
    }
    gfx_texture_cache.map.clear();
    gfx_texture_cache.lru.clear();
    rdp.textures_changed[0] = rdp.textures_changed[1] = true;
    memset(rendering_state.textures, 0, sizeof(rendering_state.textures));
    // Tell the backend the cache was wiped (all ids orphaned without delete_texture) so it can
    // drop stale per-texture state — the 3DS pre-tile bake. NULL on backends that don't need it.
    if (gfx_rapi && gfx_rapi->invalidate_texture_cache) gfx_rapi->invalidate_texture_cache();
}

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
/* Cheap O(1) content signature of a texture's source bytes: FNV over the size + four sampled words
 * (start, 1/3, 2/3, end). Two different textures at the same reused address almost always differ here. */
static inline uint32_t tex_content_sig(const uint8_t* a, size_t n) {
    if (!a) return 0;
    uint32_t h = 2166136261u ^ (uint32_t)n;
    size_t o[4] = { 0, n / 3, (2 * n) / 3, n >= 4 ? n - 4 : 0 };
    for (int k = 0; k < 4; k++) {
        size_t off = o[k] & ~(size_t)3;
        if (off + 4 <= n) { uint32_t w; memcpy(&w, a + off, 4); h = (h ^ w) * 16777619u; }
    }
    return h ? h : 1u; /* reserve 0 as "unset" */
}
int g_turok_tex_stale = 0;   /* diagnostic: count of stale-hit re-uploads (TUROK_TEXLOG) */
#endif

/* A cache node about to be erased may STILL be referenced by a rendering_state texture slot — the
 * OTHER texture unit, or this unit from a prior frame whose rdp.textures_changed went false and so
 * was never re-imported. Freeing it without clearing those slots leaves gfx_sp_tri1 reading AND
 * writing a freed node (use-after-free / write-after-free → heap corruption every frame). Clear the
 * slot and mark it changed so the texture is re-imported before the next draw. Mirrors the
 * slot-clearing gfx_texture_cache_delete already does when deleting by address. */
static inline void gfx_texture_cache_forget_node(const TextureCacheNode* node) {
    for (int s = 0; s < 2; ++s) {
        if (rendering_state.textures[s] == node) {
            rendering_state.textures[s] = nullptr;
            rdp.textures_changed[s] = true;
        }
    }
}

static bool gfx_texture_cache_lookup(int i, const TextureCacheKey& key, uint32_t content_sig) {
    TextureCacheMap::iterator it = gfx_texture_cache.map.find(key);
    TextureCacheNode** n = &rendering_state.textures[i];

    if (it != gfx_texture_cache.map.end()) {
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
        /* WRONG TEXTURES AT EXTENDED DRAW DISTANCE — the cache is keyed on the source ADDRESS, but the cart
         * cache streams+relocates blocks, so a freed address can be reused for a DIFFERENT texture. When it
         * is, this cached entry's stored content no longer matches the data now at that address, so binding
         * it would draw the wrong texture. Re-validate the content signature: on a mismatch, evict this stale
         * entry and fall through to re-upload from the current data. (Same class the 3DS handles in
         * gfx_citro3d; the PC backend has no such layer, so we validate here.) */
        if (content_sig && it->second.content_sig && it->second.content_sig != content_sig) {
            g_turok_tex_stale++;
            gfx_texture_cache_forget_node(&*it);   /* clear any slot pointing at this node before freeing it (UAF/WAF) */
            gfx_texture_cache.free_texture_ids.push_back(it->second.texture_id);
            gfx_texture_cache.lru.erase(it->second.lru_location);
            gfx_texture_cache.map.erase(it);
        } else
#endif
        {
        gfx_rapi->select_texture(i, it->second.texture_id, it->second.linear_filter);
        *n = &*it;
        gfx_texture_cache.lru.splice(gfx_texture_cache.lru.end(), gfx_texture_cache.lru,
                                     it->second.lru_location); // move to back
        return true;
        }
    }

    if (gfx_texture_cache.map.size() >= TEXTURE_CACHE_MAX_SIZE) {
        // Remove the texture that was least recently used
        it = gfx_texture_cache.lru.front().it;
        gfx_texture_cache_forget_node(&*it);   /* same UAF/WAF guard: LRU-evicted node may still be a live slot */
        gfx_texture_cache.free_texture_ids.push_back(it->second.texture_id);
        gfx_texture_cache.map.erase(it);
        gfx_texture_cache.lru.pop_front();
    }

    uint32_t texture_id;
    if (!gfx_texture_cache.free_texture_ids.empty()) {
        texture_id = gfx_texture_cache.free_texture_ids.back();
        gfx_texture_cache.free_texture_ids.pop_back();
    } else {
        texture_id = gfx_rapi->new_texture();
    }

    it = gfx_texture_cache.map.insert(std::make_pair(key, TextureCacheValue())).first;
    TextureCacheNode* node = &*it;
    node->second.texture_id = texture_id;
#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
    node->second.content_sig = content_sig;   /* remember the source content so a later reuse of this address is caught */
#endif
    node->second.lru_location = gfx_texture_cache.lru.insert(gfx_texture_cache.lru.end(), { it });

    gfx_rapi->select_texture(i, texture_id, false);
    gfx_rapi->set_sampler_parameters(i, false, 0, 0, rdp.tex_lod);
    *n = node;
    return false;
}

void gfx_texture_cache_delete(const uint8_t* orig_addr) {
    gfx_flush();

    for (int i = 0; i < 2; ++i) {
        if (rendering_state.textures[i] && rendering_state.textures[i]->first.texture_addr == orig_addr) {
            rdp.textures_changed[i] = true;
            rendering_state.textures[i] = nullptr;
        }
    }

    while (gfx_texture_cache.map.bucket_count() > 0) {
        TextureCacheKey key = { orig_addr, { 0 }, 0, 0 }; // bucket index only depends on the address
        size_t bucket = gfx_texture_cache.map.bucket(key);
        bool again = false;
        for (auto it = gfx_texture_cache.map.begin(bucket); it != gfx_texture_cache.map.end(bucket); ++it) {
            if (it->first.texture_addr == orig_addr) {
                gfx_texture_cache.lru.erase(it->second.lru_location);
                gfx_texture_cache.free_texture_ids.push_back(it->second.texture_id);
                gfx_texture_cache.map.erase(it->first);
                again = true;
                break;
            }
        }
        if (!again) {
            break;
        }
    }
}

void gfx_texture_cache_delete_range(const uint8_t* start, const uint8_t* end) {
    gfx_flush();

    for (int i = 0; i < 2; ++i) {
        if (rendering_state.textures[i]
                && rendering_state.textures[i]->first.texture_addr >= start
                && rendering_state.textures[i]->first.texture_addr < end) {
            rdp.textures_changed[i] = true;
            rendering_state.textures[i] = nullptr;
        }
    }

    for (auto it = gfx_texture_cache.map.begin(); it != gfx_texture_cache.map.end(); ) {
        if (it->first.texture_addr >= start && it->first.texture_addr < end) {
            gfx_texture_cache.lru.erase(it->second.lru_location);
            gfx_texture_cache.free_texture_ids.push_back(it->second.texture_id);
            it = gfx_texture_cache.map.erase(it);
        } else {
            ++it;
        }
    }
}

/* C hook for the game side (PORT-gated in src/**): the falling-jiggies
 * transition rewrites a model's texel data IN PLACE (CPU copy of the
 * framebuffer) — the cache is keyed by address, so without this the first
 * (pre-copy, black) upload is served forever. */
extern "C" void bkPortTexRangeInvalidate(const void *start, const void *end) {
    gfx_texture_cache_delete_range((const uint8_t *)start, (const uint8_t *)end);
}

/* BK DLs can leave the render tile's LINE field 0 (load-block style); the
 * imports below divide by it. Derive the row size from the tile's texel width
 * instead (num/den = bytes per texel), mirroring the CI4/CI8 fix. */
static uint32_t tile_line_or_fallback(int tile, uint32_t num, uint32_t den) {
    uint32_t line = rdp.texture_tile[tile].line_size_bytes;
    if (line == 0) {
        line = (rdp.texture_tile[tile].width * num + den - 1) / den;
        if (line == 0) line = 1;
    }
    return line;
}

static void import_texture_rgba16(int tile, const LoadedTexture& loaded_texture, bool gen_mipmaps) {
    const uint8_t* addr = loaded_texture.addr;
    const uint32_t size_bytes = loaded_texture.size_bytes;
    const uint32_t full_image_line_size_bytes =
        loaded_texture.full_image_line_size_bytes;
    const uint32_t line_size_bytes = loaded_texture.line_size_bytes;
    // SUPPORT_CHECK(full_image_line_size_bytes == line_size_bytes);
    // TODO: this trips in some places with a garbage size in full_image_line_size_bytes
    // probably wherever framebuffer effects are used

    uint8_t *dest = tex_upload_buffer;
    for (uint32_t i = 0; i < size_bytes / 2; i++, dest += 4) {
        const uint16_t col16 = (addr[2 * i] << 8) | addr[2 * i + 1];
        const uint8_t a = col16 & 1;
        const uint8_t r = col16 >> 11;
        const uint8_t g = (col16 >> 6) & 0x1f;
        const uint8_t b = (col16 >> 1) & 0x1f;
        dest[0] = SCALE_5_8(r);
        dest[1] = SCALE_5_8(g);
        dest[2] = SCALE_5_8(b);
        dest[3] = a ? 255 : 0;
    }

    const uint32_t safe_line = tile_line_or_fallback(tile, 2, 1);
    const uint32_t width = safe_line / 2;
    const uint32_t height = size_bytes / safe_line;

	gfx_rapi->upload_texture(tex_upload_buffer, width, height, gen_mipmaps);
    // DumpTexture(loaded_texture.otr_path, rgba32_buf, width, height);
}

static void import_texture_rgba32(int tile, const LoadedTexture& loaded_texture, bool gen_mipmaps) {
    const RawTexMetadata* metadata = &loaded_texture.raw_tex_metadata;
    const uint8_t* addr = loaded_texture.addr;
    const uint32_t size_bytes = loaded_texture.size_bytes;
    const uint32_t full_image_line_size_bytes =
        loaded_texture.full_image_line_size_bytes;
    const uint32_t line_size_bytes = loaded_texture.line_size_bytes;
    SUPPORT_CHECK(full_image_line_size_bytes == line_size_bytes);

    uint32_t *dest = (uint32_t *)tex_upload_buffer;
    const uint32_t *src = (const uint32_t *)addr;
    for (uint32_t i = 0; i < size_bytes; i += 4, ++dest, ++src) {
#ifdef PLATFORM_PORT
        /* N64 RGBA32 texels are 4 sequential bytes [R,G,B,A], which already match
         * GL_RGBA/GL_UNSIGNED_BYTE byte order — copy verbatim. Verified from real
         * texture data (e.g. bytes 02 02 02 ff = opaque dark grey); PD_BE32 would
         * reverse it to [A,B,G,R] (ff 02 02 02 = red, alpha ~0). */
        *dest = *src;
#else
        *dest = PD_BE32(*src);
#endif
    }

    const uint32_t safe_line = tile_line_or_fallback(tile, 2, 1);
    const uint32_t width = safe_line / 2;
    const uint32_t height = (size_bytes / 2) / safe_line;
	gfx_rapi->upload_texture(tex_upload_buffer, width, height, gen_mipmaps);
    // DumpTexture(loaded_texture.otr_path, addr, width, height);
}

static void import_texture_ia4(int tile, const LoadedTexture& loaded_texture, bool gen_mipmaps) {
    const RawTexMetadata* metadata = &loaded_texture.raw_tex_metadata;
    const uint8_t* addr = loaded_texture.addr;
    const uint32_t size_bytes = loaded_texture.size_bytes;
    const uint32_t full_image_line_size_bytes =
        loaded_texture.full_image_line_size_bytes;
    const uint32_t line_size_bytes = loaded_texture.line_size_bytes;
    SUPPORT_CHECK(full_image_line_size_bytes == line_size_bytes);

    uint8_t *dest = tex_upload_buffer;
    for (uint32_t i = 0; i < size_bytes * 2; i++, dest += 4) {
        const uint8_t byte = addr[i / 2];
        const uint8_t part = (byte >> (4 - (i % 2) * 4)) & 0xf;
        const uint8_t intensity = part >> 1;
        const uint8_t alpha = part & 1;
        const uint8_t c = SCALE_3_8(intensity);
        dest[0] = c;
        dest[1] = c;
        dest[2] = c;
        dest[3] = alpha ? 255 : 0;
    }

    const uint32_t safe_line = tile_line_or_fallback(tile, 1, 2);
    const uint32_t width = safe_line * 2;
    const uint32_t height = size_bytes / safe_line;

	gfx_rapi->upload_texture(tex_upload_buffer, width, height, gen_mipmaps);
    // DumpTexture(loaded_texture.otr_path, rgba32_buf, width, height);
}

static void import_texture_ia8(int tile, const LoadedTexture& loaded_texture, bool gen_mipmaps) {
    const RawTexMetadata* metadata = &loaded_texture.raw_tex_metadata;
    const uint8_t* addr = loaded_texture.addr;
    const uint32_t size_bytes = loaded_texture.size_bytes;
    const uint32_t full_image_line_size_bytes =
        loaded_texture.full_image_line_size_bytes;
    const uint32_t line_size_bytes = loaded_texture.line_size_bytes;
    SUPPORT_CHECK(full_image_line_size_bytes == line_size_bytes);

    uint8_t *dest = tex_upload_buffer;
    for (uint32_t i = 0; i < size_bytes; i++, dest += 4) {
        const uint8_t intensity = SCALE_4_8(addr[i] >> 4);
        const uint8_t alpha = SCALE_4_8(addr[i] & 0xf);
        dest[0] = intensity;
        dest[1] = intensity;
        dest[2] = intensity;
        dest[3] = alpha;
    }

    const uint32_t safe_line = tile_line_or_fallback(tile, 1, 1);
    const uint32_t width = safe_line;
    const uint32_t height = size_bytes / safe_line;

	gfx_rapi->upload_texture(tex_upload_buffer, width, height, gen_mipmaps);
    // DumpTexture(loaded_texture.otr_path, rgba32_buf, width, height);
}

static void import_texture_ia16(int tile, const LoadedTexture& loaded_texture, bool gen_mipmaps) {
    const RawTexMetadata* metadata = &loaded_texture.raw_tex_metadata;
    const uint8_t* addr = loaded_texture.addr;
    const uint32_t size_bytes = loaded_texture.size_bytes;
    const uint32_t full_image_line_size_bytes =
        loaded_texture.full_image_line_size_bytes;
    const uint32_t line_size_bytes = loaded_texture.line_size_bytes;
    SUPPORT_CHECK(full_image_line_size_bytes == line_size_bytes);

    uint8_t *dest = tex_upload_buffer;
    for (uint32_t i = 0; i < size_bytes / 2; i++, dest += 4) {
        const uint8_t intensity = addr[2 * i];
        const uint8_t alpha = addr[2 * i + 1];
        dest[0] = intensity;
        dest[1] = intensity;
        dest[2] = intensity;
        dest[3] = alpha;
    }

    const uint32_t safe_line = tile_line_or_fallback(tile, 2, 1);
    const uint32_t width = safe_line / 2;
    const uint32_t height = size_bytes / safe_line;

	gfx_rapi->upload_texture(tex_upload_buffer, width, height, gen_mipmaps);
    // DumpTexture(loaded_texture.otr_path, rgba32_buf, width, height);
}

static void import_texture_i4(int tile, const LoadedTexture& loaded_texture, bool gen_mipmaps) {
    const RawTexMetadata* metadata = &loaded_texture.raw_tex_metadata;
    const uint8_t* addr = loaded_texture.addr;
    const uint32_t size_bytes = loaded_texture.size_bytes;
    const uint32_t full_image_line_size_bytes =
        loaded_texture.full_image_line_size_bytes;
    const uint32_t line_size_bytes = loaded_texture.line_size_bytes;
    SUPPORT_CHECK(full_image_line_size_bytes == line_size_bytes);

    uint8_t *dest = tex_upload_buffer;
    for (uint32_t i = 0; i < size_bytes * 2; i++, dest += 4) {
        const uint8_t byte = addr[i / 2];
        const uint8_t part = (byte >> (4 - (i % 2) * 4)) & 0xf;
        const uint8_t intensity = SCALE_4_8(part);
        dest[0] = intensity;
        dest[1] = intensity;
        dest[2] = intensity;
        dest[3] = intensity;
    }

    const uint32_t safe_line = tile_line_or_fallback(tile, 1, 2);
    const uint32_t width = safe_line * 2;
    const uint32_t height = size_bytes / safe_line;

	gfx_rapi->upload_texture(tex_upload_buffer, width, height, gen_mipmaps);
    // DumpTexture(loaded_texture.otr_path, rgba32_buf, width, height);
}

static void import_texture_i8(int tile, const LoadedTexture& loaded_texture, bool gen_mipmaps) {
    const RawTexMetadata* metadata = &loaded_texture.raw_tex_metadata;
    const uint8_t* addr = loaded_texture.addr;
    const uint32_t size_bytes = loaded_texture.size_bytes;
    uint32_t full_image_line_size_bytes =
        loaded_texture.full_image_line_size_bytes;
    const uint32_t line_size_bytes = loaded_texture.line_size_bytes;
    SUPPORT_CHECK(full_image_line_size_bytes == line_size_bytes);

    uint8_t *dest = tex_upload_buffer;
    for (uint32_t i = 0; i < size_bytes; i++, dest += 4) {
        const uint8_t intensity = addr[i];
        dest[0] = intensity;
        dest[1] = intensity;
        dest[2] = intensity;
        dest[3] = intensity;
    }

    const uint32_t safe_line = tile_line_or_fallback(tile, 1, 1);
    const uint32_t width = safe_line;
    const uint32_t height = size_bytes / safe_line;

	gfx_rapi->upload_texture(tex_upload_buffer, width, height, gen_mipmaps);
    // DumpTexture(loaded_texture.otr_path, rgba32_buf, width, height);
}

static inline void palette_to_rgba32(const uint16_t palentry, uint8_t *rgba32_buf) {
    if (rdp.palette_fmt == G_TT_IA16) {
        const uint8_t intensity = (palentry & 0xff);
        const uint8_t alpha = palentry >> 8;
        rgba32_buf[0] = intensity;
        rgba32_buf[1] = intensity;
        rgba32_buf[2] = intensity;
        rgba32_buf[3] = alpha;
    } else {
        // assume G_TT_RGBA16
        const uint8_t a = palentry & 1;
        const uint8_t r = palentry >> 11;
        const uint8_t g = (palentry >> 6) & 0x1f;
        const uint8_t b = (palentry >> 1) & 0x1f;
        rgba32_buf[0] = SCALE_5_8(r);
        rgba32_buf[1] = SCALE_5_8(g);
        rgba32_buf[2] = SCALE_5_8(b);
        rgba32_buf[3] = a ? 255 : 0;
    }
}

static void import_texture_ci4(int tile, const LoadedTexture& loaded_texture, bool gen_mipmaps) {
	const RawTexMetadata* metadata = &loaded_texture.raw_tex_metadata;
    const uint8_t* addr = loaded_texture.addr;
    const uint32_t size_bytes = loaded_texture.size_bytes;
    const uint32_t full_image_line_size_bytes =
        loaded_texture.full_image_line_size_bytes;
    const uint32_t line_size_bytes = loaded_texture.line_size_bytes;
    const uint32_t pal_idx = rdp.texture_tile[tile].palette; // 0-15
    const uint16_t* palette = (const uint16_t *)(rdp.palette + pal_idx * 16); // 16 pixel entries, 16 bits each
    SUPPORT_CHECK(full_image_line_size_bytes == line_size_bytes);

    for (uint32_t i = 0; i < size_bytes * 2; i++) {
        const uint8_t byte = addr[i / 2];
        const uint8_t idx = (byte >> (4 - (i % 2) * 4)) & 0xf;
        palette_to_rgba32(palette[idx], tex_upload_buffer +4 * i);
    }

    uint32_t result_line_size = rdp.texture_tile[tile].line_size_bytes;
    if (result_line_size == 0) {
        /* BK DLs can leave the render tile's LINE field 0 (load-block style);
         * derive the CI4 row size from the tile's texel width (2 texels/byte)
         * instead. Without this, height = size / 0 faulted on the intro's
         * Nintendo-logo CI4 textures. */
        result_line_size = (rdp.texture_tile[tile].width + 1) / 2;
        if (result_line_size == 0) result_line_size = line_size_bytes ? line_size_bytes : 1;
    }
    if (metadata->h_byte_scale != 1) {
        result_line_size *= metadata->h_byte_scale;
    }

    const uint32_t width = result_line_size * 2;
    const uint32_t height = size_bytes / result_line_size;

	gfx_rapi->upload_texture(tex_upload_buffer, width, height, gen_mipmaps);
}

static void import_texture_ci8(int tile, const LoadedTexture& loaded_texture, bool gen_mipmaps) {
	const RawTexMetadata* metadata = &loaded_texture.raw_tex_metadata;
    const uint8_t* addr = loaded_texture.addr;
    const uint32_t size_bytes = loaded_texture.size_bytes;
    const uint32_t full_image_line_size_bytes =
        loaded_texture.full_image_line_size_bytes;
    const uint32_t line_size_bytes = loaded_texture.line_size_bytes;

    for (uint32_t i = 0, j = 0; i < size_bytes; j += full_image_line_size_bytes - line_size_bytes) {
        for (uint32_t k = 0; k < line_size_bytes; i++, k++, j++) {
            const uint8_t idx = addr[j];
            palette_to_rgba32(rdp.palette[idx], tex_upload_buffer + 4 * i);
        }
    }

    uint32_t result_line_size = rdp.texture_tile[tile].line_size_bytes;
    if (result_line_size == 0) {
        /* Same LINE=0 fallback as CI4 (1 texel/byte for CI8). */
        result_line_size = rdp.texture_tile[tile].width;
        if (result_line_size == 0) result_line_size = line_size_bytes ? line_size_bytes : 1;
    }
    if (metadata->h_byte_scale != 1) {
        result_line_size *= metadata->h_byte_scale;
    }

    const uint32_t width = result_line_size;
    const uint32_t height = size_bytes / result_line_size;

	gfx_rapi->upload_texture(tex_upload_buffer, width, height, gen_mipmaps);
}

#ifdef PLATFORM_3DS
extern "C" void pdFacadeArmUpload(uintptr_t addr); // port/fast3d/gfx_citro3d.cpp (load-time facade bake)
extern "C" void gfx_citro3d_set_render_phase(int p); // GDL phase marker → backend (record-time)
#else
static inline void pdFacadeArmUpload(uintptr_t addr) { (void)addr; }
#endif

static void import_texture(int i, int tile, bool importReplacement) {
    LoadedTexture& loaded_texture = rdp.loaded_texture[rdp.texture_tile[tile].tmem];
    const uint8_t fmt = rdp.texture_tile[tile].fmt;
    const uint8_t siz = rdp.texture_tile[tile].siz;
    const uint32_t tex_flags = loaded_texture.tex_flags;
    const uint8_t palette_index = rdp.texture_tile[tile].palette;

    if ((rdp.tex_lod && tile >= rdp.first_tile_index + rdp.tex_detail) || !loaded_texture.addr) {
        // set up miplevel 0; also acts as a catch-all for when .addr is NULL because my texture loader sucks
        loaded_texture.addr = rdp.texture_to_load.addr;
        loaded_texture.line_size_bytes = rdp.texture_tile[tile].line_size_bytes;
        loaded_texture.full_image_line_size_bytes = rdp.texture_tile[tile].line_size_bytes;
        loaded_texture.full_size_bytes = loaded_texture.full_image_line_size_bytes * rdp.texture_tile[tile].height;
        loaded_texture.size_bytes = loaded_texture.line_size_bytes * rdp.texture_tile[tile].height;
        if (siz == G_IM_SIZ_32b) {
            // HACK: fixup 32-bit LODed texture height
            loaded_texture.size_bytes <<= 1;
            loaded_texture.full_size_bytes <<= 1;
        }
        loaded_texture.orig_size_bytes = loaded_texture.size_bytes;
    }

    const RawTexMetadata* metadata = &loaded_texture.raw_tex_metadata;
    const uint8_t* orig_addr = loaded_texture.addr;
    SUPPORT_CHECK(orig_addr);

    TextureCacheKey key;
    if (fmt == G_IM_FMT_CI) {
        key = { orig_addr, { rdp.palette_addrs[0], rdp.palette_addrs[1] }, fmt, siz, palette_index };
    } else {
        key = { orig_addr, {}, fmt, siz, palette_index };
    }

#if defined(PLATFORM_PORT) && !defined(PLATFORM_3DS)
    const uint32_t content_sig = tex_content_sig(orig_addr, loaded_texture.size_bytes);
#else
    const uint32_t content_sig = 0;
#endif
    if (gfx_texture_cache_lookup(i, key, content_sig)) {
        return;
    }

#ifdef PLATFORM_3DS
    // Stage B: arm the Citro3D backend if the level-loader scan flagged this texture (by its
    // source address — same pointer gDPSetTextureImage emitted) as a tiled facade, so the
    // upcoming upload bakes it deterministically. No-op for non-facades.
    pdFacadeArmUpload((uintptr_t)orig_addr);
#endif

    if (fmt == G_IM_FMT_RGBA) {
        if (siz == G_IM_SIZ_16b) {
            import_texture_rgba16(tile, loaded_texture, rdp.tex_lod);
        } else if (siz == G_IM_SIZ_32b) {
            import_texture_rgba32(tile, loaded_texture, rdp.tex_lod);
        } else {
            sysFatalError("Bad size for RGBA texture in tile %d: %02x", tile, siz);
        }
    } else if (fmt == G_IM_FMT_IA) {
        if (siz == G_IM_SIZ_4b) {
            import_texture_ia4(tile, loaded_texture, rdp.tex_lod);
        } else if (siz == G_IM_SIZ_8b) {
            import_texture_ia8(tile, loaded_texture, rdp.tex_lod);
        } else if (siz == G_IM_SIZ_16b) {
            import_texture_ia16(tile, loaded_texture, rdp.tex_lod);
        } else {
            sysFatalError("Bad size for IA texture in tile %d: %02x", tile, siz);
        }
    } else if (fmt == G_IM_FMT_CI) {
        if (siz == G_IM_SIZ_4b) {
            import_texture_ci4(tile, loaded_texture, rdp.tex_lod);
        } else if (siz == G_IM_SIZ_8b) {
            import_texture_ci8(tile, loaded_texture, rdp.tex_lod);
        } else {
            sysFatalError("Bad size for CI texture in tile %d: %02x", tile, siz);
        }
    } else if (fmt == G_IM_FMT_I) {
        if (siz == G_IM_SIZ_4b) {
            import_texture_i4(tile, loaded_texture, rdp.tex_lod);
        } else if (siz == G_IM_SIZ_8b) {
            import_texture_i8(tile, loaded_texture, rdp.tex_lod);
        } else {
            sysFatalError("Bad size for I texture in tile %d: %02x", tile, siz);
        }
    } else {
        sysFatalError("Bad texture format in tile %d: %02x %02x", tile, fmt, siz);
    }
}

static void gfx_normalize_vector(float v[3]) {
    float s = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] /= s;
    v[1] /= s;
    v[2] /= s;
}

static void gfx_transposed_matrix_mul(float res[3], const float a[3], const float b[4][4]) {
    res[0] = a[0] * b[0][0] + a[1] * b[0][1] + a[2] * b[0][2];
    res[1] = a[0] * b[1][0] + a[1] * b[1][1] + a[2] * b[1][2];
    res[2] = a[0] * b[2][0] + a[1] * b[2][1] + a[2] * b[2][2];
}

static void calculate_normal_dir(const Light_t* light, float coeffs[3]) {
    const float light_dir[3] = { light->dir[0] / 127.f, light->dir[1] / 127.f, light->dir[2] / 127.f };

    gfx_transposed_matrix_mul(coeffs, light_dir, rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 1]);
    gfx_normalize_vector(coeffs);
}

static void calculate_normal_dir(const struct NormalColor *vcn, float coeffs[3]) {
    const float light_dir[3] = { vcn->x / 127.f, vcn->y / 127.f, vcn->z / 127.f };

    gfx_transposed_matrix_mul(coeffs, light_dir, rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 1]);
    gfx_normalize_vector(coeffs);
}

static void gfx_matrix_mul(float res[4][4], const float a[4][4], const float b[4][4]) {
    float tmp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            tmp[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];
        }
    }
    memcpy(res, tmp, sizeof(tmp));
}

static void gfx_sp_matrix(uint8_t parameters, const int32_t* addr) {
    float matrix[4][4];

#ifndef GBI_FLOATS
    // Original GBI where fixed point matrices are used
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j += 2) {
            int32_t int_part = addr[i * 2 + j / 2];
            uint32_t frac_part = addr[8 + i * 2 + j / 2];
            matrix[i][j] = (int32_t)((int_part & 0xffff0000) | (frac_part >> 16)) / 65536.0f;
            matrix[i][j + 1] = (int32_t)((int_part << 16) | (frac_part & 0xffff)) / 65536.0f;
        }
    }
#else
    // For a modified GBI where fixed point values are replaced with floats
    memcpy(matrix, addr, sizeof(matrix));
#endif

#ifdef PLATFORM_PORT
    /* Camera canary: the game emits its view/projection at frame start. If EITHER is NaN/huge the
     * whole 3D view is corrupt ("camera completely fucked up"). Catch it the instant it loads, with
     * a frame number, so an interactive repro pinpoints WHEN the camera goes bad (vs a node matrix). */
    { int _i,_j,_bad=0; for(_i=0;_i<4;_i++)for(_j=0;_j<4;_j++){ float _v=matrix[_i][_j];
        if(_v!=_v || _v>1e8f || _v<-1e8f) _bad=1; }
      if(_bad){ static int s_on=-1; if(s_on<0) s_on=getenv("TUROK_VTXBAD")?1:0;
        if(s_on){ extern uint32_t num_dls; static int _c=0; if(_c++<20)
          fprintf(stderr,"[CAMBAD] %s%s matrix corrupt (dl#%u): m00=%.2f m11=%.2f m22=%.2f m33=%.2f t=(%.1f,%.1f,%.1f)\n",
            (parameters&G_MTX_PROJECTION)?"PROJ":"MODELVIEW",(parameters&G_MTX_LOAD)?" LOAD":" MUL", num_dls,
            matrix[0][0],matrix[1][1],matrix[2][2],matrix[3][3],matrix[3][0],matrix[3][1],matrix[3][2]); } } }
#endif
    if (parameters & G_MTX_PROJECTION) {
        if (parameters & G_MTX_LOAD) {
            memcpy(rsp.P_matrix, matrix, sizeof(matrix));
        } else {
            gfx_matrix_mul(rsp.P_matrix, matrix, rsp.P_matrix);
        }
#ifdef PLATFORM_PORT
        /* Widescreen: detect a 2D ORTHO projection vs a 3D PERSPECTIVE one by the PERSPECTIVE COLUMN
         * (P[0][3],P[1][3],P[2][3]) — the part of the matrix that makes w depend on vertex position.
         *
         * ★ FLAP FIX (2026-06-26): the old test used ONLY P[2][3]. But Turok bakes the VIEW rotation into
         * the projection matrix (camera.c: m_mfProjection = mfView * mfPerspective * mfFlipX), so for the
         * combined matrix P[2][3] = -View[2][2] = -cos(yaw)-ish — it SWEEPS THROUGH 0 as the camera turns
         * ~90°. So looking sideways made |P[2][3]|<0.5 → the 3D world was misdetected as 2D and pillarboxed
         * → the view "scrunched in/out" flapping between 5:3 and a narrower aspect depending on look
         * direction (worst on the 3DS). The whole perspective column = the view's negated Z axis, a UNIT
         * vector, so its L1 norm is ≥1 for ANY 3D view rotation but ≈0 for a guOrtho 2D projection (w≡1,
         * column is [0,0,0,1]). That magnitude is rotation-INVARIANT → no flap. (P[2][3] alone, and P[3][3],
         * were both shown unreliable for this combined matrix.) */
        { float persp = fabsf(rsp.P_matrix[0][3]) + fabsf(rsp.P_matrix[1][3]) + fabsf(rsp.P_matrix[2][3]);
          s_proj_is_2d = (persp < 0.5f); }
#endif
    } else { // G_MTX_MODELVIEW
#ifdef PLATFORM_PORT
        if ((parameters & G_MTX_PUSH) && rsp.modelview_matrix_stack_size >= MODELVIEW_STACK_DEPTH) {
            extern int g_mtx_push_dropped; g_mtx_push_dropped++;
        }
#endif
        if ((parameters & G_MTX_PUSH) && rsp.modelview_matrix_stack_size < MODELVIEW_STACK_DEPTH) {
            ++rsp.modelview_matrix_stack_size;
            memcpy(rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 1],
                   rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 2], sizeof(matrix));
#ifdef PLATFORM_PORT
            { extern int g_mtx_max_depth; if ((int)rsp.modelview_matrix_stack_size > g_mtx_max_depth) g_mtx_max_depth = rsp.modelview_matrix_stack_size; }
#endif
        }
        if (parameters & G_MTX_LOAD) {
            memcpy(rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 1], matrix, sizeof(matrix));
        } else {
            gfx_matrix_mul(rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 1], matrix,
                           rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 1]);
        }
        rsp.lights_changed = 1;
    }
    gfx_matrix_mul(rsp.MP_matrix, rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 1], rsp.P_matrix);
#ifdef BK_GFX_TRACE
    { static int _mc = 0; if (_mc++ < 8) {
        const float (*M)[4] = matrix;
        fprintf(stderr, "[MTX] params=%02x %s%s%s  rows: "
            "[%.3f %.3f %.3f %.3f][%.3f %.3f %.3f %.3f][%.3f %.3f %.3f %.3f][%.3f %.3f %.3f %.3f]\n",
            parameters,
            (parameters&G_MTX_PROJECTION)?"PROJ ":"MODELVIEW ",
            (parameters&G_MTX_LOAD)?"LOAD ":"MUL ",
            (parameters&G_MTX_PUSH)?"PUSH":"",
            M[0][0],M[0][1],M[0][2],M[0][3], M[1][0],M[1][1],M[1][2],M[1][3],
            M[2][0],M[2][1],M[2][2],M[2][3], M[3][0],M[3][1],M[3][2],M[3][3]); } }
#endif
}

#ifdef PLATFORM_PORT
int g_mtx_push_dropped = 0;   /* PUSH requested while stack already full (depth 11) — push lost */
int g_mtx_pop_underflow = 0;  /* POP requested at/below the camera base level (size <= 1) */
int g_mtx_max_depth = 0;      /* high-water mark of the modelview stack within a frame */
#endif

static void gfx_sp_pop_matrix(uint32_t count) {
    while (count--) {
#ifdef PLATFORM_PORT
        /* The camera/view matrix lives at base level (size 1). A POP that would drop the stack
         * to 0 pops the camera away, corrupting the modelview for everything drawn afterward in
         * this same gfx_run (later objects AND the HUD). Real N64 DLs are push/pop balanced, but
         * a push dropped at the depth cap (or a stray pop) drifts the stack down. Floor at 1. */
        if (rsp.modelview_matrix_stack_size <= 1) { g_mtx_pop_underflow++; continue; }
#endif
        if (rsp.modelview_matrix_stack_size > 0) {
            --rsp.modelview_matrix_stack_size;
            if (rsp.modelview_matrix_stack_size > 0) {
                gfx_matrix_mul(rsp.MP_matrix, rsp.modelview_matrix_stack[rsp.modelview_matrix_stack_size - 1],
                               rsp.P_matrix);
            }
        }
    }
}

static float gfx_adjust_x_for_aspect_ratio(float x, float w = 1.f) {
    if (fbActive) {
        return x;
    } else {
        return (rsp.aspect_ofs * w + x) * rsp.aspect_scale / gfx_current_dimensions.aspect_ratio;
    }
}

#ifdef PLATFORM_PORT
/* WIDESCREEN 2D pillarbox: scale a clip-space X (NDC) into the centered 4:3 region so 2D content (HUD texrects,
 * pause/menu ortho tris, legal art) stays undistorted while the 3D world fills the wide screen. No-op unless
 * widescreen is on AND the output is wider than 4:3. */
static inline float gfx_ws_pillarbox(float x) {
    extern int g_cfg_widescreen; extern float g_turok_aspect;
    if (g_cfg_widescreen && g_turok_aspect > 1.3334f) return x * (4.0f / 3.0f) / g_turok_aspect;
    return x;
}
#endif

static void gfx_adjust_width_height_for_scale(uint32_t& width, uint32_t& height) {
    width = std::round(width * RATIO_Y);
    height = std::round(height * RATIO_Y);
    if (width == 0) {
        width = 1;
    }
    if (height == 0) {
        height = 1;
    }
}

static void gfx_sp_vertex(size_t n_vertices, size_t dest_index, const Vtx* vertices) {
    SUPPORT_CHECK(n_vertices <= MAX_VERTICES);

    for (size_t i = 0; i < n_vertices; i++, dest_index++) {
        const Vtx_t* v = &vertices[i].v; /* stock libultra Vtx_t */
        struct LoadedVertex* d = &rsp.loaded_vertices[dest_index];

        float x = v->ob[0] * rsp.MP_matrix[0][0] + v->ob[1] * rsp.MP_matrix[1][0] + v->ob[2] * rsp.MP_matrix[2][0] + rsp.MP_matrix[3][0];
        float y = v->ob[0] * rsp.MP_matrix[0][1] + v->ob[1] * rsp.MP_matrix[1][1] + v->ob[2] * rsp.MP_matrix[2][1] + rsp.MP_matrix[3][1];
        float z = v->ob[0] * rsp.MP_matrix[0][2] + v->ob[1] * rsp.MP_matrix[1][2] + v->ob[2] * rsp.MP_matrix[2][2] + rsp.MP_matrix[3][2];
        float w = v->ob[0] * rsp.MP_matrix[0][3] + v->ob[1] * rsp.MP_matrix[1][3] + v->ob[2] * rsp.MP_matrix[2][3] + rsp.MP_matrix[3][3];

#ifdef BK_GFX_TRACE
        { static int _tv = 0; if (_tv++ < 12) fprintf(stderr,
            "[TV] ob=(%d,%d,%d) -> xyzw=(%.1f,%.1f,%.1f,%.1f)  MP[0][0]=%.4f MP[3][3]=%.4f\n",
            v->ob[0], v->ob[1], v->ob[2], x, y, z, w, rsp.MP_matrix[0][0], rsp.MP_matrix[3][3]); }
#endif
#ifdef PLATFORM_PORT
        /* Always-on (uncapped) corruption detector: a NaN or wildly-huge transformed vertex means
         * a bad MP_matrix — i.e. the camera/modelview got corrupted before this draw. Reports the
         * frame so a long run catches a late, animation-frame-dependent corruption. */
        { static int s_vb = -1; if (s_vb < 0) s_vb = getenv("TUROK_VTXBAD") ? 1 : 0;
          if (s_vb) { int nan = (x != x) || (w != w);
            int huge = (x>1e8f||x<-1e8f||y>1e8f||y<-1e8f||z>1e8f||z<-1e8f);
            if (nan || huge) { static int _c=0; if(_c++<40) {
              extern int g_turok_cur_obj_type, g_turok_cur_node;
              fprintf(stderr, "[VTXBAD] obj=0x%x node=%d  clip=(%.1f,%.1f,%.1f,w=%.2f)%s%s  MP00=%.3f MP33=%.3f\n",
                g_turok_cur_obj_type, g_turok_cur_node, x, y, z, w, nan?" NAN":"", huge?" HUGE":"",
                rsp.MP_matrix[0][0], rsp.MP_matrix[3][3]); } } } }
#endif
        x = gfx_adjust_x_for_aspect_ratio(x, w);
#ifdef PLATFORM_PORT
        /* Widescreen: pillarbox 2D ORTHO geometry (pause box/bar, menu) so it matches the 2D texrects; leave
         * 3D perspective geometry alone (the game already projects it wide via camera.c). */
        if (s_proj_is_2d) x = gfx_ws_pillarbox(x);
#endif

        short U = v->tc[0] * rsp.texture_scaling_factor.s >> 16;
        short V = v->tc[1] * rsp.texture_scaling_factor.t >> 16;

        const struct NormalColor *vcn = (const struct NormalColor *)v->cn; /* cn[4] aliases NormalColor: color (Vtx_t) or normal (Vtx_tn) */

        if (rsp.geometry_mode & G_LIGHTING) {
            if (rsp.lights_changed) {
                for (int i = 0; i < rsp.current_num_lights - 1; i++) {
                    calculate_normal_dir(&rsp.current_lights[i], rsp.current_lights_coeffs[i]);
                }
                if (rsp.lookat_enabled) {
                    calculate_normal_dir(&rsp.lookat[0], rsp.current_lookat_coeffs[0]);
                    calculate_normal_dir(&rsp.lookat[1], rsp.current_lookat_coeffs[1]);
                }
                rsp.lights_changed = false;
            }

            int r = rsp.current_lights[rsp.current_num_lights - 1].col[0];
            int g = rsp.current_lights[rsp.current_num_lights - 1].col[1];
            int b = rsp.current_lights[rsp.current_num_lights - 1].col[2];

            for (int i = 0; i < rsp.current_num_lights - 1; i++) {
                float intensity = 0;
                intensity += vcn->x * rsp.current_lights_coeffs[i][0];
                intensity += vcn->y * rsp.current_lights_coeffs[i][1];
                intensity += vcn->z * rsp.current_lights_coeffs[i][2];
                intensity /= 127.0f;
                if (intensity > 0.0f) {
                    r += intensity * rsp.current_lights[i].col[0];
                    g += intensity * rsp.current_lights[i].col[1];
                    b += intensity * rsp.current_lights[i].col[2];
                }
            }

            d->color.r = r > 255 ? 255 : r;
            d->color.g = g > 255 ? 255 : g;
            d->color.b = b > 255 ? 255 : b;

            if (rsp.geometry_mode & G_TEXTURE_GEN) {
                float dotx = 0, doty = 0;
                if (rsp.lookat_enabled) {
                    dotx += vcn->x * rsp.current_lookat_coeffs[0][0];
                    dotx += vcn->y * rsp.current_lookat_coeffs[0][1];
                    dotx += vcn->z * rsp.current_lookat_coeffs[0][2];
                    doty += vcn->x * rsp.current_lookat_coeffs[1][0];
                    doty += vcn->y * rsp.current_lookat_coeffs[1][1];
                    doty += vcn->z * rsp.current_lookat_coeffs[1][2];
                    dotx /= 127.0f;
                    doty /= 127.0f;
                } else {
                    float tvcn[3];
                    calculate_normal_dir(vcn, tvcn);
                    dotx = tvcn[0];
                    doty = tvcn[1];
                }

                dotx = clampf(dotx, -1.0f, 1.0f);
                doty = clampf(doty, -1.0f, 1.0f);

                if (rsp.geometry_mode & G_TEXTURE_GEN_LINEAR) {
                    // Not sure exactly what formula we should use to get accurate values
                    /*dotx = (2.906921f * dotx * dotx + 1.36114f) * dotx;
                    doty = (2.906921f * doty * doty + 1.36114f) * doty;
                    dotx = (dotx + 1.0f) / 4.0f;
                    doty = (doty + 1.0f) / 4.0f;*/
                    dotx = acosf(-dotx) /* M_PI */ / 4.0f;
                    doty = acosf(-doty) /* M_PI */ / 4.0f;
                } else {
                    dotx = (dotx + 1.0f) / 4.0f;
                    doty = (doty + 1.0f) / 4.0f;
                }

                U = (int32_t)(dotx * rsp.texture_scaling_factor.s);
                V = (int32_t)(doty * rsp.texture_scaling_factor.t);
            }
        } else {
            d->color.r = vcn->r;
            d->color.g = vcn->g;
            d->color.b = vcn->b;
        }

        d->u = U;
        d->v = V;

        // trivial clip rejection
        d->clip_rej = 0;
        if (x < -w) {
            d->clip_rej |= 1; // CLIP_LEFT
        }
        if (x > w) {
            d->clip_rej |= 2; // CLIP_RIGHT
        }
        if (y < -w) {
            d->clip_rej |= 4; // CLIP_BOTTOM
        }
        if (y > w) {
            d->clip_rej |= 8; // CLIP_TOP
        }
        // if (z < -w) d->clip_rej |= 16; // CLIP_NEAR
        if (z > w) {
            d->clip_rej |= 32; // CLIP_FAR
        }

        d->x = x;
        d->y = y;
        d->z = z;
        d->w = w;

        if (rsp.geometry_mode & G_FOG) {
            if (fabsf(w) < 0.001f) {
                // To avoid division by zero
                w = 0.001f;
            }

            float winv = 1.0f / w;
            if (winv < 0.0f) {
                winv = std::numeric_limits<int16_t>::max();
            }

            float fog_z = z * winv * rsp.fog_mul + rsp.fog_offset;
            d->fog = clampf(fog_z, 0.f, 255.f);
        } else {
            d->fog = rdp.fog_color.a;
        }

        d->color.a = vcn->a; // can be required for SHADE_ALPHA even if fog is enabled
    }
}

static void gfx_sp_modify_vertex(uint16_t vtx_idx, uint8_t where, uint32_t val) {
    SUPPORT_CHECK(where == G_MWO_POINT_ST);

    int16_t s = (int16_t)(val >> 16);
    int16_t t = (int16_t)val;

    struct LoadedVertex* v = &rsp.loaded_vertices[vtx_idx];
    v->u = s;
    v->v = t;
}

static inline int gfx_lod_tile_offset(const int i) {
    if (gfx_detail_textures_enabled)
        return ((rdp.tex_lod && !rdp.tex_detail) ? 0 : i);
    return (rdp.tex_lod ? rdp.tex_detail : i);
}

#ifdef PLATFORM_3DS
// §29 near-plane clipping support: linearly interpolate a LoadedVertex (out = a + t*(b-a))
// for the vertex inserted ON the near plane when a triangle straddles it. Every rasterized
// attribute is interpolated so the clipped piece is correct (pos/uv/color/fog). The new
// vertex lands exactly on the near plane (z=-w), so it is "in front" and never re-clipped
// (also guarded by s_in_near_clip in gfx_sp_tri1).
static void gfx_lerp_vtx(struct LoadedVertex* out, const struct LoadedVertex* a, const struct LoadedVertex* b, float t) {
    out->x  = a->x  + t * (b->x  - a->x);
    out->y  = a->y  + t * (b->y  - a->y);
    out->z  = a->z  + t * (b->z  - a->z);
    out->w  = a->w  + t * (b->w  - a->w);
    out->u  = a->u  + t * (b->u  - a->u);
    out->v  = a->v  + t * (b->v  - a->v);
    out->color.r = (uint8_t)(a->color.r + t * ((int)b->color.r - (int)a->color.r) + 0.5f);
    out->color.g = (uint8_t)(a->color.g + t * ((int)b->color.g - (int)a->color.g) + 0.5f);
    out->color.b = (uint8_t)(a->color.b + t * ((int)b->color.b - (int)a->color.b) + 0.5f);
    out->color.a = (uint8_t)(a->color.a + t * ((int)b->color.a - (int)a->color.a) + 0.5f);
    out->fog = (uint8_t)(a->fog + t * ((int)b->fog - (int)a->fog) + 0.5f);
    out->clip_rej = 0;
}
#endif

size_t g_turok_tri_calls = 0, g_turok_tri_cliprej = 0;   /* PORT draw-path probe */
static void gfx_sp_tri1(uint8_t vtx1_idx, uint8_t vtx2_idx, uint8_t vtx3_idx, bool is_rect) {
    g_turok_tri_calls++;
    struct LoadedVertex* v1 = &rsp.loaded_vertices[vtx1_idx];
    struct LoadedVertex* v2 = &rsp.loaded_vertices[vtx2_idx];
    struct LoadedVertex* v3 = &rsp.loaded_vertices[vtx3_idx];
    struct LoadedVertex* v_arr[3] = { v1, v2, v3 };

    if ((rsp.extra_geometry_mode & G_NO_CLIPPING_EXT) == 0) {
        if (v1->clip_rej & v2->clip_rej & v3->clip_rej) {
            // The whole triangle lies outside the visible area
            g_turok_tri_cliprej++;
            return;
        }
    }

#ifdef PLATFORM_3DS
    // ★ NEAR-PLANE CLIPPING (§29): the PICA has no GL_DEPTH_CLAMP, so a triangle straddling the
    // near plane (e.g. a wall the camera hugs) is HARD-CLIPPED by the PICA's fixed-function
    // clipper → you see through the wall to the background. GL renders it via per-FRAGMENT
    // depth-clamp; we emulate that at the GEOMETRY stage with Sutherland-Hodgman: clip the tri
    // against the near plane (clip-space z+w>=0) and insert new vertices ON the plane
    // (interpolating pos/uv/color/fog), so the surviving piece rasterizes with correct depth —
    // no hole, and no residual sliver (unlike a per-vertex z-clamp, which only moves a vertex's
    // z and so gives a wrong interpolated gradient). Only near-crossing tris pay the
    // re-polygonize; every other tri takes the "all in front → fall through" fast path. New
    // verts go in spare loaded_vertices slots (free during a non-rect tri); sub-tris are emitted
    // by recursion, with s_in_near_clip guarding re-entry (the inserted verts sit on the plane,
    // so they would not re-clip anyway). Rects build clip-space directly and never near-cross.
    // This is the GL-equivalent fix proven by the PC-GL reference (§16.8).
    static bool s_in_near_clip = false;
    if (!s_in_near_clip && !is_rect) {
        // Guard-band: place the clip plane a hair PAST near (clip z + w*(1+NEAR_GB) >= 0) so a
        // vertex sitting micro-behind near — from f32 rounding, or GL's "render slightly past
        // near" behavior — is KEPT and rendered (then pinned to near depth by the §29 near-clamp
        // in the emit) instead of cut. This is what closes the residual seam GL doesn't have.
        // w-scaled because clip w spans ~1..thousands.
        const float NEAR_GB = 1e-4f;
        const float d1 = v1->z + v1->w * (1.0f + NEAR_GB);
        const float d2 = v2->z + v2->w * (1.0f + NEAR_GB);
        const float d3 = v3->z + v3->w * (1.0f + NEAR_GB);
        // Clip ONLY a STRADDLING tri (some verts past near, some in front). All-in takes the
        // fast path below; an ALL-out tri FALLS THROUGH to the emit (it is NOT discarded) — the
        // §29 near-clamp there pins each vert to the near plane, so between-eye-and-near (w>0)
        // geometry renders AT the near plane exactly like GL_DEPTH_CLAMP, and the PICA W-clips
        // truly-behind-eye (w<0) verts. ★ Discarding all-out tris black-holed geometry the camera
        // flies right up to (the Defection intro flyby goes black where a wall face lands entirely
        // inside the near distance) — GL renders it, so we must too.
        const bool anyOut = (d1 < 0.0f || d2 < 0.0f || d3 < 0.0f);
        const bool anyIn  = (d1 >= 0.0f || d2 >= 0.0f || d3 >= 0.0f);
        if (anyOut && anyIn) {
            const uint8_t orig_idx[3] = { vtx1_idx, vtx2_idx, vtx3_idx };
            struct LoadedVertex* const inv[3] = { v1, v2, v3 };
            const float dd[3] = { d1, d2, d3 };
            uint8_t poly[4];
            int np = 0, sc = 0;
            for (int e = 0; e < 3; e++) {
                const int e2 = (e + 1) % 3;
                const bool ine = dd[e] >= 0.0f, ine2 = dd[e2] >= 0.0f;
                if (ine) {
                    poly[np++] = orig_idx[e];
                }
                if (ine != ine2) {
                    // edge crosses the near plane: insert a vertex at the intersection
                    const float t = dd[e] / (dd[e] - dd[e2]);
                    const uint8_t slot = (uint8_t)(MAX_VERTICES + sc);
                    gfx_lerp_vtx(&rsp.loaded_vertices[slot], inv[e], inv[e2], t);
                    poly[np++] = slot;
                    sc++;
                }
            }
            if (np >= 3) {
                s_in_near_clip = true;
                gfx_sp_tri1(poly[0], poly[1], poly[2], is_rect);
                if (np == 4) {
                    gfx_sp_tri1(poly[0], poly[2], poly[3], is_rect); // fan the clipped quad
                }
                s_in_near_clip = false;
            }
            return;
        }
    }
#endif

    if ((rsp.geometry_mode & G_CULL_BOTH) != 0) {
        float dx1 = v1->x / (v1->w) - v2->x / (v2->w);
        float dy1 = v1->y / (v1->w) - v2->y / (v2->w);
        float dx2 = v3->x / (v3->w) - v2->x / (v2->w);
        float dy2 = v3->y / (v3->w) - v2->y / (v2->w);
        float cross = dx1 * dy2 - dy1 * dx2;

        if ((v1->w < 0) ^ (v2->w < 0) ^ (v3->w < 0)) {
            // If one vertex lies behind the eye, negating cross will give the correct result.
            // If all vertices lie behind the eye, the triangle will be rejected anyway.
            cross = -cross;
        }

        // If inverted culling is requested, negate the cross
        // if ((rsp.extra_geometry_mode & G_EX_INVERT_CULLING) == 1) {
        //     cross = -cross;
        // }

        switch (rsp.geometry_mode & G_CULL_BOTH) {
            case G_CULL_FRONT:
                if (cross <= 0) {
                    return;
                }
                break;
            case G_CULL_BACK:
                if (cross >= 0) {
                    return;
                }
                break;
            case G_CULL_BOTH:
                // Why is this even an option?
                return;
        }
    }

    bool depth_test = ((rsp.geometry_mode & G_ZBUFFER) == G_ZBUFFER || (rdp.other_mode_l & G_ZS_PRIM) == G_ZS_PRIM) &&
                      ((rdp.other_mode_h & G_CYC_1CYCLE) == G_CYC_1CYCLE || (rdp.other_mode_h & G_CYC_2CYCLE) == G_CYC_2CYCLE);
    bool depth_update = (rdp.other_mode_l & Z_UPD) == Z_UPD;
    bool depth_compare = (rdp.other_mode_l & Z_CMP) == Z_CMP;
    bool depth_source_prim = (rdp.other_mode_l & G_ZS_PRIM) == G_ZS_PRIM /* && gDP.primDepth.z == 1.0f */;
    uint16_t zmode = rdp.other_mode_l & ZMODE_DEC;
    uint8_t depth_mode = (depth_test ? 1 : 0) | (depth_update ? 2 : 0) | (depth_compare ? 4 : 0) | (depth_source_prim ? 8 : 0) | (zmode >> 6);

    if (depth_mode != rendering_state.depth_mode) {
        gfx_flush();
        gfx_rapi->set_depth_mode(depth_test, depth_update, depth_compare, depth_source_prim, zmode);
        rendering_state.depth_mode = depth_mode;
    }

    if (rdp.viewport_or_scissor_changed) {
        if (memcmp(&rdp.viewport, &rendering_state.viewport, sizeof(rdp.viewport)) != 0) {
            gfx_flush();
            gfx_rapi->set_viewport(rdp.viewport.x, rdp.viewport.y, rdp.viewport.width, rdp.viewport.height);
            rendering_state.viewport = rdp.viewport;
        }
        if (memcmp(&rdp.scissor, &rendering_state.scissor, sizeof(rdp.scissor)) != 0) {
            gfx_flush();
            gfx_rapi->set_scissor(rdp.scissor.x, rdp.scissor.y, rdp.scissor.width, rdp.scissor.height);
            rendering_state.scissor = rdp.scissor;
        }
        rdp.viewport_or_scissor_changed = false;
    }

    uint64_t cc_options = 0;
    bool use_alpha =
        (rdp.other_mode_l & (3 << 20)) == (G_BL_CLR_MEM << 20) && (rdp.other_mode_l & (3 << 16)) == (G_BL_1MA << 16);
    const bool use_fog = ((rdp.other_mode_l >> 30) == G_BL_CLR_FOG) || ((rdp.other_mode_l >> 26) == G_BL_A_FOG);
    const bool texture_edge = (rdp.other_mode_l & CVG_X_ALPHA) == CVG_X_ALPHA;
    const bool use_noise = (rdp.other_mode_l & (3U << G_MDSFT_ALPHACOMPARE)) == G_AC_DITHER;
    const bool use_2cyc = (rdp.other_mode_h & (3U << G_MDSFT_CYCLETYPE)) == G_CYC_2CYCLE;
    const bool alpha_threshold = (rdp.other_mode_l & (3U << G_MDSFT_ALPHACOMPARE)) == G_AC_THRESHOLD;
    const bool invisible = (rdp.other_mode_l & (3 << 24)) == (G_BL_0 << 24) && (rdp.other_mode_l & (3 << 20)) == (G_BL_CLR_MEM << 20);
    const bool use_grayscale = rdp.grayscale;
    const bool use_modulate = use_alpha && (rsp.extra_geometry_mode & G_MODULATE_EXT) != 0;
    const bool use_blur = (rdp.other_mode_h & (3U << G_MDSFT_TEXTFILT)) == G_TF_BLUR_EXT;

    if (texture_edge) {
        use_alpha = true;
    }

    if (use_alpha) {
        cc_options |= (uint64_t)SHADER_OPT_ALPHA;
    }
    if (use_fog) {
        cc_options |= (uint64_t)SHADER_OPT_FOG;
    }
    if (texture_edge) {
        cc_options |= (uint64_t)SHADER_OPT_TEXTURE_EDGE;
    }
    if (use_noise) {
        cc_options |= (uint64_t)SHADER_OPT_NOISE;
    }
    if (use_2cyc) {
        cc_options |= (uint64_t)SHADER_OPT_2CYC;
    }
    if (alpha_threshold) {
        cc_options |= (uint64_t)SHADER_OPT_ALPHA_THRESHOLD;
    }
    if (invisible) {
        cc_options |= (uint64_t)SHADER_OPT_INVISIBLE;
    }
    if (use_grayscale) {
        cc_options |= (uint64_t)SHADER_OPT_GRAYSCALE;
    }
    if (use_blur) {
        cc_options |= (uint64_t)SHADER_OPT_BLUR;
    }

    // If we are not using alpha, clear the alpha components of the combiner as they have no effect
    if (!use_alpha) {
        cc_options &= ~((0xfff << 16) | ((uint64_t)0xfff << 44));
    }

    ColorCombinerKey key;
    key.combine_mode = rdp.combine_mode;
    key.options = cc_options;

    ColorCombiner* comb = gfx_lookup_or_create_color_combiner(key);

    uint32_t tm = 0;
    uint32_t tex_width[2], tex_height[2], tex_width2[2], tex_height2[2];

    for (int i = 0; i < 2; i++) {
        // TODO: fix this; for now just ignore smaller mips
        const uint32_t tile = rdp.first_tile_index + gfx_lod_tile_offset(i);
        if (comb->used_textures[i]) {
            if (rdp.textures_changed[i]) {
                gfx_flush();
                import_texture(i, tile, false);
                rdp.textures_changed[i] = false;
            }

            uint8_t cms = rdp.texture_tile[tile].cms;
            uint8_t cmt = rdp.texture_tile[tile].cmt;

            uint32_t tex_size_bytes = rdp.loaded_texture[rdp.texture_tile[tile].tmem].orig_size_bytes;
            uint32_t line_size = rdp.texture_tile[tile].line_size_bytes;

            if (line_size == 0) {
                line_size = 1;
            }

            tex_height[i] = tex_size_bytes / line_size;
            switch (rdp.texture_tile[tile].siz) {
                case G_IM_SIZ_4b:
                    line_size <<= 1;
                    break;
                case G_IM_SIZ_8b:
                    break;
                case G_IM_SIZ_16b:
                    line_size /= G_IM_SIZ_16b_LINE_BYTES;
                    break;
                case G_IM_SIZ_32b:
                    line_size /= G_IM_SIZ_32b_LINE_BYTES; // this is 2!
                    tex_height[i] /= 2;
                    break;
            }
            tex_width[i] = line_size;

            tex_width2[i] = (rdp.texture_tile[tile].lrs - rdp.texture_tile[tile].uls + 4) / 4;
            tex_height2[i] = (rdp.texture_tile[tile].lrt - rdp.texture_tile[tile].ult + 4) / 4;

            uint32_t tex_width1 = tex_width[i] << (cms & G_TX_MIRROR);
            uint32_t tex_height1 = tex_height[i] << (cmt & G_TX_MIRROR);

            if ((cms & G_TX_CLAMP) && ((cms & G_TX_MIRROR) || tex_width1 != tex_width2[i])) {
                tm |= 1 << 2 * i;
                cms &= ~G_TX_CLAMP;
            }
            if ((cmt & G_TX_CLAMP) && ((cmt & G_TX_MIRROR) || tex_height1 != tex_height2[i])) {
                tm |= 1 << (2 * i + 1);
                cmt &= ~G_TX_CLAMP;
            }

            if (rendering_state.textures[i]) {
                bool linear_filter = (rdp.other_mode_h & (3U << G_MDSFT_TEXTFILT)) != G_TF_POINT;
                if (linear_filter != rendering_state.textures[i]->second.linear_filter ||
                    cms != rendering_state.textures[i]->second.cms || cmt != rendering_state.textures[i]->second.cmt) {
                    gfx_flush();
                    gfx_rapi->set_sampler_parameters(i, linear_filter, cms, cmt, rdp.tex_lod);
                    rendering_state.textures[i]->second.linear_filter = linear_filter;
                    rendering_state.textures[i]->second.cms = cms;
                    rendering_state.textures[i]->second.cmt = cmt;
                }
            }
        }
    }

    struct ShaderProgram* prg = comb->prg[tm];
    if (prg == NULL) {
        comb->prg[tm] = prg =
            gfx_lookup_or_create_shader_program(comb->shader_id0, comb->shader_id1 | (tm * SHADER_OPT_TEXEL0_CLAMP_S));
    }
    if (prg != rendering_state.shader_program) {
        gfx_flush();
        gfx_rapi->unload_shader(rendering_state.shader_program);
        gfx_rapi->load_shader(prg);
        rendering_state.shader_program = prg;
    }
    if (use_alpha != rendering_state.alpha_blend || use_modulate != rendering_state.modulate) {
        gfx_flush();
        gfx_rapi->set_use_alpha(use_alpha, use_modulate);
        rendering_state.alpha_blend = use_alpha;
        rendering_state.modulate = use_modulate;
    }
    uint8_t num_inputs;
    bool used_textures[2];

    gfx_rapi->shader_get_info(prg, &num_inputs, used_textures);

    struct GfxClipParameters clip_parameters = gfx_rapi->get_clip_parameters();

    for (int i = 0; i < 3; i++) {
        float z = v_arr[i]->z, w = v_arr[i]->w;
        if (clip_parameters.z_is_from_0_to_1) {
#ifdef PLATFORM_3DS
            // ★ PICA DEPTH PRECISION: emit the FINAL pre-perspective z = (z_orig - w)/2 here on the CPU
            // (f32), so the pass-through vshader (gfx_citro3d buildTransform row2 = 0,0,1,0) does NOT compute
            // inpos.z - inpos.w in the f24 vertex shader. That subtraction CATASTROPHICALLY CANCELS at the
            // far plane (z_orig≈w, both ~thousands) → garbage depth → z-fighting where the background bleeds
            // through geometry edges (the Carrington Villa sky slivers). NDC z = z/w is identical [-1,0];
            // only the precision improves.
            z = (z - w) / 2.0f;
            // §29 near-clamp: pin a near-crossing seam vertex to the near plane (clip z = -w →
            // NDC -1, window 0 = nearest) so an f32-epsilon seam vert that lands a hair behind
            // near is NOT hard-clipped by the PICA — matching GL_DEPTH_CLAMP's per-fragment clamp.
            // CLIP-SPACE clamp (x/y/w untouched → silhouette exact); a no-op for in-frustum
            // geometry, so the §25 far-plane precision fix is fully preserved.
            if (z < -w) { z = -w; }
#else
            z = (z + w) / 2.0f;
#endif
        }

        buf_vbo[buf_vbo_len++] = v_arr[i]->x;
        buf_vbo[buf_vbo_len++] = clip_parameters.invert_y ? -v_arr[i]->y : v_arr[i]->y;
        buf_vbo[buf_vbo_len++] = z;
        buf_vbo[buf_vbo_len++] = w;

        for (int t = 0; t < 2; t++) {
            if (!used_textures[t]) {
                continue;
            }

            // TODO: fix this; for now just ignore smaller mips
            const uint32_t tile = gfx_lod_tile_offset(t);

            float u = v_arr[i]->u / 32.0f;
            float v = v_arr[i]->v / 32.0f;

            int shifts = rdp.texture_tile[rdp.first_tile_index + tile].shifts;
            int shiftt = rdp.texture_tile[rdp.first_tile_index + tile].shiftt;
            if (shifts != 0) {
                if (shifts <= 10) {
                    u /= 1 << shifts;
                } else {
                    u *= 1 << (16 - shifts);
                }
            }
            if (shiftt != 0) {
                if (shiftt <= 10) {
                    v /= 1 << shiftt;
                } else {
                    v *= 1 << (16 - shiftt);
                }
            }

            u -= rdp.texture_tile[rdp.first_tile_index + tile].uls / 4.0f;
            v -= rdp.texture_tile[rdp.first_tile_index + tile].ult / 4.0f;

            if (!is_rect) {
                if (!(rdp.other_mode_h & G_TP_PERSP)) {
                    u *= 0.5f;
                    v *= 0.5f;
                }

                if ((rdp.other_mode_h & (3U << G_MDSFT_TEXTFILT)) != G_TF_POINT) {
                    // Linear filter adds 0.5f to the coordinates
                    u += 0.5f;
                    v += 0.5f;
                }
            }

            buf_vbo[buf_vbo_len++] = u / tex_width[t];
            buf_vbo[buf_vbo_len++] = v / tex_height[t];

            bool clampS = tm & (1 << 2 * t);
            bool clampT = tm & (1 << (2 * t + 1));

            if (clampS) {
                buf_vbo[buf_vbo_len++] = (tex_width2[t] - 0.5f) / tex_width[t];
            }
            if (clampT) {
                buf_vbo[buf_vbo_len++] = (tex_height2[t] - 0.5f) / tex_height[t];
            }
        }

        if (use_fog) {
            buf_vbo[buf_vbo_len++] = rdp.fog_color.r / 255.0f;
            buf_vbo[buf_vbo_len++] = rdp.fog_color.g / 255.0f;
            buf_vbo[buf_vbo_len++] = rdp.fog_color.b / 255.0f;
            buf_vbo[buf_vbo_len++] = v_arr[i]->fog / 255.0f; // fog factor
        }

        if (use_grayscale) {
            buf_vbo[buf_vbo_len++] = rdp.grayscale_color.r / 255.0f;
            buf_vbo[buf_vbo_len++] = rdp.grayscale_color.g / 255.0f;
            buf_vbo[buf_vbo_len++] = rdp.grayscale_color.b / 255.0f;
            buf_vbo[buf_vbo_len++] = rdp.grayscale_color.a / 255.0f; // lerp interpolation factor (not alpha)
        }

        for (int j = 0; j < num_inputs; j++) {
            struct RGBA* color = 0;
            struct RGBA tmp = { 0 };
            for (int k = 0; k < 1 + (use_alpha ? 1 : 0); k++) {
                switch (comb->shader_input_mapping[k][j]) {
                        // Note: CCMUX constants and ACMUX constants used here have same value, which is why this works
                        // (except LOD fraction).
                    case G_CCMUX_PRIMITIVE:
                        color = &rdp.prim_color;
                        break;
                    case G_CCMUX_SHADE:
                        color = &v_arr[i]->color;
                        break;
                    case G_CCMUX_SHADE_ALPHA:
                        tmp.r = tmp.g = tmp.b = v_arr[i]->color.a;
                        color = &tmp;
                        break;
                    case G_CCMUX_ENVIRONMENT:
                        color = &rdp.env_color;
                        break;
                    case G_CCMUX_PRIMITIVE_ALPHA: {
                        tmp.r = tmp.g = tmp.b = rdp.prim_color.a;
                        color = &tmp;
                        break;
                    }
                    case G_CCMUX_ENV_ALPHA: {
                        tmp.r = tmp.g = tmp.b = rdp.env_color.a;
                        color = &tmp;
                        break;
                    }
                    case G_CCMUX_PRIM_LOD_FRAC: {
                        tmp.r = tmp.g = tmp.b = rdp.prim_lod_fraction;
                        color = &tmp;
                        break;
                    }
                    case G_CCMUX_LOD_FRACTION: {
                        if (rdp.other_mode_h & G_TL_LOD) {
                            // HACK: very roughly eyeballed based on the carpets in Defection
                            // this is actually supposed to be calculated per pixel
                            const float distance_frac = std::max(0.f, std::min(w / 1024.f, 1.f));
                            tmp.r = tmp.g = tmp.b = tmp.a = (0.7f + distance_frac * 0.3f) * 255.f;
                        } else {
                            tmp.r = tmp.g = tmp.b = tmp.a = 255;
                        }
                        color = &tmp;
                        break;
                    }
                    case G_ACMUX_PRIM_LOD_FRAC:
                        tmp.a = rdp.prim_lod_fraction;
                        color = &tmp;
                        break;
                    default:
                        memset(&tmp, 0, sizeof(tmp));
                        color = &tmp;
                        break;
                }
                if (k == 0) {
                    buf_vbo[buf_vbo_len++] = color->r / 255.0f;
                    buf_vbo[buf_vbo_len++] = color->g / 255.0f;
                    buf_vbo[buf_vbo_len++] = color->b / 255.0f;
                } else {
                    buf_vbo[buf_vbo_len++] = color->a / 255.0f;
                }
            }
        }
    }

    if (++buf_vbo_num_tris == MAX_BUFFERED) {
        gfx_flush();
    }
}

static inline void gfx_sp_tri4(Gfx *cmd) {
    // the game issues gSPTri2 for quads, which uses G_TRI4 with 2 empty triangles
    uint8_t x = C1(0, 4);
    uint8_t y = C1(4, 4);
    uint8_t z = C0(0, 4);

    if(x || y || z) {
        gfx_sp_tri1(x, y, z, false);
    }

    x = C1(8, 4);
    y = C1(12, 4);
    z = C0(4, 4);

    if (x || y || z) {
        gfx_sp_tri1(x, y, z, false);
    }

    x = C1(16, 4);
    y = C1(20, 4);
    z = C0(8, 4);

    if (x || y || z) {
        gfx_sp_tri1(x, y, z, false);
    }

    x = C1(24, 4);
    y = C1(28, 4);
    z = C0(12, 4);

    if (x || y || z) {
        gfx_sp_tri1(x, y, z, false);
    }
}

/* F3DEX G_LINE3D (0xb5) — Turok draws the in-game MAP with the gspL3DEX line microcode
 * (map.c: region edges + the player arrow, all gSPLine3D i.e. wd=0). The RDP renders a
 * screen-space segment of width (1.5 + wd/2) N64 pixels; emulate it as a quad (two tris)
 * expanded perpendicular to the projected segment, reusing the whole existing tri pipeline
 * (combiner/blend/fog/scissor). The endpoints were already transformed (and widescreen
 * aspect-adjusted) at G_VTX, so the expansion happens in final NDC. Width scales with the
 * window height so the line keeps its N64 proportion at any resolution. */
static void gfx_sp_line3d(uint8_t vtx1_idx, uint8_t vtx2_idx, uint8_t wd) {
    const struct LoadedVertex* e0 = &rsp.loaded_vertices[vtx1_idx];
    const struct LoadedVertex* e1 = &rsp.loaded_vertices[vtx2_idx];
    if (e0->w <= 0.0f || e1->w <= 0.0f) return;   /* behind the eye */
    if (e0->clip_rej & e1->clip_rej) return;      /* both endpoints off the same screen edge */

    const float x0 = e0->x / e0->w, y0 = e0->y / e0->w;   /* NDC endpoints */
    const float x1 = e1->x / e1->w, y1 = e1->y / e1->w;

    /* Perpendicular in screen-pixel space so the width is uniform in any direction. */
    const float hw = gfx_current_dimensions.width * 0.5f;
    const float hh = gfx_current_dimensions.height * 0.5f;
    const float dx = (x1 - x0) * hw, dy = (y1 - y0) * hh;
    const float len = sqrtf(dx * dx + dy * dy);
    if (len < 1e-6f) return;                      /* degenerate segment */
    const float half_px = (1.5f + 0.5f * (float)wd) * 0.5f * (gfx_current_dimensions.height / 240.0f);
    const float nx = (-dy / len) * half_px / hw;  /* back to NDC units */
    const float ny = ( dx / len) * half_px / hh;

    struct LoadedVertex* c = &rsp.loaded_vertices[MAX_VERTICES + 4];
    const struct LoadedVertex* src[4] = { e0, e0, e1, e1 };
    static const float sgn[4] = { 1.0f, -1.0f, 1.0f, -1.0f };
    const float px[2] = { x0, x1 }, py[2] = { y0, y1 };
    for (int i = 0; i < 4; i++) {
        c[i] = *src[i];                            /* carries z, w, u/v, color, fog, clip_rej */
        c[i].x = (px[i >> 1] + sgn[i] * nx) * src[i]->w;
        c[i].y = (py[i >> 1] + sgn[i] * ny) * src[i]->w;
    }

    /* A line has no facing — don't let backface culling drop the quad. */
    const uint32_t saved_gm = rsp.geometry_mode;
    rsp.geometry_mode &= ~(uint32_t)G_CULL_BOTH;
    gfx_sp_tri1(MAX_VERTICES + 4, MAX_VERTICES + 5, MAX_VERTICES + 6, false);
    gfx_sp_tri1(MAX_VERTICES + 5, MAX_VERTICES + 7, MAX_VERTICES + 6, false);
    rsp.geometry_mode = saved_gm;
}

static void gfx_sp_geometry_mode(uint32_t clear, uint32_t set) {
    rsp.geometry_mode &= ~clear;
    rsp.geometry_mode |= set;
}

static inline void gfx_update_aspect_mode(void) {
    const uint32_t side = rsp.aspect_mode & G_ASPECT_CENTER_EXT;

    rsp.aspect_scale = rsp.aspect_mode ? gfx_current_native_aspect : gfx_current_window_dimensions.aspect_ratio;

    if (side == G_ASPECT_LEFT_EXT) {
        rsp.aspect_ofs = 1.f - gfx_current_dimensions.aspect_ratio / gfx_current_native_aspect;
    } else if (side == G_ASPECT_RIGHT_EXT) {
        rsp.aspect_ofs = gfx_current_dimensions.aspect_ratio / gfx_current_native_aspect - 1.f;
    } else {
        rsp.aspect_ofs = 0.f;
    }

    if (side && (rsp.aspect_mode & G_ASPECT_WIDE_EXT)) {
        constexpr float c = 16.f / 9.f;
        if (gfx_current_dimensions.aspect_ratio > c) {
            rsp.aspect_ofs *= c / gfx_current_dimensions.aspect_ratio;
        }
    }
}

static void gfx_sp_extra_geometry_mode(uint32_t clear, uint32_t set) {
    rsp.extra_geometry_mode &= ~clear;
    rsp.extra_geometry_mode |= set;
    rsp.aspect_mode = (rsp.extra_geometry_mode & G_ASPECT_MODE_EXT);
    gfx_update_aspect_mode();
}

static void gfx_adjust_viewport_or_scissor(XYWidthHeight* area, bool preserve_aspect = false) {
    // HACK: assume all target framebuffers have the same aspect
    // Use floor/ceil to ensure scissor fully contains the logical region
    // and prevents sub-pixel gaps at viewport edges
    float x1 = area->x * RATIO_X;
    float y1 = (SCREEN_HEIGHT - area->y) * RATIO_Y;
    float x2 = (area->x + area->width) * RATIO_X;
    float y2 = (SCREEN_HEIGHT - area->y + area->height) * RATIO_Y;
    
    area->x = std::floor(x1);
    area->y = std::floor(y1);
    area->width = std::ceil(x2) - area->x;
    area->height = std::ceil(y2) - area->y;
    
    if (preserve_aspect) {
        // preserve native aspect ratio
        const float ratio = gfx_current_native_aspect / gfx_current_dimensions.aspect_ratio;
        const float midx = gfx_current_dimensions.width * 0.5f;
        area->x = midx + (area->x - midx) * ratio;
        area->x += rsp.aspect_ofs * gfx_current_dimensions.width * 0.5f;
        area->width *= ratio;
    }

    if (!game_renders_to_framebuffer ||
        (gfx_msaa_level > 1 && gfx_current_dimensions.width == gfx_current_game_window_viewport.width &&
            gfx_current_dimensions.height == gfx_current_game_window_viewport.height)) {
        area->x += gfx_current_game_window_viewport.x;
        area->y += gfx_current_window_dimensions.height -
                    (gfx_current_game_window_viewport.y + gfx_current_game_window_viewport.height);
    }
}

static void gfx_calc_and_set_viewport(const Vp_t* viewport) {
    // 2 bits fraction
    float width = 2.0f * viewport->vscale[0] / 4.0f;
    float height = 2.0f * viewport->vscale[1] / 4.0f;
    float x = (viewport->vtrans[0] / 4.0f) - width / 2.0f;
    float y = ((viewport->vtrans[1] / 4.0f) + height / 2.0f);

    rdp.viewport.x = x;
    rdp.viewport.y = y;
    rdp.viewport.width = width;
    rdp.viewport.height = height;

    gfx_adjust_viewport_or_scissor(&rdp.viewport);

    rdp.viewport_or_scissor_changed = true;
}

static void gfx_sp_movemem(uint8_t index, uint8_t offset, const void* data) {
    switch (index) {
        case G_MV_VIEWPORT:
            gfx_calc_and_set_viewport((const Vp_t*)data);
            break;
        case G_MV_LOOKATY:
        case G_MV_LOOKATX:
            // I think this is only really used for guLookAtReflect
            index = !((index - G_MV_LOOKATY) / 2);
            rsp.lookat[index] = ((const Light *)data)->l;
            rsp.lookat_enabled = (index == 0) || (rsp.lookat[1].dir[0] || rsp.lookat[1].dir[1]);
            rsp.lights_changed = true;
            break;
        case G_MV_L0:
        case G_MV_L1:
        case G_MV_L2:
            // NOTE: reads out of bounds if it is an ambient light
            memcpy(rsp.current_lights + (index - G_MV_L0) / 2, data, sizeof(Light_t));
            break;
    }
}

static void gfx_sp_moveword(uint8_t index, uint16_t offset, uintptr_t data) {
    switch (index) {
        case G_MW_NUMLIGHT:
            // Ambient light is included
            // The 31th bit is a flag that lights should be recalculated
            rsp.current_num_lights = (data - 0x80000000U) / 32;
            rsp.lights_changed = 1;
            break;
        case G_MW_FOG:
            rsp.fog_mul = (int16_t)(data >> 16);
            rsp.fog_offset = (int16_t)data;
            break;
        case G_MW_SEGMENT:
#ifdef BK_GFX_TRACE
            fprintf(stderr, "[SEG] segment %u = %p\n", (unsigned)((offset >> 2) & 0xff), (void *)data);
#endif
            segmentPointers[(offset >> 2) & 0xff] = data;
            break;
    }
}

static void gfx_sp_texture(uint16_t sc, uint16_t tc, uint8_t level, uint8_t tile, uint8_t on) {
    rsp.texture_scaling_factor.s = sc;
    rsp.texture_scaling_factor.t = tc;
    rdp.tex_max_lod = level;
    if (rdp.first_tile_index != tile) {
        rdp.textures_changed[0] = true;
        rdp.textures_changed[1] = true;
        rdp.first_tile_index = tile;
    }
}

static void gfx_dp_set_scissor(uint32_t mode, uint32_t ulx, uint32_t uly, uint32_t lrx, uint32_t lry) {
    float x = ulx / 4.0f;
    float y = lry / 4.0f;
    float width = (lrx - ulx) / 4.0f;
    float height = (lry - uly) / 4.0f;

    rdp.scissor.x = x;
    rdp.scissor.y = y;
    rdp.scissor.width = width;
    rdp.scissor.height = height;

    gfx_adjust_viewport_or_scissor(&rdp.scissor, rsp.aspect_mode != 0);

    rdp.viewport_or_scissor_changed = true;
}

static void gfx_dp_set_texture_image(uint32_t format, uint32_t size, uint32_t width, uint32_t tex_flags, const void* addr) {
    rdp.texture_to_load.addr = (const uint8_t*)addr;
    rdp.texture_to_load.siz = size;
    rdp.texture_to_load.width = width;
    rdp.texture_to_load.tex_flags = tex_flags;
}

static void gfx_dp_set_tile(uint8_t fmt, uint32_t siz, uint32_t line, uint32_t tmem, uint8_t tile, uint32_t palette,
                            uint32_t cmt, uint32_t maskt, uint32_t shiftt, uint32_t cms, uint32_t masks,
                            uint32_t shifts) {
    // OTRTODO:
    // SUPPORT_CHECK(tmem == 0 || tmem == 256);
    static uint32_t max_tmem = 0;
    if (cms == G_TX_WRAP && masks == G_TX_NOMASK) {
        cms = G_TX_CLAMP;
    }
    if (cmt == G_TX_WRAP && maskt == G_TX_NOMASK) {
        cmt = G_TX_CLAMP;
    }

    if (fmt == G_IM_FMT_RGBA && siz < G_IM_SIZ_16b) {
        // HACK: sometimes the game will submit G_IM_FMT_RGBA, G_IM_SIZ_8b/4b, intending it to read as CI8/CI4 with RGBA16 palette
        fmt = G_IM_FMT_CI;
    } else if (fmt == G_IM_FMT_IA && siz == G_IM_SIZ_32b) {
        // HACK: ... and sometimes it submits this, apparently intending it to be I8
        fmt = G_IM_FMT_I;
        siz = G_IM_SIZ_8b;
    }

    rdp.texture_tile[tile].palette = palette; // palette should set upper 4 bits of color index in 4b mode
    rdp.texture_tile[tile].fmt = fmt;
    rdp.texture_tile[tile].siz = siz;
    rdp.texture_tile[tile].cms = cms;
    rdp.texture_tile[tile].cmt = cmt;
    rdp.texture_tile[tile].shifts = shifts;
    rdp.texture_tile[tile].shiftt = shiftt;
    rdp.texture_tile[tile].line_size_bytes = line * 8;
    rdp.texture_tile[tile].tmem = tmem;

    rdp.textures_changed[0] = true;
    rdp.textures_changed[1] = true;
}

static void gfx_dp_set_tile_size(uint8_t tile, uint16_t uls, uint16_t ult, uint16_t lrs, uint16_t lrt) {
    rdp.texture_tile[tile].uls = uls;
    rdp.texture_tile[tile].ult = ult;
    rdp.texture_tile[tile].lrs = lrs;
    rdp.texture_tile[tile].lrt = lrt;
    rdp.texture_tile[tile].width = (lrs - uls + 4) / 4;
    rdp.texture_tile[tile].height = (lrt - ult + 4) / 4;
    rdp.textures_changed[0] = true;
    rdp.textures_changed[1] = true;
}

static void gfx_dp_load_tlut(uint8_t tile, uint32_t uls, uint32_t ult, uint32_t lrs, uint32_t lrt) {
    // SUPPORT_CHECK(tile == G_TX_LOADTILE);
    SUPPORT_CHECK(rdp.texture_to_load.siz == G_IM_SIZ_16b);
    SUPPORT_CHECK(rdp.texture_tile[tile].tmem >= 256);

    rdp.texture_tile[tile].uls = uls;
    rdp.texture_tile[tile].ult = ult;
    rdp.texture_tile[tile].lrs = lrs;
    rdp.texture_tile[tile].lrt = lrt;

    const uint32_t width = (lrs - uls + 1);
    const uint32_t height = (lrt - ult + 1);
    const uint32_t pitch = rdp.texture_to_load.width + 1;
    const uint32_t count =  width * height;
    const uint16_t *base = (const uint16_t *)rdp.texture_to_load.addr + pitch * ult + uls;

    if (rdp.texture_tile[tile].tmem == 256) {
        rdp.palette_addrs[0] = (const uint8_t *)base;
        if (count >= 256) {
            rdp.palette_addrs[1] = (const uint8_t *)(base + 128);
        }
    } else {
        rdp.palette_addrs[1] = (const uint8_t *)base;
    }

    const uint32_t palofs = rdp.texture_tile[tile].tmem - 256;
    SUPPORT_CHECK(palofs + count <= 256);

    const uint16_t *src = base;
    uint16_t *dst = rdp.palette + palofs;
    for (uint32_t i = 0; i < count; ++i) {
        *dst++ = PD_BE16(*src++);
    }

    rdp.textures_changed[0] = rdp.textures_changed[1] = true;
}

static void gfx_dp_load_block(uint8_t tile, uint32_t uls, uint32_t ult, uint32_t lrs, uint32_t dxt) {
    // SUPPORT_CHECK(tile == G_TX_LOADTILE);
    SUPPORT_CHECK(uls == 0);
    SUPPORT_CHECK(ult == 0);

    // The lrs field rather seems to be number of pixels to load
    uint32_t orig_size_bytes = (lrs + 1) << rdp.texture_to_load.siz >> 1;
    uint32_t size_bytes = orig_size_bytes;
#ifdef PLATFORM_PORT
    /* PORT: the importers expand into tex_upload_buffer (i4 -> 8*size_bytes is the worst case).
     * A corrupted/garbage texture dimension (e.g. an unswapped big-endian width/height in a
     * model/weapon texture) yields a huge size_bytes that silently overruns the shared buffer
     * and smashes adjacent globals/heap. Clamp it so a bad dimension can never corrupt memory. */
    if (turok_tex_upload_capacity && (uint64_t)size_bytes * 8u > (uint64_t)turok_tex_upload_capacity) {
        static int _c = 0;
        if (_c++ < 16)
            fprintf(stderr, "[F3D] CLAMP load-block size_bytes=%u (lrs=%u siz=%u) -> %u (cap=%u) — bad texture dimension\n",
                    size_bytes, lrs, (unsigned)rdp.texture_to_load.siz, turok_tex_upload_capacity / 8u, turok_tex_upload_capacity);
        size_bytes = orig_size_bytes = turok_tex_upload_capacity / 8u;
    }
#endif
    if (rdp.texture_to_load.raw_tex_metadata.h_byte_scale != 1 ||
        rdp.texture_to_load.raw_tex_metadata.v_pixel_scale != 1) {
        size_bytes *= rdp.texture_to_load.raw_tex_metadata.h_byte_scale;
        size_bytes *= rdp.texture_to_load.raw_tex_metadata.v_pixel_scale;
    }

    LoadedTexture& loaded_texture = rdp.loaded_texture[rdp.texture_tile[tile].tmem];
    loaded_texture.orig_size_bytes = orig_size_bytes;
    loaded_texture.size_bytes = size_bytes;
    loaded_texture.full_size_bytes = size_bytes;
    loaded_texture.line_size_bytes = size_bytes;
    loaded_texture.full_image_line_size_bytes = size_bytes;
    loaded_texture.tex_flags = rdp.texture_to_load.tex_flags;
    loaded_texture.raw_tex_metadata = rdp.texture_to_load.raw_tex_metadata;
    loaded_texture.addr = rdp.texture_to_load.addr;

    rdp.textures_changed[0] = rdp.textures_changed[1] = true;
}

static void gfx_dp_load_tile(uint8_t tile, uint32_t uls, uint32_t ult, uint32_t lrs, uint32_t lrt) {
    SUPPORT_CHECK(tile == G_TX_LOADTILE);

    uint32_t offset_x = uls >> G_TEXTURE_IMAGE_FRAC;
    uint32_t offset_y = ult >> G_TEXTURE_IMAGE_FRAC;
    uint32_t tile_width = ((lrs - uls) >> G_TEXTURE_IMAGE_FRAC) + 1;
    uint32_t tile_height = ((lrt - ult) >> G_TEXTURE_IMAGE_FRAC) + 1;
    uint32_t full_image_width = rdp.texture_to_load.width + 1;

    uint32_t offset_x_in_bytes = offset_x << rdp.texture_to_load.siz >> 1;
    uint32_t tile_line_size_bytes = tile_width << rdp.texture_to_load.siz >> 1;
    uint32_t full_image_line_size_bytes = full_image_width << rdp.texture_to_load.siz >> 1;

    uint32_t orig_size_bytes = tile_line_size_bytes * tile_height;
    uint32_t size_bytes = orig_size_bytes;
    uint32_t start_offset_bytes = full_image_line_size_bytes * offset_y + offset_x_in_bytes;

    float h_byte_scale = rdp.texture_to_load.raw_tex_metadata.h_byte_scale;
    float v_pixel_scale = rdp.texture_to_load.raw_tex_metadata.v_pixel_scale;

    if (h_byte_scale != 1 || v_pixel_scale != 1) {
        start_offset_bytes = h_byte_scale * (v_pixel_scale * offset_y * full_image_line_size_bytes + offset_x_in_bytes);
        size_bytes *= h_byte_scale * v_pixel_scale;
        full_image_line_size_bytes *= h_byte_scale;
        tile_line_size_bytes *= h_byte_scale;
    }

    LoadedTexture& loaded_texture = rdp.loaded_texture[rdp.texture_tile[tile].tmem];
    loaded_texture.orig_size_bytes = orig_size_bytes;
    loaded_texture.size_bytes = size_bytes;
    loaded_texture.full_size_bytes = full_image_line_size_bytes * tile_height;
    loaded_texture.full_image_line_size_bytes = full_image_line_size_bytes;
    loaded_texture.line_size_bytes = tile_line_size_bytes;
    loaded_texture.tex_flags = rdp.texture_to_load.tex_flags;
    loaded_texture.raw_tex_metadata = rdp.texture_to_load.raw_tex_metadata;
    loaded_texture.addr = rdp.texture_to_load.addr + start_offset_bytes;

    rdp.texture_tile[tile].uls = uls;
    rdp.texture_tile[tile].ult = ult;
    rdp.texture_tile[tile].lrs = lrs;
    rdp.texture_tile[tile].lrt = lrt;
    rdp.texture_tile[tile].width = ((lrs - uls) >> G_TEXTURE_IMAGE_FRAC) + 1;
    rdp.texture_tile[tile].height = ((lrt - ult) >> G_TEXTURE_IMAGE_FRAC) + 1;

    rdp.textures_changed[0] = rdp.textures_changed[1] = true;
}

static void gfx_dp_set_combine_mode(uint32_t rgb, uint32_t alpha, uint32_t rgb_cyc2, uint32_t alpha_cyc2) {
    rdp.combine_mode = rgb | (alpha << 16) | ((uint64_t)rgb_cyc2 << 28) | ((uint64_t)alpha_cyc2 << 44);
}

static inline uint32_t color_comb(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    return (a & 0xf) | ((b & 0xf) << 4) | ((c & 0x1f) << 8) | ((d & 7) << 13);
}

static inline uint32_t alpha_comb(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    return (a & 7) | ((b & 7) << 3) | ((c & 7) << 6) | ((d & 7) << 9);
}

static void gfx_dp_set_grayscale_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    rdp.grayscale_color.r = r;
    rdp.grayscale_color.g = g;
    rdp.grayscale_color.b = b;
    rdp.grayscale_color.a = a;
}

static void gfx_dp_set_env_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    rdp.env_color.r = r;
    rdp.env_color.g = g;
    rdp.env_color.b = b;
    rdp.env_color.a = a;
}

static void gfx_dp_set_prim_color(uint8_t m, uint8_t l, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    rdp.prim_lod_fraction = l;
    rdp.prim_color.r = r;
    rdp.prim_color.g = g;
    rdp.prim_color.b = b;
    rdp.prim_color.a = a;
    rdp.fill_color.r = r;
    rdp.fill_color.g = g;
    rdp.fill_color.b = b;
    rdp.fill_color.a = a;
    rdp.tex_min_lod = m;

}

static void gfx_dp_set_fog_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    rdp.fog_color.r = r;
    rdp.fog_color.g = g;
    rdp.fog_color.b = b;
    rdp.fog_color.a = a;
}

static void gfx_dp_set_fill_color(uint32_t packed_color) {
    uint16_t col16 = (uint16_t)packed_color;
    uint32_t r = col16 >> 11;
    uint32_t g = (col16 >> 6) & 0x1f;
    uint32_t b = (col16 >> 1) & 0x1f;
    uint32_t a = col16 & 1;
    rdp.fill_color.r = SCALE_5_8(r);
    rdp.fill_color.g = SCALE_5_8(g);
    rdp.fill_color.b = SCALE_5_8(b);
    rdp.fill_color.a = a * 255;
}

static void gfx_dp_set_subpixel_offset(int16_t x, int16_t y) {
    rdp.subpixel_ofs_x = x;
    rdp.subpixel_ofs_y = y;
}

static void gfx_draw_rectangle(int32_t ulx, int32_t uly, int32_t lrx, int32_t lry) {
    uint32_t saved_other_mode_h = rdp.other_mode_h;
    uint32_t cycle_type = (rdp.other_mode_h & (3U << G_MDSFT_CYCLETYPE));

    if (cycle_type == G_CYC_COPY) {
        rdp.other_mode_h = (rdp.other_mode_h & ~(3U << G_MDSFT_TEXTFILT)) | G_TF_POINT;
    }

    ulx += rdp.subpixel_ofs_x;
    lrx += rdp.subpixel_ofs_x;
    uly += rdp.subpixel_ofs_y;
    lry += rdp.subpixel_ofs_y;

    // U10.2 coordinates
    float ulxf = ulx;
    float ulyf = uly;
    float lrxf = lrx;
    float lryf = lry;

    ulxf = ulxf / (4.0f * HALF_SCREEN_WIDTH) - 1.0f;
    ulyf = -(ulyf / (4.0f * HALF_SCREEN_HEIGHT)) + 1.0f;
    lrxf = lrxf / (4.0f * HALF_SCREEN_WIDTH) - 1.0f;
    lryf = -(lryf / (4.0f * HALF_SCREEN_HEIGHT)) + 1.0f;

    ulxf = gfx_adjust_x_for_aspect_ratio(ulxf);
    lrxf = gfx_adjust_x_for_aspect_ratio(lrxf);

#ifdef PLATFORM_PORT
    /* WIDESCREEN HUD pillarbox: texrects are 2D screen-space (the HUD C16BitGraphics, the options menu, the
     * legal/intro art) — keep them at native 4:3, centered, so they aren't stretched. */
    ulxf = gfx_ws_pillarbox(ulxf);
    lrxf = gfx_ws_pillarbox(lrxf);
#endif

    struct LoadedVertex* ul = &rsp.loaded_vertices[MAX_VERTICES + 0];
    struct LoadedVertex* ll = &rsp.loaded_vertices[MAX_VERTICES + 1];
    struct LoadedVertex* lr = &rsp.loaded_vertices[MAX_VERTICES + 2];
    struct LoadedVertex* ur = &rsp.loaded_vertices[MAX_VERTICES + 3];

    ul->x = ulxf;
    ul->y = ulyf;
    ul->z = -1.0f;
    ul->w = 1.0f;

    ll->x = ulxf;
    ll->y = lryf;
    ll->z = -1.0f;
    ll->w = 1.0f;

    lr->x = lrxf;
    lr->y = lryf;
    lr->z = -1.0f;
    lr->w = 1.0f;

    ur->x = lrxf;
    ur->y = ulyf;
    ur->z = -1.0f;
    ur->w = 1.0f;

    // The coordinates for texture rectangle shall bypass the viewport setting
    struct XYWidthHeight default_viewport = { 0, (int16_t)SCREEN_HEIGHT, (uint32_t)SCREEN_WIDTH, (uint32_t)SCREEN_HEIGHT };
    struct XYWidthHeight viewport_saved = rdp.viewport;
    uint32_t geometry_mode_saved = rsp.geometry_mode;

    gfx_adjust_viewport_or_scissor(&default_viewport);

    rdp.viewport = default_viewport;
    rdp.viewport_or_scissor_changed = true;
    rsp.geometry_mode = 0;

    gfx_sp_tri1(MAX_VERTICES + 0, MAX_VERTICES + 1, MAX_VERTICES + 3, true);
    gfx_sp_tri1(MAX_VERTICES + 1, MAX_VERTICES + 2, MAX_VERTICES + 3, true);

    rsp.geometry_mode = geometry_mode_saved;
    rdp.viewport = viewport_saved;
    rdp.viewport_or_scissor_changed = true;

    if (cycle_type == G_CYC_COPY) {
        rdp.other_mode_h = saved_other_mode_h;
    }
}

static void gfx_dp_texture_rectangle(int32_t ulx, int32_t uly, int32_t lrx, int32_t lry, uint8_t tile, int16_t uls,
                                     int16_t ult, int16_t dsdx, int16_t dtdy, bool flip) {
    uint64_t saved_combine_mode = rdp.combine_mode;
    if ((rdp.other_mode_h & (3U << G_MDSFT_CYCLETYPE)) == G_CYC_COPY) {
        // Per RDP Command Summary Set Tile's shift s and this dsdx should be set to 4 texels
        // Divide by 4 to get 1 instead
        dsdx >>= 2;

        // Color combiner is turned off in copy mode
        gfx_dp_set_combine_mode(color_comb(0, 0, 0, G_CCMUX_TEXEL0), alpha_comb(0, 0, 0, G_ACMUX_TEXEL0), 0, 0);

        // Per documentation one extra pixel is added in this modes to each edge
        lrx += 1 << 2;
        lry += 1 << 2;
    }

    // uls and ult are S10.5
    // dsdx and dtdy are S5.10
    // lrx, lry, ulx, uly are U10.2
    // lrs, lrt are S10.5

    const int16_t width = flip ? lry - uly : lrx - ulx;
    const int16_t height = flip ? lrx - ulx : lry - uly;
    const float lrs = ((uls << 7) + dsdx * width) >> 7;
    const float lrt = ((ult << 7) + dtdy * height) >> 7;

    struct LoadedVertex* ul = &rsp.loaded_vertices[MAX_VERTICES + 0];
    struct LoadedVertex* ll = &rsp.loaded_vertices[MAX_VERTICES + 1];
    struct LoadedVertex* lr = &rsp.loaded_vertices[MAX_VERTICES + 2];
    struct LoadedVertex* ur = &rsp.loaded_vertices[MAX_VERTICES + 3];
    ul->u = uls;
    ul->v = ult;
    lr->u = lrs;
    lr->v = lrt;
    if (!flip) {
        ll->u = uls;
        ll->v = lrt;
        ur->u = lrs;
        ur->v = ult;
    } else {
        ll->u = lrs;
        ll->v = ult;
        ur->u = uls;
        ur->v = lrt;
    }

    uint8_t saved_tile = rdp.first_tile_index;
    if (saved_tile != tile) {
        rdp.textures_changed[0] = true;
        rdp.textures_changed[1] = true;
    }
    rdp.first_tile_index = tile;

    gfx_draw_rectangle(ulx, uly, lrx, lry);
    if (saved_tile != tile) {
        rdp.textures_changed[0] = true;
        rdp.textures_changed[1] = true;
    }
    rdp.first_tile_index = saved_tile;
    rdp.combine_mode = saved_combine_mode;
}

static void gfx_dp_image_rectangle(int32_t tile, int32_t w, int32_t h,
                                   int32_t ulx, int32_t uly, int16_t uls, int16_t ult,
                                   int32_t lrx, int32_t lry, int16_t lrs, int16_t lrt) {
    uint64_t saved_combine_mode = rdp.combine_mode;

    struct LoadedVertex* ul = &rsp.loaded_vertices[MAX_VERTICES + 0];
    struct LoadedVertex* ll = &rsp.loaded_vertices[MAX_VERTICES + 1];
    struct LoadedVertex* lr = &rsp.loaded_vertices[MAX_VERTICES + 2];
    struct LoadedVertex* ur = &rsp.loaded_vertices[MAX_VERTICES + 3];
    ul->u = uls * 32;
    ul->v = ult * 32;
    lr->u = lrs * 32;
    lr->v = lrt * 32;
    ll->u = uls * 32;
    ll->v = lrt * 32;
    ur->u = lrs * 32;
    ur->v = ult * 32;

    // ensure we have the correct texture size
    rdp.texture_tile[tile].line_size_bytes = w << rdp.texture_tile[tile].siz >> 1;
    rdp.texture_tile[tile].width = w;
    rdp.texture_tile[tile].height = h;
    rdp.texture_tile[tile].cms = 0;
    rdp.texture_tile[tile].cmt = 0;
    rdp.texture_tile[tile].shifts = 0;
    rdp.texture_tile[tile].shiftt = 0;
    auto& loadtex = rdp.loaded_texture[rdp.texture_tile[tile].tmem];
    loadtex.full_image_line_size_bytes = loadtex.line_size_bytes = rdp.texture_tile[tile].line_size_bytes;
    loadtex.size_bytes = loadtex.orig_size_bytes = loadtex.full_size_bytes = loadtex.line_size_bytes * h;

    uint8_t saved_tile = rdp.first_tile_index;
    if (saved_tile != tile) {
        rdp.textures_changed[0] = true;
        rdp.textures_changed[1] = true;
    }
    rdp.first_tile_index = tile;

    gfx_draw_rectangle(ulx, uly, lrx, lry);
    if (saved_tile != tile) {
        rdp.textures_changed[0] = true;
        rdp.textures_changed[1] = true;
    }
    rdp.first_tile_index = saved_tile;

    rdp.combine_mode = saved_combine_mode;
}

static void gfx_dp_fill_rectangle(int32_t ulx, int32_t uly, int32_t lrx, int32_t lry) {
    if (rdp.color_image_address == rdp.z_buf_address) {
#ifdef PLATFORM_PORT
        // ★ WEAPON / HAND CLIPS THROUGH WALLS (PC + 3DS) — route Turok's mid-frame z-buffer clear into the
        // backend's depth reset. Turok draws the first-person weapon on top of the world by CLEARING THE
        // Z-BUFFER right before it (CEngineApp__ClearZBuffer / tengine.c: redirect the color image to the
        // z-buffer and fill-rect it with max-z — that's this very fill). The host used to DROP this (the
        // early return below), assuming the frame-start clear + desktop GL's GL_DEPTH_CLAMP keep the weapon
        // clean. It does NOT: GL_DEPTH_CLAMP only stops near/far-plane clipping, it does not make the weapon
        // draw ON TOP of the world, so on PC the viewmodel z-fights / clips INTO walls it hugs (user-reported)
        // exactly like the PICA (which has no depth-clamp at all). Re-issue it as the backend's depth-only
        // clear: clear_framebuffer(false,true) = glClear(GL_DEPTH_BUFFER_BIT) on GL / a full-screen depth-far
        // quad on citro3d, replaying AFTER the world and BEFORE the gun (the mechanism G_CLEAR_DEPTH_EXT uses
        // for Perfect Dark's viewmodel). The frame-start full clear also lands here — harmless (depth is
        // already cleared). gfx_pc.cpp is port-only, so the native N64 (which honors the fill in hardware) is
        // unaffected.
        gfx_flush();
        gfx_rapi->clear_framebuffer(false, true);
        return;
#endif
        // Don't clear Z buffer here since we already did it with glClear
        return;
    }
    uint32_t mode = (rdp.other_mode_h & (3U << G_MDSFT_CYCLETYPE));

    // Widescreen screen fades / clears / backdrops (PC + 3DS; gfx_pc is port-only, never built for N64, so the
    // native N64 target stays 4:3): a fill that spans the full 4:3 WIDTH must cover the whole widescreen width,
    // not just the central 4:3 — otherwise gfx_draw_rectangle's 2D pillarbox shrinks it and leaves black side
    // bands. Turok's RenderTint fade uses 0,0,320,240 and the frame clear uses 0,0,319,239 (both full width AND
    // height). The case that caused the DEATH-CINEMATIC side voids: the level's full-width backdrop/fog fill is
    // issued via gDPScisFillRectangle, so in cinema mode the letterbox SCISSOR clips it to the band (uly≈48,
    // lry≈190) — full width but NOT full height — which the old "full 4:3 screen" test (uly<=0 && lry>=240)
    // missed, so it got pillarboxed to 4:3 and the sky/backdrop showed black L/R bars. Broaden any full-WIDTH
    // fill's X to the whole screen; broaden Y as well only when it also spans full height (a true fullscreen
    // fade/clear). The scissor still confines the letterbox band, so this only widens, never overdraws the bars.
    if (ulx <= 0 && lrx >= 319 * 4) {
        ulx = -1024;
        lrx = 2048;
        if (uly <= 0 && lry >= 239 * 4) {
            uly = -1024;
            lry = 2048;
        }
    }

    if (mode == G_CYC_COPY || mode == G_CYC_FILL) {
        // Per documentation one extra pixel is added in this modes to each edge
        lrx += 1 << 2;
        lry += 1 << 2;
    }

    for (int i = MAX_VERTICES; i < MAX_VERTICES + 4; i++) {
        struct LoadedVertex* v = &rsp.loaded_vertices[i];
        v->color = rdp.fill_color;
    }

    uint64_t saved_combine_mode = rdp.combine_mode;

    if (mode == G_CYC_FILL) {
        gfx_dp_set_combine_mode(color_comb(0, 0, 0, G_CCMUX_SHADE), alpha_comb(0, 0, 0, G_ACMUX_SHADE), 0, 0);
    }

    gfx_draw_rectangle(ulx, uly, lrx, lry);
    rdp.combine_mode = saved_combine_mode;
}

static void gfx_dp_set_z_image(void* z_buf_address) {
    rdp.z_buf_address = z_buf_address;
}

static void gfx_dp_set_color_image(uint32_t format, uint32_t size, uint32_t width, void* address) {
    rdp.color_image_address = address;
}

static void gfx_sp_set_other_mode(uint32_t shift, uint32_t num_bits, uint64_t mode) {
    uint64_t mask = (((uint64_t)1 << num_bits) - 1) << shift;
    uint64_t om = rdp.other_mode_l | ((uint64_t)rdp.other_mode_h << 32);
    om = (om & ~mask) | mode;
    rdp.other_mode_l = (uint32_t)om;
    rdp.other_mode_h = (uint32_t)(om >> 32);
    rdp.palette_fmt = rdp.other_mode_h & (3U << G_MDSFT_TEXTLUT);
    rdp.tex_lod = (rdp.other_mode_h & G_TL_LOD) != 0;
    rdp.tex_detail = (rdp.other_mode_h & (2U << G_MDSFT_TEXTDETAIL)) == G_TD_DETAIL;
}

static void gfx_sp_set_vertex_colors(uint32_t count, const struct NormalColor *vcn) {
    // common sense dictates that we should copy the colors as the command is supposed to do,
    // but it actually doesn't seem to matter
    // SUPPORT_CHECK(count <= sizeof(rsp.vertex_colors) / sizeof(rsp.vertex_colors[0]));
    // for (uint32_t i = 0; i < count; ++i) {
    //     rsp.vertex_colors[i] = vcn[i];
    // }
    rsp.vertex_colors = vcn;
}

static void gfx_dp_set_other_mode(uint32_t h, uint32_t l) {
    rdp.other_mode_h = h;
    rdp.other_mode_l = l;
}

static inline void *seg_addr(uintptr_t w1) {
#ifdef PLATFORM_PORT
    // Banjo uses STANDARD N64 display-list addressing (not PD's lsb-marked
    // segments). An address word is one of:
    //   * 0                        -> NULL
    //   * >= 0x80000000            -> KSEG0/KSEG1 *virtual* pointer; physical =
    //                                 v & 0x1FFFFFFF. (matrices, framebuffer via
    //                                 OS_*_K0/PHYSICAL, direct osVirtualToPhysical
    //                                 pointers — the high bit set marks "direct".)
    //   * else                     -> SEGMENTED address: segment number in bits
    //                                 24-27, byte offset in bits 0-23, resolved
    //                                 through segmentPointers[] (set by gSPSegment
    //                                 / G_MW_SEGMENT). Vertices/textures/rendermode
    //                                 tables are referenced this way.
    // The port's "physical" space IS the host heap (osVirtualToPhysical is the
    // identity), so a resolved value is directly dereferenceable.
    if (w1 == 0) {
        return nullptr;
    }
    if (w1 >= 0x80000000u) {
        return (void *)(w1 & 0x1fffffffu);
    }
    {
        const uintptr_t seg = (w1 >> 24) & 0x0f;
        const uintptr_t off = w1 & 0x00ffffff;
        if (segmentPointers[seg]) {
            return (void *)(segmentPointers[seg] + off);
        }
    }
    return (void *)w1;
#else
    // all segmented addresses have the least significant bit set
    if (w1 & 1) {
        // seg 0 is reserved and doesn't count here
        const uintptr_t seg = (w1 & 0x0f000000) >> 24;
        if (seg && segmentPointers[seg]) {
            const uintptr_t addr = (w1 & 0x00fffffe);
            return (void *)(segmentPointers[seg] + addr);
        }
    }
    return (void *)w1;
#endif
}

uintptr_t clearMtx;

// TEST-ONLY (3DS bake stress harness): when >0, the backend armed a forced mid-frame
// texture-cache wipe. Decremented per DL command; fires when it hits 0 — AFTER DrawCmds are
// already recorded this frame (the room-transition corruption scenario). 0 in normal play.
int g_gfx_test_force_clear = 0;

static void gfx_run_dl(Gfx* cmd) {
    // puts("dl");
    int dummy = 0;
    char dlName[128];
    const char* fileName;

    Gfx* dListStart = cmd;
    uint64_t ourHash = -1;

#ifdef PLATFORM_PORT
    /* PORT runaway tripwire: the warn-and-skip default case (below) means a display list
     * that runs off its end (no G_ENDDL, or a G_DL branch to a bad address) walks memory
     * forever instead of aborting. Bail after an absurd command count and report where, so
     * the runaway site can be diagnosed instead of hanging. A real frame is well under this. */
    unsigned long _port_cmdcount = 0;
#endif

    for (;;) {
#ifdef PLATFORM_PORT
        if (++_port_cmdcount > 300000UL) {
            fprintf(stderr,
                "[gfx] RUNAWAY DL: start=%p now=%p op=%02x w0=%08x w1=%08x after %lu cmds — bailing\n",
                (void*)dListStart, (void*)cmd, (unsigned)(cmd->words.w0 >> 24),
                cmd->words.w0, cmd->words.w1, _port_cmdcount);
            return;
        }
#endif
#if defined(PD_DEBUG3DS) && PD_DEBUG3DS
        if (g_gfx_test_force_clear > 0 && --g_gfx_test_force_clear == 0) gfx_texture_cache_clear();
#endif
        uint32_t opcode = cmd->words.w0 >> 24;
        // gfx_print_cmd(cmd);
#ifdef BK_GFX_TRACE
        { static int _cc = 0; if (_cc++ < 80) fprintf(stderr,
            "[CMD] @%p op=%02x w0=%08x w1=%08x\n", (void*)cmd, opcode,
            cmd->words.w0, cmd->words.w1); }
#endif
        switch (opcode) {
                // RSP commands:
            case G_NOOP:
#ifdef PLATFORM_3DS
                // Render-phase marker emitted by lv.c (gDPNoOpTag 0xFACADE00|phase): set the phase
                // AT RECORD TIME (in GDL order) so draw_triangles knows sky(1)/bg(2)/props(3)/
                // artifacts(4) for each draw. (Room-GDL texturenum NOOPs are <0x1000 → no collision.)
                if ((cmd->words.w1 >> 8) == 0xFACADEu) {
                    gfx_citro3d_set_render_phase((int)(cmd->words.w1 & 0xff));
                }
#endif
                break;
            case G_MTX: {
                gfx_sp_matrix(C0(16, 8), (const int32_t*)seg_addr(cmd->words.w1));
                break;
            }
            case (uint8_t)G_POPMTX:
                gfx_sp_pop_matrix(1);
                break;
            case G_MOVEMEM:
                gfx_sp_movemem(C0(16, 8), 0, seg_addr(cmd->words.w1));
                break;
            case (uint8_t)G_MOVEWORD:
                gfx_sp_moveword(C0(0, 8), C0(8, 16), cmd->words.w1);
                break;
            case (uint8_t)G_TEXTURE:
                gfx_sp_texture(C1(16, 16), C1(0, 16), C0(11, 3), C0(8, 3), C0(0, 8));
                break;
            case G_VTX:
#if defined(F3DEX_GBI) || defined(F3DLP_GBI)
                /* F3DEX-1.x: w0 = cmd:8 | (v0*2):8 | n:6 | length:10 ; w1 = ptr.
                 * count = bits 10-15, dest = (bits 16-23)/2. */
#ifdef BK_GFX_TRACE
                { static int _vc = 0; if (_vc++ < 48) fprintf(stderr,
                    "[VTX] w0=%08x w1=%08x n=%u dest=%u ptr=%p\n",
                    cmd->words.w0, cmd->words.w1, (unsigned)C0(10,6), (unsigned)(C0(16,8)>>1),
                    seg_addr(cmd->words.w1)); }
#endif
                gfx_sp_vertex(C0(10, 6), C0(16, 8) >> 1, (const Vtx*)seg_addr(cmd->words.w1));
#else
                gfx_sp_vertex(C0(0, 16) / sizeof(Vtx), C0(16, 4), (const Vtx*)seg_addr(cmd->words.w1));
#endif
                break;
            case G_DL:
                if (C0(16, 1) == 0) {
                    // Push return address
                    Gfx* subGFX = (Gfx*)seg_addr(cmd->words.w1);

                    if (subGFX != nullptr) {
                        gfx_run_dl(subGFX);
                    }
                } else {
                    cmd = (Gfx*)seg_addr(cmd->words.w1);
                    --cmd; // increase after break
                }
                break;
            case (uint8_t)G_ENDDL:
                return;
            case (uint8_t)G_SETGEOMETRYMODE:
                gfx_sp_geometry_mode(0, cmd->words.w1);
                break;
            case (uint8_t)G_CLEARGEOMETRYMODE:
                gfx_sp_geometry_mode(cmd->words.w1, 0);
                break;
            case G_EXTRAGEOMETRYMODE_EXT:
                gfx_sp_extra_geometry_mode(~C0(0, 24), cmd->words.w1);
                break;
            case (uint8_t)G_TRI1:
#if defined(F3DEX_GBI) || defined(F3DLP_GBI)
                /* F3DEX-1.x: w1 = (v0*2):8 | (v1*2):8 | (v2*2):8 — indices are v*2. */
                gfx_sp_tri1(C1(16, 8) >> 1, C1(8, 8) >> 1, C1(0, 8) >> 1, false);
#else
                gfx_sp_tri1(C1(16, 8) / 10, C1(8, 8) / 10, C1(0, 8) / 10, false);
#endif
                break;
#if defined(F3DEX_GBI) || defined(F3DLP_GBI)
            case (uint8_t)G_TRI2:
                /* F3DEX-1.x two-triangle: indices in w0 (tri0) and w1 (tri1),
                 * each v*2. (F3DEX-1.x has no G_QUAD.) */
                gfx_sp_tri1(C0(16, 8) >> 1, C0(8, 8) >> 1, C0(0, 8) >> 1, false);
                gfx_sp_tri1(C1(16, 8) >> 1, C1(8, 8) >> 1, C1(0, 8) >> 1, false);
                break;
            case (uint8_t)G_LINE3D:
                /* F3DEX-1.x line (gspL3DEX): w1 = (v0*2):8 | (v1*2):8 | wd:8.
                 * Turok's in-game MAP is drawn entirely with these (map.c). */
                gfx_sp_line3d(C1(16, 8) >> 1, C1(8, 8) >> 1, (uint8_t)C1(0, 8));
                break;
#endif
            case (uint8_t)G_TRI4:
                gfx_sp_tri4(cmd);
                break;
            case (uint8_t)G_SETOTHERMODE_L:
                gfx_sp_set_other_mode(C0(8, 8), C0(0, 8), cmd->words.w1);
                break;
            case (uint8_t)G_SETOTHERMODE_H:
                gfx_sp_set_other_mode(C0(8, 8) + 32, C0(0, 8), (uint64_t)cmd->words.w1 << 32);
                break;
            case G_COL:
                gfx_sp_set_vertex_colors(C0(0, 16) / 4, (NormalColor *)seg_addr(cmd->words.w1));
                break;

            // RDP Commands:
            case G_SETTIMG: {
                /* G_SETTIMG width is (width-1) in a 12-bit field (gbi.h gSetImage:
                 * _SHIFTL((width)-1, 0, 12)); reading only 10 bits truncated
                 * textures wider than 1024. */
                gfx_dp_set_texture_image(C0(21, 3), C0(19, 2), C0(0, 12), 0, seg_addr(cmd->words.w1));
                break;
            }
            case G_SETTIMG_FB_EXT:
                gfx_flush();
                gfx_rapi->select_texture_fb(cmd->words.w1);
                rdp.textures_changed[0] = false;
                rdp.textures_changed[1] = false;
                break;
            case G_SETGRAYSCALE_EXT:
                rdp.grayscale = cmd->words.w1;
                break;
            case G_LOADBLOCK:
                gfx_dp_load_block(C1(24, 3), C0(12, 12), C0(0, 12), C1(12, 12), C1(0, 12));
                break;
            case G_LOADTILE:
                gfx_dp_load_tile(C1(24, 3), C0(12, 12), C0(0, 12), C1(12, 12), C1(0, 12));
                break;
            case G_SETTILE:
                gfx_dp_set_tile(C0(21, 3), C0(19, 2), C0(9, 9), C0(0, 9), C1(24, 3), C1(20, 4), C1(18, 2), C1(14, 4),
                                C1(10, 4), C1(8, 2), C1(4, 4), C1(0, 4));
                break;
            case G_SETTILESIZE:
                gfx_dp_set_tile_size(C1(24, 3), C0(12, 12), C0(0, 12), C1(12, 12), C1(0, 12));
                break;
            case G_LOADTLUT:
                gfx_dp_load_tlut(C1(24, 3), C0(14, 10), C0(2, 10), C1(14, 10), C1(2, 10));
                break;
            case G_SETENVCOLOR:
                gfx_dp_set_env_color(C1(24, 8), C1(16, 8), C1(8, 8), C1(0, 8));
                break;
            case G_SETPRIMCOLOR:
                gfx_dp_set_prim_color(C0(8, 8), C0(0, 8), C1(24, 8), C1(16, 8), C1(8, 8), C1(0, 8));
                break;
            case G_SETFOGCOLOR:
                gfx_dp_set_fog_color(C1(24, 8), C1(16, 8), C1(8, 8), C1(0, 8));
                break;
            case G_SETFILLCOLOR:
                gfx_dp_set_fill_color(cmd->words.w1);
                break;
            case G_SETBLENDCOLOR:
                rdp.blend_color.r = C1(24, 8);
                rdp.blend_color.g = C1(16, 8);
                rdp.blend_color.b = C1(8, 8);
                rdp.blend_color.a = C1(0, 8);
                break;
            case G_SETINTENSITY_EXT:
                gfx_dp_set_grayscale_color(C1(24, 8), C1(16, 8), C1(8, 8), C1(0, 8));
                break;
            case G_SETCOMBINE:
                gfx_dp_set_combine_mode(color_comb(C0(20, 4), C1(28, 4), C0(15, 5), C1(15, 3)),
                                        alpha_comb(C0(12, 3), C1(12, 3), C0(9, 3), C1(9, 3)),
                                        color_comb(C0(5, 4), C1(24, 4), C0(0, 5), C1(6, 3)),
                                        alpha_comb(C1(21, 3), C1(3, 3), C1(18, 3), C1(0, 3)));
                break;
            // G_SETPRIMCOLOR, G_CCMUX_PRIMITIVE, G_ACMUX_PRIMITIVE, is used by Goddard
            // G_CCMUX_TEXEL1, LOD_FRACTION is used in Bowser room 1
            case G_SETSUBPIXELOFFSET_EXT: {
                gfx_dp_set_subpixel_offset(C0(0, 16), C1(0, 16));
                break;
            }
            case G_TEXRECT:
            case G_TEXRECTFLIP: {
                int32_t lrx, lry, tile, ulx, uly;
                uint32_t uls, ult, dsdx, dtdy;
                lrx = C0(12, 12);
                lry = C0(0, 12);
                tile = C1(24, 3);
                ulx = C1(12, 12);
                uly = C1(0, 12);
                ++cmd;
                uls = C1(16, 16);
                ult = C1(0, 16);
                ++cmd;
                dsdx = C1(16, 16);
                dtdy = C1(0, 16);
                gfx_dp_texture_rectangle(ulx, uly, lrx, lry, tile, uls, ult, dsdx, dtdy, opcode == G_TEXRECTFLIP);
                break;
            }
            case G_FILLRECT:
                gfx_dp_fill_rectangle(C1(12, 12), C1(0, 12), C0(12, 12), C0(0, 12));
                break;
            case G_FILLRECT_WIDE_EXT: {
                int32_t lrx, lry, ulx, uly;
                lrx = (int32_t)(C0(0, 24) << 8) >> 8;
                lry = (int32_t)(C1(0, 24) << 8) >> 8;
                ++cmd;
                ulx = (int32_t)(C0(0, 24) << 8) >> 8;
                uly = (int32_t)(C1(0, 24) << 8) >> 8;
                gfx_dp_fill_rectangle(ulx, uly, lrx, lry);
                break;
            }
            case G_TEXRECT_WIDE_EXT: {
                int32_t lrx, lry, tile, ulx, uly;
                uint32_t uls, ult, dsdx, dtdy;
                bool flip;
                lrx = (int32_t)((C0(0, 24) << 8)) >> 8;
                lry = (int32_t)((C1(0, 24) << 8)) >> 8;
                tile = C1(24, 3);
                flip = C1(27, 1);
                ++cmd;
                ulx = (int32_t)((C0(0, 24) << 8)) >> 8;
                uly = (int32_t)((C1(0, 24) << 8)) >> 8;
                ++cmd;
                uls = C0(16, 16);
                ult = C0(0, 16);
                dsdx = C1(16, 16);
                dtdy = C1(0, 16);
                gfx_dp_texture_rectangle(ulx, uly, lrx, lry, tile, uls, ult, dsdx, dtdy, flip);
                break;
            }
            case G_IMAGERECT_EXT: {
                int16_t tile, iw, ih;
                int16_t x0, y0, s0, t0;
                int16_t x1, y1, s1, t1;
                tile = C0(0, 3);
                iw = C1(16, 16);
                ih = C1(0, 16);
                ++cmd;
                x0 = C0(16, 16);
                y0 = C0(0, 16);
                s0 = C1(16, 16);
                t0 = C1(0, 16);
                ++cmd;
                x1 = C0(16, 16);
                y1 = C0(0, 16);
                s1 = C1(16, 16);
                t1 = C1(0, 16);
                gfx_dp_image_rectangle(tile, iw, ih, x0, y0, s0, t0, x1, y1, s1, t1);
                break;
            }
            case G_SETSCISSOR:
                gfx_dp_set_scissor(C1(24, 2), C0(12, 12), C0(0, 12), C1(12, 12), C1(0, 12));
                break;
            case G_SETZIMG:
                gfx_dp_set_z_image(seg_addr(cmd->words.w1));
                break;
            case G_SETCIMG:
                gfx_dp_set_color_image(C0(21, 3), C0(19, 2), C0(0, 11), seg_addr(cmd->words.w1));
                break;
            case G_SETFB_EXT:
                gfx_flush();
                if (cmd->words.w1) {
                    // don't care about noise here
                    gfx_set_framebuffer(cmd->words.w1, 1.f);
                    fbActive = true;
                } else {
                    gfx_reset_framebuffer();
                    fbActive = false;
                }
                break;
            case G_COPYFB_EXT:
                gfx_copy_framebuffer(C0(11, 11), C0(0, 11), (int16_t)C1(16, 16), (int16_t)C1(0, 16), C0(22, 1));
                break;
            case G_RDPSETOTHERMODE:
                gfx_dp_set_other_mode(C0(0, 24), cmd->words.w1);
                break;
            case G_INVALTEXCACHE_EXT:
                if (cmd->words.w1) {
                    gfx_texture_cache_delete((const uint8_t *)seg_addr(cmd->words.w1));
                } else {
                    gfx_texture_cache_clear();
                }
                break;
            case (uint8_t)G_RDPHALF_1:
            case (uint8_t)G_RDPHALF_2:
            case (uint8_t)G_RDPHALF_CONT:
                // on N64 skyRender uses these to render some types of skies and skybox water
                // by issuing low-level ucode commands G_TRI_FILL and G_TRI_SHADE_TXTR
                // the port renders the sky in a different manner
                break;
            case G_RDPFLUSH_EXT:
                gfx_flush();
                break;
            case G_CLEAR_DEPTH_EXT:
                gfx_flush();
                gfx_rapi->clear_framebuffer(false, true);
                break;
            case G_RDPPIPESYNC:
            case G_RDPFULLSYNC:
            case G_RDPLOADSYNC:
            case G_RDPTILESYNC:
                break;
            default:
#ifdef PLATFORM_PORT
                /* M2 bring-up: Banjo's F3DEX-1.x stream may carry opcodes this
                 * F3DEX2-derived gfx_pc doesn't model yet. Warn (rate-limited per
                 * opcode) and skip rather than abort, so a frame can be captured
                 * and the remaining opcodes triaged from the log. */
                {
                    static uint8_t warned[256];
                    if (!warned[opcode]) {
                        warned[opcode] = 1;
                        sysLogPrintf(LOG_WARNING,
                            "gfx_pc: unhandled GBI opcode 0x%02x (w0 %08x w1 %08x) — skipping",
                            opcode, cmd->words.w0, cmd->words.w1);
                    }
                }
#else
                sysFatalError("Unknown GBI opcode 0x%02x at %p.\nw0 %08x\nw1 %08x", opcode, cmd, cmd->words.w0, cmd->words.w1);
#endif
                break;
        }
        ++cmd;
    }
}

static void gfx_sp_reset() {
    rsp.modelview_matrix_stack_size = 1;
    rsp.current_num_lights = 2;
    rsp.lights_changed = true;
}

extern "C" void gfx_get_dimensions(uint32_t* width, uint32_t* height, int32_t* posX, int32_t* posY) {
    gfx_wapi->get_dimensions(width, height, posX, posY);
}

extern "C" void gfx_init(const GfxInitSettings *settings) {
    gfx_wapi = settings->wapi;
    gfx_rapi = settings->rapi;
    gfx_wapi->init(&settings->window_settings);
    gfx_rapi->init();
    gfx_rapi->update_framebuffer_parameters(0, settings->window_settings.width, settings->window_settings.height, 1, false, true, true, true);
    gfx_current_dimensions.internal_mul = 1;
    gfx_current_game_window_viewport.width = gfx_current_dimensions.width = settings->window_settings.width;
    gfx_current_game_window_viewport.height = gfx_current_dimensions.height = settings->window_settings.height;
#ifdef PLATFORM_PORT
    /* The game's display-list coordinates (viewport/scissor) are expressed in the
     * N64 native framebuffer space (320x240). SCREEN_WIDTH/HEIGHT (= this native
     * viewport) is the denominator of RATIO_X/RATIO_Y; if left 0 the viewport math
     * divides by zero and produces a zero-size glViewport (nothing rasterizes).
     * PD sets this from its video mode; the headless port must set it explicitly. */
    gfx_current_native_viewport.x = 0;
    gfx_current_native_viewport.y = 0;
    gfx_current_native_viewport.width = 320;
    gfx_current_native_viewport.height = 240;
    gfx_current_native_aspect = 320.0f / 240.0f;
    /* aspect_ratio is otherwise only set in gfx_start_frame; a fill/texture-rect
     * drawn before the first start_frame would divide by 0 in
     * gfx_adjust_x_for_aspect_ratio -> NaN clip coords -> GL discards the quad
     * (background/UI rects vanish). Seed a sane default here. */
    gfx_current_dimensions.aspect_ratio =
        (float)gfx_current_dimensions.width / (float)gfx_current_dimensions.height;
    gfx_current_window_dimensions.aspect_ratio = gfx_current_dimensions.aspect_ratio;
#endif
    game_framebuffer = gfx_rapi->create_framebuffer();
    game_framebuffer_msaa_resolved = gfx_rapi->create_framebuffer();

    if (gfx_msaa_level > 1 && !gfx_framebuffers_enabled) {
        sysLogPrintf(LOG_WARNING, "F3D: MSAA set to %d, but framebuffers are not available; disabling", gfx_msaa_level);
        gfx_msaa_level = 1;
    }

    for (int i = 0; i < 16; i++) {
        segmentPointers[i] = 0;
    }

    if (tex_upload_buffer == nullptr) {
        // We cap texture max to 8k, because why would you need more?
        int max_tex_size = std::min(8192, gfx_rapi->get_max_texture_size());
        tex_upload_buffer = (uint8_t*)malloc(max_tex_size * max_tex_size * 4);
        turok_tex_upload_capacity = (uint32_t)max_tex_size * (uint32_t)max_tex_size * 4u;
    }

    rsp.lookat[0].dir[0] = rsp.lookat[1].dir[1] = 0x7F;
    rsp.current_lookat_coeffs[0][0] = rsp.current_lookat_coeffs[1][1] = 1.f;
    rsp.lookat_enabled = true;
}

extern "C" void gfx_destroy(void) {
    // TODO: should also destroy rapi and wapi, and any other resources acquired in fast3d

    // Texture cache and loaded textures store references to Resources which need to be unreferenced.
    gfx_texture_cache_clear();
}

extern "C" struct GfxRenderingAPI* gfx_get_current_rendering_api(void) {
    return gfx_rapi;
}

#ifdef PLATFORM_PORT
/* Banjo submits several gfx tasks (display lists) per presented frame; each one
 * arrives as a separate gfx_run(). On the N64 they all rasterize into the SAME
 * framebuffer with a single clear at frame start. So clear only on the FIRST
 * gfx_run after a frame is opened (gfx_start_frame), and let subsequent DLs
 * accumulate — otherwise a later DL's clear wipes the geometry of earlier ones. */
static bool s_bk_frame_clear_pending = true;
#endif

extern "C" void gfx_start_frame(void) {
#ifdef PLATFORM_PORT
    s_bk_frame_clear_pending = true;
#endif
    gfx_wapi->handle_events();
    gfx_wapi->get_dimensions(&gfx_current_window_dimensions.width, &gfx_current_window_dimensions.height,
                             &gfx_current_window_position_x, &gfx_current_window_position_y);

    if (gfx_current_window_dimensions.height == 0) {
        // Avoid division by zero
        gfx_current_window_dimensions.height = 1;
    }

    gfx_current_window_dimensions.aspect_ratio = (float)gfx_current_window_dimensions.width / gfx_current_window_dimensions.height;

    gfx_current_dimensions = gfx_current_window_dimensions;

#ifdef PLATFORM_PORT
    /* Publish the real output aspect ratio for the game's projection (camera.c Hor+ widescreen). */
    { extern float g_turok_aspect; if (gfx_current_dimensions.aspect_ratio > 0.1f) g_turok_aspect = gfx_current_dimensions.aspect_ratio; }
#endif

    gfx_current_game_window_viewport.width = gfx_current_dimensions.width;
    gfx_current_game_window_viewport.height = gfx_current_dimensions.height;

    if (gfx_current_dimensions.height != gfx_prev_dimensions.height) {
        for (auto& fb : framebuffers) {
            uint32_t width, height, msaa;
            if (fb.second.autoresize) {
                if (fb.second.upscale) {
                    width = fb.second.orig_width;
                    height = fb.second.orig_height;
                    gfx_adjust_width_height_for_scale(width, height);
                } else {
                    // assume this is a fullscreen fb
                    width = gfx_current_dimensions.width;
                    height = gfx_current_dimensions.height;
                }
                if (width != fb.second.applied_width || height != fb.second.applied_height) {
                    gfx_rapi->update_framebuffer_parameters(fb.first, width, height, 1, true, true, true, true);
                    fb.second.applied_width = width;
                    fb.second.applied_height = height;
                }
            }
        }
    }
    gfx_prev_dimensions = gfx_current_dimensions;

    bool different_size = gfx_current_dimensions.width != gfx_current_game_window_viewport.width ||
                          gfx_current_dimensions.height != gfx_current_game_window_viewport.height;
    if (gfx_framebuffers_enabled && (different_size || gfx_msaa_level > 1)) {
        game_renders_to_framebuffer = true;
        if (different_size) {
            gfx_rapi->update_framebuffer_parameters(game_framebuffer, gfx_current_dimensions.width,
                                                    gfx_current_dimensions.height, gfx_msaa_level, true, true, true,
                                                    true);
        } else {
            // MSAA framebuffer needs to be resolved to an equally sized target when complete, which must therefore
            // match the window size
            gfx_rapi->update_framebuffer_parameters(game_framebuffer, gfx_current_window_dimensions.width,
                                                    gfx_current_window_dimensions.height, gfx_msaa_level, false, true,
                                                    true, true);
        }
        if (gfx_msaa_level > 1 && different_size) {
            gfx_rapi->update_framebuffer_parameters(game_framebuffer_msaa_resolved, gfx_current_dimensions.width,
                                                    gfx_current_dimensions.height, 1, false, false, false, false);
        }
    } else {
        game_renders_to_framebuffer = false;
    }

    fbActive = 0;

    // update aspect scale and offset
    gfx_update_aspect_mode();
}

uint32_t num_dls = 0;

extern "C" void gfx_run(Gfx* commands) {
    ++num_dls;
    BK_TR(BK_TR_GFX, "gfx_run #%u dl=%p dims=%ux%u aspect=%.3f", num_dls, (void*)commands,
          gfx_current_window_dimensions.width, gfx_current_window_dimensions.height,
          gfx_current_window_dimensions.aspect_ratio);
    gfx_sp_reset();

    // puts("New frame");

    if (!gfx_wapi->start_frame()) {
        dropped_frame = true;
        return;
    }
    dropped_frame = false;

    gfx_rapi->update_framebuffer_parameters(0, gfx_current_window_dimensions.width,
                                            gfx_current_window_dimensions.height, 1, false, true, true,
                                            !game_renders_to_framebuffer);
    gfx_rapi->start_frame();
    gfx_rapi->start_draw_to_framebuffer(game_renders_to_framebuffer ? game_framebuffer : 0,
                                        (float)gfx_current_dimensions.height / SCREEN_HEIGHT);
#ifdef PLATFORM_PORT
    /* Clear once per presented frame (first DL), not per gfx_run — see note at
     * s_bk_frame_clear_pending. Always clear depth so later DLs depth-test sanely. */
    if (s_bk_frame_clear_pending) {
        gfx_rapi->clear_framebuffer(true, true);
        s_bk_frame_clear_pending = false;
    } else {
        gfx_rapi->clear_framebuffer(false, true);
    }
#else
    gfx_rapi->clear_framebuffer(true, false);
#endif
    rdp.viewport_or_scissor_changed = true;
    rendering_state.viewport = {};
    rendering_state.scissor = {};
#ifdef PLATFORM_PORT
    g_mtx_push_dropped = 0; g_mtx_pop_underflow = 0; g_mtx_max_depth = (int)rsp.modelview_matrix_stack_size;
#endif
    gfx_run_dl(commands);
#ifdef PLATFORM_PORT
    /* ALWAYS warn (capped) on a real stack imbalance — with MODELVIEW_STACK_DEPTH=64 this should
     * never fire; if it does, a hierarchy exceeded the cap (raise it) or there is another unbalanced
     * push/pop source. */
    if (g_mtx_push_dropped || g_mtx_pop_underflow) {
        static int s_warned = 0;
        if (s_warned++ < 8)
            fprintf(stderr, "[mtxstack] <<< IMBALANCE size=%u maxdepth=%d push_dropped=%d pop_underflow=%d (camera/HUD corruptor)\n",
              rsp.modelview_matrix_stack_size, g_mtx_max_depth, g_mtx_push_dropped, g_mtx_pop_underflow);
    }
#endif
    gfx_flush();
    gfxFramebuffer = 0;

    if (game_renders_to_framebuffer) {
        gfx_rapi->start_draw_to_framebuffer(0, 1);
        gfx_rapi->clear_framebuffer(true, true);

        if (gfx_msaa_level > 1) {
            bool different_size = gfx_current_dimensions.width != gfx_current_game_window_viewport.width ||
                                  gfx_current_dimensions.height != gfx_current_game_window_viewport.height;

            if (different_size) {
                gfx_rapi->resolve_msaa_color_buffer(game_framebuffer_msaa_resolved, game_framebuffer);
                gfxFramebuffer = (uintptr_t)gfx_rapi->get_framebuffer_texture_id(game_framebuffer_msaa_resolved);
            } else {
                gfx_rapi->resolve_msaa_color_buffer(0, game_framebuffer);
            }
        } else {
            gfxFramebuffer = (uintptr_t)gfx_rapi->get_framebuffer_texture_id(game_framebuffer);
        }
    }

    gfx_rapi->end_frame();
    gfx_wapi->swap_buffers_begin();
}

#ifdef PLATFORM_3DS
extern "C" void gfx_citro3d_set_decode_only(int on);
extern "C" void gfx_citro3d_set_facade_reg_mode(int on);
// LOAD-TIME texture pre-warm (3DS): run the GBI interpreter on a throwaway room GDL purely to
// DECODE + bake its textures up front — NO frame, NO present, NO recorded draws. The decode/bake
// path (import_texture → upload_texture → the facade bake) is CPU-only C3D texture work that does
// not need an active C3D_FrameBegin, so this is safe to call at level-load time. The backend's
// decode-only flag makes draw_triangles drop the (frameless, garbage) draws. Used by the bg.c
// facade pre-warm so building textures bake when their room loads, not on first render.
extern "C" void gfx_run_dl_decode_only(Gfx* commands) {
    gfx_citro3d_set_decode_only(1);
    gfx_sp_reset();
    gfx_run_dl(commands);
    gfx_flush();
    gfx_citro3d_set_decode_only(0);
}
// PASS 1 of the pre-warm: same decode-only run, but draw_triangles REGISTERS heavily-tiled draws as
// facades (by source address) — render-time registration that sees exactly the drawn geometry
// (sub-DLs included) the bg.c GDL scan misses, flare-safe (room geometry only). It registers +
// invalidates stale caches; the following gfx_run_dl_decode_only (pass 2) re-decodes + bakes them.
extern "C" void gfx_run_dl_facade_register(Gfx* commands) {
    gfx_citro3d_set_decode_only(1);
    gfx_citro3d_set_facade_reg_mode(1);
    gfx_sp_reset();
    gfx_run_dl(commands);
    gfx_flush();
    gfx_citro3d_set_facade_reg_mode(0);
    gfx_citro3d_set_decode_only(0);
}
#endif

extern "C" void gfx_end_frame(void) {
    BK_TR(BK_TR_PRESENT, "gfx_end_frame dropped=%d dls_this_frame=%u", (int)dropped_frame, num_dls);
    if (!dropped_frame) {
        gfx_rapi->finish_render();
        gfx_wapi->swap_buffers_end();
    }
    num_dls = 0;  /* per-presented-frame DL count (for the trace) */
#ifdef BK_GFX_TRACE
    { static int _fn = 0; fprintf(stderr, "[FRAME] %d tris_drawn=%zu\n", _fn++, g_bk_tris_drawn); g_bk_tris_drawn = 0; }
#endif
}

extern "C" void gfx_set_target_fps(int fps) {
    gfx_wapi->set_target_fps(fps);
}

extern "C" void reset_texture_state() {
    gfx_texture_cache_clear();
    if (rendering_state.shader_program) {
        gfx_rapi->unload_shader(rendering_state.shader_program);
        rendering_state.shader_program = nullptr;
    }
    gfx_rapi->clear_shaders();
    color_combiner_pool.clear();
    prev_combiner = color_combiner_pool.end();
}

extern "C" void gfx_set_texture_filter(enum FilteringMode mode) {
    reset_texture_state();
    gfx_rapi->set_texture_filter(mode);
}

extern "C" void gfx_set_mipmap_filter(enum MipmapFilteringMode mode) {
    reset_texture_state();
    gfx_rapi->set_mipmap_filter(mode);
}

extern "C" int gfx_create_framebuffer(uint32_t width, uint32_t height, int upscale, int autoresize) {
    int fb = gfx_rapi->create_framebuffer();
    gfx_resize_framebuffer(fb, width, height, upscale, autoresize);
    return fb;
}

extern "C" void gfx_resize_framebuffer(int fb, uint32_t width, uint32_t height, int upscale, int autoresize) {
    uint32_t orig_width, orig_height;

    if (width && height) {
        // user-specified size
        orig_width = width;
        orig_height = height;
        if (upscale) {
            gfx_adjust_width_height_for_scale(width, height);
        }
        gfx_rapi->update_framebuffer_parameters(fb, width, height, 1, true, true, true, true);
    } else {
        // same size as main fb
        orig_width = width = gfx_current_dimensions.width;
        orig_height = height = gfx_current_dimensions.height;
        upscale = false;
        autoresize = true;
        gfx_rapi->update_framebuffer_parameters(fb, width, height, 1, true, true, true, true);
    }

    framebuffers[fb] = { orig_width, orig_height, width, height, (bool)upscale, (bool)autoresize };
}

extern "C" void gfx_set_framebuffer(int fb, float noise_scale) {
    gfx_rapi->start_draw_to_framebuffer(fb, noise_scale);
    gfx_rapi->clear_framebuffer(true, true);
    active_fb = framebuffers.find(fb);
}

extern "C" void gfx_copy_framebuffer(int fb_dst, int fb_src, int left, int top, int use_back) {
    const bool is_main_fb = (fb_src == 0);

    if (is_main_fb) {
        if (left > 0 && top > 0) {
            // upscale the position
            left = left * gfx_current_dimensions.width / gfx_current_native_viewport.width;
            top = top * gfx_current_dimensions.height / gfx_current_native_viewport.height;
            // flip Y
            top = gfx_current_dimensions.height - top - 1;
        }
        if (use_back && gfx_msaa_level > 1) {
            // read from the framebuffer we've been rendering to
            fb_src = game_framebuffer;
        }
    }

    gfx_rapi->copy_framebuffer(fb_dst, fb_src, left, top, is_main_fb, (bool)use_back);
}

extern "C" void gfx_reset_framebuffer(void) {
    gfx_rapi->start_draw_to_framebuffer(0, (float)gfx_current_dimensions.height / SCREEN_HEIGHT);
    active_fb = framebuffers.end();
}
