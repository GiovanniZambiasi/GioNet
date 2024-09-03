#pragma once
#include <memory>

namespace GioNet
{
    class Socket;

    class Server
    {
        std::shared_ptr<Socket> listenSocket{};
        
    public:
        Server() = default;
        
        Server(const std::shared_ptr<Socket>& listenSocket);
        
        void Listen();
    
    };
}
