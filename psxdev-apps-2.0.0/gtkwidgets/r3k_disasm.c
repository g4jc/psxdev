/* $Id: r3k_disasm.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 *	R3K/GTE disassembler
 *	
 *	Copyright (C) 1998,1999,2000 Daniel Balster (dbalster@psxdev.de)
 *	http://www.psxdev.de
 *	
 *	This is not free software.
 */

#include <r3k_disasm.h>

// sprintf
#include <stdio.h>

static void decode_simple (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_baseofs (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_regimm (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_reg_st (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_target (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_reg_dst (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_reg_d (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_reg_s (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_jreg_s (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_reg_ds (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_reg_tri (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_reg_sh (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_reg_dts (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_regtgt2 (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_regtgt (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_target2 (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_regc0 (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_gtectrl (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_gtedata (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_gteofs (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_syscall (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_regcx (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_c0ofs (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);
static void decode_cxofs (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);

#define FMT_SIMPLE	"%s"
#define FMT_BASEOFS	"%s $%s,0x%x($%s)"
#define FMT_REGIMM  "%s $%s,0x%04x"
#define FMT_REGIMM2	"%s $%s,$%s,0x%04x"
#define FMT_ONEREG  "%s $%s"
#define FMT_TWOREG  "%s $%s,$%s"
#define FMT_TRIREG  "%s $%s,$%s,$%s"
#define FMT_TARGET  "%s 0x%08lx"
#define FMT_REGTGT  "%s $%s,0x%08lx"
#define FMT_REGTGT2	"%s $%s,$%s,0x%08lx"
#define FMT_SYSCALL	"%s 0x%lx"

/* static tables taken and modified from nagra's n64psx disassembler */

static opcode_t insn_table[] = {
	{ "nop",	FMT_SIMPLE, decode_simple,		0x00000000, 0xffffffff,0, 0 },
	{ "lb",		FMT_BASEOFS, decode_baseofs,	0x80000000, 0xfc000000,0, 0 },
	{ "lbu",	FMT_BASEOFS, decode_baseofs,	0x90000000, 0xfc000000,0, 0 },
	{ "lh",		FMT_BASEOFS, decode_baseofs,	0x84000000, 0xfc000000,0, 0 },
	{ "lhu",	FMT_BASEOFS, decode_baseofs,	0x94000000, 0xfc000000,0, 0 },
	{ "lw",		FMT_BASEOFS, decode_baseofs,	0x8c000000, 0xfc000000,0, 0 },
	{ "lwl",	FMT_BASEOFS, decode_baseofs,	0x88000000, 0xfc000000,0, 0 },
	{ "lwr",	FMT_BASEOFS, decode_baseofs,	0x98000000, 0xfc000000,0, 0 },
	{ "lwu",	FMT_BASEOFS, decode_baseofs,	0x9c000000, 0xfc000000,0, 0 },
	{ "sb",		FMT_BASEOFS, decode_baseofs,	0xa0000000, 0xfc000000,0, 0 },
	{ "sc",		FMT_BASEOFS, decode_baseofs,	0xe0000000, 0xfc000000,0, 0 },
	{ "scd",	FMT_BASEOFS, decode_baseofs,	0xf0000000, 0xfc000000,0, 0 },
	{ "sh",		FMT_BASEOFS, decode_baseofs,	0xa4000000, 0xfc000000,0, 0 },
	{ "sw",		FMT_BASEOFS, decode_baseofs,	0xac000000, 0xfc000000,0, 0 },
	{ "swl",	FMT_BASEOFS, decode_baseofs,	0xa8000000, 0xfc000000,0, 0 },
	{ "swr",	FMT_BASEOFS, decode_baseofs,	0xb8000000, 0xfc000000,0, 0 },
	{ "lui",	FMT_REGIMM, decode_regimm,		0x3c000000, 0xffe00000,0, 0 },
	{ "li",		FMT_REGIMM, decode_regimm,		0x24000000, 0xffe00000,0, 0 },
	{ "li",		FMT_REGIMM, decode_regimm,		0x34000000, 0xffe00000,0, 0 },
	{ "neg",	FMT_TWOREG, decode_reg_ds,		0x00000022, 0xffe007ff,0, 0 },
	{ "negu",	FMT_TWOREG, decode_reg_ds,		0x00000023, 0xffe007ff,0, 0 },
	{ "not",	FMT_TWOREG, decode_reg_ds,		0x00000027, 0xfc0007ff,0, 0 },
	{ "move",	FMT_TWOREG, decode_reg_ds,		0x00000025, 0xfc1f07ff,0, 0 },
	{ "move",	FMT_TWOREG, decode_reg_ds,		0x00000021, 0xfc1f07ff,0, 0 },
	{ "addi",	FMT_REGIMM2, decode_reg_tri,	0x20000000, 0xfc000000,0, 0 },
	{ "addiu",	FMT_REGIMM2, decode_reg_tri,	0x24000000, 0xfc000000,0, 0 },
	{ "slti",	FMT_REGIMM2, decode_reg_tri,	0x28000000, 0xfc000000,0, 0 },
	{ "sltiu",	FMT_REGIMM2, decode_reg_tri,	0x2c000000, 0xfc000000,0, 0 },
	{ "andi",	FMT_REGIMM2, decode_reg_tri,	0x30000000, 0xfc000000,0, 0 },
	{ "ori",	FMT_REGIMM2, decode_reg_tri,	0x34000000, 0xfc000000,0, 0 },
	{ "xori",	FMT_REGIMM2, decode_reg_tri,	0x38000000, 0xfc000000,0, 0 },
	{ "sll",	FMT_REGIMM2, decode_reg_sh,		0x00000000, 0xffe0003f,0, 0 },
	{ "sra",	FMT_REGIMM2, decode_reg_sh,		0x00000003, 0xffe0003f,0, 0 },
	{ "srl",	FMT_REGIMM2, decode_reg_sh,		0x00000002, 0xffe0003f,0, 0 },
	{ "sllv",	FMT_TRIREG, decode_reg_dts,		0x00000004, 0xfc0007ff,0, 0 },
	{ "srav",	FMT_TRIREG, decode_reg_dts,		0x00000007, 0xfc0007ff,0, 0 },
	{ "srlv",	FMT_TRIREG, decode_reg_dts,		0x00000006, 0xfc0007ff,0, 0 },
	{ "mult",	FMT_TWOREG, decode_reg_st,		0x00000018, 0xfc00ffff,0, 0 },
	{ "multu",	FMT_TWOREG, decode_reg_st,		0x00000019, 0xfc00ffff,0, 0 },
	{ "div",	FMT_TWOREG, decode_reg_st,		0x0000001a, 0xfc00ffff,0, 0 },
	{ "divu",	FMT_TWOREG, decode_reg_st,		0x0000001b, 0xfc00ffff,0, 0 },
	{ "rem",	FMT_TWOREG, decode_reg_st,		0x0000001a, 0xfc00ffff,0, 0 },
	{ "remu",	FMT_TWOREG, decode_reg_st,		0x0000001b, 0xfc00ffff,0, 0 },
	{ "add",	FMT_TRIREG, decode_reg_dst,		0x00000020, 0xfc0007ff,0, 0 },
	{ "addu",	FMT_TRIREG, decode_reg_dst,		0x00000021, 0xfc0007ff,0, 0 },
	{ "and",	FMT_TRIREG, decode_reg_dst,		0x00000024, 0xfc0007ff,0, 0 },
	{ "nor",	FMT_TRIREG, decode_reg_dst,		0x00000027, 0xfc0007ff,0, 0 },
	{ "or",		FMT_TRIREG, decode_reg_dst,		0x00000025, 0xfc0007ff,0, 0 },
	{ "slt",	FMT_TRIREG, decode_reg_dst,		0x0000002a, 0xfc0007ff,0, 0 },
	{ "sltu",	FMT_TRIREG, decode_reg_dst,		0x0000002b, 0xfc0007ff,0, 0 },
	{ "sub",	FMT_TRIREG, decode_reg_dst,		0x00000022, 0xfc0007ff,0, 0 },
	{ "subu",	FMT_TRIREG, decode_reg_dst,		0x00000023, 0xfc0007ff,0, 0 },
	{ "xor",	FMT_TRIREG, decode_reg_dst,		0x00000026, 0xfc0007ff,0, 0 },
	{ "b",		FMT_TARGET, decode_target2,		0x10000000, 0xffff0000,1, 0 },
	{ "b",		FMT_TARGET, decode_target2,		0x04010000, 0xffff0000,1, 0 },
	{ "bal",	FMT_TARGET, decode_target2,		0x04110000, 0xffff0000,1, 0 },
	{ "beqz",	FMT_REGTGT, decode_regtgt,		0x10000000, 0xfc1f0000,1, 0 },
	{ "bgez",	FMT_REGTGT, decode_regtgt,		0x04010000, 0xfc1f0000,1, 0 },
	{ "bgezal",	FMT_REGTGT, decode_regtgt,		0x04110000, 0xfc1f0000,1, 0 },
	{ "bgtz",	FMT_REGTGT, decode_regtgt,		0x1c000000, 0xfc1f0000,1, 0 },
	{ "blez",	FMT_REGTGT, decode_regtgt,		0x18000000, 0xfc1f0000,1, 0 },
	{ "bltz",	FMT_REGTGT, decode_regtgt,		0x04000000, 0xfc1f0000,1, 0 },
	{ "bltzal",	FMT_REGTGT, decode_regtgt,		0x04100000, 0xfc1f0000,1, 0 },
	{ "bnez",	FMT_REGTGT, decode_regtgt,		0x14000000, 0xfc1f0000,1, 0 },
	{ "beq",	FMT_REGTGT2, decode_regtgt2,	0x10000000, 0xfc000000,1, 0 },
	{ "bne",	FMT_REGTGT2, decode_regtgt2,	0x14000000, 0xfc000000,1, 0 },
	{ "jr",		FMT_ONEREG, decode_jreg_s,		0x00000008, 0xfc1fffff,1, 0 },
	{ "j",		FMT_ONEREG, decode_jreg_s,		0x00000008, 0xfc1fffff,1, 0 },
	{ "j",		FMT_TARGET, decode_target,		0x08000000, 0xfc000000,1, 0 },
	{ "jal",	FMT_TARGET, decode_target,		0x0c000000, 0xfc000000,1, 0 },
	{ "jalr",	FMT_ONEREG, decode_jreg_s,		0x0000f809, 0xfc1fffff,1, 0 },
	{ "jalr",	FMT_TWOREG, decode_reg_ds,		0x00000009, 0xfc1f07ff,1, 0 },
	{ "mfhi",	FMT_ONEREG, decode_reg_d,		0x00000010, 0xffff07ff,0, 0 },
	{ "mflo",	FMT_ONEREG, decode_reg_d,		0x00000012, 0xffff07ff,0, 0 },
	{ "mthi",	FMT_ONEREG, decode_reg_s,		0x00000011, 0xfc1fffff,0, 0 },
	{ "mtlo",	FMT_ONEREG, decode_reg_s,		0x00000013, 0xfc1fffff,0, 0 },
	{ "swc0",	FMT_BASEOFS, decode_c0ofs,		0xe0000000, 0xfc000000,0, 0 },
	{ "swc1",	FMT_BASEOFS, decode_cxofs,		0xe4000000, 0xfc000000,0, 1 },
	{ "swc2",	FMT_BASEOFS, decode_gteofs,		0xe8000000, 0xfc000000,0, 2 },
	{ "swc3",	FMT_BASEOFS, decode_cxofs,		0xec000000, 0xfc000000,0, 3 },
	{ "lwc0",	FMT_BASEOFS, decode_c0ofs,		0xc0000000, 0xfc000000,0, 0 },
	{ "lwc1",	FMT_BASEOFS, decode_cxofs,		0xc4000000, 0xfc000000,0, 1 },
	{ "lwc2",	FMT_BASEOFS, decode_gteofs,		0xc8000000, 0xfc000000,0, 2 },
	{ "lwc3",	FMT_BASEOFS, decode_cxofs,		0xcc000000, 0xfc000000,0, 3 },
	{ "cfc0",	FMT_TWOREG, decode_regc0,		0x40400000, 0xffe007ff,0, 0 },
	{ "cfc1",	FMT_TWOREG, decode_regc0,		0x44400000, 0xffe007ff,0, 1 },
	{ "cfc2",	FMT_TWOREG, decode_gtectrl,		0x48400000, 0xffe007ff,0, 2 },
	{ "cfc3",	FMT_TWOREG, decode_regc0,		0x4c400000, 0xffe007ff,0, 3 },
	{ "ctc0",	FMT_TWOREG, decode_regc0,		0x40c00000, 0xffe007ff,0, 0 },
	{ "ctc1",	FMT_TWOREG, decode_regcx,		0x44c00000, 0xffe007ff,0, 1 },
	{ "ctc2",	FMT_TWOREG, decode_gtectrl,		0x48c00000, 0xffe007ff,0, 2 },
	{ "ctc3",	FMT_TWOREG, decode_regcx,		0x4cc00000, 0xffe007ff,0, 3 },
	{ "mfc0",	FMT_TWOREG, decode_regc0,		0x40000000, 0xffe007ff,0, 0 },
	{ "mfc1",	FMT_TWOREG, decode_regcx,		0x44000000, 0xffe007ff,0, 1 },
	{ "mfc2",	FMT_TWOREG, decode_gtedata,		0x48000000, 0xffe007ff,0, 2 },
	{ "mfc3",	FMT_TWOREG, decode_regcx,		0x4c000000, 0xffe007ff,0, 3 },
	{ "mtc0",	FMT_TWOREG, decode_regc0,		0x40800000, 0xffe007ff,0, 0 },
	{ "mtc1",	FMT_TWOREG, decode_regcx,		0x44800000, 0xffe007ff,0, 1 },
	{ "mtc2",	FMT_TWOREG, decode_gtedata,		0x48800000, 0xffe007ff,0, 2 },
	{ "mtc3",	FMT_TWOREG, decode_regcx,		0x4c800000, 0xffe007ff,0, 3 },
	{ "break",	FMT_SYSCALL, decode_syscall,	0x0000000d, 0xfc00003f,0, 0 },
	{ "syscall",FMT_SYSCALL, decode_syscall,	0x0000000c, 0xfc00003f,0, 0 },
	{ "rtps",	FMT_SIMPLE,	decode_simple,		0x4a180001, 0xffffffff,0, 2 },
	{ "rtpt",	FMT_SIMPLE, decode_simple,		0x4a280030, 0xffffffff,0, 2 },
	{ "rt",		FMT_SIMPLE, decode_simple,		0x4a480012, 0xffffffff,0, 2 },
	{ "rtv0",	FMT_SIMPLE, decode_simple,		0x4a486012, 0xffffffff,0, 2 },
	{ "rtv1",	FMT_SIMPLE, decode_simple,		0x4a48e012, 0xffffffff,0, 2 },
	{ "rtv2",	FMT_SIMPLE, decode_simple,		0x4a496012, 0xffffffff,0, 2 },
	{ "rtir",	FMT_SIMPLE, decode_simple,		0x4a49e012, 0xffffffff,0, 2 },
	{ "rtirsf0",FMT_SIMPLE, decode_simple,		0x4a41e012, 0xffffffff,0, 2 },
	{ "rtv0tr",	FMT_SIMPLE, decode_simple,		0x4a480012, 0xffffffff,0, 2 },
	{ "rtv1tr",	FMT_SIMPLE, decode_simple,		0x4a488012, 0xffffffff,0, 2 },
	{ "rtv2tr",	FMT_SIMPLE, decode_simple,		0x4a490012, 0xffffffff,0, 2 },
	{ "rtirtr",	FMT_SIMPLE, decode_simple,		0x4a498012, 0xffffffff,0, 2 },
	{ "rtv0bk",	FMT_SIMPLE, decode_simple,		0x4a482012, 0xffffffff,0, 2 },
	{ "rtv1bk",	FMT_SIMPLE, decode_simple,		0x4a48a012, 0xffffffff,0, 2 },
	{ "rtv2bk",	FMT_SIMPLE, decode_simple,		0x4a492012, 0xffffffff,0, 2 },
	{ "rtirbk",	FMT_SIMPLE, decode_simple,		0x4a49a012, 0xffffffff,0, 2 },
	{ "ll",		FMT_SIMPLE, decode_simple,		0x4a4a6412, 0xffffffff,0, 2 },
	{ "llv0",	FMT_SIMPLE, decode_simple,		0x4a4a6012, 0xffffffff,0, 2 },
	{ "llv1",	FMT_SIMPLE, decode_simple,		0x4a4ae012, 0xffffffff,0, 2 },
	{ "llv2",	FMT_SIMPLE, decode_simple,		0x4a4b6012, 0xffffffff,0, 2 },
	{ "llir",	FMT_SIMPLE, decode_simple,		0x4a4be012, 0xffffffff,0, 2 },
	{ "llv0tr",	FMT_SIMPLE, decode_simple,		0x4a4a0012, 0xffffffff,0, 2 },
	{ "llv1tr",	FMT_SIMPLE, decode_simple,		0x4a4a8012, 0xffffffff,0, 2 },
	{ "llv2tr",	FMT_SIMPLE, decode_simple,		0x4a4b0012, 0xffffffff,0, 2 },
	{ "llirtr",	FMT_SIMPLE, decode_simple,		0x4a4b8012, 0xffffffff,0, 2 },
	{ "llv0bk",	FMT_SIMPLE, decode_simple,		0x4a4a2012, 0xffffffff,0, 2 },
	{ "llv1bk",	FMT_SIMPLE, decode_simple,		0x4a4aa012, 0xffffffff,0, 2 },
	{ "llv2bk",	FMT_SIMPLE, decode_simple,		0x4a4b2012, 0xffffffff,0, 2 },
	{ "llirbk",	FMT_SIMPLE, decode_simple,		0x4a4ba012, 0xffffffff,0, 2 },
	{ "lc",		FMT_SIMPLE, decode_simple,		0x4a4da412, 0xffffffff,0, 2 },
	{ "lcv0",	FMT_SIMPLE, decode_simple,		0x4a4c6012, 0xffffffff,0, 2 },
	{ "lcv1",	FMT_SIMPLE, decode_simple,		0x4a4ce012, 0xffffffff,0, 2 },
	{ "lcv2",	FMT_SIMPLE, decode_simple,		0x4a4d6012, 0xffffffff,0, 2 },
	{ "lcir",	FMT_SIMPLE, decode_simple,		0x4a4de012, 0xffffffff,0, 2 },
	{ "lcv0tr",	FMT_SIMPLE, decode_simple,		0x4a4c0012, 0xffffffff,0, 2 },
	{ "lcv1tr",	FMT_SIMPLE, decode_simple,		0x4a4c8012, 0xffffffff,0, 2 },
	{ "lcv2tr",	FMT_SIMPLE, decode_simple,		0x4a4d0012, 0xffffffff,0, 2 },
	{ "lcirtr",	FMT_SIMPLE, decode_simple,		0x4a4d8012, 0xffffffff,0, 2 },
	{ "lcv0bk",	FMT_SIMPLE, decode_simple,		0x4a4c2012, 0xffffffff,0, 2 },
	{ "lcv1bk",	FMT_SIMPLE, decode_simple,		0x4a4ca012, 0xffffffff,0, 2 },
	{ "lcv2bk",	FMT_SIMPLE, decode_simple,		0x4a4d2012, 0xffffffff,0, 2 },
	{ "lcirbk",	FMT_SIMPLE, decode_simple,		0x4a4da012, 0xffffffff,0, 2 },
	{ "dpcl",	FMT_SIMPLE, decode_simple,		0x4a680029, 0xffffffff,0, 2 },
	{ "dpcs",	FMT_SIMPLE, decode_simple,		0x4a780010, 0xffffffff,0, 2 },
	{ "dpct",	FMT_SIMPLE, decode_simple,		0x4af8002a, 0xffffffff,0, 2 },
	{ "intpl",	FMT_SIMPLE, decode_simple,		0x4a980011, 0xffffffff,0, 2 },
	{ "sqr12",	FMT_SIMPLE, decode_simple,		0x4aa80428, 0xffffffff,0, 2 },
	{ "sqr0",	FMT_SIMPLE, decode_simple,		0x4aa00428, 0xffffffff,0, 2 },
	{ "ncs",	FMT_SIMPLE, decode_simple,		0x4ac8041e, 0xffffffff,0, 2 },
	{ "nct",	FMT_SIMPLE, decode_simple,		0x4ad80420, 0xffffffff,0, 2 },
	{ "ncds",	FMT_SIMPLE, decode_simple,		0x4ae80413, 0xffffffff,0, 2 },
	{ "ncdt",	FMT_SIMPLE, decode_simple,		0x4af80416, 0xffffffff,0, 2 },
	{ "nccs",	FMT_SIMPLE, decode_simple,		0x4b08041b, 0xffffffff,0, 2 },
	{ "ncct",	FMT_SIMPLE, decode_simple,		0x4b18043f, 0xffffffff,0, 2 },
	{ "cdp",	FMT_SIMPLE, decode_simple,		0x4b280414, 0xffffffff,0, 2 },
	{ "cc",		FMT_SIMPLE, decode_simple,		0x4b38041c, 0xffffffff,0, 2 },
	{ "nclip",	FMT_SIMPLE, decode_simple,		0x4b400006, 0xffffffff,0, 2 },
	{ "avsz3",	FMT_SIMPLE, decode_simple,		0x4b58002d, 0xffffffff,0, 2 },
	{ "avsz4",	FMT_SIMPLE, decode_simple,		0x4b68002e, 0xffffffff,0, 2 },
	{ "op12",	FMT_SIMPLE, decode_simple,		0x4b78000c, 0xffffffff,0, 2 },
	{ "op0",	FMT_SIMPLE, decode_simple,		0x4b70000c, 0xffffffff,0, 2 },
	{ "gpf12",	FMT_SIMPLE, decode_simple,		0x4b98003d, 0xffffffff,0, 2 },
	{ "gpf0",	FMT_SIMPLE, decode_simple,		0x4b90003d, 0xffffffff,0, 2 },
	{ "gpl12",	FMT_SIMPLE, decode_simple,		0x4ba8003e, 0xffffffff,0, 2 },
	{ "gpl0",	FMT_SIMPLE, decode_simple,		0x4ba0003e, 0xffffffff,0, 2 },
	{ "eret",	FMT_SIMPLE, decode_simple,		0x42000018, 0xffffffff,0, 0 },
	{ "rfe",	FMT_SIMPLE, decode_simple,		0x42000010, 0xffffffff,0, 0 },
	{ "sync",	FMT_SIMPLE, decode_simple,		0x0000000f, 0xffffffff,0, 0 },
	{ "tlbp",	FMT_SIMPLE, decode_simple,		0x42000008, 0xffffffff,0, 0 },
	{ "tlbr",	FMT_SIMPLE, decode_simple,		0x42000001, 0xffffffff,0, 0 },
	{ "tlbwi",	FMT_SIMPLE, decode_simple,		0x42000002, 0xffffffff,0, 0 },
	{ "tlbwr",	FMT_SIMPLE, decode_simple,		0x42000006, 0xffffffff,0, 0 },
	{ "lcache",	FMT_BASEOFS, decode_baseofs,	0x88000000, 0xfc000000,0, 0 },
	{ "flush",	FMT_BASEOFS, decode_baseofs,	0x98000000, 0xfc000000,0, 0 },
	{ "scache",	FMT_BASEOFS, decode_baseofs,	0xa8000000, 0xfc000000,0, 0 },
	{ "invalid",FMT_BASEOFS, decode_baseofs,	0xb8000000, 0xfc000000,0, 0 },
	{ "c0",		"%s ?", decode_simple,			0x42000000, 0xfe000000,0, 0 },
	{ "c1",		"%s ?", decode_simple,			0x46000000, 0xfe000000,0, 1 },
	{ "c2",		"%s ?", decode_simple,			0x4a000000, 0xfe000000,0, 2 },
	{ "c3",		"%s ?", decode_simple,			0x4e000000, 0xfe000000,0, 3 },
};

static const int n_insns = sizeof (insn_table) / sizeof (insn_table[0]);

static const char * const reg_names[] = {
	"zero",	"at",	"v0",	"v1",	"a0",	"a1",	"a2",	"a3",
	"t0",	"t1",	"t2",	"t3",	"t4",	"t5",	"t6",	"t7",
	"s0",	"s1",	"s2",	"s3",	"s4",	"s5",	"s6",	"s7",
	"t8",	"t9",	"k0",	"k1",	"gp",	"sp",	"s8",	"ra"
};

static const char * const psx_c0[] = {
	"INDEX","RANDOM","ENTRYLO","BPC","CONTEXT","BDA","TAR","DCIC",
	"BADVADDR","BDAM","ENTRYHI","BPCM","SR","CAUSE","EPC","PRID",
	"C16","C17","C18","C19","C20","C21","C22","C23",
	"C24","C25","C26","C27","C28","C29","C30","C31"
};

static const char * const psx_cx[] = {
	"C0","C1","C2","C3","C4","C5","C6","C7",
	"C8","C9","C10","C11","C12","C13","C14","C15",
	"C16","C17","C18","C19","C20","C21","C22","C23",
	"C24","C25","C26","C27","C28","C29","C30","C31"
};

static const char * const gte_general[] = {
	"VXY0","VZ0" ,"VXY1","VZ1" ,"VXY2","VZ2" ,"RGB" ,"OTZ",
	"IR0" ,"IR1" ,"IR2" ,"IR3" ,"SXY0","SXY1","SXY2","SXYP",
	"SZ0" ,"SZ1" ,"SZ2" ,"SZ3" ,"RGB0","RGB1","RGB2","????",
	"MAC0","MAC1","MAC2","MAC3","IRGB","ORGB","LZCS","LZCR"
};

static const char * const gte_control[] = {
	"R11R12","R13R21","R22R23","R31R32","R33","TRX" ,"TRY" ,"TRZ",
	"L11L12","L13L21","L22L23","L31L32","L33","RBK" ,"GBK" ,"BBK",
	"LR1LR2","LR3LG1","LG2LG3","LB1LB2","LB3","RFC" ,"GFC" ,"BFC",
	"OFX"   ,"OFY"   ,"H"     ,"DQA"   ,"DQB","ZSF3","ZSF4","FLAG"
};

/****************************************************************************/

static void decode_syscall (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,insn.s.cd);
}

static void decode_baseofs (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.i.rt],(unsigned short)insn.i.imm,reg_names[insn.i.rs]);
}

static void decode_reg_st (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rs],reg_names[insn.r.rt]);
}

static void decode_reg_ds (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rd],reg_names[insn.r.rs]);
}

static void decode_reg_tri (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.i.rt],reg_names[insn.i.rs],(unsigned short)insn.i.imm);
}

static void decode_reg_sh (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rd],reg_names[insn.r.rt],insn.r.sh);
}

static void decode_reg_dts (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rd],reg_names[insn.r.rt],reg_names[insn.r.rs]);
}

static void decode_reg_dst (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rd],reg_names[insn.r.rs],reg_names[insn.r.rt]);
}

static void decode_reg_d (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rd]);
}
static void decode_reg_s (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rs]);
}
static void decode_jreg_s (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rs]);
	opcode->use_branch_register = 1;
	opcode->branch_register = insn.r.rs;
}

static void decode_target (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	addr_t target = (insn.j.tg << 2) | (addr & 0xf0000000);

	if (buf) sprintf (buf,opcode->format,opcode->name,target);

	opcode->has_branch_target = 1;
	opcode->branch_target = target;
}

static void decode_regtgt2 (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	addr_t target = addr + (insn.i.imm << 2) + 4;

	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.i.rs],reg_names[insn.i.rt], target);

	opcode->has_branch_target = 1;
	opcode->branch_target = target;
}

