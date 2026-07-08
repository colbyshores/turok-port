# 3DS Fast3D→Citro3D Port: Stereo + Bottom-Backlight Changes — Porting Spec

**Source:** Turok 3DS port, branch `stereo-mipmap-battery-review`, 2026-07-07.
**Target:** any Fast3D→Citro3D 3DS port that descends from the Perfect Dark backend
(`port/fast3d/gfx_citro3d.cpp` + `gfx_3ds.c`). **Written specifically so it can be forwarded to the
Perfect Dark 3DS port**, whose backend Turok's was vendored from.

This documents four community-reported issues, which ones were real, and the exact code for the fixes.
Every change is `PLATFORM_3DS`-gated and defaults such that the non-3DS builds are byte-identical.

> ## ⚠️ HEADLINE FOR PERFECT DARK
> Turok's stereo backend is a near-verbatim copy of PD's, with the **same eye-sign convention**
> (`buildTransform`: left eye `eyeSign=-1`, right `+1`; shear on matrix row `r[1]` = `eyeSign * shearZ`),
> the **same 270° panel rotation**, and the **same `sTopLeft→GFX_LEFT / sTopRight→GFX_RIGHT` wiring**.
> On real Turok hardware this convention rendered **pseudoscopic** (far objects look near / near look far).
> **⇒ Perfect Dark almost certainly has the same latent inversion and nobody caught it** (the stereo was
> validated as "3D works," not A/B'd for orthoscopy). **Action: add the default-off `stereo_swap` toggle
> below to PD, A/B it on real hardware with the 3D slider up, and if PD is also pseudoscopic, flip its
> default too.** This is the single most important thing to carry over.

---

## The four reports and the verdicts

| # | Report | Verdict | Action |
|---|--------|---------|--------|
| 1 | "Left/right of the stereo effect are inverted (distant looks near)" | **REAL (pseudoscopic)** — HW-confirmed | eye-sign toggle + flip default |
| 2 | "Stereo effect is extremely constrained / negligible" | Partly a symptom of #1; also tunable | strength multiplier (default MAX per user) |
| 3 | "Bottom screen is unused — shut its backlight off for battery" | **REAL** | power off bottom backlight (+ HOME/sleep handling) |
| 4 | "Mipmapping bug — textures vertically distorted at some distances" | **HARDWARE LIMITATION**, not a bug | no code change (see §5) |

---

## 1. Change A — Bottom-screen backlight off (battery). *Ports to PD directly.*

The bottom screen is unused (the HUD is all top-screen; the bottom `C3D_RenderTarget` is only cleared to
black each frame). Powering off its LCD backlight is a real battery win. **PD's `gfx_3ds.c` is structurally
identical and has no gspLcd/aptHook today**, so this ports almost verbatim.

### libctru API (verified in `/opt/devkitpro/libctru/include/3ds/services/gsplcd.h`, pulled in by `<3ds.h>`)
```c
Result gspLcdInit(void);
void   gspLcdExit(void);
Result GSPLCD_PowerOffBacklight(u32 screen);   // screen = GSPLCD_SCREEN_BOTTOM (BIT(GSP_SCREEN_BOTTOM))
Result GSPLCD_PowerOnBacklight (u32 screen);
```

### The two non-obvious gotchas
1. **The OS re-lights BOTH panels on sleep/wake.** Power-off once at init and it flickers back on after every
   sleep. ⇒ you need an `aptHook` on `APTHOOK_ONWAKEUP`/`ONRESTORE` to re-assert your off state.
2. **The HOME menu needs the bottom screen back.** If you leave it off on `APTHOOK_ONSUSPEND` (HOME pressed),
   the HOME menu's bottom screen stays dark. ⇒ power it back **ON** on `ONSUSPEND`/`ONSLEEP`, then re-apply
   your preference on `ONRESTORE`/`ONWAKEUP`. (`gfx_3ds.c` had no prior APT hook — this is the first one.)

### Full code (`gfx_3ds.c`, C file)
```c
// A config gate: 0 (default) = off/save battery, 1 = keep the bottom screen lit.
extern int g_cfg_bottom_backlight;   // PD: use a Video.BottomBacklight config bool instead (see §4)

static aptHookCookie s_lcd_hook;

static void lcd_set_bottom(int on) {
    if (R_SUCCEEDED(gspLcdInit())) {                 // transient gsp::Lcd session (the standard idiom)
        if (on) GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM);
        else    GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTTOM);
        gspLcdExit();
    }
}
static void lcd_backlight_apply(void) { lcd_set_bottom(g_cfg_bottom_backlight); }

// call this when the player toggles the option in-menu (both directions)
void turok3dsRefreshBottomBacklight(void) { lcd_backlight_apply(); }

static void lcd_apt_hook(APT_HookType hook, void *param) {
    (void)param;
    if (hook == APTHOOK_ONSUSPEND || hook == APTHOOK_ONSLEEP)          // HOME menu / sleep → give the screen back
        lcd_set_bottom(1);
    else if (hook == APTHOOK_ONRESTORE || hook == APTHOOK_ONWAKEUP)    // back in-game → our preference (off by default)
        lcd_backlight_apply();
}
```
At the **end of `gfx_3ds_init()`** (after `gfxInit`/`C3D_Init`; backlight power is independent of C3D):
```c
    lcd_backlight_apply();
    aptHook(&s_lcd_hook, lcd_apt_hook, NULL);
```
At the **top of `gfx_3ds_close()`**:
```c
    aptUnhook(&s_lcd_hook);   // stop re-asserting; HOME/Luma re-lights both panels on process exit
```
Failure modes are benign: if `gspLcdInit` fails, `R_SUCCEEDED` skips it (no crash); emulators no-op the call.
Optional extra GPU nicety (not needed for the battery win): when the backlight is off you can also skip the
per-frame clear+`C3D_FrameDrawOn` of the bottom target — gate it on the same flag. The Turok port left the
clear in (a 240×320 clear is negligible vs the LCD backlight).

---

## 2. Change B — Stereo eye-sign / pseudoscopic fix. *PD: A/B on HW, likely needs the same flip.*

### What it is
The per-eye horizontal parallax is a **clip-space shear** applied in `buildTransform`. In Turok/PD it rides
matrix row **`r[1]`** because the 3DS top screen renders **rotated 270°** (portrait 240×400 framebuffer), so
`r[1]` = the physical-horizontal axis after rotation.

Turok's `buildTransform` (widescreen-adjusted; PD's is the simpler `-1.f`/no `oxf` form):
```c
// eyeSign: 0 = mono, -1 = left, +1 = right.  level = raw 3D-slider value [0,1].
m->r[1].z = eyeSign * sEyeSwap * sShearZ * level;
m->r[1].w = eyeSign * sEyeSwap * sShearW * level;
```
PD's current line (identical convention, no swap term yet):
```c
m->r[1].z = eyeSign * g_citro3dStereoZ * level;
m->r[1].w = eyeSign * g_citro3dStereoW * level;
```

