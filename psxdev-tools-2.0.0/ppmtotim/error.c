/* $Id: error.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

static char *error_txt [] = {
	0,
	"TIM: invalid version",
	"TIM: invalid id",
	"TIM: unsupported mode",
	"TIM: unknown error",
};

char *TIM_GetErrorString (void)
{
	extern int tim_error_num;

	return error_txt[tim_error_num];
}
