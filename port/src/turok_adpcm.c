/* turok_adpcm.c — host C port of the ANIMATION ADPCM decoder.
 *
 * The original `adpcmDecode` lives in src/PR/tengine/adpcm.s (MIPS assembly), which can't
 * compile on the host. It decompresses the per-node, per-frame quaternion (CRotFrame) and
 * position (CPosFrame) keyframe streams in anim.c (CGameObjectInstance__DecompressAnim).
 * Without a real implementation the symbol resolved to a no-op audio stub, so every node
 * rotation decoded to a ZERO quaternion -> CQuatern__ToMatrix -> zero 3x3 -> all animated
 * model geometry collapsed to a single point -> enemies/pickups/devices invisible.
 *
 * This is a straight C port of the in-tree reference src/PR/tengine/dosvers/decadpcm.cpp.
 * It reads the ADPCM bitstream BYTE-BY-BYTE, so it is endian-agnostic (the N64 streams are
 * big-endian but the byte-wise reads reconstruct the values correctly on a little-endian host).
 */
/* Coefficients + sizes, inlined from src/PR/tengine/adpcm2.h (not on the port include path). */
#define MAX_FILTERS  4
#define F0CA   0
#define F0CB   0
#define F1CA   0.9375
#define F1CB   0
#define F2CA   1.90625
#define F2CB  -0.9375
#define F3CA   1.796875
#define F3CB  -0.8125
#define BLOCK     4              /* block size for ADPCM data */
#define BITS      3              /* bits / sample */
#define BITM     ((1<<BITS)-1)
#define BIT_SIGN (1<<(BITS-1))

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef signed char    s8;
typedef signed short   s16;
typedef signed int     s32;

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

/* 1 = fixed point coefficients (matches the N64/dosvers build) */
#define COEF_FIXED   1

#if COEF_FIXED
#define FIXED_SCALE  16
#define COEF         s32
static s32 FCA[MAX_FILTERS] = { F0CA*(1<<FIXED_SCALE), F1CA*(1<<FIXED_SCALE), F2CA*(1<<FIXED_SCALE), F3CA*(1<<FIXED_SCALE) };
static s32 FCB[MAX_FILTERS] = { F0CB*(1<<FIXED_SCALE), F1CB*(1<<FIXED_SCALE), F2CB*(1<<FIXED_SCALE), F3CB*(1<<FIXED_SCALE) };
#else
#define COEF         float
static float FCA[MAX_FILTERS] = { F0CA, F1CA, F2CA, F3CA };
static float FCB[MAX_FILTERS] = { F0CB, F1CB, F2CB, F3CB };
#endif

/* --- Header bitstream --- */
static u32 rdh_BitBuffer;
static s32 rdh_BitsInBuffer;
static u8 *rdh_BitPtr;
/* --- Data bitstream --- */
static u32 rdd_BitBuffer;
static s32 rdd_BitsInBuffer;
static u8 *rdd_BitPtr;

static void adpcmReadBitsInitH(u8 *Data)
{
	rdh_BitBuffer  = *Data++;
	rdh_BitBuffer |= *Data++ << 8;
	rdh_BitBuffer |= *Data++ << 16;
	rdh_BitBuffer |= (u32)*Data++ << 24;
	rdh_BitsInBuffer = 32;
	rdh_BitPtr = Data;
}

static s32 adpcmReadBitsH(s32 n)
{
	s32 v;
	v = rdh_BitBuffer & ~(-1 << n);
	rdh_BitBuffer >>= n;
	rdh_BitsInBuffer -= n;
	if (rdh_BitsInBuffer <= 24) {
		rdh_BitBuffer |= (u32)(*rdh_BitPtr++) << rdh_BitsInBuffer;
		rdh_BitsInBuffer += 8;
	}
	return v;
}

static void adpcmReadBitsInitD(u8 *Data, u32 Bit)
{
	rdd_BitBuffer  = *Data++;
	rdd_BitBuffer |= *Data++ << 8;
	rdd_BitBuffer |= *Data++ << 16;
	rdd_BitBuffer |= (u32)*Data++ << 24;
	rdd_BitBuffer >>= Bit;
	rdd_BitsInBuffer = 32 - Bit;
	rdd_BitPtr = Data;
}

static u16 adpcmReadBitsD(s32 n)
{
	s16 v;
	v = rdd_BitBuffer & ~(-1 << n);
	rdd_BitBuffer >>= n;
	rdd_BitsInBuffer -= n;
	if (rdd_BitsInBuffer <= 24) {
		rdd_BitBuffer |= (u32)(*rdd_BitPtr++) << rdd_BitsInBuffer;
		rdd_BitsInBuffer += 8;
	}
	return v;
}

static void adpcmDecodeSignal(s16 *RawBuffer, s32 SampleLen)
{
	s32 x1 = 0, x2 = 0;
	s32 ps, v, t;
	s16 s;
	s16 Shift = 0, Type = 0;
	s32 BlockPos = 0;
	u8  BlockHeader;
	COEF fca = 0, fcb = 0;

	/* initial prediction */
	ps = (s16)(adpcmReadBitsD(8) << 8);

	while (SampleLen-- > 0) {
		if (BlockPos-- == 0) {
			BlockPos = (BLOCK - 1);
			BlockHeader = adpcmReadBitsH(6);
			Shift = (BlockHeader) & 0x0F;
			Type  = (BlockHeader >> 4) & 0x03;
			fca = FCA[Type]; fcb = FCB[Type];
		}

		s = adpcmReadBitsD(BITS);
		if (s & BIT_SIGN)
			t = ((s | ~BITM) << Shift);
		else
			t = (s << Shift);
#if COEF_FIXED
		t += (x1 * fca + x2 * fcb) >> FIXED_SCALE;
#else
		t += (x1 * fca + x2 * fcb);
#endif
		x2 = x1;
		x1 = t;
		v = ps + t;
		ps = v;

		v = min(32767, v);
		v = max(-32768, v);
		*RawBuffer++ = v;
	}
}

s32 adpcmDecode(s16 **RawBuffers, u8 *ADPCMBuffer, s32 NumSignals)
{
	s32 SampleLen, NumBlocks, i, BitPos;
	u8 *adpcm_Hdr = ADPCMBuffer;

	SampleLen  = *adpcm_Hdr++ << 8;
	SampleLen |= *adpcm_Hdr++;

	NumBlocks = (SampleLen + BLOCK - 1) / BLOCK;

	BitPos = NumBlocks * 6;
	adpcmReadBitsInitD(adpcm_Hdr + (BitPos / 8), BitPos & 7);

	for (i = 0; i < NumSignals; i++) {
		adpcmReadBitsInitH(adpcm_Hdr);
		adpcmDecodeSignal(RawBuffers[i], SampleLen);
	}

	return SampleLen;
}
