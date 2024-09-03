#pragma once
#include <memory>
#include <thread>
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

        std::vector<std::thread> threads{};

    public:
        Server() = default;
        
        Server(const std::shared_ptr<Socket>& listenSocket);
        
        void Listen();

    private:
        void AddConnectedPeer(const Peer& peer);

        void ConnectionLoop();

        void ReceiveLoop(const Peer& peer);
    };
}
