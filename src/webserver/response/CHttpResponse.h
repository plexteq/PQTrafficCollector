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

#ifndef SRC_WEBSERVER_RESPONSE_CHTTPRESPONSE_H_
#define SRC_WEBSERVER_RESPONSE_CHTTPRESPONSE_H_

#include "CBaseHttpResponse.h"
#include <boost/shared_ptr.hpp>

class CHttpJsonResponse: public CBaseHttpResponse
{
public:
	CHttpJsonResponse(boost::shared_ptr<tcp::socket> socket) :
			CBaseHttpResponse(socket, "application/json")
	{
	}

	CHttpJsonResponse(boost::shared_ptr<tcp::socket> socket, string content) :
			CHttpJsonResponse(socket)
	{
		setBody(content);
	}
};

class CHttpTextResponse: public CBaseHttpResponse
{
public:
	CHttpTextResponse(boost::shared_ptr<tcp::socket> socket) :
			CBaseHttpResponse(socket, "text/plain")
	{
	}

	CHttpTextResponse(boost::shared_ptr<tcp::socket> socket, string content) :
			CHttpTextResponse(socket)
	{
		setBody(content);
	}
};

class CHttpErrorResponse: public CBaseHttpResponse
{
public:
	enum StatusCodes
	{
		Bad_Request = 400,
		Not_Found = 404,
		Method_Not_Allowed = 405,
		Internal_Server_Error = 500
	};

	typedef enum StatusCodes StatusCodes;

	CHttpErrorResponse(boost::shared_ptr<tcp::socket> socket) :
			CBaseHttpResponse(socket, "text/plain")
	{
	}

	CHttpErrorResponse(boost::shared_ptr<tcp::socket> socket, string status) :
			CHttpErrorResponse(socket)
	{
		setStatus(status);
	}

	CHttpErrorResponse(boost::shared_ptr<tcp::socket> socket, StatusCodes code) :
			CHttpErrorResponse(socket)
	{
		addStatusCode(code);
	}

	/* Write to the socket only response headers
	 *
	 * Parameters:
	 *
	 * Return value: void
	 * */
	virtual void write()
	{
		writeHeaders();
	}

	/* Add a status code of the responce to the status line
	 *
	 * Parameters:
	 * code - a http response error code
	 *
	 * Return value: void (request fields wiil be updated with parsed values)
	 * */
	void addStatusCode(StatusCodes code);
};

class CHttpFileResponse: public CBaseHttpResponse
{
	std::string className;
	string fileName;

	/* Write a file from a file stream'source' to the socket
	 *
	 * Parameters:
	 * source - an opened file stream fo reading a file
	 *
	 * Return value: void
	 * */
	void writeFile(std::ifstream &source);

	/* Add a file name to response headers
	 *
	 * Parameters:
	 *
	 * Return value: void
	 * */
	void addHeaderAttachmentName();

	/* Add a length of a fileto response headers
	 *
	 * Parameters:
	 * source -an opened file stream fo reading a file
	 *
	 * Return value: void
	 * */
	void addHeaderAttachmentLength(std::ifstream &source);

public:

	CHttpFileResponse(boost::shared_ptr<tcp::socket> socket) :
			CBaseHttpResponse(socket, "application/force-download")
	{
		addHeader("Content-Disposition", "attachment");
		addHeader("Content-Transfer-Encoding", "binary");
		addHeader("Accept-Ranges", "bytes");
	}

	CHttpFileResponse(boost::shared_ptr<tcp::socket> socket, string fileName) :
			CHttpFileResponse(socket)
	{
		this->className = string(__func__);
		this->fileName = fileName;
	}

	/* Write a file to the socket
	 *
	 * Parameters:
	 *
	 * Return value: void
	 * */
	virtual void write();
};

#endif /* SRC_WEBSERVER_RESPONSE_CHTTPRESPONSE_H_ */
