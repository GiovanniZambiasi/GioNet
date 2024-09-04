#pragma once
#include <memory>
#include <thread>

namespace GioNet
{
    struct NetAddress;
    class Socket;

    class Client
    {
        std::shared_ptr<Socket> socket{};

        std::thread listenThread{};
        
    public:
        Client() = default;

        Client(const NetAddress& address);
        
        ~Client();
        
        void SayHello();
        
        void Connect();

        void Disconnect();

    private:
        void ReceiveLoop();
        
    };    
}
