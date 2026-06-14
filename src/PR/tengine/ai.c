// ai.cpp
//

#include "stdafx.h"
#include "debug.h"

#ifdef IG_DEBUG
#define AI_DECLARE_EVENT_STRINGS
#endif

#include "ai.h"
#include "aistand.h"
#include "romstruc.h"
#include "tengine.h"
#include "scaling.h"
#include "audio.h"
#include "audiocfx.h"
#include "sfx.h"
#include "tmove.h"
#include "tmove.h"
#include "aiweap.h"
#include "regtype.h"
#include "aidoor.h"
#include "mantis.h"
#include "trex.h"
#include "campaign.h"
#include "humvee.h"
#include "simppool.h"
#include "boss.h"
#include "fx.h"
#include "wallcoll.h"
#include "cammodes.h"

#define	DEBUG_AI		0

// if defined uses turn animations in sniping
// if not defined sniper will turn on spot
#define	ALLOW_SNIPER_TURNING

#define AI_IsValid(pThis) ((pThis)->ah.ih.m_Type == I_ANIMATED)
void AI_AssistSniperTurn(CGameObjectInstance *pAI);


BOOL AICollidedWithWall ;


void AI_SEvent_GenerateBouncy(CInstanceHdr *pThis, CVector3 vPos, int nType) ;

/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// Zig zag defines
#define CGameObjectInstance__MagicNumber(pThis)		(((INT32)pThis) * 2823445331)

#define ZIG_ZAG_SPEED				ANGLE_DTOR(15)			// Speed of sine
#define ZIG_ZAG_SCALE_FACTOR		2							// Makes it's bigger or smaller
#define MAX_ZIG_ZAG_RADIUS			(80*SCALING_FACTOR)	// Max dist to side of player
#define ZIG_ZAG_CUT_OFF_DIST		(15*SCALING_FACTOR)	// Dist from target when zig zag stops




/////////////////////////////////////////////////////////////////////////////

BOOL IsExplosionEvent(int Event)
{
	switch (Event)
	{
		case AI_MEVENT_EXPLOSION:
		case AI_SEVENT_EXPLODE:
		case AI_MEVENT_ENEMYEXPLOSION10:
		case AI_MEVENT_ENEMYEXPLOSION15:
		case AI_MEVENT_ENEMYEXPLOSION20:
		case AI_MEVENT_EXPLOSION20:
		case AI_MEVENT_EXPLOSION30:
		case AI_MEVENT_EXPLOSION50:
		case AI_MEVENT_EXPLOSION100:
		case AI_MEVENT_EXPLOSIONSPECIAL:
		case AI_MEVENT_SMALLEXPLOSION30:
		case AI_MEVENT_SHOTGUNEXPLOSION:
		case AI_MEVENT_TEKBOWEXPLOSION:
			return TRUE;

		default:
			return FALSE;
	}
}

BOOL IsSingleEvent(int Event)
{
	switch (Event)
	{
		// list system events here (events that are processed by the sender)
		case AI_SEVENT_ACIDINCOMING:
		case AI_SEVENT_ACIDSPITPART:
		case AI_SEVENT_ACIDSPITSND:
		case AI_SEVENT_BLOWGUNFIRE:
		case AI_SEVENT_BREATHPART:
		case AI_SEVENT_BREATHSND:
		case AI_SEVENT_CHARGEHEADSLAM:
		case AI_SEVENT_DEATHRAINPART:
		case AI_SEVENT_DEATHRAINSND:
		case AI_SEVENT_EARTHQUAKE:
		case AI_SEVENT_ENERGYBLASTPART:
		case AI_SEVENT_ENERGYBLASTSND:
		case AI_SEVENT_EXPLOSIVELAUNCHED:
		case AI_SEVENT_EYEFIREPART:
		case AI_SEVENT_EYEFIRESND:
		case AI_SEVENT_FIREBALLPART:
		case AI_SEVENT_FIREBALLSND:
		case AI_SEVENT_JAWSSNAP:
		case AI_SEVENT_SHOCKWAVEPART:
		case AI_SEVENT_SHOCKWAVESND:
		case AI_SEVENT_SPELLCASTPART:
		case AI_SEVENT_SPELLCASTSND:
		case AI_SEVENT_SWINGFISTSWEAPON:
		case AI_SEVENT_WEAPONFIRE:
		case AI_SEVENT_WEAPONFLASH:
		case AI_SEVENT_WEAPONROUND:
		case AI_SEVENT_WEAPONCOCK:
		case AI_SEVENT_LOOKOFFSET:
		case AI_SEVENT_TEST:
		case AI_SEVENT_SURFACEIMPACT:
		case AI_SEVENT_CHITTER:
		case AI_SEVENT_ALERT:
		case AI_SEVENT_HYDRAULICS:
		case AI_SEVENT_SHUTDOWN:
		case AI_SEVENT_SWIVEL:
		case AI_SEVENT_JETPACKOUTOFCTRL:
		case AI_SEVENT_BURSTFROMGROUND:
		case AI_SEVENT_CHESTTHUMP:
		case AI_SEVENT_DEATH:
		case AI_SEVENT_EXPLODE:
		case AI_SEVENT_FOOTFALL:
		case AI_SEVENT_GRUNT:
		case AI_SEVENT_GURGLE:
		case AI_SEVENT_HANDFALL:
		case AI_SEVENT_HISS:
		case AI_SEVENT_HITGROUND:
		case AI_SEVENT_LICKING:
		case AI_SEVENT_RAGE:
		case AI_SEVENT_ROCKNOISE1:
		case AI_SEVENT_RUBBINGSKIN:
		case AI_SEVENT_SCRATCH:
		case AI_SEVENT_SCREECH:
		case AI_SEVENT_SNIFF:
		case AI_SEVENT_SWOOSH:
		case AI_SEVENT_TURNTOSTONE:
		case AI_SEVENT_VIOLENTDEATH:
		case AI_SEVENT_WINGFLAP:
		case AI_SEVENT_GROWL:
		case AI_SEVENT_ALERT1:
		case AI_SEVENT_ALERT2:
		case AI_SEVENT_ALERT3:
		case AI_SEVENT_ALERT4:
		case AI_SEVENT_DEATH1:
		case AI_SEVENT_DEATH2:
		case AI_SEVENT_DEATH3:
		case AI_SEVENT_DEATH4:
		case AI_SEVENT_INJURY1:
		case AI_SEVENT_INJURY2:
		case AI_SEVENT_INJURY3:
		case AI_SEVENT_LONG1:
		case AI_SEVENT_LONG2:
		case AI_SEVENT_LONG3:
		case AI_SEVENT_TAUNT1:
		case AI_SEVENT_TAUNT2:
		case AI_SEVENT_TAUNT3:
		case AI_SEVENT_VIOLENTDEATH1:
		case AI_SEVENT_VIOLENTDEATH2:
		case AI_SEVENT_SCREENSHAKE:
		case AI_SEVENT_SCREENTREMOR:
		case AI_SEVENT_INVINCIBLE:
		case AI_SEVENT_BLOODARCPARTICLES:
		case AI_SEVENT_MUZZLEFLASH:
		case AI_SEVENT_HULK_BLASTER:
		case AI_SEVENT_ALIEN_PROJECTILE:
		case AI_SEVENT_KNIFEFORWARD:
		case AI_SEVENT_KNIFELEFT:
		case AI_SEVENT_KNIFERIGHT:
		case AI_SEVENT_GREENBLOODARCPARTICLES:
		case AI_SEVENT_SPURT_BLOODPARTICLES:
		case AI_SEVENT_FOUNTAIN_BLOODPARTICLES:
		case AI_SEVENT_GUSH_BLOODPARTICLES:
		case AI_SEVENT_DROOL_PARTICLES:
		case AI_SEVENT_ARTERIAL_BLOODPARTICLES:
		case AI_SEVENT_SPURT_GBLOODPARTICLES:
		case AI_SEVENT_FOUNTAIN_GBLOODPARTICLES:
		case AI_SEVENT_GUSH_GBLOODPARTICLES:
		case AI_SEVENT_ARTERIAL_GBLOODPARTICLES:
		case AI_SEVENT_WATER_DROP:
		case AI_SEVENT_WATER_FOAM:
		case AI_SEVENT_WATER_STEAM:
		case AI_SEVENT_ARROW_DAMAGE:
		case AI_SEVENT_DAMAGETARGET:
		case AI_SEVENT_DAMAGENUDGETARGET:
		case AI_SEVENT_GENERIC1:
		case AI_SEVENT_GENERIC2:
		case AI_SEVENT_GENERIC3:
		case AI_SEVENT_GENERIC4:
		case AI_SEVENT_GENERIC5:
		case AI_SEVENT_GENERIC6:
		case AI_SEVENT_GENERIC7:
		case AI_SEVENT_GENERIC8:
		case AI_SEVENT_GENERIC9:
		case AI_SEVENT_GENERIC10:
		case AI_SEVENT_GENERIC11:
		case AI_SEVENT_GENERIC12:
		case AI_SEVENT_GENERIC13:
		case AI_SEVENT_GENERIC14:
		case AI_SEVENT_GENERIC15:
		case AI_SEVENT_GENERIC16:
		case AI_SEVENT_GENERIC17:
		case AI_SEVENT_GENERIC18:
		case AI_SEVENT_GENERIC19:
		case AI_SEVENT_GENERIC20:
		case AI_SEVENT_GENERIC21:
		case AI_SEVENT_GENERIC22:
		case AI_SEVENT_GENERIC23:
		case AI_SEVENT_GENERIC24:
		case AI_SEVENT_GENERIC25:
		case AI_SEVENT_GENERIC26:
		case AI_SEVENT_GENERIC27:
		case AI_SEVENT_GENERIC28:
		case AI_SEVENT_GENERIC29:
		case AI_SEVENT_GENERIC30:
		case AI_SEVENT_GENERIC31:
		case AI_SEVENT_GENERIC32:
		case AI_SEVENT_GENERIC33:
		case AI_SEVENT_GENERIC34:
		case AI_SEVENT_GENERIC35:
		case AI_SEVENT_GENERIC36:
		case AI_SEVENT_GENERIC37:
		case AI_SEVENT_GENERIC38:
		case AI_SEVENT_GENERIC39:
		case AI_SEVENT_GENERIC40:
		case AI_SEVENT_GENERIC41:
		case AI_SEVENT_GENERIC42:
		case AI_SEVENT_GENERIC43:
		case AI_SEVENT_GENERIC44:
		case AI_SEVENT_GENERIC45:
		case AI_SEVENT_GENERIC46:
		case AI_SEVENT_GENERIC47:
		case AI_SEVENT_GENERIC48:
		case AI_SEVENT_GENERIC49:
		case AI_SEVENT_GENERIC50:
		case AI_SEVENT_GENERIC51:
		case AI_SEVENT_GENERIC52:
		case AI_SEVENT_GENERIC53:
		case AI_SEVENT_GENERIC54:
		case AI_SEVENT_GENERIC55:
		case AI_SEVENT_GENERIC56:
		case AI_SEVENT_GENERIC57:
		case AI_SEVENT_GENERIC58:
		case AI_SEVENT_GENERIC59:
		case AI_SEVENT_GENERIC60:
		case AI_SEVENT_GENERIC61:
		case AI_SEVENT_GENERIC62:
		case AI_SEVENT_GENERIC63:
		case AI_SEVENT_GENERIC64:
		case AI_SEVENT_GENERIC65:
		case AI_SEVENT_GENERIC66:
		case AI_SEVENT_GENERIC67:
		case AI_SEVENT_GENERIC68:
		case AI_SEVENT_GENERIC69:
		case AI_SEVENT_GENERIC70:
		case AI_SEVENT_GENERIC71:
		case AI_SEVENT_GENERIC72:
		case AI_SEVENT_GENERIC73:
		case AI_SEVENT_GENERIC74:
		case AI_SEVENT_GENERIC75:
		case AI_SEVENT_GENERIC76:
		case AI_SEVENT_GENERIC77:
		case AI_SEVENT_GENERIC78:
		case AI_SEVENT_GENERIC79:
		case AI_SEVENT_GENERIC80:
		case AI_SEVENT_GENERIC81:
		case AI_SEVENT_GENERIC82:
		case AI_SEVENT_GENERIC83:
		case AI_SEVENT_GENERIC84:
		case AI_SEVENT_GENERIC85:
		case AI_SEVENT_GENERIC86:
		case AI_SEVENT_GENERIC87:
		case AI_SEVENT_GENERIC88:
		case AI_SEVENT_GENERIC89:
		case AI_SEVENT_GENERIC90:
		case AI_SEVENT_GENERIC91:
		case AI_SEVENT_GENERIC92:
		case AI_SEVENT_GENERIC93:
		case AI_SEVENT_GENERIC94:
		case AI_SEVENT_GENERIC95:
		case AI_SEVENT_GENERIC96:
		case AI_SEVENT_GENERIC97:
		case AI_SEVENT_GENERIC98:
		case AI_SEVENT_GENERIC99:
		case AI_SEVENT_GENERIC100:
		case AI_SEVENT_GENERIC101:
		case AI_SEVENT_GENERIC102:
		case AI_SEVENT_GENERIC103:
		case AI_SEVENT_GENERIC104:
		case AI_SEVENT_GENERIC105:
		case AI_SEVENT_GENERIC106:
		case AI_SEVENT_GENERIC107:
		case AI_SEVENT_GENERIC108:
		case AI_SEVENT_GENERIC109:
		case AI_SEVENT_GENERIC110:
		case AI_SEVENT_GENERIC111:
		case AI_SEVENT_GENERIC112:
		case AI_SEVENT_GENERIC113:
		case AI_SEVENT_GENERIC114:
		case AI_SEVENT_GENERIC115:
		case AI_SEVENT_GENERIC116:
		case AI_SEVENT_GENERIC117:
		case AI_SEVENT_GENERIC118:
		case AI_SEVENT_GENERIC119:
		case AI_SEVENT_GENERIC120:
		case AI_SEVENT_GENERIC121:
		case AI_SEVENT_GENERIC122:
		case AI_SEVENT_GENERIC123:
		case AI_SEVENT_GENERIC124:
		case AI_SEVENT_GENERIC125:
		case AI_SEVENT_GENERIC126:
		case AI_SEVENT_GENERIC127:
		case AI_SEVENT_GENERIC128:
		case AI_SEVENT_GENERIC129:
		case AI_SEVENT_GENERIC130:
		case AI_SEVENT_GENERIC131:
		case AI_SEVENT_GENERIC132:
		case AI_SEVENT_GENERIC133:
		case AI_SEVENT_GENERIC134:
		case AI_SEVENT_GENERIC135:
		case AI_SEVENT_GENERIC136:
		case AI_SEVENT_GENERIC137:
		case AI_SEVENT_GENERIC138:
		case AI_SEVENT_GENERIC139:
		case AI_SEVENT_GENERIC140:
		case AI_SEVENT_GENERIC141:
		case AI_SEVENT_GENERIC142:
		case AI_SEVENT_GENERIC143:
		case AI_SEVENT_GENERIC144:
		case AI_SEVENT_GENERIC145:
		case AI_SEVENT_GENERIC146:
		case AI_SEVENT_GENERIC147:
		case AI_SEVENT_GENERIC148:
		case AI_SEVENT_GENERIC149:
		case AI_SEVENT_GENERIC150:
		case AI_SEVENT_GENERIC151:
		case AI_SEVENT_GENERIC152:
		case AI_SEVENT_GENERIC153:
		case AI_SEVENT_GENERIC154:
		case AI_SEVENT_GENERIC155:
		case AI_SEVENT_GENERIC156:
		case AI_SEVENT_GENERIC157:
		case AI_SEVENT_GENERIC158:
		case AI_SEVENT_GENERIC159:
		case AI_SEVENT_GENERIC160:
		case AI_SEVENT_GENERIC161:
		case AI_SEVENT_GENERIC162:
		case AI_SEVENT_GENERIC163:
		case AI_SEVENT_GENERIC164:
		case AI_SEVENT_GENERIC165:
		case AI_SEVENT_GENERIC166:
		case AI_SEVENT_GENERIC167:
		case AI_SEVENT_GENERIC168:
		case AI_SEVENT_GENERIC169:
		case AI_SEVENT_GENERIC170:
		case AI_SEVENT_GENERIC171:
		case AI_SEVENT_GENERIC172:
		case AI_SEVENT_GENERIC173:
		case AI_SEVENT_GENERIC174:
		case AI_SEVENT_GENERIC175:
		case AI_SEVENT_GENERIC176:
		case AI_SEVENT_GENERIC177:
		case AI_SEVENT_GENERIC178:
		case AI_SEVENT_GENERIC179:
		case AI_SEVENT_GENERIC180:
		case AI_SEVENT_GENERIC181:
		case AI_SEVENT_GENERIC182:
		case AI_SEVENT_GENERIC183:
		case AI_SEVENT_GENERIC184:
		case AI_SEVENT_GENERIC185:
		case AI_SEVENT_GENERIC186:
		case AI_SEVENT_GENERIC187:
		case AI_SEVENT_GENERIC188:
		case AI_SEVENT_GENERIC189:
		case AI_SEVENT_GENERIC190:
		case AI_SEVENT_GENERIC191:
		case AI_SEVENT_GENERIC192:
		case AI_SEVENT_GENERIC193:
		case AI_SEVENT_GENERIC194:
		case AI_SEVENT_GENERIC195:
		case AI_SEVENT_GENERIC196:
		case AI_SEVENT_GENERIC197:
		case AI_SEVENT_GENERIC198:
		case AI_SEVENT_GENERIC199:
		case AI_SEVENT_GENERIC200:
		case AI_SEVENT_GENERIC201:
		case AI_SEVENT_GENERIC202:
		case AI_SEVENT_GENERIC203:
		case AI_SEVENT_GENERIC204:
		case AI_SEVENT_GENERIC205:
		case AI_SEVENT_GENERIC206:
		case AI_SEVENT_GENERIC207:
		case AI_SEVENT_GENERIC208:
		case AI_SEVENT_GENERIC209:
		case AI_SEVENT_GENERIC210:
		case AI_SEVENT_GENERIC211:
		case AI_SEVENT_GENERIC212:
		case AI_SEVENT_GENERIC213:
		case AI_SEVENT_GENERIC214:
		case AI_SEVENT_GENERIC215:
		case AI_SEVENT_GENERIC216:
		case AI_SEVENT_GENERIC217:
		case AI_SEVENT_GENERIC218:
		case AI_SEVENT_GENERIC219:
		case AI_SEVENT_GENERIC220:
		case AI_SEVENT_GENERIC221:
		case AI_SEVENT_GENERIC222:
		case AI_SEVENT_GENERIC223:
		case AI_SEVENT_GENERIC224:
		case AI_SEVENT_GENERIC225:
		case AI_SEVENT_GENERIC226:
		case AI_SEVENT_GENERIC227:
		case AI_SEVENT_GENERIC228:
		case AI_SEVENT_GENERIC229:
		case AI_SEVENT_GENERIC230:
		case AI_SEVENT_GENERIC231:
		case AI_SEVENT_GENERIC232:
		case AI_SEVENT_GENERIC233:
		case AI_SEVENT_GENERIC234:
		case AI_SEVENT_GENERIC235:
		case AI_SEVENT_GENERIC236:
		case AI_SEVENT_GENERIC237:
		case AI_SEVENT_GENERIC238:
		case AI_SEVENT_GENERIC239:
		case AI_SEVENT_GENERIC240:
		case AI_SEVENT_GENERIC241:
		case AI_SEVENT_GENERIC242:
		case AI_SEVENT_GENERIC243:
		case AI_SEVENT_GENERIC244:
		case AI_SEVENT_GENERIC245:
		case AI_SEVENT_GENERIC246:
		case AI_SEVENT_GENERIC247:
		case AI_SEVENT_GENERIC248:
		case AI_SEVENT_GENERIC249:
		case AI_SEVENT_GENERIC250:
		case AI_SEVENT_GENERIC251:
		case AI_SEVENT_GENERIC252:
		case AI_SEVENT_GENERIC253:
		case AI_SEVENT_GENERIC254:
		case AI_SEVENT_GENERIC255:
		case AI_SEVENT_GENERIC256:
		case AI_SEVENT_GENERIC257:
		case AI_SEVENT_GENERIC258:
		case AI_SEVENT_GENERIC259:
		case AI_SEVENT_GENERIC260:
		case AI_SEVENT_PLASMA2_PARTICLE:
		case AI_SEVENT_WATER_BUBBLE:
		case AI_SEVENT_WATER_RIPPLE:
		case AI_SEVENT_WATER_SPLASH:
		case AI_SEVENT_DOOR_ALLOWENTRY:
		case AI_SEVENT_DOOR_BLOCKENTRY:
		case AI_SEVENT_GENERATE_PICKUPS:
		case AI_SEVENT_MAKE_INVISIBLE:
		case AI_SEVENT_GENERAL_SOUND:
		case AI_SEVENT_SWOOSH_ALL_HANDS:
		case AI_SEVENT_SWOOSH_LEFT_HAND:
		case AI_SEVENT_SWOOSH_RIGHT_HAND:
		case AI_SEVENT_SWOOSH_ALL_FEET:
		case AI_SEVENT_SWOOSH_LEFT_FOOT:
		case AI_SEVENT_SWOOSH_RIGHT_FOOT:
		case AI_SEVENT_SWOOSH_WEAPON1:
		case AI_SEVENT_SWOOSH_WEAPON2:
		case AI_SEVENT_SWOOSH_TAIL:
		case AI_SEVENT_SWOOSH_LEFT_TOES:
		case AI_SEVENT_SWOOSH_RIGHT_TOES:
		case AI_SEVENT_NEXT_CINEMATIC_STAGE:
		case AI_SEVENT_GENERATE_PICKUP:
		case AI_SEVENT_BOUNCY_ROCK:
		case AI_SEVENT_BOUNCY_ALIEN_HEAD:
		case AI_SEVENT_BOUNCY_ALIEN_HAND:
		case AI_SEVENT_BOUNCY_ALIEN_FOOT:
		case AI_SEVENT_BOUNCY_ALIEN_TORSO:
		case AI_SEVENT_BOUNCY_LEAPER_HEAD:
		case AI_SEVENT_BOUNCY_LEAPER_HAND:
		case AI_SEVENT_BOUNCY_LEAPER_FOOT:
		case AI_SEVENT_BOUNCY_LEAPER_TORSO:
		case AI_SEVENT_INSTANTDEATH:
		case AI_SEVENT_RANDOM_BIG_SWOOSH_SOUND:
		case AI_SEVENT_RANDOM_SMALL_SWOOSH_SOUND:
		case AI_SEVENT_RANDOM_GRUNT_SOUND:
		case AI_SEVENT_START_FIRE_SOUND:
		case AI_SEVENT_STOP_FIRE_SOUND:

			return TRUE;

		// multi events default to false (events that are sent to all other ai's)
		default:
			return FALSE;
	}
}

