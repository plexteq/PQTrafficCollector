#ifndef CPCAPPACKETSNIFFER_H_
#define CPCAPPACKETSNIFFER_H_

#include "APacketSniffer.h"

#include <pcap/pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

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

template <typename T>
void CPCAPPacketSniffer<T>::pcapCallback(u_char *arg, const struct pcap_pkthdr* pkthdr, const u_char*  packet)
{
	CPCAPPacketSniffer<T> *instance = (CPCAPPacketSniffer<T>*) arg;

	struct ip* _iphdr = (struct ip*) (packet + instance->ipHeaderSize);
	struct tcphdr* _tcphdr = (struct tcphdr*) (packet + instance->ipHeaderSize + IP_HL(_iphdr));
	struct udphdr* _udphdr = (struct udphdr*) (packet + instance->ipHeaderSize + IP_HL(_iphdr));

	u_int16_t src_port = 0, dst_port = 0;
	uint32_t ip_src = (_iphdr->ip_src.s_addr),
			 ip_dst = (_iphdr->ip_dst.s_addr);

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
	if (ip_src == ip_dst) return;

	instance->handle(
			_get_long_ip(ip_src),
			_get_long_ip(ip_dst),
			_iphdr->ip_p,
			src_port,
			dst_port,
			ntohs(_iphdr->ip_len));
}

template <typename T>
CPCAPPacketSniffer<T>::CPCAPPacketSniffer(ABlockingQueue<T> **queues, CBaseIPResolver *ipresolver) : CBaseThread()
{
	this->queues = queues;
	this->ipresolver = ipresolver;
	ipHeaderSize = 0;
	pcap_handle = NULL;
	unit = new work_unit_t;
}

template <typename T>
void CPCAPPacketSniffer<T>::handle(uint32_t ip_src, uint32_t ip_dst, u_int8_t proto, u_int16_t src_port, u_int16_t dst_port, uint16_t payload)
{
	int qid = rand() % CConfiguration::getInstance()->getQueues();

	bool isLocalDst = ipresolver->isLocalIP(ip_dst),
			isLocalSrc = ipresolver->isLocalIP(ip_src);

	if (!isLocalDst && !isLocalSrc)
		return;

	struct packet *p = unit->allocate();

	p->proto = proto;
	p->payload = payload;
	p->direction = PACKET_NV;

	if (isLocalDst) {
		p->port = dst_port;
		p->host = ip_dst;
		p->direction = PACKET_IN;
	} else if (isLocalSrc) {
		p->port = src_port;
		p->host = ip_src;
		p->direction = PACKET_OUT;
	}

	unit->put(p);

	if (!unit->canPut()) {
		queues[qid]->push(unit);
		unit = new work_unit_t;
	}
}

template <typename T>
void CPCAPPacketSniffer<T>::run()
{
	pcap_handle = pcap_open_live(CConfiguration::getInstance()->getInterface(), 1024, 1, 0, errbuf);

	// setting link type in order to determine proper ip packet header size
	CConfiguration::getInstance()->setLinkType(pcap_datalink(pcap_handle));
	this->ipHeaderSize = CConfiguration::getInstance()->getPacketHeaderOffset();

	pcap_loop(pcap_handle, -1, pcapCallback, (u_char*) this);
}

#endif /* CPCAPPACKETSNIFFER_H_ */
