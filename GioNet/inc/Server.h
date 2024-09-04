#pragma once
#include <memory>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

#include "Core.h"

namespace GioNet
{
    enum class CommunicationProtocols;
    class Socket;

    struct Peer
    {
        NetAddress address{};
        
        std::shared_ptr<Socket> connection{};
    };

    class Server
    {
        std::shared_ptr<Socket> listenSocket{};

        std::unordered_map<std::shared_ptr<Socket>, std::thread> receiveThreads{};

        std::thread listenThread{};
        
        std::shared_mutex connectionMutex{};

    public:
        Server() = default;

        Server(unsigned short port, CommunicationProtocols protocol);
        
        ~Server();
        
        void Start();

        void SendToPeer(const std::shared_ptr<Socket>& peer, const char* buffer, int len);

    private:
        void AddPeer(const std::shared_ptr<Socket>& peer);

        void RemovePeer(const std::shared_ptr<Socket>& peer);

        void ConnectionLoop();

        void ReceiveLoop(const std::shared_ptr<Socket>& peer);
    };
}
