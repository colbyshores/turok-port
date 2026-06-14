// sched.cpp

#include <ultralog.h>
#include <sched.h>
#include "tengine.h"
#include "sun.h"

// private typedefs and defines
#define VIDEO_MSG				666
#define RSP_DONE_MSG			667
#define RDP_DONE_MSG			668
#define PRE_NMI_MSG			669
#define SCHED_NEWCMD_MSG	(OS_SC_LAST_MSG + 2)

// OSScTask state
#define OS_SC_DP				0x0001  // set if still needs dp
#define OS_SC_SP				0x0002  // set if still needs sp
#define OS_SC_YIELD			0x0010  // set if yield requested
#define OS_SC_YIELDED		0x0020  // set if yield completed

// OSScTask->flags type		identifier
#define OS_SC_XBUS			(OS_SC_SP | OS_SC_DP)
#define OS_SC_DRAM			(OS_SC_SP | OS_SC_DP | OS_SC_DRAM_DLIST)
#define OS_SC_DP_XBUS		(OS_SC_SP)
#define OS_SC_DP_DRAM		(OS_SC_SP | OS_SC_DRAM_DLIST)
#define OS_SC_SP_XBUS		(OS_SC_DP)
#define OS_SC_SP_DRAM		(OS_SC_DP | OS_SC_DRAM_DLIST)

// private functions
static void    __scMain(void *arg);
static void    __scHandleRetrace(OSSched *s);
static void    __scHandleRSP(OSSched *s);
static void    __scHandleRDP(OSSched *s);

static void    __scAppendList(OSSched *s, OSScTask *t);
OSScTask       *__scTaskReady(OSScTask *t);
static s32     __scTaskComplete(OSSched *s,OSScTask *t);
static void    __scExec(OSSched *sc, OSScTask *sp, OSScTask *dp);
static void		__scYield(OSSched *s);
static s32     __scSchedule(OSSched *sc, OSScTask **sp, OSScTask **dp,
                             s32 availRCP);
static void		__scNewCmd(OSSched *sc);


//#define SC_LOGGING
//#define SC_FR

//#define SC_FIC

#ifdef SC_LOGGING
#define SC_LOG_LEN      32*1024
static OSLog    scLog;
static OSLog    *l = &scLog;
static u32      logArray[SC_LOG_LEN/sizeof(u32)];
#endif

float		this_rsp_graphics_time;

OSTime	rsp_audio_start_time;
OSTime	rsp_graphics_start_time;
OSTime	rsp_start_time;
OSTime	rdp_start_time;

int		c_retrace = 0;


// Scheduler API
/////////////////////////////////////////////////////////////////////////////

void osCreateScheduler(OSSched *sc, void *stack, OSPri priority,
                       u8 mode, u8 numFields)
{
    sc->curRSPTask      = 0;
    sc->curRDPTask      = 0;
    sc->clientList      = 0;
    sc->frameCount      = 0;
    sc->audioListHead   = 0;
    sc->gfxListHead     = 0;
    sc->audioListTail   = 0;
    sc->gfxListTail     = 0;
	 sc->doAudio			= 0;
    sc->retraceMsg.type = OS_SC_RETRACE_MSG;  // sent to apps
    sc->prenmiMsg.type  = OS_SC_PRE_NMI_MSG;

    osCreateMesgQueue(&sc->interruptQ, sc->intBuf, OS_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->cmdQ, sc->cmdMsgBuf, OS_SC_MAX_MESGS);

    // set up video manager, listen for Video, RSP, and RDP interrupts
    osCreateViManager(OS_PRIORITY_VIMGR);
    osViSetMode(&osViModeTable[mode]);
    osViBlack(TRUE);
    osSetEventMesg(OS_EVENT_SP, &sc->interruptQ, (OSMesg)RSP_DONE_MSG);
    osSetEventMesg(OS_EVENT_DP, &sc->interruptQ, (OSMesg)RDP_DONE_MSG);
    osSetEventMesg(OS_EVENT_PRENMI, &sc->interruptQ, (OSMesg)PRE_NMI_MSG);

    osViSetEvent(&sc->interruptQ, (OSMesg)VIDEO_MSG, numFields);

#ifdef SC_LOGGING
    osCreateLog(l, logArray, sizeof(logArray));
#endif

    osCreateThread(&sc->thread, THREAD_SCHED, __scMain, (void *)sc, stack, priority);
    osStartThread(&sc->thread);
}

