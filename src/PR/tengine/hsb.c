// hsb.cpp

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "debug.h"

#ifdef WIN32
#include <math.h>
#pragma warning(disable : 4244)

#define ANGLE_PI			3.14159265358979323846
#define ANGLE_DTOR(deg)	(deg*(ANGLE_PI/180))
#endif

/////////////////////////////////////////////////////////////////////////////
// CParticleSettingsDlg dialog

/*
void WriteHueTable()
{
	float theta,
			l1, l2, me, r1, r2;

	FILE *pFileR = fopen("gr_ht.bin", "wb");
	FILE *pFileG = fopen("gg_ht.bin", "wb");
	FILE *pFileB = fopen("gb_ht.bin", "wb");
	if (pFileR && pFileG && pFileB)
	{
		for (int o=-128; o<128; o++)
		{
			theta = o * ANGLE_PI/127.0;

			l1 = max(0, cos(theta - ANGLE_PI));
			l2 = max(0, cos(theta - ANGLE_PI/2.0));
			me = max(0, cos(theta));
			r1 = max(0, cos(theta + ANGLE_PI/2.0));
			r2 = max(0, cos(theta + ANGLE_PI));

			BYTE r = (BYTE) min(255, (l2 + r2)*255);
			fwrite(&r, 1, 1, pFileR);

			BYTE g = (BYTE) min(255, me*255);
			fwrite(&g, 1, 1, pFileG);

			BYTE b = (BYTE) min(255, (l1 + r1)*255);
			fwrite(&b, 1, 1, pFileB);
		}

		fclose(pFileR);
		fclose(pFileG);
		fclose(pFileB);
	}
}
*/
/*
void GetOffsetHue(signed char Offset,
						BYTE inR, BYTE inG, BYTE inB,
						BYTE *pOutR, BYTE *pOutG, BYTE *pOutB)
{
	float theta,
			l1, l2, me, r1, r2;
	int	or, og, ob,
			oldMax, maxColor;

	if (Offset)
	{
		theta = Offset * ANGLE_PI/127.0;

		l1 = max(0, cos(theta - ANGLE_PI));
		l2 = max(0, cos(theta - ANGLE_PI/2.0));
		me = max(0, cos(theta));
		r1 = max(0, cos(theta + ANGLE_PI/2.0));
		r2 = max(0, cos(theta + ANGLE_PI));

		or = (int) (l1*inG + l2*inB + me*inR + r1*inG + r2*inB);
		og = (int) (l1*inB + l2*inR + me*inG + r1*inB + r2*inR);
		ob = (int) (l1*inR + l2*inG + me*inB + r1*inR + r2*inG);

		oldMax = max(inR, max(inG, inB));
		maxColor = max(or, max(og, ob));

		*pOutR = (BYTE) (or*oldMax/maxColor);
		*pOutG = (BYTE) (og*oldMax/maxColor);
		*pOutB = (BYTE) (ob*oldMax/maxColor);
	}
	else
	{
		*pOutR = inR;
		*pOutG = inG;
		*pOutB = inB;

		return;
	}
}
*/
///*
void GetOffsetHue(signed char Offset,
						BYTE inR, BYTE inG, BYTE inB,
						BYTE *pOutR, BYTE *pOutG, BYTE *pOutB)
{
	int l, m, r,
		 or, og, ob,
		 oldMax, maxColor;

	if (Offset)
	{
		if (Offset < 0)
			m = max(0, ((int)Offset) + 85)*255/85;
		else
			m = max(0, 85 - ((int)Offset))*255/85;

		if (Offset < -85)
			l = (170 + ((int)Offset))*255/85;
		else
			l = max(0, -((int)Offset))*255/85;

		if (Offset > 85)
			r = (170 - ((int)Offset))*255/85;
		else
			r = max(0, ((int)Offset))*255/85;

		or = l*inB + m*inR + r*inG;
		og = l*inR + m*inG + r*inB;
		ob = l*inG + m*inB + r*inR;

		oldMax = max(inR, max(inG, inB));
		maxColor = max(or, max(og, ob));

		if (maxColor)
		{
			*pOutR = (BYTE) (or*oldMax/maxColor);
			*pOutG = (BYTE) (og*oldMax/maxColor);
			*pOutB = (BYTE) (ob*oldMax/maxColor);
		}
		else
		{
			*pOutR = inR;
			*pOutG = inG;
			*pOutB = inB;
		}
	}
	else
	{
		*pOutR = inR;
		*pOutG = inG;
		*pOutB = inB;

		return;
	}
}
//*/
/*
void GetOffsetHue(signed char Offset,
						BYTE inR, BYTE inG, BYTE inB,
						BYTE *pOutR, BYTE *pOutG, BYTE *pOutB)
{
	static BYTE gg[] =
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x03,0x09,0x0F,0x16,0x1C,0x22,0x28,
		0x2F,0x35,0x3B,0x41,0x47,0x4D,0x53,0x59,
		0x5F,0x65,0x6A,0x70,0x76,0x7B,0x81,0x86,
		0x8C,0x91,0x96,0x9B,0xA0,0xA5,0xAA,0xAE,
		0xB3,0xB7,0xBB,0xC0,0xC4,0xC8,0xCC,0xCF,
		0xD3,0xD6,0xDA,0xDD,0xE0,0xE3,0xE6,0xE8,
		0xEB,0xED,0xEF,0xF1,0xF3,0xF5,0xF7,0xF8,
		0xFA,0xFB,0xFC,0xFD,0xFD,0xFE,0xFE,0xFE,
		0xFF,0xFE,0xFE,0xFE,0xFD,0xFD,0xFC,0xFB,
		0xFA,0xF8,0xF7,0xF5,0xF3,0xF1,0xEF,0xED,
		0xEB,0xE8,0xE6,0xE3,0xE0,0xDD,0xDA,0xD6,
		0xD3,0xCF,0xCC,0xC8,0xC4,0xC0,0xBB,0xB7,
		0xB3,0xAE,0xAA,0xA5,0xA0,0x9B,0x96,0x91,
		0x8C,0x86,0x81,0x7B,0x76,0x70,0x6A,0x65,
		0x5F,0x59,0x53,0x4D,0x47,0x41,0x3B,0x35,
		0x2F,0x28,0x22,0x1C,0x16,0x0F,0x09,0x03,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	int by,
		 va, vb, vc,
		 or, og, ob,
		 oldMax, maxColor;

	by = ((int)Offset) + 128;

	va = gg[(by + 85) & 0xff];
	vb = gg[by];
	vc = gg[(by + 170) & 0xff];

	or = va*inB + vb*inR + vc*inG;
	og = va*inR + vb*inG + vc*inB;
	ob = va*inG + vb*inB + vc*inR;

	oldMax = max(inR, max(inG, inB));
	maxColor = max(or, max(og, ob));

	*pOutR = (BYTE) (or*oldMax/maxColor);
	*pOutG = (BYTE) (og*oldMax/maxColor);
	*pOutB = (BYTE) (ob*oldMax/maxColor);
}
*/
void GetReducedSaturation(BYTE Reduce,
								  BYTE inR, BYTE inG, BYTE inB,
								  BYTE *pOutR, BYTE *pOutG, BYTE *pOutB)
{
	int	oldMax, onemr;

	if (Reduce)
	{
		onemr = 255 - Reduce;

		oldMax = max(inR, max(inG, inB));

		*pOutR = (inR*onemr + oldMax*Reduce)/255;
		*pOutG = (inG*onemr + oldMax*Reduce)/255;
		*pOutB = (inB*onemr + oldMax*Reduce)/255;
	}
	else
	{
		*pOutR = inR;
		*pOutG = inG;
		*pOutB = inB;

		return;
	}
}

void GetReducedBrightness(BYTE Reduce,
								  BYTE inR, BYTE inG, BYTE inB,
								  BYTE *pOutR, BYTE *pOutG, BYTE *pOutB)
{
	int	onemr;

	if (Reduce)
	{
		onemr = 255 - Reduce;

		*pOutR = inR*onemr/255;
		*pOutG = inG*onemr/255;
		*pOutB = inB*onemr/255;
	}
	else
	{
		*pOutR = inR;
		*pOutG = inG;
		*pOutB = inB;

		return;
	}
}

