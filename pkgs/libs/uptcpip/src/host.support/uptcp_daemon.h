/*
 * Copyright 2010
 * Princeton University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this list of
 *     conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list
 *     of conditions and the following disclaimer in the documentation and/or other materials
 *     provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY PRINCETON UNIVERSITY ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL PRINCETON UNIVERSITY OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Princeton University.
 *
 */

/* 
 *	@(#)host_daemon.h	(Princeton) 11/10/2010
 *  
 */


 #ifndef _HOST_DAEMON_H_
 #define _HOST_DAEMON_H_

#ifndef UPTCP_CLIENT
#define SERVER_PATH     "/tmp/uptcp_daemon_server"
#else
#define SERVER_PATH     "/tmp/uptcp_daemon_client"
#endif

#define MAX_SOCKBUF_LEN      0x100000  // 1MB
#define MAX_SAL_CLIENTS       10
#define SOCKFD_USED              -100

/* request type*/
#define UPTCP_REQ_SOCKET		1
#define UPTCP_REQ_BIND			2
#define UPTCP_REQ_LISTEN		3
#define UPTCP_REQ_ACCEPT		4
#define UPTCP_REQ_CONNECT		5
#define UPTCP_REQ_SOCKETPAIR		6
#define UPTCP_REQ_SENDTO		7
#define UPTCP_REQ_SEND			8
#define UPTCP_REQ_SENDMSG		9
#define UPTCP_REQ_RECVFROM		10
#define UPTCP_REQ_RECV			11
#define UPTCP_REQ_RECVMSG		12
#define UPTCP_REQ_SHUTDOWN		13
#define UPTCP_REQ_SETSOCKOPT		14
#define UPTCP_REQ_GETSOCKOPT		15
#define UPTCP_REQ_GETSOCKNAME		16
#define UPTCP_REQ_GETPEERNAME		17
#define UPTCP_REQ_SOCKFD		18
#define UPTCP_REQ_MALLOC		19
#define UPTCP_REQ_FREE			20


#endif /* _HOST_DAEMON_H_ */