// add a client to the scheduler
// clients receive messages at retrace time
void osScAddClient(OSSched *sc, OSScClient *c, OSMesgQueue *msgQ)
{
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);

    c->msgQ = msgQ;
    c->next = sc->clientList;
    sc->clientList = c;

    osSetIntMask(mask);
}

void osScRemoveClient(OSSched *sc, OSScClient *c)
{
    OSScClient *client = sc->clientList;
    OSScClient *prev   = 0;
    OSIntMask  mask;

    mask = osSetIntMask(OS_IM_NONE);

    while (client != 0) {
        if (client == c) {
	    if(prev)
		prev->next = c->next;
	    else
		sc->clientList = c->next;
            break;
        }
        prev   = client;
        client = client->next;
    }

    osSetIntMask(mask);
}

OSMesgQueue *osScGetCmdQ(OSSched *sc)
{
    return &sc->cmdQ;
}

OSMesgQueue *osScGetInterruptQ(OSSched *sc)
{
    return &sc->interruptQ;
}

// Scheduler implementation
/////////////////////////////////////////////////////////////////////////////

static void __scMain(void *arg)
{
	OSMesg		msg;
	OSSched		*sc = (OSSched *)arg;
	OSScClient	*client;
#ifdef SC_LOGGING
	static int	count = 0;
#endif

	while (TRUE)
	{
		osRecvMesg(&sc->interruptQ, &msg, OS_MESG_BLOCK);

#ifdef SC_LOGGING
		if (!(++count % 1024))
			osFlushLog(l);
#endif

		switch ((int) msg)
		{
			case (VIDEO_MSG):
				__scHandleRetrace(sc);
				break;

			case (RSP_DONE_MSG):
				__scHandleRSP(sc);
				break;

			case (RDP_DONE_MSG):
				__scHandleRDP(sc);
				break;

			case (PRE_NMI_MSG):
				// notify audio and graphics threads to fade out
				for (client=sc->clientList; client!=0; client=client->next)
				{
					osSendMesg(client->msgQ, (OSMesg) &sc->prenmiMsg,
								  OS_MESG_NOBLOCK);
				}
				break;

			case SCHED_NEWCMD_MSG:
				__scNewCmd(sc);
				break;
		}
	}
}

void scSendCommand(OSSched *sc, OSScTask *pTask)
{
#ifdef PLATFORM_PORT
	/* PORT: the scheduler thread (__scMain) never runs in the single-threaded cooperative
	 * model, so dispatch the task here. Gfx tasks go through osSpTaskStartGo -> Fast3D
	 * (os_shim/turok_gfx); audio tasks are handled by the software mixer (M4) and ignored.
	 * Then notify the task's client (the DONE message) so any wait on it completes. */
	(void)sc;
	if (pTask->list.t.type == M_GFXTASK)
	{
		osSpTaskLoad(&pTask->list);
		osSpTaskStartGo(&pTask->list);
		/* PORT: on N64 the scheduler's retrace handler (__scHandleRetrace) presents the
		 * finished gfx task via osViSwapBuffer. That scheduler thread never runs here, so
		 * without this the frame renders (gfx_run) but is NEVER presented — the main loop
		 * spins re-rendering, the render-frame counter/capture/frame-limit never advance.
		 * Present it now, just as the scheduler would. */
		osViSwapBuffer(pTask->framebuffer);
	}
	if (pTask->msgQ)
		osSendMesg(pTask->msgQ, pTask->msg, OS_MESG_NOBLOCK);
	return;
#else
	// add message to command queue
	osSendMesg(&sc->cmdQ, (OSMesg) pTask, OS_MESG_BLOCK);

	// message will be processed by __scMain as soon as scheduler thread is ready
	osSendMesg(&sc->interruptQ, (OSMesg*) (int) SCHED_NEWCMD_MSG, OS_MESG_BLOCK);
#endif
}

