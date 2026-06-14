/*------------------------------------------------------------------------------
* ADPCM Signal decode - Nintendo 64
* Copyright (c) 1996 Iguana Entertainment Inc
*
* C version by C.J.Galley
* Assembly conversion by Biscuit 11/14/96
*-------------------------------------------------------------------------------*/
/* Register defines, this is compatible with what appears in the debugger */

#define zero	$0
#define at	$1
#define v0	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7
#define	t0	$8
#define	t1	$9
#define	t2	$10
#define	t3	$11
#define	t4	$12
#define	t5	$13
#define	t6	$14
#define	t7	$15
#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24
#define t9	$25
#define k0	$26
#define k1	$27
#define gp	$28
#define sp	$29
#define fp	$30
#define ra	$31
#define return	s7

/* This is the more permanent block even though it's the t registers!!!! */
#define rdh_BitPtr	t0
#define rdh_BitsInBuffer t1
#define rdh_BitBuffer	t2
#define rdd_BitPtr	t3
#define rdd_BitsInBuffer t4
#define rdd_BitBuffer	t5
#define temp		a3

#define FIXED_SCALE	16
#define FIXED_SCALE_SHIFTED (1<<FIXED_SCALE)
#define	MAX_SIGNALS	4			/* Maximum number of Signals to process simultaneously */
#define	BLOCK		4 			/* Block Size for ADPCM Data */
#define	BITS		3			/* Number of Bits / Sample to use in Encoding */

#define	BITM		((1<<BITS)-1)		/* BitMask from BITS */
#define NOT_BITM	((-1<<BITS))		/* Not Bitmask from BITS */
#define	BIT_SIGN	(1<<(BITS-1))		/* Position of Sign Bit */
#define	VMIN		(-(1<<(BITS-1)))	/* Maximum Negative Value */
#define	VMAX		((1<<(BITS-1))-1)	/* Maximum Positive Value */

#define	MAX_SHIFTS	(16-BITS)			// Maximum number of Shifts to apply to Data

#define	F0CA	0
#define	F0CB	0
#define	F1CA	0xF000				/* 0.9375 */
#define	F1CB	0
#define	F2CA	0x1E800				/* 1.90625 */
#define	F2CB	-0xF000				/* -0.9375 */
#define	F3CA	0x1CC00				/* 1.796875 */
#define	F3CB	-0xD000				/* -0.8125 */

	.text
	.option pic0
	.set noreorder
	.globl	adpcmDecode

/* Register defines within  adpcmDecode */
#define RawBuffers	s0
#define	ADPCMBuffer	s1
#define NumSignals	s2
#define SampleLen	s3
#define NumBlocks	s4
#define	adpcm_Hdr	s5
#define Index     	s6
#define BitPos    	s7
/* Use: adpcmDecode (s16 **RawBuffers, u8 *ADPCMBuffer, s32 NumSignals) */
	.ent adpcmDecode
adpcmDecode:
	.frame $sp,0,$31
/* Save all registers on entry. Eventually I will find out which ones actually do need saved! */
/* We don't save v0 because we will be using it */
	sub	sp, sp, 40
	sw	s0,0(sp)
	sw	s1,4(sp)
	sw	s2,8(sp)
	sw	s3,12(sp)
	sw	s4,16(sp)
	sw	s5,20(sp)
	sw	s6,24(sp)
	sw	s7,28(sp)
	sw	ra,32(sp)

	move	RawBuffers,a0
	move	ADPCMBuffer,a1
	move	NumSignals,a2

	/* Initialize pointer to ADPCM data */
	move	adpcm_Hdr,ADPCMBuffer

	/* Get Header info from stream */
	lbu	temp,(adpcm_Hdr)
	lbu	SampleLen,1(adpcm_Hdr)
	sll	temp,temp,8
	add	adpcm_Hdr,adpcm_Hdr,2
	or	SampleLen,temp,SampleLen

	/* Determine number of blocks which were encoded */
	add	NumBlocks,SampleLen,BLOCK-1
	divu	NumBlocks,BLOCK

	/* Set up bitstream to read data */
	sll	BitPos,NumBlocks,2		/* BitPos = NumBlocks*4 */
	add	temp,NumBlocks,NumBlocks	/* temp = NumBlocks*2 */
	add	BitPos,temp,BitPos   		/* BitPos = NumBlocks*6 */

	srl	temp,BitPos,3 			/* Arg 0 = BitPos/8 +adpcm_Hdr*/
	add	a0,temp,adpcm_Hdr
	jal	adpcmReadBitsInitD
	and	a1,BitPos,7				/* Arg 1 = BitPos & 7 */

	/* Decompress each signal in turn */

	move	Index,zero
	sub	sp,sp,24				/* Set up space to save our registers we need */
