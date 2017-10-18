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

#ifndef SRC_WEBSERVER_COMMANDS_CREQUESTHANDLER_H_
#define SRC_WEBSERVER_COMMANDS_CREQUESTHANDLER_H_

#include "CBaseRequestHandler.h"

class CStatusRequestHandler: public CBaseRequestHandler
{
	long long serverStartTime = 0;

	/* Prepare a JSON response which store a server up-time
	 * and a list of all available DBs.
	 *
	 * Parameters:
	 * request 	- [In] a pointer to the client request
	 *
	 * Return value:
	 * 		std::string: a string representation of the response content
	 * */
	virtual std::string prepareContent(CBaseHttpRequest* request);

	/* Create a new CHttpJsonResponse type. If 'isValid' - false,
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
			boost::shared_ptr<tcp::socket> socket, std::string content);
public:
	CStatusRequestHandler(AHttpResponseWriter* responseWriter);
};

class CUnknownRequestHandler: public CBaseRequestHandler
{
	/* Prepare a status line error code
	 *
	 * Parameters:
	 * request 	- [In] a pointer to the client request
	 *
	 * Return value:
	 * 		std::string: a string representation of the response content
	 * */
	virtual std::string prepareContent(CBaseHttpRequest* request);

	/* Always return true
	 *
	 * Parameters:
	 * content 	- [In] a prepared string content
	 *
	 * Return value:
	 * 		bool: true - if content valid, false - if invalid
	 * */
	virtual bool validateContent(std::string content);

	/* Create a new CHttpErrorResponse.
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
			boost::shared_ptr<tcp::socket> socket, std::string content);

public:
	CUnknownRequestHandler(AHttpResponseWriter* responseWriter) :
			CBaseRequestHandler(responseWriter)
	{
	}
};

class CAllFilesRequestHandler: public CBaseRequestHandler
{
	/* Prepare a .tar archive of all available DB`s files
	 *
	 * Parameters:
	 * request 	- [In] a pointer to the client request
	 *
	 * Return value:
	 * 		std::string: a string path to the created archive
	 * */
	virtual std::string prepareContent(CBaseHttpRequest* request);

	/* Check if a .tar archive is exists
	 *
	 * Parameters:
	 * content 	- [In] a prepared string content
	 *
	 * Return value:
	 * 		bool: true - if content valid, false - if invalid
	 * */
	virtual bool validateContent(std::string content);

	/* Create a new CHttpFileResponse. If 'isValid' - false,
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
			boost::shared_ptr<tcp::socket> socket, std::string content);

public:
	CAllFilesRequestHandler(AHttpResponseWriter* responseWriter) :
			CBaseRequestHandler(responseWriter)
	{
	}
};

class CFileRequestHandler: public CBaseRequestHandler
{
	/* Prepare a string path to the requested file
	 *
	 * Parameters:
	 * request 	- [In] a pointer to the client request
	 *
	 * Return value:
	 * 		std::string: a string representation of the response content
	 * */
	virtual std::string prepareContent(CBaseHttpRequest* request);

	/* Check if a requested file is exists
	 *
	 * Parameters:
	 * content 	- [In] a prepared string content
	 *
	 * Return value:
	 * 		bool: true - if content valid, false - if invalid
	 * */
	virtual bool validateContent(std::string content);

	/* Create a new CHttpFileResponse. If 'isValid' - false,
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
			boost::shared_ptr<tcp::socket> socket, std::string content);

public:
	CFileRequestHandler(AHttpResponseWriter* responseWriter) :
			CBaseRequestHandler(responseWriter)
	{
	}
};
#endif /* SRC_WEBSERVER_COMMANDS_CREQUESTHANDLER_H_ */
