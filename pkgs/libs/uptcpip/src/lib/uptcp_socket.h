/*-
 * Copyright (c) 1982, 1985, 1986, 1988, 1993, 1994
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
 *	@(#)socket.h	8.4 (Berkeley) 2/21/94
 * $FreeBSD$
 */

#ifndef _UPTCP_SOCKET_H_
#define	_UPTCP_SOCKET_H_


__BEGIN_DECLS
void	uptcp_init();
void 	parsec_enter_tcpip_roi();
void 	parsec_exit_tcpip_roi();
int	uptcp_print_statis();
int	uptcp_accept(int, struct sockaddr * __restrict, socklen_t * __restrict);
int	uptcp_bind(int, const struct sockaddr *, socklen_t);
int	uptcp_connect(int, const struct sockaddr *, socklen_t);
int	uptcp_getpeername(int, struct sockaddr * __restrict, socklen_t * __restrict);
int	uptcp_getsockname(int, struct sockaddr * __restrict, socklen_t * __restrict);
int	uptcp_getsockopt(int, int, int, void * __restrict, socklen_t * __restrict);
int	uptcp_listen(int, int);
ssize_t	uptcp_recv(int, void *, size_t, int);
ssize_t	uptcp_recvfrom(int, void *, size_t, int, struct sockaddr * __restrict, socklen_t * __restrict);
ssize_t	uptcp_recvmsg(int, struct msghdr *, int);
ssize_t	uptcp_send(int, const void *, size_t, int);
ssize_t	uptcp_sendto(int, const void *,
	    size_t, int, const struct sockaddr *, socklen_t);
ssize_t	uptcp_sendmsg(int, const struct msghdr *, int);
int	uptcp_setsockopt(int, int, int, const void *, socklen_t);
int uptcp_close(int s);
int	uptcp_shutdown(int, int);
int	uptcp_sockatmark(int);
int	uptcp_socket(int, int, int);
int	uptcp_socketpair(int, int, int, int *);

void*	uptcp_malloc(int, unsigned long);
int	uptcp_free(int, void*);

int     uptcp_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);
__END_DECLS



#endif /* !_BSD_SYS_SOCKET_H_ */
