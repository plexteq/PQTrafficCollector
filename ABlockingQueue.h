#ifndef ABLOCKINGQUEUE_H_
#define ABLOCKINGQUEUE_H_

template <typename T> class ABlockingQueue
{
	public:
		virtual void pop(T **e) = 0;
		virtual void trypop(T **e) = 0;
		virtual void push(T *e) = 0;
		virtual ~ABlockingQueue() {}
};

#endif /* ABLOCKINGQUEUE_H_ */