void AI_Single_Event(CInstanceHdr *pThis, CInstanceHdr *pOrigin, int Event, CVector3 vPos, float Value)
{
	switch (Event)
	{
		case AI_SEVENT_ACIDINCOMING:
			AI_SEvent_AcidIncoming ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ACIDSPITPART:
			AI_SEvent_AcidSpitPart ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ACIDSPITSND:
			AI_SEvent_AcidSpitSnd ( pThis, vPos, Value );
			break;

		case AI_SEVENT_BLOWGUNFIRE:
			AI_SEvent_BlowGunFire ( pThis, vPos, Value );
			break;

		case AI_SEVENT_BREATHPART:
			AI_SEvent_BreathPart ( pThis, vPos, Value );
			break;

		case AI_SEVENT_BREATHSND:
			AI_SEvent_BreathSnd ( pThis, vPos, Value );
			break;

		case AI_SEVENT_CHARGEHEADSLAM:
			AI_SEvent_ChargeHeadSlam ( pThis, vPos, Value );
			break;

		case AI_SEVENT_DEATHRAINPART:
			AI_SEvent_DeathRainPart ( pThis, vPos, Value );
			break;

		case AI_SEVENT_DEATHRAINSND:
			AI_SEvent_DeathRainSnd ( pThis, vPos, Value );
			break;

		case AI_SEVENT_EARTHQUAKE:
			AI_SEvent_Earthquake ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ENERGYBLASTPART:
			AI_SEvent_EnergyBlastPart ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ENERGYBLASTSND:
			AI_SEvent_EnergyBlastSnd ( pThis, vPos, Value );
			break;

		case AI_SEVENT_EXPLOSIVELAUNCHED:
			AI_SEvent_ExplosiveLaunched ( pThis, vPos, Value );
			break;

		case AI_SEVENT_EYEFIREPART:
			AI_SEvent_EyeFirePart ( pThis, vPos, Value );
			break;

		case AI_SEVENT_EYEFIRESND:
			AI_SEvent_EyeFireSound ( pThis, vPos, Value );
			break;

		case AI_SEVENT_FIREBALLPART:
			AI_SEvent_FireBallPart ( pThis, vPos, Value );
			break;

		case AI_SEVENT_FIREBALLSND:
			AI_SEvent_FireBallSnd ( pThis, vPos, Value );
			break;

		case AI_SEVENT_JAWSSNAP:
			AI_SEvent_JawSnap ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SHOCKWAVEPART:
			AI_SEvent_ShockWavePart ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SHOCKWAVESND:
			AI_SEvent_ShockWaveSnd ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SPELLCASTPART:
			AI_SEvent_SpellCastPart ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SPELLCASTSND:
			AI_SEvent_SpellCastSnd ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SWINGFISTSWEAPON:
			AI_SEvent_SwingFistsWeapon ( pThis, vPos, Value );
			break;

		case AI_SEVENT_WEAPONFIRE:
			AI_SEvent_WeaponFire ( pThis, vPos, Value );
			break;

		case AI_SEVENT_WEAPONROUND:
			AI_SEvent_WeaponRound ( pThis, vPos, Value );
			break;

		case AI_SEVENT_WEAPONCOCK:
			AI_SEvent_WeaponCock ( pThis, vPos, Value );
			break;

		case AI_SEVENT_LOOKOFFSET:
			AI_SEvent_LookOffset ( pThis, Value );
			break;

		case AI_SEVENT_CHITTER:
			AI_SEvent_Chitter ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SURFACEIMPACT:
			AI_SEvent_SurfaceImpact ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ALERT:
			AI_SEvent_Alert ( pThis, vPos, Value );
			break;

		case AI_SEVENT_HYDRAULICS:
			AI_SEvent_Hydraulics ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SHUTDOWN:
			AI_SEvent_Shutdown ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SWIVEL:
			AI_SEvent_Swivel ( pThis, vPos, Value );
			break;

		case AI_SEVENT_JETPACKOUTOFCTRL:
			AI_SEvent_JetpackOutofControl ( pThis, vPos, Value );
			break;

		case AI_SEVENT_BURSTFROMGROUND:
			AI_SEvent_BurstFromGround ( pThis, vPos, Value );
			break;

		case AI_SEVENT_CHESTTHUMP:
			AI_SEvent_ChestThump ( pThis, vPos, Value );
			break;

		case AI_SEVENT_DEATH:
			AI_SEvent_Death ( pThis, vPos, Value );
			break;

		case AI_SEVENT_EXPLODE:
			AI_SEvent_Explode ( pThis, vPos, Value );

			break;

		case AI_SEVENT_FOOTFALL:
			AI_SEvent_Footfall ( pThis, vPos, Value );
			break;

		case AI_SEVENT_GRUNT:
			AI_SEvent_Grunt ( pThis, vPos, Value );
			break;

		case AI_SEVENT_GURGLE:
			AI_SEvent_Gurgle ( pThis, vPos, Value );
			break;

		case AI_SEVENT_HANDFALL:
			AI_SEvent_Handfall ( pThis, vPos, Value );
			break;

		case AI_SEVENT_HISS:
			AI_SEvent_Hiss ( pThis, vPos, Value );
			break;

		case AI_SEVENT_HITGROUND:
			AI_SEvent_Hitground ( pThis, vPos, Value );
			break;

		case AI_SEVENT_LICKING:
			AI_SEvent_Licking ( pThis, vPos, Value );
			break;

		case AI_SEVENT_RAGE:
			AI_SEvent_Rage ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ROCKNOISE1:
			AI_SEvent_RockNoise1 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_RUBBINGSKIN:
			AI_SEvent_RubbingSkin ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SCRATCH:
			AI_SEvent_Scratch ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SCREECH:
			AI_SEvent_Screech ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SNIFF:
			AI_SEvent_Sniff ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SWOOSH:
			AI_SEvent_Swoosh ( pThis, vPos, Value );
			break;

		case AI_SEVENT_TURNTOSTONE:
			AI_SEvent_TurntoStone ( pThis, vPos, Value );
			break;

		case AI_SEVENT_VIOLENTDEATH:
			AI_SEvent_ViolentDeath ( pThis, vPos, Value );
			break;

		case AI_SEVENT_WINGFLAP:
			AI_SEvent_WingFlap ( pThis, vPos, Value );
			break;

		case AI_SEVENT_GROWL:
			AI_SEvent_Growl ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ALERT1:
			AI_SEvent_Alert1 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ALERT2:
			AI_SEvent_Alert2 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ALERT3:
			AI_SEvent_Alert3 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ALERT4:
			AI_SEvent_Alert4 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_DEATH1:
			AI_SEvent_Death1 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_DEATH2:
			AI_SEvent_Death2 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_DEATH3:
			AI_SEvent_Death3 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_DEATH4:
			AI_SEvent_Death4 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_INJURY1:
			AI_SEvent_Injury1 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_INJURY2:
			AI_SEvent_Injury2 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_INJURY3:
			AI_SEvent_Injury3 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_LONG1:
			AI_SEvent_Long1 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_LONG2:
			AI_SEvent_Long2 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_LONG3:
			AI_SEvent_Long3 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_TAUNT1:
			AI_SEvent_Taunt1 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_TAUNT2:
			AI_SEvent_Taunt2 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_TAUNT3:
			AI_SEvent_Taunt3 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_VIOLENTDEATH1:
			AI_SEvent_ViolentDeath1 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_VIOLENTDEATH2:
			AI_SEvent_ViolentDeath2 ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SCREENSHAKE:
			AI_SEvent_ScreenShake ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SCREENTREMOR:
			AI_SEvent_ScreenTremor ( pThis, vPos, Value );
			break;

		case AI_SEVENT_INVINCIBLE:
			AI_SEvent_Invincible ( pThis, vPos, Value );
			break;

		case AI_SEVENT_BLOODARCPARTICLES:
			AI_SEvent_BloodArcParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_MUZZLEFLASH:
			AI_SEvent_MuzzleFlashParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_HULK_BLASTER:
			AI_SEvent_HulkBlasterParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ALIEN_PROJECTILE:
			AI_SEvent_AlienDeathFireParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_KNIFEFORWARD:
			AI_SEvent_KnifeForwardParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_KNIFELEFT:
			AI_SEvent_KnifeLeftParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_KNIFERIGHT:
			AI_SEvent_KnifeRightParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_GREENBLOODARCPARTICLES:
			AI_SEvent_GreenBloodArcParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SPURT_BLOODPARTICLES:
			AI_SEvent_SpurtBloodArcParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_FOUNTAIN_BLOODPARTICLES:
			AI_SEvent_FountainBloodArcParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_GUSH_BLOODPARTICLES:
			AI_SEvent_GushBloodArcParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_DROOL_PARTICLES:
			AI_SEvent_DroolParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ARTERIAL_BLOODPARTICLES:
			AI_SEvent_ArterialBloodParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_SPURT_GBLOODPARTICLES:
			AI_SEvent_SpurtGBloodParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_FOUNTAIN_GBLOODPARTICLES:
			AI_SEvent_FountainGBloodParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_GUSH_GBLOODPARTICLES:
			AI_SEvent_GushGBloodParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ARTERIAL_GBLOODPARTICLES:
			AI_SEvent_ArterialGBloodParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_WATER_DROP:
			AI_SEvent_WaterDropParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_WATER_FOAM:
			AI_SEvent_WaterFoamParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_WATER_STEAM:
			AI_SEvent_WaterSteamParticles ( pThis, vPos, Value );
			break;

		case AI_SEVENT_ARROW_DAMAGE:
			AI_SEvent_Arrow_Damage(pThis, pOrigin, vPos, Value);
			break;

		case AI_SEVENT_DAMAGETARGET:
			AI_SEvent_Damage_Target(pThis, pOrigin, vPos, Value);
			break;

		case AI_SEVENT_DAMAGENUDGETARGET:
			AI_SEvent_DamageNudge_Target(pThis, pOrigin, vPos, Value);
			break;

		case AI_SEVENT_GENERIC1:
			AI_SEvent_Generic1(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC2:
			AI_SEvent_Generic2(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC3:
			AI_SEvent_Generic3(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC4:
			AI_SEvent_Generic4(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC5:
			AI_SEvent_Generic5(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC6:
			AI_SEvent_Generic6(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC7:
			AI_SEvent_Generic7(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC8:
			AI_SEvent_Generic8(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC9:
			AI_SEvent_Generic9(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC10:
			AI_SEvent_Generic10(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC11:
			AI_SEvent_Generic11(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC12:
			AI_SEvent_Generic12(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC13:
			AI_SEvent_Generic13(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC14:
			AI_SEvent_Generic14(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC15:
			AI_SEvent_Generic15(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC16:
			AI_SEvent_Generic16(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC17:
			AI_SEvent_Generic17(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC18:
			AI_SEvent_Generic18(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC19:
			AI_SEvent_Generic19(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC20:
			AI_SEvent_Generic20(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC21:
			AI_SEvent_Generic21(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC22:
			AI_SEvent_Generic22(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC23:
			AI_SEvent_Generic23(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC24:
			AI_SEvent_Generic24(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC25:
			AI_SEvent_Generic25(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC26:
			AI_SEvent_Generic26(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC27:
			AI_SEvent_Generic27(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC28:
			AI_SEvent_Generic28(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC29:
			AI_SEvent_Generic29(pThis, vPos, Value);
			break;
		case AI_SEVENT_GENERIC30:
			AI_SEvent_Generic30(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC31:
			AI_SEvent_Generic31(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC32:
			AI_SEvent_Generic32(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC33:
			AI_SEvent_Generic33(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC34:
			AI_SEvent_Generic34(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC35:
			AI_SEvent_Generic35(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC36:
			AI_SEvent_Generic36(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC37:
			AI_SEvent_Generic37(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC38:
			AI_SEvent_Generic38(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC39:
			AI_SEvent_Generic39(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC40:
			AI_SEvent_Generic40(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC41:
			AI_SEvent_Generic41(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC42:
			AI_SEvent_Generic42(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC43:
			AI_SEvent_Generic43(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC44:
			AI_SEvent_Generic44(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC45:
			AI_SEvent_Generic45(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC46:
			AI_SEvent_Generic46(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC47:
			AI_SEvent_Generic47(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC48:
			AI_SEvent_Generic48(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC49:
			AI_SEvent_Generic49(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC50:
			AI_SEvent_Generic50(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC51:
			AI_SEvent_Generic51(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC52:
			AI_SEvent_Generic52(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC53:
			AI_SEvent_Generic53(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC54:
			AI_SEvent_Generic54(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC55:
			AI_SEvent_Generic55(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC56:
			AI_SEvent_Generic56(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC57:
			AI_SEvent_Generic57(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC58:
			AI_SEvent_Generic58(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC59:
			AI_SEvent_Generic59(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC60:
			AI_SEvent_Generic60(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC61:
			AI_SEvent_Generic61(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC62:
			AI_SEvent_Generic62(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC63:
			AI_SEvent_Generic63(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC64:
			AI_SEvent_Generic64(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC65:
			AI_SEvent_Generic65(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC66:
			AI_SEvent_Generic66(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC67:
			AI_SEvent_Generic67(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC68:
			AI_SEvent_Generic68(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC69:
			AI_SEvent_Generic69(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC70:
			AI_SEvent_Generic70(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC71:
			AI_SEvent_Generic71(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC72:
			AI_SEvent_Generic72(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC73:
			AI_SEvent_Generic73(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC74:
			AI_SEvent_Generic74(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC75:
			AI_SEvent_Generic75(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC76:
			AI_SEvent_Generic76(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC77:
			AI_SEvent_Generic77(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC78:
			AI_SEvent_Generic78(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC79:
			AI_SEvent_Generic79(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC80:
			AI_SEvent_Generic80(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC81:
			AI_SEvent_Generic81(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC82:
			AI_SEvent_Generic82(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC83:
			AI_SEvent_Generic83(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC84:
			AI_SEvent_Generic84(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC85:
			AI_SEvent_Generic85(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC86:
			AI_SEvent_Generic86(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC87:
			AI_SEvent_Generic87(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC88:
			AI_SEvent_Generic88(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC89:
			AI_SEvent_Generic89(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC90:
			AI_SEvent_Generic90(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC91:
			AI_SEvent_Generic91(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC92:
			AI_SEvent_Generic92(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC93:
			AI_SEvent_Generic93(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC94:
			AI_SEvent_Generic94(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC95:
			AI_SEvent_Generic95(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC96:
			AI_SEvent_Generic96(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC97:
			AI_SEvent_Generic97(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC98:
			AI_SEvent_Generic98(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC99:
			AI_SEvent_Generic99(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC100:
			AI_SEvent_Generic100(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERIC101:	AI_SEvent_Generic101(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC102:	AI_SEvent_Generic102(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC103:	AI_SEvent_Generic103(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC104:	AI_SEvent_Generic104(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC105:	AI_SEvent_Generic105(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC106:	AI_SEvent_Generic106(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC107:	AI_SEvent_Generic107(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC108:	AI_SEvent_Generic108(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC109:	AI_SEvent_Generic109(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC110:	AI_SEvent_Generic110(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC111:	AI_SEvent_Generic111(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC112:	AI_SEvent_Generic112(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC113:	AI_SEvent_Generic113(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC114:	AI_SEvent_Generic114(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC115:	AI_SEvent_Generic115(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC116:	AI_SEvent_Generic116(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC117:	AI_SEvent_Generic117(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC118:	AI_SEvent_Generic118(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC119:	AI_SEvent_Generic119(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC120:	AI_SEvent_Generic120(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC121:	AI_SEvent_Generic121(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC122:	AI_SEvent_Generic122(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC123:	AI_SEvent_Generic123(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC124:	AI_SEvent_Generic124(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC125:	AI_SEvent_Generic125(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC126:	AI_SEvent_Generic126(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC127:	AI_SEvent_Generic127(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC128:	AI_SEvent_Generic128(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC129:	AI_SEvent_Generic129(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC130:	AI_SEvent_Generic130(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC131:	AI_SEvent_Generic131(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC132:	AI_SEvent_Generic132(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC133:	AI_SEvent_Generic133(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC134:	AI_SEvent_Generic134(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC135:	AI_SEvent_Generic135(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC136:	AI_SEvent_Generic136(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC137:	AI_SEvent_Generic137(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC138:	AI_SEvent_Generic138(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC139:	AI_SEvent_Generic139(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC140:	AI_SEvent_Generic140(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC141:	AI_SEvent_Generic141(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC142:	AI_SEvent_Generic142(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC143:	AI_SEvent_Generic143(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC144:	AI_SEvent_Generic144(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC145:	AI_SEvent_Generic145(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC146:	AI_SEvent_Generic146(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC147:	AI_SEvent_Generic147(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC148:	AI_SEvent_Generic148(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC149:	AI_SEvent_Generic149(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC150:	AI_SEvent_Generic150(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC151:	AI_SEvent_Generic151(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC152:	AI_SEvent_Generic152(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC153:	AI_SEvent_Generic153(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC154:	AI_SEvent_Generic154(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC155:	AI_SEvent_Generic155(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC156:	AI_SEvent_Generic156(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC157:	AI_SEvent_Generic157(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC158:	AI_SEvent_Generic158(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC159:	AI_SEvent_Generic159(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC160:	AI_SEvent_Generic160(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC161:	AI_SEvent_Generic161(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC162:	AI_SEvent_Generic162(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC163:	AI_SEvent_Generic163(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC164:	AI_SEvent_Generic164(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC165:	AI_SEvent_Generic165(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC166:	AI_SEvent_Generic166(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC167:	AI_SEvent_Generic167(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC168:	AI_SEvent_Generic168(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC169:	AI_SEvent_Generic169(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC170:	AI_SEvent_Generic170(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC171:	AI_SEvent_Generic171(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC172:	AI_SEvent_Generic172(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC173:	AI_SEvent_Generic173(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC174:	AI_SEvent_Generic174(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC175:	AI_SEvent_Generic175(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC176:	AI_SEvent_Generic176(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC177:	AI_SEvent_Generic177(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC178:	AI_SEvent_Generic178(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC179:	AI_SEvent_Generic179(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC180:	AI_SEvent_Generic180(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC181:	AI_SEvent_Generic181(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC182:	AI_SEvent_Generic182(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC183:	AI_SEvent_Generic183(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC184:	AI_SEvent_Generic184(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC185:	AI_SEvent_Generic185(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC186:	AI_SEvent_Generic186(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC187:	AI_SEvent_Generic187(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC188:	AI_SEvent_Generic188(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC189:	AI_SEvent_Generic189(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC190:	AI_SEvent_Generic190(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC191:	AI_SEvent_Generic191(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC192:	AI_SEvent_Generic192(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC193:	AI_SEvent_Generic193(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC194:	AI_SEvent_Generic194(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC195:	AI_SEvent_Generic195(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC196:	AI_SEvent_Generic196(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC197:	AI_SEvent_Generic197(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC198:	AI_SEvent_Generic198(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC199:	AI_SEvent_Generic199(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC200:	AI_SEvent_Generic200(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC201:	AI_SEvent_Generic201(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC202:	AI_SEvent_Generic202(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC203:	AI_SEvent_Generic203(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC204:	AI_SEvent_Generic204(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC205:	AI_SEvent_Generic205(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC206:	AI_SEvent_Generic206(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC207:	AI_SEvent_Generic207(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC208:	AI_SEvent_Generic208(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC209:	AI_SEvent_Generic209(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC210:	AI_SEvent_Generic210(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC211:	AI_SEvent_Generic211(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC212:	AI_SEvent_Generic212(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC213:	AI_SEvent_Generic213(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC214:	AI_SEvent_Generic214(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC215:	AI_SEvent_Generic215(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC216:	AI_SEvent_Generic216(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC217:	AI_SEvent_Generic217(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC218:	AI_SEvent_Generic218(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC219:	AI_SEvent_Generic219(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC220:	AI_SEvent_Generic220(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC221:	AI_SEvent_Generic221(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC222:	AI_SEvent_Generic222(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC223:	AI_SEvent_Generic223(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC224:	AI_SEvent_Generic224(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC225:	AI_SEvent_Generic225(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC226:	AI_SEvent_Generic226(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC227:	AI_SEvent_Generic227(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC228:	AI_SEvent_Generic228(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC229:	AI_SEvent_Generic229(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC230:	AI_SEvent_Generic230(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC231:	AI_SEvent_Generic231(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC232:	AI_SEvent_Generic232(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC233:	AI_SEvent_Generic233(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC234:	AI_SEvent_Generic234(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC235:	AI_SEvent_Generic235(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC236:	AI_SEvent_Generic236(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC237:	AI_SEvent_Generic237(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC238:	AI_SEvent_Generic238(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC239:	AI_SEvent_Generic239(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC240:	AI_SEvent_Generic240(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC241:	AI_SEvent_Generic241(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC242:	AI_SEvent_Generic242(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC243:	AI_SEvent_Generic243(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC244:	AI_SEvent_Generic244(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC245:	AI_SEvent_Generic245(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC246:	AI_SEvent_Generic246(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC247:	AI_SEvent_Generic247(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC248:	AI_SEvent_Generic248(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC249:	AI_SEvent_Generic249(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC250:	AI_SEvent_Generic250(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC251:	AI_SEvent_Generic251(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC252:	AI_SEvent_Generic252(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC253:	AI_SEvent_Generic253(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC254:	AI_SEvent_Generic254(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC255:	AI_SEvent_Generic255(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC256:	AI_SEvent_Generic256(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC257:	AI_SEvent_Generic257(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC258:	AI_SEvent_Generic258(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC259:	AI_SEvent_Generic259(pThis, vPos, Value);	break;
		case AI_SEVENT_GENERIC260:	AI_SEvent_Generic260(pThis, vPos, Value);	break;

		case AI_SEVENT_PLASMA2_PARTICLE:
			AI_SEvent_Plasma2Particle ( pThis, vPos, Value );
			break;

		case AI_SEVENT_WATER_BUBBLE:
			AI_SEvent_Water_Bubble(pThis, vPos, Value);
			break;

		case AI_SEVENT_WATER_RIPPLE:
			AI_SEvent_Water_Ripple(pThis, vPos, Value);
			break;

		case AI_SEVENT_WATER_SPLASH:
			AI_SEvent_Water_Splash(pThis, vPos, Value);
			break;

		case AI_SEVENT_DOOR_ALLOWENTRY:
			DoorAI_SEvent_AllowBlockEntry(pThis, vPos, TRUE);
			break;

		case AI_SEVENT_DOOR_BLOCKENTRY:
			DoorAI_SEvent_AllowBlockEntry(pThis, vPos, FALSE);
			break;

		case AI_SEVENT_GENERATE_PICKUPS:
			AI_SEvent_GeneratePickups(pThis, vPos);
			break;

		case AI_SEVENT_GENERATE_PICKUP:
			AI_SEvent_Generate_Pickup(pThis, vPos, Value);
			break;

		case AI_SEVENT_MAKE_INVISIBLE:
			AI_SEvent_MakeInvisible(pThis, vPos, Value);
			break;

		case AI_SEVENT_GENERAL_SOUND:
/*
			if (	(pThis)
				&&	(AI_GetDyn((CGameObjectInstance *)pThis)->m_dwStatusFlags2	& AI_IGNORESOUNDEVENTS))
			{
				break ;
			}
			else
*/
			AI_SEvent_GeneralSound(pThis, vPos, Value);
			break;

		case AI_SEVENT_SWOOSH_ALL_HANDS:
			AI_SEvent_SwooshAllHands(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_LEFT_HAND:
			AI_SEvent_SwooshLeftHand(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_RIGHT_HAND:
			AI_SEvent_SwooshRightHand(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_ALL_FEET:
			AI_SEvent_SwooshAllFeet(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_LEFT_FOOT:
			AI_SEvent_SwooshLeftFoot(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_RIGHT_FOOT:
			AI_SEvent_SwooshRightFoot(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_WEAPON1:
			AI_SEvent_SwooshWeapon1(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_WEAPON2:
			AI_SEvent_SwooshWeapon2(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_TAIL:
			AI_SEvent_SwooshTail(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_LEFT_TOES:
			AI_SEvent_SwooshLeftToes(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_SWOOSH_RIGHT_TOES:
			AI_SEvent_SwooshRightToes(pThis, vPos, Value) ;
			break ;

		case AI_SEVENT_NEXT_CINEMATIC_STAGE:
			GetApp()->m_Camera.m_NextCinematicStage = TRUE ;
			break ;

		case AI_SEVENT_BOUNCY_ROCK:
			AI_SEvent_GenerateBouncy(pThis, vPos, AI_OBJECT_BOUNCY_ROCK);
			break ;
		case AI_SEVENT_BOUNCY_ALIEN_HEAD:
			AI_SEvent_GenerateBouncy(pThis, vPos, AI_OBJECT_BOUNCY_ALIEN_HEAD);
			break ;
		case AI_SEVENT_BOUNCY_ALIEN_HAND:
			AI_SEvent_GenerateBouncy(pThis, vPos, AI_OBJECT_BOUNCY_ALIEN_HAND);
			break ;
		case AI_SEVENT_BOUNCY_ALIEN_FOOT:
			AI_SEvent_GenerateBouncy(pThis, vPos, AI_OBJECT_BOUNCY_ALIEN_FOOT);
			break ;
		case AI_SEVENT_BOUNCY_ALIEN_TORSO:
			AI_SEvent_GenerateBouncy(pThis, vPos, AI_OBJECT_BOUNCY_ALIEN_TORSO);
			break ;
		case AI_SEVENT_BOUNCY_LEAPER_HEAD:
			AI_SEvent_GenerateBouncy(pThis, vPos, AI_OBJECT_BOUNCY_LEAPER_HEAD);
			break ;
		case AI_SEVENT_BOUNCY_LEAPER_HAND:
			AI_SEvent_GenerateBouncy(pThis, vPos, AI_OBJECT_BOUNCY_LEAPER_HAND);
			break ;
		case AI_SEVENT_BOUNCY_LEAPER_FOOT:
			AI_SEvent_GenerateBouncy(pThis, vPos, AI_OBJECT_BOUNCY_LEAPER_FOOT);
			break ;
		case AI_SEVENT_BOUNCY_LEAPER_TORSO:
			AI_SEvent_GenerateBouncy(pThis, vPos, AI_OBJECT_BOUNCY_LEAPER_TORSO);
			break ;

		case AI_SEVENT_INSTANTDEATH:
			AI_SEvent_InstantDeath(pThis);
			break;

		case AI_SEVENT_RANDOM_BIG_SWOOSH_SOUND:
			AI_SEvent_RandBigSwooshSnd(pThis,vPos,Value) ;
			break ;

		case AI_SEVENT_RANDOM_SMALL_SWOOSH_SOUND:
			AI_SEvent_RandSmallSwooshSnd(pThis,vPos,Value) ;
			break ;

		case AI_SEVENT_RANDOM_GRUNT_SOUND:
			AI_SEvent_RandGruntSnd(pThis,vPos,Value) ;
			break ;

		case AI_SEVENT_START_FIRE_SOUND:
			AI_SEvent_StartFireSound(pThis,vPos,Value) ;
			break ;
		case AI_SEVENT_STOP_FIRE_SOUND:
			AI_SEvent_StopFireSound(pThis,vPos,Value) ;
			break ;
	}
}

BOOL SendEventToWholeWorld(int Event)
{
	switch (Event)
	{
		case AI_MEVENT_PRESSUREPLATE:
			return TRUE;

		default:
			return FALSE;
	}
}

void AI_Event_Dispatcher(CInstanceHdr *pThis, CInstanceHdr *pOrigin,
								 int Event, DWORD dwSpecies,
								 CVector3 vPos, float Value)
{
	int						cAI, nAIs;
	CGameObjectInstance	*pAI, *instances;
	CUnindexedSet			usInstances;
	CEngineApp				*pApp;
	BOOL						gamepaused;

	TRACE("Event: %s\r\n", AI_Event_Strings[Event]);


	// is game paused - save points or user pressing pause ?
	gamepaused = (GetApp()->m_bPause && GetApp()->m_Mode == MODE_GAME);

	// save certain events for boss levels
	if (		GetApp()->m_pBossActive
			&&	(		(Event == AI_SEVENT_DOOR_ALLOWENTRY)
					||	(Event == AI_SEVENT_DOOR_BLOCKENTRY) ) )
	{
		CBossesStatus__SaveEvent(&GetApp()->m_BossesStatus, pOrigin, Event, vPos, Value) ;
	}

	if (		(IsSingleEvent(Event))
			&&	(!gamepaused) )
	{
		AI_Single_Event(pThis, pOrigin, Event, vPos, Value);
	}
	else
	{
		pApp = GetApp();

		if (SendEventToWholeWorld(Event))
		{
			ASSERT(pApp->m_Scene.m_pceInstances);

			CUnindexedSet__ConstructFromRawData(&usInstances,
															CCacheEntry__GetData(pApp->m_Scene.m_pceInstances),
															FALSE);

			instances = (CGameObjectInstance*) CUnindexedSet__GetBasePtr(&usInstances);
			nAIs = CUnindexedSet__GetBlockCount(&usInstances);

			for (cAI=0; cAI<nAIs; cAI++)
			{
				pAI = &instances[cAI];

				// don't send event to self
				if ( ((&pAI->ah.ih) != pThis) && (AI_GetEA(pAI)->m_dwSpecies & dwSpecies) )
					AI_Multi_Event(pAI, pOrigin, Event, vPos, Value);
			}

			CUnindexedSet__Destruct(&usInstances);
		}
		else if (!gamepaused)
		{
			nAIs = CEngineApp__GetAnimInstanceCount(pApp);
			for (cAI=0; cAI<nAIs; cAI++)
			{
				pAI = CEngineApp__GetAnimInstance(pApp, cAI);

				// don't send event to self
				if ( ((&pAI->ah.ih) != pThis) && (AI_GetEA(pAI)->m_dwSpecies & dwSpecies) )
					AI_Multi_Event(pAI, pOrigin, Event, vPos, Value);
			}
		}
	}
}

// get attack ai destination
//
void AI_Get_Attack_Destination(CGameObjectInstance *pAI)
{
	// declare variables
	CGameObjectInstance	*pDest;


	// is this ai a kamikaze ?
	if (AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_KAMIKAZE)
	{
		// ai is a kamikaze - make it point to turok & increase agitation to max
		pAI->m_pAttackerCGameObjectInstance = CEngineApp__GetPlayer(GetApp());
		AI_Increase_Agitation(pAI, AI_AGITATION_FIGHTING, 300);
	}
	else
	{
		// is the attack destination dead ?
		pDest = pAI->m_pAttackerCGameObjectInstance;
		if (pDest==NULL || pDest==pAI)
		{
			pAI->m_pAttackerCGameObjectInstance = CEngineApp__GetPlayer(GetApp());
		}
		else
		{
			// is the destination ai dead ?
			if (AI_GetDyn(pDest)->m_Health<=0)
			{
				// dest ai is dead - ai should now attack player
				pAI->m_pAttackerCGameObjectInstance = CEngineApp__GetPlayer(GetApp());
			}
		}
	}


}




// does this enemy feign death until approached closely by turok ?
//
void AI_FeignDeath(CGameObjectInstance *pThis)
{
	CGameObjectInstance	*pPlayer;
	CVector3					vDestPos;


	if ( (AI_GetDyn(pThis)->m_dwStatusFlags & AI_ALREADY_DEAD) &&
		  (AI_GetDyn(pThis)->m_dwStatusFlags & AI_FEIGNDEATH)        )
	{
		pPlayer  = CEngineApp__GetPlayer(GetApp());
		vDestPos = AI_GetPos(pPlayer);

		if (AI_DistanceSquared(pThis, vDestPos) <= AI_GetEA(pThis)->m_AttackRadius)
		{
			// resurrect this ai
			CAIDynamic__SetHealth(AI_GetDyn(pThis),
										 AI_GetEA(pThis),
										 CGameObjectInstance__TypeFlag(pThis));

			AI_GetDyn(pThis)->m_Health				= AI_GetDyn(pThis)->m_Health * 35 / 100;
			if (AI_GetDyn(pThis)->m_Health<1) AI_GetDyn(pThis)->m_Health = 1;
			AI_GetDyn(pThis)->m_dwStatusFlags	&= ~AI_ALREADY_DEAD;
			AI_GetDyn(pThis)->m_dwStatusFlags	&= ~AI_WAITFOR_CYCLE;
			AI_GetDyn(pThis)->m_Agitation			= 0;
			AI_GetDyn(pThis)->m_PrevAgitation	= 0;
			AI_GetDyn(pThis)->m_dwStatusFlags	&= ~AI_WOUNDEDMORTALLY;
			AI_GetDyn(pThis)->m_dwStatusFlags2	&= ~AI_GENERATED_PICKUPS;

			pThis->m_cMelt = 0;

			// ai can only feign death once
			AI_GetDyn(pThis)->m_dwStatusFlags	&= (~AI_FEIGNDEATH);
		}
	}
}




// does this enemy resurrect when turok leaves its leash area ?
//
void AI_Resurrect(CGameObjectInstance *pThis)
{
	CGameObjectInstance	*pPlayer;
	CVector3					vDestPos;


	if ( (AI_GetDyn(pThis)->m_dwStatusFlags & AI_ALREADY_DEAD) &&
		  (AI_GetDyn(pThis)->m_dwStatusFlags & AI_RESURRECTION)        )
	{
		pPlayer  = CEngineApp__GetPlayer(GetApp());
		vDestPos = AI_GetPos(pPlayer);

		// IMPORTANT !!!!!! - this distance must be within the animation rectangle
		if ( (AI_DistanceSquared(pThis, vDestPos) >= (GetApp()->m_FarClip*(0.95*SCALING_FACTOR)) * (GetApp()->m_FarClip*(0.95*SCALING_FACTOR))) &&
			  (!CGameObjectInstance__IsVisible(pThis))         )
		{
			// resurrect this ai
			CAIDynamic__SetHealth(AI_GetDyn(pThis),
										 AI_GetEA(pThis),
										 CGameObjectInstance__TypeFlag(pThis));

			pThis->m_cMelt = 0;

			AI_GetDyn(pThis)->m_Health				= AI_GetDyn(pThis)->m_Health * 85 / 100;
			if (AI_GetDyn(pThis)->m_Health<1) AI_GetDyn(pThis)->m_Health = 1;
			AI_GetDyn(pThis)->m_dwStatusFlags	&= ~AI_ALREADY_DEAD;
			AI_GetDyn(pThis)->m_dwStatusFlags	&= ~AI_WAITFOR_CYCLE;
			AI_GetDyn(pThis)->m_Agitation			= 0;
			AI_GetDyn(pThis)->m_PrevAgitation	= 0;
			AI_GetDyn(pThis)->m_dwStatusFlags	&= ~AI_WOUNDEDMORTALLY;
			AI_GetDyn(pThis)->m_dwStatusFlags2	&= ~AI_GENERATED_PICKUPS;

			// when ai dies again it can never come back
			AI_GetDyn(pThis)->m_dwStatusFlags	&= (~AI_RESURRECTION);
		}
	}
}


// do ai update for gallery cheat
//
CGameObjectInstance *AI_Advance_Gallery(CGameObjectInstance *pMe)
{
	int	picked;

	int	typeg1[]		= { AI_ANIM_IDLE_INTEL1,
								 AI_ANIM_IDLE_INTEL2,
								 AI_ANIM_IDLE,
								 AI_ANIM_RAGE,
								 AI_ANIM_GETATTENTION };

	int	typeg2[]		= { AI_ANIM_IDLE_INTEL1,
								 AI_ANIM_IDLE_INTEL2,
								 AI_ANIM_IDLE2,
								 AI_ANIM_RAGE,
								 AI_ANIM_GETATTENTION };

	int	weightg[]	= { 5, 5, 10, 2, 2 };


	// don't do anything if this is the player
	if (CEngineApp__GetPlayer(GetApp()) == pMe)
		return NULL;

	// wait for the last cycle to be completed
	if (!AI_Cycle_Completed(pMe))
		return pMe;

	// run through gallery animations randomly
	if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
		picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 5, typeg2, weightg, NULL);
	else
		picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 5, typeg1, weightg, NULL);

	AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
	AI_SetDesiredAnimByIndex(pMe, picked, TRUE);


	return pMe;
}


// do ai update
//
void AI_Advance(CGameObjectInstance *pMe)
{
	// declare variables
	short						alevel;
	float						Dist,
								DistTurok;
	CGameObjectInstance	*pLeader,
								*pPlayer;
	int						waterFlag,
								nAnim;
	BOOL						Regen;


	if (AI_GetEA(pMe)->m_wTypeFlags3 & AI_TYPE3_DUMB)
	{
		AI_GetDyn(pMe)->m_dwStatusFlags2 |= AI_VISIBLE;
		AI_SetDesiredAnim(pMe, AI_ANIM_EXTRA1, TRUE);
		return ;
	}


	// get pointer to player
	pPlayer = CEngineApp__GetPlayer(GetApp());


	// first time running ai ?
	if (AI_GetDyn(pMe)->m_FirstRun)
	{
		if (pMe == pPlayer)
		{
			AI_GetDyn(pMe)->m_FirstRun = FALSE;
		}
		else
		{
			if (		(AI_GetDyn(pMe)->m_dwStatusFlags & AI_INTERACTIVEANIMATION)
					&&	(AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_HOLDINTANIMON1STFRAME) )
			{
				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				nAnim = AI_GetEA(pMe)->m_InteractiveAnim - 1 + AI_ANIM_INTERACTIVE_ANIMATION1;
				AI_SetDesiredAnim(pMe, nAnim, TRUE);
				CGameObjectInstance__RestartAnim(pMe);
				//AI_GetDyn(pMe)->m_Invincible = TRUE;
			}

			// show interactive anim once ?
			if (AI_GetEA(pMe)->m_wTypeFlags3 & AI_TYPE3_SHOWINTANIMONCE)
				CScene__SetInstanceFlag(&GetApp()->m_Scene, pMe, TRUE);

			if (AI_GetDyn(pMe)->m_dwStatusFlags & AI_REGENERATE)
				CGameObjectInstance__SetGone(pMe);

			AI_GetDyn(pMe)->m_FirstRun = FALSE;
		}
		return;
	}

	// Clear collision structure
	AICollidedWithWall = FALSE ;

	// get water status
	waterFlag = AI_GetWaterStatus(pMe);

	// update turok specific stuff
	if (pMe == pPlayer)
	{
		// if in end sequence mode, spped up turoks animation speed
		if (GetApp()->m_Camera.m_Mode == CAMERA_CINEMA_END_SEQUENCE2_MODE)
			frame_increment *= 1.5 ;
		// update turok on screen weapon animations (only for u64 engine)
		//if (!AI_Cycle_Completed(pMe)) return;									// wait for animation cycle to be completed
		if (AI_Update_Turok_Weapon(pMe)) return;
		return;
	}

	// check for water gaff's
	if (		(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_AVOIDWATER)
			&&	((waterFlag == AI_ON_WATERSURFACE) || (waterFlag == AI_BELOW_WATERSURFACE))
			&& (AI_GetDyn(pMe)->m_Health) )
	{
		// kill this ai
		AI_SEvent_InstantDeath(&pMe->ah.ih);
//		AI_CompletelyKillOff(pMe);
		return;
	}

	// is this character frozen ?
	if (AI_GetDyn(pMe)->m_FreezeTimer != 0.0)
	{
		// ready for this ai to explode ?
		if (AI_GetDyn(pMe)->m_FreezeTimer == AI_FREEZE_END)
		{
			// set random timer for frozen explosion
			AI_GetDyn(pMe)->m_Time1 = RANDOM(25);
			AI_GetDyn(pMe)->m_FreezeTimer = AI_FREEZE_GOINGTOEXPLODE;
		}

		if (AI_GetDyn(pMe)->m_FreezeTimer == AI_FREEZE_GOINGTOEXPLODE)
		{
			AI_GetDyn(pMe)->m_Time1--;
			if (AI_GetDyn(pMe)->m_Time1 <= 0.0)
			{
				// generate particle effect of ai exploding
				AI_DoParticle(&pMe->ah.ih, PARTICLE_TYPE_FREEZE_EXPLOSION, AI_GetPos(pMe));

				// get rid of ai
				AI_GetDyn(pMe)->m_dwStatusFlags	|= AI_ALREADY_DEAD;
				AI_GetDyn(pMe)->m_MeltTimer		= 0.0;
				AI_GetDyn(pMe)->m_FreezeTimer		= 0.0;
				pMe->m_cMelt							= MELT_LENGTH;
				CGameObjectInstance__SetGone(pMe);
			}
		}

		return;
	}

	// does this ai have to hold on 1st frame ?
	if (AI_GetDyn(pMe)->m_dwStatusFlags2 & AI_HOLDINTANIMON1STFRAME)
		frame_increment = 0.0;

	// if this ai has interactive flag, it is waiting, return
	if (AI_GetDyn(pMe)->m_dwStatusFlags & AI_INTERACTIVEANIMATION)
		return;

	// does the interactive animation have a delay ?
	if (AI_GetDyn(pMe)->m_dwStatusFlags & AI_INTERANIMDELAY)
	{
		AI_GetDyn(pMe)->m_InteractiveTimer -= (1.0 / 20.0);
		if (AI_GetDyn(pMe)->m_InteractiveTimer > 0.0)
			return;

		// stop holding on 1st frame
		AI_GetDyn(pMe)->m_dwStatusFlags2 &= ~AI_HOLDINTANIMON1STFRAME;

		// make ai visible
		AI_GetDyn(pMe)->m_dwStatusFlags2 |= AI_VISIBLE;

		// is ai gone ?
		if (		(CGameObjectInstance__IsGone(pMe))
				&&	(AI_GetDyn(pMe)->m_dwStatusFlags & AI_REGENERATE) )
		{
			CGameObjectInstance__SetNotGone(pMe);
		}

		// delay has ran out - let ai run
		AI_GetDyn(pMe)->m_InteractiveTimer = 0.0;
		AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_INTERANIMDELAY;

		// start the delayed interactive animation
		nAnim = AI_GetEA(pMe)->m_InteractiveAnim - 1 + AI_ANIM_INTERACTIVE_ANIMATION1;

		// play interactive anim again while regenerating ?
		if (AI_GetDyn(pMe)->m_dwStatusFlags & AI_REGENERATEINTERANIM)
			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;

		AI_SetDesiredAnim(pMe, nAnim, TRUE);
		CGameObjectInstance__RestartAnim(pMe);
		//AI_GetDyn(pMe)->m_Invincible = TRUE;
		return;
	}

	// new regeneration that was gone ? (if interactive anims are not used)
	if (		CGameObjectInstance__IsGone(pMe)
			&& (!AI_CanWeRemoveIt(pMe))
			&&	(AI_GetDyn(pMe)->m_dwStatusFlags & AI_REGENERATE) )
	{
		CGameObjectInstance__SetNotGone(pMe);
	}

	// if ai is appearing fade it in
	if (		(AI_GetDyn(pMe)->m_cRegenerateAppearance == APPEARANCE_LENGTH*7/8)
			&&	(!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_REPLAYINTANIMONREG))
			&& (!(AI_GetDyn(pMe)->m_dwStatusFlags2 & AI_REGENERATEAPPEARANCEDELAY)) )
	{
		AI_DoParticle(&pMe->ah.ih, PARTICLE_TYPE_REGENERATION_START, AI_GetPos(pMe));
		AI_GetDyn(pMe)->m_dwStatusFlags2 |= AI_REGENERATEAPPEARANCEDELAY;
		AI_GetDyn(pMe)->m_Time1 = 1.5;		// seconds before regenerated ai appears
		AI_DoSound(&pMe->ah.ih, SOUND_GENERIC_66, 1, 0);
	}

	// waiting for regeneration to appear ?
	if (AI_GetDyn(pMe)->m_dwStatusFlags2 & AI_REGENERATEAPPEARANCEDELAY)
	{
		if (AI_GetDyn(pMe)->m_Time1 > 0.0)
		{
			AI_GetDyn(pMe)->m_Time1 -= frame_increment*(1.0/FRAME_FPS);
			if (AI_GetDyn(pMe)->m_Time1 < 0.0)
				AI_GetDyn(pMe)->m_Time1 = 0.0;

			return;
		}
		else
		{
			AI_GetDyn(pMe)->m_dwStatusFlags2 &= ~AI_REGENERATEAPPEARANCEDELAY;
			AI_DoParticle(&pMe->ah.ih, PARTICLE_TYPE_REGENERATION_APPEARANCE, AI_GetPos(pMe));
			AI_DoSound(&pMe->ah.ih, SOUND_HIGH_PRIEST_TELEPORT, 1, 0);
		}
	}


	// regenerations is now appearing
	if (		(AI_GetDyn(pMe)->m_cRegenerateAppearance)
			&&	(!(AI_GetDyn(pMe)->m_dwStatusFlags2 & AI_REGENERATEAPPEARANCEDELAY)) )
	{
		AI_GetDyn(pMe)->m_cRegenerateAppearance -= frame_increment;
		if (AI_GetDyn(pMe)->m_cRegenerateAppearance <= 0.0)
		{
			// completely faded in
			AI_GetDyn(pMe)->m_cRegenerateAppearance = 0.0;
			AI_GetDyn(pMe)->m_cRegenerateAppearanceWhite = APPEARANCEWHITE_LENGTH;
		}
	}

	// whiteness after regeneration
	if (		(AI_GetDyn(pMe)->m_cRegenerateAppearanceWhite)
			&&	(!(AI_GetDyn(pMe)->m_dwStatusFlags2 & AI_REGENERATEAPPEARANCEDELAY)) )
	{
		AI_GetDyn(pMe)->m_cRegenerateAppearanceWhite -= frame_increment;
		if (AI_GetDyn(pMe)->m_cRegenerateAppearanceWhite <= 0.0)
		{
			// not white anymore
			AI_GetDyn(pMe)->m_cRegenerateAppearanceWhite = 0.0;
		}
	}

	// allow ai to be killed outside of view when it becomes active
	if (			(AI_GetEA(pMe)->m_wTypeFlags3 & AI_TYPE3_KILLOUTSIDEOFVIEW)
			&&		(CGameObjectInstance__IsVisible(pMe))
			&&		(!(AI_GetDyn(pMe)->m_dwStatusFlags2 & AI_KILLOUTSIDEOFVIEW)) )
	{
		AI_GetDyn(pMe)->m_dwStatusFlags2 |= AI_KILLOUTSIDEOFVIEW;
	}


	// update melting timer when they are ready to melt
	AI_UpdateMeltTimer(pMe);

	// what should this ai attack ?
	AI_Get_Attack_Destination(pMe);

	// Assist in getting to target!
	if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_SNIPER)
		AI_AssistSniperTurn(pMe);
	else
		AI_AssistRun(pMe);

	// keep ai flying if it does
	AI_Standard_Keep_It_Flying(pMe, pMe->m_pAttackerCGameObjectInstance);

	// keep ai swimming if it does
	AI_Make_Swim_AtHeight(pMe);

	// is this enemy going to feign death ?
	AI_FeignDeath(pMe);

	// is this enemy going to resurrect itself ?
	AI_Resurrect(pMe);

	// ai landing on ground after an explosion ?
	AI_Standard_ExplodedLand(pMe, waterFlag);

	// has this ai been hit by an explosion ? (flying ai)
	if (		(AI_GetDyn(pMe)->m_dwStatusFlags & AI_EV_EXPLOSION)
			&&	(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_FLYING) )
	{
		// convert explosion into normal hit
		AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_EV_EXPLOSION;
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_EV_HIT;
	}

	// has this ai been hit by an explosion ? (not a flying ai)
	if (		(AI_GetDyn(pMe)->m_dwStatusFlags & AI_EV_EXPLOSION)
			&&	(!(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_FLYING)) )
	{
//#ifdef GERMAN
//		// convert explosion into normal hit
//		AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_EV_EXPLOSION;
//		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_EV_HIT;
//#else
		// ai has an explosion death anim - it is ok to explode it when it is dead
		AI_Standard_Explosion(pMe);
//#endif
	}

	// has this ai been hit ?
	else if (AI_GetDyn(pMe)->m_dwStatusFlags & AI_EV_HIT)
	{
		if (AI_GetDyn(pMe)->m_Health > 0)
		{
			AI_Standard_Hit(pMe);
		}
		else
		{
			// do dead body riddle with bullets animation
			if (waterFlag == AI_NOT_NEAR_WATER)
			{
				AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_EV_HIT;
				if ( (AI_GetDyn(pMe)->m_dwStatusFlags & AI_ALREADY_DEAD) && AI_Cycle_Completed(pMe) )
					AI_Standard_DeadBodyRiddle(pMe);
			}
		}
	}

	// do general updates
	if (AI_GetDyn(pMe)->m_Health > 0)
	{
		// reset ai running flag
		AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_RUNNING);

		// ai does its normal stuff
		if ( (! AI_Can_See_Target(pMe, pMe->m_pAttackerCGameObjectInstance)) || AI_Get_Agitation_Level(pMe) == AI_AGITATED_ATTACK )	// can ai see target ?
		{
			if ( ! AI_Cycle_Completed ( pMe ) ) return;			// wait for animation cycle to be completed (overided by turok being seen)

			if (    (waterFlag != AI_ON_WATERSURFACE)
				  && (waterFlag != AI_BELOW_WATERSURFACE)
				  && (!(AI_GetDyn(pMe)->m_dwStatusFlags & AI_TELEPORTING)) )
			{
				if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
					AI_SetDesiredAnim(pMe, AI_ANIM_IDLE2, FALSE);
				else
					AI_SetDesiredAnim(pMe, AI_ANIM_IDLE, FALSE);
			}
		}
		AI_Decrease_Agitation ( pMe, AI_AGITATION_CALM );		// decrease agitation

		// is turok dead ?
		if (AI_GetDyn(pPlayer)->m_Health == 0)
			AI_Decrease_Agitation(pMe, AI_AGITATION_CALMAFTERFIGHT);		// decrease agitation even more because turok is dead

		// make ai teleport if it has to
		AI_Make_Teleport(pMe);

		// get agitation level
		alevel = AI_Get_Agitation_Level(pMe);

		// does ai have a leash ?
		if (AI_GetEA(pMe)->m_LeashRadius>0)
		{
			Dist      = AI_DistanceSquared(pMe, AI_GetDyn(pMe)->m_vLeashCoor);
			DistTurok = AI_DistanceSquared(pPlayer, AI_GetDyn(pMe)->m_vLeashCoor);
			if (Dist     >=AI_GetEA(pMe)->m_LeashRadius ||
				 DistTurok>=AI_GetEA(pMe)->m_LeashRadius      )
			{
				// set agitation back to zero - ai will return to start position (vLeashCoor)
				alevel=0;
			}
		}

		// set first attack flag if this is the first time the ai is in attack mode
		if ( (AI_GetDyn(pMe)->m_PrevAgitation != alevel) && (alevel == AI_AGITATED_ATTACK) )
		{
			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_FIRST_ATTACK;
		}

		// is ai to avoid the player ?
		if (    (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_AVOIDPLAYER)
			  && (AI_Can_See_Target(pMe, pMe->m_pAttackerCGameObjectInstance)) )
		{
			// ai avoids player - add on a random <2 seconds>
			AI_GetDyn(pMe)->m_RetreatTimer = AI_RETREAT_TIME + (((float)RANDOM(100))/50.0);
		}
		else if (AI_GetDyn(pMe)->m_RetreatTimer > 0.0)
		{
			AI_GetDyn(pMe)->m_RetreatTimer -= frame_increment*(1.0/FRAME_FPS);

			if (AI_GetDyn(pMe)->m_RetreatTimer < 0)
				AI_GetDyn(pMe)->m_RetreatTimer = 0;
		}

		// is ai a flocker ?
		pLeader = pMe->m_pEventCGameObjectInstance;
		if (    (AI_GetDyn(pMe)->m_dwStatusFlags & AI_FLOCKING)
			  && (pLeader != NULL) )
		{
			if (AI_GetDyn(pLeader)->m_Health <= 0)
			{
				AI_GetDyn(pMe)->m_dwStatusFlags &= AI_FLOCKING;
				AI_Increase_Agitation(pMe, AI_AGITATION_FIGHTING, 300);
				pMe->m_pEventCGameObjectInstance = NULL;
			}
			AI_Standard_Follow(pMe, pLeader);
		}
		else if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_SNIPER)
		{
			// sniping
			AI_Increase_Agitation(pMe, AI_AGITATION_FIGHTING, 300);
			AI_Standard_Sniper(pMe, pMe->m_pAttackerCGameObjectInstance);
		}
		else if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_FLYING)
		{
			// flying
			AI_Increase_Agitation(pMe, AI_AGITATION_FIGHTING, 300);
			AI_Standard_Flying(pMe, pMe->m_pAttackerCGameObjectInstance);
		}
		else if (AI_GetDyn(pMe)->m_RetreatTimer)
		{
			AI_Standard_Retreat(pMe, pMe->m_pAttackerCGameObjectInstance);
		}
		else if (AI_GetEA(pMe)->m_wTypeFlags3 & AI_TYPE3_WALKINSTRAIGHTLINE)
		{
			AI_SetDesiredAnim(pMe, AI_ANIM_WALK, TRUE);
		}
		else
		{
			// do ai based on agitation state
			switch(alevel)
			{
				case AI_AGITATED_IDLE:
					AI_Standard_Idle(pMe);
					break;

				case AI_AGITATED_AGITATED:

					// Signal not going back to leash now
					AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_GOBACKTOLEASH ;

					switch(waterFlag)
					{
						case AI_ON_WATERSURFACE:
						case AI_BELOW_WATERSURFACE:
							AI_Standard_Idle(pMe);
							break;

						default:
							AI_Standard_Agitated(pMe);
							break;
					}
					break;

				case AI_AGITATED_ATTACK:

					// Signal not going back to leash now
					AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_GOBACKTOLEASH ;

					CTMove__SetCombatTimer(&CTurokMovement, TMOVE_INCOMBAT_TIMER);

					if (AI_GetDyn(pMe)->m_dwStatusFlags & AI_TELEPORTMOVESLOW)
					{
						AI_Standard_TeleportMoveSlow(pMe);
						break;
					}

					if (AI_Standard_Teleport(pMe))
						break;

					AI_Standard_Attack(pMe, pMe->m_pAttackerCGameObjectInstance);
					break;
			}
		}

		// set previous agitation level
		AI_GetDyn(pMe)->m_PrevAgitation = alevel;
	}
	else
	{
		// ai is dead - no health, is ai already done death animation ?
		if (!(AI_GetDyn(pMe)->m_dwStatusFlags & AI_ALREADY_DEAD))
		{
			// ai still to do death animation
			switch (AI_Cycle_Completed(pMe))
			{
				case 0:
					break;				// wait for animation cycle to be completed


				case 2:
					// ai has played out its death anim there is no more except that lovely place where all creatures go
					AI_GetDyn(pMe)->m_dwStatusFlags |= AI_ALREADY_DEAD;
					if (waterFlag != AI_NOT_NEAR_WATER)
					{
						if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_FLOATINWATERONDEATH)
							AI_Standard_Float(pMe);
						else
							AI_Standard_Sink(pMe);
					}

					// ai should now be terminated
					break;

				default:
					if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_FLYING)
						AI_Standard_FlyingDeath(pMe);
					else
						AI_Standard_Death(pMe);
					break;

			}
		}
		else
		{
			if (waterFlag != AI_NOT_NEAR_WATER)
			{
				if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_FLOATINWATERONDEATH)
					AI_Standard_Float(pMe);
				else
					AI_Standard_Sink(pMe);
			}

			// does this ai - die instantly
			if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_INSTANTDEATH)
			{
				AI_GetDyn(pMe)->m_MeltTimer		= 0.0;
				AI_GetDyn(pMe)->m_FreezeTimer		= 0.0;
				pMe->m_cMelt							= MELT_LENGTH;
				CGameObjectInstance__SetGone(pMe);
			}
		}
	}


	// make teleporting ai appear
	AI_Make_Teleport_Appear(pMe);

	// invincibility only lasts until cycle is completed or advance runs at least once
	AI_GetDyn(pMe)->m_Invincible = FALSE;

	// leash test
	AI_UpdateLeashPos(pMe);

	// check for water gaff's
	if (		(AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_AVOIDWATER)
			&&	((waterFlag == AI_ON_WATERSURFACE) || (waterFlag == AI_BELOW_WATERSURFACE))
			&& (AI_GetDyn(pMe)->m_Health) )
	{
		// kill this ai
//		AI_CompletelyKillOff(pMe);
		AI_SEvent_InstantDeath(&pMe->ah.ih);
		return;
	}

	// does this ai regenerate ?
	if (		(AI_CanWeKillIt(pMe))
			&&	(pMe->m_cMelt >= MELT_LENGTH)
			&&	(AI_GetDyn(pMe)->m_Regenerate) )
	{
		// initialize ai do redo its interactive animation & fade in
		AI_GetDyn(pMe)->m_Regenerate--;											// one less regeneration
		AI_GetDyn(pMe)->m_cRegenerateAppearance = APPEARANCE_LENGTH*7/8;	// start completely faded out
		AI_GetDyn(pMe)->m_dwStatusFlags |= AI_REGENERATE;
		pMe->m_cMelt = 0;
		AI_GetDyn(pMe)->m_dwStatusFlags2 &= ~AI_REGENERATEAPPEARANCEDELAY;

		// regenerate from original start position
		if (AI_GetEA(pMe)->m_wTypeFlags3 & AI_TYPE3_REGENERATEFROMSTART)
		{
			AI_GetPos(pMe) = AI_GetDyn(pMe)->m_vStartPos;
			pMe->ah.ih.m_pCurrentRegion = AI_GetDyn(pMe)->m_pStartRegion;
			Regen = TRUE;
		}
		// make ai appear in a different place from where his leash center is
		else if (AI_DoTeleport(pMe, AI_GetDyn(pMe)->m_vLeashCoor))
			Regen = TRUE;
		else
			Regen = FALSE;


		// did regeneration end up in valid position ?
		if (Regen)
		{
			// resurrect this ai
			CAIDynamic__SetHealth(AI_GetDyn(pMe),
										 AI_GetEA(pMe),
										 CGameObjectInstance__TypeFlag(pMe));

			// setup random head, weapons, body parts etc...
			CAIDynamic__RandomizeHuman(AI_GetDyn(pMe), AI_GetEA(pMe), pMe) ;


			AI_GetDyn(pMe)->m_dwStatusFlags &= ~AI_INTERACTIVEANIMATION;
			AI_GetDyn(pMe)->m_InteractiveTimer = 2.0 + RandomFloat(3.0);
			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_INTERANIMDELAY;

			AI_GetDyn(pMe)->m_dwStatusFlags		&= ~AI_ALREADY_DEAD;
			AI_GetDyn(pMe)->m_dwStatusFlags		&= ~AI_WAITFOR_CYCLE;
			AI_GetDyn(pMe)->m_Agitation			= 0;
			AI_GetDyn(pMe)->m_PrevAgitation		= 0;
			AI_GetDyn(pMe)->m_dwStatusFlags		&= ~AI_WOUNDEDMORTALLY;
			AI_GetDyn(pMe)->m_dwStatusFlags2		&= ~AI_GENERATED_PICKUPS;

			AI_SetDesiredAnim(pMe, AI_ANIM_IDLE, TRUE);
			pMe->m_pAttackerCGameObjectInstance = NULL;

			CAIDynamic__SetMeltTiming(AI_GetDyn(pMe), AI_GetEA(pMe), CGameObjectInstance__TypeFlag(pMe));

			if (AI_GetEA(pMe)->m_dwTypeFlags & AI_TYPE_REPLAYINTANIMONREG)
			{
				AI_GetDyn(pMe)->m_dwStatusFlags |= AI_REGENERATEINTERANIM;

				// should this interactive anim hold on 1st frame ?
				if (AI_GetEA(pMe)->m_dwTypeFlags2 & AI_TYPE2_HOLDINTANIMON1STFRAME)
					AI_GetDyn(pMe)->m_dwStatusFlags2 |= AI_HOLDINTANIMON1STFRAME;
				else
					AI_GetDyn(pMe)->m_dwStatusFlags2 &= ~AI_HOLDINTANIMON1STFRAME;
			}
		}
	}

}




// set an multi ai event
//
void AI_Multi_Event(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, int event, CVector3 vPos, float Value)
{
	switch (event)
	{
		case AI_MEVENT_GETATTN:
			AI_MEvent_GetAttention (pAI, pOrigin, vPos );
			break;

		case AI_MEVENT_QUIETNOISE:
			AI_MEvent_QuietNoise ( pAI, vPos );
			break;

		case AI_MEVENT_LOUDNOISE:
			AI_MEvent_LoudNoise  ( pAI, vPos );
			break;

		case AI_MEVENT_HIT1:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 1, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT2:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 2, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT3:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 3, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT5:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 5, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT10:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 10, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT15:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 15, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT20:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 20, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT30:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 30, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT40:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 40, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT50:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 50, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_HIT100:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 100, Value, DAMAGE_NORMAL);
			break;

		case AI_MEVENT_PARTICLE_HIT1:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 1, Value, DAMAGE_PARTICLE);
			break;

		case AI_MEVENT_BLADEHIT1:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 1, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT2:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 2, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT3:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 3, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT5:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 5, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT10:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 10, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT15:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 15, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT20:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 20, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT30:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 30, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT40:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 40, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT50:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 50, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_BLADEHIT100:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 100, Value, DAMAGE_BLADE);
			break;

		case AI_MEVENT_KICKHIT1:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 1, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT2:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 2, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT3:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 3, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT5:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 5, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT10:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 10, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT15:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 15, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT20:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 20, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT30:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 30, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT40:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 40, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT50:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 50, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_KICKHIT100:
			AI_MEvent_Damage(pAI, pOrigin, vPos, 100, Value, DAMAGE_KICK);
			break;

		case AI_MEVENT_ENEMYEXPLOSION10:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 10, TRUE, TRUE, FALSE, 1.0, TRUE);
			break;

		case AI_MEVENT_ENEMYEXPLOSION15:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 15, TRUE, TRUE, FALSE, 1.0, TRUE);
			break;

		case AI_MEVENT_ENEMYEXPLOSION20:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 20, TRUE, TRUE, FALSE, 1.0, TRUE);
			break;

		case AI_MEVENT_EXPLOSION:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, AI_HITS_EXPLOSION, TRUE, FALSE, FALSE, 1.0, TRUE);
			break;

		case AI_MEVENT_EXPLOSION20:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 20, TRUE, FALSE, FALSE, 1.0, TRUE);
			break;

		case AI_MEVENT_EXPLOSION30:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 30, TRUE, FALSE, FALSE, 1.0, TRUE);
			break;

		case AI_MEVENT_EXPLOSION50:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 50, TRUE, FALSE, FALSE, 1.0, TRUE);
			break;

		case AI_MEVENT_EXPLOSION100:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 100, TRUE, FALSE, FALSE, 1.0, TRUE);
			break;

		case AI_MEVENT_SMALLEXPLOSION30:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 30, TRUE, FALSE, FALSE, 0.5, FALSE);
			break;

		// doesn't allow big creatures to be hurt
		case AI_MEVENT_EXPLOSIONSPECIAL:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 100, TRUE, FALSE, TRUE, 1.0, TRUE);
			break;

		case AI_MEVENT_WEAPONIMPACT:
			AI_MEvent_WeaponImpact (pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_NUDGEBACK:
			AI_MEvent_NudgeBack ( pAI, vPos, Value );
			break;

		case AI_MEVENT_SHOVEWITHCAMERA:
			AI_MEvent_ShoveWithCamera(pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_KNIFELEFT_DAMAGE:
			AI_MEvent_KnifeLeft_Damage (pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_KNIFERIGHT_DAMAGE:
			AI_MEvent_KnifeRight_Damage (pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_KNIFEFORWARD_DAMAGE:
			AI_MEvent_KnifeForward_Damage (pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_TOMAHAWKLEFT_DAMAGE:
			AI_MEvent_TomahawkLeft_Damage (pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_TOMAHAWKRIGHT_DAMAGE:
			AI_MEvent_TomahawkRight_Damage (pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_TOMAHAWKFORWARD_DAMAGE:
			AI_MEvent_TomahawkForward_Damage (pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_SMARTBOMB:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 1000, FALSE, FALSE, FALSE, 1.0, TRUE);
			break;

		case AI_MEVENT_SHOCKWAVE_DAMAGE:
			AI_MEvent_Shockwave_Damage(pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_PRESSUREPLATE:
			AI_MEvent_PressurePlate(pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_STATUE:
			AI_MEvent_Statue(pAI, pOrigin, vPos, Value);
			break;

		case AI_MEVENT_FREEZE:
			AI_MEvent_Freeze(pAI, pOrigin, vPos, Value, 40);
			break;

		case AI_MEVENT_SHOTGUNEXPLOSION:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 50, FALSE, FALSE, FALSE, 0.5, FALSE);
			break;

		case AI_MEVENT_TEKBOWEXPLOSION:
			AI_MEvent_Explosion(pAI, pOrigin, vPos, Value, 50, FALSE, FALSE, FALSE, 0.5, FALSE);
			break;


		// Boss events are special case -  a flag is simply set in the boss vars,
		// individual boss code must check for the flag and do the appropriate thing!
		case AI_MEVENT_BOSS1:
		case AI_MEVENT_BOSS2:
		case AI_MEVENT_BOSS3:
		case AI_MEVENT_BOSS4:
		case AI_MEVENT_BOSS5:
		case AI_MEVENT_BOSS6:
		case AI_MEVENT_BOSS7:
		case AI_MEVENT_BOSS8:

			if (		(pOrigin->m_Type == I_ANIMATED)
					&&	((CGameObjectInstance *)pOrigin)->m_pBoss )
			{
				((CGameObjectInstance *)pOrigin)->m_pBoss->m_EventFlags |= 1 << (event - AI_MEVENT_BOSS1) ;
			}
			break;


		default:
			TRACE("AI_Set_Event(): ERROR - Illegal event type: %i\n", event);
			//ASSERT (FALSE);
	}
}


// value is the ID to trigger
void AI_MEvent_PressurePlate(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value)
{
//	float		radiusSquared;
//	CVector3	vDelta;
//	float		radius ;
	int		nAnim;
	FLOAT		Id ;
	BOOL		Activated = FALSE ;
	BOOL		floorcheck ;

//	radius = 90 * SCALING_FACTOR;
//	radiusSquared = radius*radius ;

//	CVector3__Subtract(&vDelta, &pAI->ah.ih.m_vPos, &vPos);

	// Special case id checking types
	switch (CGameObjectInstance__TypeFlag(pAI))
	{
		// Special walls in MANTIS BOSS stage
		case AI_OBJECT_DEVICE_WALL:
			// Only mantis can trigger breakable walls, if on wall or flying
			//if ((nType == AI_OBJECT_CHARACTER_MANTIS_BOSS) &&
			if ((CInstanceHdr__TypeFlag(pOrigin) == AI_OBJECT_CHARACTER_MANTIS_BOSS) &&
				 (((CGameObjectInstance *)pOrigin)->m_pBoss->m_ModeFlags & (BF_LEFT_WALL_MODE | BF_RIGHT_WALL_MODE | BF_FLYING_MODE)))
			{
				// if already activated, then don't bother
				if ((pAI->m_AI.m_dwStatusFlags & AI_DOOROPEN) == 0)
				{
					// Wall ids are 1,2,3,4. Pressure id's are 10,11, 20,21, 30,31, 40,41.
					Id = AI_GetEA(pAI)->m_Id * 10 ;

					// Check for matching left right actions
					if (Id == Value)
					{
						WallAI_MEvent_Left(pAI);
						Activated = TRUE ;
					}
					else
					if ((Id+1) == Value)
					{
					 	WallAI_MEvent_Right(pAI);
						Activated = TRUE ;
					}

					// Update vars
					if (Activated)
					{
						// Update broken wall flags
						((CMantis *)((CGameObjectInstance *)pOrigin)->m_pBoss)->m_WallBroken[AI_GetEA(pAI)->m_Id - 1] = TRUE ;
						((CMantis *)((CGameObjectInstance *)pOrigin)->m_pBoss)->m_WallsToBreak-- ;
					}
				}
			}
			return ;
	}


	// SPECIAL CASE,
	// if ID = id of a door +100, close it (and player on ground)
	if ((AI_GetEA(pAI)->m_Id+100) == (short)Value)
	{
		switch (CGameObjectInstance__TypeFlag(pAI))
		{
			case AI_OBJECT_DEVICE_DOOR:
//				if ((CEngineApp__GetPlayer(GetApp()) == (CGameObjectInstance *)pOrigin) &&
//					(CInstanceHdr__IsOnGround(&CEngineApp__GetPlayer(GetApp())->ah.ih)))

				DoorAI_MEvent_OpenClose(pAI, FALSE);
				break ;

			// make crystal sink
			case AI_OBJECT_DEVICE_CRYSTAL:
				CrystalAI_MEvent_Start(pAI, FALSE);
				break ;
		}
	}


	// Id checking types
	if (AI_GetEA(pAI)->m_Id != (short)Value)
		return ;


	switch (CGameObjectInstance__TypeFlag(pAI))
	{
		case AI_OBJECT_DEVICE_DOOR:
			// only player can trigger doors (so MANTIS doesnt trigger crystal)
//			if (CEngineApp__GetPlayer(GetApp()) == (CGameObjectInstance *)pOrigin)

			DoorAI_MEvent_OpenClose(pAI, TRUE);
			break;

		case AI_OBJECT_DEVICE_ACTION:
		case AI_OBJECT_DEVICE_ANIMOFFSCREENACTION:
			// only player can trigger actions (so MANTIS doesnt trigger rising walls)
			//if (CEngineApp__GetPlayer(GetApp()) == (CGameObjectInstance *)pOrigin)
				ActionAI_MEvent_Go(pAI);
			break;

		case AI_OBJECT_DEVICE_TIMERACTION:
			TimerActionAI_MEvent_Go(pAI);
			break;

		case AI_OBJECT_DEVICE_PRESSUREACTION:
			PressureActionAI_MEvent_Go(pAI);
			break;
#if 0
// Gone now
		case AI_OBJECT_DEVICE_LONGHUNTER_SWITCH:
			LonghunterSwitchAI_MEvent_Go(pAI, Value);
			break;
#endif

		case AI_OBJECT_DEVICE_PORTAL:
			PortalAI_MEvent_Start(pAI);
			break;

		case AI_OBJECT_DEVICE_LOCK:
			LockAI_MEvent_Start(pAI);
			break;


		case AI_OBJECT_CHARACTER_RAPTOR:
		case AI_OBJECT_CHARACTER_HUMAN1:
		case AI_OBJECT_CHARACTER_SABERTIGER:
		case AI_OBJECT_CHARACTER_DIMETRODON:
		case AI_OBJECT_CHARACTER_TRICERATOPS:
		case AI_OBJECT_CHARACTER_MOSCHOPS:
		case AI_OBJECT_CHARACTER_PTERANODON:
		case AI_OBJECT_CHARACTER_SUBTERRANEAN:
		case AI_OBJECT_CHARACTER_LEAPER:
		case AI_OBJECT_CHARACTER_ALIEN:
		case AI_OBJECT_CHARACTER_HULK:
		case AI_OBJECT_CHARACTER_ROBOT:
		case AI_OBJECT_CHARACTER_BADFISH:
		case AI_OBJECT_CHARACTER_GOODFISH:
		case AI_OBJECT_CHARACTER_SLUDGEBOY:
		case AI_OBJECT_CHARACTER_PLANT:
		case AI_OBJECT_CHARACTER_ANCIENTWARRIOR:
		case AI_OBJECT_CHARACTER_HIGHPRIEST:
		case AI_OBJECT_CEILING_TURRET:
		case AI_OBJECT_BUNKER_TURRET:
		case AI_OBJECT_CHARACTER_GENERICRED:
		case AI_OBJECT_CHARACTER_GENERICGREEN:
		case AI_OBJECT_CHARACTER_GENERICMETAL:

			// pickups do not need to be on floor...
			floorcheck = TRUE ;
			if (pOrigin->m_Type == I_SIMPLE)
				floorcheck = FALSE ;
			if (AI_GetDyn(pAI)->m_dwStatusFlags & AI_INTERACTIVEANIMATION)
			{
				if ((floorcheck) && (CInstanceHdr__IsOnGround(&CEngineApp__GetPlayer(GetApp())->ah.ih)==FALSE))
					break ;

				AI_GetDyn(pAI)->m_dwStatusFlags &= ~AI_INTERACTIVEANIMATION;
				AI_GetDyn(pAI)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;

				if (!(AI_GetDyn(pAI)->m_dwStatusFlags & AI_INTERANIMDELAY))
				{
					nAnim = AI_GetEA(pAI)->m_InteractiveAnim - 1 + AI_ANIM_INTERACTIVE_ANIMATION1;
					AI_GetDyn(pAI)->m_dwStatusFlags2 &= ~AI_HOLDINTANIMON1STFRAME;
					AI_SetDesiredAnim(pAI, nAnim, TRUE);
					CGameObjectInstance__RestartAnim(pAI);
					//AI_GetDyn(pAI)->m_Invincible = TRUE;
					AI_GetDyn(pAI)->m_dwStatusFlags2 |= AI_VISIBLE;
				}
			}
			break ;


		// rotating doors
		//case AI_OBJECT_DEVICE_SPINDOOR:
		//	SpinDoorAI_MEvent_Spin(pAI);
		//	break;


		// laser
		case AI_OBJECT_DEVICE_LASER:
			// only player can trigger lasers
			if (CEngineApp__GetPlayer(GetApp()) == (CGameObjectInstance *)pOrigin)
				LaserAI_MEvent_Start(pAI);
			break;

		// elevators
		case AI_OBJECT_DEVICE_ELEVATOR:
		case AI_OBJECT_DEVICE_FOOTELEVATOR:
			ElevatorAI_MEvent_Start(pAI);
			break;
		// platforms
		case AI_OBJECT_DEVICE_PLATFORM:
			PlatformAI_MEvent_Start(pAI);
			break;
		// deathelevators
		case AI_OBJECT_DEVICE_DEATHELEVATOR:
			DeathElevatorAI_MEvent_Start(pAI);
			break;
		// collapsing platform
		case AI_OBJECT_DEVICE_COLLAPSINGPLATFORM:
			CollapsingPlatformAI_MEvent_Start(pAI);
			break;

// TREX ELEVATOR AND PLATFORMS NOW GONE!!
#if 0

		// trex elevator
		case AI_OBJECT_DEVICE_TREXELEVATOR:
			TrexElevatorAI_MEvent_Start(pAI);
			break;

		// trex platform
		case AI_OBJECT_DEVICE_TREXPLATFORM:
			TrexPlatformAI_MEvent_Start(pAI);
			break;
#endif


		// drain
		case AI_OBJECT_DEVICE_DRAIN:
			DrainAI_MEvent_Start(pAI);
			break;
		// flood
		case AI_OBJECT_DEVICE_FLOOD:
			FloodAI_MEvent_Start(pAI);
			break;

		// crystal
		case AI_OBJECT_DEVICE_CRYSTAL:
			// only player can trigger crystal on mantis level
			if (CEngineApp__GetPlayer(GetApp()) == (CGameObjectInstance *)pOrigin)
				CrystalAI_MEvent_Start(pAI, TRUE);
			break;
		// spin elevator
		case AI_OBJECT_DEVICE_SPINELEVATOR:
			SpinElevatorAI_MEvent_Start(pAI);
			break;
		// spin platform
		case AI_OBJECT_DEVICE_SPINPLATFORM:
			SpinPlatformAI_MEvent_Start(pAI);
			break;


		// FadeInFadeOut device
		case AI_OBJECT_DEVICE_FADEINFADEOUT:
			FadeInFadeOutAI_MEvent_ToggleState(pAI) ;
			break ;
	}
}

// an event to trigger any statues to become active
void AI_MEvent_Statue(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value)
{
	switch (CGameObjectInstance__TypeFlag(pAI))
	{
		// cacoon on MANTIS BOSS stage
		case AI_OBJECT_DEVICE_STATUE:
			// only player can trigger statue
			//if (CEngineApp__GetPlayer(GetApp()) == (CGameObjectInstance *)pOrigin)
				StatueAI_MEvent_Go(pAI);
			break;
	}
}

void AI_SEvent_GeneratePickups(CInstanceHdr *pThis, CVector3 vPos)
{
	CGameObjectInstance *pAnim;


	// no pickups on HARD skill
	if (GetApp()->m_Difficulty == HARD)
		return ;

	if (pThis->m_Type == I_ANIMATED)
	{
		pAnim = (CGameObjectInstance*) pThis;

		if (		!(pAnim->m_AI.m_dwStatusFlags & AI_ALREADY_DEAD)
				&& !(AI_GetDyn(pAnim)->m_dwStatusFlags2 & AI_GENERATED_PICKUPS) )
		{
			// generate pickups only once
			AI_GetDyn(pAnim)->m_dwStatusFlags2 |= AI_GENERATED_PICKUPS;

			AI_GeneratePickups(pAnim, vPos);
		}
	}
}

void AI_SEvent_Generate_Pickup(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	CGameObjectInstance *pAnim;

	if (pThis->m_Type == I_ANIMATED)
	{
		pAnim = (CGameObjectInstance*) pThis;

//		if (!(pAnim->m_AI.m_dwStatusFlags & AI_ALREADY_DEAD))

		//AI_DoPickup(&pAnim->ah.ih, pAnim->ah.ih.m_vPos, (int)Value, 0);
		AI_DoPickup(&pAnim->ah.ih, vPos, (int)Value, 0);
	}
}

void AI_SEvent_MakeInvisible(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	CGameObjectInstance *pAnim;

	if (pThis->m_Type == I_ANIMATED)
	{
		pAnim = (CGameObjectInstance*) pThis;

		if (pAnim->m_Mode != TRANS_FADE_OUT_MODE)
		{
			pAnim->m_Mode = TRANS_FADE_OUT_MODE;
			pAnim->m_ModeTime = MIN_TRANS;
			pAnim->m_ModeMisc1 = (MAX_TRANS - MIN_TRANS)/SECONDS_TO_FRAMES(Value);
		}
	}
}

//	"Acid 'incoming'     Sound     (Sub)",
void AI_SEvent_AcidIncoming (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Acid 'incoming' - Sound (Sub) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_SUBTERRANEAN_ACID_ATTACK, 1, 0);
}

//	"Acid Spit           Particles (Sub)",
void AI_SEvent_AcidSpitPart (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	// sound is in "AI_SEvent_AcidIncoming"
	TRACE0("S Event Acid Spit - Particle (Sub)\r\n");
	AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ACIDSPIT, vPos);
}

//	"Acid Spit           Sound     (Sub)",
void AI_SEvent_AcidSpitSnd (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
		// sound is in "AI_SEvent_AcidIncoming"
	TRACE1("S Event Acid Spit - Sound (Sub) Volume = %f\r\n", Value);

}

//	"Blowgun Fire        Sound     (Ancient)",
void AI_SEvent_BlowGunFire (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Blowgun - Sound (Ancient) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_BLOW_GUN_SHOT, 1, 0);
}

//	"Breath              Particles (TRex)",
void AI_SEvent_BreathPart (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Breath - Particle (TRex)\r\n");
//	AI_DoParticle(pThis, PARTICLE_TYPE_BREATH, vPos);
	AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_BREATH, vPos);

//	AI_AimParticleAtPlayer(pThis, PARTICLE_TYPE_BREATH, vPos, ANGLE_DTOR(15)) ;
	AI_SEvent_StartFireSound(pThis, vPos, Value) ;
}

//	"Breath              Sound     (TRex)",
void AI_SEvent_BreathSnd (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Breath - Sound (TRex) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_TREX_FIRE_BREATHE, 1, 0);
}

//	"Charge/Head Slam    Sound     (Creature)",
void AI_SEvent_ChargeHeadSlam (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Charge Head Slam - Sound (Creature) Volume = %f\r\n", Value);
}

//	"Death Rain          Particles (Mantis)",
void AI_SEvent_DeathRainPart (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Death Rain - Particle (Mantis)\r\n");
	AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_DEATHRAIN, vPos);
}

//	"Death Rain          Sound     (Mantis)",
void AI_SEvent_DeathRainSnd (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Death Rain - Sound (Mantis) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_MANTIS_DEATH_RAIN, 1, 0);
}

//	"Earthquake          Sound     (TRex)",
void AI_SEvent_Earthquake (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Earthquake - Sound (TRex) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_SUBTERRANEAN_EARTH_RUMBLE, 1, 0);
}

//	"Energy Blast        Particles (Mantis)",
void AI_SEvent_EnergyBlastPart (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE0("S Event Energy Blast - Particle (Mantis)\r\n");
			AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ENERGYBLAST, vPos);
			break;
	}
}

//	"Energy Blast        Sound     (Mantis)",
void AI_SEvent_EnergyBlastSnd (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Energy Blast - Sound (Mantis) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_EXPLOSION_1, 1, 0);
}

//	"Explosive Launched  Sound     (Weapon)",
void AI_SEvent_ExplosiveLaunched (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Explosive Launched - Sound (Weapon) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_GRENADE_LAUNCH, 1, 0);
}

//	"Eyefire             Particles (TRex)",
void AI_SEvent_EyeFirePart (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Eyefire - Particle (TRex)\r\n");
	AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_EYEFIRE, vPos) ;
//	AI_AimParticleAtPlayer(pThis, PARTICLE_TYPE_EYEFIRE, vPos, ANGLE_DTOR(15)) ;
}

//	"Eyefire             Sound     (TRex)",
void AI_SEvent_EyeFireSound (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Eye Fire - Sound (TRex) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_TREX_EYE_LASER_FIRE, 1, 0);
}

//	"Fireball            Particles (Mantis)",
void AI_SEvent_FireBallPart (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	CGameObjectInstance *pAnimInst = (CGameObjectInstance *)pThis ;

	TRACE0("S Event Fireball - Particle (Mantis)\r\n") ;

	// Special case for mantis
	if ((pThis->m_Type == I_ANIMATED) &&
		 (CGameObjectInstance__TypeFlag(pAnimInst) == AI_OBJECT_CHARACTER_MANTIS_BOSS) &&
		 (!Mantis_CanSpit(pAnimInst, (CMantis *)pAnimInst->m_pBoss)))
		return ;

	AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_FIREBALL, vPos) ;
}

//	"Fireball            Sound     (Mantis)",
void AI_SEvent_FireBallSnd (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Fire Ball - Sound (Mantis) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_EXPLOSION_1, 1, 0);
}

//	"Jaws Snap           Sound     (Creature)",
void AI_SEvent_JawSnap (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Jaw Snap - Sound (Creature) Volume = %f\r\n", Value);
}

//	"Shockwave           Particles (Hulk)",
void AI_SEvent_ShockWavePart (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Shock Wave - Particle (Hulk)\r\n");
	AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_SHOCKWAVE, vPos);
}

//	"Shockwave           Sound     (Hulk)",
void AI_SEvent_ShockWaveSnd (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Shock Wave - Sound (Hulk) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_EXPLOSION_2, 1, 0);
}

//	"Spell Cast          Particles (Priest)",
void AI_SEvent_SpellCastPart (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Spell Cast - Particle (Priest)\r\n");
	AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_SPELLCAST, vPos);
}

//	"Spell Cast          Sound     (Priest)",
void AI_SEvent_SpellCastSnd (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Spell Cast - Sound (Priest) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_HIGH_PRIEST_SPELL_ATTACK, 1, 0);
}

//	"Swing fists/weapon  Sound     (Enemy)",
void AI_SEvent_SwingFistsWeapon (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Swing fists / Weapon - Sound (Enemy) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_KNIFE_SWISH_1, 1, 0);
}

//	"Weapon Round         Particles (Weapon)",
void AI_SEvent_WeaponRound (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Weapon Round - Particles (Weapon) Volume = %f\r\n", Value);

	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_PLAYER:
			break;

		case AI_OBJECT_CHARACTER_RAPTOR:
			AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_RAPTOR_PROJECTILE, vPos);
			AI_DoParticle(pThis, PARTICLE_TYPE_ENEMY_MUZZLEFLASH, vPos);
			AI_DoSound(pThis, SOUND_RAPTOR_LASER_FIRE, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HUMAN1:
			if (	(pThis->m_pEA->m_AttackStyle == 8)
				|| (pThis->m_pEA->m_AttackStyle == 27))
			{
				// do seargent longhunter particle
				AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ENEMY_LONGHUNTER, vPos);
				AI_DoSound(pThis, SOUND_MACHINE_GUN_SHOT_2, 1, 0);
			}
			else
			{
				AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ENEMY_BULLET, vPos);
				AI_DoParticle(pThis, PARTICLE_TYPE_ENEMY_MUZZLEFLASH, vPos);
				AI_DoSound(pThis, SOUND_GENERIC_8, 1, 0);
			}
			break;

		case AI_OBJECT_CHARACTER_SABERTIGER:
			break;

		case AI_OBJECT_CHARACTER_GENERICRED:
			break;

		case AI_OBJECT_CHARACTER_GENERICGREEN:
			break;

		case AI_OBJECT_CHARACTER_GENERICMETAL:
			break;

		case AI_OBJECT_CHARACTER_DIMETRODON:
			break;

		case AI_OBJECT_CHARACTER_TRICERATOPS:
			switch((int)Value)
			{
				case 0:
					AI_DoSound(pThis, SOUND_MISSILE_LAUNCH, 1, 0);
					AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ROBOT_ROCKET, vPos);
					break;

				case 1:
					AI_DoSound(pThis, SOUND_AUTO_SHOTGUN_SHOT, 1, 0);
					AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ROBOT_BULLET, vPos);
					break;
			}
			break;

		case AI_OBJECT_CHARACTER_MOSCHOPS:
			break;

		case AI_OBJECT_CHARACTER_PTERANODON:
			break;

		case AI_OBJECT_CHARACTER_SUBTERRANEAN:
			break;

		case AI_OBJECT_CHARACTER_LEAPER:
			break;

		case AI_OBJECT_CHARACTER_ALIEN:
			AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ENEMY_PLASMA1, vPos);
			AI_DoParticle(pThis, PARTICLE_TYPE_ENEMY_MUZZLEFLASH, vPos);
			AI_DoSound(pThis, SOUND_TEK_WEAPON_1, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HULK:
			AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_HULK_BLASTER, vPos);
			AI_DoSound(pThis, SOUND_EXPLOSION_1, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
			AI_DoSound(pThis, SOUND_MACHINE_GUN_SHOT_2, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
			AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ROBOT_ROCKET, vPos);
			AI_DoSound(pThis, SOUND_CAMPAIGNERS_SCATTER_BLAST, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ROBOT:
			switch((int)Value)
			{
				case 0:
					AI_DoSound(pThis, SOUND_MISSILE_LAUNCH, 1, 0);
					AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ROBOT_ROCKET, vPos);
					break;

				case 1:
					AI_DoSound(pThis, SOUND_AUTO_SHOTGUN_SHOT, 1, 0);
					AI_DoParticleAtPlayer(pThis, PARTICLE_TYPE_ROBOT_BULLET, vPos);
					break;
			}
			break;

		case AI_OBJECT_CHARACTER_BADFISH:
			break;

		case AI_OBJECT_CHARACTER_GOODFISH:
			break;
	}
}

//	"Weapon Fire         Particles/Sound     (Weapon)",
void AI_SEvent_WeaponFire (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Weapon Fire - Sound (Weapon) Volume = %f\r\n", Value);
}

//	"Weapon Cock         Sound     (Weapon)",
void AI_SEvent_WeaponCock (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Weapon Cock - Sound (Weapon) Volume = %f\r\n", Value);
			break;
	}
}

//	"Get Attention       AI        -"
void AI_MEvent_GetAttention(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos)
{
	CGameObjectInstance	*pAnimOrigin;

	TRACE0("M Event Get Attention - AI\r\n");

	/* did ai hear a get attention */
	if ( AI_GetEA(pAI)->m_LoudRadius >= AI_DistanceSquared(pAI, vPos) )
	{
		AI_GetDyn(pAI)->m_dwStatusFlags |= AI_EV_GETATTN;
		AI_Increase_Agitation ( pAI, AI_AGITATION_GETATTN, 300 );

		if (pOrigin->m_Type == I_ANIMATED)
		{
			pAnimOrigin = (CGameObjectInstance*) pOrigin;

			if (AI_GetEA(pAnimOrigin)->m_dwTypeFlags & AI_TYPE_LEADER)
			{
				pAI->m_pEventCGameObjectInstance = pAnimOrigin;
			}
		}
	}
	else
	{
		AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_EV_GETATTN);
	}
}

//	"Quiet Noise         AI        -",
void AI_MEvent_QuietNoise(CGameObjectInstance *pAI, CVector3 vPos)
{
	TRACE0("M Event Quiet Noise - AI\r\n");


	// did ai hear quiet noise
	if ( AI_GetEA(pAI)->m_QuietRadius >= AI_DistanceSquared(pAI, vPos) )
	{
		AI_GetDyn(pAI)->m_dwStatusFlags |= AI_EV_QUIETNOISE;
		AI_Increase_Agitation ( pAI, AI_AGITATION_QUIETNOISE, 300 );
	}
	else
		AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_EV_QUIETNOISE);
}

//	"Loud Noise          AI        -",
void AI_MEvent_LoudNoise(CGameObjectInstance *pAI, CVector3 vPos)
{
	TRACE0("M Event Loud Noise - AI\r\n");


	// did ai hear loud noise
	if ( AI_GetEA(pAI)->m_LoudRadius >= AI_DistanceSquared(pAI, vPos) )
	{
		AI_GetDyn(pAI)->m_dwStatusFlags |= AI_EV_LOUDNOISE;
		AI_Increase_Agitation ( pAI, AI_AGITATION_LOUDNOISE, 300 );
	}
	else
		AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_EV_LOUDNOISE);
}

//	"Look Offset <deg>   AI        -",
void AI_SEvent_LookOffset (CInstanceHdr *pThis, float Value )
{
	CGameObjectInstance *pAnim;

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			pAnim = (CGameObjectInstance*) pThis;
			TRACE1("S Event Look Offset - AI (Any) Degrees = %f\r\n", ANGLE_DTOR(Value) );
			AI_GetDyn(pAnim)->m_ViewAngleOffset = ANGLE_DTOR(Value);
			break;
	}
}


//	"Damage   1, 2, 3, 5, 10, 15, 20, 30, 40, 50, 100 pt       AI        -",
void AI_MEvent_Damage (CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, int hits, float rad, int nType)
{
	INT32	Impact = FALSE ;
	CGameObjectInstance	*pAttacker ;
	CMantis					*pMantis = NULL ;

	// Setup attacker
	switch (pOrigin->m_Type)
	{
		case I_PARTICLE:
			break;

		case I_ANIMATED:
			pAttacker = (CGameObjectInstance*) pOrigin ;
			break;

		default:
			pAttacker = NULL ;
	}

	// Can't hurt self - added by S.B. 11/30/96
	if (&pAI->ah.ih == pOrigin)
		return ;

	// Is this the mantis?
	if ((nType == DAMAGE_PARTICLE) && (CGameObjectInstance__TypeFlag(pAttacker) == AI_OBJECT_CHARACTER_MANTIS_BOSS))
		pMantis = (CMantis *)pAttacker->m_pBoss ;

	// Only hurt player once a frame if this is a mantis steamy slime
	if ((pMantis) && ((pMantis->m_AcidHurtTurok) || (game_frame_number & 3)))
		return ;

	TRACE2("M Event Damage %d - AI (Any) Radius = %f\r\n", hits, rad);

	// did ai get hit
	rad *= SCALING_FACTOR;

	// did player get hit or was it another ai ?
	if ( pAI == CEngineApp__GetPlayer(GetApp()) )
	{
		// hit player (do fudge on distance /2 - to get ai hitting turok without getting too close)
		if ( (sqrt(AI_DistanceSquared(pAI, vPos))/2) <= ( rad + AI_GetEA(pAI)->m_CollisionRadius) )
		{
			AI_DoHit(pAI, hits);
			AI_SetAttacker(pAI, pOrigin);
			Impact = TRUE ;
		}
	}
	else
	{
		// hit another ai (do normal proper test between two ai's)
		if ( (sqrt(AI_DistanceSquared(pAI, vPos))/2) <= ( rad + AI_GetEA(pAI)->m_CollisionRadius) )
		{
			if (		(pAI->ah.ih.m_Type == I_ANIMATED)
					&&	(pOrigin->m_nObjType != pAI->ah.ih.m_nObjType) )
			{
				AI_DoHit(pAI, hits);
				AI_SetAttacker(pAI, pOrigin);
				Impact = TRUE ;
			}
		}
	}

	// any sfx on this impact ?
	if (Impact)
	{
		switch(nType)
		{
			case DAMAGE_NORMAL:
				// Boss impact effect?
				if ((pOrigin->m_Type == I_ANIMATED) && (((CGameObjectInstance *)pOrigin)->m_pBoss))
				{
					Impact = ((CGameObjectInstance *)pOrigin)->m_pBoss->m_ModeFlags ;
					if (Impact & (BF_KICK_IMPACT_MODE | BF_PUNCH_IMPACT_MODE))
						AI_DoSound(&pAI->ah.ih, 466, 1, RANDOMIZE_PITCH);
				}
				break;

			case DAMAGE_BLADE:
				AI_DoSound(&pAI->ah.ih, SOUND_TOMAHAWK_IMPACT_FLESH, 1, RANDOMIZE_PITCH);
				break;

			case DAMAGE_KICK:
				AI_DoSound(&pAI->ah.ih, SOUND_GENERIC_81, 1, RANDOMIZE_PITCH);
				break;

			case DAMAGE_PARTICLE:
				if (pMantis)
				{
					pMantis->m_AcidHurtTurok = TRUE ;
					if (!(game_frame_number & 3))
						AI_DoSound(&pAI->ah.ih, SOUND_GENERIC_95, 1, RANDOMIZE_PITCH);
				}
				break;
		}
	}
}




#define EXPLOSION_SCALE_FACTOR 0.01

//	"Explosion -",
void AI_MEvent_Explosion(CGameObjectInstance	*pAI,
								 CInstanceHdr			*pOrigin,
								 CVector3				vPos,
								 float					rad,
								 float					ExpHits,
								 BOOL						HurtPlayer,
								 BOOL						EnemyFired,
								 BOOL						DontHurtBigCreatures,
								 float					ForceMultiplier,
								 BOOL						AllowDamageToDevices)
{
	// declare variables
	float				rad2, damage, hits;
	CVector3			vAIPos,
						vExp;
//	CGameRegion		*pRegion;
	CVector3			vCurrentPos;

	// display debug info
	TRACE1("M Event Explosion - AI (Any) Radius = %f\r\n", rad);


	if (!(AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_COLLISION))
		return;

	// scale rad
	rad *= SCALING_FACTOR;
	rad *= rad;

	// did ai get hit by explosion
	rad2 = AI_DistanceSquared(pAI, vPos);
	if (rad2 < rad)
	{
//		pRegion = pAI->ah.ih.m_pCurrentRegion;
		vCurrentPos = pAI->ah.ih.m_vPos;

		// check to see if explosion is blocked
		if (CAnimInstanceHdr__CastYeThyRay(&pAI->ah, pOrigin))
		{
			// decrease ai's health depending on how close they are to the explosion
			damage = (rad - rad2)/rad;
			damage = sqrt(damage);
			hits = ExpHits * damage;

			// do the explosion
			if (!(AI_GetDyn(pAI)->m_Invincible))
			{
				AI_GetDyn(pAI)->m_dwStatusFlags |= AI_EV_EXPLOSION;
				AI_Increase_Agitation(pAI, AI_AGITATION_EXPLOSION, 300);

				// calculate explosion vector effect on ai
				vAIPos = AI_GetPos(pAI);

				CVector3__Subtract(&vExp, &vAIPos, &vPos);
				vExp.y = 0;
				CVector3__Normalize(&vExp);

				AI_SetAttacker(pAI, pOrigin);

				// set explosion magnitude
				damage *= 50;
				damage *= ForceMultiplier;
				vExp.x *= damage;
				vExp.z *= damage;

				// do different explosion effects based on type
				switch (CGameObjectInstance__TypeFlag(pAI))
				{

					// to be exploded fully
					case AI_OBJECT_CHARACTER_HUMAN1:
					case AI_OBJECT_CHARACTER_PLAYER:
					case AI_OBJECT_CHARACTER_RAPTOR:
					case AI_OBJECT_CHARACTER_SABERTIGER:
						vExp.y  = damage*0.85 + ((RANDOM(3)+1)*SCALING_FACTOR);
						break;

					// to be exploded only through animation
					case AI_OBJECT_CHARACTER_MOSCHOPS:
					case AI_OBJECT_CHARACTER_DIMETRODON:
					case AI_OBJECT_CHARACTER_HULK:
					case AI_OBJECT_CHARACTER_ROBOT:
					case AI_OBJECT_CHARACTER_LEAPER:
					case AI_OBJECT_CHARACTER_ALIEN:
					case AI_OBJECT_CHARACTER_GENERICRED:
					case AI_OBJECT_CHARACTER_GENERICGREEN:
					case AI_OBJECT_CHARACTER_GENERICMETAL:
						vExp.y = 0;
						break;

					case AI_OBJECT_CHARACTER_PTERANODON:
					case AI_OBJECT_CHARACTER_TRICERATOPS:
						vExp.x = 0;
						vExp.y = 0;
						vExp.z = 0;
						break;

					case AI_OBJECT_CHARACTER_TREX_BOSS:
					case AI_OBJECT_CHARACTER_MANTIS_BOSS:
					case AI_OBJECT_CHARACTER_SUBTERRANEAN:
						vExp.x = 0;
						vExp.y = 0;
						vExp.z = 0;
						if (DontHurtBigCreatures)
							hits = 0;
						break;

					case AI_OBJECT_DEVICE_EXPTARGET:
						vExp.x = 0;
						vExp.y = 0;
						vExp.z = 0;
						if (AllowDamageToDevices)
						{
							// device already exploded ?
							if (!(AI_GetDyn(pAI)->m_dwStatusFlags & AI_EXPTARGETEXPLODED))
								ExplosiveTarget_ParticleCollision(pAI);
						}
						break;

					case AI_OBJECT_CEILING_TURRET:
					case AI_OBJECT_BUNKER_TURRET:
					default:
						vExp.x = 0;
						vExp.y = 0;
						vExp.z = 0;
						break;
				}

//				// remove any velocity for exploding enemies in german version
//				#ifdef	GERMAN
//				vExp.x = 0;
//				vExp.y = 0;
//				vExp.z = 0;
//				#endif

				// store explosion vector
				vExp.x *= 15;
				vExp.y *= 15;
				vExp.z *= 15;
				AI_GetDyn(pAI)->m_vExplosion = vExp;
			}

			if (!CGameObjectInstance__IsDevice(pAI))
			{
				// did the player cause this explosion or did an enemy ?
				if (EnemyFired)
				{
					// enemy caused this explosion
					if (HurtPlayer || pAI != CEngineApp__GetPlayer(GetApp()))
						AI_Decrease_Health(pAI, hits, TRUE, TRUE);
				}
				else
				{
					// player caused this explosion - if player is hit do less damage
					if (pAI == CEngineApp__GetPlayer(GetApp()))
					{
						if (HurtPlayer)
							AI_Decrease_Health(pAI, (hits*0.25), TRUE, TRUE);
					}
					else
						AI_Decrease_Health(pAI, hits, TRUE, TRUE);
				}

				// was ai not killed ?
				if (pAI != CEngineApp__GetPlayer(GetApp()))
				{
					if (AI_GetDyn(pAI)->m_Health > 0)
					{
						AI_GetDyn(pAI)->m_dwStatusFlags &= ~AI_EV_EXPLOSION;
						vExp.x = 0;
						vExp.y = 0;
						vExp.z = 0;
						AI_GetDyn(pAI)->m_vExplosion = vExp;
					}
				}
			}
		}
	}
}



//	"Freeze -",
void AI_MEvent_Freeze(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float rad, float ExpHits)
{
	float			rad2, BossHits = rad * 3 ;

	// display debug info
	TRACE1("M Event Freeze - AI (Any) Radius = %f\r\n", rad);

	// if this ai is melting away dont allow freezing
	if (AI_CanWeKillIt(pAI))
		return;

	// scale rad
	rad *= SCALING_FACTOR;
	rad *= rad;

	// did ai get hit by explosion
	rad2 = AI_DistanceSquared(pAI, vPos);
	if (rad2 < rad)
	{
		// check to see if explosion is blocked
//		if (!CAnimInstanceHdr__IsObstructed(&pAI->ah, vPos, NULL))
//		{
			// increase ai's agitation
			AI_Increase_Agitation(pAI, AI_AGITATION_EXPLOSION, 300);
			AI_SetAttacker(pAI, pOrigin);

			// make sure it only freezes creatures
			switch (CGameObjectInstance__TypeFlag(pAI))
			{
				// creatures that need freezing
				case AI_OBJECT_CHARACTER_RAPTOR:
				case AI_OBJECT_CHARACTER_HUMAN1:
				case AI_OBJECT_CHARACTER_SABERTIGER:
				case AI_OBJECT_CHARACTER_TRICERATOPS:
				case AI_OBJECT_CHARACTER_PTERANODON:
				case AI_OBJECT_CHARACTER_MOSCHOPS:
				case AI_OBJECT_CHARACTER_DIMETRODON:
				case AI_OBJECT_CHARACTER_HULK:
				case AI_OBJECT_CHARACTER_ROBOT:
				case AI_OBJECT_CHARACTER_LEAPER:
				case AI_OBJECT_CHARACTER_ALIEN:
				case AI_OBJECT_CEILING_TURRET:
				case AI_OBJECT_BUNKER_TURRET:
				case AI_OBJECT_CHARACTER_SUBTERRANEAN:
				case AI_OBJECT_CHARACTER_GENERICRED:
				case AI_OBJECT_CHARACTER_GENERICGREEN:
				case AI_OBJECT_CHARACTER_GENERICMETAL:
					// don't hit player
					if (pAI != CEngineApp__GetPlayer(GetApp()))
					{
						AI_Decrease_Health(pAI, ExpHits, TRUE, TRUE);

						// if enemy died freeze it
						if (		(AI_GetDyn(pAI)->m_Health == 0)
								&&	(AI_GetDyn(pAI)->m_FreezeTimer == 0.0) )
						{
							AI_GetDyn(pAI)->m_FreezeTimer = AI_FREEZE_START;
							AI_DoParticle(&pAI->ah.ih, PARTICLE_TYPE_FREEZE_START, AI_GetPos(pAI));
						}
					}
					break;

				// Hurt the Bosses
				case AI_OBJECT_CHARACTER_TREX_BOSS:
				case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
				case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
				case AI_OBJECT_CHARACTER_MANTIS_BOSS:
				case AI_OBJECT_CHARACTER_HUMVEE_BOSS:
					AI_Decrease_Health(pAI, BossHits, TRUE, TRUE) ;
					break ;

				default:
					break;
			}
//		}
	}
}


//	"Weapon Impact (Any)",
void AI_MEvent_WeaponImpact(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float rad)
{
	// declare variables
	float			rad2, damage, mag;
	CVector3		vAIPos,
					vExp,
//					vPlayerPos,
					vEnemy;


	// display debug info
	TRACE1("M Event Weapon Impact (Any) Radius = %f\r\n", rad);

	// scale rad
	rad *= SCALING_FACTOR;
	rad *= rad;

	// did ai get hit by explosion
	rad2 = AI_DistanceSquared ( pAI, vPos );
	if ( rad >= rad2 )
	{
		AI_Increase_Agitation ( pAI, AI_AGITATION_EXPLOSION, 300 );

		// calculate explosion vector effect on ai
		vAIPos = AI_GetPos  ( pAI );
		CVector3__Subtract  ( &vExp, &vAIPos, &vPos );
		vExp.y = 0;
		CVector3__Normalize ( &vExp );

		// decrease ai's health depending on how close they are to the explosion
		damage = rad - rad2;
		if ( damage < 0 ) damage = 0;

		// set explosion magnitude
		damage = sqrt(damage);
		damage *= 0.3;
		vExp.x *= damage;
		vExp.z *= damage;

		// get distance from player to this ai
//		vPlayerPos = AI_GetPos(CEngineApp__GetPlayer(GetApp()));
		vEnemy     = AI_GetPos(pAI);
		CVector3__Subtract(&vEnemy, &vPos, &vEnemy);
		mag = CVector3__Magnitude(&vEnemy);

		// reduce weapon impact effect over distance
		if (mag>=650) mag = 650;
		mag = mag * 10 / 650;
		mag *= mag;
		if (mag<1) mag = 1;

		// do different explosion effects based on type
		switch (CGameObjectInstance__TypeFlag(pAI))
		{
			// to be exploded fully
			case AI_OBJECT_CHARACTER_PLAYER:
			case AI_OBJECT_CHARACTER_RAPTOR:
			case AI_OBJECT_CHARACTER_HUMAN1:
			case AI_OBJECT_CHARACTER_SABERTIGER:
			case AI_OBJECT_CHARACTER_TRICERATOPS:
			case AI_OBJECT_CHARACTER_PTERANODON:
			case AI_OBJECT_CHARACTER_LEAPER:
			case AI_OBJECT_CHARACTER_ALIEN:
			case AI_OBJECT_CHARACTER_GENERICRED:
			case AI_OBJECT_CHARACTER_GENERICGREEN:
			case AI_OBJECT_CHARACTER_GENERICMETAL:
				vExp.y = (vExp.y * damage);
				break;

			// to be exploded only through animation
			case AI_OBJECT_CHARACTER_DIMETRODON:
				vExp.y = 0;
				break;

			case AI_OBJECT_CHARACTER_HULK:
				vExp.x/=6;
				vExp.z/=6;
				vExp.y=0;
				break;

			// does not move from impacts
			default:
			case AI_OBJECT_CHARACTER_BADFISH:
			case AI_OBJECT_CHARACTER_GOODFISH:
			case AI_OBJECT_CHARACTER_SUBTERRANEAN:
			case AI_OBJECT_CHARACTER_MOSCHOPS:
			case AI_OBJECT_CHARACTER_ROBOT:
			case AI_OBJECT_CEILING_TURRET:
			case AI_OBJECT_BUNKER_TURRET:
				vExp.x = 0;
				vExp.y = 0;
				vExp.z = 0;
				break;

		}

		// set impact vector
		vExp.x *= 15;
		vExp.y *= 15;
		vExp.z *= 15;
		AI_GetVelocity(pAI)->x=(vExp.x/mag);
		AI_GetVelocity(pAI)->y=(vExp.y/mag);
		AI_GetVelocity(pAI)->z=(vExp.z/mag);

		// set attacker
		AI_SetAttacker(pAI, pOrigin);
	}
}

//	"'Test'              Particles -",
void AI_SEvent_Test (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	CEngineApp				*pApp;
	CVector3					vParticlePos;
	CGameRegion				*pParticleRegion;
	CGameObjectInstance	*pAnim;


	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			pAnim = (CGameObjectInstance*) pThis;
			TRACE1("S Event 'Test' - Particles (Any) Value = %f\r\n", Value);

			// get pointer to engine app
			pApp = GetApp();
			if ( pApp == NULL ) return;

			// get particle position
			CInstanceHdr__GetNearPositionAndRegion(&pAnim->ah.ih, vPos, &vParticlePos, &pParticleRegion, INTERSECT_BEHAVIOR_STICK, INTERSECT_BEHAVIOR_STICK);

			CParticleSystem__CreateParticle(&pApp->m_Scene.m_ParticleSystem, pThis, PARTICLE_TYPE_TEST,
													  pAnim->ah.m_vVelocity,
													  CGameObjectInstance__GetAimRotation(pAnim),
													  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, TRUE);
			break;
	}
}

//	"Surface Impact      Particles (Any)",
void AI_SEvent_SurfaceImpact (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	int						nType;
	CGameObjectInstance	*pAnim;

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			pAnim = (CGameObjectInstance*) pThis;
			TRACE0("S Event Surface Impact - Particle (Any)\r\n");

			// which character is standing on what ?
			switch (CGameObjectInstance__TypeFlag(pAnim))
			{
				case AI_OBJECT_CHARACTER_PLAYER:
					break;

				case AI_OBJECT_CHARACTER_RAPTOR:
					break;

				case AI_OBJECT_CHARACTER_GENERICRED:
					break;

				case AI_OBJECT_CHARACTER_GENERICGREEN:
					break;

				case AI_OBJECT_CHARACTER_GENERICMETAL:
					break;

				case AI_OBJECT_CHARACTER_HUMAN1:
					break;

				case AI_OBJECT_CHARACTER_SABERTIGER:
					break;

				case AI_OBJECT_CHARACTER_DIMETRODON:
					break;

				case AI_OBJECT_CHARACTER_TRICERATOPS:
					break;

				case AI_OBJECT_CHARACTER_MOSCHOPS:
					break;

				case AI_OBJECT_CHARACTER_PTERANODON:
					break;

				case AI_OBJECT_CHARACTER_SUBTERRANEAN:
					break;

				case AI_OBJECT_CHARACTER_LEAPER:
					break;

				case AI_OBJECT_CHARACTER_ALIEN:
					break;

				case AI_OBJECT_CHARACTER_HULK:
					break;

				case AI_OBJECT_CHARACTER_ROBOT:
					break;

				case AI_OBJECT_CHARACTER_BADFISH:
					break;

				case AI_OBJECT_CHARACTER_GOODFISH:
					break;

				case AI_OBJECT_CHARACTER_MANTIS_BOSS:
					break;

				case AI_OBJECT_CEILING_TURRET:
				case AI_OBJECT_BUNKER_TURRET:
					break ;

			}

			if (Value)
				nType = PARTICLE_TYPE_DUSTCLOUD_BODYFALL;
			else
				nType = PARTICLE_TYPE_DUSTCLOUD_FOOTFALL;

			AI_DoParticle(pThis, nType, vPos);
			break;
		}
}


//	"'Chitter'           Sound     (Leaper)",
void AI_SEvent_Chitter (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	int						n, s;

	TRACE1("S Event 'Chitter' - Sound (Leaper) Volume = %f\r\n", Value);

	// which character is standing on what ?
	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_PLAYER:
			break;

		case AI_OBJECT_CHARACTER_RAPTOR:
			AI_DoSound(pThis, SOUND_RAPTOR_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HUMAN1:
			if (		(pThis->m_pEA->m_AttackStyle >= 18)
					&&	(pThis->m_pEA->m_AttackStyle <= 24) )
			{
				AI_DoSound(pThis, SOUND_ANCIENT_WARRIOR_SCREAM, 1, 0);
			}
			else
			{
				n = RANDOM(3);
				switch (n)
				{
					case 0:	s = SOUND_HUMAN_ALERT_1;	break;
					case 1:	s = SOUND_HUMAN_ALERT_2;		break;
					default:	s = SOUND_HUMAN_ALERT_3;		break;
				}

				if (		(pThis->m_pEA->m_AttackStyle >= 28)
						&&	(pThis->m_pEA->m_AttackStyle <= 29) )
				{
					// make pitch of this sfx lower for demons
					AI_DoSound(pThis, s, 1, BIGENEMY_PITCH);
				}
				else
				{
					// normal sfx for human screams
					AI_DoSound(pThis, s, 1, 0);
				}
			}
			break;

		case AI_OBJECT_CHARACTER_SABERTIGER:
			AI_DoSound(pThis, SOUND_TIGER_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_DIMETRODON:
			AI_DoSound(pThis, SOUND_DIMETRODON_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_TRICERATOPS:
			AI_DoSound(pThis, SOUND_TRICERATOPS_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_MOSCHOPS:
			AI_DoSound(pThis, SOUND_MOSCHOPS_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_PTERANODON:
			AI_DoSound(pThis, SOUND_PTERANODN_SCREAM, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_SUBTERRANEAN:
			AI_DoSound(pThis, SOUND_SUBTERRANEAN_SCREAM, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_LEAPER:
			AI_DoSound(pThis, SOUND_LEAPER_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ALIEN:
			AI_DoSound(pThis, SOUND_ALIEN_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HULK:
			AI_DoSound(pThis, SOUND_HULK_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ROBOT:
			AI_DoSound(pThis, SOUND_ROBOT_SERVO_1, 1, 0);
			break;

		case AI_OBJECT_CEILING_TURRET:
		case AI_OBJECT_BUNKER_TURRET:
			AI_DoSound(pThis, SOUND_ROBOT_SERVO_1, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_BADFISH:
			break;

		case AI_OBJECT_CHARACTER_GOODFISH:
			break;

		case AI_OBJECT_CHARACTER_GENERICRED:
			break;

		case AI_OBJECT_CHARACTER_GENERICGREEN:
			break;

		case AI_OBJECT_CHARACTER_GENERICMETAL:
			break;

		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
			AI_DoSound(pThis, SOUND_MANTIS_CHITTER, 1, 0);
			break;
	}
}


//	"'Alert'             Sound     (Creature)",
void AI_SEvent_Alert (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	int						n, s;

	TRACE1("S Event 'Alert' - Sound (Creature) Volume = %f\r\n", Value);

	// which character is standing on what ?
	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_PLAYER:
			break;

		case AI_OBJECT_CHARACTER_GENERICRED:
			break;

		case AI_OBJECT_CHARACTER_GENERICGREEN:
			break;

		case AI_OBJECT_CHARACTER_GENERICMETAL:
			break;

		case AI_OBJECT_CHARACTER_RAPTOR:
			AI_DoSound(pThis, SOUND_RAPTOR_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HUMAN1:
			if (		(pThis->m_pEA->m_AttackStyle >= 18)
					&&	(pThis->m_pEA->m_AttackStyle <= 24) )
			{
				AI_DoSound(pThis, SOUND_ANCIENT_WARRIOR_ALERT, 1, 0);
			}
			else
			{
				n = RANDOM(3);
				switch (n)
				{
					case 0:	s = SOUND_HUMAN_ALERT_1;	break;
					case 1:	s = SOUND_HUMAN_ALERT_2;		break;
					default:	s = SOUND_HUMAN_ALERT_3;		break;
				}

				if (		(pThis->m_pEA->m_AttackStyle >= 28)
						&&	(pThis->m_pEA->m_AttackStyle <= 29) )
				{
					// make pitch of this sfx lower for demons
					AI_DoSound(pThis, s, 1, BIGENEMY_PITCH);
				}
				else
				{
					// normal sfx for human screams
					AI_DoSound(pThis, s, 1, 0);
				}
			}
			break;

		case AI_OBJECT_CHARACTER_SABERTIGER:
			AI_DoSound(pThis, SOUND_TIGER_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_DIMETRODON:
			AI_DoSound(pThis, SOUND_DIMETRODON_ALERT, 1,0 );
			break;

		case AI_OBJECT_CHARACTER_TRICERATOPS:
			AI_DoSound(pThis, SOUND_TRICERATOPS_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_MOSCHOPS:
			AI_DoSound(pThis, SOUND_MOSCHOPS_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_PTERANODON:
			AI_DoSound(pThis, SOUND_PTERANODN_SCREAM, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_SUBTERRANEAN:
			AI_DoSound(pThis, SOUND_SUBTERRANEAN_SCREAM, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_LEAPER:
			AI_DoSound(pThis, SOUND_LEAPER_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ALIEN:
			AI_DoSound(pThis, SOUND_ALIEN_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HULK:
			AI_DoSound(pThis, SOUND_HULK_ALERT, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ROBOT:
			AI_DoSound(pThis, SOUND_ROBOT_SERVO_1, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_BADFISH:
			break;

		case AI_OBJECT_CHARACTER_GOODFISH:
			break;

		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
			AI_DoSound(pThis, SOUND_MANTIS_SCREAM_1, 1, 0);
			break;
	}
}


//	"'Hydraulics'        Sound     (Robot)",
void AI_SEvent_Hydraulics (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event 'Hydraulics' - Sound (Robot) Pitch = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_ROBOT_SERVO_1, 1, 0);
}

//	"'Shutdown'          Sound     (Robot)",
void AI_SEvent_Shutdown (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event 'Shutdown' - Sound (Robot) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_ROBOT_SHUTDOWN, 1, 0);
}

//	"'Swivel'            Sound     (Robot)",
void AI_SEvent_Swivel (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event 'Swivel' - Sound (Robot) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_ROBOT_SERVO_2, 1, 0);
}

//	"Jetpack out of ctrl Sound     (Alien)",
void AI_SEvent_JetpackOutofControl (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Jet Pack Out of Control - Sound (Alien) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_ALIEN_JETPACK_MALFUNCTION, 1, 0);
}

//	"Burst from Ground   Sound     (Sub)",
void AI_SEvent_BurstFromGround (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Burst From Ground - Sound (Sub) Volume = %f\r\n", Value);
}

//	"Chest Thump         Sound     (Hulk)",
void AI_SEvent_ChestThump (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Chest Thump - Sound (Hulk) Volume = %f\r\n", Value);
			break;
	}
}

//	"Death               Sound     (Creature)",
void AI_SEvent_Death (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	int						n, s;

	TRACE1("S Event Death - Sound (Creature) Volume = %f\r\n", Value);

	// which character is standing on what ?
	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_PLAYER:
			break;

		case AI_OBJECT_CHARACTER_RAPTOR:
			AI_DoSound(pThis, SOUND_RAPTOR_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HUMAN1:
			// CYBORG death scream (formerly ancient warrior, but using same slot)
			if (		(pThis->m_pEA->m_AttackStyle >= 25)
					&&	(pThis->m_pEA->m_AttackStyle <= 27) )
			{
				AI_DoSound(pThis, SOUND_ANCIENT_WARRIOR_DEATH, 1, 0);
			}
			else
			{
				n = RANDOM(3);
				switch (n)
				{

					case 0:	s = SOUND_HUMAN_DEATH_SCREAM_1;	break;
					case 1:	s = SOUND_HUMAN_DEATH_SCREAM_2;		break;
					default:	s = SOUND_HUMAN_DEATH_SCREAM_3;		break;
				}

				if (		(pThis->m_pEA->m_AttackStyle >= 28)
						&&	(pThis->m_pEA->m_AttackStyle <= 29) )
				{
					// make pitch of this sfx lower for demons
					AI_DoSound(pThis, s, 1, BIGENEMY_PITCH);
				}
				else
				{
					// normal sfx for human screams
					AI_DoSound(pThis, s, 1, RANDOMIZE_PITCH);
				}
			}
			break;

		case AI_OBJECT_CHARACTER_SABERTIGER:
			AI_DoSound(pThis, SOUND_TIGER_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_DIMETRODON:
			AI_DoSound(pThis, SOUND_DIMETRODON_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_TRICERATOPS:
			AI_DoSound(pThis, SOUND_TRICERATOPS_SCREAM, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_MOSCHOPS:
			AI_DoSound(pThis, SOUND_MOSCHOPS_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_PTERANODON:
			AI_DoSound(pThis, SOUND_PTERANODN_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_SUBTERRANEAN:
			AI_DoSound(pThis, SOUND_SUBTERRANEAN_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_LEAPER:
			AI_DoSound(pThis, SOUND_LEAPER_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ALIEN:
			AI_DoSound(pThis, SOUND_ALIEN_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_GENERICRED:
			break;

		case AI_OBJECT_CHARACTER_GENERICGREEN:
			break;

		case AI_OBJECT_CHARACTER_GENERICMETAL:
			break;

		case AI_OBJECT_CHARACTER_HULK:
			AI_DoSound(pThis, SOUND_HULK_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ROBOT:
			AI_DoSound(pThis, SOUND_ROBOT_SHUTDOWN, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_BADFISH:
			break;

		case AI_OBJECT_CHARACTER_GOODFISH:
			break;

		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
			AI_DoSound(pThis, SOUND_MANTIS_SCREAM_1, 1, 0);
			break;
	}
}

//	"Explode             Sound     (Robot)",
void AI_SEvent_Explode (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Explode - Sound (Robot) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_EXPLOSION_1, 1, 0);
}

//	"Footfall            Sound     (Any)",
void AI_SEvent_Footfall (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	// which character is standing on what ?
	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_PLAYER:
			break;

		case AI_OBJECT_CHARACTER_GENERICRED:
			break;

		case AI_OBJECT_CHARACTER_GENERICGREEN:
			break;

		case AI_OBJECT_CHARACTER_GENERICMETAL:
			break;

		case AI_OBJECT_CHARACTER_RAPTOR:
			AI_DoSound(pThis, SOUND_RAPTOR_FOOTFALL, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ALIEN:
		case AI_OBJECT_CHARACTER_HUMAN1:
			switch(CGameRegion__GetGroundMaterial(pThis->m_pCurrentRegion))
			{
				case REGMAT_WATERSURFACE:
					AI_DoSound(pThis, SOUND_WATER_FOOTFALL, 1, 0);
					break;

				case REGMAT_STONE:
					AI_DoSound(pThis, SOUND_STONE_FOOTFALL, 1, 0);
					break;

				case REGMAT_STEEL:
					AI_DoSound(pThis, SOUND_METAL_FOOTFALL, 1, 0);
					break;

				case REGMAT_GRASS:
					AI_DoSound(pThis, SOUND_LIGHT_JUNGLE_FOOTFALL, 1, 0);
					break;

				default:
					AI_DoSound(pThis, SOUND_STONE_FOOTFALL, 1, 0);
					break;
			}
			break;

		case AI_OBJECT_CHARACTER_SABERTIGER:
			AI_DoSound(pThis, SOUND_TIGER_FOOTFALL, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_DIMETRODON:
			AI_DoSound(pThis, SOUND_DIMETRODON_FOOTFALL, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_TRICERATOPS:
			AI_DoSound(pThis, SOUND_TRICERATOPS_FOOTFALL, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_MOSCHOPS:
			AI_DoSound(pThis, SOUND_MOSCHOPS_FOOTFALL, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_LEAPER:
			AI_DoSound(pThis, SOUND_LEAPER_FOOTFALL, 1, 0);
			break;

//		case AI_OBJECT_CHARACTER_ALIEN:
//			AI_DoSound(pThis, SOUND_ALIEN_FOOTFALL, 1, 0);
//			break;

		case AI_OBJECT_CHARACTER_HULK:
			AI_DoSound(pThis, SOUND_HULK_FOOTFALL, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ROBOT:
			AI_DoSound(pThis, SOUND_ROBOT_FOOTFALL, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_BADFISH:
			break;

		case AI_OBJECT_CHARACTER_GOODFISH:
			break;

		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
//			AI_DoSound(pThis, SOUND_MANTIS_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_TREX_BOSS:
			AI_DoSound(pThis, SOUND_TREX_FOOTFALL, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
			AI_DoSound(pThis, SOUND_LIGHT_JUNGLE_FOOTFALL, 1, 0);
			break;
	}
}

//	"Grunt               Sound     (Creature)",
void AI_SEvent_Grunt (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	int			n, s;

	TRACE1("S Event Grunt - Sound (Creature) Volume = %f\r\n", Value);

	// which character is standing on what ?
	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_PLAYER:
			break;

		case AI_OBJECT_CHARACTER_GENERICRED:
			break;

		case AI_OBJECT_CHARACTER_GENERICGREEN:
			break;

		case AI_OBJECT_CHARACTER_GENERICMETAL:
			break;

		case AI_OBJECT_CHARACTER_RAPTOR:
			AI_DoSound(pThis, SOUND_RAPTOR_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HUMAN1:
			// CYBORG death scream (formerly ancient warrior, but using same slot)
			if (		(pThis->m_pEA->m_AttackStyle >= 25)
					&&	(pThis->m_pEA->m_AttackStyle <= 27) )
			{
				AI_DoSound(pThis, SOUND_ANCIENT_WARRIOR_DEATH, 1, 0);
			}
			else
			{
				n = RANDOM(3);
				switch (n)
				{
					case 0:	s = SOUND_HUMAN_EFFORTINJURY_GRUNT_1;	break;
					case 1:	s = SOUND_HUMAN_EFFORTINJURY_GRUNT_2;		break;
					default:	s = SOUND_HUMAN_EFFORTINJURY_GRUNT_3;		break;
				}

				if (		(pThis->m_pEA->m_AttackStyle >= 28)
						&&	(pThis->m_pEA->m_AttackStyle <= 29) )
				{
					// make pitch of this sfx lower for demons
					AI_DoSound(pThis, s, 1, BIGENEMY_PITCH);
				}
				else
				{
					// normal sfx for human screams
					AI_DoSound(pThis, s, 1, RANDOMIZE_PITCH);
				}
			}
			break;

		case AI_OBJECT_CHARACTER_SABERTIGER:
			AI_DoSound(pThis, SOUND_TIGER_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_DIMETRODON:
			AI_DoSound(pThis, SOUND_DIMETRODON_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_TRICERATOPS:
			AI_DoSound(pThis, SOUND_TRICERATOPS_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_MOSCHOPS:
			AI_DoSound(pThis, SOUND_MOSCHOPS_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_PTERANODON:
			AI_DoSound(pThis, SOUND_PTERANODN_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_SUBTERRANEAN:
			AI_DoSound(pThis, SOUND_SUBTERRANEAN_SCREAM, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_LEAPER:
			AI_DoSound(pThis, SOUND_LEAPER_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ALIEN:
			AI_DoSound(pThis, SOUND_ALIEN_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HULK:
			AI_DoSound(pThis, SOUND_HULK_INJURY, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ROBOT:
			AI_DoSound(pThis, SOUND_ROBOT_SHORT_1, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_BADFISH:
			break;

		case AI_OBJECT_CHARACTER_GOODFISH:
			break;

		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
			AI_DoSound(pThis, SOUND_MANTIS_INJURY, 1, 0);
			break;
	}
}

//	"Gurgle              Sound     (Hulk)",
void AI_SEvent_Gurgle (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Gurgle - Sound (Hulk) Volume = %f\r\n", Value);
}

//	"Handfall            Sound     (Hulk)",
void AI_SEvent_Handfall (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Handfall - Sound (Hulk) Volume = %f\r\n", Value);

	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_HULK:
            AI_DoSound(pThis, SOUND_GENERIC_139, 1, 0);
			break;

		default:
			break;
	}
}

//	"Hiss                Sound     (Dimetrodon)",
void AI_SEvent_Hiss (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Hiss - Sound (Dimetrodon) Volume = %f\r\n", Value);
}

//	"Hit Ground          Sound     (Any)",
void AI_SEvent_Hitground (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Hit Ground - Sound (Any) Volume = %f\r\n", Value);
}

//	"Licking             Sound     (Leaper)",
void AI_SEvent_Licking (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Licking - Sound (Leaper) Volume = %f\r\n", Value);
}

//	"Rage                Sound     (Any)",
void AI_SEvent_Rage (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Rage - Sound (Any) Volume = %f\r\n", Value);
	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
			AI_DoSound(pThis, SOUND_CAMPAIGNERS_RAGE_UNIVERSE, 1, 0);
			break;
	}
}

//	"Rock Noise 1        Sound     -",
void AI_SEvent_RockNoise1 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Rock Noise 1 - Sound (Any) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_STONEROCK_CRUMBLE_1, 1, 0);
}

//	"Rubbing Skin        Sound     (Leaper)",
void AI_SEvent_RubbingSkin (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	// there is currently on sound type defined for this
	TRACE1("S Event Rubbing Skin - Sound (Leaper) Volume = %f\r\n", Value);
}

//	"Scratch             Sound     (Dimetrodon)",
void AI_SEvent_Scratch (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	// there is currently on sound type defined for this
	TRACE1("S Event Scratch - Sound (Dimetrodon) Volume = %f\r\n", Value);
}

//	"Screech             Sound     (Pteranodon)",
void AI_SEvent_Screech (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Screech - Sound (Pteranodon) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_PTERANODN_SCREAM, 1, 0);
}


//	"Sniff               Sound     (Raptor)",
void AI_SEvent_Sniff (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Sniff - Sound (Raptor) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_RAPTOR_SNIFF, 1, 0);
}

//	"Swoosh              Sound     (Pteranodon)",
void AI_SEvent_Swoosh (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Swoosh - Sound (Pteranodon) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_PTERANODON_FLY_SWOOSH, 1, 0);
}

//	"Turn To Stone       Sound     (Mantis)",
void AI_SEvent_TurntoStone (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Turn To Stone - Sound (Mantis) Volume = %f\r\n", Value);
	AI_DoSound(pThis, SOUND_STONE_SCRAPE_ON_STONE, 1, 0);
}

//	"Violent Death       Sound     (Creature)",
void AI_SEvent_ViolentDeath (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	int		n, s;

	TRACE1("S Event Violent Death - Sound (Creature) Volume = %f\r\n", Value);

	// which character is standing on what ?
	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_PLAYER:
			break;

		case AI_OBJECT_CHARACTER_GENERICRED:
			break;

		case AI_OBJECT_CHARACTER_GENERICGREEN:
			break;

		case AI_OBJECT_CHARACTER_GENERICMETAL:
			break;

		case AI_OBJECT_CHARACTER_RAPTOR:
			AI_DoSound(pThis, SOUND_RAPTOR_VIOLENT_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HUMAN1:
			// CYBORG death scream (formerly ancient warrior, but using same slot)
			if (		(pThis->m_pEA->m_AttackStyle >= 25)
					&&	(pThis->m_pEA->m_AttackStyle <= 27) )
			{
				AI_DoSound(pThis, SOUND_ANCIENT_WARRIOR_DEATH, 1, 0);
			}
			else
			{
				n = RANDOM(3);
				switch (n)
				{
					case 0:	s = SOUND_HUMAN_VIOLENT_DEATH_1;		break;
					case 1:	s = SOUND_HUMAN_VIOLENT_DEATH_2;		break;
					default:	s = SOUND_HUMAN_VIOLENT_DEATH_3;		break;
				}

				if (		(pThis->m_pEA->m_AttackStyle >= 28)
						&&	(pThis->m_pEA->m_AttackStyle <= 29) )
				{
					// make pitch of this sfx lower for demons
					AI_DoSound(pThis, s, 1, BIGENEMY_PITCH);
				}
				else
				{
					// normal sfx for human screams
					AI_DoSound(pThis, s, 1, RANDOMIZE_PITCH);
				}
			}
			break;

		case AI_OBJECT_CHARACTER_SABERTIGER:
			AI_DoSound(pThis, SOUND_TIGER_VIOLENT_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_DIMETRODON:
			AI_DoSound(pThis, SOUND_DIMETRODON_VIOLENT_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_TRICERATOPS:
			AI_DoSound(pThis, SOUND_TRICERATOPS_SCREAM, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_MOSCHOPS:
			AI_DoSound(pThis, SOUND_MOSCHOPS_VIOLENT_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_PTERANODON:
			AI_DoSound(pThis, SOUND_PTERANODN_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_SUBTERRANEAN:
			AI_DoSound(pThis, SOUND_SUBTERRANEAN_VIOLENT_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_LEAPER:
			AI_DoSound(pThis, SOUND_LEAPER_VIOLENT_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ALIEN:
			AI_DoSound(pThis, SOUND_ALIEN_VIOLENT_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_HULK:
			AI_DoSound(pThis, SOUND_HULK_VIOLENT_DEATH, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_ROBOT:
			AI_DoSound(pThis, SOUND_ROBOT_SHUTDOWN, 1, 0);
			break;

		case AI_OBJECT_CHARACTER_BADFISH:
			break;

		case AI_OBJECT_CHARACTER_GOODFISH:
			break;

		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
			AI_DoSound(pThis, SOUND_MANTIS_VIOLENT_DEATH, 1, 0);
			break;
	}
}

//	"Wing Flap           Sound     (Mantis)",
void AI_SEvent_WingFlap (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	// there is currently no type defined for this
	TRACE1("S Event Wing Flap - Sound (Mantis) Volume = %f\r\n", Value);
}

//	"Growl               Sound     (Tiger)",
void AI_SEvent_Growl (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Growl - Sound (Tiger) Volume = %f\r\n", Value);
	AI_SEvent_Grunt(pThis, vPos, Value);
}

//	"Alert 1             Speech    (Human)",
void AI_SEvent_Alert1 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Alert 1 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Alert 2             Speech    (Human)",
void AI_SEvent_Alert2 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Alert 2 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Alert 3             Speech    (Human)",
void AI_SEvent_Alert3 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Alert 3 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Alert 4             Speech    (Human)",
void AI_SEvent_Alert4 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Alert 4 - Speech (Human) Volume = %f\r\n", Value);
}

//	"Death 1             Speech    (Human)",
void AI_SEvent_Death1 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Death 1 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Death 2             Speech    (Human)",
void AI_SEvent_Death2 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Death 2 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Death 3             Speech    (Human)",
void AI_SEvent_Death3 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Death 3 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Death 4             Speech    (Human)",
void AI_SEvent_Death4 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Death 4 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Injury 1            Speech    (Human)",
void AI_SEvent_Injury1 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Injury 1 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Injury 2            Speech    (Human)",
void AI_SEvent_Injury2 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Injury 2 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Injury 3            Speech    (Human)",
void AI_SEvent_Injury3 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE1("S Event Injury 3 - Speech (Human) Volume = %f\r\n", Value);
			break;
	}
}

//	"Long 1              Speech    (Human)",
void AI_SEvent_Long1 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Long 1 - Speech (Human) Volume = %f\r\n", Value);
}

//	"Long 2              Speech    (Human)",
void AI_SEvent_Long2 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Long 2 - Speech (Human) Volume = %f\r\n", Value);
}

//	"Long 3              Speech    (Human)",
void AI_SEvent_Long3 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Long 3 - Speech (Human) Volume = %f\r\n", Value);
}

//	"Taunt 1             Speech    (Human)",
void AI_SEvent_Taunt1 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Taunt 1 - Speech (Human) Volume = %f\r\n", Value);
	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
			AI_DoSound(pThis, SOUND_CAMPAIGNERS_TAUNT_1_PATHETIC, 1, 0);
			break;
	}
}

//	"Taunt 2             Speech    (Human)",
void AI_SEvent_Taunt2 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Taunt 2 - Speech (Human) Volume = %f\r\n", Value);
	switch (CInstanceHdr__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
			AI_DoSound(pThis, SOUND_CAMPAIGNERS_TAUNT_2_DIE, 1, 0);
			break;
	}
}

//	"Taunt 3             Speech    (Human)",
void AI_SEvent_Taunt3 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Taunt 3 - Speech (Human) Volume = %f\r\n", Value);
}

//	"Violent Death 1     Speech    (Human)",
void AI_SEvent_ViolentDeath1 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Violent Death 1 - Speech (Human) Volume = %f\r\n", Value);
}

//	"Violent Death 2     Speech    (Human)",
void AI_SEvent_ViolentDeath2 (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE1("S Event Violent Death 2 - Speech (Human) Volume = %f\r\n", Value);
}

//	"Nudge Back AI"
//
void AI_MEvent_NudgeBack(CGameObjectInstance *pAI, CVector3 vPos, float Value)
{
	CVector3	vAIPos, vDelta, vNudge;
	float		mag,
				radius,
				collrad;

	// display debug info
	TRACE0("M Event Nudge Back - AI\r\n");

	vAIPos = AI_GetPos(pAI);
	vAIPos.y += AI_GetEA(pAI)->m_CollisionHeight/2;

	CVector3__Subtract(&vDelta, &vAIPos, &vPos);
	mag = sqrt(CVector3__MagnitudeSquared(&vDelta));

	radius = (Value*SCALING_FACTOR);
	collrad = AI_GetEA(pAI)->m_CollisionRadius;

	// is pAI within range?
	if ( mag <= (radius+collrad) )
	{
		// nudge in direction of vDelta
		CVector3__MultScaler(&vNudge, &vDelta, 0.09*15.0*SCALING_FACTOR);

		// add to pAI's velocity vector
		CVector3__Add(&pAI->ah.m_vVelocity, &pAI->ah.m_vVelocity, &vNudge);
		pAI->ah.m_vVelocity.y = (2.0*15.0*SCALING_FACTOR);
	}
}



//	"mevent shove with camera"
//
void AI_MEvent_ShoveWithCamera(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value)
{
	CVector3	vAIPos, vDelta, vNudge;
	float		mag,
				radius,
				collrad;

	// display debug info
	TRACE0("M Event Shove with camera - AI\r\n");

	vAIPos = AI_GetPos(pAI);
	vAIPos.y += AI_GetEA(pAI)->m_CollisionHeight/2;

	CVector3__Subtract(&vDelta, &vAIPos, &vPos);
	mag = sqrt(CVector3__MagnitudeSquared(&vDelta));

	radius = (Value*SCALING_FACTOR);
	collrad = AI_GetEA(pAI)->m_CollisionRadius;

	// is pAI within range?
	if ( mag <= (radius+collrad) )
	{
		// nudge in direction of vDelta
		CVector3__Normalize(&vDelta);
		CVector3__MultScaler(&vNudge, &vDelta, 7*15.0*SCALING_FACTOR);

		// add to pAI's velocity vector
		CVector3__Add(&pAI->ah.m_vVelocity, &pAI->ah.m_vVelocity, &vNudge);
		pAI->ah.m_vVelocity.y = (3.5*15.0*SCALING_FACTOR);

		// was origin animated type & was the player being shoved ?
		if (		(pOrigin->m_Type == I_ANIMATED)
				&&	(CEngineApp__GetPlayer(GetApp()) == pAI) )
		{
			CTurokMovement.LookAtShover = 0.6;			// seconds you must look at the origin
			CTurokMovement.pShoverAI = (CGameObjectInstance *)pOrigin;
		}
	}
}


// 	"Screen Shake AI"
void AI_SEvent_ScreenShake (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Screen Shake - AI\r\n");

	// Do the good, new improved, sine wobble screen shake for some characters!
	switch (pThis->m_Type)
	{
		case I_ANIMATED:

			switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
			{
				case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
					CCamera__SetShakeY(&GetApp()->m_Camera, (INT32)(Value/2.5)) ;
					return ;

				case AI_OBJECT_CHARACTER_TREX_BOSS:
					CCamera__SetShakeY(&GetApp()->m_Camera, (INT32)(Value*2.5)) ;
					return ;

				case AI_OBJECT_CHARACTER_MANTIS_BOSS:
				case AI_OBJECT_DEVICE_WALL:
					CCamera__SetShakeY(&GetApp()->m_Camera, (INT32)(Value*1)) ;
					return ;
			}
			break ;
	}

	CTMove__ShakeScreen ( &CTurokMovement, GetApp(), (Value*SCALING_FACTOR/20), ANGLE_DTOR(Value), FALSE );
}

// 	"Screen Tremor AI"
void AI_SEvent_ScreenTremor(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Screen Tremor - AI\r\n");
	CTMove__DoTremorEffectQuick(&CTurokMovement, vPos, Value);
}

//	"Invincible AI"
void AI_SEvent_Invincible (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	CGameObjectInstance	*pAnim;

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			pAnim = (CGameObjectInstance*) pThis;
			TRACE0("S Event Invincible - AI\r\n");
			AI_GetDyn(pAnim)->m_Invincible = TRUE;
			break;
	}
}

// 	"Blood Arc Particles (Any)"
void AI_SEvent_BloodArcParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Blood Arc - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_BLOOD, vPos);
}

//	"Muzzle Flash Particles (Any)",
void AI_SEvent_MuzzleFlashParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Muzzle Flash - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_ENEMY_MUZZLEFLASH, vPos);
}

//	"Blaster Particles (Hulk)",
void AI_SEvent_HulkBlasterParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Hulk Blaster - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_HULK_BLASTER, vPos);
}

//	"S Plasma 2 Particles (Player)"
void AI_SEvent_Plasma2Particle (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Plasma 2 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_PLASMA2, vPos);
	AI_DoSound(pThis, SOUND_EXPLOSION_1, 1, 0);

	// Record fire
	CTurokMovement.FiredParticle = TRUE ;
	CTurokMovement.FiredParticleType = PARTICLE_TYPE_PLASMA2 ;
}

//	"Alien Death Fire Projectile (Alien)",
void AI_SEvent_AlienDeathFireParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Alien Death Fire Projectile - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_ENEMY_PLASMA2, vPos);
}

//	"Knife Forward Particles (Weapon)",
void AI_SEvent_KnifeForwardParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Knife Forward - Weapon\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_WEFFECT_KNIFEFORWARD, vPos);
}

//	"Knife Left Particles (Weapon)",
void AI_SEvent_KnifeLeftParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Knife Left - Weapon\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_WEFFECT_KNIFELEFT, vPos);
}

//	"Knife Right Particles (Weapon)"
void AI_SEvent_KnifeRightParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Knife Right - Weapon\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_WEFFECT_KNIFERIGHT, vPos);
}

// 	"Green Blood Arc Particles (Any)"
void AI_SEvent_GreenBloodArcParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Green Blood Arc - Particles\r\n");
	//AI_DoParticle(pThis, PARTICLE_TYPE_GREENBLOOD, vPos);

	if (pThis->m_Type == I_ANIMATED)
	{
		AI_DoParticleSpread((CAnimInstanceHdr*) pThis,
								  PARTICLE_TYPE_GREENBLOOD, vPos,
								  (int) max(1, Value));
	}
}

// 	"Spurt Blood Arc Particles (Any)"
void AI_SEvent_SpurtBloodArcParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Spurt Blood Arc - Particles\r\n");
	//AI_DoParticle(pThis, PARTICLE_TYPE_SPURT_BLOOD, vPos);

	if (pThis->m_Type == I_ANIMATED)
	{
		AI_DoParticleSpread((CAnimInstanceHdr*) pThis,
								  PARTICLE_TYPE_SPURT_BLOOD, vPos,
								  (int) max(1, Value));
	}
}

// 	"Fountain Blood Arc Particles (Any)"
void AI_SEvent_FountainBloodArcParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Fountain Blood Arc - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_FOUNTAIN_BLOOD, vPos);
}

// 	"Gush Arc Particles (Any)"
void AI_SEvent_GushBloodArcParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Gush Blood Arc - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GUSH_BLOOD, vPos);
}

// 	"Drool Particles (Any)"
void AI_SEvent_DroolParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Drool - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_DROOL, vPos);
}


//	"Damage Knife Left (Weapon)",
void AI_MEvent_KnifeLeft_Damage(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value)
{
	int	nHits;

	// display debug info
	TRACE0("M Event Knife Left Damage - Particles\r\n");

	nHits = AI_KnifeTomahawkDamage(pAI, pOrigin, vPos, Value, AI_KNIFE_LEFT_ATTACK);
	if (nHits) AI_SEvent_KnifeTomahawkBlood_Particles ((CInstanceHdr *)pAI, vPos, AI_KNIFE_LEFT_ATTACK, nHits);
}

//	"Damage Knife Right (Weapon)",
void AI_MEvent_KnifeRight_Damage(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value)
{
	int	nHits;

	// display debug info
	TRACE0("M Event Knife Right Damage - Particles\r\n");

	nHits = AI_KnifeTomahawkDamage(pAI, pOrigin, vPos, Value, AI_KNIFE_RIGHT_ATTACK);
	if (nHits) AI_SEvent_KnifeTomahawkBlood_Particles ((CInstanceHdr *)pAI, vPos, AI_KNIFE_RIGHT_ATTACK, nHits);
}

//	"Damage Knife Forward (Weapon)",
void AI_MEvent_KnifeForward_Damage(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value)
{
	int	nHits;

	// display debug info
	TRACE0("M Event Knife Forward Damage - Particles\r\n");

	nHits = AI_KnifeTomahawkDamage(pAI, pOrigin, vPos, Value, AI_KNIFE_FORWARD_ATTACK);
	if (nHits) AI_SEvent_KnifeTomahawkBlood_Particles ((CInstanceHdr *)pAI, vPos, AI_KNIFE_FORWARD_ATTACK, nHits);
}

//	"Damage Tomahawk Left (Weapon)",
void AI_MEvent_TomahawkLeft_Damage    (CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value)
{
	int	nHits;

	// display debug info
	TRACE0("M Event Tomahawk Left Damage - Particles\r\n");

	nHits = AI_KnifeTomahawkDamage(pAI, pOrigin, vPos, Value, AI_TOMAHAWK_LEFT_ATTACK);
	if (nHits) AI_SEvent_KnifeTomahawkBlood_Particles ((CInstanceHdr *)pAI, vPos, AI_TOMAHAWK_LEFT_ATTACK, nHits);
}

//	"Damage Tomahawk Right (Weapon)",
void AI_MEvent_TomahawkRight_Damage(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value)
{
	int	nHits;

	// display debug info
	TRACE0("M Event Tomahawk Right Damage - Particles\r\n");

	nHits = AI_KnifeTomahawkDamage(pAI, pOrigin, vPos, Value, AI_TOMAHAWK_RIGHT_ATTACK);
	if (nHits) AI_SEvent_KnifeTomahawkBlood_Particles ((CInstanceHdr *)pAI, vPos, AI_TOMAHAWK_RIGHT_ATTACK, nHits);
}

//	"Damage Tomahawk Forward (Weapon)"
void AI_MEvent_TomahawkForward_Damage(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float Value)
{
	int	nHits;

	// display debug info
	TRACE0("M Event Tomahawk Forward Damage - Particles\r\n");

	nHits = AI_KnifeTomahawkDamage(pAI, pOrigin, vPos, Value, AI_TOMAHAWK_FORWARD_ATTACK);
	if (nHits) AI_SEvent_KnifeTomahawkBlood_Particles ((CInstanceHdr *)pAI, vPos, AI_TOMAHAWK_FORWARD_ATTACK, nHits);
}

//	"Arterial Blood Particle (Any)",
void AI_SEvent_ArterialBloodParticles   (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Arterial Blood - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_ARTERIAL_BLOOD, vPos);
}

//	"Spurt G. Blood Particles (Any)",
void AI_SEvent_SpurtGBloodParticles     (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Spurt Green Blood - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_SPURT_GREENBLOOD, vPos);
}

//	"Fountain G. Blood Particles (Any)",
void AI_SEvent_FountainGBloodParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Fountain Green Blood - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_FOUNTAIN_GREENBLOOD, vPos);
}

//	"Gush G. Blood Particles (Any)",
void AI_SEvent_GushGBloodParticles      (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Gush Green Blood - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GUSH_GREENBLOOD, vPos);
}

//	"Arterial G. Blood Particle (Any)"
void AI_SEvent_ArterialGBloodParticles  (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Arterial Green Blood - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_ARTERIAL_GREENBLOOD, vPos);
}

// do knife damage
int AI_KnifeTomahawkDamage (CGameObjectInstance *pThis, CInstanceHdr *pOrigin, CVector3 vPos, float rad, int nKnife)
{
	// object type hit points for knife & tomahawk
	// knife - left/right/forward | tomahawk - left/right/forward
	int	RaptorHits			[]={3,3,3, 3,3,3};
	int	Human1Hits			[]={15,15,15, 20,20,20};		// knife / tomahawk needs to kill human with one hit
	int	SaberTigerHits		[]={1,1,1, 1,1,1};
	int	DimetrodonHits		[]={1,1,1, 1,1,1};
	int	TriceratopsHits	[]={1,1,1, 1,1,1};
	int	MoschopsHits		[]={1,1,1, 1,1,1};
	int	PteranodonHits		[]={1,1,1, 1,1,1};
	int	SubterraneanHits	[]={1,1,1, 1,1,1};
	int	LeaperHits			[]={7,7,7, 7,7,7};
	int	AlienHits			[]={1,1,1, 1,1,1};
	int	HulkHits				[]={1,1,1, 1,1,1};
	int	RobotHits			[]={-1,-1,-1, -1,-1,-1};
	int	BadFishHits			[]={10,10,10, 10,10,10};
	int	GoodFishHits		[]={10,10,10, 10,10,10};
	int	SludgeBoyHits		[]={1,1,1, 1,1,1};
	int	PlantHits			[]={1,1,1, 1,1,1};

#define TREX_KNIFE_HITS			25
#define CAMPAIGNER_KNIFE_HITS	20
#define LONGHUNTER_KNIFE_HITS	15
#define MANTIS_KNIFE_HITS		25


	int	*pHits = NULL;
	int	nHits=0;
	int	ey, py, angdiff;



	// special case for no collision
	if (!(AI_GetEA(pThis)->m_dwTypeFlags & AI_TYPE_COLLISION))
		return 0;

	if (AI_CanWeKillIt(pThis))
		return 0;

	if (AI_GetDyn(pThis)->m_dwStatusFlags2 & AI_HOLDINTANIMON1STFRAME)
		return 0;

	if (AI_GetDyn(pThis)->m_cRegenerateAppearance > 0)
		return 0;

	// did ai get hit
	rad *= SCALING_FACTOR;
	ey = ((int)ANGLE_RTOD(pThis->m_RotY))%360;
	py = ((int)ANGLE_RTOD(CEngineApp__GetPlayer(GetApp())->m_RotY))%360;
	if (ey<0) ey+=360;
	if (py<0) py+=360;
	angdiff = (ey-py);

	// test for more accurate mortal wound hit - radius is smaller & enemy must be facing away from us
	if ( (sqrt(AI_DistanceSquared(pThis, vPos)) <= ( (rad/2) + AI_GetEA(pThis)->m_CollisionRadius)) &&
		  (abs(angdiff)<=40)         &&
		  (AI_GetDyn(pThis)->m_Health>0)     )
	{
		// which character is standing on what ?
		switch (CGameObjectInstance__TypeFlag(pThis))
		{
			case AI_OBJECT_CHARACTER_PLAYER:
				nHits=0;
				break;

			case AI_OBJECT_CHARACTER_RAPTOR:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_GENERICRED:
			case AI_OBJECT_CHARACTER_GENERICGREEN:
			case AI_OBJECT_CHARACTER_GENERICMETAL:
			case AI_OBJECT_CHARACTER_HUMAN1:
				nHits = AI_MORTAL_DEATH_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_SABERTIGER:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_DIMETRODON:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_TRICERATOPS:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_MOSCHOPS:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_PTERANODON:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_SUBTERRANEAN:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_LEAPER:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_ALIEN:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_HULK:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_ROBOT:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_BADFISH:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			case AI_OBJECT_CHARACTER_GOODFISH:
				nHits = AI_MORTAL_WOUND_KT_HITS;
				break;

			// Bosses
			case AI_OBJECT_CHARACTER_TREX_BOSS:
				nHits = TREX_KNIFE_HITS	;
				break ;

			case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
				nHits = CAMPAIGNER_KNIFE_HITS	;
				break ;

			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
				nHits = LONGHUNTER_KNIFE_HITS	;
				break ;

			case AI_OBJECT_CHARACTER_MANTIS_BOSS:
				nHits = MANTIS_KNIFE_HITS	;
				break ;

			default:
				break;
		}
		// do mortal hits
		AI_DoHit(pThis, nHits);
		AI_SetAttacker(pThis, pOrigin);
	}
	// hit another ai (do normal proper test between two ai's)
	else if ( sqrt(AI_DistanceSquared(pThis, vPos)) <= ( rad + AI_GetEA(pThis)->m_CollisionRadius) )
	{
		// which character is standing on what ?
		switch (CGameObjectInstance__TypeFlag(pThis))
		{
			case AI_OBJECT_CHARACTER_PLAYER:
				break;

			case AI_OBJECT_CHARACTER_RAPTOR:
				pHits = &RaptorHits[0];
				break;

			case AI_OBJECT_CHARACTER_GENERICRED:
			case AI_OBJECT_CHARACTER_GENERICGREEN:
			case AI_OBJECT_CHARACTER_GENERICMETAL:
			case AI_OBJECT_CHARACTER_HUMAN1:
				pHits = &Human1Hits[0];
				break;

			case AI_OBJECT_CHARACTER_SABERTIGER:
				pHits = &SaberTigerHits[0];
				break;

			case AI_OBJECT_CHARACTER_DIMETRODON:
				pHits = &DimetrodonHits[0];
				break;

			case AI_OBJECT_CHARACTER_TRICERATOPS:
				pHits = &TriceratopsHits[0];
				break;

			case AI_OBJECT_CHARACTER_MOSCHOPS:
				pHits = &MoschopsHits[0];
				break;

			case AI_OBJECT_CHARACTER_PTERANODON:
				pHits = &PteranodonHits[0];
				break;

			case AI_OBJECT_CHARACTER_SUBTERRANEAN:
				pHits = &SubterraneanHits[0];
				break;

			case AI_OBJECT_CHARACTER_LEAPER:
				pHits = &LeaperHits[0];
				break;

			case AI_OBJECT_CHARACTER_ALIEN:
				pHits = &AlienHits[0];
				break;

			case AI_OBJECT_CHARACTER_HULK:
				pHits = &HulkHits[0];
				break;

			case AI_OBJECT_CHARACTER_ROBOT:
				pHits = &RobotHits[0];
				break;

			case AI_OBJECT_CHARACTER_BADFISH:
				pHits = &BadFishHits[0];
				break;

			case AI_OBJECT_CHARACTER_GOODFISH:
				pHits = &GoodFishHits[0];
				break;

			case AI_OBJECT_CHARACTER_SLUDGEBOY:
				pHits = &SludgeBoyHits[0];
				break;

			case AI_OBJECT_CHARACTER_PLANT:
				pHits = &PlantHits[0];
				break;

			// Bosses
			case AI_OBJECT_CHARACTER_TREX_BOSS:
				nHits = TREX_KNIFE_HITS	;
				break ;

			case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
				nHits = CAMPAIGNER_KNIFE_HITS	;
				break ;

			case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
				nHits = LONGHUNTER_KNIFE_HITS	;
				break ;

			case AI_OBJECT_CHARACTER_MANTIS_BOSS:
				nHits = MANTIS_KNIFE_HITS	;
				break ;


			default:
				pHits = NULL;
				nHits = 0;
				break;

		}


		// do damage ?
		if (pHits)
		{
			switch (nKnife)
			{
				case AI_KNIFE_LEFT_ATTACK:
					nHits = *(pHits+0);
					break;

				case AI_KNIFE_RIGHT_ATTACK:
					nHits = *(pHits+1);
					break;

				case AI_KNIFE_FORWARD_ATTACK:
					nHits = *(pHits+2);
					break;

				case AI_TOMAHAWK_LEFT_ATTACK:
					nHits = *(pHits+3);
					break;

				case AI_TOMAHAWK_RIGHT_ATTACK:
					nHits = *(pHits+4);
					break;

				case AI_TOMAHAWK_FORWARD_ATTACK:
					nHits = *(pHits+5);
					break;
			}

			// do hit damage to object
			if (nHits<0)
			{
				// knife / tomahawk does no damage on this creature
				// return a hit so particle effect still takes place
				nHits = 1;
			}
			else
			{
				AI_DoHit(pThis, nHits);
				AI_SetAttacker(pThis, pOrigin);

				// was mortal wound caused ?
				if (AI_GetDyn(pThis)->m_Health==0)
					AI_GetDyn(pThis)->m_dwStatusFlags |= AI_WOUNDEDMORTALLY;

			}
		}
	}

	// do knife slash sound
	if (nHits)
		AI_DoSound(&pThis->ah.ih, SOUND_TOMAHAWK_IMPACT_FLESH, 1, RANDOMIZE_PITCH);

	return nHits;
}


// do knife blood particles
//
void AI_SEvent_KnifeTomahawkBlood_Particles (CInstanceHdr *pThis, CVector3 vPos, int nKnife, int nHits)
{
	int						nType=-1, redblood, greenblood, spark;
	CGameObjectInstance	*pAnim;


	// is blood off ?
//	if (!GetApp()->m_Options.m_Blood)
//		return;

	if (nHits == AI_MORTAL_WOUND_KT_HITS)
	{
		redblood   = PARTICLE_TYPE_KTFATAL_BLOOD;
		greenblood = PARTICLE_TYPE_KTFATAL_GBLOOD;
		spark      = PARTICLE_TYPE_KTFATAL_SPARK;
	}
	else
	{
		switch (nKnife)
		{
			case AI_KNIFE_LEFT_ATTACK:
				redblood   = PARTICLE_TYPE_KNIFELEFT_BLOOD;
				greenblood = PARTICLE_TYPE_KNIFELEFT_GBLOOD;
				spark      = PARTICLE_TYPE_KNIFELEFT_SPARK;
				break;

			case AI_KNIFE_RIGHT_ATTACK:
				redblood   = PARTICLE_TYPE_KNIFERIGHT_BLOOD;
				greenblood = PARTICLE_TYPE_KNIFERIGHT_GBLOOD;
				spark      = PARTICLE_TYPE_KNIFERIGHT_SPARK;
				break;

			case AI_KNIFE_FORWARD_ATTACK:
				redblood   = PARTICLE_TYPE_KNIFEFORWARD_BLOOD;
				greenblood = PARTICLE_TYPE_KNIFEFORWARD_GBLOOD;
				spark      = PARTICLE_TYPE_KNIFEFORWARD_SPARK;
				break;

			case AI_TOMAHAWK_LEFT_ATTACK:
				redblood   = PARTICLE_TYPE_TOMAHAWKLEFT_BLOOD;
				greenblood = PARTICLE_TYPE_TOMAHAWKLEFT_GBLOOD;
				spark      = PARTICLE_TYPE_TOMAHAWKLEFT_SPARK;
				break;

			case AI_TOMAHAWK_RIGHT_ATTACK:
				redblood   = PARTICLE_TYPE_TOMAHAWKRIGHT_BLOOD;
				greenblood = PARTICLE_TYPE_TOMAHAWKRIGHT_GBLOOD;
				spark      = PARTICLE_TYPE_TOMAHAWKRIGHT_SPARK;
				break;

			case AI_TOMAHAWK_FORWARD_ATTACK:
				redblood   = PARTICLE_TYPE_TOMAHAWKFORWARD_BLOOD;
				greenblood = PARTICLE_TYPE_TOMAHAWKFORWARD_GBLOOD;
				spark      = PARTICLE_TYPE_TOMAHAWKFORWARD_SPARK;
				break;
		}
	}


	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			pAnim = (CGameObjectInstance*) pThis;
//			if (AI_GetEA(pAnim)->m_dwTypeFlags2 & AI_TYPE2_NOBLOOD)
//				break;

			// don't generate blood if enemy is invincible
			if (AI_GetDyn(pAnim)->m_Invincible)
				break;

			// which character is standing on what ?
			switch (CGameObjectInstance__TypeFlag(pAnim))
			{
				case AI_OBJECT_CHARACTER_PLAYER:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_GENERICRED:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_GENERICGREEN:
					nType = greenblood;
					break;

				case AI_OBJECT_CHARACTER_GENERICMETAL:
					nType = spark;
					break;

				case AI_OBJECT_CHARACTER_RAPTOR:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_HUMAN1:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_SABERTIGER:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_DIMETRODON:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_TRICERATOPS:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_MOSCHOPS:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_PTERANODON:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_SUBTERRANEAN:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_LEAPER:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_ALIEN:
					nType = greenblood;
					break;

				case AI_OBJECT_CHARACTER_PLANT:
					nType = greenblood;
					break;

				case AI_OBJECT_CHARACTER_SLUDGEBOY:
					nType = greenblood;
					break;

				case AI_OBJECT_CHARACTER_HULK:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_ROBOT:
					nType = spark;
					break;

				case AI_OBJECT_CHARACTER_BADFISH:
					nType = redblood;
					break;

				case AI_OBJECT_CHARACTER_GOODFISH:
					nType = redblood;
					break;

				// Bosses
				case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
				case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
				case AI_OBJECT_CHARACTER_TREX_BOSS:
					nType = redblood;
					break ;

				case AI_OBJECT_CHARACTER_MANTIS_BOSS:
					nType = greenblood;
					break ;
			}

			if (nType!=-1)
				AI_AimParticleAtPlayer(pThis, nType, vPos, ANGLE_DTOR(360));

			break;
	}
}


//	"Water Drop Particle (Any)",
void AI_SEvent_WaterDropParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Water Drop - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_WATER_DROP, vPos);
}


//	"Water Foam Particle (Any)",
void AI_SEvent_WaterFoamParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Water Foam - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_WATER_FOAM, vPos);
}


//	"Water Steam Particle (Any)"
void AI_SEvent_WaterSteamParticles (CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Water Steam - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_WATER_STEAM, vPos);
}



void AI_SEvent_Arrow_Damage(CInstanceHdr *pThis, CInstanceHdr *pOrigin, CVector3 vPos, float rad)
{
	CParticle				*pArrow;
	CGameObjectInstance	*pAI;
	int			nHits;
	float			magnitude;
	BOOL			Fatal;

	// display debug info
	TRACE0("S Event Arrow Damage\r\n");

	if ((pOrigin->m_Type != I_PARTICLE) || (pThis->m_Type != I_ANIMATED))
		return;

	pArrow = (CParticle*) pOrigin;
	pAI = (CGameObjectInstance*) pThis;

	// fatal kill ?
	magnitude = CVector3__Magnitude(&pArrow->m_CI.vCollisionVelocity) / 15.0;
	Fatal = ((magnitude + 2.0) >= TMOVE_BOW_MAXVELOCITY);

	// scale hit points based on max velocity speed of arrow
	nHits = (int) (TWEAPON_ARROW_HITS*magnitude/TMOVE_BOW_MAXVELOCITY);

	// what creatures can we kill with a fatal blow or a bigger damage than normal
	if (		(AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_SARROWMORTALWOUND)
			&&	(Fatal) )
	{
		nHits = 999999;
		AI_GetDyn(pAI)->m_dwStatusFlags |= AI_WOUNDEDMORTALLY;
	}

	AI_DoHit(pAI, nHits);
	AI_SetAttacker(pAI, pOrigin);
}

void AI_SEvent_InstantDeath(CInstanceHdr *pThis)
{
	CGameObjectInstance	*pAI;


	if (pThis->m_Type == I_ANIMATED)
	{
		pAI = (CGameObjectInstance *)pThis;

		AI_CompletelyKillOff(pAI);
		AI_GetDyn(pAI)->m_MeltTimer		= 0.0;
		AI_GetDyn(pAI)->m_FreezeTimer		= 0.0;
		pAI->m_cMelt							= MELT_LENGTH;
		CGameObjectInstance__SetGone(pAI);
	}
}

void AI_SEvent_Damage_Target(CInstanceHdr *pThis, CInstanceHdr *pOrigin, CVector3 vPos, float damage)
{
	CGameObjectInstance	*pAnim,
								*pAttacker;
	CInstanceHdr			*pSource;
	CParticle				*pParticle = NULL ;

	if (pOrigin)
	{
		switch (pOrigin->m_Type)
		{
			case I_PARTICLE:
				pParticle = (CParticle*)pOrigin ;
				pSource = pParticle->m_pSource;
				if (pSource && (pSource->m_Type == I_ANIMATED))
					pAttacker = (CGameObjectInstance*) pSource;
				else
					pAttacker = NULL;
				break;

			case I_ANIMATED:
				pAttacker = (CGameObjectInstance*) pOrigin;
				break;

			default:
				pAttacker = NULL;
				break;
		}
	}
	else
	{
		pAttacker = NULL;
	}

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE0("S Event Damage Target\r\n");
			pAnim = (CGameObjectInstance*) pThis;

			if (		(pAttacker == NULL)
					||	(		(pAnim->ah.ih.m_Type == I_ANIMATED)
							&&	(pAttacker->ah.ih.m_nObjType != pAnim->ah.ih.m_nObjType) ) )
			{
				AI_DoHit(pAnim, (int) damage);
			}

			AI_SetAttacker(pAnim, pOrigin);
			break;
	}
}

void AI_SEvent_DamageNudge_Target(CInstanceHdr *pThis, CInstanceHdr *pOrigin, CVector3 vPos, float damage)
{
	CGameObjectInstance	*pAnim,
								*pAttacker;
	CInstanceHdr			*pSource;
	CParticle				*pParticle ;

	switch (pOrigin->m_Type)
	{
		case I_PARTICLE:
			pParticle = (CParticle *)pOrigin ;
			pSource = pParticle->m_pSource;
			if (pSource->m_Type == I_ANIMATED)
				pAttacker = (CGameObjectInstance*) pSource;
			else
				pAttacker = NULL;
			break;

		case I_ANIMATED:
			pAttacker = (CGameObjectInstance*) pOrigin;
			break;

		default:
			pAttacker = NULL;
			break;
	}

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			TRACE0("S Event Damage & Nudge Target\r\n");
			pAnim = (CGameObjectInstance*) pThis;

			if (		(pAttacker == NULL)
					||	(		(pAnim->ah.ih.m_Type == I_ANIMATED)
							&&	(pAttacker->ah.ih.m_nObjType != pAnim->ah.ih.m_nObjType) ) )
			{
				AI_DoHit(pAnim, (int) damage);
			}
			AI_SetAttacker(pAnim, pOrigin);
			AI_MEvent_WeaponImpact(pAnim, pOrigin, vPos, 15);
			break;
	}
}


void AI_SEvent_Generic1(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 1 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC1, vPos);
}


void AI_SEvent_Generic2(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 2 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC2, vPos);
}


void AI_SEvent_Generic3(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 3 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC3, vPos);
}


void AI_SEvent_Generic4(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 4 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC4, vPos);
}


void AI_SEvent_Generic5(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 5 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC5, vPos);
}


void AI_SEvent_Generic6(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 6 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC6, vPos);
}


void AI_SEvent_Generic7(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 7 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC7, vPos);
}


void AI_SEvent_Generic8(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 8 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC8, vPos);
}


void AI_SEvent_Generic9(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 9 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC9, vPos);
}


void AI_SEvent_Generic10(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 10 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC10, vPos);
}


void AI_SEvent_Generic11(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 11 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC11, vPos);
}


void AI_SEvent_Generic12(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 12 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC12, vPos);
}


void AI_SEvent_Generic13(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 13 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC13, vPos);
}


void AI_SEvent_Generic14(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 14 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC14, vPos);
}


void AI_SEvent_Generic15(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 15 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC15, vPos);
}


void AI_SEvent_Generic16(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 16 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC16, vPos);
}


void AI_SEvent_Generic17(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 17 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC17, vPos);
}


void AI_SEvent_Generic18(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 18 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC18, vPos);
}


void AI_SEvent_Generic19(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 19 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC19, vPos);
}


void AI_SEvent_Generic20(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 20 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC20, vPos);
}


void AI_SEvent_Generic21(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 21 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC21, vPos);
}


void AI_SEvent_Generic22(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 22 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC22, vPos);
}


void AI_SEvent_Generic23(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 23 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC23, vPos);
}


void AI_SEvent_Generic24(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 24 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC24, vPos);
}


void AI_SEvent_Generic25(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 25 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC25, vPos);
}


void AI_SEvent_Generic26(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 26 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC26, vPos);
}


void AI_SEvent_Generic27(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 27 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC27, vPos);
}


void AI_SEvent_Generic28(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 28 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC28, vPos);
}


void AI_SEvent_Generic29(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 29 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC29, vPos);
}


void AI_SEvent_Generic30(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 30 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC30, vPos);
}

void AI_SEvent_Generic31(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 31 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC31, vPos);
}

void AI_SEvent_Generic32(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 32 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC32, vPos);
}

void AI_SEvent_Generic33(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 33 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC33, vPos);
}

void AI_SEvent_Generic34(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 34 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC34, vPos);
}

void AI_SEvent_Generic35(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 35 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC35, vPos);
}

void AI_SEvent_Generic36(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 36 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC36, vPos);
}

void AI_SEvent_Generic37(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 37 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC37, vPos);
}

void AI_SEvent_Generic38(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 38 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC38, vPos);
}

void AI_SEvent_Generic39(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 39 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC39, vPos);
}

void AI_SEvent_Generic40(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 40 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC40, vPos);
}

void AI_SEvent_Generic41(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 41 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC41, vPos);
}

void AI_SEvent_Generic42(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 42 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC42, vPos);
}

void AI_SEvent_Generic43(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 43 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC43, vPos);
}

void AI_SEvent_Generic44(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 44 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC44, vPos);
}

void AI_SEvent_Generic45(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 45 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC45, vPos);
}

void AI_SEvent_Generic46(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 46 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC46, vPos);
}

void AI_SEvent_Generic47(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 47 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC47, vPos);
}

void AI_SEvent_Generic48(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 48 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC48, vPos);
}

void AI_SEvent_Generic49(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 49 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC49, vPos);
}

void AI_SEvent_Generic50(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 50 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC50, vPos);
}

void AI_SEvent_Generic51(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 51 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC51, vPos);
}

void AI_SEvent_Generic52(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 52 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC52, vPos);
}

void AI_SEvent_Generic53(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 53 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC53, vPos);
}

void AI_SEvent_Generic54(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 54 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC54, vPos);
}

void AI_SEvent_Generic55(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 55 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC55, vPos);
}

void AI_SEvent_Generic56(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 56 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC56, vPos);
}

void AI_SEvent_Generic57(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 57 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC57, vPos);
}

void AI_SEvent_Generic58(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 58 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC58, vPos);
}

void AI_SEvent_Generic59(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 59 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC59, vPos);
}

void AI_SEvent_Generic60(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 60 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC60, vPos);
}

void AI_SEvent_Generic61(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 61 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC61, vPos);
}

void AI_SEvent_Generic62(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 62 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC62, vPos);
}

void AI_SEvent_Generic63(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 63 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC63, vPos);
}

void AI_SEvent_Generic64(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 64 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC64, vPos);
}

void AI_SEvent_Generic65(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 65 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC65, vPos);
}

void AI_SEvent_Generic66(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 66 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC66, vPos);
}

void AI_SEvent_Generic67(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 67 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC67, vPos);
}

void AI_SEvent_Generic68(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 68 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC68, vPos);
}

void AI_SEvent_Generic69(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 69 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC69, vPos);
}

void AI_SEvent_Generic70(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 70 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC70, vPos);
}

void AI_SEvent_Generic71(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 71 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC71, vPos);
}

void AI_SEvent_Generic72(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 72 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC72, vPos);
}

void AI_SEvent_Generic73(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 73 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC73, vPos);
}

void AI_SEvent_Generic74(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 74 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC74, vPos);
}

void AI_SEvent_Generic75(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 75 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC75, vPos);
}

void AI_SEvent_Generic76(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 76 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC76, vPos);
}

void AI_SEvent_Generic77(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 77 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC77, vPos);
}

void AI_SEvent_Generic78(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 78 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC78, vPos);
}

void AI_SEvent_Generic79(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 79 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC79, vPos);
}

void AI_SEvent_Generic80(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 80 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC80, vPos);
}

void AI_SEvent_Generic81(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 81 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC81, vPos);
}

void AI_SEvent_Generic82(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 82 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC82, vPos);
}

void AI_SEvent_Generic83(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 83 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC83, vPos);
}

void AI_SEvent_Generic84(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 84 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC84, vPos);
}

void AI_SEvent_Generic85(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 85 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC85, vPos);
}

void AI_SEvent_Generic86(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 86 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC86, vPos);
}

void AI_SEvent_Generic87(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 87 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC87, vPos);
}

void AI_SEvent_Generic88(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 88 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC88, vPos);
}

void AI_SEvent_Generic89(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 89 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC89, vPos);
}

void AI_SEvent_Generic90(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 90 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC90, vPos);
}

void AI_SEvent_Generic91(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 91 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC91, vPos);
}

void AI_SEvent_Generic92(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 92 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC92, vPos);
}

void AI_SEvent_Generic93(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 93 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC93, vPos);
}

void AI_SEvent_Generic94(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 94 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC94, vPos);
}

void AI_SEvent_Generic95(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 95 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC95, vPos);
}

void AI_SEvent_Generic96(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 96 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC96, vPos);
}

void AI_SEvent_Generic97(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 97 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC97, vPos);
}

void AI_SEvent_Generic98(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 98 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC98, vPos);
}

void AI_SEvent_Generic99(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 99 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC99, vPos);
}

void AI_SEvent_Generic100(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 100 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC100, vPos);
}

void AI_SEvent_Generic101(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 101 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC101, vPos);
}

void AI_SEvent_Generic102(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 102 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC102, vPos);
}

void AI_SEvent_Generic103(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 103 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC103, vPos);
}

void AI_SEvent_Generic104(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 104 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC104, vPos);
}

void AI_SEvent_Generic105(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 105 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC105, vPos);
}

void AI_SEvent_Generic106(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 106 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC106, vPos);
}

void AI_SEvent_Generic107(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 107 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC107, vPos);
}

void AI_SEvent_Generic108(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 108 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC108, vPos);
}

void AI_SEvent_Generic109(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 109 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC109, vPos);
}

void AI_SEvent_Generic110(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 110 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC110, vPos);
}

void AI_SEvent_Generic111(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 111 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC111, vPos);
}

void AI_SEvent_Generic112(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 112 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC112, vPos);
}

void AI_SEvent_Generic113(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 113 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC113, vPos);
}

void AI_SEvent_Generic114(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 114 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC114, vPos);
}

void AI_SEvent_Generic115(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 115 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC115, vPos);
}

void AI_SEvent_Generic116(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 116 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC116, vPos);
}

void AI_SEvent_Generic117(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 117 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC117, vPos);
}

void AI_SEvent_Generic118(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 118 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC118, vPos);
}

void AI_SEvent_Generic119(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 119 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC119, vPos);
}

void AI_SEvent_Generic120(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 120 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC120, vPos);
}

void AI_SEvent_Generic121(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 121 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC121, vPos);
}

void AI_SEvent_Generic122(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 122 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC122, vPos);
}

void AI_SEvent_Generic123(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 123 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC123, vPos);
}

void AI_SEvent_Generic124(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 124 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC124, vPos);
}

void AI_SEvent_Generic125(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 125 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC125, vPos);
}

void AI_SEvent_Generic126(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 126 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC126, vPos);
}

void AI_SEvent_Generic127(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 127 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC127, vPos);
}

void AI_SEvent_Generic128(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 128 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC128, vPos);
}

void AI_SEvent_Generic129(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 129 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC129, vPos);
}

void AI_SEvent_Generic130(CInstanceHdr *pThis, CVector3 vPos, float damage)
{
	TRACE0("S Event Generic 130 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC130, vPos);
}

void AI_SEvent_Generic131(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 131 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC131, vPos);
}

void AI_SEvent_Generic132(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 132 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC132, vPos);
}

void AI_SEvent_Generic133(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 133 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC133, vPos);
}

void AI_SEvent_Generic134(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 134 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC134, vPos);
}

void AI_SEvent_Generic135(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 135 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC135, vPos);
}

void AI_SEvent_Generic136(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 136 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC136, vPos);
}

void AI_SEvent_Generic137(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 137 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC137, vPos);
}

void AI_SEvent_Generic138(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 138 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC138, vPos);
}

void AI_SEvent_Generic139(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 139 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC139, vPos);
}

void AI_SEvent_Generic140(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 140 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC140, vPos);
}

void AI_SEvent_Generic141(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 141 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC141, vPos);
}

void AI_SEvent_Generic142(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 142 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC142, vPos);
}

void AI_SEvent_Generic143(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 143 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC143, vPos);
}

void AI_SEvent_Generic144(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 144 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC144, vPos);
}

void AI_SEvent_Generic145(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 145 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC145, vPos);
}

void AI_SEvent_Generic146(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 146 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC146, vPos);
}

void AI_SEvent_Generic147(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 147 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC147, vPos);
}

void AI_SEvent_Generic148(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 148 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC148, vPos);
}

void AI_SEvent_Generic149(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 149 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC149, vPos);
}

void AI_SEvent_Generic150(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 150 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC150, vPos);
}

void AI_SEvent_Generic151(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 151 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC151, vPos);
}

void AI_SEvent_Generic152(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 152 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC152, vPos);
}

void AI_SEvent_Generic153(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 153 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC153, vPos);
}

void AI_SEvent_Generic154(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 154 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC154, vPos);
}

void AI_SEvent_Generic155(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 155 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC155, vPos);
}

void AI_SEvent_Generic156(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 156 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC156, vPos);
}

void AI_SEvent_Generic157(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 157 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC157, vPos);
}

void AI_SEvent_Generic158(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 158 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC158, vPos);
}

void AI_SEvent_Generic159(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 159 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC159, vPos);
}

void AI_SEvent_Generic160(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 160 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC160, vPos);
}

void AI_SEvent_Generic161(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 161 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC161, vPos);
}

void AI_SEvent_Generic162(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 162 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC162, vPos);
}

void AI_SEvent_Generic163(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 163 - Particles\n");
	AI_AimParticleAtPlayer(pThis, PARTICLE_TYPE_GENERIC163, vPos, ANGLE_DTOR(10)) ;
}

void AI_SEvent_Generic164(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 164 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC164, vPos);
}

void AI_SEvent_Generic165(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 165 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC165, vPos);
}

void AI_SEvent_Generic166(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 166 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC166, vPos);
}

void AI_SEvent_Generic167(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 167 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC167, vPos);
}

void AI_SEvent_Generic168(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 168 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC168, vPos);
}

void AI_SEvent_Generic169(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 169 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC169, vPos);
}

void AI_SEvent_Generic170(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 170 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC170, vPos);
}

void AI_SEvent_Generic171(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 171 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC171, vPos);
}

void AI_SEvent_Generic172(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 172 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC172, vPos);
}

void AI_SEvent_Generic173(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 173 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC173, vPos);
}

void AI_SEvent_Generic174(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 174 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC174, vPos);
}

void AI_SEvent_Generic175(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 175 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC175, vPos);
}

void AI_SEvent_Generic176(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 176 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC176, vPos);
}

void AI_SEvent_Generic177(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 177 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC177, vPos);
}

void AI_SEvent_Generic178(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 178 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC178, vPos);
}

void AI_SEvent_Generic179(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 179 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC179, vPos);
}

void AI_SEvent_Generic180(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 180 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC180, vPos);
}

void AI_SEvent_Generic181(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 181 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC181, vPos);
}

void AI_SEvent_Generic182(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 182 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC182, vPos);
}

void AI_SEvent_Generic183(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 183 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC183, vPos);
}

void AI_SEvent_Generic184(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 184 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC184, vPos);
}

void AI_SEvent_Generic185(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 185 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC185, vPos);
}

void AI_SEvent_Generic186(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 186 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC186, vPos);
}

void AI_SEvent_Generic187(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 187 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC187, vPos);
}

void AI_SEvent_Generic188(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 188 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC188, vPos);
}

void AI_SEvent_Generic189(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 189 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC189, vPos);
}

void AI_SEvent_Generic190(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 190 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC190, vPos);
}

void AI_SEvent_Generic191(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 191 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC191, vPos);
}

void AI_SEvent_Generic192(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 192 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC192, vPos);
}

void AI_SEvent_Generic193(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 193 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC193, vPos);
}

void AI_SEvent_Generic194(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 194 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC194, vPos);
}

void AI_SEvent_Generic195(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 195 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC195, vPos);
}

void AI_SEvent_Generic196(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 196 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC196, vPos);
}

void AI_SEvent_Generic197(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 197 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC197, vPos);
}

void AI_SEvent_Generic198(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 198 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC198, vPos);
}

void AI_SEvent_Generic199(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 199 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC199, vPos);
}

void AI_SEvent_Generic200(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 200 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC200, vPos);
}

void AI_SEvent_Generic201(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 201 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC201, vPos);
}

void AI_SEvent_Generic202(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 202 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC202, vPos);
}

void AI_SEvent_Generic203(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 203 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC203, vPos);
}

void AI_SEvent_Generic204(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 204 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC204, vPos);
}

void AI_SEvent_Generic205(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 205 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC205, vPos);
}

void AI_SEvent_Generic206(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 206 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC206, vPos);
}

void AI_SEvent_Generic207(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 207 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC207, vPos);
}

void AI_SEvent_Generic208(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 208 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC208, vPos);
}

void AI_SEvent_Generic209(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 209 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC209, vPos);
}

void AI_SEvent_Generic210(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 210 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC210, vPos);
}

void AI_SEvent_Generic211(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 211 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC211, vPos);
}

void AI_SEvent_Generic212(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 212 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC212, vPos);
}

void AI_SEvent_Generic213(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 213 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC213, vPos);
}

void AI_SEvent_Generic214(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 214 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC214, vPos);
}

