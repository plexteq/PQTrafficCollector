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

#include <stdexcept>

#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "CHttpServer.h"
#include "CHttpServerConfiguration.h"

#include "../utility/Logger.h"
#include "../CConfiguration.h"

#include "commands/AHandleRequestCommand.h"

#include "request/CBaseHttpRequest.h"
#include "request/CHttpRequestBuilder.h"

using namespace boost::filesystem;
using namespace boost::asio;
using namespace boost::asio::ip;


CHttpServer::CHttpServer(int port, AHandleRequestCommandPool* commandPool,
		AHttpRequestReader* requestReader, AHttpRequestBuilder* requestBuilder) :
		AHttpServer(port)
{
	this->className = string(__func__);
	setCommandPool(commandPool);
	setRequestReader(requestReader);
	setRequestBuilder(requestBuilder);
}

void CHttpServer::setCommandPool(AHandleRequestCommandPool* commandPool)
{
	if (commandPool == NULL)
		throw std::invalid_argument("The command pool pointer cannot be NULL");

	this->commandPool = commandPool;
}

void CHttpServer::setRequestReader(AHttpRequestReader* requestReader)
{
	if (requestReader == NULL)
		throw std::invalid_argument("The request reader pointer cannot be NULL");

	this->requestReader = requestReader;
}

void CHttpServer::setRequestBuilder(AHttpRequestBuilder* requestBuilder)
{
	if (requestBuilder == NULL)
		throw std::invalid_argument("The request builder pointer cannot be NULL");

	this->requestBuilder = requestBuilder;
}

void CHttpServer::doWork(boost::shared_ptr<tcp::socket> socket)
{
	CBaseHttpRequest* request = requestBuilder->get(socket, requestReader);

	CBaseHttpRequest::RequestType type =
			(request != NULL) ?
					request->getType() :
					CBaseHttpRequest::RequestType::UNKNOWN_T;

	AHandleRequestCommand* command = commandPool->get(type);

	if (command != NULL)
	{
		command->execute(socket, request);
	}
	else
	{
		Logger::error(className,
				"Pointer to the request handler command is NULL!");
	}

	commandPool->free(command);
	requestBuilder->free(request);
}

void CHttpClientThread::run()
{
	boost::asio::io_service io_service;

	tcp::resolver resolver(io_service);
	tcp::resolver::query query("127.0.0.1",
			boost::lexical_cast<std::string>(
					CConfiguration::getInstance()->getServerPort()));
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

	for (int i = 0; i < 10; i++)
	{
		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);
		for (;;)
		{
			boost::array<char, 128> buf;
			boost::system::error_code error;

			size_t len = socket.read_some(boost::asio::buffer(buf), error);

			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.

			std::cout.write(buf.data(), len);
		}
#ifdef _MSC_VER
		Sleep(10000);
#else
		sleep(10);
#endif
	}
}
void CHttpServerThread::run()
{
	AHttpServer* server = 0;

	try
	{
		server =
				new CHttpServer(
						CHttpServerConfiguration::getInstance()->getPortNumber(),
						CHttpServerConfiguration::getInstance()->getCommandPool(),
						CHttpServerConfiguration::getInstance()->getRequestReader(),
						CHttpServerConfiguration::getInstance()->getRequestBuilder());

		server->run();
	} catch (std::exception& e)
	{
		Logger::error(className, "Can't start HttpServer");
		Logger::error(className, e.what());
	}

	if (server != 0)
		delete server;
}

