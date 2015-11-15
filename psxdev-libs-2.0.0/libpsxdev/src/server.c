/* $Id: server.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
	libpsxdev - library of useful functions for PSX developers

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
#include <psxdev.h>
#include <pccl.h>

#include <sys/file.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>

/*
	Attach a server function to an already opened /dev/pccl.
	This serves:
	
	- "pcdrv:" access
	- stdio
	
	each ``psxdev_server_func_t'' works like this:
	
	input: /dev/pccl file handle
	output:
		0 = abort
		1 = continue
		negative == error
*/

typedef struct {
	DIR *dirstream;
	int iswild;
	char pathname[BUFSIZ], filename[BUFSIZ];
	int dev, in, out;
} server_context_t;

typedef int (*psxdev_server_func_t)(server_context_t *);

static const char *copyright = N_("Copyright (C) 2000 by Daniel Balster <dbalster@psxdev.de>");


static int server_nop (server_context_t *);
static int server_putchar (server_context_t *);
static int server_getchar (server_context_t *);
static int server_open (server_context_t *);
static int server_create (server_context_t *);
static int server_read (server_context_t *);
static int server_write (server_context_t *);
static int server_seek (server_context_t *);
static int server_close (server_context_t *);
static int server_delete (server_context_t *);
static int server_rename (server_context_t *);
static int server_files (server_context_t *);
static int server_nfiles (server_context_t *);
static int server_chdir (server_context_t *);
static int server_curdir (server_context_t *);
static int server_chdrv (server_context_t *);
static int server_curdrv (server_context_t *);
static int server_quit (server_context_t *);

static psxdev_server_func_t psxdev_server_table[] = {
	server_nop,
	server_putchar, server_getchar, server_open,
	server_create,  server_read,    server_write,
	server_seek,	server_close,   server_delete,
	server_rename,  server_files,   server_nfiles,
	server_chdir,   server_curdir,  server_chdrv,
	server_curdrv,  server_quit
};
#define NFUNC sizeof(psxdev_server_table)/sizeof(psxdev_server_func_t)

int psxdev_server (int dev, int sockin, int sockout)
{
	int code;
	server_context_t ctx;
	
	ctx.dev = dev;
	ctx.in = sockin;
	ctx.out = sockout;
	ctx.dirstream = NULL;
	
	do {
		/*
			it is normal that this functions timeouts. then we do a fake
			opcode "code=0", which simply waits a second.
		*/
	
		code = ioctl (dev,CAEIOC_COMMAND_RAW,0);
		if (code < 0) code = 0;
		if (code >= NFUNC) return -1;
		
		code = (*psxdev_server_table[code])(&ctx);

	} while (code==0);

	if (ctx.dirstream) closedir(ctx.dirstream);
	
	return code;
}

/* this NOP is called on timeout of the main loop (saves CPU time) */

static int server_nop (server_context_t *ctx)
{
	int err;
	caetla_status_t cst;
	
	// new: abort server automatically in case of loosing control
	
	err = ioctl(ctx->dev, CAEIOC_GET_STATUS_CAETLA, &cst);

//	fprintf (stderr, "server ping.. (%02x %d)\n",cst,err);

	if (err!=-1) return (int) cst;

	return -1;
}

/**/

static int server_putchar (server_context_t *ctx)
{
	int err;
	u_char byte;

	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;
	err = write (ctx->out,&byte,1);
	if (err < 0) return err;

	return 0;
}

/**/

static int server_getchar (server_context_t *ctx)
{
	int err;
	u_char byte;

	err = read (ctx->in,&byte,1);
	if (err < 0) return err;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_open (server_context_t *ctx)
{
	char buf[BUFSIZ];
	u_char byte;
	u_short word;
	int len;
	int err;
	int fd;
	
	len = psxdev_pccl_get_string (ctx->dev,buf,BUFSIZ);
	if (len < 0) return len;

//	psxdev_filename_psx_to_unix (buf);

	// get the mode. bug: result is always 2 ?

	byte = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	fd = open (buf, O_RDWR);
	if (fd < 0)
	{
		fd = open (buf, O_RDONLY);
		if (fd < 0) perror ("open");
	}

	byte = (fd < 0) ? 0xFF : 0x00;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	word = (u_short) fd;
	err = ioctl(ctx->dev,CAEIOC_SWAP_16,&word);
	if (err < 0) return err;

	return 0;
}

static int server_create (server_context_t *ctx)
{
	char buf[BUFSIZ];
	u_char byte;
	u_short word;
	int len;
	int err;
	int fd;

	len = psxdev_pccl_get_string (ctx->dev,buf,BUFSIZ);
	if (len < 0) return len;

//	psxdev_filename_psx_to_unix (buf);

	// todo: make flags configurable
	fd = creat (buf, 0644);
	if (fd < 0) perror ("creat");

	byte = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	byte = (err < 0) ? 0xFF : 0x00;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	word = (u_short) fd;
	err = ioctl(ctx->dev,CAEIOC_SWAP_16,&word);
	if (err < 0) return err;

	return 0;
}

static int server_read (server_context_t *ctx)
{
	u_char byte, buf[1024];
	int err;
	u_short word;
	u_long dword;
	int fd, size, r;

	word = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_16,&word);
	if (err < 0) return err;
	fd = (word>255) ? (word>>8) : word;

	size = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_32,&size);
	if (err < 0) return err;

