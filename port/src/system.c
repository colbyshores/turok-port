/* system.c — minimal host implementations of the shared `sys*` helpers (the `system.h`
 * contract, same role as perfect_dark / banjo-kazooie `port/src/system.c`). turok only
 * needs the subset the borrowed Fast3D layer (gfx_opengl.cpp etc.) calls — log + bail +
 * args + sleep/relax; the rest of the system.h contract (sysInit, sysGetMicroseconds, the
 * mem helpers, crash*) is declared there for the shared library but not yet needed here. */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "system.h"

void sysLogPrintf(s32 level, const char *fmt, ...)
{
    va_list ap;
    (void)level;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

void sysFatalError(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "[turok] FATAL: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    abort();
}

s32 sysArgCheck(const char *arg) { (void)arg; return 0; }
const char *sysArgGetString(const char *arg) { (void)arg; return 0; }
s32 sysArgGetInt(const char *arg, s32 defval) { (void)arg; return defval; }
void sysSleep(long long hns) { (void)hns; }
void sysCpuRelax(void) {
#if defined(__i386__) || defined(__x86_64__)
    __asm__ __volatile__("pause" ::: "memory");   /* x86 spin-wait hint */
#else
    __asm__ __volatile__("" ::: "memory");        /* ARM/other: a compiler barrier is enough */
#endif
}