void AI_SEvent_Generic215(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 215 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC215, vPos);
}

void AI_SEvent_Generic216(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 216 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC216, vPos);
}

void AI_SEvent_Generic217(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 217 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC217, vPos);
}

void AI_SEvent_Generic218(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 218 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC218, vPos);
}

void AI_SEvent_Generic219(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 219 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC219, vPos);
}

void AI_SEvent_Generic220(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 220 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC220, vPos);
}

void AI_SEvent_Generic221(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 221 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC221, vPos);
}

void AI_SEvent_Generic222(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 222 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC222, vPos);
}

void AI_SEvent_Generic223(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 223 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC223, vPos);
}

// Explosion with parts
void AI_SEvent_Generic224(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	CCampaigner *pCampaigner ;

	TRACE0("S Event Generic 224 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC224, vPos);

	// Play explosion
	AI_SEvent_GeneralSound(pThis, vPos, SOUND_EXPLOSION_2) ;

	// Play hurt sound
	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
			{
				case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
					pCampaigner = (CCampaigner *)((CGameObjectInstance *)pThis)->m_pBoss ;
					if (pCampaigner->m_PlayingSoundTimer == 0)
					{
						pCampaigner->m_PlayingSoundTimer = SECONDS_TO_FRAMES(13) ;
						AI_SEvent_GeneralSound(pThis, vPos, SOUND_CAMPAIGNERS_DEATH) ;
					}
					break ;
			}
			break ;
	}
}

