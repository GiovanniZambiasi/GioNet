#pragma once
#include <memory>
#include <optional>
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

        CommunicationProtocols GetProtocol() const { return protocol; }

        const NetAddress& GetAddress() const { return address; }
        
        bool IsValid() const;

        std::string ToString() const;

        int Send(const char* buffer, int len, std::optional<NetAddress> destination = {});

        int Receive(char* buffer, int len, NetAddress* outFrom = nullptr);
    
        // SERVER
        bool Bind();

        bool Listen();

        std::shared_ptr<Socket> AcceptConnection();

        //CLIENT
        bool Connect();

        void Close();

    private:
        void FreeAddressInfo();
        
    };
}
