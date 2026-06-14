/******************************************************************************
 * audiomgr.c
 *
 * This code implements the audio manager. This provides a low level
 * interface to the audio library, and manages the routines needed to
 * create the audio task.
 *
 * At the begining of each video retrace, the scheduler sends a message
 * that wakes up the audio thread, which calls alAudioFrame to build an
 * audio task. Once this task is built, it is sent to the scheduler, that
 * will then send the task to the rsp for execution.
 *
 * Copyright 1993, Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
 * Inc.; the contents of this file may not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in part,
 * without the prior written permission of Silicon Graphics, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, and/or in similar or successor clauses in the FAR,
 * DOD or NASA FAR Supplement. Unpublished - rights reserved under the
 * Copyright Laws of the United States.
 *****************************************************************************/

#include "stdafx.h"
#include "tengine.h"
#include "audio.h"

#define AUDIO_DONE_MSG (OS_SC_LAST_MSG + 3)

/****  type define's for structures unique to audiomgr ****/
typedef union {

    struct
	{
        short     type;
    } gen;

    struct {
        short     type;
        struct    AudioInfo_s *info;
    } done;

    OSScMsg       app;

} AudioMsg;

typedef struct AudioInfo_s
{
    short         *data;          /* Output data pointer */
    short         frameSamples;   /* # of samples synthesized in this frame */
    OSScTask      task;           /* scheduler structure */
    AudioMsg      msg;            /* completion message */
} AudioInfo;

typedef struct
{
    Acmd          *ACMDList[NUM_ACMD_LISTS];
    AudioInfo     *audioInfo[NUM_OUTPUT_BUFFERS];
    OSThread      thread;
    OSMesgQueue   audioFrameMsgQ;
    OSMesg        audioFrameMsgBuf[MAX_MESGS];
    OSMesgQueue   audioReplyMsgQ;
    OSMesg        audioReplyMsgBuf[MAX_MESGS];
    ALGlobals     g;
} AMAudioMgr;

typedef struct
{
    ALLink        node;
    u32           startAddr;
    u32           lastFrame;
    char          *ptr;
} AMDMABuffer;

typedef struct
{
    u8            initialized;
    AMDMABuffer   *firstUsed;
    AMDMABuffer   *firstFree;
} AMDMAState;


/**** audio manager globals ****/
extern OSSched         sc;
extern int	 audio_debug_flag ;


AMAudioMgr      __am;

AMDMAState      dmaState;
AMDMABuffer     dmaBuffs[NUM_DMA_BUFFERS];
u32             audFrameCt = 0;
u32             nextDMA = 0;
u32             curAcmdList = 0;
u32             minFrameSize;
u32             frameSize;
u32             maxFrameSize;
u32             maxRSPCmds;
AudioMsg			 AudioConsumedMsg;					// message buffer for audio consumed msg

/** Queues and storage for use with audio DMA's ****/
OSIoMesg        audDMAIOMesgBuf[NUM_DMA_MESSAGES];
OSMesgQueue     audDMAMessageQ;
OSMesg          audDMAMessageBuf[NUM_DMA_MESSAGES];

/**** private routines ****/
static void __amMain(void *arg);
static s32  __amDMA(s32 addr, s32 len, void *state);
static ALDMAproc __amDmaNew(AMDMAState **state);
static u32  __amHandleFrameMsg(AudioInfo *, AudioInfo *);
static void __amHandleDoneMsg(AudioInfo *);
static void __clearAudioDMA(void);

/******************************************************************************
 * Audio Manager API
 *****************************************************************************/
