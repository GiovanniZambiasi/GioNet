#pragma once
#include <memory>

namespace GioNet
{
    class Socket;

    class Server
    {
        std::shared_ptr<Socket> listenSocket{};
        
    public:
        void BindSocket(std::shared_ptr<Socket> socket);
        
        void Listen();
    
    };
}
