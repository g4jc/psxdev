/* $Id: pccl.c,v 1.1.1.1 2001/05/17 11:51:04 dbalster Exp $ */

/*
	pccl - a driver for PCcommslink board (connected to actionreplay & caetla)

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

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/wrapper.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/segment.h>
#include <linux/malloc.h>
#include <linux/ioport.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <linux/types.h>
#include <linux/wrapper.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>

#if LINUX_VERSION_CODE <= 0x020100
#error currently only for kernel 2.1 and above
#error you may need to adapt the file_operations for kernel 2.0
#endif

#ifndef MODULE
#error this driver is currently only available as a kernel module
#endif

#if CONFIG_MODVERSIONS==1
#include <linux/modversions.h>
#endif        

#ifdef CONFIG_KMOD
#include <linux/kmod.h>
#endif

#include "pccl.h"

/********************************************************/

#define PARANOIA
#define dprintk if (debug) printk

/********************************************************/

#define PCCL_NAME            "pccl"

#define PCCL_INIT_MESSAGE \
	"pccl: pccl driver for linux $Revision: 1.1.1.1 $\n"

#define PCCL_BASE_ADDRESS	0x80000000
#define PCCL_LOWER_ADDRESS	0x00000000
#define PCCL_UPPER_ADDRESS	0x001ffff0

#define PCCL_VRAM_SIZE 0x100000

/*
	the first /proc/pccl/?/mem interface had not a very good design.
	the second interface provides:
	
	/proc/pccl/?/fullmem

RAM
A	0x00000000 - 0x001FFFFF effective
B	0x80000000 - 0x801FFFFF effective
C	0xA0000000 - 0x801FFFFF not effective

Scratchpad (4KB)
S	0x1F800000 - 0x1F8003FF not effective

Device
X	0x1F801000 - 0x1FBFFFFF not effective

ROM
P	0x1FC00000 - 0x1FCFFFFF not effective
Q	0x9FC00000 - 0x9FC7FFFF effective
R	0xBFC00000 - 0xBFC7FFFF not effective

*/

/*
	description of device flags. PCCL_??() are just macros.
	
	CONFIGURED:  device is correctly set up and physically present
	OPEN:        device is currently in use (exclusive only)
	AUTO_RESUME: not yet. give control back to caetla as fast as possible
	READ_MODE:   last ioctl() put device in read mode
	WRITE_MODE:  last ioctl() put device in write mode
*/

#define PCCL_FLAG_CONFIGURED  (1L<<0)
#define PCCL_FLAG_OPEN        (1L<<1)
#define PCCL_FLAG_AUTO_RESUME (1L<<2)
#define PCCL_FLAG_READ_MODE   (1L<<3)
#define PCCL_FLAG_WRITE_MODE  (1L<<4)

#define PCCL_CHECK(dev,flag) \
	(((dev)->flags & PCCL_FLAG_ ## flag)==PCCL_FLAG_ ## flag)

#define PCCL_SET(dev,flag) \
	((dev)->flags |= PCCL_FLAG_ ## flag)

#define PCCL_CLEAR(dev,flag) \
	((dev)->flags &=~ PCCL_FLAG_ ## flag)

/********************************************************/

#ifdef MODULE

MODULE_AUTHOR("Daniel Balster <dbalster@psxdev.de>");
MODULE_DESCRIPTION("A driver for the PC commslink card.");

MODULE_PARM(io, "1-" __MODULE_STRING(PCCL_MAX_BOARDS) "s");
MODULE_PARM_DESC(io, "override autodetection");
char *io = 0;

MODULE_PARM(debug, "i");
MODULE_PARM_DESC(debug, "be more verbose");
int debug = 0;

MODULE_PARM(delay, "i");
MODULE_PARM_DESC(delay, "number of jiffies until timeout (default: 200)");
int delay = 200;

EXPORT_NO_SYMBOLS;

#endif

/********************************************************/

static struct pccl_device pccl[PCCL_MAX_BOARDS];
static pid_t pccl_lockers[PCCL_MAX_BOARDS];
static int pccl_boards = 0;

/********************************************************/

/*
**	lowlevel I/O port communication. exchange 8,16 and 32 bit values.
*/

u_char pccl_swap_8 (struct pccl_device *dev, u_char val)
{
	time_t timeout = jiffies + delay;

	outb_p (val, dev->base);
	while ( inb_p(dev->base+2) & 1 )
	{
		if (sigismember(&current->signal, SIGINT)) break;

		if (timeout < jiffies)
		{
			dprintk (KERN_DEBUG "pccl: I/O timeout\n");
			break;
		}

		schedule();
	}

	return inb_p (dev->base);
}

u_short pccl_swap_16 (struct pccl_device *dev, u_short val)
{
	return ( pccl_swap_8(dev,val>>8) << 8 | pccl_swap_8(dev,val) );
}

u_long pccl_swap_32 (struct pccl_device *dev, u_long val)
{
	return ( pccl_swap_16(dev,val>>16) << 16 | pccl_swap_16(dev,val) );
}

/*
** lowlevel (non user-mode) string and block transfer functions
*/

void pccl_send_string (struct pccl_device *dev, char *string)
{
	while (*string) pccl_swap_8 (dev,*string++);
}

void pccl_recv_string (struct pccl_device *dev, char *string)
{
	while ((*string++ = pccl_swap_8 (dev,0)));
}

u_char pccl_send_mem (struct pccl_device *dev, void *buffer, u_long size)
{
	u_char *p=buffer, chk=0;
	u_long c;

	for (c=0;c<size;c++) pccl_swap_8 (dev,*p++);

	return chk;
}

u_char pccl_recv_mem (struct pccl_device *dev, void *buffer, u_long size)
{
	u_char *p=buffer, chk=0, ck;
	u_long c;

	for (c=0;c<size;c++,chk+=ck) *p++ = (ck = pccl_swap_8 (dev,0));

	return chk;
}

/********************************************************/

/* caetla command handshake: exchange 'CL' against 'do' */

static int pccl_caetla_command (struct pccl_device *dev, u_char cmd)
{
	u_char x;
	time_t timeout = jiffies + delay;

	while (timeout > jiffies)
	{
		x = pccl_swap_8(dev,'C');
		if (x == 'd') break;
		if (sigismember(&current->signal, SIGINT)) return -EINTR;
		schedule();
	}

	x = pccl_swap_8 (dev,'L');
	if (x != 'o') return -EREMOTEIO;

	return pccl_swap_8 (dev, cmd);
}

/* datel (?) command handshake: exchange 'PR' against 'do' */

static int pccl_caetla_command_raw (struct pccl_device *dev, u_char cmd)
{
	u_char x;
	time_t timeout = jiffies + delay;

	while (timeout > jiffies)
	{
		x = pccl_swap_8(dev,'P');
		if (x == 'd') break;
		if (sigismember(&current->signal, SIGINT)) return -EINTR;
		schedule();
	}

	x = pccl_swap_8 (dev,'R');
	if (x != 'o') return -EREMOTEIO;

	return pccl_swap_8 (dev, cmd);
}

/* basic open/close/read/write function of /dev/pccl* devices */

static int common_open (struct inode *inode, struct file *file, struct pccl_device *dev)
{
	if (dev == NULL) return -ENODEV;

	dprintk (KERN_DEBUG "pccl: open(io=0x%03x flags=%x)\n",dev->base,dev->flags);
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (PCCL_CHECK(dev,OPEN)) return -EBUSY;
	MOD_INC_USE_COUNT;
	PCCL_SET(dev,OPEN);
	spin_lock (&dev->lock);

	// write() and read() on /dev/pccl should fail by default.
	// you have to select a mode first with ioctl() !

	dev->offset = 0;
	dev->offset2 = 0;
	
	PCCL_CLEAR(dev,READ_MODE);
	PCCL_CLEAR(dev,WRITE_MODE);

	pccl_lockers[dev->id & 3] = current->pid;
	
	return 0;
}

static int common_close (struct inode *inode, struct file *file, struct pccl_device *dev)
{
	if (dev == NULL) return -ENODEV;

	pccl_lockers[dev->id & 3] = 0;

	dprintk (KERN_DEBUG "pccl: release(io=0x%03x flags=%x)\n",dev->base,dev->flags);
	spin_unlock (&dev->lock);
	PCCL_CLEAR(dev,OPEN);
	MOD_DEC_USE_COUNT;
	
	return 0;
}

/* open: try to lock device against concurrent accesses */

static int pccl_open (struct inode *inode, struct file *file)
{
	struct pccl_device *dev = &pccl[MINOR(inode->i_rdev) & 3];
	return common_open (inode,file,dev);
}

/* close: release the lock */

static int pccl_release (struct inode *inode, struct file *file)
{
	struct pccl_device *dev = &pccl[MINOR(inode->i_rdev) & 3];
	return common_close (inode,file,dev);
}

/* read: get ``size'' bytes, this is interruptible */

static ssize_t pccl_read (
	struct file *file,
	char *buffer,
	size_t size,
	loff_t *offset)
{
	struct inode *inode = file->f_dentry->d_inode;
	int devnum = MINOR(inode->i_rdev);
	struct pccl_device *dev = &pccl[devnum];
	int i, n, m, o, err;
	u_char buf[PAGE_SIZE];
	short x;

	dprintk (KERN_DEBUG "pccl: read(size=%d)\n",size);

	if (!PCCL_CHECK(dev,READ_MODE)) return -EBADFD;

#if defined PARANOIA
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
	if ((size<=0) || (size>0x200000)) return -EINVAL;
#endif

	dev->checksum = 0x00;
	dev->checksum2 = 0x0000;

	/*
		copy_to_user seems to be slow, so we work on a PAGE_SIZE'd buffer
	*/

	m = size;
	o = 0;
	while (m > 0)
	{
		n = (m>PAGE_SIZE) ? PAGE_SIZE : m;
	
		for (i=0; i<n; i++)
		{
			x = (short) buf[i] = pccl_swap_8 (dev,0);
			dev->checksum += buf[i];
			dev->checksum2 += x;

			if (sigismember(&current->signal, SIGINT)) { size = -EINTR; break; }
		}
		
		err = copy_to_user ((void*)buffer+o,(void*)buf,n);
		if (err < 0) { size = err; break; }
		
		o += n;
		m -= n;
	}

	PCCL_CLEAR (dev,READ_MODE); // read mode is over

	return size;
}

/* write: put ``size'' bytes, this is interruptible */

static ssize_t pccl_write (
	struct file *file,
	const char *buffer,
	size_t size,
	loff_t *offset)
{
	struct inode *inode = file->f_dentry->d_inode;
	int devnum = MINOR(inode->i_rdev);
	struct pccl_device *dev = &pccl[devnum];
	int i, n, m, o, err;
	u_char buf[PAGE_SIZE];

	dprintk (KERN_DEBUG "pccl: write(size=%d)\n",size);

	if (!PCCL_CHECK(dev,WRITE_MODE)) return -EBADFD;

#if defined PARANOIA
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
	if ((size<=0) || (size>0x200000)) return -EINVAL;
#endif

	dev->checksum = 0x00;
	dev->checksum2 = 0x0000;

	m = size;
	o = 0;
	while (m > 0)
	{
		n = (m>PAGE_SIZE) ? PAGE_SIZE : m;

		err = copy_from_user ((void*)buf,(void*)buffer+o,n);
		if (err < 0) { size = err; break; }
		

		for (i=0; i<n; i++)
		{
			pccl_swap_8 (dev,buf[i]);
			dev->checksum += buf[i];
			dev->checksum2 += (short) buf[i];

			if (sigismember(&current->signal, SIGINT)) { size = -EINTR; break; }
		}
		
		o += n;
		m -= n;
	}

	PCCL_CLEAR (dev,WRITE_MODE); // write mode is over

	return size;
}

/************************************************************/

/*
	lowlevel functions for the user-mode. it is possible to serve caetla
	completly with these xxx_swap_xxx ioctl()s, if you want. it's not a
	very clean API, but the stdio+fileserver has to live in the user-mode
	and it needs these lowlevel functions.
*/

static int caeioc_swap_8 (
	struct pccl_device *dev,
	unsigned long param)
{
	u_char data;
	int err;
	
	err = copy_from_user ((void*)&data,(void*)param,sizeof(data));
	if (err < 0) return err;

	data = pccl_swap_8 (dev,data);

	return copy_to_user ((void*)param,(void*)&data,sizeof(data));
}

static int caeioc_swap_16 (
	struct pccl_device *dev,
	unsigned long param)
{
	u_short data;
	int err;
	
	err = copy_from_user ((void*)&data,(void*)param,sizeof(data));
	if (err < 0) return err;

	data = pccl_swap_16 (dev,data);

	return copy_to_user ((void*)param,(void*)&data,sizeof(data));
}

static int caeioc_swap_32 (
	struct pccl_device *dev,
	unsigned long param)
{
	u_long data;
	int err;
	
	err = copy_from_user ((void*)&data,(void*)param,sizeof(data));
	if (err < 0) return err;

	data = pccl_swap_32 (dev,data);

	return copy_to_user ((void*)param,(void*)&data,sizeof(data));
}

/*
	control: get control over caetla and select a menu/mode
	memory card manager (mcm) may take some time to become active,
	so the jiffie timeout is set by default to something arround 3 seconds.
*/

static int caeioc_control (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	int menu = param;

	do {
		err = pccl_caetla_command (dev,0x01);
		if (err < 0) return err;

		param = pccl_swap_8 (dev,menu);

		if (param == 0xFE) return -EIO;
		if (param == 0xFF) return -EINVAL;

	} while ( param == 0x01 );

	pccl_swap_8 (dev,0);

	err = pccl_caetla_command (dev,0x01);
	if (err < 0) return err;
	err = pccl_swap_8 (dev,CAETLA_CURRENT);

	if (err != menu)
	{	// one unchecked retry, I don't know _exactly_ why it works then.
		err = pccl_caetla_command (dev,0x01);
		pccl_swap_8 (dev,menu);
		pccl_swap_8 (dev,0);
	}

	return err;
}

/*
	a very expensive function, which checks the versions of all subsystems.
	this is not the ioctl() wrapper (with user-mode data transfer), this
	thing is also used in the procfs file "version".
*/

static int caetla_version (
	struct pccl_device *dev,
	caetla_version_t *ver)
{
	int err;

	err = caeioc_control (dev,CAETLA_CDM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x10);
	if (err < 0) return err;

	ver->cdm = pccl_swap_16 (dev,0);
	pccl_swap_8 (dev,0);

	err = caeioc_control (dev,CAETLA_FBV);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x10);
	if (err < 0) return err;

	ver->fbv = pccl_swap_16 (dev,0);
	pccl_swap_8 (dev,0);

	err = caeioc_control (dev,CAETLA_CFG);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x10);
	if (err < 0) return err;
	
	ver->cfg = pccl_swap_16 (dev,0);
	pccl_swap_8 (dev,0);

	err = caeioc_control (dev,CAETLA_CS);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x10);
	if (err < 0) return err;
	
	ver->cs = pccl_swap_16 (dev,0);
	pccl_swap_8 (dev,0);

	err = caeioc_control (dev,CAETLA_MCM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x10);
	if (err < 0) return err;

	ver->mcm = pccl_swap_16 (dev,0);
	pccl_swap_8 (dev,0);

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x10);
	if (err < 0) return err;

	ver->du = pccl_swap_16 (dev,0);
	pccl_swap_8 (dev,0);

	err = caeioc_control (dev,CAETLA_MM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x0f);
	if (err < 0) return err;
	
	ver->firmware = pccl_swap_16 (dev,0);
	pccl_swap_8 (dev,0);

	err = caeioc_control (dev,CAETLA_MM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x10);
	if (err < 0) return err;
	
	ver->mm = pccl_swap_16 (dev,0);
	pccl_swap_8 (dev,0);

	if (PCCL_CHECK(dev,AUTO_RESUME))
	{
		err = pccl_caetla_command (dev,0x2);
		if (err < 0) return err;
	}

	return 0;
}

