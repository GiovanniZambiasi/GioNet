#include "Socket.h"

#include <cassert>
#include <cstdio>
#include <format>

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
        
        if(address.IsServer())
        {
            info.ai_flags = AI_PASSIVE;
        }
        break;
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

int GioNet::Socket::Send(const char* buffer, int len, std::optional<NetAddress> destination)
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
        
        res = send(winSocket, buffer, len, 0);

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
            res = sendto(winSocket, buffer, len, 0, reinterpret_cast<sockaddr*>(&windowsSockAddr), sizeof(windowsSockAddr));
        }
        else
        {
            assert(winAddrInfo);
            res = sendto(winSocket, buffer, len, 0, winAddrInfo->ai_addr, sizeof(sockaddr));
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

int GioNet::Socket::Receive(char* buffer, int len, NetAddress* outFrom)
{
    int result{};
    
    switch (protocol)
    {
    case CommunicationProtocols::TCP:
        result = recv(winSocket, buffer, len, 0);

        if(result == SOCKET_ERROR)
        {
            WINSOCK_REPORT_ERROR();
            return SOCKET_ERROR;
        }

        if(outFrom)
        {
            *outFrom = address;
        }
        
        return result;
    case CommunicationProtocols::UDP:
        sockaddr_in from{};
        int size = sizeof(sockaddr_in);
        result = recvfrom(winSocket, buffer, len, 0, reinterpret_cast<sockaddr*>(&from), &size);

        if(result == SOCKET_ERROR)
        {
            WINSOCK_REPORT_ERROR();
            return SOCKET_ERROR;
        }

        if(outFrom)
        {
            char buff[25];
            inet_ntop(AF_INET, &from.sin_addr, &buff[0], sizeof(buff));
            (*outFrom) = { {buff}, from.sin_port};
        }

        return result;
    }

    printf("Unimplemented protocol...\n");
    return -1;
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
    
    // addrinfo no longer needed
    FreeAddressInfo();

    return true;
}

bool GioNet::Socket::Listen()
{
    if ( listen( winSocket, SOMAXCONN ) == SOCKET_ERROR ) {
        WINSOCK_REPORT_ERROR();
        Close();
        return false;
    }

    return true;
}

std::shared_ptr<GioNet::Socket> GioNet::Socket::Accept()
{
    // TODO - Only do this if TCP
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
