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

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

#include <sqlite3.h>

#include <map>
#include <stdint.h>

#ifndef _MSC_VER
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#else
#include "win32\win_headers.h"

typedef void* pthread_mutex_t;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef void* pthread_t;
#define LITTLE_ENDIAN 0
#define BIG_ENDIAN 1
#endif

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
	char _interface[54];

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

	/*
	 * Port on which server will provide a status
	 */
	int serverPort;
#ifdef _MSC_VER
    char currentDirectory[512];
#endif
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
	u_int16_t payload;
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
	unsigned long in_bytes;
	unsigned long out_bytes;
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

typedef struct host_stats host_stats_struct;

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

int getEndian();

uint32_t _get_long_ip(uint32_t ip);

uint32_t _get_long_ip(struct in_addr *inaddr);

#endif /* COMMON_H_ */
