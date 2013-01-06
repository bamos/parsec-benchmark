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
 *	@(#)if_host.h	(Princeton) 11/19/2010
 *  
 */


/*
 * Host interface driver for protocol testing and timing. 
 * This driver is derived from Intel Gigabit Ethernet Driver.
 */



#ifndef _IF_HOST_H_
#define _IF_HOST_H_

/*
 * Listen Ports
 */ 
#define HIF_SEND_PORT    48972
#define HIF_RECV_PORT    48973

/*
 * Share memory buffer
 */
#define HIF_MAX_SHMBUF 		8192
#define HIF_SHMBUF_SIZE		2048

/*
 * HIF_TXB: Maximum number of Transmit Buffers in One TX Ring
 */
#define HIF_MIN_TXB		80
#define HIF_DEFAULT_TXB		256
#define HIF_MAX_TXB		4096

/*
 * HIF_RXB: Maximum number of Receive Buffers in one RX Ring
 */
#define HIF_MIN_RXB		80
#define HIF_DEFAULT_RXB		256
#define HIF_MAX_RXB		4096


/*
 * Micellaneous constants
 */
#define HIF_VENDOR_ID			0x8086

#define HIF_JUMBO_PBA			0x00000028
#define HIF_DEFAULT_PBA			0x00000030
#define HIF_SMARTSPEED_DOWNSHIFT	3
#define HIF_SMARTSPEED_MAX		15
#define HIF_MAX_LOOP			10
#define HIF_RX_PTHRESH			16
#define HIF_RX_HTHRESH			8
#define HIF_RX_WTHRESH			1

#define MAX_NUM_MULTICAST_ADDRESSES     128
#define PCI_ANY_ID                      (~0U)
#define ETHER_ALIGN                     2
#define HIF_TX_BUFFER_SIZE		((uint32_t) 1514)
#define HIF_FC_PAUSE_TIME		0x0680
#define HIF_EEPROM_APME			0x400
#define HIF_MTU				1500
//#define HIF_MTU				16436 //16KB

/*
 * The number of TX_Rings and RX_Rings
*/
#define HIF_TXRING_NUM		1	
#define HIF_RXRING_NUM		1	


#define HIF_MAX_SCATTER		64
#define HIF_VFTA_SIZE		128
#define HIF_BR_SIZE		4096	/* ring buf size */
#define HIF_TSO_SIZE		(65535 + sizeof(struct ether_vlan_header))
#define HIF_TSO_SEG_SIZE	4096	/* Max dma segment size */
#define HIF_HDR_BUF		128
#define ETH_ZLEN		60
#define ETH_ADDR_LEN		6

/* Offload bits in mbuf flag */
#if __FreeBSD_version >= 800000
#define CSUM_OFFLOAD		(CSUM_IP|CSUM_TCP|CSUM_UDP|CSUM_SCTP)
#else
#define CSUM_OFFLOAD		(CSUM_IP|CSUM_TCP|CSUM_UDP)
#endif

#ifdef NIF_CSUM_ENABLE
#define HIF_CSUM_FEATURES	(CSUM_IP | CSUM_TCP | CSUM_UDP | CSUM_SCTP)
#define HIF_CSUM_SET		(CSUM_DATA_VALID | CSUM_PSEUDO_HDR | \
				    CSUM_IP_CHECKED | CSUM_IP_VALID | \
				    CSUM_SCTP_VALID)
#else
#define HIF_CSUM_FEATURES	0
#define HIF_CSUM_SET		0	
#endif

/* Header split codes for get_buf */
#define HIF_CLEAN_HEADER		1
#define HIF_CLEAN_PAYLOAD		2
#define HIF_CLEAN_BOTH			3

/*
 * Interrupt Moderation parameters
 */
#define HIF_LOW_LATENCY         128
#define HIF_AVE_LATENCY         450
#define HIF_BULK_LATENCY        1200
#define HIF_LINK_ITR            2000

/* Precision Time Sync (IEEE 1588) defines */
#define ETHERTYPE_IEEE1588	0x88F7
#define PICOSECS_PER_TICK	20833
#define TSYNC_PORT		319 /* UDP port for the protocol */


;

