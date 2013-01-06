/*-
 * Copyright (c) 1982, 1986, 1989, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * sendfile(2) and related extensions:
 * Copyright (c) 1998, David Greenman. All rights reserved.
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
 *	@(#)uipc_syscalls.c	8.4 (Berkeley) 2/21/94
 */

#include <sys/bsd_cdefs.h>
__FBSDID("$FreeBSD$");

#include "bsd_opt_inet.h"
#include "bsd_opt_inet6.h"
#include "bsd_opt_sctp.h"
#include "bsd_opt_compat.h"

#include <sys/bsd_param.h>
#include <sys/bsd_systm.h>
#include <sys/bsd_kernel.h>
#include <sys/bsd_lock.h>
#include <sys/bsd_mutex.h>
//#include <sys/bsd_sysproto.h>
#include <sys/bsd_malloc.h>
//#include <sys/bsd_filedesc.h>
#include <sys/bsd_event.h>
//#include <sys/bsd_proc.h>
//#include <sys/bsd_fcntl.h>
//#include <sys/bsd_file.h>
//#include <sys/bsd_filio.h>
//#include <sys/bsd_jail.h>
//#include <sys/bsd_mount.h>
#include <sys/bsd_mbuf.h>
#include <sys/bsd_protosw.h>
//#include <sys/bsd_sf_buf.h>
#include <sys/bsd_socket.h>
#include <sys/bsd_socketvar.h>
//#include <sys/bsd_signalvar.h>
//#include <sys/bsd_syscallsubr.h>
//#include <sys/bsd_sysctl.h>
#include <sys/bsd_uio.h>
//#include <sys/bsd_vnode.h>

#include <net/bsd_vnet.h>

//#include <security/audit/audit.h>
//#include <security/mac/mac_framework.h>

//#include <vm/bsd_vm.h>
//#include <vm/vm_object.h>
//#include <vm/vm_page.h>
//#include <vm/vm_pageout.h>
//#include <vm/vm_kern.h>
//#include <vm/vm_extern.h>

#if defined(INET) || defined(INET6)
#ifdef SCTP
#include <netinet/sctp.h>
#include <netinet/sctp_peeloff.h>
#endif /* SCTP */
#endif /* INET || INET6 */

#include <netinet/bsd_tcp_var.h>
#include <netinet/bsd_in.h>
#include <netinet/bsd_in_pcb.h>

#include <host_serv.h>
#include <uptcp_api.h>



#define MAX_SOCKFD_NUM  65536

typedef struct fd_socket fd_socket_t;

struct fd_socket{
	int	fd;
	struct socket *so;
};

static int           fd_num = 0;
static int           total_fd = 0;
static fd_socket_t  *fd_socket_map;


/* UpTCP initializing*/
void uptcp_init(); 

SET_DECLARE(sysinit_set, struct sysinit);





static int sendit(int s, struct msghdr *mp, int flags, int* sendlen);
static int recvit(int s, struct msghdr *mp, void *namelenp, int* recvlen);

static int accept1(int	s, 	int newfd, struct sockaddr	* name, socklen_t	* anamelen, int compat);
//static int do_sendfile(struct sendfile_args *uap, int compat);
//static int getsockname1(struct getsockname_args *uap, int compat);
//static int getpeername1(struct getpeername_args *uap, int compat);
#if 0
/*
 * NSFBUFS-related variables and associated sysctls
 */
int nsfbufs;
int nsfbufspeak;
int nsfbufsused;

SYSCTL_INT(_kern_ipc, OID_AUTO, nsfbufs, CTLFLAG_RDTUN, &nsfbufs, 0,
    "Maximum number of sendfile(2) sf_bufs available");
SYSCTL_INT(_kern_ipc, OID_AUTO, nsfbufspeak, CTLFLAG_RD, &nsfbufspeak, 0,
    "Number of sendfile(2) sf_bufs at peak usage");
SYSCTL_INT(_kern_ipc, OID_AUTO, nsfbufsused, CTLFLAG_RD, &nsfbufsused, 0,
    "Number of sendfile(2) sf_bufs in use");
#endif //0

/*
 * Initialize UpTCP
 * the codes is derived from mi_startup() in sys/kern/init_main.c
 */
void bsd_uptcp_init()
{
	register struct sysinit **sipp;		/* system initialization*/
	register struct sysinit **xipp;		/* interior loop of sort*/
	register struct sysinit *save;		/* bubble*/
	struct sysinit **sysinit, **sysinit_end;

#if defined(VERBOSE_SYSINIT)
	int last;
	int verbose;
#endif

	sysinit = NULL;
	if (sysinit == NULL) {
		sysinit = SET_BEGIN(sysinit_set);
		sysinit_end = SET_LIMIT(sysinit_set);
	}

	/*
	 * Perform a bubble sort of the system initialization objects by
	 * their subsystem (primary key) and order (secondary key).
	 */
	for (sipp = sysinit; sipp < sysinit_end; sipp++) {
		for (xipp = sipp + 1; xipp < sysinit_end; xipp++) {
			if ((*sipp)->subsystem < (*xipp)->subsystem ||
			     ((*sipp)->subsystem == (*xipp)->subsystem &&
			      (*sipp)->order <= (*xipp)->order))
				continue;	/* skip*/
			save = *sipp;
			*sipp = *xipp;
			*xipp = save;
		}
	}

#if defined(VERBOSE_SYSINIT)
	last = SI_SUB_COPYRIGHT;
	verbose = 0;
#if !defined(DDB)
	printf("VERBOSE_SYSINIT: DDB not enabled, symbol lookups disabled.\n");
#endif
#endif

	/*
	 * Traverse the (now) ordered list of system initialization tasks.
	 * Perform each task, and continue on to the next task.
	 *
	 * The last item on the list is expected to be the scheduler,
	 * which will not return.
	 */
	for (sipp = sysinit; sipp < sysinit_end; sipp++) {

		if ((*sipp)->subsystem == SI_SUB_DUMMY)
			continue;	/* skip dummy task(s)*/

		if ((*sipp)->subsystem == SI_SUB_DONE)
			continue;

#if defined(VERBOSE_SYSINIT)
		if ((*sipp)->subsystem > last) {
			verbose = 1;
			last = (*sipp)->subsystem;
			printf("subsystem %x\n", last);
		}
		if (verbose) {
#if defined(DDB)
			const char *name;
			c_db_sym_t sym;
			db_expr_t  offset;

			sym = db_search_symbol((vm_offset_t)(*sipp)->func,
			    DB_STGY_PROC, &offset);
			db_symbol_values(sym, &name, NULL);
			if (name != NULL)
				printf("   %s(%p)... ", name, (*sipp)->udata);
			else
#endif
				printf("   %p(%p)... ", (*sipp)->func,
				    (*sipp)->udata);
		}
#endif //VERBOSE_SYSINIT

		/* Call function */
		(*((*sipp)->func))((*sipp)->udata);

#if defined(VERBOSE_SYSINIT)
		if (verbose)
			printf("done.\n");
#endif
	}

	//panic("Shouldn't get here!");
}

static MALLOC_DEFINE(M_FDSOCK, "fdsock", "fd socket map");

static int fd_socket_init()
{
	int i;
	fd_socket_map = bsd_malloc(MAX_SOCKFD_NUM*sizeof(struct fd_socket), M_FDSOCK, M_WAITOK);

	if(fd_socket_map == NULL)
		return -1;

	total_fd = MAX_SOCKFD_NUM;
	for(i = 0; i < MAX_SOCKFD_NUM; i ++){
		fd_socket_map[i].fd = -1;
		fd_socket_map[i].so = NULL;
	}
	return 0;
}


static void fd_socket_fini()
{
    if(fd_socket_map != NULL)
       bsd_free(fd_socket_map, M_FDSOCK);
}

SYSINIT(fd_socket_init, SI_SUB_MBUF, SI_ORDER_ANY, fd_socket_init, NULL);
SYSUNINIT(fd_socket_fini, SI_SUB_MBUF, SI_ORDER_ANY, fd_socket_fini, NULL);


static int init_fd(int fd, struct socket *so)
{
	int cnt = 0;
	int tmpfd;
	fd_socket_t  * newbuf;

again:
	tmpfd = fd;
	while(fd_socket_map[tmpfd % total_fd].fd != -1
	    && cnt < total_fd){
		tmpfd ++;
		cnt ++;
	}

	if(cnt == total_fd){
		total_fd *= 2;
		newbuf = realloc(fd_socket_map, total_fd*sizeof(struct fd_socket));
		if(newbuf){
			free(fd_socket_map);
			fd_socket_map = newbuf;
			goto again;
		} else {
			printf("init_fd(): realloc () failed\n");
			return -1;
		}
	}

	fd_socket_map[tmpfd].fd = fd;
	fd_socket_map[tmpfd].so = so;	
	
	fd_num ++;    //FIXME: here exists potiontial data race
	return 0;
}

static int free_fd(int fd)
{
	int tmpfd = fd;
	
	if(fd != -1){
		while(fd_socket_map[tmpfd % total_fd].fd != fd)
			tmpfd ++;
	
	        fd_socket_map[tmpfd].fd = -1;
        	fd_socket_map[tmpfd].so = NULL;

	        fd_num --; //FIXME: here exists potiontial data race
	}
	return 0;
}

static struct socket* getsockbyfd(int fd)
{
	int tmpfd = fd;
	int cnt = 0;
	while(fd_socket_map[tmpfd % total_fd].fd != fd
		&& cnt < total_fd){
		tmpfd ++;
		cnt ++;
	}

	/* not find */
	if(cnt == total_fd){
		return NULL;
	} else {
		return fd_socket_map[tmpfd].so;
	}
	
}

#if 0
/*
 * Convert a user file descriptor to a kernel file entry.  A reference on the
 * file entry is held upon returning.  This is lighter weight than
 * fgetsock(), which bumps the socket reference drops the file reference
 * count instead, as this approach avoids several additional mutex operations
 * associated with the additional reference count.  If requested, return the
 * open file flags.
 */
static int
getsock(struct filedesc *fdp, int fd, struct file **fpp, u_int *fflagp)
{
	struct file *fp;
	int error;

	fp = NULL;
	if (fdp == NULL || (fp = fget_unlocked(fdp, fd)) == NULL) {
		error = EBADF;
	} else if (fp->f_type != DTYPE_SOCKET) {
		fdrop(fp, curthread);
		fp = NULL;
		error = ENOTSOCK;
	} else {
		if (fflagp != NULL)
			*fflagp = fp->f_flag;
		error = 0;
	}
	*fpp = fp;
	return (error);
}

#endif //0

/*
 * System call interface to the socket abstraction.
 */
#if defined(COMPAT_43)
#define COMPAT_OLDSOCK
#endif

int
bsd_syscall_socket(int sockfd, int domain, int type, int protocol)
{
	struct socket *so;
	int error;

	/* An extra reference on `fp' has been held for us by falloc(). */
	error = socreate(domain, &so, type, protocol, NULL/*td->ucred*/, NULL/*td*/);
	if (error) {
		return -1;
	} else {
		init_fd(sockfd, so);
	}
	return sockfd;
}


