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

#include "CWorkerThread.h"
#include <string>//for memcpy()

template class CWorkerThread<work_unit_t>;

template<typename T>
void WatchThread<T>::run()
{
	Logger::info(className,
			"Starting WatchThread ("
					+ boost::lexical_cast<std::string>(this->sleepTime) + ")");

	CWorkerThread<T> *t = static_cast<CWorkerThread<T>*>(this->workerThread);
	ABlockingQueue<hstat_t> *q = t->reportQueue;

	while (1)
	{
#ifdef _MSC_VER
		Sleep(this->sleepTime * 1000);
#else
		sleep(this->sleepTime);
#endif
		hstat_t *vp = t->swap();
		if (vp == NULL || vp->size() == 0)
			continue;
		Logger::info(className,
				"WatchThread is about to send a job for aggregator of "
						+ boost::lexical_cast<std::string>(vp->size())
						+ " size");
		q->push(vp);
	}
}

template<typename K>
void CWorkerThread<K>::handlePacket(work_unit_t *unit)
{
	__sync_synchronize();

	pthread_mutex_lock(mutex);
	/* while (mutex == CONDVAR_LOCKED)
	 {
	 Sleep(10);
	 }*/
	//if (mutex == CONDVAR_LOCKED) return;
	__sync_synchronize();

	for (int i = 0; i < unit->count; i++)
	{
		struct packet *packet = unit->packets[i];

		hstat_t::iterator j = stats->find(packet->host);
		struct host_stats* entry;

		if (j == stats->end())
		{
			entry = merge(packet, NULL);
			stats->insert(make_pair(packet->host, entry));
		}
		else
		{
			entry = j->second;
			merge(packet, entry);
		}
	}
	pthread_mutex_unlock(mutex);
}

template<typename K> 
host_stats_struct* CWorkerThread<K>::merge(struct packet *packet,
                                           host_stats_struct* existing)
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
		map<u_int16_t, struct proto_stats*>::iterator i = ports->find(
				packet->port);

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

		if (i != ports->end())
		{
			i->second->in_bytes += s->in_bytes;
			i->second->in_pkts += s->in_pkts;
			i->second->out_bytes += s->out_bytes;
			i->second->out_pkts += s->out_pkts;
		}
		else
		{
			ports->insert(make_pair(packet->port, s));
		}
	}

	return sp;
}

template<typename K>
CWorkerThread<K>::CWorkerThread(ABlockingQueue<K> *workQueue,
		ABlockingQueue<hstat_t> *reportQueue) :
		CBaseWorkerThread<K>(workQueue)
{
	this->reportQueue = reportQueue;

#ifdef _MSC_VER
	this->mutex = NULL;
	mutex = (pthread_mutex_t*)CreateMutex(
			NULL,              // default security attributes
			FALSE,// initially not owned
			NULL);// unnamed mutex
	//mutex = 0;
#else
	pthread_mutex_init(mutex, NULL);
#endif
	stats = new hstat_t();
}

template<typename K>
void CWorkerThread<K>::run()
{
	((CBaseThread*) (new WatchThread<K>(this, WATCHER_THREAD_SLEEP)))->start();

	K *unit = NULL;

	while (1)
	{
		this->getQueue()->pop(&unit);

		if (unit == NULL)
			continue;

		handlePacket(unit);

		delete unit;
	}
}