/**************************************************************/
/*** memory card ***/
/**************************************************************/

/* mc_directory: get directory of the specified memory card. */

static int caeioc_mc_directory (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_mc_directory_t dir;
	int i,j;

	// get parameter
	err = copy_from_user ((void*)&dir,(void*)param,sizeof(dir));
	if (err < 0) return err;

	// check parameter
	if (!((dir.slot == 0) || (dir.slot == 1))) return -EINVAL;

	// get listing
	err = pccl_caetla_command (dev,0x17);
	if (err < 0) return err;

	pccl_swap_8 (dev,dir.slot);
	dir.count = pccl_swap_16 (dev,0);
	
	// we don't want the linux kernel to be killed
	if (dir.count > CAETLA_MC_DIRECTORY_MAX_COUNT) return -EREMOTEIO;
	
	for (i=0; i<dir.count; i++)
	{
		u_char *p = (u_char*) &(dir.file[i]);
		
		for (j=0;j<128;j++,p++) *p = pccl_swap_8(dev,0);
	}
	
	// put result
	return copy_to_user ((void*)param,(void*)&dir,sizeof(dir));
}

/*
	mc_format, unlock, lock, delete and status: have all nearly the same
	calling struct, so this saves some bytes
*/

static int caeioc_mc_multipurpose (
	struct pccl_device *dev,
	unsigned long param,
	u_char command,
	int usefileno)
{
	int err;
	caetla_mc_result_t arg;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	// check parameter
	if (!((arg.slot == 0) || (arg.slot == 1))) return -EINVAL;

	// get listing
	err = pccl_caetla_command (dev,command);
	if (err < 0) return err;

	pccl_swap_8 (dev,arg.slot);
	if (usefileno==1) pccl_swap_8 (dev,arg.fileno);
	arg.result = pccl_swap_16 (dev,0);

	// put result
	return copy_to_user ((void*)param,(void*)&arg,sizeof(arg));
}


/* mc_create: */

static int caeioc_mc_create (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_mc_file_create_t data;

	// get parameter
	err = copy_from_user ((void*)&data,(void*)param,sizeof(data));
	if (err < 0) return err;

	// check parameter
	if (!((data.slot == 0) || (data.slot == 1))) return -EINVAL;

	// read entry
	err = pccl_caetla_command (dev,0x19);
	if (err < 0) return err;

	pccl_swap_8 (dev,data.slot);
	pccl_send_string (dev, data.name);
	pccl_swap_8 (dev,0); // string null byte
	pccl_swap_32 (dev,data.size);
	
	data.blocks = pccl_swap_8 (dev,0);

	PCCL_SET (dev,WRITE_MODE);

	return copy_to_user ((void*)param,(void*)&data,sizeof(data));
}


/* mc_read: */

static int caeioc_mc_read (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_mc_file_read_t data;

	// get parameter
	err = copy_from_user ((void*)&data,(void*)param,sizeof(data));
	if (err < 0) return err;

	// check parameter
	if (!((data.slot == 0) || (data.slot == 1))) return -EINVAL;
	if (data.fileno > CAETLA_MC_DIRECTORY_MAX_COUNT) return -EINVAL;

	// read entry
	err = pccl_caetla_command (dev,0x18);
	if (err < 0) return err;

	pccl_swap_8 (dev,data.slot);
	pccl_swap_8 (dev,data.fileno);
	data.blocks = pccl_swap_8 (dev,0);

	if (data.blocks > CAETLA_MC_DIRECTORY_MAX_COUNT) return -EREMOTEIO;

	// get the bytes with a normal read() call
	PCCL_SET (dev,READ_MODE);

	// put result
	return copy_to_user ((void*)param,(void*)&data,sizeof(data));
}

