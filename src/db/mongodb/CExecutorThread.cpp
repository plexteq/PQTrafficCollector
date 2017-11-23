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

#include "CExecutorThread.h"

#include <mongocxx/client.hpp>
#include "../../utility/Logger.h"

CExecutorThread::CExecutorThread(ADataConnectionProvider *provider,
		dbVector *retryVector, int sleepTime)
{
	this->className_ = string(__func__);
	this->provider = provider;
	this->retryVector = retryVector;
	this->sleepTime = sleepTime;
}

CExecutorThread::~CExecutorThread()
{
	// TODO Auto-generated destructor stub
}

void CExecutorThread::run()
{
	while (true)
	{
#ifdef _MSC_VER
		Sleep(this->sleepTime * 1000);
#else
		sleep(this->sleepTime);
#endif
		provider->lock();

		try
		{
			if (retryVector->size() > 0)
				handleDocuments();

		} catch (std::exception& ex)
		{
			Logger::error(className_, "Cannot insert documents to the database");
			Logger::error(className_, ex.what());
		}

		provider->unlock();
	}
}

void CExecutorThread::handleDocuments()
{
	char* dbName = provider->getDBFileName();

	mongocxx::client* client =
			(mongocxx::client*) provider->getConnection()->handle;
	mongocxx::database db = (*client)[dbName];

	for (dbVector::iterator i = retryVector->begin(); i != retryVector->end();
			++i)
	{
		mongocxx::collection coll = db[i->tableName];

		coll.insert_many(i->documents);

		i = retryVector->erase(i);
	}
}