void __scNewCmd(OSSched *sc)
{
	OSScTask		*rspTask = NULL;	// initialization avoids silly warning
	OSScTask    *sp = NULL;
	OSScTask    *dp = NULL;
	u32			state;

	// read the task command queue and schedule tasks
	while (osRecvMesg(&sc->cmdQ, (OSMesg*) &rspTask, OS_MESG_NOBLOCK) != -1)
		__scAppendList(sc, rspTask);

	if (sc->doAudio && sc->curRSPTask)
	{
		// Preempt the running gfx task.  Note: if the RSP
		// component of the graphics task has finished, but the
		// RDP component is still running, we can start an audio
		// task which will freeze the RDP (and save the RDP cmd
		// FIFO) while the audio RSP code is running.
		__scYield(sc);
	}
	else
	{
		state = ((sc->curRSPTask == 0) << 1) | (sc->curRDPTask == 0);
		if (__scSchedule (sc, &sp, &dp, state) != state)
			__scExec(sc, sp, dp);
	}
}

static int dp_busy = 0;
static int dpCount = 0;

void __scHandleRetrace(OSSched *sc)
{
	OSScTask		*rspTask = NULL;	// initialization avoids silly warning
	OSScClient  *client;
	s32         state;
	OSScTask    *sp = 0;
	OSScTask    *dp = 0;
	CFrameData	*pFrameData;
	char buffer[50];
 
 	sc->frameCount++;

#ifdef SC_LOGGING
	osLogEvent(l, 500, 4, sc->frameCount, sc->curRSPTask, sc->curRDPTask);
#endif

	// read the task command queue and schedule tasks
	while (osRecvMesg(&sc->cmdQ, (OSMesg*) &rspTask, OS_MESG_NOBLOCK) != -1)
		__scAppendList(sc, rspTask);

	c_field++;

	ASSERT((n_pending == 0) || (n_pending == 1));

	if (n_pending)
	{
		c_field = 0;
		n_pending = 0;
		p_current_fb = p_pending_fb;
		p_pending_fb = NULL;
	}

	if (n_waiting)
	{
		pFrameData = p_waiting_frames[c_finished];
		if ((c_field + 1) >= pFrameData->m_nPredictFields)
		{
#ifdef SC_FIC
			rmonPrintf("retrace- actual:%d  predict:%d\n", c_field+1, pFrameData->m_nPredictFields);
#endif

			p_pending_fb = pFrameData->m_pFrameBuffer;
			osViSwapBuffer(pFrameData->m_pFrameBuffer);
			n_waiting--;
			n_pending++;

			if (++c_finished == 3)
				c_finished = 0;
		}
		else
		{
#ifdef SC_FIC
			rmonPrintf("retrace- delaying\n");
#endif
		}
	}
	else
	{
#ifdef SC_FIC
		rmonPrintf("retrace- none ready\n");
#endif
	}

	// recover from lockup
	if (!sc->curRSPTask && !sc->curRDPTask && sc->gfxListHead)
	{
		if (		(sc->gfxListHead->list.t.type == M_GFXTASK)
				&& (sc->gfxListHead->state == (OS_SC_YIELD | OS_SC_YIELDED | OS_SC_SP)) )
		{
			// task got yielded and completed at the same time
			// sp complete message got lost

			// prepare for scHandleRSP()
			sc->gfxListHead->state &= ~(OS_SC_YIELD | OS_SC_YIELDED);
			sc->curRSPTask = sc->gfxListHead;

			// remove from queue
			sc->gfxListHead = sc->gfxListHead->next;
			if (!sc->gfxListHead)
				sc->gfxListTail = NULL;

			__scHandleRSP(sc);
		}
	}


	// memory corruption?
	ASSERT(c_retrace < 160);

	if (++c_retrace == 160)
	{
		// RCP crash or infinite loop in game thread
#ifndef SHIP_IT
		trap_thread = THREAD_MAIN;

		if (sc->curRSPTask)
		{
			if (sc->curRSPTask->list.t.type == M_GFXTASK)
				igAssert(__FILE__, __LINE__, "TIMEOUT: RSP graphics task");
			else
				igAssert(__FILE__, __LINE__, "TIMEOUT: RSP audio task");
		}
		else
		{
			if (sc->curRDPTask)
			{
				if (sc->curRDPTask->list.t.type == M_GFXTASK)
					igAssert(__FILE__, __LINE__, "TIMEOUT: RDP graphics task");
				else
					igAssert(__FILE__, __LINE__, "TIMEOUT: RDP audio task");
			}
			else
			{
				if (sc->gfxListHead)
				{
					sprintf(buffer, "TIMEOUT: can't sched:0x%x\n", sc->gfxListHead->state);
					igAssert(__FILE__, __LINE__, buffer);
				}
				else
				{
					igAssert(__FILE__, __LINE__, "TIMEOUT: no tasks");
				}
			}
		}
#endif
		
		// reboot
		*((char*)(NULL)) = 0;
	}

	if (sc->doAudio && sc->curRSPTask)
	{
		// Preempt the running gfx task.  Note: if the RSP
		// component of the graphics task has finished, but the
		// RDP component is still running, we can start an audio
		// task which will freeze the RDP (and save the RDP cmd
		// FIFO) while the audio RSP code is running.
		__scYield(sc);
	}
	else
	{
		state = ((sc->curRSPTask == 0) << 1) | (sc->curRDPTask == 0);
		if (__scSchedule (sc, &sp, &dp, state) != state)
			__scExec(sc, sp, dp);
	}

	// notify audio and graphics threads to start building the command
	// lists for the next frame (client threads may choose not to
	// build the list in overrun case)
	for (client=sc->clientList; client!=0; client=client->next)
		osSendMesg(client->msgQ, (OSMesg) &sc->retraceMsg, OS_MESG_NOBLOCK);
}

