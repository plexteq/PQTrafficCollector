#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <pthread.h>
#include <apr-1/apr_pools.h>

#define __FAVOR_BSD 

using namespace std;

#define NSTATS 1
#define WATCHER_THREAD_SLEEP 60
#define AGGREGATION_DELAY 65

#define HPTC_FAILURE 0
#define HPTC_SUCCESS 1

#define CONDVAR_UNLOCKED 0
#define CONDVAR_LOCKED 1

#define DISK_FREE_PRCNT_THRESHOLD 20
#define DBFILE_SIZE_PRCNT_THRESHOLD 1

/*
 * Stores program settings
 */
struct configuration
{
	/*
	 * Network interface to listen
	 */
	char interface[8];

	/*
	 * Number of blocking queues to balance load coming to worker threads
	 */
	ushort queues;

	/*
	 * Number of worker threads used to precess incoming packets
	 */
	ushort threads;

	/*
	 * Linktype of the current network interface
	 * DLT_NULL/DLT_EN10MB/DLT_SLIP/DLT_PPP
	 */
	int linkType;

	/*
	 * Datalink header size
	 */
	int headerOffset;

	/*
	 * Path to read/write database files
	 */
	char databasePath[512];
};

/*
 * Constants used to determine packet direction
 * 		PACKET_NV - packet with src/dst addresses that don't match local interfaces
 * 		PACKET_IN - packet destined for the host
 * 		PACKET_OUT - packet originating from the host
 */
#define PACKET_NV -1
#define PACKET_IN 0
#define PACKET_OUT 1

/*
 * Single packet representation grabbed by a sniffer
 */
struct packet
{
	uint32_t host;
	uint32_t ip_src;
	uint32_t ip_dst;
	u_int8_t proto;
	u_int16_t port;
	uint16_t payload;
	u_char direction; //0 - in, 1 - out
};

/*
 * Traffic usage stats on particular protocol
 */
#define PS_TCP 0
#define PS_UDP 1
#define PS_ICMP 2
#define PS_OTHER 3
struct proto_stats
{
	ulong in_bytes;
	ulong out_bytes;
	uint32_t in_pkts;
	uint32_t out_pkts;
};

/*
 * Represents stats collected by worker thread during a time interval
 */
typedef map<u_int16_t, struct proto_stats*> pstat_t;

struct host_stats
{
	private:
		apr_pool_t *pool;

	public:

		struct proto_stats *icmp;
		struct proto_stats *udp;
		struct proto_stats *tcp;
		struct proto_stats *other;

		pstat_t *udp_ports;
		pstat_t *tcp_ports;

		struct proto_stats* allocate()
		{
			struct proto_stats *p = (struct proto_stats *) apr_palloc(pool, sizeof(struct proto_stats));
			memset(p, 0, sizeof(struct proto_stats));
			return p;
		}

		struct proto_stats* allocate(struct proto_stats * s)
		{
			struct proto_stats *p = (struct proto_stats *) apr_palloc(pool, sizeof(struct proto_stats));
			memcpy(p, s, sizeof(struct proto_stats));
			return p;
		}

		void mergeFrom(host_stats* from)
		{
			mergeFrom(from->icmp, icmp);
			mergeFrom(from->udp, udp);
			mergeFrom(from->tcp, tcp);
			mergeFrom(from->other, other);

			mergeFrom(from->udp_ports, udp_ports);
			mergeFrom(from->tcp_ports, tcp_ports);
		}

		void mergeFrom(pstat_t *from, pstat_t *to)
		{
			for (pstat_t::iterator i = from->begin() ; i != from->end() ; i++)
			{
				pstat_t::iterator j = to->find(i->first);

				if (j == to->end())
					to->insert(make_pair(i->first, allocate(i->second)));
				else
					mergeFrom(i->second, j->second);
			}
		}

