/**
 * Copyright (c) 2014, Plexteq
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
#include "CConfiguration.h"

#include <pcap.h>

#ifdef _MSC_VER
#include <getopt.h>

#define W_OK 0
#endif

CConfiguration* CConfiguration::instance = NULL;
configuration* CConfiguration::config = NULL;
std::string CConfiguration::className;

CConfiguration::CConfiguration()
{
	className = string(__func__);
	config = new configuration;
	memset(config, 0, sizeof(configuration));
}

CConfiguration::~CConfiguration()
{
	delete config;
	config = NULL;
}
#ifdef _MSC_VER
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\ + serviceName - path to a real exe file when it run as a Windows Service
void CConfiguration::updateCurrentDirectory(char* serviceName)
{
	if (config != NULL)
	{
		char regPath[512];
		HKEY key = NULL;
		long ret = 0;
		DWORD bufLen = sizeof(config->currentDirectory);

		strcpy(regPath, "SYSTEM\\CurrentControlSet\\services\\");
		strcat(regPath, serviceName);

		ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath, 0, KEY_QUERY_VALUE, &key);
		if (ret == ERROR_SUCCESS)
		{
			ret = RegQueryValueExA(key, "ImagePath", 0, 0, (LPBYTE)config->currentDirectory, &bufLen);
			RegCloseKey(key);
		}
	}

	for (int i = strlen(config->currentDirectory); i > 0; i--)
	{
		if (config->currentDirectory[i] == '\\' || config->currentDirectory[i] == '/')
		{
			config->currentDirectory[i + 1] = '\0';
			break;
		}
	}
}
#endif
void CConfiguration::configure(int argc, char** argv)
{
	if (instance)
		return;

	char* pargs[CONFIG_ARGC];
	char args[CONFIG_ARGC][256];

	for (int i = 0; i < CONFIG_ARGC; i++)
	{
		pargs[i] = args[i];
	}

	instance = new CConfiguration();

#ifdef _MSC_VER
	updateCurrentDirectory(argv[0]);
#endif

	if (argc == 1)
	{
		argc = parseConfigFile(pargs);
		argv = pargs;
	}

	parseCommandLine(argc, argv);

}

int CConfiguration::parseConfigFile(char** argv)
{
	char fullPath[512];

#ifdef _MSC_VER
	strcpy(fullPath, config->currentDirectory);
#endif

	strcat(fullPath, CONFIG_PATH);

	std::ifstream config(fullPath, ios::in);
	string configStr;

	if (config.is_open())
	{
		std::getline(config, configStr);
	}

	int index = 0;
	int i = 1;

	for (; i < CONFIG_ARGC; i++)
	{
		int next = configStr.find(" ", index);

		int len = next - index + 1;
		if (next == -1)
			len = configStr.length() - index + 1;

		strcpy(argv[i], configStr.substr(index, len - 1).c_str());

		if (next == -1)
			break;
		index = next + 1;
	}

	return i + 1;
}

void CConfiguration::parseCommandLine(int argc, char** argv)
{
	char argname;

	while ((argname = getopt(argc, argv, "t:q:i:f:p:d:")) != -1)
	{
		switch (argname)
		{
		case 't':
			config->threads = atoi(optarg);
			Logger::info(className,
					string("Read value '") + optarg + string("' for ")
							+ argname);
			break;
		case 'q':
			config->queues = atoi(optarg);
			Logger::info(className,
					string("Read value '") + optarg + string("' for ")
							+ argname);
			break;
		case 'i':
			strcpy(config->_interface, optarg);
			Logger::info(className,
					string("Read value '") + optarg + string("' for ")
							+ argname);
			break;
		case 'f':
			strcpy(config->databasePath, optarg);
			Logger::info(className,
					string("Read value '") + optarg + string("' for ")
							+ argname);
			break;
		case 'p':
			config->serverPort = atoi(optarg);
			Logger::info(className,
					string("Read value '") + optarg + string("' for ")
							+ argname);
			break;
		case 'd':
			config->dbType = atoi(optarg);
			Logger::info(className,
					string("Read value '") + optarg + string("' for ")
							+ argname);
			break;
		default:
			Logger::error(className,
					string("Unsupported option '") + argname
							+ string("' with value '") + optarg + string("'!"));
			exit(1);
			break;
		}
	}

	if (config->threads <= 0)
	{
		Logger::error(className, "Threads are not configured. Use -t option.");
		exit(-1);
	}

	if (config->queues <= 0)
	{
		Logger::error(className, "Queues are not configured. Use -q option.");
		exit(-1);
	}

	if (strlen(config->_interface) == 0)
	{
		Logger::error(className,
				"Interface name is not specified or is invalid. Use -i option");
		exit(-1);
	}

	if (config->serverPort <= 0 || config->serverPort > 65535)
	{
		Logger::error(className,
				"The server port is not configured. Use -p option.");
		exit(-1);
	}

	if (config->dbType != USE_MONGODB && config->dbType != USE_SQLITE)
	{
		config->dbType = USE_SQLITE;
	}

	if (strlen(config->databasePath) == 0)
	{
		Logger::error(className, "Unspecified database directory");
		exit(-1);
	}
	else
	{
		char *path = config->databasePath;

		if (path[strlen(path) - 1] != '/'
				&& strlen(path) < sizeof(config->databasePath) - 1) // checking if we can write to the end of the buffer
			sprintf(config->databasePath, "%s/", config->databasePath);

#ifdef _MSC_VER
		if (strchr(path, ':') == NULL)
		{
			char full_path[512];
			strcpy(full_path, config->currentDirectory);

			strcat(full_path, path);

			strcpy(config->databasePath, full_path);
		}

		DWORD dwAttr = GetFileAttributesA(config->databasePath);
		if (!(dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)))
		{
			Logger::error(className,string("Database path '") + path + string("' doesn't exist or not writable"));
			exit(1);
		}
#else
		if (config->dbType == USE_SQLITE && access(path, W_OK) != 0)
		{
			Logger::error(className,
					string("Database path '") + path
							+ string("' doesn't exist or not writable"));
			exit(1);
		}
#endif
	}
}

CConfiguration* CConfiguration::getInstance()
{
	return instance;
}

void CConfiguration::dump()
{
	if (config == NULL)
		return;

	Logger::info(className,
			"CConfiguration::dump(): dumping run configuration");
	Logger::info(className,
			" worker threads: "
					+ boost::lexical_cast<std::string>(config->threads));
	Logger::info(className,
			" queues: " + boost::lexical_cast<std::string>(config->queues));
	Logger::info(className, " listen interface: " + string(config->_interface));
	Logger::info(className, " database path: " + string(config->databasePath));
}

void CConfiguration::setLinkType(int linkType)
{
	config->linkType = linkType;

	// Set the datalink layer header size.
	switch (linkType)
	{
	case DLT_NULL:
		config->headerOffset = 4;
		break;
	case DLT_EN10MB:
		config->headerOffset = 14;
		break;
	case DLT_SLIP:
	case DLT_PPP:
		config->headerOffset = 24;
		break;
	default:
		Logger::error(className,
				string("Unsupported datalink '")
						+ boost::lexical_cast<std::string>(config->linkType)
						+ "'");

		exit(1);
	}
}