// called when an RSP task signals that it has
// finished or yielded (at the hosts request)
void __scHandleRSP(OSSched *sc)
{
	OSScTask *t, *sp = 0, *dp = 0;
	s32 state, ret;

	ASSERT(sc->curRSPTask);

	t = sc->curRSPTask;
	sc->curRSPTask = 0;

#ifdef SC_LOGGING
	osLogEvent(l, 510, 3, t, t->state, t->flags);
#endif

	if ((t->state & OS_SC_YIELD) && osSpTaskYielded(&t->list))
	{
		t->state |= OS_SC_YIELDED;

		if ((t->flags & OS_SC_TYPE_MASK) == OS_SC_XBUS)
		{
			// push the task back on the list
			t->next = sc->gfxListHead;
			sc->gfxListHead = t;
			if (sc->gfxListTail == 0)
				sc->gfxListTail = t;
		}

#ifdef SC_LOGGING
		osLogEvent(l, 521, 1, t);
#endif

		ASSERT(t->list.t.type == M_GFXTASK);

		if (t->flags & OS_SC_LAST_TASK)
			this_rsp_graphics_time += ((float)(CYCLES_TO_NSEC(osGetTime() - rsp_graphics_start_time)))/1000000;

	}
	else
	{
		t->state &= ~OS_SC_NEEDS_RSP;

		if (t->list.t.type == M_GFXTASK)
		{
			c_retrace = 0;

			// RSP complete for graphics task
			if (t->flags & OS_SC_LAST_TASK)
			{
				this_rsp_graphics_time += ((float)(CYCLES_TO_NSEC(osGetTime() - rsp_graphics_start_time)))/1000000;
				rsp_graphics_time = this_rsp_graphics_time;
				rsp_total_time = ((float)(CYCLES_TO_NSEC(osGetTime() - rsp_start_time)))/1000000;

				ret = osSendMesg(t->msgQ, t->msg, OS_MESG_NOBLOCK);
				if (ret == -1)
				{
#ifndef SHIP_IT
					trap_thread = THREAD_MAIN;
					igAssert(__FILE__, __LINE__, "-- MQ FULL --");
#endif

					// reboot
					*((char*)(NULL)) = 0;
				}
			}
		}
		else
		{
			rsp_audio_time = ((float)(CYCLES_TO_NSEC(osGetTime() - rsp_audio_start_time)))/1000000;

			ret = osSendMesg(t->msgQ, t->msg, OS_MESG_NOBLOCK);
			if (ret == -1)
			{
#ifndef SHIP_IT
				trap_thread = THREAD_MAIN;
				igAssert(__FILE__, __LINE__, "-- MQ FULL --");
#endif

				// reboot
				*((char*)(NULL)) = 0;
			}
		}

		__scTaskComplete(sc, t);
	}

	state = ((sc->curRSPTask == 0) << 1) | (sc->curRDPTask == 0);
	if ( (__scSchedule (sc, &sp, &dp, state)) != state)
		__scExec(sc, sp, dp);
}