/**************************************************************/

/* version: get version number from subsystem */

static int caeioc_version (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_version_t ver;

	err = caetla_version (dev,&ver);
	if (err < 0) return err;

	return copy_to_user ((void*)param,(void*)&ver,sizeof(ver));
}

/* status: return current caetla status */

static int caeioc_device_info (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_device_info_t arg;

	arg.checksum = dev->checksum;
	arg.checksum2 = dev->checksum2;

	return copy_to_user ((void*)param,(void*)&arg,sizeof(arg));
}

/* mode: change global device modes */

static int caeioc_device_get_mode (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_device_mode_t arg;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	switch (arg.mode)
	{
		case PCCL_MODE_AUTO_RESUME:
			arg.value = PCCL_CHECK(dev,AUTO_RESUME);
			break;
		default:
			return -EINVAL;
	}

	return copy_to_user ((void*)param,(void*)&arg,sizeof(arg));
}

static int caeioc_device_set_mode (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_device_mode_t arg;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	switch (arg.mode)
	{
		case PCCL_MODE_AUTO_RESUME:
			PCCL_CLEAR(dev,AUTO_RESUME);
			if (arg.value==1) PCCL_SET(dev,AUTO_RESUME);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

/* write_mem: transfer bytes to caetla */

static int caeioc_write_mem (
	struct pccl_device *dev,
	struct file *file,
	unsigned long param)
{
	caetla_memory_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_MM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x14);
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.address);
	pccl_swap_32 (dev,arg.size);

	PCCL_SET (dev,WRITE_MODE);

	return err;
}

/* read_mem: transfer bytes to caetla */

static int caeioc_read_mem (
	struct pccl_device *dev,
	struct file *file,
	unsigned long param)
{
	caetla_memory_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_MM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x15);
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.address);
	pccl_swap_32 (dev,arg.size);

	PCCL_SET (dev,READ_MODE);

	return err;
}

static int caeioc_arguments (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_arguments_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_MM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x1D);
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.address);
	pccl_swap_32 (dev,arg.argv);
	pccl_swap_32 (dev,arg.argc);

	/* RESUME not wanted */

	return err;
}

static int caeioc_console (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;

	err = caeioc_control (dev,CAETLA_MM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x04);
	if (err < 0) return err;

	param = pccl_swap_8 (dev, param);
	pccl_swap_8 (dev,0);

	/* RESUME not wanted */

	return 0;
}

static int caeioc_jump_address (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;

	err = caeioc_control (dev,CAETLA_MM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x17);
	if (err < 0) return err;

	pccl_swap_32 (dev,param);
	pccl_swap_8 (dev,0);

	/* RESUME not wanted */

	return 0;
}

static int caeioc_get_status_byte (
	struct pccl_device *dev,
	unsigned long param,
	unsigned char command)
{
	int err;
	u_char byte;

	err = pccl_caetla_command (dev,command);
	if (err < 0) return err;
	
	byte = pccl_swap_8 (dev,0);

	return copy_to_user ((void*)param,(void*)&byte,sizeof(byte));
}


static int caeioc_execute (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_execute_t arg;
	int err;

	err = copy_from_user ((void*)&arg,(void*)param,sizeof(caetla_execute_t));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_MM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x16);	// execute
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.task);
	pccl_swap_32 (dev,arg.event);
	pccl_swap_32 (dev,arg.stack);

	pccl_swap_32 (dev,arg.pc0);
	pccl_swap_32 (dev,arg.gp0);
	pccl_swap_32 (dev,arg.t_addr);
	pccl_swap_32 (dev,arg.t_size);
	pccl_swap_32 (dev,arg.d_addr);
	pccl_swap_32 (dev,arg.d_size);
	pccl_swap_32 (dev,arg.b_addr);
	pccl_swap_32 (dev,arg.b_size);
	pccl_swap_32 (dev,arg.s_addr);
	pccl_swap_32 (dev,arg.s_size);
	pccl_swap_32 (dev,arg.sp);
	pccl_swap_32 (dev,arg.fp);
	pccl_swap_32 (dev,arg.gp);
	pccl_swap_32 (dev,arg.ret);
	pccl_swap_32 (dev,arg.base);

// mm:0x11 send data size, data

	pccl_swap_8 (dev,arg.mode);

	/* RESUME not wanted */

	return 0;
}

/**************************************************************/
/*** debug utility ***/
/**************************************************************/

static int caeioc_flush_icache (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	/* RESUME not wanted */

	return pccl_caetla_command (dev,0x14);
}

static int caeioc_disable_hbp (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	/* RESUME not wanted */

	return pccl_caetla_command (dev,0x16);
}

static int caeioc_configure_hbp (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_hbp_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x15);
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.address);
	pccl_swap_32 (dev,arg.condition);
	pccl_swap_32 (dev,arg.datamask);
	pccl_swap_32 (dev,arg.execmask);

	/* RESUME not wanted */

	return 0;
}

static int caeioc_get_registers (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_registers_t arg;
	int err, i;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x11);
	if (err < 0) return err;

	for (i=0; i<36; i++)
	{
		arg.data[i] = pccl_swap_32 (dev,0);
	}

	/* RESUME not wanted */

	return copy_to_user ((void*)param,(void*)&arg,sizeof(arg));
}

static int caeioc_set_register (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_register_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x12);	// execute
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.number);
	pccl_swap_32 (dev,arg.data);

	/* RESUME not wanted */

	return 0;
}

static int caeioc_write_byte (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_byte_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x19);
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.address);
	pccl_swap_8  (dev,arg.data);

	/* RESUME not wanted */

	return 0;
}

static int caeioc_write_word (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_word_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x1a);
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.address);
	pccl_swap_32 (dev,arg.data);

	/* RESUME not wanted */

	return 0;
}

static int caeioc_read_word (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_word_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x1b);
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.address);
	arg.data = pccl_swap_32 (dev,0);

	/* RESUME not wanted */

	return copy_to_user ((void*)param,(void*)&arg,sizeof(arg));
}

/* TODO :: not yet tested ! */

static int caeioc_du_read_mem (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_memory_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x17);
	if (err < 0) return err;

	pccl_swap_32 (dev,0);	// dummy
	pccl_swap_32 (dev,arg.address);
	pccl_swap_32 (dev,arg.size);

	PCCL_SET (dev,READ_MODE);

	return err;
}

/* no checksum required ? */

static int caeioc_du_write_mem (
	struct pccl_device *dev,
	unsigned long param)
{
	caetla_memory_t arg;
	int err;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = caeioc_control (dev,CAETLA_DU);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x18);
	if (err < 0) return err;

	pccl_swap_32 (dev,arg.address);
	pccl_swap_32 (dev,arg.size);
	pccl_swap_8 (dev,0);			// dummy

	PCCL_SET (dev,WRITE_MODE);

	return err;
}

/*
	code preservation area related. (mm:0x1a and mm:0x1b)
	they just initiate the command and return the "size".
	the rest of data transfer has to be done with read/write on /dev/pccl!
*/

static int caeioc_code_memory (
	struct pccl_device *dev,
	unsigned long param,
	int mode)
{
	int err;
	u_long size;

	err = caeioc_control (dev,CAETLA_MM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,(mode==0) ? 0x1A : 0x1B);
	if (err < 0) return err;

	size = pccl_swap_32 (dev,0);

	if (mode==0)
		PCCL_SET (dev,WRITE_MODE);
	else
		PCCL_SET (dev,READ_MODE);

	return copy_to_user ((void*)param,(void*)&size,sizeof(size));
}

static int caeioc_read_xalist (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	u_long size;

	err = caeioc_control (dev,CAETLA_CDM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x12);
	if (err < 0) return err;

	size = pccl_swap_32 (dev,0);

	PCCL_SET (dev,READ_MODE);

	return copy_to_user ((void*)param,(void*)&size,sizeof(size));
}

static int caeioc_write_xalist (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;

	err = caeioc_control (dev,CAETLA_CDM);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x13);
	if (err < 0) return err;

	PCCL_SET (dev,WRITE_MODE);

	return 0;
}

static int caeioc_fb_read (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_fb_mode_t arg;
	int during_game;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x08);
	if (err < 0) return err;
	during_game = pccl_swap_8(dev,0);

	err = caeioc_control (dev,CAETLA_FBV);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,during_game ? 0x11 : 0x15);
	if (err < 0) return err;

	pccl_swap_16 (dev,arg.x);
	pccl_swap_16 (dev,arg.y);
	pccl_swap_16 (dev,arg.width);
	pccl_swap_16 (dev,arg.height);

	PCCL_SET (dev,READ_MODE);

	return 0;
}

static int caeioc_fb_write (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_fb_mode_t arg;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x08);
	if (err < 0) return err;

	// upload during game attempted
	if (pccl_swap_8(dev,0)==0x04) return -EINVAL;

	err = caeioc_control (dev,CAETLA_FBV);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x12);
	if (err < 0) return err;

	pccl_swap_16 (dev,arg.x);
	pccl_swap_16 (dev,arg.y);
	pccl_swap_16 (dev,arg.width);
	pccl_swap_16 (dev,arg.height);

	PCCL_SET (dev,WRITE_MODE);

	return 0;
}

