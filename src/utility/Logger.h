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

#ifndef SRC_UTILITY_LOGGER_H_
#define SRC_UTILITY_LOGGER_H_

#include <stdlib.h>
#include <iostream>
#include <map>
#include <string>
#include <boost/lexical_cast.hpp>

#include "FileHelper.h"

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"

using namespace std;

/* A wrapper for the 'log4cpp' logger */
class Logger
{
public:

	/* Logger priority levels.
	 * DEBUG - will be printed all messages
	 * INFO  - will be printed all info and error messages
	 * ERROR - will be printed only error messages
	 *
	 * */
	typedef enum
	{
		ERROR, INFO, DEBUG
	} PriorityLevel;

private:
	/* A flag which allow an initialization only one time */
	static bool initialized;
	/* A name of the root category */
	static string rootCategoryName;
	/* Map of a string category name and pointer to the actual category*/
	static map<string, log4cpp::Category*> logCategories;

	/* Check if a category with 'category' name exists.
	 *
	 * Parameters:
	 * category - [In] a variable by value of a category name
	 *
	 * Return value:
	 * 		bool: true - if exists, false - if not exists.
	 *
	 * */
	static bool checkCategoryExist(std::string& category);

	/* Add a new category name and object to the category map, if it not exists.
	 * Otherwise do nothing
	 *
	 * Parameters:
	 * category - [In] a variable by value of a category name
	 *
	 * Return value:
	 * 		void (the new category pair will be added to the category map)
	 *
	 * */
	static void addSubCategory(std::string& category);

	/* Write a message with a priority 'priority' of a category 'category'
	 * (if it not exists, add a new category)
	 *
	 * Parameters:
	 * category - [In] a variable by value of a category name
	 * priority - [In] a priority level of this message
	 * category - [In] a variable by value of a log message
	 *
	 * Return value:
	 * 		void (the new message will be wrote to the log if a priority level allows this)
	 *
	 * */
	static void write(std::string& category,
			log4cpp::Priority::PriorityLevel priority, std::string& message);

	/* Convert a Logger priority to a log4cpp priority constant value
	 *
	 * Parameters:
	 * level - [In] a Logger priority level value
	 *
	 * Return value:
	 * 		log4cpp::Priority::PriorityLevel: the relevant priority level
	 *
	 * */
	static log4cpp::Priority::PriorityLevel convertToLog4Priority(
			PriorityLevel level);
public:

	/* Initialize Logger and set a priority level 'priority'.
	 * If it cannot create a log file, all log messages will
	 * be written to the console.
	 *
	 * Parameters:
	 * priority - [In] a Logger priority level value
	 *
	 * Return value:
	 * 		void (Logger became initialized)
	 *
	 * */
	static void init(PriorityLevel priority);

	/* Write a log message with INFO priority
	 *
	 * Parameters:
	 * category - [In] a variable by value of a category name
	 * message  - [In] a string message to writing
	 *
	 * Return value:
	 * 		void (the new message will be wrote to the log if a priority level allows this)
	 *
	 * */
	static void info(std::string& category, std::string message);

	/* Write a log message with DEBUG priority
	 *
	 * Parameters:
	 * category - [In] a variable by value of a category name
	 * message  - [In] a string message to writing
	 *
	 * Return value:
	 * 		void (the new message will be wrote to the log if a priority level allows this)
	 *
	 * */
	static void debug(std::string& category, std::string message);

	/* Write a log message with ERROR priority
	 *
	 * Parameters:
	 * category - [In] a variable by value of a category name
	 * message  - [In] a string message to writing
	 *
	 * Return value:
	 * 		void (the new message will be wrote to the log if a priority level allows this)
	 *
	 * */
	static void error(std::string& category, std::string message);

	/* Write a log message with ERROR priority
	 *
	 * Parameters:
	 * category 	- [In] a variable by value of a category name
	 * errMessage   - [In] a string message to writing
	 * errValue  	- [In] an error value to writing
	 *
	 * Return value:
	 * 		void (the new message will be wrote to the log if a priority level allows this)
	 *
	 * */
	static void error(std::string& category, std::string errMessage,
			int errValue);
};

#endif /* SRC_UTILITY_LOGGER_H_ */
