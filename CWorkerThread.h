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

#ifndef CWORKERTHREAD_H_
#define CWORKERTHREAD_H_

#include "CBaseWorkerThread.h"
#include "ABlockingQueue.h"
#include <iostream>
#include <algorithm>
#include <map>
#include <queue>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

using namespace std;

template <typename T>
class WatchThread : public CBaseThread
{
	private:
		CBaseThread *workerThread;
		int sleepTime;

	public:
		virtual ~WatchThread() {}
		WatchThread(CBaseThread *thread, int sleep) : CBaseThread()
		{
			this->workerThread = thread;
			this->sleepTime = sleep;
		}
		virtual void run();
};

template <typename T>
class CWorkerThread : public CBaseWorkerThread<T>
{
	// Allowing access for watcher thread to access private members
	// It's safe to do this
	friend class WatchThread<T>;

	private:
		// Map indexed by locally bound IP address in long representation
		hstat_t *stats;

		// Atomic char used to prevent concurrent access to stats[ip]
		volatile char mutex;

		// Queue where thread pushes gathered stats
		ABlockingQueue<hstat_t> *reportQueue;

		void handlePacket(work_unit_t *unit)
		{
			__sync_synchronize();
			if (mutex == CONDVAR_LOCKED) return;
			__sync_synchronize();

			for (int i = 0 ; i < unit->count ; i++)
			{
				struct packet *packet = unit->packets[i];

				hstat_t::iterator j = stats->find(packet->host);
				struct host_stats* entry;

				if (j == stats->end()) {
					entry = merge(packet, NULL);
					stats->insert(make_pair(packet->host, entry));
				} else {
					entry = j->second;
					merge(packet, entry);
				}
			}
		}

		struct host_stats* merge(struct packet *packet, struct host_stats* existing)
		{
			struct host_stats* sp = existing;

			if (sp == NULL)
				sp = new struct host_stats;

			struct proto_stats* stats;
			map<u_int16_t, struct proto_stats*> *ports = NULL;

			switch (packet->proto)
			{
				case IPPROTO_TCP:
					stats = sp->tcp;
					ports = sp->tcp_ports;
				break;
				case IPPROTO_UDP:
					stats = sp->udp;
					ports = sp->udp_ports;
				break;
				case IPPROTO_ICMP:
					stats = sp->icmp;
				break;
				default:
					stats = sp->other;
				break;
			}

			// summing total packet count and payload
			switch (packet->direction)
			{
				case PACKET_IN:
					stats->in_bytes += packet->payload;
					stats->in_pkts++;
				break;
				case PACKET_OUT:
					stats->out_bytes += packet->payload;
					stats->out_pkts++;
				break;
			}

			// adding stats by port
			if (ports != NULL && packet->port > 0)
			{
				map<u_int16_t, struct proto_stats*>::iterator i = ports->find(packet->port);

				struct proto_stats *s = sp->allocate();

				switch (packet->direction)
				{
					case PACKET_IN:
						s->in_pkts++;
						s->in_bytes += packet->payload;
					break;
					case PACKET_OUT:
						s->out_pkts++;
						s->out_bytes += packet->payload;
					break;
				}

				if (i != ports->end()) {
					i->second->in_bytes += s->in_bytes;
					i->second->in_pkts += s->in_pkts;
					i->second->out_bytes += s->out_bytes;
					i->second->out_pkts += s->out_pkts;
				} else {
					ports->insert(make_pair(packet->port, s));
				}
			}

			return sp;
		}

	public:
		virtual ~CWorkerThread(){}

		CWorkerThread(ABlockingQueue<T> *workQueue,
					  ABlockingQueue<hstat_t> *reportQueue) : CBaseWorkerThread<T>(workQueue)
		{
			this->reportQueue = reportQueue;

			mutex = 0;
			stats = new hstat_t();
		}

		virtual void run()
		{
			((CBaseThread*)(new WatchThread<T>(this, WATCHER_THREAD_SLEEP)))->start();

			T *unit = NULL;

			while (1)
			{
				this->getQueue()->pop(&unit);

				if (unit == NULL)
					continue;

				handlePacket(unit);

				delete unit;
			}
		}