int kern_bind(int fd, struct sockaddr *sa);

/* ARGSUSED */
int
bsd_syscall_bind(int sockfd, caddr_t name, int namelen)
{
	struct sockaddr *sa;
	int error;

	if ((error = getsockaddr(&sa, (caddr_t)name, (size_t)namelen)) != 0)
		return (error);

	error = kern_bind(sockfd, sa);
	
	//bsd_free(sa, M_SONAME);
	return (error);
}

int
kern_bind(fd, sa)
	int fd;
	struct sockaddr *sa;
{
	struct socket *so;
	int error;

#if 0
#ifdef KTRACE
	if (KTRPOINT(td, KTR_STRUCT))
		ktrsockaddr(sa);
#endif
#ifdef MAC
	error = mac_socket_check_bind(td->td_ucred, so, sa);
	if (error == 0)
#endif
#endif //0

	so = getsockbyfd(fd);

	error = sobind(so, sa, NULL);

	return (error);
}

/* ARGSUSED */
int
bsd_syscall_listen(int sockfd, int  backlog)
{
	struct socket *so;
	int error = 0;

	so = getsockbyfd(sockfd);
	
//	CURVNET_SET(so->so_vnet);

	error = solisten(so, backlog, NULL);

//	CURVNET_RESTORE();

	return(error);
}


int kern_accept(int s, int newfd, struct sockaddr **name,   socklen_t *namelen/*, struct file **fp*/);

/*
 * accept1()
 */
static int
accept1(int	s, int newfd, struct sockaddr *name, socklen_t *anamelen, int compat)
{
	struct sockaddr *_name;
	socklen_t _namelen;
	int error;

	if (name == NULL)
		return (kern_accept(s, newfd, NULL, NULL));

	error = copyin(anamelen, &_namelen, sizeof (_namelen));
	if (error)
		return (error);

	error = kern_accept(s, newfd, &_name, &_namelen);

	/*
	 * return a namelen of zero for older code which might
	 * ignore the return value from accept.
	 */
	if (error) {
		(void) copyout(&_namelen,
		    anamelen, sizeof(*anamelen));
		return (error);
	}

	if (error == 0 && _name != NULL) {
#ifdef COMPAT_OLDSOCK
		if (compat)
			((struct osockaddr *)name)->sa_family =
			    name->sa_family;
#endif
		error = copyout(_name, name, _namelen);
	}
	if (error == 0)
		error = copyout(&_namelen, anamelen,
		    sizeof(_namelen));

	//bsd_free(name, M_SONAME);
	return (error);
}

int
kern_accept(int s, int newfd, struct sockaddr **name,  socklen_t *namelen/*, struct file **fp*/)
{
	struct sockaddr *sa = NULL;
	int error;
	struct socket *head, *so;
	int fd;

	if (name) {
		*name = NULL;
		if (*namelen < 0)
			return (EINVAL);
	}

	head = getsockbyfd(s);
	if ((head->so_options & SO_ACCEPTCONN) == 0) {
		error = EINVAL;
		goto done;
	}
#ifdef MAC
	error = mac_socket_check_accept(td->td_ucred, head);
	if (error != 0)
		goto done;
#endif
	fd = newfd; 

	ACCEPT_LOCK();
	if ((head->so_state & SS_NBIO) && TAILQ_EMPTY(&head->so_comp)) {
		ACCEPT_UNLOCK();
		error = EWOULDBLOCK;
		goto noconnection;
	}
	while (TAILQ_EMPTY(&head->so_comp) && head->so_error == 0) {
		if (head->so_rcv.sb_state & SBS_CANTRCVMORE) {
			head->so_error = ECONNABORTED;
			break;
		}
		//host_usleep(100); //FIXME
		
		error = msleep(head->so_timeo_cond, &accept_mtx, PSOCK | PCATCH, 
		    "accept", 0);
		if (error) {
			ACCEPT_UNLOCK();
			goto noconnection;
		}
	}
	if (head->so_error) {
		error = head->so_error;
		head->so_error = 0;
		ACCEPT_UNLOCK();
		goto noconnection;
	}
	so = TAILQ_FIRST(&head->so_comp);
	KASSERT(!(so->so_qstate & SQ_INCOMP), ("accept1: so SQ_INCOMP"));
	KASSERT(so->so_qstate & SQ_COMP, ("accept1: so not SQ_COMP"));

	/*
	 * Before changing the flags on the socket, we have to bump the
	 * reference count.  Otherwise, if the protocol calls sofree(),
	 * the socket will be released due to a zero refcount.
	 */
	SOCK_LOCK(so);			/* soref() and so_state update */
	soref(so);			/* file descriptor reference */

	TAILQ_REMOVE(&head->so_comp, so, so_list);
	head->so_qlen--;
	so->so_state |= (head->so_state & SS_NBIO);
	so->so_qstate &= ~SQ_COMP;
	so->so_head = NULL;

	SOCK_UNLOCK(so);
	ACCEPT_UNLOCK();

	/* An extra reference on `nfp' has been held for us by falloc(). */
	//td->td_retval[0] = fd;

	/* connection has been removed from the listen queue */
	//KNOTE_UNLOCKED(&head->so_rcv.sb_sel.si_note, 0);

	//pgid = fgetown(&head->so_sigio);
	//if (pgid != 0)
	//	fsetown(pgid, &so->so_sigio);

	//finit(nfp, fflag, DTYPE_SOCKET, so, &socketops);
	init_fd(fd, so);
	
	/* Sync socket nonblocking/async state with file flags */
	//tmp = fflag & FNONBLOCK;
	//(void) fo_ioctl(nfp, FIONBIO, &tmp, td->td_ucred, td);
	//tmp = fflag & FASYNC;
	//(void) fo_ioctl(nfp, FIOASYNC, &tmp, td->td_ucred, td);
	
	sa = 0;
	CURVNET_SET(so->so_vnet);
	error = soaccept(so, &sa);
	CURVNET_RESTORE();
	if (error) {
		/*
		 * return a namelen of zero for older code which might
		 * ignore the return value from accept.
		 */
		if (name)
			*namelen = 0;
		goto noconnection;
	}
	if (sa == NULL) {
		if (name)
			*namelen = 0;
		goto done;
	}
	if (name) {
		/* check sa_len before it is destroyed */
		if (*namelen > sa->sa_len)
			*namelen = sa->sa_len;
#ifdef KTRACE
		if (KTRPOINT(td, KTR_STRUCT))
			ktrsockaddr(sa);
#endif
		*name = sa;
		sa = NULL;
	}
noconnection:
	if (sa)
		bsd_free(sa, M_SONAME);

	/*
	 * close the new descriptor, assuming someone hasn't ripped it
	 * out from under us.
	 */
	//if (error)
	//	fdclose(fdp, nfp, fd, td);

	/*
	 * Release explicitly held references before returning.  We return
	 * a reference on nfp to the caller on success if they request it.
	 */
done:
	//if (fp != NULL) {
	//	if (error == 0) {
	//		*fp = nfp;
	//		nfp = NULL;
	//	} else
	//		*fp = NULL;
	//}
	//if (nfp != NULL)
	//	fdrop(nfp, td);
	//fdrop(headfp, td);
	return (error);

#if 0 ////need fix	
	struct sockaddr *sa = NULL;
	int error;
	struct socket *head, *so;
	int fd;
	int soid;

	if (name) {
		*name = NULL;
		if (*namelen < 0)
			return (EINVAL);
	}

	head = fd_socket_map[s].so; 
	if ((head->so_options & SO_ACCEPTCONN) == 0) {
		error = EINVAL;
		goto done;
	}
#ifdef MAC
	error = mac_socket_check_accept(td->td_ucred, head);
	if (error != 0)
		goto done;
#endif

	//soid = host_get_sockid();
	if (soid == -1){
		error = -1;
		goto done;
	}

	ACCEPT_LOCK();
	if ((head->so_state & SS_NBIO) && TAILQ_EMPTY(&head->so_comp)) {
		ACCEPT_UNLOCK();
		error = EWOULDBLOCK;
		goto noconnection;
	}
	while (TAILQ_EMPTY(&head->so_comp) && head->so_error == 0) {
		if (head->so_rcv.sb_state & SBS_CANTRCVMORE) {
			head->so_error = ECONNABORTED;
			break;
		}

                /*
                  * instead of mseelp, we wait for datain from host raw socket.
                  * In host_rawsock_waitfor_datain(), ip_input->tcp_input are 
                  * called. 
                  *
                  * For an accept_socket, sonewconn() is called to fill new 
                  * connections into so_comp queue.
                  */
		//host_rawsock_waitfor_datain(head->so_emuldata);
#if 0
		error = msleep(&head->so_timeo, &accept_mtx, PSOCK | PCATCH,
		    "accept", 0);
		if (error) {
			ACCEPT_UNLOCK();
			goto noconnection;
		}
#endif //0
	}
	if (head->so_error) {
		error = head->so_error;
		head->so_error = 0;
		ACCEPT_UNLOCK();
		goto noconnection;
	}
	so = TAILQ_FIRST(&head->so_comp);
//	KASSERT(!(so->so_qstate & SQ_INCOMP), ("accept1: so SQ_INCOMP"));
//	KASSERT(so->so_qstate & SQ_COMP, ("accept1: so not SQ_COMP"));

	/*
	 * Before changing the flags on the socket, we have to bump the
	 * reference count.  Otherwise, if the protocol calls sofree(),
	 * the socket will be released due to a zero refcount.
	 */
	SOCK_LOCK(so);			/* soref() and so_state update */
	soref(so);			/* file descriptor reference */

	TAILQ_REMOVE(&head->so_comp, so, so_list);
	head->so_qlen--;
	so->so_state |= (head->so_state & SS_NBIO);
	so->so_qstate &= ~SQ_COMP;
	so->so_head = NULL;

	SOCK_UNLOCK(so);
	ACCEPT_UNLOCK();

	/* An extra reference on `nfp' has been held for us by falloc(). */
//	td->td_retval[0] = fd;

	/* connection has been removed from the listen queue */
//	KNOTE_UNLOCKED(&head->so_rcv.sb_sel.si_note, 0);

//	pgid = fgetown(&head->so_sigio);
//	if (pgid != 0)
//		fsetown(pgid, &so->so_sigio);
//
//	finit(nfp, fflag, DTYPE_SOCKET, so, &socketops);
//	/* Sync socket nonblocking/async state with file flags */
//	tmp = fflag & FNONBLOCK;
//	(void) fo_ioctl(nfp, FIONBIO, &tmp, td->td_ucred, td);
//	tmp = fflag & FASYNC;
//	(void) fo_ioctl(nfp, FIOASYNC, &tmp, td->td_ucred, td);

	sa = 0;
//	CURVNET_SET(so->so_vnet);
	error = soaccept(so, &sa);
//	CURVNET_RESTORE();
	if (error) {
		/*
		 * return a namelen of zero for older code which might
		 * ignore the return value from accept.
		 */
		if (name)
			*namelen = 0;
		goto noconnection;
	}
	if (sa == NULL) {
		if (name)
			*namelen = 0;
		goto done;
	}
	if (name) {
		/* check sa_len before it is destroyed */
		if (*namelen > sa->sa_len)
			*namelen = sa->sa_len;
#ifdef KTRACE
		if (KTRPOINT(td, KTR_STRUCT))
			ktrsockaddr(sa);
#endif
		*name = sa;
		sa = NULL;
	}
noconnection:
	if (sa)
		bsd_free(sa, M_SONAME);

	/*
	 * close the new descriptor, assuming someone hasn't ripped it
	 * out from under us.
	 */
	if (error)
		free_fd(fd); //fdclose(fdp, nfp, fd, td);

	/*
	 * Release explicitly held references before returning.  We return
	 * a reference on nfp to the caller on success if they request it.
	 */
done:
#if 0
	if (fp != NULL) {
		if (error == 0) {
			*fp = nfp;
			nfp = NULL;
		} else
			*fp = NULL;
	}
	if (nfp != NULL)
		fdrop(nfp, td);
	fdrop(headfp, td);
#endif //0

	return (error);
#endif////0
}