void amCreateAudioMgr(ALSynConfig *c, OSPri pri, amConfig *amc)
{
    u32     i;
    f32     fsize;

    dmaState.initialized = FALSE;	/* Reset this before the first call to __amDmaNew */

	 AudioConsumedMsg.gen.type = AUDIO_DONE_MSG;
    c->dmaproc    = __amDmaNew;
    c->outputRate = osAiSetFrequency(amc->outputRate);

    /*
     * Calculate the frame sample parameters from the
     * video field rate and the output rate
     */
    fsize = (f32) amc->framesPerField * c->outputRate / (f32) 60;
    frameSize = (s32) fsize;

    if (frameSize < fsize)
        frameSize++;

    if (frameSize & 0xf)
        frameSize = (frameSize & ~0xf) + 0x10;

    minFrameSize = frameSize - 16;
    maxFrameSize = frameSize + EXTRA_SAMPLES + 16;

    alInit(&__am.g, c);

	// allocate buffers for voices
    dmaBuffs[0].node.prev = 0;
    dmaBuffs[0].node.next = 0;

    for (i=0; i<NUM_DMA_BUFFERS-1; i++)
    {
        alLink((ALLink*)&dmaBuffs[i+1],(ALLink*)&dmaBuffs[i]);
        dmaBuffs[i].ptr = alHeapAlloc(c->heap, 1, DMA_BUFFER_LENGTH);
        //rmonPrintf("Buffer(%d): %x\n", i, dmaBuffs[i].ptr );
    }

    /* last buffer already linked, but still needs buffer */
    dmaBuffs[i].ptr = alHeapAlloc(c->heap, 1, DMA_BUFFER_LENGTH);

    for(i=0;i<NUM_ACMD_LISTS;i++)
        __am.ACMDList[i] = (Acmd*)alHeapAlloc(c->heap, 1, amc->maxACMDSize * sizeof(Acmd));

    maxRSPCmds = amc->maxACMDSize;

    /**** initialize the done messages ****/
    for (i = 0; i < NUM_OUTPUT_BUFFERS; i++) // num output buffers = 3
    {
        __am.audioInfo[i] = (AudioInfo *)alHeapAlloc(c->heap, 1, sizeof(AudioInfo));
        __am.audioInfo[i]->msg.done.type = OS_SC_DONE_MSG;
        __am.audioInfo[i]->msg.done.info = __am.audioInfo[i];
        __am.audioInfo[i]->data = alHeapAlloc(c->heap, 1, 4*maxFrameSize);  // allocate output buffer
    }

    osCreateMesgQueue(&__am.audioReplyMsgQ, __am.audioReplyMsgBuf, MAX_MESGS);
    osCreateMesgQueue(&__am.audioFrameMsgQ, __am.audioFrameMsgBuf, MAX_MESGS);
    osCreateMesgQueue(&audDMAMessageQ, audDMAMessageBuf, NUM_DMA_MESSAGES);

	// Attach audio buffer consumed interrupt to the audio message queue
	osSetEventMesg(OS_EVENT_AI, &__am.audioFrameMsgQ, &AudioConsumedMsg);

    osCreateThread(&__am.thread, THREAD_AUDIO, __amMain, 0,
                   (void *)(audio_stack+AUDIO_STACKSIZE/sizeof(u64)), pri);
    osStartThread(&__am.thread);

	// removed!
	// Start the audio thread ticking!
	//osSendMesg(&__am.audioFrameMsgQ,&AudioConsumedMsg,OS_MESG_NOBLOCK);
}

/******************************************************************************
 *
 * Audio Manager implementation. This thread wakes up at every retrace,
 * and builds an audio task, which it returns to the scheduler, who then
 * is responsible for its finally execution on the RSP. Once the task has
 * finished execution, the scheduler sends back a message saying the task
 * is complete. The audio is triple buffered because the switching to a new
 * audio buffer does not occur exactly at the gfx swapbuffer time.  With
 * 3 buffers you ensure that the program does not destroy data before it is
 * played.
 *
 *****************************************************************************/
//#define DISABLE_AUDIO

static void __amMain(void *arg)
{
	u32         validTask;
	u32         done = 0;
	AudioMsg    *msg = 0;
	AudioInfo   *lastInfo = 0;
	OSScClient  client;
	u32			RetraceCount=0;
//	static BOOL	first = TRUE;
//	OSTime		lastTime, thisTime;

#ifndef DISABLE_AUDIO
    osScAddClient(&sc, &client, &__am.audioFrameMsgQ);
#endif

    while (!done)
    {
		osRecvMesg(&__am.audioFrameMsgQ, (OSMesg *)&msg, OS_MESG_BLOCK);

        switch (msg->gen.type)
        {
			case (OS_SC_RETRACE_MSG):
				RetraceCount++;
				if (RetraceCount>20)
				{
					RetraceCount=0;
					TRACE0("Needed to kick the audio\n");
				}
				else
				{
					break;
				}

			case (AUDIO_DONE_MSG):
				//
				// We whizz about this little loop until we fill the audio FIFO
				//
				while ( !(osAiGetStatus() & AI_STATUS_FIFO_FULL) )
				{
					validTask = __amHandleFrameMsg(__am.audioInfo[audFrameCt % 3], lastInfo);
            		if(validTask)
            		{
						RetraceCount=0;
						osRecvMesg(&__am.audioReplyMsgQ, (OSMesg *)&msg,OS_MESG_BLOCK);
				 		__amHandleDoneMsg(msg->done.info);
						lastInfo = msg->done.info;
						//osSyncPrintf("Data process time =%lld\n",OS_CYCLES_TO_USEC(osGetTime()-ProcTime));
					}
					else
					{
						break;
						//rmonPrintf("not valid task\n");
					}
				}
            	break;

            case (OS_SC_PRE_NMI_MSG):
                break;

            case (QUIT_MSG):
                done = 1;
                break;

            default:
                break;
        }
    }

    alClose(&__am.g);
}

