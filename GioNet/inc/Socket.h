#pragma once
#include <memory>
#include <string>

#include "Core.h"

namespace GioNet
{
    struct Peer;
    
    class Socket
    {
        NetAddress address{};

        CommunicationProtocols protocol{};
        
        SOCKET winSocket{INVALID_SOCKET};

        addrinfo* winAddrInfo{nullptr};
        
    public:
        Socket() = default;
        
        Socket(const NetAddress& address, CommunicationProtocols protocol);

        ~Socket();

        GIONET_NOCOPY(Socket)
        
        // TODO - Does this work?
        Socket(Socket&& other) = default;
        Socket& operator=(Socket&& other) = default;

        bool IsValid() const;

        std::string ToString() const;

        int Send(const char* buffer, int len);

        int Receive(char* buffer, int len);
    
        // SERVER
        bool Bind();

        bool Listen();

        std::shared_ptr<Socket> Accept();

        //CLIENT
        bool Connect();

        void Close();

    private:
        void FreeAddressInfo();
        
    };
}
