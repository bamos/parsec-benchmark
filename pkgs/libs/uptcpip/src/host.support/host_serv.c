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


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include<sys/mman.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <net/if.h>
//#include <linux/filter.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <sys/ioctl.h>

#include <sys/bsd_lock.h>

#include "host_serv.h"

#define	LC_SLEEPLOCK	0x00000001	/* Sleep lock. */
#define	LC_SPINLOCK	0x00000002	/* Spin lock. */
#define	LC_SLEEPABLE	0x00000004	/* Sleeping allowed with this lock. */
#define	LC_RECURSABLE	0x00000008	/* Locks of this type may recurse. */
#define	LC_UPGRADABLE	0x00000010	/* Upgrades and downgrades permitted. */

#define	LO_CLASSFLAGS	0x0000ffff	/* Class specific flags. */
#define	LO_INITIALIZED	0x00010000	/* Lock has been initialized. */
#define	LO_WITNESS	0x00020000	/* Should witness monitor this lock. */
#define	LO_QUIET	0x00040000	/* Don't log locking operations. */
#define	LO_RECURSABLE	0x00080000	/* Lock may recurse. */
#define	LO_SLEEPABLE	0x00100000	/* Lock may be held while sleeping. */
#define	LO_UPGRADABLE	0x00200000	/* Lock may be upgraded/downgraded. */
#define	LO_DUPOK	0x00400000	/* Don't check for duplicate acquires */
#define	LO_CLASSMASK	0x0f000000	/* Class index bitmask. */
#define LO_NOPROFILE    0x10000000      /* Don't profile this lock */


/*
 * Declare some variables
 */

static pthread_mutexattr_t	 mutexattr;

#ifdef UPTCP_CLIENT
static struct sockaddr_in   server_addr;
#else
static struct sockaddr_in   client_addr;
#endif

/*
 * Thread related
 */ 

thread_wrapper	thread_list[THREAD_LIST_SIZE];
unsigned char	thread_index[THREAD_INDEX_SIZE]; 

int		uptcpip_thread_count = 0;
pthread_mutex_t uptcpip_thread_count_mutex  = PTHREAD_MUTEX_INITIALIZER;

extern struct timeval global_roi_start_tv;

int		in_tcpip_roi = 0;

/*
 * Function Implementation
 */ 
void enter_tcpip_roi()
{
	int i;
	for(i = 0; i < uptcpip_thread_count; i++)
		thread_list[i].last_active_time = global_roi_start_tv;
	in_tcpip_roi = 1;
}

void exit_tcpip_roi()
{
	//NOP
	in_tcpip_roi = 0;
}


int pthread_create_wrapper(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg, int type)
{
	thread_wrapper * tw_ptr;

	int res = pthread_create(thread, attr, start_routine, arg);	
	
	/* initialize thread_wrapper for the created thread */
	if(res == 0){
		pthread_mutex_lock(&uptcpip_thread_count_mutex);

		tw_ptr = &thread_list[uptcpip_thread_count];
		tw_ptr->ptr = *thread;
		tw_ptr->id = uptcpip_thread_count; //start from "1"
		tw_ptr->type = type;
		gettimeofday(&(tw_ptr->last_active_time), NULL);
		
		int hash = (unsigned long)(*thread) % THREAD_INDEX_SIZE;
		while(thread_index[hash] != 0xff)
			hash = (hash + 1) % THREAD_INDEX_SIZE;
		thread_index[hash] = tw_ptr->id;
		
		uptcpip_thread_count ++;

		pthread_mutex_unlock(&uptcpip_thread_count_mutex);
	}
	
	return res;
}


int host_thread_get_uptcp_tid()
{
	pthread_t self = pthread_self();
	
	int hash = (unsigned long)self % THREAD_INDEX_SIZE;
	while(thread_index[hash] == 0xff || self != thread_list[thread_index[hash]].ptr)
		hash = (hash + 1) % THREAD_INDEX_SIZE;

	return thread_index[hash];
}


void host_thread_enter_wait_region()
{
	if(!in_tcpip_roi)
		return;

	struct timeval cur_time;
	pthread_t self = pthread_self();
	
	int hash = (unsigned long)self % THREAD_INDEX_SIZE;
	while(thread_index[hash] == 0xff || self != thread_list[thread_index[hash]].ptr)
		hash = (hash + 1) % THREAD_INDEX_SIZE;

	thread_wrapper  *ptr = &(thread_list[thread_index[hash]]);

	/* enter wait region*/
	gettimeofday(&(cur_time), NULL);

	unsigned long long  interval = (cur_time.tv_sec * 1000000 + cur_time.tv_usec) - (ptr->last_active_time.tv_sec * 1000000 + ptr->last_active_time.tv_usec);
	ptr->active_time += interval;
}