/******************************************************************************
 *
 * __amHandleFrameMsg. First, clear the past audio dma's, then calculate
 * the number of samples you will need for this frame. This value varies
 * due to the fact that audio is synchronised off of the video interupt
 * which can have a small amount of jitter in it. Varying the number of
 * samples slightly will allow you to stay in synch with the video. This
 * is an advantageous thing to do, since if you are in synch with the
 * video, you will have fewer graphics yields. After you've calculated
 * the number of frames needed, call alAudioFrame, which will call all
 * of the synthesizer's players (sequence player and sound player) to
 * generate the audio task list. If you get a valid task list back, put
 * it in a task structure and send a message to the scheduler to let it
 * know that the next frame of audio is ready for processing.
 *
 *****************************************************************************/

static u32 __amHandleFrameMsg(AudioInfo *info, AudioInfo *lastInfo)
{
    OSScTask	*t;
	 s16			*audioPtr;
    Acmd			*cmdp;
    s32			cmdLen;
    //int			samplesLeft = 0;
	 int			check = 0;

//	static s16			*buffer1 = NULL;
//	static s16			*buffer2 = NULL;
//	static s16			*buffer3 = NULL;
//	int					temp = 0;
//	INT32					*pTemp;


	 // audFrameCnt updated here
    __clearAudioDMA(); /* call once a frame, before doing alAudioFrame */


    audioPtr = (s16 *) osVirtualToPhysical(info->data); //info->data addr of current buffer

	 // set up the next DMA transfer from DRAM to the audio interface buffer.
	 // lastInfo->data is the buffer used in the last audio frame. It should be full.
	if (lastInfo)
	{
		check = osAiSetNextBuffer(lastInfo->data, lastInfo->frameSamples<<2);

		if (!(osAiGetStatus() & AI_STATUS_FIFO_FULL))
		{
			TRACE0("osAiGetStatus() returned not full\n");

			// no longer needed
			//osAiSetNextBuffer(lastInfo->data, lastInfo->frameSamples<<2);
		}

		if (check < 0)
		{
			TRACE0("Warning: AI_STATUS_FIFO_FULL, DMA's aborted\n");
		}
	}

    /* calculate how many samples needed for this frame to keep the DAC full */
    /* this will vary slightly frame to frame, must recalculate every frame */
    //samplesLeft = osAiGetLength() >> 2; /* divide by four, to convert bytes */
                                        /* to stereo 16 bit samples */
    //info->frameSamples = 16 + (frameSize - samplesLeft + EXTRA_SAMPLES)& ~0xf;
	 // no longer necessary to have extra samples, because the buffers
	 // will be swapped exactly when the buffer runs out
	 info->frameSamples = frameSize;

    if(info->frameSamples < minFrameSize)
        info->frameSamples = minFrameSize;

	//ASSERT(((DWORD)lastInfo->data + (lastInfo->frameSamples<<2)) & 0x00003FFF != 0x00002000);
	//check = osAiSetNextBuffer(lastInfo->data, lastInfo->frameSamples<<2);

	cmdp = alAudioFrame(__am.ACMDList[curAcmdList], &cmdLen, audioPtr,
							info->frameSamples);

	 ASSERT(cmdLen <= maxRSPCmds);

    if(cmdLen == 0)  /* no task produced, return zero to show no valid task */
        return 0;

	 // this is the task for the next audio frame
   t = &info->task;
   t->next      = 0;                    /* paranoia */
   t->msgQ      = &__am.audioReplyMsgQ; /* reply to when finished */
   t->msg       = (OSMesg)&info->msg;   /* reply with this message */
   t->flags     = OS_SC_NEEDS_RSP;

   t->list.t.data_ptr    = (u64 *) __am.ACMDList[curAcmdList];
   t->list.t.data_size   = (cmdp - __am.ACMDList[curAcmdList]) * sizeof(Acmd);
   t->list.t.type  = M_AUDTASK;
   t->list.t.ucode_boot = (u64 *)rspbootTextStart;
   t->list.t.ucode_boot_size =
        ((int) rspbootTextEnd - (int) rspbootTextStart);

#ifdef USE_FIFO
	t->list.t.flags  = 0;
#else
//   t->list.t.flags  = OS_TASK_DP_WAIT;
   t->list.t.flags  = 0;
#endif

   t->list.t.ucode = (u64 *) aspMainTextStart;
   t->list.t.ucode_data = (u64 *) aspMainDataStart;
   t->list.t.ucode_data_size = SP_UCODE_DATA_SIZE;
   t->list.t.yield_data_ptr = NULL;
   t->list.t.yield_data_size = 0;

	t->list.t.dram_stack      	= NULL;
	t->list.t.dram_stack_size	= 0;
	t->list.t.output_buff		= NULL;
	t->list.t.output_buff_size = 0;

	scSendCommand(&sc, t);


    curAcmdList ^= 1; /* swap which acmd list you use each frame */

	 UpdateWorldSound();

	 return 1;
}

