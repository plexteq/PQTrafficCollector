/**
 * Copyright (c) 2017, Plexteq
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

#include "CPCAPPacketSniffer.h"
#include "../CConfiguration.h"

template class ABlockingQueue<work_unit_t>;
template class CPCAPPacketSniffer<work_unit_t>;

template<typename T>
void CPCAPPacketSniffer<T>::pcapCallback(u_char *arg,
		const struct pcap_pkthdr* pkthdr, const u_char* packet)
{
	CPCAPPacketSniffer<T> *instance = (CPCAPPacketSniffer<T>*) arg;

	struct ip* _iphdr = (struct ip*) (packet + instance->ipHeaderSize);
	struct tcphdr* _tcphdr = (struct tcphdr*) (packet + instance->ipHeaderSize
			+ IP_HL(_iphdr));
	struct udphdr* _udphdr = (struct udphdr*) (packet + instance->ipHeaderSize
			+ IP_HL(_iphdr));

	u_int16_t src_port = 0, dst_port = 0;
	uint32_t ip_src = (_iphdr->ip_src.s_addr), ip_dst = (_iphdr->ip_dst.s_addr);

	switch (_iphdr->ip_p)
	{
	case IPPROTO_TCP:
		src_port = ntohs(_tcphdr->th_sport);
		dst_port = ntohs(_tcphdr->th_dport);
		break;
	case IPPROTO_UDP:
		src_port = ntohs(_udphdr->uh_sport);
		dst_port = ntohs(_udphdr->uh_dport);
		break;
	case IPPROTO_ICMP:
		break;
	default:
		src_port = dst_port = 0;
		//return;
		break;
	}

	// local traffic
	if (ip_src == ip_dst)
		return;

	instance->handle(_get_long_ip((unsigned int) ip_src),
			_get_long_ip((unsigned int) ip_dst), _iphdr->ip_p, src_port,
			dst_port, ntohs(_iphdr->ip_len));
}

template<typename T>
void CPCAPPacketSniffer<T>::handle(uint32_t ip_src, uint32_t ip_dst,
		u_int8_t proto, u_int16_t src_port, u_int16_t dst_port,
		uint16_t payload)
{
	int qid = rand() % CConfiguration::getInstance()->getQueues();

	bool isLocalDst = ipresolver->isLocalIP(ip_dst), isLocalSrc =
			ipresolver->isLocalIP(ip_src);

	if (!isLocalDst && !isLocalSrc)
		return;

	struct packet *p = unit->allocate();

	p->proto = proto;
	p->payload = payload;
	p->direction = PACKET_NV;

	if (isLocalDst)
	{
		p->port = dst_port;
		p->host = ip_dst;
		p->direction = PACKET_IN;
	}
	else if (isLocalSrc)
	{
		p->port = src_port;
		p->host = ip_src;
		p->direction = PACKET_OUT;
	}

	unit->put(p);

	if (!unit->canPut())
	{
		queues[qid]->push(unit);
		unit = new work_unit_t;
	}
}

template<typename T>
void CPCAPPacketSniffer<T>::run()
{
	pcap_handle = pcap_open_live(CConfiguration::getInstance()->getInterface(),
			1024, 1, 0, errbuf);

	// setting link type in order to determine proper ip packet header size
	CConfiguration::getInstance()->setLinkType(pcap_datalink(pcap_handle));
	this->ipHeaderSize = CConfiguration::getInstance()->getPacketHeaderOffset();

	pcap_loop(pcap_handle, -1, pcapCallback, (u_char*) this);
}

