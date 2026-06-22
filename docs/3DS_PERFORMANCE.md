# Turok 3DS — Performance Optimizations (on the table)

> A prioritized, actionable TODO distilled from a 9-dimension multi-agent perf audit of the Turok
> 3DS port against the **Perfect Dark** baseline (both share the vendored `gfx_citro3d.cpp` /
> `gfx_pc.cpp`). Target hardware = **New 3DS** (804 MHz ARM11, cores 0/1 app + 2 spare).
>
> Line numbers are **approximate** (the shared files drift); treat them as starting pointers.
> Every change to the **shared** `gfx_pc.cpp` / `gfx_citro3d.cpp` MUST be verified **byte-identical**
> against a PC headless capture across warps 0/2000/6000/8000 (PC + 3DS compile the same file).

---

## ⚠️ Read this first: you cannot benchmark these on Mandarine

The audit's single most important caveat: **Mandarine (the emulator) on a fast PC does NOT model
ARM11 cost faithfully.** The VFP double-precision / denormal stalls, the software 64-bit integer
divides, the per-draw `C3D` register writes, and the `SYNCDRAW`/vsync idle are **real-hardware wins
that show little or nothing on the emulator.** "Turok runs worse than PD on Mandarine" is a shaky
benchmark — most of this list only pays off on a real New 3DS.

**→ Action zero: get an on-device PROF capture before investing effort** (item M1 below). It settles
whether the frame is CPU-bound (`present≈0`) or GPU-bound, and which phase dominates.

---

## Diagnosis (why Turok trails PD)

The renderer is the *same vendored PD code*, so the deficit is **not** the backend. It's:

1. **One saturated ARM11 core.** Game sim → Fast3D DL interpretation + per-vertex MVP → per-eye
   Citro3D replay → `C3D_FrameEnd` vsync wait all run **serially** on core 0 (`sched.c scSendCommand`
   → `gfx_run` → `finish_render`, inline). The N64 overlapped game-sim (CPU) with DL interpretation
   (RSP) via the game's **triple-buffered** DL (`frame_number % 3`) — the synchronous dispatch throws
   that overlap away. Meanwhile the New-3DS **spare core 2 sits idle** (audio gated off, bake off).
2. **Per-element overheads that scale with Turok's bigger scenes:** redundant per-draw GPU state
   (only fog is deduped), `f64` math from the leaked Iguana source (unsuffixed `double` literals),
   `DoAI`/`Advance` run every render frame, textures uploaded as `RGBA8` not `RGBA5551`, and always-on
   debug instrumentation in the per-vertex/-matrix/-tri hot path that PD doesn't have.
3. **PD renders less.** PD **portal-culls** (BSP rooms); Turok draws the whole visible level. Some of
   the real lever may be **game-side draw-distance/LOD**, not a backend tweak — confirm with the
   `g_turok_tris` / `g_turok_tri_calls` / `g_turok_tri_cliprej` counters.

---

## M — Measure first

