#pragma once
#include "Core.h"

namespace GioNet
{
    class Socket
    {
        SOCKET windowsSocket{INVALID_SOCKET};
        addrinfo* addressInfo{nullptr};

    public:
        Socket() = default;
        Socket(SOCKET socket, addrinfo* addrInfo);

        ~Socket();

        GIONET_NOCOPY(Socket)
        
        // TODO - Does this work?
        Socket(Socket&& other) = default;
        Socket& operator=(Socket&& other) = default;

        bool IsValid() const
        {
            return windowsSocket != INVALID_SOCKET;
        }

        // SERVER
        bool Bind();

        bool Listen();

        SOCKET Accept();

        //CLIENT
        bool Connect();

    private:
        void FreeAddressInfo();
    };
}