void AI_SEvent_Generic225(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 225 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC225, vPos);
}

void AI_SEvent_Generic226(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 226 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC226, vPos);
}

void AI_SEvent_Generic227(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 227 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC227, vPos);
}

void AI_SEvent_Generic228(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 228 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC228, vPos);
}

void AI_SEvent_Generic229(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 229 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC229, vPos);
}

void AI_SEvent_Generic230(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 230 - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC230, vPos);
}

void AI_SEvent_Generic231(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 231 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC231, vPos);
}

void AI_SEvent_Generic232(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 232 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC232, vPos);
}

void AI_SEvent_Generic233(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 233 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC233, vPos);
}

void AI_SEvent_Generic234(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 234 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC234, vPos);
}

void AI_SEvent_Generic235(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 235 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC235, vPos);
}

void AI_SEvent_Generic236(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 236 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC236, vPos);
}

void AI_SEvent_Generic237(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 237 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC237, vPos);
}

void AI_SEvent_Generic238(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 238 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC238, vPos);
}

void AI_SEvent_Generic239(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 239 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC239, vPos);
}

void AI_SEvent_Generic240(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 240 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC240, vPos);
}

void AI_SEvent_Generic241(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 241 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC241, vPos);
}

void AI_SEvent_Generic242(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 242 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC242, vPos);
}

