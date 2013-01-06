/*-
 * Copyright (c) 1982, 1986, 1991, 1993, 1994
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
 *	@(#)types.h	8.6 (Berkeley) 2/19/95
 * $FreeBSD$
 */

#ifndef _BSD_SYS_TYPES_H_
#define	_BSD_SYS_TYPES_H_

#include <sys/bsd_cdefs.h>

/* Machine type dependent parameters. */
#include <machine/bsd_endian.h>
#include <sys/bsd__types.h>

#include <sys/bsd__pthreadtypes.h>

#ifndef __USE_BSD
#define __USE_BSD
#endif

#include <sys/types.h>
#include <stdint.h>

#if 0
#if __BSD_VISIBLE
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
#ifndef _FREEBSD_KERNEL
typedef	unsigned short	ushort;		/* Sys V compatibility */
typedef	unsigned int	uint;		/* Sys V compatibility */
#endif
#endif

/*
 * XXX POSIX sized integrals that should appear only in <sys/stdint.h>.
 */
#ifndef __int8_t_defined
typedef	__int8_t	int8_t;
#define	__int8_t_defined
#endif

#ifndef __int16_t_defined
typedef	__int16_t	int16_t;
#define	__int16_t_defined
#endif

#ifndef __int32_t_defined
typedef	__int32_t	int32_t;
#define	__int32_t_defined
#endif

#ifndef __int64_t_defined
typedef	__int64_t	int64_t;
#define	__int64_t_defined
#endif

#ifndef __uint8_t_defined
typedef	__uint8_t	uint8_t;
#define	__uint8_t_defined
#endif

#ifndef __uint16_t_defined
typedef	__uint16_t	uint16_t;
#define	__uint16_t_defined
#endif

#ifndef __uint32_t_defined
typedef	__uint32_t	uint32_t;
#define	__uint32_t_defined
#endif

#ifndef __uint64_t_defined
typedef	__uint64_t	uint64_t;
#define	__uint64_t_defined
#endif

#ifndef __intptr_t_defined
typedef	__intptr_t	intptr_t;
typedef	__uintptr_t	uintptr_t;
#define	__intptr_t_defined
#endif

typedef __uint8_t	u_int8_t;	/* unsigned integrals (deprecated) */
typedef __uint16_t	u_int16_t;
typedef __uint32_t	u_int32_t;
typedef __uint64_t	u_int64_t;

typedef	__uint64_t	u_quad_t;	/* quads (deprecated) */
typedef	__int64_t	quad_t;
#endif //0
typedef	quad_t *	qaddr_t;

//typedef	char *		caddr_t;	/* core address */
typedef	__const char *	c_caddr_t;	/* core address, pointer to const */
typedef	__volatile char *v_caddr_t;	/* core address, pointer to volatile */

#ifndef __blksize_t_defined
typedef	__blksize_t	blksize_t;
#define	__blksize_t_defined
#endif

typedef	__cpuwhich_t	cpuwhich_t;
typedef	__cpulevel_t	cpulevel_t;
typedef	__cpusetid_t	cpusetid_t;

#ifndef __blkcnt_t_defined
typedef	__blkcnt_t	blkcnt_t;
#define	__blkcnt_t_defined
#endif

#ifndef __clock_t_defined
typedef	__clock_t	clock_t;
#define	__clock_t_defined
#endif

#ifndef __clockid_t_defined
typedef	__clockid_t	clockid_t;
#define	__clockid_t_defined
#endif

typedef	__cpumask_t	cpumask_t;
typedef	__critical_t	critical_t;	/* Critical section value */

#if 0
typedef	__int64_t	daddr_t;	/* disk address */

#ifndef __dev_t_defined
typedef	__dev_t		dev_t;		/* device number or struct cdev */
#define	__dev_t_defined
#endif
#endif //0

#ifndef __fflags_t_defined
typedef	__fflags_t	fflags_t;	/* file flags */
#define	__fflags_t_defined
#endif

typedef	__fixpt_t	fixpt_t;	/* fixed point number */

#if 0
#ifndef __fsblkcnt_t_defined		/* for statvfs() */
typedef	__fsblkcnt_t	fsblkcnt_t;
typedef	__fsfilcnt_t	fsfilcnt_t;
#define	__fsblkcnt_t_defined
#endif

#ifndef __gid_t_defined
typedef	__gid_t		gid_t;		/* group id */
#define	__gid_t_defined
#endif

#ifndef __in_addr_t_defined
typedef	__uint32_t	in_addr_t;	/* base type for internet address */
#define	__in_addr_t_defined
#endif

#ifndef __in_port_t_defined
typedef	__uint16_t	in_port_t;
#define	__in_port_t_defined
#endif

#ifndef __id_t_defined
typedef	__id_t		id_t;		/* can hold a uid_t or pid_t */
#define	__id_t_defined
#endif

#ifndef __ino_t_defined
typedef	__ino_t		ino_t;		/* inode number */
#define	__ino_t_defined
#endif

#ifndef __key_t_defined
typedef	__key_t		key_t;		/* IPC key (for Sys V IPC) */
#define	__key_t_defined
#endif

