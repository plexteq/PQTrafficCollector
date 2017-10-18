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

#ifndef SRC_WEBSERVER_CHTTPSERVERCONFIGURATION_H_
#define SRC_WEBSERVER_CHTTPSERVERCONFIGURATION_H_

#include "commands/AHandleRequestCommandPool.h"
#include "request/CHttpRequestReader.h"
#include "request/CHttpRequestBuilder.h"

class CHttpServerConfiguration
{
	AHttpResponseWriter* responseWriter = 0;
	AHttpRequestBuilder* requestBuilder = 0;
	AHttpRequestReader* requestReader = 0;
	AHandleRequestCommandPool* commandPool = 0;

	int serverPort = 0;

	static CHttpServerConfiguration instance;

	/* Set a string representation of the route and
	 * it relevant request type
	 *
	 * Parameters:
	 *
	 * Return value: void
	 * */
	void configRoutes();

	/* Create and add to the server commands pool new commands with
	 * a specific handler for each request type
	 *
	 * Parameters:
	 *
	 * Return value: void
	 * */
	void configCommandPool();

	template<typename THandler>
	void addCommandToPool(CBaseHttpRequest::RequestType type)
	{
		THandler* handler = new THandler(responseWriter);

		AHandleRequestCommandPool::addCommand(type,
				new CHandleRequestCommand(handler));
	}

public:
	CHttpServerConfiguration();
	virtual ~CHttpServerConfiguration();

	virtual void init();

	static CHttpServerConfiguration* getInstance()
	{
		return &instance;
	}

	AHandleRequestCommandPool* getCommandPool()
	{
		return commandPool;
	}

	AHttpRequestReader* getRequestReader()
	{
		return requestReader;
	}

	AHttpRequestBuilder* getRequestBuilder()
	{
		return requestBuilder;
	}

	AHttpResponseWriter* getResponseWriter()
	{
		return responseWriter;
	}

	int getPortNumber()
	{
		return serverPort;
	}

	void setResponseWriter(AHttpResponseWriter* responseWriter);
	void setRequestBuilder(AHttpRequestBuilder* requestBuilder);
	void setRequestReader(AHttpRequestReader* requestReader);
	void setCommandPool(AHandleRequestCommandPool* commandPool);
	void setPortNumber(int port);
};

#endif /* SRC_WEBSERVER_CHTTPSERVERCONFIGURATION_H_ */
