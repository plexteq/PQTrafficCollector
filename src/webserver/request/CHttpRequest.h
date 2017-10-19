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

#ifndef SRC_WEBSERVER_REQUEST_CHTTPREQUEST_H_
#define SRC_WEBSERVER_REQUEST_CHTTPREQUEST_H_

#include "CBaseHttpRequest.h"

class CHttpRequest: public CBaseHttpRequest
{
	friend class CHttpRequestBuilder;
	CHttpRequest(boost::shared_ptr<tcp::socket> socket, RequestType type,
			string route, string requestData);

	CHttpRequest(boost::shared_ptr<tcp::socket> socket) :
			CBaseHttpRequest(socket)
	{
	}

public:
	virtual ~CHttpRequest()
	{
	}

	/* Lazy parameters parsing.
	 *
	 * Parameters:
	 *
	 * Return value:
	 * 		std::vector<pair<std::string, std::string>>:
	 * 		vector of parameter pairs name-value.
	 * */
	virtual vector<pair<string, string>> getParams()
	{
		if (params.size() == 0)
		{
			parseParams();
		}

		return params;
	}

	/* Check if the request method and type is allowed.
	 *
	 * Parameters:
	 *
	 * Return value:
	 * 		bool: 	true - if the method is GET and type is not UNKNOWN_T or files request
	 * 				false - if the method is not GET or type is UNKNOWN_T of FILE
	 * */
	virtual bool isValid();

	/* Prepare a string representation of the request
	 *
	 * Parameters:
	 *
	 * Return value: string: a string representation of the request
	 * */
	virtual string serialize();

	/* Write a serialized request to the socket
	 *
	 * Parameters:
	 *
	 * Return value: void
	 * */
	virtual void write();
};

class CHttpFileRequest: public CBaseHttpRequest
{
	friend class CHttpRequestBuilder;

	CHttpFileRequest(boost::shared_ptr<tcp::socket> socket, string route,
			string requestData) :
			CBaseHttpRequest(socket, RequestType::FILE, route, requestData)
	{
	}

	CHttpFileRequest(boost::shared_ptr<tcp::socket> socket) :
			CBaseHttpRequest(socket)
	{
		this->type = RequestType::FILE;
	}

public:
	virtual ~CHttpFileRequest()
	{
	}

	/* Return a requested file name
	 *
	 * Parameters:
	 *
	 * Return value: std::string - a requested file name
	 * */
	string getFileName();

	/* Check if the request method and type is allowed, and request file name is not empty.
	 *
	 * Parameters:
	 *
	 * Return value: bool: true - if a file name is not empty, the request method GET and
	 * 							the request type is FILE
	 * 					   false - otherwise
	 * */
	virtual bool isValid();

	/* Send a requested file to the socket
	 *
	 * Parameters:
	 *
	 * Return value: void
	 * */
	virtual void write();
};

#endif /* SRC_WEBSERVER_REQUEST_CHTTPREQUEST_H_ */
