#pragma once
#include <memory>
#include <thread>

namespace GioNet
{
    class Socket;

    class Client
    {
        std::shared_ptr<Socket> socket{};

        std::thread listenThread{};
        
    public:
        Client() = default;

        Client(const std::shared_ptr<Socket>& socket);

        ~Client();
        
        void SayHello();
        
        void Connect();

    private:
        void ReceiveLoop();
        
    };    
}
