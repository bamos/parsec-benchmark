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
 *	@(#)if_host.c	(Princeton) 11/8/2010
 *  
 */


/*
 * Host interface driver for protocol testing and timing.
 */


#include "bsd_opt_atalk.h"
#include "bsd_opt_inet.h"
#include "bsd_opt_inet6.h"
#include "bsd_opt_ipx.h"


#include <sys/bsd_param.h>
#include <sys/bsd_systm.h>
#include <sys/bsd_kernel.h>
#include <sys/bsd_mbuf.h>
//#include <machine/bsd_bus.h>
#include <sys/bsd_rman.h>
#include <sys/bsd_socket.h>
#include <sys/bsd_sockio.h>

#include <net/bsd_if.h>
#include <net/bsd_if_types.h>
#include <net/bsd_route.h>
#include <net/bsd_vnet.h>
#include <net/bsd_ethernet.h>
#include <net/bsd_netisr.h>

#ifdef	INET
#include <netinet/bsd_in.h>
#include <netinet/bsd_in_var.h>
#endif

#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <sys/mman.h>
#include <poll.h>

#include "host_serv.h"
#include "if_host.h"
#include "shmbuf_ring.h"

extern int	mp_ncpus;

int		hif_ioctl(struct ifnet *, u_long, caddr_t);
int		hif_output(struct ifnet *ifp, struct mbuf *m, struct sockaddr *dst, struct route *ro);
void     	 hif_input(struct ifnet *ifp, struct mbuf *m);

static int	hif_create(char*);
static void	hif_destroy(struct ifnet *);
static int    hif_setup_interface(char *name, struct adapter* padapter);

static int hif_allocate_rings(struct adapter *adapter);
static int hif_free_rings(struct adapter* p_adapter);
static int	hif_create_thread(struct adapter* adap, int i,  void *(*routine)(void*), int type);
 
static void* hif_tx_link(void * t_args);
static void* hif_rx(void * t_args);
static void* hif_rx_link(void * t_args);

#ifdef FOR_SIMULATION
static char	*hif_sendbuf;
static char	*sendbuf_ptr;
static int	hif_link_send_fd;
static int	hif_link_recv_fd;
#endif

#ifdef PERFORMANCE_TUNING
extern struct timeval global_roi_start_tv;
#endif



VNET_DEFINE(struct ifnet *, hif);	/* Used externally */


static void
hif_destroy(struct ifnet *ifp)
{
	if_detach(ifp);
	if_free(ifp);
	
}

/*
 * @hif_create()
 * 
 * This function is responsible for:
 * 1) initilizing adapter sturcture.
 * 2) creating N hif_tx/rx threads and initialize them with tx/rx_rings
 * 3) creating 1 hif_rx_link thread 
 */
static int
hif_create(char* name)
{

	struct adapter	*p_adapter;
	int		error = 0;
	int		i;

	/* 
	 * Step.1 create adapter 
	 */
	p_adapter = (struct adapter*)bsd_malloc(sizeof(struct adapter), M_DEVBUF, M_ZERO);
	HIF_CORE_LOCK_INIT(p_adapter, name);

	/*
	 * Set the frame limits assuming
	 * standard ethernet sized frames.
	 */
	p_adapter->max_frame_size = ETHER_MAX_LEN; 
	p_adapter->min_frame_size = ETHER_MIN_LEN; 
	

	/* Initialize ifnet structure */
	error = hif_setup_interface(name, p_adapter);
	if(error < 0)
		goto failed;

	/* 
         * Step.2 creating N hif_tx/rx threads and initialize them with tx/rx_rings
         *  NOTE: the actualy ring buffer allocation is done in the hif_tx()
         *          and hif_rx() routine.
	 */

	p_adapter->num_tx_rings = 1;
	p_adapter->num_rx_rings = mp_ncpus;
	p_adapter->num_tx_buf = HIF_DEFAULT_TXB;
	p_adapter->num_rx_buf = HIF_DEFAULT_RXB;
	p_adapter->rx_mbuf_sz = MCLBYTES;
		
	/* Allocate Ring Buffers*/
	if((error = hif_allocate_rings(p_adapter)) < 0)
		goto failed;

	/* Create dedicated threads for rx */ 
	for(i = 0; i < p_adapter->num_rx_rings; i ++){
		error = hif_create_thread(p_adapter, i, hif_rx, THREAD_TYPE_RX_TCPIP); 
		if(error < 0)
			goto failed;
	}

	/* 
         * Step.3 creating 1 hif_tx_link & 1 hif_rx_link thread
         * NOTE: we call the following function just for creating a thread
         *          by passing index = -1;
	 */
	error = hif_create_thread(p_adapter, -1, hif_tx_link, THREAD_TYPE_TX_LINK); 
	if(error < 0)
		goto failed;

	error = hif_create_thread(p_adapter, -1, hif_rx_link, THREAD_TYPE_RX_LINK); 
	if(error < 0)
		goto failed;
	
	/* Tell the stack that the interface is not active */
	p_adapter->ifp->if_drv_flags &= ~(IFF_DRV_RUNNING | IFF_DRV_OACTIVE);
	p_adapter->ifp->if_drv_flags |= ~(IFF_DRV_RUNNING | IFF_UP);	
	
	return 0;

failed:
	hif_destroy(p_adapter->ifp);
	hif_free_rings(p_adapter);
	return error;

}

void
vnet_hif_init(const void *unused )
{
    hif_create("hif");
}
//VNET_SYSINIT(vnet_hif_init, SI_SUB_PROTO_IFATTACHDOMAIN, SI_ORDER_ANY,
//    vnet_hif_init, NULL);

static int 
hif_setup_interface(char *name, struct adapter* padapter)
{
	struct ifnet *ifp;
	struct ifreq ifr;
	struct sockaddr_in *hif_addr;
	//struct in_ifaddr * ia;
	//register struct ifaddr *ifa;	
	//int error = 0;

	ifp = padapter->ifp = if_alloc(IFT_OTHER);	
	if (ifp == NULL)
		return (ENOSPC);

	if_initname(ifp, name, HIF_MTU);
	ifp->if_mtu = HIF_MTU;
	ifp->if_softc = padapter;
	ifp->if_flags = IFF_SIMPLEX | IFF_MULTICAST;

	ifp->if_ioctl = hif_ioctl;
	ifp->if_output = hif_output;
	ifp->if_input = hif_input;
#if 0//__FreeBSD_version >= 800000
	ifp->if_transmit = igb_mq_start;
	ifp->if_qflush = igb_qflush;
#endif
	ifp->if_snd.ifq_maxlen = ifqmaxlen;

	/* Although the flag of hif is not set for CKSUM,
	 * we still set the cksum flag for incoming pacet. 
	 * Please check hif_link_recv_fd () */
	ifp->if_capabilities = ifp->if_capenable = 0; //IFCAP_HWCSUM;
	ifp->if_hwassist = 0;//HIF_CSUM_FEATURES;

	if_attach(ifp);
	

	memset(&ifr, 0, sizeof(ifr));
	bcopy(name, ifr.ifr_name, sizeof(ifr.ifr_name));
	hif_addr = (struct sockaddr_in*)&(ifr.ifr_addr);
	hif_addr->sin_len = sizeof(struct sockaddr_in);
	hif_addr->sin_family = AF_INET;
	hif_addr->sin_port = 0;

#ifdef UPTCP_CLIENT
	/* client: 10.10.10.11*/
	hif_addr->sin_addr.s_addr = htonl((u_int32_t)0x0a0a0a0b);
#else
	/* server: 10.10.10.12*/
	hif_addr->sin_addr.s_addr = htonl((u_int32_t)0x0a0a0a0c);
#endif

	bzero(&(hif_addr->sin_zero),8);

	in_control(NULL, SIOCSIFADDR,  (caddr_t)&ifr, ifp, NULL);	

	return 0;
}

/*
 * @hif_output()
 *
 * this function is called by ip_output to send data. hif_output() does not
 * perform the send operation, instead it just puts the packet into one of
 * adpater's tx_rings' ring_buf.
 */

int
hif_output(struct ifnet *ifp, struct mbuf *m, struct sockaddr *dst,
    struct route *ro)
{
	struct adapter	*p_adapter = ifp->if_softc;
	struct tx_ring	*txr;
	int 		i = 0, err = 0;

	/* Which queue to use */
	if ((m->m_flags & M_FLOWID) != 0)
		i = m->m_pkthdr.flowid % p_adapter->num_tx_rings;
	else 	i = 0;


	txr = &p_adapter->tx_rings[i];

	err = drbr_enqueue(ifp, txr->br, m);

	if(txr->tx_sleep){
		HIF_TX_LOCK(txr);
		HIF_TX_WAKEUP(txr);
		HIF_TX_UNLOCK(txr);
	}
	return (err);
}


void hif_input(struct ifnet *ifp, struct mbuf *m)
{
	struct ether_header *eh;
	u_short ether_type;

	/*
	 * 	 * If the CRC is still on the packet, triom it off. We do this once
	 * 	 	 * and once only in case we are re-entered. Nothing else on the
	 * 	 	 	 * Ethernet receive path expects to see the FCS.
	 * 	 	 	 	 */

	if (m->m_flags & M_HASFCS) {
		m_adj(m, -ETHER_CRC_LEN);
		m->m_flags &= ~M_HASFCS;
	}
	ifp->if_ibytes += m->m_pkthdr.len;

	eh = mtod(m, struct ether_header *);
	ether_type = ntohs(eh->ether_type);

	/*
	 * 	 * Reset layer specific mbuf flags to avoid confusing upper layers.
	 * 	 	 * Strip off Ethernet header.
	 * 	 	 	 */
	m->m_flags &= ~M_VLANTAG;
	m->m_flags &= ~(M_PROTOFLAGS);
	m_adj(m, ETHER_HDR_LEN);
	
	if(ether_type == ETHERTYPE_IP)
		netisr_dispatch(NETISR_IP, m);
	else 	m_freem(m);
}



static int
hif_allocate_rings(struct adapter *p_adapter)
{
	struct tx_ring *txr;
	struct rx_ring *rxr;
	int  	error = 0;
	int 	i;


	/* First allocate the TX ring struct memory */
	if (!(p_adapter->tx_rings =
	    (struct tx_ring *) bsd_malloc(sizeof(struct tx_ring) * p_adapter->num_tx_rings, M_DEVBUF, M_ZERO))) {
		printf("hif_allocate_rings(): Unable to allocate TX ring memory\n");
		error = ENOMEM;
		goto fail;
	}
	txr = p_adapter->tx_rings;

	/* Next allocate the RX */
	if (!(p_adapter->rx_rings =
	    (struct rx_ring *) bsd_malloc(sizeof(struct rx_ring) * p_adapter->num_rx_rings, M_DEVBUF, M_ZERO))) {
		printf("hif_allocate_rings(): Unable to allocate RX ring memory\n");
		error = ENOMEM;
		goto rx_fail;
	}
	rxr = p_adapter->rx_rings;


	/*
	 * Now set up the TX queues, txconf is needed to handle the
	 * possibility that things fail midcourse and we need to
	 * undo memory gracefully
	 */ 
	for (i = 0; i < p_adapter->num_tx_rings; i++) {
		/* Set up some basics */
		txr = &p_adapter->tx_rings[i];
		txr->adapter = p_adapter;
		txr->me = i;

		/* Initialize the TX lock */
		snprintf(txr->mtx_name, sizeof(txr->mtx_name), "%s:tx(%d)",
		    p_adapter->ifp->if_dname, txr->me);
		mtx_init(&txr->tx_mtx, txr->mtx_name, NULL, MTX_DEF);
		txr->tx_cond = host_pthread_cond_init();
		txr->tx_sleep = 0;

		/* Allocate a buf ring */
		txr->br = buf_ring_alloc(HIF_BR_SIZE, M_DEVBUF,
		    M_WAITOK, &txr->tx_mtx);
		
		if(txr->br == NULL)
			goto err_tx_buf;
	}

	/*
	 * Next the RX queues...
	 */ 
	for (i = 0; i < p_adapter->num_rx_rings; i++) {
		rxr = &p_adapter->rx_rings[i];
		rxr->adapter = p_adapter;
		rxr->me = i;

		/* Initialize the RX lock */
		snprintf(rxr->mtx_name, sizeof(rxr->mtx_name), "%s:rx(%d)",
		    p_adapter->ifp->if_dname, rxr->me);
		mtx_init(&rxr->rx_mtx, rxr->mtx_name, NULL, MTX_DEF);
		rxr->rx_cond = host_pthread_cond_init();
		rxr->rx_sleep = 0;

        	/* Allocate a ring buffer for the ring*/
		rxr->br = buf_ring_alloc(HIF_BR_SIZE, M_DEVBUF,
		    M_WAITOK, &rxr->rx_mtx);	

		if(rxr->br == NULL)
			goto err_rx_buf;
	}

return (0);

err_rx_buf:
	for (i = 0; i < p_adapter->num_tx_rings; i++) {	
		txr = &p_adapter->tx_rings[i];
		bsd_free(txr->br, M_DEVBUF);
	}
err_tx_buf:
	bsd_free(p_adapter->rx_rings, M_DEVBUF);
rx_fail:
	bsd_free(p_adapter->tx_rings, M_DEVBUF);
fail:
	return (error);
}


static int hif_free_rings(struct adapter* p_adapter)
{
	int i;
	struct tx_ring  *txr;
	struct rx_ring  *rxr;	
	
	if(p_adapter->tx_rings != NULL){
		for (i = 0; i < p_adapter->num_tx_rings; i++) {
			txr = &p_adapter->tx_rings[i];
			host_pthread_cond_destroy(txr->tx_cond);
			if(txr->br != NULL && txr->br->br_ring != NULL)
				bsd_free(txr->br->br_ring, M_DEVBUF);
		}
	}
	bsd_free(p_adapter->tx_rings, M_DEVBUF);


	if(p_adapter->rx_rings != NULL){
		for (i = 0; i < p_adapter->num_rx_rings; i++) {
			rxr = &p_adapter->rx_rings[i];
			host_pthread_cond_destroy(rxr->rx_cond);
			if(rxr->br != NULL && rxr->br->br_ring != NULL)
				bsd_free(rxr->br->br_ring, M_DEVBUF);
		}
	}
	bsd_free(p_adapter->rx_rings, M_DEVBUF);
	
	return 0;
}

static struct mbuf*
hif_get_buf(struct adapter *p_adapter)
{
	struct mbuf		*mp;

	mp = m_getjcl(M_DONTWAIT, MT_DATA,
	    M_PKTHDR, p_adapter->rx_mbuf_sz);
	if (mp == NULL)
		return NULL;
	mp->m_len = p_adapter->rx_mbuf_sz;
	mp->m_flags |= M_PKTHDR;
	mp->m_pkthdr.len = mp->m_len;

	return mp;
}

static int	
hif_create_thread(struct adapter* adap, int index,  void *(*routine)(void*), int type)
{
	struct thread_args  *t_args;
	int error;
	
	t_args = (struct thread_args*) bsd_malloc(sizeof(struct thread_args), M_DEVBUF, M_ZERO);
	if(!t_args){
		printf("hif_alloc_ring_and_thread(): bsd_malloc() for thread_args failed\n");
		return -1;
	}
	t_args->p_adapter = adap;
	t_args->ring_index = index;


	/* we create a new thread to handle the accept socket */
	error = host_thread_create(routine, (void*)t_args, type);
	if(error < 0){
		printf("hif_alloc_ring_and_thread(): pthread_create() failed\n");
		bsd_free(t_args, M_DEVBUF);
		return -1;
	}
	
	return 0;

}

/*
 * @hif_tx_link()
 
 * This is a routine running within a thread. hif_tx_link() is responsible for the
 * following operations:
 * 1) bind the local socket to a listen port 
 * 2) check if the ring buffer is empty
 *     if so, sleep;
 *     or, send data out via raw socket
 *
 */
static void* hif_tx_link(void * t_args)
{
	struct thread_args *args = (struct thread_args*)t_args;
	struct adapter        *p_adapter = args->p_adapter;
	struct ifnet              *ifp = p_adapter->ifp;
	int 				index = 0; //args->ring_index; only one tx_thread
	struct tx_ring		*txr = &p_adapter->tx_rings[index];
        struct mbuf     *next;

	sigset_t signal_set;

	/*
	* Block all signals in this thread. 
	*/
	sigfillset ( &signal_set );
	pthread_sigmask (SIG_BLOCK, &signal_set, NULL );	


	bsd_free(args, M_DEVBUF);  

#ifdef FOR_SIMULATION
	int res;
	/* Step.1 create send buffer */
	if((hif_sendbuf = (char*)malloc(HIF_MTU)) == NULL){
		printf("hif_tx(): malloc hif_sendbuf error\n");
		return (void*)-1; 
	}

        /* Step.2 create a socket */
#ifdef UPTCP_CLIENT
	if((hif_link_send_fd = host_connect_link(HIF_RECV_PORT)) < 0){ //client
#else
	if((hif_link_send_fd = host_connect_link(HIF_SEND_PORT)) < 0){ //server
#endif
		printf("hif_tx(): create raw socket failed\n");
		return (void*)-1; 
	}

#else //!FOR_SIMULATION

        int 	tmpfd, shmid;
	char	shm_key_file[64];
	struct shm_bufring	*hif_tx_shmbuf_ring;

	/*Step.1 amalloc share memory*/	
#ifdef UPTCP_CLIENT 
	sprintf(shm_key_file, "/tmp/uptcp_hif_rx_buf.dat");	//client
#else
	sprintf(shm_key_file, "/tmp/uptcp_hif_tx_buf.dat");	//server	
#endif
	
	if((tmpfd = open(shm_key_file, O_CREAT | O_RDWR, S_IRWXU)) < 0){
		printf("Error: open keyFile error\n");
		return (void*)-1;
	}
	close(tmpfd);

	key_t shm_key = ftok(shm_key_file, 1);

	/* Step.1.5 check if already existed? */
	if((shmid = shmget(shm_key, sizeof(struct shm_bufring), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0){
        	printf("Error: shmget error\n");
		return (void*)-1;
	}

    	if((hif_tx_shmbuf_ring = (struct shm_bufring *) shmat(shmid, NULL, 0)) == (void*)-1){
        	printf("Error: shmat error\n");
		return (void*)-1;
	}	

	/* Step.2 initial shared buffer */
#ifndef UPTCP_CLIENT
	hif_tx_shmbuf_ring->read_off = 0;
	hif_tx_shmbuf_ring->write_off = 0;
#endif

#endif //!FOR_SIMULATION



	/* Step.3 handle data of tx_ring's ring_buf*/
	while (1) {
		next = drbr_dequeue(ifp, txr->br);
		if (next == NULL){
			host_thread_enter_wait_region();

			HIF_TX_LOCK(txr);
			txr->tx_sleep = 1;
			HIF_TX_SLEEP(txr);
			txr->tx_sleep = 0;
			HIF_TX_UNLOCK(txr);

			host_thread_exit_wait_region();
			continue;
		}

#ifdef PERFORMANCE_TUNING
		if(0){
			struct timeval cur_time;
			gettimeofday(&(cur_time), NULL);
			unsigned long long  _interval = (cur_time.tv_sec * 1000000 + cur_time.tv_usec) - (global_roi_start_tv.tv_sec * 1000000 + global_roi_start_tv.tv_usec);

			printf("%llu: %-10s  p_head=%u, p_tail=%u, c_head=%d, c_tail=%d\n",
					 _interval, 
					"txl_txr", 
					txr->br->br_prod_head,
					txr->br->br_prod_tail,
					txr->br->br_cons_head,
					txr->br->br_cons_tail);
			}		
#endif


#ifdef FOR_SIMULATION
		if(next->m_next == NULL){
			/* send out the packet*/
			if ((res = host_link_send(hif_link_send_fd, next->m_data, next->m_pkthdr.len)) < 0) {
				if (next != NULL)
					res = drbr_enqueue(ifp, txr->br, next);
			}else {
				drbr_stats_update(ifp, next->m_pkthdr.len, next->m_flags);
				m_freem(next);
			}
		} else {
			sendbuf_ptr = hif_sendbuf;
			struct mbuf *m;
			for(m = next; m != NULL; m = m->m_next){
				bcopy(mtod(m, void *), (void*)sendbuf_ptr, m->m_len);
				sendbuf_ptr += m->m_len;
			}
			/* send out the packet*/
			if ((res = host_link_send(hif_link_send_fd, hif_sendbuf, next->m_pkthdr.len)) < 0) {
				if (next != NULL)
					res = drbr_enqueue(ifp, txr->br, next);
			}else {
				drbr_stats_update(ifp, next->m_pkthdr.len, next->m_flags);
				m_freem(next);
			}
		}

#else //!FOR_SIMULATION
		shm_bufring_enqueue(hif_tx_shmbuf_ring, next);
		m_freem(next);
#endif //!FOR_SIMULATION
	}

#ifdef FOR_SIMULATION
	free(hif_sendbuf);
#endif
	return NULL;
}


/*
 * @hif_rx()
 
 * This is a routine running within a thread. hif_rx() is responsible for the
 * following operations:
 * 1) check if the ring buffer is empty
 *     if so, sleep;
 *     or, call if_input() to handle data, and eventually send the data 
 *          to sockbuf
 */
static void* hif_rx(void * t_args)
{
	struct thread_args *args = (struct thread_args*)t_args;
	struct adapter		*p_adapter = args->p_adapter;
	int 				index = args->ring_index;
	struct ifnet		*ifp = p_adapter->ifp;
	struct rx_ring		*rxr = &p_adapter->rx_rings[index];
	struct mbuf*    next;

	sigset_t signal_set;
	
#ifdef PERFORMANCE_TUNING
	int	just_sleeped = 0;
#endif
	/*
	* Block all signals in this thread. 
	*/
	sigfillset ( &signal_set );
	pthread_sigmask (SIG_BLOCK, &signal_set, NULL );	


	bsd_free(args, M_DEVBUF);
	
	while(1){
		next = drbr_dequeue(ifp, rxr->br);
		if (next == NULL){
#ifdef PERFORMANCE_TUNING
			if(1){
				struct timeval cur_time;
				gettimeofday(&(cur_time), NULL);
				unsigned long long  _interval = (cur_time.tv_sec * 1000000 + cur_time.tv_usec) - (global_roi_start_tv.tv_sec * 1000000 + global_roi_start_tv.tv_usec);

				printf("%llu: %-10s  RXR-%d (p_head=%-4u \tp_tail=%-4u \tc_head=%-4d \tc_tail=%d)\n",
					 _interval, 
					"rx_sleep", 
					rxr->me,	
					rxr->br->br_prod_head,
					rxr->br->br_prod_tail,
					rxr->br->br_cons_head,
					rxr->br->br_cons_tail);
				just_sleeped = 1;
			}		
#endif
			host_thread_enter_wait_region();

			HIF_RX_LOCK(rxr);
			rxr->rx_sleep = 1;
			HIF_RX_SLEEP(rxr);
			rxr->rx_sleep = 0;
			HIF_RX_UNLOCK(rxr);

			host_thread_exit_wait_region();

			continue;
		}
#ifdef PERFORMANCE_TUNING
		if(1){
			if(just_sleeped == 1){
				struct timeval cur_time;
				gettimeofday(&(cur_time), NULL);
				unsigned long long  _interval = (cur_time.tv_sec * 1000000 + cur_time.tv_usec) - (global_roi_start_tv.tv_sec * 1000000 + global_roi_start_tv.tv_usec);

#define IP_HLEN_OFFSET          0 
#define IP_TYPE_OFFSET          9 
#define TCP_SEQ_OFFSET          4  
#define TCP_ACK_OFFSET          8  
                        char *buf_ptr = mtod(next, char *); 
                        int iphlen = ((*(unsigned char*)(buf_ptr + IP_HLEN_OFFSET)) & 0xF) << 2; 
                        unsigned int tcpseq = ntohl(*(unsigned int*)(buf_ptr + TCP_SEQ_OFFSET + iphlen));  
                        unsigned int tcpack = ntohl(*(unsigned int*)(buf_ptr + TCP_ACK_OFFSET + iphlen));  

				printf("%llu: %-10s  RXR-%d (p_head=%-4u \tp_tail=%-4u \tc_head=%-4d \tc_tail=%d) \tTCP (seq=%-8u ack=%u)\n",
						 _interval, 
						"rx_wakeup", 
						rxr->me,	
						rxr->br->br_prod_head,
						rxr->br->br_prod_tail,
						rxr->br->br_cons_head,
						rxr->br->br_cons_tail,
						tcpseq,
						tcpack);
					just_sleeped = 0;
			}
		}		
#endif

#ifdef PERFORMANCE_TUNING
		//host_thread_enter_performance_region();
#endif
		netisr_dispatch(NETISR_IP, next);
#ifdef PERFORMANCE_TUNING
		//host_thread_exit_performance_region();
#endif


		drbr_stats_update(ifp, next->m_pkthdr.len, next->m_flags);
	}
	
	return NULL;
}

/*
 * @hif_rx_link()
 
 * This is a routine running within a thread. It is responsible for capture 
 * all incoming packet received by host NIC and put the data into 
 * corresponding rx_rings.
 *
 */
static void* hif_rx_link(void * t_args)
{
	struct thread_args   *args = (struct thread_args*)t_args;
	struct adapter		*p_adapter = args->p_adapter;
	struct ifnet		*ifp = p_adapter->ifp;
	struct rx_ring		*rxr;
	struct mbuf              *mp;
	int	i = 0;	
	int    error;   
	unsigned long long	total_packets = 0;

	sigset_t signal_set;

	/*
	* Block all signals in this thread. 
	*/
	sigfillset ( &signal_set );
	pthread_sigmask (SIG_BLOCK, &signal_set, NULL );	

	bsd_free(args, M_DEVBUF);

#ifdef FOR_SIMULATION
 	/*
	 * Create hif_link_recv_fd		
	 */		

        /* Step.0 create a socket */
#ifdef UPTCP_CLIENT
	if((hif_link_recv_fd = host_setup_link(HIF_SEND_PORT)) < 0){
#else 
	if((hif_link_recv_fd = host_setup_link(HIF_RECV_PORT)) < 0){
#endif
		printf("hif_tx(): create raw socket failed\n");
		return (void*)-1; 
	}

#else //!FOR_SIMULATION

        int 	tmpfd, shmid;
	char	shm_key_file[64];
	struct shm_bufring	*hif_rx_shmbuf_ring;

	/* malloc share memory*/	
#ifdef UPTCP_CLIENT 
	sprintf(shm_key_file, "/tmp/uptcp_hif_tx_buf.dat");	//client
#else
	sprintf(shm_key_file, "/tmp/uptcp_hif_rx_buf.dat");	//server	
#endif

	if((tmpfd = open(shm_key_file, O_CREAT | O_RDWR, S_IRWXU)) < 0){
		printf("Error: open keyFile error\n");
		return (void*)-1;
	}
	close(tmpfd);

	key_t shm_key = ftok(shm_key_file, 1);

	if((shmid = shmget(shm_key, sizeof(struct shm_bufring), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0){
        	printf("Error: shmget error\n");
		return (void*)-1;
	}

    	if((hif_rx_shmbuf_ring = (struct shm_bufring *) shmat(shmid, NULL, 0)) == (void*)-1){
        	printf("Error: shmat error\n");
		return (void*)-1;
	}	

#ifndef UPTCP_CLIENT
	hif_rx_shmbuf_ring->read_off = 0;
	hif_rx_shmbuf_ring->write_off = 0;
#endif
#endif //!FOR_SIMULATION

	/* Step.2 recv */
	while(1) {

		/* get a mbuf */
		if ((mp = hif_get_buf(p_adapter)) == NULL) {
			ifp->if_iqdrops++;
			continue;
		}
#ifdef FOR_SIMULATION
		if((error = host_link_recv(hif_link_recv_fd, mp->m_data, mp->m_len)) > 0){
#else
		if((error = shm_bufring_dequeue(hif_rx_shmbuf_ring, mp->m_data, mp->m_len)) > 0){
#ifdef PERFORMANCE_TUNING
			if(0){
				struct timeval cur_time;
				gettimeofday(&(cur_time), NULL);
				unsigned long long  _interval = (cur_time.tv_sec * 1000000 + cur_time.tv_usec) - (global_roi_start_tv.tv_sec * 1000000 + global_roi_start_tv.tv_usec);

				printf("%llu: %-10s  read=%-5d : write=%d\n",
					 _interval, 
					"rxl_shm", 
					hif_rx_shmbuf_ring->read_off, 
					hif_rx_shmbuf_ring->write_off);
			}		
#endif

#endif //FOR_SIMULATION

			// FIXME: we need a way to classify incoming packets, 
			// for example, hashing based on port.
			//
			// We just use Round-Robin policy
			//
			if ((mp->m_flags & M_FLOWID) != 0)
				i = mp->m_pkthdr.flowid % p_adapter->num_rx_rings;
			else	i = total_packets % mp_ncpus;
			total_packets ++;


			 mp->m_pkthdr.rcvif = ifp;
				
			/* Although the flag of hif is not set for CKSUM,
			 * we still set the cksum flag for incoming pacet. 
			 * Please check hif_link_recv_fd () */
			//if (ifp->if_capenable & IFCAP_RXCSUM) {
			mp->m_pkthdr.csum_data = 0xffff;
			mp->m_pkthdr.csum_flags = HIF_CSUM_SET;
			//}
			mp->m_pkthdr.csum_flags &= ~HIF_CSUM_FEATURES;
			rxr = &p_adapter->rx_rings[i];
			error = drbr_enqueue(ifp, rxr->br, mp);		
#ifdef PERFORMANCE_TUNING
			if(0){
				struct timeval cur_time;
				gettimeofday(&(cur_time), NULL);
				unsigned long long  _interval = (cur_time.tv_sec * 1000000 + cur_time.tv_usec) - (global_roi_start_tv.tv_sec * 1000000 + global_roi_start_tv.tv_usec);

				printf("%llu: %-10s  p_head=%u, p_tail=%u, c_head=%d, c_tail=%d\n",
					 _interval, 
					"rxl_rxr", 
					rxr->br->br_prod_head,
					rxr->br->br_prod_tail,
					rxr->br->br_cons_head,
					rxr->br->br_cons_tail);
			}		
#endif


			if(rxr->rx_sleep){
				HIF_RX_LOCK(rxr);
				HIF_RX_WAKEUP(rxr);
				HIF_RX_UNLOCK(rxr);
			}
		}
		
	} //while (1)
 	return NULL;
}

/*
 * Process an ioctl request.
 */
/* ARGSUSED */
int
hif_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	return 0;
}

