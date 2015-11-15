/* $Id: file.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

#include <tim.h>

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

tim_header *TIM_Open (char *filename)
{
	struct stat st;
	int fd;
	tim_header *tim = 0;

	fd = open (filename,O_RDONLY);
	if (fd != -1)
	{
		fstat(fd, &st);
		tim = (tim_header *) mmap (NULL,st.st_size,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0);
		close (fd);
	}
	else return NULL;

	if (TIM_Check(tim))
	{
		return tim;
	}
	else
	{
		munmap ((void*)tim,st.st_size);
		return NULL;
	}
}

void TIM_Close (tim_header *tim)
{
	if (tim) munmap ((void*)tim,TIM_SizeOf(tim));
}

char *TIM_Convert_TIM2RGBA (tim_header *tim, int palette)
{
	char *rgba, *run;
	tim_clut *clut;
	tim_data *data;
	int i,j;
	unsigned short p, *pixel, *color;
	unsigned char *dat;

	rgba = (char *) malloc (TIM_GetWidth(tim,0)*TIM_GetHeight(tim,0)*4);
	
	if (rgba==NULL) return rgba;

	clut = TIM_GetCLUT(tim);
	data = TIM_GetData(tim);
	run = rgba;

	switch (TIM_GetMode(tim))
	{
		case TIM_4BIT:
			dat = (unsigned char *) TIM_GetFirstPixel(tim);
			color = TIM_GetFirstColorFromPalette(tim,palette);
			for (i=0;i<TIM_NumPixel(tim);i+=2)
			{
				j = dat[(i>>1)];
				p = color[j&15];
				*run = TIM_BlueOfPixel8(p); run++;
				*run = TIM_GreenOfPixel8(p); run++;
				*run = TIM_RedOfPixel8(p); run++;
				*run = TIM_AlphaOfPixel(p); run++;
				p = color[(j>>4)&15];
				*run = TIM_BlueOfPixel8(p); run++;
				*run = TIM_GreenOfPixel8(p); run++;
				*run = TIM_RedOfPixel8(p); run++;
				*run = TIM_AlphaOfPixel(p); run++;
			}
			break;

		case TIM_8BIT:
			dat = (unsigned char *) TIM_GetFirstPixel(tim);
			color = TIM_GetFirstColorFromPalette(tim,palette);
			for (i=0;i<TIM_NumPixel(tim);i++)
			{
				j = dat[i];
				p = color[j];
				*run = TIM_BlueOfPixel8(p); run++;
				*run = TIM_GreenOfPixel8(p); run++;
				*run = TIM_RedOfPixel8(p); run++;
				*run = TIM_AlphaOfPixel(p); run++;
			}
			break;

		case TIM_15BIT:
			pixel = TIM_GetFirstPixel(tim);
			for (i=0;i<TIM_NumPixel(tim);i++)
			{
				p = pixel[i];
				*run = TIM_BlueOfPixel8(p); run++;
				*run = TIM_GreenOfPixel8(p); run++;
				*run = TIM_RedOfPixel8(p); run++;
				*run = TIM_AlphaOfPixel(p); run++;
			}
			break;

		case TIM_24BIT:
			break;
	}

	return rgba;
}

char *TIM_Convert_TIM2RGB (tim_header *tim, int palette)
{
	char *rgb, *run;
	tim_clut *clut;
	tim_data *data;
	int i,j;
	unsigned short p, *pixel, *color;
	unsigned char *dat;

	rgb = (char *)malloc (TIM_GetWidth(tim,0)*TIM_GetHeight(tim,0)*3);
	
	if (rgb==NULL) return rgb;

	clut = TIM_GetCLUT(tim);
	data = TIM_GetData(tim);
	run = rgb;

	switch (TIM_GetMode(tim))
	{
		case TIM_4BIT:
			dat = (unsigned char *) TIM_GetFirstPixel(tim);
			color = TIM_GetFirstColorFromPalette(tim,palette);
			for (i=0;i<TIM_NumPixel(tim);i+=2)
			{
				j = dat[(i>>1)];
				p = color[j&15];
				*run = TIM_BlueOfPixel8(p); run++;
				*run = TIM_GreenOfPixel8(p); run++;
				*run = TIM_RedOfPixel8(p); run++;
				p = color[(j>>4)&15];
				*run = TIM_BlueOfPixel8(p); run++;
				*run = TIM_GreenOfPixel8(p); run++;
				*run = TIM_RedOfPixel8(p); run++;
			}
			break;

		case TIM_8BIT:
			dat = (unsigned char *) TIM_GetFirstPixel(tim);
			color = TIM_GetFirstColorFromPalette(tim,palette);
			for (i=0;i<TIM_NumPixel(tim);i++)
			{
				j = dat[i];
				p = color[j];
				*run = TIM_BlueOfPixel8(p); run++;
				*run = TIM_GreenOfPixel8(p); run++;
				*run = TIM_RedOfPixel8(p); run++;
			}
			break;

		case TIM_15BIT:
			pixel = TIM_GetFirstPixel(tim);
			for (i=0;i<TIM_NumPixel(tim);i++)
			{
				p = pixel[i];
				*run = TIM_BlueOfPixel8(p); run++;
				*run = TIM_GreenOfPixel8(p); run++;
				*run = TIM_RedOfPixel8(p); run++;
			}
			break;

		case TIM_24BIT:
			break;
	}

	return rgb;
}
