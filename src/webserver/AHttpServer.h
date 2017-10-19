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

#ifndef AHTTPSERVER_H_
#define AHTTPSERVER_H_

#include "../utility/Logger.h"
#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

/**
 * This is a synchronous http server based on bost::asio
 */
class AHttpServer
{
	std::string className = "AHttpServer";
	io_service io_serv;
	int port;

protected:

	/* Add a length of a fileto response headers
	 *
	 * Parameters:
	 * source -an opened file stream fo reading a file
	 *
	 * Return value: void
	 * */
	virtual void doWork(boost::shared_ptr<tcp::socket> socket) = 0;

	AHttpServer(int port)
	{
		this->port = port;
	}

public:

	/* Start listening a server port. In the endless loop create a socket
	 * and wait for connection. When connection is accepted, call doWork()
	 * method that must be implemented in the children class.
	 * Catch all std::exception-s inside an endless loop
	 *
	 * Parameters:
	 *
	 * Return value: void
	 * */
	void run()
	{
		tcp::acceptor acceptor(io_serv, tcp::endpoint(tcp::v4(), port));
		for (;;)
		{
			try
			{
				boost::shared_ptr<tcp::socket> shp_socket(
						new tcp::socket(io_serv));

				acceptor.accept(*(shp_socket.get()));

				doWork(shp_socket);
			} catch (std::exception& e)
			{
				Logger::error(className,
						std::string("Error occured during request processing: ")
								+ e.what());
			}
		}
	}

	virtual ~AHttpServer()
	{
	}
};

#endif
