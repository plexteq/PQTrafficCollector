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

#include "CBaseHttpResponse.h"

void CBaseHttpResponse::addGenericHeaders()
{
	addHeader("Server", "hptc");
	addHeader("Cache-Control", "no-cache");
	addHeader("Pragma", "no-cache");
	addHeader("Connection", "close");
	addHeader("Content-Length", "0");
}

void CBaseHttpResponse::addHeader(string name, string value)
{
	headers[name] = value;
}

void CBaseHttpResponse::setBody(string body)
{
	this->body = body;

	setContentLength(body.size());
}

void CBaseHttpResponse::setContentLength(long length)
{
	std::stringstream strm;
	strm << length;

	addHeader("Content-Length", strm.str());
}

void CBaseHttpResponse::setContentType(string contentType)
{
	addHeader("Content-Type", contentType);
}

string CBaseHttpResponse::serialize()
{
	std::stringstream strm;

	strm << serializeStatusLine();
	strm << serializeHeaders();
	strm << body << headerSeparator;

	return strm.str();
}

string CBaseHttpResponse::serializeStatusLine()
{
	std::stringstream strm;
	strm << httpVersion << " " << status << headerSeparator;

	return strm.str();
}

string CBaseHttpResponse::serializeHeaders()
{
	std::stringstream strm;

	std::map<std::string, std::string>::iterator iter;

	for (iter = headers.begin(); iter != headers.end(); ++iter)
	{
		strm << iter->first << ": " << iter->second << headerSeparator;
	}

	strm << headerSeparator;

	return strm.str();
}
