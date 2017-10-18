/****************************** Module Header ******************************\
* Module Name:  SampleService.h
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once
#ifdef _MSC_VER
#include "ServiceBase.h"


class CSampleService : public CServiceBase
{
public:

    CSampleService(char* pszServiceName, 
        int fCanStop = 1, 
                   int fCanShutdown = 1,
                   int fCanPauseContinue = 0);
    virtual ~CSampleService(void);

protected:

    virtual void OnStart(unsigned long dwArgc, char** pszArgv);
    virtual void OnStop();
    
private:

    int m_fStopping;
    void* m_hStoppedEvent;
};
#endif