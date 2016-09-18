#ifndef CBASETHREAD_H_
#define CBASETHREAD_H_

#include <pthread.h>
#include <unistd.h>

class CBaseThread
{
	private:
		static int gtcnt;
		pthread_t thread;
		int id;

		static void * _thread_start(void* instance) {
			CBaseThread *casted = ((CBaseThread *) instance);
			cout << "Thread #" << casted->getId() << " starting" << endl;
			casted->run();
			return NULL;
		}

	public:
		virtual void run() = 0;

		int getId() { return id; }

		CBaseThread() {
			id = gtcnt++;
			thread = 0;
		}

		virtual ~CBaseThread() {
		}

		void join() {
			pthread_join(thread, NULL);
		}

		void start() {
			pthread_create(&thread, NULL, _thread_start, this);
		}
};

int CBaseThread::gtcnt = 0;

#endif /* CBASETHREAD_H_ */
