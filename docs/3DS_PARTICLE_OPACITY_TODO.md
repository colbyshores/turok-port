# 3DS Particle Opacity + Enlargement — DEFERRED TODO (minor / hardly noticeable)

> Status: **OPEN, deferred.** Low priority — the user reports it is minor and hardly noticeable. This doc
> records the full investigation so it can be picked up later without re-deriving anything.

## Symptom (user-reported, real New-3DS-XL hardware)

Translucent **billboard particles** — the **torch FLAME** and the animated **SPARKLES on key pickups** (and
similar glow particles) — at **certain camera angles**:
- render **fully OPAQUE** (lose their semi-transparency), and
- appear **slightly bigger AND blockier / lower-resolution** ("bigger pixels, though still filtered").
- It flips in **FULL BATCHES** (the whole batch at once, not per-pixel).

## Confirmed by the user (the discriminators that matter)

| Question | Answer | Implication |
|---|---|---|
| PC build vs 3DS? | **3DS only — PC is correct** | It's in the **Citro3D/PICA backend**, not shared game/Fast3D code. |
| Quad bigger, or same-size blockier? | **Both — bigger AND blockier** | Either a real geometry scale, or a stretched/opaque-filled texture. |
| 3D slider up or down? | **Happens either way** | **Not** stereo-specific — it's in the common mono render path. |

## Ruled OUT (with code evidence — these are dead ends, do not re-try)

