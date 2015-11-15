/* $Id: read.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

unsigned short TIM_GetWidth (tim_header *tim,int ofclut)
{
	if (ofclut)
	{
		return (TIM_GetCLUT(tim)->width);
	}
	else
	{
		tim_data *data = TIM_GetData(tim);
	
		switch (TIM_GetMode(tim))
		{
			case TIM_4BIT:
				return data->width * 4;
			case TIM_8BIT:
				return data->width * 2;
			case TIM_15BIT:
				return data->width;
			case TIM_24BIT:
				return (data->width*2) / 3;
			default:
				return NULL;
		}
	}
}

unsigned short TIM_GetHeight (tim_header *tim,int ofclut)
{
	if (ofclut)
	{
		return (TIM_GetCLUT(tim)->height);
	}
	else
	{
		return (TIM_GetData(tim)->height);
	}
}

tim_clut *TIM_GetCLUT (tim_header *tim)
{
	if (TIM_HasCLUT(tim))
	{
		return (tim_clut *) (tim + 1);
	}
	else
	{
		return (tim_clut *) NULL;
	}
}

tim_data *TIM_GetData (tim_header *tim)
{
	if (TIM_HasCLUT(tim))
	{
		volatile tim_clut *clut = (tim_clut *) (tim + 1);

		return (tim_data *) (((unsigned long)(clut)) + clut->count);
	}
	else
	{
	    return (tim_data *) (tim + 1);
	}
}

unsigned short *TIM_GetFirstPixel (tim_header *tim)
{
	return (unsigned short *) (TIM_GetData(tim) + 1);
}

unsigned short *TIM_GetFirstColor (tim_header *tim)
{
	if (TIM_HasCLUT(tim))
	{
		return (unsigned short *) (TIM_GetCLUT(tim) + 1);
	}
	else
	{
		return NULL;
	}
}

unsigned short *TIM_GetFirstColorFromPalette (tim_header *tim, int palette)
{
	if (TIM_HasCLUT(tim))
	{
		volatile tim_clut *clut = (tim_clut *) (tim + 1);

		switch (TIM_GetMode(tim))
		{
			case TIM_4BIT:
				return (unsigned short *) (((int)(clut+1)) + ( 32*palette));
				break;
			case TIM_8BIT:
				return (unsigned short *) (((int)(clut+1)) + (512*palette));
				break;
			default:
				return NULL;
		}
	}
	else
	{
	    return NULL;
	}
}

unsigned long TIM_SizeOf (tim_header *tim)
{
	int size = sizeof(tim_header);

	if (TIM_HasCLUT(tim)) size += TIM_GetCLUT(tim)->count;

	size += TIM_GetData(tim)->count;

	return size;
}
