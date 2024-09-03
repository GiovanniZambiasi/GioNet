#pragma once
#include <string>

#include "Core.h" 

namespace GioNet
{
    struct Peer
    {
        SOCKET windowsSocket{INVALID_SOCKET};

        std::string ip{};

        unsigned short port{};

        std::string ToString() const;

        bool IsValid() const;

        bool operator==(const Peer& other) const;
    };


}

template<>
struct std::hash<GioNet::Peer>
{
    size_t operator()(const GioNet::Peer& k) const noexcept
    {
        return std::hash<SOCKET>{}(k.windowsSocket);
    }
};
