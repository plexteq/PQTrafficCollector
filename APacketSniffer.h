#ifndef APACKETSNIFFER_H_
#define APACKETSNIFFER_H_

class APacketSniffer
{
	public:
		virtual ~APacketSniffer() {}
	protected:

		/*
		 * Handles packet
		 *
		 * ip_src - source IP address
		 * ip_dst - destination IP address
		 * proto - protocol (can be IPPROTO_TCP, IPPROTO_UDP, IPPROTO_ICMP)
		 * size - packet length (including header)
		 */
		virtual void handle(uint32_t ip_src, uint32_t ip_dst, u_int8_t proto, u_int16_t src_port, u_int16_t dst_port, uint16_t payload) = 0;

};

#endif /* APACKETSNIFFER_H_ */
