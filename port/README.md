# `port/` — the Turok platform layer

This directory is the **only platform-specific code** in the port; everything under `src/PR/tengine/**` is the
original N64 game engine, compiled unmodified wherever possible. The layout deliberately mirrors the sibling
**banjo-kazooie** and **perfect_dark** ports so the three share one pattern — see
[`../docs/N64_PORTING_PLAYBOOK.md`](../docs/N64_PORTING_PLAYBOOK.md) for the cross-project rationale.

```
port/
├── src/        ← libultra shims, the host boot driver, the ROM/RCP/audio/input seams
├── include/    ← the platform-abstraction headers (force-include + OS/arch macros + trace)
├── fast3d/     ← the Fast3D (F3DEX) interpreter + pluggable GL/EGL/Citro3D backends
│   ├── glad/   ← GL loader
│   └── shaders/← PICA200 vertex shader (3DS)
└── tools/      ← host-side debug aids
```

## Naming convention (identical to banjo-kazooie / perfect_dark)

| Category | Rule | Examples |
|---|---|---|
| **`turok_` prefix** | engine/boot **glue rewritten for Turok** (the analog of banjo's `bk_` / PD's `pd*`) | `turok_main.c`, `turok_gfx.c`, `turok_runtime.c`, `turok_adpcm.c`, `turok_port.h`, `turok_trace.h` |
| **plain names** | the **portable / shared platform layer** kept under the same name as banjo/PD so the framework stays diffable (and a reusable library can lift it verbatim) | `os_shim.c`, `romdata.c`, `audio.c`/`audio.h`, `input.c`/`input.h`, `system.c`/`system.h`, `audio_lib_stub.c`, `platform.h`, and all `fast3d/gfx_*` files |
| **`_3ds` suffix** | **3DS-only** implementation of a shared role (compiled only when `PLATFORM_3DS`) | `gfx_3ds.c`, `gfx_citro3d.cpp` (more arrive at M3 — see roadmap) |

> Banjo kept one shim un-prefixed (`os_shim.c`) by precedent; turok follows that exactly. PD calls the same
> file `libultra.c`. The audio layer uses the plain shared name `audio.c` (matching the siblings) even though
> the game also has `src/PR/tengine/audio.c`: they compile to **distinct objects** (`audio.o` vs `port_audio.o`)
> and the port build's `-I` path never reaches `src/PR/tengine`, so there is no real collision — and keeping the
> shared name is what lets a future reusable library lift the file verbatim.

## File roles (current) and their sibling analog

### `port/src/`
| File | Role | banjo / PD analog |
|---|---|---|
| `turok_main.c` | host entry + boot driver (`romdataInit → boot → mainproc`, frame-limit `longjmp`) | `bk_main.c` / `main.c` |
| `os_shim.c` | the central libultra (`os*`) shim — RCP/VI/AI/SI seam, **the 30 Hz logic-tick clock** (`g_turok_logic_tick`, `turok_render_alpha`), gfx-task dispatch, audio-out + input wiring | `os_shim.c` / `libultra.c` |
| `romdata.c` | the single cartridge DMA seam — `romPiRead()` = bounds-checked `memcpy` from the cart blob; Path A (`TUROK_CARTDATA`) / Path B (`TUROK_ROM` @ ROM `0x1F00`) | `romdata.c` / `romdata.c` |
| `turok_runtime.c` | the linker symbols the N64 `spec`/`makerom` would provide — `_staticSegmentRomStart`, boot segments, **zero-span audio-bank aliases** | `bk_runtime.c` / (`system.c`) |
| `turok_gfx.c` | Fast3D bridge (`gfx_init/run/start_frame/end_frame`) + frame capture (`TUROK_CAPTURE_*`→PNG) | `bk_gfx.c` / `video.c` |
| `input.c` | host controller seam — `inputReadController(idx, void *npad)` (fill the N64 pad, **opaque handle**) + `inputSetState` (backend push) + `inputInit`; `TUROK_FAKEINPUT` for headless | `input.c` / `input.c` |
| `audio.c` | host audio **sink** — the same stash (`audioSetNextBuffer`) + push-with-back-pressure (`audioEndFrame`) contract as PD/banjo `audio.c` (SDL2 queue / headless WAV), **plus the dedicated audio thread** (`audioThreadStart`/`SynthLock`, pthread, gated by `TUROK_AUDIO_THREAD`) folded in — PD spins that thread only on 3DS, turok also on desktop | `audio.c` (sink) + `audio_3ds.c` (thread) |
| `turok_adpcm.c` | host C port of `adpcmDecode` (was `adpcm.s`) — decodes anim keyframe quaternion/position streams | *(none — turok-specific)* |
| `audio_lib_stub.c` | benign libaudio (`al*`) synth stubs until the M4 mixer lands | `audio_lib_stub.c` / — |
| `system.c` | host `sys*` helpers (log, fatal-error, args, sleep/relax) implementing the `system.h` contract | `system.c` / `system.c` |

