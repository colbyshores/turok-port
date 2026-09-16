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

static s32 s_argc;
static const char **s_argv;

void sysInitArgs(long argc, const char **argv)
{
    s_argc = argc;
    s_argv = argv;
}

const char *sysArgGetString(const char *arg)
{
    s32 i;
    size_t n;
    if (!arg || !s_argv) return 0;
    n = __builtin_strlen(arg);
    for (i = 1; i < s_argc; i++) {
        if (!__builtin_strcmp(s_argv[i], arg) && i + 1 < s_argc && s_argv[i + 1][0] != '-')
            return s_argv[i + 1];
        if (!__builtin_strncmp(s_argv[i], arg, n) && s_argv[i][n] == '=')
            return s_argv[i] + n + 1;
    }
    return 0;
}

s32 sysArgCheck(const char *arg)
{
    s32 i;
    if (!arg || !s_argv) return 0;
    for (i = 1; i < s_argc; i++)
        if (!__builtin_strcmp(s_argv[i], arg)) return 1;
    return 0;
}
s32 sysArgGetInt(const char *arg, s32 defval)
{
    const char *value = sysArgGetString(arg);
    return value ? (s32)strtol(value, 0, 0) : defval;
}
void sysSleep(long long hns) { (void)hns; }
void sysCpuRelax(void) {
#if defined(__i386__) || defined(__x86_64__)
    __asm__ __volatile__("pause" ::: "memory");   /* x86 spin-wait hint */
#else
    __asm__ __volatile__("" ::: "memory");        /* ARM/other: a compiler barrier is enough */
#endif
}