static int caeioc_fb_set (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_fb_mode_t arg;

	// get parameter
	err = copy_from_user ((void*)&arg,(void*)param,sizeof(arg));
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x08);
	if (err < 0) return err;

	// invalid operation during game attempted
	if (pccl_swap_8(dev,0)==0x04) return -EINVAL;

	err = caeioc_control (dev,CAETLA_FBV);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x17);
	if (err < 0) return err;

	switch (arg.width)
	{
		case 256: arg.width = 0; break;
		case 320: arg.width = 1; break;
		case 384: arg.width = 2; break;
		case 512: arg.width = 3; break;
		case 640: arg.width = 4; break;
		default: arg.width = 0;
	}

	switch (arg.height)
	{
		case 240: arg.height = 0; break;
		case 480: arg.height = 1; break;
		default: arg.height = 0;
	}

	pccl_swap_16 (dev,arg.x);
	pccl_swap_16 (dev,arg.y);
	pccl_swap_16 (dev,arg.width);
	pccl_swap_16 (dev,arg.height);
	pccl_swap_16 (dev,arg.mode & 1);

	/* TODO_RESUME_HERE_? */

	return 0;
}

static int caeioc_fb_get (
	struct pccl_device *dev,
	unsigned long param)
{
	int err;
	caetla_fb_mode_t arg;

	err = pccl_caetla_command (dev,0x08);
	if (err < 0) return err;

	// invalid operation during game attempted
	if (pccl_swap_8(dev,0)==0x04) return -EINVAL;

	err = caeioc_control (dev,CAETLA_FBV);
	if (err < 0) return err;

	err = pccl_caetla_command (dev,0x16);
	if (err < 0) return err;

	arg.x = pccl_swap_16 (dev,0);
	arg.y = pccl_swap_16 (dev,0);
	arg.width = pccl_swap_16 (dev,0);
	arg.height = pccl_swap_16 (dev,0);
	arg.mode = pccl_swap_16 (dev,0);

	// 4/8 bit currently ignored

	pccl_swap_16 (dev,0);
	pccl_swap_16 (dev,0);
	pccl_swap_16 (dev,0);
	pccl_swap_16 (dev,0);
	pccl_swap_16 (dev,0);

	/* TODO_RESUME_HERE_? */

	return copy_to_user ((void*)param,(void*)&arg,sizeof(arg));
}

/************************************************************/

static int pccl_ioctl (
	struct inode *inode,
	struct file *file,
	unsigned int number,
	unsigned long param)
{
	int devnum = MINOR(inode->i_rdev);
	struct pccl_device *dev = &pccl[devnum];
	int err = -ENOIOCTLCMD;	/* linux specific ? */

	dprintk (KERN_DEBUG "pccl: ioctl(%08x)\n",number);

#if defined PARANOIA
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	switch (number)
	{
		/*
		** low level caetla communication (for user mode applications)
		*/
	
		case CAEIOC_COMMAND:
			return pccl_caetla_command (dev,(u_char) param);
		case CAEIOC_COMMAND_RAW:
			return pccl_caetla_command_raw (dev,(u_char) param);
		case CAEIOC_SWAP_8:
			return caeioc_swap_8 (dev, param);
		case CAEIOC_SWAP_16:
			return caeioc_swap_16 (dev, param);
		case CAEIOC_SWAP_32:
			return caeioc_swap_32 (dev, param);

		/* get version number */
		case CAEIOC_VERSION:
			return caeioc_version (dev, param);
		/* return pclink status */
		case CAEIOC_DEVICE_INFO:
			return caeioc_device_info (dev, param);
		/* change device modes */
		case CAEIOC_DEVICE_SET_MODE:
			return caeioc_device_set_mode (dev, param);
		case CAEIOC_DEVICE_GET_MODE:
			return caeioc_device_get_mode (dev, param);
	
		/* nop: do nothing (useful for heartbeats ?)*/
		case CAEIOC_NOP:
			return pccl_caetla_command (dev, 0x00);
		/* get control over caetla and select mode */
		case CAEIOC_CONTROL:
			return caeioc_control (dev, param);
		/* give control back to caetla */
		case CAEIOC_RESUME:
			if (PCCL_CHECK(dev,AUTO_RESUME))
			{
				int err = pccl_caetla_command (dev,0x2);
				if (err < 0) return err;
			}
			return 0;
		/* softreset caetla */
		case CAEIOC_RESET:
			return pccl_caetla_command (dev, 0x03);
		/* enable/disable stdio console */
		case CAEIOC_CONSOLE:
			return caeioc_console (dev, param);

		/* read/write mem block to/from an address */
		case CAEIOC_WRITE_MEMORY:
			return caeioc_write_mem (dev, file, param);
		case CAEIOC_READ_MEMORY:
			return caeioc_read_mem (dev, file, param);
		/* jump directly to address */
		case CAEIOC_JUMP_ADDRESS:
			return caeioc_jump_address (dev, param);
		/* send run environment */
		case CAEIOC_EXECUTE:
			return caeioc_execute (dev, param);
		/* set arguments */
		case CAEIOC_ARGUMENTS:
			return caeioc_arguments (dev, param);

		case CAEIOC_GET_STATUS_PSPAR:
			return caeioc_get_status_byte (dev, param, 0x08);
		case CAEIOC_GET_STATUS_CAETLA:
			return caeioc_get_status_byte (dev, param, 0x09);
		case CAEIOC_GET_EXECUTE_FLAGS:
			return caeioc_get_status_byte (dev, param, 0x0A);

		/* frame buffer */
		case CAEIOC_FB_READ_MEMORY:
			return caeioc_fb_read (dev, param);
		case CAEIOC_FB_WRITE_MEMORY:
			return caeioc_fb_write (dev, param);
		case CAEIOC_FB_SET_MODE:
			return caeioc_fb_set (dev, param);
		case CAEIOC_FB_GET_MODE:
			return caeioc_fb_get (dev, param);

		/* mc: get directory */
		case CAEIOC_MC_DIRECTORY:
			return caeioc_mc_directory (dev, param);
		/* mc: multipurpose */
		case CAEIOC_MC_LOCK:
			return caeioc_mc_multipurpose (dev, param, 0x11,0);
		case CAEIOC_MC_UNLOCK:
			return caeioc_mc_multipurpose (dev, param, 0x12,0);
		case CAEIOC_MC_STATUS:
			return caeioc_mc_multipurpose (dev, param, 0x13,0);
		case CAEIOC_MC_FORMAT:
			return caeioc_mc_multipurpose (dev, param, 0x1B,0);
		case CAEIOC_MC_RESCAN_FILES:
			return caeioc_mc_multipurpose (dev, param, 0x16,0);
		case CAEIOC_MC_DELETE_FILE:
			return caeioc_mc_multipurpose (dev, param, 0x1A,1);
		case CAEIOC_MC_READ_FILE:
			return caeioc_mc_read (dev, param);
		case CAEIOC_MC_CREATE_FILE:
			return caeioc_mc_create (dev, param);

		case CAEIOC_WRITE_XALIST:
			return caeioc_write_xalist (dev, param);
		case CAEIOC_READ_XALIST:
			return caeioc_read_xalist (dev, param);

		/* du: flush icache */
		case CAEIOC_DU_FLUSH_ICACHE:
			return caeioc_flush_icache (dev, param);
		/* du: disable hbp */
		case CAEIOC_DU_DISABLE_HBP:
			return caeioc_disable_hbp (dev, param);
		/* du: configure hbp */
		case CAEIOC_DU_CONFIGURE_HBP:
			return caeioc_configure_hbp (dev, param);
		/* du: get registers */
		case CAEIOC_DU_GET_REGISTERS:
			return caeioc_get_registers (dev, param);
		/* du: set register */
		case CAEIOC_DU_SET_REGISTER:
			return caeioc_set_register (dev, param);
		/* du: write memory */
		case CAEIOC_DU_WRITE_MEMORY:
			return caeioc_du_write_mem (dev, param);
		/* du: read memory */
		case CAEIOC_DU_READ_MEMORY:
			return caeioc_du_read_mem (dev, param);
		/* du: write byte */
		case CAEIOC_DU_WRITE_BYTE:
			return caeioc_write_byte (dev, param);
		/* du: write word */
		case CAEIOC_DU_WRITE_WORD:
			return caeioc_write_word (dev, param);
		/* du: read word */
		case CAEIOC_DU_READ_WORD:
			return caeioc_read_word (dev, param);

		case CAEIOC_WRITE_CODE_MEMORY:
			return caeioc_code_memory (dev, param, 0);
		case CAEIOC_READ_CODE_MEMORY:
			return caeioc_code_memory (dev, param, 1);
	}

	return err;
}

static struct file_operations pccl_fops = {
	NULL,
	pccl_read,
	pccl_write,
	NULL,
	NULL,
	pccl_ioctl,
	NULL,
	pccl_open,
	NULL, /* flush */
	pccl_release,
};

#if defined PCCL_WITH_PROCFS

/*
	all procfs operations may need the device number. since the path
	it coded like
	
	/proc/pccl/xxx/yyy
	
	xxx = device number (0..3)
	yyy = special file
	
	so we simply atoi() the parent filename to get the device number.
*/

static struct pccl_device *get_device_from_parent_of_file (struct file *file)
{
	struct pccl_device *dev = NULL;
	int devnum;

	if (file->f_dentry->d_parent)
	{
		devnum = simple_strtoul (file->f_dentry->d_parent->d_iname,0,0);
		
		if (devnum < PCCL_MAX_BOARDS)
		{
			dev = &pccl[devnum & 3];
		}
	}
	return dev;
}

/* it's necessary to forbid concurrent access */

static int fops_open (struct inode *inode, struct file *file)
{
	return common_open (inode,file,get_device_from_parent_of_file(file));
}

static int fops_close (struct inode *inode, struct file *file)
{
	return common_close (inode,file,get_device_from_parent_of_file(file));
}

static loff_t mem_lseek (struct file *file, loff_t offset, int whence)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	
	dev->offset = offset;

	return offset;
}

