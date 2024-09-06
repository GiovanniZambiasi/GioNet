#pragma once
#include <memory>
#include <thread>

namespace GioNet
{
    enum class CommunicationProtocols;
    struct NetAddress;
    class Socket;

    class Client
    {
        std::shared_ptr<Socket> socket{};

        std::thread listenThread{};
        
    public:
        Client() = default;

        Client(const NetAddress& address, CommunicationProtocols protocol);
        
        ~Client();
        
        void SayHello();
        
        void Start();

        void Stop();
        
        bool IsConnected() const;

    private:
        void ReceiveLoop();
        
    };    
}
