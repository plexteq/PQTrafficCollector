/**
 * Copyright (c) 2014, Plexteq                                   
 * All rights reserved.
 *  
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 * may be used to endorse or promote products derived from this software without 
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CPCAPPACKETSNIFFER_H_
#define CPCAPPACKETSNIFFER_H_

#include "APacketSniffer.h"
#include "../ABlockingQueue.h"
#include "CBaseIPResolver.h"
#include "../CBaseThread.h"

#include <pcap/pcap.h>
#ifdef _MSC_VER
#include "..\win32\win_headers.h"
struct ip
{
#if BYTE_ORDER == LITTLE_ENDIAN 
    u_char	ip_hl : 4,		/* header length */
        ip_v : 4;			/* version */
#endif
#if BYTE_ORDER == BIG_ENDIAN 
    u_char	ip_v : 4,			/* version */
        ip_hl : 4;		/* header length */
#endif
    u_char	ip_tos;			/* type of service */
    short	ip_len;			/* total length */
    u_short	ip_id;			/* identification */
    short	ip_off;			/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
    u_char	ip_ttl;			/* time to live */
    u_char	ip_p;			/* protocol */
    u_short	ip_sum;			/* checksum */
    struct	in_addr ip_src, ip_dst;	/* source and dest address */
};

struct tcphdr
{
    u_short	th_sport;		/* source port */
    u_short	th_dport;		/* destination port */
    u_long th_seq;			/* sequence number */
    u_long th_ack;			/* acknowledgement number */
#if BYTE_ORDER == LITTLE_ENDIAN 
    u_char	th_x2 : 4,		/* (unused) */
        th_off : 4;		/* data offset */
#endif
#if BYTE_ORDER == BIG_ENDIAN 
    u_char	th_off : 4,		/* data offset */
        th_x2 : 4;		/* (unused) */
#endif
    u_char	th_flags;
#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
    u_short	th_win;			/* window */
    u_short	th_sum;			/* checksum */
    u_short	th_urp;			/* urgent pointer */
};
struct udphdr
{
    u_int16_t uh_sport;           /* source port */
    u_int16_t uh_dport;           /* destination port */
    u_int16_t uh_ulen;            /* udp length */
    u_int16_t uh_sum;             /* udp checksum */
};
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#endif
template <typename T>
class CPCAPPacketSniffer : public APacketSniffer, public CBaseThread
{
	private:
		char errbuf[256];
		pcap_t *pcap_handle;
		ABlockingQueue<T> **queues;
		CBaseIPResolver *ipresolver;
		static void pcapCallback(u_char *arg,const struct pcap_pkthdr* pkthdr,const u_char*  packet);
		int ipHeaderSize;
		work_unit_t *unit;
	protected:
		virtual void handle(uint32_t ip_src, uint32_t ip_dst, u_int8_t proto, u_int16_t src_port, u_int16_t dst_port, uint16_t payload);
	public:
		CPCAPPacketSniffer(ABlockingQueue<T> **queues, CBaseIPResolver *ipresolver);
		virtual ~CPCAPPacketSniffer() {};
		virtual void run();
};

#endif /* CPCAPPACKETSNIFFER_H_ */
