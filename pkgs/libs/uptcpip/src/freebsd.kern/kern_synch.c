/*-
 * Copyright (c) 1982, 1986, 1990, 1991, 1993
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
 *	@(#)kern_synch.c	8.9 (Berkeley) 5/19/95
 */

#include <sys/bsd_cdefs.h>
//__FBSDID("$FreeBSD$");

#include <sys/bsd_param.h>
#include <sys/bsd_lock.h>

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "host_serv.h"


//#define UPTCP_SLEEP_DEBUG
#ifndef UPTCP_SLEEP_DEBUG
#define UPTCP_SLEEP_DEBUG_PRINT(fmt, args...)		
#else
#include <sys/bsd_malloc.h>
#include <sys/bsd_socket.h>
#include <sys/bsd_socketvar.h>
#include <net/bsd_vnet.h>
#include <netinet/bsd_tcp_var.h>

#define UPTCP_SLEEP_DEBUG_PRINT(op, iden, wmsg)		\
	do{	\
		struct timeval t;	\
		gettimeofday(&t, NULL); 	\
		unsigned long ts = t.tv_sec*1000000 + t.tv_usec;	\
		printf("[%14lu]%6s: addr=%p %s - [%lu - %lu]\n", ts, op, iden, wmsg, V_tcpstat.tcps_sndpack, V_tcpstat.tcps_rcvpack);\
	}while(0)	
#endif

/*
 * General sleep call.  Suspends the current thread until a wakeup is
 * performed on the specified identifier.  The thread will then be made
 * runnable with the specified priority.  Sleeps at most timo/hz seconds
 * (0 means no timeout).  If pri includes PCATCH flag, signals are checked
 * before and after sleeping, else signals are not checked.  Returns 0 if
 * awakened, EWOULDBLOCK if the timeout expires.  If PCATCH is set and a
 * signal needs to be delivered, ERESTART is returned if the current system
 * call should be restarted if possible, and EINTR is returned if the system
 * call should be interrupted by the signal (return EINTR).
 *
 * The lock argument is unlocked before the caller is suspended, and
 * re-locked before _sleep() returns.  If priority includes the PDROP
 * flag the lock is not re-locked before returning.
 */
int
_sleep(void *ident, struct lock_object *lock, int priority,
    const char *wmesg, int timo)
{

	UPTCP_SLEEP_DEBUG_PRINT("sleep", ident, lock->lo_name);
	
	return  host_pthread_cond_wait(ident, lock);
}


/*
 * Make all threads sleeping on the specified identifier runnable.
 */
void
wakeup(void *ident)
{
	UPTCP_SLEEP_DEBUG_PRINT("wakeup", ident, "NA");
	
	host_pthread_cond_signal(ident);
}


/*
 * Make a thread sleeping on the specified identifier runnable.
 * May wake more than one thread if a target thread is currently
 * swapped out.
 */
void
wakeup_one(void *ident)
{
	host_pthread_cond_signal(ident);
}

