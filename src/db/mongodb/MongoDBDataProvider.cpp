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

#include "MongoDBDataProvider.h"
#include "CExecutorThread.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/make_unique.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include "../../CConfiguration.h"
#include "../../utility/Logger.h"

using namespace bsoncxx::document;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

MongoDBDataProvider::MongoDBDataProvider(ADataConnectionProvider *provider)
{
	this->className = string(__func__);
	this->provider = provider;
	this->executorThread = new CExecutorThread(provider, &retryVector, 60);

	this->executorThread->start();
}

MongoDBDataProvider::~MongoDBDataProvider()
{

}

void MongoDBDataProvider::insertRecord(hstat_t *packet)
{
	time_t ts = time(NULL);

	provider->lock();

	for (hstat_t::iterator i = packet->begin(); i != packet->end(); ++i)
		insertRecord(i->first, ts, i->second);

	provider->unlock();
}

void MongoDBDataProvider::insertTotals(uint32_t ip, time_t ts,
		struct host_stats* stats)
{
	view_or_value doc_value = totalsToJSON(ip, ts, stats);

	std::vector<view_or_value> documents;
	documents.push_back(doc_value);

	retryVector.push_back(dbItem{documents, "totals"});
}

mongocxx::collection MongoDBDataProvider::getCollection(
		const char* collectionName)
{
	char* dbName = provider->getDBFileName();

	return (*((mongocxx::client*) provider->getConnection()->handle))[dbName][collectionName];
}

view_or_value MongoDBDataProvider::portsToJSON(uint32_t ip,
		time_t ts, int ptype, u_int16_t port, struct proto_stats *ps)
{
	bsoncxx::builder::stream::document builder
	{ };

	view_or_value doc_value = builder << "info"
			<< bsoncxx::builder::stream::open_document << "ts" << (int64_t) ts
			<< "ip" << (int64_t) ip << "port" << port << "ptype" << ptype
			<< "in_bytes" << (int64_t) ps->in_bytes << "out_bytes"
			<< (int64_t) ps->out_bytes << "in_pkts" << (int) ps->in_pkts
			<< "out_pkts" << (int) ps->out_pkts

			<< bsoncxx::builder::stream::close_document
			<< bsoncxx::builder::stream::finalize;

	return doc_value;
}

view_or_value MongoDBDataProvider::totalsToJSON(uint32_t ip,
		time_t ts, struct host_stats* stats)
{
	bsoncxx::builder::stream::document builder
	{ };

	view_or_value doc_value = builder << "info"
			<< bsoncxx::builder::stream::open_document << "tcp_in_bytes"
			<< (int64_t) stats->tcp->in_bytes << "tcp_out_bytes"
			<< (int64_t) stats->tcp->out_bytes << "tcp_in_pkts"
			<< (int64_t) stats->tcp->in_pkts << "tcp_out_pkts"
			<< (int64_t) stats->tcp->out_pkts

			<< "udp_in_bytes" << (int64_t) stats->udp->in_bytes
			<< "udp_out_bytes" << (int64_t) stats->udp->out_bytes
			<< "udp_in_pkts" << (int64_t) stats->udp->in_pkts << "udp_out_pkts"
			<< (int64_t) stats->udp->out_pkts

			<< "icmp_in_bytes" << (int64_t) stats->icmp->in_bytes
			<< "icmp_out_bytes" << (int64_t) stats->icmp->out_bytes
			<< "icmp_in_pkts" << (int64_t) stats->icmp->in_pkts
			<< "icmp_out_pkts" << (int64_t) stats->icmp->out_pkts

			<< "other_in_bytes" << (int64_t) stats->other->in_bytes
			<< "other_out_bytes" << (int64_t) stats->other->out_bytes
			<< "other_in_pkts" << (int64_t) stats->other->in_pkts
			<< "other_out_pkts" << (int64_t) stats->other->out_pkts

			<< bsoncxx::builder::stream::close_document
			<< bsoncxx::builder::stream::finalize;

	return doc_value;
}

void MongoDBDataProvider::insertRecord(uint32_t ip, time_t ts,
		struct host_stats* stats)
{
	insertTotals(ip, ts, stats);

	std::vector<view_or_value> documents;

	insertStats(ip, ts,stats->tcp_ports, PS_TCP, documents);
	insertStats(ip, ts,stats->udp_ports, PS_UDP, documents);

	retryVector.push_back(dbItem{documents, "protos"});
}

void MongoDBDataProvider::insertStats(uint32_t ip, time_t ts, pstat_t* stat,
		int type, std::vector<view_or_value> &documents)
{
	if (stat != NULL)
	{
		for (pstat_t::iterator i = stat->begin(); i != stat->end(); ++i)
		{
			documents.push_back(portsToJSON(ip, ts, type, i->first, i->second));
		}
	}
}

