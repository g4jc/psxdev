/* $Id: write.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

#include <stdlib.h>
#include <string.h>
#include <tim.h>

#include <stdio.h>
tim_header *TIM_Create (
	unsigned short mode,	// TIM_xBIT
	unsigned short width,	// 0..1023
	unsigned short height,	// 0..511
	unsigned short numcluts	// 0..511
	)
{
	tim_header *tim;
	tim_data data = {0}, *pdata = NULL;
	tim_clut clut = {0}, *pclut = NULL;
	int size = 0, hasclut = 0;
	extern int tim_error_num;
	
	size += sizeof(tim_header);

	switch (mode)
	{
		case TIM_4BIT:
			hasclut     = 1;
			clut.width  = 16;
			clut.height = numcluts;
			clut.count  = (numcluts * 16 * 2) + sizeof(tim_clut);
			size       += clut.count;
			data.count  = (width * height / 2) + sizeof(tim_data);
			data.width  = width / 4;
			data.height = height;
			size       += data.count;
			break;

		case TIM_8BIT:
			hasclut     = 1;
			clut.width  = 256;
			clut.height = numcluts;
			clut.count  = (numcluts * 256 * 2) + sizeof(tim_clut);
			size       += clut.count;
			data.count  = (width * height) + sizeof(tim_data);
			data.width  = width / 2;
			data.height = height;
			size       += data.count;
			break;

		case TIM_15BIT:
			data.count  = (width * height * 2) + sizeof(tim_data);
			data.width  = width;
			data.height = height;
			size       += data.count;
			break;

		case TIM_24BIT:
			/* untested ! */
			data.count  = (width * height * 3) + sizeof(tim_data);
			size       += data.count;
			data.width  = (width/2)*3;
			data.height = height;
			break;

		default:			/* unknown tim format */
			tim_error_num = TIMERROR_UNSUPPORTED_MODE;
			return NULL;
	}

	tim = malloc(size);

	if (tim)
	{
		memset(tim,0,size);
	
		tim->id       = TIM_ID;
		tim->version  = TIM_VERSION;
		tim->reserved = TIM_RESERVED;
		tim->flags    = mode;

		if (hasclut)
		{
			tim->flags |= TIMF_CLUT;
			pclut = (tim_clut *) (tim + 1);
			memcpy(pclut,&clut,sizeof(tim_clut));
			pdata = (tim_data *) (((unsigned long)(pclut)) + pclut->count);
		}
		else
		{
			pdata = (tim_data *) (tim + 1);
		}

		memcpy(pdata,&data,sizeof(tim_data));
	}

	return tim;
}
