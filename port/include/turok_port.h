/* turok_port.h — port-wide compatibility shim, force-included on the host build.
 *
 * This header is injected into every tengine translation unit via `-include` from
 * Makefile.port (PLATFORM_PORT builds only). It restores the few N64/IDO-era
 * assumptions the original source was written against, so the game C compiles on a
 * modern host toolchain WITHOUT editing the game files themselves. Keep it tiny and
 * codegen-neutral; anything bigger belongs in a proper port/ source file.
 *
 * See ../../CLAUDE.md for the porting strategy and milestone roadmap.
 */
#ifndef _TUROK_PORT_H
#define _TUROK_PORT_H

#ifdef PLATFORM_PORT

/* Symbol collision with libc: the game defines its own 5-arg qsort(base,num,wid,comp,SWAP),
 * which interposes (as an executable global) on libc's 4-arg qsort that OSMesa/Mesa call
 * internally — Mesa then invokes the missing `swap` arg as a function pointer (garbage/NULL)
 * and crashes. Rename the game's version so libc keeps its own qsort. (This force-include is
 * applied to game TUs only, so it renames the definition AND every game call site.) */
#define qsort turok_qsort

/* NULL-as-integer note:
 * The IDO N64 toolchain expanded NULL to integer 0, which tengine relied on in a few
 * flag-field initialisers (`NULL | SOME_FLAG` in the boss tables) and switch labels
 * (`case NULL:`). Modern <stddef.h> makes NULL `((void*)0)`, illegal there. A force-
 * include can't fix it (libultra's <stddef.h> re-#defines NULL *after* this header),
 * so those ~10 sites were edited in place to literal `0` (trex.c, campaign.c,
 * loadsave.c, pause.c) — see CLAUDE.md "Port edits to game source". This header is
 * kept as the home for genuine codegen-neutral host shims as the port grows.
 */

#endif /* PLATFORM_PORT */
#endif /* _TUROK_PORT_H */
