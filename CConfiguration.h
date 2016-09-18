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

#ifndef CCONFIGURATION_H_
#define CCONFIGURATION_H_

#include "common.h"

class CConfiguration
{
	private:
		static CConfiguration* instance;
		static struct configuration* config;
		CConfiguration();
		CConfiguration(CConfiguration&);
		void operator = (CConfiguration const&);
		static void parseCommandLine(int argc, char** argv);
	public:
		~CConfiguration();
		static void configure(int argc, char** argv);
		static CConfiguration* getInstance();
		static void dump();

		/*
		 * Methods that return configuration data
		 */
		ushort getThreads() { return config->threads; }
		ushort getQueues() { return config->queues; }
		char* getInterface() { return config->interface; }
		char *getDatabasePath() { return config->databasePath; }
		int getLinkType() { return config->linkType; }
		void setLinkType(int linkType);
		int getPacketHeaderOffset() {return config->headerOffset; }
};

CConfiguration* CConfiguration::instance = NULL;
configuration* CConfiguration::config = NULL;

CConfiguration::CConfiguration() {
	config = new configuration;
	memset(config, 0, sizeof(configuration));
}

CConfiguration::~CConfiguration() {
	if (config != NULL) delete config;
}

void CConfiguration::configure(int argc, char** argv)
{
	if (instance == NULL) {
		instance = new CConfiguration();
		parseCommandLine(argc, argv);
	}
}

void CConfiguration::parseCommandLine(int argc, char** argv)
{
	char argname;

	while ((argname = getopt(argc, argv, "t:q:i:d:")) != -1)
	{
		cout << "Handling input argument " << argname << endl;

		switch (argname) {
			case 't':
				config->threads = atoi(optarg);
				cout << "Read value '" << atoi(optarg) << "' for " << argname << endl;
			break;
			case 'q':
				config->queues = atoi(optarg);
				cout << "Read value '" << atoi(optarg) << "' for " << argname << endl;
			break;
			case 'i':
				strcpy(config->interface, optarg);
				cout << "Read value '" << optarg << "' for " << argname << endl;
			break;
			case 'd':
				strcpy(config->databasePath, optarg);
				cout << "Read value '" << optarg << "' for " << argname << endl;
			break;
			default:
				fprintf(stderr, "Unsupported option '%c' with value '%s'!\n", argname, optarg);
				exit(1);
			break;
		}
	}

	if (config->threads <= 0) {
		cout << "Threads are not configured. Use -t option." << endl;
		exit(-1);
	}

	if (config->queues <= 0) {
		cout << "Queues are not configured. Use -q option." << endl;
		fprintf(stderr, ".\n");
		exit(-1);
	}

	if (strlen(config->interface) == 0) {
		cout << "Interface name is not specified or is invalid. Use -i option" << endl;
		exit(-1);
	}

	if (strlen(config->databasePath) == 0) {
		cout << "Unspecified database directory" << endl;
		exit(-1);
	} else {
		char *path = config->databasePath;

		if (path[strlen(path)-1] != '/' &&
				strlen(path) < sizeof(config->databasePath)-1) // checking if we can write to the end of the buffer
			sprintf(config->databasePath, "%s/", config->databasePath);

		if (access(path, W_OK) != 0) {
			cout << "Database path '" << path << "' doesn't exist or not writable" << endl;
			exit(1);
		}
	}
}

CConfiguration* CConfiguration::getInstance() {
	return instance;
}

void CConfiguration::dump()
{
	if (config == NULL)
		return;

	cout << "CConfiguration::dump(): dumping run configuration" << endl;
	cout << " worker threads: " << config->threads << endl;
	cout << " queues: " << config->queues << endl;
	cout << " listen interface: " << config->interface << endl;
	cout << " database path: " << config->databasePath << endl;
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
			cout << "Unsupported datalink '" << config->linkType << "'" << endl;
			exit(1);
	}
}

#endif /* CCONFIGURATION_H_ */
