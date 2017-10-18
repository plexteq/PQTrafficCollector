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

#ifndef CBASEHTTPREQUEST_H_
#define CBASEHTTPREQUEST_H_

#include <boost/asio.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

class CHttpRequestBuilder;

class CBaseHttpRequest
{
public:
	typedef enum
	{
		UNKNOWN_T, ALL_FILES, FILE, STATUS
	} RequestType;

	typedef enum
	{
		UNKNOWN_M, POST, GET
	} RequestMethod;

	virtual ~CBaseHttpRequest()
	{
	}

	virtual bool isValid() = 0;

	virtual boost::shared_ptr<tcp::socket> getSocket()
	{
		return socket;
	}

	virtual string getRoute()
	{
		return route;
	}

	virtual string serialize()
	{
		return requestStr;
	}

	virtual RequestType getType()
	{
		return type;
	}

	virtual RequestMethod getMethod()
	{
		return method;
	}

	virtual string getStatusLine()
	{
		return statusLine;
	}

	virtual vector<pair<string, string>> getParams()
	{
		return params;
	}

	virtual void setRoute(string route)
	{
		this->route = route;
	}

	virtual void setString(string requestStr)
	{
		this->requestStr = requestStr;
	}

	virtual void setMethod(RequestMethod method)
	{
		this->method = method;
	}

	virtual void setParams(vector<pair<string, string>> params)
	{
		this->params = params;
	}

	virtual void write() = 0;

protected:
	friend class CHttpRequestBuilder;

	const char* _httpVersion = "HTTP/1.1";
	const char* bodySeparator = "\r\n\r\n";
	const char* headerSeparator = "\r\n";
	const char* defaultStatus = "200 OK";

	boost::shared_ptr<tcp::socket> socket;

	string route;
	string httpVersion;
	string requestStr;
	string statusLine;

	vector<pair<string, string>> params;

	RequestMethod method;
	RequestType type;

	/* Call methods for parsing incoming requests text representation
	 *
	 * Parameters:
	 *
	 * Return value: void (request fields wiil be updated with parsed values)
	 * */
	virtual void parse();

	/* Parse parameters from the request status line

	 * Parameters:
	 *
	 * Return value: void (request vector 'params' will be updated with parsed values)
	 * */
	virtual void parseParams();

	/* Parse the request method (GET or POST, otherwise - UNKNOWN_M)
	 *
	 * Parameters:
	 *
	 * Return value: void (request field 'method' will be updated with the parsed value)
	 * */
	virtual void parseMethod();

	/* Prepare a JSON response which store a server up-time
	 * and a list of all available DBs.
	 *
	 * Parameters:
	 *
	 * Return value: void (request field 'statusLine' will be updated with the parsed value)
	 * */
	virtual void parseStatusLine();

	/* Parse the request route from the request status line
	 *
	 * Parameters:
	 *
	 * Return value: void (request field 'route' will be updated with the parsed value)
	 * */
	static string parseRoute(string statusLine);

	CBaseHttpRequest(boost::shared_ptr<tcp::socket> socket, RequestType type,
			string route, string requestData) :
			CBaseHttpRequest(socket)
	{
		this->route = route;
		this->type = type;
		this->requestStr = requestData;
	}

	CBaseHttpRequest(boost::shared_ptr<tcp::socket> socket)
	{
		this->type = RequestType::UNKNOWN_T;
		this->method = RequestMethod::UNKNOWN_M;
		this->socket = socket;
		this->httpVersion = _httpVersion;
	}

	/* Replace URL code '%20' with a space
	 *
	 * Parameters:
	 * str - a string to decode
	 *
	 * Return value:
	 * 		std::string: a decoded string with a spaces and other not-url symbols
	 * */
	string decodeSpaces(string str);
};

#endif /* CBASEHTTPREQUEST_H_ */