__DecodeLoop:
	bge	Index,NumSignals,__DecodeLoopTerm
	nop
	jal	adpcmReadBitsInitH		/* Initialize for each signal */
	move	a0,adpcm_Hdr			/* Delay slot */

	lw	a0,(RawBuffers)
	add	RawBuffers,RawBuffers,4

	/* Save registers we know will be trashed in adpcmDecodeSignal */
	sw	Index,0(sp)
	sw	RawBuffers,4(sp)
	sw	SampleLen,8(sp)
	sw	NumSignals,12(sp)
	sw	adpcm_Hdr,16(sp)

	move	return,zero		  	/* DEBUG CODE */
	jal	adpcmDecodeSignal
	move	a1,SampleLen			/* Delay Slot */

	/* TEST CODE, EXIT LOOP IF DECODE COMPARE FAILS */
	bne	return,zero,__DecodeLoopTerm
	nop
	/* Restore registers from adpcmDecodeSignal and branch back for loop */
	lw	Index,0(sp)
	lw	RawBuffers,4(sp)
	lw	SampleLen,8(sp)
	lw	NumSignals,12(sp)
	lw	adpcm_Hdr,16(sp)
	j	__DecodeLoop
	add	Index,Index,1    		/* Delay slot */

__DecodeLoopTerm:
	add	sp,sp,24
	move	v0,SampleLen
	move	v0,return			/* DEBUG VALUE */

/* Restore all registers except v0 */
	lw	s0,0(sp)
	lw	s1,4(sp)
	lw	s2,8(sp)
	lw	s3,12(sp)
	lw	s4,16(sp)
	lw	s5,20(sp)
	lw	s6,24(sp)
	lw	ra,32(sp)			/* Need an instr after for pipeline */
	lw	s7,28(sp)
	jr	ra
	add	sp, sp, 40

	.end ASMadpcmDecode

#undef RawBuffers
#undef ADPCMBuffer
#undef NumSignals
#undef SampleLen
#undef NumBlocks
#undef adpcm_Hdr
#undef Index
#undef BitPos

/* static void adpcmReadBitsInitH (u8 *Data) */
	.ent adpcmReadBitsInitH
adpcmReadBitsInitH:
	.frame $sp,0,$31
	lbu	rdh_BitBuffer,(a0)
	lbu	temp,1(a0)
	lbu	a1,2(a0)
	sll	temp,temp,8
	sll	a1,a1,16
	or	rdh_BitBuffer,rdh_BitBuffer,temp
	lbu	temp,3(a0)
	or	rdh_BitBuffer,rdh_BitBuffer,a1
	sll	temp,temp,24
	add	rdh_BitsInBuffer,zero,32
	or	rdh_BitBuffer,rdh_BitBuffer,temp
	jr	ra
	add	rdh_BitPtr,a0,4
	.end adpcmReadBitsInitH

/* static s32 adpcmReadBitsH (s32 n) */
#define v v0
#define n a0
	.ent adpcmReadBitsH
adpcmReadBitsH:
	.frame $sp,0,$31

	/* Creates bitmask for masking out return value */
	add	temp,zero,1
	sub	rdh_BitsInBuffer,rdh_BitsInBuffer,n
	sll	temp,temp,n
	sub	temp,temp,1
	and	v,rdh_BitBuffer,temp
	bgt	rdh_BitsInBuffer,24,RDH_NoReload
	srl	rdh_BitBuffer,rdh_BitBuffer,n
	lbu	temp,(rdh_BitPtr)
	add	rdh_BitPtr,rdh_BitPtr,1
	sll	temp,temp,rdh_BitsInBuffer
	add	rdh_BitsInBuffer,rdh_BitsInBuffer,8
	or	rdh_BitBuffer,rdh_BitBuffer,temp
RDH_NoReload:
	jr	ra
	nop
	.end adpcmReadBitsH
#undef n
#undef v

/* static void adpcmReadBitsInitD (u8 *Data,u32 Bit) */
	.ent adpcmReadBitsInitD
adpcmReadBitsInitD:
	.frame $sp,0,$31
	add	rdd_BitsInBuffer,zero,32
	lbu	rdd_BitBuffer,(a0)
	sub	rdd_BitsInBuffer,rdd_BitsInBuffer,a1
	lbu	temp,1(a0)
	lbu	v0,2(a0)
	sll	temp,temp,8
	sll	v0,v0,16
	or	rdd_BitBuffer,rdd_BitBuffer,temp
	lbu	temp,3(a0)
	or	rdd_BitBuffer,rdd_BitBuffer,v0
	sll	temp,temp,24
	or	rdd_BitBuffer,rdd_BitBuffer,temp
	srl	rdd_BitBuffer,rdd_BitBuffer,a1
	jr	ra
	add	rdd_BitPtr,a0,4
	.end adpcmReadBitsInitD

/* static s32 adpcmReadBitsD (s32 n) */
#define v v0
#define n a0
	.ent adpcmReadBitsD
