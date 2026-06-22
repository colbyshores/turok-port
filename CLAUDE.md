# CLAUDE.md — Turok: Dinosaur Hunter (N64) → PC + 3DS Port

> ## ⚠️ WORKING DIRECTIVE (from the user, standing): KEEP GOING. DO NOT STOP TO ASK.
> This is an autonomous, long-running port effort. When a crash is fixed, immediately diagnose and fix the
> next one. Do NOT pause to ask "should I continue?", "want me to keep going?", or to summarize-and-wait —
> just make good engineering decisions and keep working until genuinely blocked on something only the user
> can answer (a real product decision), or the milestone is actually done. Bank progress in this file + memory
> as you go (so context compaction loses nothing), but banking is not a reason to stop. Default = continue.

> Engineering rosetta stone for porting *Turok: Dinosaur Hunter* to PC (ground-truth)
> and the Nintendo 3DS, following the proven pattern established by the sibling
> **Banjo-Kazooie**, **Perfect Dark**, and **Forsaken** ports in `/mnt/nas/Development/`.
>
> Read this first. Every non-obvious decision here was paid for by a real session in a
> sibling project — reuse the lessons verbatim instead of re-discovering them.

---

## 0. TL;DR — What makes Turok different (and easier)

Banjo-Kazooie and Perfect Dark are **decompilations**: their teams reverse-engineered MIPS
assembly back into matching C. **Turok is not.** The 2018 leak gives us the **actual original
C source** of the shipping game engine. We do **not** need to decompile or byte-match anything.

That changes the whole job. Our work is:

1. **Make the original source build off-IRIX** (modern GCC/Clang, 64-bit host, little-endian).
2. **Shim libultra** (`os*`, threads, PI/DMA, VI/AI) — same shims Banjo/PD already wrote.
3. **Interpret Turok's F3DEX display lists** on the host via the shared **Fast3D** interpreter
   (`gfx_pc.cpp`) → pluggable backends (OpenGL on PC, Citro3D on 3DS).
4. **Load assets from the retail ROM** (`baserom.us.v12.z64`) via a single DMA seam, RNC-decompressing
   on demand. The source provides the *code*; the ROM provides the *assets*.
5. **Fix endianness** (N64 assets are big-endian; PC/3DS are little-endian) and **ARM alignment**.

**PC target is the ground truth.** It iterates ~100× faster than 3DS and every bug found on PC
reproduces identically on 3DS (same code, same Fast3D seam). Stand up PC first; the 3DS target is
then only backend-specific (TEV mapping, heap tuning, ARM alignment).

---

## 1. What the leak actually contains (inventory)

The leak is the **SGI/IRIX development tree** Acclaim/Iguana used, not just a game folder. Map:

| Path | What it is | Relevance |
|------|-----------|-----------|
| **`src/PR/tengine/`** | ★ **THE TUROK GAME ENGINE** — 78 `.c`, 85 `.h`, ~137K LOC | **The crown jewel.** This is the whole game. |
| `turoksnd/` | Turok's audio toolchain + the custom sequence player (`csp*`), sound banks, sequences | Audio asset build + the `n_audio`-style synth |
| `seqtest/` | `.Z`-compressed audio/engine test harness (`audio.c`, `gfx.c`, `controller.c`, `gfxdlists.c`) | Early engine scaffolding; reference only |
| `src/PR/` (rest) | The N64 SDK: `libultra/`, NuSystem demos, `gng/` (N64 CPU/RDP simulator), microcode | libultra source to shim against |
| `src/gnu/acpp`, `src/slatec`, `src/X11/motif` | IRIX host toolchain bits (preprocessor, math lib, Motif) | Ignore — host build infra |
| `baserom.us.v12.z64` | Retail US v1.2 ROM, 8 MB, big-endian `.z64` | **Asset source** (levels, textures, models, banks) |

### `src/PR/tengine/` — the engine, by subsystem

- **Boot / core**: `boot.c` (RNC bootstrap loader by "Biscuit"), `tengine.c` (`main`→`boot()`→`mainproc`, libultra threads), `sched.c`, `stacks.c`, `memory.c`, `mempool.c`, `simppool.c`
- **Rendering**: `gdl.c` (graphics display-list segment), `dlists.c`, `gfx16bit.c`, `gfxyield.c`, `graphu64.c`, `geometry.c`, `zbuffer.c`, `cfb.c` (color frame buffers), `scene.c`, `sun.c`, `particle.c`, `fx.c`
- **Game / AI**: `ai.c`, `aistand.c`, `aistruc.c`, `aiweap.c`, `aidoor.c`, `boss.c`, `trex.c`, `mantis.c`, `humvee.c`, `turret.c`, `train.c`, `pickup.c`, `longhunt.c`, `campaign.c`, `warp.c`
- **Player / camera / control**: `camera.c`, `cinecam.c`, `introcam.c`, `crdcam.c`, `galrecam.c`, `hedtrack.c`, `control.c`, `tcontrol.c`, `tmove.c`, `cammodes` (`.h`)
- **Collision**: `coll.c`, `collinfo.c`, `wallcoll.c`, `unicol.c`, `instcol.c`, `wradcol.c`, `regicol.c`, `hash.c` (spatial hash)
- **Assets / IO / compression**: `cart.c` (cart cache heap / streaming), `romstruc.c`, `unpack.c` + `pp.h` + `huffman.c` (**RNC ProPack decompressor**), `textload.c`, `loadsave.c`, `persist.c`, `anim.c`, `mattable.c`
- **Audio**: `audio.c`, `audiomgr.c`, `audheap.c`, `audiocfx.c`, `volume.c`
- **Frontend / UI**: `frontend.c`, `onscrn.c`, `pause.c`, `options.c`, `attract.c`, `cheats.c`, `hsb.c`
- **Misc**: `qsort.c`, `lists.c`, `defs.c`, `prfault.c` (fault handler), `fifo.c`, `unpack.c`

### Hard facts pulled from the source / ROM

