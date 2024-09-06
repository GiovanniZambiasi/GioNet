#pragma once
#include "Server.h"

namespace GioNet
{
    class ServerUDP : public Server
    {
        std::thread listenThread{};
        
    public:
        ServerUDP(unsigned short port);

        ~ServerUDP() override;

    private:
        void Start() override;

        void RunListenThread();
        
        std::optional<int> DoSend(const Buffer& buffer, const Peer& peer) override;
    };   
}
