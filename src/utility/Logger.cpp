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
#include "Logger.h"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Layout.hh"

void Logger::init(PriorityLevel priority)
{
	if (initialized)
		return;

	char fileError = 0;
	FileHelper::createFolder("./logs/", fileError);

	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.setPriority(Logger::convertToLog4Priority(priority));

	log4cpp::Appender *appender = NULL;

	if (fileError)
	{
		appender = new log4cpp::OstreamAppender("console", &std::cout);
		Logger::error(rootCategoryName,
				"Cannot create a directory ./logs/. Output will be redirected to the console");
	}
	else
		appender = new log4cpp::FileAppender("default", "./logs/pqtc.log");

	log4cpp::PatternLayout *layout = new log4cpp::PatternLayout();
	layout->setConversionPattern("[%d{%d %m %Y %H:%M:%S,%l}] %c [%p]: %m%n");

	appender->setLayout(layout);

	root.addAppender(appender);
	logCategories[rootCategoryName] = &log4cpp::Category::getRoot();

	initialized = true;
}

log4cpp::Priority::PriorityLevel Logger::convertToLog4Priority(
		PriorityLevel level)
{
	return level == DEBUG ? log4cpp::Priority::DEBUG :
			level == INFO ? log4cpp::Priority::INFO : log4cpp::Priority::ERROR;
}

bool Logger::checkCategoryExist(std::string& category)
{
	log4cpp::Category::exists((const string&) category);

	std::map<string, log4cpp::Category*>::iterator it;

	it = logCategories.find(category);
	return it != logCategories.end();
}

void Logger::write(std::string& category,
		log4cpp::Priority::PriorityLevel priority, std::string& message)
{
	if (!checkCategoryExist(category))
		addSubCategory(category);

	log4cpp::Category* ctg = logCategories[category];

	*ctg << priority << message;
}

void Logger::addSubCategory(std::string& category)
{
	logCategories[category] = &log4cpp::Category::getInstance(
			std::string(category));
}

void Logger::info(std::string& category, std::string message)
{
	write(category, log4cpp::Priority::INFO, message);
}

void Logger::debug(std::string& category, std::string message)
{
	write(category, log4cpp::Priority::DEBUG, message);
}

void Logger::error(std::string& category, std::string message)
{
	write(category, log4cpp::Priority::ERROR, message);
}

void Logger::error(std::string& category, std::string errMessage, int errValue)
{
	errMessage += " " + boost::lexical_cast<std::string>(errValue);

	write(category, log4cpp::Priority::ERROR, errMessage);
}

map<string, log4cpp::Category*> Logger::logCategories;
bool Logger::initialized = false;
string Logger::rootCategoryName = "root";