/******************************************************************************
 *
 * __amHandleDoneMsg. Really just debugging info in this frame. Checks
 * to make sure we completed before we were out of samples.
 *
 *****************************************************************************/
static void __amHandleDoneMsg(AudioInfo *info)
{
    s32    samplesLeft;
    static int firstTime = 1;

    samplesLeft = osAiGetLength()>>2;
    if (samplesLeft == 0 && !firstTime)
    {
        TRACE0("audio: ai out of samples\n");
//		  ASSERT(FALSE);	// not needed
        firstTime = 0;
    }
}

/******************************************************************************
 *
 * __amDMA This routine handles the dma'ing of samples from rom to ram.
 * First it checks the current buffers to see if the samples needed are
 * already in place. Because buffers are linked sequentially by the
 * addresses where the samples are on rom, it doesn't need to check all
 * of them, only up to the address that it needs. If it finds one, it
 * returns the address of that buffer. If it doesn't find the samples
 * that it needs, it will initiate a DMA of the samples that it needs.
 * In either case, it updates the lastFrame variable, to indicate that
 * this buffer was last used in this frame. This is important for the
 * __clearAudioDMA routine.
 *
 *****************************************************************************/
s32 __amDMA(s32 addr, s32 len, void *state)
{
    void            *foundBuffer;
    s32             delta, addrEnd, buffEnd;
    AMDMABuffer     *dmaPtr, *lastDmaPtr;

    lastDmaPtr = 0;
    dmaPtr = dmaState.firstUsed;
    addrEnd = addr+len;

    /* first check to see if a currently existing buffer contains the
       sample that you need.  */

#if 1
    while(dmaPtr)
    {
        buffEnd = dmaPtr->startAddr + DMA_BUFFER_LENGTH;
        if(dmaPtr->startAddr > addr) /* since buffers are ordered */
            break;                   /* abort if past possible */

        else if(addrEnd <= buffEnd) /* yes, found a buffer with samples */
        {
            dmaPtr->lastFrame = audFrameCt; /* mark it used */
            foundBuffer = dmaPtr->ptr + addr - dmaPtr->startAddr;
            return (int) osVirtualToPhysical(foundBuffer);
        }
        lastDmaPtr = dmaPtr;
        dmaPtr = (AMDMABuffer*)dmaPtr->node.next;
    }
#endif

    /* get here, and you didn't find a buffer, so dma a new one */

    /* get a buffer from the free list */
    dmaPtr = dmaState.firstFree;

	 ASSERT(dmaPtr);  // be sure you have a buffer,
							// if you don't have one, you're fucked

    dmaState.firstFree = (AMDMABuffer*)dmaPtr->node.next;
    alUnlink((ALLink*)dmaPtr);

    /* add it to the used list */
    if(lastDmaPtr) /* if you have other dmabuffers used, add this one */
    {              /* to the list, after the last one checked above */
        alLink((ALLink*)dmaPtr,(ALLink*)lastDmaPtr);
    }
    else if(dmaState.firstUsed) /* if this buffer is before any others */
    {                           /* jam at begining of list */
        lastDmaPtr = dmaState.firstUsed;
        dmaState.firstUsed = dmaPtr;
        dmaPtr->node.next = (ALLink*)lastDmaPtr;
        dmaPtr->node.prev = 0;
        lastDmaPtr->node.prev = (ALLink*)dmaPtr;
    }
    else /* no buffers in list, this is the first one */
    {
        dmaState.firstUsed = dmaPtr;
        dmaPtr->node.next = 0;
        dmaPtr->node.prev = 0;
    }

    foundBuffer = dmaPtr->ptr;
    delta = addr & 0x1;
    addr -= delta;
    dmaPtr->startAddr = addr;
    dmaPtr->lastFrame = audFrameCt;  /* mark it */

	ASSERT(nextDMA < NUM_DMA_MESSAGES);

	 //osPiStartDma(&audDMAIOMesgBuf[nextDMA++], OS_MESG_PRI_NORMAL, OS_READ,
	osPiStartDma(&audDMAIOMesgBuf[nextDMA++], OS_MESG_PRI_HIGH, OS_READ,
                 (u32)addr, foundBuffer, DMA_BUFFER_LENGTH, &audDMAMessageQ);

    return (int) osVirtualToPhysical(foundBuffer) + delta;
}

