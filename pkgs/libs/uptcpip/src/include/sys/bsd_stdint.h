/*-
 * Copyright (c) 2001 Mike Barcroft <mike@FreeBSD.org>
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
 * $FreeBSD$
 */

#ifndef _BSD_SYS_STDINT_H_
#define _BSD_SYS_STDINT_H_

#include <sys/bsd_cdefs.h>
#include <sys/bsd__types.h>

#include <machine/bsd__stdint.h>

#if 0
#ifndef __int8_t_defined
typedef	__int8_t		int8_t;
#define	__int8_t_defined
#endif

#ifndef __int16_t_defined
typedef	__int16_t		int16_t;
#define	__int16_t_defined
#endif

#ifndef __int32_t_defined
typedef	__int32_t		int32_t;
#define	__int32_t_defined
#endif

#ifndef __int64_t_defined
typedef	__int64_t		int64_t;
#define	__int64_t_defined
#endif

#ifndef __uint8_t_defined
typedef	__uint8_t		uint8_t;
#define	__uint8_t_defined
#endif

#ifndef __uint16_t_defined
typedef	__uint16_t		uint16_t;
#define	__uint16_t_defined
#endif

#ifndef __uint32_t_defined
typedef	__uint32_t		uint32_t;
#define	__uint32_t_defined
#endif

#ifndef __uint64_t_defined
typedef	__uint64_t		uint64_t;
#define	__uint64_t_defined
#endif

typedef	__int_least8_t		int_least8_t;
typedef	__int_least16_t		int_least16_t;
typedef	__int_least32_t		int_least32_t;
typedef	__int_least64_t		int_least64_t;

typedef	__uint_least8_t		uint_least8_t;
typedef	__uint_least16_t	uint_least16_t;
typedef	__uint_least32_t	uint_least32_t;
typedef	__uint_least64_t	uint_least64_t;

typedef	__int_fast8_t		int_fast8_t;
typedef	__int_fast16_t		int_fast16_t;
typedef	__int_fast32_t		int_fast32_t;
typedef	__int_fast64_t		int_fast64_t;

typedef	__uint_fast8_t		uint_fast8_t;
typedef	__uint_fast16_t		uint_fast16_t;
typedef	__uint_fast32_t		uint_fast32_t;
typedef	__uint_fast64_t		uint_fast64_t;

typedef	__intmax_t		intmax_t;
typedef	__uintmax_t		uintmax_t;

#ifndef __intptr_t_defined
typedef	__intptr_t		intptr_t;
typedef	__uintptr_t		uintptr_t;
#define	__intptr_t_defined
#endif

#endif //0

#endif /* !_BSD_SYS_STDINT_H_ */