static ssize_t config_read (struct file *file, char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err, len;
	char buf[1024];

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if (*offset > 0) return 0;
	if (size == 0) return 0;

	sprintf(buf,
	"Fixed:\n"
	"I/O base = 0x%03x\n"
	"\n"
	"Variable:\n"
	"console = %d\n"
	"width = %d\n"
	"height = %d\n"
	"x = %d\n"
	"y = %d\n"
	"mode = %d\n"
	,dev->base
	,dev->console
	,dev->width
	,dev->height
	,dev->x
	,dev->y
	,dev->mode
	);

	len = strlen(buf);

	if (*offset > 0) return 0;
	if (size == 0) return 0;

	err = copy_to_user ((void*)buffer,(void*)buf,len);
	if (err < 0) return err;

	*offset = len;
	return len;
}

static ssize_t status_read (struct file *file, char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err, len;
	char buf[1024];
	u_char status,par,flags;

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if (*offset > 0) return 0;
	if (size == 0) return 0;

	status = par = flags = 0xff;

	err = pccl_caetla_command (dev,0x08);
	if (err >= 0)
	{
		par = pccl_swap_8(dev,0);

		err = pccl_caetla_command (dev,0x09);
		if (err < 0) return err;
		status = pccl_swap_8(dev,0);

		err = pccl_caetla_command (dev,0x0A);
		if (err < 0) return err;
		flags = pccl_swap_8(dev,0);

		sprintf(buf,
		"Status (PS/PAR) = (0x%02x) :%s\n"
		"Status (CAETLA) = (0x%02x) :%s%s%s%s%s%s%s\n"
		"Execution Flags = (0x%02x) :%s%s%s%s%s\n"
		"\n"
		,par
		,(par == 0x00) ? " CAETLA" : " GAME RUNNING"
		,status
		,(CAESTF_IN_CONTROL & status)   ? " IN_CONTROL" : ""
		,(CAESTF_CONSOLE_MODE & status) ? " CONSOLE_MODE" : ""
		,(CAESTF_SERVER_MODE & status)  ? " SERVER_MODE" : ""
		,(CAESTF_HBP_STOP & status)     ? " HBP_STOP" : ""
		,(CAESTF_BRK_STOP & status)     ? " BRK_STOP" : ""
		,(CAESTF_NOTIFY_HBP & status)   ? " NOTIFY_HBP" : ""
		,(CAESTF_NOTIFY_BRK & status)   ? " NOTIFY_BRK" : ""
		,flags
		,(flags & (1<<0)) ? " EXCEPTIONS":""
		,(flags & (1<<1)) ? " PCDRV:":""
		,(flags & (1<<2)) ? " FILESERVER":""
		,(flags & (1<<3)) ? " STDIO":""
		,(flags & (1<<4)) ? " BREAKPOINTS":""
		);
	}
	else
	{
		strcpy (buf,
		"Error: can't obtain informations!\n"
		"Possible reasons:\n"
		"1) a game is running without PLUS start settings\n"
		"2) the PSX is hung! Please press RESET button on PSX\n"
		);
	}

	len = strlen(buf);

	if (*offset > 0) return 0;
	if (size == 0) return 0;

	err = copy_to_user ((void*)buffer,(void*)buf,len);
	if (err < 0) return err;

	*offset = len;
	return len;
}

static ssize_t config_write (struct file *file, const char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err;
	char buf[512];

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if (size >= 512) return -EINVAL;

	err = copy_from_user ((void*)buf,(void*)buffer,size);
	if (err < 0) return err;
	
	if (strstr(buf,"mode")) dev->mode = simple_strtoul(strstr(buf,"=")+1,0,0);
	if (strstr(buf,"width")) dev->width = simple_strtoul(strstr(buf,"=")+1,0,0);
	if (strstr(buf,"height")) dev->height = simple_strtoul(strstr(buf,"=")+1,0,0);
	if (strstr(buf,"x")) dev->x = simple_strtoul(strstr(buf,"=")+1,0,0);
	if (strstr(buf,"y")) dev->y = simple_strtoul(strstr(buf,"=")+1,0,0);
	if (strstr(buf,"console")) dev->console = simple_strtoul(strstr(buf,"=")+1,0,0);

	return -1;
}

#define VER_REV(x) ((x)>>8),((x)&0xff)

static ssize_t version_read (struct file *file, char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err, len;
	char buf[256];
	caetla_version_t ver;

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if (*offset > 0) return 0;
	if (size == 0) return 0;

	err = caetla_version (dev,&ver);
	if (err < 0) return err;

	sprintf(buf,
	"caetla firmware     : %2x.%x\n"
	"CD-ROM manager      : %2x.%x\n"
	"memory card manager : %2x.%x\n"
	"frame buffer viewer : %2x.%x\n"
	"cheat code selector : %2x.%x\n"
	"debug utility       : %2x.%x\n"
	"configuration menu  : %2x.%x\n"
	"main menu           : %2x.%x\n"
	,VER_REV(ver.firmware)
	,VER_REV(ver.cdm)
	,VER_REV(ver.mcm)
	,VER_REV(ver.fbv)
	,VER_REV(ver.cs)
	,VER_REV(ver.du)
	,VER_REV(ver.cfg)
	,VER_REV(ver.mm)
	);
	
	len = strlen(buf);

	err = copy_to_user ((void*)buffer,(void*)buf,len);
	if (err < 0) return err;

	*offset = len;

	return len;
}

static ssize_t mem_read (struct file *file, char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err, i;
	u_short checksum;

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if (*offset >= PCCL_UPPER_ADDRESS) return 0;

	if ((*offset + size) >= PCCL_UPPER_ADDRESS)
	{
		size = PCCL_UPPER_ADDRESS - *offset;
	}

	if (size == 0) return 0;

	err = pccl_caetla_command (dev,0x08);
	if (err < 0) return err;
	if (pccl_swap_8(dev,0) != 4)
	{
		err = caeioc_control (dev,CAETLA_MM);
		if (err < 0) return err;

		err = pccl_caetla_command (dev,0x15);
		if (err < 0) return err;

		pccl_swap_32 (dev,dev->offset + PCCL_BASE_ADDRESS);
		pccl_swap_32 (dev,size);

		checksum = 0;
		for (i=0;i<size;i++)
		{
			u_char x;

			x = pccl_swap_8 (dev,0);
			checksum += x;

			if (sigismember(&current->signal, SIGINT)) return -EINTR;

			err = copy_to_user ((void*)buffer+i,(void*)&x,sizeof(u_char));
			if (err < 0) return err;
		}

		dev->offset += size;
		*offset = dev->offset;

		(void) pccl_swap_8 (dev,checksum);	// checksum
	} else {
		err = caeioc_control (dev,CAETLA_DU);
		if (err < 0) return err;

		err = pccl_caetla_command (dev,0x17);
		if (err < 0) return err;

		pccl_swap_32 (dev,0);	// dummy read

		pccl_swap_32 (dev,dev->offset + PCCL_BASE_ADDRESS);
		pccl_swap_32 (dev,size);

		checksum = 0;
		for (i=0;i<size;i++)
		{
			u_char x;

			x = pccl_swap_8 (dev,0);
			checksum += x;

			if (sigismember(&current->signal, SIGINT)) return -EINTR;

			err = copy_to_user ((void*)buffer+i,(void*)&x,sizeof(u_char));
			if (err < 0) return err;
		}

		dev->offset += size;
		*offset = dev->offset;

		(void) pccl_swap_8 (dev,checksum);	// checksum
		(void) pccl_swap_32 (dev,0);		// dummy read
		(void) pccl_swap_32 (dev,0);		// dummy read
		(void) pccl_swap_8 (dev,0);			// check O
		(void) pccl_swap_8 (dev,0);			// check K
	}

	if (PCCL_CHECK(dev,AUTO_RESUME))
	{
		err = pccl_caetla_command (dev,0x2);
		if (err < 0) return err;
	}

	return size;
}

static ssize_t mem_write (struct file *file, const char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err, i;
	u_char checksum;

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if (*offset >= PCCL_UPPER_ADDRESS) return 0;

	if ((*offset + size) >= PCCL_UPPER_ADDRESS)
	{
		size = PCCL_UPPER_ADDRESS - *offset;
	}

	if (size == 0) return 0;

	err = pccl_caetla_command (dev,0x08);
	if (err < 0) return err;
	if (pccl_swap_8(dev,0) != 0x04)
	{
		err = caeioc_control (dev,CAETLA_MM);
		if (err < 0) return err;

		err = pccl_caetla_command (dev,0x14);
		if (err < 0) return err;

		pccl_swap_32 (dev,dev->offset + PCCL_BASE_ADDRESS);
		pccl_swap_32 (dev,size);

		checksum = 0;
		for (i=0;i<size;i++)
		{
			u_char x;

			if (sigismember(&current->signal, SIGINT)) return -EINTR;

			err = copy_from_user ((void*)&x,(void*)buffer+i,sizeof(u_char));
			if (err < 0) return err;
			(void) pccl_swap_8 (dev,x);
			checksum += x;
		}

		dev->offset += size;
		*offset = dev->offset;

		(void) pccl_swap_8 (dev,checksum);	// checksum
	}
	else
	{
		err = caeioc_control (dev,CAETLA_DU);
		if (err < 0) return err;

		err = pccl_caetla_command (dev,0x18);
		if (err < 0) return err;

		pccl_swap_32 (dev,dev->offset + PCCL_BASE_ADDRESS);
		pccl_swap_32 (dev,size);
		pccl_swap_8 (dev,0);			// dummy

		for (i=0;i<size;i++)
		{
			u_char x;

			if (sigismember(&current->signal, SIGINT)) return -EINTR;

			err = copy_from_user ((void*)&x,(void*)buffer+i,sizeof(u_char));
			if (err < 0) return err;
			(void) pccl_swap_8 (dev,x);
		}

		dev->offset += size;
		*offset = dev->offset;
	}

	if (PCCL_CHECK(dev,AUTO_RESUME))
	{
		err = pccl_caetla_command (dev,0x2);
		if (err < 0) return err;
	}

	return size;
}