// called when an RDP task signals that it has finished
void __scHandleRDP(OSSched *sc)
{
    OSScTask *t, *sp = 0, *dp = 0;
    s32 state;
	 CFrameData	*pFrameData;
	 CEngineApp	*pApp;

    ASSERT(sc->curRDPTask);
    ASSERT(sc->curRDPTask->list.t.type == M_GFXTASK);

    t = sc->curRDPTask;

#ifdef SC_LOGGING
    osLogEvent(l, 515, 3, t, t->state, t->flags);
#endif

    t->state &= ~OS_SC_NEEDS_RDP;

	ASSERT(t->list.t.type == M_GFXTASK);
	if (t->list.t.type == M_GFXTASK)
	{
		if (t->flags & OS_SC_LAST_TASK)
			rdp_time = ((float)(CYCLES_TO_NSEC(osGetTime() - rdp_start_time)))/1000000;

		sc->curRDPTask = 0;

		if (t->flags & OS_SC_LAST_TASK)
		{
			pApp = GetApp();

			if (t->framebuffer == (void*) pApp->m_FrameData[0].m_pFrameBuffer)
				pFrameData = &pApp->m_FrameData[0];
			else if (t->framebuffer == (void*) pApp->m_FrameData[1].m_pFrameBuffer)
				pFrameData = &pApp->m_FrameData[1];
			else if (t->framebuffer == (void*) pApp->m_FrameData[2].m_pFrameBuffer)
				pFrameData = &pApp->m_FrameData[2];
			else
				ASSERT(FALSE);

			n_lll_fields = n_ll_fields;
			n_ll_fields = n_l_fields;
			n_l_fields = n_fields;
			n_fields = c_field + 1;
			if (((c_field + 1) >= pFrameData->m_nPredictFields) && !n_waiting && !n_pending)
			{
#ifdef SC_FIC
				rmonPrintf("rsp done- actual:%d  predict:%d\n", c_field+1, pFrameData->m_nPredictFields);
#endif

				p_pending_fb = pFrameData->m_pFrameBuffer;
				osViSwapBuffer(pFrameData->m_pFrameBuffer);
				n_pending++;
			}
			else
			{
#ifdef SC_FIC
				rmonPrintf("rsp done- wait\n");
#endif

				n_waiting++;

				p_waiting_frames[c_waiting] = pFrameData;
				if (++c_waiting == 3)
					c_waiting = 0;
			}

			CSunFrameData__RDPComplete(&pFrameData->m_SunFrame);
		}
	}

	__scTaskComplete(sc, t);


	state = ((sc->curRSPTask == 0) << 1) | (sc->curRDPTask == 0);
	if ( (__scSchedule (sc, &sp, &dp, state)) != state)
		__scExec(sc, sp, dp);
}