void host_thread_exit_wait_region()
{
	if(!in_tcpip_roi)
		return;

	pthread_t self = pthread_self();
	
	int hash = (unsigned long)self % THREAD_INDEX_SIZE;
	while(thread_index[hash] == 0xff || self != thread_list[thread_index[hash]].ptr)
		hash = (hash + 1) % THREAD_INDEX_SIZE;

	thread_wrapper  *ptr = &(thread_list[thread_index[hash]]);

	/* exit wait region*/
	gettimeofday(&(ptr->last_active_time), NULL);
}


int host_thread_enter_tcpip_region(int type)
{
	if(!in_tcpip_roi)
		return 0;

	pthread_t self = pthread_self();
	
	int hash = (unsigned long)self % THREAD_INDEX_SIZE;
	while(thread_index[hash] == 0xff || self != thread_list[thread_index[hash]].ptr)
		hash = (hash + 1) % THREAD_INDEX_SIZE;

	thread_wrapper  *ptr = &(thread_list[thread_index[hash]]);
	if(ptr->type == THREAD_TYPE_APP)
		ptr->type  = type;

	/* enter tcpip region*/
	gettimeofday(&(ptr->last_tcpip_time), NULL);
	
	return ptr->id;
}


int host_thread_exit_tcpip_region(int thread_id)
{
	if(!in_tcpip_roi)
		return 0;

	struct timeval tv;
	thread_wrapper *ptr = &(thread_list[thread_id]);

	gettimeofday(&tv, NULL);
	
	unsigned long long  interval = (tv.tv_sec * 1000000 + tv.tv_usec)- (ptr->last_tcpip_time.tv_sec * 1000000 + ptr->last_tcpip_time.tv_usec);

	ptr->intcpip_time += interval;
	
	return 0;

}


int host_thread_create(void* (*routine)(void*), void* t_args, int type)
{
	pthread_t *pthread;
	
	pthread = (pthread_t*) malloc(sizeof(pthread_t));
	if(!pthread){
		printf("hif_alloc_ring_and_thread(): malloc() for pthread failed\n");
		return -1;
	}
	memset(pthread, 0, sizeof(pthread_t));
		
	if(pthread_create_wrapper(pthread, NULL, routine, t_args, type) < 0){
		printf("hif_alloc_ring_and_thread(): pthread_create() failed\n");
		free(pthread);
		return -1;
	}
	
	return 0;
}


int host_usleep(int num)
{
	usleep(num);
	return 0;
}




