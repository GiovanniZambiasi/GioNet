#pragma once
#include "Core.h"

namespace GioNet
{
    class Socket;

    struct Peer
    {
        NetAddress address{};

        std::shared_ptr<Socket> connection{};
        
        std::string ToString() const;

        bool operator==(const Peer& other) const = default;
    };
}

template<>
struct std::hash<GioNet::Peer>
{
    std::size_t operator()(const GioNet::Peer& peer) const noexcept;
};