### Why it's pseudoscopic and why it's HW-only
The eye→panel **wiring is correct** (`sTopLeft→GFX_LEFT`, replay renders `eyeSign=-1` into the left target) —
this is **not** a swapped-image bug. But the 270° rotation applies a **common-mode horizontal sign** to *both*
eye images; if that sign is negative for this rotation, it flips crossed↔uncrossed disparity = pseudoscopic,
**without** swapping which physical eye sees what. This is undeterminable off-hardware: **Mandarine/Citra read
the 3D slider as 0 (mono)**, so stereo can only be judged on a real 3DS with the slider up. The
"keep SHEAR_Z positive" reasoning in the original comments is done in *render* space and misses the rotation
flip.

### The fix (a sign multiplier, defaulted to the corrected value once HW-confirmed)
Add a `sEyeSwap` (PD: `g_citro3dEyeSwap`) that negates `eyeSign`:
```c
// HW-CONFIRMED: the original sign was pseudoscopic, so the CORRECTED sign (-1) is the DEFAULT.
static float sEyeSwap = -1.0f;                 // stereo_swap 0 (default) = corrected; 1 = old inverted
...
sEyeSwap = g_cfg_stereo_swap ? 1.0f : -1.0f;   // set in the refresh function below
```
Then multiply it into **both** shear terms (`... = eyeSign * sEyeSwap * shearZ * level;`). Mono (`eyeSign=0`)
is inert, so 2D/HUD draws are unaffected.

### ★ The correct *sequence* (this is the reusable methodology)
1. Ship it as a **default-OFF toggle** (`stereo_swap 0` = the *existing* sign) so you don't regress anything.
2. **A/B on real hardware** with the 3D slider up: does `stereo_swap 1` make depth pop correctly?
3. **Only then flip the default** so the corrected sign is `stereo_swap 0`, and keep the toggle as an escape
   hatch. (Turok: shipped opt-in → user confirmed inverted → flipped `sEyeSwap` default to `-1.0f`.)