/*
	this is a unique feature to get the filesize. use it like this:
	
	int size;
	read (fd,&size,-1)
	lseek (fd,-sizeof(int),0)

	the "size=-1 indicates the mode. you have to fix the position
	afterwards with the lseek().
*/

	if (size == -1)
	{
		struct stat st;
		
		fstat (fd,&st);
	
		r = st.st_size;

		dword = 4;
		err = ioctl(ctx->dev,CAEIOC_SWAP_32,&dword);
		if (err < 0) return err;

		byte = psxdev_pccl_put_memory (ctx->dev, (u_char*) &r, 4);

		// send checksum
		err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
		if (err < 0) return err;
	}
	else
	{
		while (size>0)
		{
			r = (size >= 1024) ? 1024 : size;
			r = read (fd, buf, r);
			
			if (r == 0) break;	// eof
			
			if (r < 0)
			{
				// todo: handle error
				perror("read");
				break;
			}

			// send size
			dword = r;
			err = ioctl(ctx->dev,CAEIOC_SWAP_32,&dword);
			if (err < 0) return err;

			byte = psxdev_pccl_put_memory (ctx->dev, (u_char*)buf, r);

			// send checksum
			err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
			if (err < 0) return err;

			size -= r;
		}
	}

	// stop sending chunks (send zero length and checksum)

	dword = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_32,&dword);
	if (err < 0) return err;

	byte = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_write (server_context_t *ctx)
{
	u_char byte, buf[1024];
	int err;
	u_short word;
	u_long dword;
	int fd, size, r;

	word = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_16,&word);
	if (err < 0) return err;
	fd = (word>255) ? (word>>8) : word;

	size = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_32,&size);
	if (err < 0) return err;

	while (size>0)
	{
		r = (size >= 1024) ? 1024 : size;

		// send size
		dword = r;
		err = ioctl(ctx->dev,CAEIOC_SWAP_32,&dword);
		if (err < 0) return err;

		byte = psxdev_pccl_get_memory (ctx->dev, buf, r);

		// send checksum
		err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
		if (err < 0) return err;

		r = write (fd, buf, r);
		if (r < 0)
		{
			// todo: handle error
			perror("write");
			break;
		}

		size -= r;
	}

	// stop sending chunks (send zero length and checksum)

	dword = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_32,&dword);
	if (err < 0) return err;

	byte = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_seek (server_context_t *ctx)
{
	int err;
	u_char byte;
	u_long dword;
	u_short word;
	int fd, mode, rc;
	off_t offset;

	word = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_16,&word);
	if (err < 0) return err;
	fd = (word>255) ? (word>>8) : word;

	dword = (u_long) rc;
	err = ioctl(ctx->dev,CAEIOC_SWAP_32,&dword);
	if (err < 0) return err;
	offset = dword;

	mode = (offset < 0) ? SEEK_CUR : SEEK_SET;

	rc = lseek (fd,offset,mode);
	if (rc < 0) perror ("lseek");

	byte = (rc < 0) ? 0xFF : 0x00;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	dword = (u_long) rc;
	err = ioctl(ctx->dev,CAEIOC_SWAP_32,&dword);
	if (err < 0) return err;

	return 0;
}

