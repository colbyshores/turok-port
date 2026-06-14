#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>
static volatile void *g_ret, *g_dst, *g_src; static volatile size_t g_n; static volatile const char* g_fn;
void *__real_memcpy(void*,const void*,size_t);
void *__real_memmove(void*,const void*,size_t);
void  __real_bcopy(const void*,void*,size_t);
void *__wrap_memcpy(void *d,const void *s,size_t n){ g_fn="memcpy";g_ret=__builtin_return_address(0);g_dst=d;g_src=(void*)s;g_n=n; return __real_memcpy(d,s,n);}
void *__wrap_memmove(void *d,const void *s,size_t n){ g_fn="memmove";g_ret=__builtin_return_address(0);g_dst=d;g_src=(void*)s;g_n=n; return __real_memmove(d,s,n);}
void  __wrap_bcopy(const void *s,void *d,size_t n){ g_fn="bcopy";g_ret=__builtin_return_address(0);g_dst=d;g_src=(void*)s;g_n=n; __real_bcopy(s,d,n);}
static void segv(int sig, siginfo_t *si, void *uc){
    (void)sig;(void)uc;
    fprintf(stderr,"\n[SEGV] fault addr=%p\n", si->si_addr);
    fprintf(stderr,"[SEGV] last %s: caller=%p dst=%p src=%p n=%zu\n",
            g_fn?(char*)g_fn:"?", (void*)g_ret,(void*)g_dst,(void*)g_src,g_n);
    void *bt[6]; int k=backtrace(bt,6); backtrace_symbols_fd(bt,k,2);
    fflush(stderr); _exit(42);
}
__attribute__((constructor)) static void inst(void){
    struct sigaction sa; sa.sa_sigaction=segv; sa.sa_flags=SA_SIGINFO; sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV,&sa,0);
}