struct hif_tx_buffer {
	int		next_eop;  /* Index of the desc to watch */
        struct mbuf    *m_head;
};

struct hif_rx_buffer {
        struct mbuf    *m_head;
};


/*
 * Transmit ring: one per tx queue
 */
struct tx_ring {
	struct adapter		*adapter;
	unsigned int			me;
	struct mtx		tx_mtx;
	char			mtx_name[16];
	void*			tx_cond;
	unsigned int		tx_sleep;

	struct buf_ring		*br;

	unsigned long			no_buf_avail;
	unsigned long			tx_irq;
	unsigned long			tx_packets;
};

/*
 * Receive ring: one per rx queue
 */
struct rx_ring {
	struct adapter		*adapter;
	unsigned int			me;

	struct mtx		rx_mtx;
	char			mtx_name[16];
	void*			rx_cond;
	unsigned int		rx_sleep;
	
	struct buf_ring		*br;	

	/* Soft stats */
	unsigned long			rx_irq;
	unsigned long			rx_split_packets;
	unsigned long			rx_packets;
	unsigned long			rx_bytes;
};

struct adapter {
	struct ifnet	*ifp;

	int		if_flags;
	int		max_frame_size;
	int		min_frame_size;
	struct mtx	core_mtx;

        unsigned short		num_tx_rings;
        unsigned short		num_rx_rings;

	/*
	 * Transmit rings
	 */
	struct tx_ring		*tx_rings;
        unsigned short			num_tx_buf;

	/* 
	 * Receive rings
	 */
	struct rx_ring		*rx_rings;
        unsigned short			num_rx_buf;
	unsigned int			rx_mbuf_sz;		
	//int			rx_process_limit;


	/* Misc stats maintained by the driver */
	unsigned long	dropped_pkts;
	unsigned long	mbuf_defrag_failed;
	unsigned long	mbuf_header_failed;
	unsigned long	mbuf_packet_failed;
	unsigned long	rx_overruns;

};

struct thread_args{
	struct adapter*  p_adapter;
	int              ring_index;
};

#define	HIF_CORE_LOCK_INIT(_sc, _name) mtx_init(&(_sc)->core_mtx, _name, "IGB Core Lock", MTX_DEF)
#define	HIF_CORE_LOCK_DESTROY(_sc)	mtx_destroy(&(_sc)->core_mtx)
#define	HIF_TX_LOCK_DESTROY(_sc)		mtx_destroy(&(_sc)->tx_mtx)
#define	HIF_RX_LOCK_DESTROY(_sc)		mtx_destroy(&(_sc)->rx_mtx)
#define	HIF_CORE_LOCK(_sc)		mtx_lock(&(_sc)->core_mtx)
#define	HIF_TX_LOCK(_sc)			mtx_lock(&(_sc)->tx_mtx)
#define	HIF_TX_TRYLOCK(_sc)			mtx_trylock(&(_sc)->tx_mtx)
#define	HIF_RX_LOCK(_sc)			mtx_lock(&(_sc)->rx_mtx)
#define	HIF_CORE_UNLOCK(_sc)		mtx_unlock(&(_sc)->core_mtx)
#define	HIF_TX_UNLOCK(_sc)		mtx_unlock(&(_sc)->tx_mtx)
#define	HIF_RX_UNLOCK(_sc)		mtx_unlock(&(_sc)->rx_mtx)
#define	HIF_CORE_LOCK_ASSERT(_sc)	mtx_assert(&(_sc)->core_mtx, MA_OWNED)
#define	HIF_TX_LOCK_ASSERT(_sc)		mtx_assert(&(_sc)->tx_mtx, MA_OWNED)

#define HIF_TX_SLEEP(_sc)		mtx_sleep((_sc)->tx_cond, &(_sc)->tx_mtx, 0, NULL, 0)
#define HIF_RX_SLEEP(_sc)		mtx_sleep((_sc)->rx_cond, &(_sc)->rx_mtx, 0, NULL, 0)
#define HIF_TX_WAKEUP(_sc)		wakeup((_sc)->tx_cond)
#define HIF_RX_WAKEUP(_sc)		wakeup((_sc)->rx_cond)

#endif /* _IF_HOST_H_ */

