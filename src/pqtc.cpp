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

#include <iostream>
#include <fstream>  
#include <string.h>

#ifdef _MSC_VER
#include "win32\win_headers.h"
#endif

#include "PQTrafficCollector.h"


#ifdef _MSC_VER

#include "service/AServiceInstaller.h"
#include "service\commands\CServiceInstallerCommandFactory.h"
#include "service/win32/Win32ServiceInstaller.h"
#include "service/win32/ServiceBase.h"
#include "service/win32/SampleService.h"

using namespace std;

void runWindows32Installer(char* command, bool* result);
void startService();
void printUsage();
#endif

int main(int argc, char *argv[])
{
    bool installerRes = false;

#ifdef _MSC_VER
    if ((argc > 1) && (*argv[1] == (TCHAR)'/'))
    {
        installer = runWindows32Installer(argv[1] + 1, &installerRes);
    }
    else
    {
        printUsage();
        startService();
    }
#endif

    if (!installerRes)
    {
        PQTrafficCollector::InitAndStart(argc, argv);
        PQTrafficCollector::Wait();
    }

	return 0;
}

#ifdef _MSC_VER
void runWindows32Installer(char* command, bool* result)
{
    AServiceInstaller* installer = new Win32ServiceInstaller();

    ACommand* cmd = CServiceInstallerCommandFactory::createCommand(command, installer);

    if (cmd)
    {
        cmd->execute();

        delete cmd;

        *result = true;
    }

    delete installer;
}
void startService()
{   
    CSampleService service(AServiceInstaller::ServiceName);
    if (!CServiceBase::Run(service))
    {
        printf("Service failed to run w/err 0x%08lx\n", GetLastError());
    }
}

void printUsage()
{
    printf("Parameters:\n");
    printf(" -install  to install the service.\n");
    printf(" -remove   to remove the service.\n");
}
#endif
