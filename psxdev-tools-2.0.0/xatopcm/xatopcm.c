/* $Id: xatopcm.c,v 1.2 2001/05/17 12:25:49 dbalster Exp $ */

/*
	xatopcm - multichannel XA track ripper for linux

	Copyright (C) 1999, 2000 by Daniel Balster <dbalster@psxdev.de>
	  
	  XA decoder by unknown author (he/she forgot to sign
	  the source, please contact me)

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
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

/*
	if this doesn't compile, you probably have a broken kernel.
	We need FIBMAP from linux/fs.h, but kernels 2.3.99-pre?
	seem to include dcache.h without __KERNEL__ defined.
*/

#include <linux/fs.h>

#include "xadecode.h"

#define XAAUDIO		0x64
#define XAVIDEO		0x48
#define XABREAK		0xE4

void frame2msf(unsigned long i, struct cdrom_msf *msf)
{
	msf->cdmsf_min0  =   i   /CD_SECS   /CD_FRAMES;
	msf->cdmsf_sec0  =  (i   /CD_FRAMES)%CD_SECS;
	msf->cdmsf_frame0=   i   %CD_FRAMES;
	msf->cdmsf_min1  =  (i+1)/CD_SECS   /CD_FRAMES;
	msf->cdmsf_sec1  = ((i+1)/CD_FRAMES)%CD_SECS;
	msf->cdmsf_frame1=  (i+1)%CD_FRAMES;
}

int main (int argc, char **argv)
{
	char *inname = argv[1];
	char *outname = argv[2];
	int fd, rc, device;
	struct stat st;
	long sector = 0;
	long size;
	long nsectors;
	long i;
	FILE *cfd[256];	// maximum no. of channel file descriptors
	int brks[256];
	char fname[1024];
	char *devname;
	SoundSector buffer;
	char wave[8192];

	if (argc==1)
	{
		fprintf(stderr,"usage: xatopcm <infile> <outfile> <devicename>\n");
		return 0;
	}

	devname = "/dev/cdrom";
	if (argc>3) devname = argv[3];

	// close all channel fds
	for (i=0; i<256; ++i) { cfd[i] = 0; brks[i] = 0; }

	device = open(devname,O_RDONLY);
	if (device == -1) { perror(devname); exit(1); }

	if (strcmp(inname,"raw")==0)
	{
		sector = 400;
		nsectors = 10000000;
	}
	else
	{
		fd = open(inname,O_RDONLY);
		if (fd == -1) { perror(inname); exit(1); }

		rc = ioctl (fd,FIBMAP,&sector);
		if (rc == -1) { perror(inname); exit(1); }

		rc = fstat (fd,&st);
		if (rc == -1) { perror(inname); exit(1); }

		nsectors = st.st_size / 2048;
		
		close (fd);
	}
	fprintf(stderr,"*** XA to PCM multi-stream ripper ***\n");
	fprintf(stderr,"Device      : %s\n",devname);
	fprintf(stderr,"File        : %s\n",inname);
	fprintf(stderr,"Template    : %s\n",outname);
	fprintf(stderr,"First sector: %d\n",sector);
	fprintf(stderr,"No.  sectors: %d\n",nsectors);

	initXaDecode();
	for (i=sector; i<nsectors+sector; ++i)
	{
		int n,j;
		char channel;
		long length;

		frame2msf (i,(struct cdrom_msf*)&buffer);
		
		rc = ioctl (device,CDROMREADRAW,&buffer);
		if (rc == -1)
		{
			perror(devname);
	//		break;
		}

		channel = xachannel (&buffer);

		switchXaDecode(channel);
		length = convXaToWave ((char*)&buffer, wave,channel,0,255);
		saveXaDecode(channel);

		if (xatype(&buffer)==XABREAK)
		{
			if (cfd[channel]!=0)
			{
				printf ("%dc%d closed\n",brks[channel],channel);
				fclose(cfd[channel]);
				cfd[channel] = 0;
				brks[channel]++;
			}
		}

		if (length)
		{
			if (cfd[channel]==0)
			{
				sprintf(fname,"%s%dc%d.pcm",outname,brks[channel],channel);
				cfd[channel] = fopen(fname,"wb");
				printf("new file: %s (%s, %s Hz)\n"
					,fname
					,xastereo(&buffer)?"stereo":"mono"
					,xahalfhz(&buffer)?"18900":"37800"
					);
				reinitXaDecode(channel);
			}
			fwrite (wave,length,1,cfd[channel]);
		}
	}

	for (i=0; i<256; ++i) if(cfd[i] != 0) fclose(cfd[i]);

	return 0;
}