int
bsd_syscall_accept(int sockfd, int newfd, struct sockaddr *name, socklen_t *anamelen)
{
	return (accept1(sockfd, newfd, name,  anamelen, 0));
}

#ifdef COMPAT_OLDSOCK
int
oaccept(td, uap)
	struct thread *td;
	struct accept_args *uap;
{

	return (accept1(td, uap, 1));
}
#endif /* COMPAT_OLDSOCK */


int kern_connect(int fd, struct sockaddr *sa);

/* ARGSUSED */
int
bsd_syscall_connect(int sockfd, caddr_t name, int namelen)
{
	struct sockaddr *sa;
	int error;

	error = getsockaddr(&sa, (caddr_t)name, (size_t)namelen);
	if (error)
		return (error);

	error = kern_connect(sockfd, sa);
	//bsd_free(sa, M_SONAME);
	return (error);
}


int
kern_connect(fd, sa)
	int fd;
	struct sockaddr *sa;
{
	struct socket *so;
	int error;
	int interrupted = 0;

//	AUDIT_ARG_FD(fd);
//	error = getsock(td->td_proc->p_fd, fd, &fp, NULL);
//	if (error)
//		return (error);
	so = getsockbyfd(fd); //fp->f_data;
	if (so->so_state & SS_ISCONNECTING) {
		error = EALREADY;
		goto done1;
	}
#ifdef KTRACE
	if (KTRPOINT(td, KTR_STRUCT))
		ktrsockaddr(sa);
#endif
#ifdef MAC
	error = mac_socket_check_connect(td->td_ucred, so, sa);
	if (error)
		goto bad;
#endif
	error = soconnect(so, sa, NULL);
	if (error)
		goto bad;
	if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING)) {
		error = EINPROGRESS;
		goto done1;
	}
	SOCK_LOCK(so);
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0) {

		error = msleep(so->so_timeo_cond, SOCK_MTX(so), PSOCK | PCATCH,
		    "connec", 0);
		if (error) {
			if (error == EINTR || error == ERESTART)
				interrupted = 1;
			break;
		}
	}
	if (error == 0) {
		error = so->so_error;
		so->so_error = 0;
	}
	SOCK_UNLOCK(so);
bad:
	if (!interrupted)
		so->so_state &= ~SS_ISCONNECTING;
	if (error == ERESTART)
		error = EINTR;
done1:
//	fdrop(fp, td);
	return (error);
}

int
kern_socketpair(struct thread *td, int domain, int type, int protocol,
    int *rsv)
{
//	struct filedesc *fdp = td->td_proc->p_fd;
//	struct file *fp1, *fp2;
	struct socket *so1, *so2;
	int fd, error;

//	AUDIT_ARG_SOCKET(domain, type, protocol);
#ifdef MAC
	/* We might want to have a separate check for socket pairs. */
	error = mac_socket_check_create(td->td_ucred, domain, type,
	    protocol);
	if (error)
		return (error);
#endif
	error = socreate(domain, &so1, type, protocol, NULL, NULL/*td->td_ucred, td*/);
	if (error)
		return (error);
	error = socreate(domain, &so2, type, protocol, NULL, NULL/*td->td_ucred, td*/);
	if (error)
		goto free1;
	/* On success extra reference to `fp1' and 'fp2' is set by falloc. */
	error = fd = -1; //FIXME alloc_fd();
	if (error == -1)
		goto free2;
	rsv[0] = fd;
	init_fd(fd, so1); //fp1->f_data = so1;	/* so1 already has ref count */
	
    error = fd = -1; //FIXME  alloc_fd();
	if (error == -1)
		goto free3;
    init_fd(fd, so2);
	rsv[1] = fd;
	error = soconnect2(so1, so2);
	if (error)
		goto free4;
	if (type == SOCK_DGRAM) {
		/*
		 * Datagram socket connection is asymmetric.
		 */
		 error = soconnect2(so2, so1);
		 if (error)
			goto free4;
	}
//	finit(fp1, FREAD | FWRITE, DTYPE_SOCKET, fp1->f_data, &socketops);
//	finit(fp2, FREAD | FWRITE, DTYPE_SOCKET, fp2->f_data, &socketops);
//	fdrop(fp1, td);
//	fdrop(fp2, td);
	return (0);
free4:
    free_fd(rsv[1]);
//	fdclose(fdp, fp2, rsv[1], td);
//	fdrop(fp2, td);
free3:
    free_fd(rsv[0]);
//	fdclose(fdp, fp1, rsv[0], td);
//	fdrop(fp1, td);
free2:
	if (so2 != NULL)
		(void)soclose(so2);
free1:
	if (so1 != NULL)
		(void)soclose(so1);
	return (error);
}

int
bsd_syscall_socketpair(int sockfd, int domain, int type, int protocol, int *rsv)
{
	int error, sv[2];

	error = kern_socketpair(NULL, domain, type, protocol, sv);
	if (error)
		return (error);
	error = copyout(sv, rsv, 2 * sizeof(int));
	if (error) {
		(void)free_fd(sv[0]);
		(void)free_fd(sv[1]);
	}
	return (error);
}

int kern_sendit(int s,	struct msghdr *mp,	int flags,	struct mbuf *control,	enum uio_seg segflg, int* sendlen);

static int
sendit(s, mp, flags, sendlen)
	int s;
	struct msghdr *mp;
	int flags;
	int* sendlen;
{
	struct mbuf *control;
	struct sockaddr *to;
	int error;

	if (mp->msg_name != NULL) {
		error = getsockaddr(&to, mp->msg_name, mp->msg_namelen);
		if (error) {
			to = NULL;
			goto bad;
		}
		mp->msg_name = to;
	} else {
		to = NULL;
	}

	if (mp->msg_control) {
		if (mp->msg_controllen < sizeof(struct cmsghdr)
#ifdef COMPAT_OLDSOCK
		    && mp->msg_flags != MSG_COMPAT
#endif
		) {
			error = EINVAL;
			goto bad;
		}
		error = sockargs(&control, mp->msg_control,
		    mp->msg_controllen, MT_CONTROL);
		if (error)
			goto bad;
#ifdef COMPAT_OLDSOCK
		if (mp->msg_flags == MSG_COMPAT) {
			struct cmsghdr *cm;

			M_PREPEND(control, sizeof(*cm), M_WAIT);
			cm = mtod(control, struct cmsghdr *);
			cm->cmsg_len = control->m_len;
			cm->cmsg_level = SOL_SOCKET;
			cm->cmsg_type = SCM_RIGHTS;
		}
#endif
	} else {
		control = NULL;
	}

	error = kern_sendit(s, mp, flags, control, UIO_USERSPACE, sendlen);

bad:
	//if (to)
	//	bsd_free(to, M_SONAME);
	return (error);
}

int
kern_sendit(s, mp, flags, control, segflg, sendlen)
	int s;
	struct msghdr *mp;
	int flags;
	struct mbuf *control;
	enum uio_seg segflg;
	int* sendlen;
{
//	struct file *fp;
	struct uio auio;
	struct iovec *iov;
	struct socket *so;
	int i;
	int len, error;

//	AUDIT_ARG_FD(s);
//	error = getsock(td->td_proc->p_fd, s, &fp, NULL);
//	if (error)
//		return (error);
	so = fd_socket_map[s].so; //(struct socket *)fp->f_data;

#ifdef MAC
	if (mp->msg_name != NULL) {
		error = mac_socket_check_connect(td->td_ucred, so,
		    mp->msg_name);
		if (error)
			goto bad;
	}
	error = mac_socket_check_send(td->td_ucred, so);
	if (error)
		goto bad;
#endif

	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = segflg;
	auio.uio_rw = UIO_WRITE;
	auio.uio_td = NULL;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if ((auio.uio_resid += iov->iov_len) < 0) {
			error = EINVAL;
			goto bad;
		}
	}
#ifdef KTRACE
	if (KTRPOINT(td, KTR_GENIO))
		ktruio = cloneuio(&auio);
#endif
	len = auio.uio_resid;
	error = sosend(so, mp->msg_name, &auio, 0, control, flags, NULL);
	if (error) {
		if (auio.uio_resid != len && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
		/* Generation of SIGPIPE can be controlled per socket */
		if (error == EPIPE && !(so->so_options & SO_NOSIGPIPE) &&
		    !(flags & MSG_NOSIGNAL)) {
//			PROC_LOCK(td->td_proc);
//			psignal(td->td_proc, SIGPIPE);
//			PROC_UNLOCK(td->td_proc);
		}
	}
	if (error == 0)
		*sendlen = len - auio.uio_resid;
//		td->td_retval[0] = len - auio.uio_resid;
bad:
//	fdrop(fp, td);
	return (error);
}

int
bsd_syscall_sendto(int sockfd, caddr_t buf, size_t len, int flags, caddr_t  to, int tolen)
{
	struct msghdr msg;
	struct iovec aiov;
	int error;
	int sendlen;

	msg.msg_name = to;
	msg.msg_namelen = tolen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	msg.msg_control = 0;
#ifdef COMPAT_OLDSOCK
	msg.msg_flags = 0;
#endif
	aiov.iov_base = buf;
	aiov.iov_len = len;
	error = sendit(sockfd, &msg, flags, &sendlen);
	if(error)
		return error;
	else return sendlen;
}

int
bsd_syscall_send(int sockfd, caddr_t buf, int len, int flags)
{
	struct msghdr msg;
	struct iovec aiov;
	int error;
	int sendlen;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = buf; 
	aiov.iov_len = len;
	msg.msg_control = 0;
	msg.msg_flags = 0;
	error = sendit(sockfd, &msg, flags, &sendlen);
	if(error)
		return error;
	else 	return sendlen;
}


