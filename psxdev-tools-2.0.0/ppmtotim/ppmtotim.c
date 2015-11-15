/* $Id: ppmtotim.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <getopt.h>

#include <tim.h>

extern int yylex();
extern void yyrestart(FILE *);

/* GNU long-options */

static char *option_chars = "hV0123c:x:y:X:Y:m:ntp:";

static struct option long_options[] =
{
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V'},
	{ "4bit", no_argument, NULL, '0' },
	{ "8bit", no_argument, NULL, '1' },
	{ "15bit", no_argument, NULL, '2' },
	{ "24bit", no_argument, NULL, '3' },
	{ "numcluts", required_argument, NULL, 'c' },
	{ "clutx", required_argument, NULL, 'x' },
	{ "cluty", required_argument, NULL, 'y' },
	{ "posx", required_argument, NULL, 'X' },
	{ "posy", required_argument, NULL, 'Y' },
	{ "mask", required_argument, NULL, 'm' },
	{ "noheader", no_argument, NULL, 'n' },
	{ "transparent", no_argument, NULL, 't' },
	{ "palette", required_argument, NULL, 'p' },
};

static char *help_text =
"Usage: %s [-h|--help] [-V|--version] [-0|--4bit] [-1|--8bit]"
"\t[-2|--15bit] [-3|--24bit] [-m|--mask=.ppm,.pgm] [-c|--numcluts=n]"
"\t[-x|--clutx=x] [-y|--cluty=y] [-X|--posx=x] [-Y|--posy=y]"
"\t[-n|--noheader] [-t|--transparent] [-p|--palette] [filename]\n";

static char *version_text =
"%s version 2.8.0\n";

static char *copyright_text =
"\n*** %s - NetPBM TIM file writer\n"
"\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n";

	extern int yylex (void);
	extern FILE* yyin;

struct {
	unsigned char r,g,b,stp;
} CLUT[257];

signed long histogramm [257];
long histogramm_usage;


/*
	read a 1-bit P1/P4 pbm file , allocate memory and read in the bitmask.
*/

char *bitmask = 0;
int bitmask_w = 0;
int bitmask_h = 0;
int bitmask_d = 0;

void ppm_to_mask (int aorb)
{
	int i, j, size;
	char bits;

	bitmask_w = yylex();
	bitmask_h = yylex();

	size = bitmask_w * bitmask_h;

	bitmask = malloc(size);

	if (bitmask)
	{
		if (aorb)
		{
			for (i=0; i<(size/8); i++)
			{
				bits = getchar();
				for (j=7;j>=0;j--)
					bitmask[(i*8)+j] = ((bits & (1<<j))==(1<<j)) ? 1 : 0;
			}
		}
		else
		{
			for (i=0; i<size; i++)
			{
				bits = yylex();
				bitmask[i] = bits;
			}
		}
	}
	else
	{
		fprintf(stderr,"Couldn't allocate memory for bitmask!\n");
	}
}

void read_ppm_palette (char *palette, int stp)
{
	int width, height, depth;
	char ppmid[2];
	int i;

	freopen(palette,"r",stdin);

	ppmid[0] = getchar();
	ppmid[1] = getchar();
		
	if (!((ppmid[0]=='P') && (ppmid[1]=='3' || ppmid[1]=='6')))
	{		
		fprintf(stderr,"not a P3 or P6 file\n");
		exit(1);
	}

	yyrestart(stdin);

	width = yylex();
	height = yylex();
	depth = yylex();

	if (ppmid[1] - '1')
	{	
		for (i=0;i<width*height;i++)
		{
			CLUT[i].b = getchar();
			CLUT[i].g = getchar();
			CLUT[i].r = getchar();

			/* in index color mode the mask image is used on the palette !!! */
			if (bitmask)
				CLUT[i].stp = bitmask[i];
			else
				CLUT[i].stp = stp;
		}
	}
	else
	{
		for (i=0;i<width*height;i++)
		{
			CLUT[i].b = yylex();
			CLUT[i].g = yylex();
			CLUT[i].r = yylex();

			/* in index color mode the mask image is used on the palette !!! */
			if (bitmask)
				CLUT[i].stp = bitmask[i];
			else
				CLUT[i].stp = stp;
		}
	}
}

