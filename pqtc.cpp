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

#include <iostream>
#include "CPCAPLocalIPAddressResolver.h"
#include "APRBlockingQueue.h"
#include "CWorkerThread.h"
#include "CConfiguration.h"
#include "CPCAPPacketSniffer.h"
#include "CSQLiteDataConnectionProvider.h"
#include "CSQLiteDataProvider.h"
#include "CAggregatorThread.h"
#include "common.h"

using namespace std;

int main(int argc, char **argv)
{
	/*
	 * APR library initialization
	 */
	apr_initialize();

	/*
	 * Analyzing input arguments
	 * and setting up control structures
	 */
	CConfiguration::configure(argc, argv);
	CConfiguration::dump();

	pid_t pid;
 
	/* Clone ourselves to make a child */  
	pid = fork(); 
 
	/* If the pid is less than zero,
	 *    something went wrong when forking */
	if (pid < 0) {
	    exit(EXIT_FAILURE);
	}
 
	/* If the pid we got back was greater
	 *    than zero, then the clone was
	 *       successful and we are the parent. */
	if (pid > 0) {
	    exit(EXIT_SUCCESS);
	}

	/**
	 * Initialize DB connection
	 */
	ADataConnectionProvider *connectionProvider = CSQLiteDataConnectionProvider::getInstance();
	connectionProvider->dump();

	ADataProvider *dataProvider = new CSQLiteDataProvider(connectionProvider);

	/*
	 * Creating IP resolver
	 */
	CBaseIPResolver *ipresolver = new CPCAPLocalIPAddressResolver();
	ipresolver->dump();

	ushort queues = CConfiguration::getInstance()->getQueues(),
			threads = CConfiguration::getInstance()->getThreads();

	/*
	 * Crearting blocking queues for handling stats and packet data
	 */
	ABlockingQueue<hstat_t> *reportQueue = new APRBlockingQueue<hstat_t>();
	ABlockingQueue<work_unit_t> **workQueues = new ABlockingQueue<work_unit_t>*();
	for (int i = 0 ; i < queues ; i++)
		workQueues[i] = new APRBlockingQueue<work_unit_t>();

	/*
	 * Creating worker threads
	 */
	CBaseThread *workThread[threads];
	for (int i = 0 ; i < threads ; i++) {
		int queue = rand() % queues;
		workThread[i] = new CWorkerThread<work_unit_t>(workQueues[queue], reportQueue);
		workThread[i]->start();
	}

	CBaseThread *aggregatorThread = new CAggregatorThread(reportQueue, dataProvider);
	aggregatorThread->start();

	/*
	 * Creating and starting sniffer
	 */
	CBaseThread *sniffer = new CPCAPPacketSniffer<work_unit_t>(workQueues, ipresolver);
	sniffer->start();
	sniffer->join();

	return 0;
}
