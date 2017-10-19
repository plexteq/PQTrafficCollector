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

#ifndef SRC_WEBSERVER_REQUEST_CHTTPREQUESTREADER_H_
#define SRC_WEBSERVER_REQUEST_CHTTPREQUESTREADER_H_

#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

class AHttpRequestReader
{
public:

	AHttpRequestReader()
	{
	}

	virtual ~AHttpRequestReader()
	{
	}

	/* Read some data to the string from the socket 'socket'
	 *
	 * Parameters:
	 * socket - socket to reading request data
	 *
	 * Return value: std::string - data from the socket
	 * */
	virtual std::string read(
			boost::shared_ptr<boost::asio::ip::tcp::socket> socket) = 0;
};

class CHttpRequestReader: public AHttpRequestReader
{
	/* Read data to the string from the socket 'socket'
	 * until it does contain a '\r\n\r\n' sequence
	 *
	 * Parameters:
	 * socket - socket to reading request data
	 *
	 * Return value: std::string - a request up to body string from the socket
	 * */
	std::string readRequestUpToBody(
			const boost::shared_ptr<boost::asio::ip::tcp::socket>& socket);
public:

	CHttpRequestReader()
	{
	}

	virtual ~CHttpRequestReader()
	{
	}

	/* Read a request status line and headers to the string from the socket 'socket'
	 *
	 * Parameters:
	 * socket - socket to reading request data
	 *
	 * Return value: std::string - data from the socket
	 * */
	virtual std::string read(
			boost::shared_ptr<boost::asio::ip::tcp::socket> socket);
};

#endif /* SRC_WEBSERVER_REQUEST_CHTTPREQUESTREADER_H_ */
