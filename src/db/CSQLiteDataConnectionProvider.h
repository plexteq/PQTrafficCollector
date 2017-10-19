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
//#include "../common.h"
#include "ADataConnectionProvider.h"
#include "../CBaseThread.h"
#include "../utility/Logger.h"
#include <sys/stat.h>
#ifdef _MSC_VER
#include "..\win32\win_headers.h"
#else
#include <sys/statvfs.h>
#include <unistd.h>
#endif
/*
 * Singleton database connection manager class
 */
class CSQLiteDataConnectionProvider: public CBaseThread,
		public ADataConnectionProvider
{
private:
	/*
	 * A name of a logger category
	 */
	std::string className;
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
	CSQLiteDataConnectionProvider() :
			CBaseThread(), ADataConnectionProvider()
	{
		className = string(__func__);
		databaseFile = NULL;
		openConnection();
		start();
	}


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
	void operator =(CSQLiteDataConnectionProvider const&);

	static CSQLiteDataConnectionProvider* getInstance();
	virtual ~CSQLiteDataConnectionProvider()
	{
		lock();
		closeConnection();
		delete databaseFile;
		unlock();
	}
	;

	char* getDBFileName()
	{
		return databaseFile;
	}
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

#endif /* CSQLITEDATACONNECTIONPROVIDER_H_ */
