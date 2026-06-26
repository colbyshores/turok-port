# N64 → PC / 3DS Porting Playbook

> Cross-cutting, **reusable** findings distilled from the Turok port, cross-checked against the sibling
> Banjo-Kazooie, Perfect Dark, and Forsaken ports in `/mnt/nas/Development/`. Read this before starting a
> new N64→PC/3DS port — every lesson here was paid for by a real debugging session. Project-specific detail
> lives in `CLAUDE.md`; this file is the **generalizable** layer.

---

## 0. Two starting points: decomp vs leaked source

| | **Decompilation** (Banjo, Perfect Dark) | **Leaked source** (Turok) |
|---|---|---|
| What you have | RE'd C that **byte-matches** the ROM | The **actual shipping C** |
| The constraint | Preserve the matching build (IDO, exact flags) | Just make it build off-IRIX, modern GCC/Clang |
| The risk shifts to | "does it still match the ROM" | "does it build clean + do asset offsets line up" |
| Editing game source | avoid (breaks the match) | **OK** — light, documented edits, logged in CLAUDE.md |

Either way the **job is the same**: make the original C build for a 64-bit little-endian host, shim libultra,
interpret the display lists with Fast3D, stream assets from the ROM, and fix endianness + ARM alignment. The
platform layer (`port/`) is identical in shape across all of them — see §11.

---

## 1. The platform seam (the same for every N64 port)

Keep game logic **unmodified**; put every platform divergence behind `PLATFORM_PORT` / `PLATFORM_3DS`, ideally
entirely inside `port/`. The seams:

| N64 mechanism | Host replacement |
|---|---|
| PI/DMA cart read | `romdata.c::piRead()` → `memcpy` from the in-RAM ROM image (+ decompress on demand) |
| Decompression (RNC / rzip / MIO0…) | compile the engine's **in-tree** decompressor natively — it's portable C, don't rewrite |
| RSP gfx task (F3DEX) | dispatch the `Gfx` list synchronously through `gfx_pc.cpp` |
| RSP audio task | a C mixer (ADPCM decode + resample + envelope) |
| VI (present) | `videoEndFrame()` once per host frame |
| AI (audio out) | SDL2 queue (PC) / ndsp ring (3DS) |
| Threads / scheduler | cooperative shims in `os_shim.c`; one frame per main-loop iteration |
| Controller | SDL gamepad (PC) / libctru hid (3DS) |

**Fast3D** (`gfx_pc.cpp`) interprets the display lists once, into a backend-agnostic state, then pluggable
backends realize it (OpenGL/EGL on PC, Citro3D on 3DS). **Fix decode/state bugs ONCE in `gfx_pc.cpp`**; the
backends only differ in how they realize the state. Identify the exact microcode first (F3D vs F3DEX-1.x vs
F3DEX2) — it picks your interpreter. (Turok = `gspF3DEX_NoN`, so Banjo's F3DEX-1.x `gfx_pc.cpp` was the drop-in;
SM64 is plain F3D.)

---

## 2. Endianness — THE dominant bug class

This is where most of the time goes. N64 assets are **big-endian**; PC/3DS are little-endian. The patterns,
in rough order of how much pain each saved:

- **The encode path IS the spec.** Most engines have an asset-authoring/encode function (often under
  `#ifdef WIN32` or in the toolchain — `TakeFromXxx`, `Encode`, `Write`) that byte-swaps *every field* when it
  builds the asset. Your DECODE path must mirror it field-for-field. **Diff decode against encode** — that is
  the authoritative, exhaustive list of what to swap. (This is what makes the multi-agent endian sweep in §12
  tractable.)
- **Swap structure, not payload.** Swap counts / offsets / headers / struct scalar fields at the typed
  consumer. Leave **pixel, texture, and audio payloads big-endian** and convert them at upload/render time.
- **A generic size-based byte-swap macro is a NO-OP on aggregates.** A `CVector3` / `float[3]` read raw on LE
  becomes a denormal (~1e-41, *prints as 0.0*) or junk → objects collapse to the origin, bounds explode. Swap
  vectors/matrices **component-wise**, explicitly.
- **In-place block swaps must start at the DATA base, past the container header.** If a block is
  `[big-endian count][data...]`, swap from `GetBasePtr()` (after the header). Swapping from the block start
  corrupts the count → the next `GetBlockCount()` returns a byte-swapped huge number → out-of-bounds. (Cost an
  hour, twice.)
- **Static lookup tables stored as big-endian float arrays are a SILENT trap.** e.g. an `acos()` implemented as
  `((float*)table)[i]` over a 1024-entry big-endian array → byte-swapped garbage on LE. The killer: a wild
  *angle's* sin/cos stay **finite**, so it never trips a NaN / huge-matrix detector — it just subtly skews
  quaternions and the view matrix ("camera feels broken while I walk"). Byte-swap the table **once on first
  use**. Audit every `((float*)tbl)[i]` cast and every libc-math fn the engine reimplements with a static table.
- **A key table and the struct it gates are a PAIR.** A binary-search/range over big-endian DWORD **keys** must
  swap the keys (else lookups silently never match). But fixing the key lookup **ungates** the payload struct —
  if *that* isn't also swapped you trade a silent no-op for a SEGV. Fix/gate both together. (On Turok, sound and
  particles each did exactly this: the key fix turned a no-op into a crash.)
- **WORD-array lookups often have TWO read sites.** An object-types table read **forward** (type→index) *and*
  **reverse** (index→type) needs the swap in **both** places (the weapon used one, item pickups the other).
- Centralizing every swap in one `*_swap.c` registry is the textbook ideal, but **inline per-consumer swaps
  also work and are lower-risk** to retrofit into a large existing codebase — don't force a risky consolidation
  of working code just for tidiness.

---

## 3. The implicit-declaration ABI trap (endianness's evil twin)

