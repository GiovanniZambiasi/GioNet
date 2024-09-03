#pragma once
#include <memory>

namespace GioNet
{
    class Socket;

    class Client
    {
        std::shared_ptr<Socket> socket{};
        
    public:
        Client() = default;

        Client(const std::shared_ptr<Socket>& socket);

        void SayHello();
        
    };    
}
