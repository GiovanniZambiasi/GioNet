#include "Logger.h"

#include <cassert>

#include "Core.h"

std::unique_ptr<GioNet::Logger> GioNet::Logger::instance{};

GioNet::Logger::Logger()
{
    char loggingDisabled[2] = {'0', '\n'};
    size_t size = sizeof(loggingDisabled);
    auto err = getenv_s(&size, &loggingDisabled[0], sizeof(loggingDisabled), GIONET_DISABLE_LOGGING_ENV_VAR);

    if(err)
    {
        printf("[ERROR]: Failed to retrieve logging variable. Use only 0 or 1\n", loggingDisabled);
    }
    else if(size > 0 && loggingDisabled[0] != '0')
    {
        enabled = false;
    }
}

GioNet::Logger& GioNet::Logger::Get()
{
    if(!instance)
    {
        instance = std::unique_ptr<Logger>{new Logger{}};
    }
    
    return *instance;
}
