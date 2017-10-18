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

#ifndef SRC_WEBSERVER_COMMANDS_AHANDLEREQUESTCOMMAND_H_
#define SRC_WEBSERVER_COMMANDS_AHANDLEREQUESTCOMMAND_H_
#include <stdexcept>

#include "CBaseRequestHandler.h"
#include "../../utility/Logger.h"

#include "../request/CBaseHttpRequest.h"

class AHandleRequestCommand
{
protected:
	/* Receiver - a saved pointer to the concrete handler instance */
	CBaseRequestHandler* handler = NULL;

public:

	AHandleRequestCommand(CBaseRequestHandler* handler)
	{
		this->handler = handler;
	}

	virtual ~AHandleRequestCommand()
	{
		delete this->handler;
		this->handler = NULL;
	}

	/* Method which will call some Receiver`s method and pass them parameters
	 *
	 * Parameters:
	 * socket 	- [In/Out] the socket for writing a response
	 * request	- [In] a pointer to the received client request
	 *
	 * Return value:
	 * 		void (a server response will be written to the socket)
	 * */
	virtual void execute(boost::shared_ptr<tcp::socket> socket,
			CBaseHttpRequest* request) = 0;
};

class CHandleRequestCommand: public AHandleRequestCommand
{
	string className;

public:
	CHandleRequestCommand(CBaseRequestHandler* handler) :
			AHandleRequestCommand(handler)
	{
		className = string(__func__);
	}

	virtual ~CHandleRequestCommand()
	{
	}

	/* Method which will call Receiver`s method 'process' and pass them parameters
	 * If a pointer to the Receiver is NULL - write error message to the log
	 *
	 * Parameters:
	 * socket 	- [In/Out] the socket for writing a response
	 * request	- [In] a pointer to the received client request
	 *
	 * Return value:
	 * 		void (a server response will be written to the socket)
	 * */
	virtual void execute(boost::shared_ptr<tcp::socket> socket,
			CBaseHttpRequest* request = NULL)
	{
		if (handler)
		{
			handler->process(socket, request);
		}
		else
		{
			std::stringstream strm;

			strm << "A handler is not defined for URL ";

			if(request) strm << request->getRoute();
			else strm << "<unknown>";

			Logger::error(className, strm.str());
		}
	}
};

#endif /* SRC_WEBSERVER_COMMANDS_AHANDLEREQUESTCOMMAND_H_ */