/*************************************************************************/

static loff_t fullmem_lseek (struct file *file, loff_t offset, int whence)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	
//	printk ("offset = %08lx, %08lx\n",offset,dev->offset2);

	dev->offset2 = offset;

	return offset;
}

/*************************************************************************/

/*
	the following function will clip non existing addresses

RAM
A	0x00000000 - 0x001FFFFF effective
B	0x80000000 - 0x801FFFFF effective
C	0xA0000000 - 0x801FFFFF not effective

Scratchpad (4KB)
S	0x1F800000 - 0x1F8003FF not effective

Device
X	0x1F801000 - 0x1FBFFFFF not effective

ROM
P	0x1FC00000 - 0x1FCFFFFF not effective
Q	0x9FC00000 - 0x9FC7FFFF effective
R	0xBFC00000 - 0xBFC7FFFF not effective

EEPROM
E	0x1F000000 - 0x1F01FFFF not effective

	----....----....----...
	    A__B



	if (lower in A) and (upper in A) -> ok
	if (lower in A) and (upper not in A) -> clip upper=hi(A)
	if (lower not in A) and (upper in A) -> clip lower=lo(A)
	if not (lower in A and upper in A) -> clip lower=lo(A),upper=hi(A)

*/

#define IN_A(x) ( ((x)>=0x00000000) && ((x)<=0x00200000) )
#define IN_B(x) ( ((x)>=0x80000000) && ((x)<=0x80200000) )
#define IN_C(x) ( ((x)>=0xA0000000) && ((x)<=0xA0200000) )
#define IN_S(x) ( ((x)>=0x1F800000) && ((x)<=0x1F800400) )
#define IN_X(x) ( ((x)>=0x1F801000) && ((x)<=0x1FC00000) )
#define IN_P(x) ( ((x)>=0x1FC00000) && ((x)<=0x1FD00000) )
#define IN_Q(x) ( ((x)>=0x9FC00000) && ((x)<=0x9FC80000) )
#define IN_R(x) ( ((x)>=0xBFC00000) && ((x)<=0xBFC80000) )
#define IN_E(x) ( ((x)>=0x1F000000) && ((x)<=0x1F020000) )

#define LOWER_A 0x00000000
#define UPPER_A 0x00200000

#define LOWER_B 0x80000000
#define UPPER_B 0x80200000

#define LOWER_C 0xA0000000
#define UPPER_C 0xA0200000

#define LOWER_S 0x1F800000
#define UPPER_S 0x1F800400

#define LOWER_X 0x1F801000
#define UPPER_X 0x1FC00000

#define LOWER_P 0x1FC00000
#define UPPER_P 0x1FD00000

#define LOWER_Q 0x9FC00000
#define UPPER_Q 0x9FC80000

#define LOWER_R 0xBFC00000
#define UPPER_R 0xBFC80000

#define LOWER_E 0x1F000000
#define UPPER_E 0x1F020000

#define IS_IN_RANGE(x,lo,hi) (((x)>=(lo))&&((x)<=(hi)))

static int
clip_lower_and_upper (u_long LOWER, u_long UPPER, u_long *lower, u_long *upper)
{
	if (IS_IN_RANGE(*lower,LOWER,UPPER))
	{
//		printk ("lower=%p in %p..%p\n",*lower,LOWER,UPPER);

		if (*upper>UPPER)
		{
			*upper = UPPER;
//			printk ("clipped upper to %p\n",*upper);
		}
		
		return 1;
	}

	if (IS_IN_RANGE(*upper,LOWER,UPPER))
	{
//		printk ("upper=%p in %p..%p\n",*upper,LOWER,UPPER);

		if (*lower<LOWER)
		{
			*lower = LOWER;
//			printk ("clipped lower to %p\n",*lower);
		}
		
		return 1;
	}

	return 0;
}

static int
clip (u_long *lower, u_long *upper)
{
	if (clip_lower_and_upper(LOWER_A,UPPER_A,lower,upper)) return 1;
	if (clip_lower_and_upper(LOWER_B,UPPER_B,lower,upper)) return 1;
	if (clip_lower_and_upper(LOWER_C,UPPER_C,lower,upper)) return 1;
	if (clip_lower_and_upper(LOWER_S,UPPER_S,lower,upper)) return 1;
	if (clip_lower_and_upper(LOWER_X,UPPER_X,lower,upper)) return 1;
	if (clip_lower_and_upper(LOWER_P,UPPER_P,lower,upper)) return 1;
	if (clip_lower_and_upper(LOWER_Q,UPPER_Q,lower,upper)) return 1;
	if (clip_lower_and_upper(LOWER_E,UPPER_E,lower,upper)) return 1;
	if (clip_lower_and_upper(LOWER_R,UPPER_R,lower,upper)) return 1;

	return 0;
}

static ssize_t fullmem_read (struct file *file, char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err, i;
	u_short checksum;
	u_long base, upper, lower, shift;

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if (size == 0)
	{
		if (PCCL_CHECK(dev,AUTO_RESUME))
		{
			err = pccl_caetla_command (dev,0x2);
			if (err < 0) return err;
		}
		return 0;
	}

	base  = dev->offset2;
	lower = base;
	upper = base + size;

	lower = 0;
	upper = size;
	shift = 0;

	memset (buffer,0xff,size);

//	if (clip (&lower,&upper))
	if (lower != upper)
	{
//		shift = lower - base;

//		printk ("%p,%p,%p\n",shift,lower,upper);

		err = pccl_caetla_command (dev,0x08);
		if (err < 0) return err;
		if (pccl_swap_8(dev,0) != 4)
		{
			err = caeioc_control (dev,CAETLA_MM);
			if (err < 0) return err;

			err = pccl_caetla_command (dev,0x15);
			if (err < 0) return err;

			pccl_swap_32 (dev,(base+shift));
			pccl_swap_32 (dev,(upper-lower));

			checksum = 0;
			for (i=0;i<(upper-lower);i++)
			{
				u_char x;

				x = pccl_swap_8 (dev,0);
				checksum += x;

				if (sigismember(&current->signal, SIGINT)) return -EINTR;

				err = copy_to_user ((void*)(buffer+i+shift),(void*)&x,sizeof(u_char));
				if (err < 0) return err;
			}

			dev->offset2 += size;
			*offset = dev->offset2;

			(void) pccl_swap_8 (dev,checksum);	// checksum
		} else {
			err = caeioc_control (dev,CAETLA_DU);
			if (err < 0) return err;

			err = pccl_caetla_command (dev,0x17);
			if (err < 0) return err;

			pccl_swap_32 (dev,0);	// dummy read

			pccl_swap_32 (dev,(base+shift));
			pccl_swap_32 (dev,(upper-lower));

			checksum = 0;
			for (i=0;i<(upper-lower);i++)
			{
				u_char x;

				x = pccl_swap_8 (dev,0);
				checksum += x;

				if (sigismember(&current->signal, SIGINT)) return -EINTR;

				err = copy_to_user ((void*)(buffer+i+shift),(void*)&x,sizeof(u_char));
				if (err < 0) return err;
			}

			dev->offset2 += size;
			*offset = dev->offset2;

			(void) pccl_swap_8 (dev,checksum);	// checksum
			(void) pccl_swap_32 (dev,0);		// dummy read
			(void) pccl_swap_32 (dev,0);		// dummy read
			(void) pccl_swap_8 (dev,0);			// check O
			(void) pccl_swap_8 (dev,0);			// check K
		}
	}

	if (PCCL_CHECK(dev,AUTO_RESUME))
	{
		err = pccl_caetla_command (dev,0x2);
		if (err < 0) return err;
	}

	return size;
}

/*************************************************************************/

static ssize_t fullmem_write (struct file *file, const char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err, i;
	u_char checksum;
	u_long base, upper, lower, shift;

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if (size == 0)
	{
		if (PCCL_CHECK(dev,AUTO_RESUME))
		{
			err = pccl_caetla_command (dev,0x2);
			if (err < 0) return err;
		}
		return 0;
	}

	base  = dev->offset2;
	lower = base;
	upper = base + size;

	if (clip (&lower,&upper))
	if (lower != upper)
	{
		shift = lower - base;

		err = pccl_caetla_command (dev,0x08);
		if (err < 0) return err;
		if (pccl_swap_8(dev,0) != 0x04)
		{
			err = caeioc_control (dev,CAETLA_MM);
			if (err < 0) return err;

			err = pccl_caetla_command (dev,0x14);
			if (err < 0) return err;

			pccl_swap_32 (dev,(base+shift));
			pccl_swap_32 (dev,(upper-lower));

			checksum = 0;
			for (i=0;i<(upper-lower);i++)
			{
				u_char x;

				if (sigismember(&current->signal, SIGINT)) return -EINTR;

				err = copy_from_user ((void*)&x,(void*)buffer+i+shift,sizeof(u_char));
				if (err < 0) return err;
				(void) pccl_swap_8 (dev,x);
				checksum += x;
			}

			dev->offset2 += size;
			*offset = dev->offset2;

			(void) pccl_swap_8 (dev,checksum);	// checksum
		}
		else
		{
			err = caeioc_control (dev,CAETLA_DU);
			if (err < 0) return err;

			err = pccl_caetla_command (dev,0x18);
			if (err < 0) return err;

			pccl_swap_32 (dev,(base+shift));
			pccl_swap_32 (dev,(upper-lower));
			pccl_swap_8 (dev,0);			// dummy

			for (i=0;i<size;i++)
			{
				u_char x;

				if (sigismember(&current->signal, SIGINT)) return -EINTR;

				err = copy_from_user ((void*)&x,(void*)buffer+i+shift,sizeof(u_char));
				if (err < 0) return err;
				(void) pccl_swap_8 (dev,x);
			}

			dev->offset2 += size;
			*offset = dev->offset2;
		}
	}

	if (PCCL_CHECK(dev,AUTO_RESUME))
	{
		err = pccl_caetla_command (dev,0x2);
		if (err < 0) return err;
	}

	return size;
}