#ifdef COMPAT_OLDSOCK
int
osend(td, uap)
	struct thread *td;
	struct osend_args /* {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} */ *uap;
{
	struct msghdr msg;
	struct iovec aiov;
	int error;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_control = 0;
	msg.msg_flags = 0;
	error = sendit(td, uap->s, &msg, uap->flags);
	return (error);
}

int
osendmsg(td, uap)
	struct thread *td;
	struct osendmsg_args /* {
		int	s;
		caddr_t	msg;
		int	flags;
	} */ *uap;
{
	struct msghdr msg;
	struct iovec *iov;
	int error;

	error = copyin(uap->msg, &msg, sizeof (struct omsghdr));
	if (error)
		return (error);
	error = copyiniov(msg.msg_iov, msg.msg_iovlen, &iov, EMSGSIZE);
	if (error)
		return (error);
	msg.msg_iov = iov;
	msg.msg_flags = MSG_COMPAT;
	error = sendit(td, uap->s, &msg, uap->flags);
	bsd_free(iov, M_IOV);
	return (error);
}
#endif

int
bsd_syscall_sendmsg(int sockfd, caddr_t msg, int flags)
{
	struct msghdr _msg;
	struct iovec *iov;
	int error;
	int sendlen;

	error = copyin(msg, &_msg, sizeof (_msg));
	if (error)
		return (error);
	error = copyiniov(_msg.msg_iov, _msg.msg_iovlen, &iov, EMSGSIZE);
	if (error)
		return (error);
	_msg.msg_iov = iov;
#ifdef COMPAT_OLDSOCK
	_msg.msg_flags = 0;
#endif
	error = sendit(sockfd, &_msg, flags, &sendlen);
	bsd_free(iov, M_IOV);

	if(error)
		return error;
	else return sendlen;
}

int
kern_recvit(s, mp, fromseg, controlp, recvlen)
	int s;
	struct msghdr *mp;
	enum uio_seg fromseg;
	struct mbuf **controlp;
	int *recvlen;
{
	struct uio auio;
	struct iovec *iov;
	int i;
	socklen_t len;
	int error;
	struct mbuf *m, *control = 0;
	caddr_t ctlbuf;
//	struct file *fp;
	struct socket *so;
	struct sockaddr *fromsa = 0;
#ifdef KTRACE
	struct uio *ktruio = NULL;
#endif

	if(controlp != NULL)
		*controlp = 0;

//	AUDIT_ARG_FD(s);
//	error = getsock(td->td_proc->p_fd, s, &fp, NULL);
//	if (error)
//		return (error);
	so = fd_socket_map[s].so; //fp->f_data;

#ifdef MAC
	error = mac_socket_check_receive(td->td_ucred, so);
	if (error) {
		fdrop(fp, td);
		return (error);
	}
#endif

	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_td = NULL;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if ((auio.uio_resid += iov->iov_len) < 0) {
//			fdrop(fp, td);
			return (EINVAL);
		}
	}
#ifdef KTRACE
	if (KTRPOINT(td, KTR_GENIO))
		ktruio = cloneuio(&auio);
#endif
	len = auio.uio_resid;
//	CURVNET_SET(so->so_vnet);
	error = soreceive(so, &fromsa, &auio, (struct mbuf **)0,
	    (mp->msg_control || controlp) ? &control : (struct mbuf **)0,
	    &mp->msg_flags);
//	CURVNET_RESTORE();
	if (error) {
		if (auio.uio_resid != (int)len && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
	}
#ifdef KTRACE
	if (ktruio != NULL) {
		ktruio->uio_resid = (int)len - auio.uio_resid;
		ktrgenio(s, UIO_READ, ktruio, error);
	}
#endif
	if (error)
		goto out;
//	td->td_retval[0] = (int)len - auio.uio_resid;
	*recvlen =  (int)len - auio.uio_resid;
	
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || fromsa == 0)
			len = 0;
		else {
			/* save sa_len before it is destroyed by MSG_COMPAT */
			len = MIN(len, fromsa->sa_len);
#ifdef COMPAT_OLDSOCK
			if (mp->msg_flags & MSG_COMPAT)
				((struct osockaddr *)fromsa)->sa_family =
				    fromsa->sa_family;
#endif
			if (fromseg == UIO_USERSPACE) {
				error = copyout(fromsa, mp->msg_name,
				    (unsigned)len);
				if (error)
					goto out;
			} else
				bcopy(fromsa, mp->msg_name, len);
		}
		mp->msg_namelen = len;
	}
	if (mp->msg_control && controlp == NULL) {
#ifdef COMPAT_OLDSOCK
		/*
		 * We assume that old recvmsg calls won't receive access
		 * rights and other control info, esp. as control info
		 * is always optional and those options didn't exist in 4.3.
		 * If we receive rights, trim the cmsghdr; anything else
		 * is tossed.
		 */
		if (control && mp->msg_flags & MSG_COMPAT) {
			if (mtod(control, struct cmsghdr *)->cmsg_level !=
			    SOL_SOCKET ||
			    mtod(control, struct cmsghdr *)->cmsg_type !=
			    SCM_RIGHTS) {
				mp->msg_controllen = 0;
				goto out;
			}
			control->m_len -= sizeof (struct cmsghdr);
			control->m_data += sizeof (struct cmsghdr);
		}
#endif
		len = mp->msg_controllen;
		m = control;
		mp->msg_controllen = 0;
		ctlbuf = mp->msg_control;

		while (m && len > 0) {
			unsigned int tocopy;

			if (len >= m->m_len)
				tocopy = m->m_len;
			else {
				mp->msg_flags |= MSG_CTRUNC;
				tocopy = len;
			}

			if ((error = copyout(mtod(m, caddr_t),
					ctlbuf, tocopy)) != 0)
				goto out;

			ctlbuf += tocopy;
			len -= tocopy;
			m = m->m_next;
		}
		mp->msg_controllen = ctlbuf - (caddr_t)mp->msg_control;
	}
out:
//	fdrop(fp, td);
#ifdef KTRACE
	if (fromsa && KTRPOINT(td, KTR_STRUCT))
		ktrsockaddr(fromsa);
#endif
	if (fromsa)
		bsd_free(fromsa, M_SONAME);

	if (error == 0 && controlp != NULL)  
		*controlp = control;
	else  if (control)
		m_freem(control);

	return (error);
}

static int
recvit(s, mp, namelenp, recvlen)
	int s;
	struct msghdr *mp;
	void *namelenp;
	int* recvlen;
{
	int error;

	error = kern_recvit(s, mp, UIO_USERSPACE, NULL, recvlen);
	if (error)
		return (error);
	if (namelenp) {
		error = copyout(&mp->msg_namelen, namelenp, sizeof (socklen_t));
#ifdef COMPAT_OLDSOCK
		if (mp->msg_flags & MSG_COMPAT)
			error = 0;	/* old recvfrom didn't check */
#endif
	}
	return (error);
}

int
bsd_syscall_recvfrom(int sockfd, caddr_t buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlenaddr)
{
	struct msghdr msg;
	struct iovec aiov;
	int error;
	int recvlen;

	if (fromlenaddr) {
		error = copyin(fromlenaddr,
		    &msg.msg_namelen, sizeof (msg.msg_namelen));
		if (error)
			goto done2;
	} else {
		msg.msg_namelen = 0;
	}
	msg.msg_name = from;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = buf;
	aiov.iov_len = len;
	msg.msg_control = 0;
	msg.msg_flags = flags;
	error = recvit(sockfd, &msg, fromlenaddr, &recvlen);
done2:
	if(error == 0)
		return recvlen;
	else 	return(error);
}

#ifdef COMPAT_OLDSOCK
int
orecvfrom(td, uap)
	struct thread *td;
	struct recvfrom_args *uap;
{

	uap->flags |= MSG_COMPAT;
	return (recvfrom(td, uap));
}
#endif

int
bsd_syscall_recv(int sockfd, caddr_t buf, int  len,  int flags)
{
	struct msghdr msg;
	struct iovec aiov;
	int error;
	int recvlen;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = buf;
	aiov.iov_len = len;
	msg.msg_control = 0;
	msg.msg_flags = flags;
	error = recvit(sockfd, &msg, NULL, &recvlen);
	if(error)
		return error;
	else return recvlen;
}


#ifdef COMPAT_OLDSOCK
int
orecv(td, uap)
	struct thread *td;
	struct orecv_args /* {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} */ *uap;
{
	struct msghdr msg;
	struct iovec aiov;
	int error;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_control = 0;
	msg.msg_flags = uap->flags;
	error = recvit(s, &msg, NULL);
	return (error);
}

/*
 * Old recvmsg.  This code takes advantage of the fact that the old msghdr
 * overlays the new one, missing only the flags, and with the (old) access
 * rights where the control fields are now.
 */
int
orecvmsg(td, uap)
	struct thread *td;
	struct orecvmsg_args /* {
		int	s;
		struct	omsghdr *msg;
		int	flags;
	} */ *uap;
{
	struct msghdr msg;
	struct iovec *iov;
	int error;

	error = copyin(uap->msg, &msg, sizeof (struct omsghdr));
	if (error)
		return (error);
	error = copyiniov(msg.msg_iov, msg.msg_iovlen, &iov, EMSGSIZE);
	if (error)
		return (error);
	msg.msg_flags = uap->flags | MSG_COMPAT;
	msg.msg_iov = iov;
	error = recvit(s, &msg, &msg->msg_namelen);
	if (msg.msg_controllen && error == 0)
		error = copyout(&msg.msg_controllen,
		    &uap->msg->msg_accrightslen, sizeof (int));
	bsd_free(iov, M_IOV);
	return (error);
}
#endif

int
bsd_syscall_recvmsg(int sockfd, struct sal_recvmsg_args *args
		/* struct msghdr *msg, int  flags*/)
{
	struct msghdr _msg;
	struct iovec *uiov, *iov;
	int error;
	int recvlen;

	error = copyin(args->msg, &_msg, sizeof (_msg));
	if (error)
		return (error);
	error = copyiniov(_msg.msg_iov, _msg.msg_iovlen, &iov, EMSGSIZE);
	if (error)
		return (error);
	_msg.msg_flags = args->flags;
#ifdef COMPAT_OLDSOCK
	msg.msg_flags &= ~MSG_COMPAT;
#endif
	uiov = _msg.msg_iov;
	_msg.msg_iov = iov;
	error = recvit(sockfd, &_msg, NULL, &recvlen);
	if (error == 0) {
		_msg.msg_iov = uiov;
		error = copyout(&_msg, args->msg, sizeof(_msg));
	}
	bsd_free(iov, M_IOV);
	if(error)
		return (error);
	else return recvlen;
}

