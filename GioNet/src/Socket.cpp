#include "Socket.h"
#include <cstdio>

#include "Peer.h"

GioNet::Socket::Socket(SOCKET socket, addrinfo* addrInfo)
    : windowsSocket(socket), addressInfo(addrInfo)
{
}

GioNet::Socket::~Socket()
{
    if(windowsSocket != INVALID_SOCKET)
    {
        Close();
    }
}

bool GioNet::Socket::IsValid() const
{
    return windowsSocket != INVALID_SOCKET;
}

int GioNet::Socket::Send(const char* buffer, int len)
{
    return SendTo(windowsSocket, buffer, len);
}

int GioNet::Socket::SendTo(SOCKET socket, const char* buffer, int len)
{
    int res = send(socket, buffer, len, 0);

    if(res == SOCKET_ERROR)
    {
        WINSOCK_REPORT_ERROR();
        return SOCKET_ERROR;
    }

    return res;
}

int GioNet::Socket::ReceiveFrom(SOCKET socket, char* buffer, int len)
{
    int result = recv(socket, buffer, len, 0);

    if(result == SOCKET_ERROR)
    {
        WINSOCK_REPORT_ERROR();
        return SOCKET_ERROR;
    }

    return result;
}

int GioNet::Socket::Receive(char* buffer, int len)
{
    return ReceiveFrom(windowsSocket, buffer, len);
}

bool GioNet::Socket::Bind()
{
    int errorCode = bind(windowsSocket, addressInfo->ai_addr, static_cast<int>(addressInfo->ai_addrlen));

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
    if ( listen( windowsSocket, SOMAXCONN ) == SOCKET_ERROR ) {
        WINSOCK_REPORT_ERROR();
        Close();
        return false;
    }

    return true;
}

GioNet::Peer GioNet::Socket::Accept()
{
    sockaddr_in addr{};
    ZeroMemory(&addr, sizeof(sockaddr_in));
    int size = sizeof(addr);
    SOCKET ClientSocket = accept(windowsSocket,reinterpret_cast<sockaddr*>(&addr), &size);
    
    if (ClientSocket == INVALID_SOCKET)
    {
        WINSOCK_REPORT_ERROR();
        return {};
    }

    char buff[16];
    inet_ntop(addr.sin_family, addr.sin_zero, &buff[0], sizeof(buff));

    return {ClientSocket, {buff}, addr.sin_port};
}

bool GioNet::Socket::Connect()
{
    int result = connect( windowsSocket, addressInfo->ai_addr, static_cast<int>(addressInfo->ai_addrlen));
    if (result == SOCKET_ERROR)
    {
        WINSOCK_REPORT_ERROR();
        return false;
    }

    FreeAddressInfo();

    if (windowsSocket == INVALID_SOCKET) {
        return false;
    }

    return true;
}

void GioNet::Socket::Close()
{
    FreeAddressInfo();

    if(windowsSocket != INVALID_SOCKET)
    {
        closesocket(windowsSocket);
        windowsSocket = INVALID_SOCKET;
    }
}

void GioNet::Socket::FreeAddressInfo()
{
    if(addressInfo != nullptr)
    {
        freeaddrinfo(addressInfo);
        addressInfo = nullptr;
    }
}
