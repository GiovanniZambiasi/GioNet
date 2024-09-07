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
    public:
        using DataReceivedDelegate = std::function<void(Buffer&&)>;
        
    private:
        std::shared_ptr<Socket> socket{};

        std::jthread listenThread{};

        DataReceivedDelegate dataReceived{};
        
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

        void BindDataReceived(DataReceivedDelegate&& delegate);

        std::shared_ptr<Socket> GetSocket(){ return socket;}
        
    protected:
        Client(const std::shared_ptr<Socket>& socket);

        void RunListenThread();

        virtual std::optional<Buffer> DoReceive() = 0;

        void InvokeDataReceived(Buffer&& buffer);
        
    private:
        void ListenThreadImpl();
        
    };    
}
