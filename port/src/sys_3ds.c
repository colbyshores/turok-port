/*
 * sys_3ds.c — Turok 3DS system layer: stack size, the on-device diagnostic log, framebuffer capture.
 *
 * The whole N64 boot (boot() -> mainproc()) runs INLINE on the single 3dsx main thread (the libultra
 * shim's osStartThread is a cooperative no-op), and the deep cart/asset loader chain overflows libctru's
 * default 32 KB stack -> heap free-list corruption / data-abort. Override __stacksize__ to 2 MB (both
 * sibling 3DS ports do this).
 *
 * plat3dsBootLog is THE on-hardware trace channel (a .3dsx has no visible stdout): append+flush to
 * sdmc:/3ds/turok/boot.log every line (so a data-abort can't lose the trail) + svcOutputDebugString
 * (Luma / Mandarine log). It's the equivalent of the PC build's stderr trace; gfx_citro3d.cpp calls it.
 */
#ifdef PLATFORM_3DS
#include <3ds.h>
#include <stdio.h>
#include <string.h>

/* 2 MB; overrides libctru's WEAK 32 KB default. */
unsigned int __stacksize__ = 2 * 1024 * 1024;

void plat3dsBootLog(const char *msg)
{
    FILE *f;
    svcOutputDebugString(msg, (int)strlen(msg));   /* Luma3DS / Mandarine debug console */
    f = fopen("sdmc:/3ds/turok/boot.log", "a");
    if (f) { fputs(msg, f); fputc('\n', f); fclose(f); }   /* append+flush+close every line */
}

/* "Did anything rasterize?" signal — count non-black pixels on the top-left framebuffer (mirror of
 * the PC TUROK_CAPTURE_FRAME). Cheap, no file written; logs the count. */
void plat3dsCaptureTopFB(void)
{
    u16 w = 0, h = 0;
    u8 *fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &w, &h);
    long nonblack = 0, i, n = (long)w * h * 3;
    char b[80];
    if (!fb) return;
    for (i = 0; i + 2 < n; i += 3)
        if (fb[i] | fb[i + 1] | fb[i + 2]) nonblack++;
    snprintf(b, sizeof(b), "[capture] top-fb %ux%u nonblack=%ld", w, h, nonblack);
    plat3dsBootLog(b);
}

/* Pass the real newlib stderr (a macro here) to the global-symbol shim in stderr_3ds.c — the game's
 * port traces reference a global `stderr` symbol that newlib (macro) doesn't provide. */
void *plat3dsRealStderr(void) { return (void *)stderr; }
#endif /* PLATFORM_3DS */
