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

#ifndef SRC_UTILITY_TIMEDATEHELPER_H_
#define SRC_UTILITY_TIMEDATEHELPER_H_

#ifndef _MSC_VER
#include <sys/time.h>
#else
#include "..\win32\win_headers.h"
#include <time.h>
#endif

class TimeDateHelper
{
public:
	/* Return a current timestamp in seconds
	 *
	 * Parameters:
	 *
	 * Return value:
	 * 		long long: a current timestamp in seconds
	 *
	 * */
	static long long getCurrentTimeS()
	{
#ifndef _MSC_VER
		struct timeval tp;
		gettimeofday(&tp, 0);
		return tp.tv_sec;
#else
		return GetCurrentTime() / 1000;
#endif
	}

	/* Return a current timestamp in miliseconds
	 *
	 * Parameters:
	 *
	 * Return value:
	 * 		long long: a current timestamp in miliseconds
	 *
	 * */
	static long long getCurrentTimestamp()
	{
#ifndef _MSC_VER
		struct timeval tp;
		gettimeofday(&tp, 0);
		return tp.tv_sec * 1000 + tp.tv_usec / 1000;
#else
		return GetCurrentTime();
#endif
	}
};

#endif /* SRC_UTILITY_TIMEDATEHELPER_H_ */