### `port/include/`
| File | Role | banjo / PD analog |
|---|---|---|
| `turok_port.h` | **force-included into every game TU** (`-include`): `#define qsort turok_qsort` (libc collision), `extern float fmodf(float,float)` prototype (the implicit-decl ABI trap), NULL-as-int notes | `bk_heap.h` (force-include role) |
| `platform.h` | OS/arch detection macros | `platform.h` / `platform.h` |
| `system.h` | `sys*` API declarations | `system.h` / `system.h` |
| `audio.h` | the **audio backend contract** — `audioInit`/`audioSetNextBuffer`/`audioEndFrame`/`audioGetBytesBuffered` (sink) + `audioThreadStart`/`Stop`/`SynthLock`/`Unlock` (the dedicated thread) — same 5-function sink contract as PD/banjo | `audio.h` / `audio.h` |
| `input.h` | the **input backend contract** — `inputInit`/`inputReadController`/`inputSetState`, with the N64 pad passed as an **opaque `void*`** so the header is libultra-free and the C++ Fast3D layer includes it cleanly (turok subset of PD's full input.h) | `input.h` (subset) |
| `turok_trace.h` | gated categorized pipeline trace (inert without `-DBK_TRACE`, which turok never sets); renamed from `bk_trace.h` | `bk_trace.h` |

### `port/fast3d/`  — copied from banjo, adapted; **already matches the sibling naming exactly**
`gfx_pc.cpp` (the F3DEX-1.x/NoN interpreter — **fix decode bugs here, once**), `gfx_cc.*` (combiner→TEV),
`gfx_opengl.*` (PC ground truth), `gfx_egl.*` (headless hardware GL via GBM render node), `gfx_sdl2.cpp` +
`gfx_glcapture.cpp` (SDL2 window + capture), `gfx_osmesa.*` (software headless), `gfx_citro3d.*` + `gfx_3ds.*`
(3DS, M3), `glad/`, `shaders/`. Turok **adds** `gfx_egl` + `gfx_glcapture` over banjo — net-new headless-capture
backends, not divergence.

### `port/tools/`
`memcpy_guard.c` — `-Wl,--wrap` memcpy/bcopy recorder + SIGSEGV handler (fault addr + last copy + backtrace);
linked only in `debug`/`asan` builds. The reliable debugger here (gdb can't unwind the asm memcpy).

## The port ↔ game seam

Cross-layer symbols resolve **at final link by name** (not via headers — see the deviations below):

| Symbol(s) | Defined in | Used by |
|---|---|---|
| `romPiRead`, `romdataInit` | `romdata.c` | `os_shim.c`, `turok_main.c` |
| `turokGfxRun/StartFrame/EndFrame/Init/SavePng` | `turok_gfx.c` | `os_shim.c`, `turok_main.c` |
| `audioInit`, `audioSetNextBuffer`/`audioEndFrame`/`audioGetBytesBuffered`, `audioThreadStart`/`Stop`/`SynthLock` | `audio.c` (via `audio.h`) | `os_shim.c`, `turok_main.c` |
| `inputReadController` / `inputSetState` / `inputInit` | `input.c` (via `input.h`) | `os_shim.c` / `gfx_sdl2.cpp` |
| `g_turok_logic_tick`, `turok_render_alpha`, `g_turok_anim_step` | `os_shim.c` / `tengine.c` | `tengine.c`, `romstruc.c` (the framerate-interpolation seam) |

## Deliberate deviations from the sibling pattern (and why)

These are **intentional** — turok's approach works and the sibling form would add risk for no functional gain.
Documented so the divergence is a known choice, not drift:

1. **Byteswap is inline `ORDERBYTES` at the typed consumer in the game source, not a `turok_swap.c` registry**
   (banjo `bk_swap.c` / PD `preprocess/*`). The ~33 per-parser swaps were applied crash-by-crash and verified;
   consolidating them into one registry would be a risky logic-refactor of working code. See the playbook §2.
2. **Headless rendering is real off-screen GL** (`GFX=egl` via GBM, or `GFX=osmesa`), **not** a
   `backend_headless.c` null renderer. A legitimate different choice — it exercises the real Fast3D path.
3. **The RCP/task seam is the `#ifdef PLATFORM_PORT` patch in `src/PR/tengine/sched.c::scSendCommand`**
   (dispatch the gfx task **and present immediately** — the M2 "renders-forever" blocker fix) plus the dispatch
   in `os_shim.c`, **not** a standalone `turok_rcp.c` (banjo `bk_rcp.c` / PD `pdsched.c`). The present-after-task
   call is load-bearing; extracting it from game source is high-risk for zero gain.
4. **The audio AND input seams use contract headers (`audio.h`, `input.h`); only the gfx-bridge + romdata seams
   stay inline `extern`.** Both `audio.h` and `input.h` are exposed exactly like the siblings. The input C++
   friction — `inputReadController` needs the `OSContPad` libultra type the **C++** `gfx_sdl2.cpp` can't cleanly
   include — is solved by an **opaque `void *npad` pad handle**: the header carries no libultra type, the C
   consumer (`os_shim.c`) passes `&osContPad`, and `input.c` casts it back. Only `turokGfx*` (the gfx bridge) and
   `romPiRead`/`romdataInit` (romdata) remain inline `extern` — single-consumer C seams with no friction;
   header-ize them too if the eventual library wants full uniformity.

## Conformance roadmap — files that arrive at their milestone

| Missing role | Sibling file | Lands at |
|---|---|---|
| classic-ABI Acmd software mixer | `mixer.c` (plain) | **M4** audio |
| save / EEPROM VFS | `fs.c` (plain) | when `loadsave.c`/`persist.c` port |
| 3DS sys / heap / stack override | `sys_3ds.c` | **M3** 3DS bring-up |
| 3DS audio sink (ndsp) | `audio_3ds.c` | **M3** |
| 3DS input (libctru HID) | `input_3ds.c` | **M3** |
| interface headers / `turok_swap.c` | `video.h`…, `bk_swap.c` | optional (see deviations 1 & 4) |

## Build

The canonical build is **`tools/build_port.sh`** (not `make` — NFS mtimes defeat incremental builds; the script
compiles every TU fresh to a local dir). Both build paths **glob `port/src/*.c`**, so adding/renaming a port
source is picked up automatically — no source list to edit.

```bash
GFX=sdl2 TUROK_OUT=/tmp/tb bash tools/build_port.sh release   # hardware GL window (play_level.sh path)
GFX=egl  TUROK_OUT=/tmp/tb bash tools/build_port.sh release   # headless hardware GL (no X server)
```

`GFX_DEF` is threaded identically into three compile groups — `port/src` (so `turok_gfx.c` picks the window
manager), the `fast3d` C++ TUs (so `gfx_opengl.cpp` picks the GL loader), and the link libs. `turok_port.h` is
force-included by exact path in both `build_port.sh` and `Makefile.port`; do not move it. `coll.c` is excluded
(deferred). Build `-m32` (N64 pointer width), link with `g++` (Fast3D C++ runtime).
