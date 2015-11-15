/* $Id: init.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

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

#include "config.h"
#include "common.h"
#include <tim.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

void tim_init (void)
{
}

char *tim_get_alpha (tim_t *tim)
{
	return NULL;
}


char *tim_get_rgb (tim_t *tim, int shift)
{
	tim_chunk_t *pixel, *clut;
	unsigned char *mem = NULL;
	int i, j, k=0;

	clut = tim_clut_addr (tim);
	pixel = tim_pixel_addr (tim);

	mem = malloc (tim_width(tim,0) * tim_height(tim,0) * 3);

	switch (tim->flags.mode)
	{
		case TIM_4BIT:
		{
			tim_pixel_15bit_t *p;
			tim_pixel_15bit_t *palette = (tim_pixel_t*) tim_clut (tim,0,0);
			u_char *q = (u_char*) tim_pixel (tim,0,0);

			for (i=0;i<tim_pixel_count(tim);i+=2)
			{
				j = q[i>>1];
			
				p = &palette[j & 0xf];
				mem[k++] = p->blue  << shift;
				mem[k++] = p->green << shift;
				mem[k++] = p->red   << shift;

				p = &palette[(j>>4) & 0xf];
				mem[k++] = p->blue  << shift;
				mem[k++] = p->green << shift;
				mem[k++] = p->red   << shift;
			}
		}
		break;

		case TIM_8BIT:
		{
			tim_pixel_15bit_t *p;
			tim_pixel_15bit_t *palette = (tim_pixel_t*) tim_clut (tim,0,0);
			u_char *q = (u_char*) tim_pixel (tim,0,0);
			
			for (i=0;i<tim_pixel_count(tim);i++)
			{
				p = &palette[q[i]];
			
				mem[k++] = p->blue  << shift;
				mem[k++] = p->green << shift;
				mem[k++] = p->red   << shift;
			}
		}
		break;

		case TIM_15BIT:
		{
			tim_pixel_15bit_t *p = tim_pixel (tim,0,0);
			
			for (i=0;i<tim_pixel_count(tim);i++,p++,k+=3)
			{
				mem[k+0] = p->blue << shift;
				mem[k+1] = p->green << shift;
				mem[k+2] = p->red << shift;
			}
		}
		break;

		case TIM_24BIT:
		{
			char *p = tim_pixel (tim,0,0);
		
			for (i=0;i<tim_pixel_count(tim);i++)
			{
				mem[k+0] = p[k+2];
				mem[k+1] = p[k+1];
				mem[k+2] = p[k+0];
				k+=3;
			}
		}
		break;
	}
	
	return mem;
}


int tim_width (tim_t *tim,int ofclut)
{
	if (ofclut==0)
	{
		tim_chunk_t *pixel = tim_pixel_addr(tim);
	
		switch (tim->flags.mode)
		{
			case TIM_4BIT:
				return pixel->width * 4;
			case TIM_8BIT:
				return pixel->width * 2;
			case TIM_15BIT:
				return pixel->width;
			case TIM_24BIT:
				return (pixel->width*2) / 3;
			default:
				return NULL;
		}
	}
	return (tim->flags.clut) ? (tim_clut_addr(tim)->width) : 0;
}


int tim_height (tim_t *tim,int ofclut)
{
	if (ofclut)
	{
		return (tim->flags.clut) ? (tim_clut_addr(tim)->height) : 0;
	}

	return (tim_pixel_addr(tim)->height);
}

tim_chunk_t *tim_clut_addr (tim_t *tim)
{
	if (tim->flags.clut)
	{
		return (tim_chunk_t *) (tim + 1);
	}
	else
	{
		return (tim_chunk_t *) NULL;
	}
}


tim_chunk_t *tim_pixel_addr (tim_t *tim)
{
	if (tim->flags.clut)
	{
		volatile tim_chunk_t *clut = (tim_chunk_t *) (tim + 1);
		return (tim_chunk_t *) (((unsigned long)(clut)) + clut->size);
	}
	else
	{
	    return (tim_chunk_t *) (tim + 1);
	}
}


int tim_sizeof (tim_t *tim)
{
	int size = sizeof(tim_t);
	if (tim->flags.clut) size += tim_clut_addr(tim)->size;
	size += tim_pixel_addr(tim)->size;
	return size;
}


tim_t *tim_new (tim_mode_t mode, u_short width, u_short height, u_short ncluts)
{
	tim_t *tim = NULL;
	tim_chunk_t pixel, clut;
	int size = 0, hasclut = 0;

	pixel.size = clut.size = sizeof(tim_chunk_t);
	pixel.x = clut.x = pixel.y = clut.y = 0;

	switch (mode)
	{
		case TIM_4BIT:
			hasclut      = 1;
			clut.width   = 16;
			clut.height  = ncluts;
			clut.size   += (ncluts * 16 * 2);
			size        += clut.size;
			pixel.size  += (width * height / 2);
			pixel.width  = width / 4;
			pixel.height = height;
			size        += pixel.size;
			break;

		case TIM_8BIT:
			hasclut      = 1;
			clut.width   = 256;
			clut.height  = ncluts;
			clut.size   += (ncluts * 256 * 2);
			size        += clut.size;
			pixel.size  += (width * height);
			pixel.width  = width / 2;
			pixel.height = height;
			size        += pixel.size;
			break;

		case TIM_15BIT:
			pixel.size  += (width * height * 2);
			size        += pixel.size;
			pixel.width  = width;
			pixel.height = height;
			break;

		case TIM_24BIT:
			pixel.size  += (width * height * 3);
			size        += pixel.size;
			pixel.width  = (width/2)*3;
			pixel.height = height;
			break;
		
		default:
			return NULL;
	}
	
	tim = malloc (size);
	if (tim)
	{
		tim_chunk_t *ptr;
	
		tim->magic = TIM_MAGIC;
		tim->flags.mode = mode;
		tim->flags.clut = hasclut;

		if (hasclut)
		{
			ptr = (tim_chunk_t *) (tim + 1);
			*ptr = clut;
			ptr = (tim_chunk_t *) (((unsigned long)(ptr)) + ptr->size);
		}
		else
		{
			ptr = (tim_chunk_t *) (tim + 1);
		}

		*ptr = pixel;
	}

	return tim;
}

void tim_free (tim_t *tim)
{
	if (tim != NULL)
	{
		if (tim->magic != TIM_MAGIC)
		{
			fprintf(stderr,_("libtim: freeing tim image with bad magic!\n"));
		}
	
		free (tim);
	}
}

tim_pixel_t *tim_pixel (tim_t *tim, int x, int y)
{
	tim_pixel_t *p;
	tim_chunk_t *c = tim_pixel_addr(tim);

	p = (tim_pixel_t *) (c + 1);
	
	return p + ((y * c->width) + x);
}

tim_pixel_t *tim_clut (tim_t *tim, int x, int y)
{
	tim_pixel_t *p;
	tim_chunk_t *c = tim_clut_addr(tim);

	if (tim->flags.clut)
	{
		p = (tim_pixel_t *) (c + 1);
	}
	else return NULL;
	
	return p + ((y * c->width) + x);
}
