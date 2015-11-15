/* $Id: pccl.h,v 1.1.1.1 2001/05/17 11:51:04 dbalster Exp $ */

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

#ifndef __LINUX_PCCL_H
#define __LINUX_PCCL_H

#ifndef __KERNEL__
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ioctl.h>
#else
#include <linux/types.h>
#if LINUX_VERSION_CODE >= 0x020331
#include <linux/spinlock.h>
#else
#include <asm/spinlock.h>
#endif
#include <asm/semaphore.h>

#if LINUX_VERSION_CODE >= 0x020100
#include <linux/poll.h>
#endif

struct pccl_device {
	int base;			/* I/O port base */
	int flags;			/* internal */
	spinlock_t lock;	// exclusive lock
	int id;				// ID

	// used by memory transfers and device_info
	unsigned char checksum;     // 8-bit checksum
	unsigned short checksum2;   // 16-bit checksum

	// here are some configurable runtime variables,
	// they can be get/set with ioctl(CAEIOC_CONFIG) or
	// directly with the /proc/pccl/?/config file

	int console;	// console mode on/off (1/0) for /proc/.../execute
	int stack;		// these three are known from system.cnf
	int task;
	int event;
	int x, y, width, height, mode;	// vram snapshot parameter

	// private
	loff_t offset;
	loff_t offset2;
	size_t vram_size;
	short vram_mode;
	unsigned char vram_waste[3];
	short vram_align;
	unsigned char vram_cmd;
};

#endif

/* some pccl specific constants */

#define PCCL_MAJOR        240   /* I'm still waiting for my number ! */
#define PCCL_MAX_BOARDS   4

/* CAEIOC_DEVICE_MODE bitflags */

#define PCCL_MODE_AUTO_RESUME   (1)

/* CAEIOC_??_MEMORY parameter packet */
typedef struct {
	int mode;       // mode to report
	int value;      // value to set/get
} caetla_device_mode_t;

/* CAEIOC_??_MEMORY parameter packet */
typedef struct {
	u_char checksum;        // last checksum of a memory transfer
	u_short checksum2;      // the same, but in 16-bit
} caetla_device_info_t;

/* CAEIOC_??_MEMORY parameter packet */
typedef struct {
	u_long address;         // target memory base address
	u_long size;            // number of bytes to transfer
} caetla_memory_t;

/* CAEIOC_EXECUTE parameter packet */
typedef struct {
	u_long pc0;              // program counter
	u_long gp0;              // global pointer
	u_long t_addr;           // text segment address and size
	u_long t_size;
	u_long d_addr;           // data segment address and size
	u_long d_size;
	u_long b_addr;           // bss-data segment address and size
	u_long b_size;
	u_long s_addr;           // stack pointer address and size
	u_long s_size;
	u_long sp,fp,gp,ret,base;// saved values (normally zero)

	u_long task;             // max. number of tasks
	u_long event;            // max. number of events
	u_long stack;            // initial stack pointer address
	
#define CAETLA_EXECUTE_HOOKED 0
#define CAETLA_EXECUTE_NORMAL 1
	u_char mode;             // 0x00: hooked start, 0x01: normal start
} caetla_execute_t;

/* CAEIOC_VERSION parameter packet */
typedef struct {
	u_short firmware;
	u_short cdm, mcm, fbv;
	u_short cs, du, cfg, mm;
} caetla_version_t;

/* CAEIOC_ARGUMENTS parameter packet */
typedef struct {
	u_long address;
	u_long argv;
	u_long argc;
} caetla_arguments_t;

/* CAEIOC_CONFIGURE_HBP parameter packet */
#define HBP_COND_NONE      0xe0800000
#define HBP_COND_WRITE     0xea800000
#define HBP_COND_EXECUTE   0xe1800000
#define HBP_COND_BOTH      0xeb800000
typedef struct {
	u_long address;
	u_long condition;
	u_long datamask;
	u_long execmask;
} caetla_hbp_t;

