/**************************************************************************
 *
 * Copyright (c) 2007-2009 Kip Macy kmacy@freebsd.org
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. The name of Kip Macy nor the names of other
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 *
 ***************************************************************************/

#ifndef	_SHMBUF_RING_H_
#define	_SHMBUF_RING_H_

#include <machine/bsd_cpu.h>
#include <sys/bsd_mbuf.h>
#include <netinet/bsd_in.h>
#include <unistd.h>

#include <pthread.h>
#include "if_host.h"
#include "host_serv.h"

extern void mtx_lock(struct mtx *m);
extern void mtx_unlock(struct mtx * m);

struct shm_bufring {
	char	buf[HIF_SHMBUF_SIZE*HIF_MAX_SHMBUF];
	volatile int	read_off;
	volatile int 	write_off;
	char	pad[4088];
};

#if 0
struct shm_bufring {
	volatile uint32_t	br_prod_head;
	volatile uint32_t	br_prod_tail;	
	int              	br_prod_size;
	int              	br_prod_mask;
	uint64_t		br_drops;
	uint64_t		br_prod_bufs;
	uint64_t		br_prod_bytes;
	/*
	 * Pad out to next L2 cache line
	 */
	uint64_t	  	_pad0[11];

	volatile uint32_t	br_cons_head;
	volatile uint32_t	br_cons_tail;
	int		 	br_cons_size;
	int              	br_cons_mask;
	
	/*
	 * Pad out to next L2 cache line
	 */
	uint64_t	  	_pad1[14];
//#ifdef DEBUG_SHMBUFRING
	struct mtx		*br_lock;
//#endif	
	void			*br_ring[0];
};
#endif //0

static inline int shm_bufring_enqueue(struct shm_bufring *br, struct mbuf* m)
{
	struct mbuf  *mm;
	char    *shmbuf, *buf_ptr;
	int	next_off = (br->write_off + 1) % HIF_MAX_SHMBUF;

	/* wait for the full case */
	while(next_off == br->read_off)
		next_off = (br->write_off + 1) % HIF_MAX_SHMBUF;

	shmbuf = br->buf + HIF_SHMBUF_SIZE * br->write_off;
	
	/* copy data into shm buffer*/
	if(m->m_next == NULL){
		/* send out the packet*/
		bcopy(m->m_data, shmbuf, m->m_pkthdr.len);
	} else {
		buf_ptr = shmbuf;
		for(mm = m; mm != NULL; mm = mm->m_next){
			bcopy(mtod(mm, void *), (void*)buf_ptr, mm->m_len);
			buf_ptr += mm->m_len;
		}
	}

	/* update buffer*/
	br->write_off = (br->write_off + 1) % HIF_MAX_SHMBUF;

	return 0;
}

/*
 * single-consumer dequeue 
 * use where dequeue is protected by a lock
 * e.g. a network driver's tx queue lock
 */
static inline int shm_bufring_dequeue(struct shm_bufring *br, char* buf, int len)
{
	unsigned short pkt_len;
	char	*pkt_ptr;
	int	waited = 0;

	/* wait for the empty case */
	while(br->read_off == br->write_off){
		if(waited == 0){
			host_thread_enter_wait_region();
			waited = 1;
		}
		usleep(1);
	}

	if(waited == 1){
		host_thread_exit_wait_region();
		waited = 0;
	}

	pkt_ptr = br->buf + HIF_SHMBUF_SIZE * br->read_off;

	/* copy data from shm to mbuf*/
	pkt_len = ntohs(*((unsigned short*)(pkt_ptr+2)));
	//pkt_len = (pkt_len << 8) | pkt_ptr[3];
	if(pkt_len > len){
		return 0;
	} else{
		bcopy(pkt_ptr, buf, pkt_len);
		br->read_off = (br->read_off + 1) % HIF_MAX_SHMBUF;
		return pkt_len;
	}
}

#endif
