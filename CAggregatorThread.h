/*
 * CAggregatorThread.h
 *
 *  Created on: May 12, 2014
 *      Author: root
 */

#ifndef CAGGREGATORTHREAD_H_
#define CAGGREGATORTHREAD_H_

#include "common.h"
#include "CBaseWorkerThread.h"
#include "ABlockingQueue.h"
#include <math.h>

class CAggregatorThread : public CBaseWorkerThread<hstat_t>
{
	private:
		ADataProvider *dataProvider;
		vector<hstat_t*> *chunks;
		void dispose(hstat_t* r);
	public:
		CAggregatorThread(ABlockingQueue<hstat_t> *queue, ADataProvider *dataProvider);
		hstat_t* aggregate(vector<hstat_t*> *items);

		virtual ~CAggregatorThread() {
			delete chunks;
		}
		virtual void run();
};

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
			sleep(1);
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

#endif /* CAGGREGATORTHREAD_H_ */