template<typename K>
void CWorkerThread<K>::dump()
{
	Logger::info(className,
			"\n----------------------Dumping stats--------------------------\n");

	for (hstat_t::iterator j = stats->begin(); j != stats->end(); ++j)
	{
		struct host_stats* s = j->second;
        
        struct in_addr addr;
        memcpy(&addr, &j->first, sizeof(struct in_addr));

        std::stringstream buf;
        buf << "\t IP: " << j->first << ", " << inet_ntoa(addr);

		Logger::info(className, buf.str());

		Logger::info(className,
				"\t icmp_in_bytes = "
						+ boost::lexical_cast<std::string>(s->icmp->in_bytes));
		Logger::info(className,
				"\t icmp_in_pkts = "
						+ boost::lexical_cast<std::string>(s->icmp->in_pkts));
		Logger::info(className,
				"\t icmp_out_bytes = "
						+ boost::lexical_cast<std::string>(s->icmp->out_bytes));
		Logger::info(className,
				"\t icmp_out_pkts = "
						+ boost::lexical_cast<std::string>(s->icmp->out_pkts));

		Logger::info(className,
				"\t udp_in_bytes = "
						+ boost::lexical_cast<std::string>(s->udp->in_bytes));
		Logger::info(className,
				"\t udp_in_pkts = "
						+ boost::lexical_cast<std::string>(s->udp->in_pkts));
		Logger::info(className,
				"\t udp_out_bytes = "
						+ boost::lexical_cast<std::string>(s->udp->out_bytes));
		Logger::info(className,
				"\t udp_out_pkts = "
						+ boost::lexical_cast<std::string>(s->udp->out_pkts));

		for (map<u_int16_t, proto_stats*>::iterator k = s->udp_ports->begin();
				k != s->udp_ports->end(); k++)
		{
			printf(
					"\t\t port %d: in_bytes=%lu, out_bytes=%lu, in_pkts=%u, out_pkts=%u\n",
					k->first, k->second->in_bytes, k->second->out_bytes,
					k->second->in_pkts, k->second->out_pkts);
		}

		Logger::info(className,
				"\t tcp_in_bytes = "
						+ boost::lexical_cast<std::string>(s->tcp->in_bytes));
		Logger::info(className,
				"\t tcp_in_pkts = "
						+ boost::lexical_cast<std::string>(s->tcp->in_bytes));
		Logger::info(className,
				"\t tcp_out_bytes = "
						+ boost::lexical_cast<std::string>(s->tcp->in_bytes));
		Logger::info(className,
				"\t tcp_out_pkts = "
						+ boost::lexical_cast<std::string>(s->tcp->in_bytes));

		for (map<u_int16_t, proto_stats*>::iterator k = s->tcp_ports->begin();
				k != s->tcp_ports->end(); k++)
		{
			printf(
					"\t\t port %d: in_bytes=%lu, out_bytes=%lu, in_pkts=%u, out_pkts=%u\n",
					k->first, k->second->in_bytes, k->second->out_bytes,
					k->second->in_pkts, k->second->out_pkts);
		}

		Logger::info(className,
				"\t other_in_bytes = "
						+ boost::lexical_cast<std::string>(s->other->in_bytes));
		Logger::info(className,
				"\t other_in_pkts = "
						+ boost::lexical_cast<std::string>(s->other->in_bytes));
		Logger::info(className,
				"\t other_out_bytes = "
						+ boost::lexical_cast<std::string>(s->other->in_bytes));
		Logger::info(className,
				"\t other_out_pkts = "
						+ boost::lexical_cast<std::string>(s->other->in_bytes));
	}
	Logger::info(className, " ----------------------- \n");
}

template<typename K>
hstat_t* CWorkerThread<K>::swap()
{
	hstat_t *s = stats;
	int count = s->size();

	//dump();

	if (count > 0)
	{
		// using CAS and memory barrier to prevent
		// old map being used during new map allocation
		// pthread mutexes intentionally not used from the performance reasons
		pthread_mutex_lock(mutex);
		//__sync_lock_test_and_set(&mutex, CONDVAR_LOCKED);
		__sync_synchronize();

		// attaching new map for gathering stats
		// old one is till used for sorting and stats aggregation
		stats = new hstat_t();

		//__sync_lock_test_and_set(&mutex, CONDVAR_UNLOCKED);
		pthread_mutex_unlock(mutex);
		__sync_synchronize();

		// returing pointer to the stats
		return s;
	}

	return NULL;
}
