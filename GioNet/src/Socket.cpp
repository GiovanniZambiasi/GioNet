#include "Socket.h"
#include <cstdio>
#include <format>

GioNet::Socket::Socket(const NetAddress& address, CommunicationProtocols protocol)
    : address(address), protocol(protocol)
{
    addrinfo info{};
    ZeroMemory(&info, sizeof(addrinfo));
    info.ai_family = AF_INET;

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
    return std::format("(SOCKET:{} IP:{}:{})", winSocket, address.ip.c_str(), address.port);
}

int GioNet::Socket::Send(const char* buffer, int len)
{
    int res = send(winSocket, buffer, len, 0);

    if(res == SOCKET_ERROR)
    {
        WINSOCK_REPORT_ERROR();
        return SOCKET_ERROR;
    }

    return res;
}

int GioNet::Socket::Receive(char* buffer, int len)
{
    int result = recv(winSocket, buffer, len, 0);

    if(result == SOCKET_ERROR)
    {
        WINSOCK_REPORT_ERROR();
        return SOCKET_ERROR;
    }

    return result;
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
