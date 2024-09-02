#include "Socket.h"

#include <cstdio>

GioNet::Socket::Socket(SOCKET socket, addrinfo* addrInfo)
    : windowsSocket(socket), addressInfo(addrInfo)
{
}

GioNet::Socket::~Socket()
{
    FreeAddressInfo();

    if(windowsSocket != INVALID_SOCKET)
    {
        
        windowsSocket = INVALID_SOCKET;
    }
}

bool GioNet::Socket::Bind()
{
    int errorCode = bind(windowsSocket, addressInfo->ai_addr, static_cast<int>(addressInfo->ai_addrlen));

    if (errorCode == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        FreeAddressInfo();
        closesocket(windowsSocket);
        return false;
    }
    
    // addrinfo no longer needed 
    FreeAddressInfo();

    return true;
}

void GioNet::Socket::FreeAddressInfo()
{
    if(addressInfo != nullptr)
    {
        freeaddrinfo(addressInfo);
        addressInfo = nullptr;
    }
}
