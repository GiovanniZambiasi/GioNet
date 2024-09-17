#pragma once
#include <cassert>
#include <memory>
#include <thread>

#include "Buffer.h"
#include "Socket.h"

namespace GioNet
{
    struct NetAddress;
    class Socket;

    class Client
    {
    public:
        using DataReceivedDelegate = std::function<void(Buffer&&)>;
        
    private:
        NetAddress serverAddress{};
        
        std::shared_ptr<Socket> socket{};

        std::jthread listenThread{};

        DataReceivedDelegate dataReceived{};
        
    public:
        Client(const NetAddress& serverAddress);
        
        virtual ~Client();
        
        virtual void Start();

        void Stop();

        void Send(const Buffer& buffer);
        
        bool IsConnected() const;

        Socket& GetSocketChecked()
        {
            assert(socket && socket->IsValid());
            return *socket;
        }

        void BindDataReceived(DataReceivedDelegate&& delegate);

        std::shared_ptr<Socket> GetSocket(){ return socket;}
        
    private:
        void RunListenThread();

        void InvokeDataReceived(Buffer&& buffer);
        
        void ListenThreadImpl();
        
    };    
}