int host_setup_link(int port) 
{
	int link_fd;
	
	/* Step.2 bind & listen  */	
        struct sockaddr_in      sin;

	if((link_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("host_setup_link(): socket() fail, errno = %d\n", errno);
		return -1;
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = PF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);	

       if (bind(link_fd, (struct sockaddr *) &sin, sizeof(sin)) == -1) { 
		printf("host_setup_link(): bind() fail\n");
		return -1; 
        } 
	
	return link_fd;
}


int  host_connect_link(int port) 
{
	int 	link_fd;

	/* Step.0 create a socket  */	
	if((link_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("host_connect_link(): socket() fail, errno = %d\n", errno);
		return -1;
	}

	/* Step.1 connect to server  */	
#ifdef UPTCP_CLIENT
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(0x7f000001);
        server_addr.sin_port = htons(port);	
#else
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = htonl(0x0a0a0a01);
        client_addr.sin_port = htons(port);	
#endif

	return link_fd;
}

int host_link_send(int link_fd, const void * buf, size_t len)
{
	int res = 0;
#ifdef UPTCP_CLIENT
	res = sendto(link_fd, buf, len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
#else
	res = sendto(link_fd, buf, len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
#endif
	if(res < 0)
		printf("host_link_send: errno = %d\n", errno);
	return res;
}

int host_link_recv(int link_fd,void *buf, size_t len)
{
	int res;
	
	res = recvfrom(link_fd, buf, len, 0, NULL, NULL);

	if(res < 0)
		printf("host_link_recv(): res = %d, errno = %d\n", res, errno);
	return res;
}


/* host memory management */
void* host_malloc(size_t size, int align)
{
	if(align == -1)
		return malloc(size);
	else return memalign(align, size);
}

void host_free(void * ptr)
{
	free(ptr);
}


/*
 * mutex
 */
void host_pthread_mutexattr_init()
{
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);	
}


void* host_pthread_mutex_init(int flag)
{
	pthread_mutex_t  *p_mutex;

	p_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if((flag & LO_RECURSABLE) != 0)
		pthread_mutex_init(p_mutex, &mutexattr);
	else 	pthread_mutex_init(p_mutex, NULL);

	return p_mutex;
}

void host_pthread_mutex_destory(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_mutex_t *p_mutex = (pthread_mutex_t*)p_lo->ext_lock;

	pthread_mutex_destroy(p_mutex);

	free(p_mutex);
}

void host_pthread_mutex_lock(void* lock)
{
	int error;
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_mutex_t *p_mutex = (pthread_mutex_t*)p_lo->ext_lock;

	if(p_lo->lo_flags & LO_RECURSABLE){
		error = pthread_mutex_lock(p_mutex);
		p_lo->lo_data ++;		
		p_lo->lo_owner = (unsigned long int)pthread_self();
		if(error){
			printf("panic\n");
			exit(-1);
		}
	}else{
		pthread_mutex_lock(p_mutex);
		if(p_lo->lo_data != 0){
			printf("panic\n");
			exit(-1);
		}		
		p_lo->lo_data ++;		
		p_lo->lo_owner = (unsigned long int)pthread_self();
	}

	
}


int host_pthread_mutex_trylock(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_mutex_t *p_mutex = (pthread_mutex_t*)p_lo->ext_lock;
	
	int error, retval;

	if(p_lo->lo_flags & LO_RECURSABLE){
		error = pthread_mutex_trylock(p_mutex);
		if(error == 0){
			p_lo->lo_data ++;			
			p_lo->lo_owner = (unsigned long int)pthread_self();
			retval = 1;
		}else{
			retval = 0;
		}
		
	} else {
		error = pthread_mutex_trylock(p_mutex);
		if(error == 0){
			if(p_lo->lo_data != 0){
				printf("panic\n");
				exit(-1);
			}			
			p_lo->lo_data ++;						
			p_lo->lo_owner = (unsigned long int)pthread_self();
			retval = 1;
		}else{ 
			retval = 0;
		}
	}
	return retval;
}


void host_pthread_mutex_unlock(void* lock)
{
	int	error;
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_mutex_t *p_mutex = (pthread_mutex_t*)p_lo->ext_lock;
	
	if(p_lo->lo_flags & LO_RECURSABLE){
		p_lo->lo_data --;
		p_lo->lo_owner = (unsigned long int)pthread_self();
		error = pthread_mutex_unlock(p_mutex);
		if(error){
			printf("panic\n");
			exit(-1);
		}
	}else{
		if(p_lo->lo_data != 1){
			printf("panic\n");
			exit(-1);
		}
		p_lo->lo_data --;
		p_lo->lo_owner = (unsigned long int)pthread_self();
		pthread_mutex_unlock(p_mutex);
	}
}



void* host_pthread_rwlock_init()
{
	pthread_rwlock_t  *p_rwlock;

	p_rwlock = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t));
	pthread_rwlock_init(p_rwlock, NULL);

	return p_rwlock;	
}


int host_pthread_rwlock_destory(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_rwlock_t *p_rwlock = (pthread_rwlock_t*)p_lo->ext_lock;
	
	int error = pthread_rwlock_destroy(p_rwlock);

	free(p_rwlock);

	return (error == 0) ? 1 : 0;
}


void host_pthread_rwlock_wlock(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_rwlock_t *p_rwlock = (pthread_rwlock_t*)p_lo->ext_lock;
	
	pthread_t  tid;

	if(p_lo->lo_flags & LO_RECURSABLE){
		tid = pthread_self();

		if(p_lo->lo_owner == tid && p_lo->lo_data != 0){
			p_lo->lo_data ++; 
		} else { /*!= tid*/
			pthread_rwlock_wrlock(p_rwlock);			
			p_lo->lo_owner = tid;
			p_lo->lo_data = 1;
		}

	} else {
		pthread_rwlock_wrlock(p_rwlock);
		if(p_lo->lo_data != 0){
			printf("panic\n");
			exit(-1);
		}		
		p_lo->lo_data ++;		
		p_lo->lo_owner = (unsigned long int)pthread_self();
	}
}


void host_pthread_rwlock_wunlock(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_rwlock_t *p_rwlock = (pthread_rwlock_t*)p_lo->ext_lock;

	pthread_t tid;

	if(p_lo->lo_flags & LO_RECURSABLE){
		tid = pthread_self();
	
		if(p_lo->lo_owner == tid){
			if(p_lo->lo_data == 1){
				p_lo->lo_owner = 0;
				p_lo->lo_data = 0;
				pthread_rwlock_unlock(p_rwlock);
			} else {
				p_lo->lo_data --;
			}
		} else { /*error, lock can be unlock by owner*/
			printf("host_pthread_rwlock_wunlck(): ERROR, try to unlock an unacquired lock\n");
		}
		
	} else {
		if(p_lo->lo_data != 1){
			printf("panic\n");
			exit(-1);
		}
		p_lo->lo_data --;	
		p_lo->lo_owner = 0;
		pthread_rwlock_unlock(p_rwlock);
	}
	
}


void host_pthread_rwlock_rlock(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_rwlock_t *p_rwlock = (pthread_rwlock_t*)p_lo->ext_lock;

	int error;
	
	if(p_lo->lo_flags & LO_RECURSABLE){
		error = pthread_rwlock_rdlock(p_rwlock);
		if(error){
			printf("panic\n");
			exit(-1);
		}
	} else {
		pthread_rwlock_rdlock(p_rwlock);
		p_lo->lo_data ++;				
	}
}


void host_pthread_rwlock_runlock(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_rwlock_t *p_rwlock = (pthread_rwlock_t*)p_lo->ext_lock;

	int error;

	if(p_lo->lo_flags & LO_RECURSABLE){
		error = pthread_rwlock_unlock(p_rwlock);
		if(error){
			printf("panic\n");
			exit(-1);
		}
	}else{
		p_lo->lo_data --;
		pthread_rwlock_unlock(p_rwlock);
	}
		
}


int host_pthread_rwlock_tryrlock(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_rwlock_t *p_rwlock = (pthread_rwlock_t*)p_lo->ext_lock;
	
	int error, retval;

	if(p_lo->lo_flags & LO_RECURSABLE){
		error = pthread_rwlock_tryrdlock(p_rwlock);
		if(error == 0){
			retval = 1;
		}else{
			retval = 0;
		}
		
	} else {
		error = pthread_rwlock_tryrdlock(p_rwlock);
		if(error == 0){
			p_lo->lo_data ++;						
			retval = 1;
		}else{ 
			retval = 0;
		}
	}
	return retval;
	
}


int host_pthread_rwlock_trywlock(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	
	pthread_rwlock_t *p_rwlock = (pthread_rwlock_t*)p_lo->ext_lock;

	int error;
	pthread_t  tid;

	if(p_lo->lo_flags & LO_RECURSABLE){
		tid = pthread_self();
		
		if(p_lo->lo_owner == tid && p_lo->lo_data != 0){
			p_lo->lo_data ++; 
			error = 0;
		} else { /*!= tid*/
			error = pthread_rwlock_trywrlock(p_rwlock);			
			if(error == 0){
				p_lo->lo_owner = tid;
				if(p_lo->lo_data != 0){
					printf("panic\n");
					exit(-1);
				}
				p_lo->lo_data = 1;
			}
		}
	} else {
		error = pthread_rwlock_trywrlock(p_rwlock);
		if(error == 0){
			if(p_lo->lo_data != 0){
				printf("panic\n");
				exit(-1);
			}			
			p_lo->lo_data ++;						
			p_lo->lo_owner = (unsigned long int)pthread_self();
		}
		
	}

	return (error == 0) ? 1 : 0;

}


void host_pthread_rwlock_unlock(void* lock)
{
	struct lock_object *p_lo = (struct lock_object *)lock;
	pthread_t tid = pthread_self();

	if(p_lo->lo_owner == tid)
		host_pthread_rwlock_wunlock(lock);
	else host_pthread_rwlock_runlock(lock);
}


/*
 * condition
 */
void* host_pthread_cond_init()
{
	pthread_cond_t  *p_cond;

	p_cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(p_cond, NULL);

	return p_cond;	
}



void host_pthread_cond_destroy(void* cond)
{
	pthread_cond_t *p_cond = (pthread_cond_t*)cond;
	
	pthread_cond_destroy(p_cond);

	free(p_cond);
}


int host_pthread_cond_wait(void* cond, void* lock)
{
	pthread_cond_t *p_cond = (pthread_cond_t*)cond;
	struct lock_object *p_lo = (struct lock_object *)lock;
	pthread_mutex_t *p_mutex = (pthread_mutex_t*)p_lo->ext_lock;
	int error;
	
	host_thread_enter_wait_region();
	
	if(p_lo->lo_flags & LO_RECURSABLE){
		p_lo->lo_data --;
		p_lo->lo_owner = (unsigned long int)pthread_self();
	}else{
		if(p_lo->lo_data != 1){
			printf("panic\n");
			exit(-1);
		}
		p_lo->lo_data --;
		p_lo->lo_owner = (unsigned long int)pthread_self();
	}

	error = pthread_cond_wait(p_cond, p_mutex);

	if(p_lo->lo_flags & LO_RECURSABLE){
		p_lo->lo_data ++;		
		p_lo->lo_owner = (unsigned long int)pthread_self();
	}else{
		if(p_lo->lo_data != 0){
			printf("panic\n");
			exit(-1);
		}		
		p_lo->lo_data ++;		
		p_lo->lo_owner = (unsigned long int)pthread_self();
	}

	host_thread_exit_wait_region();

	return (error == 0) ? 0 : 1;	
}


void host_pthread_cond_signal(void* cond)
{
	pthread_cond_t *p_cond = (pthread_cond_t*)cond;

	pthread_cond_signal(p_cond);
}