#ifndef __lwpid_t_defined
typedef	__lwpid_t	lwpid_t;	/* Thread ID (a.k.a. LWP) */
#define	__lwpid_t_defined
#endif

#ifndef __mode_t_defined
typedef	__mode_t	mode_t;		/* permissions */
#define	__mode_t_defined
#endif
#endif //0

#ifndef __accmode_t_defined
typedef	__accmode_t	accmode_t;	/* access permissions */
#define	__accmode_t_defined
#endif

#if 0
#ifndef __nlink_t_defined
typedef	__nlink_t	nlink_t;	/* link count */
#define	__nlink_t_defined
#endif

#ifndef __off_t_defined
typedef	__off_t		off_t;		/* file offset */
#define	__off_t_defined
#endif

#ifndef __pid_t_defined
typedef	__pid_t		pid_t;		/* process id */
#define	__pid_t_defined
#endif

typedef	__register_t	register_t;

#ifndef __rlim_t_defined
typedef	__rlim_t	rlim_t;		/* resource limit */
#define	__rlim_t_defined
#endif
#endif //0

typedef	__segsz_t	segsz_t;	/* segment size (in pages) */

#if 0
#ifndef __size_t_defined
typedef	__size_t	size_t;
#define	__size_t_defined
#endif

#ifndef __ssize_t_defined
typedef	__ssize_t	ssize_t;
#define	__ssize_t_defined
#endif

#ifndef __suseconds_t_defined
typedef	__suseconds_t	suseconds_t;	/* microseconds (signed) */
#define	__suseconds_t_defined
#endif

#ifndef __time_t_defined
typedef	__time_t	time_t;
#define	__time_t_defined
#endif

#ifndef __timer_t_defined
typedef	__timer_t	timer_t;
#define	__timer_t_defined
#endif

#ifndef __mqd_t_defined
typedef	__mqd_t	mqd_t;
#define	__mqd_t_defined
#endif
#endif //0

typedef	__u_register_t	u_register_t;

#if 0
#ifndef __uid_t_defined
typedef	__uid_t		uid_t;		/* user id */
#define	__uid_t_defined
#endif

#ifndef __useconds_t_defined
typedef	__useconds_t	useconds_t;	/* microseconds (unsigned) */
#define	__useconds_t_defined
#endif
#endif //0

typedef	__vm_offset_t	vm_offset_t;
typedef	__vm_ooffset_t	vm_ooffset_t;
typedef	__vm_paddr_t	vm_paddr_t;
typedef	__vm_pindex_t	vm_pindex_t;
typedef	__vm_size_t	vm_size_t;

#ifdef _FREEBSD_KERNEL
typedef	int		boolean_t;
typedef	struct device	*device_t;
typedef	__intfptr_t	intfptr_t;

/*-
 * XXX this is fixed width for historical reasons.  It should have had type
 * __int_fast32_t.  Fixed-width types should not be used unless binary
 * compatibility is essential.  Least-width types should be used even less
 * since they provide smaller benefits.
 * XXX should be MD.
 * XXX this is bogus in -current, but still used for spl*().
 */
typedef	__uint32_t	intrmask_t;	/* Interrupt mask (spl, xxx_imask...) */

typedef	__uintfptr_t	uintfptr_t;
typedef	__uint64_t	uoff_t;
typedef	char		vm_memattr_t;	/* memory attribute codes */
typedef	struct vm_page	*vm_page_t;

#define offsetof(type, field) __offsetof(type, field)

#endif /* !_FREEBSD_KERNEL */

#if 0
/*
 * The following are all things that really shouldn't exist in this header,
 * since its purpose is to provide typedefs, not miscellaneous doodads.
 */
#if __BSD_VISIBLE

#include <sys/bsd_select.h>

/*
 * minor() gives a cookie instead of an index since we don't want to
 * change the meanings of bits 0-15 or waste time and space shifting
 * bits 16-31 for devices that don't use them.
 */
#define	major(x)	((int)(((u_int)(x) >> 8)&0xff))	/* major number */
#define	minor(x)	((int)((x)&0xffff00ff))		/* minor number */
#define	makedev(x,y)	((dev_t)(((x) << 8) | (y)))	/* create dev_t */


/*
 * These declarations belong elsewhere, but are repeated here and in
 * <stdio.h> to give broken programs a better chance of working with
 * 64-bit off_t's.
 */
#ifndef _FREEBSD_KERNEL
__BEGIN_DECLS
#ifndef __ftruncate_defined
#define	__ftruncate_defined
int	 ftruncate(int, off_t);
#endif
#ifndef __lseek_defined
#define	__lseek_defined
off_t	 lseek(int, off_t, int);
#endif
#ifndef __mmap_defined
#define	__mmap_defined
void *	 mmap(void *, size_t, int, int, int, off_t);
#endif
#ifndef __truncate_defined
#define	__truncate_defined
int	 truncate(const char *, off_t);
#endif
__END_DECLS
#endif /* !_FREEBSD_KERNEL */

#endif /* __BSD_VISIBLE */
#endif //0


#endif /* !_BSD_SYS_TYPES_H_ */
