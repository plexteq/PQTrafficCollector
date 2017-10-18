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

#ifndef SRC_WEBSERVER_RESPONSE_CBASEHTTPRESPONSE_H_
#define SRC_WEBSERVER_RESPONSE_CBASEHTTPRESPONSE_H_

#include <boost/asio.hpp>
#include <boost/asio/read_until.hpp>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include "../../utility/Logger.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

class CBaseHttpResponse
{
	std::string className;

protected:

	const char* httpVersion = "HTTP/1.1";
	const char* headerSeparator = "\r\n";
	const char* defaultStatus = "200 OK";

	boost::shared_ptr<tcp::socket> socket;
	map<string, string> headers;
	string body;
	string status;

	/* Call methods for parsing incoming requests text representation
	 *
	 * Parameters:
	 *
	 * Return value: void (request fields wiil be updated with parsed values)
	 * */
	void addGenericHeaders();

	CBaseHttpResponse(boost::shared_ptr<tcp::socket> socket, string contentType)
	{
		this->className = string(__func__);
		this->socket = socket;
		addGenericHeaders();
		setStatus(defaultStatus);
		setContentType(contentType);
	}

	/* Call methods for parsing incoming requests text representation
	 *
	 * Parameters:
	 *
	 * Return value: void (request fields wiil be updated with parsed values)
	 * */
	void writeHeaders()
	{
		boost::system::error_code error;
		boost::asio::write(*(socket.get()),
				boost::asio::buffer(serializeStatusLine() + serializeHeaders()),
				error);

		if (error.value() != 0)
		{
			Logger::error(className, error.message());
		}
	}

public:
	virtual ~CBaseHttpResponse()
	{
	}

	void setStatus(string status)
	{
		this->status = status;
	}

	/* Add basic headers that are the same for all server responses
	 *
	 * Parameters:
	 * name - a header name
	 * value - a header value
	 *
	 * Return value: void
	 * */
	void addHeader(string name, string value);

	void setContentType(string contentType);
	void setBody(string body);
	void setContentLength(long length);

	string serialize();
	string serializeHeaders();
	string serializeStatusLine();

	/* Serialize a response and write it to the socket
	 *
	 * Parameters:
	 *
	 * Return value: void
	 * */
	virtual void write()
	{
		boost::system::error_code error;

		string data = serialize();

		boost::asio::write(*(socket.get()), boost::asio::buffer(data), error);

		if (error.value() != 0)
		{
			Logger::error(className, error.message());
		}
	}
};

class AHttpResponseWriter
{
public:
	virtual ~AHttpResponseWriter()
	{
	}

	/* Write a response data to the socket
	 *
	 * Parameters:
	 * response - a response pointer to write
	 *
	 * Return value: void (request fields wiil be updated with parsed values)
	 * */
	virtual void writeResponse(CBaseHttpResponse* response) = 0;
};

class CHttpResponseWriter: public AHttpResponseWriter
{
public:
	virtual ~CHttpResponseWriter()
	{
	}

	/* Write a response data to the socket
	 *
	 * Parameters:
	 * response - a response pointer to write (it must be not NULL)
	 *
	 * Return value: void (request fields wil be updated with parsed values)
	 * */
	virtual void writeResponse(CBaseHttpResponse* response)
	{
		response->write();
	}

};
#endif /* SRC_WEBSERVER_RESPONSE_CBASEHTTPRESPONSE_H_ */
