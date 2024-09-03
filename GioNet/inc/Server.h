#pragma once
#include <memory>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include "Peer.h"

namespace GioNet
{
    class Socket;
    struct Peer;

    class Server
    {
        std::shared_ptr<Socket> listenSocket{};

        std::unordered_map<Peer, std::thread> connectionThreads{};

        std::thread listenThread{};
        
        std::shared_mutex connectionMutex{};

    public:
        Server() = default;
        
        Server(const std::shared_ptr<Socket>& listenSocket);

        ~Server();
        
        void Listen();

        void SendToPeer(const Peer& peer, const char* buffer, int len);

    private:
        void AddPeer(const Peer& peer);

        void RemovePeer(const Peer& peer);

        void ConnectionLoop();

        void ReceiveLoop(const Peer& peer);
    };
}
