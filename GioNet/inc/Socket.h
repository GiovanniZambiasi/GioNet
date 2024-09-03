#pragma once
#include "Core.h"

namespace GioNet
{
    struct Peer;
    
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

        int Send(const char* buffer, int len);

        int SendTo(SOCKET socket, const char* buffer, int len);

        int ReceiveFrom(SOCKET socket, char* buffer, int len);

        int Receive(char* buffer, int len);
    
        // SERVER
        bool Bind();

        bool Listen();

        Peer Accept();

        //CLIENT
        bool Connect();

    private:
        void FreeAddressInfo();
    };
}