/******************************************************************************
 *
 * __amDmaNew.  Initialize the dma buffers and return the address of the
 * procedure that will be used to dma the samples from rom to ram. This
 * routine will be called once for each physical voice that is created.
 * In this case, because we know where all the buffers are, and since
 * they are not attached to a specific voice, we will only really do any
 * initialization the first time. After that we just return the address
 * to the dma routine.
 *
 *****************************************************************************/
ALDMAproc __amDmaNew(AMDMAState **state)
{
//    int         i;

    if(!dmaState.initialized)  /* only do this once */
    {
        dmaState.firstUsed = 0;
        dmaState.firstFree = &dmaBuffs[0];
        dmaState.initialized = 1;
    }

    *state = &dmaState;  /* state is never used in this case */

	//rmonPrintf("dmaNew\n");

    return __amDMA;
}

/******************************************************************************
 *
 * __clearAudioDMA.  Routine to move dma buffers back to the unused list.
 * First clear out your dma messageQ. Then check each buffer to see when
 * it was last used. If that was more than FRAME_LAG frames ago, move it
 * back to the unused list.
 *
 *****************************************************************************/
static void __clearAudioDMA(void)
{
    u32          i;
    OSIoMesg     *iomsg = 0;
    AMDMABuffer  *dmaPtr,*nextPtr;
    static int		DMAbuffers = 0; //, maxDMAbuffers = 0;
//	 static int		maxDMAcount = 0;
//	 static float			maxAudioTime = 0.0;




     // Don't block here. If dma's aren't complete, you've had an audio
     // overrun. (Bad news, but go for it anyway, and try and recover.
	for (i=0; i<nextDMA; i++)
	{
		if (osRecvMesg(&audDMAMessageQ,(OSMesg *)&iomsg,OS_MESG_NOBLOCK) == -1)
		{
			TRACE0("Dma not done\n");
		}
	}


   dmaPtr = dmaState.firstUsed;
	DMAbuffers = 0;

    while(dmaPtr)
    {

			DMAbuffers++;
        nextPtr = (AMDMABuffer*)dmaPtr->node.next;

        /* remove old dma's from list */
        /* Can change FRAME_LAG value if we want.  Should be at least one.  */
        /* Larger values mean more buffers needed, but fewer DMA's */
        if (dmaPtr->lastFrame + FRAME_LAG  < audFrameCt)
        {
            if (dmaState.firstUsed == dmaPtr)
                dmaState.firstUsed = (AMDMABuffer*)dmaPtr->node.next;

				alUnlink((ALLink*)dmaPtr);

				if (dmaState.firstFree)
				{
                alLink((ALLink*)dmaPtr,(ALLink*)dmaState.firstFree);
				}
            else
            {
                dmaState.firstFree = dmaPtr;
                dmaPtr->node.next = 0;
                dmaPtr->node.prev = 0;
            }
        }

		  dmaPtr = nextPtr;
    }


#if 0
	if(DMAbuffers > maxDMAbuffers)
		maxDMAbuffers = DMAbuffers;

	if(nextDMA > maxDMAcount)
		maxDMAcount = nextDMA;

	if(rsp_audio_time > maxAudioTime)
		maxAudioTime = rsp_audio_time;

	if((AW.sfx_timer % 120) == 0)
	{
		//osSyncPrintf("maxDMAbuffers: %d\n", maxDMAbuffers );
		//osSyncPrintf("maxDMAcount: %d\n", maxDMAcount);
		//osSyncPrintf("DMAcount: %d\n", nextDMA);
		//osSyncPrintf("maxAudioTime: %2.2f msec\n\n", maxAudioTime);
		//osSyncPrintf("rsp_audio_time: %2.2f msec\n\n", rsp_audio_time);
	}
#endif

    nextDMA = 0;  // Reset number of DMAs
    audFrameCt++;
}

