#pragma once
#include <memory>

namespace GioNet
{
    class Socket;

    class Client
    {
        std::shared_ptr<Socket> socket{};
        
    public:
        bool Connect(std::shared_ptr<Socket> connectionSocket);
    };    
}
