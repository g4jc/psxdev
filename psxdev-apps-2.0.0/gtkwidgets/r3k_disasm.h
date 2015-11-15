/* $Id: r3k_disasm.h,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 *	R3K/GTE disassembler
 *	
 *	Copyright (C) 1998,1999,2000 Daniel Balster (dbalster@psxdev.de)
 *	http://www.psxdev.de
 *	
 *	This is not free software.
 */

#ifndef __R3K_DISASM_H__
#define __R3K_DISASM_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct Opcode opcode_t;
typedef unsigned long addr_t;

typedef struct {
	signed imm : 16;
	unsigned rt : 5;
	unsigned rs : 5;
	unsigned op : 6;
} i_insn_t;

typedef struct {
	unsigned fn : 6;
	unsigned sh : 5;
	unsigned rd : 5;
	unsigned rt : 5;
	unsigned rs : 5;
	unsigned op : 6;
} r_insn_t;

typedef struct {
	unsigned tg : 26;
	unsigned op : 6;
} j_insn_t;

typedef struct {
	unsigned fn : 6;
	unsigned cd : 20;
	unsigned op : 6;
} s_insn_t;

typedef union {
	i_insn_t i;
	r_insn_t r;
	j_insn_t j;
	s_insn_t s;
	unsigned long value;
} insn_t;

typedef void (*opcode_decoder_t)(opcode_t *opcode, char *buf, int size, addr_t addr, insn_t insn);

struct Opcode {
	char *name;
	char *format;
	opcode_decoder_t decode;
	unsigned long match;
	unsigned long mask;

	unsigned cause_delay_slot : 1;	// jump, coprocessor, etc.
	unsigned coprocessor : 2;		// coprocessor ID
	unsigned has_branch_target : 1;	//
	unsigned use_branch_register : 1;

	addr_t branch_target;
	unsigned long branch_register;
};

/* cause register */

typedef struct {
	unsigned long p3 : 2;
	unsigned long exccode : 5;
	unsigned long p2 : 1;
	unsigned long ip : 8;
	unsigned long p1 : 12;
	unsigned long ce : 2;
	unsigned long p0 : 1;
	unsigned long bd : 1;
} cr_t;

/* status register */

typedef struct {
	unsigned long cu3 : 1;
	unsigned long cu2 : 1;
	unsigned long cu1 : 1;
	unsigned long cu0 : 1;
	unsigned long p0 : 2;
	unsigned long re : 1;
	unsigned long p1 : 2;
	unsigned long bev : 1;
	unsigned long ts : 1;
	unsigned long pe : 1;
	unsigned long cm : 1;
	unsigned long pz : 1;
	unsigned long swc : 1;
	unsigned long isc : 1;
	unsigned long im : 8;
	unsigned long p2 : 2;
	unsigned long KUo : 1;
	unsigned long IEo : 1;
	unsigned long KUp : 1;
	unsigned long IEp : 1;
	unsigned long KUc : 1;
	unsigned long IEc : 1;
} sr_t;

/** prototypes **/

opcode_t *r3k_decode (char *buf, int size, addr_t addr, insn_t insn);
opcode_t *r3k_find_opcode (addr_t addr, insn_t insn);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __R3K_DISASM_H__ */
