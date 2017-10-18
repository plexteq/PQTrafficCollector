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

#include "CHttpRequest.h"

CHttpRequest::CHttpRequest(boost::shared_ptr<tcp::socket> socket,
		RequestType type, string route, string requestData) :
		CBaseHttpRequest(socket, type, route, requestData)
{
}

void CHttpRequest::write()
{
	boost::system::error_code ignored_error;

	boost::asio::write(*(socket.get()), boost::asio::buffer(serialize()),
			ignored_error);
}

bool CHttpRequest::isValid()
{
	return method == RequestMethod::GET
			&& (type != RequestType::UNKNOWN_T && type != RequestType::FILE);
}

string CHttpRequest::serialize()
{
	if (requestStr.empty() && method != RequestMethod::UNKNOWN_M)
	{
		requestStr = method == RequestMethod::GET ? "GET " :
						method == RequestMethod::POST ? "POST " : "";

		requestStr += route;

		if (params.size() != 0)
			requestStr += "?";

		for (int i = 0; i < params.size(); i++)
		{
			requestStr += params[i].first + "=" + params[i].second;

			if (i != params.size() - 1)
				requestStr += "&";
		}

		requestStr += httpVersion + "\r\n";
	}

	return requestStr;
}

bool CHttpFileRequest::isValid()
{
	return (method == RequestMethod::GET && type == RequestType::FILE
			&& getFileName().length() > 0);
}

string CHttpFileRequest::getFileName()
{
	if (params.size() > 0 && params[0].first.compare("file"))
	{
		return params[0].second;
	}

	return "";
}

void CHttpFileRequest::write()
{
	boost::system::error_code ignored_error;

	boost::asio::write(*(socket.get()), boost::asio::buffer(serialize()),
			ignored_error);
}

