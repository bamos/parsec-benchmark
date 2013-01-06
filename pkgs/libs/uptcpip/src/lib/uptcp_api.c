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
 *	@(#)uptcp_api.c	(Princeton) 11/10/2010
 *
 *      A library for UpTCP's Socket API Layer (SAL). SAL uses a local Unix domain to 
 *      communicate with UpTCP's deamon process. This library has a global variable
 *      "SAL_local_socket" which is a file descriptor of local socket. 
 *  
 */

#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>

#include "uptcp_daemon.h"
#include "host_serv.h"


int	mp_ncpus;
int	mp_maxid;

extern void vnet_hif_init(const void *unused );
extern void* clock_intr_handler(void* arg);
extern int bsd_uptcp_init();

extern int bsd_syscall_socket(int sockfd,int domain, int type, int protocol);
extern int bsd_syscall_bind(int sockfd, char* name, int namelen);
extern int bsd_syscall_listen(int sockfd, int  backlog);
extern int bsd_syscall_accept(int sockfd, int newfd, struct sockaddr *name, socklen_t *anamelen);
extern int bsd_syscall_connect(int sockfd, char* name, int namelen);
extern int bsd_syscall_socketpair(int sockfd, int domain, int type, int protocol, int *rsv);
extern int bsd_syscall_sendto(int sockfd, char* buf, size_t len, int flags, char*  to, int tolen);
extern int bsd_syscall_send(int sockfd, char* buf, int len, int flags);
extern int bsd_syscall_sendmsg(int sockfd, char* msg, int flags);
extern int bsd_syscall_recvfrom(int sockfd, char* buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlenaddr);
extern int bsd_syscall_recv(int sockfd, char* buf, int  len,  int flags);
extern int bsd_syscall_recvmsg(int sockfd, struct msghdr *msg, int  flags);
extern int bsd_syscall_shutdown(int sockfd, int how);
extern int bsd_syscall_setsockopt(int sockfd, int level, int name, char* val, int valsize);
extern int bsd_syscall_getsockopt(int sockfd, int level, int name, void * val, socklen_t *avalsize);
extern int bsd_syscall_getsockname(int sockfd, struct sockaddr *asa, socklen_t *alen) ;
extern int bsd_syscall_getpeername(int sockfd, struct sockaddr *asa, socklen_t *alen);

void uptcp_print_statis();

//#define UPTCP_API_DEBUG
#ifndef UPTCP_API_DEBUG
#define UPTCP_API_DEBUG_PRINT(fmt, ...)		
#else
#define UPTCP_API_DEBUG_PRINT(fmt, args...)		\
	do{									\
		char	str[256];						\
		sprintf(str, "[API] %s", (fmt));	\
		printf(str, ## args);			\
	}while(0);					
#endif


#define SOCKADDR_LINUX2BSD(bsd, len)	\
	do{\
		unsigned char *ptr = (unsigned char*)(bsd);\
		unsigned char ch = (unsigned char)((bsd)->sa_family);	\
		*ptr = (unsigned char)(len);	\
		ptr++;\
		*ptr = (unsigned char)ch;\
	}while(0)
	

#define SOCKADDR_BSD2LINUX(bsd)	\
	do{\
		unsigned char *ptr = (unsigned char*)(bsd);\
		(bsd)->sa_family = (sa_family_t)(*(ptr+1));\
	}while(0)


struct timeval global_roi_start_tv, global_roi_end_tv;

static int uptcp_initialized;
static pthread_mutex_t uptcp_init_mutex  = PTHREAD_MUTEX_INITIALIZER;

static int uptcp_accept_fd = 1024;
static pthread_mutex_t uptcp_accept_fd_mutex  = PTHREAD_MUTEX_INITIALIZER;

/*
 * Implementation
 */ 

void uptcp_init()
{
	pthread_t pthread;

	/* 1. initialize all TCP/IP stack */

	if(!uptcp_initialized){
		/* 1.1 initialize global thread list */
		memset(thread_list, 0, THREAD_LIST_SIZE*sizeof(thread_wrapper));
		memset(thread_index, 0xff, THREAD_INDEX_SIZE);
	
		pthread_mutex_lock(&uptcpip_thread_count_mutex);

		thread_wrapper * tw_ptr;
		tw_ptr = &thread_list[uptcpip_thread_count];
		tw_ptr->ptr = pthread_self(); /* main thread */
		tw_ptr->id = uptcpip_thread_count; //start from "0"
		tw_ptr->type = THREAD_TYPE_APP ;
		
		int hash = (unsigned long)tw_ptr->ptr % THREAD_INDEX_SIZE;
		thread_index[hash] = tw_ptr->id;
		
		uptcpip_thread_count ++;

		pthread_mutex_unlock(&uptcpip_thread_count_mutex);


		/* 1.2 initialize mp_maxcpus */
		char * rx_threads;
		rx_threads = getenv ("UPTCPIP_RX_THREADS");

		if (rx_threads != NULL)
		     mp_ncpus = atoi(rx_threads);
		else mp_ncpus = 1;
		
		mp_maxid = mp_ncpus;

		
		/* 1.3 initialize TCP/IP stack */
		bsd_uptcp_init();			

		vnet_hif_init(NULL);

		/* setup clock */
		if(pthread_create_wrapper(&pthread, NULL, clock_intr_handler, NULL, THREAD_TYPE_TIMER) < 0){
			printf("uptcp_init(): pthread_create() failed\n");
		}
		else uptcp_initialized = 1;		
	}


}

void parsec_enter_tcpip_roi()
{
	int i;

	pthread_mutex_lock(&uptcp_init_mutex);
	if(!uptcp_initialized){
		/* initialize TCP/IP stack */
		uptcp_init();			
		uptcp_initialized = 1;		
	}
	pthread_mutex_unlock(&uptcp_init_mutex);

	printf("\n");
	for(i = 0; i < 60; i ++) printf("="); printf("\n");

	printf("  Enter PARSEC TCPIP ROI \n");	

	for(i = 0; i < 60; i ++) printf("="); printf("\n");

	fflush(stdout);

	gettimeofday(&global_roi_start_tv, NULL);
	enter_tcpip_roi();
}

void parsec_exit_tcpip_roi()
{
	int i;
	gettimeofday(&global_roi_end_tv, NULL);
	exit_tcpip_roi();
	uptcp_print_statis();

	for(i = 0; i < 60; i ++) printf("="); printf("\n");

	printf("  Exit PARSEC TCPIP ROI \n");	

	for(i = 0; i < 60; i ++) printf("="); printf("\n\n");

	fflush(stdout);

}

char	* thread_type_str [] = {
	"App",
	"Socket",
	"Rx_Tcpip",
	"Tx_Link",
	"Rx_Link",
	"Timer",
	NULL
};

void uptcp_print_statis()
{
	int i;


	printf("\n");
	for(i = 0; i < 60; i ++) printf("-"); printf("\n");

	printf("      Execution Statistics of u-TCP/IP Stack \n");	

	for(i = 0; i < 60; i ++) printf("-"); printf("\n");

	int not_involved = 0;
	for(i = 0; i < uptcpip_thread_count; i++)
		if(thread_list[i].type == THREAD_TYPE_APP)
			not_involved ++;
 	printf("There are %d threads, %d involved to u-TCP/IP stack\n", uptcpip_thread_count, (uptcpip_thread_count - not_involved));
	printf("(u-TCP/IP_Threads) / (Total_Threads) = %4.2f\%\n", 100.0*((uptcpip_thread_count - not_involved))/uptcpip_thread_count);
	printf("\n");

	unsigned long long  interval = (global_roi_end_tv.tv_sec * 1000000 + global_roi_end_tv.tv_usec) - (global_roi_start_tv.tv_sec * 1000000 + global_roi_start_tv.tv_usec);
	printf("ROI_RUN_TIME = %6.2fs\n", interval/1000000.0);

	printf("\n");
#ifdef PERFORMANCE_TUNING
	printf("%16s%20s%20s%24s\n", "THREAD_TYPE", "ACTIVE_TIME",  "ACTIVE(\%)", "PERF_ROI");
#else
	printf("%16s%20s%20s\n", "THREAD_TYPE", "ACTIVE_TIME",  "ACTIVE(\%)");
#endif

	for(i = 0; i < uptcpip_thread_count; i ++){
		if(thread_list[i].type == THREAD_TYPE_APP)
			continue;
		
		printf("%16s", thread_type_str[thread_list[i].type]);
					
		printf("%13s%6.3fs", " ", thread_list[i].active_time/1000000.0);

		printf("%14s%6.2f", " ", 100.0*thread_list[i].active_time/interval);

#ifdef PERFORMANCE_TUNING
		if(thread_list[i].perf_enter_roi_cnt != 0){
			printf("%10s(Enter:%llu, Exit:%llu)\t%4.3fs / %llu = %4.1fus\n", " ", 
				thread_list[i].perf_roi_time/1000000.0, 
				thread_list[i].perf_enter_roi_cnt,
				thread_list[i].perf_exit_roi_cnt,
				1.0*thread_list[i].perf_roi_time/thread_list[i].perf_exit_roi_cnt);

			printf("\t\t\t%16s %16s %16s\n", "Overhead(us)", "Count", "%");
			int j;
			for(j = 0; j < 100; j++)
				if(thread_list[i].perf_time_dist[j] != 0)
					printf("\t\t\t%16d %16d %16.2f\n",
						j, 
						thread_list[i].perf_time_dist[j], 
						100.0*thread_list[i].perf_time_dist[j]/thread_list[i].perf_exit_roi_cnt);
		}
#endif

		printf("\n");
	}	
	for(i = 0; i < 60; i ++) printf("-"); printf("\n");
	printf("\n");

#ifdef LOCK_PROFILING 
	printf("\n+++++++++++ Lock Profiling Begin ++++++++++\n");


	for(i = 0; i < MAX_LOCK_TRACE; i ++){
		if(lock_profile[i].lock_addr != 0){

			if(lock_profile[i].lock_cond == 0){
				printf("[%d]\t%24s%12u%12llu%12llu\n", i, 
					host_get_lock_name((void*)lock_profile[i].lock_addr),
					lock_profile[i].lock_cnt,
					lock_profile[i].wait_time,
					lock_profile[i].hold_time);
				
				unsigned long long tmask = lock_profile[i].threads_mask;
				int j = 0;
				while(tmask != 0){
					if(tmask & 0x1)
						printf("\t%36s\n", thread_type_str[thread_list[j].type]);
					j++;
					tmask = (tmask >> 1);
				}
#ifdef ENABLE_DUMP_STACK
				stack_trace *p_stack_log = lock_profile[i].stack_log;
				if(p_stack_log != 0){
					int k;
					printf("\t\t\t--------------\n");	
					for(k = 0; k < MAX_STACK_TRACK; k++){
						if(p_stack_log[k].hash != 0){
							printf("\t\t\t%10s:%6d %s\n",
								thread_type_str[thread_list[p_stack_log[k].tid].type],
								p_stack_log[k].cnt,
								p_stack_log[k].str);	
						}
					}
				}
#endif
			}else{
				printf("[%d]\t%18s(cond)%12u%12llu%12llu\n", i, 
					host_get_lock_name((void*)lock_profile[i].lock_cond),
					lock_profile[i].lock_cnt,
					lock_profile[i].wait_time,
					lock_profile[i].hold_time);
			}
		}
	}
	printf("\n+++++++++++ Lock Profiling End ++++++++++\n");
#endif

#ifdef ENABLE_TIME_PROBE

	printf("\n+++++++++++ Time Probe Begin ++++++++++\n");
	printf("\n");
	printf("%5s", " ");
	for(i = 0; i < MAX_TIME_PROBES; i++)
		if(global_time_probes[i].timestamp.tv_sec != 0)
			printf("%2d->%-2d ", i, global_time_probes[i].pre_index); 

	printf("\n");
	printf("%5s", " ");
	for(i = 0; i < MAX_TIME_PROBES; i++)
		if(global_time_probes[i].timestamp.tv_sec != 0)
			printf("%7d", global_time_probes[i].pass_cnt); 

	printf("\n");
	printf("%5s", " ");
	for(i = 0; i < MAX_TIME_PROBES; i++)
		if(global_time_probes[i].timestamp.tv_sec != 0)
			printf("%7.1f", 1.0*global_time_probes[i].pass_time/global_time_probes[i].pass_cnt);

	printf("\n");
	int j;	
	for(j = 0; j < 50; j++){
		printf("%-5d", j);
		for(i = 0; i < MAX_TIME_PROBES; i++)
			if(global_time_probes[i].timestamp.tv_sec != 0)
				printf("%7.1f", 
					global_time_probes[i].time_dist[j], 
					100.0*global_time_probes[i].time_dist[j]/global_time_probes[i].pass_cnt);
		printf("\n");
	}
	printf("\n+++++++++++ Time Probe End ++++++++++\n");
#endif 
}
	
/*
 * @uptcp_accept()
 *
 * A new socket is created when application calls accept().  
 *
 */
int	uptcp_accept(int s, struct sockaddr * name, socklen_t * anamelen)
{
	int    newfd = -1;
	
	UPTCP_API_DEBUG_PRINT(">> uptcp_accept(): sockfd = %d\n", s)

        pthread_mutex_lock(&uptcp_accept_fd_mutex);
	newfd = uptcp_accept_fd;
	uptcp_accept_fd ++;
        pthread_mutex_unlock(&uptcp_accept_fd_mutex);

	/*convert freebsd.sockaddr to linux.sockaddr */
	if(bsd_syscall_accept(s, newfd, name, anamelen) < 0){
		printf("uptcp_accept(): bsd_syscall_accept() failed\n");
		return -1;
	}
	SOCKADDR_BSD2LINUX(name);

	UPTCP_API_DEBUG_PRINT(">> uptcp_accept(): newfd = %d\n", newfd)
	
	return newfd;
}


int	uptcp_bind(int s, const struct sockaddr * name, socklen_t namelen)
{
	UPTCP_API_DEBUG_PRINT(">> uptcp_bind(): sockfd = %d\n", s)
	
	/*convert local.sockaddr to freebsd.sockaddr */
	SOCKADDR_LINUX2BSD(name, namelen);

	return bsd_syscall_bind(s, (void*)name, namelen); 
}

int	uptcp_connect(int s, const struct sockaddr * name, socklen_t namelen)
{
	UPTCP_API_DEBUG_PRINT(">> uptcp_connect(): sockfd = %d\n", s)
	
	/*convert local.sockaddr to freebsd.sockaddr */
	SOCKADDR_LINUX2BSD(name, namelen);

	return bsd_syscall_connect(s, (void*)name, namelen);

}

int	uptcp_getpeername(int s,  struct sockaddr * asa, socklen_t * alen)
{
	return bsd_syscall_getpeername(s, asa, alen);
}


int	uptcp_getsockname(int s, struct sockaddr * asa, socklen_t * alen)
{
	return bsd_syscall_getsockname(s, asa, alen);
}


int	uptcp_getsockopt(int s, int level, int name, void * val, socklen_t * avalsize)
{
	return bsd_syscall_getsockopt(s, level, name, val, avalsize);
}


int	uptcp_listen(int s, int backlog)
{
	UPTCP_API_DEBUG_PRINT(">> uptcp_listen(): sockfd = %d\n", s)

	return bsd_syscall_listen(s, backlog);
}


ssize_t	uptcp_recv(int s, void * buf, size_t len, int flags)
{
	
	UPTCP_API_DEBUG_PRINT(">> uptcp_recv(): sockfd = %d", s)

	int thread_id = host_thread_enter_tcpip_region(THREAD_TYPE_SOCKET);
	ssize_t res = bsd_syscall_recv(s, buf, len, flags);	
	host_thread_exit_tcpip_region(thread_id);
	return res;
}


ssize_t	uptcp_recvfrom(int s, void * buf, size_t len, int flags, struct sockaddr * from, socklen_t * fromlenaddr)
{
	return bsd_syscall_recvfrom(s, buf, len, flags, from, fromlenaddr);
}


ssize_t	uptcp_recvmsg(int s, struct msghdr * msg, int flags)
{
	return bsd_syscall_recvmsg(s, msg, flags);
}


ssize_t	uptcp_send(int s, void * msg, size_t len, int flags)
{
	UPTCP_API_DEBUG_PRINT(">> uptcp_send(): sockfd = %d, len = %d\n", s, len)

	int thread_id = host_thread_enter_tcpip_region(THREAD_TYPE_SOCKET);
	ssize_t res = bsd_syscall_send(s, msg, len, flags);	
	host_thread_exit_tcpip_region(thread_id);
	return res;
}


ssize_t	uptcp_sendto(int s, const void * buf,    size_t len, int flags, const struct sockaddr * to, socklen_t tolen)
{
	return bsd_syscall_sendto(s, (char*)buf, len, flags, (char*)to, tolen);
}


ssize_t	uptcp_sendmsg(int s, const struct msghdr * msg, int flags)
{
	return bsd_syscall_sendmsg(s, (char*)msg, flags);
}


int	uptcp_setsockopt(int s, int level, int name, const void * val, socklen_t valsize)
{
	return bsd_syscall_setsockopt(s, level, name, (char*)val, valsize);
}


int	uptcp_shutdown(int s, int how)
{
	int res;
	UPTCP_API_DEBUG_PRINT(">> uptcp_shutdown(): sockfd = %d\n", s)
		
	res = bsd_syscall_shutdown(s, how);
	if(how == SHUT_RDWR)
		close(s);

	return res;
}


int uptcp_close(int s)
{
	return uptcp_shutdown(s, SHUT_RDWR);
}


int	uptcp_sockatmark(int arg)
{
	printf("FIXME: sockatmark()\n");
	return -1;
}

/*
 * @uptcp_socket()
 *
 * Upon application's socket(), we create a local AF_UNIX socket 
 * to communicate with UpTCP daemon's local socket. 
 */
int uptcp_socket(int domain, int type, int protocol)
{
	int		sockfd;

	UPTCP_API_DEBUG_PRINT(">> uptcp_socket(): (do, type, pro) = (%d, %d, %d)\n", domain, type, protocol)

	/* initialize all TCP/IP stack */
	pthread_mutex_lock(&uptcp_init_mutex);
	if(!uptcp_initialized){
		/* initialize TCP/IP stack */
		uptcp_init();			
		uptcp_initialized = 1;		
	}
	pthread_mutex_unlock(&uptcp_init_mutex);

	if((sockfd = open("/dev/null", O_RDWR)) < 0){
		printf("uptcp_socket(): open() failed\n");
		return -1;
	}		
	
	return bsd_syscall_socket(sockfd, domain, type, protocol);
}


int	uptcp_socketpair(int s, int domain, int type, int protocol, int * rsv)
{	
	return bsd_syscall_socketpair(s, domain, type, protocol, rsv);
}


int     uptcp_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg)
{
	pthread_mutex_lock(&uptcp_init_mutex);
	if(!uptcp_initialized){
		/* initialize TCP/IP stack */
		uptcp_init();			
		uptcp_initialized = 1;		
	}
	pthread_mutex_unlock(&uptcp_init_mutex);

	return pthread_create_wrapper(thread, attr, start_routine, arg, THREAD_TYPE_APP);
}
