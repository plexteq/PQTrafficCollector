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
#include "../CConfiguration.h"

#include "CSQLiteDataConnectionProvider.h"

void CSQLiteDataConnectionProvider::dump()
{
	Logger::info(className, "Dumping DB layer configuration:");
	Logger::info(className, string("database file: ") + databaseFile);
}

void CSQLiteDataConnectionProvider::openConnection()
{
	Logger::info(className, "Opening new database connection");

	// getting current time to form proper database file name
	time_t *cts = &(getConnection()->ts);
	*cts = time(NULL);

	struct tm datetime;
	memcpy(&datetime, localtime(cts), sizeof(struct tm));

	// forming database file name
	char fileName[256];
	strftime(fileName, sizeof(fileName), "ts-%m-%Y.sl3", &datetime);
	databaseFile = new char[strlen(
			CConfiguration::getInstance()->getDatabasePath()) + strlen(fileName)
			+ 1];

	strcpy(databaseFile, CConfiguration::getInstance()->getDatabasePath());
	strcat(databaseFile, fileName);

	Logger::info(className, string("database file: ") + databaseFile);

	// making connection
	int errorCode;
	if ((errorCode = sqlite3_open_v2(databaseFile, &(getConnection()->handle),
	SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL)))
	{
		Logger::error(className,
				string("Cannot create/open database file: ") + databaseFile);
		exit(1);
	}

	// creating database structure if doesn't exist (e.g. new empty database)
	if (!isDatabaseStrucureExists())
		createDatabaseStructure();
}

void CSQLiteDataConnectionProvider::closeConnection()
{
	Logger::info(className,"Closing database connection");

	if (getConnection()->handle != NULL)
		sqlite3_close(getConnection()->handle);

	if (databaseFile != NULL)
		delete databaseFile;
}

CSQLiteDataConnectionProvider* CSQLiteDataConnectionProvider::getInstance()
{
	if (instance == NULL)
	{
		instance = new CSQLiteDataConnectionProvider();
	}
	return instance;
}

bool CSQLiteDataConnectionProvider::isDatabaseStrucureExists()
{
	sqlite3_stmt *table_check_stmt;
	const char *tail;
	int table_count = 0;

	sqlite3_prepare_v2(getConnection()->handle,
			"SELECT count(*) FROM sqlite_master WHERE type='table'", 1024,
			&table_check_stmt, &tail);
	sqlite3_step(table_check_stmt);
	table_count = sqlite3_column_int(table_check_stmt, 0);
	sqlite3_finalize(table_check_stmt);

	return table_count;
}

void CSQLiteDataConnectionProvider::createDatabaseStructure()
{
	Logger::info(className,"Opened database is empty, creating tables");

	// creating tables
	sqlite3_exec(getConnection()->handle,
			"CREATE TABLE protos (ts INTEGER, ip INTEGER, port INTEGER, ptype INTEGER, "
					"in_bytes INTEGER, out_bytes INTEGER, in_pkts INTEGER, out_pkts INTEGER)",
			0, 0, 0);

	sqlite3_exec(getConnection()->handle,
			"CREATE TABLE totals (ts INTEGER, ip INTEGER, "
					"tcp_in_bytes INTEGER, tcp_out_bytes INTEGER, tcp_in_pkts INTEGER, tcp_out_pkts INTEGER, "
					"udp_in_bytes INTEGER, udp_out_bytes INTEGER, udp_in_pkts INTEGER, udp_out_pkts INTEGER, "
					"icmp_in_bytes INTEGER, icmp_out_bytes INTEGER, icmp_in_pkts INTEGER, icmp_out_pkts INTEGER, "
					"other_in_bytes INTEGER, other_out_bytes INTEGER, other_in_pkts INTEGER, other_out_pkts INTEGER)",
			0, 0, 0);

	// creating indices
	sqlite3_exec(getConnection()->handle,
			"CREATE INDEX protos_idx_ip_ts_ptype ON protos (ip, ts, ptype)", 0,
			0, 0);
	sqlite3_exec(getConnection()->handle,
			"CREATE INDEX totals_idx_ts_ip ON totals (ts, ip)", 0, 0, 0);
}

