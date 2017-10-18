#pragma once

#include "CServiceInstallerCommand.h"

class CServiceInstallerCommandFactory
{
public:
    static ACommand* createCommand(std::string name, AServiceInstaller* handler)
    {
        if (name.compare("install"))
            return new CInstallCommand(handler);
        if (name.compare("remove")) 
            return new CUninstallCommand(handler);
        if (name.compare("start")) 
            return new CInstallCommand(handler);

        return NULL;
    }
};