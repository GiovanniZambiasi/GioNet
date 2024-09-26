#pragma once
#include <cassert>
#include <memory>
#include <thread>

#include "Buffer.h"
#include "Connection.h"
#include "Socket.h"

namespace GioNet
{
    struct Message;
    struct NetAddress;
    class Socket;

    class Client
    {
    public:
        using DataReceivedDelegate = std::function<void(Buffer&&)>;
        
    private:
        NetAddress serverAddress{};
        
        std::shared_ptr<Socket> socket{};

        std::shared_ptr<Connection> connectionToServer;

        std::jthread listenThread{};
        
        std::jthread sendThread{};

        DataReceivedDelegate dataReceived{};
        
    public:
        Client(const NetAddress& serverAddress);
        
        virtual ~Client();
        
        virtual void Start();

        void Stop();

        void Send(Message&& message);
        
        bool IsConnected() const;

        Socket& GetSocketChecked()
        {
            assert(socket && socket->IsValid());
            return *socket;
        }

        void BindDataReceived(DataReceivedDelegate&& delegate);

        std::shared_ptr<Socket> GetSocket(){ return socket;}
        
    private:
        void ListenThread();
        
        void ProcessIncomingPacket();
        
        void SendThread();

        void InvokeDataReceived(Buffer&& buffer);
    };    
}
