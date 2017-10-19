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

#include "CAggregatorThread.h"

#include "../CConfiguration.h"

CAggregatorThread::CAggregatorThread(ABlockingQueue<hstat_t> *queue, ADataProvider *dataProvider) : CBaseWorkerThread<hstat_t>(queue) {
	this->dataProvider = dataProvider;
	chunks = new vector<hstat_t*>();
}

void CAggregatorThread::run()
{
	hstat_t *packet = NULL;
	time_t pts = time(NULL);

	unsigned int nThreads = CConfiguration::getInstance()->getThreads();

	while (1)
	{
		this->getQueue()->trypop(&packet);

		// trypop didn't return anything
		if (packet == NULL) {
#ifdef _MSC_VER
            Sleep(1000);
#else
			sleep(1);
#endif
			continue;
		}

		chunks->push_back(packet);

		// if time hasn't come or number of chunks to aggregate is less than
		// number of producing threads, skipping aggregation
		unsigned int tsdiff = abs(time(NULL) - pts);
		if (tsdiff < AGGREGATION_DELAY || chunks->size() < nThreads)
			continue;

		hstat_t* result = aggregate(chunks);
		dataProvider->insertRecord(result);
		dispose(result);

		pts = time(NULL);
	}
}

hstat_t* CAggregatorThread::aggregate(vector<hstat_t*> *items)
{
	if (items == NULL)
		return NULL;

	if (items->size() == 0)
		return NULL;

	if (items->size() == 1)
		return items->front();

	hstat_t* to = items->front();

	vector<hstat_t*>::iterator i = items->begin();
	while (++i != items->end())
	{
		hstat_t* from = (*i);
		for (hstat_t::iterator fromIterator = from->begin() ; fromIterator != from->end() ; fromIterator++)
		{
			hstat_t::iterator toNode = to->find(fromIterator->first);
			if (toNode != to->end())
				toNode->second->mergeFrom(fromIterator->second);
			else
				to->insert(make_pair(fromIterator->first, new struct host_stats(fromIterator->second)));

			delete fromIterator->second;

		}
		delete from;
	}

	items->clear();

	return to;
}

void CAggregatorThread::dispose(hstat_t* r)
{
	for (hstat_t::iterator i = r->begin() ; i != r->end() ; i++)
		delete i->second;
	delete(r);
}

