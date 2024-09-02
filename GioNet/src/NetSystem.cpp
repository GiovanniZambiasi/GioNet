#include "NetSystem.h"

#include <assert.h>
#include <cstdio>

#include "Server.h"
#include "Socket.h"

std::unique_ptr<GioNet::NetSystem> GioNet::NetSystem::instance{};

GioNet::NetSystem& GioNet::NetSystem::Get()
{
    if(!instance)
    {
        instance = std::unique_ptr<NetSystem>{new NetSystem{}};
    }

    assert(instance);
    return *instance;
}

GioNet::NetSystem::NetSystem()
{
    int errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(errorCode != 0)
    {
        printf("[ERROR]: Winsock initialization with result %i\n", errorCode);
    }
}

GioNet::NetSystem::~NetSystem()
{
    int result = WSACleanup();

    if(result != 0)
    {
        printf("[ERROR]: Winsock initialization with result %ld\n", WSAGetLastError());
    }
}

GioNet::Server GioNet::NetSystem::StartServer(const char* port)
{
    Server s{};
    std::shared_ptr<Socket> socket = OpenServerSocket(port);
    s.BindSocket(socket);
    return s;
}

std::shared_ptr<GioNet::Socket> GioNet::NetSystem::OpenServerSocket(const char* port)
{
    addrinfo hints{};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    
    return OpenSocket({nullptr, port, hints});
}

std::shared_ptr<GioNet::Socket> GioNet::NetSystem::OpenClientSocket(const char* ip, const char* port)
{
    addrinfo hints{};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    return OpenSocket({ip, port, hints});
}

std::shared_ptr<GioNet::Socket> GioNet::NetSystem::OpenSocket(const SocketCreationParams& config)
{
    addrinfo* result{nullptr};
    int errorCode = getaddrinfo(config.ip, config.port ? config.port : GIONET_DEFAULT_PORT, &config.winSettings, &result);

    if(errorCode != 0)
    {
        printf("[ERROR]: Starting socket failed with error code %i\n", errorCode);
        return {};
    }

    std::shared_ptr<Socket> s = std::make_shared<Socket>(
        socket(result->ai_family, result->ai_socktype, result->ai_protocol),
        result
    );

    if(!s->IsValid())
    {
        printf("[ERROR]: Starting socket failed with error code %ld\n", WSAGetLastError());
        return {};
    }

    return s;
}