void ppm_to_tim (int aorb, int indexed, int mode,int cluts,int clutx,int cluty,int datax,int datay,int stp,char *mask,char *palette)
{
	int width, height, depth;
	unsigned char *r,*g,*b;
	int i,j,k,sh,n;
	unsigned short *pixel;
	char ppmid[2];
	unsigned char *index;

	tim_header *tim;
	tim_data   *data;
	tim_clut   *clut;

	width = yylex();
	height = yylex();
	depth = yylex();

	if (ferror(stdin))
	{
		perror("ppm header");
		exit(1);
	}

	tim = TIM_Create(mode,width,height,cluts);

	if (tim)
	{
		index = malloc(width*height);
		if (!index)
		{
			fprintf(stderr,"unable to malloc ?\n");
			exit(1);
		}

		if (indexed)
		{
			if (aorb)
			{	
				for (i=0;i<width*height;i++) index[i] = getchar();
			}
			else
			{
				for (i=0;i<width*height;i++) index[i] = yylex();
			}
		}
		else
		{
			r = malloc(width*height);
			g = malloc(width*height);
			b = malloc(width*height);

			if (!(r && g && b))
			{
				fprintf(stderr,"unable to malloc ?\n");
				exit(1);
			}

			if (aorb)
			{	
				for (i=0;i<width*height;i++)
				{
					b[i] = getchar();
					g[i] = getchar();
					r[i] = getchar();
				}
			}
			else
			{
				for (i=0;i<width*height;i++)
				{
					b[i] = yylex();
					g[i] = yylex();
					r[i] = yylex();
				}
			}
		}

	/* read a bitplane to use as the semitransparent mask plane */

	if (mask)
	{
		freopen(mask,"r",stdin);
		
		ppmid[0] = getchar();
		ppmid[1] = getchar();
		
		if (!((ppmid[0]=='P') && (ppmid[1]=='1' || ppmid[1]=='4')))
		{		
			fprintf(stderr,"not a P1 or P4 file\n");
			exit(1);
		}
		yyrestart(stdin);
		ppm_to_mask (ppmid[1] - '1');
	}

		data = TIM_GetData(tim);

		data->dx = datax;
		data->dy = datay;

		if (TIM_HasCLUT(tim))
		{
			clut = TIM_GetCLUT(tim);
			
			clut->dx = clutx;
			clut->dy = cluty;
			
			/* make histogram */

			k = TIM_GetNumColors(tim);

			/*
				picture is organized as a 16*n picture
			*/

			if (TIM_GetMode(tim) == TIM_4BIT)
			{
				k *= cluts;
			}

			if (palette)
			{
				fprintf(stderr,"reading palette from file %s..\n",palette);
				read_ppm_palette(palette,stp);
			}
			else
			{
				fprintf(stderr,"creating palette from histogram..\n");

				// clear histogram table
				for (i=0;i<k;i++) histogramm[i] = -1;

				for (i=0;i<width*height;i++)
				{
					unsigned long mark,found;
	
					mark = (unsigned long) (r[i] << 16) | (g[i]<<8) | (b[i]);
					
					// search free entry
					for (found=0;found<k;found++)
					{
						// leer oder passend gefüllter slot ?
					
						if (histogramm[found]==-1)
						{					
							CLUT[found].r = r[i];
							CLUT[found].g = g[i];
							CLUT[found].b = b[i];
							
							if ((r[i] == 0) && (g[i] == 0) && (b[i] == 0))
							{
								CLUT[found].r = r[i] = 0;
								CLUT[found].g = g[i] = 0;
								CLUT[found].b = b[i] = 1;
							}
							
							if (bitmask)
								CLUT[found].stp = bitmask[i];
							else
								CLUT[found].stp = stp;

							histogramm[found] = mark;
						}
					
						if (histogramm[found] == mark)
						{
							mark = 0x12345678;
							break;
						}			
					}

					if (mark != 0x12345678)
					{
						fprintf(stderr,"please quantize input file!\n");
						exit(1);
					}

					// set the index representation at X,Y to this index				
					index[i] = found;
				}
			}

			pixel = TIM_GetFirstPixel(tim);

			switch (TIM_GetMode(tim))
			{
				case TIM_4BIT:
					{
						unsigned char *p = (unsigned char*) pixel;
						
						fprintf(stderr,"writing 4-bit data..\n");
						
						for (i=0;i<(width*height);i+=2)
						{
							*p++ = ((index[i+1]&15)<<4) | (index[i]&15);
						}
					}
					break;
				case TIM_8BIT:
					// data are already 8-bit, just memcpy!
						fprintf(stderr,"writing 8-bit data..\n");
					memcpy(pixel,index,width*height);
					break;
			}

			/* correct color gun bit depth */
			sh=0;
			while (depth>31)
			{
				sh++;
				depth >>= 1;
			}

			pixel=TIM_GetFirstColor(tim);
			for (i=0;i<k;i++,pixel++)
			{
				*pixel = (((CLUT[i].r>>sh)&TIM_CLUT_REDMASK) << TIM_CLUT_REDSHIFT) |
						 (((CLUT[i].g>>sh)&TIM_CLUT_GRNMASK) << TIM_CLUT_GRNSHIFT) |
						 (((CLUT[i].b>>sh)&TIM_CLUT_BLUMASK) << TIM_CLUT_BLUSHIFT) |
						 (CLUT[i].stp ? 0x8000 : 0x0000);
			}
		}

		// please note that 4/8 bit images were handled (direct->indexed color) above

		else switch (TIM_GetMode(tim))
		{
			case TIM_15BIT:

				fprintf(stderr,"writing 15-bit data..\n");
				/* correct color gun bit depth */
				sh=0;
				while (depth>31)
				{
					sh++;
					depth >>= 1;
				}

				if (bitmask)
				{
					for (pixel=TIM_GetFirstPixel(tim),i=0;i<width*height;i++,pixel++)
					{
						// for each bitmask[i] set, the msb will be set
			
						*pixel = (((r[i]>>sh)&TIM_CLUT_REDMASK) << TIM_CLUT_REDSHIFT) |
								 (((g[i]>>sh)&TIM_CLUT_GRNMASK) << TIM_CLUT_GRNSHIFT) |
								 (((b[i]>>sh)&TIM_CLUT_BLUMASK) << TIM_CLUT_BLUSHIFT) |
								 (bitmask[i] ? 0x8000 : 0x0000);
					}
				}
				else	// without mask (no stb processing)
				{
					for (pixel=TIM_GetFirstPixel(tim),i=0;i<width*height;i++,pixel++)
					{
						*pixel = (((r[i]>>sh)&TIM_CLUT_REDMASK) << TIM_CLUT_REDSHIFT) |
								 (((g[i]>>sh)&TIM_CLUT_GRNMASK) << TIM_CLUT_GRNSHIFT) |
								 (((b[i]>>sh)&TIM_CLUT_BLUMASK) << TIM_CLUT_BLUSHIFT) |
								 (stp ? 0x8000 : 0x0000);
					}
				}
				break;
			
			case TIM_24BIT:
			{
				unsigned char *pixel2;
			
				fprintf(stderr,"writing 24-bit data..\n",TIM_SizeOf(tim));

				for (pixel2=(unsigned char *)TIM_GetFirstPixel(tim),i=0;i<(width*height);i++)
				{
					*pixel2 = b[i]; pixel2++;
					*pixel2 = g[i]; pixel2++;
					*pixel2 = r[i]; pixel2++;
				}
			}	break;
				
			default:
				break;
		
		}

		fwrite(tim,1,TIM_SizeOf(tim),stdout);

		if (indexed)
		{
			free(index);
		}
		else
		{
			free(r);
			free(g);
			free(b);
		}
		free(tim);
	}
	else fprintf(stderr,TIM_GetErrorString());
}

