/*-
 * Copyright (c) 1986, 1988, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)subr_prf.c	8.3 (Berkeley) 1/21/94
 */

#include <sys/bsd_cdefs.h>
//__FBSDID("$FreeBSD$");

#include "bsd_opt_ddb.h"
#include "bsd_opt_printf.h"

#include <sys/bsd_param.h>
//#include <sys/bsd_systm.h>
#include <sys/bsd_lock.h>
//#include <sys/bsd_kdb.h>
#include <sys/bsd_mutex.h>
//#include <sys/bsd_sx.h>
#include <sys/bsd_kernel.h>
//#include <sys/bsd_msgbuf.h>
//#include <sys/bsd_malloc.h>
//#include <sys/bsd_priv.h>
////#include <sys/bsd_proc.h>
#include <sys/bsd_stddef.h>
//#include <sys/bsd_sysctl.h>
//#include <sys/bsd_tty.h>
//#include <sys/bsd_syslog.h>
//#include <sys/bsd_cons.h>
//#include <sys/bsd_uio.h>
#include <sys/bsd_ctype.h>
#include <sys/bsd_types.h>
#include <sys/bsd_stdint.h>

/*
 * Note that stdarg.h and the ANSI style va_start macro is used for both
 * ANSI and traditional C compilers.
 */
#include <stdio.h>
#include <stdarg.h>

#define TOCONS	0x01
#define TOTTY	0x02
#define TOLOG	0x04

/* Max number conversion buffer length: a u_quad_t in base 2, plus NUL byte. */
#define MAXNBUF	(sizeof(intmax_t) * NBBY + 1)

/*
 * Log writes to the log buffer, and guarantees not to sleep (so can be
 * called by interrupt routines).  If there is no process reading the
 * log yet, it writes to the console also.
 */
void
bsd_log(int level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	//kvprintf(fmt, putchar, &pca, 10, ap);
	va_end(ap);

}

#if 0
/*
int
printf(const char *fmt, ...)
{
	va_list ap;
	int retval;

	va_start(ap, fmt);
	//retval = vprintf(fmt, ap);
	va_end(ap);

	return (retval);
}
*/
int
vprintf(const char *fmt, va_list ap)
{
	int retval = 1;

	//retval = kvprintf(fmt, putchar, &pca, 10, ap);

	return (retval);
}


/*
 * Scaled down version of sprintf(3).
 */
#if 0
int
sprintf(char *buf, const char *cfmt, ...)
{
	int retval;
	va_list ap;


	va_start(ap, cfmt);
	retval = host_printf(buf, cfmt, ap);	
	buf[retval] = '\0';
	va_end(ap);
	return (retval);
}
#endif

/*
 * Scaled down version of vsprintf(3).
 */
int
vsprintf(char *buf, const char *cfmt, va_list ap)
{
	int retval;

	retval = 0;//kvprintf(cfmt, NULL, (void *)buf, 10, ap);
	buf[retval] = '\0';
	return (retval);
}

/*
 * Scaled down version of snprintf(3).
 */
int
snprintf(char *str, size_t size, const char *format, ...)
{
	int retval;
	va_list ap;

	va_start(ap, format);
	retval = 0;//vsnprintf(str, size, format, ap);
	va_end(ap);
	return(retval);
}

/*
 * Scaled down version of vsnprintf(3).
 */
int
vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	int retval;

	retval = 0;//kvprintf(format, snprintf_func, &info, 10, ap);
	return (retval);
}
#endif //0