static ssize_t vram_read (struct file *file, char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err, i,j=0;
	u_short width, height, x, y, mode;
	u_short cx1, cy1, cx2, cy2, clut;
	size_t sent;

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if (*offset == 0) dev->vram_size = 0;

	if ((dev->vram_size <= 0) && (*offset!=0)) return 0;

	sent = size;

	if (*offset == 0)
	{
		struct {
			u_long magic, mode;
		} header;
		struct {
			u_long size;
			u_short x,y;
			u_short width,height;
		} chunk;

		err = pccl_caetla_command (dev,0x08);
		if (err < 0) return err;

		dev->vram_cmd = (pccl_swap_8(dev,0)==0x04) ? 0x11 : 0x15;

		err = caeioc_control (dev,CAETLA_FBV);
		if (err < 0) return err;

		if (dev->vram_cmd == 0x15)
		{
			// get VRAM display mode
			err = pccl_caetla_command (dev,0x16);
			if (err < 0) return err;

			x = pccl_swap_16 (dev,0);
			y = pccl_swap_16 (dev,0);
			width = pccl_swap_16 (dev,0);
			height = pccl_swap_16 (dev,0);
			mode = pccl_swap_16 (dev,0);
			cx1  = pccl_swap_16 (dev,0);
			cy1  = pccl_swap_16 (dev,0);
			cx2  = pccl_swap_16 (dev,0);
			cy2  = pccl_swap_16 (dev,0);
			clut = pccl_swap_16 (dev,0);
		}
		else
		{
			width = 320;
			height = 240;
			x = 0;
			y = 0;
			mode = 0;
		}

		if (dev->width > 0) width = dev->width;
		if (dev->height > 0) height = dev->height;
		if (dev->x > 0) x = dev->x;
		if (dev->y > 0) y = dev->y;
		if (dev->mode > 0) mode = dev->mode;
		
		switch (mode)
		{
			case 0: mode=2; break;
			case 1: mode=3; break;
			case 2: mode=8; break;
			case 3: mode=9; break;
			default: mode=2; break;
		}
		
		dprintk (KERN_INFO "PIXEL: x=%d y=%d w=%d h=%d m=%d\n",
			x,y,width,height,mode);

		header.magic = 0x00000010;
		header.mode = mode;

		err = copy_to_user ((void*)buffer,(void*)&(header),sizeof(header));
		if (err < 0) return err;
		sent -= sizeof(header);
		*offset += sizeof(header);
		j += sizeof(header);

		if (mode > 3)
		{
			// caetla bug ? protocol does not work as described.
			
			return -EINVAL;
		
			if (mode == 8) { cx2 = 16; cy2 = 1; }
			if (mode == 9) { cx2 = 256; cy2 = 1; }
		
			err = pccl_caetla_command (dev,dev->vram_cmd);
			if (err < 0) return err;

			pccl_swap_16 (dev,cx1);
			pccl_swap_16 (dev,cy1);
			pccl_swap_16 (dev,cx2);
			pccl_swap_16 (dev,cy2);

			chunk.size = (cx2 * cy2 * 2) + sizeof(chunk);
			chunk.x = cx1;
			chunk.y = cy1;
			chunk.width = cx2;
			chunk.height = cy2;

			err = copy_to_user ((void*)buffer+j,(void*)&(chunk),sizeof(chunk));
			if (err < 0) return err;
			sent -= sizeof(chunk);
			*offset += sizeof(chunk);
			j += sizeof(chunk);

			for (i=0;i<(cx2 * cy2 * 2);i+=2,j+=2,sent+=2)
			{
				u_short x, *p = (u_short*) (buffer+j);

				x = __arch__swab16 (pccl_swap_16 (dev,0));

				if (sigismember(&current->signal, SIGINT)) return -EINTR;

				err = copy_to_user ((void*)p,(void*)&x,sizeof(u_short));
				if (err < 0) return err;
			}
		}
		
		if (mode==3) width = (width * 3) / 2;
		
		dev->vram_mode = mode;
		dev->vram_align = 0;

		err = pccl_caetla_command (dev,dev->vram_cmd);
		if (err < 0) return err;

		pccl_swap_16 (dev,x);
		pccl_swap_16 (dev,y);
		pccl_swap_16 (dev,width);
		pccl_swap_16 (dev,height);

		dev->vram_size = (width * height * 2);

		chunk.size = (dev->vram_size) + sizeof(chunk);
		chunk.x = x;
		chunk.y = y;
		chunk.width = width;
		chunk.height = height;
		
		err = copy_to_user ((void*)buffer+j,(void*)&(chunk),sizeof(chunk));
		if (err < 0) return err;
		sent -= sizeof(chunk);
		*offset += sizeof(chunk);
		j += sizeof(chunk);
	}

	if (sent > dev->vram_size) sent = dev->vram_size;

	if (dev->vram_mode==2)
	{
		for (i=0;i<sent;i+=2,j+=2)
		{
			u_short x, *p = (u_short*) (buffer+j);

			x = __arch__swab16 (pccl_swap_16 (dev,0));

			if (sigismember(&current->signal, SIGINT)) return -EINTR;

			err = copy_to_user ((void*)p,(void*)&x,sizeof(u_short));
			if (err < 0) return err;
		}
	}
	else
	{
		/*
			this is somewhat tricky. we need to sort 3-byte RGB triplets,
			but we mostly get "SIZE % 3 != 0". so we cache the head and
			the tail of the stream. 
		*/
	
		if (dev->vram_align)
		{
			err = copy_to_user ((void*)(buffer+j),(void*)dev->vram_waste+dev->vram_align,3-dev->vram_align);
			if (err < 0) return err;

			sent -= 3-dev->vram_align;
			j += 3-dev->vram_align;
		}
		
		dev->vram_align = (sent - ((sent / 3) * 3));
		sent = ((sent / 3) * 3);
	
		for (i=0;i<sent;i+=3,j+=3)
		{
			u_char x[3];

			x[2] = pccl_swap_8 (dev,0);
			x[1] = pccl_swap_8 (dev,0);
			x[0] = pccl_swap_8 (dev,0);

			if (sigismember(&current->signal, SIGINT)) return -EINTR;

			err = copy_to_user ((void*)(buffer+j),(void*)x,3);
			if (err < 0) return err;
		}

		if (dev->vram_align)
		{
			dev->vram_waste[2] = pccl_swap_8 (dev,0);
			dev->vram_waste[1] = pccl_swap_8 (dev,0);
			dev->vram_waste[0] = pccl_swap_8 (dev,0);

			if (sent==0)
			{
				sent = dev->vram_size;
				dev->vram_align = 3;
			}

			err = copy_to_user ((void*)(buffer+j),(void*)dev->vram_waste,dev->vram_align);
			if (err < 0) return err;
		}
	}

	*offset += sent;
	dev->vram_size -= sent;

	if (dev->vram_size <= 0)
	{
		if (PCCL_CHECK(dev,AUTO_RESUME))
		{
			err = pccl_caetla_command (dev,0x2);
			if (err < 0) return err;
		}
		
		return sent;
	}

	return size;
}

static ssize_t vram_write (struct file *file, const char *buffer, size_t size, loff_t *offset)
{
	struct pccl_device *dev = get_device_from_parent_of_file(file);
	int err, i,j=0;
	size_t sent;

#if defined PARANOIA
	if (dev == NULL) return -ENODEV;
	if (!PCCL_CHECK(dev,CONFIGURED)) return -ENXIO;
	if (!PCCL_CHECK(dev,OPEN)) return -EBUSY;
#endif

	if ((dev->vram_size <= 0) && (*offset!=0)) return 0;

	sent = size;

	if (*offset == 0)
	{
		struct {
			u_long magic;
			struct {
				u_long mode : 3;
				u_long clut : 1;
			} flags;
			u_long size;
			u_short x,y;
			u_short width,height;
		} image;
		
		err = copy_from_user ((void*)&(image),(void*)buffer,sizeof(image));
		if (err < 0) return err;

		sent -= sizeof(image);
		*offset += sizeof(image);
		j = sizeof(image);

		err = pccl_caetla_command (dev,0x08);
		if (err < 0) return err;

		// invalid operation during game attempted
		if (pccl_swap_8(dev,0)==0x04) return -EINVAL;

		err = caeioc_control (dev,CAETLA_FBV);
		if (err < 0) return err;
		
		if (image.flags.clut)
		{
			err = pccl_caetla_command (dev,0x12);
			if (err < 0) return err;

			pccl_swap_16 (dev,image.x);
			pccl_swap_16 (dev,image.y);
			pccl_swap_16 (dev,image.width);
			pccl_swap_16 (dev,image.height);

			dprintk (KERN_INFO "CLUT: x=%d y=%d w=%d h=%d\n",
				image.x,image.y,image.width,image.height);
			
			for (i=0;i<image.width*2*image.height;i+=2,j+=2)
			{
				u_short x, *p = (u_short*) (buffer+j);

				err = copy_from_user ((void*)&x,(void*)p,sizeof(u_short));
				if (err < 0) return err;

				if (sigismember(&current->signal, SIGINT)) return -EINTR;

				pccl_swap_16 (dev,__arch__swab16 (x));
				
				sent -= 2;
				*offset += 2;
			}
			
			err = copy_from_user ((void*)&(image),(void*)buffer+j-8,sizeof(image));
			if (err < 0) return err;
			
			j += sizeof(image) - 8;
			sent -= sizeof(image) - 8;
			*offset += sizeof(image) - 8;
		}

		dprintk (KERN_INFO "PIXEL: x=%d y=%d w=%d h=%d\n",
			image.x,image.y,image.width,image.height);

		err = pccl_caetla_command (dev,0x12);
		if (err < 0) return err;

		pccl_swap_16 (dev,image.x);
		pccl_swap_16 (dev,image.y);
		pccl_swap_16 (dev,image.width);
		pccl_swap_16 (dev,image.height);

		dev->vram_size = image.size-12;
	}

	if (sent > dev->vram_size)
	{
		sent = dev->vram_size;
	}
	
	for (i=0;i<sent;i+=2,j+=2)
	{
		u_short x, *p = (u_short*) (buffer+j);

		err = copy_from_user ((void*)&x,(void*)p,sizeof(u_short));
		if (err < 0) return err;

		if (sigismember(&current->signal, SIGINT)) return -EINTR;

		pccl_swap_16 (dev,__arch__swab16 (x));
	}

	*offset += sent;
	dev->vram_size -= sent;

	if (dev->vram_size <= 0)
	{
		if (PCCL_CHECK(dev,AUTO_RESUME))
		{
			err = pccl_caetla_command (dev,0x2);
			if (err < 0) return err;
		}
		
		return sent;
	}

	return size;
}