/* ARGSUSED */
int
bsd_syscall_shutdown(int sockfd, int how)
{
	struct socket *so;
//	struct file *fp;
	int error;
	struct inpcb* inp;
	struct tcpcb* tcb;

//	AUDIT_ARG_FD(uap->s);
//	error = getsock(td->td_proc->p_fd, uap->s, &fp, NULL);
//	if (error == 0) {

	so = getsockbyfd(sockfd); //fp->f_data;
	inp = (struct inpcb*)so->so_pcb;	
	if(inp != NULL){
		tcb = (struct tcpcb*)inp->inp_ppcb;

		while(tcb != NULL && tcb->snd_una < tcb->snd_max)
			usleep(1000);
	}
	error = soshutdown(so, how);

	free_fd(sockfd);

//		fdrop(fp, td);
//	}
	return (error);
}

int kern_setsockopt(int s, int level, int name, void *val, enum uio_seg valseg, socklen_t valsize);

/* ARGSUSED */
int
bsd_syscall_setsockopt(int sockfd, int level, int name, caddr_t val, int valsize)
{

	return (kern_setsockopt(sockfd, level, name,
	    val, UIO_USERSPACE, valsize));
}

int
kern_setsockopt(s, level, name, val, valseg, valsize)
	int s;
	int level;
	int name;
	void *val;
	enum uio_seg valseg;
	socklen_t valsize;
{
	int error;
	struct socket *so;
//	struct file *fp;
	struct sockopt sopt;

	if (val == NULL && valsize != 0)
		return (EFAULT);
	if ((int)valsize < 0)
		return (EINVAL);

	sopt.sopt_dir = SOPT_SET;
	sopt.sopt_level = level;
	sopt.sopt_name = name;
	sopt.sopt_val = val;
	sopt.sopt_valsize = valsize;
	switch (valseg) {
	case UIO_USERSPACE:
		sopt.sopt_td = NULL;
		break;
	case UIO_SYSSPACE:
		sopt.sopt_td = NULL;
		break;
	default:
		panic("kern_setsockopt called with bad valseg");
	}

//	AUDIT_ARG_FD(s);
//	error = getsock(td->td_proc->p_fd, s, &fp, NULL);
//	if (error == 0) {
		so = fd_socket_map[s].so; //fp->f_data;
//		CURVNET_SET(so->so_vnet);
		error = sosetopt(so, &sopt);
//		CURVNET_RESTORE();
//		fdrop(fp, td);
//	}
	return(error);
}

int kern_getsockopt(int s,int level,int name, void *val, enum uio_seg valseg, socklen_t *valsize);

/* ARGSUSED */
int
bsd_syscall_getsockopt(int sockfd, int level, int name, void * val, socklen_t *avalsize)
{
	socklen_t valsize;
	int	error;

	if (val) {
		error = copyin(avalsize, &valsize, sizeof (valsize));
		if (error)
			return (error);
	}

	error = kern_getsockopt(sockfd, level, name,
	    val, UIO_USERSPACE, &valsize);

	if (error == 0)
		error = copyout(&valsize, avalsize, sizeof (valsize));
	return (error);
}

/*
 * Kernel version of getsockopt.
 * optval can be a userland or userspace. optlen is always a kernel pointer.
 */
int
kern_getsockopt(s, level, name, val, valseg, valsize)
	int s;
	int level;
	int name;
	void *val;
	enum uio_seg valseg;
	socklen_t *valsize;
{
	int error;
	struct  socket *so;
//	struct file *fp;
	struct	sockopt sopt;

	if (val == NULL)
		*valsize = 0;
	if ((int)*valsize < 0)
		return (EINVAL);

	sopt.sopt_dir = SOPT_GET;
	sopt.sopt_level = level;
	sopt.sopt_name = name;
	sopt.sopt_val = val;
	sopt.sopt_valsize = (size_t)*valsize; /* checked non-negative above */
	switch (valseg) {
	case UIO_USERSPACE:
		sopt.sopt_td = NULL;
		break;
	case UIO_SYSSPACE:
		sopt.sopt_td = NULL;
		break;
	default:
		panic("kern_getsockopt called with bad valseg");
	}

//	AUDIT_ARG_FD(s);
//	error = getsock(td->td_proc->p_fd, s, &fp, NULL);
//	if (error == 0) {
		so = fd_socket_map[s].so; //fp->f_data;
//		CURVNET_SET(so->so_vnet);
		error = sogetopt(so, &sopt);
//		CURVNET_RESTORE();
		*valsize = sopt.sopt_valsize;
//		fdrop(fp, td);
//	}
	return (error);
}

int kern_getsockname(int fd, struct sockaddr **sa,  socklen_t *alen);

/*
 * getsockname1() - Get socket name.
 */
/* ARGSUSED */
static int
getsockname1(int	fdes,
		struct sockaddr * asa,
		socklen_t *alen,
	    int compat)
{
	struct sockaddr *sa;
	socklen_t len;
	int error;

	error = copyin(alen, &len, sizeof(len));
	if (error)
		return (error);

	error = kern_getsockname(fdes, &sa, &len);
	if (error)
		return (error);

	if (len != 0) {
#ifdef COMPAT_OLDSOCK
		if (compat)
			((struct osockaddr *)sa)->sa_family = sa->sa_family;
#endif
		error = copyout(sa, asa, (u_int)len);
	}
	bsd_free(sa, M_SONAME);
	if (error == 0)
		error = copyout(&len, alen, sizeof(len));
	return (error);
}

int
kern_getsockname(int fd, struct sockaddr **sa,
    socklen_t *alen)
{
	struct socket *so;
//	struct file *fp;
	socklen_t len;
	int error;

	if (*alen < 0)
		return (EINVAL);

//	AUDIT_ARG_FD(fd);
//	error = getsock(td->td_proc->p_fd, fd, &fp, NULL);
//	if (error)
//		return (error);
	so = getsockbyfd(fd); //fp->f_data;
	*sa = NULL;
//	CURVNET_SET(so->so_vnet);
	error = (*so->so_proto->pr_usrreqs->pru_sockaddr)(so, sa);
//	CURVNET_RESTORE();
	if (error)
		goto bad;
	if (*sa == NULL)
		len = 0;
	else
		len = MIN(*alen, (*sa)->sa_len);
	*alen = len;
#ifdef KTRACE
	if (KTRPOINT(td, KTR_STRUCT))
		ktrsockaddr(*sa);
#endif
bad:
//	fdrop(fp, td);
	if (error && *sa) {
		bsd_free(*sa, M_SONAME);
		*sa = NULL;
	}
	return (error);
}

int
bsd_syscall_getsockname(int sockfd, struct sockaddr *asa, socklen_t *alen) 
{

	return (getsockname1(sockfd, asa, alen, 0));
}

#ifdef COMPAT_OLDSOCK
int
ogetsockname(td, uap)
	struct thread *td;
	struct getsockname_args *uap;
{

	return (getsockname1(td, uap, 1));
}
#endif /* COMPAT_OLDSOCK */

int kern_getpeername(int fd, struct sockaddr **sa,  socklen_t *alen);

/*
 * getpeername1() - Get name of peer for connected socket.
 */
/* ARGSUSED */
static int
getpeername1(int fdes,
		struct sockaddr *asa,
		socklen_t *alen,
	    int compat)
{
	struct sockaddr *sa;
	socklen_t len;
	int error;

	error = copyin(alen, &len, sizeof (len));
	if (error)
		return (error);

	error = kern_getpeername(fdes, &sa, &len);
	if (error)
		return (error);

	if (len != 0) {
#ifdef COMPAT_OLDSOCK
		if (compat)
			((struct osockaddr *)sa)->sa_family = sa->sa_family;
#endif
		error = copyout(sa, asa, (u_int)len);
	}
	bsd_free(sa, M_SONAME);
	if (error == 0)
		error = copyout(&len, alen, sizeof(len));
	return (error);
}

int
kern_getpeername(int fd, struct sockaddr **sa,
    socklen_t *alen)
{
	struct socket *so;
//	struct file *fp;
	socklen_t len;
	int error;

	if (*alen < 0)
		return (EINVAL);

//	AUDIT_ARG_FD(fd);
//	error = getsock(td->td_proc->p_fd, fd, &fp, NULL);
//	if (error)
//		return (error);
	so = getsockbyfd(fd); //fp->f_data;
	if ((so->so_state & (SS_ISCONNECTED|SS_ISCONFIRMING)) == 0) {
		error = ENOTCONN;
		goto done;
	}
	*sa = NULL;
//	CURVNET_SET(so->so_vnet);
	error = (*so->so_proto->pr_usrreqs->pru_peeraddr)(so, sa);
//	CURVNET_RESTORE();
	if (error)
		goto bad;
	if (*sa == NULL)
		len = 0;
	else
		len = MIN(*alen, (*sa)->sa_len);
	*alen = len;
#ifdef KTRACE
	if (KTRPOINT(td, KTR_STRUCT))
		ktrsockaddr(*sa);
#endif
bad:
	if (error && *sa) {
		bsd_free(*sa, M_SONAME);
		*sa = NULL;
	}
done:
//	fdrop(fp, td);
	return (error);
}

int
bsd_syscall_getpeername(int sockfd, struct sockaddr *asa, socklen_t *alen)
{

	return (getpeername1(sockfd, asa, alen, 0));
}

#ifdef COMPAT_OLDSOCK
int
ogetpeername(td, uap)
	struct thread *td;
	struct ogetpeername_args *uap;
{

	/* XXX uap should have type `getpeername_args *' to begin with. */
	return (getpeername1(td, (struct getpeername_args *)uap, 1));
}
#endif /* COMPAT_OLDSOCK */

int
sockargs(mp, buf, buflen, type)
	struct mbuf **mp;
	caddr_t buf;
	int buflen, type;
{
	struct sockaddr *sa;
	struct mbuf *m;
	int error;

	if ((u_int)buflen > MLEN) {
#ifdef COMPAT_OLDSOCK
		if (type == MT_SONAME && (u_int)buflen <= 112)
			buflen = MLEN;		/* unix domain compat. hack */
		else
#endif
			if ((u_int)buflen > MCLBYTES)
				return (EINVAL);
	}
	m = m_get(M_WAIT, type);
	if ((u_int)buflen > MLEN)
		MCLGET(m, M_WAIT);
	m->m_len = buflen;
	error = copyin(buf, mtod(m, caddr_t), (u_int)buflen);
	if (error)
		(void) m_free(m);
	else {
		*mp = m;
		if (type == MT_SONAME) {
			sa = mtod(m, struct sockaddr *);

#if defined(COMPAT_OLDSOCK) && BYTE_ORDER != BIG_ENDIAN
			if (sa->sa_family == 0 && sa->sa_len < AF_MAX)
				sa->sa_family = sa->sa_len;
#endif
			sa->sa_len = buflen;
		}
	}
	return (error);
}

