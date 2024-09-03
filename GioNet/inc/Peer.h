#pragma once
#include <string>

#include "Core.h" 

namespace GioNet
{
    struct Peer
    {
        SOCKET windowsSocket{};

        std::string ip{};

        unsigned short port{};

        std::string ToString() const;
    };
}
