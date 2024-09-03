#pragma once
#include <memory>
#include <vector>
#include "Peer.h"

namespace GioNet
{
    class Socket;
    struct Peer;

    class Server
    {
        std::shared_ptr<Socket> listenSocket{};

        std::vector<Peer> connectedPeers{};

    public:
        Server() = default;
        
        Server(const std::shared_ptr<Socket>& listenSocket);
        
        void Listen();

    private:
        void AddConnectedPeer(const Peer& peer);
    };
}