void CSQLiteDataConnectionProvider::run()
{
	while (1)
	{
		time_t currentTs = time(NULL), connectionTs = getConnection()->ts;

		struct tm currentDt, connectionDt;
		memcpy(&currentDt, localtime(&currentTs), sizeof(struct tm));
		memcpy(&connectionDt, localtime(&connectionTs), sizeof(struct tm));

		if (currentDt.tm_mon != connectionDt.tm_mon)
		{
			std::stringstream strm;
			strm << "Month has changed " << currentDt.tm_mon << " -> "
					<< connectionDt.tm_mon;

			Logger::info(className, strm.str());
			Logger::info(className, "Going to recreate stats database");

			reopenConnection(true);
		}

		if (getFreeDiskPercentage() < DISK_FREE_PRCNT_THRESHOLD
				&& getDBSizePercentage() > DBFILE_SIZE_PRCNT_THRESHOLD)
		{
			std::stringstream strm;

			strm << "Less than " << DISK_FREE_PRCNT_THRESHOLD
					<< "% of disk space left on "
					<< CConfiguration::getInstance()->getDatabasePath();
			Logger::info(className, strm.str());

			strm.clear();

			strm << "And " << databaseFile << " occupies more than "
					<< DBFILE_SIZE_PRCNT_THRESHOLD << "% of "
					<< CConfiguration::getInstance()->getDatabasePath()
					<< " total size";
			Logger::info(className, strm.str());

			Logger::info(className, "Going to re-create DB");

			reopenConnection(true);
		}

#ifdef _MSC_VER
		Sleep(5 * 60 * 1000);
#else
		sleep(5 * 60);
#endif
	}
}

int CSQLiteDataConnectionProvider::getFreeDiskPercentage()
{
	unsigned long long bfree = 0;
	unsigned long long btotal = 1;
#ifdef _MSC_VER
	DWORD dwSectPerClust = 0;
	DWORD dwBytesPerSect = 0;
	DWORD dwFreeClusters = 0;
	DWORD dwTotalClusters = 0;

	char* disk = strtok(CConfiguration::getInstance()->getDatabasePath(), "\\");
	int res = GetDiskFreeSpaceA(disk,
			&dwSectPerClust,
			&dwBytesPerSect,
			&dwFreeClusters,
			&dwTotalClusters);
	btotal = (__int64)dwTotalClusters * dwSectPerClust * dwBytesPerSect;
	bfree = (__int64)dwFreeClusters * dwSectPerClust * dwBytesPerSect;
#else
	struct statvfs s;
	statvfs(CConfiguration::getInstance()->getDatabasePath(), &s);

	bfree = s.f_bsize * s.f_bfree;
	btotal = s.f_bsize * s.f_blocks;
#endif
	return (bfree * 100) / btotal;
}

int CSQLiteDataConnectionProvider::getDBSizePercentage()
{
	struct stat s;
	stat(databaseFile, &s);
	long dbsize = s.st_size;
	unsigned long long btotal = 1;

#ifdef _MSC_VER
	DWORD dwSectPerClust = 0;
	DWORD dwBytesPerSect = 0;
	DWORD dwFreeClusters = 0;
	DWORD dwTotalClusters = 0;

	char* disk = strtok(CConfiguration::getInstance()->getDatabasePath(), "\\");

	int res = GetDiskFreeSpaceA(disk,
			&dwSectPerClust,
			&dwBytesPerSect,
			&dwFreeClusters,
			&dwTotalClusters);

	btotal = (__int64) dwTotalClusters * dwSectPerClust * dwBytesPerSect;
#else
	struct statvfs vs;

	statvfs(CConfiguration::getInstance()->getDatabasePath(), &vs);
	btotal = vs.f_bsize * vs.f_blocks;

#endif
	return (dbsize * 100) / btotal;
}

void CSQLiteDataConnectionProvider::reopenConnection(bool remove)
{
	char* _db = strdup(databaseFile);

	lock();
	closeConnection();

	if (remove)
		unlink(_db);

	openConnection();
	unlock();

	free(_db);
}

CSQLiteDataConnectionProvider* CSQLiteDataConnectionProvider::instance = NULL;



