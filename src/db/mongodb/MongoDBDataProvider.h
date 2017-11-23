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

#ifndef SRC_DB_MONGODB_MONGODBDATAPROVIDER_H_
#define SRC_DB_MONGODB_MONGODBDATAPROVIDER_H_


#include "../ADataProvider.h"

#include "../../CBaseThread.h"
#include "../../utility/Logger.h"
#include "../../common.h"
#include <boost/lexical_cast.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>

typedef struct
{
	std::vector<bsoncxx::document::view_or_value> documents;
	const char* tableName;
} dbItem;

typedef std::vector<dbItem> dbVector;

class MongoDBDataProvider: public ADataProvider
{
private:
	/*
	 * A name of a logger category
	 */
	std::string className;

	CBaseThread* executorThread;

	ADataConnectionProvider* provider;

	dbVector retryVector;
	void insertStats(uint32_t ip, time_t ts, pstat_t* stat, int type,
			std::vector<bsoncxx::document::view_or_value> &documents);

	void insertTotals(uint32_t ip, time_t ts, struct host_stats* stats);

	void insertRecord(uint32_t ip, time_t ts, struct host_stats* stats);

	bsoncxx::document::view_or_value portsToJSON(uint32_t ip, time_t ts, int ptype, u_int16_t port,
			struct proto_stats *ps);

	bsoncxx::document::view_or_value totalsToJSON(uint32_t ip, time_t ts, struct host_stats* stats);

	mongocxx::collection getCollection(const char* collectionName);

public:
	MongoDBDataProvider(ADataConnectionProvider *provider);

	virtual ~MongoDBDataProvider();

	virtual ADataConnectionProvider* getProvider()
	{
		return provider;
	}

	void insertRecord(hstat_t *packet);
};


#endif /* SRC_DB_MONGODB_MONGODBDATAPROVIDER_H_ */
