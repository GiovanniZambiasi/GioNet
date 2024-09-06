#pragma once
#include <cassert>
#include <memory>
#include <thread>

#include "Socket.h"

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
        virtual ~Client();
        
        void SayHello();
        
        virtual void Start();

        void Stop();
        
        bool IsConnected() const;

        Socket& GetSocketChecked()
        {
            assert(socket && socket->IsValid());
            return *socket;
        }

        std::shared_ptr<Socket> GetSocket(){ return socket;}
        
    protected:
        Client(const std::shared_ptr<Socket>& socket);

        void RunListenThread();
        
    private:
        void ListenThreadImpl();
        
    };    
}
