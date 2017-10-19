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

#ifndef APRBLOCKINGQUEUE_H_
#define APRBLOCKINGQUEUE_H_

#include <apr-1/apr.h>
#include <apr-1/apr_pools.h>
#include <apr-1/apr_queue.h>
#include "ABlockingQueue.h"
#include <stdlib.h>

#define POOL_MAX_FREE_BLOCKS 128
#define QUEUE_MAX_ELEMENTS 1024

using namespace std;

template <typename T>
class APRBlockingQueue : public ABlockingQueue<T>
{
	private:
		apr_pool_t *queuePool;
		apr_allocator_t *poolAllocator;
		apr_queue_t *queue;
	public:
		APRBlockingQueue();
		virtual ~APRBlockingQueue();
		virtual void pop(T **e);
		virtual void trypop(T **e);
		virtual void push(T *e);
};

template <typename T>
APRBlockingQueue<T>::APRBlockingQueue(){
	queuePool = NULL;
	poolAllocator = NULL;
	queue = NULL;

	apr_pool_create(&queuePool, NULL);
	poolAllocator = apr_pool_allocator_get(queuePool);
	apr_allocator_max_free_set(poolAllocator, POOL_MAX_FREE_BLOCKS);
	apr_queue_create(&queue, QUEUE_MAX_ELEMENTS, queuePool);
}

template <typename T>
APRBlockingQueue<T>::~APRBlockingQueue() {
	apr_queue_interrupt_all(queue);
	apr_queue_term(queue);
	apr_pool_clear(queuePool);
	apr_pool_destroy(queuePool);
}

template <typename T>
void APRBlockingQueue<T>::push(T *e) {
	apr_queue_push(queue, (void*) e);
}

template <typename T>
void APRBlockingQueue<T>::pop(T **e) {
	int s = apr_queue_pop(queue, (void**) e);
	if (s != APR_SUCCESS) *e = NULL;
}

template <typename T>
void APRBlockingQueue<T>::trypop(T **e) {
	int s = apr_queue_trypop(queue, (void**) e);
	if (s != APR_SUCCESS) *e = NULL;
}

#endif /* APRBLOCKINGQUEUE_H_ */