- [ ] **M1. Enable + relocate the PROF profiler, capture an on-device frame breakdown.**
      The profiler already exists, gated behind `PD_DEBUG3DS`, but writes to `sdmc:/3ds/perfectdark/`.
      Build with `PD_DEBUG3DS=1` (or a new `TUROK_PROF`), fix the SD path to `sdmc:/3ds/turok/`, and
      read the `PROF game= interp= replay= present=` line on busy level-2/6 views. `present≈0` ⇒
      CPU-bound (do the quick wins / game-CPU items); large `present` ⇒ GPU/replay-bound (do #Q2 / #B1).
      *Evidence:* `gfx_citro3d.cpp` ~2686-2752. *Impact: high · Effort: small · Risk: low · 3DS-only.*

---

## Q — Quick wins (low risk, 3DS-gated, PC-verifiable; do these regardless of PROF)

- [ ] **Q1. Per-draw GPU-state dedup in `replayRange`/`applyCmdState` (the biggest practical lever).**
      Today only *fog* is deduped; depth/blend/alpha/TEV/viewport/scissor/texbind are re-applied
      **every draw**. Add a per-pass "last applied" shadow (reset at the top of each replay range) and
      skip the `C3D_*` call when the recorded state equals the previous draw's. Collapses long runs of
      identical-state world draws from N re-applies to 1 — **and halves the stereo second-eye cost for
      free** (the doubled cost *is* these per-draw `C3D` calls). `DrawCmd` already carries all fields.
      *Evidence:* `gfx_citro3d.cpp` ~2279-2404 (`applyCmdState`, fog dedup at ~2326), ~2433-2473
      (`applyViewport`), replay at ~2482-2505. *Impact: high · Effort: medium · Risk: low · 3DS-only.*

- [ ] **Q2. Strip always-on debug instrumentation from the hot paths.**
      Move the per-vertex `[VTXBAD]` NaN/huge check, the per-`G_MTX` `[CAMBAD]` 16-float scan, the
      per-triangle `g_turok_tri_calls`/`cliprej` global stores, and the per-skeletal-node `[CANARY]`
      matrix scan from `#ifdef PLATFORM_PORT` to a new `TUROK_GFX_DEBUG` flag **left undefined** in
      `Makefile.3ds`. PD's identical interpreter has none of these; they run thousands of times/frame.
      *Evidence:* `gfx_pc.cpp` ~1213, ~1327, ~1506-1517; `romstruc.c` ~10367-10373. *Impact: medium ·
      Effort: small · Risk: low.* (Shared file — keep them live on PC debug builds; verify PC capture.)

- [ ] **Q3. NaN-safe math fast-path build flags.**
      Add `-fno-math-errno -ffp-contract=fast -freciprocal-math` to `Makefile.3ds`. Collapses the 3
      serial per-vertex perspective `vdiv` into 1 `vdiv` + reused reciprocal, and inlines `sqrtf` to
      `vsqrt`. **Do NOT add `-ffast-math` / `-ffinite-math-only`** — they delete the load-bearing NaN
      guards (`turok_wrap_pi`, the interp-snap, the matrix canary; verified to optimize to constant-
      false). *Evidence:* `gfx_pc.cpp` perspective divides; `turok_port.h` + `romstruc.c` NaN guards.
      *Impact: medium · Effort: small · Risk: low · 3DS-only.*

- [ ] **Q4. Cache `turok_render_alpha()` once per frame.**
      The render-interp calls it twice per drawn instance; on 3DS each call = `svcGetSystemTick` + two
      **software 64-bit divides** (ARM11 has no HW integer divide) for a frame-constant value. Compute
      once at the top of `CScene__Draw`; read the cached float per-instance. Same for `g_turok_anim_step`.
      (No-op at the 3DS default `tick=0`, where alpha is already 0 — this is for the `fps>tick` configs.)
      *Evidence:* `romstruc.c` ~8781/8946; `os_shim.c` `port_mono`/`turok_render_alpha`. *Impact: medium ·
      Effort: small · Risk: low · 3DS-only.*

- [ ] **Q5. Skip the facade UV-span scan + invalidation churn when baking is OFF (the default).**
      The facade-registration block (UV min/max scan over all verts + `pdFacadeRegister`/`QueueInval`)
      runs per tiled textured batch every frame, gated on `sRenderPhase`, **not** on `sAutoBake`. With
      bake off (Turok's default) it scans for nothing AND queues invalidations that drop+re-decode each
      tiled texture on area entry with no bake to show. Gate it on `sAutoBake`.
      *Evidence:* `gfx_citro3d.cpp` ~1770-1808, ~2710. *Impact: medium · Effort: small · Risk: low · 3DS-only.*

- [ ] **Q6. Re-evaluate the `doTess` tessellation threshold (live by default, even with bake off).**
      `doTess` fires `emitTessTri` (recursive split to depth 7) on every tiled-axis textured draw — and
      with bake off it pays the full tess pass for no payoff (no bake to feed). Compute the batch's max
      tiling-axis UV span once and set `doTess=false` below ~3-4 tiles (near-1× walls show no f24 stripe
      yet still pay). Measure `interp` µs with PROF; cross-check stripes vs PC GL ground truth.
      *Evidence:* `gfx_citro3d.cpp` ~1966 (`doTess` default), ~1674-1697 (`emitTessTri`). *Impact: medium ·
      Effort: small · Risk: medium · 3DS-only.* **(The PD-comparison's top low-risk win.)**

- [ ] **Q7. Drop the double present-pacing on 3DS.**
      The frame is paced twice: `C3D_FrameEnd` blocks on vsync AND the `os_shim` `svcSleepThread` cap.
      Drop the software sleep on 3DS (present cap → 0 / vsync-only) so `C3D` vsync is the sole pacer,
      removing the granularity jitter that beats the 30 Hz tick. Verify game speed stays correct
      (`tick=0` advances logic per present). *Evidence:* `gfx_citro3d.cpp` ~2806/3009; `os_shim.c`
      ~229-239. *Impact: medium · Effort: small · Risk: medium · 3DS-only.*

- [ ] **Q8. Set the VFP flush-to-zero (FZ) bit at 3DS boot.**
      Set `FPSCR.FZ` (bit 24) once in `sys_3ds` boot via `vmrs`/`vmsr`. ARM11 VFPv2 handles denormals on
      a slow support path (10-100×); the port's endianness traps and blend math routinely drift to
      near-denormal (~1e-41). Neither Turok nor PD sets it. Hardware-only win — verify on real HW.
      *Evidence:* no `FPSCR`/`FZ` in `port/src`. *Impact: medium · Effort: small · Risk: low · 3DS-only.*

- [ ] **Q9. Trim: bottom screen + scratch buffer.**
      (a) Clear the unused bottom render target **once** (static guard), not every frame — saves a
      76.8K-px fill + a target-switch flush/frame. (b) `tex_upload_buffer` is `malloc`'d 1024×1024×4 =
      4 MB but Turok textures are ≤64×64 — size it to ~256×256 (256 KB) to free ~3.75 MB FCRAM and ease
      texture OOM. *Evidence:* `gfx_citro3d.cpp` ~2990-2995; `gfx_pc.cpp` ~3104. *Impact: low · Effort:
      small · Risk: low.*

---

## G — Game-CPU items (medium; help most at `fps>tick`, e.g. the 60 fps config)

- [ ] **G1. Gate `DoAI`/`Advance` to logic-tick frames.**
      `CGameObjectInstance__Draw` runs `DoAI` + `Advance` for every in-range instance **every render
      frame**, not every 30 Hz tick. The heavy physics is already gated on `frame_increment!=0`, but the
      AI state machine, water queries, anim-event sends, and head-tracking still execute on render-only
      frames (`fps>tick`) producing no state change. Wrap in `if (g_turok_logic_tick)` (PLATFORM_PORT;
      no-op on N64). Both a win and more correct (AI is designed for 30 Hz). Keep `SendAnimEvents`
      outside the gate if a side-effect must fire per-frame. *Evidence:* `romstruc.c` ~8747/8756, tick
      flag ~8617. *Impact: high (at fps>tick) · Effort: medium · Risk: medium · 3DS-only.*

- [ ] **G2. Memoize the per-frame `IndexedSet`/`UnindexedSet` re-parse + `PORT_REGION_BAD` verdict.**
      `CScene__GetRegionAttributes` re-parses cart-block headers (`ConstructFromRawData`×2 +
      `Destruct`×2, ORDERBYTES swaps) on **every call**, several times per tick; `PORT_REGION_BAD` re-runs
      ~12 pointer compares per ground-height/normal query. Both are frame-invariant (the collision buffer
      only relocates on streaming events). Cache the regionSets base + a region-good flag; invalidate on
      the already-tracked relocation. *Evidence:* `scene.c` ~4385-4418; `romstruc.h` `PORT_REGION_BAD`;
      callers `unicol.c`/`tengine.c`/`romstruc.c`. *Impact: medium · Effort: medium · Risk: low · port-only.*

- [ ] **G3. Throttle/localize the per-frame O(nRegions) `CScene__NearestRegion` band-aid.**
      The cinematic re-acquire calls `NearestRegion` every frame for a 30-frame window; on level 2 that's
      a 5786-region point-in-poly scan/frame during deaths/key-pickups/warps. Drop the `_rw>0` term (rely
      on `PORT_REGION_BAD` so it only runs when the pointer is actually bad), OR seed the search from the
      player's current region + neighbors to make it O(local). Add a one-shot counter to catch a
      regression where `PORT_REGION_BAD` fires every frame in normal walking. *Evidence:* `tengine.c`
      ~5111-5115/4766; `scene.c` ~4469-4525. *Impact: medium · Effort: medium · Risk: medium · port-only.*

- [ ] **G4. Single-precision `sincosf` for the rotation-matrix builders.**
      newlib `sinf`/`cosf` compute in double precision (~2× VFPv2 cost); the `CMtxF` rotation builders
      call `sin()` AND `cos()` of the same angle as two separate calls (two range reductions). Provide a
      port-local `turok_sincosf` (minimax poly after one `fmodf` 2π reduction). Validate against the
      `[CAMTRACK]`/qGround unit-quaternion canary. *Evidence:* `graphu64.c` ~271-272/312-313. *Impact:
      medium · Effort: medium · Risk: medium · port-only.* (Re-measure after Q3 lands.)

---

## T — Texture items (matter MORE on real HW; FCRAM is tight there, masked on Mandarine)

- [ ] **T1. Upload paletted/RGBA16 textures as `RGBA5551`/`RGB565`, not `RGBA8`.**
      Turok's art is CI/RGBA16, ≤64×64, 1-bit alpha. Choose `GPU_RGBA5551` (matches the source) or
      `GPU_RGB565` (opaque) for non-32-bit imports via the existing `swizzleTex16`/packers. **Halves**
      upload bytes, VRAM, sampler bandwidth, and FCRAM eviction pressure (fewer evictions = fewer
      re-decodes). Visually lossless for CI/RGBA16. Keep a needs-8-bit check for rare true-32-bit
      textures. *Evidence:* `gfx_citro3d.cpp` ~1473 (RGBA8 upload), packers ~800-887; `textload.c`
      palettes. *Impact: high · Effort: medium · Risk: low · 3DS-only.*

- [ ] **T2. Content-key the texture cache to stop streaming-relocation re-decode thrash.**
      The host cache keys on the **cart-cache source pointer**; Turok streams texture sets through the
      relocating `MEMORY_POOL`, so an evicted+re-decompressed set lands at a NEW address → key miss →
      full re-decode+re-upload of identical texels on area entry (a hitch PD doesn't have; doubling the
      cache slot count can't help — old entries are dead at the wrong address). Add a content hash (or a
      stable `set_id`/`bitmap_index`/`palette_index` tag) to the key, reusing the bake path's
      content-key precedent (`bakeHash`/`regFind`). *Evidence:* `gfx_pc.h` `TextureCacheKey.texture_addr`;
      `gfx_citro3d.cpp` ~904. *Impact: high · Effort: medium · Risk: medium · port-only.*

- [ ] **T3. Budget per-frame texture decodes/uploads on area entry.**
      All decode + `linearAlloc` + `GSPGPU` flush happens inline on the one thread, each preceded by a
      GPU sync — on an area-entry frame the whole new working set uploads in one frame = a multi-frame
      hitch. Cap new decodes/frame (like `BAKE_PERFRAME`) and draw not-yet-uploaded textures with a
      placeholder for 1-2 frames (already the OOM fallback). *Impact: medium · Effort: medium · Risk:
      medium · 3DS-only.* (The larger "offload decode to core 2" variant is a follow-up of B1.)

---

## B — Big bets (high ceiling, large effort; do AFTER the quick wins + PROF confirm the bottleneck)

- [ ] **B1. Render thread on core 2 (the structural #1 lever).**
      Hand the completed DL to a worker pinned to **core 2** (mirror `audio_3ds.c threadCreate`) that owns
      `gfx_run` + `end_frame` + `finish_render`, so game-sim for frame N+1 overlaps interp+replay+present
      (the vsync IDLE wait) for frame N. The game **already triple-buffers the DL** (`frame_number % 3`)
      — the exact overlap the N64 used and the synchronous dispatch discards. A 1-deep two-semaphore
      handoff bounds input lag to ~1 frame. **High risk:** Fast3D uses global `rdp`/segment state and the
      `SYNCDRAW`/RT-from-tex (blur/scope) paths assume a drained GPU — gate carefully. *Evidence:*
      `sched.c` ~213-222; `tengine.c` ~2994-3008 (triple buffer); `audio_3ds.c` ~156-178. *Impact: high ·
      Effort: large · Risk: high · 3DS-only.*

- [ ] **B2. Re-enable the threaded facade bake on core 2 (New-3DS only) — *if* bake is wanted.**
      Already wired (`PD_BAKE_THREAD=1`), just defaulted off because Turok's organic art rarely needs the
      bake. If a level shows facade stripes, re-enable per-level/per-area with the worker on core 2 so the
      cost lands off the main thread. *Impact: medium · Effort: medium · Risk: medium · 3DS-only.*

---

## ✗ Investigated and explicitly NOT worth doing (don't re-litigate)

- **Render interpolation is a non-issue at the 3DS default** (`tick=0` → `turok_render_alpha()` returns
  0, all interp short-circuits). Only the opt-in `fps 60 / tick 30` config pays it. *Do not "optimize it
  away."* Consider forcing `tick=0` whenever `fps<=30` (interpolation is pointless when render ≤ logic).
- **Collision does NOT scale with region count** (5786 on level 2). It's a **local neighbor-walk** capped
  at 100, gated to 30 Hz. The old worry is wrong; the win is per-query overhead (G2), not a hash rewrite.
- **Build *flags* are at parity with PD** (`-O2`, no LTO/fast-math). The math *source* is the delta (Q3/Q4).
- **`-ffast-math` is banned** — it deletes the NaN freeze-guards the port relies on.
- **Single-pass stereo is already correct and efficient** — see the architecture note below; nothing to do.

---

## Stereo / GPU-MVP architecture note (why we do NOT change it)

Turok/PD do **single-pass stereo via a clip-space shear**, NOT Forsaken's GPU-MVP projection-shift —
and that is the **right** call, dictated by Fast3D. See
[`N64_PORTING_PLAYBOOK.md` §14](N64_PORTING_PLAYBOOK.md) for the full reasoning. In short: Fast3D does
the **full MVP on the CPU** and hands the GPU **pre-projected clip-space** verts, so you can't re-project
per eye without re-running the (already dominant) CPU interpreter. The shear reuses the single CPU
transform for both eyes; an off-axis projection per eye would force a second CPU pass. **Empirically
confirmed:** Perfect Dark *took a measurable perf hit when GPU-MVP was enabled* — the CPU-transform
path is faster for these Fast3D ports. Forsaken can afford projection-shift only because its renderer
leaves the transform to the GPU. **Do not port Forsaken's stereo method into Turok.**

---

*Created 2026-06-22 from the 9-dimension perf audit (`turok-3ds-perf-audit` workflow). Update as items land.*