void AI_SEvent_Generic243(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 243 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC243, vPos);
}

void AI_SEvent_Generic244(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 244 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC244, vPos);
}

void AI_SEvent_Generic245(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 245 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC245, vPos);
}

void AI_SEvent_Generic246(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 246 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC246, vPos);
}

void AI_SEvent_Generic247(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 247 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC247, vPos);
}

// Humvee explosion
void AI_SEvent_Generic248(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 248 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC248, vPos);

	// This sound must use "AI_DoSound" because of special humvee code!!!
	AI_DoSound(pThis, SOUND_GENERIC_250, 1, 0);
}

void AI_SEvent_Generic249(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 249 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC249, vPos);
}

void AI_SEvent_Generic250(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 250 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC250, vPos);
}

void AI_SEvent_Generic251(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 251 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC251, vPos);
}

void AI_SEvent_Generic252(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 252 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC252, vPos);
}

void AI_SEvent_Generic253(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 253 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC253, vPos);
}

void AI_SEvent_Generic254(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 254 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC254, vPos);
}

void AI_SEvent_Generic255(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 255 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC255, vPos);
}

void AI_SEvent_Generic256(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 256 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC256, vPos);
}

void AI_SEvent_Generic257(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 257 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC257, vPos);
}

void AI_SEvent_Generic258(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 258 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC258, vPos);
}