/*
	this is not the register order of the
	CAEIOC_SET/GET_REGISTER(S) ioctl!
	register zero is not saved, so all values are shifted by one
*/
#define MIPSREG_AT	0
#define MIPSREG_V0	1
#define MIPSREG_V1	2
#define MIPSREG_A0	3
#define MIPSREG_A1	4
#define MIPSREG_A2	5
#define MIPSREG_A3	6
#define MIPSREG_T0	7
#define MIPSREG_T1	8
#define MIPSREG_T2	9
#define MIPSREG_T3	10
#define MIPSREG_T4	11
#define MIPSREG_T5	12
#define MIPSREG_T6	13
#define MIPSREG_T7	14
#define MIPSREG_S0	15
#define MIPSREG_S1	16
#define MIPSREG_S2	17
#define MIPSREG_S3	18
#define MIPSREG_S4	19
#define MIPSREG_S5	20
#define MIPSREG_S6	21
#define MIPSREG_S7	22
#define MIPSREG_T8	23
#define MIPSREG_T9	24
#define MIPSREG_K0	25
#define MIPSREG_K1	26
#define MIPSREG_GP	27
#define MIPSREG_SP	28
#define MIPSREG_FP	29
#define MIPSREG_RA	30
#define MIPSREG_EPC	31
#define MIPSREG_HI	32
#define MIPSREG_LO	33
#define MIPSREG_SR	34
#define MIPSREG_CR	35

/* CAEIOC_SET_REGISTER parameter packet */
typedef struct {
	u_long number;              // register number
	u_long data;                // register content
} caetla_register_t;

/* CAEIOC_GET_REGISTERS parameter packet */
typedef struct {
	u_long data[36];            // registers
} caetla_registers_t;

/* CAEIOC_GET_CPCOND parameter packet */
#define CAECOND_COP0 (1l<<0)
#define CAECOND_COP1 (1l<<1)
#define CAECOND_COP2 (1l<<2)
#define CAECOND_COP3 (1l<<3)
typedef u_char caetla_cpcond_t;

/* CAEIOC_STATUS parameter packet */
#define CAESTF_INTERNAL      (1l<<0) // internal use only, always 1
#define CAESTF_IN_CONTROL    (1l<<1) // controlled by 0:own 1:pc
#define CAESTF_CONSOLE_MODE  (1l<<2) // console mode (1=on)
#define CAESTF_SERVER_MODE   (1l<<3) // stdio and file server, (0=not managed)
#define CAESTF_HBP_STOP      (1l<<4) // h.b.p. stop (1=stopped)
#define CAESTF_BRK_STOP      (1l<<5) // break instruction stop (1=stopped)
#define CAESTF_NOTIFY_HBP    (1l<<6) // notify h.b.p. stop (1=yes)
#define CAESTF_NOTIFY_BRK    (1l<<7) // notify brk insn stop (1=yes)
typedef u_char caetla_status_t;

#define PSPAR_IN_MENU		(0x00)
#define PSPAR_IN_GAME_NORMAL	(0x02)
#define PSPAR_IN_GAME_PLUS		(0x04)
typedef u_char pspar_status_t;

/* CAEIOC_??_BYTE parameter packet */
typedef struct {
	u_long address;             // address
	u_char data;                // data
} caetla_byte_t;

/* CAEIOC_??_WORD parameter packet */
typedef struct {
	u_long address;             // address
	u_long data;                // data
} caetla_word_t;

#define MCFILE_MAGIC 0x4353
typedef struct {
	u_short magic;              // "SC"
	u_char type;                // 0x11=1 icon, .. , 0x13=3 icons.
	u_char size;                // number of 8KB blocks
	u_char name[64];            // shifto-jis encoded !
	u_char reserved[28];        // unused ?
	u_short clut[16];           // 16-bit 16 color palette
	u_short icon[3][(16*16)/4]; // 3 icons, 16x16 in 4-bit
} caetla_mc_file_header_t;

/* new: caetla supports memory card executables (mexe) */

#define MEXE_MAGIC 0x4558454d
typedef struct {
	u_long magic;			    // "MEXE"
	u_long reserved;			// ??
	u_long pc0;					// program entry point
	u_long gp0;					// global pointer 
	u_long t_addr;				// text segment, address + size 
	u_long t_size;
	u_long d_addr;				// data segment 
	u_long d_size;
	u_long b_addr;				// bss segment 
	u_long b_size;
	u_long s_addr;				// stack 
	u_long s_size;
} caetla_mc_mexe_header_t;

typedef struct {
	u_short slot;
	u_short fileno;
	u_short blocks;
	u_short checksum;
	u_short result;
} caetla_mc_file_read_t;

#define CAETLA_MC_FILENAME_MAXLEN 20
typedef struct {
	u_short slot;
	u_char name[CAETLA_MC_FILENAME_MAXLEN+1];
	u_long size;
	u_short blocks;
	u_short checksum;
	u_short result;
} caetla_mc_file_create_t;

typedef struct {
	u_short slot;
	u_short sector;
	u_long size;
	u_char *data;
	u_short checksum;
	u_short result;
} caetla_mc_sector_t;

