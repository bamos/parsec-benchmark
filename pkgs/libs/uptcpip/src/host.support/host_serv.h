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
 *	@(#)host_serv.h	(Princeton) 11/11/2010
 *  
 */


#ifndef _HOST_SERV_H_
#define _HOST_SERV_H_

#include <pthread.h>
#include <sys/time.h>

/* host pthread service*/
#define THREAD_TYPE_APP		0
#define THREAD_TYPE_SOCKET	1
#define THREAD_TYPE_RX_TCPIP	2
#define THREAD_TYPE_TX_LINK	3
#define THREAD_TYPE_RX_LINK	4
#define THREAD_TYPE_TIMER	5



typedef struct {
	pthread_t 	 ptr;
	int	  	 id;
	int	   	 type;
	struct timeval   last_active_time;
	struct timeval   last_tcpip_time;
	unsigned long long intcpip_time;
	unsigned long long active_time;
} thread_wrapper;

/*
 * | thread_index | ----> | thread_list |
 */
#define THREAD_LIST_SIZE	256
#define THREAD_INDEX_SIZE	4093  //prime number 

extern thread_wrapper	thread_list[THREAD_LIST_SIZE];
extern unsigned char	thread_index[THREAD_INDEX_SIZE]; 

extern int		uptcpip_thread_count;
extern pthread_mutex_t uptcpip_thread_count_mutex;


extern int host_thread_create(void* (*routine)(void*), void* t_args, int type);
extern int pthread_create_wrapper(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg, int type);
extern int host_thread_get_uptcp_tid();
extern int host_thread_change_type(int type);
extern int host_thread_enter_tcpip_region(int type);
extern int host_thread_exit_tcpip_region(int thread_id);
extern void enter_tcpip_roi();
extern void exit_tcpip_roi();
extern void host_thread_enter_wait_region();
extern void host_thread_exit_wait_region();

/* host time service */
extern int host_usleep(int num);


/* host raw socket service */
#define RAWSOCK_SEND		0
#define RAWSOCK_RECV		1

extern int host_setup_link(int port);
extern int host_connect_link(int port);
int host_link_send(int link_fd, const void * buf, size_t len);
int host_link_recv(int link_fd,void *buf, size_t len);

void*	host_malloc(size_t size, int align);
void host_free(void *ptr);

/* lock */
void host_pthread_mutexattr_init();
void* host_pthread_mutex_init(int flag);
void host_pthread_mutex_destory(void* lock);
void host_pthread_mutex_lock(void* lock);
int host_pthread_mutex_trylock(void* lock);
void host_pthread_mutex_unlock(void* lock);

void* host_pthread_rwlock_init();
int host_pthread_rwlock_destory(void* lock);
void host_pthread_rwlock_wlock(void* lock);
void host_pthread_rwlock_wunlock(void* lock);
void host_pthread_rwlock_rlock(void* lock);
void host_pthread_rwlock_runlock(void* lock);
int host_pthread_rwlock_tryrlock(void* lock);
int host_pthread_rwlock_trywlock(void* lock);
void host_pthread_rwlock_unlock(void* lock);


void* host_pthread_cond_init();
void host_pthread_cond_destroy(void* cond);
int host_pthread_cond_wait(void* cond, void* lock);
void host_pthread_cond_signal(void* cond);

void dump_stackframe(int depth);

unsigned short host_shadow_socket_open(unsigned short port);
void host_shadow_socket_close(unsigned short sockfd, unsigned short port);

#endif /* _HOST_SERV_H_ */