int
getsockaddr(namp, uaddr, len)
	struct sockaddr **namp;
	caddr_t uaddr;
	size_t len;
{
	struct sockaddr *sa;

	if (len > SOCK_MAXADDRLEN)
		return (ENAMETOOLONG);
	if (len < offsetof(struct sockaddr, sa_data[0]))
		return (EINVAL);
	
	sa = (struct sockaddr*)uaddr;
	
#if defined(COMPAT_OLDSOCK) && BYTE_ORDER != BIG_ENDIAN
	if (sa->sa_family == 0 && sa->sa_len < AF_MAX)
		sa->sa_family = sa->sa_len;
#endif
	sa->sa_len = len;
	*namp = sa;

	return (0);
}


#if 0 //for sendfile
#include <sys/bsd_condvar.h>

struct sendfile_sync {
	struct mtx	mtx;
	struct cv	cv;
	unsigned 	count;
};

/*
 * Detach mapped page and release resources back to the system.
 */
void
sf_buf_mext(void *addr, void *argss)
{
	vm_page_t m;
	struct sendfile_sync *sfs;

	m = sf_buf_page(args);
	sf_buf_free(args);
	vm_page_lock_queues();
	vm_page_unwire(m, 0);
	/*
	 * Check for the object going away on us. This can
	 * happen since we don't hold a reference to it.
	 * If so, we're responsible for freeing the page.
	 */
	if (m->wire_count == 0 && m->object == NULL)
		vm_page_free(m);
	vm_page_unlock_queues();
	if (addr == NULL)
		return;
	sfs = addr;
	mtx_lock(&sfs->mtx);
	KASSERT(sfs->count> 0, ("Sendfile sync botchup count == 0"));
	if (--sfs->count == 0)
		cv_signal(&sfs->cv);
	mtx_unlock(&sfs->mtx);
}


/*
 * sendfile(2)
 *
 * int sendfile(int fd, int s, off_t offset, size_t nbytes,
 *	 struct sf_hdtr *hdtr, off_t *sbytes, int flags)
 *
 * Send a file specified by 'fd' and starting at 'offset' to a socket
 * specified by 's'. Send only 'nbytes' of the file or until EOF if nbytes ==
 * 0.  Optionally add a header and/or trailer to the socket output.  If
 * specified, write the total number of bytes sent into *sbytes.
 */
int
sendfile(struct thread *td, struct sendfile_args *uap)
{

	return (do_sendfile(td, uap, 0));
}

static int
do_sendfile(struct thread *td, struct sendfile_args *uap, int compat)
{
	struct sf_hdtr hdtr;
	struct uio *hdr_uio, *trl_uio;
	int error;

	hdr_uio = trl_uio = NULL;

	if (uap->hdtr != NULL) {
		error = copyin(uap->hdtr, &hdtr, sizeof(hdtr));
		if (error)
			goto out;
		if (hdtr.headers != NULL) {
			error = copyinuio(hdtr.headers, hdtr.hdr_cnt, &hdr_uio);
			if (error)
				goto out;
		}
		if (hdtr.trailers != NULL) {
			error = copyinuio(hdtr.trailers, hdtr.trl_cnt, &trl_uio);
			if (error)
				goto out;

		}
	}

	error = kern_sendfile(td, uap, hdr_uio, trl_uio, compat);
out:
	if (hdr_uio)
		bsd_free(hdr_uio, M_IOV);
	if (trl_uio)
		bsd_free(trl_uio, M_IOV);
	return (error);
}

#ifdef COMPAT_FREEBSD4
int
freebsd4_sendfile(struct thread *td, struct freebsd4_sendfile_args *uap)
{
	struct sendfile_args args;

	args.fd = uap->fd;
	args.s = uap->s;
	args.offset = uap->offset;
	args.nbytes = uap->nbytes;
	args.hdtr = uap->hdtr;
	args.sbytes = uap->sbytes;
	args.flags = uap->flags;

	return (do_sendfile(td, &args, 1));
}
#endif /* COMPAT_FREEBSD4 */

int
kern_sendfile(struct thread *td, struct sendfile_args *uap,
    struct uio *hdr_uio, struct uio *trl_uio, int compat)
{
	struct file *sock_fp;
	struct vnode *vp;
	struct vm_object *obj = NULL;
	struct socket *so = NULL;
	struct mbuf *m = NULL;
	struct sf_buf *sf;
	struct vm_page *pg;
	off_t off, xfsize, fsbytes = 0, sbytes = 0, rem = 0;
	int error, hdrlen = 0, mnw = 0;
	int vfslocked;
	struct sendfile_sync *sfs = NULL;

	/*
	 * The file descriptor must be a regular file and have a
	 * backing VM object.
	 * File offset must be positive.  If it goes beyond EOF
	 * we send only the header/trailer and no payload data.
	 */
	AUDIT_ARG_FD(uap->fd);
	if ((error = fgetvp_read(td, uap->fd, &vp)) != 0)
		goto out;
	vfslocked = VFS_LOCK_GIANT(vp->v_mount);
	vn_lock(vp, LK_SHARED | LK_RETRY);
	if (vp->v_type == VREG) {
		obj = vp->v_object;
		if (obj != NULL) {
			/*
			 * Temporarily increase the backing VM
			 * object's reference count so that a forced
			 * reclamation of its vnode does not
			 * immediately destroy it.
			 */
			VM_OBJECT_LOCK(obj);
			if ((obj->flags & OBJ_DEAD) == 0) {
				vm_object_reference_locked(obj);
				VM_OBJECT_UNLOCK(obj);
			} else {
				VM_OBJECT_UNLOCK(obj);
				obj = NULL;
			}
		}
	}
	VOP_UNLOCK(vp, 0);
	VFS_UNLOCK_GIANT(vfslocked);
	if (obj == NULL) {
		error = EINVAL;
		goto out;
	}
	if (uap->offset < 0) {
		error = EINVAL;
		goto out;
	}

	/*
	 * The socket must be a stream socket and connected.
	 * Remember if it a blocking or non-blocking socket.
	 */
	if ((error = getsock(td->td_proc->p_fd, uap->s, &sock_fp,
	    NULL)) != 0)
		goto out;
	so = sock_fp->f_data;
	if (so->so_type != SOCK_STREAM) {
		error = EINVAL;
		goto out;
	}
	if ((so->so_state & SS_ISCONNECTED) == 0) {
		error = ENOTCONN;
		goto out;
	}
	/*
	 * Do not wait on memory allocations but return ENOMEM for
	 * caller to retry later.
	 * XXX: Experimental.
	 */
	if (uap->flags & SF_MNOWAIT)
		mnw = 1;

	if (uap->flags & SF_SYNC) {
		sfs = bsd_malloc(sizeof *sfs, M_TEMP, M_WAITOK);
		memset(sfs, 0, sizeof *sfs);
		mtx_init(&sfs->mtx, "sendfile", MTX_DEF, 0);
		cv_init(&sfs->cv, "sendfile");
	}

#ifdef MAC
	error = mac_socket_check_send(td->td_ucred, so);
	if (error)
		goto out;
#endif

	/* If headers are specified copy them into mbufs. */
	if (hdr_uio != NULL) {
		hdr_uio->uio_td = td;
		hdr_uio->uio_rw = UIO_WRITE;
		if (hdr_uio->uio_resid > 0) {
			/*
			 * In FBSD < 5.0 the nbytes to send also included
			 * the header.  If compat is specified subtract the
			 * header size from nbytes.
			 */
			if (compat) {
				if (uap->nbytes > hdr_uio->uio_resid)
					uap->nbytes -= hdr_uio->uio_resid;
				else
					uap->nbytes = 0;
			}
			m = m_uiotombuf(hdr_uio, (mnw ? M_NOWAIT : M_WAITOK),
			    0, 0, 0);
			if (m == NULL) {
				error = mnw ? EAGAIN : ENOBUFS;
				goto out;
			}
			hdrlen = m_length(m, NULL);
		}
	}

	/*
	 * Protect against multiple writers to the socket.
	 *
	 * XXXRW: Historically this has assumed non-interruptibility, so now
	 * we implement that, but possibly shouldn't.
	 */
	(void)sblock(&so->so_snd, SBL_WAIT | SBL_NOINTR);

