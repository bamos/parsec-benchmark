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
 *	@(#)uptcp_api.h	(Princeton) 11/10/2010
 *
 */

#ifndef _UPTCP_API_H_
#define  _UPTCP_API_H_


#define MAX_STRLEN    128
#define REQTYPE_LEN  (sizeof(int))

/*
 *  Socket API Layer: data structure for API requests
 */ 
struct sal_api_req {
	int      type;
	char   args[1];
};


struct sal_socket_args {
	int domain;
	int type;
	int protocol;
};

#if 0
struct freebsd.sockaddr {
	unsigned char	sa_len;		/* total length */
	sa_family_t	sa_family;	/* address family */
	char		sa_data[14];	/* actually longer; address value */
};

struct linux.sockaddr {
	sa_family_t	sa_family;	/* address family */
	char		sa_data[14];	/* actually longer; address value */
};

#endif //0

struct sal_bind_args {
	unsigned char	sa_len;
	unsigned char     sa_family;
	char                  sa_data[14];
};

struct sal_listen_args {
	int	backlog;
};

struct sal_accept_args {
	int	sockfd;
};

struct sal_connect_args {
	unsigned char	sa_len;
	unsigned char     sa_family;
	char                  sa_data[14];
};

struct sal_socketpair_args {
	int domain; 
	int type; 
	int protocol; 
	int *rsv;
};

struct sal_sendto_args {
	//int	s; 
	char	buf[MAX_STRLEN]; ; 
	size_t	len; 
	int	flags; 
	char	to[MAX_STRLEN];  
	int	tolen;
};

struct sal_send_args {
	int	flags;
	int	len;
	int	shm_offset;	
	char	buf[1]; 	
};

struct sal_sendmsg_args {
	//int	s;	
	char	msg[MAX_STRLEN];  
	int	flags;
};

struct sal_recvfrom_args {
	//int s;	
	char	buf[MAX_STRLEN];  
	size_t	len; 
	int flags; 
	struct sockaddr *from; 
	socklen_t *fromlenaddr;
};

struct sal_recv_args {
	int	len; 	
	int	flags;
	int	shm_offset;
};

struct sal_recvmsg_args {
	//int	s; 
	struct msghdr *msg; 
	int	flags;
};

struct sal_shutdown_args {
	int how;
};

struct sal_setsockopt_args {
	//int	s;
	int	level;
	int	name;
	char	val[MAX_STRLEN]; 
	int	valsize;
};

struct sal_getsockopt_args {
	//int	s;
	int	level;
	int	name; 
	void * val;
	socklen_t * avalsize;
};

struct sal_getsockname_args {
	//int	fdes; 
	struct sockaddr * asa; 	
	socklen_t *alen;
} ;

struct sal_getpeername_args {
	//int fdes; 		
	struct sockaddr *asa; 	
	socklen_t *alen;
};

struct sal_malloc_args {
	unsigned long size;
};



/*
 *  Socket API Layer: data structure for API results
 */ 
struct sal_api_res {
	int      res;
	char    retval[1];
};


struct sal_accept_res {
	unsigned char	sa_len;		/* total length */
	unsigned char	sa_family;	/* address family */
	char			sa_data[14];	/* actually longer; address value */
	socklen_t		anamelen;
};


struct sal_socketpair_res {
	int *rsv;
};

struct sal_sendto_res {
	int	s; 
	char	buf[MAX_STRLEN]; ; 
	size_t	len; 
	int	flags; 
	char	to[MAX_STRLEN];  
	int	tolen;
};



struct sal_recvfrom_res {
	int s;	
	char	buf[MAX_STRLEN];  
	size_t	len; 
	int flags; 
	struct sockaddr *from; 
	socklen_t *fromlenaddr;
};

struct sal_recv_res {
	char	buf[1];  
};

struct sal_recvmsg_res {
	int	s; 
	struct msghdr *msg; 
	int	flags;
};


struct sal_setsockopt_res {
	int	s;
	int	level;
	int	name;
	char	val[MAX_STRLEN]; 
	int	valsize;
};

struct sal_getsockopt_res {
	int	s;
	int	level;
	int	name; 
	void * val;
	socklen_t * avalsize;
};

struct sal_getsockname_res {
	int	fdes; 
	struct sockaddr * asa; 	
	socklen_t *alen;
} ;

struct sal_getpeername_res {
	int fdes; 		
	struct sockaddr *asa; 	
	socklen_t *alen;
};


#endif 

