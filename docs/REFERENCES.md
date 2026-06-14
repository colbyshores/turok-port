# Reference Projects — Distilled Notes

Digested 2026-06-12 from the sibling ports in `/mnt/nas/Development/`. These are the "rosetta stones"
the Turok port draws on. Keep this current so future sessions don't have to re-read every sibling repo.

---

## The shared architecture (all N64 ports converge on this)

```
Original/decompiled game C  ──►  Fast3D interpreter (gfx_pc.cpp)  ──►  GfxRenderingAPI backend
   (untouched logic)              decodes N64 display lists,            ├─ gfx_opengl.cpp  (PC, GROUND TRUTH)
                                  emulates RSP vtx + RDP state          ├─ gfx_osmesa.cpp  (PC headless capture)
                                  on the CPU                            └─ gfx_citro3d.cpp (3DS / PICA200)
```

- **Two interface contracts**: `GfxRenderingAPI` (~18 fns: upload_texture, set_combine, draw_triangles,
  framebuffer ops, set_fog…) and `GfxWindowManagerAPI` (~20 fns: display mode, events, swap, timing).
- **Backend selected at compile time** in one `.c` (`video.c`) via `#ifdef PLATFORM_3DS`.
- **Game logic is untouched / portable.** Only the seam files differ per platform.
- This abstraction has shipped across SM64, OoT, MM, Banjo-Kazooie, DK64, Perfect Dark.

---

## Banjo-Kazooie  — `/mnt/nas/Development/banjo-kazooie`  ★ CLOSEST MATCH

Why closest: it interprets **F3DEX-1.x** (`G_TRI2`, `G_QUAD`, `G_LINE3D`, `G_MOVEWORD`) — the same
microcode *family* as Turok's `gspF3DEX_NoN`. Its `port/` skeleton is the template to copy.

**Layout**
- `Makefile` — original IDO matching build (untouched). `Makefile.port` — PC (`gcc -m32`, `RENDER=null|gl`,
  `-DPLATFORM_PORT`). `Makefile.3ds` — devkitARM (`-DPLATFORM_3DS -D__3DS__`, `__stacksize__=2MB`).
- `port/fast3d/`: `gfx_pc.cpp` (interpreter — fix bugs here once), `gfx_cc.cpp` (combiner decoder),
  `gfx_opengl.cpp`, `gfx_osmesa.cpp` (headless PNG capture), `gfx_sdl2.cpp`, `gfx_citro3d.cpp`, `gfx_3ds.c`,
  `gfx_rendering_api.h`, `gfx_window_manager_api.h`, `shaders/gfx_citro3d.v.pica`.
- `port/src/`: `bk_main.c` (boot driver), `bk_rcp.c` (thread5 gfx+audio task interception),
  `os_shim.c` (libultra shims), `bk_swap.c` (**byteswap registry** — per-type swappers, keyed to *load
  generation* not pointer), `romdata.c` (`romPiRead` DMA seam, reads `sdmc:/3ds/banjo/baserom…z64`),
  `mixer.c` (RSP audio → C, NEON on ARM), `backend_headless.c`, `audio_3ds.c`, `input_3ds.c`, `sys_3ds.c`.
- `port/include/`: `video.h`, `audio.h`, `input.h`, `system.h`, `romdata.h`, `bk_swap.h`, `mixer.h`,
  `platform.h`, `bk_trace.h`.
- `tools/`: `bk_rom_compressor/` (Rust), `bk_asset_tool/` (Rust), `n64splat/` (Python), `rareunzip.py`.

