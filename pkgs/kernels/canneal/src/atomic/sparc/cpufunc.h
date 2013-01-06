/*-
 * Copyright (c) 2001 Jake Burkholder.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/sparc64/include/cpufunc.h,v 1.20.2.1 2005/08/05 19:46:13 jhb Exp $
 */

#ifndef	_MACHINE_CPUFUNC_H_
#define	_MACHINE_CPUFUNC_H_

#include "asi.h"

/*
 * membar operand macros for use in other macros when # is a special
 * character.  Keep these in sync with what the hardware expects.
 */
#define	C_Lookaside	(0)
#define	C_MemIssue	(1)
#define	C_Sync		(2)
#define	M_LoadLoad	(0)
#define	M_StoreLoad	(1)
#define	M_LoadStore	(2)
#define	M_StoreStore	(3)

#define	CMASK_SHIFT	(4)
#define	MMASK_SHIFT	(0)

#define	CMASK_GEN(bit)	((1 << (bit)) << CMASK_SHIFT)
#define	MMASK_GEN(bit)	((1 << (bit)) << MMASK_SHIFT)

#define	Lookaside	CMASK_GEN(C_Lookaside)
#define	MemIssue	CMASK_GEN(C_MemIssue)
#define	Sync		CMASK_GEN(C_Sync)
#define	LoadLoad	MMASK_GEN(M_LoadLoad)
#define	StoreLoad	MMASK_GEN(M_StoreLoad)
#define	LoadStore	MMASK_GEN(M_LoadStore)
#define	StoreStore	MMASK_GEN(M_StoreStore)

#define	casa(rs1, rs2, rd, asi) ({					\
	u_int __rd = (uint32_t)(rd);					\
	__asm __volatile("casa [%2] %3, %4, %0"				\
	    : "+r" (__rd), "=m" (*rs1)					\
	    : "r" (rs1), "n" (asi), "r" (rs2), "m" (*rs1));		\
	__rd;								\
})

#define	casxa(rs1, rs2, rd, asi) ({					\
	u_long __rd = (uint64_t)(rd);					\
	__asm __volatile("casxa [%2] %3, %4, %0"			\
	    : "+r" (__rd), "=m" (*rs1)					\
	    : "r" (rs1), "n" (asi), "r" (rs2), "m" (*rs1));		\
	__rd;								\
})

#define	membar(mask) do {						\
	__asm __volatile("membar %0" : : "n" (mask) : "memory");	\
} while (0)

#undef LDNC_GEN
#undef STNC_GEN

#endif /* !_MACHINE_CPUFUNC_H_ */
