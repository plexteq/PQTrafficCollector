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

#ifndef SRC_WEBSERVER_COMMANDS_AHANDLEREQUESTCOMMANDPOOL_H_
#define SRC_WEBSERVER_COMMANDS_AHANDLEREQUESTCOMMANDPOOL_H_

#include "AHandleRequestCommand.h"

#include <map>

class AHandleRequestCommandPool
{
protected:
	/* A map which store pairs of a request type and a pointer to the
	 * relevant command to handle this request
	 * */
	static std::map<CBaseHttpRequest::RequestType, AHandleRequestCommand*> poolMap;

public:
	/* Add a new pair of a request type and a pointer to the
	 * relevant command
	 *
	 * Parameters:
	 * type 	- [In] a request type
	 * command	- [In] a pointer to the relevant command to handle this request
	 *
	 * Return value:
	 * 		void (a new pair will be added to a pool map)
	 * */
	static void addCommand(CBaseHttpRequest::RequestType type,
			AHandleRequestCommand* command)
	{
		poolMap[type] = command;
	}

	virtual ~AHandleRequestCommandPool()
	{
		for (auto& typeCommandPair : poolMap)
		{
			delete typeCommandPair.second;
			typeCommandPair.second = NULL;
		}
	}

	/* Return a pointer to the command which is relevant to request type 'type'
	 *
	 * Parameters:
	 * type 	- [In] a request type
	 *
	 * Return value:
	 * 		AHandleRequestCommand*: pointer to the command for handling this request type
	 * */
	virtual AHandleRequestCommand* get(CBaseHttpRequest::RequestType type) = 0;

	/* Release all resources of a command 'command'
	 *
	 * Parameters:
	 * command 	- [In] a pointer to the command for releasing
	 *
	 * Return value: void
	 * */
	virtual void free(AHandleRequestCommand* command) = 0;
};

class CHandleRequestCommandPool: public AHandleRequestCommandPool
{
public:

	/* Return a pointer to the command which is relevant to request type 'type'
	 * If a command for this request type is not exists, try to return a pointer
	 * to a command for UNKNOWN_T request type.
	 *
	 * Parameters:
	 * type 	- [In] a request type
	 *
	 * Return value:
	 * 		AHandleRequestCommand*: pointer to the command for handling this
	 * 		request type or NULL if a command for this request type is not exists
	 * */
	virtual AHandleRequestCommand* get(CBaseHttpRequest::RequestType type)
	{
		if (poolMap.find(type) == poolMap.end())
			type = CBaseHttpRequest::RequestType::UNKNOWN_T;

		return poolMap[type];
	}

	/* Release all resources of a command 'command'. Ignoring in this case, because
	 * all commands is reused during a program life cycle.
	 *
	 * Parameters:
	 * command 	- [In] a pointer to the command for releasing
	 *
	 * Return value: void
	 * */
	virtual void free(AHandleRequestCommand* command)
	{
		//No op
	}
};

#endif /* SRC_WEBSERVER_COMMANDS_AHANDLEREQUESTCOMMANDPOOL_H_ */
