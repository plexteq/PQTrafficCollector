#ifdef _MSC_VER

#include "Win32ServiceInstaller.h"

// Displayed name of the service
#define SERVICE_DISPLAY_NAME     "PQTrafficCollectorService"

// Service start options.
#define SERVICE_START_TYPE       SERVICE_DEMAND_START

// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     ""

// The name of the account under which the service should run
#define SERVICE_ACCOUNT          NULL

// The password to the service account name
#define SERVICE_PASSWORD         NULL

#include <iostream>
#include "../../win32/win_headers.h"

#include "../../utility/FileHelper.h"
#include "../../utility/Logger.h"

char* AServiceInstaller::ServiceName = "PQTrafficCollector";

void Win32ServiceInstaller::Cleanup(void* schSCManager, void* schService)
{
    if (schSCManager)
    {
        CloseServiceHandle((SC_HANDLE)schSCManager);
    }
    if (schService)
    {
        CloseServiceHandle((SC_HANDLE)schService);
    }
}
//
//   FUNCTION: InstallService
//
//   PURPOSE: Install the current application as a service to the local
//   service control manager database.
//
//   PARAMETERS:
//   * pszServiceName - the name of the service to be installed
//   * pszDisplayName - the display name of the service
//   * dwStartType - the service start option. This parameter can be one of
//     the following values: SERVICE_AUTO_START, SERVICE_BOOT_START,
//     SERVICE_DEMAND_START, SERVICE_DISABLED, SERVICE_SYSTEM_START.
//   * pszDependencies - a pointer to a double null-terminated array of null-
//     separated names of services or load ordering groups that the system
//     must start before this service.
//   * pszAccount - the name of the account under which the service runs.
//   * pszPassword - the password to the account name.
//
//   NOTE: If the function fails to install the service, it prints the error
//   in the standard output stream for users to diagnose the problem.
//
void Win32ServiceInstaller::InstallService()
//                    str_t pszServiceName,
//                    str_t pszDisplayName,
//                    DWORD dwStartType,
//                    str_t pszDependencies,
//                    str_t pszAccount,
//                    str_t pszPassword)
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;

    // Open the local default service control manager database
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |
                                 SC_MANAGER_CREATE_SERVICE);
    if (schSCManager == NULL)
    {
        Logger::error(className, "OpenSCManager failed ", GetLastError());
        return;
    }

    // Install the service into SCM by calling CreateService
    schService = CreateService(
        schSCManager,// SCManager database
        AServiceInstaller::ServiceName,// Name of service
        SERVICE_DISPLAY_NAME,// Name to display
        SERVICE_QUERY_STATUS,// Desired access
        SERVICE_WIN32_OWN_PROCESS,// Service type
        SERVICE_START_TYPE,// Service start type
        SERVICE_ERROR_NORMAL,// Error control type
        FileHelper::getCurrentExePath().c_str(),// Service's binary
        NULL,// No load ordering group
        NULL,// No tag identifier
        SERVICE_DEPENDENCIES,// Dependencies
        SERVICE_ACCOUNT,// Service running account
        SERVICE_PASSWORD// Password of the account
    );
    if (schService == NULL)
    {
        Logger::error(className, "CreateService failed ", GetLastError());
    }
    else
    {
        Logger::info(className, std::string(AServiceInstaller::ServiceName) + " is installed");
    }

    Cleanup(schSCManager, schService);
}

void Win32ServiceInstaller::_StartService()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssSvcStatus =
    {};

    // Open the local default service control manager database
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == NULL)
    {
        Logger::error(className, "OpenSCManager failed ", GetLastError());
        return;
    }

    schService = OpenService(
        schSCManager,         // SCM database
        AServiceInstaller::ServiceName,// name of service
        SERVICE_ALL_ACCESS);// full access

    if (schService == NULL)
    {
        Logger::error(className, "OpenService failed ", GetLastError());
        Cleanup(schSCManager, schService);
        return;
    }

    if (!StartService(
        schService,  // handle to service
        0,// number of arguments
        NULL))// no arguments
    {
        Logger::error(className, "StartService failed ", GetLastError());
    }

    Cleanup(schSCManager, schService);
}
//
//   FUNCTION: UninstallService
//
//   PURPOSE: Stop and remove the service from the local service control
//   manager database.
//
//   PARAMETERS:
//   * pszServiceName - the name of the service to be removed.
//
//   NOTE: If the function fails to uninstall the service, it prints the
//   error in the standard output stream for users to diagnose the problem.
//
void Win32ServiceInstaller::UninstallService()
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssSvcStatus =
    {};

    // Open the local default service control manager database
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == NULL)
    {
        Logger::error(className, "OpenSCManager failed ", GetLastError());
        return;
    }

    // Open the service with delete, stop, and query status permissions
    schService = OpenService(schSCManager, AServiceInstaller::ServiceName, SERVICE_STOP |
                             SERVICE_QUERY_STATUS | DELETE);
    if (schService == NULL)
    {
        Logger::error(className, "OpenService failed ", GetLastError());
        Cleanup(schSCManager, schService);
        return;
    }

    // Try to stop the service
    if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
    {
        Logger::info(className, std::string("Stopping ") + AServiceInstaller::ServiceName);
        Sleep(1000);

        while (QueryServiceStatus(schService, &ssSvcStatus))
        {
            if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING)
            {
                Sleep(1000);
            }
            else break;
        }

        if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED)
        {
            Logger::info(className, std::string(AServiceInstaller::ServiceName) + std::string(" is stopped"));
        }
        else
        {
            Logger::info(className, std::string(AServiceInstaller::ServiceName) + " failed to stop");
        }
    }

    // Now remove the service by calling DeleteService.
    if (DeleteService(schService))
    {
        Logger::info(className, std::string(AServiceInstaller::ServiceName) + " is removed");
    }
    else
    {
        Logger::error(className, "DeleteService failed ", GetLastError());
    }

    Cleanup(schSCManager, schService);
}
std::string Win32ServiceInstaller::className = "Win32ServiceInstaller";
#endif