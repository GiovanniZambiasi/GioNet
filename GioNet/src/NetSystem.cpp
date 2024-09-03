#include "NetSystem.h"
#include <assert.h>
#include <cstdio>
#include "Client.h"
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
    WINSOCK_CALL_AND_REPORT(WSACleanup())
}

GioNet::Server GioNet::NetSystem::StartServer(const char* port)
{
    Server s{};
    std::shared_ptr<Socket> socket = OpenServerSocket(port);
    s.BindSocket(socket);
    return s;
}

GioNet::Client GioNet::NetSystem::StartClient(const char* ip, const char* port)
{
    Client c{};
    std::shared_ptr<Socket> socket = OpenClientSocket(ip, port);
    c.Connect(socket);
    return c;
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
    WINSOCK_CALL_AND_REPORT(getaddrinfo(config.ip, config.port ? config.port : GIONET_DEFAULT_PORT, &config.winSettings, &result))
   
    if(!success)
    {
        return {};
    }

    std::shared_ptr<Socket> s = std::make_shared<Socket>(
        socket(result->ai_family, result->ai_socktype, result->ai_protocol),
        result
    );

    if(!s->IsValid())
    {
        WINSOCK_REPORT_ERROR();
        return {};
    }

    return s;
}
