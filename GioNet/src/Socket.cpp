#include "Socket.h"

GioNet::Socket::Socket(SOCKET socket, addrinfo* addrInfo)
    : windowsSocket(socket), addressInfo(addrInfo)
{
}

GioNet::Socket::~Socket()
{
    if(windowsSocket != INVALID_SOCKET)
    {
        freeaddrinfo(addressInfo);
        addressInfo = nullptr;
        windowsSocket = INVALID_SOCKET;
    }
}
