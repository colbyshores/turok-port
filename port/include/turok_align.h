/* turok_align.h — unaligned-access accessors for the ARM11 (ARMv6K / 3DS) port.
 *
 * WHY: N64 assets are big-endian blobs DMA'd/streamed into byte buffers, then cast to
 * typed structs (`(CROMxxx*)pBytes`) and read field-by-field. Whether such a cast lands
 * 0-mod-4 or 2-mod-4 depends on the byte length of whatever preceded it (a u16 count, a
 * variable-length name). x86 tolerates the resulting misaligned loads; ARM11 does NOT for
 * the strict-alignment forms the modern devkitARM GCC emits:
 *   - `vldr`  (VFP float load)      — faults on any non-4-aligned address
 *   - `ldrd`/`strd` (load/store dual) — fault on any non-8-aligned address
 *   - `ldm`/`stm` (load/store multiple, GCC's fusion of a >=12-byte struct copy) — non-4 faults
 * A single integer `ldr`/`ldrh`/`ldrb`/`str` is unaligned-TOLERANT on ARMv6K (SCTLR.U=1,
 * default on 3DS) — so the fix is to route every float/struct read of a byte-parsed buffer
 * through an INTEGER load into an aligned local, then use it from there.
 *
 * These accessors are codegen-neutral on x86 (each compiles to a plain mov) and ARM-safe
 * (verified: the scalar reads emit `ldr`/`ldrh`, never `vldr`; the bulk copy is a noinline
 * byte loop GCC can't fuse to `ldm`). Use them anywhere a possibly-misaligned buffer is read.
 *
 * Always-on (NOT gated on PLATFORM_PORT) so a single fix is correct on PC and 3DS alike.
 */
#ifndef _TUROK_ALIGN_H
#define _TUROK_ALIGN_H

/* Bulk copy out of a possibly-misaligned buffer. `noinline` is CRITICAL: without it GCC at
 * -O2 sees the constant size at the call site and fuses the copy into `ldm/stm`, which faults
 * on a non-4-aligned source — defeating the entire purpose. One call per copy, load-time only. */
__attribute__((noinline))
static void turok_memcpy_unaligned(void *dst, const void *src, unsigned long n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++;
}

/* Scalar reads: integer load (unaligned-tolerant on ARMv6K) into an aligned local, returned
 * by value. The float reader deliberately loads as u32 then bit-casts via an aligned local,
 * so the load FROM the byte buffer is an integer `ldr` (never `vldr`). */
static __inline__ unsigned short turok_rd_u16(const void *p)
{ unsigned short v; __builtin_memcpy(&v, p, 2); return v; }

static __inline__ short turok_rd_s16(const void *p)
{ short v; __builtin_memcpy(&v, p, 2); return v; }

static __inline__ unsigned int turok_rd_u32(const void *p)
{ unsigned int v; __builtin_memcpy(&v, p, 4); return v; }

static __inline__ int turok_rd_s32(const void *p)
{ int v; __builtin_memcpy(&v, p, 4); return v; }

static __inline__ float turok_rd_f32(const void *p)
{ unsigned int u; __builtin_memcpy(&u, p, 4);   /* integer ldr from the buffer (unaligned-OK) */
  float f; __builtin_memcpy(&f, &u, 4);          /* aligned local -> local bit-cast */
  return f; }

#endif /* _TUROK_ALIGN_H */
