#pragma once
#include "Server.h"

namespace GioNet
{
    class ServerTCP : public Server
    {
        std::thread connectionThread{};

        std::unordered_map<NetAddress, std::thread> receiveThreads{};
        
    public:
        ServerTCP(unsigned short port);
        
        ~ServerTCP() override;
        
        void Start() override;

    private:
        void RunConnectionThread();

        void OnPostPeerAdded(const Peer& peer) override;

        void OnPrePeerRemoved(const Peer& peer) override;

        void RunReceiveThreadForPeer(const Peer& peer);

        std::optional<int> DoSend(const Buffer& buffer, const Peer& peer) override;
    };
}
