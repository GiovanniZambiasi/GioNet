#pragma once
#include "Server.h"

namespace GioNet
{
    class ServerTCP : public Server
    {
        std::jthread connectionThread{};

        std::unordered_map<NetAddress, std::jthread> receiveThreads{};
        
    public:
        ServerTCP(unsigned short port);
        
        ~ServerTCP() override;
        
        void Start() override;

        void Stop() override;

    private:
        void RunConnectionThread();

        void OnPostPeerAdded(const Peer& peer) override;

        void OnPrePeerRemoved(const Peer& peer) override;

        void RunReceiveThreadForPeer(const Peer& peer, std::stop_token stop);

        std::optional<int> DoSend(const Buffer& buffer, const Peer& peer) override;
    };
}
