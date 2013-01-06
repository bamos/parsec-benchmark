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
 *	@(#)host_daemon.c	(Princeton) 11/10/2010
 *
 * *    A daemon for UpTCP. The daemon uses a local Unix domain to 
 *      communicate with UpTCP's Socket API Layer (SAL). The daemon
 *      is responsible for 
 *
 *     1) global initialization (mbuf, ifnet etc.)
 *     2) setup interrupt handler thread
 *     3) setup timer thread
 *     4) create a server socket   
 *  
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netpacket/packet.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/time.h>

#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <pthread.h>
#include <sys/shm.h>

#include <signal.h>


#include "uptcp_daemon.h"
#include "uptcp_api.h"


#define AUTO_EXIT_FOR_DEBUG

extern unsigned long long stat_raw_cnt;
extern unsigned long long stat_raw_time;
extern unsigned long long stat_rx_cnt;
extern unsigned long long stat_rx_time;
extern unsigned long long stat_sock_cnt;
extern unsigned long long stat_sock_time;


extern u_long sb_max;
extern u_long tcp_sendspace;
extern u_long tcp_recvspace;

extern int uptcp_initialized;
extern void uptcp_init(); 

extern void clock_intr_handler(int sig);

extern int bsd_syscall_socket(int sockfd, struct sal_socket_args *args/*int domain, int type, int protocol*/);
extern int bsd_syscall_bind(int sockfd, struct sal_bind_args *args/* caddr_t name, int namelen*/);
extern int bsd_syscall_listen(int sockfd, struct sal_listen_args *args/* int  backlog*/);
extern int bsd_syscall_accept(int sockfd, struct sal_accept_args *args, struct sal_accept_res *retval/* struct sockaddr *name, socklen_t *anamelen*/);
extern int bsd_syscall_connect(int sockfd, struct sal_connect_args *args/* caddr_t name, int namelen*/);
extern int bsd_syscall_socketpair(int sockfd, struct sal_socketpair_args *args/*int domain, int type, int protocol, int *rsv*/);
extern int bsd_syscall_sendto(int sockfd, struct sal_sendto_args *args/* caddr_t buf, size_t len, int flags, caddr_t  to, int tolen*/);
extern int bsd_syscall_send(int sockfd, struct sal_send_args *args,  char *sendbuf/* caddr_t buf, int len, int flags*/);
extern int bsd_syscall_sendmsg(int sockfd, struct sal_sendmsg_args *args/* caddr_t msg, int flags*/);
extern int bsd_syscall_recvfrom(int sockfd, struct sal_recvfrom_args *args/* caddr_t buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlenaddr*/);
extern int bsd_syscall_recv(int sockfd, struct sal_recv_args *args, char* recvbuf/*  caddr_t buf, int  len,  int flags*/);
extern int bsd_syscall_recvmsg(int sockfd, struct sal_recvmsg_args *args/* struct msghdr *msg, int  flags*/);
extern int bsd_syscall_shutdown(int sockfd, struct sal_shutdown_args *args/* int how*/);
extern int bsd_syscall_setsockopt(int sockfd, struct sal_setsockopt_args *args/* int level, int name, caddr_t val, int valsize*/);
extern int bsd_syscall_getsockopt(int sockfd, struct sal_getsockopt_args *args/* int level, int name, void * val, socklen_t *avalsize*/);
extern int bsd_syscall_getsockname(int sockfd, struct sal_getsockname_args *args/* struct sockaddr *asa, socklen_t *alen*/) ;
extern int bsd_syscall_getpeername(int sockfd, struct sal_getpeername_args *args/* struct sockaddr *asa, socklen_t *alen*/);


