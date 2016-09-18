/*
 * CSQLiteDataProvider.h
 *
 *  Created on: May 14, 2014
 *      Author: root
 */

#ifndef CSQLITEDATAPROVIDER_H_
#define CSQLITEDATAPROVIDER_H_

#include "common.h"
#include "ADataProvider.h"

class CSQLiteDataProvider : public ADataProvider
{
	private:
		sqlite3_stmt *totals_insert_stmt;
		sqlite3_stmt *ports_insert_stmt;
		ADataConnectionProvider* provider;

		void insertTotals(uint32_t ip, time_t ts, struct host_stats* stats);
		void insertPorts(uint32_t ip, time_t ts, int ptype, u_int16_t port, struct proto_stats *ps);

		bool initStatements();
		void destroyStatements();
		void insertRecord(uint32_t ip, time_t ts, struct host_stats* stats);
	public:
		CSQLiteDataProvider(ADataConnectionProvider *provider);
		virtual ~CSQLiteDataProvider();
		virtual ADataConnectionProvider* getProvider() {return provider;};
		void insertRecord(hstat_t *packet);
};

CSQLiteDataProvider::CSQLiteDataProvider(ADataConnectionProvider *provider) {
	this->provider = provider;
}

CSQLiteDataProvider::~CSQLiteDataProvider()
{
}

bool CSQLiteDataProvider::initStatements()
{
	int s1, s2;

	s1 = sqlite3_prepare_v2(getProvider()->getConnection()->handle,
		"INSERT INTO totals VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &totals_insert_stmt, 0);

	s2 = sqlite3_prepare_v2(provider->getConnection()->handle,
		"INSERT INTO protos VALUES(?, ?, ?, ?, ?, ?, ?, ?)", -1, &ports_insert_stmt, 0);

	if (s1 != SQLITE_OK || s2 != SQLITE_OK) {
		cout << "Cannot initialize SQLite prepared statements" << endl;
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

void CSQLiteDataProvider::insertTotals(uint32_t ip, time_t ts, struct host_stats* stats)
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
		cout << "Couldn't insert into 'totals' table. SQLite engine returned " << c << endl;

	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
}

void CSQLiteDataProvider::insertPorts(uint32_t ip, time_t ts, int ptype, u_int16_t port, struct proto_stats *ps)
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
		cout << "Couldn't insert into 'protos' table. SQLite engine returned " << c << endl;

	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
}

void CSQLiteDataProvider::insertRecord(hstat_t *packet)
{
	time_t ts = time(NULL);

	provider->lock();
	initStatements();

	sqlite3_exec(getProvider()->getConnection()->handle, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	for (hstat_t::iterator i = packet->begin() ; i != packet->end() ; ++i)
		insertRecord(i->first, ts, i->second);
	sqlite3_exec(getProvider()->getConnection()->handle, "END TRANSACTION;", NULL, NULL, NULL);

	destroyStatements();
	provider->unlock();
}

void CSQLiteDataProvider::insertRecord(uint32_t ip, time_t ts, struct host_stats* stats)
{
	insertTotals(ip, ts, stats);

	if (stats->tcp_ports != NULL) {
		for (pstat_t::iterator i = stats->tcp_ports->begin() ; i != stats->tcp_ports->end() ; ++i) {
			insertPorts(ip, ts, PS_TCP, i->first, i->second);
		}
	}

	if (stats->udp_ports != NULL) {
		for (pstat_t::iterator i = stats->udp_ports->begin() ; i != stats->udp_ports->end() ; ++i) {
			insertPorts(ip, ts, PS_UDP, i->first, i->second);
		}
	}
}


#endif /* CSQLITEDATAPROVIDER_H_ */
