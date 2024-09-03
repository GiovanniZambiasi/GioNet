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
    std::shared_ptr<Socket> socket = CreateAndBindServerSocket(port);
    return {socket};
}

GioNet::Client GioNet::NetSystem::StartClient(const char* ip, const char* port)
{
    std::shared_ptr<Socket> socket = CreateClientSocketAndConnect(ip, port);
    return {socket};
}

std::shared_ptr<GioNet::Socket> GioNet::NetSystem::CreateAndBindServerSocket(const char* port)
{
    addrinfo hints{};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    std::shared_ptr<Socket> socket = CreateSocket(nullptr, port, hints);
    socket->Bind();
    return socket;
}

std::shared_ptr<GioNet::Socket> GioNet::NetSystem::CreateClientSocketAndConnect(const char* ip, const char* port)
{
    addrinfo hints{};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::shared_ptr<Socket> socket = CreateSocket(ip, port, hints);
    socket->Connect();
    return socket;
}

std::shared_ptr<GioNet::Socket> GioNet::NetSystem::CreateSocket(const char* ip, const char* port, const addrinfo& config)
{
    addrinfo* result{nullptr};
    WINSOCK_CALL_AND_REPORT(getaddrinfo(ip, port ? port : GIONET_DEFAULT_PORT, &config, &result))
   
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
