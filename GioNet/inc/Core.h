﻿#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <string>
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define GIONET_DEFAULT_PORT 27015

#define GIONET_DEFAULT_BUFFER 512 

#define GIONET_NOCOPY(ClassName) \
    ClassName(const ClassName&) = delete;\
    ClassName& operator=(const ClassName&) = delete;\

#define GIONET_NOMOVE(ClassName)\
    ClassName(ClassName&&) = delete;\
    ClassName& operator=(ClassName&&) = delete;

#define GIONET_NOCOPY_NOMOVE(ClassName)\
    GIONET_NOCOPY(ClassName)\
    GIONET_NOMOVE(ClassName)

#define WINSOCK_REPORT_ERROR() printf("[ERROR][Winsock]: Call failed with code %ld. ('%s':%i)\n", WSAGetLastError(), __FILE__, __LINE__)

#define WINSOCK_CALL_AND_REPORT(Call)\
    bool success = Call == 0;\
    if(!success)\
    { WINSOCK_REPORT_ERROR(); }\

namespace GioNet
{
    enum class CommunicationProtocols
    {
        TCP,
        UDP,
    };

    struct NetAddress
    {
        std::string ip{};

        unsigned short port{0};

        bool IsServer() const { return ip.empty(); }
        
        bool operator==(const NetAddress& address) const = default;
    };
}