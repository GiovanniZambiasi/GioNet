#include "Socket.h"

#include <cassert>
#include <cstdio>
#include <format>

#include "Buffer.h"

namespace
{
    int GioNetAddressFamily = AF_INET;
}

GioNet::Socket::Socket(const NetAddress& address, CommunicationProtocols protocol)
    : address(address), protocol(protocol)
{
    addrinfo info{};
    ZeroMemory(&info, sizeof(addrinfo));
    info.ai_family = GioNetAddressFamily;

    switch (protocol)
    {
    case CommunicationProtocols::UDP:
        info.ai_socktype = SOCK_DGRAM;
        info.ai_protocol  = IPPROTO_UDP;
        break;
    default:
        info.ai_socktype = SOCK_STREAM;
        info.ai_protocol = IPPROTO_TCP;
        break;
    }

    if(address.IsServer())
    {
        info.ai_flags = AI_PASSIVE;
    }

    std::string portString = std::to_string(address.port);
    WINSOCK_CALL_AND_REPORT(getaddrinfo(address.IsServer() ? (PCSTR)INADDR_ANY : address.ip.c_str(), portString.c_str(), &info, &winAddrInfo))

    if(success)
    {
        winSocket = socket(winAddrInfo->ai_family, winAddrInfo->ai_socktype, winAddrInfo->ai_protocol);

        if(winSocket == INVALID_SOCKET)
        {
            Close();
        }
    }
}

GioNet::Socket::~Socket()
{
    if(winSocket != INVALID_SOCKET)
    {
        Close();
    }
}

bool GioNet::Socket::IsValid() const
{
    return winSocket != INVALID_SOCKET;
}

std::string GioNet::Socket::ToString() const
{
    return std::format("(SOCKET:{} IP:{}:{})", winSocket, address.ip, address.port);
}

int GioNet::Socket::Send(const Buffer& buffer, std::optional<NetAddress> destination)
{
    int res{};
    switch (protocol)
    {
    case CommunicationProtocols::TCP:
        if(destination.has_value() && *destination != address)
        {
            printf("[ERROR]: TCP socket cannot send to arbitrary address. Can only send to connected sockets.\n");
            return SOCKET_ERROR;
        }
        
        res = send(winSocket, buffer.Data(), buffer.Length(), 0);

        if(res == SOCKET_ERROR)
        {
            WINSOCK_REPORT_ERROR();
            return SOCKET_ERROR;
        }

        return res;
    case CommunicationProtocols::UDP:
        if(destination.has_value())
        {
            sockaddr_in windowsSockAddr{};
            windowsSockAddr.sin_family = GioNetAddressFamily;
            windowsSockAddr.sin_port = destination->port;
            in_addr windowsAddr{};
            inet_pton(GioNetAddressFamily, destination->ip.c_str(), &windowsAddr);
            windowsSockAddr.sin_addr = windowsAddr;
            res = sendto(winSocket, buffer.Data(), buffer.Length(), 0, reinterpret_cast<sockaddr*>(&windowsSockAddr), sizeof(windowsSockAddr));
        }
        // For clients communicating with the server (no destination is required)
        else
        {
            assert(winAddrInfo);
            res = sendto(winSocket, buffer.Data(), buffer.Length(), 0, winAddrInfo->ai_addr, sizeof(sockaddr));
        }

        if(res == SOCKET_ERROR)
        {
            WINSOCK_REPORT_ERROR();
            return SOCKET_ERROR;
        }
        
        return res;
    default:
        printf("Unimplemented protocol...\n");
        return -1;
    }
}

std::optional<GioNet::Buffer> GioNet::Socket::Receive(NetAddress* outFrom)
{
    int receivedBytes;
    
    switch (protocol)
    {
    case CommunicationProtocols::TCP:
        char received[GIONET_BUFFER_MAX];
        receivedBytes = recv(winSocket, &received[0], sizeof(received), 0);

        if(receivedBytes == SOCKET_ERROR)
        {
            WINSOCK_REPORT_ERROR();
            return {};
        }

        if(outFrom)
        {
            *outFrom = address;
        }
        
        return {Buffer{&received[0], receivedBytes}};
    case CommunicationProtocols::UDP:
        sockaddr_in from{};
        int fromSize = sizeof(sockaddr_in);

        char outBuffer[GIONET_BUFFER_MAX];
        receivedBytes = recvfrom(winSocket, &outBuffer[0], sizeof(outBuffer), 0, reinterpret_cast<sockaddr*>(&from), &fromSize);

        if(receivedBytes == SOCKET_ERROR)
        {
            WINSOCK_REPORT_ERROR();
            return {};
        }

        if(outFrom)
        {
            char buff[25];
            inet_ntop(AF_INET, &from.sin_addr, &buff[0], sizeof(buff));
            (*outFrom) = { {buff}, from.sin_port};
        }

        return {Buffer{&outBuffer[0], receivedBytes}};
    }

    printf("Unimplemented protocol...\n");
    return {};
}

bool GioNet::Socket::Bind()
{
    int errorCode = bind(winSocket, winAddrInfo->ai_addr, static_cast<int>(winAddrInfo->ai_addrlen));

    if (errorCode == SOCKET_ERROR)
    {
        WINSOCK_REPORT_ERROR();
        Close();
        return false;
    }
    
    FreeAddressInfo();

    return true;
}

bool GioNet::Socket::Listen()
{
    assert(protocol == CommunicationProtocols::TCP);
    if ( listen( winSocket, SOMAXCONN ) == SOCKET_ERROR ) {
        WINSOCK_REPORT_ERROR();
        Close();
        return false;
    }

    return true;
}

std::shared_ptr<GioNet::Socket> GioNet::Socket::AcceptConnection()
{
    assert(protocol == CommunicationProtocols::TCP);
    sockaddr_in addr{};
    ZeroMemory(&addr, sizeof(sockaddr_in));
    int size = sizeof(addr);
    SOCKET clientSocket = accept(winSocket,reinterpret_cast<sockaddr*>(&addr), &size);
    
    if (clientSocket == INVALID_SOCKET)
    {
        WINSOCK_REPORT_ERROR();
        return {};
    }

    char buff[16];
    inet_ntop(addr.sin_family, addr.sin_zero, &buff[0], sizeof(buff));

    std::shared_ptr<Socket> connectionSocket = std::make_shared<Socket>();
    connectionSocket->winSocket = clientSocket;
    return connectionSocket;
}

bool GioNet::Socket::Connect()
{
    assert(protocol == CommunicationProtocols::TCP);
    int result = connect( winSocket, winAddrInfo->ai_addr, static_cast<int>(winAddrInfo->ai_addrlen));
    if (result == SOCKET_ERROR)
    {
        WINSOCK_REPORT_ERROR();
        return false;
    }

    FreeAddressInfo();

    if (winSocket == INVALID_SOCKET) {
        return false;
    }

    return true;
}

void GioNet::Socket::Close()
{
    FreeAddressInfo();

    if(winSocket != INVALID_SOCKET)
    {
        closesocket(winSocket);
        winSocket = INVALID_SOCKET;
    }
}

void GioNet::Socket::FreeAddressInfo()
{
    if(winAddrInfo != nullptr)
    {
        freeaddrinfo(winAddrInfo);
        winAddrInfo = nullptr;
    }
}
