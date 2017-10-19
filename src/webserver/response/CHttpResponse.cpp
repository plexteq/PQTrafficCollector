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

#include "CHttpResponse.h"

#include <fstream>

void CHttpErrorResponse::addStatusCode(StatusCodes code)
{
	stringstream strm;

	strm << code << " ";

	switch (code)
	{
	case Bad_Request:
		strm << "Bad Request";
		break;
	case Not_Found:
		strm << "Not Found";
		break;
	case Method_Not_Allowed:
		strm << "Method Not Allowed";
		break;
	case Internal_Server_Error:
		strm << "Internal Server Error";
		break;
	}

	setStatus(strm.str());
}

void CHttpFileResponse::write()
{
	std::ifstream source_file(fileName,
			std::ios_base::binary | std::ios_base::ate);

	if (!source_file)
	{
		Logger::error(className,
				string("Cannot open a file \"") + fileName + "\". Skipped.");

		return;
	}

	addHeaderAttachmentName();
	addHeaderAttachmentLength(source_file);

	writeHeaders();
	writeFile(source_file);
}

void CHttpFileResponse::addHeaderAttachmentName()
{
	std::string shortFileName;

	std::size_t found = fileName.find_last_of("/\\");
	if (found != std::string::npos)
	{
		shortFileName = fileName.substr(found + 1);
	}
	else
	{
		shortFileName = fileName;
	}

	addHeader("Content-Disposition",
			"attachment; filename=\"" + shortFileName + "\"");
}

void CHttpFileResponse::addHeaderAttachmentLength(std::ifstream &source)
{
	size_t fileLength = source.tellg();
	source.seekg(0);

	setContentLength(fileLength);
}

void CHttpFileResponse::writeFile(std::ifstream &source)
{
	boost::system::error_code error;
	boost::array<char, 2048> buf;

	while (true)
	{
		if (source.eof() == false)
		{
			source.read(buf.c_array(), (std::streamsize) buf.size());
			if (source.gcount() <= 0)
			{
				string message = "Read file '" + fileName + "' error. "
						+ boost::lexical_cast<std::string>(source.gcount())
						+ " was read.";

				Logger::error(className, message);
				break;
			}

			boost::asio::write(*(this->socket),
					boost::asio::buffer(buf.c_array(), source.gcount()),
					boost::asio::transfer_all(), error);

			if (error)
			{
				Logger::error(className,
						"send error:"
								+ boost::lexical_cast<std::string>(error));
				break;
			}
		}
		else
		{
			Logger::debug(className,
					"send file " + fileName + " completed successfully.");
			break;
		}
	}
}
