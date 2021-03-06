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

#include "CSQLiteDataProvider.h"


CSQLiteDataProvider::CSQLiteDataProvider(ADataConnectionProvider *provider)
{
	this->className = string(__func__);
	this->provider = provider;
}

CSQLiteDataProvider::~CSQLiteDataProvider()
{
}

bool CSQLiteDataProvider::initStatements()
{
	int s1, s2;

	s1 = sqlite3_prepare_v2(getProvider()->getConnection()->handle,
					"INSERT INTO totals VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
					-1, &totals_insert_stmt, 0);

	s2 = sqlite3_prepare_v2(provider->getConnection()->handle,
			"INSERT INTO protos VALUES(?, ?, ?, ?, ?, ?, ?, ?)", -1,
			&ports_insert_stmt, 0);

	if (s1 != SQLITE_OK || s2 != SQLITE_OK)
	{
		Logger::error(className,
				"Cannot initialize SQLite prepared statements");
		return false;
	}

	return true;
}

void CSQLiteDataProvider::destroyStatements()
{
	sqlite3_finalize(totals_insert_stmt);
	sqlite3_finalize(ports_insert_stmt);

	totals_insert_stmt = NULL;
	ports_insert_stmt = NULL;
}

void CSQLiteDataProvider::insertTotals(uint32_t ip, time_t ts,
		struct host_stats* stats)
{
	sqlite3_stmt *stmt = totals_insert_stmt;
	int f = 1, c;

	sqlite3_bind_int64(stmt, f++, ts);
	sqlite3_bind_int64(stmt, f++, ip);

	sqlite3_bind_int64(stmt, f++, stats->tcp->in_bytes);
	sqlite3_bind_int64(stmt, f++, stats->tcp->out_bytes);
	sqlite3_bind_int64(stmt, f++, stats->tcp->in_pkts);
	sqlite3_bind_int64(stmt, f++, stats->tcp->out_pkts);

	sqlite3_bind_int64(stmt, f++, stats->udp->in_bytes);
	sqlite3_bind_int64(stmt, f++, stats->udp->out_bytes);
	sqlite3_bind_int64(stmt, f++, stats->udp->in_pkts);
	sqlite3_bind_int64(stmt, f++, stats->udp->out_pkts);

	sqlite3_bind_int64(stmt, f++, stats->icmp->in_bytes);
	sqlite3_bind_int64(stmt, f++, stats->icmp->out_bytes);
	sqlite3_bind_int64(stmt, f++, stats->icmp->in_pkts);
	sqlite3_bind_int64(stmt, f++, stats->icmp->out_pkts);

	sqlite3_bind_int64(stmt, f++, stats->other->in_bytes);
	sqlite3_bind_int64(stmt, f++, stats->other->out_bytes);
	sqlite3_bind_int64(stmt, f++, stats->other->in_pkts);
	sqlite3_bind_int64(stmt, f++, stats->other->out_pkts);

	if ((c = sqlite3_step(stmt)) != SQLITE_DONE)
		Logger::error(className,
				string(
						"Couldn't insert into 'totals' table. SQLite engine returned ")
						+ boost::lexical_cast<std::string>(c));

	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
}

void CSQLiteDataProvider::insertPorts(uint32_t ip, time_t ts, int ptype,
		u_int16_t port, struct proto_stats *ps)
{
	sqlite3_stmt *stmt = ports_insert_stmt;
	int f = 1, c;

	sqlite3_bind_int64(stmt, f++, ts);
	sqlite3_bind_int64(stmt, f++, ip);
	sqlite3_bind_int(stmt, f++, port);
	sqlite3_bind_int(stmt, f++, ptype);

	sqlite3_bind_int64(stmt, f++, ps->in_bytes);
	sqlite3_bind_int64(stmt, f++, ps->out_bytes);
	sqlite3_bind_int(stmt, f++, ps->in_pkts);
	sqlite3_bind_int(stmt, f++, ps->out_pkts);

	if ((c = sqlite3_step(stmt)) != SQLITE_DONE)
		Logger::error(className,
				string(
						"Couldn't insert into 'protos' table. SQLite engine returned ")
						+ boost::lexical_cast<std::string>(c));

	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
}

void CSQLiteDataProvider::insertRecord(hstat_t *packet)
{
	time_t ts = time(NULL);

	provider->lock();
	initStatements();

	sqlite3_exec(getProvider()->getConnection()->handle, "BEGIN TRANSACTION;",
			NULL, NULL, NULL);
	for (hstat_t::iterator i = packet->begin(); i != packet->end(); ++i)
		insertRecord(i->first, ts, i->second);
	sqlite3_exec(getProvider()->getConnection()->handle, "END TRANSACTION;",
			NULL, NULL, NULL);

	destroyStatements();
	provider->unlock();
}

void CSQLiteDataProvider::insertRecord(uint32_t ip, time_t ts,
		struct host_stats* stats)
{
	insertTotals(ip, ts, stats);

	if (stats->tcp_ports != NULL)
	{
		for (pstat_t::iterator i = stats->tcp_ports->begin();
				i != stats->tcp_ports->end(); ++i)
		{
			insertPorts(ip, ts, PS_TCP, i->first, i->second);
		}
	}

	if (stats->udp_ports != NULL)
	{
		for (pstat_t::iterator i = stats->udp_ports->begin();
				i != stats->udp_ports->end(); ++i)
		{
			insertPorts(ip, ts, PS_UDP, i->first, i->second);
		}
	}
}