static int server_close (server_context_t *ctx)
{
	int err;
	u_char byte;
	u_short word;
	int fd;

	word = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_16,&word);
	if (err < 0) return err;
	fd = (word>255) ? (word>>8) : word;

	close (fd);

	byte = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_delete (server_context_t *ctx)
{
	char buf[BUFSIZ];
	int err;
	u_char byte;

	err = psxdev_pccl_get_string (ctx->dev,buf,BUFSIZ);
	if (err < 0) return err;

	err = unlink (buf);
	if (err < 0) perror ("unlink");

	byte = (err < 0) ? 0xFF : 0x00;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_rename (server_context_t *ctx)
{
	char buf1[BUFSIZ];
	char buf2[BUFSIZ];
	int err;
	u_char byte;

	err = psxdev_pccl_get_string (ctx->dev,buf1,BUFSIZ);
	if (err < 0) return err;

	err = psxdev_pccl_get_string (ctx->dev,buf2,BUFSIZ);
	if (err < 0) return err;

	err = rename (buf1,buf2);
	if (err < 0) perror ("unlink");

	byte = (err < 0) ? 0xFF : 0x00;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_files (server_context_t *ctx)
{
	u_char byte;
	char buf[BUFSIZ], *p;
	u_short word;
	u_long dword;
	int len;
	struct stat st;
	int err, i;
	struct dirent * direntry;

	word = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_16,&word);
	if (err < 0) return err;

	len = psxdev_pccl_get_string (ctx->dev,buf,BUFSIZ);
	if (len < 0) return len;

	err = stat (buf,&st);
	
	if ( (err>=0) && S_ISDIR(st.st_mode))
	{
		strcpy (ctx->pathname,buf);
		ctx->filename[0] = 0;
	}
	else
	{
		p = (char*) strrchr((char*)buf,(int)'/');
		if (p == NULL)
		{
			strcpy (ctx->filename,buf);
			strcpy (ctx->pathname,"./");
		}
		else
		{
			*p = 0;
			strcpy (ctx->filename,p+1);
			strcpy (ctx->pathname,buf);
		}
	}
	strcat (ctx->pathname,"/");
	
	if (strlen(ctx->filename)) ctx->iswild = 1;

	if (ctx->dirstream) closedir(ctx->dirstream);
	ctx->dirstream = opendir (ctx->pathname);

	return server_nfiles (ctx);
}

static int server_nfiles (server_context_t *ctx)
{
	u_char byte, success = 0xFF;
	char buf[BUFSIZ], *p;
	u_short word;
	u_long dword;
	int len;
	struct stat st;
	int err, i;
	struct dirent * direntry;
	u_char attr = 0x00;

	if (ctx->dirstream != NULL)
	{
		int found = 1;
		do
		{
			direntry = readdir (ctx->dirstream);
		
			if (direntry != NULL)
			{
				found = fnmatch(ctx->filename, direntry->d_name, 0);

				if ((ctx->iswild==0) || (found==0))
				{
		    		success=0x00;
		    		strncpy(buf, ctx->pathname, BUFSIZ);
		    		strncat(buf, "/", BUFSIZ);
		    		strncat(buf, direntry->d_name, BUFSIZ);
		    		err = stat(buf, &st);
					if (err < 0) perror("stat");
				}
			}
		}
		while (ctx->iswild && (direntry!=NULL) && (found==1));
	}
	else perror ("opendir");

	if (st.st_mode & S_IFDIR) attr |= 0x10;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&attr);
	if (err < 0) return err;

	// we' don't want to be Y2K compliant (these are the time fields)

	word = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_16,&word);
	if (err < 0) return err;
	err = ioctl(ctx->dev,CAEIOC_SWAP_16,&word);
	if (err < 0) return err;

	dword =
		(((st.st_size)>>24)&0xff) |
		(((st.st_size)>>16)&0xff)<<8 |
		(((st.st_size)>>8)&0xff)<<16 |
		(((st.st_size))&0xff)<<24;
	err = ioctl(ctx->dev,CAEIOC_SWAP_32,&dword);
	if (err < 0) return err;

	/* caetla/msdos only allow 8+3 filenames */

	for (i=0;i<13;i++)
	{
		byte = (success == 0x00) ? direntry->d_name[i] : 0;
		err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
		if (err < 0) return err;
	}
	
	byte = success;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_chdir (server_context_t *ctx)
{
	char buf[BUFSIZ];
	int err;
	u_char byte;

	err = psxdev_pccl_get_string (ctx->dev,buf,BUFSIZ);
	if (err < 0) return err;

	err = chdir (buf);
	if (err < 0) perror ("chdir");

	byte = (err < 0) ? 0xFF : 0x00;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_curdir (server_context_t *ctx)
{
	char buf[BUFSIZ];
	int err, rc;
	u_char byte;

	byte = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	getcwd (buf,BUFSIZ);
	rc = errno;
	
	if (rc < 0)
	{
		perror ("getcwd");

		byte = 0;
		err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
		if (err < 0) return err;
	}
	else
	{
		err = psxdev_pccl_put_string (ctx->dev,buf,BUFSIZ);
		if (err < 0) return err;
	}

	byte = (rc < 0) ? 0xFF : 0x00;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_chdrv (server_context_t *ctx)
{
	int err;
	u_char byte;

	byte = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_curdrv (server_context_t *ctx)
{
	int err;
	u_char byte;

	byte = 0;
	err = ioctl(ctx->dev,CAEIOC_SWAP_8,&byte);
	if (err < 0) return err;

	return 0;
}

static int server_quit (server_context_t *ctx)
{
	return 1;
}