	/*
	 * Loop through the pages of the file, starting with the requested
	 * offset. Get a file page (do I/O if necessary), map the file page
	 * into an sf_buf, attach an mbuf header to the sf_buf, and queue
	 * it on the socket.
	 * This is done in two loops.  The inner loop turns as many pages
	 * as it can, up to available socket buffer space, without blocking
	 * into mbufs to have it bulk delivered into the socket send buffer.
	 * The outer loop checks the state and available space of the socket
	 * and takes care of the overall progress.
	 */
	for (off = uap->offset, rem = uap->nbytes; ; ) {
		int loopbytes = 0;
		int space = 0;
		int done = 0;

		/*
		 * Check the socket state for ongoing connection,
		 * no errors and space in socket buffer.
		 * If space is low allow for the remainder of the
		 * file to be processed if it fits the socket buffer.
		 * Otherwise block in waiting for sufficient space
		 * to proceed, or if the socket is nonblocking, return
		 * to userland with EAGAIN while reporting how far
		 * we've come.
		 * We wait until the socket buffer has significant free
		 * space to do bulk sends.  This makes good use of file
		 * system read ahead and allows packet segmentation
		 * offloading hardware to take over lots of work.  If
		 * we were not careful here we would send off only one
		 * sfbuf at a time.
		 */
		SOCKBUF_LOCK(&so->so_snd);
		if (so->so_snd.sb_lowat < so->so_snd.sb_hiwat / 2)
			so->so_snd.sb_lowat = so->so_snd.sb_hiwat / 2;
retry_space:
		if (so->so_snd.sb_state & SBS_CANTSENDMORE) {
			error = EPIPE;
			SOCKBUF_UNLOCK(&so->so_snd);
			goto done;
		} else if (so->so_error) {
			error = so->so_error;
			so->so_error = 0;
			SOCKBUF_UNLOCK(&so->so_snd);
			goto done;
		}
		space = sbspace(&so->so_snd);
		if (space < rem &&
		    (space <= 0 ||
		     space < so->so_snd.sb_lowat)) {
			if (so->so_state & SS_NBIO) {
				SOCKBUF_UNLOCK(&so->so_snd);
				error = EAGAIN;
				goto done;
			}
			/*
			 * sbwait drops the lock while sleeping.
			 * When we loop back to retry_space the
			 * state may have changed and we retest
			 * for it.
			 */
			error = sbwait(&so->so_snd);
			/*
			 * An error from sbwait usually indicates that we've
			 * been interrupted by a signal. If we've sent anything
			 * then return bytes sent, otherwise return the error.
			 */
			if (error) {
				SOCKBUF_UNLOCK(&so->so_snd);
				goto done;
			}
			goto retry_space;
		}
		SOCKBUF_UNLOCK(&so->so_snd);

		/*
		 * Reduce space in the socket buffer by the size of
		 * the header mbuf chain.
		 * hdrlen is set to 0 after the first loop.
		 */
		space -= hdrlen;

		/*
		 * Loop and construct maximum sized mbuf chain to be bulk
		 * dumped into socket buffer.
		 */
		while(space > loopbytes) {
			vm_pindex_t pindex;
			vm_offset_t pgoff;
			struct mbuf *m0;

			VM_OBJECT_LOCK(obj);
			/*
			 * Calculate the amount to transfer.
			 * Not to exceed a page, the EOF,
			 * or the passed in nbytes.
			 */
			pgoff = (vm_offset_t)(off & PAGE_MASK);
			xfsize = omin(PAGE_SIZE - pgoff,
			    obj->un_pager.vnp.vnp_size - uap->offset -
			    fsbytes - loopbytes);
			if (uap->nbytes)
				rem = (uap->nbytes - fsbytes - loopbytes);
			else
				rem = obj->un_pager.vnp.vnp_size -
				    uap->offset - fsbytes - loopbytes;
			xfsize = omin(rem, xfsize);
			xfsize = omin(space - loopbytes, xfsize);
			if (xfsize <= 0) {
				VM_OBJECT_UNLOCK(obj);
				done = 1;		/* all data sent */
				break;
			}

			/*
			 * Attempt to look up the page.  Allocate
			 * if not found or wait and loop if busy.
			 */
			pindex = OFF_TO_IDX(off);
			pg = vm_page_grab(obj, pindex, VM_ALLOC_NOBUSY |
			    VM_ALLOC_NORMAL | VM_ALLOC_WIRED | VM_ALLOC_RETRY);

			/*
			 * Check if page is valid for what we need,
			 * otherwise initiate I/O.
			 * If we already turned some pages into mbufs,
			 * send them off before we come here again and
			 * block.
			 */
			if (pg->valid && vm_page_is_valid(pg, pgoff, xfsize))
				VM_OBJECT_UNLOCK(obj);
			else if (m != NULL)
				error = EAGAIN;	/* send what we already got */
			else if (uap->flags & SF_NODISKIO)
				error = EBUSY;
			else {
				int bsize, resid;

				/*
				 * Ensure that our page is still around
				 * when the I/O completes.
				 */
				vm_page_io_start(pg);
				VM_OBJECT_UNLOCK(obj);

				/*
				 * Get the page from backing store.
				 */
				vfslocked = VFS_LOCK_GIANT(vp->v_mount);
				error = vn_lock(vp, LK_SHARED);
				if (error != 0)
					goto after_read;
				bsize = vp->v_mount->mnt_stat.f_iosize;

				/*
				 * XXXMAC: Because we don't have fp->f_cred
				 * here, we pass in NOCRED.  This is probably
				 * wrong, but is consistent with our original
				 * implementation.
				 */
				error = vn_rdwr(UIO_READ, vp, NULL, MAXBSIZE,
				    trunc_page(off), UIO_NOCOPY, IO_NODELOCKED |
				    IO_VMIO | ((MAXBSIZE / bsize) << IO_SEQSHIFT),
				    td->td_ucred, NOCRED, &resid, td);
				VOP_UNLOCK(vp, 0);
			after_read:
				VFS_UNLOCK_GIANT(vfslocked);
				VM_OBJECT_LOCK(obj);
				vm_page_io_finish(pg);
				if (!error)
					VM_OBJECT_UNLOCK(obj);
				mbstat.sf_iocnt++;
			}
			if (error) {
				vm_page_lock_queues();
				vm_page_unwire(pg, 0);
				/*
				 * See if anyone else might know about
				 * this page.  If not and it is not valid,
				 * then free it.
				 */
				if (pg->wire_count == 0 && pg->valid == 0 &&
				    pg->busy == 0 && !(pg->oflags & VPO_BUSY) &&
				    pg->hold_count == 0) {
					vm_page_free(pg);
				}
				vm_page_unlock_queues();
				VM_OBJECT_UNLOCK(obj);
				if (error == EAGAIN)
					error = 0;	/* not a real error */
				break;
			}

			/*
			 * Get a sendfile buf.  We usually wait as long
			 * as necessary, but this wait can be interrupted.
			 */
			if ((sf = sf_buf_alloc(pg,
			    (mnw ? SFB_NOWAIT : SFB_CATCH))) == NULL) {
				mbstat.sf_allocfail++;
				vm_page_lock_queues();
				vm_page_unwire(pg, 0);
				/*
				 * XXX: Not same check as above!?
				 */
				if (pg->wire_count == 0 && pg->object == NULL)
					vm_page_free(pg);
				vm_page_unlock_queues();
				error = (mnw ? EAGAIN : EINTR);
				break;
			}

			/*
			 * Get an mbuf and set it up as having
			 * external storage.
			 */
			m0 = m_get((mnw ? M_NOWAIT : M_WAITOK), MT_DATA);
			if (m0 == NULL) {
				error = (mnw ? EAGAIN : ENOBUFS);
				sf_buf_mext((void *)sf_buf_kva(sf), sf);
				break;
			}
			MEXTADD(m0, sf_buf_kva(sf), PAGE_SIZE, sf_buf_mext,
			    sfs, sf, M_RDONLY, EXT_SFBUF);
			m0->m_data = (char *)sf_buf_kva(sf) + pgoff;
			m0->m_len = xfsize;

			/* Append to mbuf chain. */
			if (m != NULL)
				m_cat(m, m0);
			else
				m = m0;

			/* Keep track of bits processed. */
			loopbytes += xfsize;
			off += xfsize;

			if (sfs != NULL) {
				mtx_lock(&sfs->mtx);
				sfs->count++;
				mtx_unlock(&sfs->mtx);
			}
		}

		/* Add the buffer chain to the socket buffer. */
		if (m != NULL) {
			int mlen, err;

			mlen = m_length(m, NULL);
			SOCKBUF_LOCK(&so->so_snd);
			if (so->so_snd.sb_state & SBS_CANTSENDMORE) {
				error = EPIPE;
				SOCKBUF_UNLOCK(&so->so_snd);
				goto done;
			}
			SOCKBUF_UNLOCK(&so->so_snd);
			CURVNET_SET(so->so_vnet);
			/* Avoid error aliasing. */
			err = (*so->so_proto->pr_usrreqs->pru_send)
				    (so, 0, m, NULL, NULL, td);
			CURVNET_RESTORE();
			if (err == 0) {
				/*
				 * We need two counters to get the
				 * file offset and nbytes to send
				 * right:
				 * - sbytes contains the total amount
				 *   of bytes sent, including headers.
				 * - fsbytes contains the total amount
				 *   of bytes sent from the file.
				 */
				sbytes += mlen;
				fsbytes += mlen;
				if (hdrlen) {
					fsbytes -= hdrlen;
					hdrlen = 0;
				}
			} else if (error == 0)
				error = err;
			m = NULL;	/* pru_send always consumes */
		}

		/* Quit outer loop on error or when we're done. */
		if (done) 
			break;
		if (error)
			goto done;
	}

	/*
	 * Send trailers. Wimp out and use writev(2).
	 */
	if (trl_uio != NULL) {
		sbunlock(&so->so_snd);
		error = kern_writev(td, uap->s, trl_uio);
		if (error == 0)
			sbytes += td->td_retval[0];
		goto out;
	}

done:
	sbunlock(&so->so_snd);
out:
	/*
	 * If there was no error we have to clear td->td_retval[0]
	 * because it may have been set by writev.
	 */
	if (error == 0) {
		td->td_retval[0] = 0;
	}
	if (uap->sbytes != NULL) {
		copyout(&sbytes, uap->sbytes, sizeof(off_t));
	}
	if (obj != NULL)
		vm_object_deallocate(obj);
	if (vp != NULL) {
		vfslocked = VFS_LOCK_GIANT(vp->v_mount);
		vrele(vp);
		VFS_UNLOCK_GIANT(vfslocked);
	}
	if (so)
		fdrop(sock_fp, td);
	if (m)
		m_freem(m);

	if (sfs != NULL) {
		mtx_lock(&sfs->mtx);
		if (sfs->count != 0)
			cv_wait(&sfs->cv, &sfs->mtx);
		KASSERT(sfs->count == 0, ("sendfile sync still busy"));
		cv_destroy(&sfs->cv);
		mtx_destroy(&sfs->mtx);
		bsd_free(sfs, M_TEMP);
	}

	if (error == ERESTART)
		error = EINTR;

	return (error);
}

/*
 * SCTP syscalls.
 * Functionality only compiled in if SCTP is defined in the kernel Makefile,
 * otherwise all return EOPNOTSUPP.
 * XXX: We should make this loadable one day.
 */
int
sctp_peeloff(td, uap)
	struct thread *td;
	struct sctp_peeloff_args /* {
		int	sd;
		caddr_t	name;
	} */ *uap;
{
#if (defined(INET) || defined(INET6)) && defined(SCTP)
	struct filedesc *fdp;
	struct file *nfp = NULL;
	int error;
	struct socket *head, *so;
	int fd;
	u_int fflag;

	fdp = td->td_proc->p_fd;
	AUDIT_ARG_FD(uap->sd);
	error = fgetsock(td, uap->sd, &head, &fflag);
	if (error)
		goto done2;
	error = sctp_can_peel_off(head, (sctp_assoc_t)uap->name);
	if (error)
		goto done2;
	/*
	 * At this point we know we do have a assoc to pull
	 * we proceed to get the fd setup. This may block
	 * but that is ok.
	 */

	error = falloc(td, &nfp, &fd);
	if (error)
		goto done;
	td->td_retval[0] = fd;

	CURVNET_SET(head->so_vnet);
	so = sonewconn(head, SS_ISCONNECTED);
	if (so == NULL) 
		goto noconnection;
	/*
	 * Before changing the flags on the socket, we have to bump the
	 * reference count.  Otherwise, if the protocol calls sofree(),
	 * the socket will be released due to a zero refcount.
	 */
        SOCK_LOCK(so);
        soref(so);                      /* file descriptor reference */
        SOCK_UNLOCK(so);

	ACCEPT_LOCK();

	TAILQ_REMOVE(&head->so_comp, so, so_list);
	head->so_qlen--;
	so->so_state |= (head->so_state & SS_NBIO);
	so->so_state &= ~SS_NOFDREF;
	so->so_qstate &= ~SQ_COMP;
	so->so_head = NULL;
	ACCEPT_UNLOCK();
//	finit(nfp, fflag, DTYPE_SOCKET, so, &socketops);
//	error = sctp_do_peeloff(head, so, (sctp_assoc_t)uap->name);
//	if (error)
//		goto noconnection;
//	if (head->so_sigio != NULL)
//		fsetown(fgetown(&head->so_sigio), &so->so_sigio);

noconnection:
	/*
	 * close the new descriptor, assuming someone hasn't ripped it
	 * out from under us.
	 */
	if (error)
		fdclose(fdp, nfp, fd, td);

	/*
	 * Release explicitly held references before returning.
	 */
	CURVNET_RESTORE();
done:
	if (nfp != NULL)
		fdrop(nfp, td);
	fputsock(head);
done2:
	return (error);
#else  /* SCTP */
	return (EOPNOTSUPP);
#endif /* SCTP */
}

