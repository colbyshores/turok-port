// cammodes.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _INC_CAMMODES
#define _INC_CAMMODES


/////////////////////////////////////////////////////////////////////////////
// Modes:
// NOTE: Make sure mode table is in sync with these or you'll be trouble!
/////////////////////////////////////////////////////////////////////////////
enum CameraModes
{
	CAMERA_LEGALSCREEN_MODE,						// looking at the TUROK logo
	CAMERA_INTRO_ACCLAIM_LOGO_MODE,				// The cheap'n'nasty acclaim logo
	CAMERA_INTRO_IGGY_MODE,							// Watching iggy intro
	CAMERA_INTRO_TUROK_DRAWING_BOW_MODE,		// Circling turok firing bow
	CAMERA_INTRO_ZOOM_TO_LOGO_MODE,				// Zooming into turok logo

	CAMERA_GAME_INTRO_MODE,							// Telling the player what to do
	CAMERA_GAME_INTRO_KEY_MODE,  					// Showing the player a key

	CAMERA_TUROK_EYE_MODE,							// View from Turok's eyes

	CAMERA_CINEMA_TUROK_DEATH_MODE,				// Tracking turok's collapsing death
	CAMERA_CINEMA_TUROK_FALL_DEATH_MODE,		// Tracking turok's falling death
	CAMERA_CINEMA_TUROK_WATER_DEATH_MODE,		// Tracking turok's death under water
	CAMERA_CINEMA_TUROK_RESURRECT_MODE,			// Comming back to life

	CAMERA_GALLERY_IDLE_MODE,						// Waiting for joypad input

	CAMERA_CINEMA_LONGHUNTER_MODE,				// Longhunter summoning humvee
	CAMERA_CINEMA_FIRST_HUMVEE_MODE,				// Humvee jumping in
	CAMERA_CINEMA_FINAL_HUMVEE_MODE,				// The final humvee jumping in!

	CAMERA_CINEMA_TUROK_KILL_MANTIS_MODE,		// Turok slays the mantis
	CAMERA_CINEMA_MANTIS_KILL_TUROK_MODE,		// Mantis slays turok
	CAMERA_CINEMA_TUROK_KILL_TREX_MODE,			// Turok slays the trex
	CAMERA_CINEMA_TREX_KILL_TUROK_MODE,			// Trex slays turok
	CAMERA_CINEMA_TUROK_KILL_LONGHUNTER_MODE,	// Turok slays the longhunter
	CAMERA_CINEMA_TUROK_KILL_CAMPAIGNER_MODE,	// Turok slays the campaigner
	CAMERA_CINEMA_CAMPAIGNER_KILL_TUROK_MODE,	// Campaigner slays turok

	CAMERA_CINEMA_TUROK_PICKUP_KEY_MODE, 		// Turok picking up a key

	CAMERA_CINEMA_END_SEQUENCE_MODE, 			// Turok escaping from campainer
	CAMERA_CINEMA_END_SEQUENCE2_MODE, 			// Turok in extra bit, running away

	CAMERA_CINEMA_CREDIT_MODE,						// End credits camera mode

	CAMERA_CINEMA_TUROK_VICTORY_MODE,			// Victory dance after killing a boss

	CAMERA_END_MODE
} ;



/////////////////////////////////////////////////////////////////////////////
// Flags
/////////////////////////////////////////////////////////////////////////////
#define CF_TUROK_LOADED		(1<<0)
#define CF_TUROK_IN_WATER	(1<<1)
#define CF_SOUND_PLAYED		(1<<2)


/////////////////////////////////////////////////////////////////////////////
#endif // _INC_CAMMODES

