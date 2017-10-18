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

#include "CBaseThread.h"

#ifdef _MSC_VER
#include <iostream>
#include "win32\win_headers.h"

#endif

std::string CBaseThread::className;
int CBaseThread::gtcnt = 0;

void * CBaseThread::_thread_start(void* instance)
{
	CBaseThread *casted = ((CBaseThread *) instance);

	Logger::info(className,
			string("Thread #")
					+ boost::lexical_cast<std::string>(casted->getId())
					+ " starting");
	casted->run();
	return NULL;
}

CBaseThread::CBaseThread()
{
	className = string(__func__);
	id = gtcnt++;
	thread = 0;
}

void CBaseThread::join()
{
#ifdef _MSC_VER
	WaitForSingleObject(thread, INFINITE);
#else
	pthread_join(thread, NULL);
#endif
}

void CBaseThread::start()
{
#ifdef _MSC_VER
	thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_thread_start, this, 0, NULL);
#else
	pthread_create(&thread, NULL, _thread_start, this);
#endif
}
