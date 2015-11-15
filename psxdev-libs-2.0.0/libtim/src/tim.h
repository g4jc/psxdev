/* $Id: tim.h,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
	libtim - library for the TIM image format

	Copyright (C) 1997, 1998, 1999, 2000 by these people, who contributed to this project

	  Daniel Balster <dbalster@psxdev.de>
	  Sergio Moreira <sergio@x-plorer.co.uk>
	  Andrew Kieschnick <andrewk@cerc.utexas.edu>
	  Kazuki Sakamoto <bsd-ps@geocities.co.jp>

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

#ifndef __LIB_TIM_H
#define __LIB_TIM_H

#define _GNU_SOURCE

#include <sys/types.h>
#include <stdarg.h>

#define TIM_MAGIC (0x00000010)

#define tim_depth(t)\
	((((t)->flags.mode)==TIM_24BIT)?(255):(31))

#define tim_pixel_count(t)\
	(tim_width((t),0)*tim_height((t),0))

#define tim_clut_count(t)\
	((((t)->flags.mode)==TIM_4BIT)?(16):(256))

#define tim_pixel_width(t)\
	(((t)->flags.clut)?((((t)->flags.mode==TIM_4BIT)?(4):(2)):(1)))

typedef enum {
	TIM_4BIT=0,
	TIM_8BIT=1,
	TIM_15BIT=2,
	TIM_24BIT=3,
} tim_mode_t;

typedef struct {
	u_long size;
	u_short x,y;
	u_short width,height;
} tim_chunk_t;

typedef struct {
	unsigned short blue : 5;
	unsigned short green : 5;
	unsigned short red : 5;
	unsigned short stp : 1;
} tim_pixel_15bit_t;

typedef struct {
	unsigned short byte0 : 8;
	unsigned short byte1 : 8;
} tim_pixel_8bit_t;

typedef struct {
	unsigned short nibble0 : 4;
	unsigned short nibble1 : 4;
	unsigned short nibble2 : 4;
	unsigned short nibble3 : 4;
} tim_pixel_4bit_t;

typedef struct {
	union type {
		tim_pixel_4bit_t p4bit;
		tim_pixel_8bit_t p8bit;
		tim_pixel_15bit_t p15bit;
	};
} tim_pixel_t;

typedef struct {
	u_long magic;
	struct {
		unsigned long mode : 3;
		unsigned long clut : 1;
	} flags;
} tim_t;

/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

void tim_init (void);

tim_t *tim_new (tim_mode_t mode, u_short width, u_short height, u_short ncluts);
void tim_free (tim_t *tim);

tim_chunk_t *tim_clut_addr (tim_t *tim);
tim_chunk_t *tim_pixel_addr (tim_t *tim);
int tim_sizeof (tim_t *tim);
int tim_width (tim_t *tim,int ofclut);
int tim_height (tim_t *tim,int ofclut);

tim_pixel_t *tim_pixel (tim_t *tim, int x, int y);
tim_pixel_t *tim_clut (tim_t *tim, int x, int y);

char *tim_get_alpha (tim_t *tim);
char *tim_get_rgb (tim_t *tim, int shift);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_PSXDEV_H */