Do **not** blindly flip PD's default without the HW A/B — but expect that you will, because PD shares the
convention.

---

## 3. Change C — Stereo strength multiplier. *PD: scale both shear terms.*

"Negligible" is partly a *symptom* of the inversion (a pseudoscopic image reads as flat because the brain
won't fuse it), so fix #2 first. Separately, expose a **strength multiplier that scales BOTH shear terms
together** — this increases the depth *pop* while keeping the convergence plane fixed (`z/w = -shearW/shearZ`
stays constant), which is what a user actually wants and what raising `shearZ` alone fails to do:
```c
if (g_cfg_stereo_strength > 0.f) { sShearZ *= g_cfg_stereo_strength; sShearW *= g_cfg_stereo_strength; }
```
Do **not** change the compiled base (`0.10 / 0.006` — HW-validated; too much = ghosting/eye-strain). Expose the
multiplier and let the user pick. **Turok defaults it to 3.0 ("max") per the port owner's preference**; a
conservative port would default 1.0.

---

## 4. Change D — Config plumbing + live-apply + menu. *PD uses INI config + its own menu.*

### Config values (Turok: plain `key value` `turok.cfg`; **PD: `[Video]` INI section via `config.c`/`video.c`**)
Turok (`port/src/config.c`) — global + parse + save for each key:
```c
float g_cfg_stereo_strength = 3.0f;   // MAX; scales both shear terms
int   g_cfg_stereo_swap     = 0;      // 0 = corrected depth (default), 1 = old inverted
int   g_cfg_bottom_backlight= 0;      // 0 = backlight off (default), 1 = keep lit
// parse:  else if (!strcmp(key,"stereo_strength")) g_cfg_stereo_strength = (float)val;   (etc.)
// save:   fprintf(f, "stereo_strength %.4f\n", g_cfg_stereo_strength);                   (etc.)
```
**PD equivalent:** register `Video.StereoSwap` / `Video.StereoStrength` / `Video.BottomBacklight` next to the
existing `Video.StereoShearZ/W` (grep `g_citro3dStereoZ` and `configRegister*` in PD's `video.c`).

### One refresh function so a menu toggle applies LIVE (no restart)
Refactor the init-time config read into a function that **recomputes from the compiled base every call** (so a
repeated strength toggle doesn't compound the multiplier). `extern "C"` because it's called from C menu code:
```c
extern "C" void turok3dsRefreshStereo(void) {
    extern float g_cfg_stereo_z, g_cfg_stereo_w, g_cfg_stereo_strength; extern int g_cfg_stereo_swap;
    sShearZ = (g_cfg_stereo_z >= 0.f) ? g_cfg_stereo_z : STEREO_SHEAR_Z;   // base or override
    sShearW = (g_cfg_stereo_w >= 0.f) ? g_cfg_stereo_w : STEREO_SHEAR_W;
    if (g_cfg_stereo_strength > 0.f) { sShearZ *= g_cfg_stereo_strength; sShearW *= g_cfg_stereo_strength; }
    sEyeSwap = g_cfg_stereo_swap ? 1.0f : -1.0f;
}
```
Call it once at `gfx_citro3d_init` (replacing the inline reads) and from the menu on every toggle. Same idea
for `turok3dsRefreshBottomBacklight()` (§1).

### Menu integration (Turok specifics — PD has its own menu system)
Turok added a `PLATFORM_3DS`-only **"display" submenu** off the options screen (mirroring the existing
CONTROLS-submenu pattern: a `s_DisplayActive` file-scope flag branched at the top of the options `Update`/`Draw`;
never a struct field — growing the menu struct corrupts a stale build). Rows: **invert 3d** (swap toggle),
**3d depth low/med/high/max** (a strength *cycler* — named levels avoid the font's missing `.` glyph),
**bottom light on/off**, **back**. Each row activates (A) and applies live via the refresh functions; back
saves the config. For PD, wire the three values into its own options UI however PD does it — the portable part
is the **config + live-refresh seam**, not Turok's menu widget.

**★ Menu-row DEFAULT states (Turok, shipped):** the **"invert 3d" row defaults to OFF** (`stereo_swap 0`),
which — after the §2 default flip — is the *corrected* depth, so a fresh install gets correct 3D with the row
left off (turning it ON gives the old inverted look). "3d depth" defaults to **max**; "bottom light" defaults
to **off**. Keep the label semantics honest: "invert 3d OFF" must be the *good* state, so the corrected sign
has to be the `stereo_swap 0` side (that's why §2 flips `sEyeSwap`'s default, not the config default value).

---

## 5. Change "D" that wasn't — Mipmap "vertical distortion" = the PICA aniso limit (no fix)

"Textures vertically distorted at some distances" on a Fast3D→PICA port is **almost always the PICA200's lack
of anisotropic filtering** (a hardware limitation): tiled surfaces viewed at shallow angles smear in one axis.
It is already mitigated by the mip + facade-bake system (even-integer UV fold + clip-space midpoint split +
box-filtered mip chain + content-keyed bake). **Do not blind-edit the bake/mip constants** — they're HW-tuned
and a wrong change regresses flare-safety / the FCRAM budget / near-panel sharpness. If a user insists there's
a specific defect, get a concrete on-HW repro (level, surface, distance) and A/B the existing read-at-init
knobs first (Turok/PD: the SD `.txt` toggles `texlodmips`, `mips`, `mipnearest`, `autobake`). Only a repro that
isolates a specific asset the bake heuristic misses justifies a *scoped* per-texture LOD-bias.

---

## 6. Perfect Dark porting checklist

- [ ] **Backlight (§1):** add `lcd_set_bottom` / `lcd_backlight_apply` / `lcd_apt_hook` + the `aptHook` in
      `gfx_3ds_init` and `aptUnhook` in `gfx_3ds_close`. Gate on a `Video.BottomBacklight` bool (default off).
      *This is the cleanest, highest-confidence port.*
- [ ] **Stereo swap (§2):** add `g_citro3dEyeSwap` (default `+1.0f` = current sign), multiply into both
      `r[1].z/.w` shear terms in `buildTransform`. Expose `Video.StereoSwap`. **A/B on real HW with the slider
      up.** If PD is pseudoscopic (expected), flip the default to `-1.0f`.
- [ ] **Strength (§3):** `Video.StereoStrength` multiplier scaling both `g_citro3dStereoZ/W`. Default 1.0 (or
      the owner's preference).
- [ ] **Live refresh (§4):** one function that recomputes shear from the base each call; call at init + on any
      menu change.
- [ ] **Verify** the non-3DS build is byte-identical (all gated), and that Mandarine still boots (it renders
      the UI; it just can't judge stereo).

## 7. Gotchas / lessons (carry these over)

1. **Stereo is real-HW-only.** Mandarine/Citra read the slider as 0 = mono. Never conclude anything about
   3D depth from an emulator. Ship a default-off toggle, A/B on device, then flip the default.
2. **Eye-target wiring and shear-sign are independent.** A pseudoscopic report is a *sign* bug, not a swapped
   image — and when the shear rides a rotated axis, the panel rotation itself can flip physical disparity.
3. **The OS owns the backlights across APT transitions.** Power-off is not sticky: re-assert on wake, and hand
   the bottom screen back to the HOME menu on suspend, or the HOME menu goes dark.
4. **Scale both shear terms** for a "more 3D" knob (keeps the convergence plane); scaling only Z moves the
   screen plane instead of adding pop.
5. **Recompute stereo state from the compiled base on each live refresh**, or a repeated strength toggle
   compounds the multiplier.
6. **"Vertical distortion at distance" = missing anisotropic filtering (hardware), not a mipmap bug.** Don't
   touch the tuned bake/mip code without an asset-isolating repro.
7. **Flipping a default that was previously an opt-in toggle leaves stale SAVED values on devices.** After you
   ship the swap as opt-in (`stereo_swap 0` = old sign), any tester who toggled it ON has `stereo_swap 1` saved
   in their config. When you then flip the compiled default (make `sEyeSwap`'s `swap 0` side the corrected
   sign), that saved `1` now selects the *wrong* (old) sign — the config override wins over the new default. So
   after a default flip: keep the toggle's on/off *labels* aligned to good/bad (flip `sEyeSwap`, NOT the config
   default value, so `swap 0` stays the good state and the menu row stays "off"=correct), and expect testers to
   have a stale saved value — reset it (or tell them to set the row back to off once). A fresh install is
   unaffected (it takes the compiled default). *(Turok: the shipped default is `stereo_swap 0`, and both the
   compiled default and the device's own `turok.cfg` read `stereo_swap 0` = "invert 3d off" = corrected — so
   nothing to migrate there; the hazard only bites a device that saved `1` mid-testing.)*

*Turok commits (branch `stereo-mipmap-battery-review`): `9f316a9` (knobs) · `d30e760` (menu) · `bc0d304`
(default flip) · `779be40` (HOME backlight + MAX default).*
