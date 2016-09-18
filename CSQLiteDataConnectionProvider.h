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

#ifndef CSQLITEDATACONNECTIONPROVIDER_H_
#define CSQLITEDATACONNECTIONPROVIDER_H_
#include "common.h"
#include "ADataConnectionProvider.h"
#include "CBaseThread.h"
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Singleton database connection manager class
 */
class CSQLiteDataConnectionProvider : public CBaseThread, public ADataConnectionProvider
{
	private:
		/*
		 * Full path to actively opened database file
		 */
		char *databaseFile;

		/*
		 * Single class instance
		 */
		static CSQLiteDataConnectionProvider* instance;

		/*
		 * Private constructor (singleton)
		 */
		CSQLiteDataConnectionProvider() : CBaseThread(), ADataConnectionProvider() {
			databaseFile = NULL;
			openConnection();
			start();
		};

		int getFreeDiskPercentage();
		int getDBSizePercentage();

	protected:
		virtual void openConnection();
		virtual void closeConnection();
		virtual void reopenConnection(bool remove);

		virtual bool isDatabaseStrucureExists();
		virtual void createDatabaseStructure();

	public:
		/*
		 * Because of singleton we don't implement following stuff
		 */
		CSQLiteDataConnectionProvider(CSQLiteDataConnectionProvider&);
		void operator = (CSQLiteDataConnectionProvider const&);

		static CSQLiteDataConnectionProvider* getInstance();
		virtual ~CSQLiteDataConnectionProvider() {
			lock();
			closeConnection();
			delete databaseFile;
			unlock();
		};

		/*
		 * Dumps configs and related stuff
		 */
		virtual void dump();

		/*
		 * Periodically checks date and re-creates (rolls)
		 * database file on monthly basis
		 */
		virtual void run();
};

void CSQLiteDataConnectionProvider::dump()
{
	cout << "Dumping DB layer configuration:" << endl;
	cout << "database file: " << databaseFile << endl;
}

void CSQLiteDataConnectionProvider::openConnection()
{
	cout << "Opening new database connection" << endl;

	// getting current time to form proper database file name
	time_t *cts = &(getConnection()->ts);
	*cts = time(NULL);

	struct tm datetime;
	memcpy(&datetime, localtime(cts), sizeof(struct tm));

	// forming database file name
	char fileName[256];
	strftime(fileName, sizeof(fileName), "ts-%m-%Y.sl3", &datetime);
	databaseFile = new char[strlen(CConfiguration::getInstance()->getDatabasePath()) + strlen(fileName) + 1];

	strcpy(databaseFile, CConfiguration::getInstance()->getDatabasePath());
	strcat(databaseFile, fileName);

	cout << "Database file: " << databaseFile << endl;

	// making connection
	int errorCode;
	if ((errorCode = sqlite3_open_v2(databaseFile, &(getConnection()->handle),
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))) {
		cerr << "Cannot create/open database file: " << databaseFile << endl;
		exit(1);
	}

	// creating database structure if doesn't exist (e.g. new empty database)
	if (!isDatabaseStrucureExists())
		createDatabaseStructure();
}

void CSQLiteDataConnectionProvider::closeConnection()
{
	cout << "Closing database connection" << endl;

	if (getConnection()->handle != NULL)
		sqlite3_close(getConnection()->handle);

	if (databaseFile != NULL)
		delete databaseFile;
}

CSQLiteDataConnectionProvider* CSQLiteDataConnectionProvider::getInstance()
{
	if (instance == NULL) {
		instance = new CSQLiteDataConnectionProvider();
	}
	return instance;
}

bool CSQLiteDataConnectionProvider::isDatabaseStrucureExists()
{
	sqlite3_stmt *table_check_stmt;
	const char *tail;
	int table_count = 0;

	sqlite3_prepare_v2(getConnection()->handle, "SELECT count(*) FROM sqlite_master WHERE type='table'", 1024, &table_check_stmt, &tail);
	sqlite3_step(table_check_stmt);
	table_count = sqlite3_column_int(table_check_stmt, 0);
	sqlite3_finalize(table_check_stmt);

	return table_count;
}