		void mergeFrom(struct proto_stats *from, struct proto_stats *to)
		{
			to->in_bytes += from->in_bytes;
			to->in_pkts += from->in_pkts;
			to->out_bytes += from->out_bytes;
			to->out_pkts += from->out_pkts;
		}

		void copyFrom(host_stats* from)
		{
			memcpy(icmp, from->icmp, sizeof(struct proto_stats));
			memcpy(udp, from->udp, sizeof(struct proto_stats));
			memcpy(tcp, from->tcp, sizeof(struct proto_stats));
			memcpy(other, from->other, sizeof(struct proto_stats));

			copyFrom(from->udp_ports, udp_ports);
			copyFrom(from->tcp_ports, tcp_ports);
		}

		void copyFrom(pstat_t* from, pstat_t* to)
		{
			for (pstat_t::iterator i = from->begin() ; i != from->end() ; i++)
				to->insert(make_pair(i->first, allocate(i->second)));
		}

		host_stats(struct host_stats* from) {
			init(false);
			copyFrom(from);
		}

		host_stats() {
			init(true);
		}

		void init(bool clean)
		{
			apr_pool_create(&pool, NULL);

			udp_ports = new pstat_t();
			tcp_ports = new pstat_t();

			icmp = (struct proto_stats *) apr_palloc(pool, sizeof(struct proto_stats));
			udp = (struct proto_stats *) apr_palloc(pool, sizeof(struct proto_stats));
			tcp = (struct proto_stats *) apr_palloc(pool, sizeof(struct proto_stats));
			other = (struct proto_stats *) apr_palloc(pool, sizeof(struct proto_stats));

			if (!clean) return;

			memset(icmp, 0, sizeof(struct proto_stats));
			memset(udp, 0, sizeof(struct proto_stats));
			memset(tcp, 0, sizeof(struct proto_stats));
			memset(other, 0, sizeof(struct proto_stats));

		}

		~host_stats()
		{
			apr_pool_destroy(pool);

			delete udp_ports;
			delete tcp_ports;
		}
};

typedef map<uint32_t, struct host_stats*> hstat_t;


/*
 * Macro to calculate IP header length
 */
#define IP_HL(ip) (ip)->ip_hl*4

/*
 * Represents data connection handle
 */
struct dbh_t
{
	/*
	 * SQLite handle
	 */
	sqlite3 *handle;

	/*
	 * Mutex to lock this handle on concurrent operations
	 */
	pthread_mutex_t mutex;

	/*
	 * Date/time when program has started
	 * Needed to make DB file rolling once month changes
	 */
	time_t ts;
};

#define WORK_UNIT_SZ 256
struct work_unit_t
{
	public:
		apr_pool_t *pool;
		struct packet **packets;
		int count;

		struct packet* allocate() {
			struct packet *p = (struct packet *) apr_palloc(pool, sizeof(struct packet));
			memset(p, 0, sizeof(struct packet));
			return p;
		}

		bool canPut() {
			return (count < WORK_UNIT_SZ-1);
		}

		void put(struct packet *p) {
			if (canPut()) packets[count++] = p;
		}

		work_unit_t() {
			apr_pool_create(&pool, NULL);
			packets = (struct packet **) apr_palloc(pool, sizeof(struct packet*) * WORK_UNIT_SZ);
			count = 0;
		}

		~work_unit_t() {
			apr_pool_destroy(pool);
		}
};

int getEndian()
{
	int num = 1;
	if(*(char *)&num == 1)
		return LITTLE_ENDIAN;

	return BIG_ENDIAN;
}

uint32_t _get_long_ip(uint32_t ip)
{
	if (getEndian() == LITTLE_ENDIAN)
		ip = htonl(ip);

	return ip;
}

uint32_t _get_long_ip(struct in_addr *inaddr)
{
	uint32_t ip = inaddr->s_addr;

	if (getEndian() == LITTLE_ENDIAN)
		ip = htonl(ip);

	return ip;
}

#endif /* COMMON_H_ */
