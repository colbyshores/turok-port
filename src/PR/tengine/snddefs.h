// snddefs.h

#ifndef _INC_SNDDEFS
#define _INC_SNDDEFS

/////////////////////////////////////////////////////////////////////////////

// m_dwFlags
#define SOUNDEFFECT_LOOP			(1 << 0)
#define PITCH_ENV_ENABLED			(1 << 1)
#define VOL_ENV_ENABLED				(1 << 2)
#define CH_FLAG_STARTED				(1 << 3)
#define CH_FLAG_DELAYED_START		(1 << 4)
#define CH_FLAG_ENV_UPDATE			(1 << 5)
#define CH_SAVER						(1 << 6)
#define CH_FLAG_ENV_FADEOUT		(1 << 7)
#define CH_FLAG_RAND_PITCH			(1 << 8)
#define FADE_IF_OBSTRUCTED			(1 << 9)
#define UPDATE_NONENV				(1 << 10)
#define ALWAYS_MAXVOL				(1 << 11)
#define LINK_TO_ENV					(1 << 12)

/////////////////////////////////////////////////////////////////////////////
// radio volume defines
#define	VOLUME_QUIET				0
#define	VOLUME_NORMAL				1
#define	VOLUME_LOUD					2



/////////////////////////////////////////////////////////////////////////////
#endif // _INC_SNDDEFS
