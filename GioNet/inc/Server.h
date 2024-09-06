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
        
        std::string ToString() const;

        bool operator==(const Peer& other) const = default;
    };

    class Server
    {
        std::shared_ptr<Socket> listenSocket{};

        std::unordered_map<NetAddress, std::thread> receiveThreads{};

        std::vector<Peer> peers{};

        std::thread listenThread{};
        
        std::shared_mutex connectionMutex{};

    public:
        Server() = default;

        Server(unsigned short port, CommunicationProtocols protocol);

        GIONET_NOCOPY(Server)
        
        ~Server();
        
        void Start();

    private:
        void AddPeer(const Peer& peer);

        void RemovePeer(const Peer& peer);

        void ConnectionLoop();

        void ReceiveLoop(const std::shared_ptr<Socket>& socket);
    };
}
