/* turok_sys.c — minimal implementations of the Banjo `sys*` helpers the borrowed
 * Fast3D layer (gfx_opengl.cpp etc.) calls. Just enough to log and bail on the host. */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void sysLogPrintf(int level, const char *fmt, ...)
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

int sysArgCheck(const char *arg) { (void)arg; return 0; }
const char *sysArgGetString(const char *arg) { (void)arg; return 0; }
int sysArgGetInt(const char *arg, int defval) { (void)arg; return defval; }
void sysSleep(long long hns) { (void)hns; }
void sysCpuRelax(void) { __asm__ __volatile__("pause" ::: "memory"); }