- **`is2d` / stereo transform swap** — [gfx_citro3d.cpp:2579-2581](../port/fast3d/gfx_citro3d.cpp#L2579):
  `is2d` only swaps `sMonoTf` vs `eyeTf`. With 3D off, `eyeTf` is built with `eyeSign=0, zoom=1.0`
  ([line 3047](../port/fast3d/gfx_citro3d.cpp#L3047)) = identical to `sMonoTf`
  ([line 3019](../port/fast3d/gfx_citro3d.cpp#L3019)). So the swap is a **no-op with 3D off**, and the user
  confirmed the bug happens with 3D off. Not the cause.
- **Geometry scaling in the backend** — [gfx_citro3d.cpp:2011](../port/fast3d/gfx_citro3d.cpp#L2011) copies
  clip-space positions **verbatim** (`t[0..3]=src[0..3]`); [line 2021](../port/fast3d/gfx_citro3d.cpp#L2021)
  scales only the **UVs** (`su/sv`), never positions; the per-eye transform is uniform across all draws. The
  backend is **not** enlarging the quad geometry. So the perceived "bigger" is the texture/alpha, not a quad scale.
- **Per-vertex color fold (`chainSel`)** — the PICA interpolates one vertex color so the backend folds the
  other combiner input to a constant ([gfx_citro3d.cpp:1914-1965](../port/fast3d/gfx_citro3d.cpp#L1914)). It
  *cannot* be collapsing the particle alpha, because the particle alpha is a **per-draw PRIM constant**, not
  per-vertex: [particle.c:2943](../src/PR/tengine/particle.c#L2943) sets it via `gDPSetPrimColor` (the fade
  `alpha × m_AlphaScaler`). Combiner alpha = `TEXEL0.a × PRIM.a`.
- **Mipmaps** — user disabled the mip chain; **did not help**. Particle textures are almost certainly not
  `gen_mipmaps`/`tex_lod`-flagged ([gfx_citro3d.cpp:1444](../port/fast3d/gfx_citro3d.cpp#L1444)), so there are
  no mips to mis-select.

## Tried — did NOT fix

1. **NPOT padding theory / 2-cycle-collapse** — both refuted earlier (wrong class of bug).
2. **Additive blend scoped to `opt_noise` (the N64 dither bit)** — *structurally* could only reach the
   **sparkle** (the only mode with `G_AC_DITHER`, [particle.c:2808](../src/PR/tengine/particle.c#L2808)); the
   **flame** uses `RM_CLD_SURF` / `RM_ZB_CLD_SURF` ([particle.c:2824/2838](../src/PR/tengine/particle.c#L2824),
   no dither), so it never entered the branch → "no change."
3. **Alpha-test threshold raise (`atest`, the shipped `GREATER 0` made tunable), incl. a live 3D-slider sweep**
   — ★ **DEFINITIVELY RULED OUT on-device.** Fixed `atest 16` did not kill the boxes; sweeping the slider, the
   opacity only disappears at the **very top (≈128)**, and at that ref the **fire/particles are over-thinned and
   look wrong**. So there is **no threshold that removes the artifact without ruining the sprite** → this is
   **NOT an alpha-coverage problem** (discarding low alpha can't fix it). The experiment lives on branch
   `3ds-particle-atest` (commit then reset away) for reference. Scope was `useAlpha && !modulate`.
4. **Additive blend scoped to `useAlpha && !modulate && !is2d && !depthMask`** (intended to reach the flame
   this time) — **NO visible change.**

> **Bottom line after #3 + #4:** neither the blend nor the alpha-coverage is the lever. It is a **per-batch
> render state that flips at certain angles** (3DS-only). Resume at the STRONGEST LEAD below — diff the
> billboard's draw state good-angle vs bad-angle and force the normal value.

## ★ STRONGEST REMAINING LEAD (start here next time)

Attempt #4 (additive, `!is2d && !depthMask`) made **no change**, but attempt #3 (alpha-test, `useAlpha &&
!modulate`) **did** reach the particles. The only difference is `!is2d && !depthMask`. Therefore **these
particle draws are either classified `is2d` (vertex-0 clip-w ≈ 1.0) or they write depth (`depthMask`)** — which
is why additive skipped them.

**First action next time:** add a one-shot debug log in `applyCmdState` (or `draw_triangles`) that, for
alpha-blended non-modulate draws, prints `is2d`, `depthMask`, `depthTest`, `tex0` dims, and `PRIM.a`. That
tells us definitively:
- If **`is2d` is true** for these 3D billboards → an `is2d` **misclassification** (w≈1 heuristic at
  [gfx_citro3d.cpp:2192](../port/fast3d/gfx_citro3d.cpp#L2192)) is mis-routing them. Even though the *transform*
  swap is a no-op with 3D off, the `is2d` flag may gate **other** per-batch state — worth auditing every
  `cmd->is2d` consumer.
- If **`depthMask` is true** → they write depth; the additive/whatever fix must include depth-writing
  translucent, and depth-write on translucent particles can itself cause batch-order opacity artifacts.

## Other candidate causes still worth testing (3DS-only texture path)

Because additive (the blend) didn't fix it and the geometry isn't scaled, the opacity is most likely the
**texture's own alpha (`TEXEL0.a`) reading opaque** for these batches — a 3DS-only texture-path effect
(PC handles NPOT natively and has no texture pool, consistent with "PC is fine"):

- **POT-padding wrap-fill bleed.** NPOT particle textures are padded to POT by **wrapping** source texels into
  the pad ([gfx_citro3d.cpp:1452](../port/fast3d/gfx_citro3d.cpp#L1452), `(y%height)*width+(x%width)`) —
  *regardless* of CLAMP vs REPEAT. For a CLAMP sprite with transparent edges, linear filtering near the real
  region boundary (`su = width/pw`) can sample the **wrapped opaque core texels**, bleeding opacity into the
  transparent border → fatter + more opaque. **Fix to try:** for non-tiling (CLAMP) textures, pad by **clamping
  the edge texel** instead of wrapping, and/or `CLAMP_TO_EDGE` + inset the UV so the filter never samples the pad.
- **Valid-but-stale texture slot.** The texture pool can evict+reuse a slot mid-frame; replay re-binds with only
  a `sTexValid` check ([gfx_citro3d.cpp:2588](../port/fast3d/gfx_citro3d.cpp#L2588)), no content/dimension
  re-validation. A churned slot → a *different* texture (different alpha/size) for the whole batch → opaque +
  wrong-res, intermittent, per-batch, 3DS-only. (The reverted `b7f014e` content-validate was disproven for the
  *yellow-square* bug, not for this one — re-add it flag-gated to test.)

## Why additive is NOT the fix

`TEXEL0.a` reading opaque is upstream of the blend. SRC_ALPHA renders it opaque-dark; additive renders it
opaque-bright. Both look "solid." The N64 render modes for these particles are SRC_ALPHA-over (`RM_CLD_SURF` /
`RM_ROB_ZB_PCL_SURF_BLEND` + `G_AC_DITHER`), **not** additive — so additive is also not faithful. The real fix
is to make `TEXEL0.a` (and/or the depth/is2d routing) correct, not to change the blend.

## Key code locations

- Particle render-mode + PRIM alpha: [particle.c:2790-2943](../src/PR/tengine/particle.c#L2790)
- Citro3D blend / alpha-test / depth: [gfx_citro3d.cpp:2332-2380](../port/fast3d/gfx_citro3d.cpp#L2332)
- Citro3D vertex repack (positions verbatim, UV `su/sv`): [gfx_citro3d.cpp:2006-2035](../port/fast3d/gfx_citro3d.cpp#L2006)
- POT padding / `su/sv`: [gfx_citro3d.cpp:1407-1430](../port/fast3d/gfx_citro3d.cpp#L1407)
- `is2d` classify + consume: [gfx_citro3d.cpp:2192](../port/fast3d/gfx_citro3d.cpp#L2192), [:2579](../port/fast3d/gfx_citro3d.cpp#L2579)
- Texture slot validity at replay: [gfx_citro3d.cpp:2588](../port/fast3d/gfx_citro3d.cpp#L2588)

## The committed, SHIPPED fix that IS in master (for reference, do not confuse with the above)

The **alpha == 0 "yellow square"** PICA blend bug *is* fixed in master (commit `4510566`): a `GREATER 0`
alpha-test discards zero-coverage fragments for alpha-blended non-modulate draws
([gfx_citro3d.cpp:2373](../port/fast3d/gfx_citro3d.cpp#L2373)). That is a different, confirmed-fixed bug. The
*this-doc* bug is **low/mid-alpha** going opaque, which is still open.

## ★ 2026-06-23 session — more dead ends; the BLEND is the user's prime suspect; SCOPE is the trap

User reconfirmed (emphatically): the particles **"don't get their alpha at some angles"**, it is **HW-only**
(not Mandarine, not PC), and the fix is **at the BLEND** ([gfx_citro3d.cpp:2346](../port/fast3d/gfx_citro3d.cpp#L2346)),
not the alpha test. A multi-agent trace **confirmed the command stream is correct** — `PRIM.a` (the fade) is
delivered per-vertex into `buf_vbo[off_input1 + k*isz + 3]` and read into the TEV per-stage CONSTANT alpha
(`stageConst`), `use_alpha` decodes TRUE, textures are POT, combiner alpha = `TEXEL0.a*PRIM.a`. So it is a
**PICA silicon execution** difference, same fingerprint as the SOLVED Bug C.

**Tried this session, ALL REJECTED:**
- **Remove the Bug-C alpha test** (pure alpha-over) → yellow squares came back, translucent still opaque.
- **`DST_COLOR, ZERO` multiply** (probe) → not alpha-weighted.
- **Additive `SRC_ALPHA, ONE`** on `useAlpha && !modulate` (and + the zero-coverage discard) → **(a) WRONG
  SCOPE: turned the PLANTS/foliage ICY-BLUE** — `useAlpha && !modulate` also matches alpha-blended foliage / HUD
  / sky / menus, not just particles — **and (b) did NOT fix the alpha** (particles still opaque). So "additive
  dst=ONE can't hide the background" was the wrong model: the per-fragment alpha simply isn't being applied on
  PICA for these draws.

**What this rules in for next time:**
1. **SCOPE to ONLY the particle billboards.** A blanket `useAlpha && !modulate` blend change wrecks foliage
   (icy-blue plants), HUD, sky. Identify the particle draws specifically — by combiner `shader_id0`
   (DECALRGBA_PRIMALPHA / PSEUDOCOLOR_PRIMALPHA, particle.c:2769/2774) or render mode (CLD_SURF / PCL_SURF).
2. **The blend is not the whole story** — additive didn't restore the alpha. Next suspects: the **PICA TEV
   per-stage CONSTANT alpha** (`stageConst`, the fade carrier — verify on HW it's actually applied per stage,
   `e.color = cmd->stageConst[i]` at applyCmdState), or a **valid-but-stale texture-pool slot** churning a
   different texture into the batch.
3. **Method:** a `turok.cfg` flag-gated **on-device A/B scoped to the particle draws** (the proven Bug-C
   approach). The physical 3D-slider sweep was tried and the user found it useless here — use a cfg flag.
