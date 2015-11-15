/* $Id: tim.h,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

/*
	ppmtotim - complex TIM image format creation utility

	Copyright (C) 1997, 1998, 1999, 2000 by
	  Daniel Balster <dbalster@psxdev.de>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __LIB_TIM_H__
#define __LIB_TIM_H__

#if !(defined FALSE && defined TRUE)
typedef enum { FALSE=0, TRUE=1 } bool;
#else
typedef short bool;
#endif

#ifndef NULL
#define NULL (0)
#endif

#define ADR(x,y,op) (((long)(x)) op ((long)(y)))

#ifdef BIGENDIAN
	#define get32(a) ( ((a >> 24) & 0xff) | ((a >> 8) & 0xff00) | ((a << 8) & 0x00ff0000) | ((a << 24) & 0xff000000) ) 
	#define get16(a) ( ((a >> 8) & 0xff) | ((a << 8) & 0xff00) )
#else
	#define get32(a) (a) 
	#define get16(a) (a)
#endif

#if defined DEBUG
  #define dprintf(fmt,args...) printf(fmt, ## args)
  #define assertf(X,args...) ({ if(!(X)) printf("\n\nASSERTION FAILED: %s in %s:%ld [%s]\n\n",__FUNCTION__,__FILE__,__LINE__, ## args);})
#else
  #define dprintf(fmt,args...)
  #define assertf(X,args...)
#endif

#define BIT_TEST(x,b)    (((x) & (1l<<(b))) == (1l<<(b)))
#define BIT_SET(x,b)     (x) |=  (1L << (b))
#define BIT_CLEAR(x,b)   (x) &=~ (1L << (b))

#define FLAG_TEST(x,f)    (((x) & (f)) == (f))
#define FLAG_SET(x,f)     (x) |=  (f)
#define FLAG_CLEAR(x,f)   (x) &=~ (f)

/** errors **/

enum {
	TIM_OK=0,
	TIMERROR_INVALID_VERSION=1,
	TIMERROR_INVALID_ID,
	TIMERROR_UNSUPPORTED_MODE,
	TIMERROR_UNKNOWN
};

#define TIM_VERSION		0x0000
#define TIM_ID			0x10
#define TIM_RESERVED	0x0000
#define TIM_MODEMASK	0x07

#define TIMF_CLUT   (1L << 3)

#define TIM_4BIT		0
#define TIM_8BIT		1
#define TIM_15BIT		2
#define TIM_24BIT		3
#define TIM_MULTI		4	/* found somewhere, unsupported ! */
#define TIM_LASTMODE	3

#define TIM_CLUT_STP	(0x8000)	/* semi transparency bit */
#define TIM_CLUT_RED	(0x7c00)	/* 5 bit red component */
#define TIM_CLUT_GRN	(0x03e0)	/* 5 bit green component */
#define TIM_CLUT_BLU	(0x001f)	/* 5 bit blue component */

#define TIM_CLUT_REDSHIFT	10
#define TIM_CLUT_GRNSHIFT	5
#define TIM_CLUT_BLUSHIFT	0

#define TIM_CLUT_REDMASK	31
#define TIM_CLUT_GRNMASK	31
#define TIM_CLUT_BLUMASK	31

typedef struct
{
	unsigned char  id;
	unsigned char  version;
	unsigned short reserved;
	unsigned long  flags;
} tim_header;

typedef struct
{
	unsigned long  count;
	unsigned short dx,dy;
	unsigned short width,height;
} tim_data;

typedef struct
{
	unsigned long  count;
	unsigned short dx,dy;
	unsigned short width,height;
} tim_clut;

/** macros **/

#define TIM_GetPixelWidth(t)	((((t)->info->flags&TIMF_CLUT)==TIMF_CLUT)?(((((t)->info->flags&TIM_4BIT)==TIM_4BIT)?(4):(2))):(1))
#define TIM_GetDepth(t)			((((t)->flags&TIM_MODEMASK)==TIM_24BIT)?(255):(31))
#define TIM_HasCLUT(t)			((t)->flags&TIMF_CLUT)
#define TIM_GetMode(t)			((t)->flags&TIM_MODEMASK)
#define TIM_NumPixel(t)			(TIM_GetWidth(t,0)*TIM_GetHeight(t,0))
#define TIM_GetNumColors(t)		((((t)->flags&TIM_MODEMASK)==TIM_4BIT)?(16):(256))

/* pixel converting */

#define TIM_RedOfPixel(pixel)	(((pixel)&TIM_CLUT_RED)>>TIM_CLUT_REDSHIFT)
#define TIM_GreenOfPixel(pixel)	(((pixel)&TIM_CLUT_GRN)>>TIM_CLUT_GRNSHIFT)
#define TIM_BlueOfPixel(pixel)	(((pixel)&TIM_CLUT_BLU)>>TIM_CLUT_BLUSHIFT)
#define TIM_AlphaOfPixel(pixel)	((((pixel)&TIM_CLUT_STP)==TIM_CLUT_STP)?255:0)

#define TIM_RedOfPixel8(pixel)	(((((pixel)&TIM_CLUT_RED)>>TIM_CLUT_REDSHIFT)<<3)|7)
#define TIM_GreenOfPixel8(pixel)	(((((pixel)&TIM_CLUT_GRN)>>TIM_CLUT_GRNSHIFT)<<3)|7)
#define TIM_BlueOfPixel8(pixel)	(((((pixel)&TIM_CLUT_BLU)>>TIM_CLUT_BLUSHIFT)<<3)|7)

/** prototypes **/

          char *TIM_GetModeString (int type);
          bool  TIM_Check (tim_header *tim);
          char *TIM_GetErrorString (void);
unsigned short *TIM_GetFirstColorFromPalette (tim_header *tim, int palette);
unsigned short *TIM_GetFirstColor (tim_header *tim);
unsigned short *TIM_GetFirstPixel (tim_header *tim);
      tim_data *TIM_GetData (tim_header *tim);
      tim_clut *TIM_GetCLUT (tim_header *tim);
unsigned short  TIM_GetHeight (tim_header *tim,int ofclut);
unsigned short  TIM_GetWidth (tim_header *tim,int ofclut);
    tim_header *TIM_Create (unsigned short mode,unsigned short width,unsigned short height,unsigned short numcluts);
unsigned long   TIM_SizeOf (tim_header *tim);
    tim_header *TIM_Open (char *filename);
         void   TIM_Close (tim_header *tim);
         char  *TIM_Convert_TIM2RGBA (tim_header *tim, int palette);
         char  *TIM_Convert_TIM2RGB (tim_header *tim, int palette);

#endif  __LIB_TIM_H__
