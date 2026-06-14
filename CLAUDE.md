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

## 10. Port edits to game source (keep this log honest)

We own this source outright (no IDO byte-matching build to preserve), so light, documented edits to the
game files are acceptable. Keep them minimal and listed here so they're reviewable:

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
perf issue) which blocks testing in-view enemies; then HUD endianness (`TUROK_HUD`), the ~150 brightness, M3
(3DS Citro3D). See §10 + memory `turok_port`.*
