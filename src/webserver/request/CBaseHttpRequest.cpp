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

#include "CBaseHttpRequest.h"

#include "CHttpRequest.h"

string CBaseHttpRequest::parseRoute(string statusLine)
{
	size_t posStart = statusLine.find('/');
	size_t posEnd = string::npos;

	if (posStart == string::npos)
		return "";

	posEnd = statusLine.find_first_of(" ?", posStart);

	if (posEnd == string::npos)
		return "";

	return statusLine.substr(posStart, posEnd - posStart);
}

void CBaseHttpRequest::parse()
{
	parseStatusLine();
	parseMethod();

	if (method != RequestMethod::UNKNOWN_M)
	{
		parseParams();
	}
}

void CBaseHttpRequest::parseStatusLine()
{
	int sepPos = requestStr.find(headerSeparator);

	if (sepPos != string::npos)
	{
		statusLine = requestStr.substr(0, sepPos);
	}
}

void CBaseHttpRequest::parseMethod()
{
	if (requestStr.find("GET ") == 0)
		method = RequestMethod::GET;
	else if (requestStr.find("POST ") == 0)
		method = RequestMethod::POST;
}

string CBaseHttpRequest::decodeSpaces(string str)
{
	stringstream s;
	size_t pos = string::npos;

	do
	{
		pos = str.find("%20");

		if (pos != string::npos)
			str = str.replace(pos, 3, " ");

	} while (pos != string::npos);

	return str;
}

void CBaseHttpRequest::parseParams()
{
	if (method == RequestMethod::UNKNOWN_M)
		return;

	size_t posStart = requestStr.find('?');
	size_t posEnd = requestStr.find(" HTTP/1");
	size_t currPos = posStart + 1;

	if (posStart == string::npos || posEnd == string::npos)
		return;

	while (currPos < posEnd)
	{
		size_t eqwPos = requestStr.find('=', currPos);
		size_t delimPos = requestStr.find('&', currPos);

		string key;
		string value;

		if (eqwPos != string::npos)
		{
			key = requestStr.substr(currPos, eqwPos - currPos);

			if (delimPos == string::npos)
				delimPos = posEnd;

			value = requestStr.substr(eqwPos + 1, delimPos - eqwPos - 1);
		}

		value = decodeSpaces(value);

		if (!key.empty())
		{
			params.push_back(pair<string, string>(key, value));
		}

		if (delimPos != string::npos)
		{
			currPos = delimPos + 1;
		}
		else
		{
			break;
		}
	}
}