int
sctp_generic_sendmsg (td, uap)
	struct thread *td;
	struct sctp_generic_sendmsg_args /* {
		int sd, 
		caddr_t msg, 
		int mlen, 
		caddr_t to, 
		__socklen_t tolen, 
		struct sctp_sndrcvinfo *sinfo, 
		int flags
	} */ *uap;
{
#if (defined(INET) || defined(INET6)) && defined(SCTP)
	struct sctp_sndrcvinfo sinfo, *u_sinfo = NULL;
	struct socket *so;
	struct file *fp = NULL;
	int use_rcvinfo = 1;
	int error = 0, len;
	struct sockaddr *to = NULL;
#ifdef KTRACE
	struct uio *ktruio = NULL;
#endif
	struct uio auio;
	struct iovec iov[1];

	if (uap->sinfo) {
		error = copyin(uap->sinfo, &sinfo, sizeof (sinfo));
		if (error)
			return (error);
		u_sinfo = &sinfo;
	}
	if (uap->tolen) {
		error = getsockaddr(&to, uap->to, uap->tolen);
		if (error) {
			to = NULL;
			goto sctp_bad2;
		}
	}

	AUDIT_ARG_FD(uap->sd);
	error = getsock(td->td_proc->p_fd, uap->sd, &fp, NULL);
	if (error)
		goto sctp_bad;
#ifdef KTRACE
	if (KTRPOINT(td, KTR_STRUCT))
		ktrsockaddr(to);
#endif

	iov[0].iov_base = uap->msg;
	iov[0].iov_len = uap->mlen;

	so = (struct socket *)fp->f_data;
#ifdef MAC
	error = mac_socket_check_send(td->td_ucred, so);
	if (error)
		goto sctp_bad;
#endif /* MAC */

	auio.uio_iov =  iov;
	auio.uio_iovcnt = 1;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_WRITE;
	auio.uio_td = td;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	len = auio.uio_resid = uap->mlen;
	CURVNET_SET(so->so_vnet);
	error = sctp_lower_sosend(so, to, &auio,
		    (struct mbuf *)NULL, (struct mbuf *)NULL,
		    uap->flags, use_rcvinfo, u_sinfo, td);
	CURVNET_RESTORE();
	if (error) {
		if (auio.uio_resid != len && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
		/* Generation of SIGPIPE can be controlled per socket. */
		if (error == EPIPE && !(so->so_options & SO_NOSIGPIPE) &&
		    !(uap->flags & MSG_NOSIGNAL)) {
			PROC_LOCK(td->td_proc);
			psignal(td->td_proc, SIGPIPE);
			PROC_UNLOCK(td->td_proc);
		}
	}
	if (error == 0)
		td->td_retval[0] = len - auio.uio_resid;
#ifdef KTRACE
	if (ktruio != NULL) {
		ktruio->uio_resid = td->td_retval[0];
		ktrgenio(uap->sd, UIO_WRITE, ktruio, error);
	}
#endif /* KTRACE */
sctp_bad:
	if (fp)
		fdrop(fp, td);
sctp_bad2:
	if (to)
		bsd_free(to, M_SONAME);
	return (error);
#else  /* SCTP */
	return (EOPNOTSUPP);
#endif /* SCTP */
}

int
sctp_generic_sendmsg_iov(td, uap)
	struct thread *td;
	struct sctp_generic_sendmsg_iov_args /* {
		int sd, 
		struct iovec *iov, 
		int iovlen, 
		caddr_t to, 
		__socklen_t tolen, 
		struct sctp_sndrcvinfo *sinfo, 
		int flags
	} */ *uap;
{
#if (defined(INET) || defined(INET6)) && defined(SCTP)
	struct sctp_sndrcvinfo sinfo, *u_sinfo = NULL;
	struct socket *so;
	struct file *fp = NULL;
	int use_rcvinfo = 1;
	int error=0, len, i;
	struct sockaddr *to = NULL;
#ifdef KTRACE
	struct uio *ktruio = NULL;
#endif
	struct uio auio;
	struct iovec *iov, *tiov;

	if (uap->sinfo) {
		error = copyin(uap->sinfo, &sinfo, sizeof (sinfo));
		if (error)
			return (error);
		u_sinfo = &sinfo;
	}
	if (uap->tolen) {
		error = getsockaddr(&to, uap->to, uap->tolen);
		if (error) {
			to = NULL;
			goto sctp_bad2;
		}
	}

	AUDIT_ARG_FD(uap->sd);
	error = getsock(td->td_proc->p_fd, uap->sd, &fp, NULL);
	if (error)
		goto sctp_bad1;

	error = copyiniov(uap->iov, uap->iovlen, &iov, EMSGSIZE);
	if (error)
		goto sctp_bad1;
#ifdef KTRACE
	if (KTRPOINT(td, KTR_STRUCT))
		ktrsockaddr(to);
#endif

	so = (struct socket *)fp->f_data;
#ifdef MAC
	error = mac_socket_check_send(td->td_ucred, so);
	if (error)
		goto sctp_bad;
#endif /* MAC */

	auio.uio_iov =  iov;
	auio.uio_iovcnt = uap->iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_WRITE;
	auio.uio_td = td;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	tiov = iov;
	for (i = 0; i <uap->iovlen; i++, tiov++) {
		if ((auio.uio_resid += tiov->iov_len) < 0) {
			error = EINVAL;
			goto sctp_bad;
		}
	}
	len = auio.uio_resid;
	CURVNET_SET(so->so_vnet);
	error = sctp_lower_sosend(so, to, &auio,
		    (struct mbuf *)NULL, (struct mbuf *)NULL,
		    uap->flags, use_rcvinfo, u_sinfo, td);
	CURVNET_RESTORE();
	if (error) {
		if (auio.uio_resid != len && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
		/* Generation of SIGPIPE can be controlled per socket */
		if (error == EPIPE && !(so->so_options & SO_NOSIGPIPE) &&
		    !(uap->flags & MSG_NOSIGNAL)) {
			PROC_LOCK(td->td_proc);
			psignal(td->td_proc, SIGPIPE);
			PROC_UNLOCK(td->td_proc);
		}
	}
	if (error == 0)
		td->td_retval[0] = len - auio.uio_resid;
#ifdef KTRACE
	if (ktruio != NULL) {
		ktruio->uio_resid = td->td_retval[0];
		ktrgenio(uap->sd, UIO_WRITE, ktruio, error);
	}
#endif /* KTRACE */
sctp_bad:
	bsd_free(iov, M_IOV);
sctp_bad1:
	if (fp)
		fdrop(fp, td);
sctp_bad2:
	if (to)
		bsd_free(to, M_SONAME);
	return (error);
#else  /* SCTP */
	return (EOPNOTSUPP);
#endif /* SCTP */
}

int
sctp_generic_recvmsg(td, uap)
	struct thread *td;
	struct sctp_generic_recvmsg_args /* {
		int sd, 
		struct iovec *iov, 
		int iovlen,
		struct sockaddr *from, 
		__socklen_t *fromlenaddr,
		struct sctp_sndrcvinfo *sinfo, 
		int *msg_flags
	} */ *uap;
{
#if (defined(INET) || defined(INET6)) && defined(SCTP)
	u_int8_t sockbufstore[256];
	struct uio auio;
	struct iovec *iov, *tiov;
	struct sctp_sndrcvinfo sinfo;
	struct socket *so;
	struct file *fp = NULL;
	struct sockaddr *fromsa;
	int fromlen;
	int len, i, msg_flags;
	int error = 0;
#ifdef KTRACE
	struct uio *ktruio = NULL;
#endif

	AUDIT_ARG_FD(uap->sd);
	error = getsock(td->td_proc->p_fd, uap->sd, &fp, NULL);
	if (error) {
		return (error);
	}
	error = copyiniov(uap->iov, uap->iovlen, &iov, EMSGSIZE);
	if (error) {
		goto out1;
	}

	so = fp->f_data;
#ifdef MAC
	error = mac_socket_check_receive(td->td_ucred, so);
	if (error) {
		goto out;
		return (error);
	}
#endif /* MAC */

	if (uap->fromlenaddr) {
		error = copyin(uap->fromlenaddr,
		    &fromlen, sizeof (fromlen));
		if (error) {
			goto out;
		}
	} else {
		fromlen = 0;
	}
	if(uap->msg_flags) {
		error = copyin(uap->msg_flags, &msg_flags, sizeof (int));
		if (error) {
			goto out;
		}
	} else {
		msg_flags = 0;
	}
	auio.uio_iov = iov;
	auio.uio_iovcnt = uap->iovlen;
  	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_td = td;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	tiov = iov;
	for (i = 0; i <uap->iovlen; i++, tiov++) {
		if ((auio.uio_resid += tiov->iov_len) < 0) {
			error = EINVAL;
			goto out;
		}
	}
	len = auio.uio_resid;
	fromsa = (struct sockaddr *)sockbufstore;

#ifdef KTRACE
	if (KTRPOINT(td, KTR_GENIO))
		ktruio = cloneuio(&auio);
#endif /* KTRACE */
	CURVNET_SET(so->so_vnet);
	error = sctp_sorecvmsg(so, &auio, (struct mbuf **)NULL,
		    fromsa, fromlen, &msg_flags,
		    (struct sctp_sndrcvinfo *)&sinfo, 1);
	CURVNET_RESTORE();
	if (error) {
		if (auio.uio_resid != (int)len && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
	} else {
		if (uap->sinfo)
			error = copyout(&sinfo, uap->sinfo, sizeof (sinfo));
	}
#ifdef KTRACE
	if (ktruio != NULL) {
		ktruio->uio_resid = (int)len - auio.uio_resid;
		ktrgenio(uap->sd, UIO_READ, ktruio, error);
	}
#endif /* KTRACE */
	if (error)
		goto out;
	td->td_retval[0] = (int)len - auio.uio_resid;

	if (fromlen && uap->from) {
		len = fromlen;
		if (len <= 0 || fromsa == 0)
			len = 0;
		else {
			len = MIN(len, fromsa->sa_len);
			error = copyout(fromsa, uap->from, (unsigned)len);
			if (error)
				goto out;
		}
		error = copyout(&len, uap->fromlenaddr, sizeof (socklen_t));
		if (error) {
			goto out;
		}
	}
#ifdef KTRACE
	if (KTRPOINT(td, KTR_STRUCT))
		ktrsockaddr(fromsa);
#endif
	if (uap->msg_flags) {
		error = copyout(&msg_flags, uap->msg_flags, sizeof (int));
		if (error) {
			goto out;
		}
	}
out:
	bsd_free(iov, M_IOV);
out1:
	if (fp) 
		fdrop(fp, td);

	return (error);
#else  /* SCTP */
	return (EOPNOTSUPP);
#endif /* SCTP */
}
#endif //0
