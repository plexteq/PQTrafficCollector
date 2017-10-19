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

#ifndef CWORKERTHREAD_H_
#define CWORKERTHREAD_H_

#include "CBaseWorkerThread.h"
#include "../ABlockingQueue.h"
#include "../utility/Logger.h"
#include <iostream>
#include <algorithm>
#include <map>
#include <queue>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#ifndef _MSC_VER
#include <netinet/in.h>
#endif

#include "../common.h"

#ifdef _MSC_VER
#include "..\win32\win_headers.h"
#define __sync_synchronize() _ReadWriteBarrier();
#define __sync_lock_test_and_set(target, value) InterlockedExchange((volatile LONG*)target, value);

#define pthread_mutex_lock(mutex) WaitForSingleObject(mutex, INFINITE);
#define pthread_mutex_unlock(mutex) ReleaseMutex(mutex);
//typedef void pthread_mutex_t;
#endif
using namespace std;

template<typename T>
class WatchThread: public CBaseThread
{
private:
	std::string className = "WatchThread";

	CBaseThread *workerThread;
	int sleepTime;

public:
	virtual ~WatchThread()
	{
	}
	WatchThread(CBaseThread *thread, int sleep) :
			CBaseThread()
	{
		this->workerThread = thread;
		this->sleepTime = sleep;
	}
	virtual void run();
};

template<typename K>
class CWorkerThread: public CBaseWorkerThread<K>
{
	// Allowing access for watcher thread to access private members
	// It's safe to do this
	friend class WatchThread<K> ;

private:
	std::string className = "CWorkerThread";
	// Map indexed by locally bound IP address in long representation
	hstat_t *stats;

#ifdef _MSC_VER
	// Atomic char used to prevent concurrent access to stats[ip]
	pthread_mutex_t* mutex;
#else
	/*
	 * It`s for a Windows capability
	 */
	pthread_mutex_t mtx;
	pthread_mutex_t* mutex = &mtx;
#endif

	// Queue where thread pushes gathered stats
	ABlockingQueue<hstat_t> *reportQueue;

	void handlePacket(work_unit_t *unit);

	struct host_stats* merge(struct packet *packet,
			struct host_stats* existing);
public:
	virtual ~CWorkerThread()
	{
	}

	CWorkerThread(ABlockingQueue<K> *workQueue,
			ABlockingQueue<hstat_t> *reportQueue) :
			CBaseWorkerThread<K>(workQueue)
	{
		this->reportQueue = reportQueue;

#ifdef _MSC_VER
		this->mutex = NULL;
		mutex = (pthread_mutex_t*)CreateMutex(
				NULL,              // default security attributes
				FALSE,// initially not owned
				NULL);// unnamed mutex
		//mutex = 0;
#else
		pthread_mutex_init(mutex, NULL);
#endif
		stats = new hstat_t();
	}

	virtual void run();

	void dump();

	hstat_t* swap();
};

#endif /* CWORKERTHREAD_H_ */
