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
#include "CHttpServerConfiguration.h"

#include <stdexcept>

#include "commands/CRequestHandler.h"
#include "request/CHttpRequestReader.h"
#include "request/CHttpRequestBuilder.h"

#include "../utility/TimeDateHelper.h"

/* Make sure that all static members initialize in the right order */
std::map<CBaseHttpRequest::RequestType, AHandleRequestCommand*> AHandleRequestCommandPool::poolMap;
map<string, int> AHttpRequestBuilder::routesMap;

CHttpServerConfiguration CHttpServerConfiguration::instance;

CHttpServerConfiguration::CHttpServerConfiguration()
{
	commandPool = NULL;
	requestReader = NULL;
	responseWriter = NULL;
	requestBuilder = NULL;
}

CHttpServerConfiguration::~CHttpServerConfiguration()
{
	delete responseWriter;
	responseWriter = NULL;

	delete requestBuilder;
	requestBuilder = NULL;

	delete requestReader;
	requestReader = NULL;

	delete commandPool;
	commandPool = NULL;
}

void CHttpServerConfiguration::init()
{
	setResponseWriter(new CHttpResponseWriter());
	setRequestBuilder(new CHttpRequestBuilder());
	setRequestReader(new CHttpRequestReader());
	setCommandPool(new CHandleRequestCommandPool());
	setPortNumber(4332);

	configRoutes();
	configCommandPool();
}

/* Configure a conformity of a request type to the concrete command */
void CHttpServerConfiguration::configCommandPool()
{
	addCommandToPool<CAllFilesRequestHandler>(CBaseHttpRequest::ALL_FILES);
	addCommandToPool<CUnknownRequestHandler>(CBaseHttpRequest::UNKNOWN_T);
	addCommandToPool<CStatusRequestHandler>(CBaseHttpRequest::STATUS);
	addCommandToPool<CFileRequestHandler>(CBaseHttpRequest::FILE);
}

/* Configure a conformity of a request type to the route */
void CHttpServerConfiguration::configRoutes()
{
	AHttpRequestBuilder::addRoute("/status",
			CBaseHttpRequest::RequestType::STATUS);
	AHttpRequestBuilder::addRoute("/files",
			CBaseHttpRequest::RequestType::ALL_FILES);
	AHttpRequestBuilder::addRoute("/get", CBaseHttpRequest::RequestType::FILE);
	AHttpRequestBuilder::addRoute("", CBaseHttpRequest::RequestType::UNKNOWN_T);
}

void CHttpServerConfiguration::setRequestBuilder(
		AHttpRequestBuilder* requestBuilder)
{
	if (requestBuilder == NULL)
		throw std::invalid_argument(
				"Server configuration error! The request builder pointer cannot be NULL");

	this->requestBuilder = requestBuilder;
}

void CHttpServerConfiguration::setResponseWriter(
		AHttpResponseWriter* responseWriter)
{
	if (responseWriter == NULL)
		throw std::invalid_argument(
				"Server configuration error! The response writer pointer cannot be NULL");

	this->responseWriter = responseWriter;
}
void CHttpServerConfiguration::setRequestReader(
		AHttpRequestReader* requestReader)
{
	if (requestReader == NULL)
		throw std::invalid_argument(
				"Server configuration error! The request reader pointer cannot be NULL");

	this->requestReader = requestReader;
}

void CHttpServerConfiguration::setCommandPool(
		AHandleRequestCommandPool* commandPool)
{
	if (commandPool == NULL)
		throw std::invalid_argument(
				"Server configuration error! The command factory pointer cannot be NULL");

	this->commandPool = commandPool;
}

void CHttpServerConfiguration::setPortNumber(int port)
{
	if (port == 0)
		throw std::invalid_argument(
				"Server configuration error! The port number cannot be 0");

	this->serverPort = port;
}

