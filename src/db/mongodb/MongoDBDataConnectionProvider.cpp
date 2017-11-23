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

#include "../../CConfiguration.h"

#include "../../utility/TimeDateHelper.h"
#include "MongoDBDataConnectionProvider.h"
#include <mongocxx/cursor.hpp>

void MongoDBDataConnectionProvider::dump()
{
	Logger::info(className, "Dumping DB layer configuration:");
	Logger::info(className, string("database file: ") + databaseFile);
}

void MongoDBDataConnectionProvider::openConnection()
{
	Logger::info(className, "Opening new database connection");

	// forming database file name
	char fileName[256];
	FileHelper::generateDBName(fileName, sizeof(fileName),
			&(getConnection()->ts));

	databaseFile = new char[strlen(fileName)];
	strcpy(databaseFile, fileName);

	getConnection()->handle = &client;

	Logger::info(className, string("database file: ") + databaseFile);
}

void MongoDBDataConnectionProvider::closeConnection()
{
	Logger::info(className, "Closing database connection");

	getConnection()->handle = NULL;

	delete databaseFile;
	databaseFile = NULL;
}

MongoDBDataConnectionProvider* MongoDBDataConnectionProvider::getInstance()
{
	if (instance == NULL)
	{
		instance = new MongoDBDataConnectionProvider();
	}
	return instance;
}

void MongoDBDataConnectionProvider::run()
{
	while (1)
	{
		if (TimeDateHelper::isMonthHasChanged(&getConnection()->ts))
		{
			Logger::info(className, "Going to recreate stats database");

			reopenConnection(true);
		}

#ifdef _MSC_VER
		Sleep(5 * 60 * 1000);
#else
		sleep(5 * 60);
#endif
	}
}

MongoDBDataConnectionProvider* MongoDBDataConnectionProvider::instance = NULL;