// don't run graphics task that will draw on current frame buffer
OSScTask *__scTaskReady(OSScTask *t)
{
	static int count = 0;

	if (t)
	{
		//if (		((t->framebuffer != osViGetCurrentFramebuffer()) && (t->framebuffer != osViGetNextFramebuffer()))
		if (		((t->framebuffer != p_current_fb) && (t->framebuffer != p_pending_fb))
				|| (count++ == 20) )
		{
			count = 0;
			return t;
		}
	}

	return NULL;
}

// checks to see if the task is complete (all RCP
// operations have been performed) and sends the done message to the
// client if it is.
s32 __scTaskComplete(OSSched *sc, OSScTask *t)
{
	if ((t->state & OS_SC_RCP_MASK) == 0)
	{
		// none of the needs bits set

		ASSERT (t->msgQ);

#ifdef SC_LOGGING
		osLogEvent(l, 504, 1, t);
#endif
		if (t->list.t.type == M_GFXTASK)
		{
			if ((t->flags & OS_SC_SWAPBUFFER) && (t->flags & OS_SC_LAST_TASK))
			{
				osViSwapBuffer(t->framebuffer);

#ifdef SC_LOGGING
				osLogEvent(l, 525, 1, t->framebuffer);
#endif
			}
		}

		return 1;
	}

	return 0;
}

// place task on either the audio or graphics queue
void __scAppendList(OSSched *sc, OSScTask *t)
{
	long type = t->list.t.type;

	ASSERT((type == M_AUDTASK) || (type == M_GFXTASK));

	if (type == M_AUDTASK)
	{
		if (sc->audioListTail)
			sc->audioListTail->next = t;
		else
			sc->audioListHead = t;

		sc->audioListTail = t;
		sc->doAudio = 1;
#ifdef SC_LOGGING
		osLogEvent(l, 506, 1, t);
#endif
	}
	else
	{
		if (sc->gfxListTail)
			sc->gfxListTail->next = t;
		else
			sc->gfxListHead = t;

		sc->gfxListTail = t;
#ifdef SC_LOGGING
		osLogEvent(l, 507, 1, t);
#endif
	}

	t->next = NULL;
	t->state = t->flags & OS_SC_RCP_MASK;
}

void __scExec(OSSched *sc, OSScTask *sp, OSScTask *dp)
{
	int rv;

#ifdef SC_LOGGING
	osLogEvent(l, 511, 2, sp, dp);
#endif

	ASSERT(sc->curRSPTask == 0);

	if (dp)
		if (dp->flags & OS_SC_LAST_TASK)
			if (!(dp->state & OS_SC_YIELD))
				rdp_start_time = osGetTime();

	if (sp)
	{
		if (sp->list.t.type == M_AUDTASK)
		{
			osWritebackDCacheAll();  // flush the cache

			rsp_audio_start_time = osGetTime();
		}
		else
		{
			if (sp->flags & OS_SC_LAST_TASK)
			{
				rsp_graphics_start_time = osGetTime();

				if (!(sp->state & OS_SC_YIELD))
				{
					rsp_start_time = rsp_graphics_start_time;
					this_rsp_graphics_time = 0;
				}
			}
		}

		sp->state &= ~(OS_SC_YIELD | OS_SC_YIELDED);
		osSpTaskLoad(&sp->list);
		osSpTaskStartGo(&sp->list);
		sc->curRSPTask = sp;

		if (sp == dp)
			sc->curRDPTask = dp;
	}

	if (dp && (dp != sp))
	{
		ASSERT(dp->list.t.output_buff);

#ifdef SC_LOGGING
		osLogEvent(l, 523, 3, dp, dp->list.t.output_buff,
					  (u32)*dp->list.t.output_buff_size);
#endif
		rv = osDpSetNextBuffer(dp->list.t.output_buff,
									  *dp->list.t.output_buff_size);

		dp_busy = 1;
		dpCount = 0;

		ASSERT(rv == 0);

		sc->curRDPTask = dp;
	}
}