void AI_SEvent_Generic259(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 259 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC259, vPos);
}

void AI_SEvent_Generic260(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Generic 260 - Particles\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_GENERIC260, vPos);
}


void AI_MEvent_Shockwave_Damage(CGameObjectInstance *pAI, CInstanceHdr *pOrigin, CVector3 vPos, float rad)
{
	float			rad2, damage,
					ydiff;
	CVector3		vAIPos,
					vExp;

	// ignore shockwave damage if it is another hulk
	if (CGameObjectInstance__TypeFlag(pAI) == AI_OBJECT_CHARACTER_HULK)
		return;

	// if character is not on ground it can't be hit
	if (!CInstanceHdr__IsOnGround(&pAI->ah.ih))
		return;

	// if character is below wave, it can't be hit
	ydiff = fabs(pAI->ah.ih.m_vPos.y - vPos.y);
	if (ydiff > (3*SCALING_FACTOR))
		return;

	// display debug info
	TRACE1("M Event Shockwave Damage, Radius = %f\r\n", rad);

	// scale rad
	rad *= SCALING_FACTOR;
	rad *= rad;

	// did ai get hit by shockwave
	rad2 = AI_DistanceSquared (pAI, vPos);
	if (rad2 < rad)
	{
		// decrease ai's health depending on how close they are to the explosion
		damage = (rad - rad2)/rad;
		AI_Decrease_Health(pAI, AI_HITS_SHOCKWAVE*damage, TRUE, TRUE);

		// Can't hurt invincible ai's
		if (!(AI_GetDyn(pAI)->m_Invincible))
		{
			AI_GetDyn(pAI)->m_dwStatusFlags |= AI_EV_EXPLOSION;
			AI_Increase_Agitation(pAI, AI_AGITATION_EXPLOSION, 300);

			// calculate explosion vector effect on ai
			vAIPos = AI_GetPos(pAI);

			CVector3__Subtract(&vExp, &vAIPos, &vPos);
			vExp.y = 0;
			CVector3__Normalize(&vExp);

			AI_SetAttacker(pAI, pOrigin);

			// set explosion magnitude
			damage = sqrt(damage);
			damage *= 20;
			vExp.x *= damage;
			vExp.z *= damage;


			// do different explosion effects based on type
			switch (CGameObjectInstance__TypeFlag(pAI))
			{
				// to be exploded fully
				case AI_OBJECT_CHARACTER_PLAYER:
				case AI_OBJECT_CHARACTER_RAPTOR:
				case AI_OBJECT_CHARACTER_HUMAN1:
				case AI_OBJECT_CHARACTER_SABERTIGER:
				case AI_OBJECT_CHARACTER_TRICERATOPS:
				case AI_OBJECT_CHARACTER_PTERANODON:
					vExp.y  = damage*0.25 + ((RANDOM(2)+1)*SCALING_FACTOR);
					//rmonPrintf("damage:%f, vExp (%f, %f, %f)\n", damage, vExp.x, vExp.y, vExp.z);
					break;

				// to be exploded only through animation
				case AI_OBJECT_CHARACTER_MOSCHOPS:
				case AI_OBJECT_CHARACTER_DIMETRODON:
				case AI_OBJECT_CHARACTER_HULK:
				case AI_OBJECT_CHARACTER_ROBOT:
				case AI_OBJECT_CHARACTER_LEAPER:
				case AI_OBJECT_CHARACTER_ALIEN:
					vExp.y = 0;
					break;

				default:
				case AI_OBJECT_CEILING_TURRET:
				case AI_OBJECT_BUNKER_TURRET:
				case AI_OBJECT_CHARACTER_SUBTERRANEAN:
				case AI_OBJECT_CHARACTER_BADFISH:
				case AI_OBJECT_CHARACTER_GOODFISH:
				case AI_OBJECT_CHARACTER_GENERICRED:
				case AI_OBJECT_CHARACTER_GENERICGREEN:
				case AI_OBJECT_CHARACTER_GENERICMETAL:
					vExp.x = 0;
					vExp.y = 0;
					vExp.z = 0;
					break;
			}

			// Store explosion vector
			vExp.x *= 15;
			vExp.y *= 15;
			vExp.z *= 15;
			AI_GetDyn(pAI)->m_vExplosion = vExp ;
		}
	}
}


void AI_SEvent_Water_Bubble(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	int	cnt;

	TRACE0("S Event Water Bubble - Particles\r\n");

	if (Value<1) Value = 1.0;

	for (cnt=0; cnt<((int)Value); cnt++)
		AI_DoParticle(pThis, PARTICLE_TYPE_WATER_BUBBLE, vPos);
}

void AI_SEvent_Water_Ripple(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Water Ripple - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_WATER_RIPPLE, vPos);
}

void AI_SEvent_Water_Splash(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	TRACE0("S Event Water Splash - Particles\r\n");
	AI_DoParticle(pThis, PARTICLE_TYPE_WATER_SPLASH, vPos);
}

void AI_SEvent_GeneralSound(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	INT32 SoundHandle, nValue ;

	// Special case checks for turning off looping sounds
	switch (pThis->m_Type)
	{
		case I_ANIMATED:

			switch(CGameObjectInstance__TypeFlag((CGameObjectInstance *)pThis))
			{
				case AI_OBJECT_CHARACTER_HUMVEE_BOSS:
					Humvee_SEvent_GeneralSound(pThis, vPos, Value) ;
					return ;
			}
	}

	// Store if not ready yet
	if (game_frame_number <= 2)
		CAudioList__AddGeneralSound(pThis, vPos, Value) ;
	else
	{
		//TRACE1("S Event General - Sound Type = %d\n", (int) Value);
		SoundHandle = AI_DoSound(pThis, (int) Value, 1, 0);

		// Special case cinema sound which camera needs to keep handle of so they can be turned
		// off is a cinema is exited early due to start being pressed
		nValue = Value ;
		switch(nValue)
		{
			case SOUND_GENERIC_116:	// Intro (bow draw) wind sound
			case SOUND_GENERIC_214:	// Resurrection wind sound
				CCamera__SetSoundHandle(&GetApp()->m_Camera, 0, SoundHandle) ;
				break ;
		}
	}
}




// bullets and bites
//
void AI_DoHit (CGameObjectInstance *pAI, int hits)
{
	// don't allow hits to interactive animations
	if (		(!(AI_GetDyn(pAI)->m_dwStatusFlags & AI_INTERACTIVEANIMATION))
			&&	(!(AI_GetDyn(pAI)->m_dwStatusFlags & AI_INTERANIMDELAY))
			&&	(!(AI_GetDyn(pAI)->m_dwStatusFlags2 & AI_HOLDINTANIMON1STFRAME))
			&& (!CInstanceHdr__IsDevice(&pAI->ah.ih)) )
	{
		// Can't hurt invincible ai's
		if (!(AI_GetDyn(pAI)->m_Invincible))
			AI_GetDyn(pAI)->m_dwStatusFlags |= AI_EV_HIT;

		AI_Increase_Agitation ( pAI, AI_AGITATION_HIT, 300 );

		// decrease ai's health
		AI_Decrease_Health ( pAI, hits, TRUE, TRUE );

		// interrupt animations
		//if (AI_GetDyn(pAI)->m_Health)
		//	AI_GetDyn(pAI)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
	}
}


void AI_DoParticleSpread(CAnimInstanceHdr *pThis, int nType, CVector3 vPos, int nParticles)
{
	int			cParticle;
	CVector3		vLastPos, vDistance,
					vDesiredPos, vParticlePos;
	CGameRegion	*pParticleRegion;
	float			u;
	CQuatern		qRot;
	BOOL			Blood = TRUE;

	if (pThis->ih.m_Type == I_ANIMATED)
		if (AI_GetEA((CGameObjectInstance*)pThis)->m_dwTypeFlags2 & AI_TYPE2_NOBLOOD)
			Blood = FALSE;

	// events happen at FRAME_FPS, no matter what frame_increment is
	CVector3__MultScaler(&vDistance, &pThis->m_vVelocity, 1.0/FRAME_FPS);
	CVector3__Subtract(&vLastPos, &vPos, &vDistance);

	qRot = CAnimInstanceHdr__GetVelocityRotation(pThis);

	for (cParticle=0; cParticle<nParticles; cParticle++)
	{
		u = ((float) cParticle)/nParticles;
		CVector3__Blend(&vDesiredPos, u, &vPos, &vLastPos);

		CInstanceHdr__GetNearPositionAndRegion(&pThis->ih, vDesiredPos, &vParticlePos, &pParticleRegion, INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);

		CParticleSystem__CreateParticle(&GetApp()->m_Scene.m_ParticleSystem, &pThis->ih, nType,
												  pThis->m_vVelocity,
												  qRot,
												  vParticlePos, pParticleRegion,
												  PARTICLE_NOEDGE, Blood);
	}
}

// create a particle
//
void AI_DoParticle(CInstanceHdr *pThis, int nType, CVector3 vPos)
{
	CVector3					vParticlePos,
								vVelocity;
	CGameRegion				*pParticleRegion;
	CQuatern					qRot;
	CGameObjectInstance	*pAnim;
	CGameSimpleInstance	*pSimple;
	BOOL						Blood = TRUE;


	// get particle position
	CInstanceHdr__GetNearPositionAndRegion(pThis, vPos, &vParticlePos, &pParticleRegion, INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_STICK);

	switch (pThis->m_Type)
	{
		case I_STATIC:
		case I_PARTICLE:
			vVelocity.x = vVelocity.y = vVelocity.z = 0;
			CQuatern__Identity(&qRot);
			break;


		case I_ANIMATED:
			pAnim = (CGameObjectInstance*) pThis;

			if (AI_GetEA(pAnim)->m_dwTypeFlags2 & AI_TYPE2_NOBLOOD)
				Blood = FALSE;

			vVelocity = pAnim->ah.m_vVelocity;
			qRot = CGameObjectInstance__GetAimRotation(pAnim);
			break;

		case I_SIMPLE:
			pSimple = (CGameSimpleInstance*) pThis;
			vVelocity = pSimple->ah.m_vVelocity;
			qRot = CGameSimpleInstance__GetAimRotation(pSimple);
			break;

		default:
			ASSERT(FALSE);
	}

	CParticleSystem__CreateParticle(&GetApp()->m_Scene.m_ParticleSystem, pThis, nType,
											  vVelocity,
											  qRot,
											  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, Blood);
}


void AI_AimParticleAtPlayer(CInstanceHdr *pThis, int nType, CVector3 vPos, FLOAT MaxYRot)
{
	CEngineApp				*pApp;
	CGameObjectInstance	*pPlayer;
	float						dot, theta;
	CVector3					vParticlePos,
								vPlayerPos,
								vDelta, vZ, vZi, vAxis,
								vVelocity;
	CQuatern					qAim, qRot, qFinal;
	CMtxF						mfRot;
	CGameRegion				*pParticleRegion;
	CGameObjectInstance	*pAnim;
	BOOL						Blood = TRUE;

	if (pThis->m_Type == I_ANIMATED)
	{
		pAnim = (CGameObjectInstance*) pThis;

		if (AI_GetEA(pAnim)->m_dwTypeFlags2 & AI_TYPE2_NOBLOOD)
			Blood = FALSE;

		qRot = CGameObjectInstance__GetAimRotation(pAnim);
		CQuatern__ToMatrix(&qRot, mfRot);
		vZi.x = 0;
		vZi.y = 0;
		vZi.z = 1;
		CMtxF__VectorMult(mfRot, &vZi, &vZ);

	}
	else
	{
		vZ.x = 0;
		vZ.y = 0;
		vZ.z = 1;
	}

	pApp = GetApp();
	if (pThis->m_Type == I_ANIMATED)
	{
		pPlayer = ((CGameObjectInstance*)pThis)->m_pAttackerCGameObjectInstance;
		if (!pPlayer)
			pPlayer = CEngineApp__GetPlayer(pApp);
	}
	else
	{
		pPlayer = CEngineApp__GetPlayer(pApp);
	}

	// get particle position
	CInstanceHdr__GetNearPositionAndRegion(pThis, vPos, &vParticlePos, &pParticleRegion, INTERSECT_BEHAVIOR_STICK, INTERSECT_BEHAVIOR_STICK);

	// find aiming rotation
	vPlayerPos = AI_GetPos(pPlayer);
	vPlayerPos.y += (TUROK_HEIGHT/2)*SCALING_FACTOR;

	CVector3__Subtract(&vDelta, &vPlayerPos, &vParticlePos);
	CVector3__Normalize(&vDelta);

	CVector3__Cross(&vAxis, &vZ, &vDelta);
	CVector3__Normalize(&vAxis);

	dot = CVector3__Dot(&vZ, &vDelta);
	theta = min(acos(dot), MaxYRot);

	CQuatern__BuildFromAxisAngle(&qAim, vAxis.x, vAxis.y, vAxis.z, theta);

	if (pThis->m_Type == I_ANIMATED)
		CQuatern__Mult(&qFinal, &qRot, &qAim);
	else
		qFinal = qAim;

	vVelocity.x = vVelocity.y = vVelocity.z = 0;
	CParticleSystem__CreateParticle(&pApp->m_Scene.m_ParticleSystem, pThis, nType,
											  vVelocity,
											  qFinal,
											  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, Blood);

/*
	CEngineApp				*pApp;
	CGameObjectInstance	*pPlayer;
	CVector3					vParticlePos,
								vPlayerPos,
								vDelta,
								vVelocity;
	CQuatern					qFinal, qYRot, qXRot ;
	CGameRegion				*pParticleRegion;

	FLOAT						XZAngle, XYAngle, DeltaAngle ;

	// Get player pointer
	pApp = GetApp() ;
	if (pThis->m_Type == I_ANIMATED)
	{
		pPlayer = ((CGameObjectInstance*)pThis)->m_pAttackerCGameObjectInstance ;
		if (!pPlayer)
			pPlayer = CEngineApp__GetPlayer(pApp) ;
	}
	else
		pPlayer = CEngineApp__GetPlayer(pApp) ;

	// Get particle position
	CInstanceHdr__GetNearPositionAndRegion(pThis, vPos, &vParticlePos, &pParticleRegion, INTERSECT_BEHAVIOR_STICK, INTERSECT_BEHAVIOR_STICK) ;

	// Find aiming rotation
	vPlayerPos = AI_GetPos(pPlayer);
	vPlayerPos.y += (TUROK_HEIGHT/2)*SCALING_FACTOR;

	CVector3__Subtract(&vDelta, &vPlayerPos, &vParticlePos) ;
	XZAngle = CVector3__XZAngle(&vDelta) ;

	// Limit XZ angle
	if (pThis->m_Type == I_ANIMATED)
	{
		DeltaAngle = AngleDiff(((CGameObjectInstance *)pThis)->m_RotY, XZAngle) ;
		if (DeltaAngle < -MaxYRot)
			DeltaAngle = -MaxYRot ;
		else
		if (DeltaAngle > MaxYRot)
			DeltaAngle = MaxYRot ;

		XZAngle = ((CGameObjectInstance *)pThis)->m_RotY + DeltaAngle ;
	}

	// Calculate direction - note ground orientation is not taken into account
	XYAngle = CVector3__XYAngle(&vDelta) ;
	CQuatern__BuildFromAxisAngle(&qXRot, 1, 0, 0, XYAngle) ;
	CQuatern__BuildFromAxisAngle(&qYRot, 0, 1, 0, XZAngle + ANGLE_DTOR(180)) ;
	CQuatern__Mult(&qFinal, &qXRot, &qYRot) ;


	vVelocity.x = vVelocity.y = vVelocity.z = 0 ;
	CParticleSystem__CreateParticle(&pApp->m_Scene.m_ParticleSystem, pThis, nType,
											  vVelocity,
											  qFinal,
											  vParticlePos, pParticleRegion, PARTICLE_NOEDGE, TRUE) ;
*/
}


int AI_DoSound(CInstanceHdr *pThis, int nSound, float Volume, int SndFlags)
{
	TRACE1("Event Sound:%d", nSound);


	// do cheat mode sfx changes
	if (		(pThis->m_Type == I_ANIMATED)
			&&	(((CGameObjectInstance *)pThis) != CEngineApp__GetPlayer(GetApp())) )
	{
		if (GetApp()->m_dwCheatFlags & CHEATFLAG_TINYENEMYMODE)
			SndFlags |= SMALLENEMY_PITCH;
		else if (GetApp()->m_dwCheatFlags & (/*CHEATFLAG_BIGENEMYMODE |*/ CHEATFLAG_BIGHEADS))
			SndFlags |= BIGENEMY_PITCH;
	}

	return CScene__DoSoundEffect(&GetApp()->m_Scene,
										  nSound, Volume,
										  pThis, &pThis->m_vPos, SndFlags);
}


// increase ai's agitation state
//
void AI_Increase_Agitation ( CGameObjectInstance *pAI, short amt, short max )
{
	short startAgitation;

	ASSERT(AI_IsValid(pAI));

	// already over max ?
	if ( AI_GetDyn(pAI)->m_Agitation > max-1 ) return;

	startAgitation = AI_GetDyn(pAI)->m_Agitation;

	// increase agitation level of ai
	AI_GetDyn(pAI)->m_Agitation += amt;
	if ( AI_GetDyn(pAI)->m_Agitation > max-1 ) AI_GetDyn(pAI)->m_Agitation = max-1;

	// has agitation level overflowed
	if ( AI_GetDyn(pAI)->m_Agitation > AI_AGITATION_MAX-1 ) AI_GetDyn(pAI)->m_Agitation = AI_AGITATION_MAX-1;

	// wake up enemy if it just got agitated
	if ((startAgitation < max) && (AI_GetDyn(pAI)->m_Agitation == max))
		AI_GetDyn(pAI)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
}