Any libc function the engine never called itself — `fmodf`, the single-precision math fns — is **implicitly
declared `int f()`** if no prototype is in scope. Wrong ABI → **garbage return**, compiled *silently* under
`-Wno-implicit-function-declaration` (which you'll need for old code). The garbage propagates: NaN angle → NaN
rotation → NaN view matrix → the whole frame clip-rejects → renders as a flat fog color. **Force-include a
prototype for every libc fn you introduce.** This masqueraded as "the world terrain never renders / player
spawns NaN" for a full session; bisect against the last-good commit and check implicit-decls first when a
render change blanks the world.

---

## 4. N64 null-tolerance (no MMU)

The N64 has no MMU: reads of NULL / low / small-garbage addresses return harmless RDRAM bytes. A protected host
**SEGVs** on the identical read, and you **cannot** map page 0 (`vm.mmap_min_addr=65536`). Two big sources:

- **Title / attract / legal screens** run the per-frame update with **no level loaded** → deref a NULL player /
  camera / weapon.
- **Cinematics** enable code paths (player collision, model-swaps, map reveal) that deref stale/uninitialised
  pointers the engine assumed were harmless.

Fix by guarding at the **subsystem / single-chokepoint** level (one guard at the collision-query entry covers
all callers), **not per-deref** — per-deref is whack-a-mole across N sites (Turok hit ~7 for one cinematic).
Gate the guards behind `PLATFORM_PORT`; they go inert once real data loads.

---

## 5. Streaming & relocation — re-acquire, don't null

Engines stream assets through a small cache that **relocates** buffers as you explore (re-decompressing at a new
address). Long-lived pointers into them — the player's current collision region, etc. — go **stale**, and the
engine never updates them because on N64 the stale read was harmless.

- **Re-acquire by POSITION, not by pointer-validity.** A pointer check (NULL / N64-range / too-far) catches a
  *bad pointer* but NOT a **valid-but-WRONG** resource — e.g. a region whose pointer is fine but that **doesn't
  contain** the entity's new X/Z, so the ground query finds nothing and the entity free-falls through the floor.
  Re-acquire the correct resource by position (nearest-region-at-pos). *(This was Turok's death fall-through:
  the respawn left the player on a valid region that didn't contain them — two audits blamed interpolation
  before a state trace showed it was the region.)*
- **Scope the re-acquire to the event that causes the staleness** (e.g. the cinematic window), so normal play
  keeps the engine's own cheaper tracking.

---

## 6. The present pipeline (the "renders forever / never shows" bug)

The N64 main loop submits a gfx task per frame and relies on the **scheduler thread** to present finished tasks
(`osViSwapBuffer` in the retrace handler). On a **cooperative** host that scheduler thread never runs → frames
render but are **never presented** → the render-frame counter never advances → infinite re-render ("hangs at
frame 4 / renders forever"). **Fix: present right after dispatching the gfx task**, in the task-submit seam
(`scSendCommand` / equivalent), replicating what the scheduler would have done. This is frequently *the* "first
pixels" blocker and looks like a performance problem when it's a control-flow one.

---

## 7. Angle-wrap infinite loops

Iterative angle normalizers — `while (a >= 2*PI) a -= 2*PI;` — spin ~1e17× (apparent hang) when fed a garbage /
huge angle from an unspawned or uninitialised AI instance off-N64. Replace with O(1) `fmodf` wraps under
`PLATFORM_PORT`, mapping NaN→0. (And remember §3 — `fmodf` needs a prototype, or the "fix" returns garbage.)

---

## 8. Frame pacing: decouple logic from render, then interpolate

N64 game logic is sized for a **fixed step** (commonly a 30fps tick). Render at 60/144 while ticking logic every
render frame and everything runs **2–5× too fast**.

- Tick **logic** at the native rate (e.g. 30Hz); render **uncapped** (vsync); **interpolate** between ticks for
  smooth motion at the correct speed.
- **The tick clock MUST phase-accumulate** — advance the deadline by **exactly one interval** each fire, NEVER
  reseed it to `now`. Reseeding *beats* against an equal-rate vsync (when the tick interval ≈ the frame time) →
  randomly skipped ticks → **"feels like 30 on a 60Hz panel."** This is subtle and masquerades as a different
  bug; the speed intuition ("TICK=30 looks smoother") is a red herring — 30 is just *stable* against 60Hz.
- Interpolate by **snapshotting** state at the tick, **lerping** per render frame (angle-wrap for yaw/pitch,
  nlerp/slerp for quaternions), and **restoring the exact tick state immediately after the draw** so gameplay
  stays bit-exact. **Snap (don't lerp) across genuine teleports/warps** — gate the snap on the cinematic/teleport
  *state*, not a distance threshold (short teleports slip under a distance guard).
- **★ THE RENDER-ONLY-FRAME TRAP (the decouple's #1 latent bug class).** Once you tick logic at 30Hz but render
  faster, most frames are **render-only** — the logic step is frozen (`frame_increment == 0`) and only the
  render/interpolation advances. ANY game logic that **(a)** compares a `frame_increment`-decremented counter for
  **exact equality** to detect a one-shot transition, or **(b)** **snapshots/restores** state assuming every frame is
  a logic tick, **misbehaves on render-only frames — and ONLY when render-rate > tick-rate.** Real examples (all from
  one port, found the hard way): a respawn/warp repositions an entity *outside* the per-frame loop, so the interp's
  stale snapshot is restored *over* the new position on the next render-only frame → the entity is dragged back →
  death/warp loop; a "regenerate appearance" one-shot keyed off `counter == INITIAL` re-fires forever because the
  counter can't decrement on the render-only frame where its gate clears → the enemy loops the spawn, never fights.
  **Fixes:** for snapshots, add a **discontinuity guard at the TOP of the update** (before any logic runs the live
  position must equal the last restored snapshot — if it diverged, a reposition happened outside the loop, so
  re-snapshot); for exact-equality one-shots, **nudge the counter off the exact value** when the gate fires, or use a
  real flag. **★ THE TESTING RULE: verify EVERY respawn / warp / teleport / AI-timer / cinematic fix at
  render-rate > tick-rate (e.g. 120/30, or uncapped) — NEVER at render==tick.** A matched-rate run has **no
  render-only frames** and gives a **FALSE PASS** that masks the entire class. When a fix works in your headless
  harness but the user still reproduces it, **render==tick masking is the first suspect** — match the user's pacing.

---

## 9. Debugging methodology that actually works here

- **gdb often can't unwind through the asm `memcpy`; 32-bit ASan aborts in its own init and prints nothing.**
  Instead use a `-Wl,--wrap` **memcpy guard** that records the last copy and installs a SIGSEGV handler printing
  the fault address + last copy + a short backtrace, then `addr2line -f -e <bin> 0x<addr>` to map the game frame.
  The loop: run → fault addr+bt → addr2line → inspect that source → fix (usually a missing swap) → rebuild →
  repeat. Each cycle advances the boot one crash further. Also log every DMA `(dst, romAddr, nbytes)` and
  bounds-check it — a wrong size/offset is an endianness/parse bug caught early.
- **Verify a render change by VIEWING pixels — count distinct color buckets — not tri-count or nonblank-pixel
  count.** A fog fill is "nonblank" and emits hundreds of *clip-rejected* tris; it looks like it's rendering
  when it's blank. (Bisect proof on Turok: 34 colors = renders, 1 color = blank fog.)
- **When a "camera/render" bug can't be reproduced/verified in the harness, suspect the AUDIT, not the code.**
  Get a per-frame **STATE TRACE** of the actual variables (player pos + region, the quaternion magnitude, the
  region-valid flag…) *before* committing a fix. On the Turok death fall-through, **two independent audits
  blamed render-interpolation**; a one-line position+region trace proved it was collision the entire time. A
  state trace beats any number of plausible-sounding audits — and the failed fixes become code smell you then
  have to clean up.

---

## 10. Build & headless-rendering infra

- **NFS defeats `make`.** On an NFS mount with unreliable mtimes, `make` skips rebuilds of changed files and
  links **stale** objects (a header edit silently won't take). Use a build **script** that compiles every TU
  fresh to a **local** dir. Objects are keyed by `-D` flags → **clean-build after any flag change**.
- Build **-m32** for N64 pointer width (the source assumes 32-bit pointers in places; structs/casts depend on it).
- **Headless hardware GL with no X server:** EGL via a **GBM render node** —
  `open("/dev/dri/renderD128")` → `gbm_create_device` → `eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR)` → an FBO
  render target → `glReadPixels`→PNG. The **surfaceless** EGL path may hang in driver init; **GBM is the
  reliable one.** Confirm 32-bit GPU drivers exist for the render node (a 32-bit build needs the 32-bit DRI
  driver). Switching backends needs a **fresh** object dir on NFS (stale `.o`s aren't cleaned).
- **Scissor-on-FBO gotcha:** a degenerate (zero-height) `G_SETSCISSOR` interleaved with the real full-screen one
  produces a zero-area GL scissor that clips **everything** on an FBO → blank frame. **Ignore zero-area scissor
  rects** in the backend.
- **One depth space.** PICA NDC is `[-1,0]`, GL is `[-1,1]`; clear depth to FAR and normalize in the Fast3D
  seam, not per-backend ad hoc.

---

## 11. The `port/` layout (mirror banjo-kazooie / perfect_dark)

All three sibling ports share this shape. **Copy it; don't reinvent.**

```
port/
├── fast3d/      ← the Fast3D interpreter + backends (copied from banjo, adapted to your microcode)
│   ├── gfx_pc.cpp / gfx_pc.h        ← the F3DEX interpreter — fix decode bugs HERE, once
│   ├── gfx_cc.cpp / gfx_cc.h        ← color-combiner → TEV decoder (portable)
│   ├── gfx_opengl.cpp               ← PC backend (the ground truth)
│   ├── gfx_egl.cpp                  ← PC headless hardware GL (GBM, frame capture)
│   ├── gfx_sdl2.cpp                 ← PC window manager
│   ├── gfx_citro3d.cpp / gfx_3ds.c  ← 3DS backend (PICA200 TEV) + window manager
│   ├── glad/                        ← GL loader
│   └── shaders/
├── include/     ← platform.h, system.h, video.h, audio.h, input.h, romdata.h, mixer.h, fs.h,
│                  <game>_swap.h, <game>_trace.h   (the platform-abstraction headers)
├── src/
│   ├── <game>_main.c     ← host boot driver (replaces idle/main-thread bring-up)
│   ├── os_shim.c         ← libultra shims (osCreateThread, osSpTaskLoad, osViSwapBuffer, osAiSetNextBuffer…)
│   ├── <game>_rcp.c      ← RCP/task seam: gfx tasks → gfx_pc; audio tasks → mixer
│   ├── romdata.c         ← the single DMA seam: piRead(dst, romAddr, n)
│   ├── <game>_swap.c     ← endian byteswap (or inline per-consumer — see §2)
│   ├── <game>_runtime.c  ← segment / ucode / linker-symbol stand-ins
│   ├── mixer.c           ← RSP audio microcode → C
│   ├── audio.c / audio_3ds.c, input.c / input_3ds.c, sys_3ds.c, fs.c
│   └── backend_headless.c ← null renderer for the boot-test milestone
└── tools/       ← memcpy_guard.c, asset tools
```

**Naming convention (consistent across banjo & PD):**
- **Game-prefixed** files (`bk_*`, `<game>_*`) = the **engine-specific glue** — the bridge into the game's own
  gfx/boot/rcp/runtime/swap.
- **Plain names** (`os_shim`, `romdata`, `mixer`, `audio`, `input`, `fs`, `backend_headless`) = the **shared,
  portable platform layer** — the parts that look the same in every port.
- **`_3ds` suffix** = the 3DS-specific implementation of a shared role (`audio_3ds.c`, `input_3ds.c`,
  `sys_3ds.c`).
- Banjo keeps the perfect_dark originals as **`*.pdref`** next to their adapted `bk_*` — a handy audit trail of
  what each file was derived from.

Milestone discipline (lockstep across the siblings): **M0** compiles → **M1** boots headless → **M2** PC-GL
renders the title (ground-truth checkpoint — nothing 3DS until green) → **M3** 3DS renders the title → **M4**
audio → **M5** first level loads + walkable → **M6** combiner fidelity (fog/particles/HUD) → **M7** 3DS memory
tuning. **Stand up the PC OpenGL ground truth FIRST** — it iterates ~100× faster than the 3DS and every bug
reproduces identically through the shared Fast3D seam.

---

## 12. Use multi-agent workflows for the endian sweeps

The big repetitive task — swapping dozens of asset structs — **parallelizes** well, and the **encode path is the
ground-truth spec** (§2), which makes it verifiable rather than guesswork:

- **Fan out** one agent per file group, each **diffing the decode path against the encode path** field-by-field
  and applying the missing swaps.
- **Adversarially verify** each finding (a struct that "needs swapping" but is actually a payload that should
  stay big-endian is a false positive).
- Apply **crash-by-crash with build verification** — the memcpy-guard loop (§9) tells you exactly which struct
  is next.
- The same pattern cleans up afterward: a fan-out per-file sweep to strip debug knobs, or to audit every
  `((float*)tbl)` cast for the lookup-table trap (§2).

3DS-specific catalogue (from Forsaken / Perfect Dark): `__stacksize__ = 2 MB` (default 32 KB is too small);
`svcGetSystemTick()` not `gettimeofday()`; GPU-visible buffers from `linearAlloc`, not `malloc`; name your VFS
init so it doesn't collide with libctru `fsInit`; ARM11 (ARMv6K) **faults on misaligned loads** x86 tolerates —
run a UBSan alignment pass on the PC build before every 3DS push and use `memcpy`-style accessors
(`-mno-unaligned-access` does **not** save you); always full power-cycle between hardware tests.

---

## 13. Audio: the classic libultra ABI, the bank, the rate, and the placeholder trap

Audio is its own world. The DSP is portable; the friction is the ABI, the addressing, the rates, and the assets.

- **The N64 already runs audio on its own thread — make that thread REAL.** `audiomgr` does
  `osCreateThread(THREAD_AUDIO, __amMain)`; the cooperative host no-ops every thread so audio never runs. Make the
  AUDIO thread a real `pthread` (PC) / `threadCreate` (3DS) while game + gfx stay cooperative — exactly the N64's
  own split (and the PD/banjo/Forsaken pattern). A **recursive** synth-lock mutex guards the voice list (game
  thread mutates on SFX triggers; audio thread reads it in `alAudioFrame`). Keep the lock NARROW (around the synth
  walk, not the mixer) or lock contention starves the game thread.
- **Two N64 audio ABIs — classic vs n_audio — need DIFFERENT mixers.** banjo/PD/SM64 use **n_audio** (the
  `aXxxImpl` kernels carry their in/out/count inline). Older titles (Turok) use the **CLASSIC** ABI: a STATEFUL
  `aSetBuffer` (`A_SETBUFF`) sets the DMEM in/out/count for the *following* op, out-of-band. The DSP math is
  **identical** — lift PD's `mixer.c` scalar kernels verbatim (VADPCM decode, polyphase resample, envelope mixer,
  gain-mix, interleave). The only new code is a ~150-LOC **packed-Acmd dispatch loop + a SETBUFFER latch** that
  n_audio never needed (the A_MAIN + A_AUX register triples — A_AUX *overloads* its 3 fields as the envmixer's
  MAIN_R/AUX_L/AUX_R targets). Decode each packed `Acmd` by hand (shifts, not the MSB-first bitfield struct) to be
  endian-safe. The synth (`alAudioFrame`) is portable C and builds the Acmd list; you only replace the RSP.
- **★ Address masking is the #1 audio hazard.** `osVirtualToPhysical(p) = p & 0x1FFFFFFF` and the `K0_TO_PHYS`
  family mask the top bits — harmless for low static/BSS addresses, but they **corrupt malloc'd bank pointers**
  (high on a host). EVERY DRAM address in the Acmd list (`LOADBUFF`/`SAVEBUFF`/`LOADADPCM`/decoder state) flows
  through them. **Make them IDENTITY under `PLATFORM_PORT`** (the host heap *is* physical). And pass the synth its
  REAL output pointer, not the masked one, so the final `A_SAVEBUFF` writes straight into the device buffer.
- **The bank `.ctl` is big-endian — swap it with a host `BnkfNew`.** The
  `ALBankFile → ALBank → ALInstrument → ALSound → ALWaveTable / Envelope / ADPCMBook / loop` tree is big-endian;
  swap every offset/count/scalar **during the relocation walk**, after each struct's `flags` guard so shared nodes
  swap exactly once. The `.tbl` VADPCM payload stays raw big-endian; the keymap (all `u8`/`s8`) needs no swap. The
  SGI `alBnkfNew` only RELOCATES (N64 is natively big-endian) — the host version *adds* the swap, so write your own
  and call it instead. Watch for two missing symbols: `adpcmDecode` (the per-node ANIMATION ADPCM decoder, shipped
  as MIPS asm — port the in-tree C reference; a no-op stub collapses every animated model to a point) and any
  effect symbol referenced-but-undefined in the leak (`alReverbSetType`) — host-stub it in a TRACKED file.
- **★★ `ALBank.sampleRate` is the RECORDING rate, NOT the playback rate.** The classic synth plays each voice at
  ratio = `2^(cents/1200)` (a pure musical ratio) with **no runtime sampleRate/outputRate correction** — that
  factor was baked into the keymap (`keyBase`/`detune`) at bank-BUILD time. So **the host device + synth output
  rate must equal the bank's STORED sample rate**, which is NOT necessarily its `sampleRate` field. PD honors this
  (device 22020 = bank stored 22020); SM64/SoH additionally carry a runtime `32000/gAiFrequency` reconciliation the
  classic ABI **lacks**. The diagnostic: a rate mismatch makes **ALL** sounds wrong-speed by a uniform factor (a
  global octave shift) — vs a per-sample tuning error, which is wrong on only *some* instruments. Match output to
  the STORED rate (find it by ear / the known shipped `OUTPUT_RATE`), not to the metadata field. *(Turok's dev and
  retail banks both said 44100, but the dev samples were 44100-stored and the retail 22050-stored — chasing the
  field sent us 2× the wrong way twice.)*
- **★★ A leaked DEV tree's bundled audio can be PLACEHOLDER from another title.** Turok's
  `src/PR/tengine/sfx.ctl/.tbl` were **sports-announcer scratch samples** (Iguana reused another game's bank during
  development) — nonsensical in-game ("right on the concrete floor"). The SHIPPED audio lives in the RETAIL ROM as
  plain `B1` `ALBankFile` segments. Find them by **signature-scanning the ROM for `b'\x42\x31'` ("B1" revision)**
  with a sane bankCount / instCount / sampleRate, then **walk the ALBank tree** to get each `.ctl`/`.tbl` extent
  (cross-check the `.tbl` offset against the ROM's segment alignment — Turok's was 16 bytes). Load THOSE, not the
  dev files. **General rule: a leaked tree's bundled assets are dev-state — verify every one against the retail ROM**
  (this is the audio sibling of the level-geometry "v49 placeholder vs retail" finding).
- **SFX trigger latency = the audio buffer depth.** The audio thread buffers up to a back-pressure threshold ahead
  of the device; a freshly-triggered SFX waits behind it. PD's 8192 samples ≈ **371 ms @22050** = an audible ~¼ s
  delay. Lower the threshold (2048 ≈ 93 ms) for responsive SFX — the thread refills every ~2 ms, so it stays clear
  of underrun. This is purely a latency-vs-underrun trade; start conservative and tighten by ear.
- **Headless audio testing: pace the GAME, not just the audio.** Audio is inherently real-time; a headless game
  runs *faster* than real-time, so the audio thread can't catch transient SFX. (a) Pace the per-frame PRESENT to
  real-time (a gated `nanosleep`) so game and audio stay in sync. (b) Give a non-device sink (WAV dump / null)
  **synthetic back-pressure** — `produced_bytes − drained_at_the_sample_rate` — or the thread spins a core and
  starves the game. (c) The flush must **NOT** re-check the back-pressure (the producer loop already gates it) or
  it drops every frame produced right at the LIMIT boundary (a paced 10 s capture yields 0.4 s). Then capture to a
  WAV and measure peak / dominant-frequency / duration. A 2× pitch ratio between two captures confirms a rate change.
- **Compressed-sequence / MIDI music: swap the sequence HEADER, leave the event stream raw.** Music in the classic
  ABI is a Compressed-Sequence Player (CSP) fed an `ALCMidiHdr` = a fixed-size **big-endian DWORD table** (N track
  offsets + a division word). The sequence parser (`alCSeqNew`) reads each track offset raw and walks `base+offset`
  into a track pointer — so on a LE host the un-swapped offsets are **wild pointers → SIGSEGV**. Swap the header
  DWORDs once at load (right after the memcpy into the play buffer, *before* the parser runs); the **compact-MIDI
  event stream after the header is byte-oriented** (status / var-len delta / data + byte-assembled tempo & loop
  offsets) and is **endian-neutral — leave it raw** (same rule as the VADPCM sample payload). Two gotchas: (1) the
  music INSTRUMENT bank is a separate `ALBankFile` — swap it like the SFX bank; (2) a leaked DEV tree may ship music
  **stubbed OFF** (e.g. a `LoadSeq` that just `return FALSE`s) — re-enable the load path; don't assume the leak's
  default state is what shipped.

## 14. The GPU pipeline: Fast3D transforms on the CPU — keep it there

The single biggest architectural fact about a Fast3D-based port (`gfx_pc.cpp`, the SM64→Banjo→PD→Turok lineage):
**the vertex transform happens on the host CPU, not the host GPU.** Fast3D *interprets* the N64 RSP display list, and
on the real N64 the **RSP did that MVP transform in HARDWARE** — the RSP is a programmable vector coprocessor running
microcode (F3D/F3DEX), i.e. the N64 (1996) was itself an early hardware-T&L machine, three years before the consumer
PC's GeForce 256 (and the PlayStation's GTE did hardware T&L in 1994). So `gfx_pc` is *re-emulating the RSP's hardware
T&L in software* — it does the **full MVP on the host CPU** and hands the backend **already-projected clip-space**
geometry. The host GPU vertex shader is almost a pass-through (on 3DS it adds only the panel rotation + PICA depth
remap). (Lifecycle of that transform: **hardware** on the RSP → **software** on the host CPU → and "GPU-MVP" tries to
push it back to **hardware** on the host GPU — which is where it gets entangled; see below.)
Contrast a **native-engine** port like **Forsaken**: its *original* 1998 engine also transformed on the CPU (no
consumer *PC GPU* had hardware T&L until the GeForce 256 in late 1999, so PC games of the era did software T&L on the
CPU — even as consoles already had dedicated geometry hardware), but its Citro3D port
**deliberately moved that transform onto the GPU** — it uploads **model-space** verts + MVP uniforms and the PICA
vertex shader computes `projection·(modelView·inpos)` (`render_c3d.c`: *"GPU-side modelview+projection transforms via
PICA200 vertex shader"*; `inpos` is in MODEL space). For a native engine that relocation is a clean CPU **offload**.
So the decisive question is **not** "CPU or GPU" in the abstract — both engines started on the CPU — it's whether the
transform stage is **separable**, and that's what splits the two port types:

- **★★ Do NOT "move the MVP to the GPU."** It looks like a free win (offload math to the GPU vertex shader), but it
  is a **regression** for a Fast3D port: the interpreter already computed the transform on the CPU, so a GPU-MVP path
  either redundantly re-transforms or forces you to ship un-projected verts the rest of the pipeline doesn't expect.
  **★ Empirically settled — Perfect Dark BUILT this exact move on a `3ds-gpu-mvp` branch and measured it a WASH
  (~9.0 vs ~9.2 ms host frame), explicitly *not* freeing CPU cycles** — because the CPU still computes clip-space per
  vertex for cull/fog/near-clip/stereo, so the MVP ends up **DUPLICATED** on CPU+GPU, not moved (the naive variant +
  GPU-skinning actually *regressed*, to ~9 fps from 20-30). The matrix multiply is only ~15-40% of *one* per-vertex
  stage; the rest (`G_VTX` unpack, lighting, texgen, trivial-reject, fog) stays on the CPU regardless — gfx_pc *is*
  the interpreter and must walk every vertex. So you'd save a sliver and *add* a per-**draw** modelView upload (at
  ≤32-vert `G_VTX` granularity, since Fast3D has no per-object concept). **★ The subtlety (don't mis-state it):
  Forsaken's port *also* moved a CPU transform to the GPU — and there it was a WIN.** Same operation, opposite
  outcome, because of **separability**. A native engine's transform is a *separable* stage: relocate it and the CPU is
  cleanly offloaded, with nothing downstream needing the CPU-side result. In Fast3D it is *entangled* — the CPU-side
  near/far/frustum **clip + cull**, the per-vertex **fog**, and the clip-space-shear **stereo** all consume the
  CPU-computed clip-space positions, and the per-vertex DL walk stays on the CPU regardless — so GPU-MVP surrenders
  those for nothing. The right question is *whether the transform can be cleanly detached from the rest of the
  pipeline*, not just *where it runs*. **The genuine way to use a spare core here is the RENDER-THREAD SPLIT** (move
  the whole `gfx_run` onto core 2 — it offloads the *entire* per-vertex walk, ~10× the multiply, with zero fidelity
  risk because the proven clip/cull/stereo/fog math is *relocated byte-identical, not rewritten*), NOT GPU-MVP. The
  decision gate is one **on-device PROF capture** (CPU-bound ⇒ the split is the lever; GPU-bound ⇒ chase texture
  format + state-batching) — see `docs/3DS_PERFORMANCE.md` B1/M1.
- **★ Single-pass stereo: shear the clip space, don't re-project.** Because the projection is already baked into the
  verts on the CPU, you **cannot change the projection per eye** without re-running the whole CPU interpreter (≈2× the
  dominant cost). So PD/Turok apply stereo as a **post-projection clip-space shear** — record the draw stream once,
  then replay it for the second eye with a small per-eye `transform` that adds `eyeSign·(SHEAR_Z·z + SHEAR_W·w)` to the
  horizontal clip coord (after the perspective divide that's a depth-proportional disparity = parallax). The heavy CPU
  pass is **shared** between eyes; only a cheap shear differs. Forsaken's port does the *geometrically
  correct* thing instead — an **off-axis projection shift per eye** (re-upload proj, GPU re-transforms) — which it can
  afford precisely *because* it relocated the transform to the GPU, so re-transforming the 2nd eye is just another GPU
  pass. **Neither is "drift": each method fits its pipeline.** The shear is
  a slight approximation (it ignores the per-eye scale change of a true off-axis frustum), but at the 3DS's tiny IOD it
  is visually indistinguishable and **the only cheap option for Fast3D.** Porting the off-axis method into a Fast3D
  engine would double the CPU cost; the proof is Forsaken's *own* non-Fast3D fallback backend (picaGL), which has no
  record-replay and therefore **disables stereo entirely** ("no display-list replay = half framerate").
- **Record-once / replay-per-eye is the enabling trick.** The backend records the CPU-transformed draw stream
  (`sCmds`/VBO uploaded once) and replays it; mono = one replay, stereo = two with different shear. This is also why
  the per-eye cost is *draw submission*, not geometry — so the real stereo optimization is **deduping per-draw GPU
  state in the replay** (only re-issue depth/blend/TEV/viewport/scissor/texbind when it changes), which speeds up the
  mono frame too. (See `docs/3DS_PERFORMANCE.md` for the full per-frame optimization list.)
- **Even moving the WHOLE per-vertex pipeline to the GPU ("full hardware T&L") doesn't free a Fast3D port.** Two
  things stay on the CPU regardless: the per-frame **DL walk** (opcode dispatch, segment resolution, state tracking,
  the combiner→TEV key build, texture cache) and the per-vertex **VBO pack** (N64 tile-UV / shade / prim / fog
  expansion — RDP state, not model-space, so it *can't* move to the GPU) — together ≥50% of the interpreter's cost.
  And on PICA specifically there's a hard blocker: no `GL_DEPTH_CLAMP`/geometry-shader means the **near-clip is a CPU
  clip-space op with no hardware home**, so clip-space can never fully leave the CPU. Perfect Dark built the full
  pipeline and measured a wash; the scope is multi-week with an unfalsifiable-in-emulator payoff. The CPU levers that
  actually work are **threading** (offload the *whole* walk to a spare core — byte-identical, no fidelity risk) and
  **retained-mode / DL caching** (skip re-walking static geometry), NOT pushing T&L to the GPU. (Full Turok scoping:
  `docs/3DS_PERFORMANCE.md`.)
- **Lesson:** when a "speed up the GPU" idea appears for a Fast3D port, first ask *where the transform happens*. The
  CPU already did it — most "use the GPU more" ideas just add a second copy of work. The genuine GPU-side wins are
  about **what you submit** (state-change batching, texture format/bandwidth, draw count via game-side culling), not
  about re-deriving geometry the interpreter already produced.

## 15. Real-hardware-only rendering bugs: don't analyze, *localize* (and the PICA alpha=0 blend rule)

A bug that reproduces on the **real device but never in the emulator** (Citra/Mandarine HLE) is a different
species from a logic bug, and the usual static code analysis is a trap. The C runs **byte-identically** on the
device and in the emulator — same combiner decode, same texture bytes, same blend setup — so if HLE is always
correct and hardware misbehaves, **the bug is in how the silicon executes a correct command stream**, not in your
code. Add **intermittent / angle-dependent / flickering** and you have a GPU-execution quirk (cache, sync, blend,
coverage, depth) that no amount of staring at the combiner will reveal.

- **Stop guessing; localize empirically with an on-device, flag-gated A/B that needs NO rebuild per variant.**
  Gate the experiments behind a value your config reader already loads from the SD (`turok.cfg`), so the user
  flips a number and relaunches instead of waiting for a rebuild+redeploy cycle. One deployment then splits the
  whole hypothesis space. The Turok torch-flame "yellow square" took **six blind fixes** (texture format,
  combiner math, NPOT padding, dead-TEV-stage collapse, stale-slot, cache-coherency — all refuted by the
  *intermittent + HW-only* profile) and then **one flag-gated probe** to solve: forcing an alpha-test on the
  suspect draws showed the transparent fragments' alpha really was 0 (the test discarded them cleanly) — proving
  the **blend**, not the texture/TEV, was the culprit.
- **★ The PICA200 SRC_ALPHA blend can render `alpha == 0` fragments as OPAQUE (intermittently).** On real
  silicon the standard `GPU_SRC_ALPHA / GPU_ONE_MINUS_SRC_ALPHA` blend does not reliably honor a zero source
  alpha, so any alpha-blended surface with fully-transparent texels (sprites, flames, smoke, HUD overlays,
  menus) can flicker its transparent regions to a solid coloured quad. HLE always honors alpha 0, so it's
  invisible in the emulator. **Fix (general, DRY — one rule for every alpha-blended draw, not a per-effect
  patch): give such draws a `GREATER 0` alpha-test to discard zero-coverage fragments.** It's a *mathematical
  no-op* for correct blending — an `alpha==0` fragment contributes `src*0 + dst*(1-0) = dst` (nothing), so
  discarding it yields the identical pixel — so it cannot harden a soft edge or erase anything visible; it only
  removes the wrong-opaque artifact. It is also faithful to the N64's `CLD_SURF` coverage semantic (`CLR_ON_CVG`
  = don't draw zero-coverage). **Exclude modulate (`DST_COLOR`) blends** — there alpha isn't coverage, so a
  zero-alpha fragment still multiplies the destination and must not be discarded. Put the rule in the single
  per-draw alpha-test decision (one source of truth). The PC/GL backend honors alpha 0 correctly and is a
  separate TU — leave it alone. (Reusable across every Fast3D→Citro3D port that shares this backend.)
- **General rule for the bug *class*:** when a render artifact is **HW-only + intermittent**, match the user's
  exact runtime config (it often only appears at a particular FPS/tick/angle/scene), reach for a **flag-gated
  on-device probe** before any fix, and prefer a fix that is **provably a no-op in the correct case** (like the
  zero-coverage discard) so generalizing it across all draws can't regress anything.

## 16. REDUNDANT per-draw register writes provoke PICA blend quirks — DEDUP them (a perf win that's also a HW fix)

This one cost a long on-device bisection on Turok and is **the most counter-intuitive PICA finding so far** — bank
it, because it will recur on any Fast3D→Citro3D port.

- **The symptom:** translucent billboard/particle sprites (flames, smoke, sparkles) render **OPAQUE** at some
  camera angles — whole-batch, intermittent, **real-hardware-ONLY** (HLE emulators never show it). Same fingerprint
  as the §15 alpha=0 quirk, and an on-device probe confirmed the fragments' alpha really IS 0 — so it's a silicon
  execution quirk, not a data/combiner/texture bug.
- **The non-obvious root cause:** **re-issuing the SAME fixed-function register writes on EVERY draw** is itself the
  trigger. Fast3D faithfully re-asserts `C3D_DepthTest`/`C3D_DepthMap`/`C3D_AlphaBlend`/`C3D_AlphaTest` per draw
  (the N64 DL re-states them constantly). On real PICA200 that redundant write storm intermittently corrupts the
  blend so `alpha==0` fragments come out opaque. It is the **redundancy of the writes, not their values**.
- **The fix = a per-draw GPU-STATE DEDUP.** Shadow the last-applied *effective* depth/blend/alpha state and SKIP
  the `C3D_*` call when it's bit-identical to what's already set; invalidate the shadow at each record/replay pass
  start (per eye) and after any path that writes that state directly (e.g. a mid-frame depth clear). The applied
  values are **byte-identical** — you change only *how often* the registers are written. This is the exact same
  optimization §14 names as the real **single-pass-stereo perf win** (the per-eye cost is draw *submission*, and the
  second eye re-issues every state write): so **one change is both a measurable perf win AND the cure for a
  hardware render bug.** It's also DRY-compatible with the §15 zero-coverage alpha-test (resolve the effective
  alpha-test once, then dedup-apply it).
- **★ Don't trust a banked root-cause that a live measurement contradicts.** This bug was mis-banked for weeks as
  "FCRAM texture eviction, fixed by a smaller texture format (RGBA5551)" — but the on-device heartbeat showed
  `texOOM=0` with tens of MB of GPU heap FREE the whole time, i.e. *nothing was being evicted*. That single
  reading should have killed the theory; instead a whole "fresh start" branch was built around the wrong fix
  before bisection found the truth. **A counter-measurement beats a confident note.**
- **The bisection method that cracked it (reusable):** build ONE change at a time and A/B each on real HW
  (`fix-A`, `fix-B`, `A+B`, full). When the full build is good but the isolated pieces aren't, do a **one-line
  "defeat the suspect" test** on the *known-good* full build — here, force the dedup to never engage
  (`sStValid = false`) so it re-applies every draw like the broken baseline. If the bug returns, that suspect IS
  the fix, proven, in a single flip. Far faster than additive guessing once you suspect a specific mechanism.

---

## 17. The camera sees THROUGH walls it hugs — the near clip is an N64 16-bit-z-buffer relic

**Symptom (real-3DS / PICA only, NOT desktop GL):** walk the first-person camera right up to a wall and you see
*through* it — the wall face vanishes / goes transparent and you see the void or the next room behind it. The
*symptom* was reported on BOTH this port AND the sibling Perfect Dark 3DS port — but **the fix is ENGINE-SPECIFIC:
it fixed Turok ("looks fantastic" on HW); the identical change was a NO-OP on PD and was dropped** (why: see the end
of this section). It looks like a collision bug ("the camera is inside the wall"), and the collision *is* the
enabling factor (collision verts == render verts, so the eye can sit flush against the rendered surface), but the
actual cause is the **near clip plane**.

**Root cause — a too-FAR near plane meets the PICA's hard near-clip.** N64 games set a large near clip because the
console's **16-bit z-buffer** needed a tight near/far ratio for usable depth precision (Turok: `SCALING_NEAR_CLIP =
far/64 = 16` units; Perfect Dark: per-level `env->near`, typically **15**, default 30). The microcode is usually
**F3DEX No-Nearclipping** (`gspF3DEX_NoN`), so the *N64 itself* never near-clips and never shows the bug. Two
things then conspire on the port:
- **The eye can get within `near` units of a wall** (collision standoff < near clip). When it does, the wall is
  **entirely behind the near plane** in clip space.
- **The PICA has no `GL_DEPTH_CLAMP`** — it *hard-clips* geometry crossing/behind the near plane. Desktop GL clamps
  per-fragment instead, which is why the **bug is 3DS-only** (the PC-GL reference renders the hugged wall fine).

Mature Fast3D→PICA backends already ship a **§29-style near-plane clipper** (Sutherland-Hodgman: split a
*straddling* tri at the near plane, insert interpolated verts on it; for an *all-behind* tri, clamp each vert's
clip-space `z` to the plane). But that emulation has a hard limit: it can only fix **depth** for an all-behind
vertex — it **cannot fix that vertex's x/y projection** (a point behind the eye projects to garbage). So once the
whole wall is inside the near distance, §29 can't save it and you get the see-through. **The big near plane is what
lets the eye reach that state.**

**The fix — pull the near plane WAY in; you have the depth bits now.** The 16-unit value is obsolete: the 3DS (and
every PC) has a **24-bit depth buffer = 256× more depth values than the N64's 16-bit**. Drop the *projection* near
to ~4 units and the eye can no longer get within it of a wall in normal play, while far-plane precision stays far
better than the N64 ever had (`near/far = 4/1024` ≈ ratio 256 at 24-bit ≫ the N64's ratio 64 at 16-bit). User-
confirmed it "looks fantastic" on **Turok** HW. Make it a **config knob** (Turok `turok.cfg nearclip`, default 4)
so the precision↔clip trade is tunable on-device without a rebuild: *lower* (2) if a wall still clips, *higher* (8)
if distant z-fighting appears.

**★★ But it's NOT universal — verify on HW per-engine.** The identical clamp on **Perfect Dark** (the `viWorldNear()`
helper below) was **A/B'd on real hardware and made NO visible difference**, so it was dropped. Why it helped Turok
but not PD: PD already ships a robust §29 software near-clip emulation AND its collision keeps the camera far enough
that stock near=15 never makes a *collision* wall near-cross; PD's remaining see-through is the **point-blank
NON-COLLISION** decorative geometry (cliffs you can touch at <1 unit), and PD's own logs already proved a near
reduction (even to 0.5) does NOT fix *that* (the §29 residual is the best achievable there). Turok's bug was the
common collision-wall case, which the pull-in *does* fix. **So: the near pull-in fixes the collision-wall case where
the engine LACKS sufficient near-clip handling; it's a no-op where a §29-style emulation + collision already prevent
the crossing. Build it, A/B it on HW, and keep it only if it moves the needle.**

**★ The one trap that makes this non-trivial: in some engines the near value also feeds the FOG/shade math.** Turok
uses the near clip *only* in the projection (`guPerspectiveF`), so changing it is a clean one-liner. **Perfect Dark
does NOT** — its `env.c` computes the per-vertex fog/shade alpha (`alphafar`/`alphanear`) from the *stored* `znear`
**independently of the projection**. So you must clamp the near **only where the projection MATRIX is built**, and
leave the stored `znear` (the fog's input) untouched — else you shift the fog. Concretely for PD: the 3 world-view
`guPerspectiveF` calls in `src/lib/vi.c` all read `g_ViBackData->znear`; route just those through a
`viWorldNear()` clamp helper, don't touch `viSetZRange`/`g_ViBackData->znear`. (The vestigial `player->c_perspnear`
is stored-but-never-read — a decomp red herring; verify by grepping for the *consumer*, not the *setter*.)

**Generalizable checklist for "camera sees through hugged walls" on any N64→PICA/GL-clamp-less port:**
1. Find the near clip — a single `SCALING_NEAR_CLIP`-style constant, or per-level env data fed to `guPerspectiveF`.
   Note it's almost certainly sized for the N64's 16-bit z-buffer (a near/far ratio around 64).
2. Confirm the microcode is No-Nearclipping (so the N64 never showed it) and that the bug is **GL-clamp-less-backend
   only** (PICA/3DS yes, desktop GL no) — that pins it to near-plane handling, not collision.
3. Reduce the **projection** near to ~4 (24-bit depth makes this free); expose it as a tunable.
4. **Before you touch it, grep whether the near value also feeds fog/shade/anything-but-the-projection.** If it
   does (PD), clamp *only at the projection matrix build*, not at the stored value.
5. Scope it to the platform that needs it (3DS) if the desktop backend already depth-clamps; keep the §29 near-clip
   emulation — the smaller near just makes the case it can't handle (all-behind-near walls) essentially unreachable.

---

## 18. The first-person weapon clips through walls — the dropped mid-frame Z-CLEAR

**Symptom (3DS / PICA only, NOT desktop GL):** the first-person weapon/hand z-fights and clips INTO walls the
camera hugs. Distinct from §17 (that's the WORLD seen through; this is the VIEWMODEL clipping into the world).

**Root cause — N64 FPS games draw the viewmodel "always on top" by CLEARING THE Z-BUFFER mid-frame, and the
Fast3D port drops that clear.** The N64 trick: after drawing the world, **redirect the color image to the
z-buffer** (`gDPSetColorImage(..., zbuffer)`) and **`gDPFillRectangle` it with max-z** — that resets depth over
the weapon's screen area, so the weapon then draws on top of everything. (Turok: `CEngineApp__ClearZBuffer`;
SM64/PD do the same.) But the Fast3D interpreter **drops this fill** — `gfx_dp_fill_rectangle` early-returns when
the fill target == the z-buffer address (the comment says "already cleared at frame start"). On **desktop GL that's
invisible** because GL has `GL_DEPTH_CLAMP` and the weapon stays clean anyway; on the **PICA there's no depth-clamp
and it hard-clips**, so the dropped mid-frame reset means the weapon z-fights / clips into walls. (Same
desktop-GL-masks-it, PICA-exposes-it shape as §17.)

**The fix — route the z-buffer fill to the backend's depth-only clear (3DS).** When a fill targets the z-buffer,
instead of dropping it, call `gfx_rapi->clear_framebuffer(false, true)` — the mature Fast3D→PICA backends already
implement this as a **recorded full-screen depth-far quad** (replays in stream order: after the world, before the
gun), the same path the `G_CLEAR_DEPTH_EXT` GBI extension uses for an engine that emits it explicitly (PD). Turok
doesn't emit that extension — it uses the standard color-image-to-z + fill — so you bridge the standard form to
the same backend call. Gate it to the platform that needs it (3DS); desktop keeps the early return.

**Detection detail:** the early-return test is `rdp.color_image_address == rdp.z_buf_address`. For it to fire, the
game must have set the depth image (`gsDPSetDepthImage(zbuffer)`) to the same buffer it later redirects the color
image to — verify both point at the same symbol. The **frame-start full z-clear** uses the identical fill, so it
routes too; that's harmless (a redundant depth-far quad before any geometry — the RT is already depth-cleared).

**★ Residual (accept it): in very tight/point-blank spots a little weapon clipping remains** — the z-clear makes
the weapon draw on top of the WORLD, but the weapon's OWN geometry can still cross the camera near plane at
point-blank, which the PICA hard-clips (the §17 near-plane limit applies to the viewmodel too). The §29-style
near-clip emulation reduces it; a tiny near clip (§17) reduces it further; it doesn't fully vanish without
per-fragment depth clamp the PICA lacks.

---

*Distilled 2026-06-18 from the Turok: Dinosaur Hunter port (incl. the full classic-ABI audio pipeline: threaded
synth, software Acmd mixer, bank/rate/placeholder fixes). §17 added 2026-06-26 (the near-clip wall-see-through fix —
fixed Turok; tried-and-dropped as a HW no-op on Perfect Dark). §18 added 2026-06-26 (the viewmodel mid-frame
z-clear — 3DS weapon-through-walls fix). See `CLAUDE.md` for the project-specific log and `docs/REFERENCES.md` for
the per-sibling reference notes.*