**Reusable rules (cost real sessions)**
- Build PC-GL ground truth **first**; headless self-diagnosis before any 3DS.
- Clear depth to FAR (`0xFFFFFFFF`), not 0 — else all depth-tested 3D is rejected. One `[0,1]` depth space.
- `svcGetSystemTick()` not `gettimeofday()` (3DS newlib recursion bug); name VFS init to avoid libctru `fsInit`.
- `__stacksize__ = 2 MB`; GPU buffers from `linearAlloc`; full power-cycle between HW tests (leftover FCRAM → texture smear).
- UBSan alignment smoke test on PC before every 3DS push: `UBSAN=1 … 2>&1 | grep -E "misaligned|runtime error"`.
- Endian: swap **structure only** (offsets/counts/headers) at the typed consumer; pixels stay big-endian → convert at upload.
- Macro trick for shared files: `#ifdef PLATFORM_PORT` (UB-free value) `#else` (original IDO tokens) — keeps matching build byte-identical.
- Anti-patterns that wasted time: GPU-side MVP caches (CPU wall is the GBI walk, not MVP); `-mno-unaligned-access` to "fix" ARM faults (doesn't work); HW soft-relaunch between tests.

---

## Perfect Dark — `/mnt/nas/Development/perfect_dark`  ★ MOST MATURE 3DS BACKEND

**Layout**: `src/game/`, `src/lib/` (decomp); `port/src/video.c` (backend selector @ ~L86–104),
`port/fast3d/` (same file set as Banjo). Desktop build = **CMake**; 3DS = hand-written `Makefile.3ds`
(`make -f Makefile.3ds headers` then `-j`). Assets extracted from `data/pd.<region>.z64` at load.

**Citro3D backend (`gfx_citro3d.cpp`, ~175 KB) — the techniques to copy:**
- **Record-replay**: during display-list interp, CPU transforms verts to clip space + records `DrawCmd`s;
  at `end_frame` replays them. **Single-pass stereo**: replay twice (L/R eye) with only a shear uniform
  changed → interp + VBO upload happen once.
- **Fixed 10-float vertex** (pos4, uv2, rgba4) repacked on CPU from variable-stride `buf_vbo`.
- **Combiner → TEV**: N64 colour mux precomputed into a `C3D_TexEnv` chain per shader program (6 stages,
  2-cycle). Only one varying colour (PRIMARY) at a time — fold constants per draw (the SM64 trick).
- **Depth**: CPU `z=(z+w)/2` → `[0,1]`; backend `C3D_DepthMap(true,-1,0)` for PICA `[-1,0]` NDC.
- **90° rotation**: render targets are 240×400 (swapped); viewport/scissor axes swapped; **PICA viewport =
  hardware clip rect** (unlike GL) so portrait sub-rects must FLIP Y: `y = TOP_H-(vpX+vpW)` (§5.4 gotcha).
- **Texture baking**: PICA has no anisotropic filtering → tiled facades stripe at glancing angles. Pre-tile
  small N64 textures into larger baked ones at level load (FCRAM budget ~26 MB, ~256 concurrent), CPU mip chain.
- **Heap split** (`sys_3ds.c`): adaptive MAIN (malloc, ~72 MB incl. ROM) vs GPU (`linearAlloc`, ~48 MB N3DS).
- Shader pipeline: `.pica` → `picasso` → `.shbin` → `bin2s` → object.
- Boot/debug via SD text files: `bootstage.txt`, `autopilot.txt`, `teleport.txt` for headless testing.

See `perfect_dark/docs/3ds_building_texture_stripes.md` for the facade-baking case study.

---

## Forsaken — `/mnt/nas/Development/forsaken`  ★ "N BACKENDS SIDE-BY-SIDE" + ARM ALIGNMENT CATALOGUE

A D3D PC game ported to 3DS; keeps **D3D3 / GL1 / GL2 / GL3 / Citro3D** renderers in one tree, all behind a
~41-function interface in `render.h`, selected by `RENDERER` in `Makefile.3ds`
(`make -f Makefile.3ds RENDERER=citro3d|picagl`). Shared pipeline in `render_gl_shared.c`; native PICA path
in `render_c3d.c` (3,096 lines); GL1 path in `render_gl1.c` runs on the `picaGL/` submodule (GL1-over-PICA).

**Most valuable to Turok — the documented 3DS gotchas:**
- **ARM11 misaligned-float aborts**: 34+ loader sites cast `char*`→struct after reading `u16` fields,
  leaving 2-byte alignment. Fix = byte-wise `memcpy_unaligned()` (prevents GCC `-O2` fusing to `ldm`,
  which needs 4-byte alignment; even a 12-byte `memcpy` fuses — 4-byte copies emit unaligned-tolerant `ldr`).
- **PICA200 culls CCW-front only** (no CW mode); D3D meshes are CW → invert the cull arg on 3DS.
- **Depth `[-1,0]` vs `[0,1]`** → rewrite projection at upload.
- **VRAM budget**: GPU_RGBA4 (16-bit) over RGBA8 to avoid exhaustion; ETC1/ETC1A4 for walls/sprites with
  Gaussian mip chains. Linear (`linearAlloc`) vs BSS heap discipline; Quake-style two-tag hunk allocator.
- Per-vertex lighting moved into a PICA vertex shader to offload CPU on OG-3DS.
- `osGet3DSliderState()` returns garbage on emulators → clamp + config override.

Heap budget cited (OG-3DS ≈ 96 MB): 4 MB code + 18 MB BSS + 32 MB malloc + 32 MB linear + 1 MB stack.

---

## N64Recomp — `/mnt/nas/Development/N64Recomp`  (ALTERNATIVE PATH / ORACLE)

Static recompiler: MIPS → C nearly 1:1 (`addiu $r4,$r4,0x20` → `ctx->r4 = ADD32(ctx->r4,0x20);`), function
granularity, self-repair loop for unresolvable jump targets. Needs ROM + ELF symbols + a TOML config; links
against N64ModernRuntime. Has a working SM64 3DS target (`3ds_project/`, citro3d at
`source/renderer/renderer_3ds.c`, ~106 KB `.3dsx`). Graphics aren't auto-emulated — you hand-write a
citro3d bridge.

**For Turok**: not our primary path (we have real source, which is more maintainable and moddable). But it's
a useful **oracle**: recompiling the retail ROM gives a reference we can diff behavior against, and its 3DS
renderer is another citro3d sample.

---

## SM64 ports (GitHub, not local) — THE ANCESTOR

`sm64-port` / `sm64ex` (PC) and `sm64_3ds` (masterfeizz, 3DS) are where the Fast3D→GL and Fast3D→Citro3D
mappings were first written; Banjo and PD descend from them. **SM64 uses plain F3D**; Turok uses **F3DEX-NoN
(1.x)** — a superset (32-vtx loads, `G_TRI2`/`G_QUAD`). Clone these when we need the canonical citro3d
combiner mapping or to cross-check opcode coverage. The N64Recomp CLAUDE notes reference `sm64_syms.toml`
(4,281 funcs) for the recomp route.

---

## One-line decision table

| Need | Go to |
|------|-------|
| F3DEX-1.x interpreter to copy | `banjo-kazooie/port/fast3d/gfx_pc.cpp` |
| `port/` skeleton + Makefiles | `banjo-kazooie/port/`, `banjo-kazooie/Makefile.{port,3ds}` |
| Best 3DS Citro3D backend | `perfect_dark/port/fast3d/gfx_citro3d.cpp` + `gfx_3ds.c` |
| ARM unaligned-access fixes | `forsaken/3DS_PORT_NOTES.md`, `forsaken/render_gl_shared.c` |
| 3DS heap split / linearAlloc | `perfect_dark/port/src/sys_3ds.c`, `forsaken` README heap section |
| Audio RSP→C mixer | `banjo-kazooie/port/src/mixer.c` (shared with PD) |
| Behavior oracle for the retail ROM | `N64Recomp` recompiled build |