// decrease ai's agitation state
//
void AI_Decrease_Agitation ( CGameObjectInstance *pAI, short amt )
{
	/* decrease agitation level of ai */
	AI_GetDyn(pAI)->m_Agitation -= amt;

	/* has agitation level overflowed */
	if ( AI_GetDyn(pAI)->m_Agitation < 0 ) AI_GetDyn(pAI)->m_Agitation = 0;
}





// get agitation level
//
short AI_Get_Agitation_Level ( CGameObjectInstance *pAI )
{
	return (short) ( AI_GetDyn(pAI)->m_Agitation / AI_AGITATION_STEP );
}


////////////////// HOLD ON TO sin(pAI->m_RotY) AND cos(pAI->m_RotY)
//        FOR SPEED.................

float AI_GetAngle(CGameObjectInstance *pAI, CVector3 vPos)
{
	float		dx, dz,
				magDelta,
				fx, fz,
				rx, rz,
				sinRotY, cosRotY,
				dot, theta;

	dx = vPos.x - AI_GetPos(pAI).x;
	dz = vPos.z - AI_GetPos(pAI).z;

	magDelta = sqrt(dx*dx + dz*dz);

	if (magDelta == 0)
		return 0;

	sinRotY = sin(pAI->m_RotY);
	cosRotY = cos(pAI->m_RotY);

	fx = -sinRotY;
	fz = -cosRotY;

	rx = -cosRotY;
	rz = sinRotY;

	dot = max(-1, min(1, (dx*fx + dz*fz)/magDelta));
	theta = acos(dot);


	return ((dx*rx + dz*rz) > 0) ? theta : -theta;
}

float AI_GetAvoidanceAngle(CGameObjectInstance *pAI, CVector3 vPos, CGameObjectInstance *pTarget, float radiusFactor)
{
	int					cAng, direction;
	float					goalAngle, offsetAngle, tryAngle, returnOffset;
	CVector3				vTryPos;
	BOOL					rightHanded;
	CCollisionInfo2	*pCI;


	if (AI_GetEA(pAI)->m_wTypeFlags3 & AI_TYPE3_WALKINSTRAIGHTLINE)
		return 0.0;

	goalAngle = AI_GetAngle(pAI, vPos);

	// it's okay to act stupid if nobody's looking
	if (!CGameObjectInstance__IsVisible(pAI))
		return goalAngle;

	if (!CAnimInstanceHdr__IsObstructed(&pAI->ah, vPos, &pCI))
	{
		//rmonPrintf("unobstructed\r\n");
		return goalAngle;
	}

	// set collided flag
	if (pCI->pWallCollisionRegion)
		AICollidedWithWall = TRUE;

	//rightHanded = (((DWORD) pAI) & 0x100) ? 1 : 0;
	rightHanded = ((((DWORD) pAI) % sizeof(CGameObjectInstance)) & 1) ? 1 : 0;

	vTryPos.y = pAI->ah.ih.m_vPos.y;
	for (cAng=1; cAng<6; cAng++)
	{
		offsetAngle = cAng*ANGLE_DTOR(30);

		// make direction random
		for (direction=0; direction<2; direction++)
		{
			tryAngle = goalAngle;

			if (direction ^ rightHanded)
			{
				tryAngle += offsetAngle;
				returnOffset = ANGLE_DTOR(15);
			}
			else
			{
				tryAngle -= offsetAngle;
				returnOffset = ANGLE_DTOR(-15);
			}

			//vTryPos.x = pAI->ah.ih.m_vPos.x + sin(pAI->m_RotY + ANGLE_PI + tryAngle)*(AVOIDANCE_DISTANCE*SCALING_FACTOR);
			//vTryPos.z = pAI->ah.ih.m_vPos.z + cos(pAI->m_RotY + ANGLE_PI + tryAngle)*(AVOIDANCE_DISTANCE*SCALING_FACTOR);
			vTryPos.x = pAI->ah.ih.m_vPos.x + sin(pAI->m_RotY + ANGLE_PI + tryAngle)*(radiusFactor*pAI->ah.ih.m_pEA->m_CollisionRadius);
			vTryPos.z = pAI->ah.ih.m_vPos.z + cos(pAI->m_RotY + ANGLE_PI + tryAngle)*(radiusFactor*pAI->ah.ih.m_pEA->m_CollisionRadius);

			if (!CAnimInstanceHdr__IsObstructed(&pAI->ah, vTryPos, NULL))
			{
				//rmonPrintf("offset:%f\r\n", (tryAngle - goalAngle)*180/ANGLE_PI);
				tryAngle += returnOffset;
				NormalizeRotation(&tryAngle);
				return tryAngle;
			}
		}
	}

	// go toward goal
	return goalAngle;

/*
	if (RANDOM(2))
	{
		// go toward goal
		return goalAngle;
	}
	else
	{
		return ANGLE_DTOR(RANDOM(360)) - ANGLE_DTOR(180);
	}
*/
}




void AI_NudgeRotY(CGameObjectInstance *pAI, float Angle)
{
	pAI->m_RotY += Angle;
	NormalizeRotation(&pAI->m_RotY);
}


// make ai teleport away
//
BOOL AI_Make_Teleport_Away(CGameObjectInstance *pAI)
{
	int			picked;
	int			awaytype[]={AI_ANIM_TELEPORT_AWAY, AI_ANIM_TELEPORT_AWAY2};
	int			awayweight[]={2, 2};


	// can this ai teleport ?
	if (AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_TELEPORT)
	{
		// set desired teleport away animation
		picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 2, awaytype, awayweight, NULL);
		AI_SetDesiredAnimByIndex(pAI, picked, TRUE);
		AI_GetDyn(pAI)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		AI_GetDyn(pAI)->m_dwStatusFlags |= AI_TELEPORTAWAY;
		return TRUE;
	}

	return FALSE;
}


// make ai teleport to new destination
//
void AI_Make_Teleport(CGameObjectInstance *pAI)
{
	CGameObjectInstance	*pDest;
	CVector3					vDestPos;


	// has this ai to teleport to new destination ?
	if (AI_GetDyn(pAI)->m_dwStatusFlags & AI_TELEPORTING)
		return;

	// teleport this ai ?
	if (AI_GetDyn(pAI)->m_dwStatusFlags & AI_TELEPORTAWAY)
	{
		if (AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_TELEPORTMOVESLOW)
		{
			// ai is doing slow teleport - make ai move towards target
			AI_GetDyn(pAI)->m_dwStatusFlags |= AI_TELEPORTING;
			AI_GetDyn(pAI)->m_dwStatusFlags |= AI_TELEPORTMOVESLOW;
			AI_GetDyn(pAI)->m_TeleportTime = (float)(RANDOM(3)+1.5);
		}
		else
		{
			// ai is now teleporting - quick teleport
			AI_GetDyn(pAI)->m_dwStatusFlags |= AI_TELEPORTING;
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_TELEPORTAWAY);

			// get target
			pDest = pAI->m_pAttackerCGameObjectInstance;
			vDestPos = AI_GetPos(pDest);

			AI_DoTeleport(pAI, vDestPos);
		}
	}
}


// move the ai - do the actual teleport
// true means teleport worked
//
BOOL AI_DoTeleport(CGameObjectInstance *pAI, CVector3 vDestPos)
{
	float						destX,
								destZ,
								rad,
								destA;
	CVector3					vLookPos;
	CGameObjectInstance	*pDest;
	int						waterFlag;


	// get target
	pDest = pAI->m_pAttackerCGameObjectInstance;
	vLookPos = AI_GetPos(pDest);

	// make target the position the ai teleports around
	destX = vDestPos.x;
	destZ = vDestPos.z;

	// choose projectile distance or attack distance randomly
	if (RANDOM(100)<50)
	{
		// teleport to projectile distance
		rad = sqrt(AI_GetEA(pAI)->m_ProjectileRadius);
	}
	else
	{
		// teleport to attack distance
		rad = sqrt(AI_GetEA(pAI)->m_AttackRadius);
	}

	// move character in random angle direction
	destA = ANGLE_DTOR( (RANDOM(10)*36) );
	destX += rad * cosf(destA);
	destZ += rad * sinf(destA);

	// move the player
	CAnimInstanceHdr__Teleport((CAnimInstanceHdr *)pAI, destX, destZ);

	// make ai face target
	AI_GetDyn(pAI)->m_AvoidanceAngle =
	destA = AI_GetAvoidanceAngle(pAI, vLookPos, pDest, AVOIDANCE_RADIUS_DISTANCE_FACTOR);
	
	AI_NudgeRotY(pAI, frame_increment*destA);

	// did ai end up in water ?
	if (AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_AVOIDWATER)
	{
		waterFlag = AI_GetWaterStatus(pAI);
		if (waterFlag != AI_NOT_NEAR_WATER)
		{
			// kill this ai for landing in the water
			AI_CompletelyKillOff(pAI);
			CGameObjectInstance__SetGone(pAI);
			return FALSE;
		}
	}

	return TRUE;
}


// make ai teleport appear
//
BOOL AI_Make_Teleport_Appear(CGameObjectInstance *pAI)
{
	int	picked;
	int	appeartype[]={AI_ANIM_TELEPORT_APPEAR};
	int	appearweight[]={1};


	// set desired teleport appear animation
	if (    (AI_GetDyn(pAI)->m_dwStatusFlags & AI_TELEPORTING)
		  && (!(AI_GetDyn(pAI)->m_dwStatusFlags & AI_TELEPORTAWAY)) )
	{
		picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 1, appeartype, appearweight, NULL);
		AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
		AI_GetDyn(pAI)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_TELEPORTING);
		return TRUE;
	}


	return FALSE;
}


// make ai run forwards based on given angle (radians)
//
BOOL AI_Make_Run ( CGameObjectInstance *pAI, float ang )
{
	int		picked;

	int		turn180type1[]={AI_ANIM_RTURN180, AI_ANIM_RTURNL90,
									 AI_ANIM_WTURN180, AI_ANIM_WTURNL90,
									 AI_ANIM_TURN180,  AI_ANIM_TURNL90 };

	int		turn180type2[]={AI_ANIM_RTURN1802, AI_ANIM_RTURNL902,
									 AI_ANIM_WTURN1802, AI_ANIM_WTURNL902,
									 AI_ANIM_TURN1802,  AI_ANIM_TURNL902 };

	int		turn180weight[]={1000000, 10000,
									  100,     1,
									  10,      1    };

	int		turnl90type1[]={AI_ANIM_RTURNL90,
									 AI_ANIM_WTURNL90,
									 AI_ANIM_TURNL90   };

	int		turnl90type2[]={AI_ANIM_RTURNL902,
									 AI_ANIM_WTURNL902,
									 AI_ANIM_TURNL902   };

	int		turnl90weight[]={1000000, 10000, 1};

	int		turnr90type1[]={AI_ANIM_RTURNR90,
									 AI_ANIM_WTURNR90,
									 AI_ANIM_TURNR90   };

	int		turnr90type2[]={AI_ANIM_RTURNR902,
									 AI_ANIM_WTURNR902,
									 AI_ANIM_TURNR902   };

	int		turnr90weight[]={1000000, 10000, 1};

	int		runtype1[]={AI_ANIM_RUN, AI_ANIM_WALK};

	int		runtype2[]={AI_ANIM_RUN2, AI_ANIM_WALK2};

	int		runweight[]={1000000,1};
	float		u, dist;
	CVector3	vDelta;


	// which way should we turn ?
	if ( ang < ANGLE_DTOR(-170) )
	{
#if DEBUG_AI
		rmonPrintf("AI_Make_Run: Turn 180\r\n") ;
#endif
		// turn around 180 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
	else if ( ang < ANGLE_DTOR(-80) )
	{
#if DEBUG_AI
		rmonPrintf("AI_Make_Run: Turn left 90\r\n") ;
#endif
		// turn left 90 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnl90type2, turnl90weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnl90type1, turnl90weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
	else if ( ang < ANGLE_DTOR(80) )
	{
#if DEBUG_AI
		rmonPrintf("AI_Make_Run: Run\r\n") ;
#endif
		// Running
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 2, runtype2, runweight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 2, runtype1, runweight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       &= (~AI_WAITFOR_CYCLE);
			AI_GetDyn(pAI)->m_dwStatusFlags |= AI_RUNNING;
		}

// TEMP
// S.B. 08/23/96
// TOOK OUT RANDOM TURN
#if 0
		// do random turn ?
		if (RANDOM(300)<2)
		{
#if DEBUG_AI
		rmonPrintf("AI_Make_Run: Random Turn\r\n") ;
#endif
			if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
				picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
			else
				picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

			if (picked==-1)
			{
				AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
				AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
				return FALSE;
			}
			else
			{
				AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
				AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
				return FALSE;
			}
		}
#endif

		// make ai turn a direction
		AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);

		if ( ang>ANGLE_DTOR(-10) && ang<ANGLE_DTOR(10) )
			return TRUE;
	}
	else if ( ang < ANGLE_DTOR(170) )
	{
#if DEBUG_AI
		rmonPrintf("AI_Make_Run: Turn right 90\r\n") ;
#endif
		// turn right 90 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnr90type2, turnr90weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnr90type1, turnr90weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
	else if ( ang <= ANGLE_DTOR(180) )
	{
		// turn around 180 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}


	CVector3__Subtract(&vDelta, &pAI->ah.ih.m_vPos, &CEngineApp__GetPlayer(GetApp())->ah.ih.m_vPos);
	dist = CVector3__Magnitude(&vDelta);

	if (GetApp()->m_FarClip == 0)
		u = 1;
	else
		u = dist/GetApp()->m_FarClip;

	frame_increment *= BlendFLOAT(u, 1.7, 1);


	return FALSE;
}


// make ai retreat forwards based on given angle (radians)
//
BOOL AI_Make_Retreat ( CGameObjectInstance *pAI, float ang )
{
	int		picked;

	int		turn180type1[]={AI_ANIM_RTURN180, AI_ANIM_RTURNL90,
									 AI_ANIM_WTURN180, AI_ANIM_WTURNL90,
									 AI_ANIM_TURN180,  AI_ANIM_TURNL90 };

	int		turn180type2[]={AI_ANIM_RTURN1802, AI_ANIM_RTURNL902,
									 AI_ANIM_WTURN1802, AI_ANIM_WTURNL902,
									 AI_ANIM_TURN1802,  AI_ANIM_TURNL902 };

	int		turn180weight[]={1000000, 10000,
									  100,     1,
									  10,      1    };

	int		turnl90type1[]={AI_ANIM_RTURNL90,
									 AI_ANIM_WTURNL90,
									 AI_ANIM_TURNL90   };

	int		turnl90type2[]={AI_ANIM_RTURNL902,
									 AI_ANIM_WTURNL902,
									 AI_ANIM_TURNL902   };

	int		turnl90weight[]={1000000, 10000, 1};

	int		turnr90type1[]={AI_ANIM_RTURNR90,
									 AI_ANIM_WTURNR90,
									 AI_ANIM_TURNR90   };

	int		turnr90type2[]={AI_ANIM_RTURNR902,
									 AI_ANIM_WTURNR902,
									 AI_ANIM_TURNR902   };

	int		turnr90weight[]={1000000, 10000, 1};

	int		runtype1[]={AI_ANIM_RETREAT, AI_ANIM_RUN, AI_ANIM_WALK};

	int		runtype2[]={AI_ANIM_RETREAT, AI_ANIM_RUN2, AI_ANIM_WALK2};

	int		runweight[]={1000000,10000,1};


	// which way should we turn ?
	if ( ang < ANGLE_DTOR(-170) )
	{
		// turn around 180 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
	else if ( ang < ANGLE_DTOR(-80) )
	{
		// turn left 90 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnl90type2, turnl90weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnl90type1, turnl90weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
	else if ( ang < ANGLE_DTOR(80) )
	{
		// running
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, runtype2, runweight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, runtype1, runweight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       &= (~AI_WAITFOR_CYCLE);
			AI_GetDyn(pAI)->m_dwStatusFlags |= AI_RUNNING;
		}

		// make ai turn a direction
		AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);

		if ( ang>ANGLE_DTOR(-10) && ang<ANGLE_DTOR(10) )
			return TRUE;
	}
	else if ( ang < ANGLE_DTOR(170) )
	{
		// turn right 90 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnr90type2, turnr90weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnr90type1, turnr90weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
	else if ( ang <= ANGLE_DTOR(180) )
	{
		// turn around 180 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}


	return FALSE;
}


// make ai walk forwards based on given angle (radians)
//
void AI_Make_Walk ( CGameObjectInstance *pAI, float ang )
{
	int		picked;

	int		turn180type1[]={AI_ANIM_RTURN180, AI_ANIM_RTURNR90,
									 AI_ANIM_WTURN180, AI_ANIM_WTURNR90,
									 AI_ANIM_TURN180,  AI_ANIM_TURNR90 };

	int		turn180type2[]={AI_ANIM_RTURN1802, AI_ANIM_RTURNR902,
									 AI_ANIM_WTURN1802, AI_ANIM_WTURNR902,
									 AI_ANIM_TURN1802,  AI_ANIM_TURNR902 };

	int		turn180weight[]={100,     1,
									  1000000, 10000,
									  10,      1    };

	int		turnl90type1[]={AI_ANIM_RTURNL90,
									 AI_ANIM_WTURNL90,
									 AI_ANIM_TURNL90   };

	int		turnl90type2[]={AI_ANIM_RTURNL902,
									 AI_ANIM_WTURNL902,
									 AI_ANIM_TURNL902   };

	int		turnl90weight[]={10000, 1000000, 1};

	int		turnr90type1[]={AI_ANIM_RTURNR90,
									 AI_ANIM_WTURNR90,
									 AI_ANIM_TURNR90   };

	int		turnr90type2[]={AI_ANIM_RTURNR902,
									 AI_ANIM_WTURNR902,
									 AI_ANIM_TURNR902   };

	int		turnr90weight[]={10000, 1000000, 1};

	int		walktype1[]={AI_ANIM_RUN, AI_ANIM_WALK};

	int		walktype2[]={AI_ANIM_RUN2, AI_ANIM_WALK2};

	int		walkweight[]={1, 1000000};


	// which way should we turn ?
	if ( ang < ANGLE_DTOR(-170) )
	{
		// turn around 180 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
	else if ( ang < ANGLE_DTOR(-80) )
	{
		// turn left 90 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnl90type2, turnl90weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnl90type1, turnl90weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
	else if ( ang < ANGLE_DTOR(80) )
	{
		// walking
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 2, walktype2, walkweight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 2, walktype1, walkweight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       &= (~AI_WAITFOR_CYCLE);
		}

		// do random turn ?
		if (RANDOM(300)<2)
		{
			if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
				picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
			else
				picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

			if (picked==-1)
			{
				AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
				AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
			}
			else
			{
				AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
				AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
			}
		}

		// make ai turn a direction
		AI_NudgeRotY(pAI, frame_increment*ang/AI_WALKTURN_SMOOTHNESS);
	}
	else if ( ang < ANGLE_DTOR(170) )
	{
		// turn right 90 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnr90type2, turnr90weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 3, turnr90type1, turnr90weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
	else if ( ang <= ANGLE_DTOR(180) )
	{
		// turn around 180 degrees
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags       |= AI_WAITFOR_CYCLE;
		}
	}
}


// make ai swim at correct height & angle
//
void AI_Make_Swim_AtHeight(CGameObjectInstance *pAI)
{
	CGameObjectInstance	*pTarget;
	float						dm, tcy, acy, num, ghgt;
	int						waterFlag;
	BOOL						DontSetHeight = FALSE;


	// is ai near water
	waterFlag = AI_GetWaterStatus(pAI);
	if (waterFlag == AI_NOT_NEAR_WATER)
		return;


	// if it floats remove any velocity
	if (		(AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_FLOATINWATERONDEATH)
			&&	((waterFlag == AI_ON_WATERSURFACE) || (waterFlag == AI_BELOW_WATERSURFACE)) )
	{
		AI_GetVelocity(pAI)->x = 0;
		AI_GetVelocity(pAI)->y = 0;
		AI_GetVelocity(pAI)->z = 0;
	}

	// make ai match its targets height
	pTarget = pAI->m_pAttackerCGameObjectInstance;
	if (pTarget)
	{
		// what is water status of target ?
		waterFlag = AI_GetWaterStatus(pTarget);
		switch (waterFlag)
		{
			case AI_BELOW_WATERSURFACE:
				if (!AI_GetDyn(pAI)->m_Health)
				{
					if (AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_FLOATINWATERONDEATH)
						tcy = AI_GetWaterHeight(pAI) + (AI_GetEA(pAI)->m_DeadHeight/2);
					else
						DontSetHeight = TRUE;
				}
				else
				{
					// set target center y position
					tcy  = AI_GetPos(pTarget).y + (AI_GetEA(pTarget)->m_CollisionHeight/2);
					tcy -= (0.2*SCALING_FACTOR);
				}
				break;

			case AI_ON_WATERSURFACE:
			default:
				if (!AI_GetDyn(pAI)->m_Health)
				{
					if (AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_FLOATINWATERONDEATH)
						tcy = AI_GetWaterHeight(pAI) + (AI_GetEA(pAI)->m_DeadHeight/2);
					else
						DontSetHeight = TRUE;
				}
				else
				{
					// make ai swim underwater
					tcy = AI_GetWaterHeight(pAI);
					tcy -= (0.2*SCALING_FACTOR);
				}
				break;
		}

		// should height be set ?
		if (DontSetHeight)
			return;

		// anything that avoids the player should appear at a slightly different height while swimming
		if (		(AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_AVOIDPLAYER)
				&&	(AI_GetDyn(pAI)->m_Health) )
		{
			num = (((float) (((DWORD)pAI) & 0x1f)) / 9.0) * SCALING_FACTOR;
			tcy -= num;
		}

		// get ai center y position
		acy = AI_GetPos(pAI).y + (AI_GetEA(pAI)->m_CollisionHeight/2);

		if (		(AI_GetEA(pAI)->m_dwTypeFlags & AI_TYPE_AVOIDPLAYER)
				&&	(AI_GetDyn(pAI)->m_Health) )
		{
			if (tcy > AI_GetWaterHeight(pAI) - (5*SCALING_FACTOR))
				tcy -= (5*SCALING_FACTOR);

			ghgt = CGameRegion__GetGroundHeight(pAI->ah.ih.m_pCurrentRegion, AI_GetPos(pAI).x, AI_GetPos(pAI).z);

			if (tcy < ghgt + (5*SCALING_FACTOR) )
				tcy += 5*SCALING_FACTOR;

			dm = (tcy - acy) / 35;
			if (dm >  0.08*SCALING_FACTOR) dm =  0.08 * SCALING_FACTOR;
			else
				if (dm < -0.08*SCALING_FACTOR) dm = -0.08 *SCALING_FACTOR;
		}
		else
		{
			dm = (tcy - acy) / 15;
			if (dm >  0.3*SCALING_FACTOR) dm =  0.3 * SCALING_FACTOR;
			else
				if (dm < -0.3*SCALING_FACTOR) dm = -0.3 *SCALING_FACTOR;
		}

		// set new height ?
		if (!AI_GetDyn(pAI)->m_RetreatTimer)
			AI_GetPos(pAI).y += dm * frame_increment * 2;
	}
}


// make ai swim forwards based on given angle (radians)
//
void AI_Make_Swim(CGameObjectInstance *pAI, float ang)
{
	int						picked;
	int						swimtype[]={AI_ANIM_SWIM, AI_ANIM_SWIM2};
	int						swimweight[]={4, 2};


	AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
	if (AI_GetDyn(pAI)->m_dwStatusFlags & AI_CYCLECOMPLETED)
	{
		// swimming
		picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 2, swimtype, swimweight, NULL);
		AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
	}

	// make ai turn
	if (ang<=ANGLE_DTOR(-AI_MAX_SWIMTURN)) ang=ANGLE_DTOR(-AI_MAX_SWIMTURN);
	if (ang>=ANGLE_DTOR(+AI_MAX_SWIMTURN)) ang=ANGLE_DTOR(+AI_MAX_SWIMTURN);
	AI_NudgeRotY(pAI, frame_increment*ang/AI_SWIMTURN_SMOOTHNESS);
}


// make ai swim retreat based on given angle (radians)
//
void AI_Make_SwimRetreat(CGameObjectInstance *pAI, float ang)
{
	int						picked;
	int						swimtype[]={AI_ANIM_SWIMRETREAT};
	int						swimweight[]={1};


	// swimming
	picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 1, swimtype, swimweight, NULL);
	AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
	AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);

	// make ai turn
	if (ang<=ANGLE_DTOR(-AI_MAX_SWIMTURN)) ang=ANGLE_DTOR(-AI_MAX_SWIMTURN);
	if (ang>=ANGLE_DTOR(+AI_MAX_SWIMTURN)) ang=ANGLE_DTOR(+AI_MAX_SWIMTURN);
	AI_NudgeRotY(pAI, frame_increment*ang/(AI_SWIMTURN_SMOOTHNESS/2));
}

void AI_AssistSniperTurn(CGameObjectInstance *pAI)
{
	float		projectileang;
//				dist;
	CVector3	vDestPos;

#ifdef ALLOW_SNIPER_TURNING
	float		turnAng;
	BOOL		negative;
//	int		curAnim,
//				curAnimTest;

	int		picked;

	int		turn180type1[]={AI_ANIM_RTURN180, AI_ANIM_RTURNR90,
									 AI_ANIM_WTURN180, AI_ANIM_WTURNR90,
									 AI_ANIM_TURN180,  AI_ANIM_TURNR90 };

	int		turn180type2[]={AI_ANIM_RTURN1802, AI_ANIM_RTURNR902,
									 AI_ANIM_WTURN1802, AI_ANIM_WTURNR902,
									 AI_ANIM_TURN1802,  AI_ANIM_TURNR902 };

	int		turn180weight[]={100,     1,
									  10,      1,
									  1000000, 10000 };

	int		turnl90type1[]={AI_ANIM_RTURNL90,
									 AI_ANIM_WTURNL90,
									 AI_ANIM_TURNL90   };

	int		turnl90type2[]={AI_ANIM_RTURNL902,
									 AI_ANIM_WTURNL902,
									 AI_ANIM_TURNL902   };

	int		turnl90weight[]={1, 10000, 1000000};

	int		turnr90type1[]={AI_ANIM_RTURNR90,
									 AI_ANIM_WTURNR90,
									 AI_ANIM_TURNR90   };

	int		turnr90type2[]={AI_ANIM_RTURNR902,
									 AI_ANIM_WTURNR902,
									 AI_ANIM_TURNR902   };

	int		turnr90weight[]={1, 10000, 1000000};
#endif


	// only assist sniper if he is alive
	if (AI_GetDyn(pAI)->m_Health <= 0)
		return;

	// get projectile angle
	vDestPos = AI_GetPos(pAI->m_pAttackerCGameObjectInstance);
//	dist = AI_DistanceSquared(pAI, vDestPos);
	projectileang = AI_GetAngle(pAI, vDestPos);

#ifdef ALLOW_SNIPER_TURNING
	if (projectileang < 0)
	{
		turnAng = -projectileang;
		negative = TRUE;
	}
	else
	{
		turnAng = projectileang;
		negative = FALSE;
	}

	if (turnAng > ANGLE_DTOR(135))
	{
		// 135 - 180
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*projectileang/AI_RUNTURN_SMOOTHNESS);
			AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		}
	}
	else if (turnAng > ANGLE_DTOR(20))
	{
		// 20 - 135		(note: threshold at 20 prevents oscillating)
		if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, negative ? turnl90type2 : turnr90type2, negative ? turnl90weight : turnr90weight, NULL);
		else
			picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, negative ? turnl90type1 : turnr90type1, negative ? turnl90weight : turnr90weight, NULL);

		if (picked==-1)
		{
			AI_NudgeRotY(pAI, frame_increment*projectileang/AI_TURN_SMOOTHNESS);
		}
		else
		{
			AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
			AI_GetDyn(pAI)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
		}
	}
	else
	{
		// 0 - 20
		AI_NudgeRotY(pAI, frame_increment*projectileang/AI_TURN_SMOOTHNESS);
	}
#else
		AI_NudgeRotY(pAI, frame_increment*projectileang/AI_TURN_SMOOTHNESS);
#endif
}

BOOL AI_Make_Turn ( CGameObjectInstance *pAI, float ang )
{
	float	turnAng;
	BOOL	negative;
	int	curAnim,
			curAnimTest,
			waterFlag;

	int		picked;

	int		turn180type1[]={AI_ANIM_RTURN180, AI_ANIM_RTURNR90,
									 AI_ANIM_WTURN180, AI_ANIM_WTURNR90,
									 AI_ANIM_TURN180,  AI_ANIM_TURNR90 };

	int		turn180type2[]={AI_ANIM_RTURN1802, AI_ANIM_RTURNR902,
									 AI_ANIM_WTURN1802, AI_ANIM_WTURNR902,
									 AI_ANIM_TURN1802,  AI_ANIM_TURNR902 };

	int		turn180weight[]={100,     1,
									  10,      1,
									  1000000, 10000 };

	int		turnl90type1[]={AI_ANIM_RTURNL90,
									 AI_ANIM_WTURNL90,
									 AI_ANIM_TURNL90   };

	int		turnl90type2[]={AI_ANIM_RTURNL902,
									 AI_ANIM_WTURNL902,
									 AI_ANIM_TURNL902   };

	int		turnl90weight[]={1, 10000, 1000000};

	int		turnr90type1[]={AI_ANIM_RTURNR90,
									 AI_ANIM_WTURNR90,
									 AI_ANIM_TURNR90   };

	int		turnr90type2[]={AI_ANIM_RTURNR902,
									 AI_ANIM_WTURNR902,
									 AI_ANIM_TURNR902   };

	int		turnr90weight[]={1, 10000, 1000000};

	int		swimtype[]={AI_ANIM_IDLE_SWIM};
	int		swimweight[]={2};

	//TRACE("ang:%f\r\n", ang*180.0/ANGLE_PI);

	waterFlag = AI_GetWaterStatus(pAI);

	if (ang < 0)
	{
		turnAng = -ang;
		negative = TRUE;
	}
	else
	{
		turnAng = ang;
		negative = FALSE;
	}

	switch(waterFlag)
	{
		case AI_BELOW_WATERSURFACE:
		case AI_ON_WATERSURFACE:
			AI_GetDyn(pAI)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
			if (ang<=ANGLE_DTOR(-AI_MAX_SWIMTURN)) ang=ANGLE_DTOR(-AI_MAX_SWIMTURN);
			if (ang>=ANGLE_DTOR(+AI_MAX_SWIMTURN)) ang=ANGLE_DTOR(+AI_MAX_SWIMTURN);
			AI_NudgeRotY(pAI, frame_increment*ang/AI_TURN_SMOOTHNESS);

			if (turnAng < ANGLE_DTOR(4))
				return TRUE;

			break;

		default:
			if (turnAng > ANGLE_DTOR(135))
			{
				// 135 - 180
				if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
					picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type2, turn180weight, NULL);
				else
					picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, turn180type1, turn180weight, NULL);

				if (picked==-1)
				{
					AI_NudgeRotY(pAI, frame_increment*ang/AI_RUNTURN_SMOOTHNESS);
					AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
				}
				else
				{
					AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
					AI_GetDyn(pAI)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;
				}
			}
			else if (turnAng > ANGLE_DTOR(20))
			{
				// 20 - 135		(note: threshold at 20 prevents oscillating)
				if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
					picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, negative ? turnl90type2 : turnr90type2, negative ? turnl90weight : turnr90weight, NULL);
				else
					picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 6, negative ? turnl90type1 : turnr90type1, negative ? turnl90weight : turnr90weight, NULL);

				if (picked==-1)
				{
					AI_NudgeRotY(pAI, frame_increment*ang/AI_TURN_SMOOTHNESS);
				}
				else
				{
					AI_SetDesiredAnimByIndex(pAI, picked, FALSE);
					AI_GetDyn(pAI)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
				}
			}
			else
			{
				// 0 - 20
				curAnim = AI_GetCurrentAnim(pAI);
				if (AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_USE2NDSETOFMOVES)
				{
					curAnimTest = CGameObjectInstance__LookupAIAnimType(pAI, AI_ANIM_IDLE2);
					if (curAnim == curAnimTest)
						AI_SetDesiredAnim(pAI, AI_ANIM_IDLE2, FALSE);
				}
				else
				{
					curAnimTest = CGameObjectInstance__LookupAIAnimType(pAI, AI_ANIM_IDLE);

					if (curAnim == curAnimTest)
						AI_SetDesiredAnim(pAI, AI_ANIM_IDLE, FALSE);
				}

				AI_GetDyn(pAI)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;
				AI_NudgeRotY(pAI, frame_increment*ang/AI_TURN_SMOOTHNESS);

				if (turnAng < ANGLE_DTOR(2))
				{
					// 0 - 2

					// pointing at desired direction
					// allow avoidance
				}
				return TRUE;
			}
			break;
	}

	// don't do avoidance yet
	return FALSE;
}


// make ai move forwards based on given angle (radians)
//
void AI_Make_TeleportMoveSlow(CGameObjectInstance *pAI, float ang)
{
	int						picked;
	int						movetype[]={AI_ANIM_TELEPORT_MOVE};
	int						moveweight[]={1};


	AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
	picked = CGameObjectInstance__GetRandomAnimIndex(pAI, 1, movetype, moveweight, NULL);
	AI_SetDesiredAnimByIndex(pAI, picked, FALSE);

	// make ai turn
	if (ang<=ANGLE_DTOR(-AI_MAX_TELEPORTTURN)) ang=ANGLE_DTOR(-AI_MAX_TELEPORTTURN);
	if (ang>=ANGLE_DTOR(+AI_MAX_TELEPORTTURN)) ang=ANGLE_DTOR(+AI_MAX_TELEPORTTURN);
	AI_NudgeRotY(pAI, frame_increment*ang/AI_TURN_SMOOTHNESS);
}


// wait for animation cycle to be completed ?
//
unsigned char AI_Cycle_Completed ( CGameObjectInstance *pAI )
{
	// wait for cycle completed ?
	if ( AI_GetDyn(pAI)->m_dwStatusFlags & AI_WAITFOR_CYCLE )
	{
		// Make movement more responsive
//		if (AI_IsInMovementAnim(pAI))
		if (0)
		{
			// No interruptions on blends!
			if (!CGameObjectInstance__IsBlending(pAI))
			{
				AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
				return 2 ;
			}
			else
				return 0 ;
		}
		else
		{
			if ( AI_GetDyn(pAI)->m_dwStatusFlags & AI_CYCLECOMPLETED )
			{
				AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
				return 2;
			}
			else
				return 0;
		}
	}
	else
	{
		AI_GetDyn(pAI)->m_dwStatusFlags &= (~AI_WAITFOR_CYCLE);
		return 1;
	}
}


// can ai see target ?
//
BOOL AI_Can_See_Target(CGameObjectInstance *pMe, CGameObjectInstance *pTarget)
{
	// declare variables
	float dx, dz, dist, angle;

	// can this ai see target ?
	if (    (pTarget == NULL)
		  || (AI_GetDyn(pTarget)->m_Health == 0) )
	{
		AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_EV_SEETARGET);
		AI_GetDyn(pMe)->m_ViewAngleOffset = 0;
		return 0;
	}

	// get distance between ai & target
	dx   = AI_GetPos(pMe).x - AI_GetPos(pTarget).x;
	dz   = AI_GetPos(pMe).z - AI_GetPos(pTarget).z;

	dist = dx * dx + dz * dz;

	// did ai hear a get attention
	if ( AI_GetEA(pMe)->m_SightRadius >= dist )
	{
		angle = AI_GetAngle(pMe, AI_GetPos(pTarget)) - AI_GetDyn(pMe)->m_ViewAngleOffset;
		if (angle > ANGLE_PI)
			angle -= 2*ANGLE_PI;
		else if (angle < -ANGLE_PI)
			angle += 2*ANGLE_PI;

		// within sight angle ?
		if (		(fabs(angle) <= AI_GetEA(pMe)->m_SightAngle)
				&& CAnimInstanceHdr__CastYeThyRay(&pMe->ah, &pTarget->ah.ih) )
		{
			// make ai enter attack mode - it has spotted target
			AI_GetDyn(pMe)->m_dwStatusFlags |= AI_EV_SEETARGET;
			AI_Increase_Agitation ( pMe, AI_AGITATION_SEETARGET, 300 );
			AI_GetDyn(pMe)->m_ViewAngleOffset = 0;

			return 1;
		}
		else AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_EV_SEETARGET);
	}
	else AI_GetDyn(pMe)->m_dwStatusFlags &= (~AI_EV_SEETARGET);


	AI_GetDyn(pMe)->m_ViewAngleOffset = 0;
	return 0;
}


// make ai perform a get attention
//
void AI_Make_Get_Attention(CGameObjectInstance *pMe)
{
	int			type[]={AI_ANIM_GETATTENTION, AI_ANIM_RAGE};
	int			weight[]={5, 2};
	int			picked;


	picked = CGameObjectInstance__GetRandomAnimIndex(pMe, 2, type, weight, NULL);
	AI_SetDesiredAnimByIndex(pMe, picked, FALSE);
	AI_GetDyn(pMe)->m_dwStatusFlags |= AI_WAITFOR_CYCLE;

	// ai should get attention
	AI_Event_Dispatcher(&pMe->ah.ih, &pMe->ah.ih, AI_MEVENT_GETATTN, AI_GetEA(pMe)->m_dwSpecies, AI_GetPos(pMe), 0);
}