adpcmReadBitsD:
	.frame $sp,0,$31

	/* Creates bitmask for masking out return value */
	add	temp,zero,1
	sub	rdd_BitsInBuffer,rdd_BitsInBuffer,n
	sll	temp,temp,n
	sub	temp,temp,1
	and	v,rdd_BitBuffer,temp
	bgt	rdd_BitsInBuffer,24,RDD_NoReload
	srl	rdd_BitBuffer,rdd_BitBuffer,n

	lbu	temp,(rdd_BitPtr)
	add	rdd_BitPtr,rdd_BitPtr,1
	sll	temp,temp,rdd_BitsInBuffer
	add	rdd_BitsInBuffer,rdd_BitsInBuffer,8
	or	rdd_BitBuffer,rdd_BitBuffer,temp
RDD_NoReload:
	jr	ra
	nop
	.end adpcmReadBitsD
#undef n
#undef v

/********************************************************************************/
/* ADPCMDecodeSignal:								*/
/* Entry:	A0 - ptr to raw data buffer					*/
/*		A1 - Sample length						*/
/* Exit:	None								*/
/* Note: this will use all registers and assumes some have already been set up  */
/*       by calling routine.							*/
/********************************************************************************/
/* Register defines within  adpcmDecodeSignal */
#define RawBuffer	s0
#define	SampleLen	s1
#define	x1		s2
#define x2		s3
#define ps		s4
#define	v		s5
#define Shift		s6

#define	Type		t6
#define BlockPos	t7
#define	BlockHeader	t8
#define	t		t9
#define	fca		v1
#define fcb		a2

	.ent adpcmDecodeSignal
adpcmDecodeSignal:
	.frame $sp,0,$31
	sub	sp,sp,8
	sw	ra,(sp)
	move	RawBuffer,a0
	move	SampleLen,a1

	move	x1,zero
	move	x2,zero
	move	BlockPos,zero

	/* Get initial prediction from buffer */
	jal	adpcmReadBitsD			/* ps = (s16)(adpcmReadBitsD(8)<<8) */
	add	a0,zero,8 			/* Delay Slot */

	sll	ps,v0,24			/* Sign extend return value */
	sra	ps,ps,16

__ADPCMDecode_Loop:
	/* Loop while Data Left */
	beq	SampleLen,zero,__ADPCMEndOfData
	subu	SampleLen,SampleLen,1

	/* If First sample in New Block then read Block Header Details */

	bne	BlockPos,zero,__ADPCMNoNewBlock
	sub	BlockPos,BlockPos,1			/* Delay Slot */

	add	BlockPos,zero,BLOCK-1
	jal	adpcmReadBitsH				/* Read 6 bits */
	add	a0,zero,6
	move	BlockHeader,v0				/* Store return value in correct reg */

	and	Shift,BlockHeader, 0x0f			/* Get some sort of shift? */
	srl	temp,BlockHeader,4-2			/* Get the block type */
	and	Type,temp,0x03*4			/* Use this to make it into a long addr ptr */
	lw	fca,FCA(Type)
	lw	fcb,FCB(Type)
__ADPCMNoNewBlock:
	/* Get next Sample & apply filter to decode it */
	jal	adpcmReadBitsD
	add	a0,zero,BITS				/* Delay slot, entry parameter */

	mult	fca,x1
	mflo	temp
	sll	t,v0,32-BITS
	sra	t,t,32-BITS
	sll	t,t,Shift
	mult	fcb,x2
	mflo	v0
	addu	temp,temp,v0
	sra	temp,temp,FIXED_SCALE

	addu	t,t,temp
	move	x2,x1
	move	x1,t
	add	v,ps,t
	move	ps,v

	/* Here we're doing some wierd stuff to do the limit setting, don't worry it'll work. Honest */
	sra	temp,v,15			/* Get sign extend bits */
	beq	temp,zero,__ADPCMNoLimitsHit	/* Result was positive and in range */
	add	temp,temp,1
	beq	temp,zero,__ADPCMNoLimitsHit	/* Result was negative and in range */
	sra	temp,v,16			/* Check sign extension */
	beq	temp,zero,__ADPCMNoLimitsHit
	add	v,zero,32767			/* This is the upper limit - delay slot */
	addu	v,zero,0x8000			/* This is the lower limit */
__ADPCMNoLimitsHit:

	sh	v,(RawBuffer)
__skip:
	j	__ADPCMDecode_Loop
	add	RawBuffer,RawBuffer,2
__ADPCMEndOfData:

	lw	ra,(sp)
	add	sp,sp,8
	jr	ra
	nop
	.end adpcmDecodeSignal

/***************************************************************************************/
/* Data tables for coefficients                                                        */
/***************************************************************************************/
	.data
	.align 4
FCA:
	.word	F0CA
	.word	F1CA
	.word	F2CA
	.word	F3CA

FCB:	.word	F0CB
	.word	F1CB
	.word	F2CB
	.word	F3CB