typedef struct {
	u_short p1;
	u_long size;
	u_short p2;
	char name[CAETLA_MC_FILENAME_MAXLEN+1];
	u_char p3;
	u_short p4;
	u_short p5;
	char comment[94];
} caetla_mc_file_info_t;

typedef struct {
	u_short slot;              // slot number (0="slot 1", 1="slot 2")
	u_short result;            //
	u_short fileno;	           // only used by DELETE_FILE
} caetla_mc_result_t;

#define CAETLA_MC_DIRECTORY_MAX_COUNT 15
typedef struct {
	u_short slot;              // slot number (0="slot 1", 1="slot 2")
	u_short count;             // 0 == empty, 15 == max/full
	caetla_mc_file_info_t file[16];
} caetla_mc_directory_t;

typedef struct {
	u_char action;             // ..after flashing: 0=reset ps,  1=return to caetla
	u_long address;            // write address (0x1f000000..)
	u_long size;               // write data size
} caetla_write_eeprom_t;

#define CAETLA_FB_MODE_4BIT  2
#define CAETLA_FB_MODE_8BIT  3
#define CAETLA_FB_MODE_15BIT 0
#define CAETLA_FB_MODE_24BIT 1
typedef struct {
	u_short x,y;
	u_short width, height;
	u_short mode;
} caetla_fb_mode_t;

// used for CAEIOC_CONTROL (of mode) and CAEIOC_VERSION (of subsystem)

#define CAETLA_CURRENT   -1 // return current menu
#define CAETLA_MM         0 // main menu
#define CAETLA_MCM        1 // memory card manager
#define CAETLA_FBV        2 // frame buffer viewer
#define CAETLA_CS         3 // code selector
#define CAETLA_CDM        4 // CD-ROM manager
#define CAETLA_CFG        5 // configuration
#define CAETLA_DU         6 // debug utility

/* CAEIOC_ = caetla ioctl ... */

// 0x00..0x0f : lowlevel operations (always possible)
#define CAEIOC_COMMAND          _IOWR(PCCL_MAJOR, 0x00, u_char)
#define CAEIOC_COMMAND_RAW      _IOWR(PCCL_MAJOR, 0x01, u_char)
#define CAEIOC_SWAP_8           _IOWR(PCCL_MAJOR, 0x02, u_char *)
#define CAEIOC_SWAP_16          _IOWR(PCCL_MAJOR, 0x03, u_short *)
#define CAEIOC_SWAP_32          _IOWR(PCCL_MAJOR, 0x04, u_long *)
#define CAEIOC_DEVICE_INFO       _IOR(PCCL_MAJOR, 0x05, caetla_device_info_t *)
#define CAEIOC_DEVICE_GET_MODE   _IOR(PCCL_MAJOR, 0x06, caetla_device_mode_t *)
#define CAEIOC_DEVICE_SET_MODE   _IOW(PCCL_MAJOR, 0x07, caetla_device_mode_t *)

// 0x10..0x1f : base operations (always possible)
#define CAEIOC_NOP                _IO(PCCL_MAJOR, 0x10)
#define CAEIOC_CONTROL          _IOWR(PCCL_MAJOR, 0x11, u_char)
#define CAEIOC_RESUME             _IO(PCCL_MAJOR, 0x12)
#define CAEIOC_RESET              _IO(PCCL_MAJOR, 0x13)
#define CAEIOC_CONSOLE           _IOW(PCCL_MAJOR, 0x14, u_char)
#define CAEIOC_VERSION           _IOR(PCCL_MAJOR, 0x15, caetla_version_t *)
#define CAEIOC_GET_STATUS_PSPAR  _IOR(PCCL_MAJOR, 0x16, u_char *)
#define CAEIOC_GET_STATUS_CAETLA _IOR(PCCL_MAJOR, 0x17, u_char *)
#define CAEIOC_GET_EXECUTE_FLAGS _IOR(PCCL_MAJOR, 0x18, u_char *)

// 0x20..0x2f : normal communication (CAETLA_MM)
#define CAEIOC_WRITE_MEMORY      _IOW(PCCL_MAJOR, 0x20, caetla_memory_t *)
#define CAEIOC_READ_MEMORY       _IOR(PCCL_MAJOR, 0x21, caetla_memory_t *)
#define CAEIOC_EXECUTE           _IOW(PCCL_MAJOR, 0x22, caetla_execute_t *)
#define CAEIOC_JUMP_ADDRESS      _IOW(PCCL_MAJOR, 0x23, u_long)
#define CAEIOC_ARGUMENTS         _IOW(PCCL_MAJOR, 0x24, caetla_arguments_t *)