//#define UPTCP_DAEMON_DEBUG
#ifndef UPTCP_DAEMON_DEBUG
#define UPTCP_DAEMON_DEBUG_PRINT(fmt, args...)		
#else
#define UPTCP_DAEMON_DEBUG_PRINT(fmt, args...)		\
	do{									\
		char	str[256];						\
		sprintf(str, "[DAMON] %s", (fmt));	\
		printf(str, ## args);			\
	}while(0);							
#endif

int		gblopt_timer = 1;    /* option for setuping global timer */
pthread_mutex_t	global_mutex = PTHREAD_MUTEX_INITIALIZER;		/**/


char *req_descriptor [] ={ 
	"\0",
	"SOCKET",
	"BIND",
	"LISTEN",
	"ACCEPT",
	"CONNECT",
	"SOCKETPAIR",
	"SENDTO",
	"SEND",
	"SENDMSG",
	"RECVFROM",
	"RECV",
	"RECVMSG",
	"SHUTDOWN",
	"SETSOCKOPT",
	"GETSOCKOPT",
	"GETSOCKNAME",
	"GETPEERNAME",
	"SOCKFD",
	"MALLOC",
	"FREE"
};


void* connection_handler(void * accept_sockfd);
void* interrupt_handler(void* data);

void display_errno(int num);

void usage(char* prog)
{
  	printf("usage: %s [-h] [-t]\n",prog);
  	printf("-t\t\tset global timer\n");	
  	printf("-h \t\t\thelp\n");
}

int main(int argc, char ** argv)
{
	int server_sockfd, accept_sockfd;
	struct sockaddr_un serveraddr;
	int    res;
	char ch;

	UPTCP_DAEMON_DEBUG_PRINT(">> main()\n")

	/* parse the args */
	opterr = 0;
	optind = 1;
	while (-1 != (ch = getopt(argc, argv, "tphx:r:b:"))) {
		switch (ch) {
			case 't':
				gblopt_timer = 0;
				break;
			case 'b':
				sb_max = atoi(optarg) << 10;
				break;
			case 'x':
				tcp_sendspace = atoi(optarg) << 10;
				break;
			case 'r':
				tcp_recvspace = atoi(optarg) << 10;
				break;
			case 'h':
				usage(argv[0]);
				return -1;
			case '?':
				printf("main(): Unknown option `-%c'.\n", optopt);
				usage(argv[0]);
				return -1;
	    	}
	}	

	/* initialize all TCP/IP stack */
	if(!uptcp_initialized){
		/* initialize TCP/IP stack */
		uptcp_init();			

		/* setup clock */
		if(gblopt_timer){
		
			struct itimerval old, new;

			/* Establish a handler for SIGALRM signals. */
			signal (SIGALRM, clock_intr_handler);
		  
			new.it_interval.tv_usec = 10000; /* 10ms*/
			new.it_interval.tv_sec = 0;
			new.it_value.tv_usec = 10000;
			new.it_value.tv_sec = 0;
			if (setitimer (ITIMER_REAL, &new, &old) < 0){
				printf("main():  setitimer() failed\n");			
			}
		}

		uptcp_initialized = 1;		
	}
	printf("[sb_max, tcp_sendspace, tcp_recvspace] = %dK %dK %dK\n", 
             (sb_max >> 10),(tcp_sendspace >> 10), (tcp_recvspace >> 10));

	if(unlink(SERVER_PATH) < 0){
		printf("main(): unlink() failed\n");
	}
	
	/* create a Unix domain server socket */
	server_sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (server_sockfd < 0){
		printf("main(): create AF_UNIX socket failed\n");
		return -1;
        }
	
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sun_family = PF_UNIX;
	strcpy(serveraddr.sun_path, SERVER_PATH);
	
	res = bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (res < 0){
		printf("main(): bind() failed\n");
		close(server_sockfd);
		return -1;
	}

	res = listen(server_sockfd, MAX_SAL_CLIENTS);
	if (res < 0){
		printf("main(): listen() failed\n");
		close(server_sockfd);
		return -1;
	}

	while(1){
		pthread_t * pthread;
		
		accept_sockfd = accept(server_sockfd, NULL, NULL);
		if(accept_sockfd < 0){
			printf("main(): accept() failed\n");
			display_errno(errno);
			continue;
		}

		
		/* we create a new thread to handle the accept socket */
		pthread = (pthread_t*) malloc(sizeof(pthread_t));
		if(!pthread){
			printf("main(): malloc() for pthread failed\n");
			close(accept_sockfd);
			continue;
		}
		memset(pthread, 0, sizeof(pthread_t));
		
		if(pthread_create(pthread, NULL, connection_handler, &accept_sockfd) < 0){
			printf("main(): pthread_create() failed\n");
			free(pthread);
			close(accept_sockfd);
			return -1;
		}

		/* wait until the connection_handler thread get accept_sockfd */
		while(accept_sockfd != SOCKFD_USED){
			usleep(10);
		}
		
	}


	return 0;
}


/*
 * Once application sends a socket() requests, a thread with 
 * connection_handler() is created to deal with the application's
 * socket.
 *
 * connection_handler()'s main routines are
 * 1) receive requests from app's SAL via local socket;
 * 2) call corresponding functions based the request type;
 * 3) send results to app's SAL via local socket.
 *
 */
void* connection_handler(void * accept_sockfd)
{
	int 	sockfd = *(int*)accept_sockfd;
	int     shmid;
	char 	*sockbuf = NULL;
	char  *shm_sockbuf = NULL;	
	int  	res, reslen;
	struct sal_api_req *sal_req;
	struct sal_api_res *daemon_res;
	char	shm_key_file[64];
	unsigned long shm_sockbuf_size;
	key_t 	shm_key;
	struct 	shmid_ds shmid_buf;

	sigset_t signal_set;

#ifdef AUTO_EXIT_FOR_DEBUG
	int	can_exit = 0;
#endif
	/*
	* Block all signals in this thread. 
	*/
	sigfillset ( &signal_set );
	pthread_sigmask (SIG_BLOCK, &signal_set, NULL );	

	UPTCP_DAEMON_DEBUG_PRINT(">> connection_handler(): new connection\n")

	*(int*)accept_sockfd = SOCKFD_USED; /* let main thread continue to run */

	sockbuf = (char*)malloc(MAX_SOCKBUF_LEN);

	/*
	 * Although both sal_req and daemon_res point to sockbuf, the sockbuf 
	 * is used exclusively, either sal_api_req for receving or sal_api_res for 
	 * sending results.
	 */
	sal_req = (struct sal_api_req*)sockbuf;
	daemon_res = (struct sal_api_res*)sockbuf;

	while((res = recv(sockfd, sockbuf, MAX_SOCKBUF_LEN, 0)) > 0){

		switch(sal_req->type){
			case UPTCP_REQ_SEND:
				UPTCP_DAEMON_DEBUG_PRINT(">> connection_handler(): sockfd = %d, type = %s, len = %d\n", 
					sockfd,  req_descriptor[sal_req->type], ((struct sal_send_args *)sal_req->args)->len)	
				break;
			default:
				UPTCP_DAEMON_DEBUG_PRINT(">> connection_handler(): sockfd = %d, type = %s\n", 
					sockfd, req_descriptor[sal_req->type]);
		}

		reslen = sizeof(int); /* daemon_res->res*/
		
		/* call freebsd's TCP/IP stack */
		switch(sal_req->type){
			case UPTCP_REQ_SOCKET:
				res = bsd_syscall_socket(sockfd, &sal_req->args);
				break;

			case UPTCP_REQ_BIND	:
				res = bsd_syscall_bind(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_LISTEN:
				res = bsd_syscall_listen(sockfd, &sal_req->args);
#ifdef AUTO_EXIT_FOR_DEBUG
				can_exit = 1;
#endif
				break;
				
			case UPTCP_REQ_ACCEPT:
				res = bsd_syscall_accept(sockfd, &sal_req->args, &daemon_res->retval);
				if(res >= 0)
					reslen += sizeof(struct sal_accept_res);
				break;
				
			case UPTCP_REQ_CONNECT:
				res = bsd_syscall_connect(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_SOCKETPAIR:
				res = bsd_syscall_socketpair(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_SENDTO:
				res = bsd_syscall_sendto(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_SEND:
				res = bsd_syscall_send(sockfd, &sal_req->args, shm_sockbuf);
				break;
				
			case UPTCP_REQ_SENDMSG:
				res = bsd_syscall_sendmsg(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_RECVFROM:
				res = bsd_syscall_recvfrom(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_RECV:
				res = bsd_syscall_recv(sockfd, &sal_req->args, shm_sockbuf);
				break;
				
			case UPTCP_REQ_RECVMSG:
				res = bsd_syscall_recvmsg(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_SHUTDOWN:
				res = bsd_syscall_shutdown(sockfd, &sal_req->args);
#ifdef AUTO_EXIT_FOR_DEBUG
				can_exit = 2;
#endif
				break;
				
			case UPTCP_REQ_SETSOCKOPT:
				res = bsd_syscall_setsockopt(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_GETSOCKOPT:
				res = bsd_syscall_getsockopt(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_GETSOCKNAME:
				res = bsd_syscall_getsockname(sockfd, &sal_req->args);
				break;
				
			case UPTCP_REQ_GETPEERNAME:
				res = bsd_syscall_getpeername(sockfd, &sal_req->args);
				break;

			case UPTCP_REQ_SOCKFD:
				res = sockfd;
				break;

			case UPTCP_REQ_MALLOC:
			    	/* allocate share-memory */
#ifdef UPTCP_CLIENT
				sprintf(shm_key_file, "/tmp/uptcp_shmkey_c%d.dat", sockfd);		
#else
				sprintf(shm_key_file, "/tmp/uptcp_shmkey_s%d.dat", sockfd);		
#endif
				shm_key = ftok(shm_key_file, 1);
				shm_sockbuf_size = *((unsigned long*)(&sal_req->args));

				if((shmid = shmget(shm_key, shm_sockbuf_size, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0){
			        	printf("Error: shmget error\n");
			        	goto err;
				}

			    	if((shm_sockbuf = (void *) shmat(shmid, NULL, 0)) == (void*)-1){
			        	printf("Error: shmat error\n");
			        	goto err;
				}
				res = 0;
				break;

			case UPTCP_REQ_FREE:

				shmctl(shmid, IPC_RMID, &shmid_buf);
				if(shm_sockbuf != NULL){
					shmdt(shm_sockbuf);
					shm_sockbuf = NULL;
				}
				res = 0;
				break;

			default:
				printf("connection_handler(): received SAL_req type error\n");
				break;
		}

		UPTCP_DAEMON_DEBUG_PRINT("~~ connection_handler(): sockfd = %d, type = %s, req__res = %d\n", 
				sockfd, req_descriptor[sal_req->type], res)
			
		/* send results to app's SAL */
		daemon_res->res = res;
		res = send(sockfd, sockbuf, reslen, 0);
		if (res < 0){
			printf("connection_handler: send results failed\n");
			goto failed;
		}

#ifdef AUTO_EXIT_FOR_DEBUG
		if(can_exit == 2){
			//printf("raw = %lld, tim = %lld, %8.2f\n", stat_raw_cnt, stat_raw_time/stat_raw_cnt, stat_raw_time/stat_raw_cnt/2600.0);
			//printf("rx = %lld,  tim = %lld, %8.2f\n", stat_rx_cnt, stat_rx_time/stat_rx_cnt, stat_rx_time/stat_rx_cnt/2600.0);
			//if(stat_sock_cnt != 0)
			//	printf("sock = %lld, tim = %lld, %8.2f\n", stat_sock_cnt, stat_sock_time/stat_sock_cnt, stat_sock_time/stat_sock_cnt/2600.0);
			//exit(-1); /*just for DEBUG: leave memory unfree*/
			goto done;
		}
#endif
	}

failed:
	if (res < 0){
		printf("connection_handler(): recv() failed\n");
		close(sockfd);
		free(sockbuf);
	}
err:

done:
	close(sockfd);
	if(sockbuf != NULL)
		free(sockbuf);
	
	exit(-1);
	pthread_exit(NULL);
	return NULL;	
}


void* interrupt_handler(void* data)
{
	pthread_exit(NULL);
	return NULL;
}



void display_errno(int num)
{
	switch(errno){
	case EAGAIN: 		printf("errno: EAGAIN\n"); break;
	case EBADF: 		printf("errno: EBADF\n"); break;
	case ECONNABORTED: printf("errno: ECONNABORTED\n"); break;
	case EINTR: 		printf("errno: EINTR\n"); break;
	case EINVAL: 		printf("errno: EINVAL\n"); break;
	case EMFILE: 		printf("errno: EMFILE\n"); break;
	case ENFILE: 		printf("errno: ENFILE\n"); break;
	case ENOTSOCK: 	printf("errno: ENOTSOCK\n"); break;
	case EOPNOTSUPP: 	printf("errno: EOPNOTSUPP\n"); break;
	case EFAULT: 		printf("errno: EFAULT\n"); break;
	case ENOBUFS: 	printf("errno: ENOBUFS\n"); break;
	case ENOMEM: 	printf("errno: ENOMEM\n"); break;			
	case EPROTO: 		printf("errno: EPROTO\n"); break;
	case EPERM: 		printf("errno: EPERM\n"); break;
	default: 			printf("errno: Unknown\n");
	}
	
}



/*
 * Backup Codes
 */
 #if 0
	int                 rawsock_id;
    	/* register signal handler */
    	struct sigaction sa[2];

	const char *   key_file = "/tmp/uptcp_key.dat";		
	int     	    msqid;
	key_t           msg_key;
	int                tmpfd;
	struct mymsgbuf   msg_recvbuf;
	int                error;
	char             sockbuf[MAX_MSGSIZE];
	int               pktlen;

        /* create a raw socket */		
	rawsock_id = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));
	if(rawsock_id == -1){
		printf("Cann't create a packet socket\n");
		return -1;
	}

        /* create a msgqueue */
    	if((tmpfd = open(key_file, O_CREAT | O_RDWR, S_IRWXU)) < 0){
        	printf("Error: open keyFile error\n");
        	return(-1);
    	}
    	close(tmpfd);

    	msg_key = ftok(key_file, 1);

    	if((msqid = msgget(msg_key, (IPC_CREAT | 0666) )) < 0){
        	printf("Error:  msgget error\n");
		switch(errno){
		case EACCES: printf("EACCESS\n"); break;
		case EEXIST: printf("EEXIST\n"); break;
		case ENOENT: printf("ENOENT\n"); break;
		case ENOMEM: printf("ENOMEM\n"); break;
		case ENOSPC: printf("ENOSPC\n"); break;
		}
        	return(-1);
    	}       

	

	/* receive msg */	
	while(1) {

		/* check msg queue */
		error = msgrcv(msqid, &msg_recvbuf, MAX_MSGSIZE, 0, IPC_NOWAIT);
		if(error < 0 && errno != ENOMSG){
			printf("ERROR: msgrcv\n");
			switch(errno){
			case E2BIG: printf("E2BIG\n"); break;
			case EACCES: printf("EACCES\n"); break;
			case EAGAIN: printf("EAGAIN\n"); break;
			case EFAULT: printf("EFAULT\n"); break;
			case EIDRM: printf("EIDRM\n"); break;
			case EINTR: printf("EINTR\n"); break;
			case EINVAL: printf("EINVAL\n"); break;
			case ENOMSG: printf("ENOMSG\n"); break;			
			default: printf("errno=%d\n", errno); break;
			}			
			return -1;
		} 

		if(error >= 0){
			/* we receive a msg */
			switch(msg_recvbuf.mtype){
			case UPTCP_REQ_SEND:
		  		break;
			
			case UPTCP_REQ_HASHTBL:

			  	break;

			default:
				break;
			}
		}

		/* check incoming data from raw socket*/
		while ((pktlen = read (rawsock_id, sockbuf, MAX_MSGSIZE)) > 0){
			//lookup hash table to find a msgid
			//send to up layer via msgid
			//printf("pktlen = %d\n", pktlen);
			
		}
	}
		



	close(msqid);
	
#endif



/*
 *  Discard the following codes
 */

#if 0
/*
 * Signals 
 */
int  daemon_SIGUSR1_raised = 0; /* for request */
int  daemon_SIGUSR2_raised = 0; /* for interrupt */
#endif //0

#if 0
int main()
{
    	memset(sa, 0, 2 * sizeof(sigaction));
    	sa[0].sa_handler = daemon_SIGUSR1_handler;
    	sa[1].sa_handler = daemon_SIGUSR2_handler;
    	sigaction(SIGUSR1, &sa[0], NULL);
    	sigaction(SIGUSR2, &sa[1], NULL);
    
    	for(;;){
        	if(!daemon_SIGUSR1_raised && !daemon_SIGUSR2_raised 
           	&& !daemon_SIGALRM_raised){
            		pause();
        	}

        	if(daemon_SIGUSR1_raised){
            	daemon_SIGUSR1_raised = 0;
            		foward_signal_to_request_handler_thread();
        	}  
   
        	if(daemon_SIGUSR2_raised){
            	daemon_SIGUSR2_raised = 0;
            		foward_signal_to_interrupt_handler_thread();
        	}  
    	}
}
#endif //0

#if 0
void daemon_SIGUSR1_handler()
{
     daemon_SIGUSR1_raised = 1; /* for request */
}

void daemon_SIGUSR2_handler()
{
     daemon_SIGUSR2_raised = 1; /* for interrupt */
}


int  foward_signal_to_request_handler_thread()
{
    int i;

    for(i = 0; i < UPTCPIP_MAX_SOCKET; i ++){
        if(req_queue[i].state == UPTCPIP_ACTIVE){
        }
    }

    return 0;
}

int  foward_signal_to_interrupt_handler_thread()
{
    return 0;
}

int  handle_timer()
{
    return 0;
}
#endif //0




 