static void __scYield(OSSched *sc)
{
#ifdef SC_LOGGING
	osLogEvent(l, 503, 1, sc->curRSPTask);
#endif

	if (sc->curRSPTask->list.t.type == M_GFXTASK)
	{
		sc->curRSPTask->state |= OS_SC_YIELD;

		osSpTaskYield();
	}
	else
	{
#ifdef SC_LOGGING
		osLogEvent(l, 508, 1, sc->curRSPTask);
#endif
	}
}

// schedules the tasks to be run on the RCP
s32 __scSchedule(OSSched *sc, OSScTask **sp, OSScTask **dp, s32 availRCP)
{
	s32 avail = availRCP;
	OSScTask *gfx = sc->gfxListHead;
	OSScTask *audio = sc->audioListHead;

#ifdef SC_LOGGING
	osLogEvent(l, 517, 3, *sp, *dp, availRCP);
#endif

	if (sc->doAudio && (avail & OS_SC_SP))
	{
		if (gfx && (gfx->flags & OS_SC_PARALLEL_TASK))
		{
			*sp = gfx;
			avail &= ~OS_SC_SP;
		}
		else
		{
			*sp = audio;
			avail &= ~OS_SC_SP;
			sc->doAudio = 0;
			sc->audioListHead = sc->audioListHead->next;
			if (sc->audioListHead == NULL)
				sc->audioListTail = NULL;
		}
	}
	else
	{
#ifdef SC_LOGGING
		osLogEvent(l, 520, 1, gfx);
#endif
		if (__scTaskReady(gfx))
		{
#ifdef SC_LOGGING
			osLogEvent(l, 522, 3, gfx, gfx->state, gfx->flags);
#endif
			switch (gfx->flags & OS_SC_TYPE_MASK)
			{
				case (OS_SC_XBUS):
					if (gfx->state & OS_SC_YIELDED)
					{
#ifdef SC_LOGGING
						osLogEvent(l, 518, 0);
#endif
						// can hit this if RDP finishes at yield req
						// ASSERT(gfx->state & OS_SC_DP);

						if (avail & OS_SC_SP)
						{
							// if SP is available
#ifdef SC_LOGGING
							osLogEvent(l, 519, 0);
#endif
							*sp = gfx;
							avail &= ~OS_SC_SP;

							if (gfx->state & OS_SC_DP)
							{
								// if it needs DP */
								*dp = gfx;
								avail &= ~OS_SC_DP;

								if (avail & OS_SC_DP == 0)
									ASSERT(sc->curRDPTask == gfx);
							}

							sc->gfxListHead = sc->gfxListHead->next;
							if (sc->gfxListHead == NULL)
								sc->gfxListTail = NULL;
						}
					}
					else
					{
						if (avail == (OS_SC_SP | OS_SC_DP))
						{
							*sp = *dp = gfx;
							avail &= ~(OS_SC_SP | OS_SC_DP);
							sc->gfxListHead = sc->gfxListHead->next;
							if (sc->gfxListHead == NULL)
								sc->gfxListTail = NULL;
						}
               }
               break;

				case (OS_SC_DRAM):
				case (OS_SC_DP_DRAM):
				case (OS_SC_DP_XBUS):
					if (gfx->state & OS_SC_SP)
					{
						// if needs SP
						if (avail & OS_SC_SP)
						{
							// if SP is available
							*sp = gfx;
							avail &= ~OS_SC_SP;
						}
					}
					else if (gfx->state & OS_SC_DP)
					{
						// if needs DP
						if (avail & OS_SC_DP)
						{
							// if DP available
							*dp = gfx;
							avail &= ~OS_SC_DP;
							sc->gfxListHead = sc->gfxListHead->next;
							if (sc->gfxListHead == NULL)
								sc->gfxListTail = NULL;
						}
					}
					break;

				case (OS_SC_SP_DRAM):
				case (OS_SC_SP_XBUS):
				default:
					break;
			}
		}
	}

	if (avail != availRCP)
		avail = __scSchedule(sc, sp, dp, avail);

	return avail;
}

