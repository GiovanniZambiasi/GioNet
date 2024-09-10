#pragma once
#include <memory>
#include <optional>
#include <string>

#include "Core.h"

namespace GioNet
{
    struct Peer;
    class Buffer;
    
    class Socket
    {
        NetAddress address{};

        CommunicationProtocols communicationProtocol{};
        
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

        CommunicationProtocols GetProtocol() const { return communicationProtocol; }

        const NetAddress& GetAddress() const { return address; }
        
        bool IsValid() const;

        std::string ToString() const;

        std::optional<int> Send(const Buffer& buffer);

        std::optional<int> SendTo(const Buffer& buffer, std::optional<NetAddress> destination = {});

        std::optional<Buffer> Receive();

        std::optional<Buffer> ReceiveFrom(NetAddress* outFrom = nullptr);
    
        bool Bind();

        bool Listen();

        std::shared_ptr<Socket> AcceptConnection();

        bool Connect();

        void Close();

    private:
        void FreeAddressInfo();
        
    };
}
