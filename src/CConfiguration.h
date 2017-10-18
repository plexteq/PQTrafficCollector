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
#include <iostream>
#include <fstream>
#include <string>
#include "utility/Logger.h"
#include <boost/lexical_cast.hpp>

#define CONFIG_ARGC 5

#define CONFIG_PATH "config.ini"

class CConfiguration
{
private:
	static std::string className;
    static CConfiguration* instance;
    static struct configuration* config;
    CConfiguration();
    CConfiguration(CConfiguration&);
    void operator = (CConfiguration const&);
    static void parseCommandLine(int argc, char** argv);
    static int parseConfigFile(char** argv);
#ifdef _MSC_VER
    static void updateCurrentDirectory(char* serviceName);
#endif
public:
    ~CConfiguration();
    static void configure(int argc, char** argv);
    static CConfiguration* getInstance();
    static void dump();

    /*
     * Methods that return configuration data
     */
    ushort getThreads()
    {
        return config->threads;
    }
    ushort getQueues()
    {
        return config->queues;
    }
    char* getInterface()
    {
        return config->_interface;
    }
    char *getDatabasePath()
    {
        return config->databasePath;
    }
    int getLinkType()
    {
        return config->linkType;
    }
    int getServerPort()
	{
		return config->serverPort;
	}
    void setLinkType(int linkType);
    int getPacketHeaderOffset()
    {
        return config->headerOffset;
    }
};

#endif /* CCONFIGURATION_H_ */
