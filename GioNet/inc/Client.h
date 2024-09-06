#pragma once
#include <cassert>
#include <memory>
#include <thread>

#include "Buffer.h"
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
        
        virtual void Start();

        void Stop();

        virtual void Send(const Buffer& buffer) = 0;
        
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

        virtual std::optional<Buffer> DoReceive() = 0;
        
    private:
        void ListenThreadImpl();
        
    };    
}
