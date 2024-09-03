#include "Socket.h"
#include <cstdio>

#include "Peer.h"

GioNet::Socket::Socket(SOCKET socket, addrinfo* addrInfo)
    : windowsSocket(socket), addressInfo(addrInfo)
{
}

GioNet::Socket::~Socket()
{
    FreeAddressInfo();
    windowsSocket = INVALID_SOCKET;
}

bool GioNet::Socket::Bind()
{
    int errorCode = bind(windowsSocket, addressInfo->ai_addr, static_cast<int>(addressInfo->ai_addrlen));

    if (errorCode == SOCKET_ERROR)
    {
        WINSOCK_REPORT_ERROR();
        FreeAddressInfo();
        closesocket(windowsSocket);
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
        closesocket(windowsSocket);
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
        closesocket(windowsSocket);
        return {};
    }

    char buff[16];
    inet_ntop(addr.sin_family, addr.sin_zero, &buff[0], sizeof(buff));

    return {ClientSocket, {buff}, addr.sin_port};
}

bool GioNet::Socket::Connect()
{
    // Connect to server.
    int result = connect( windowsSocket, addressInfo->ai_addr, static_cast<int>(addressInfo->ai_addrlen));
    if (result == SOCKET_ERROR) {
        closesocket(windowsSocket);
        windowsSocket = INVALID_SOCKET;
    }

    // Should really try the next address returned by getaddrinfo
    // if the connect call failed
    // But for this simple example we just free the resources
    // returned by getaddrinfo and print an error message
    FreeAddressInfo();

    if (windowsSocket == INVALID_SOCKET) {
        printf("Connection failed!\n");
        return false;
    }

    printf("Connection successfully established!");
    
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
