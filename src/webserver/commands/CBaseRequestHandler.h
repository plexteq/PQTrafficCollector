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

#ifndef SRC_WEBSERVER_COMMANDS_CBASEREQUESTHANDLER_H_
#define SRC_WEBSERVER_COMMANDS_CBASEREQUESTHANDLER_H_

#include "../request/CBaseHttpRequest.h"
#include "../response/CBaseHttpResponse.h"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

class CBaseRequestHandler
{
	std::string className;
	AHttpResponseWriter* responseWriter = NULL;
protected:

	/* Prepare a response content for a given request 'request'
	 *
	 * Parameters:
	 * request 	- [In] a pointer to the client request
	 *
	 * Return value:
	 * 		std::string: a string representation of the response content
	 * */
	virtual std::string prepareContent(CBaseHttpRequest* request) = 0;

	/* Check if a prepared content is valid for writing to the response
	 *
	 * Parameters:
	 * content 	- [In] a prepared string content
	 *
	 * Return value:
	 * 		bool: true - if content valid, false - if invalid
	 * */
	virtual bool validateContent(std::string content);

	/* Create a new response of a needed type. If 'isValid' - false,
	 * create a new CHttpErrorResponse.
	 *
	 * Parameters:
	 * isValid 	- [In]  a flag which inform valid response content or
	 * 					need to write an error response
	 * socket 	- [In]  a socket for writing a response
	 * content 	- [In]  a string representation of a response content
	 *
	 * Return value:
	 * 		CBaseHttpResponse*: a pointer to the created server response
	 * */
	virtual CBaseHttpResponse* createResponse(bool isValid,
			boost::shared_ptr<tcp::socket> socket, std::string content) = 0;

	/* Write a response 'response'
	 *
	 * Parameters:
	 * response - [In] a pointer to the server response
	 *
	 * Return value: void
	 * */
	void writeResponse(CBaseHttpResponse* response);

	/* Release resources of a response 'response'
	 *
	 * Parameters:
	 * response - [In] a pointer to the server response
	 *
	 * Return value: void
	 * */
	void freeResponse(CBaseHttpResponse* response);

public:
	CBaseRequestHandler(AHttpResponseWriter* writer)
	{
		this->className = string(__func__);
		setResponseWriter(writer);
	}

	virtual ~CBaseRequestHandler()
	{
	}

	/* Process the given request and write a response to the socket 'sock'
	 *
	 * Parameters:
	 * socket 	- [In/Out] a socket for writing a response
	 * request  - [In]  a pointer to the client request, can be NULL if request
	 * 					haven`t data for handling during a response creation
	 *
	 * Return value: void
	 * */
	void process(boost::shared_ptr<tcp::socket> socket,
			CBaseHttpRequest* request = NULL);

	void setResponseWriter(AHttpResponseWriter* writer);
};

#endif /* SRC_WEBSERVER_COMMANDS_CBASEREQUESTHANDLER_H_ */
