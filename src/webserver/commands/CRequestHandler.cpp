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

#include "CRequestHandler.h"

#include "../../CConfiguration.h"
#include "../../db/sqlite3/CSQLiteDataConnectionProvider.h"
#include "../../utility/FileHelper.h"
#include "../../utility/JSONHelper.h"
#include "../../utility/TimeDateHelper.h"

#include "../request/CHttpRequest.h"
#include "../response/CHttpResponse.h"

/* CStatusRequestHandler */
CStatusRequestHandler::CStatusRequestHandler(
		AHttpResponseWriter* responseWriter) :
		CBaseRequestHandler(responseWriter)
{
	serverStartTime = TimeDateHelper::getCurrentTimeS();
}

std::string CStatusRequestHandler::prepareContent(CBaseHttpRequest* request)
{
	if (request == NULL)
		return "";

	std::string jsonStr;
	std::vector<boost::filesystem::path> dbNames;
	FileHelper::getAvailableDBList(&dbNames,
			CConfiguration::getInstance()->getDatabasePath(),
			CSQLiteDataConnectionProvider::getInstance()->getDBFileName());

	void *root = JSONHelper::createObject();

	JSONHelper::addInteger(root, "uptime",
			TimeDateHelper::getCurrentTimeS() - serverStartTime);
	JSONHelper::addArrayPaths(root, "avail_dbs", dbNames);

	jsonStr = JSONHelper::serialize(root);

	JSONHelper::free(root);

	return jsonStr;
}

CBaseHttpResponse* CStatusRequestHandler::createResponse(bool isValid,
		boost::shared_ptr<tcp::socket> socket, std::string content)
{
	if (isValid)
		return new CHttpJsonResponse(socket, content);

	return new CHttpErrorResponse(socket,
			CHttpErrorResponse::StatusCodes::Internal_Server_Error);
}
/* CStatusRequestHandler */

/* CUnknownRequestHandler */
std::string CUnknownRequestHandler::prepareContent(CBaseHttpRequest* request)
{
	if (request == NULL)
		return "400 Bad Request";

	if (request->getMethod() == CBaseHttpRequest::RequestMethod::UNKNOWN_M)
		return "405 Method Not Allowed";

	return "404 Not Found";
}

bool CUnknownRequestHandler::validateContent(std::string content)
{
	return true;
}

CBaseHttpResponse* CUnknownRequestHandler::createResponse(bool isValid,
		boost::shared_ptr<tcp::socket> socket, std::string content)
{
	return new CHttpErrorResponse(socket, content);
}
/* CUnknownRequestHandler */

/* CAllFilesRequestHandler */
std::string CAllFilesRequestHandler::prepareContent(CBaseHttpRequest* request)
{
	char tarballError = 0;

	std::vector<boost::filesystem::path> dbNames;
	FileHelper::getAvailableDBList(&dbNames,
			CConfiguration::getInstance()->getDatabasePath(),
			CSQLiteDataConnectionProvider::getInstance()->getDBFileName());

	std::string tarFileName = FileHelper::createTarball(&dbNames, tarballError);

	if (tarballError)
	{
		FileHelper::deleteFile(tarFileName);
		return "";
	}

	return tarFileName;
}

bool CAllFilesRequestHandler::validateContent(std::string content)
{
	return FileHelper::fileExist(content);
}

CBaseHttpResponse* CAllFilesRequestHandler::createResponse(bool isValid,
		boost::shared_ptr<tcp::socket> socket, std::string content)
{
	if (isValid)
		return new CHttpFileResponse(socket, content);

	return new CHttpErrorResponse(socket,
			CHttpErrorResponse::StatusCodes::Internal_Server_Error);
}
/* CAllFilesRequestHandler */

/* CFileRequestHandler */
std::string CFileRequestHandler::prepareContent(CBaseHttpRequest* request)
{
	if (request == NULL)
		return "";

	CHttpFileRequest* fileRequest = (CHttpFileRequest*) request;

	std::string filePath = CConfiguration::getInstance()->getDatabasePath();
	filePath += "/" + fileRequest->getFileName();

	return filePath;
}

bool CFileRequestHandler::validateContent(std::string content)
{
	return FileHelper::fileExist(content);
}

CBaseHttpResponse* CFileRequestHandler::createResponse(bool isValid,
		boost::shared_ptr<tcp::socket> socket, std::string content)
{
	if (isValid)
		return new CHttpFileResponse(socket, content);

	return new CHttpErrorResponse(socket,
			CHttpErrorResponse::StatusCodes::Not_Found);

}
/* CFileRequestHandler */