static struct file_operations fops_fullmem = {
	fullmem_lseek,
	fullmem_read,
	fullmem_write,
	NULL,NULL,NULL,NULL,
	fops_open,
	NULL,
	fops_close,
};

static struct file_operations fops_mem = {
	mem_lseek,
	mem_read,
	mem_write,
	NULL,NULL,NULL,NULL,
	fops_open,
	NULL,
	fops_close,
};

static struct file_operations fops_vram = {
	NULL,
	vram_read,
	vram_write,
	NULL,NULL,NULL,NULL,
	fops_open,
	NULL,
	fops_close,
};

static struct file_operations fops_version = {
	NULL,
	version_read,
	NULL,
	NULL,NULL,NULL,NULL,
	fops_open,
	NULL,
	fops_close,
};

static struct file_operations fops_config = {
	NULL,
	config_read,
	config_write,
	NULL,NULL,NULL,NULL,
	fops_open,
	NULL,
	fops_close,
};

static struct file_operations fops_status = {
	NULL,
	status_read,
	NULL,
	NULL,NULL,NULL,NULL,
	fops_open,
	NULL,
	fops_close,
};

#if LINUX_VERSION_CODE < 0x020331
static struct inode_operations iops_fullmem = { &fops_fullmem, };
static struct inode_operations iops_mem     = { &fops_mem, };
static struct inode_operations iops_vram    = { &fops_vram, };
static struct inode_operations iops_version = { &fops_version, };
static struct inode_operations iops_config  = { &fops_config, };
static struct inode_operations iops_status  = { &fops_status, };
#endif

static struct proc_dir_entry *procfs_root;
static struct proc_dir_entry *procfs_card[4];
static struct proc_dir_entry *procfs_version[4];
static struct proc_dir_entry *procfs_mem[4];
static struct proc_dir_entry *procfs_fullmem[4];
static struct proc_dir_entry *procfs_vram[4];
static struct proc_dir_entry *procfs_config[4];
static struct proc_dir_entry *procfs_status[4];
static const char *card_names[4] = { "0", "1", "2", "3" };

static int pccl_install_procfs (void)
{
	int i;

	procfs_root = create_proc_entry ("pccl",(S_IFDIR | 0555),&proc_root);
	if (!procfs_root) return -ENOMEM;

	for (i=0;i<4;++i)
	{
		procfs_card[i] = create_proc_entry (card_names[i],(S_IFDIR | 0555),procfs_root);
		if (!procfs_card[i]) return -ENOMEM;

		procfs_version[i] = create_proc_entry ("version",0444,procfs_card[i]);
		if (!procfs_version[i]) return -ENOMEM;
#if LINUX_VERSION_CODE >= 0x020331
		procfs_version[i]->proc_fops = &fops_version;
#else
		procfs_version[i]->ops = &iops_version;
#endif
		procfs_mem[i] = create_proc_entry ("mem",0666,procfs_card[i]);
		if (!procfs_mem[i]) return -ENOMEM;
#if LINUX_VERSION_CODE >= 0x020331
		procfs_mem[i]->proc_fops = &fops_mem;
#else
		procfs_mem[i]->ops = &iops_mem;
#endif

		procfs_fullmem[i] = create_proc_entry ("fullmem",0666,procfs_card[i]);
		if (!procfs_fullmem[i]) return -ENOMEM;
#if LINUX_VERSION_CODE >= 0x020331
		procfs_fullmem[i]->proc_fops = &fops_fullmem;
#else
		procfs_fullmem[i]->ops = &iops_fullmem;
#endif

		procfs_vram[i] = create_proc_entry ("vram",0666,procfs_card[i]);
		if (!procfs_vram[i]) return -ENOMEM;
#if LINUX_VERSION_CODE >= 0x020331
		procfs_vram[i]->proc_fops = &fops_vram;
#else
		procfs_vram[i]->ops = &iops_vram;
#endif

		procfs_config[i] = create_proc_entry ("config",0666,procfs_card[i]);
		if (!procfs_config[i]) return -ENOMEM;
#if LINUX_VERSION_CODE >= 0x020331
		procfs_config[i]->proc_fops = &fops_config;
#else
		procfs_config[i]->ops = &iops_config;
#endif

		procfs_status[i] = create_proc_entry ("status",0444,procfs_card[i]);
		if (!procfs_status[i]) return -ENOMEM;
#if LINUX_VERSION_CODE >= 0x020331
		procfs_status[i]->proc_fops = &fops_status;
#else
		procfs_status[i]->ops = &iops_status;
#endif
	}

	return 0;
}

static void pccl_remove_procfs (void)
{
	int i;

	for (i=0;i<4;++i)
	{
		remove_proc_entry ("version",procfs_card[i]);
		remove_proc_entry ("mem",procfs_card[i]);
		remove_proc_entry ("fullmem",procfs_card[i]);
		remove_proc_entry ("vram",procfs_card[i]);
		remove_proc_entry ("config",procfs_card[i]);
		remove_proc_entry ("status",procfs_card[i]);
		remove_proc_entry (card_names[i],procfs_root);
	}

	remove_proc_entry ("pccl",&proc_root);
}
#endif

#ifdef MODULE
int init_module (void)
{
	int err;
	int base;

	printk(KERN_NOTICE PCCL_INIT_MESSAGE);
	
#if defined PCCL_WITH_PROCFS
	err = pccl_install_procfs ();
	if (err < 0) return err;
#endif

	/* register the device in the system */

	err = module_register_chrdev(PCCL_MAJOR, "pccl", &pccl_fops);
	if (err < 0)
	{
		printk(KERN_NOTICE "pccl: error %d (module)\n", -err);
		return err;
	}
	
	pccl_boards = 0;
	
	if (io != NULL)
	{
		char *next = io;
		int i;
		base = 0;
		
		for (i=0;i<PCCL_MAX_BOARDS;i++)
		{
			struct pccl_device *dev = &pccl[i];

			if (next == NULL) break;
			base = simple_strtoul (next, &next, 0); next++;
			
			if (base == 0) break;
			dprintk (KERN_DEBUG "pccl: checking port 0x%03x (forced)\n",base);

			if (check_region(base,3) != 0) continue;

			dprintk (KERN_DEBUG "pccl: requesting region at 0x%03x\n",base);
			request_region (base, 3, "pccl");

			dev->base = base;
			dev->id   = i;
			dev->flags |= PCCL_FLAG_CONFIGURED | PCCL_FLAG_AUTO_RESUME;
			dev->lock = SPIN_LOCK_UNLOCKED;

			pccl_boards += 1;
		}
	}
	else
	{
		/* search and autodetect caetla firmware */

		base = 0x300;
		do {
			struct pccl_device *dev = &pccl[pccl_boards];

			dprintk (KERN_DEBUG "pccl: checking port 0x%03x (auto)\n",base);
			if (check_region(base,3) != 0) continue;

			/* preliminary use this base (unrequested region!) */
			dev->base = base;
			dev->flags = 0;
			dev->lock = SPIN_LOCK_UNLOCKED;

			// we don't need to lock here

			// reset caetla
			err = pccl_caetla_command (dev,0x03);
			if (err < 0) continue;

			err = caeioc_control (dev,CAETLA_MM);
			if (err < 0) continue;

			// give control back to caetla
			err = pccl_caetla_command (dev,0x02);
			if (err < 0) continue;

			dprintk (KERN_DEBUG "pccl: requesting region at 0x%03x\n",base);
			request_region (base, 3, "pccl (auto)");
			dev->flags |= PCCL_FLAG_CONFIGURED | PCCL_FLAG_AUTO_RESUME;

			pccl_boards += 1;

		} while ( (base+=0x10) < 0x340);
	}

	if (pccl_boards == 0)
	{
		printk(KERN_NOTICE "pccl: no pccl board(s) found\n");
		module_unregister_chrdev(PCCL_MAJOR, "pccl");
#if defined PCCL_WITH_PROCFS
		pccl_remove_procfs ();
#endif
		return -ENODEV;
	}

	printk(KERN_NOTICE "pccl: %d pccl board(s) found\n", pccl_boards);

	return 0;
}
#endif

#ifdef MODULE		
void cleanup_module (void)
{
	int i;

	if (MOD_IN_USE) return;

	for (i=0;i<PCCL_MAX_BOARDS;i++)
	{
		if (PCCL_CHECK(&pccl[i],CONFIGURED))
		{
			dprintk(KERN_DEBUG "pccl: releasing region at 0x%03x\n", pccl[i].base);
			release_region (pccl[i].base, 3);
		}
	}

	unregister_chrdev(PCCL_MAJOR, "pccl");
#if defined PCCL_WITH_PROCFS
	pccl_remove_procfs ();
#endif

	dprintk (KERN_DEBUG "pccl: driver unloaded\n");
}
#endif