void CSQLiteDataConnectionProvider::createDatabaseStructure()
{
	cout << "Opened database is empty, creating tables" << endl;

	// creating tables
	sqlite3_exec(getConnection()->handle,
			"CREATE TABLE protos (ts INTEGER, ip INTEGER, port INTEGER, ptype INTEGER, "
			"in_bytes INTEGER, out_bytes INTEGER, in_pkts INTEGER, out_pkts INTEGER)", 0, 0, 0);

	sqlite3_exec(getConnection()->handle,
			"CREATE TABLE totals (ts INTEGER, ip INTEGER, "
			"tcp_in_bytes INTEGER, tcp_out_bytes INTEGER, tcp_in_pkts INTEGER, tcp_out_pkts INTEGER, "
			"udp_in_bytes INTEGER, udp_out_bytes INTEGER, udp_in_pkts INTEGER, udp_out_pkts INTEGER, "
			"icmp_in_bytes INTEGER, icmp_out_bytes INTEGER, icmp_in_pkts INTEGER, icmp_out_pkts INTEGER, "
			"other_in_bytes INTEGER, other_out_bytes INTEGER, other_in_pkts INTEGER, other_out_pkts INTEGER)", 0, 0, 0);

	// creating indices
	sqlite3_exec(getConnection()->handle, "CREATE INDEX protos_idx_ip_ts_ptype ON protos (ip, ts, ptype)", 0, 0, 0);
	sqlite3_exec(getConnection()->handle, "CREATE INDEX totals_idx_ts_ip ON totals (ts, ip)", 0, 0, 0);
}

void CSQLiteDataConnectionProvider::run()
{
	while (1)
	{
		time_t currentTs = time(NULL),
			   connectionTs = getConnection()->ts;

		struct tm currentDt, connectionDt;
		memcpy(&currentDt, localtime(&currentTs), sizeof(struct tm));
		memcpy(&connectionDt, localtime(&connectionTs), sizeof(struct tm));

		if (currentDt.tm_mon != connectionDt.tm_mon)
		{
			cout << "Month has changed " << currentDt.tm_mon << " -> " << connectionDt.tm_mon << endl;
			cout << "Going to recreate stats database" << endl;

			reopenConnection(true);
		}

		cout << getFreeDiskPercentage() << ", " << getDBSizePercentage() << endl;

		if (getFreeDiskPercentage() < DISK_FREE_PRCNT_THRESHOLD && 
			getDBSizePercentage() > DBFILE_SIZE_PRCNT_THRESHOLD)
		{
			cout << "Less than " << DISK_FREE_PRCNT_THRESHOLD << "% of disk space left on " << CConfiguration::getInstance()->getDatabasePath() << endl;
			cout << "And " << databaseFile << " occupies more than " << DBFILE_SIZE_PRCNT_THRESHOLD << "% of " << CConfiguration::getInstance()->getDatabasePath() << " total size" << endl;
			cout << "Going to re-create DB" << endl;

			reopenConnection(true);
		}

		sleep(5 * 60);
	}
}

int CSQLiteDataConnectionProvider::getFreeDiskPercentage()
{
	struct statvfs s;
	statvfs(CConfiguration::getInstance()->getDatabasePath(), &s);
	
	long bfree = s.f_bsize * s.f_bfree,
		btotal = s.f_bsize * s.f_blocks;

	return (bfree * 100) / btotal;
}

int CSQLiteDataConnectionProvider::getDBSizePercentage()
{
	struct stat s;
	struct statvfs vs;

	stat(databaseFile, &s);
	long dbsize = s.st_size;

	statvfs(CConfiguration::getInstance()->getDatabasePath(), &vs);
	long btotal = vs.f_bsize * vs.f_blocks;

	return (dbsize * 100) / btotal;
}

void CSQLiteDataConnectionProvider::reopenConnection(bool remove)
{
	char* _db = strdup(databaseFile);

	lock();
	closeConnection();

	if (remove) unlink(_db);

	openConnection();
	unlock();

	free(_db);
}

CSQLiteDataConnectionProvider* CSQLiteDataConnectionProvider::instance = NULL;

#endif /* CSQLITEDATACONNECTIONPROVIDER_H_ */
