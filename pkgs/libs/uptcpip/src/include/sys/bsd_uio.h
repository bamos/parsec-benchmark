/*-
 * Copyright (c) 1982, 1986, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)uio.h	8.5 (Berkeley) 2/22/94
 * $FreeBSD$
 */

#ifndef _BSD_SYS_UIO_H_
#define	_BSD_SYS_UIO_H_

#include <sys/bsd_cdefs.h>
#include <sys/bsd__types.h>
//#include <sys/bsd__iovec.h>
#include <sys/uio.h>

#ifndef __ssize_t_defined
typedef	__ssize_t	ssize_t;
#define	__ssize_t_defined
#endif

#ifndef __off_t_defined
typedef	__off_t	off_t;
#define	__off_t_defined
#endif

#if __BSD_VISIBLE
enum	uio_rw { UIO_READ, UIO_WRITE };

/* Segment flag values. */
enum uio_seg {
	UIO_USERSPACE,		/* from user data space */
	UIO_SYSSPACE,		/* from system space */
	UIO_NOCOPY		/* don't copy, already in object */
};
#endif

#ifdef _FREEBSD_KERNEL

struct uio {
	struct	iovec *uio_iov;		/* scatter/gather list */
	int	uio_iovcnt;		/* length of scatter/gather list */
	off_t	uio_offset;		/* offset in target object */
	ssize_t	uio_resid;		/* remaining bytes to process */
	enum	uio_seg uio_segflg;	/* address space */
	enum	uio_rw uio_rw;		/* operation */
	struct	thread *uio_td;		/* owner */
};

/*
 * Limits
 *
 * N.B.: UIO_MAXIOV must be no less than IOV_MAX from <sys/bsd_syslimits.h>
 * which in turn must be no less than _XOPEN_IOV_MAX from <limits.h>.  If
 * we ever make this tunable (probably pointless), then IOV_MAX should be
 * removed from <sys/syslimits.h> and applications would be expected to use
 * sysconf(3) to find out the correct value, or else assume the worst
 * (_XOPEN_IOV_MAX).  Perhaps UIO_MAXIOV should be simply defined as
 * IOV_MAX.
 */
#define UIO_MAXIOV	1024		/* max 1K of iov's */

struct vm_object;
struct vm_page;

struct uio *cloneuio(struct uio *uiop);
int	copyinfrom(const void * __restrict src, void * __restrict dst,
	    size_t len, int seg);
int	copyiniov(struct iovec *iovp, u_int iovcnt, struct iovec **iov,
	    int error);
int	copyinstrfrom(const void * __restrict src, void * __restrict dst,
	    size_t len, size_t * __restrict copied, int seg);
int	copyinuio(struct iovec *iovp, u_int iovcnt, struct uio **uiop);
void	uio_yield(void);
int	uiomove(void *cp, int n, struct uio *uio);
int	uiomove_frombuf(void *buf, int buflen, struct uio *uio);
int	uiomove_fromphys(struct vm_page *ma[], vm_offset_t offset, int n,
	    struct uio *uio);
int	uiomoveco(void *cp, int n, struct uio *uio, int disposable);

#else /* !_FREEBSD_KERNEL */

__BEGIN_DECLS
ssize_t	readv(int, const struct iovec *, int);
ssize_t	writev(int, const struct iovec *, int);
#if __BSD_VISIBLE
ssize_t	preadv(int, const struct iovec *, int, off_t);
ssize_t	pwritev(int, const struct iovec *, int, off_t);
#endif
__END_DECLS

#endif /* _FREEBSD_KERNEL */

#endif /* !_BSD_SYS_UIO_H_ */