// 0x30..0x3f : memory card (CAETLA_MCM)
#define CAEIOC_MC_DIRECTORY      _IOR(PCCL_MAJOR, 0x30, caetla_mc_directory_t*)
#define CAEIOC_MC_READ_SECTOR    _IOR(PCCL_MAJOR, 0x31, caetla_mc_sector_t*)
#define CAEIOC_MC_WRITE_SECTOR   _IOR(PCCL_MAJOR, 0x32, caetla_mc_sector_t*)
#define CAEIOC_MC_FORMAT         _IOR(PCCL_MAJOR, 0x34, caetla_mc_result_t*)
#define CAEIOC_MC_LOCK           _IOR(PCCL_MAJOR, 0x35, caetla_mc_result_t*)
#define CAEIOC_MC_UNLOCK         _IOR(PCCL_MAJOR, 0x36, caetla_mc_result_t*)
#define CAEIOC_MC_STATUS         _IOR(PCCL_MAJOR, 0x37, caetla_mc_result_t*)
#define CAEIOC_MC_READ_FILE      _IOR(PCCL_MAJOR, 0x38, caetla_mc_file_read_t*)
#define CAEIOC_MC_CREATE_FILE    _IOR(PCCL_MAJOR, 0x39, caetla_mc_file_create_t*)
#define CAEIOC_MC_DELETE_FILE    _IOR(PCCL_MAJOR, 0x3a, caetla_mc_result_t*)
#define CAEIOC_MC_RESCAN_FILES   _IOR(PCCL_MAJOR, 0x3b, caetla_mc_result_t*)

// 0x40..0x4f : frame buffer (CAETLA_FBV)
#define CAEIOC_FB_READ_MEMORY    _IOR(PCCL_MAJOR, 0x40, caetla_fb_mode_t *)
#define CAEIOC_FB_WRITE_MEMORY   _IOW(PCCL_MAJOR, 0x41, caetla_fb_mode_t *)
#define CAEIOC_FB_GET_MODE       _IOR(PCCL_MAJOR, 0x42, caetla_fb_mode_t *)
#define CAEIOC_FB_SET_MODE       _IOW(PCCL_MAJOR, 0x43, caetla_fb_mode_t *)

// 0x50..0x5f : debugger commands (CAETLA_DU)
#define CAEIOC_DU_FLUSH_ICACHE    _IO(PCCL_MAJOR, 0x50)
#define CAEIOC_DU_DISABLE_HBP     _IO(PCCL_MAJOR, 0x51)
#define CAEIOC_DU_CONFIGURE_HBP  _IOW(PCCL_MAJOR, 0x52, caetla_hbp_t *)
#define CAEIOC_DU_GET_REGISTERS  _IOR(PCCL_MAJOR, 0x53, caetla_registers_t *)
#define CAEIOC_DU_SET_REGISTER   _IOR(PCCL_MAJOR, 0x54, caetla_register_t *)
#define CAEIOC_DU_GET_CPCOND     _IOR(PCCL_MAJOR, 0x55, caetla_cpcond_t *)
#define CAEIOC_DU_WRITE_MEMORY   _IOW(PCCL_MAJOR, 0x56, caetla_memory_t *)
#define CAEIOC_DU_READ_MEMORY    _IOR(PCCL_MAJOR, 0x57, caetla_memory_t *)
#define CAEIOC_DU_WRITE_BYTE     _IOW(PCCL_MAJOR, 0x58, caetla_byte_t *)
#define CAEIOC_DU_WRITE_WORD     _IOW(PCCL_MAJOR, 0x59, caetla_word_t *)
#define CAEIOC_DU_READ_WORD      _IOR(PCCL_MAJOR, 0x5a, caetla_word_t *)
#define CAEIOC_DU_HBP_DETECTION  _IOW(PCCL_MAJOR, 0x5b, u_char)

// 0x60..0x6f : code preservation area and eep-rom related ((CAETLA_MM)
#define CAEIOC_READ_CODE_MEMORY  _IOR(PCCL_MAJOR, 0x60, u_long *)
#define CAEIOC_WRITE_CODE_MEMORY _IOW(PCCL_MAJOR, 0x61, u_long *)
#define CAEIOC_READ_XALIST       _IOR(PCCL_MAJOR, 0x62, u_long *)
#define CAEIOC_WRITE_XALIST       _IO(PCCL_MAJOR, 0x63)
#define CAEIOC_WRITE_EEPROM      _IOW(PCCL_MAJOR, 0x64, caetla_eeprom_t *)

#endif
