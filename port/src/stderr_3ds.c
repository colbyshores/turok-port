/*
 * stderr_3ds.c — provide the `stderr` GLOBAL SYMBOL the game's port traces link against.
 *
 * The game's port-hook diagnostics use `extern void *stderr; fprintf(stderr, ...)` (they avoid <stdio.h>
 * because the engine headers #define abs(n)/etc. that collide with it). On glibc `stderr` is a real global
 * symbol, so that links. On 3DS newlib `stderr` is a MACRO (_impure_ptr->_stderr) — there is no symbol —
 * so define one here and point it at the real stream early in main() (turok_main.c calls turok3dsSetStderr).
 *
 * This TU MUST NOT include <stdio.h> (that would make `stderr` a macro here and shadow the global).
 */
#ifdef PLATFORM_3DS
void *stderr = 0;                              /* the symbol the game's `extern void *stderr` resolves to */
void turok3dsSetStderr(void *s) { stderr = s; }
#endif /* PLATFORM_3DS */
