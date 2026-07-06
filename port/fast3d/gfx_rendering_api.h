#ifndef GFX_RENDERING_API_H
#define GFX_RENDERING_API_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct ShaderProgram;

struct GfxClipParameters {
    bool z_is_from_0_to_1;
    bool invert_y;
};

enum FilteringMode { FILTER_NONE, FILTER_LINEAR, FILTER_THREE_POINT };
enum MipmapFilteringMode { MIPMAP_DISABLED, MIPMAP_NEAREST, MIPMAP_LINEAR };

struct GfxRenderingAPI {
    const char* (*get_name)(void);
    int (*get_max_texture_size)(void);
    struct GfxClipParameters (*get_clip_parameters)(void);
    void (*unload_shader)(struct ShaderProgram* old_prg);
    void (*load_shader)(struct ShaderProgram* new_prg);
    struct ShaderProgram* (*create_and_load_new_shader)(uint64_t shader_id0, uint32_t shader_id1);
    struct ShaderProgram* (*lookup_shader)(uint64_t shader_id0, uint32_t shader_id1);
    void (*shader_get_info)(struct ShaderProgram* prg, uint8_t* num_inputs, bool used_textures[2]);
    void (*clear_shaders)(void);
    uint32_t (*new_texture)(void);
    void (*select_texture)(int tile, uint32_t texture_id, bool linear_filter);
    void (*upload_texture)(const uint8_t* rgba32_buf, uint32_t width, uint32_t height, bool gen_mipmaps);
    void (*set_sampler_parameters)(int sampler, bool linear_filter, uint32_t cms, uint32_t cmt, bool mipmaps);
    void (*set_depth_mode)(bool depth_test, bool depth_update, bool depth_compare, bool depth_source_prim, uint16_t zmode);
    void (*set_depth_range)(float znear, float zfar);
    void (*set_viewport)(int x, int y, int width, int height);
    void (*set_scissor)(int x, int y, int width, int height);
    void (*set_use_alpha)(bool use_alpha, bool modulate);
    void (*draw_triangles)(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris);
    void (*init)(void);
    void (*on_resize)(void);
    void (*start_frame)(void);
    void (*end_frame)(void);
    void (*finish_render)(void);
    int (*create_framebuffer)();
    void (*update_framebuffer_parameters)(int fb_id, uint32_t width, uint32_t height, uint32_t msaa_level,
                                          bool opengl_invert_y, bool render_target, bool has_depth_buffer,
                                          bool can_extract_depth);
    bool (*start_draw_to_framebuffer)(int fb_id, float noise_scale);
    void (*copy_framebuffer)(int fb_dst, int fb_src, int left, int top, bool flip_y, bool use_back);
    void (*clear_framebuffer)(bool clear_color, bool clear_depth);
    void (*resolve_msaa_color_buffer)(int fb_id_target, int fb_id_source);
    void* (*get_framebuffer_texture_id)(int fb_id);
    void (*select_texture_fb)(int fb_id);
    void (*delete_texture)(uint32_t texID);
    void (*set_texture_filter)(enum FilteringMode mode);
    enum FilteringMode (*get_texture_filter)(void);
    void (*set_mipmap_filter)(enum MipmapFilteringMode mode);
	void (*set_anisotropy_level)(int);
	int (*get_max_anisotropy_level)(void);
	// Optional (NULL on backends that don't need it; gfx_pc NULL-checks). Called when
	// gfx_pc wipes its whole texture cache — lets a backend drop per-texture side data
	// (the 3DS pre-tile bake) that gfx_pc orphans without a delete_texture call.
	void (*invalidate_texture_cache)(void);
	// FOG (3DS/Citro3D, CLAUDE.md §3.3/§20.12). Optional — NULL on backends that render fog
	// in their fragment shader from the buf_vbo fog floats (the GL backend). The 3DS has no
	// programmable fragment stage, so it uses the PICA fixed-function fog unit: gfx_pc calls
	// this from gfx_flush with the current N64 fog state (G_FOG on/off + fog_mul/fog_offset =
	// the per-vertex fog line `clamp(z/w*mul+offset,0,255)` + the fog colour). The backend
	// builds a depth FogLut from mul/offset and binds C3D_FogColor/C3D_FogGasMode per draw.
	void (*set_fog)(bool enabled, int16_t mul, int16_t offset, uint8_t r, uint8_t g, uint8_t b);
	// Optional (NULL on backends that don't need them — 3DS never uses internal-resolution
	// scaling; gfx_pc NULL-checks). PC-only INTERNAL RESOLUTION pair: turok.cfg's resolution row
	// picked a non-native size while fullscreen (gfx_sdl2.cpp refresh_internal_resolution()) —
	// fullscreen never mode-switches the actual display, so the chosen size is realized as a
	// render-target scale instead. Deliberately a SELF-CONTAINED GL object pair, independent of
	// the create_framebuffer/update_framebuffer_parameters/gfx_framebuffers_enabled machinery
	// above: turok_gfx.c disables that flag UNCONDITIONALLY on every PC backend ("render straight
	// to the default framebuffer" — the old headless-capture path had no FBO-blit step), which
	// would otherwise silently no-op every one of those calls.
	//   internal_res_bind: ensure (create/resize as needed) and BIND the dedicated internal
	//   render target for this frame's draws, at exactly width x height.
	void (*internal_res_bind)(uint32_t width, uint32_t height);
	//   internal_res_present: scale+letterbox the internal target onto the real screen at the
	//   given dest rect (the caller has already computed an aspect-preserving fit) — clears the
	//   screen to black first, which becomes the letterbox bars. screen_fb is the GL framebuffer
	//   OBJECT id the CALLER's window-manager backend treats as "the real screen"
	//   (GfxWindowManagerAPI::get_screen_framebuffer) — NOT always literal GL id 0: a surfaceless
	//   EGL context has no default framebuffer 0 at all and renders into its own FBO instead.
	void (*internal_res_present)(uint32_t screen_fb, int dst_x, int dst_y, int dst_width, int dst_height);
};

#endif