static void decode_regtgt (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	addr_t target = addr + (insn.i.imm << 2) + 4;

	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.i.rs],target);

	opcode->has_branch_target = 1;
	opcode->branch_target = target;
}

static void decode_target2 (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	addr_t target = addr + (insn.i.imm << 2) + 4;

	if (buf) sprintf (buf,opcode->format,opcode->name,target);

	opcode->has_branch_target = 1;
	opcode->branch_target = target;
}

static void decode_regc0 (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rt],psx_c0[insn.r.rd]);
}

static void decode_gtectrl (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rt],gte_control[insn.r.rd]);
}

static void decode_gtedata (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rt],gte_general[insn.r.rd]);
}

static void decode_gteofs (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.i.rt],(unsigned short)insn.i.imm,gte_general[insn.i.rs]);
}

static void decode_regcx (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.r.rt],psx_cx[insn.r.rd]);
}

static void decode_c0ofs (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.i.rt],(unsigned short)insn.i.imm,psx_c0[insn.i.rs]);
}

static void decode_cxofs (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.i.rt],(unsigned short)insn.i.imm,psx_cx[insn.i.rs]);
}

static void decode_simple (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name);
}

static void decode_regimm (opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn)
{
	if (buf) sprintf (buf,opcode->format,opcode->name,
		reg_names[insn.i.rt],(unsigned short)insn.i.imm);
}

/****************************************************************************/

opcode_t *r3k_decode (char *buf,int size, addr_t addr, insn_t insn)
{
	int i;
	opcode_t *opcode = 0;
	
	for (i=0; i<n_insns; i++)
	{
		if ((insn.value & insn_table[i].mask) == insn_table[i].match)
		{
			opcode=&insn_table[i];
			opcode->use_branch_register = 0;
			opcode->has_branch_target = 0;
			break;
		}
	}

	if (opcode) opcode->decode(opcode,buf,size,addr,insn);
	else sprintf (buf,"dw 0x%08lx",insn.value);
	
	return opcode;
}

/****************************************************************************/

opcode_t *r3k_find_opcode (addr_t addr, insn_t insn)
{
	int i;
	opcode_t *opcode = 0;
	
	for (i=0; i<n_insns; i++)
	{
		if ((insn.value & insn_table[i].mask) == insn_table[i].match)
		{
			opcode=&insn_table[i];
			opcode->use_branch_register = 0;
			opcode->has_branch_target = 0;
			break;
		}
	}

	if (opcode) opcode->decode(opcode,NULL,0,addr,insn);
	
	return opcode;
}
