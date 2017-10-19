#include "APRBlockingQueue.h"
#include "CConfiguration.h"
#include "common.h"
#include "PQTrafficCollector.h"
#include "network/CPCAPLocalIPAddressResolver.h"
#include "network/CWorkerThread.h"
#include "network/CPCAPPacketSniffer.h"
#include "network/CAggregatorThread.h"
#include "db/CSQLiteDataConnectionProvider.h"
#include "db/CSQLiteDataProvider.h"
#include "utility/Logger.h"
#include "webserver/CHttpServerConfiguration.h"
#include "webserver/CHttpServer.h"

void *PQTrafficCollector::sniffer = NULL;


void PQTrafficCollector::InitAndStart(int argc, char** argv)
{
	Logger::init(Logger::PriorityLevel::DEBUG);

	CConfiguration::configure(argc, argv);
	CConfiguration::dump();
	/*
	 * APR library initialization
	 */
	apr_initialize();

	/**
	 * Initialize DB connection
	 */
	ADataConnectionProvider *connectionProvider =
			CSQLiteDataConnectionProvider::getInstance();
	connectionProvider->dump();

	ADataProvider *dataProvider = new CSQLiteDataProvider(connectionProvider);

	/*
	 * Creating IP resolver
	 */
	CBaseIPResolver *ipresolver = new CPCAPLocalIPAddressResolver();
	ipresolver->dump();

	/* Initialize the webserver commands factory and a requests route map */
	int serverPort = CConfiguration::getInstance()->getServerPort();

	CHttpServerConfiguration::getInstance()->init();
	CHttpServerConfiguration::getInstance()->setPortNumber(serverPort);

	/*
	 * Create and starting a web server
	 */
	CBaseThread *server = new CHttpServerThread();
	server->start();

	ushort queues = CConfiguration::getInstance()->getQueues();
	const ushort threads = CConfiguration::getInstance()->getThreads();

	/*
	 * Crearting blocking queues for handling stats and packet data
	 */
	ABlockingQueue<hstat_t> *reportQueue = new APRBlockingQueue<hstat_t>();
	ABlockingQueue<work_unit_t> **workQueues =
			new ABlockingQueue<work_unit_t>*();
	for (int i = 0; i < queues; i++)
		workQueues[i] = new APRBlockingQueue<work_unit_t>();

	/*
	 * Creating worker threads
	 */
	CBaseThread **workThread = new CBaseThread*[threads]();
	for (int i = 0; i < threads; i++)
	{
		int queue = rand() % queues;
		workThread[i] = new CWorkerThread<work_unit_t>(workQueues[queue],
				reportQueue);
		workThread[i]->start();
	}

	CBaseThread *aggregatorThread = new CAggregatorThread(reportQueue,
			dataProvider);
	aggregatorThread->start();
	/*
	 * Creating and starting sniffer
	 */
	sniffer = new CPCAPPacketSniffer<work_unit_t>(workQueues, ipresolver);
	((CPCAPPacketSniffer<work_unit_t>*) sniffer)->start();
}

void PQTrafficCollector::Wait()
{
	((CPCAPPacketSniffer<work_unit_t>*) sniffer)->join();
}