// decrease ai's health
//
void AI_Decrease_Health(CGameObjectInstance *pAI, int hits, BOOL FlashScreen, BOOL DoSounds)
{
	int	flash,
			flashdec,
			waterFlag,
			s,
			n;

	// Invincible cannot be hurt
	if (AI_GetDyn(pAI)->m_Invincible)
	{
		// Boss with shield?
		if ((pAI->m_pBoss) && (pAI->m_pBoss->m_pShield) && (CShield__IsOn(pAI->m_pBoss->m_pShield)))
		{
			pAI->m_pBoss->m_pShield->m_Health -= hits ;
			if (pAI->m_pBoss->m_pShield->m_Health < 0)
				pAI->m_pBoss->m_pShield->m_Health = 0 ;
		}

		// ai is invincible
		return ;
	}

	if (AI_GetDyn(pAI)->m_Health==0) return;			// ai is already dead
	if (AI_GetDyn(pAI)->m_dwStatusFlags2 & AI_HOLDINTANIMON1STFRAME) return;
	if (AI_GetDyn(pAI)->m_dwStatusFlags & AI_INTERACTIVEANIMATION) return;
	if (AI_GetDyn(pAI)->m_dwStatusFlags & AI_INTERANIMDELAY) return;
	if (CGameObjectInstance__IsDevice(pAI)) return;

	// if AI is player, check if any armor, and flash a different color
	if (CEngineApp__GetPlayer(GetApp()) == pAI)
	{
		// Doing a cinematic?
		if (CCamera__InCinemaMode(&GetApp()->m_Camera))
			return ;

		// if pausing, or cheating, don't decrease health!
		if ((GetApp()->m_dwCheatFlags & CHEATFLAG_INVINCIBILITY) || (GetApp()->m_bPause))
			return ;

		// invincible?? then do nothing!
		if (CTurokMovement.SpiritualTime)
			return ;

		// which injury sfx shall we play ?
		waterFlag = CEngineApp__GetPlayerWaterStatus(GetApp());
		switch (waterFlag)
		{
			default:
			case PLAYER_NOT_NEAR_WATER:
			case PLAYER_ABOVE_WATERSURFACE:
			case PLAYER_ON_WATERSURFACE:
				n = RANDOM(4);
				switch (n)
				{
					case 0:	s = SOUND_GENERIC_10;		break;
					case 1:	s = SOUND_GENERIC_11;		break;
					case 2:	s = SOUND_GENERIC_12;		break;
					case 3:	s = SOUND_GENERIC_13;		break;
				}
				break;

			case PLAYER_BELOW_WATERSURFACE:
				if (CTurokMovement.InAntiGrav)
				{
					n = RANDOM(4);
					switch (n)
					{
						case 0:	s = SOUND_GENERIC_10;		break;
						case 1:	s = SOUND_GENERIC_11;		break;
						case 2:	s = SOUND_GENERIC_12;		break;
						case 3:	s = SOUND_GENERIC_13;		break;
					}
				}
				else
				{
					n = RANDOM(2);
					switch (n)
					{
						case 0:	s = SOUND_GENERIC_14;		break;
						case 1:	s = SOUND_GENERIC_15;		break;
					}
				}
				break;
		}
		if (DoSounds && CTurokMovement.ArmorFlag==0)
		{
			if (CTurokMovement.HitSoundTimerScream)
			{
				if (CTurokMovement.HitSoundTimer >= TMOVE_HITSOUND_TIMER_SCREAM)
				{
					CTurokMovement.HitSoundTimer = TMOVE_HITSOUND_TIMER_MAX;
					CTurokMovement.HitSoundTimerScream = FALSE;
				}
			}
			else
			{
				if (CTurokMovement.HitSoundTimer < TMOVE_HITSOUND_TIMER_MAX)
				{
					CTurokMovement.HitSoundTimer = 0.0;
					CTurokMovement.HitSoundTimerScream = TRUE;

					// do special scream because turok is being hit alot
					// for the now lets just do the normal grunt sfx
					switch(waterFlag)
					{
						case PLAYER_NOT_NEAR_WATER:
						case PLAYER_ABOVE_WATERSURFACE:
						case PLAYER_ON_WATERSURFACE:
							AI_DoSound(&pAI->ah.ih, s, 1, 0);
							break;

						case PLAYER_BELOW_WATERSURFACE:
							AI_DoSound(&pAI->ah.ih, s, 1, 0);
							break;
					}
				}
				else
				{
					AI_DoSound(&pAI->ah.ih, s, 1, 0);
					CTurokMovement.HitSoundTimer = 0.0;
				}
			}
		}


		if (CTurokMovement.ArmorFlag)
		{
			//CTurokMovement.ArmorAmount = max(0, CTurokMovement.ArmorAmount - hits);
			CTurokMovement.ArmorAmount -= hits ;

			// calc flash size & duration
			if (FlashScreen)
			{
				flash = 255*hits/20;
				flash = max(128, min(255, flash));

				flashdec = 30;

				CEngineApp__DoColorFlash(GetApp(), flash, flashdec, 255, 255, 0);
			}

			// if damage is more than armor can take, spill over to normal damage...
			if (CTurokMovement.ArmorAmount <= 3)
			{
				hits = 3-CTurokMovement.ArmorAmount ;
				CTurokMovement.ArmorFlag = FALSE;
				CTurokMovement.ArmorAmount = 0;
			}
			else
				hits = 0 ;
		}

		if (hits)
		{
			// decrease health

			AI_GetDyn(pAI)->m_Health = max(0, AI_GetDyn(pAI)->m_Health - hits);

			// calc flash size & duration
			if (FlashScreen)
			{
				if (AI_GetDyn(pAI)->m_Health)
				{
					flash = 255*hits/20;
					flash = max(128, min(255, flash));

					flashdec = 30;
				}
				else
				{
					flash = 255;

					flashdec = 12;
				}

				// flash screen
				CEngineApp__DoFlash(GetApp(), flash, flashdec);
			}
		}
	}
	// non player ai
	else
	{
		// decrease health
		AI_GetDyn(pAI)->m_Health -= hits;

		// does this character generate pickups when damaged ?
		if (		(AI_GetDyn(pAI)->m_Health)
				&&	(AI_GetEA(pAI)->m_dwTypeFlags2 & AI_TYPE2_DOPICKUPONDAMAGE) )
		{
			AI_GeneratePickups(pAI, pAI->ah.ih.m_vPos);
		}
	}


	// is ai now dead ?
	if ( AI_GetDyn(pAI)->m_Health <= 0 )
	{
		AI_GetDyn(pAI)->m_Health = 0;			// set health to zero

		// do the following so final death anim will play immediately
		AI_GetDyn(pAI)->m_dwStatusFlags &= ~AI_ALREADY_DEAD;
		AI_GetDyn(pAI)->m_dwStatusFlags &= ~AI_WAITFOR_CYCLE;

		if (		(AI_CanWeRemoveIt(pAI))
				&& (CEngineApp__GetPlayer(GetApp()) != pAI)
				&&	(!AI_DoesItRegenerate(pAI)) )
		{
			CScene__SetInstanceFlag(&GetApp()->m_Scene, pAI, TRUE);
		}

		if (AI_GetEA(pAI)->m_DeathId)
			AI_Event_Dispatcher (&pAI->ah.ih, &pAI->ah.ih, AI_MEVENT_PRESSUREPLATE, AI_SPECIES_ALL, AI_GetPos(pAI), (float)AI_GetEA(pAI)->m_DeathId);
	}
}


float AI_DistanceSquared(CGameObjectInstance *pAI, CVector3 vPos)
{
	CVector3	vThisPos ;
	float		dx, dy, dz ;

	vThisPos = AI_GetPos(pAI) ;

	dx = vThisPos.x - vPos.x ;
	dy = (vThisPos.y + pAI->ah.ih.m_CollisionHeightOffset) - vPos.y ;
	dz = vThisPos.z - vPos.z ;

	return ((dx*dx) + (dy*dy) + (dz*dz)) ;
}


float AI_XZDistanceSquared(CGameObjectInstance *pAI, CVector3 vPos)
{
	CVector3	vThisPos ;
	FLOAT		dx, dz ;

	vThisPos = AI_GetPos(pAI) ;

	dx = vThisPos.x - vPos.x ;
	dz = vThisPos.z - vPos.z ;

	return ((dx*dx) + (dz*dz)) ;
}


BOOL AI_SetDesiredAnim(CGameObjectInstance *pAI, int nAnim, BOOL InterruptBlend)
{
	int lookupAnim;

	lookupAnim = CGameObjectInstance__LookupAIAnimType(pAI, nAnim);
	return AI_SetDesiredAnimByIndex(pAI, lookupAnim, InterruptBlend);
}

BOOL AI_SetDesiredAnimByIndex(CGameObjectInstance *pAI, int nAnim, BOOL InterruptBlend)
{
	ASSERT(nAnim >= -1);

	if (nAnim == -1)
			return FALSE;

	if (!InterruptBlend)
	{
		if (CGameObjectInstance__IsBlending(pAI))
			return TRUE;
	}

	CGameObjectInstance__SetDesiredAnim(pAI, nAnim);
	return TRUE;
}


int AI_GetCurrentAnim(CGameObjectInstance *pAI)
{
	return CGameObjectInstance__GetCurrentAnim(pAI);
}


// get material type of object
//
int AI_GetMaterial(CInstanceHdr *pThis)
{
	int nType;

	nType = CInstanceHdr__TypeFlag(pThis);
	if ((nType >= AI_OBJECT_WEAPON_START) && (nType <= AI_OBJECT_WEAPON_END))
		nType = AI_OBJECT_CHARACTER_PLAYER;

	switch (nType)
	{
		// special human case
		case AI_OBJECT_CHARACTER_HUMAN1:
			switch(pThis->m_pEA->m_AttackStyle)
			{
				// cyborg 25/26 robosuit 27
				case 25:
				case 26:
				case 27:
					return REGMAT_STEEL;

				default:
					return REGMAT_FLESH;
			}

		// Red blood
		case AI_OBJECT_CHARACTER_PLAYER:
		case AI_OBJECT_CHARACTER_RAPTOR:
		case AI_OBJECT_CHARACTER_SABERTIGER:
		case AI_OBJECT_CHARACTER_DIMETRODON:
		case AI_OBJECT_CHARACTER_TRICERATOPS:
		case AI_OBJECT_CHARACTER_MOSCHOPS:
		case AI_OBJECT_CHARACTER_PTERANODON:
		case AI_OBJECT_CHARACTER_LEAPER:
		case AI_OBJECT_CHARACTER_HULK:
		case AI_OBJECT_STATIC_FLESH:
		case AI_OBJECT_CHARACTER_BADFISH:
		case AI_OBJECT_CHARACTER_GOODFISH:
		case AI_OBJECT_CHARACTER_TREX_BOSS:
		case AI_OBJECT_CHARACTER_CAMPAIGNER_BOSS:
		case AI_OBJECT_CHARACTER_LONGHUNTER_BOSS:
		case AI_OBJECT_CHARACTER_GENERICRED:
		case AI_OBJECT_CHARACTER_SLUDGEBOY:
			return REGMAT_FLESH;

		// Green blood
		case AI_OBJECT_CHARACTER_ALIEN:
		case AI_OBJECT_STATIC_ALIENFLESH:
		case AI_OBJECT_CHARACTER_GENERICGREEN:
		case AI_OBJECT_CHARACTER_PLANT:
		case AI_OBJECT_CHARACTER_SUBTERRANEAN:
		case AI_OBJECT_CHARACTER_MANTIS_BOSS:
			return REGMAT_ALIENFLESH;

		case AI_OBJECT_STATIC_STONE:
			return REGMAT_STONE;

		case AI_OBJECT_STATIC_WATERSURFACE:
			return REGMAT_WATERSURFACE;

//		case AI_OBJECT_STATIC_WATERFLOOR:
//			return REGMAT_WATERFLOOR;

		case AI_OBJECT_DEVICE_EXPTARGET:
		case AI_OBJECT_CHARACTER_ROBOT:
		case AI_OBJECT_STATIC_STEEL:
		case AI_OBJECT_CHARACTER_HUMVEE_BOSS:
		case AI_OBJECT_CEILING_TURRET:
		case AI_OBJECT_BUNKER_TURRET:
		case AI_OBJECT_CHARACTER_GENERICMETAL:
			return REGMAT_STEEL;

//		case AI_OBJECT_STATIC_FOLIAGE:
//			return REGMAT_FOLIAGE;

		case AI_OBJECT_STATIC_GRASS:
			return REGMAT_GRASS;

//		case AI_OBJECT_STATIC_DIRT:
//			return REGMAT_DIRT;
	}

	// default
	return REGMAT_GRASS;
}

void AI_SetAttacker(CGameObjectInstance *pThis, CInstanceHdr *pOrigin)
{
	CGameObjectInstance	*pAttacker;
	CInstanceHdr			*pSource;

	if (!pOrigin)
		return;

	switch (pOrigin->m_Type)
	{
		case I_PARTICLE:
			pSource = ((CParticle*)pOrigin)->m_pSource;
			if (pSource && (pSource->m_Type == I_ANIMATED))
				pAttacker = (CGameObjectInstance*) pSource;
			else
				pAttacker = NULL;
			break;

		case I_ANIMATED:
			pAttacker = (CGameObjectInstance*) pOrigin;
			break;

		default:
			pAttacker = NULL;
			break;
	}

	if (pAttacker)
	{
		if (pAttacker->ah.ih.m_nObjType != pThis->ah.ih.m_nObjType)
		{
			if (!(AI_GetEA(pAttacker)->m_dwTypeFlags & AI_TYPE_CANNOTCAUSEAFIGHT))
				pThis->m_pAttackerCGameObjectInstance = pAttacker;
		}
	}
}


float AI_GetWaterHeight(CGameObjectInstance *pThis)
{
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;


	if (!pThis)
		return 0;

	pRegion = pThis->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return 0;

	if (!(pRegion->m_wFlags & REGFLAG_HASWATER))
		return 0;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pRegion);
	if (!pRegionSet)
		return 0;

	// is ai in a water region ?
	return pRegionSet->m_WaterElevation;
}


int AI_GetWaterStatus(CGameObjectInstance *pThis)
{
	CGameRegion				*pRegion;
	CROMRegionSet			*pRegionSet;
	int						waterFlag;
	float						EyeY;

	waterFlag = AI_NOT_NEAR_WATER;

	if (!pThis)
		return waterFlag;

	pRegion = pThis->ah.ih.m_pCurrentRegion;
	if (!pRegion)
		return waterFlag;

	pRegionSet = CScene__GetRegionAttributes(&GetApp()->m_Scene, pRegion);
	if (!pRegionSet)
		return waterFlag;

	// calc eye pos
	EyeY = AI_GetEA(pThis)->m_CollisionHeight + AI_GetPos(pThis).y;

	// is ai in a water region ?
	if (		(pRegion->m_wFlags & REGFLAG_HASWATER)
			&&	(!(pRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)) )
	{
		if (EyeY < pRegionSet->m_WaterElevation)
		{
			// ai is below water surface level
//			if (pRegionSet->m_dwFlags & REGFLAG_SHALLOWWATER)
//				waterFlag = AI_ON_WATERSURFACE;
//			else
				waterFlag = AI_BELOW_WATERSURFACE;
		}
		else if (AI_GetPos(pThis).y >= pRegionSet->m_WaterElevation)
		{
			// ai is above water surface level
			waterFlag = AI_ABOVE_WATERSURFACE;
		}
		else
		{
			// ai is on water surface
			waterFlag = AI_ON_WATERSURFACE;
		}
	}
	else
	{
		// ai is not in a water region
		waterFlag = AI_NOT_NEAR_WATER;
	}


	// if ai is in shallow water & is touching ground make them move like they are not in water
//	if (		(CInstanceHdr__IsOnGround(&pThis->ah.ih))
//			&&	(waterFlag == AI_ON_WATERSURFACE) )
//	{
//		waterFlag = AI_NOT_NEAR_WATER;
//	}

	return waterFlag;
}


// make sure dead water ai's stays in the water
//
void AI_StayInWater(CGameObjectInstance *pThis)
{
	int		waterFlag;
	float		waterHeight,
				insHeight,
				dm;


	waterFlag = AI_GetWaterStatus(pThis);

	if (    (!AI_GetDyn(pThis)->m_Health)
		  && (    (waterFlag == AI_ON_WATERSURFACE)
		       || (waterFlag == AI_ABOVE_WATERSURFACE) ) )
	{
		waterHeight = AI_GetWaterHeight(pThis);
		insHeight = AI_GetPos(pThis).y ;
		dm = (waterHeight - insHeight) / 5;
		AI_GetPos(pThis).y += dm * frame_increment * 2;
		pThis->ah.m_vVelocity.y = 0;
	}
}


// does this ai float when dead ?
//
BOOL AI_DoesItFloat(CGameObjectInstance *pThis)
{
	return (BOOL) (AI_GetEA(pThis)->m_dwTypeFlags & AI_TYPE_FLOATINWATERONDEATH);
}


// update melt timer if ready for melting
//
void AI_UpdateMeltTimer(CGameObjectInstance *pThis)
{
	if (AI_GetDyn(pThis)->m_MeltTimer)
	{
		if (		(AI_GetDyn(pThis)->m_dwStatusFlags & AI_ALREADY_DEAD)
				&& (AI_GetDyn(pThis)->m_Health <= 0)
				&& (!(AI_GetDyn(pThis)->m_dwStatusFlags & AI_RESURRECTION))
				&& (!(AI_GetDyn(pThis)->m_dwStatusFlags & AI_FEIGNDEATH))
				&&	(CInstanceHdr__IsOnGround(&pThis->ah.ih)) )
		{
			AI_GetDyn(pThis)->m_MeltTimer -= frame_increment*(1.0/FRAME_FPS);
			if (AI_GetDyn(pThis)->m_MeltTimer < 0.0)
				AI_GetDyn(pThis)->m_MeltTimer = 0.0;
		}
	}
}


// can this ai be removed from the game ?
//
BOOL AI_CanWeKillIt(CGameObjectInstance *pThis)
{
	if (		(AI_GetDyn(pThis)->m_dwStatusFlags & AI_ALREADY_DEAD)
			&& (AI_GetDyn(pThis)->m_Health <= 0)
			&& (!(AI_GetDyn(pThis)->m_dwStatusFlags & AI_RESURRECTION))
			&& (!(AI_GetDyn(pThis)->m_dwStatusFlags & AI_FEIGNDEATH))
			&& (AI_GetDyn(pThis)->m_MeltTimer == 0.0) )
	{
		return TRUE;
	}

	return FALSE;
}

// is this ai effectively gone ?
//
BOOL AI_CanWeRemoveIt(CGameObjectInstance *pThis)
{
	if (		(AI_GetDyn(pThis)->m_Health <= 0)
			&& (!(AI_GetDyn(pThis)->m_dwStatusFlags & AI_RESURRECTION))
			&& (!(AI_GetDyn(pThis)->m_dwStatusFlags & AI_FEIGNDEATH))
			/*&& AI_DoesItMelt(pThis)*/ )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL AI_DoesItMelt(CGameObjectInstance *pThis)
{
	return (AI_GetEA(pThis)->m_dwTypeFlags & AI_TYPE_MELTINGDEATH);
}


void AI_GeneratePickups(CGameObjectInstance *pThis, CVector3 vPos)
{
	DWORD		pickups;
	UINT32	i, mask;
	int		type;

	pickups = AI_GetEA(pThis)->m_dwTypeFlags2;

	mask = 1 ;
	i = 31 ;
	while (i--)
	{
		if (pickups & mask)
		{
			switch (mask)
			{
				case	AI_TYPE2_PICKUP_EXPLOSIVESHELLS:
					type = AI_OBJECT_PICKUP_AMMO_EXPSHOTGUN ;
					break ;
				case	AI_TYPE2_PICKUP_GRENADE:
					type = AI_OBJECT_PICKUP_AMMO_GRENADES ;
					break ;
				case	AI_TYPE2_PICKUP_HEALTH10:
					type = AI_OBJECT_PICKUP_HEALTH_10 ;
					break ;
				case	AI_TYPE2_PICKUP_HEALTH100:
					type = AI_OBJECT_PICKUP_HEALTH_100 ;
					break ;
				case	AI_TYPE2_PICKUP_HEALTH100PLUS:
					type = AI_OBJECT_PICKUP_HEALTH_100PLUS ;
					break ;
				case	AI_TYPE2_PICKUP_HEALTH2:
					type = AI_OBJECT_PICKUP_HEALTH_2 ;
					break ;
				case	AI_TYPE2_PICKUP_HEALTH25:
					type = AI_OBJECT_PICKUP_HEALTH_25 ;
					break ;
				case	AI_TYPE2_PICKUP_MINIGUNSHELLS:
					type = AI_OBJECT_PICKUP_AMMO_MINIGUN ;
					break ;
				case	AI_TYPE2_PICKUP_MORTALWOUND:
					// random chance on mortal wound
					if (AI_IsMortalWound(pThis))
					{
						if (RANDOM(100) < 15)
							type = AI_OBJECT_PICKUP_MORTAL_WOUND ;
						else
							type = 0;
					}
					else
					{
						type = 0;
					}
					break ;
				case	AI_TYPE2_PICKUP_ROCKETS:
					type = AI_OBJECT_PICKUP_AMMO_ROCKETS ;
					break ;
				case	AI_TYPE2_PICKUP_SHOTGUNSHELLS:
					type = AI_OBJECT_PICKUP_AMMO_SHOTGUN ;
					break ;
				case	AI_TYPE2_PICKUP_BATTERYLARGE:
					type = AI_OBJECT_PICKUP_AMMO_TECHSMALL ;
					break ;
				case	AI_TYPE2_PICKUP_BATTERYSMALL:
					type = AI_OBJECT_PICKUP_AMMO_TECHLARGE ;
					break ;
				case	AI_TYPE2_PICKUP_BULLETCLIP:
					type = AI_OBJECT_PICKUP_AMMO_BULLETCLIP ;
					break ;

				default:
					type = 0;
					break ;
			}

			if (type)
				AI_DoPickup(&pThis->ah.ih, vPos, type, SECONDS_TO_FRAMES(8)) ;
		}
		mask <<= 1 ;
	}
}


CDynamicSimple *AI_DoPickup(CInstanceHdr *pThis, CVector3 vPos, int nType, FLOAT Time)
{
	CVector3		vUp, vRandom, vScaled,//vFinal,
					vPickupPos;
	CGameRegion	*pPickupRegion;
	CDynamicSimple	*pDyn ;

	// get pickup position
	CInstanceHdr__GetNearPositionAndRegion(pThis, vPos, &vPickupPos, &pPickupRegion, INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);

	vUp.x = 0;
	vUp.y = 1;
	vUp.z = 0;

	vRandom = CVector3__RandomizeDirection(&vUp, 0.25);
	CVector3__Normalize(&vRandom) ;

  	CVector3__MultScaler(&vScaled, &vRandom, 40*SCALING_FACTOR);

	//CVector3__MultScaler(&vFinal, &vScaled, 15) ;

	// Special case time
	if (nType == AI_OBJECT_PICKUP_MORTAL_WOUND)
		Time = SECONDS_TO_FRAMES(4) ;

	pDyn = CSimplePool__CreateSimpleInstance(&GetApp()->m_Scene.m_SimplePool, nType,
													     vScaled,
//														  vFinal,
														  vPickupPos, pPickupRegion, Time);

	return pDyn ;
}




void AI_SEvent_GenerateBouncy(CInstanceHdr *pThis, CVector3 vPos, int nType)
{
	CVector3		vUp, vRandom, vScaled,
					vOutPos;
	CVector3		vFinal ;
	CGameRegion	*pOutRegion;

	CDynamicSimple *pDyn ;
	CVector3			vN, vV, vAxis ;
	float				r ;


	// get collision free position
	CInstanceHdr__GetNearPositionAndRegion(pThis, vPos, &vOutPos, &pOutRegion, INTERSECT_BEHAVIOR_IGNORE, INTERSECT_BEHAVIOR_IGNORE);

	vUp.x = 0;
	vUp.y = 1;
	vUp.z = 0;

	// rock velocity
	if (nType == AI_OBJECT_BOUNCY_ROCK)
	{
		vRandom = CVector3__RandomizeDirection(&vUp, 0.4);
		CVector3__Normalize(&vRandom) ;

		CVector3__MultScaler(&vScaled, &vRandom, RANDOM(1000)*0.001*6*SCALING_FACTOR);
		if (vScaled.y < (2*SCALING_FACTOR))
			vScaled.y = (2 + (RANDOM(4))) *SCALING_FACTOR;
	}
	// bodypart velocity, bigger than rock coz spawned by an explosion
	else
	{
		vRandom = CVector3__RandomizeDirection(&vUp, 0.3);
		CVector3__Normalize(&vRandom) ;

		CVector3__MultScaler(&vScaled, &vRandom, (3+RANDOM(2))*SCALING_FACTOR);
	}

	// make compatable with new system
	CVector3__MultScaler(&vFinal, &vScaled, 15) ;

	pDyn = CSimplePool__CreateSimpleInstance(&GetApp()->m_Scene.m_SimplePool, nType,
												 vFinal,
												 vOutPos, pOutRegion, SECONDS_TO_FRAMES(10));

	if (pDyn)
	{
		switch (nType)
		{
			case AI_OBJECT_BOUNCY_ROCK:
				pDyn->s.m_Scale = 0.001 + (RANDOM(100)*0.01 * 0.004) ;
				pDyn->s.m_OffsetY = ((pDyn->s.m_Bounds.m_vMax.y - pDyn->s.m_Bounds.m_vMin.y)/2)*pDyn->s.m_Scale;
				break ;

				case AI_OBJECT_BOUNCY_ALIEN_HEAD:
					pDyn->s.m_Scale = .6 ;
					pDyn->s.m_OffsetY = 1*SCALING_FACTOR ;
					break ;
				case AI_OBJECT_BOUNCY_ALIEN_HAND:
				case AI_OBJECT_BOUNCY_ALIEN_FOOT:
				case AI_OBJECT_BOUNCY_ALIEN_TORSO:
					pDyn->s.m_Scale = .5 ;
					pDyn->s.m_OffsetY = 1*SCALING_FACTOR ;
					break ;


				case AI_OBJECT_BOUNCY_LEAPER_HEAD:
					pDyn->s.m_Scale = .4 ;
					pDyn->s.m_OffsetY = 1*SCALING_FACTOR ;
					break ;
				case AI_OBJECT_BOUNCY_LEAPER_HAND:
				case AI_OBJECT_BOUNCY_LEAPER_FOOT:
				case AI_OBJECT_BOUNCY_LEAPER_TORSO:
					pDyn->s.m_Scale = .3 ;
					pDyn->s.m_OffsetY = 1*SCALING_FACTOR ;
					break ;
		}

		pDyn->s.m_wFlags = SIMPLE_FLAG_VISIBLE | SIMPLE_FLAG_DYNAMIC ;

		// set initial spin based on ground normal and initial velocity
		pDyn->m_LifeTime = 60 ;

		r = 40 *15;
		// make bloody bits spin slower
		if ((nType == AI_OBJECT_BOUNCY_ALIEN_HEAD) ||
			 (nType == AI_OBJECT_BOUNCY_ALIEN_TORSO) ||
			 (nType == AI_OBJECT_BOUNCY_LEAPER_HEAD) ||
			 (nType == AI_OBJECT_BOUNCY_LEAPER_TORSO))
			 r = 100 *15;


		vN = CGameRegion__GetGroundUnitNormal(pDyn->s.ah.ih.m_pCurrentRegion) ;
		vV = vFinal ;
		CVector3__Normalize(&vV) ;
		CVector3__Cross(&vAxis, &vN, &vV) ;
		CVector3__Normalize(&vAxis) ;
		pDyn->s.m_AngleVel = (CVector3__Magnitude(&pDyn->s.ah.m_vVelocity) / r) ;
		pDyn->s.vRotAxis = vAxis ;
	}

}

BOOL AI_IsMortalWound(CGameObjectInstance *pThis)
{
	return (AI_GetDyn(pThis)->m_dwStatusFlags & AI_WOUNDEDMORTALLY);
}


BOOL AI_IsPickup(int nType)
{
	return ((nType >= AI_OBJECT_PICKUP_START) && (nType <= AI_OBJECT_PICKUP_END));
}

BOOL AI_IsWarp(int nType)
{
	return ((nType >= AI_OBJECT_WARP_START) && (nType <= AI_OBJECT_WARP_END));
}

BOOL AI_IsMorph(int nType)
{
	return ((nType >= AI_OBJECT_MORPH_START) && (nType <= AI_OBJECT_MORPH_END));
}

BOOL AI_IsBouncy(int nType)
{
	return ((nType >= AI_OBJECT_BOUNCY_START) && (nType <= AI_OBJECT_BOUNCY_END));
}


BOOL AI_DoesItAvoidWater(CGameObjectInstance *pThis)
{
	return (AI_GetEA(pThis)->m_dwTypeFlags & AI_TYPE_AVOIDWATER);
}


BOOL AI_DoesItRegenerate(CGameObjectInstance *pThis)
{
	return (BOOL) (AI_GetDyn(pThis)->m_Regenerate);
}


BOOL AI_DoesItFireProjectiles(CGameObjectInstance *pThis)
{
	return (		(AI_GetEA(pThis)->m_dwTypeFlags  & (AI_TYPE_PROJECTILEWEAPON1  | AI_TYPE_PROJECTILEWEAPON2))
				||	(AI_GetEA(pThis)->m_dwTypeFlags2 & (AI_TYPE2_PROJECTILEWEAPON3 | AI_TYPE2_PROJECTILEWEAPON4 | AI_TYPE2_PROJECTILEWEAPON5 | AI_TYPE2_PROJECTILEWEAPON6 | AI_TYPE2_PROJECTILEWEAPON7 | AI_TYPE2_PROJECTILEWEAPON8))
				||	(AI_GetEA(pThis)->m_wTypeFlags3 & (AI_TYPE3_PROJECTILEWEAPON9 | AI_TYPE3_PROJECTILEWEAPON10)) );
}

void AI_CompletelyKillOff(CGameObjectInstance *pThis)
{
	AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_RESURRECTION;
	AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_FEIGNDEATH;
	AI_GetDyn(pThis)->m_dwStatusFlags &= ~AI_REGENERATE;
	AI_GetDyn(pThis)->m_Regenerate = 0;					// no more regenerations
	AI_Decrease_Health(pThis, 999999, FALSE, TRUE);
}

BOOL AI_IsSfxUnderwater(int nType)
{
	switch(nType)
	{
		case	SOUND_MENU_OPTION_SELECT:
		case	SOUND_GAME_SAVED:
		case	SOUND_WATER_SPLASH_1:
		case	SOUND_WATER_SPLASH_2:
		case	SOUND_WATER_SPLASH_3:
		case	SOUND_WATER_SPLASH_4:
		case	SOUND_WATER_SPLASH_5:
		case	SOUND_UNDERWATER_SWIM_1:
		case	SOUND_UNDERWATER_SWIM_2:
		case	SOUND_UNDERWATER_SWIM_3:
		case	SOUND_UNDERWATER_SWIM_4:
		case	SOUND_UNDERWATER_SWIM_5:
		case	SOUND_GENERIC_1:
		case	SOUND_GENERIC_2:
		case	SOUND_GENERIC_3:
		case	SOUND_GENERIC_4:
		case	SOUND_GENERIC_5:
		case	SOUND_GENERIC_6:
		case	SOUND_GENERIC_7:
		case	SOUND_GENERIC_14:
		case	SOUND_GENERIC_15:
		case	SOUND_GENERIC_16:
		case	SOUND_GENERIC_17:
		case	SOUND_GENERIC_21:
		case	SOUND_GENERIC_22:
		case	SOUND_HUMAN_EFFORTINJURY_GRUNT_3:
		case	SOUND_WATERFALL_RUSH:
		case	SOUND_HEALTH_PICKUP_1:
		case 21:								// gunshot sound, used during menu selections...
		case	SOUND_GENERIC_218:				// Enemy Greneade
		case	SOUND_GENERIC_133:				// Underwater Leaper SFX
		case	SOUND_GENERIC_134:
		case	SOUND_GENERIC_135:
		case	SOUND_GENERIC_136:
		case	SOUND_GENERIC_137:              // End Leaper
		case	SOUND_GYSER_SPOUTING:

			return TRUE;

		default:
			return FALSE;
	}
}

//	Random big swoosh sound
void AI_SEvent_RandBigSwooshSnd(CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Random big swoosh sound\r\n") ;
	if (RANDOM(2))
		AI_DoSound(pThis, SOUND_GENERIC_178, 1, 0) ;
	else
		AI_DoSound(pThis, SOUND_GENERIC_179, 1, 0) ;
}

//	Random small swoosh sound
void AI_SEvent_RandSmallSwooshSnd(CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	TRACE0("S Event Random small swoosh sound\r\n") ;
	if (RANDOM(2))
		AI_DoSound(pThis, SOUND_GENERIC_180, 1, 0) ;
	else
		AI_DoSound(pThis, SOUND_GENERIC_181, 1, 0) ;
}


//	Random grunt swoosh sound
void AI_SEvent_RandGruntSnd(CInstanceHdr *pThis, CVector3 vPos, float Value )
{
	BOOL		DoSound = TRUE ;
	CCampaigner *pCampaigner = NULL ;

	TRACE0("S Event Random grunt sound\r\n") ;

	// Campaigner special case
	if (pThis->m_Type == I_ANIMATED)
		pCampaigner = (CCampaigner *)((CGameObjectInstance*)pThis)->m_pBoss ;

	if (pCampaigner)
	{
		if (pCampaigner->m_PlayingSoundTimer > 0)
			DoSound = FALSE ;
		else
			pCampaigner->m_PlayingSoundTimer = SECONDS_TO_FRAMES(0.5) ;
	}

	// Play the grunt
	if (DoSound)
	{
		if (RANDOM(2))
			AI_DoSound(pThis, SOUND_GENERIC_171, 1, 0) ;
		else
			AI_DoSound(pThis, SOUND_GENERIC_172, 1, 0) ;
	}
}


void AI_SEvent_StartFireSound(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
	CGameObjectInstance	*pAttacker = NULL ;
	CInstanceHdr			*pSource = NULL ;
	CParticle				*pParticle = NULL ;

	switch (pThis->m_Type)
	{
		case I_ANIMATED:
			pAttacker = (CGameObjectInstance *)pThis ;
			break ;

		case I_PARTICLE:
			pParticle = (CParticle*)pThis ;
			pSource = pParticle->m_pSource;
			if (pSource && (pSource->m_Type == I_ANIMATED))
				pAttacker = (CGameObjectInstance*)pSource ;
			break ;
	}

	// TRex special case
	if ((pAttacker) && (CInstanceHdr__TypeFlag(&pAttacker->ah.ih) == AI_OBJECT_CHARACTER_TREX_BOSS))
		TRex_StartFireSound(pAttacker, (CTRex *)pAttacker->m_pBoss) ;
}

void AI_SEvent_StopFireSound(CInstanceHdr *pThis, CVector3 vPos, float Value)
{
}

float AI_GetAvoidanceRadiusFactor(CGameObjectInstance *pThis)
{
	float avoidrad;


	switch(CGameObjectInstance__TypeFlag(pThis))
	{
		case AI_OBJECT_CHARACTER_HUMAN1:
			avoidrad = 2.5;
			break;

		case AI_OBJECT_CHARACTER_PTERANODON:
			avoidrad = 2.8;
			break;

		default:
			avoidrad = AVOIDANCE_RADIUS_DISTANCE_FACTOR;
	}

	return avoidrad;
}


// Returns zig zag position
CVector3 AI_GetZigZagPosition(CGameObjectInstance *pThis, CGameObjectInstance *pTarget)
{
	CVector3		vPos ;
	FLOAT			Angle, Radius ;
	CLine			Line ;

	// Setup target as being target instance position
	vPos = pTarget->ah.ih.m_vPos ;

// TAKEN OUT FOR NOW
#if 0
	// Don't zig zag to other enemies
	if (CEngineApp__GetPlayer(GetApp()) != pTarget)
		return vPos ;

	// Get angle of sine
	Angle = (game_frame_number * frame_increment * ZIG_ZAG_SPEED) +
			  (FLOAT)CGameObjectInstance__MagicNumber(pThis) ;

	// Calculate radius of sine
	Radius = (sqrt(AI_DistanceSquared(pThis, vPos)) - (ZIG_ZAG_CUT_OFF_DIST)) * ZIG_ZAG_SCALE_FACTOR ;
	if (Radius > MAX_ZIG_ZAG_RADIUS)
		Radius = MAX_ZIG_ZAG_RADIUS ;
	else
	if (Radius < 0)
		Radius = 0 ;

	// Add sine along normal
	CLine__Construct(&Line, &pThis->ah.ih.m_vPos, &vPos) ;
	CVector3__Normalize(&Line.m_vNormal) ;
	vPos.x += Line.m_vNormal.x * Radius * sin(Angle) ;
	vPos.z += Line.m_vNormal.z * Radius * cos(Angle) ;
#endif

	return vPos ;
}




