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

#ifndef SRC_WEBSERVER_REQUEST_CHTTPREQUESTBUILDER_H_
#define SRC_WEBSERVER_REQUEST_CHTTPREQUESTBUILDER_H_

#include "CHttpRequestReader.h"
#include <map>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "CBaseHttpRequest.h"

using namespace boost::asio;
using namespace boost::asio::ip;

class AHttpRequestBuilder
{
protected:
	/* A map that store pairs of string representation of a route and
	 * it a relevant request type.
	 * */
	static std::map<std::string, int> routesMap;

public:

	AHttpRequestBuilder()
	{
	}

	virtual ~AHttpRequestBuilder()
	{
	}

	/* Add a new pair to the route map
	 *
	 * Parameters:
	 * route - a string representation of a route
	 * type  - a relevant request type value
	 *
	 * Return value: std::string - a requested file name
	 * */
	static void addRoute(std::string route, int type)
	{
		routesMap[route] = type;
	}

	/* Read request data from the socket and create a new request object
	 *
	 * Parameters:
	 * sock - socket to reading request data
	 * reader - a pointer to the request reader which
	 * 			know how to read request from the socket
	 *
	 * Return value: CBaseHttpRequest* - a new request with data read from the socket
	 * */
	virtual CBaseHttpRequest* get(
			boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
			AHttpRequestReader* reader) = 0;

	/* Release a resources of 'request' object, created by AHttpRequestBuilder
	 *
	 * Parameters:
	 *
	 * Return value: std::string - a requested file name
	 * */
	virtual void free(CBaseHttpRequest* request)
	{
		delete request;
	}
};

class CHttpRequestBuilder: public AHttpRequestBuilder
{
public:

	/* Read request data from the socket and create a new request object
	 *
	 * Parameters:
	 * socket - socket to reading request data
	 * reader - a pointer to the request reader which
	 * 			know how to read request from the socket
	 *
	 * Return value: CBaseHttpRequest* - a new request with data read from the socket
	 * */
	virtual CBaseHttpRequest* get(
			boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
			AHttpRequestReader* reader);

private:
	/* Check is the given route 'route' exists in the route map and return a relevant request type,
	 *
	 * Parameters:
	 * route - a string representation of a route
	 *
	 * Return value: CBaseHttpRequest::RequestType - a relevant to the route request type, if
	 * 					this route doesn`t exist in the route map, return UNKNOWN_T
	 * */
	static CBaseHttpRequest::RequestType parseType(string route);
};

#endif /* SRC_WEBSERVER_REQUEST_CHTTPREQUESTBUILDER_H_ */