- **Microcode**: `gspF3DEX_NoN` (main), `gspF3DLX_NoN`, `gspL3DEX` (lines). This is **F3DEX 1.x, No-Nearclipping** — *not* F3DEX2. (Mario 64 uses plain **F3D**; Turok's is the superset with 32-vtx loads + `G_TRI2`/`G_QUAD`.) → **Banjo's `gfx_pc.cpp` (already F3DEX-1.x) is the closest drop-in interpreter.**
- **Compression**: **RNC** (Rob Northen ProPack, methods 1 & 2). The decompressor source is *in-tree*: `unpack.c`/`pp.h`/`huffman.c`. The ROM holds many small RNC blocks (signature `RNC\x01`/`RNC\x02`); the cart cache heap (`cart.c`) streams + decompresses them.
- **ROM header**: byte order `0x80371240` (native big-endian `.z64`), entry `0x80000400`, game code **`NTUE`** (US), version `0x02`, title `TUROK_DINOSAUR_HUNTE`. IPL3 at `0x40`, game code at ROM `0x1000` (starts `3c0a8000` = `lui $t2,0x8000`).
- **N64 memory map** (from `tengine/spec`): code 1363K @ `0x80000400`; audio heap 292K; display lists 256K @ `0x801b0000`; FIFO 128K; 3× color frame buffers 450K @ `0x80200000`; cart cache heap 1448K; z-buffer 150K. Total 4 MB (assumes base RAM; Expansion Pak gives us headroom on real HW, and PC/3DS dwarf it).
- **Internal version**: `VERSION_NUMBER 49`. Source has JP **kanji overlays** (`overlay/kanji/*`) → multi-region.

> ✅ **Source-vs-ROM "drift" — investigated and largely resolved (2026-06-12, byte-verified).**
> The original worry (that v49 asset offsets wouldn't match retail v1.2) is **not a blocker**, because Turok
> **never compiles in per-asset ROM offsets.** Assets are located by a *runtime self-describing directory*:
> a tree of `CIndexedSet` blocks (a `blockCount` header + an array of big-endian DWORD offsets, each relative
> to the block's own base). The whole tree hangs off **one** linker symbol, `_staticSegmentRomStart`
> ([scene.c:101](src/PR/tengine/scene.c#L101) → [tengine.c:3614](src/PR/tengine/tengine.c#L3614)); the code
> reads `offsets[index]` straight out of the loaded block ([cart.c:74](src/PR/tengine/cart.c#L74)) and returns
> `base + offset` ([cart.c:142](src/PR/tengine/cart.c#L142)). `cartdir.h` holds only *logical indices*
> (`CART_ROOT_isLevels = 6`), never byte offsets. So the engine reads whatever directory sits in front of it.
>
> Two consequences, both verified against bytes:
> - **The source tree already ships the complete asset payload.** `src/PR/cartdata.dat` = **6.49 MB**
>   (root word `0x0000000b` = 11 items; 50 levels, 1035 texture sets, ~2.27 MB models). The spec currently
>   `include`s a 7,576-byte **stub** `src/PR/tengine/cartdata.dat` — swap in the real one + re-enable the
>   commented-out `makerom` rule and **v49 code+data are a matched, drift-free pair** (**Path A**).
> - **The retail ROM is self-describing too.** Its root `CIndexedSet` sits at ROM **`0x1F00`**
>   (`blockCount=11`, `offs[0]=0x38=IS_INDEX_SIZE(11)`, 11 monotonic in-bounds offsets — confirmed by `xxd`),
>   and child block counts match `cartdir.h` (persist=3, warp=2). So we can also point `m_pStaticSegment` at
>   the loaded retail blob and walk it unchanged (**Path B**). The port seam already exists: `cart.c`'s
>   `#ifdef WIN32` branches fetch blocks via `memcpy` instead of `osPiStartDma`.
>
> **The only residual risk is the *leaf-decode* layer**: if a `romstruc.h` `CROM*` struct gained/reordered a
> field between build 49 and v1.2-final, an individual asset's *bytes* could mis-parse even though location
> succeeds. Close it at M5 by extracting a few leaves (a model, a texture set, a level) and parsing them with
> the v49 structs. Also keep `ORDERBYTES` (big-endian) live on LE hosts and retain the RNC decode path.
> **"v49" itself is cosmetic** — an internal build counter compiled out under `SHIP_IT`; the tree is the
> US/English `SHIP_IT` SKU matching the `NTUE`/`0x02` header. See `docs/REFERENCES.md` and the workflow report.

---

## 2. Strategy — one codebase, two first-class targets

```
        ┌─────────────────────────────────────────────┐
        │   src/PR/tengine/**   (original game C, ~137K LOC, UNTOUCHED logic) │
        └───────────────┬─────────────────────────────┘
                        │  compiled identically for both targets
        ┌───────────────┴─────────────────────────────┐
        │   port/   (the only platform-specific layer)  │
        │  ┌────────────┬───────────────┬────────────┐ │
        │  │ libultra   │  Fast3D seam  │  asset/ROM │ │
        │  │ shims      │  (gfx_pc)     │  + endian  │ │
        │  └────────────┴───────┬───────┴────────────┘ │
        └──────────────────────┬───────────────────────┘
            PC backend ◄───────┴────────► 3DS backend
        gfx_opengl + SDL2          gfx_citro3d + libctru
        audio: SDL queue           audio: ndsp ring
        (GROUND TRUTH)             (target)
```

**Golden rules (inherited from Banjo/PD/Forsaken — honor from day one):**

- Keep `src/PR/tengine/**` game logic **unmodified** wherever possible. Platform divergence goes behind
  `#ifdef PLATFORM_PORT` / `#ifdef PLATFORM_3DS`, and ideally lives entirely in `port/`.
- **Build the PC OpenGL ground truth FIRST.** Headless capture (`glReadPixels`→PNG) before any 3DS work.
- **One depth space**: clear depth to FAR, set up a single `[0,1]` depth convention; PICA NDC is `[-1,0]`,
  GL is `[-1,1]` — normalize in the Fast3D seam, not per-backend ad hoc.
- **Endianness is the dominant bug class.** Centralize all asset byte-swaps in one file
  (`port/src/turok_swap.c`), swap *structure* (offsets/counts/headers) at the typed consumer; leave
  pixel/audio payloads big-endian and convert at upload/render time.
- **ARM11 (ARMv6K) faults on misaligned loads x86 tolerates.** Run a UBSan alignment smoke test on the PC
  build before every 3DS push. `-mno-unaligned-access` does **not** save you; use `memcpy`-style accessors.
- **3DS specifics that bit every sibling**: `__stacksize__ = 2 MB` (default 32 KB is too small);
  `svcGetSystemTick()` not `gettimeofday()`; name your VFS init to avoid colliding with libctru `fsInit`;
  GPU-visible buffers from `linearAlloc`, not `malloc`; always full power-cycle between HW tests.

---

## 3. Target directory layout (mirrors Banjo / Perfect Dark)

```
turok/
├── CLAUDE.md                 ← this file
├── baserom.us.v12.z64        ← retail ROM (asset source)
├── src/PR/tengine/**         ← original game engine (compiled as-is)
├── docs/
│   ├── REFERENCES.md         ← distilled notes on sibling ports + SM64 lineage
│   └── (per-subsystem deep dives as they're written)
├── tools/
│   ├── turok_rom.py          ← ROM header / cart-directory / RNC inspector (START HERE)
│   ├── rnc_unpack.*          ← RNC1/RNC2 decompressor (port of in-tree unpack.c)
│   └── (asset extract/pack tools, mirror banjo-kazooie/tools)
├── port/
│   ├── include/              ← platform.h, video.h, audio.h, input.h, system.h, romdata.h, turok_swap.h
│   ├── src/
│   │   ├── turok_main.c       ← host boot driver (replaces idle/main thread bring-up)
│   │   ├── os_shim.c          ← libultra shims (osCreateThread, osSpTaskLoad, osViSwapBuffer, osAiSetNextBuffer…)
│   │   ├── turok_rcp.c        ← RCP/task seam: gfx tasks → gfx_pc; audio tasks → mixer
│   │   ├── romdata.c          ← single DMA seam: piRead(dst, romAddr, n) → memcpy from ROM image (+RNC)
│   │   ├── turok_swap.c       ← endian byte-swap registry (all asset types in one place)
│   │   ├── mixer.c            ← RSP audio microcode → C (shared w/ Banjo/PD)
│   │   ├── audio.c / audio_3ds.c, input.c / input_3ds.c, sys_3ds.c, fs.c
│   │   └── backend_headless.c ← null renderer for M1 boot test
│   └── fast3d/               ← copied from banjo-kazooie/port/fast3d, adapted for F3DEX-NoN
│       ├── gfx_pc.cpp         ← F3DEX-1.x interpreter (the reference; fix bugs HERE once)
│       ├── gfx_cc.cpp/.h      ← color-combiner → TEV decoder (portable)
│       ├── gfx_opengl.cpp     ← PC backend (ground truth)
│       ├── gfx_osmesa.cpp     ← PC headless GL (frame capture, no X11)
│       ├── gfx_sdl2.cpp       ← PC window manager
│       ├── gfx_citro3d.cpp    ← 3DS backend (PICA200 TEV)
│       ├── gfx_3ds.c          ← 3DS window manager (libctru)
│       └── shaders/gfx_citro3d.v.pica
├── Makefile.port             ← PC build (gcc, -DPLATFORM_PORT, RENDER=null|gl)
└── Makefile.3ds              ← 3DS build (arm-none-eabi-gcc, -DPLATFORM_3DS)
```

**Copy, don't reinvent.** Banjo's `port/fast3d/` already handles F3DEX-1.x `G_TRI2`/`G_QUAD`/`G_MOVEWORD`;
PD's `gfx_citro3d.cpp` (record-replay, stereo, 90° rotation, texture baking) is the most mature 3DS backend.
Start from those files and adapt to Turok's exact combiner/segment usage.

---

## 4. The seams (where original N64 code meets the host)

| N64 mechanism | Original site (in `tengine/`) | Host replacement |
|---------------|-------------------------------|------------------|
| **PI/DMA cart read** | `cart.c`, `romstruc.c`, `BootTransfer` in `boot.c` | `romdata.c::piRead()` → `memcpy` from in-RAM ROM image; RNC-decompress via ported `unpack.c` |
| **RNC decompression** | `unpack.c` / `pp.h` / `huffman.c` | Compile the in-tree source natively (it's portable C) — reuse, don't rewrite |
| **RSP gfx task** (`gspF3DEX_NoN`) | `gdl.c`, `dlists.c`, `gfxyield.c`, `osSpTaskLoad` | `turok_rcp.c` dispatches the `Gfx` list synchronously through `gfx_pc.cpp` |
| **RSP audio task** | `audiomgr.c`, `audheap.c` | `mixer.c` (ADPCM decode + resample + envelope) — shared with Banjo/PD |
| **VI (frame present)** | `osViSwapBuffer`, `cfb.c` | `videoEndFrame()` once per host frame |
| **AI (audio out)** | `osAiSetNextBuffer` | SDL2 queue (PC) / ndsp ring (3DS) |
| **Threads / scheduler** | `tengine.c`, `sched.c`, `osCreateThread` | `os_shim.c` cooperative shims; main loop drives one frame per iteration |
| **Controller** | `control.c`, `tcontrol.c`, `osContRead` | SDL gamepad (PC) / libctru hid (3DS) |

---

## 5. Milestone roadmap (lockstep with sibling-project numbering)

- **M0 — Compiles. ✅ DONE (2026-06-12): 77/78 TUs compile clean.** `make -f Makefile.port` against the
  vendored portable libultra (`lib/ultralib`, from banjo-kazooie). 2,261 functions compiled; `boot`/`mainproc`
  present. It was far easier than expected — the N64 path is plain C, the only real dependency is
  `<ultra64.h>` (everything else is host stdlib or behind `WIN32`/PSX `#ifdef`s). Build `-m32` (N64 pointer
  width). The one deferred file is **`coll.c`** (stale vs the current `CCollisionInfo2` layout — old monolithic
  collision; not on the M0→M2 path, fix at M5). See "Port edits to game source" below.
- **M1 — Boots headless. ✅ DONE (2026-06-13).** `tools/build_port.sh` → `/tmp/turok_build/turok` runs
  **600 frames clean, rc=0, zero crashes** (release AND debug). The boot chain now runs stably: OS init →
  audio deferred → full cartridge asset load (endianness-correct) → legal-screen transitional state →
  steady frame loop → clean exit via the frame-limit longjmp. Each crash along the way was a distinct,
  fixable issue:
  - ✅ links (all 120 externs); ✅ cooperative driver (`boot()`→`mainproc()`, idle bypassed, frame pump
    synthesizes `OS_SC_RETRACE_MSG`); ✅ loads `cartdata.dat` + **validates 11-item cart root at runtime (Path A)**.
  - ✅ **ENDIANNESS — cart directory.** `ORDERBYTES` (cart.h) was a no-op off-N64; now a **type-generic swap**
    (size-based `__builtin_choose_expr` → union swap for 4/2-byte scalars, aggregates like `CVector3` pass
    through — layering-free, no `CVector3` type needed in cart.h). Cart directory DMAs now correct (56-byte
    root index, real asset blocks).
  - ✅ **Audio deferred (M4).** `initAudio` computed DMA sizes from dummy audio-segment symbols (multi-GB →
    crash); now the `_seqctl/_sfxctl` Start/End symbols are **zero-span aliases** (`turok_runtime.c`), and
    `audio.c` early-returns before `amCreateAudioMgr` under `PLATFORM_PORT` (keeps the harmless AW-globals init).
  - ✅ **ENDIANNESS — asset data.** `scene.c CScene__PersistantCountsReceived`: `pickupCounts[i]`/`animCounts[i]`/
    `*pMaxRegionCount` were read RAW (big-endian) → wrapped in `ORDERBYTES`. Full cart/scene asset load now
    completes (11 DMAs: persistant counts, levels, …).
  - ✅ **N64 null-tolerance guards.** The legal/title screens load no level, so there's no player — but the game
    runs its per-frame update (player/weapon/camera) which derefs the NULL player. On N64 (no MMU) this reads
    low RDRAM harmlessly; a protected host faults (and we **can't** map page 0 — `vm.mmap_min_addr=65536`).
    Guarded under `PLATFORM_PORT`: `CTMove__NewLifeSetup` (skip `SetHealth` if no player), `AI_Update_Turok_Weapon`
    (early-return if `pMe==NULL`), `CCamera__Update` (skip whole camera update if no player). Expect a few more
    such guards in the title/attract phase; gate at the subsystem-update level, not per-deref.

- **M2 — PC-GL renders (first pixels). 🔶 IN PROGRESS (2026-06-13): pipeline wired + interpreter validated;
  geometry not yet visible.** What works:
  - ✅ **Full render pipeline wired end-to-end.** Copied `banjo-kazooie/port/fast3d/` → `port/fast3d/`; bridge
    `port/src/turok_gfx.c` drives `gfx_init`/`gfx_run`/`gfx_start_frame`/`gfx_end_frame` over **OSMesa** (headless)
    + the **OpenGL** rendering API. Built via `tools/build_port.sh` (g++ `-std=gnu++20`, `-DGFX_USE_OSMESA`,
    links `-l:libOSMesa.so.8 -l:libGL.so.1`). `TUROK_CAPTURE_FRAME=N TUROK_CAPTURE_PATH=x.png` → PNG.
  - ✅ **The cooperative scheduler seam.** The game submits frames via `scSendCommand` → the scheduler thread
    (which never runs cooperatively). Patched `sched.c scSendCommand` (`PLATFORM_PORT`) to dispatch the gfx task
    directly: `osSpTaskStartGo` → `os_shim` → `turokGfxRun` → `gfx_run`. `osViSwapBuffer` →
    `turokGfxEndFrame`(present)+capture+`turokGfxStartFrame`.
  - ✅ **The F3DEX-NoN interpreter WORKS for Turok.** `TUROK_GFX_DUMP=1` shows gfx_pc walking a valid command
    stream: `G_DL`, `G_MOVEMEM`, `G_SETOTHERMODE_H/L`, `G_SETSCISSOR`, `G_SETCIMAGE`, `G_FILLRECT`, `G_VTX`
    (vertices), `G_TEXRECT`/`G_LOADBLOCK`/`G_SETTILE`/`G_SETTIMG`, `G_ENDDL` — all valid opcodes, valid host-pointer
    addresses, segment pass-through correct. (Display-list addresses are host pointers; `seg_addr` passes them
    through since only segment 0 is set.)
  - ✅ **Symbol-collision fix:** the game's own 5-arg `qsort` interposed on libc's (Mesa calls libc qsort →
    crash); renamed via `#define qsort turok_qsort` (game-only, in `turok_port.h`).
  - ✅ **Capture-path bug fixed (key insight).** `osViSwapBuffer` (real present) AND the `osRecvMesg` frame-pump
    BOTH called `turokVideoSwap`, so `TUROK_CAPTURE_FRAME=N` (keyed to the inflated `g_frame`) captured a
    frame-pump tick (blank), not a rendered frame. **Capture now keys off REAL render frames** (`s_frame_no` in
    `turokGfxEndFrame`, after `gfx_end_frame`), and exits right after. So `TUROK_CAPTURE_FRAME=N` = the Nth real
    present. **Render frame 0 = a clear/empty present (0 tri1 calls).** The content (legal screen) is render
    frame 1+.
  - 🔶 **FRONTIER: software OSMesa is too slow (~minutes/content-frame) to visually confirm pixels.** Command
    *interpretation* is fast (churns 200k cmds in ~20s); the bottleneck is software-GL *rasterization* of the
    legal screen's many textured draws (the logo also fades in over dozens of frames, so early content frames are
    near-blank anyway). Reaching even render frame 1 took >5 min. The pipeline is validated; this is purely a
    rendering-speed wall.
  - **EGL hardware-GL backend — WRITTEN, compiles, but 🔶 HANGS AT STARTUP (WIP).** Built it all:
    `port/fast3d/gfx_egl.cpp` + `gfx_egl.h` (surfaceless EGL `EGL_PLATFORM_SURFACELESS_MESA` + an FBO render
    target + `glReadPixels`→PNG), a `GFX_USE_EGL` branch in `gfx_opengl.cpp` (glad via `eglGetProcAddress`; SDL-skip
    widened to EGL), `turok_gfx.c` switchable (`TUROK_WM`/`TUROK_SAVEPNG`), and `tools/build_port.sh` takes
    `GFX=egl|osmesa` (compiles `gfx_egl`, links `-l:libEGL.so.1`). It **compiles clean** but the EGL binary
    **hangs before `main`'s first print** (zero output, process never exits) — i.e. in EGL/GPU library init at
    load/`eglInitialize`/surfaceless-context time, NOT in our code. **`GFX=osmesa` is the working default.**
    Next on EGL: try a **GBM device** path (`gbm_create_device(open("/dev/dri/renderD128"))` + `eglGetPlatformDisplay
    (EGL_PLATFORM_GBM_KHR, ...)`) instead of surfaceless; or run under `strace`/a pre-main breakpoint to find the
    hang; confirm 32-bit GL drivers exist for the render node (the hang may be a missing/mismatched 32-bit GPU
    driver — a 64-bit build might "just work"). NFS caveat: switching `GFX` needs a fresh `TUROK_OUT` dir
    (`export TUROK_OUT=/tmp/tb_x`) — stale `.o`s in the default obj dir don't get cleaned on NFS.
  - **ALTERNATIVE to chasing render speed: force warp-to-level** (set `m_WarpID` to a real level in
    `MODE_RESETGAME` instead of `LEGALSCREEN_WARP_ID`) for real 3D geometry — opens the level-data endianness
    work (M5) but gets actual gameplay rendering. Even under software OSMesa a 3D level may be fewer draws than
    the texrect-heavy legal screen.
  - Debug aids in place: `TUROK_GFX_DUMP=1` (per-render-frame tri counts: `g_turok_tris`/`g_turok_tri_calls`/
    `g_turok_tri_cliprej`, + first-90 opcode dump). Capture (now correct): `TUROK_CAPTURE_FRAME=N` = Nth real
    render present, written by `turokGfxEndFrame`, exits after.
  - **Remaining systematic work for levels (M5):** the big-endian **asset-data** layer — count/offset/pointer
    values inside every asset block (objects/textures/levels/collision/instances) are big-endian, read raw, and
    each needs `ORDERBYTES` (as done for persistant counts). Do it parser-by-parser. Once a real level loads,
    the player exists and the null-tolerance guards above become inert.
- **M2 — PC-GL renders the title screen.** Fast3D interprets the first display lists; `gfx_opengl.cpp`
  draws; `TUROK_CAPTURE_FRAME=0 → PNG`. **This is the ground-truth checkpoint** — nothing 3DS until green.
- **M3 — 3DS renders the title.** `gfx_citro3d.cpp` on hardware / in Mandarine. Heap split, stack size,
  rotation, depth remap all wired.
- **M4 — Audio.** `mixer.c` + the n_audio-style synth: music + SFX on both targets.
- **M5 — First level loads + is walkable.** Wire the cart cache (`CCartCache`/`CIndexedSet`) over the host
  ROM seam (extend the existing `#ifdef WIN32` memcpy path in `cart.c`), keep `ORDERBYTES` + RNC decode, and
  **close the leaf-decode risk**: extract a model / texture set / level and parse with v49 `romstruc.h` structs.
  Prefer **Path A** (rebuild from the tree's own `src/PR/cartdata.dat`, a drift-free matched pair) for the
  first playable; **Path B** (walk the retail ROM's directory at `0x1F00`) when v1.2-exact content is wanted.
- **M6 — Combiner fidelity.** Fog, particles (`particle.c`, `fx.c`), framebuffer effects, sun, HUD.
- **M7 — 3DS memory tuning + production flags.** Old-3DS budget, texture format choices, ETC1 where it helps.

Track milestone state in memory (`darkengine_3ds_port`-style note) so it survives context resets.

---

## 6. ROM & asset tooling plan

Mirror `banjo-kazooie/tools/`. Turok's compression is **RNC**, and we already have the reference
decompressor in `src/PR/tengine/unpack.c`. First tools:

1. **`tools/turok_rom.py`** (start here): parse the ROM header, walk the cart directory, locate + list
   RNC blocks, dump a manifest. (Initial version written this session.)
2. **`rnc_unpack`**: standalone RNC1/RNC2 decoder — port `unpack.c`/`huffman.c` to host C or Python.
3. **Asset extractor**: ROM → typed asset bins (textures, models, levels, banks), keyed off the structures
   in `romstruc.c` / `cart.c` / `mattable.c`.
4. **Endian/swap generator**: enumerate asset structs to drive `turok_swap.c` (PD/Banjo do this by hand;
   consider generating from the headers).

Keep a decompressed working ROM image cached on disk; the runtime `romdata.c` reads from it (SD card on 3DS:
`sdmc:/3ds/turok/baserom.us.v12.z64`, like Banjo/PD).

---

## 7. Reference projects (the rosetta stones) — see `docs/REFERENCES.md` for detail

> 📘 **Starting a NEW N64→PC/3DS port?** Read [`docs/N64_PORTING_PLAYBOOK.md`](docs/N64_PORTING_PLAYBOOK.md)
> first — the generalizable, cross-cutting findings distilled from this port (endianness traps incl. the
> lookup-table/key-table/aggregate gotchas, N64 null-tolerance, streaming re-acquire-by-position, the
> present-pipeline blocker, frame-pacing/tick-decouple/interpolation, the `fmodf` implicit-decl ABI trap, the
> memcpy-guard + state-trace debug methodology, headless EGL/GBM, and the shared `port/` layout & milestones).

| Project | Path | Borrow this |
|---------|------|-------------|
| **Banjo-Kazooie** | `/mnt/nas/Development/banjo-kazooie` | **Closest match.** F3DEX-1.x `gfx_pc.cpp`; entire `port/` skeleton; `Makefile.port`/`Makefile.3ds`; byteswap registry; mixer; milestone discipline. |
| **Perfect Dark** | `/mnt/nas/Development/perfect_dark` | Most mature **3DS Citro3D backend**: record-replay, single-pass stereo, 90° rotation, viewport-as-clip-rect gotcha, texture baking for aniso-less PICA. `Makefile.3ds`. |
| **Forsaken** | `/mnt/nas/Development/forsaken` | The "keep N backends side-by-side" discipline; ARM unaligned-access catalogue (34+ sites); linear-vs-BSS heap strategy; ETC1 texture budgeting; shipped-on-3DS lessons. |
| **N64Recomp** | `/mnt/nas/Development/N64Recomp` | Static-recomp **alternative** (fast PC port, citro3d at `3ds_project/source/renderer/renderer_3ds.c`). Useful as a cross-check oracle, *not* our main path — we have real source. |
| **SM64 ports** (GitHub) | *not local* — `sm64-port`, `sm64ex`, `sm64_3ds` (masterfeizz) | **Ancestral origin** of the entire Fast3D→GL and Fast3D→Citro3D mapping that Banjo & PD descend from. SM64 uses plain **F3D**; clone when we need the canonical citro3d combiner mapping or to diff opcode coverage. |

**Why we have it easy vs the siblings**: they reverse-engineered C from a binary; we start from the
original source. The risk shifts from "does it match the ROM byte-for-byte" to "does the source build
clean off-IRIX and do its asset offsets line up with the retail ROM" (the M5 drift question).

---

## 8. Build & run quick reference (to be filled in as Makefiles land)

```bash
# PC — logic only (fast, no GL):    make -f Makefile.port RENDER=null -j$(nproc)
# PC — with OpenGL:                 make -f Makefile.port RENDER=gl  -j$(nproc)
# PC — bounded deterministic run:   TUROK_MAX_FRAMES=600 ./build_port/turok
# PC — frame capture (ground truth):TUROK_CAPTURE_FRAME=0 TUROK_CAPTURE_PATH=t.png ./build_port/turok
# 3DS:                              make -f Makefile.3ds -j$(nproc)   →  build_3ds/turok.3dsx
# 3DS emulator:                     mandarine build_3ds/turok.3dsx
```

devkitPro is installed at `/opt/devkitpro` (devkitARM + libctru + citro3d + picasso). Toolchain matches
what the sibling 3DS builds already use.

---

## 9. Conventions

- Platform gates: `PLATFORM_PORT` (any host), `PLATFORM_3DS` (adds 3DS-only). Never `#ifdef`-fork game logic
  when an endian/alignment accessor in `port/` will do.
- All endian swaps in `port/src/turok_swap.c`; one `turokSwap<Type>()` per asset type.
- The single ROM read seam is `piRead()` in `romdata.c` — platform-agnostic signature.
- Fix Fast3D decode/state bugs **once** in `gfx_pc.cpp`; backends only differ in how they realize state.
- After any `-D` flag change, clean-build (objects are keyed by defines).
- Update this file and `docs/` as findings land — it is the project's memory across sessions.

---

## 11. Build & debug recipe (READ THIS — `make` is unreliable here)

The repo is on an **NFS mount with unreliable timestamps**, so `make` skips rebuilds of changed files and
links **stale objects** (a `cart.h` edit silently won't take). Always build with the script, which compiles
every TU fresh to a **local** dir:

```bash
tools/build_port.sh            # -> /tmp/turok_build/turok   (release, -O1)
tools/build_port.sh debug      # -O0 + memcpy/SIGSEGV guard linked in (for crash diagnosis)
TUROK_MAX_FRAMES=8 /tmp/turok_build/turok      # run headless, bounded
```

**Debugging methodology that WORKS here** (gdb can't unwind through the asm `memcpy`; 32-bit ASan aborts in
its own init and prints nothing):
- `port/src/romdata.c` logs every DMA `(dst, devAddr, nbytes)` and bounds-checks vs the cartdata buffer —
  catches bad cart reads (wrong size/offset = an endianness or parse bug).
- `port/tools/memcpy_guard.c` (linked in `debug`/`asan` builds via `-Wl,--wrap`) records the last
  memcpy/bcopy and installs a **SIGSEGV handler** that prints the fault address + last copy + a short
  backtrace. Then map frames with `addr2line -f -e /tmp/turok_build/turok 0x<addr>`.
- Pattern: run → read the SIGSEGV fault addr + backtrace → `addr2line` the game frame → inspect that source →
  fix (usually a missing `ORDERBYTES` on a big-endian asset value) → rebuild → repeat. Each cycle advances the
  boot one crash further.

### 11.1 ★ THE RALPH WIGGUM LOOP — autonomous 3DS boot bring-up (launch → trace → fix → repeat)

> *"I'm helping!"* — a deliberately dumb, persistent loop: keep launching, read where it died, fix that one
> thing, launch again. Each turn advances the boot exactly one hang/crash further. Run it AUTONOMOUSLY until
> the game reaches its loop / renders a frame / plays audio — don't stop to ask between iterations.

**The loop (one iteration):**
1. **Build** the change: `make -f Makefile.3ds` → `build_3ds/turok.3dsx` (the 3DS make is NOT NFS-stale like the
   PC one; it's fine. After a PC-shared file also `tools/build_port.sh` to confirm PC still builds.)
2. **Health-check Mandarine** (it WEDGES after ~5-10 rapid launches — the #1 time-sink): `pkill -9 -f
   'AppRun.wrapped'; pkill -9 -f '\.mount_mandar'` (the AppImage child is `AppRun.wrapped`, NOT matched by
   `pkill mandarine`), then a **~40 s cooldown** before relaunch. NEVER `rm -rf /tmp/.mount_mandar*` while one is
   live — it corrupts the AppImage mount and Mandarine then exits-on-launch (empty log) until a long recovery.
3. **Launch + trace.** Two channels, pick per need:
   - **boot.log (FAST, default):** gdbstub OFF (`~/.config/mandarine-emu/qt-config.ini use_gdbstub=false`),
     `DISPLAY=:1 ~/Desktop/citra/mandarine.AppImage build_3ds/turok.3dsx`, wait ~30 s, read
     `~/.local/share/mandarine-emu/sdmc/3ds/turok/boot.log`. **The LAST line = where it hung/crashed** (written by
     `plat3dsBootLog`, aka the `BL("…")` macro in turok_main.c, per-line fflush). To narrow, add more `BL("…")`
     trace points around the suspect span and rebuild. (Stage the ROM at `…/sdmc/3ds/turok/baserom.us.v12.z64`.)
   - **gdb (RELIABLE pin, slower):** gdbstub ON; harness `/tmp/boot_probe.sh <gdb-cmds> <elf> <3dsx>` (polls
     `:24689`, runs `gdb-multiarch -batch` timeout-bounded — a hung `continue` ⇒ rc 124, last printed breakpoint
     = the hang boundary; `[New Thread N]` = a thread was created). Use when boot.log is too coarse or Mandarine's
     no-gdb path is being flaky. ★ GOTCHA: needs `use_gdbstub=true` AND `use_gdbstub\default=false` (a reboot
     resets `\default=true`, which makes Mandarine IGNORE the value → port never opens). NB gdb's `continue` runs
     ASYNC on Mandarine's stub and a guest data-abort is NOT forwarded as a signal — so gdb is good for HANGS, bad
     for CRASHES (use the log file below for crashes).
   - **★ THE MANDARINE LOG FILE (BEST for a CRASH — gives the faulting PC, no gdb):** with `log_filter=*:Trace`
     (qt-config.ini), Mandarine writes `~/.local/share/mandarine-emu/log/mandarine_log.txt`. A guest wild/NULL
     deref logs `HW.Memory <Error> ... unmapped ReadNN @ 0x<addr> at PC 0x<pc>` — `addr2line -e build_3ds/turok.elf
     0x<pc>` gives the function. Tells you the bad address class too: `@ 0xEA00xxxx`/garbage = a wild pointer
     (endianness/relocation); `@ 0x0000000C/0xE` = a NULL+offset struct-field deref (N64 null-tolerance — Mandarine
     RETURNS 0 and continues, so a one-shot is "tolerated", but on real HW it data-aborts → guard it). A repeated
     same-PC read = a SPIN. `: > mandarine_log.txt` before each run so the trace is fresh.
4. **Diagnose + fix the ONE thing**, then go to 1. The recurring 3DS hang/crash classes (fix-and-advance):
   - **hosted-libc shadow** — the game ships its own `memset`/`memcpy`/etc. (memory.c) that statically SHADOWS
     newlib's, and libctru calls it pre-`main` → hang. Gate the override out on `PLATFORM_3DS`. (THE pre-main fix,
     commit `e1a6b47`. Detect by intersecting game symbols vs BOTH `libctru.a` AND `libc.a`.)
   - **emulator-unsupported blocking call** — e.g. `ndspInit()` BLOCKS in Mandarine without `dspfirm.cdc` instead
     of returning an error → gate it behind a flag / skip (commit `27f2932`, `audio_3ds` cfg).
   - **ARM byte-alignment fault** (ARMv6K) — a `vldr`/`ldrd`/`ldm` on a misaligned byte-parsed buffer → the
     `turok_align.h` accessors (see the alignment sweep). A data-abort shows in the Luma crash dump / a gdb SIGSEGV.
   - **N64 null-tolerance / endianness** — same classes the PC boot hit; fix the same way (guards / `ORDERBYTES`).
5. **Audio verification (the "pipe the audio" tooling).** The classic-ABI synth+mixer is PC-validated; to hear/
   measure SFX+music, capture the game's synthesized PCM to a WAV and analyze it (no speakers needed):
   `TUROK_FPS=30 TUROK_FAKEINPUT=5 TUROK_AUDIO_WAV=/tmp/a.wav TUROK_MUSIC=1 <pc-build>` + frames = seconds×30
   (the headless audio recipe), then inspect peak / %-active / dominant frequency. On the **3DS** path, audio is
   gated off until `ndspInit` works (the DSP-firmware item); once on, capture Mandarine's PulseAudio out
   (`parec`/`pacat`) to a WAV and analyze the same way. Use audio as a liveness signal too (silence vs. a tone vs.
   real SFX tells you how far the synth got).

**Stop conditions:** the game reaches `mainproc`/its frame loop, renders a non-black frame (capture the
Mandarine top-FB via `plat3dsCaptureTopFB` / a screenshot), or a genuine product decision / a wedged emulator
that needs a human (e.g. dumping `dspfirm.cdc`, a Mandarine restart). Otherwise: keep looping.

## 10. Port edits to game source (keep this log honest)

We own this source outright (no IDO byte-matching build to preserve), so light, documented edits to the
game files are acceptable. Keep them minimal and listed here so they're reviewable:

- **★★ ARM11 (3DS) BYTE-ALIGNMENT SWEEP — found+eliminated, verified on the real ARM binary (2026-06-21,
  commit `fee2e4e`, 10-agent Workflow + objdump/UBSan).** N64 assets are big-endian blobs streamed into byte
  buffers then cast to typed structs and read field-by-field. x86 tolerates the misaligned loads; **ARM11
  (ARMv6K) FAULTS** on the strict forms modern devkitARM GCC emits: **`vldr`** (float), **`ldrd`/`strd`** (8-byte),
  **`ldm`/`stm`** (GCC's fusion of a ≥12-byte struct copy). A single integer **`ldr`/`ldrh` is unaligned-tolerant**
  (SCTLR.U=1, 3DS default), so a u32/u16 read of a misaligned buffer does NOT fault — **the fault surface is
  exactly a FLOAT/struct read whose base traces to a byte-parsed asset buffer.** (This is why UBSan-on-x86
  under-reports: x86 buffers land 4-aligned at runtime and the faulting forms only exist in ARM codegen.)
  **★ THE AUTHORITATIVE TOOL is `arm-none-eabi-objdump -d build_3ds/turok.elf --disassemble=<fn>`** grepped for
  the strict forms with a non-stack base; UBSan (`tools/build_port.sh ubsan`, `-fsanitize=alignment`) is the x86
  cross-check (its FLOAT findings map to `vldr`). **`port/include/turok_align.h`** (new, force-included via
  turok_port.h) provides the fix accessors — `turok_rd_f32/u32/u16/s16/s32` (route a buffer read through an
  integer `ldr` into an aligned local; ARM-verified to emit `ldr`, never `vldr`) + a **noinline**
  `turok_memcpy_unaligned` (byte loop GCC can't re-fuse to `ldm`). **Codegen-neutral on x86** (compile to a plain
  `mov`), so a single fix is correct on PC+3DS — verified by **pixel-IDENTICAL** PC capture vs baseline.
  - **★ SYSTEMIC (would abort at boot): `memory.c i3D_mallocPool`** — GCC fused the `next`(+4)/`addr`(+8) field
    stores into one `strd` at a 4-mod-8 address → a **data abort on EVERY pool allocation** on ARM11. Broke the
    fusion with a compiler barrier (`PLATFORM_3DS`-gated). This one alone would have made the 3DS build
    unrunnable the instant it reached `main`.
  - **Asset/collision parsers** (float-only via accessor; integer reads left as safe `ldr`; struct copies via
    `turok_memcpy_unaligned`; all `PLATFORM_PORT`/`3DS`-gated, **ORDERBYTES byte-swap preserved**):
    `romstruc.c` (GetGroundNormal corner CVector3 `ldm`, CalculateOrientationMatrix CROMBounds `ldrd`),
    `scene.c` (WarpPointsReceived warp-point struct copy `ldm`), `geometry.c` (m_cMorph float RMW),
    `anim.c` (DecompressAnim m_Scale/m_vOffset), `particle.c` (impact m_ImpactEventNumber, 3 sites), and the
    **collision family** `unicol.c`/`regicol.c`/`wallcoll.c`/`wradcol.c`/`map.c` (region-corner CVector3 reads
    from the streamed collision buffer — the dominant per-frame buffer-float source), plus the `unpack.c:232`
    RNC ULONG read (a safe `ldr`, wrapped anyway for a clean UBSan run).
  - **KEY INSIGHT (low-churn rule): `ORDERBYTES` already protects most CROM\* reads** — its union-bswap forces an
    integer `ldr`+`rev` (never `vldr`), so the big named decoders (TakeFromROMObjectInstance, Simple/Static)
    needed NO change; their "strict" instrs are writes to the aligned pool-allocated `pThis`. Only GCC's
    **aggregate-copy fusion** (`ldm`/`ldrd`/`strd`) and the **raw float reads NOT wrapped in ORDERBYTES** drop
    below the 4/8 threshold from a buffer — those are the real bugs.
  - **Audio path confirmed alignment-clean** (one agent rate-limited; I audited it): `turokBnkfNew`/
    `turokCSeqHeaderSwap`/`turokAudioLoadBankFromROM` read banks/headers as integers/offsets only; `alCSeqNew`
    (untracked `cseq.c`) reads the seq buffer only as `u32` (trackOffset/division → `ldr`) and writes floats to
    the aligned ALCSeq object.
  - **Verified:** clean 3DS relink; objdump confirms zero buffer-base `vldr`/`ldrd`/`ldm` remain in every fixed
    function (remaining strict instrs are stack spills or aligned static/heap bases); PC **UBSan run = ZERO
    misaligned findings**; render byte-IDENTICAL to baseline (warp 0 + 6000); patrol+audio+music rc=0, 0
    `[CAMTRACK]`/`[CANARY]` anomalies across warps 0/2000/3000/6000/8000. **LESSON: on ARM11 the fault surface is
    float/struct reads from byte-parsed buffers — find them by disassembling the ACTUAL ARM elf for `vldr`/`ldrd`/
    `ldm` with a non-stack base, not by UBSan-on-x86 (which can't see ARM codegen) nor by fixing every misaligned
    integer read (those are safe `ldr`). And watch the allocator: a fused `strd` on a 4-mod-8 node field aborts
    every malloc.** (This is necessary groundwork for 3DS hardware; it does NOT fix the separate pre-`main`
    libctru `srvInit` boot hang, which is still user-gated via Mandarine.)

- **`src/PR/tengine/pp.h`** — widened the RNC-types guard from `#ifdef WIN32` to
  `#if defined(WIN32) || defined(PLATFORM_PORT)` so the runtime RNC decompressor (`unpack.c`/`huffman.c`,
  method-2 = Huffman) builds on the host. (We need it to decode the cart's RNC assets.)
- **`NULL`-as-integer** (10 sites) → literal `0`: `campaign.c` (7 boss-table flag fields), `trex.c` (1),
  `pause.c` + `loadsave.c` (`case NULL:` → `case 0:`). IDO made `NULL==0`; modern `<stddef.h>` makes it
  `((void*)0)`, illegal in those integer contexts. A force-include can't fix it (libultra re-`#define`s NULL
  after us), so the sites were edited. Faithful — these were always integer-0 uses.
- **`cart.h`** — `ORDERBYTES` for `PLATFORM_PORT` is now a type-generic size-based byte-swap (was identity).
- **`audio.c`** — `initAudio` early-returns under `PLATFORM_PORT` before `amCreateAudioMgr` (audio → M4).
- **`scene.c`** — `CScene__PersistantCountsReceived`: wrapped `pickupCounts[i]`/`animCounts[i]`/`*pMaxRegionCount`
  reads in `ORDERBYTES` (big-endian asset data). More such per-parser fixes will follow as the boot advances.
  (Also has a `PLATFORM_PORT` `fprintf` trace at the `m_pPlayer` set — harmless, can stay or be removed.)
- **`cart.c`** — `PLATFORM_PORT` `fprintf` trace of each `RequestBlock` description (debug aid for the load chain).
- **N64 null-tolerance guards** (`PLATFORM_PORT`): `tmove.c CTMove__NewLifeSetup`, `aiweap.c AI_Update_Turok_Weapon`,
  `camera.c CCamera__Update` — all skip player-dependent work when `GetPlayer()` is NULL (legal/title screens).
- **`frontend.c`** — `CLegalScreen__Update` early-returns under `PLATFORM_PORT` to **freeze on the legal screen**.
  The cooperative frame-pump has no real frame timing, so `LegalScreen.m_Time -= frame_increment` reaches 0 in
  ~1 frame and cascades the entire intro sequence (legal→Acclaim logo→Iggy→…→attract), each step a full scene
  reload — the game spins re-loading the cartridge and never settles to present. Freezing holds it in `MODE_GAME`
  on the legal screen (**confirmed**: cartridge reloads dropped from every-~3-frames to 2). Remove once real
  frame-rate pacing + controller input drive the intro. (Pairs with the os_shim 60 Hz pacing below.)
- **`port/src/os_shim.c`** (port infra, not game source) — added **60 Hz frame pacing** in the `osRecvMesg`
  frame-pump (`TUROK_FPS` env, default 60, 0 = unbounded). Without it the intro/attract state machine advances
  by `frame_increment` per pump tick at unbounded speed → constant scene reloads → CPU peg, never presents.
- **★ LOGIC-TICK / RENDER DECOUPLE (2026-06-16, commit 4aff8df) — fixes the game running ~2x too fast.** Turok's
  `frame_increment` is sized for a **30fps step** (`CALC_FRAMERATE` floors `nNextFields` at 2 → `frame_increment
  ≥ 1.0`, tengine.c:4710), but the port renders at 60fps, so it applied a 30fps step 60×/sec = ~2× speed. Fix:
  render at `TUROK_FPS` (60) but advance the LOGIC only at `TUROK_TICK_FPS` (default 30 = Turok's native rate).
  **`os_shim.c osViSwapBuffer`** (THE per-frame present — NOT the `osRecvMesg` BLOCK branch, which is only hit
  per-frame in SDL2, not headless EGL where sched.c→osViSwapBuffer drives frames) runs a real-time clock and sets
  the global `g_turok_logic_tick=1` only when ~1/TICK_FPS sec has elapsed; **`tengine.c CEngineApp__UpdateGAME`**
  forces `frame_increment=0` on the other (render-only) frames so they just re-present the same state. Verified
  by counting ticks over a fixed real time: TICK_FPS=30→~30Hz logic, 60→60Hz, 0→every frame (old too-fast), while
  the render ran 2700fps headless. `play_level.sh` exposes it as `TICK=<n>` (default 30). NOTE: no interpolation,
  so visuals update at 30Hz on a 60Hz display (the N64-authentic cadence) — smooth 60fps *content* would need
  position/anim interpolation between ticks (a later feature). **LESSON: the per-frame hook headless is
  `osViSwapBuffer` (sched.c drives it), not the `osRecvMesg` BLOCK frame-pump (SDL2 only).**
- **★ RENDER INTERPOLATION — PLAYER (2026-06-16, commit 756cb54) — smooth 60fps motion from 30Hz logic.** The
  tick decouple fixed the speed but left motion at 30Hz (choppy on 60Hz). `os_shim.c turok_render_alpha()` returns
  0..1 (real-time progress from the last logic tick to the next, from the file-scope `g_tick_last`/`g_tick_interval_ns`).
  `tengine.c CEngineApp__UpdateGAME` snapshots the player's true `m_vPos`+`m_RotY` on logic-tick frames (`_ip*`
  statics) and, every frame, renders the player at `lerp(prev,cur,alpha)` **before `SetCameraToTurok`/
  `CreateGraphicsTask`** (so the camera that follows the player + the 1st-person weapon are smooth), then
  **restores the exact logic pos right after `SendGraphicsTask`** so the next tick is exact + gameplay is
  unaffected. A >1000-unit inter-tick jump (warp/respawn) SNAPS not slides; yaw lerps the short way around the
  2π wrap. Verified: `renderZ=lerp(prevZ,curZ,alpha)` sweeps ~40 intermediate positions per ~12.8-unit logic step;
  patrols rc=0, warp-0 renders 171 colour buckets. **OPEN follow-ups: enemy/object instances are NOT interpolated
  yet (they judder at 30Hz — the player/camera/weapon, the dominant FPS view, ARE smooth); look-PITCH not yet
  interpolated (only pos+yaw).** ★ GOTCHA: with the 30Hz gate, headless captures need a LATE `TUROK_CAPTURE_FRAME`
  (the unpaced render runs ~2700fps so render-frame 20 is <1 logic tick in → blank; use frame ~4000+).
- **★ BRANCH `framerate-interpolation` (2026-06-16).** Per the user, ALL framerate/feel work (tick decouple,
  player + camera/pitch interpolation, render uncap) lives on the **`framerate-interpolation`** branch; **`master`
  = correctness fixes only** (ends at the re-acquire key-cinematic fix `ce40c14`, force-pushed back — master runs
  gameplay-correct but at the old 2x render speed). Merge once the feel is dialed in. The decouple/interp commits
  (4aff8df, 756cb54, 18d9c68) are on the branch, NOT master.
- **★ CAMERA INTERPOLATION + RENDER UNCAP (branch, commit 18d9c68).** A 3-agent Workflow mapped the camera
  view-matrix (`CCamera__Update`: view = m_XPos/YPos/ZPos + m_RotY yaw + **m_RotXOffset pitch** + m_qGround), the
  SDL2 present (v-sync ON `SwapInterval(1)` + a `target_fps=120` CPU timer), and the look-pitch field
  (`m_RotXOffset` on CEngineApp, set by `CTMove` tmove.c:919). Added: (1) **pitch interpolation** —
  `tengine.c CEngineApp__UpdateGAME` snapshots/lerps/restores `pThis->m_RotXOffset` alongside player pos+yaw so
  looking up/down is smooth. (2) **render uncap** — `gfx_sdl2.cpp` honours TUROK_FPS for `target_fps` (FPS=0 →
  timer off → render limited only by v-sync = monitor refresh, e.g. 144Hz). (3) **play_level.sh defaults TICK=60
  + FPS=0** (60Hz logic, uncapped render, interpolation fills the gap). qGround (ground slope) NOT yet slerp'd —
  gradual, low judder, follow-up. Enemy/object instances still 30/60Hz (only player/camera/weapon interpolated).
- **★ CAMERA "FEELS LIKE 30" ON A 60Hz MONITOR — tick-clock BEAT + non-interpolated camera state (2026-06-16).**
  User: turning (hold A/D) feels like 30, not buttery; TICK=30 looked smoother (they thought "only because it's
  slower"). Two root causes (3-agent Workflow + the tick-clock read): (1) **THE BEAT.** `os_shim.c`'s logic-tick
  clock reset its phase to `now` on every fire (`g_tick_last = now`). When the tick interval == the v-sync frame
  time (TICK_FPS == monitor Hz, e.g. 60 on a 60Hz panel) timing jitter made `elapsed >= interval` pass-or-fail
  unpredictably → randomly skipped ticks = a 30Hz-feeling beat. TICK=30 was smoother because 33ms ticking is
  *stable* against 60Hz v-sync (clearly fires every other present), NOT because of the slower speed — the user's
  speed intuition was a red herring. **Fix: phase-accumulate** — advance `g_tick_last` by exactly one interval
  per fire (self-correcting cadence), with a snap-forward cap so a slow frame doesn't bank a fast-forward backlog.
  (2) **Non-interpolated view inputs.** The view matrix also reads `m_qGround` (ground slope), `m_RotYOffset`
  (head yaw) and `m_RotZOffset` (head roll) — all updated per tick but never interpolated → snapping during
  turns/on slopes. Now interpolated in `tengine.c CEngineApp__UpdateGAME` alongside pos/yaw/pitch: head offsets
  angle-wrap-lerp, qGround shortest-path **nlerp** (re-normalized), restored to exact tick values after the
  graphics task. Verified: egl+sdl2 build, all warps rc=0 at TICK=30/60, no `[CAMTRACK]` anomalies, tris
  unchanged. **LESSON: a fixed-timestep tick clock MUST phase-accumulate (advance by interval), never reseed to
  now — reseeding beats against any equal-rate vsync.** Enemy animation interpolation is the next step.
- **★ TICK=30 IS THE CORRECT SPEED + SKELETAL-ANIMATION INTERPOLATION (2026-06-16).** After the beat fix, the user
  found TICK=60 "buttery but everything moves 2x as fast" — because the beat had been secretly dropping ~half the
  ticks (≈30Hz effective); a clean 60Hz reveals the TRUE TICK=60 rate, and Turok's per-tick step is sized for
  **30fps**, so 60 ticks/sec = 2x. The user nailed it: **TICK=30 = Turok's native step = the correct speed**, and
  on a 60Hz panel it's ideal (30Hz logic, 60Hz render interpolates between ticks). **`play_level.sh` now defaults
  TICK=30** (no speed-scaling hack — keeps the simple "TICK = speed" model). Then **animation-frame interpolation**
  (the user's ask "interpolate between frames for animation lerp"): `CGameObjectInstance__DoDraw` poses the skeleton
  from `m_asCurrent/m_asBlend.m_cFrame`, already blending keyframes by the FRACTIONAL frame — so drawing at an
  interpolated `m_cFrame` = smooth limbs. Every anim advances by the same global `frame_increment` per tick
  (anim.c:179), so **prev = cur − step** with NO per-instance storage (`g_turok_anim_step` captured at the tick
  gate in tengine.c). `romstruc.c` (before the DoDraw call ~8982) overrides `m_cFrame = cur − step·(1−alpha)` for
  m_asCurrent + m_asBlend, **snapping (no interp) when `m_cFrame < step`** (just started/wrapped — avoids blending
  backward across a loop; wrap resets to `m_nExitToFrame`, anim.c:198), restored right after the draw. Applies to
  enemies, devices, AND the 1st-person weapon (all go through DoDraw). Verified: egl+sdl2 build, warps 0/3000/6000
  rc=0, animated objects render (boss/enemies), zero `[CAMTRACK]`/`[CANARY]` anomalies. KNOWN MINOR: a loop point
  `m_nExitToFrame > step` (rare) or a JERK_FRAMES hit-react (m_cFrame decremented) gets a 1-interval glitch — fix
  with per-instance prev storage if a specific anim shows it. Instance WORLD position/rotation still not interpolated
  (enemies translate at 30Hz; the skeletal POSE is now smooth — the dominant visual).
- **★ INSTANCE POSITION/ROTATION INTERPOLATION (2026-06-17).** Completes the enemy smoothness: enemy/object WORLD
  pos+yaw now interpolate between 30Hz ticks too (the body-glide, complementing the skeletal pose). Per-instance
  prev is stored ON the struct — `m_ipPrevPos`/`m_ipPrevRotY` added to the tail of `CGameObjectInstance`
  (romstruc.h); SAFE because instances are allocated by `US_TOTAL_SIZE(sizeof(CGameObjectInstance), n)` (scene.c:1592)
  and indexed by that stride, so a tail field grows everything consistently. `romstruc.c CGameObjectInstance__Draw`:
  snapshot pre-tick pos/yaw right after the `isPlayer` def (8638), BEFORE DoAI/Advance (8742/8805) move it — tick
  frames only; then around the orientation-matrix build (8826) override pos/yaw with `lerp(prev,cur,alpha)` and
  restore right after (so the AI still sees the true pos; only the rendered matrix is interpolated). The **d2 < 1000²
  guard** snaps (no lerp) on a warp/teleport OR a garbage/NaN/pre-first-snapshot delta (NaN<x is false → snap).
  Player EXCLUDED (interpolated in UpdateGAME). Verified: warps 0/3000/6000 rc=0, enemies/boss render, no anomalies.
- **★★ INTERPOLATION ANGLE-WRAP HANG — the render-interp angle wraps weren't fmodf-guarded (2026-06-18).** User:
  "the game has been locking up quite a bit, just locking up no crash dump." A HANG, not a crash. Reproduced
  headless (warp 0 → TESTPORTAL warp to the lvl-26 bonus + patrol): froze at frame ~180 in the bonus. Confirmed a
  SPIN (not a deadlock) via /proc (gdb-attach is blocked by yama `ptrace_scope`): the real game pid's MAIN thread
  was `state=R` (running) with the frame counter frozen → an infinite `while` loop (the other 10 threads were idle
  GPU/driver workers in `futex_do_wait`). ROOT: the port's render-interpolation angle wraps used RAW `while (a > PI)
  a -= 2PI; while (a < -PI) a += 2PI;` loops — `tengine.c` CEngineApp__UpdateGAME (player RotY/pitch/RotYOffset/
  RotZOffset = 4 loops) + `romstruc.c CGameObjectInstance__Draw` (instance yaw `_dr`). Unlike the GAME's own angle
  funcs (`graphu64.c NormalizeRotation`, `boss.c`, `tmove.c`) which all got PLATFORM_PORT **fmodf** O(1) fixes (the
  while-loops live in the dead `#else`), MY interpolation loops were never converted. A bonus instance (or the
  player) with a garbage/uninitialised yaw makes the delta HUGE → the loop spins ~1e17× = a freeze with no crash
  dump (a NaN delta EXITS the loop — only a huge FINITE delta spins). **Fix:** new `turok_wrap_pi()` (`port/include/
  turok_port.h`, O(1) `fmodf` wrap to [-π,π), NaN→0) replaces all 5 loops; build clean. **DIAGNOSIS RECIPE for a
  hang-with-no-crash-dump = it's a SPIN: find the real game pid (the MULTI-THREADED child, not the bash wrapper),
  read `/proc/<pid>/task/<main-tid>/stat` field 3 — `state=R` + a frozen frame counter = an infinite loop (the
  angle-wrap while-loop is the #1 suspect). gdb-attach is blocked by `ptrace_scope` here.** LESSON: EVERY angle-wrap
  while-loop in PORT-added code must be fmodf — the render-interp ones were missed (the game's own ones were already
  done). **★ VERIFIED FIXED** via a new watchdog: `TUROK_WATCHDOG=1` (`turok_main.c`) starts a thread that
  `pthread_kill(main, SIGUSR1)`s + `backtrace()`s the MAIN thread if `g_frame` stalls ~4s — the gdb-substitute for
  the `ptrace_scope` block (it catches a spin AND a stall, and prints the exact frozen call stack). The original
  repro (warp 0 → TESTPORTAL → bonus, patrol+audio AND forward-walk, 4 runs to 800-1500 frames) now runs clean,
  watchdog silent, rc=0. **★★ TWO TEST-HARNESS GOTCHAS cost most of this session — the hang LOOKED un-fixable when
  it was already fixed: (1) `pkill -9 -f 'tbm/turok'` is SUICIDAL — `pkill -f` matches the running bash command's
  OWN command line (it contains the binary path `/tmp/tbm/turok`), so it kills the very shell running it → every
  turok launch came back "failed, exit 1, no output". Use `pkill -9 -x turok` (exact PROCESS-NAME match, never
  matches your bash). (2) Repeatedly `kill -9`-ing a RENDERING EGL turok can briefly WEDGE the GPU (a stuck D-state
  `kworker`; new EGL inits then stall with no output); it self-recovers in ~a minute. A one-off "froze at frame
  180" was one of these (a GPU level-reload upload stall), NOT the game spin — the watchdog never caught it, and it
  didn't reproduce.** LESSON: when "the binary won't even run / produces no output", suspect the TEST HARNESS
  (self-kill, GPU wedge, output buffering on a killed proc) before the binary — verify the shell works, then run
  with `pkill -x`, line-buffered (`stdbuf -oL`), and a `SIGKILL` timeout.
- **★ DEATH FALL-THROUGH — re-acquire timing fix (2026-06-17, 3-agent Workflow).** User: dying drops the player
  through the floor; suspected a band-aid regression. Root cause (workflow): the DEATH cinematic
  (cinecam.c `CScene__LoadObjectModelType`, AI_ANIM_DEATH_*) does a model-swap INSIDE `CCamera__Update`, streaming
  assets that RELOCATE the cart-cache collision buffer → re-stales the player's region AFTER the once-per-frame
  re-acquire (which runs at the TOP of UpdateGAME). So the graphics-task `Collision3` runs with a bad region → bails
  → no ground. **Fix: a SECOND re-acquire right after `CCamera__Update`** (tengine.c, before the graphics task),
  mirroring the top-of-frame one. **VERIFIED FIRING** (KILLSELF headless test: the region goes bad on the death
  frame and is re-acquired) — but NOTE: the player stays grounded in headless death tests *with or without* the fix,
  because the dead player is frozen (no gravity) and the top-of-frame re-acquire catches it the next frame. So I
  could NOT reproduce the user's *sustained* interactive fall headlessly; the fix tightens the same-frame window
  (correct + low-risk) but needs interactive confirmation. If it persists, get: death type (enemy/water/fell) + where.
- **★★ DEATH GHOST/FALL — this 2nd audit (render-interpolation) was ALSO WRONG; its interp-snap was a FAILED
  band-aid, REMOVED in cleanup (2026-06-17).** The audit blamed 756cb54 (player render interpolation): claimed the
  respawn teleports the player a sub-1000-unit jump the interp lerps across, sliding the model ("ghost") + dragging
  the camera through the floor, and "fixed" it (8a22bf7) by snapping the interp while `CCamera__InCinemaMode` + a
  12-tick `_ipWasCin` window. **The user's `TUROK_DEATHLOG` trace DISPROVED it: on respawn the position NEVER
  teleports (prev==cur, interp already snapped) — the player FREE-FALLS because the respawned region is
  valid-but-WRONG (no ground). The real fix is the region re-acquire — see the ★★★ DEATH FALL-THROUGH note below.**
  The interp-snap was reverted to the plain 1000-unit big-jump guard once the real fix landed (the death respawn
  doesn't teleport, so the snap guarded a non-existent slide). **LESSON: when a "camera/render" bug's fix can't be
  reproduced/verified in the harness, suspect the AUDIT — get a per-frame STATE TRACE (here: player pos + region)
  before committing. TWO audits guessed interpolation; the trace showed it was collision/region all along.**
- **★★★ DEATH FALL-THROUGH — ACTUAL ROOT CAUSE = respawn leaves a VALID-but-WRONG region (2026-06-17, fixed via the
  user's TUROK_DEATHLOG trace).** The two fixes above (re-acquire timing; interp snap) did NOT fix it — both were
  wrong. The user's death log was the key: on respawn the position NEVER teleports (prev==cur, interp snapped) — the
  Y just **free-falls under gravity** (`819→803→…`, accelerating) because the respawned player has **no floor**. AND
  the region pointer is **rbad=0 (valid)** the whole time. So it's NOT a NULL/stale region (what PORT_REGION_BAD
  catches) — the respawn sets `m_pCurrentRegion` to a **valid region that does NOT contain the player's new X/Z**, so
  `GetGroundHeight`/Collision3 find no ground and the player drops through. Reproduced exactly with
  `TUROK_SPAWNAT=-1988,819,-7703` (spawn far from the streamed start → same wrong-region free-fall), and **forcing a
  `CScene__NearestRegion` re-acquire holds the player rock-solid grounded** (region corrected). **FIX (tengine.c
  CEngineApp__UpdateGAME, the post-CCamera__Update re-acquire): re-acquire the CORRECT region (NearestRegion at the
  live pos) whenever `_rw>0 || PORT_REGION_BAD`, where `_rw` is a 30-frame countdown armed by `CCamera__InCinemaMode`
  — i.e. for the whole death/resurrect cinematic + a window after, when the respawn lands. Scoped to cinematics, so
  normal play keeps the game's own region tracking (no per-frame NearestRegion cost/override).** Verified headless:
  SPAWNAT-far + kill → respawn stays grounded (Y stable) instead of free-falling; normal patrols rc=0. **LESSON:
  PORT_REGION_BAD only validates the region POINTER (NULL/range/distance) — it does NOT verify the region CONTAINS
  the player. A respawn/warp that sets position without correctly setting the region produces a valid-but-wrong
  region → no ground → fall-through. Re-acquire by position (NearestRegion) on respawn, not just on a bad pointer.**
- **★ "MISSING PLATFORM" — was a v49-vs-retail ASSET issue, NOT a framerate regression (2026-06-16).** User reported
  the warp-0 fire-pit "initial platform" missing on the branch + suspected the level resources weren't importing.
  Bisected with byte-identical headless captures: the branch renders the warp-0 spawn **identical to master**
  (firepit, md5 6ce37827, consistent across frames 200–14000 at TICK=0 and TICK=60) — so the framerate CODE is
  innocent. The "stone pillar in water" screenshot was a one-off flaky capture, not reproducible. **Root cause:**
  the level-1 **walkway** over the water is a **RETAIL-only asset** — the v49 leak's `cartdata.dat` lacks it; walk
  FORWARD at the fire-pit on v49 assets and you drop into a **blue void** (verified: retail-forward = canyon path
  md5 75db37f2; v49-forward = empty blue d41cfdb3). `play_level.sh` defaulted to v49 unless `ROM=` was passed, so a
  plain `./play_level.sh` loaded the leak assets → no walkway. **Fix: `play_level.sh` now defaults to the retail ROM
  (Path B) when `baserom.us.v12.z64` is present** (`ROM=none` forces v49). **LESSON: when geometry is "missing,"
  first confirm WHICH asset set is loaded (v49 placeholder vs retail Path B) before suspecting code — many "missing
  stuff" reports are the v49 leak being incomplete, fixed by Path B, not a bug.** Headless-capture gotcha found:
  `TUROK_CAPTURE_FRAME=N` on no-tick-gate builds needs `TUROK_MAX_FRAMES` WELL above N (render-frame s_frame_no lags
  the frame-pump g_frame), else the capture silently never fires.
- **`sched.c`** — `scSendCommand` (PLATFORM_PORT) now calls **`osViSwapBuffer(pTask->framebuffer)` right after
  dispatching the gfx task**. On N64 the scheduler thread's `__scHandleRetrace` presents finished gfx tasks;
  that thread never runs cooperatively, so without this every frame rendered but was NEVER presented — the
  main loop spun re-rendering and the render-frame/capture/frame-limit counters never advanced (the "renders
  forever / hangs at frame 4" symptom). **This was the core M2 blocker.**
- **`tengine.c`** — (a) optional **warp-to-level** in MODE_RESETGAME (`TUROK_WARP=<0..8000>` env skips the
  intro and loads a real level); (b) **HUD-draw guard** in `CEngineApp__DrawGAME` (skip `COnScreen__Draw`
  unless `TUROK_HUD=1` — the HUD's big-endian `C16BitGraphic` assets fault until M5). Both PLATFORM_PORT-gated.
- **`port/fast3d/gfx_egl.cpp`** — rewritten to use a **GBM render node** (`/dev/dri/renderD128` →
  `gbm_create_device` → `eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR)`): headless HARDWARE GL on the GPU with
  NO X server (radeonsi). Requests a compat profile; capture (`gfx_egl_save_png`) resets colormask/scissor +
  `glReadBuffer(COLOR_ATTACHMENT0)` before readback. This is the working in-sandbox renderer (`GFX=egl`).
- **`port/fast3d/gfx_opengl.cpp`** — **scissor root fix**: `gfx_opengl_set_scissor` ignores degenerate
  (zero-area) rects. Turok interleaves a zero-height `G_SETSCISSOR` (uly==lry → GL scissor h=0) with the real
  full-screen one; a zero-height GL scissor clips EVERYTHING, blanking the frame. Also: absurd-texture-dimension
  guard in `upload_texture`; VAO bound unconditionally for EGL; env-gated diagnostics (TUROK_VP_LOG, etc.).
- **`port/fast3d/gfx_pc.cpp`** — runaway-DL tripwire (bail after 300k cmds) so a non-terminating display list
  reports instead of hanging.
- **M5 RNC-decompression endianness (2026-06-13)** — compressed cart blocks (Object Attributes, Sound
  Effects, level data) DMA'd but never decompressed, stalling the whole level-load streaming chain:
  - **`cart.c CMP_GetDecompressedSize`** read the RNC unpacked-size DWORD raw (big-endian) → ~2.5 GB → alloc
    fails → cache entry's "received" callback never fires. Fixed with `__builtin_bswap32` under PLATFORM_PORT.
  - **`pp.h`** — widened the `#ifdef WIN32` endian-macro guard to also cover `PLATFORM_PORT` so
    `BIGENDIANL/BIGENDIANW/GETBIGENDIANW` byte-swap (no-ops before → `InitUnpack` OutputEnd overran the buffer).
  After both, the streaming chain loads every per-level block (Collision, Instances, geometry, textures).
- **`romstruc.c CGameRegion__GetGroundNormal`** — PLATFORM_PORT NULL-guard on `m_pCorners[]` (region corner
  pointers not populated until the collision/region parse lands; M5). Falls back to an up-normal.
- **`cart.c`** — temporary always-on `[cart] DECOMPRESS …` stderr trace (M5 debugging; trim later).
- **`coll.c`** — NOT edited; deferred (excluded from the build). It references collision-struct fields
  (`m_dwFlags`, `m_GroundCollision`) and flags (`COLLISIONFLAG_NOFLOORCOLLISION`) that are renamed/commented
  out in the current `collinfo.h`. It's the old monolithic collision, partly superseded by
  `wallcoll.c`/`unicol.c`/`instcol.c`/`regicol.c`/`wradcol.c`. Reconcile at M5 (collision), not before.

- **★ INPUT (Item 1, 2026-06-14) — DONE.** Controller seam: `port/src/turok_input.c` (NEW) holds the N64 pad;
  backend pushes via `turokInputSetState`, the libultra shim reads via `turokInputGetPad`. `os_shim.c` wires the
  SI chain — `osSetEventMesg(OS_EVENT_SI)` stores the mq+msg, `osContStartReadData` posts it (→ the game's
  `CONTROLLER_MSG` → `UpdateController`/`ReadControllerAdvanced`), `osContGetReadData` fills the pad (was a
  no-op, so input never reached the player). `gfx_sdl2.cpp` maps keyboard+gamepad→N64 bits each frame.
  `TUROK_FAKEINPUT=1|2` injects forward / forward+turn for headless validation. Forward+turn move the player
  through the rendered level (verified) with no hang.
- **★ PC FPS CONTROLS + FILE SAVE (2026-06-20, commits 6aa3fa1/9add349/ef5e185/69e4536; all PLATFORM_PORT).**
  Modern KB+M scheme on top of the N64 pad seam. HELD mouse-look (no spring-back): the mouse feeds a port seam
  `g_look_yaw`/`g_look_pitch` (input.c, always-linked) consumed by a `tengine.c CEngineApp__UpdateGAME` hook that
  turns the body (`m_RotY`, holds) + accumulates a clamped held pitch on `m_RotXOffset` — NOT the spring-centered
  N64 look-stick. Exclusive cursor (Alt-Tab releases via SDL focus events), 1280x1024 window (`TUROK_WIN_W/H`).
  **★ THREE root-caused refinements (ef5e185):** (1) **"W toggled run/walk"** — the engine's DEFAULT config is
  right-handed (`m_RHControl=TRUE`, options.c:205) where MOVEMENT is the **C-buttons** and the **D-PAD is the
  native run/walk toggle** (CTTYPE_SINGLE, tcontrol.c:269); my WASD set both groups so every keypress fired the
  toggle. Map WASD to the **C-buttons only**. The correct N64 action map for this config: Z=fire, R_TRIG=jump,
  L_TRIG=map, A=next-/B=prev-weapon, Start=pause (the earlier labels were scrambled — N64_B is SelectWeaponPrev,
  not jump). (2) **mouse pitch inverted** — default now forward=look-up (`TUROK_MOUSE_INVERT=1` flips). (3)
  **scroll cycled weapons too fast / skipped** — the **render-only-frame bug class again**: weapon-select switches
  whenever `SelectWeaponTimer==0` but that timer only grows via `frame_increment` (=0 on render-only frames at
  FPS>TICK), so a held weapon button re-switches every render frame. Fix: gate the button switch on a logic tick
  (`TUROK_IS_TICK = frame_increment!=0`, no-op on N64) AND route the wheel through a discrete `g_weapon_cycle`
  seam consumed ONE notch per tick in `tmove.c` → exactly one weapon per notch, render-rate independent. E =
  walk toggle (port `g_turok_walk_mode` → `TUROK_WALKCAP` halves speed; D-pad now clear so the native toggle is
  never fired). **★ FILE SAVE (69e4536):** F5 quick-save / F9 quick-load. Writes the SAME `CPersistantData` blob
  the N64 pak-save uses (packed by `CSave__PrepareData`) to a host file (PC: `$TUROK_SAVE` or `./turok_save.bin`;
  3DS: `sdmc:/3ds/turok/`) with a magic+size header; quick-load reads it, applies via `CLoad__ExtractData`, and
  restarts the level at the saved checkpoint — mirroring the pause-menu load (loadsave.c:695-710). Host-order on
  both targets (x86 + 3DS ARM = LE → cross-compatible). New port files: `loadsave.c CSave__QuickSaveToFile`/
  `CLoad__QuickLoadFromFile` (declares `getenv` directly — the engine `#define abs(n)` collides with `<stdlib.h>`),
  request flags in input.c, the consume hook + headless self-test (`TUROK_QUICKSAVE_FRAME`/`TUROK_QUICKLOAD_FRAME`)
  in tengine.c. Verified headless: egl+sdl2 build clean, round-trip (save@200→load@400 on a warp-0 patrol) writes a
  256-byte file, restarts at the checkpoint, rc=0. **NEEDS INTERACTIVE CONFIRM** (mouse turn/look sign + scroll
  direction can't be headless-tested). **NEXT: an in-game OPTIONS menu** to tune sensitivity/invert/walk + rebinds.
- **★ SETTINGS PERSISTENCE — `turok.cfg` (2026-06-20, commit 65b521a).** `port/src/config.c` (new, shared-layer):
  loads/saves a plain `key value` file (PC `$TUROK_CFG`/`./turok.cfg`; 3DS `sdmc:/3ds/turok/turok.cfg`) at
  startup for mouse_sensitivity / mouse_invert / walk_default / window_width/height. The env knobs still
  override at read time. The data layer the eventual in-game options menu reads/writes.
- **★★ 3DS BUILD STOOD UP — M0 (compiles) + M1 (links to turok.3dsx) (2026-06-20, a8a96b2/f5caa21/b79e4c2).**
  A SECOND build system, `Makefile.3ds` (devkitARM + libctru + Citro3D), ADDITIVE alongside the PC
  `tools/build_port.sh`. The game tree compiles UNCHANGED under `-DPLATFORM_PORT` (PC+3DS shared) + a new
  `-DPLATFORM_3DS` (3DS-only). **The Citro3D backend (`port/fast3d/gfx_citro3d.cpp` + `gfx_3ds.c` + the PICA
  shader) was already vendored from Perfect Dark**, so the work was the Makefile + sys/audio/input/stub TUs +
  a small bridge branch, NOT a from-scratch backend. **3DS-M0** (`make -f Makefile.3ds objects`): all 201 TUs
  (77 engine + 105 libaudio + gu + port + Citro3D C++) compile clean on ARM — devkitARM gcc 15 promotes
  legacy-C (implicit-int/K&R/pointer-puns) to hard errors `-w` doesn't suppress, so the Makefile adds the
  `-Wno-*` downgrades + `-fno-short-enums` (CRITICAL: ARM-EABI packs enums → breaks the `-1` sentinels) +
  force-includes `turok_port.h` (qsort rename / fmodf proto / `turok_wrap_pi` inline). **3DS-M1** (`make -f
  Makefile.3ds`): LINKS → `build_3ds/turok.3dsx` (1.05 MB). New 3DS port TUs (all `#ifdef PLATFORM_3DS`, empty
  on PC, auto-globbed): **`audio_3ds.c`** (audio.h contract on ndsp + the dedicated audio thread via libctru
  threadCreate/RecursiveLock — same stash/push sink + `turokAudioManagerFrame` synth seam as the PC `audio.c`,
  22050Hz, wave buffers in linearAlloc); **`input_3ds.c`** (HID → the portable `inputSetState` seam; `input.c`
  KEPT as platform-agnostic; first-boot FPS map, polled per frame from `gfx_3ds.c::handle_events`);
  **`sys_3ds.c`** (`__stacksize__=2MB` — the inline boot overflows the 32KB default — + `plat3dsBootLog` = the
  on-device trace `sdmc:/3ds/turok/boot.log`+svcOutputDebugString = the PC-stderr equivalent, + fb capture);
  **`stderr_3ds.c`** (provides the `stderr` GLOBAL SYMBOL the game's port traces link against — newlib makes
  stderr a macro, not a symbol; `turok_main.c` points it at the real stream early). Port host-bits gated for
  3DS: `system.c` x86-`pause`→ARM barrier; `turok_main.c` watchdog (pthread+execinfo)→no-op; `romdata.c`
  DMA-OOB backtrace→host-only + **the asset ROM defaults to `sdmc:/3ds/turok/baserom.us.v12.z64`** (Path B).
  `turok_gfx.c` routes to `gfx_3ds`+`gfx_citro3d_api`. PC re-verified unaffected at every step. **NEXT —
  3DS-M2 (boot in Mandarine, the RUNTIME phase):** run `build_3ds/turok.3dsx` in Mandarine
  (`~/Desktop/citra/mandarine.AppImage`) with `baserom.us.v12.z64` on the virtual SD at `sdmc:/3ds/turok/`;
  `boot.log` on the SD shows where it faults. **ARM byte-alignment fixes** (ARMv6K faults on unaligned
  LDM/LDRD/VLDR — the big-endian asset parsers are the suspects) are crash-driven from there (same methodology
  as the PC port), + heap/dspfirm tuning.
- **★★★★ 3DS PRE-MAIN BOOT HANG — SOLVED (2026-06-21, commit `e1a6b47`). ROOT CAUSE = Turok's custom `memset`
  overriding newlib's.** `src/PR/tengine/memory.c` defines its own `memset` (for the N64 bare-metal build, under
  `#ifndef WIN32`) — a correct byte-loop, but when statically linked on 3DS it **SHADOWS newlib's `memset`**, and
  **libctru's own code calls `memset`** (notably `threadCreate`, which zeroes the new thread's stack/TLS). libctru's
  `aptInit` creates the APT event-handler thread (the 2nd thread) via `threadCreate → svcCreateThread`; with Turok's
  `memset` shadowing newlib's, that thread creation **HANGS**, so `aptInit` never returns and the game never reaches
  `main`. **FIX:** gate Turok's `memset` out on `PLATFORM_3DS` so newlib's optimised, ABI-correct `memset` is used
  (PC keeps Turok's — its system threading uses glibc's internal `memset`, so the override is harmless there; PC
  build+run verified unaffected). **VERIFIED on Mandarine via gdb: full Turok now hits `[New Thread 2]` → `hidInit`
  (aptInit done) → `main` → `turokConfigLoad` → `romdataInit` (ROM/cartdata load) → `turokGfxInit` (Citro3D init).**
  **★ HOW IT WAS FOUND (the method that cracked it): MINIMAL-REPRO BISECTION.** Build a near-empty `main.c` 3dsx with
  Turok's EXACT flags/specs/libs and gdb-probe whether it reaches `main`. Findings (all via gdb on clean Mandarine —
  the no-gdb `boot.log` method is UNRELIABLE because repeated Mandarine launches WEDGE it, so a "hang" there can be a
  wedge, not the binary): vanilla minimal BOOTS; +Turok-flags (`-mtp=soft -fno-short-enums` etc.) BOOTS; +16 MB BSS
  BOOTS; +2 MB `__stacksize__` BOOTS; +the `__system_allocateHeaps` override BOOTS — so it's NOT flags/size/stack/heap.
  Full Turok hangs even at 9.6 MB BSS → it's CONTENT, not size. Then: a **symbol-intersection of Turok's objects vs
  newlib `libc.a`** (NOT just libctru.a — that was the earlier blind spot) found the ONLY override: **`memset`**. The
  gdb pin (hung right at `threadCreate`'s `memset` call) matched. **LESSON: when a static port HANGS pre-`main` in
  libctru and every config/size variant of a minimal repro boots, check whether the game overrides a libc/newlib
  function libctru calls (`memset`/`memcpy`/`malloc`/locks) — intersect the game's defined symbols against BOTH
  libctru.a AND libc.a. A legacy bare-metal game commonly ships its own `memset`/`memcpy`; on a hosted libc those
  MUST NOT shadow the system ones, or libctru's internal calls hit the game's version pre-`main`.** **NEXT = the
  in-`main` game boot** (asset load + the crash-driven ARM byte-alignment work) continues from `turokGfxInit`. The
  `__system_allocateHeaps` headroom override (commit `dafde8d`, verified safe via the minimal repro) + the gated
  diagnostic `__appInit` are KEPT. **★ REUSABLE HARNESS: `/tmp/boot_probe.sh <gdb-cmds> <elf> <3dsx>`** (kills
  `AppRun.wrapped`/`.mount_mandar`, launches Mandarine + polls `:24689`, runs `gdb-multiarch -batch` timeout-bounded —
  hung `continue` ⇒ rc 124, last printed breakpoint = the hang boundary). Give Mandarine a ~40 s cooldown between
  runs or it wedges. The minimal-repro builds live in `/tmp/min*.c`.
- **★★★ [SUPERSEDED by the SOLVED entry above — the cause was Turok's `memset` override, found via minimal-repro
  bisection] 3DS BOOT HANG — CORRECTED + DEEPLY root-caused (2026-06-21, commit `dafde8d`). The "srvInit" entry just
  below is SUPERSEDED: `srvInit` does NOT hang.** A fresh gdb-on-Mandarine pass on the current build proved the
  boot chain runs `srvInit` to completion (`svcConnectToPort` returns, `srvRegisterClient`'s `svcSendSyncRequest`
  returns) and reaches **`aptInit`**, where it hangs. Pinned further with breakpoints on `aptInit`'s internals:
  `APT_GetLockHandle` + `APT_Initialize` complete, then `threadCreate` for the APT event-handler thread is
  entered, its stack `memalign` SUCCEEDS, and **`svcCreateThread` never returns + no new guest thread spawns**. A
  **throwaway test thread created BEFORE `aptInit`** (via a diagnostic `__appInit`) hangs identically → **`svcCreateThread`
  hangs for ANY thread in Turok's process under Mandarine**, independent of APT. **Perfect Dark (same libctru,
  same Mandarine) makes the identical call and boots** (`[New Thread 2]` → `hidInit` → `main`), so it is
  **Turok-specific**. Confirmed WITHOUT gdb (no `boot.log` ⇒ hung pre-`main`) and reproduced with the **JIT off**.
  **RULED OUT (each empirically tested, none fixed it):** heap exhaustion — libctru's default `__system_allocateHeaps`
  grabs ~100% of the N3DS application FCRAM (main 75.8 MB + linear 32 MB + 16 MB image = ~124 MB = the whole
  region), but **capping it to leave 32 MB headroom did NOT fix the hang**; **BSS size** (16 MB → 9.6 MB by
  dropping the 6.49 MB `_staticSegmentRomStart`, no change); the **JIT**; the thread **`core_id`** (−2 and −1);
  the **`-fno-short-enums`** enum-ABI mismatch vs libctru's packed enums (the linker warns "use of enum values
  across objects may fail", but building without it did NOT fix it); and **New-3DS vs OG-3DS** mode. So it is a
  **Mandarine `svcCreateThread` HLE limitation** triggered by some Turok process trait, not an obvious config/memory
  bug. **KEPT (3DS-only, PC unaffected, both in `sys_3ds.c`):** (1) a `__system_allocateHeaps` **override** that
  leaves a 32 MB FCRAM safety margin + caps the linear heap + calls `mappableInit` — PD-faithful, a correct
  *latent-bug* fix (the default's 100%-grab would starve later threads/shared-mem) even though it is NOT the boot
  fix; (2) a **gated diagnostic `__appInit`** (`-DTUROK_DIAG_APPINIT`, OFF by default) with a test-thread probe +
  `svcOutputDebugString` traces + clean gdb-breakpoint boundaries, for the next step. **★ NEXT = needs REAL-3DS-HARDWARE
  (Luma) confirmation — only the user can run it:** if `turok.3dsx` boots on real HW, the hang is a Mandarine HLE
  limitation (change the dev loop / try a newer emulator build); if it hangs identically on HW, it's a real bug and
  HW debugging continues from the `svcCreateThread` point (rebuild with `-DTUROK_DIAG_APPINIT` for traces). The
  remaining un-eliminated Turok-only trait is the **large/odd binary image** itself (16 MB memsz, 201 linked TUs) —
  a candidate for the minimal-repro approach (strip to a near-empty 3dsx on Turok's exact Makefile and see if IT
  can `svcCreateThread`). **★ HARNESS that works (reusable): `/tmp/boot_probe.sh <gdb-cmds> <elf> <3dsx>`** —
  kills `AppRun.wrapped`/`.mount_mandar`, launches Mandarine + polls `ss -ltn | grep :24689`, runs `gdb-multiarch
  -batch` (timeout-bounded; a hung `continue` ⇒ rc=124, last printed breakpoint = the hang boundary), kills
  Mandarine. No-gdb boot check: `use_gdbstub=false`, stage the ROM at `~/.local/share/mandarine-emu/sdmc/3ds/turok/`,
  run, look for `boot.log` (only written post-`__appInit`/sdmc-mount).
- **★★ [SUPERSEDED by the entry above — `srvInit` actually completes] 3DS BOOT HANG — pinpointed via gdb to libctru `srvInit` (2026-06-21). THE GDB METHODOLOGY (reusable):**
  the build BOOTS but hangs before reaching the game's `main`. Mandarine's gdbstub DOES work (despite the first
  impression) — the breakthrough was process hygiene + a robust harness. **★ PROCESS GOTCHA (cost hours): the
  Mandarine AppImage forks a child named `AppRun.wrapped` under `/tmp/.mount_mandar*/` — `pkill -i mandarine`
  NEVER matches it, so launches STACK UP (5+ instances all holding the single gdbstub port 24689), making every
  gdb run flaky/stale. Kill with `pkill -9 -f "AppRun.wrapped"; pkill -9 -f "\.mount_mandar"`.** Other harness
  rules: (a) a backgrounded Mandarine dies (SIGHUP) when the Bash call returns — run Mandarine + gdb in ONE
  blocking call; (b) POLL for the port (`ss -ltn | grep :24689`) before connecting, not a fixed sleep; (c) the
  gdbstub does NOT support async interrupt of a running `continue` (SIGINT won't break a hang) — use BREAKPOINTS
  + `stepi`, not interrupt; (d) enable via `~/.config/mandarine-emu/qt-config.ini` `use_gdbstub=true` AND
  `use_gdbstub\default=false` (the `\default=true` flag makes Mandarine ignore the value) — it logs
  `Debug.GDBStub: Waiting for gdb to connect`; (e) Mandarine's log file flushes only on graceful close, and it
  does NOT surface `svcOutputDebugString` — the SD `boot.log` (per-line fflush) is the only code-trace channel,
  but it needs sdmc (mounted late in `__appInit`), so use gdb for anything pre-sdmc. **THE FINDING:** breakpoints
  on the boot chain trace `_start → initSystem → __libctru_init (__system_initSyscalls, __system_allocateHeaps)
  → __appInit (appInit.c:14, REACHED) → srvInit (REACHED + executing — stepped through osGetKernelVersion at
  srv.c:29-32)`. `__appInit`'s disasm is `bl srvInit; bl aptInit; bl hidInit; bl fsInit; bl archiveMountSdmc`;
  the `aptInit` breakpoint is NEVER hit → **srvInit does not return → the hang is the srv: service connection
  inside libctru `srvInit`** (a blocking `svc` — svcConnectToPort/IPC — Mandarine isn't answering for Turok,
  though it does for banjo+PD). RULED OUT (all via the now-working gdb / earlier tests): BSS size (tested
  15.9→3.26 MB, below PD's 2.1 MB), `__stacksize__`, symbol overrides, the heap split, `-fno-short-enums`, the
  C++ ctors (init_array not reached). **NEXT:** one clean gdb run setting a breakpoint after srvInit's
  `svcConnectToPort` to confirm the exact blocking syscall, then diff Turok's srv:/environment setup against PD
  (`../perfect_dark`) + sm64-port (`../sm64-port`, the user-provided complete N64→3DS Citro3D reference) — both
  use the same libctru and boot, so the difference is the loader environment / a Turok-specific binary trait.
- **★ ANGLE-WRAP HANG CLASS (Item 4) — fixed; all 8 level warps load+render, no hang.** Three iterative
  angle-normalization `while` loops spin ~1e17× (hang) on a garbage/huge angle from an unspawned AI off-N64.
  Replaced with O(1) `fmodf` wraps under PLATFORM_PORT: `graphu64.c NormalizeRotation` (turning toward an
  enemy), `boss.c AngleDiffFromZero` (Campaigner boss, warp 6000), `tmove.c` RotXPlayer.
  **★★ CRITICAL PITFALL — `fmodf` MUST be prototyped.** tengine's math headers declare the DOUBLE fns
  (sin/cos/sqrt) but NOT the float ones, so a bare `fmodf()` is implicitly declared `int fmodf()` → wrong ABI →
  GARBAGE return (compiles silently under `-Wno-implicit-function-declaration`). The garbage angle → NaN player
  rotation/position → NaN camera matrix → the WHOLE level clip-rejects → renders as a flat fog color. This cost
  most of a session masquerading as "the world terrain never renders / player spawns NaN" — it was entirely the
  missing prototype. FIX: `extern float fmodf(float, float);` added to `port/include/turok_port.h`. Bisect vs
  the M5 commit nailed it (M5=34 colors → +graphu64=1 color → +prototype=34 colors). **LESSON: verify any
  render change by VIEWING pixels (distinct color-bucket count), NOT tri-count/nonblank-px — a fog fill is
  "nonblank" with hundreds of clip-rejected tris. And any libc fn the game never called needs an implicit-decl
  check.**
- **AUDIO (Item 2) — output foundation done; classic-ABI mixer is the remaining hard piece.**
  `port/src/turok_audio.c` (NEW): host audio OUTPUT sink — SDL2 `SDL_QueueAudio` (16-bit stereo) or headless
  WAV dump (`TUROK_AUDIO_WAV=path`); `TUROK_NOAUDIO=1` off. `os_shim.c` AI seam: `osAiSetNextBuffer→turokAudioPush`,
  `osAiGetLength→turokAudioQueuedBytes`, `osAiSetFrequency→turokAudioSetRate`. Both match the Perfect Dark model.
  GAP: Turok uses the CLASSIC libultra audio ABI (stateful `aSetBuffer`), unlike Banjo/PD's n_audio, so PD's
  `mixer.c aXxxImpl` is NOT a drop-in — needs a classic-ABI Acmd interpreter (~multi-session). Turok's own
  libaudio (`turoksnd/abi/*.c`, 105 TUs incl. the CSP music player) all compile on host; banks on disk
  (`src/PR/tengine/sfx.ctl`+`sfx.tbl`). `audio.c initAudio` still early-returns.
- **★ AUDIO THREAD — branch `audio-thread`; S1 (threaded skeleton) DONE (2026-06-17).** Goal: audio working under
  its OWN THREAD like PD/banjo/forsaken. A 5-agent study mapped the siblings' audio threading + turok's state.
  **KEY INSIGHT:** the N64 already runs audio on its own thread (`audiomgr.c osCreateThread(THREAD_AUDIO,__amMain)`);
  the port made every libultra thread a cooperative no-op, so audio never ran. The fix = make the AUDIO thread
  REAL (pthread on PC, `threadCreate` on 3DS) while game/gfx stay cooperative — exactly the N64's own split + the
  sibling pattern (PD/banjo `sSynthLock`, forsaken `LightLock`). **MIXER DE-RISKED** (corrects the old "PD mixer
  not a drop-in" worry): PD's `port/src/mixer.c` `aXxxImpl` kernels ARE classic-ABI (over an emulated 3072-byte
  `rspa` DMEM struct); turok needs only the packed-`Acmd` DISPATCH LOOP that PD/banjo deleted (their `audioRunAcmd`
  is a no-op because n_audio never builds a list) — ~150 LOC new + ~600 LOC kernels lifted from PD. The difference:
  turok's synth (`turoksnd/abi/synthesizer.c:142 alAudioFrame`) emits a packed Acmd list (classic ABI, `abi.h`
  A_SETBUFF=8 stateful), PD's calls the mixer inline (`n_abi.h`). **STAGED PLAN (each headless-testable via
  `TUROK_AUDIO_WAV`):** S1 thread+device+sync+test-tone (DONE) → S2 un-gate `initAudio` + load+endian-swap the
  on-disk banks (`sfx.ctl/tbl`, `testbank.*`; ALBankFile is big-endian, `.tbl` payload stays raw-BE) → S3 drive
  the real synth (`amgrPumpOneFrame`→`alAudioFrame`→Acmd list) on the thread → S4 the mixer (`turokAcmdRun`: Acmd
  → PCM, audible) → S5 SFX correct → S6 music (CSP `csplayer.c`). **S1 = `port/src/turok_audio_thread.c`** (NEW):
  the dedicated pthread audio thread; a RECURSIVE `synthLock` mutex (`audioSynthLock/Unlock`, taken by the game
  thread around synth voice-list mutations at S3+); a poll-paced refill loop (2ms; back-pressure = device queue
  depth `turokAudioQueuedBytes`); `produce_one_frame()` = SILENCE by default / a 440Hz tone with
  `TUROK_AUDIO_TESTTONE=1` → `turokAudioPush` sink. `turok_main.c` starts it post-`boot()` + joins+closes pre-exit;
  `turok_audio.c` `TUROK_AUDIO_WAV` now takes precedence in ANY build (headless verify even on GFX=sdl2);
  `build_port.sh` links `-lpthread`. `TUROK_AUDIO_THREAD=0` disables (inline/none). **VERIFIED:** egl+sdl2 build;
  the test tone is a clean 440Hz, peak-8000, 22050Hz WAV; silence by default; patrols all warps rc=0 (thread
  stable alongside the game); clean start/stop/join. The loop body's `produce_one_frame()` is the SINGLE seam S3+
  swaps to the real synth+mixer — the threading never changes again. **NEXT: S2** (un-gate `initAudio`, load +
  endian-swap the disk banks, replacing the `turok_runtime.c` zero-span aliases; drop `audio_lib_stub.c` to avoid
  shadowing the real `turoksnd/abi` once it's added to the build).
  **★ AUDIO LAYER REALIGNED to the PD/banjo contract (2026-06-17, per the user — for an eventual reusable
  N64→PC/3DS library).** Folded `turok_audio.c` + `turok_audio_thread.c` into a single **`port/src/audio.c`** (the
  sink + the dedicated thread) implementing the EXACT `port/include/audio.h` contract (`audioInit` /
  `audioGetBytesBuffered` / `audioGetSamplesBuffered` / `audioSetNextBuffer` / `audioEndFrame` + the thread
  `audioThreadStart`/`Stop`/`SynthLock`/`Unlock`/`audioThreadActive`), matching perfect_dark (`port/src/audio.c` +
  the `audio_3ds.c` thread). The sink is the same **stash (`audioSetNextBuffer`) + push-with-back-pressure
  (`audioEndFrame` if `audioGetSamplesBuffered() < 8192`)** model. Audio is a **SHARED-layer** file → **plain name
  `audio.c`** (NOT the `turok_` glue prefix — it compiles to `port_audio.o`, distinct from the game's
  `tengine/audio.c`→`audio.o`, and the port `-I` never reaches `src/PR/tengine`, so the reorg's "collision" worry
  was wrong). `os_shim.c` AI seam → `audioSetNextBuffer`/`audioGetBytesBuffered` (`osAiSetFrequency` now just
  returns the rate — the device rate is fixed at `audioInit`, the PD model); `turok_main.c` → `audioInit()` (open
  the device on the main thread) + `audioThreadStart()` post-boot, `audioThreadStop()`+`audioClose()` pre-exit.
  The thread loop's `audio_synth_frame()` is the SINGLE S3 seam (S1: tone/silence; S3: `amgrFrame()` — unchanged
  threading). VERIFIED: egl+sdl2 build, clean 440Hz tone through the new contract, patrols all warps rc=0.
  port/README.md updated (audio is a plain shared file; deviation #4 corrected — the audio seam uses `audio.h`,
  only gfx/input stay inline-extern for the OSContPad C++ friction). **LESSON for the reusable lib: classify each
  port file by ROLE — SHARED → plain name + a contract header (so it lifts verbatim); GLUE → game-prefix.**
  **★ S2 GATEWAY CONFIRMED (2026-06-17):** all **105** `turoksnd/abi/*.c` compile clean in the host build with
  `-Ituroksnd/abi -Ilib/ultralib/include/PR -Ilib/ultralib/include -Ituroksnd/abi/buildss` + the game defines
  (`-DPLATFORM_PORT -D_LANGUAGE_C=1 -D_MIPS_SZLONG=32 -DF3DEX_GBI`). **Use the GAME's `libaudio.h`
  (`lib/ultralib/include/PR`), NOT turoksnd's `buildss/libaudio.h`** — they DIFFER (961 vs 935 lines), so ABI
  consistency requires the game + the lib share ONE header (PR-first in the `-I`; `buildss` only supplies `em.h`,
  which exists nowhere else). The real lib DEFINES the `al*` the stub does (`alAudioFrame`=synthesizer.o,
  `alSndpNew`=sndplayer.o, `alCSPNew`=csplayer.o, `alBnkfNew`=bnkf.o, `alHeapInit`=heapinit.o), so
  **`audio_lib_stub.c` MUST be dropped** when the lib is added (duplicate-symbol otherwise). **★ COUPLING — S2
  lib+banks+un-gate land TOGETHER, not separably:** `initAudio`'s PROLOGUE (lines 130/136, *before* the
  early-return at 191) already calls the real `alHeapInit`+`alBnkfNew`, so integrating the lib while the banks are
  still zero-span (`turok_runtime.c` aliases) would run real `alBnkfNew` on an empty bank → likely crash. So the
  lib add + the disk-bank load/endian-swap + the un-gate must be one commit. (S1 thread is independent + already
  landed; S2 is the next coupled unit.)
  **★ S2 ATTEMPTED → confirmed ALL-OR-NOTHING; reverted to clean S1; 3 sub-findings banked (2026-06-17).** Tried
  integrating the lib incrementally (add lib + drop stub + guard the SEQ-bank prologue + KEEP initAudio's
  early-return). It LINKS (105 TUs) but CRASHES at runtime: the game calls real al* EVERY FRAME
  (`CEngineApp__Main → SetAudioVolume → alCSPSetVol → alEvtqPostEvent`, fault 0x48) on the CSP/sound players the
  early-return never created — the STUB made these no-ops, the real lib derefs the uninitialised players. So the
  lib CANNOT be added while initAudio defers: **S2 is ONE atomic commit = add lib + load/swap banks + un-gate
  (create the players), together.** Sub-findings for the focused next session:
  - **COMMITTABILITY: `turoksnd/` is UNTRACKED** (user-supplied leaked SDK, like the ROM; only `.tbl`/`.ctl` are
    gitignored — `src/PR/tengine` IS tracked, `turoksnd` is NOT). So the bank endian-swap + any host shim MUST
    live in TRACKED files (port/ or tengine), NOT by editing `turoksnd/abi/*.c` (e.g. NOT bnkf.c). Provide a
    TRACKED `turokBnkfNew` (swap+relocate) and call it from audio.c under PLATFORM_PORT instead of `alBnkfNew`.
  - **MISSING SYMBOL: `alReverbSetType`** — referenced by `turoksnd/abi/synsetfxtype.c` (`alSynSetFXtype`),
    defined NOWHERE in the leak. Provide a tracked host stub (no-op resolves the link; `SOUND_USE_REVERB` is on so
    SFX/music play DRY until reverb is ported or `AL_FX_NONE` is forced). Put it in a tracked port file.
  - **THE BANK SWAP — worked out, ready to write in a TRACKED file:** swap-on-read during the relocation walk,
    AFTER each struct's `flags` guard (so shared structs aren't double-swapped): revision/bankCount; per ALBank
    instCount/sampleRate + bank/perc/inst offsets; per ALInstrument bendRange/soundCount + sound offsets; per
    ALSound the 3 offsets (envelope/keyMap/wavetable); per ALWaveTable len + base/book/loop offsets; then the
    LEAF structs the walk only points at — ALEnvelope (3×s32 times), ALADPCMBook (order, npredictors,
    book[order*npredictors*8] s16 with a >4096 garbage-cap), ALADPCMloop (start/end/count, state[]). The `.tbl`
    VADPCM payload stays raw big-endian. Banks: `src/PR/tengine/sfx.ctl` (127,688 B, `B1`=0x4231 BE) + `sfx.tbl`
    (7,298,152 B — too big for the 292 KB audio heap, malloc it as the wave base passed to alBnkfNew); music
    `turoksnd/sequences/testbank.ctl/.tbl`.
  - **THE S2 ATOMIC COMMIT (next session):** build_port.sh add turoksnd/abi (recipe `-Ituroksnd/abi
    -Ilib/ultralib/include/PR -Ilib/ultralib/include -Ituroksnd/abi/buildss` + game defines) + skip
    audio_lib_stub.c; tracked alReverbSetType stub + tracked turokBnkfNew + disk bank loader; audio.c load
    SFX+SEQ banks from disk via turokBnkfNew + REMOVE the early-return so amCreateAudioMgr/alCSPNew/alSndpNew/
    SortSounds create the players. Verify rc=0 (players exist, banks parse, per-frame al* now safe), still
    SILENT. Then S3 drives the synth on the audio thread (amgrPumpOneFrame → alAudioFrame), S4 the Acmd mixer.
  - **★ S2 DONE (2026-06-17, commit 724409b, branch `audio-s2`).** Executed the atomic commit exactly as planned:
    `build_port.sh` skips `audio_lib_stub.c` + compiles all 105 `turoksnd/abi/*.c` (the real `al*`); new TRACKED
    **`port/src/turok_audiobank.c`** provides `turokBnkfNew` (the bank swap+relocate during the walk, `flags`-guarded
    once-per-node), `turokAudioLoadBank` (disk loader — N64 ROM segments are zero-span aliases on host), and the
    no-op `alReverbSetType` (closed the one undefined-ref gap). `audio.c` (PLATFORM_PORT) loads SFX (`sfx.ctl/.tbl`)
    + SEQ (`src/PR/testbank.ctl/.tbl`) from disk via `turokBnkfNew` and REMOVES the early-return so amCreateAudioMgr/
    alCSPNew/the SFX player/SortSounds build real state. VERIFIED: warps 0/2000/6000/8000 patrol rc=0, 4 banks
    load+swap each, render unaffected; `SortSounds` walking `sfxBank->instArray[0]->soundArray` proves the swap is
    correct (a bad count/ptr would fault). Still SILENT — the audio thread runs S1 silence until S3 drives the synth.
    **NEXT: S3+S4** — a multi-agent Workflow mapped the classic-ABI synth→Acmd→mixer chain for the S4 mixer plan.
  - **★ S3+S4 DONE — synth-drive + classic-ABI Acmd software mixer (2026-06-17, commit c3713e1, branch `audio-s2`).**
    The audio thread now drives turok's real synth (`alAudioFrame`) + a from-scratch interpreter for the CLASSIC libultra
    Acmd list, running clean every frame (zero crashes). **`port/src/turok_amixer.c` (NEW):** emulated DMEM + the classic
    **SETBUFFER latch** (A_MAIN + A_AUX triples — A_AUX overloads its 3 fields as the envmixer's MAIN_R/AUX_L/AUX_R) + a
    16-case dispatch decoding the packed Acmd by hand. DSP (VADPCM/resample/envmix/mix/interleave) lifted from PD `mixer.c`
    SCALAR paths, but each op pulls its DMEM in/out/count from the preceding `aSetBuffer` (verified vs load.c/resample.c/
    env.c/save.c/mainbus.c), not inline args; classic `aSetVolume` written fresh (PD's n_audio flag packing differs);
    A_POLEF stubbed (AL_FX_NONE). **`audiomgr.c turokAudioManagerFrame`:** one frame on the audio thread, bypassing the dead
    `__amMain` msg-loop + scheduler — `alAudioFrame` writes the REAL host out ptr (`info->data`) into A_SAVEBUFF, mixer
    renders straight there, `audioSetNextBuffer` pushes; `__amDMA` returns the sample ptr directly (no ROM-DMA buffering).
    **★ ADDRESS PLUMBING (#1 hazard):** `osVirtualToPhysical` (os_shim.c) + `K0_TO_PHYS`-family (R4300.h) are now IDENTITY
    under PLATFORM_PORT — the `0x1FFFFFFF` mask corrupts malloc'd bank pointers on host, so every DRAM addr word in the Acmd
    list stays a real host pointer. **★ READY-GATE:** `turok_audio_ready` (set at end of initAudio) — the audio thread starts
    before initAudio runs, so it pushes silence until the players/banks exist (fixed an early SIGSEGV on the uninit'd mgr).
    **★ WAV PACER:** the headless WAV sink now models a device draining at the sample rate (synthetic back-pressure) so the
    audio thread produces ~real-time instead of flat-out — fixed a **100x game slowdown** (synth-lock starvation from
    over-production; 400 frames 4min→0.5s). VERIFIED: warps rc=0, synth+mixer every frame no crash. Output SILENT because the
    SFX dispatch (`scene.c CScene__DoSoundEffect`) is still gated off → no active voices. Debug: `TUROK_AUDIO_DUMP` (opcode
    histogram). **NEXT: S5** — un-gate `CScene__DoSoundEffect` (CROMSoundElement/CROMEnvelope endianness) → first audible
    sound + validates the mixer (a 4-agent Workflow is mapping the SFX-path endianness + safe un-gate).
  - **★★ S5 DONE — FIRST AUDIBLE SOUND + mixer validated end-to-end (2026-06-17, commits 44c9ad7 + 1125b0f, branch
    `audio-s2`).** Un-gated the game's SFX dispatch: `scene.c CScene__DoSoundEffect`'s `#ifdef PLATFORM_PORT return -1`
    became a `turok_audio_ready` guard. **`CROMSoundElement` endianness:** it's 15 consecutive big-endian WORDs (5 element
    fields: m_nSampleNum/m_nDelayTime/m_Priority/m_wFlags/m_Probability, then 2 `CROMEnvelope` × 5 WORDs each — SF==WORD;
    romstruc.h:46/1233/1247) + 2 endian-safe trailing BYTEs → `turokSwapSoundElement` swaps a per-trigger STACK COPY. A
    copy is SAFE because the whole SFX path is **read-only** on pElement: `SetCFXPitch`/`SetCFXVolume` copy the envelope to a
    channel buffer (`*pEnv = pElement->m_Pitch`) BEFORE mutating, and `initCFX`/`PlayEnvironmentSound` copy field values
    (`sfxnum = pElement->m_nSampleNum`); none store the pointer. (4-agent Workflow hit the session limit → did it solo.)
    **VERIFIED (warp 0 + fire, headless `TUROK_AUDIO_WAV`):** rc=0, the SFX renders as REAL audio — smooth waveform
    (mean-delta/peak 0.06-0.09 = tonal, not noise), mono/centered (L==R), clean bursts. **Mixer proven CORRECT** via the
    `TUROK_AUDIO_DUMP` envmixer diagnostic: **inpk=31603** (96% full-scale — VADPCM decode + resample emit full-amplitude
    samples), **voldry=32767** (full), gain ramps to the synth-requested target. The quiet per-SFX peak (~1.6-20%) is the
    AUTHORED level, NOT a bug: `SetCFXVolume` sets `pSound->Vol = CFXVolumeTable[dbVolume]` (the SFX's authored dB) and the
    SFX master defaults to 255/255=1.0 (options.c:224) — the port plays each SFX exactly as the N64 would. **The classic-ABI
    software audio mixer works end-to-end (ADPCM→resample→envmix→interleave→save).** play_level.sh (GFX=sdl2) plays SFX
    through the speakers (same sink). **NEXT: S6** — music (the CSP sequence player `csplayer.c`; same synth/mixer chain, no
    new opcodes — needs the sequence start + the seq-bank already loaded in S2). Pitch reads slightly high in the
    zero-cross estimate (unreliable for broadband SFX; the full-scale smooth waveform argues the decode/resample are fine —
    confirm by ear once music lands or via a known single-tone SFX).
  - **★ AUDIO PACER FIXES + reliable HEADLESS audio testing (2026-06-17, commits 8bb8da0 + de7c690).** Two fixes so
    headless audio capture works (and they improve the SDL device path): (1) **sink-agnostic real-time pacer** — the
    audio thread had no back-pressure on the NOAUDIO/null sink (the WAV pacer only covered the WAV path), so it
    over-produced flat-out, spun a core, hogged the synth lock, and starved the game (a NOAUDIO patrol crawled to
    ~6fps). `audioSetNextBuffer` now accumulates `s_produced_bytes` + a start stamp; `audioGetBytesBuffered` (no SDL
    device) returns produced-minus-drained-at-the-sample-rate as synthetic back-pressure → the thread paces to ~real
    time for ANY non-device sink. (2) **`audioEndFrame` frame-drop fix** — it re-checked the back-pressure before
    flushing, but the producer loop already gates on that; with the pacer holding the buffer at ~LIMIT, nearly every
    frame was DROPPED at the boundary (a paced 10s capture yielded ~0.4s). Back-pressure belongs in the producer, not
    the flush — once produced, always write/push (also stops the SDL device dropping boundary frames). (3) **headless
    present pacing** — `os_shim.c osViSwapBuffer` (the EGL present, which the SDL2 `osRecvMesg` pacer never reaches)
    gains TUROK_FPS pacing (default 0 = unbounded; existing render tests unaffected). The audio thread renders in real
    time, so to capture SFX/music headlessly the game must too: **TUROK_FPS=30 paces it**. VERIFIED: paced fire test
    (TUROK_FPS=30, 300 frames = 10s) captures the weapon SFX — peak 10960 (33% full-scale, loud), 92% active. The
    classic-ABI mixer is fully validated headless. **★ HEADLESS AUDIO TEST RECIPE: `TUROK_FPS=30 TUROK_FAKEINPUT=5
    TUROK_AUDIO_WAV=x.wav` + a frame count = seconds×30.** (User gameplay unaffected throughout — the SDL device
    queue is the real back-pressure; these are headless-test fixes.)
  - **★ S6 (MUSIC) SCOPED — root cause found, gated re-enable (2026-06-17, commit 50f8aec, branch `audio-s2`).** Music
    never plays because the leaked source STUBS music loading off: `audio.c LoadSeq()` has an unconditional
    `return FALSE;` (the `if(!cache_is_valid)` guard is commented out → the real `CScene__RequestBinaryBlock` is dead
    code). Confirmed via `TUROK_SEQLOG` (traces UpdateSeq's seq state machine; values: SEQSTATE_IDLE=16/FADE=32/LOAD=64/
    LOADING=128/PLAY=256): music levels DO request music (warp 6000 MusicID=5, 3000=14, 8000=8) but it never loads
    (cur=-1). Re-enabling LoadSeq makes the sequence LOAD but then **SIGSEGVs in `alCSeqNew` (via SetupSeq)** deref'ing a
    garbage ptr — the big-endian ALSeq is parsed RAW (same class as the banks needing turokBnkfNew). **S6 NEXT STEP: a
    turok-style sequence endian-swap (a `turokCSeqNew` analogous to `turokBnkfNew`) before alCSeqNew** — find the
    ALSeqFile/ALCSeq struct layout, swap the header/track offsets, then verify CSP playback. The re-enable is gated
    behind **TUROK_MUSIC=1** (default off = the original stub → SFX work, music levels don't crash). Then merge
    audio-s2 → master.
  - **★ SFX WRONG-SPEED FIX — output rate must equal the 44100 bank rate (2026-06-18, commit dc6c810, branch
    `audio-s2`).** User: SFX play "all wrong and at the wrong speed" (suspected the N64 "chopped & screwed" reduced-rate
    sample trick). A 4-agent cross-engine Workflow (turok synth + PD + banjo + SM64/Ship-of-Harkinian) nailed it: the
    classic N64 libaudio synth computes a voice's resample ratio as a PURE musical ratio (`2^(cents/1200)`) and applies
    **NO runtime sampleRate/outputRate correction** — the sample-rate factor is baked into the keymap (`keyBase`/`detune`)
    at bank-BUILD time, on the contract that **the synth output rate == the bank's recorded sampleRate** (so ratio=1.0 =
    native). Nothing in `turoksnd/abi` reads `ALBank.sampleRate`; turok's SFX player even ignores keyBase/detune (uses
    `state->pitch` directly). PD honors this (output 22020 = its bank 22020); SM64/SoH add a runtime `32000/gAiFrequency`
    term turok's classic ABI LACKS. **The bug:** turok's banks (sfx.ctl/testbank.ctl) are authored at **sampleRate=44100**
    (verified in every ALBank header via xxd) while OUTPUT_RATE/device were **22050** — a 2:1 mismatch with no correction,
    so every 44100 sample played 1:1 at 22050 = half speed, an octave low. **Fix:** `OUTPUT_RATE` (audio.h, PLATFORM_PORT)
    + the SDL device `AUDIO_RATE` (port/audio.c) → **44100** (match the bank). ratio=1.0 now plays native + the envelope
    timebase (`_timeToSamples` uses outputRate) stays consistent. Verified headless: captured SFX pitch DOUBLES (dominant
    ~10.4kHz→~20.7kHz, exactly 2x), rc=0, no heap issue at the higher rate. Also fixes music pitch (the seq bank is 44100).
    **LESSON: N64 audio banks bake sampleRate/outputRate into keyBase/detune at build time — the runtime synth is
    rate-agnostic, so the HOST device + synth output rate MUST equal the bank's `ALBank.sampleRate`, or all sound plays
    at a uniform wrong speed (a global octave shift = the giveaway vs a per-sample tuning error).** User to confirm by ear;
    if a sound is still wrong beyond SPEED, that's a separate sample-selection/decode issue.
  - **★★ DEV AUDIO BANKS ARE PLACEHOLDERS — load the REAL banks from the retail ROM (2026-06-18, commit 02b3b4f, branch
    `audio-s2`).** After the rate fix the user heard the right SPEED but nonsensical SPORTS-ANNOUNCER phrases ("he took a
    big chance there", "right on the concrete floor") — Iguana placeholder/scratch samples (Iguana also made sports
    games), NOT Turok sounds. Confirmed: the dev tree's `src/PR/tengine/sfx.ctl/.tbl` are NOT present anywhere in the
    retail ROM, and the ROM holds the REAL banks (found by scanning for the ALBankFile `42 31`="B1" revision + walking the
    ALBank tree, boundaries cross-checked vs 16-byte segment alignment): **SFX bank @ 0x667230 (.ctl 0xb2d0) + sfxtbl @
    0x672500 (~1.1MB) = 228 sounds; music bank @ 0x626dd0 (.ctl 0x1580) + seqtbl @ 0x628350 (~256KB) = 24 instruments;
    both 44100 Hz** (US v1.2). New `turok_audiobank.c turokAudioLoadBankFromROM(rompath, offset, size, expectBank)` reads
    a chunk straight from the ROM file, validating the "B1" header so a wrong offset / non-v1.2 ROM falls back to the dev
    bank. `audio.c` SFX+SEQ loads prefer the retail ROM banks when **TUROK_ROM** is set (Path B = play_level.sh default),
    else the dev placeholder. The cart sound-elements come from the same ROM (Path B) so their `m_nSampleNum` indices
    match the 228-sound bank. Verified: both banks load, SortSounds walks the 228-sound bank rc=0, all warps rc=0. **With
    the 44100 rate fix + the retail bank, SFX should now be the correct Turok sounds at the correct speed (user to
    confirm).** LESSON: a leaked DEV tree's bundled audio (`sfx.ctl/.tbl`) can be SCRATCH/PLACEHOLDER from another title —
    the shipped assets live in the retail ROM as plain `B1` ALBankFile segments; find them by signature-scan + ALBank-tree
    walk, not by trusting the dev files. (Python ROM-scan: find `b'\x42\x31'` with sane bankCount/instCount/sampleRate.)
  - **★ RATE FIX REVERTED to 22050 once the retail banks landed (2026-06-18, commit 5dce26b).** The 44100 rate fix
    (dc6c810) was correct for the dev PLACEHOLDER bank (44100-stored), but the RETAIL ROM banks store samples at **22050**
    — so at 44100 the real SFX played 2x too FAST (user-confirmed). Reverted `OUTPUT_RATE` (audio.h) + `AUDIO_RATE`
    (port/audio.c) to the shipped 22050. **KEY CORRECTION to the rate-fix lesson above: `ALBank.sampleRate` (44100 in BOTH
    banks) is the RECORDING rate, NOT the playback/stored rate — do NOT match the output rate to it. The synth plays
    ratio=1.0 native, so device+synth rate must equal the bank's STORED sample rate, which here is 22050 (the shipped
    OUTPUT_RATE).** The dev placeholder happened to be 44100-stored, which sent us on the 44100 detour; the retail bank is
    22050. Verified: device opens 22050, retail banks load, rc=0.
  - **★ AUDIO MERGED TO MASTER + SFX latency cut (2026-06-18).** The whole audio stack (S2 libaudio+banks → S3+S4
    classic-ABI mixer → S5 SFX un-gate → pitch/rate fix → retail-ROM banks) is merged to **master** and pushed
    (origin f1e6f8b), `audio-s2` deleted — single branch again. **User-confirmed: real Turok SFX play correct
    (content + pitch + speed).** Then cut the SFX trigger LATENCY: the audio thread buffered `AUDIO_QUEUE_LIMIT`
    samples ahead of the device, so a freshly-triggered sound sat behind it — 8192 (PD's value) = ~371ms @22050,
    audible as a ~1/4s delay. Reduced to **2048 (~93ms)** (commit 5643e95, on master+origin); the thread refills
    every ~2ms so it stays clear of underrun. Drop to 1024 (~46ms) if even lower latency is wanted (only risk is
    crackle on a busy host). **S6 music is now DONE — see the next bullet.**
  - **★ MUSIC (S6) WORKS — `ALCMidiHdr` endian-swap (2026-06-18, branch `audio-music`).** The compressed-sequence
    player (CSP) SIGSEGV'd in `alCSeqNew` (cseq.c:47,60) because the on-disk **`ALCMidiHdr`** (16× u32 `trackOffset`
    + 1× u32 `division` = 68 bytes) is **big-endian**: `alCSeqNew` reads each track offset raw and walks `base+offset`
    into a track pointer → wild pointer → fault. **Fix:** `port/src/turok_audiobank.c turokCSeqHeaderSwap()` swaps the
    17 header DWORDs in place, called in **`audio.c SeqReceived`** right after the `memcpy(SeqBuffer,...)` (one event,
    before `alCSeqNew`; `SetupSeq`/`alCSeqNew` stay STOCK — putting the swap at the load decouples it from the parser
    and can't double-swap). The compact-MIDI **event stream** after the 68-byte header is byte-oriented (status /
    var-len delta / data + byte-assembled tempo & loop offsets) — **endian-neutral, stays raw** big-endian (same rule
    as the VADPCM `.tbl` payload). Music loading was **stubbed OFF in the leak** (`LoadSeq` `return FALSE` — the dev
    tree shipped with music-load disabled); re-enabled under PLATFORM_PORT, **default-ON** (`TUROK_MUSIC=0` disables).
    The sequence is a cart binary block (RNC-decompressed, keyed by **MusicID** type-key — Path B); the music
    instrument bank is `seqbankPtr->bankArray[0]` (retail seqctl @0x626dd0, 24 inst, swapped by `turokBnkfNew`, passed
    to `alCSPSetBank` in `SetupSeq`). Verified: **rc=0 + continuous music on warps 0/3000/6000/8000** (distinct MusicID
    0/14/5/8 = distinct tracks) **+ SFX coexist** (gunshot transients over the music bed), no `[CAMTRACK]`/`[CANARY]`.
    **LESSON: a compressed-sequence / MIDI header is just another fixed-size big-endian DWORD table (offsets +
    division) — swap it at load; the event stream past it is byte-oriented and endian-neutral. And a leaked DEV tree
    may ship a subsystem stubbed OFF (here music `LoadSeq return FALSE`) — re-enable + endian-fix it, don't assume the
    leak's default state is the shipped one.**
  - **★★ AUDIO-THREAD DEADLOCK (the c6e12dc fix was INCOMPLETE) — COMPLETED (2026-06-21, commit 1aad5fe).** User
    hit a freeze; the watchdog showed the MAIN thread STUCK in `audioSynthLock`(pthread_mutex_lock) inside
    `CEngineApp__UpdateGAME` = the audio thread was holding `s_synthLock` and spinning forever. Root: the N64
    serialises **~12** audio-event-queue critical sections (the per-frame SFX channel updates `alSndpSetVol/
    SetPitch/Play/Stop`, alloc/dealloc, CSP — in `audio.c` + `audiocfx.c`) by raising to `PRIORITY_AUDIOLOCK`
    via `osSetThreadPri`, a NO-OP in the cooperative port. **c6e12dc only converted 3** (DoSoundEffect,
    SetAudioVolume, SetupSeq); the other ~9 stayed unlocked → the game thread's per-frame SFX mutations raced the
    dedicated audio thread and spliced the libaudio event list into a CYCLE → the audio thread spins in the list
    walk holding `s_synthLock` → the game thread blocks on `audioSynthLock` forever. **Fix: every
    `osSetThreadPri(NULL, PRIORITY_AUDIOLOCK)` acquire now also `audioSynthLock()`, every `osSetThreadPri(NULL,
    ospri)` restore `audioSynthUnlock()`** — the recursive synthLock is the port's faithful replacement for the
    N64 priority lock at ALL 12 sections. Balanced (each acquire matched by an ospri restore on every path;
    recursive mutex handles the nested PlayEnvironmentSound→SetCFX* case); `audioSynthLock/Unlock` are no-ops
    off-PLATFORM_PORT. **GOTCHA: `frontend.c:999` ALSO uses `PRIORITY_AUDIOLOCK` but for a frame-buffer DMA, NOT
    an audio-queue mutation — left UNWRAPPED (correct).** The watchdog now also backtraces the AUDIO thread
    (SIGUSR2 via `audioThreadSignal`) before exiting — the main-thread backtrace only ever shows it blocked on
    the lock, so the real loop is in the audio thread. Verified: heavy-audio stress (warps 0/3000/6000/8000,
    music + continuous fire, 30s real-time each, watchdog armed) all rc=0, NO deadlock, audio captured.
    **LESSON: when porting an N64 audio engine, EVERY `osSetThreadPri(PRIORITY_AUDIOLOCK)` critical section is a
    serialisation point vs the (now-real) audio thread — convert them ALL to the synthLock, not a sampling.**
  - **★★ AUDIO-THREAD RACE = the "locks up, no crash dump" FREEZE — FIXED (2026-06-18, commit c6e12dc, on master).**
    User: "the game has been locking up quite a bit.. just locking up no crash dump" + "music is playing but sdl2
    crashed". Root cause: the synth runs on the dedicated audio thread (`port/audio.c`) holding the recursive
    `s_synthLock` across `alAudioFrame`, but the **game thread mutates the SAME libaudio event queue every frame**
    (`CEngineApp__Main → SetAudioVolume → alCSPSetVol → alEvtqPostEvent`, and `UpdateSeq`/`SetupSeq` for music) and
    **never took the lock** — so the two threads raced on the `ALEventQueue` linked list, occasionally splicing it
    into a CYCLE → the audio OR game thread spins forever in the list walk = a hang with no crash dump (or a wild
    deref = "sdl2 crashed"). The tell: **music keeps playing** (the audio thread owns the lock + keeps producing)
    **while the game freezes**. CLAUDE.md had CLAIMED the synthLock was wired "at S3+" but a grep proved the game
    thread NEVER locked — the lock existed but only the audio side used it. **Fix (`audio.c` + `tengine.c`, all
    PLATFORM_PORT):** wrap the game-thread libaudio calls in `audioSynthLock()/audioSynthUnlock()` — `SetAudioVolume`,
    `SetupSeq`, the per-frame `UpdateSeq` call, AND `CScene__DoSoundEffect`'s critical section (the cfx_counter bump
    + the SFX event posts). ★ The **source itself documents this exact race** at `DoSoundEffect` (scene.c:2853-2894:
    "This function can be called by the audio thread... can preempt the game thread... corrupt CCartCache") and
    serialized it on the N64 via `osSetThreadPri(PRIORITY_AUDIOLOCK)` — a **no-op in our cooperative port**, so the
    synthLock is its direct port-equivalent (placed in the same PRIORITY_AUDIOLOCK window; both lock+unlock kept
    INSIDE the `if (nIndex != -1)` block so a no-match path can't unbalance the recursive mutex). VERIFIED: the
    freeze repro that reliably hung at frame ~3180 now runs clean past 5460 with SFX firing + music + the
    `TUROK_WATCHDOG` silent; a 2000-frame heavy-fire (`FAKEINPUT=5`) run exits rc=0, watchdog silent, 250KB of SFX
    audio produced (no deadlock, SFX fires through the lock). **Diagnosis tooling (kept):**
    `TUROK_WATCHDOG=1` (`turok_main.c`) arms a thread that `pthread_kill(main, SIGUSR1)`+`backtrace()`s the MAIN
    thread after ~4s of no `g_frame` progress — the only way to catch an infinite-loop spin here, since gdb-attach
    is blocked by yama `ptrace_scope`. **LESSON: any data the game thread shares with a real (non-cooperative)
    audio/render thread — here the libaudio event queue — MUST be taken under the SAME lock on BOTH sides; a
    one-sided lock is a no-op. A "music plays but the game freezes, no crash dump" symptom = a shared-structure
    race, not a deadlock (a deadlock stops the audio too).** Also hardened the render-INTERPOLATION angle wraps
    (`tengine.c`/`romstruc.c`) from `while(a>PI)a-=2PI;` to O(1) `turok_wrap_pi()` (commit e50814b) — a separate
    latent freeze class (a huge finite interp-delta angle spins the while-loop ~1e17×; the game's own angle funcs
    already had the `fmodf` fix but the interp loops I added did not).

- **★ ANIMATED-OBJECT RENDERING (Item 3, 2026-06-14) — objects were all invisibly at the origin; fixed.**
  Found via a multi-agent workflow + runtime gate-counting: every animated instance (enemies, AI_OBJECT_DEVICE_*
  platforms/elevators/doors, pickups, AND the player) decoded to `pos=(0,0,0)` with a garbage scale, so they
  all stacked at the world origin → culled/clipped → invisible. **Root: `m_vPos`/`m_vScale` are `CVector3`, and
  the generic `ORDERBYTES` macro is a NO-OP on aggregates (it only swaps 2/4-byte scalars). A big-endian float
  read raw on LE becomes a denormal (~1e-41, prints as 0.0) or junk.** Fixes (all romstruc.c / scene.c,
  PLATFORM_PORT):
  - `romstruc.c CGameObjectInstance__TakeFromROMObjectInstance` (~2623/2628): swap `m_vPos`/`m_vScale`
    COMPONENT-WISE (the aggregate ORDERBYTES never swapped them).
  - `romstruc.c CGameObjectInstance__CalculateOrientationMatrix` (~3095): the model bounding box
    `CROMBounds m_vMin/m_vMax` (big-endian floats) were read raw → garbage `m_BoundsRect` (~1e17) → object
    fails the `anim_bounds_rect` overlap test in `CScene__DrawInstances` → culled. Swap component-wise.
  - `romstruc.c` (~2605): added the `m_nVariation==-1` → `m_pEA=NULL` guard (devices have no enemy variation),
    matching the simple/static decoders; without it `Variations[65535]` is garbage. Plus NULL-`m_pEA` guards on
    the dependent derefs (`CalculateOrientationMatrix` m_CollisionHeight ~3042; the 3 `Draw` m_wTypeFlags3
    checks at ~1609/2318/8593).
  - `scene.c` warp spawn (~1600): the player is `pROMInstances[0]`; now that the decoder swaps `m_vPos`/`m_RotY`,
    the warp must FEED BIG-ENDIAN (m_WarpPoint.m_vPos is host-order → re-swap; m_RotY is big-endian → decode,
    normalize, re-encode) or the player double-swaps to a garbage camera and the world clip-rejects.
  Verified: world still renders (403 tris), objects now decode to real coords (e.g. an elevator at
  (-2602,823,-7984) scale 0.2).
  - **ENEMY/ACTIVATION THREAD — investigated end-to-end; the pipeline WORKS, position was the bug.** Runtime
    gate-tracing (debug tools added: `TUROK_SPAWNAT="x,y,z[,rotY]"` teleport in scene.c warp spawn, `TUROK_DRAWALL`
    in scene.c `CScene__DrawInstances` + romstruc.c `CGameObjectInstance__Draw` to bypass the bounds/view-volume
    culls) established: (a) ACTIVATION is fine — `m_bPlayerActiveFlags=0xff`, 114/115 objects active
    (`CScene__SetUpActiveFlags`/`CScene__IsActive` scene.c:60/3265); (b) every type reaches `CGameObjectInstance__
    Draw` with valid `m_nAnims`/`m_pceObjectInfo`/`m_pceAnim`, passes the synchronous re-request gate (romstruc.c:
    8639); (c) the MODEL GEOMETRY EMISSION WORKS — bypassing the two in-Draw culls (`CBoundsRect__IsOverlapping`
    @8757 + `CViewVolume__IsOverlapping` @8762) jumps tri1 ~1124→1509 (the object models DO emit triangles);
    (d) the culls correctly reject FAR objects (the level's objects are spread x=-6000..300, z=-5000..-8000, so
    few are near any one spot). So enemies/devices render when the player is near + aimed at them. Couldn't get a
    clean HEADLESS screenshot of one — the EGL capture has no mouse-look/pitch control so the camera looks at
    sky and ground objects sit below frame; needs interactive (mouse-look) verification. No remaining code bug
    found in this thread beyond the already-applied position/bounds fixes.
  - **STILL OPEN:** the WATER surface (per the workflow, Turok has NO dedicated water renderer — it's baked level
    geometry / transparent instances, scene.c:3461/3671 — likely the transparent-instance combiner/alpha in
    gfx_pc); the first-person WEAPON viewmodel + HUD (`C16BitGraphic` endianness, `TUROK_HUD` guarded off); and
    whether a SPECIFIC enemy at a given spot fails to draw (interactive test needed). 0xBE opcode = G_CULLDL, harmless.

- **★ PATH B — retail v1.2 assets (2026-06-15).** `port/src/romdata.c` now honors `TUROK_ROM=<retail .z64>`:
  it `fread`s `TUROK_CARTDATA_SIZE` (6,489,336) bytes from ROM offset **`0x1F00`** straight into
  `_staticSegmentRomStart` instead of loading the v49 `cartdata.dat`. **Byte-verified the same CIndexedSet
  format/size** (root word `0x0b` = 11 items, index size `0x38`), so the entire cart-cache / offset / RNC /
  `ORDERBYTES` path runs **unchanged** — i.e. **no struct drift** between v49 code and v1.2 data (this closes
  the last M5 leaf-decode risk empirically: the v49 `romstruc.h` structs parse retail content fine).
  `play_level.sh` gained `ROM=<file>` to select it.
  - **Validated the user's "alpha build" hypothesis.** Path A (`cartdata.dat`) and Path B (ROM `0x1F00`) are
    the *same size & directory format* but ~**99% different data**. Captured the SAME object (type `0x14`
    GENERICRED, at `-1820,360,-3850`) under both: v49 draws an **angular gray block**, retail v1.2 draws a
    **different, rounded/tapered model** — so the v49 leak ships **earlier/placeholder model art**; the retail
    ROM has the finished models. Object *types* are nearly identical between versions (retail adds a `0x135`
    device), so it isn't "missing objects" — it's the **model/texture art** that differs. Path B is how to
    play finished v1.2 content: `ROM=baserom.us.v12.z64 ./play_level.sh`.
  - **★ USER-CONFIRMED (2026-06-15): Path B fixes MISSING *LEVEL* GEOMETRY too.** A bridge/walkway platform
    near the warp=0 fire-pit spawn that was ABSENT in the v49 leak renders correctly (fully textured stone)
    under retail — so the v49 *level* data is also incomplete/earlier, not just the object models. This was
    one of the "missing platform" elements the user flagged from the retail reference video (yt 1:04). Net:
    Path B is the fix for the whole "alpha build is missing stuff" class (level geo + object art).
  - **Open follow-up (M6):** objects may render dull/gray under BOTH paths (world geometry textures
    correctly), so there may be a separate **object material/texture** issue — likely big-endian object UVs /
    vertex-colors / normals, or a combiner default — NOT version-specific. **Needs interactive verification:**
    headless static captures can't mouse-look, so creatures near a wall/boulder can't be isolated. Best
    confirmed by walking up to one with `ROM=… ./play_level.sh`.
  - **Spawn-creature finding (warp=0, retail):** the player spawns at `(-1837,358,-3290)` facing `RotY=0`;
    several type `0x14` (GENERICRED) creatures cluster `184–229u` away at `+X` (e.g. `(-1692,358,-3403)`).
    They **reach `CGameObjectInstance__Draw`** (so they're active + decoded), but in the headless forward/sweep
    views they're occluded by boulders / off the no-mouse-look frame — couldn't isolate one's pixels. The
    big dark rounded shapes in the sweep are LEVEL boulders (persist with `TUROK_ANIMOBJ=0`), not the creatures.
  - **New debug env knobs** (all PLATFORM_PORT, headless camera control): `TUROK_YAW=<rad>` (camera.c, adds to
    `RotYOffset`), `TUROK_CAMLOG=1` (prints player pos + RotY), `TUROK_OBJPOS=1` (romstruc.c — prints each drawn
    object's world pos + distance from player, ≤4000u), `TUROK_NOWORLD=1` (scene.c — skips
    `CScene__DrawEnvironment` so only animated objects draw, isolating creatures against the clear color). Join
    the existing `TUROK_PITCH`, `TUROK_DRAWALL`, `TUROK_ANIMOBJ`, `TUROK_SPAWNAT`, `TUROK_OBJLOG`, `TUROK_GFX_DUMP`.
  - **★ KEY GOTCHA for headless creature-hunting:** `TUROK_YAW` rotates only the *render camera* (`RotYOffset`,
    applied in `CCamera__Update` AFTER the frame's cull frustum is built from the real `RotY`). So objects the
    real frustum culled stay culled even when the debug camera "looks at" them — a yaw-swept capture of a
    side/behind object shows the clear color, NOT the object. To aim at a culled object you must turn the
    *player* (real `RotY`), which `TUROK_YAW` does not do. With `TUROK_NOWORLD` + default facing, all animated
    objects together emit only ~4 tris (just a near pickup/weapon) — the 4 spawn creatures (type `0x14`, ~190u
    at `+X`) are correctly side-culled. **Net: confirming creature ART needs INTERACTIVE play (mouse-look),
    not headless captures.** Walk east/right of the warp=0 spawn toward `~(-1650,-3350)` to reach them.

- **★ WATER / TRANSLUCENCY — investigated end-to-end (2026-06-15, 5-agent Workflow + empirical traces); the
  pipeline is CORRECT, warp 0/1 simply have no water.** A multi-agent workflow mapped the whole translucent
  path (engine submission → Fast3D render-mode decode → N64 blend spec → GL realization) and found the
  Fast3D→GL blend chain **provably correct**: the build is `F3DEX_GBI` (not GBI_2), so `gSPSetOtherMode`'s
  `C0(8,8)/C0(0,8)` decode lands the render-mode word in `other_mode_l` intact; the 2-cycle `XLU_SURF2`/
  `CLD_SURF2` water modes set `use_alpha=true` (`gfx_pc.cpp:1598`, bits[21:20]=CLR_MEM, [17:16]=1MA) → GL
  `glBlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA)` + depth-mask-off; texture alpha imports standard. Confirmed
  empirically with three new env-gated traces:
  - `TUROK_MATLOG` (geometry.c `CGeometry__DrawSection`) — counts world sections + prints `m_dwMatFlags`/prim
    RGBA for any with `MATERIAL_TRANSPARENCY`(0x100)/`SHADE_ALPHA`(0x1000).
  - `TUROK_BLENDLOG` (gfx_pc.cpp before `use_alpha`) — per-draw `other_mode_l/h`, `use_alpha`/`invisible`/
    `alpha_threshold`/`2cyc`, with running totals.
  - `TUROK_XINSTLOG` (scene.c `CScene__DrawTransparentInstances`) — peak transparent-INSTANCE count/frame.
  - **Findings:** warp 0 (fire-pit) & warp 1000: **0 transparent world sections AND 0 transparent instances**,
    even walking 150 frames — there is no translucent water there. ~1400/7200 draws/frame ARE alpha-blended
    (sky CLD_SURF + alpha-tested foliage `texedge`), **0 invisible**. Warps **3000/4000/6000 DO have transparent
    sections** (`matFlags` 0x54c/0x56c/0x148, all with bit 0x100) with **non-zero** prim alpha (e.g. 78,77,91,255
    bluish-gray two-sided = water; 0,0,0,173 = 68% tinted) — so the flag decode AND alpha values are fine
    (rules out the "flag lost" and "alpha=0" hypotheses). warp 6000 visibly renders translucent cyan energy
    bars. **CONCLUSION: water rendering works; the user's "missing water" at the fire pit was a v49 artifact
    (same class as the missing platform) — the spot has no translucent geometry in EITHER version, so any water
    there is opaque animated-texture geometry that Path B restores like the platform, or it's elsewhere in the
    level. Not a translucency bug.** (Trace knobs left in, gated.)

- **★ HUD RENDERS (2026-06-15) — `C16BitGraphic` endianness fixed; HUD now ON by default.** A multi-agent
  Workflow (repro agent pinned it before a session-limit killed the others) found the fault: the HUD overlay
  graphics (`HealthOverlay[]`, digits, lives, etc.) are **hand-authored BIG-ENDIAN static C arrays** (under
  `src/PR/tengine/overlay/**`, `extern UINT8[]` in `gfx16bit.h`) interpreted through `C16BitGraphic`
  (4×UINT16 header: BlocksAcross/Down/Width/Height) and `C16BitPart` (2×UINT32 header: BlockWidth/Height).
  Read raw on LE, `m_BlockWidth` became `0x20000000` → the per-block advance `(w*h*2)+(w*h/2)+8` jumped ~5e17
  bytes → wild pointer → SEGV at `onscrn.c:3202` (`COnScreen__Draw16BitGraphic`, drawing HealthOverlay).
  **Fix (onscrn.h/onscrn.c):** added read-time swap macros `ONSCRN_SW16/SW32` (identity off-PLATFORM_PORT;
  the blobs are const, so swap on read, never in place) and wrapped the header reads in
  `COnScreen__Draw16BitGraphic`, `COnScreen__Draw16BitScaledGraphic`, and the `AIR_XPOS`/`BOSS_XPOS` centering
  macros (`m_Width`). The 16bpp RGBA / 4bpp opacity **pixel** payloads stay big-endian (Fast3D converts at
  `gDPLoadTextureBlock` upload — verified, colors correct). The `CGridGraphic` `DrawGrid*` consumers are dead
  (`#if 0`). **`tengine.c` HUD gate flipped to default-ON** (was `TUROK_HUD=1` to force; now `TUROK_HUD=0`
  disables, for clean geometry captures). Verified: warp 0 shows Turok's face + life-force "600"; warp 3000
  adds the lives "x2"; 150 frames clean across levels, digits render. (`glerr=0x501` seen only in the
  EGL capture-readback path, not from HUD textures — transient, non-fatal.)

- **★ LEVELS 2-8 LOAD — RESOLVED (2026-06-15).** The old "warp 2000+ stalls at frame-pump frame 3" is GONE
  (it was the angle-wrap infinite-loop hang the `fmodf` O(1) fix already cured). All 9 warps (0,1000,…,8000)
  now load + render real geometry in ~0.1s headless (page-cached assets): warp 2000=239 tris, 5000=416,
  8000=645, player spawns, HUD draws, exit rc=0. Level 2's 5786 regions are no longer a problem. Verified with
  retail (Path B) assets.
- **OBJECT/CREATURE VISIBILITY — pipeline sound; final confirm needs interactive play.** Added `TUROK_FACE=<rad>`
  (scene.c warp spawn — adds to the PLAYER's spawn RotY, rotating the *real* cull frustum, unlike `TUROK_YAW`
  which only spins the render camera) and `TUROK_VTXLOG` (gfx_pc.cpp — prints transformed clip x/y/z/w + NAN/
  BEHIND/HUGE flags for the first verts; with `TUROK_NOWORLD` these are all object verts). Findings: animated
  object models DO emit geometry (`NOWORLD+DRAWALL` = 391 tri1 calls), but most clip-reject because `DRAWALL`
  force-draws FAR objects (verts at w≈4700, z/w≈1.02 = correctly far-plane-clipped — NOT a bug; objpos only
  lists ≤4000u). Near creatures (d~190) are correctly view-culled when faced away; couldn't get a clean
  headless pixel-confirm of one (near/far conflation + no mouse-look + model-stream timing). **Net: rendering
  logic is intact; whether a given creature looks right is best verified by walking up to one interactively.**

- **★★ ENEMIES / OBJECTS / PICKUPS NOW RENDER (2026-06-15) — the user's "no monkey/powerups/key, triggers
  dead" report, root-caused to TWO bugs (chained).** Interactive play showed NO animated objects at all
  despite earlier headless analysis claiming "pipeline sound." A gate trace (`TUROK_GATELOG` in romstruc.c
  `CGameObjectInstance__Draw` @8758) showed enemies were **in view** (`boundsOverlap=1 viewVol=1`) but culled
  by `AI_VISIBLE=0`. Two fixes:
  1. **`CEnemyAttributes` (Variations) endianness — the deferred M5 "#6".** scene.c `CScene__ObjectAttributesReceived`
     casts the loaded block straight to `CEnemyAttributes*` and uses it RAW (big-endian). The AI type-flag tests
     (`m_dwTypeFlags`/`m_wTypeFlags3`/`m_InteractiveAnim` etc.) read garbage → `CAIDynamic__ResetInteractiveAnim`
     never set `AI_VISIBLE` → every enemy/pickup culled, and triggers/AI misbehaved. **Fix:** swap each variation
     in-place once in `ObjectAttributesReceived` (16 contiguous 4-byte fields = 13 floats + 3 DWORD flags, then
     WORD `m_wTypeFlags3` + 4 shorts; the 6 trailing bytes are endian-safe). Spec = the `#ifdef WIN32`
     `CEnemyAttributes__TakeFromVariation` (aistruc.c). → `AI_VISIBLE=1`, enemies pass the cull.
  2. **★ `adpcmDecode` was a NO-OP AUDIO STUB — collapsed every animated model.** `adpcmDecode` (the ANIMATION
     keyframe ADPCM decompressor for per-node quaternion `CRotFrame` + position streams, anim.c) lives in
     `adpcm.s` (MIPS asm, can't compile on host); the symbol resolved to a no-op stub in `audio_lib_stub.c`
     (`int adpcmDecode(void*,int,void*,void*)`). So every node rotation decoded to a ZERO quaternion →
     `CQuatern__ToMatrix` → zero 3×3 → all model verts collapsed to one point (confirmed via `TUROK_VTXLOG`/
     `TUROK_MTXLOG`/`TUROK_QCLOG`: orientation matrix valid, but node MUL matrices had zero 3×3). **Fix:** ported
     the in-tree C reference `dosvers/decadpcm.cpp` → **`port/src/turok_adpcm.c`** (`s32 adpcmDecode(s16**,u8*,s32)`,
     bitstream, byte-by-byte so endian-safe; coeffs inlined from `adpcm2.h`); removed the bad stub. → node
     rotations are valid unit quats, models render. Verified: the spawn enemy renders as a real bipedal creature
     (warp=0 now 486 tris vs 405; no crash across warps 0/2000/6000).
  **New debug knobs (all PLATFORM_PORT, env-gated):** `TUROK_GATELOG` (per-type draw-gate result),
  `TUROK_QLOG` (orientation quats + mfOrient 3×3), `TUROK_QCLOG` (per-node rotation quats), `TUROK_MTXLOG`
  (gfx_pc matrix loads), `TUROK_VTXLOG` (gfx_pc vertex transforms + NAN/BEHIND/HUGE), `TUROK_FACE=<rad>`
  (scene.c — adds to the PLAYER spawn RotY, rotating the real cull frustum; unlike `TUROK_YAW`).
  **STILL OPEN:** still want a clean close-up of a creature's finished art (interactive); the enemy at spawn
  is small/distant headless. Pickups (shiny triangle powerups) + key + the pillar trigger should now work too
  (same `AI_VISIBLE`/anim path) — user to confirm interactively.

- **★ CAMERA/HUD CORRUPTION-WHILE-WALKING (2026-06-15, overnight) — 3 render-state-leak fixes.** User report:
  "camera completely fucked up when I press W," SUSTAINED, a regression that appeared when the animated-object
  draw was enabled; **`TUROK_ANIMOBJ=0` = completely stable** (user-confirmed). Exhaustively ruled out (NaN/huge
  matrix+vertex detectors silent; modelview stack balanced+bounded headless; viewport/scissor stable; no node
  OOB). A **6-agent Workflow** mapped every non-matrix render-state vector; the convergent classes (all:
  camera/HUD, interactive-only, ANIMOBJ-gated, NOT-NaN so detectors stay silent, NOT headless-reproducible
  because a fixed/short headless capture never hits the trigger) were fixed:
  1. **Render/combine cache desync** (`geometry.c` `CGeometry__SetRenderMode/SetCombineMode`). The engine
     memoizes render/combine in game-side globals `current_render_mode/current_combine_mode` and emits NOTHING
     on a cache hit; that cache is a *separate shadow* of gfx_pc's PERSISTENT `rdp.other_mode_l/h`/`combine_mode`
     (camera setup re-issues projection/viewport/scissor each frame but NOT combine/othermode — they persist).
     A desync → a needed emit is skipped → wrong mode bleeds into world+HUD. **Fix:** on `PLATFORM_PORT` disable
     the memo — always emit (the globals are read ONLY for dedup; always-emit re-syncs gfx_pc every section; a
     visual no-op when synced). Also makes the masked-material alpha-compare (`DrawSection ~937`, emitted only on
     `SetRenderMode`-returns-TRUE) always re-emit.
  2. **★ `fx_mode` LEAK — CONFIRMED firing at warp 6000 (Campaigner boss), value `FXMODE_TOCOLOR`.**
     `CGameObjectInstance__PreDraw` (romstruc.c:9003+) sets the file-global `fx_mode` (geometry.c:28) to GLARE/
     TOTRANSPARENT/TOCOLOR for an enemy in a special state (regenerating, dying-fade `TRANS_FADE_OUT_MODE`
     @8918). The reset (romstruc.c:8944 + PostDraw `fx_mode=NONE` @9217) is gated on `m_asCurrent.m_pceAnim`,
     which `DoAI`/`Advance` (8754/8763, **DoAI runs INSIDE the draw**) can NULL mid-draw — leaking a non-NONE
     `fx_mode`. It's a global never reset per-frame, so it bleeds into later object/weapon draws; `DrawSection`
     renders those as a **flat solid color** (proven: forcing `fx_mode=TOCOLOR` turns the weapon viewmodel solid
     red). The WORLD grid does NOT use `fx_mode` — so it's the animated **objects + first-person weapon** that
     render as solid-color blobs (sustained: frames 155-200+ leaked consecutively at warp 6000). **Fix:** reset
     `fx_mode=FXMODE_NONE`+`fx_color` in `CGeometry__ResetDrawModes` (the per-frame reset, tengine.c:1037 before
     DrawGAME + scene.c:2978 before HUD). `TUROK_FXLEAK=1` (default in `play_level.sh`) logs each caught leak.
  3. **Modelview matrix stack depth 11→64** (`gfx_pc.cpp`, `MODELVIEW_STACK_DEPTH`). DoDraw recurses the
     skeletal node hierarchy with paired `gSPMatrix(PUSH)`/`gSPPopMatrix`; deeper than 11 → push dropped but pop
     kept → camera-base matrix popped away → whole-frame camera+HUD corruption. `push_dropped`/`pop_underflow`
     counters read 0 only headless (shallow enemies, peak depth observed = 5); the imbalance warning is now
     ALWAYS-ON (capped). Pairs with the earlier pop-floor (pop never drops below the camera base).
  4. **★★ THE ACTUAL WARP-0 "camera fucked up when I press W" BUG — `acos` LOOKUP-TABLE ENDIANNESS (graphu64.c).**
     Found by adding a camera-ANGLE anomaly detector (`[CAMTRACK]`, camera.c) after the render-state ones stayed
     silent — it fired 12× at warp 0 while walking, with `|qGround|^2 = 6.488` (the ground-slope quaternion was
     NON-UNIT, should be 1.0; 48 at warp 8000). `graphu64.c acos()` is a 1024-entry float lookup table `act[]`
     stored BIG-ENDIAN (N64), read via `((float*)act)[i]` → byte-swapped GARBAGE on the LE host. acos feeds the
     quaternion blends `CQuatern__BlendThreshold`(669)/`GetCloser`(706) that update the player's `m_qGround`
     EVERY FRAME WHILE WALKING (romstruc.c:3934) → garbage angle → m_qGround drifts non-unit → a non-unit q in
     `qRotZ*qRotX*qRotY*qGround` (camera.c:792-797) SKEWS the view matrix = "camera completely fucked up." FINITE
     (sin/cos of garbage stay bounded) → all NaN/huge matrix+vertex detectors stayed silent; WALK-triggered (the
     blend only runs when moving) + SUSTAINED (non-unit accumulates). Same class as the documented `fmodf` bug —
     and acos garbage was likely the TRUE root of the earlier garbage-angle hangs the fmodf wraps band-aided (AI
     angles, head-tracking, particle alignment all call acos). **Fix:** byte-swap `act[]` to host order once,
     in place, on first call (graphu64.c, PLATFORM_PORT). **Verified: camera anomalies 12→0, |qGround|^2=1.0000,
     all 8 levels render clean rc=0, camera upright while walking.** The only `((float*)table)` cast in graphu64.c
     is `act` (no sibling lookup-table bugs; sin/cos/sqrt redefine to libc). ★ This is the fix the user wanted.
     ANIMOBJ=0 "fixing" it was a red herring overlap (skipping the player Advance also skips the qGround blend).
  New debug knobs: `[CAMTRACK]` (always-on camera-angle/qGround anomaly), `TUROK_FXLEAK`, `TUROK_RSLOG`,
  `TUROK_MTXSTACK`, `TUROK_FAKEINPUT=3` (forward) / `=5` (fwd+fire) / `=6` (fire) / `=7` (patrol), `TUROK_KILLALL`.
  **LESSON (re-confirmed): any libc-style math fn the N64 source reimplements with a STATIC FLOAT TABLE or that
  the host implicitly declares is an endianness/ABI trap — check acos-family + lookup tables for big-endian bytes.
  And when render-state detectors are silent on a "camera" bug, check the camera ANGLES/quaternions at the source
  (a wild angle's sin/cos stay finite, so it never shows as a NaN/huge matrix).**

- **★ FIRST-PERSON WEAPON — "Turok's leg up on the screen" (2026-06-16) — Object Types WORD-array endianness.**
  After the acos camera fix, the first-person view showed Turok's full standing BODY (a boot at eye level), not
  the weapon. Root: `CScene__LookupObjectType` (scene.c:2063) scans the Object Types block (`m_pceObjectTypes`,
  a big-endian WORD array consumed RAW — `CScene__ObjectTypesReceived` never swaps it); on the LE host every
  entry is byte-swapped, so the player's weapon-type lookup (100 = `AI_OBJECT_WEAPON_KNIFE`) never matches →
  returns -1 → `CScene__LoadObjectModelType` bails → the player keeps the BODY model stuck on anim 0 (idle), and
  the `FollowView` placement put its feet where the weapon should be. **Fix:** wrap the compare in `ORDERBYTES`
  (scene.c, identity off-PLATFORM_PORT). Now lookup(100)→object 664, the player loads the knife model (type
  0x64, 10 anims), hand+knife render lower-right. The cinema-mode guard in `LoadObjectModelType` is untouched, so
  the **full body still loads for cutscenes (death / key pickup / ending)** — the body draw is correct + needed
  there. `TUROK_VMLOG` logs the viewmodel type/anim/weapon + the `[lookup]` type→object result. Verified warps
  0/2000/6000/8000 rc=0, camera healthy, weapon loads. (Commit 3434348.) NOTE: the player briefly shows type=0x0
  for the first spawn frames until the async weapon model finishes loading — expected.

- **★ MISSING ITEM PICKUPS (2026-06-16) — Object Types REVERSE-lookup endianness (sibling of the weapon fix).**
  Item pickups (floating gold triangle/token powerups, health, ammo) never drew. Pickups are
  `CGameSimpleInstance` (the simple pool / static grid-section simples, NOT animated instances), drawn at
  scene.c:3749 / via `CSimplePool__Draw`. `CGameSimpleInstance__Draw` (romstruc.c:1593) bails unless
  `m_wFlags & SIMPLE_FLAG_VISIBLE` (1618), which `TakeFromROMSimpleInstance` (romstruc.c:1336) sets only when
  `AI_IsPickup(CInstanceHdr__TypeFlag(...))`. For simple/static instances `CInstanceHdr__TypeFlag` →
  `CScene__GetObjectTypeFlag` (scene.c:2114) which did `return objectTypes[nObjType]` RAW — the same big-endian
  Object Types WORD array, so on LE the pickup type came back byte-swapped (409/438 → garbage) → `AI_IsPickup`
  false → never VISIBLE → decodes + activates + reaches the draw call but emits nothing. **Fix:** `ORDERBYTES`
  the return (scene.c, identity off-port). This is the REVERSE companion to the forward `LookupObjectType` fix
  (commit 3434348, the weapon). Verified: warp 0 pickups now type 409/438, wFlags=0x2, render as gold floating
  pickups; warps 0/2000/8000 visible simples, rc=0, no regression. (Commit f23ed93.) Debug: `TUROK_SIMPLOG`
  (simple decode `[simple]` / draw-gate `[simdraw]` / pool `[simpool]`), `TUROK_INSTLOG` (`[inst]` anim types).
  **LESSON: the Object Types WORD array has TWO read sites — forward `LookupObjectType` (type→index, weapon
  swap) AND reverse `GetObjectTypeFlag` (index→type, pickup/simple/static visibility). Both need ORDERBYTES.**
  (Triggers/devices were confirmed WORKING — that part of the user report was a false alarm.)

- **★ ENDIANNESS AUDIT (2026-06-16) — 9-agent Workflow diffing decode vs the `#ifdef WIN32` encode spec; 26
  confirmed un-swapped reads. Fixed the HIGH/live ones (commit 9785b90); LOW ones are gated (M4/attract/option).**
  - **★ SHARED FIX — `defs.c BinarySearch`/`BinaryRange`**: the big-endian DWORD KEY TABLES (object anim types,
    particle/sound/binary-block type keys) were compared RAW vs native search keys → lookups never matched. Wrap
    each `Keys[]` read in `ORDERBYTES` (arrays stay big-endian, monotonic in NATIVE value). One edit fixes ALL
    four callers. ★ GOTCHA: `scene.c` warp-id BinaryRange had an in-place `ids[]` swap (WarpPointsReceived) —
    REMOVED it (BinaryRange now swaps on read; keeping it would double-swap → miss the warp point). When you fix
    a shared search/lookup, audit every caller for a pre-existing per-array swap that now double-swaps.
  - **ANIM TYPES (romstruc.c `GetAnimType`)**: `usAnimTypes[idx]` read raw → byte-swapped anim type →
    `IsAbsoluteAnim` wrong (root-motion) + LookupAIAnimType BinarySearch fails → enemies/objects mis-select idle/
    walk/attack/death anims. LIVE render path. ORDERBYTES the read (left big-endian to avoid double-swap w/ the
    BinarySearch fix).
  - **PARTICLES (particle.c `DecompressParticles`)**: the whole `CROMParticleEffect`+`CROMParticleImpact` payload
    read RAW. Once the type lookup started matching, particles spawned and `CParticle__Advance/EndLife` deref'd
    the byte-swapped `m_pImpact` index (`&impacts[wild]` → SEGV) with garbage SF physics. Now swap each effect +
    impact in place once, mirroring the WIN32 encoders (romstruc.c:8352/8373) field-for-field (m_pImpact+m_dwFlags
    DWORD, 6 short counts, ~32 SF=WORD physics; impact WORD/float arrays; BYTE color/behavior left raw). Verified:
    40+ particle effects spawn at warps 3000/6000, all 8 warps rc=0 (were SIGSEGV), zero garbage verts.
    `TUROK_PARTLOG` logs spawns.
  - **SOUND gated (scene.c `CScene__DoSoundEffect` early-return on PLATFORM_PORT)**: the BinarySearch fix made the
    SFX type lookup succeed → reached the M4-deferred audio path (no audio mgr → NULL deref in
    PlayEnvironmentSound). Gated until M4 — which must ALSO ORDERBYTES `CROMSoundElement`/`CROMEnvelope` + add the
    NULL-bank guard (audit ranks #6/#7).
  - **DEFERRED (gated, fix when their subsystem lights up)**: CROMSoundElement/CROMEnvelope (M4 audio, scene.c:2827
    +audiocfx.c); CAttractHeader 16-bit fields (attract.c:737, attract-only — also a stray `;` at scene.c:348 to
    fix); RGBA5551 palette recolor (textload.c:108, green-blood option-gated, default off). All documented in the
    audit output. **LESSON: an asset KEY TABLE (BinarySearch/BinaryRange over big-endian DWORD keys) and the
    STRUCT PAYLOAD it gates are a pair — fixing the key lookup ungates the payload, so fix/gate both together or
    you trade a silent-no-op for a SEGV (sound + particles both did exactly this).**

- **★ KEY-PICKUP / ALL-CINEMATIC COREDUMP — fixed (2026-06-16, commit 4b359e7).** Picking up any key (and every
  death/ending cutscene) crashed with a SEGV. Root: a cinematic ENABLES the player's per-frame collision
  (`romstruc.c CGameObjectInstance__Advance` runs `Collision3` for the player only when `CCamera__InCinemaMode`),
  and the camera (`KeepBelowCeiling`), minimap (`RevealMap`) and AI reset (`CAIDynamic__Reset`) all run collision
  queries against the player's region. The **M5 collision parse is still incomplete**, so some regions carry
  corner pointers that are **NULL**, **stale** (the cinematic's model-swap asset loads evict/move the collision
  buffer the region's `m_pCurrentRegion` points into → dangling `m_pCorners`), or **uninitialised poison**
  (`0x14141414`). **N64 (no MMU) tolerates these as harmless low-RDRAM reads; a protected host faults** — the same
  null-tolerance class as the title-screen guards. Diagnosed by chasing the crash through ~7 distinct
  corner-deref sites (regicol edge loop → unicol TrackGround → SetCameraToTurok → unicol:249 → map DoRevealMap →
  ClearRecurseFlags → GetCeilingNormal → CAIDynamic GetGroundHeight); the dummy-corner trace proved indices are
  **valid at parse time**, so it's a *runtime* stale/poison pointer, not a parse bug. Fixes (all PLATFORM_PORT,
  graceful no-collision / default fallback):
  - **`romstruc.h`** — `PORT_CORNER_BAD(rgn,i)` / `PORT_REGION_BAD(rgn)` macros: a corner pointer is bad if NULL,
    N64-range (≥0x80000000), or **>16MB from its region** (a level's whole collision blob is only a few MB, so the
    window accepts every real corner yet rejects wild/poison pointers — the distance-from-region heuristic works
    because corners+regions share one `pBytes` buffer; **256MB was too loose, let 0x14141414 poison slip through
    ~190MB from a valid heap region → tightened to 16MB**). `(0)` off-port.
  - **`unicol.c CAnimInstanceHdr__Collision3`** — bail with no collision at the entry when the instance's region is
    `PORT_REGION_BAD`. **THE single chokepoint** for every collision caller (player Advance, cinecam
    `GetNearPositionAndRegion`, weapon `GetOffsetPositionAndRegion`).
  - **`romstruc.c`** — `GetGround/CeilingNormal` + `GetGround/CeilingHeight` fall back to flat-ground/ceiling
    defaults via `PORT_REGION_BAD` (replaced the older `TUROK_BADPTR`-only guards, which missed host-range
    garbage). `TakeFromROMRegion` now takes `nCorners` + bounds-checks the corner index, substituting a static
    zeroed dummy for any OOB/absent corner (belt-and-suspenders; doesn't engage on current data = indices valid).
  - **`tmove.c`** — skip the dead/cinema "drop to ground" physics (velocity + `Collision3`) on host.
  - **`tengine.c CEngineApp__SetCameraToTurok`** — NULL-guard `pRegionSet->m_dwFlags` (cinema forces
    `keepInSphere=TRUE` even when the region has no attributes → the N64-deliberate "let it crash" out-of-bounds
    snap faulted on host).
  - **`map.c`** (`RevealMap`/`DoRevealMap`) + **`regicol.c`** (edge loop) — skip `PORT_REGION_BAD` regions.
  Verified: all 9 levels (warp 0..8000) run the real `FadeToCinema` path 200 frames at rc=0; normal patrols +
  rendering unaffected (warp 0 ~587 tris, 160 colour buckets). **The real M5 fix is making the collision corners
  parse/relocate correctly + keeping the collision cache entry resident across cinematic asset loads; these guards
  are the host-null-tolerance stopgap until then.**

- **★ KEY-CINEMATIC "PLAYS THEN CRASHES" — root-caused + fixed (2026-06-16, commit d2ace13).** After the guards
  above, picking up a key showed the cinematic but then SEGV'd again. Runtime traces (CPickup_Pickup + per-frame
  cinematic + Collision3-bail, reproduced on warp 3000) nailed it: **at the pickup the player's region is valid
  (`regionBad=0`), but during the cinematic it becomes a DIFFERENT, BAD region** (`bad=1`). The key cinematic's
  model-swap (`CScene__LoadObjectModelType` → body model + `AI_ANIM_EXTRA10`) loads assets that **relocate/evict
  the collision buffer**, so the player's `m_pCurrentRegion` ends up on a region whose corner pointers are now
  NULL/stale. Collision3 bails (guarded), but the camera/map/region-attribute paths also deref the region and
  fault. **Root fix: `tengine.c CEngineApp__UpdateGAME` detects a `PORT_REGION_BAD` player region once per frame
  and NULLs it.** The engine already handles a NULL region everywhere (`if(!region)` → defaults) — it just never
  produced a bad-but-non-NULL one — so every downstream path degrades gracefully until the level reset re-spawns
  the player with a fresh region. Verified: key cinematic + reset completes rc=0 (cinematic now sees `region=nil`,
  no Collision3-bail spam); patrols on all 9 levels stay grounded, no false NULLing in normal play (warps
  4000/5000 NULL once on a transient streaming region — harmless, Y stays grounded). A capped `[KEYTRACE]` in
  pickup.c/tengine.c reports key pickups + when the band-aid engages. **The proper M5 fix remains: keep the
  collision cache resident (ResetAge) across the cinematic model-swap so the region never goes stale.**
  - **★ FOLLOW-UP: the NULL-on-bad caused a TELEPORT regression — fixed (commit b4d6892).** With the region
    NULLed, `Collision3` no longer bailed (`PORT_REGION_BAD(NULL)` is false), so it ran `CCollide__TrackGround`,
    which projects the player's position onto the **vZero fallback corner** (unicol.c:245) → snaps X/Z toward the
    world origin. The player visibly teleported DURING the key cinematic (user-reported regression; previously,
    the bad-but-non-NULL region made Collision3 bail → position held → correct). **Fix: `romstruc.c
    CGameObjectInstance__Advance` skips the PLAYER's collision while `CCamera__InCinemaMode`** (that block only
    runs for the player when in cinema mode). The animation still advances; the player holds `vOldPos` = the
    pickup position. Verified on warp 3000: player stays at the exact pickup point (-153,0,-536) through the whole
    cinematic; normal-play collision unaffected (cinema-gated), patrols grounded. NOTE: the key cinematic ends
    with `MODE_RESETLEVEL` → respawn at `CINEMA_WARP_ID` (-2) → `GetApp()->m_CinemaWarp` (captured at FadeToCinema
    = the pickup pos, camera.c:1650). So the post-cinematic respawn should also be the pickup point (host-order,
    no endianness) — user to confirm the FINAL position is right.
  - **★★ ROOT FIX (commit 0551634) — RE-ACQUIRE the stale region; reverts the NULL-on-bad + skip-collision
    band-aids.** The user (rightly) pushed back on the band-aid cascade (crash → teleport → fog-black + fall-
    through + crash-after). Traced the actual ROOT with `[COLL]` instrumentation: **the N64 STREAMS collision
    through the small cart cache** (`MEMORY_POOL_SIZE` 1.4MB), re-decompressing the collision buffer at a NEW
    address (`CScene__DecompressCollision` re-runs) as the player explores / when the key cinematic's model-swap
    loads the body model. **The engine never updates the player's `m_pCurrentRegion` after a relocation** → it
    goes stale (corners point into the freed/old buffer). NULLing it (d2ace13) lost the fog (region attributes)
    + dropped the player through the floor (no ground); skip-collision (b4d6892) then masked a teleport (NULL
    region → TrackGround projects onto vZero → origin). **CORRECT FIX: `tengine.c CEngineApp__UpdateGAME`
    re-acquires the player's region at its current pos in the LIVE buffer via `CScene__NearestRegion` when
    `PORT_REGION_BAD`.** Valid region → collision/ground/fog/camera all work normally; reverted skip-collision
    + NULL (net −13 lines); kept the 4b359e7 `Collision3` entry guard as the same-frame safety net (bails for
    the 1-frame window before re-acquire runs). Verified: player holds the exact pickup point (-153,0,-536) with
    a VALID region (0x87e4228) through the whole cinematic, rc=0; patrols on all 9 levels stay grounded. (Tried
    a 48MB cache first — DIDN'T stop the re-decompress, so it's not room-based; the streaming is by design.)
    **LESSON: when a host-port symptom cascades (each fix breaks the next thing), STOP and find the streaming/
    lifecycle ROOT — re-acquiring a relocated resource beats nulling/skipping around the stale handle.** ALSO
  REPORTED by the user (deferred): a **blue-portal warp bug** — entering a portal → bonus area, then re-entering
  → wrong-warps to the Campaigner boss instead of back. Warp/portal level-transition logic to fix next.
- **★ BLUE-PORTAL WARP BUG — investigated, endianness RULED OUT, instrumented for interactive capture (2026-06-18,
  on master).** Traced the dynamic-warp / store-and-return mechanic end to end (Explore agent + direct
  read). The warp triggers on the player COLLIDING with an `AI_OBJECT_WARP_DYNAMIC` (=600) simple-instance
  (`tmove.c:1356` → `warp.c CWarp__Warp`). For each: if `m_pEA->m_wTypeFlags3 & AI_TYPE3_RETURNWARP`(1<<0) →
  `CEngineApp__WarpReturn` (restore the saved point); else forward-warp to `m_pEA->m_Id`, optionally storing the
  return point if `m_dwTypeFlags2 & AI_TYPE2_STOREWARPRETURN`(1<<29). The return restore is `scene.c
  CScene__RequestWarpPoints:417` — `m_nWarpID==RETURN_WARP_ID(-1)` → if `GetApp()->m_ReturnWarpSaved` use
  `m_ReturnWarp` (host-order pos/RotY/level/region saved at `tengine.c DoWarp:2437`), else fall back to
  `m_WarpFound=FALSE,m_nLevel=0`. **RULED OUT:** (1) **endianness** — the three read fields (`m_Id` short,
  `m_dwTypeFlags2` DWORD, `m_wTypeFlags3` WORD) are ALL correctly byte-swapped in `scene.c
  CScene__ObjectAttributesReceived:912-921` (verified field-by-field); (2) **BOOL truncation** — `BOOL=int`
  (defs.h:127), so `returnWarp = flags2 & (1<<29)` doesn't truncate. So it's a **logic/data** issue, NOT the usual
  endianness class. Suspects needing RUNTIME data: the bonus return-portal's `RETURNWARP` bit not set in DATA (→
  treated as a forward warp to a boss `m_Id`), or `m_ReturnWarpSaved` cleared between entry and return. Can't repro
  headlessly (can't script walking into a trigger volume), so added a **`TUROK_WARPLOG=1`** trace at the 3 decision
  points (`warp.c` portal flags+`m_Id`+branch; `tengine.c CEngineApp__Warp` the invoked WarpID vs RETURN/CAMPAIGNER;
  `scene.c` RETURN restore `returnSaved`/level) — exactly the methodology that cracked the death-fall-through bug.
  **NEXT: user plays with `TUROK_WARPLOG=1`, walks the portal→bonus→re-enter sequence, and the trace pins which of
  the suspects fires.** Build+rc=0 verified; trace default-off, no behavior change. NOTE for 3DS/v1.2: CAMPAIGNER_
  BOSS_WARP_ID=8999, RETURN_WARP_ID=-1, bonus levels are warp IDs 9000-9999 (`tengine.c:2347`).
- **★ BLUE-PORTAL "OUT OF BOUNDS" — 6-agent Workflow + headless FORCEWARP repro (2026-06-18).** User's interactive
  `TUROK_WARPLOG=1` trace: the portal dispatches `m_Id=9600 STORE=1 RETURN=0` → `CEngineApp__Warp(9600,
  WARP_WITHINLEVEL)`. The Workflow (adversarial verify) RULED OUT (a) the region double-swap (masked by the
  per-frame `CScene__NearestRegion` re-acquire — an analyst empirically built the scene.c:1635 region "fix" and it
  BROKE warp 0) and (b) "geometry not reloaded" (`MODE_RESETLEVEL` → `CScene__Construct(m_WarpID)` DOES reload,
  selected by `m_WarpPoint.m_nLevel` at scene.c:471→1083 `nLevel %= GetBlockCount`). Added headless repro hooks
  (`TUROK_FORCEWARP=<id>` fires a within-level warp once after the level settles, since `TUROK_WARP` clamps to
  warp-point 0; `TUROK_FORCECHOICE=<n>` picks the n-th point when a warp ID matches multiple) + expanded the
  resolve trace (per-point dump of `[first..last]`). **KEY FINDING: warp ID 9600 has TWO warp points** (ids[184]==
  ids[185]==0x2580): `wp[184]`→level 26 `(-2296,-154,-459)` region 22, `wp[185]`→level 27 `(115,34,0)` region 59 —
  **DIFFERENT levels**, RANDOM-picked at `scene.c:458 choice=first+RANDOM(last+1-first)`. That's the "this time"
  variability. **BUT both land in VALID geometry in the headless FORCEWARP repro** (26 = a cave, 27 = a temple
  courtyard, captured) — so the random pick alone isn't the user's black void. FORCEWARP uses store=FALSE from the
  spawn; the real portal is store=TRUE from the walked-up position. **STILL UNREPRODUCED headlessly — NEXT: user
  recaptures `TUROK_WARPLOG=1` on the OOB run; the new `[WARP] resolve nWarpID=9600 FOUND choice=N vPos=... nLevel=N`
  + `RequestLevel` lines show the EXACT failing warp point + level + position.** Also note the secondary
  store/return inconsistency the Workflow flagged (m_ReturnWarp.m_nRegion host-order at tengine.c:2451 vs the
  spawn decode's ORDERBYTES) — likely the ORIGINAL "re-enter → Campaigner" report; fix once the forward OOB lands.
  ★ NFS GOTCHA hit again: an Edit to scene.c was silently lost (stale read-after-write) — re-add + `grep`-verify
  edits to the warp traces persisted before building.
- **★★ BLUE-PORTAL OOB — FIXED via a key gate (2026-06-18, root-caused + fixed headlessly).** Added `TUROK_FORCEPORTAL`
  (fire the real `CWarp__Warp` on a collision-list warp simple) + `TUROK_WARPTABLE` (dump the whole warp-dest table)
  + `TUROK_FORCEWARP`/`TUROK_FORCECHOICE`. The table shows the bonus/hub warp IDs **9100-9800 each have TWO
  destinations**, RANDOM-picked at `scene.c:458`. For **9600**: `wp[184]`→lvl 26 (the bonus cave) and `wp[185]`→lvl 27
  `(115,34,0)` — and lvl-27 `(115,34,0)` is the **exact same destination as the dedicated `id=9004` Campaigner-boss
  approach**. So entering the early bonus portal RANDOM-jumped to the boss (→ a 2nd auto-warp `9004` → reddish-fog
  void); this was BOTH the "out of bounds" AND the older "re-enter → Campaigner" report. The user confirmed the
  Campaigner is legitimately **key-gated** at the central hub (`aidoor.c PortalAI_MEvent_Start` opens each LevelN
  portal on `LevelN_Access >= MAX_KEYN`). **FIX (user chose "key-gate"):** new `tmove.c CTMove__HasAllKeys()` (all 7
  `LevelN_Access >= MAX_KEYN`); in `scene.c` warp-point selection, until all keys are in, **skip any matched
  destination that RE-LISTS a dedicated lower-id warp's destination** (same id-is-lower + same level + same x/y/z) and
  use the warp's own primary point. So 9600 → lvl 26 (bonus) until all keys, then RANDOM (boss reachable) — mirroring
  the hub gate. **GOTCHA in the fix:** the position match MUST compare all of x/**y**/z — `wp[184]` (lvl 26, y=-154)
  coincidentally shares x+z with a *different* lower-id warp `9018` (lvl 26, y=-171), so an x/z-only match wrongly
  gated the bonus; +0/-0 differs in raw bytes so compare the **ORDERBYTES'd (host) floats**, not memcmp. Only the
  multi-point warps (all ≥9100) are affected; single-point/in-level warps untouched. Verified headless: 5/5 runs 9600
  →lvl 26 (was RANDOM 26/27), `FORCECHOICE=1` still forces the boss (key-path intact), bonus renders (cave), patrols
  on warps 0/2000/6000/8000 rc=0 no anomalies. **User to confirm interactively** (walk the early portal repeatedly →
  always the bonus; the Campaigner only via the all-keys hub portal). LESSON: a "leaking" destination wasn't a parse
  bug — the cart genuinely lists the boss under the bonus id; the original gates it by KEYS (hub PortalAI), so the
  port must reproduce that gate in the warp-point selection, not just trust the data.
  - **★★ "PORTAL TAKES ME TO THE VOID" — CORRECTED: a REAL render-interpolation/warp REGRESSION (fixed 7bfd06c),
    NOT the "dark cave interior" I first concluded below. The (now-wrong) first investigation is kept verbatim as a
    lesson in how a TICK=0 headless capture MASKED it — the real root cause + fix follow it.**
  - **[SUPERSEDED — first, WRONG conclusion] "PORTAL TAKES ME TO THE VOID" = the DARK lvl-26 bonus-cave interior (2026-06-18).**
    After the key-gate, the user: "portal takes me to the void btw" (a pure-black screenshot with the HUD + weapon
    still visible). Root-caused headlessly: the warp resolves CORRECTLY to lvl 26 (`resolve nWarpID=9600
    choice=184 nLevel=26`, the key-gated bonus cave — confirmed in BOTH the SDL2 and EGL builds). An EGL capture of
    the **landing spot** (`TUROK_FORCEWARP=9600 TUROK_FAKEINPUT=0`, no walk, HUD on) shows the player arrives
    **facing a large dark cave-tunnel mouth**: side walls lit dim-green stone, pickup sparkles marking a path
    forward, but straight ahead is a deep UNLIT tunnel. Walk into it → surrounded by black (unlit) geometry while
    the HUD + weapon (screen-space) stay lit = EXACTLY the user's "void" screenshot. So it is **not** OOB / not a
    broken warp / not a missing level — it's the bonus cave's genuinely-dark interior. (Whether it's TOO dark vs
    the N64 — a fog/ambient/brightness question — is the separate open "brightness polish" item; the lighting
    isn't obviously broken since the walls light correctly and deep tunnels reading black is normal.) **HEADLESS-
    REPRO GOTCHA: the SDL2 backend can't be captured under Xvfb** — `xvfb-run` has no GPU, so its software-GL/
    capture path renders EVERYTHING black (level 1 fire-pit AND lvl 26 both came out pure black), so SDL2 captures
    are NOT diagnostic. Use the **EGL/GBM** build (real GPU, render-node, no X) for any headless pixel check; the
    user's real SDL2 runs on their GPU and renders fine. **LESSON: a "void" report is usually one of three —
    (a) wrong warp / OOB (ruled out by the resolve trace), (b) the v49-vs-retail missing-asset class (ruled out,
    EGL renders the cave), or (c) a genuinely dark area the player walked into. Capture the LANDING spot (no walk)
    in EGL to tell them apart before assuming a bug.** NOTE: an intermittent EGL segfault appeared in the lvl-26
    path during testing (attempt 1 crashed, attempt 2 clean) — most likely the documented GPU-wedge from repeated
    headless kill -9 runs, but watch for a real intermittent lvl-26 crash if it recurs.
  - **★★ THE ACTUAL ROOT CAUSE = RENDER-INTERPOLATION CLOBBERS THE WARP DESTINATION (2026-06-18, commit 7bfd06c) —
    the user was right, my "dark cave" call above was WRONG.** The user pushed back: "trust me... it's absolutely
    pitch black... the geometry either isn't rendering or I'm being dropped out of bounds... it RENDERED in an
    earlier commit so there's a regression." Re-investigated and REPRODUCED headlessly. ROOT: the player
    render-interpolation (smooth motion between 30Hz logic ticks, `tengine.c CEngineApp__UpdateGAME`) ran
    **regardless of warp state**, and its restore (~line 4903) UNCONDITIONALLY wrote `m_vPos = _ipCurPos` (the last
    logic-tick snapshot). On **render-only frames** (FPS>TICK — the default TICK=30 + uncapped render), after a warp
    repositioned the player to the destination, the interp dragged `m_vPos` back to the stale pre-warp snapshot AND
    the next snapshot re-read that clobbered value → the player was **stuck at the OLD position inside the NEW level
    → out of bounds → the world culled to PITCH BLACK** (only the HUD + weapon, both screen-space, drew); moving
    re-triggered a warp because the player sat on the wrong spot. **FIX:** skip interpolation + its restore while
    `pThis->m_Warp != WARP_NOT_WARPING`, and force `_ipHave=0` so the first post-warp frame re-snapshots from the
    real destination. **★ WHY I FIRST GOT IT WRONG: the bug ONLY manifests at TICK<FPS (render-only frames exist).
    My headless captures used TICK=0 — every frame is a logic tick, so there's no stale snapshot — so lvl 26
    rendered fine and I mis-concluded "dark cave". REPRODUCED by matching the user's config class: TICK=30/FPS=120
    = BLACK, TICK=30/FPS=30 = renders, TICK=30/FPS=120 + fix = renders.** Verified: lvl-26 bonus renders the cave;
    fire-pit spawn clean; patrols warp 0/2000/6000 rc=0, zero anomalies. **LESSON: a headless capture at a
    non-default tick/fps can MASK a bug that only exists in the user's real config — when a render bug "won't
    reproduce," MATCH the user's TICK/FPS (you need render-only frames, i.e. FPS>TICK). And when the user says "it's
    a regression" and "it rendered before," TRUST that over a headless capture that looks fine — bisect the
    mechanism, don't explain the screenshot away.** (This is the same render-interpolation system behind the death
    fall-through saga — interpolation interacting badly with a position discontinuity; warps are another one.)
  - **★ BONUS-EXIT OUT-OF-BOUNDS = the RETURN-warp point stored HOST-order but the spawn reads it BIG-ENDIAN
    (2026-06-18, commit 16b5fa8).** User: exiting the bonus stage "teleports me out of bounds... not being
    teleported back to where I entered the portal." Root: the blue portal's STORE (`tengine.c CEngineApp__DoWarp`,
    ~line 2448) saves the entry point (pos/rot/level/region) in HOST order, but the RETURN spawn (`scene.c
    CScene__RequestWarpPoints` RETURN branch:434 -> the player injection ~scene.c:1701/1710) consumes m_WarpPoint
    with the CART warp-point endianness convention: m_vPos + m_nLevel HOST, but **m_RotY + m_nRegion BIG-ENDIAN**
    (the spawn / instance-decoder ORDERBYTES them back to host). So on return the rotation + region byte-swapped to
    GARBAGE: a wrong region -> wrong `GetGroundHeight` -> the player spawned ~530u too HIGH (**Y=891 vs the entry's
    358**) on a frozen wrong region = OOB; m_vPos is host in both so the xyz was ~right (lands near the entry but
    floating OOB). FIX: encode m_RotY (4-byte, via a `union`+`__builtin_bswap32`) + m_nRegion (2-byte WORD,
    `__builtin_bswap16`) BIG-ENDIAN at store time (PLATFORM_PORT) to match the spawn; m_vPos + m_nLevel stay host.
    **PROVEN by A/B (`TUROK_OLDSTORE`): old store -> return Y=891 on a stuck wrong region; fixed -> Y=358 grounded
    at the entry, region tracks as you walk.** New debug hooks: `TUROK_FORCERETURN` (headless return-warp trigger)
    + `TUROK_POSLOG` (per-~second player level/pos/region log — the position-trace methodology the user suggested).
    **★ SIBLING (latent, deferred): `m_CinemaWarp` (camera.c:1641-1644, the death/key-cinematic respawn) has the
    IDENTICAL host-order store -> the same garbage m_RotY + m_nRegion on the cinema respawn. Its REGION is currently
    MASKED by the death-fix per-frame `CScene__NearestRegion` re-acquire (which overrides the stored region at the
    correct live position), so only the respawn FACING is wrong (not user-reported). Fix it the same way if a
    death/key respawn-facing issue surfaces — but the re-acquire makes the region a no-op, so it's low priority.**
    LESSON: a SAVED warp/return/cinema point that's later fed through the CART warp-point spawn MUST match the cart
    endianness convention (m_vPos host, but **m_RotY + m_nRegion big-endian**), not pure host order — else the
    respawn region + facing byte-swap to garbage (wrong region -> wrong ground height -> floats OOB).
- **★ DEBUG-KNOB CLEANUP (2026-06-17, 9-agent Workflow).** Stripped ~36 one-off `TUROK_*` debug env knobs that
  accreted across the porting sessions — the `*LOG` trace prints (OBJLOG/GATELOG/BLENDLOG/RSLOG/MTXLOG/VTXLOG/
  QLOG/QCLOG/SIMPLOG/INSTLOG/XINSTLOG/PARTLOG/MATLOG/VMLOG/VP_LOG/CAMLOG/MOVELOG/GFX_DUMP/GFX_DRAWLOG/OBJLOG/
  OBJPOS/MTXSTACK/TRACE/SWAP_BT/RUN_BT) and the headless debug BEHAVIOR toggles (SPAWNAT/DRAWALL/NOWORLD/
  ANIMOBJ/KILLALL/PITCH/YAW/FACE/DRAW_NOCLIP/CLEAR_MAGENTA/EGL_GREEN/EGL_BLUE_CAP). Toggles were removed by
  deleting the debug branch and keeping the production default (normal cull/draw/spawn/clip). **KEPT** (so any
  inline references above are now historical): gameplay/asset/infra knobs (`TUROK_WARP`/`FPS`/`TICK_FPS`/`ROM`/
  `CARTDATA`/`MAX_FRAMES`/`CAPTURE_FRAME`/`CAPTURE_PATH`/`HUD`/`NOAUDIO`/`AUDIO_WAV`/`FAKEINPUT`/`EGL_SURFACELESS`/
  `DRI_NODE`/`FORCERUN`) and the always-on anomaly detectors (`[CAMTRACK]`/`[CANARY]`/`[CAMBAD]`/`VTXBAD`/
  `FXLEAK` + the GLIST-overflow / DMA-OOB / byte-swap-implausible safety prints, which fire only on a real
  anomaly). Verified: egl+sdl2 clean build+link (cross-file `g_turok_drawall` removal consistent), patrols all
  warps rc=0, 0 removed-knob `getenv` sites remain.

- **★ "MISSING PLATFORM" = v49-vs-retail ASSET issue, not a code bug (2026-06-16).** User reported the warp-0
  fire-pit "initial platform" missing + suspected the level resources weren't importing. Root cause: the level-1
  **walkway** over the water is a **RETAIL-only asset** — the v49 leak's `cartdata.dat` lacks it; walk FORWARD at
  the fire-pit on v49 assets and you drop into a **blue void** (verified headless: retail-forward = canyon path;
  v49-forward = empty blue). `play_level.sh` defaulted to v49 unless `ROM=` was passed, so a plain
  `./play_level.sh` loaded the leak assets → no walkway. **Fix: `play_level.sh` now defaults to the retail ROM
  (Path B) when `baserom.us.v12.z64` is present** (`ROM=none` forces v49). **LESSON: when geometry is "missing,"
  first confirm WHICH asset set is loaded (v49 placeholder vs retail Path B) before suspecting code — many
  "missing stuff" reports are the v49 leak being incomplete, fixed by Path B, not a bug.** (Headless-capture
  gotcha: `TUROK_CAPTURE_FRAME=N` on no-tick-gate builds needs `TUROK_MAX_FRAMES` WELL above N — the render-frame
  counter `s_frame_no` lags the frame-pump `g_frame`, else the capture silently never fires.)

- **★★ PROJECTILES DON'T DAMAGE + TORCHES DON'T ANIMATE — ROOT CAUSE = `PARTICLES_MAX_COUNT` was a DEBUG
  value of `2` (fixed 2026-06-19, commit 326eabe).** The leaked dev source left `defs.h`'s
  `PARTICLES_MAX_COUNT` at **2** (the real `128` commented out right above it). With only 2 particle slots,
  every weapon shot's muzzle-flash + smoke instantly fills the pool, and `CParticleSystem__AllocateParticle`
  PRIORITY-EVICTS the just-fired BULLET before `CParticleSystem__Advance` ever advances it — so the bullet
  (traced: `m_nFrames=56`, valid region, velocity 7680) is created but **never enters the active-particle
  loop** → never moves, collides, or damages. The SAME starvation freezes smoke/steam/flame particles → the
  "torches not animating." One debug constant broke BOTH; restored to 128. **Verified:** at pool=2 the player
  bullet is created but absent from the loop; at pool=128 it flies the level (`Z: -509→-1230`) and registers
  instance collisions (`inst=1`). **★ THE SUB-BULLET BELOW ("the pistol works, no fix needed — it was the
  CTTYPE_DOWN firing fix") WAS WRONG** — it only *looked* fine because `TICK=0` (every render frame is a logic
  tick) runs the bullet fast enough to occasionally beat the 2-slot eviction; at the user's `TICK=30` it's
  always evicted. **LESSONS: (1)** a leaked DEV tree can ship a core constant at a tiny debug value
  (`PARTICLES_MAX_COUNT=2`) — when "projectiles never register" AND "particle FX frozen" co-occur, suspect a
  pool/count cap before the per-particle logic. **(2)** NEVER trust a `TICK=0` headless "it works" for anything
  rate/eviction-sensitive — `TICK=0` produces a degenerate tick cadence; always pace `TICK=30 FPS>0` to match
  real play. **(3)** the geometry.c animated-TEXTURE 30Hz decouple (62de09e) is a valid correctness fix but the
  smoke is PARTICLES, so the POOL fix is what actually restores it. (The now-superseded original investigation:)
- **★ WEAPONS + TORCHES + BAND-AID AUDIT (2026-06-19).** Three of the user's reports.
  - **PISTOL "not damaging enemies" — the damage chain is SOUND; no code fix needed (it works).** A headless
    harness (`TUROK_AUTOAIM`/`TUROK_GIVEPISTOL`/`TUROK_AIMROT`/`TUROK_PBULLET`, all REMOVED after) arms the
    semi-auto pistol, places the player on a real enemy, fires, and traces the chain. Verified end-to-end: the
    bullet is a `PARTICLE_TYPE_BULLET` (NOT a hit-scan ray — the user's mental model was off), it collides
    (`instcol.c CCollide__InstanceCollision` → `IntersectCylinder` returns COLLIDED=1), dispatches the impact
    event (`particle.c CParticle__InstanceCollision` → `AI_Event_Dispatcher` → `AI_MEvent_Damage` AREA-damage),
    and `AI_DoHit` decrements enemy health (100→95→90… at point-blank AND ~205u range through the game's bullet
    auto-aim `GetAutoAimRotation`, which correctly picks the most-aligned enemy in the cone). The bullet inherits
    the player's VALID region so the `unicol.c:85 PORT_REGION_BAD` band-aid does NOT bail it — the audit's #1
    suspect was wrong for this bug. The "no damage" was almost certainly the FIRING itself, already fixed by the
    `CTTYPE_DOWN` auto-fire commit 1f516f2 (with the old `CTTYPE_SINGLE` the gun barely fired = looked like no
    damage). **USER TO RE-TEST on the current build.** ★ HARNESS GOTCHAS that cost real time: (a) headless weapon
    tests REQUIRE `TUROK_TICK_FPS=0` (every render frame = a logic tick) — at the default 30Hz tick + uncapped
    render, 250 render frames is <1s of game logic so the weapon never even raises/fires; (b) teleporting the
    player BEHIND an enemy (`enemy.z - aimdist`) goes OOB → the bullet spawns in a wall and its Advance never
    runs — rotate-only (face the enemy from the valid spawn) is the clean range test; (c) the area-damage means
    the bullet need only land within ~35-60u of an enemy, and a CLOSER enemy in the line of fire takes the hit
    (so the aimed-at enemy may not be the one damaged — correct behavior, not a targeting bug).
  - **TORCHES NOT ANIMATING — REAL FIX (commit 62de09e).** Torch flames are animated-texture WORLD surfaces
    driven by `geometry.c nFrame = game_frame_number / m_PlaybackSpeed`. `m_PlaybackSpeed` is authored for the
    N64's fixed 30fps, but `game_frame_number` increments every PRESENTED frame, so at the port's uncapped/high
    render rate the texture frame advanced in huge irregular steps and aliased into a frozen-looking shimmer.
    Fix: drive it from a new 30Hz-paced `g_turok_tex_anim_frame` (incremented only on logic ticks — the SAME gate
    that already decouples game motion at `tengine.c CEngineApp__UpdateGAME`) + guard a 0 `m_PlaybackSpeed`.
    Underlying-cause fix, not a band-aid: the world texture-animation clock was simply never decoupled from the
    present count the way gameplay motion already was. **LESSON: anything keyed off `game_frame_number` (per
    PRESENTED frame) needs the 30Hz `g_turok_tex_anim_frame`/`g_turok_logic_tick` decouple, or it runs at render
    rate and aliases/over-speeds — the same class as the original "game ran 2× too fast".**
  - **BAND-AID AUDIT (per the user's "I only ever want to solve an underlying issue").** The dominant band-aid is
    the **REGION-RESIDENCY cluster**: `unicol.c:85` `PORT_REGION_BAD` Collision3 bail, the per-frame + cinematic
    `CScene__NearestRegion` re-acquires (tengine.c UpdateGAME), the `PORT_REGION_BAD`/`PORT_CORNER_BAD` distance
    heuristics (romstruc.h), the flat-default ground/ceiling fallbacks (romstruc.c), and the corner-0→origin
    fallback (unicol.c). They all stand in for ONE real M5 fix: **keep the collision-cache entry RESIDENT / rebase
    region+corner pointers when the cart cache re-decompresses the collision buffer at a new address** (the cart
    streams + relocates by design; `cart.c ResetAge`/`KeepAroundAnotherFrame` exist for the residency half; a
    bigger cache does NOT stop the re-decompress). The blue-portal key-gate (9dbe14f) is BORDERLINE (faithful
    KEYS condition, but a post-filter of warp-point selection rather than gating the portal at its source in
    `aidoor.c PortalAI`). The recent CTTYPE_DOWN / warp-interp-skip / return-warp-endianness / audio-synthLock /
    fmodf-wrap commits are clean real fixes, not band-aids.

- **★ 3 RESPAWN / GAME-OVER BUGS — root-caused via scouting; fix directions banked (2026-06-19). NOT yet
  implemented (session limit; the investigation Workflow's agents all hit the limit before returning, so this is
  my own read — VERIFY each before coding).**
  - **(BUG 2) FALL/WATER-DEATH RESPAWNS AT THE DEATH SPOT (off the cliff) + "ghost" loop.** `CCamera__FadeToCinema`
    (camera.c:1641-1644) captures `m_CinemaWarp` = the player's CURRENT pos at the moment of death; for a FALL
    death that's MID-FALL off the cliff (the fall death is detected at camera.c:1689-1690 only once
    `m_vPos.y < GetGroundHeight`). The resurrect (`CEngineApp__PlayerLostLife` tengine.c:2360 sets
    `CINEMA_FLAG_PLAY_RESURRECT` when lives remain) respawns at `m_CinemaWarp` via `CScene__Construct(CINEMA_WARP_ID)`
    (tengine.c:4058-4066 → scene.c:459-462) — i.e. back off the cliff → falls again → the death/resurrect transition
    never resolves = the "ghost form". **FIX:** a CHECKPOINT system EXISTS — `CTurokMovement.CurrentCheckpoint`
    (a warp ID; set on warp at tengine.c:3993-4003, used on load loadsave.c:706/1012; regions carry
    `m_SaveCheckpointID` scene.c:1409). For a FALL/WATER death, route the respawn to
    `CScene__Construct(CurrentCheckpoint)` instead of `CINEMA_WARP_ID` (detect via the CinemaFlag FALL/WATER bits
    set at camera.c:1692/1703 — e.g. a PLATFORM_PORT global set in FadeToCinema, consumed+cleared at the
    tengine.c:4065 respawn). ★ RISK TO VERIFY FIRST: is `CurrentCheckpoint` reliably the CURRENT level's checkpoint
    when the port BOOTS straight into a level via `TUROK_WARP`? If it's 0/stale it would respawn in the WRONG level
    (level-1 start) — confirm the boot-warp sets it, else guard/fallback. SECONDARY (latent): `m_CinemaWarp.m_RotY`
    + `m_nRegion` are stored HOST-order (camera.c:1642/1644) but the spawn reads them BIG-ENDIAN (the sibling of the
    fixed return-warp bug 16b5fa8) → wrong respawn facing (region currently masked by the per-frame NearestRegion
    re-acquire). Store big-endian to match.
  - **(BUG 3) GAME-OVER → BLACK VOID (lose all lives).** `PlayerLostLife` (tengine.c:2347-2369): lives remain ->
    resurrect; NO lives -> `m_bGameOver=TRUE` + a 4s game-over overlay (`GameOverOverlay`, tengine.c:1674-1693)
    then `MODE_RESETGAME` (tengine.c:1690). In the port MODE_RESETGAME reboots straight into the level (the
    TUROK_WARP boot) and the legal/attract/title intro is FROZEN/skipped (frontend.c `CLegalScreen__Update`
    early-returns under PLATFORM_PORT) -> game-over lands in a non-rendering state = black void (the HUD + weapon
    are screen-space so they still draw). **FIX:** either (a) make the game-over overlay draw + on timeout restart
    the current level from `CurrentCheckpoint` (not the frozen attract), or (b) reload the level on game-over
    instead of MODE_RESETGAME's frozen-intro path. Verify whether GameOverOverlay actually draws (it's a
    C16BitGraphic — the HUD-class endianness was already fixed in onscrn.c) and what MODE_RESETGAME renders in the
    port.
  - **(BUG 1) ENEMY RESPAWN LOOP — regenerating enemy never re-engages.** Regen path: ai.c:2377-2381 starts it
    (`m_Regenerate--`; `m_cRegenerateAppearance = APPEARANCE_LENGTH*7/8`), ai.c:2044-2052 counts it down by
    `frame_increment` to 0 then sets `m_cRegenerateAppearanceWhite`. **FIX DIRECTION (not yet root-caused):** trace
    a killed+regenerating enemy — check (a) the appearance countdown completes at 30Hz, (b) the post-regen AI STATE
    transitions back to attack vs staying in an idle/birth ANIM loop (the anim exit-to-frame / m_CycleCompleted),
    or (c) an endianness bug in the respawn anim-type / AI type-flags leaving the AI stuck.

- **★ 3 RESPAWN/GAME-OVER BUGS — RESOLVED (BUG 2 + BUG 3 fixed & verified; BUG 1 not a bug in repro) (2026-06-19).**
  All three diagnosed headlessly (the actual death/respawn flow, not the pre-implementation guesses above). The
  repro harness: `TUROK_FORCEYAW=<rad>` (steer the player off a cliff), `TUROK_SETCHECKPOINT=<id>` /
  `TUROK_SETLIVES=<n>` / `TUROK_KILLSELF=1` (force respawn/game-over states), `TUROK_POSLOG`/`TUROK_MODELOG`/
  enhanced `TUROK_DEATHLOG` (`wasFall`), `TUROK_KILLALL`/`TUROK_REGENLOG`/`TUROK_REGENSTATE` (enemy regen) — all
  env-gated.
  - **(BUG 2) FALL/WATER-DEATH RESPAWN — FIXED (commits a453228 + the camera.c flag, 6e5ca02).** A fall/water death
    runs **THREE** `MODE_RESETLEVEL` passes (proven by trace): pass 1 = DEATH (`m_UseCinemaWarp==1`,
    CinemaFlags=FALL_DEATH 0x4 → the cinema-warp branch, `m_CinemaWarp`=off-the-cliff); passes 2/3 = RESURRECT (0x8)
    + FINAL (0x0), both `m_UseCinemaWarp==0` → the **else branch** `CScene__Construct(m_WarpID)`. The player's FINAL
    position is set by passes 2/3, which use `m_WarpID` = the level-**ENTRY** warp (e.g. 201), NOT
    `CTurokMovement.CurrentCheckpoint` (751) → respawn at the entry (which on lvl 1 is right by the cliff) → death
    loop. The earlier 1-line fix only covered pass 1; the actual fix routes **all three passes** to CurrentCheckpoint
    via a `g_turok_death_was_fall` flag (set in `CCamera__FadeToCinema` for FALL/WATER, consumed in BOTH the
    cinema-warp branch AND the else branch at tengine.c ~4071/4082, cleared once no death/resurrect cinematic remains
    pending). VERIFIED: with `m_WarpID=751` but `CurrentCheckpoint=0`, all 3 passes land at CurrentCheckpoint (fire
    pit, grounded) not `m_WarpID`. The "ghost form" = the resurrect TELEPORT_APPEAR anim, now plays at the checkpoint.
    **★ LESSON: a fall death is a 3-reset sequence; the FINAL respawn uses `m_WarpID` (level entry), not the
    checkpoint — a fix must cover the resurrect/final passes (the else branch), not just the death pass.** GOTCHA:
    `TUROK_FORCEWARP` set `m_WarpID==CurrentCheckpoint`, masking the bug headlessly — needed `TUROK_SETCHECKPOINT` to
    make them differ.
  - **★★ (BUG 2, follow-up) THE RESPAWN STILL LOOPED IN REAL PLAY — render-interp clobbered the respawn on
    render-only frames (FPS>TICK) (2026-06-20, commit 3d16ef4).** The a453228 reroute was correct, but the user
    STILL looped because `play_level.sh` defaults to **FPS=0 (uncapped) > TICK=30** → RENDER-ONLY frames where the
    player render-interpolation runs; my FPS=30==TICK verification had NO render-only frames so it MASKED it (the
    EXACT "portal to the void" trap, re-fallen-into). Mechanism: the respawn repositions the player to the
    checkpoint during MODE_RESETLEVEL where UpdateGAME (and the interp SNAPSHOT) never runs, so the interp's
    `_ipCurPos` keeps the STALE pre-death off-cliff pos; on the first MODE_GAME frame back, if render-only, the
    tick-gated snapshot is skipped and the end-of-frame restore (`m_vPos=_ipCurPos`, gated on `_ipActive`) writes
    the stale pos OVER the checkpoint respawn → dragged back off the cliff → dies again → loop. Neither existing
    guard caught it: a death respawn keeps `m_Warp==WARP_NOT_WARPING` (warp-skip off) and the off-cliff→checkpoint
    jump (~773u) is under the 1000u inter-tick snap (which compares prev-vs-cur, not actual-vs-snapshot). **Fix:**
    a DISCONTINUITY guard at the TOP of `CEngineApp__UpdateGAME` (before any logic moves the player, where `m_vPos`
    must still equal the prior restore `_ipCurPos`): if `game_frame_number==0` (first frame after ANY reset) OR
    `m_vPos` diverged from `_ipCurPos` (>100u), invalidate `_ipHave` → the interp re-snapshots prev=cur from the
    live respawn pos (snap, no clobber, no slide); a re-snapshot is always safe. The enemy/instance interp
    (romstruc.c) is already safe (per-instance prev, d² vs live `m_vPos` each frame, never made canonical from a
    stale snapshot). Verified FPS=120/TICK=30: respawn lands at 751 (9569,1178) not off-cliff (9175,415); patrols
    rc=0; game-over restart still works. Root-caused via a 4-agent workflow (map interp + adversarial verify).
    **★★ LESSON (re-learned the hard way): the render interpolation interacts badly with EVERY position
    discontinuity (warp, fall-death respawn, game-over restart, cinema), and the bug ONLY appears on render-only
    frames (FPS>TICK). ALWAYS verify respawn/warp/teleport fixes at the USER's config (FPS=0 or FPS>TICK), NEVER
    at FPS==TICK — a matched-rate test has no render-only frames and gives a FALSE PASS. The general guard
    (game_frame_number==0 + actual-vs-snapshot divergence at the top of the frame) now covers ALL repositions.**
  - **(BUG 3) GAME-OVER → BLACK VOID — FIXED (commit db42c8f).** Confirmed: lose all lives → `m_GameOverTime<0` →
    `SetupFadeTo(MODE_RESETGAME)` (tengine.c:1694) → MODE_RESETGAME sets `m_WarpID=LEGALSCREEN_WARP_ID` (3830/3863)
    → `MODE_LEGALSCREEN` (frozen) = black void; even WITH TUROK_WARP it does a full heavy game-reinit. **Fix
    (PLATFORM_PORT):** on game-over, clear game-over + restore lives (TMOVE_START_LIVES=2) + clear
    g_turok_death_was_fall + restart at `CurrentCheckpoint` via the lighter **MODE_RESETLEVEL** (the proven-rendering
    path), i.e. a clean "continue". VERIFIED headlessly (KILLSELF+SETLIVES=0): `[GAMEOVER] ... restart at checkpoint
    N via MODE_RESETLEVEL`, never MODE_RESETGAME, the restarted level renders (12678 colors, not 1-color black). NOTE:
    a true attract/title on game-over is a SEPARATE task (un-freeze the legal/intro frontend + intro frame-pacing);
    the port does a checkpoint-continue until then.
  - **(BUG 1) ENEMY RESPAWN LOOP — ★ FIXED (FPS>TICK regen-appear loop, 2026-06-20, commit 9844a7e).** The earlier
    "NOT REPRODUCED" was the SAME FPS==TICK masking as the player loop. The regen WORKS at FPS=30 (regen → agit=299
    fighting + appear fades 6.6→0 in ~4s when near) but LOOPS at FPS>TICK (play_level.sh default FPS=0). Root cause
    (traced the AI timers FPS=30 vs FPS=120): the regen "appearance" phase has a 1.5s delay (`m_Time1`); when it clears
    (ai.c ~2055) the code re-checks `m_cRegenerateAppearance == APPEARANCE_LENGTH*7/8` (EXACT equality) to detect
    "appear just started", but that counter only decrements via `frame_increment` = 0 on RENDER-ONLY frames — so when
    the delay clears ON a render-only frame (3/4 of frames at FPS=120) the counter stays EXACTLY at the initial → the
    check re-arms the delay (m_Time1=1.5) → the enemy loops the appearance forever, stuck `agit=0`/gone, never
    re-engaging. **Fix (ai.c, PLATFORM_PORT):** on the delay clear, nudge `m_cRegenerateAppearance -= 0.01f` off its
    exact initial value so the check can't re-fire; the appear then fades normally on the next logic tick. Verified
    FPS=120: re-engages (`agit=299`), appear fades 6.1→0. (The regen COUNT was always fine — the `ORDERBYTES`
    double-swap suspicion is moot: aistruc.c:69 is `#ifdef WIN32` ENCODE-only, not compiled. The 16s "far-stall" is
    N64-faithful distant-AI throttle, not the bug.) Repro tooling: `TUROK_KILLALL`/`REGENLOG`/`REGENSTATE` (60a23ca).

- **★★ RECURRING BUG CLASS — RENDER-ONLY FRAMES (FPS>TICK) break `frame_increment`-gated EXACT-EQUALITY / one-shot /
  snapshot logic (2026-06-20).** `play_level.sh` defaults to **FPS=0 (uncapped) > TICK=30**, so the render runs faster
  than the 30Hz logic and produces RENDER-ONLY frames where `frame_increment=0` (the logic step is frozen; only the
  render + interpolation advance). ANY game logic that (a) compares a `frame_increment`-decremented counter for EXACT
  equality to detect a one-shot transition, or (b) takes/restores a position snapshot assuming every frame is a logic
  tick, MISBEHAVES on render-only frames. Hit **3× now**: (1) **portal-to-void** — interp clobbered m_vPos to the
  pre-warp snapshot (7bfd06c); (2) **player fall-death respawn loop** — interp restored the stale pre-death pos over
  the checkpoint respawn (3d16ef4 = a discontinuity guard at the TOP of UpdateGAME: `game_frame_number==0 || m_vPos
  diverged from _ipCurPos` → re-snapshot); (3) **enemy regen-appear loop** — the `==exact` appear check re-armed because
  the counter didn't decrement on the render-only clear frame (9844a7e). **★ LESSON: ALWAYS test respawn / warp /
  teleport / AI-timer / cinematic fixes at FPS>TICK (FPS=120 TICK=30, or FPS=0), NEVER at FPS==TICK — a matched-rate
  run has NO render-only frames and gives a FALSE PASS that masks the bug. When a "fix" works headless but the user
  still sees it, FPS==TICK masking is the #1 suspect.** Generalized in `docs/N64_PORTING_PLAYBOOK.md`.

- **★ FALLING-DEATH CINEMATIC FIXED + BAND-AID AUDIT (2026-06-20, commit 973e23d).** User: Turok hangs SUSPENDED
  mid-air in the falling pose instead of falling to his death. TWO band-aids combined: (1) **a453228** rerouted the
  DEATH pass of the fall-death reset (tengine.c cinema-warp branch) to CurrentCheckpoint — teleporting the cinematic
  to the checkpoint instead of the off-cliff fall position (`m_CinemaWarp`); (2) **tmove.c:744** skips the dead/cinema
  drop-to-ground velocity+Collision3 on the port (it crashes on KEY-PICKUP cinematics where the M5 collision parse
  derefs garbage region corners) — which ALSO froze the fall-death descent. **Fix (keeps the checkpoint respawn the
  user wants):** revert ONLY the death-pass reroute → the death pass uses STOCK `CINEMA_WARP_ID` (the fall pos) so the
  cinematic plays AT THE CLIFF; the RESURRECT+FINAL passes (else branch) still reroute to CurrentCheckpoint so the
  player ENDS at the checkpoint. AND re-enable velocity+Collision3 ONLY for `CAMERA_CINEMA_TUROK_FALL_DEATH_MODE`,
  crash-safe because that cinematic sets `GroundBehavior=INTERSECT_BEHAVIOR_IGNORE` (gates the ground-corner deref off,
  unicol.c:1039) + `waterFlag==PLAYER_NOT_NEAR_WATER`. Verified FPS=120: at the death pass Turok is at the fall spot
  (9201,-3339), Y descends 338→-2551 (falls), then respawns at the 751 checkpoint grounded.
  - **★ BAND-AID AUDIT VERDICT** (user asked which respawn fixes are source-faithful vs band-aids / regression risks;
    4-agent workflow). **NECESSARY host-portability (KEEP** — replace the N64's no-MMU tolerance of stale pointers a
    protected host can't replicate**):** the render-interp discontinuity guard (3d16ef4), the per-frame + cinematic
    `CScene__NearestRegion` re-acquire when `PORT_REGION_BAD` (~4759/5014), the GetGround/Ceiling flat-default
    fallbacks. **BAND-AID (papers over the real M5 collision-STREAMING root: the cart re-decompresses the collision
    buffer at a NEW address without updating the player's region pointer → stale/garbage corners; becomes DEAD CODE
    once M5 keeps the cache resident / rebases corners):** the tmove.c:744 cinema-physics skip, the
    `PORT_REGION_BAD`/`PORT_CORNER_BAD` distance heuristics. **a453228 death-pass reroute = a true band-aid that caused
    the cinematic regression → REVERTED.** **DESIRED DEVIATIONS (not source-faithful but the user wants them):** the
    checkpoint respawn (else-branch; the N64 respawns at `m_WarpID`), the game-over→checkpoint reroute (db42c8f; until
    the frozen legal/attract frontend is unfrozen). **DEFERRED faithful alternative:** sync `m_WarpID` to
    CurrentCheckpoint at checkpoint-pass time (tmove.c) and drop a453228 entirely — higher-risk (`m_WarpID` is read by
    several paths), so the verified reroute stays the safe default.

- **★ TUROK_WATCHDOG=1 — in-game freeze backtracer, default-ON in `play_level.sh` (2026-06-20, turok_main.c, commits
  973e23d/7c22d59).** gdb-attach is blocked by yama `ptrace_scope`, so to catch a freeze-with-no-crash-dump (an
  infinite loop / spin) a watchdog thread `pthread_kill(main, SIGUSR1)`s + `backtrace()`s the MAIN thread when
  `g_frame` stops advancing. **★ GOTCHA fixed (7c22d59):** it must NOT arm until frames are advancing — the SDL2
  GL/shader/driver init in `turokGfxInit` runs BEFORE the first frame and can exceed the timeout (g_frame stays 0),
  which the watchdog read as a hang and `_exit`'d a healthy boot (the user couldn't start the game; the backtrace was
  all SDL2 init frames). Now it skips monitoring while `g_frame<=0` and uses a 6s stall threshold. A real in-game
  freeze leaves g_frame>0 + frozen → caught, and the backtrace points at the stuck GAME function. `WATCHDOG=0
  ./play_level.sh` disables.

The port build infra (not game source): `Makefile.port`, `port/include/turok_port.h` (host compat shim),
`lib/ultralib/` (vendored libultra headers), `tools/turok_rom.py`.

---

*Created 2026-06-12. Status: **M0 (compile) DONE**; **M1 (boot headless) DONE**; **★ M2 (FIRST PIXELS)
ACHIEVED 2026-06-13** — the port renders the real Turok LEGAL/COPYRIGHT screen correctly (full Acclaim/GBPC/
Nintendo 1997 text), captured headless on the AMD GPU via the new **EGL/GBM hardware-GL backend** (no X server).
The core M2 blocker was a present-starved spin (frames rendered but never presented because the N64 scheduler
thread that calls `osViSwapBuffer` doesn't run cooperatively) — fixed by presenting in `sched.c scSendCommand`.
A degenerate zero-height `G_SETSCISSOR` was blanking the frame (scissor root-fixed in gfx_opengl). The `port/`
layer: `os_shim.c`, `romdata.c`, `turok_runtime.c`, `turok_gfx.c`, `turok_main.c`, `fast3d/gfx_egl.cpp` (GBM).
Build+run: `GFX=egl TUROK_OUT=/tmp/tbe bash tools/build_port.sh release`; `unset DISPLAY;
TUROK_CARTDATA=$PWD/src/PR/cartdata.dat TUROK_FPS=0 TUROK_MAX_FRAMES=130 TUROK_CAPTURE_FRAME=120
TUROK_CAPTURE_PATH=x.png /tmp/tbe/turok`.*

*Status: **★★ M5 (FIRST LEVEL RENDERS) ACHIEVED 2026-06-14** — `TUROK_WARP=0` loads, parses, and renders the
opening level as real textured 3D geometry (canyon, mossy ground, stone well, fog; ~405 world tris/frame, no
crash). The whole collision/instance/grid/geometry asset layer is big-endian; a multi-agent Workflow mapped
every field against the WIN32 encode path → 33 `ORDERBYTES` fixes (warp injection, instance/static/grid decoders,
region blocks/sets, grid bounds, poly-stream facets + Vtx). **GOTCHA**: in-place block swaps must start at
`GetBasePtr` (skip the US header) or they corrupt the big-endian block-count header → `GetBlockCount` returns
nonsense. **Animated objects (enemies/weapon/doors) now draw by DEFAULT** — a 2nd multi-agent Workflow swept the
model node-hierarchy / animation / transition / event endianness (23 fixes: geometry.c child/part index blocks,
anim.c DecompressAnim streams, transition reads) → `CGameObjectInstance__Draw` no longer crashes (`TUROK_ANIMOBJ=0`
disables). Validated at warp=0 (the player + a device draw without crashing; both culled/off-screen so not yet
*visible* — first-person hides the player, and levels 0/1 have no enemy in view). Capture a level:
`TUROK_WARP=0 TUROK_MAX_FRAMES=30 TUROK_CAPTURE_FRAME=20 TUROK_CAPTURE_PATH=x.png /tmp/tbe/turok`. **Next:**
levels 2-8 (warp 2000+) load very slowly (level 2 = 5786 regions vs level 0's 153 — a per-frame collision-list
perf issue) which blocks testing in-view enemies; the ~150 brightness, M3 (3DS Citro3D). See §10 + memory
`turok_port`.*

*Status: **★ HUD RENDERS + PATH B + WATER PROVEN (2026-06-15).** (1) **Path B** (`TUROK_ROM=<retail .z64>` →
`romdata.c` reads the v1.2 asset blob at ROM 0x1F00) plays finished retail content — confirmed it restores
MISSING LEVEL GEOMETRY (a fire-pit walkway absent in the v49 leak) + finished object models. `ROM=… ./play_level.sh`.
(2) **Water/translucency** investigated end-to-end (5-agent Workflow + runtime traces `TUROK_MATLOG`/`BLENDLOG`/
`XINSTLOG`): the Fast3D→GL blend chain is provably correct; warps 0/1 simply have no water; warps 3000/4000/6000
have transparent surfaces that render (warp 6000 shows translucent cyan bars). Not a bug. (3) **HUD now renders
by default** — fixed the big-endian `C16BitGraphic`/`C16BitPart` header decode in onscrn.c/onscrn.h (`ONSCRN_SW16/32`
read-time swaps); Turok's face + life-force "600" + lives "x2" draw, 150 frames clean, `TUROK_HUD=0` disables.
**Next:** levels 2-8 load perf (collision-list), object/creature material (interactive), brightness polish, M3
(3DS Citro3D).*

*Status: **★ GAMEPLAY SOLID — DEATH / RESPAWN / ENEMY / AUDIO all working (2026-06-20).** This session closed the
gameplay-breaking respawn/death cluster: a fall death respawns at the last CHECKPOINT (not off the cliff → no death
loop), the fall-death CINEMATIC plays correctly (Turok falls at the cliff, then respawns at the checkpoint), GAME-OVER
restarts at the checkpoint (not a black void), and regenerating ENEMIES re-engage combat instead of looping the spawn.
**The dominant bug class was RENDER-ONLY FRAMES (FPS>TICK):** three separate "fixed headless but still broken in play"
bugs (player respawn loop 3d16ef4, enemy regen loop 9844a7e, portal-to-void 7bfd06c) all traced to logic that breaks
when render-rate > tick-rate (`play_level.sh` defaults FPS=0 > TICK=30) — **ALWAYS test respawn/warp/AI-timer/cinematic
fixes at FPS>TICK, never FPS==TICK** (the matched-rate run has no render-only frames → false pass; see §8 of
`docs/N64_PORTING_PLAYBOOK.md`). A BAND-AID AUDIT (4-agent workflow) classified the death-fall-through fix cluster:
NECESSARY host-portability (render-interp guard, region re-acquire, ground fallbacks) vs BAND-AID for the M5
collision-STREAMING root (tmove cinema-skip, region distance heuristics) vs DESIRED deviations (checkpoint respawn,
game-over reroute); the one true band-aid that caused a regression (a453228 death-pass reroute) was reverted.
`TUROK_WATCHDOG` (default-ON in play_level.sh, fixed to not false-fire on the slow SDL2 GL startup) backtraces an
in-game freeze. **Next:** the deeper M5 collision-streaming fix (keep the cache resident / rebase corners on
relocation → removes most of the band-aid cluster); brightness polish; M3 (3DS Citro3D).*

*Status: **★★ PC FEATURES + 3DS BUILD STOOD UP (2026-06-20).** Added the PC FPS control scheme (held mouse-look —
no spring-back, WASD on the C-buttons so movement no longer fires the engine's native D-pad run/walk toggle,
inverted pitch, one-weapon-per-scroll-notch via a tick-gated discrete seam, E walk toggle, exclusive cursor,
1280x1024), a FILE-BASED SAVE (F5/F9 quick-save/load of the same CPersistantData blob the N64 pak-save uses,
restart-at-checkpoint), and SETTINGS PERSISTENCE (`turok.cfg`). Then STOOD UP THE 3DS BUILD: `Makefile.3ds`
(devkitARM + libctru + Citro3D, additive), 3DS-M0 (all 201 TUs compile on ARM) + 3DS-M1 (links →
`build_3ds/turok.3dsx`, 1.05 MB). The Citro3D backend was already vendored from Perfect Dark; the new work is
the Makefile + the sys/audio(ndsp)/input(HID)/stderr 3DS TUs, all `#ifdef PLATFORM_3DS` so the PC build is
untouched (re-verified clean at every step). **Two interactive handoffs:** (1) PC input/save feel needs the
user to confirm mouse turn/look SIGN + scroll direction (can't be headless-tested); (2) 3DS-M2 = the user runs
`turok.3dsx` in Mandarine (ROM at `sdmc:/3ds/turok/baserom.us.v12.z64`) → `boot.log` shows where it faults →
ARM byte-alignment + boot fixes are crash-driven from there. **Next autonomous (unblocked):** the M5
collision-streaming root fix; the in-game options menu UI (after input-feel confirm).*