int main (int argc, char **argv)
{
	char *filename = NULL;
	char ppmid[2];
	int opt;
	int indexed = -1;

	int flag_mode  = TIM_15BIT;
    int flag_cluts = 1;
	int flag_clutx = 0;
	int flag_cluty = 0;
	int flag_datax = 0;
	int flag_datay = 0;
	int flag_noheader = 1;
	char *flag_mask = 0;
	int flag_stp = 0;
	char *flag_palette = 0;

	bitmask = 0;

	while ((opt = getopt_long(argc,argv,option_chars,long_options,(int*)NULL)) != EOF)
	{
		switch (opt)
		{
			case 'h':	/* --help */
				fprintf(stderr,help_text,argv[0]);
				exit(0);
				break;
			case 'V':	/* --version */
				fprintf(stderr,version_text,argv[0]);
				exit(0);
				break;
			case '0':	/* --4bit */
				flag_mode = TIM_4BIT;
				break;
			case '1':	/* --8bit */
				flag_mode = TIM_8BIT;
				break;
			case '2':	/* --15bit */
				flag_mode = TIM_15BIT;
				break;
			case '3':	/* --24bit */
				flag_mode = TIM_24BIT;
				break;
			case 'c':	/* --numcluts */
				flag_cluts = atoi(optarg);
				break;
			case 'x':	/* --clutx */
				flag_clutx = atoi(optarg);
				break;
			case 'y':	/* --cluty */
				flag_cluty = atoi(optarg);
				break;
			case 'X':	/* --posx */
				flag_datax = atoi(optarg);
				break;
			case 'Y':	/* --posy */
				flag_datay = atoi(optarg);
				break;
			case 'm':	/* --mask */
				flag_mask = optarg;
				break;
			case 'n':	/* --noheader */
				flag_noheader = 0;
				break;
			case 't':	/* --transparent */
				flag_stp = 1;
				break;
			case 'p':	/* --palette */
				flag_palette = optarg;
				break;
			default:
				fprintf(stderr,"%s: unknown option %c, aborting\n",argv[0],opt);
				break;
		}
	}

	if (flag_noheader) fprintf(stderr,copyright_text,argv[0]);

	if (optind < argc) filename = argv[optind];

	/* add some checking code */

	switch (flag_mode)
	{
		case TIM_4BIT:
			if ((flag_cluts<1) || (flag_cluts>16)) fprintf(stderr,"Number of CLUTS for 4-bit must be 1<=x<=16!\n");
			break;
		case TIM_8BIT:
			if (flag_cluts!=1) fprintf(stderr,"Number of CLUTS for 8-bit must be 1!\n");
			break;
		case TIM_15BIT:
			break;
		case TIM_24BIT:
			break;
		default:
			break;
	}

	/* redirect file from stdin */

	if (filename) freopen(filename,"rb",stdin);

	if (ferror(stdin))
	{
		perror(filename);
		exit(1);
	}
	
	/* read header bytes. only P3/P6 are allowed. */

	ppmid[0] = getchar(); ppmid[1] = getchar();

	if (ferror(stdin))
	{
		perror("could not read header");
		exit(1);
	}

	if (((ppmid[0]=='P') && (ppmid[1]=='3' || ppmid[1]=='6')))
	{
		indexed = 0;		
	}

	if (((ppmid[0]=='P') && (ppmid[1]=='2' || ppmid[1]=='5')))
	{
		indexed = 1;
	}

	if (indexed==-1) {
		fprintf(stderr,"input file format neither pgm/ppm (P2/P4,P3/P6 format)!\n");
		exit(1);
	}

	ppmid[1] -= '3';	/* make a boolean, if 0 then ascii else binary */

	yyrestart(stdin);

	ppm_to_tim (ppmid[1],indexed,flag_mode,flag_cluts,flag_clutx,flag_cluty,flag_datax,flag_datay,flag_stp,flag_mask,flag_palette);

	if (bitmask) free(bitmask);

	return 0;
}
