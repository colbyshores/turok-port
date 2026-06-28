# 3DS distance-fog — RESOLVED 2026-06-27 (merged to master)

Status: **FIXED, USER-CONFIRMED on HW, merged.** The 3DS now does ALL distance fog in the TEV (per-vertex where
the shade alpha is free, per-draw constant where it isn't); the banding f24 `1/w` hardware FogLut is no longer
used for any game geometry. It is **unconditional** — there is no `fogmode`/`fogscale`/`fogbias`/`fogzflip`
knob anymore (all four were diagnostic scaffolding and were removed). The "Next step" investigation plan below
is kept as the historical record of how it was solved; **the per-draw lever was implemented (`7e93535`) and it
was the fix** — the "solid blue rectangles" were shade-alpha translucent surfaces the f24 FogLut over-fogged.

See CLAUDE.md §10 (the ★★ DISTANCE-FOG entry) and [playbook §20](N64_PORTING_PLAYBOOK.md) for the distilled
result + the generalizable lesson. The sections below are the original investigation log.

---

## The original problem

On the 3DS, distance fog is rendered by the **PICA200 fixed-function FogLut** — per-fragment, indexed by the
hardware fog source derived from **f24 `1/w`** (hyperbolic depth). At distance the f24 `1/w` precision is
coarse, so the fog **bands / fails to render smoothly on geometry at certain angles/depths**. User: *"the fog
not rendering on geometry at certain angles… this has always been an issue, I just waited to the end to fully
address it."* Validated the f24 root on PC: crushing the PC fog factor to 24-bit float reproduced the banding.

The fix direction: compute the fog factor in **f32 per-vertex** (like the PC/GL backend) and hand the PICA a
`[0,1]` value it interpolates accurately, sidestepping the f24 `1/w` index entirely. That's `fogmode 1`.

---

## The hardware wall (why per-vertex fog can't simply cover everything)

Per-vertex fog needs a `[0,1]` factor delivered to the **TEV per fragment**. On this pipeline the TEV's only
interpolated per-vertex inputs are:

- **`GPU_PRIMARY_COLOR`** — the ONE per-vertex colour (the shade). 1 rgba.
- **3 texture samples** — and **all 3 units are already taken**:
  - unit 0 = texture0, unit 1 = texture1,
  - unit 2 = the **load-bearing white texture** that sources the combiner's `0`/`1` literals
    (`SHADER_0`/`SHADER_1` → `GPU_TEXTURE2`, see `sWhiteTex` in `gfx_citro3d.cpp`).
- Fragment-lighting colours (`GPU_FRAGMENT_PRIMARY/SECONDARY_COLOR`) **cannot carry an arbitrary varying**
  (they're computed from normals/lights), so they're not usable as a fog channel.

**Freeing unit 2 is infeasible.** There is no other constant-`1.0` TEV source on the PICA, and moving the
`0`/`1` literals to the per-stage `GPU_CONSTANT` collides with every stage that pairs a literal with a real
constant (e.g. `1 × PRIM` = `MODULATE(white, PRIM_const)`). The white-literal-on-unit-2 design exists
specifically to keep `GPU_CONSTANT` free for the real per-stage constant. So unit 2 stays.

**Conclusion:** the fog factor has nowhere to ride **except the shade alpha (`PRIMARY.a`)**. There is no
free per-vertex channel for fog on this PICA pipeline.

---

## What was tried on the branch (commits, newest last)

The fog stage is appended as the final TEV stage: `out.rgb = INTERPOLATE(CONST[fogColor], PREVIOUS, factor)`,
`alpha = PREVIOUS` (so the combiner's real alpha is preserved for blend/alpha-test). The factor rides
`PRIMARY.a`. The whole question is **which draws may ride it**:

1. **`32d707d`** — per-vertex fog for ALL fog draws (rode `PRIMARY.a` unconditionally). Distant fog looked
   great (*"looks good!"*), but **alpha-tested plants vanished up close** — their alpha is `texel.a*shade.a`,
   so fog→0 near the camera zeroed it → §15 zero-coverage discard killed them.
2. **`d1d0199`** — per-DRAW gate: exclude alpha-tested (`opt_texture_edge`/`opt_alpha_threshold`) and
   `useAlpha && !modulate`. Plants came back, but **solid blue rectangles** appeared where translucent
   surfaces (modulate-blended) were treated as opaque and fogged to ~100% fog colour at distance.
3. **`dde1472`** — exclude **all** `useAlpha` (both blend kinds) + alpha-test → per-vertex fog on fully-opaque
   geometry only. **Did NOT fix the blue rectangles.** (Key data point: if excluding translucent draws — i.e.
   putting them back on the FogLut — leaves the rectangles, they are not simply "translucent fogged solid".)
4. **`956119e`** — precise gate: exclude only when **(alpha is used: blend OR alpha-test) AND (the effective
   alpha pipe references `SHADER_INPUT_1` = the shade)**. The fog stage preserves the combiner alpha, so
   translucent/alpha-tested surfaces whose alpha comes from the **texture** (`texel.a`) ride fog fine; only
   genuinely shade-alpha-dependent draws (foliage `texel.a*shade.a`, shade-alpha blends) fall back to FogLut.
   This covers opaque **and** texel-alpha translucent geometry. **Still the same blue rectangles.**

`alphaUsesShade` detection (`gfx_citro3d.cpp`, the repack): for each cycle, the effective alpha pipe is
`c[cyc][1]` when `opt_alpha && !color_alpha_same[cyc]`, else the shared `c[cyc][0]`; the draw uses shade alpha
iff `SHADER_INPUT_1` appears there. The colour pipe's RGB use of the shade is irrelevant (fog only touches
alpha), so opaque lit terrain (`colour = texel*shade`) is correctly NOT excluded.

---

## Where it stands

Neither (3) opaque-only nor (4) the precise shade-alpha gate removed the **"solid blue rectangles in mid-air"**
(seen in the foggy temple level — two vertical blue panels in front of stone walls; photo on file). That
rules out the obvious per-vertex-fog explanations:

- Not "translucent surface fogged solid" — excluding translucent draws (→ FogLut) left them (data point #3).
- Not "alpha = texel.a translucent surface" — the precise gate (#4) would have given those correct fog.

So the blue rectangles are **either** genuinely shade-alpha-dependent translucent surfaces (which per-vertex
fog fundamentally cannot reach on this hardware — they're stuck on the FogLut), **or it is not a per-vertex-fog
problem at all** — candidates: a blend/decode issue (the surface isn't blending and renders opaque regardless
of fogmode), the FogLut genuinely over-fogging them, or they render this way in `fogmode 0` too (untested A/B).

---

## Next step (when we circle back)

**STOP guessing the classification. Identify the actual draws first.** Wire a one-shot, `turok.cfg`-gated
on-device diagnostic (the proven Bug-C / flag-gated-A-B method) that, for the draws covering the blue
rectangles, dumps: `shader_id0`/`shader_id1` (the combiner), the alpha source (texel vs shade vs const), the
blend mode (`useAlpha`/`modulate`), `opt_texture_edge`/`opt_alpha_threshold`, and the per-vertex fog factor.
Concretely:

1. **First A/B `fogmode 0` vs `fogmode 1`** on that exact spot — does the rectangle exist in stock FogLut too?
   If YES, it's NOT the per-vertex fog work at all (look at blend/decode); if NO, it's specific to the fog
   stage and the diagnostic above pins which combiner.
2. With the combiner identified, decide: is it a missing alpha-blend (decode bug, fix in `gfx_pc`), a FogLut
   over-fog (calibration via `fogscale`/`fogbias`), or a true shade-alpha translucent surface (accept FogLut,
   or revisit whether a per-draw constant-factor fog stage — factor in the appended stage's own free
   `GPU_CONSTANT.a`, no shade-alpha, no texture unit — is acceptable for those specific surfaces).

The **per-draw constant-factor** idea is the one untried lever that needs no free channel: the appended fog
stage has its OWN `GPU_CONSTANT` (unused by the combiner), so a single per-draw fog factor (computed CPU-side
from the draw's representative depth) could fog shade-alpha translucent surfaces without touching `PRIMARY.a`.
It's constant across the draw (fine for small billboards, flat for large planes) but banding-free.

### IMPLEMENTED 2026-06-27 (`7e93535`) — per-draw constant fog

Built the per-draw lever (`cmd->perDrawFog` in `gfx_citro3d.cpp`): a fog draw whose alpha depends on the shade
(`!perVertexFog`) now also gets the appended fog TEV stage, but the INTERPOLATE factor comes from the fog
stage's own `GPU_CONSTANT.alpha` = the avg of the draw's verts' f32 fog factors (accumulated in the repack).
`PRIMARY.a` is untouched and the stage alpha = `PREVIOUS`, so blend/alpha-test still see the real combiner
alpha. Net: under `fogmode 1` EVERY `opt_fog` draw does TEV fog (per-vertex where the shade alpha is free,
per-draw constant where it isn't) — the f24 hardware FogLut is no longer used for any `opt_fog` geometry.

**If the blue rectangles persist even now**, they are NOT fog at all (the FogLut is fully out of the picture
for them) — pivot to the blend/decode angle: A/B `fogmode 0` (they'll look the same if it's not fog), then the
one-shot on-device diagnostic to dump the offending draws' combiner id / alpha source / blend mode. A surface
that renders opaque-blue with the entire fog path removed is a translucency (blend/decode) bug, not a fog bug.

## Diagnostic knobs currently on the branch (clean up on resolve/merge)

`turok.cfg`: `fogmode` (0 stock / 1 per-vertex), `fogscale`, `fogbias`, `fogzflip` — the live FogLut
calibration tunables (config.c `g_cfg_fog*`). All default to stock. Remove once the fog approach is settled.