		void dump()
		{
			cout << "\n----------------------Dumping stats--------------------------\n";
			for (hstat_t::iterator j = stats->begin() ; j != stats->end() ; ++j)
			{
				struct host_stats* s = j->second;
				cout << "\t IP: " << j->first << ", " << inet_ntoa((struct in_addr) {j->first}) << endl;

				cout << "\t icmp_in_bytes = " << s->icmp->in_bytes << endl;
				cout << "\t icmp_in_pkts = " << s->icmp->in_pkts << endl;
				cout << "\t icmp_out_bytes = " << s->icmp->out_bytes << endl;
				cout << "\t icmp_out_pkts = " << s->icmp->out_pkts << endl;

				cout << "\t udp_in_bytes = " << s->udp->in_bytes << endl;
				cout << "\t udp_in_pkts = " << s->udp->in_pkts << endl;
				cout << "\t udp_out_bytes = " << s->udp->out_bytes << endl;
				cout << "\t udp_out_pkts = " << s->udp->out_pkts << endl;

				for (map<u_int16_t, proto_stats*>::iterator k = s->udp_ports->begin() ; k != s->udp_ports->end() ; k++) {
					printf("\t\t port %d: in_bytes=%lu, out_bytes=%lu, in_pkts=%u, out_pkts=%u\n",
							k->first, k->second->in_bytes, k->second->out_bytes, k->second->in_pkts, k->second->out_pkts);
				}

				cout << "\t tcp_in_bytes = " << s->tcp->in_bytes << endl;
				cout << "\t tcp_in_pkts = " << s->tcp->in_pkts << endl;
				cout << "\t tcp_out_bytes = " << s->tcp->out_bytes << endl;
				cout << "\t tcp_out_pkts = " << s->tcp->out_pkts << endl;

				for (map<u_int16_t, proto_stats*>::iterator k = s->tcp_ports->begin() ; k != s->tcp_ports->end() ; k++) {
					printf("\t\t port %d: in_bytes=%lu, out_bytes=%lu, in_pkts=%u, out_pkts=%u\n",
							k->first, k->second->in_bytes, k->second->out_bytes, k->second->in_pkts, k->second->out_pkts);
				}

				cout << "\t other_in_bytes = " << s->other->in_bytes << endl;
				cout << "\t other_in_pkts = " << s->other->in_pkts << endl;
				cout << "\t other_out_bytes = " << s->other->out_bytes << endl;
				cout << "\t other_out_pkts = " << s->other->out_pkts << endl;

			}
			cout << " ----------------------- \n";
		}

		hstat_t* swap()
		{
			hstat_t *s = stats;
			int count = s->size();

			//dump();

			if (count > 0)
			{
				// using CAS and memory barrier to prevent
				// old map being used during new map allocation
				// pthread mutexes intentionally not used from the performance reasons
				__sync_lock_test_and_set(&mutex, CONDVAR_LOCKED);
				__sync_synchronize();

				// attaching new map for gathering stats
				// old one is till used for sorting and stats aggregation
				stats = new hstat_t();

				__sync_lock_test_and_set(&mutex, CONDVAR_UNLOCKED);
				__sync_synchronize();

				// returing pointer to the stats
				return s;
			}

			return NULL;
		}
};

template <typename T>
void WatchThread<T>::run()
{
	cout << "Starting WatchThread (" << this->sleepTime << ")" << endl;

	CWorkerThread<T> *t = static_cast<CWorkerThread<T>*>(this->workerThread);
	ABlockingQueue<hstat_t> *q = t->reportQueue;

	while (1) {
		sleep(this->sleepTime);

		hstat_t *vp = t->swap();
		if (vp == NULL || vp->size() == 0) continue;

		cout << "WatchThread is about to send a job for aggregator of " << vp->size() << " size" << endl;
		q->push(vp);
	}
}

#endif /* CWORKERTHREAD_H_ */
