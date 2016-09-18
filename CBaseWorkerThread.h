#ifndef CBASEWORKERTHREAD_H_
#define CBASEWORKERTHREAD_H_

#include "CBaseThread.h"
#include "ABlockingQueue.h"

template <typename T>
class CBaseWorkerThread : public CBaseThread
{
	private:
		ABlockingQueue<T> *queue;
	protected:
		virtual ABlockingQueue<T> *getQueue() { return queue; }
	public:
		CBaseWorkerThread(ABlockingQueue<T> *queue) : CBaseThread() {
			this->queue = queue;
		}
		virtual ~CBaseWorkerThread() {}
};

#endif /* CBASEWORKERTHREAD_H_ */
