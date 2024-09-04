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

std::shared_ptr<GioNet::Server> GioNet::NetSystem::StartServer(unsigned short port)
{
    return std::make_shared<Server>(port);
}

std::shared_ptr<GioNet::Client> GioNet::NetSystem::StartClient(const char* ip, unsigned short port)
{
    std::shared_ptr<Client> client = std::make_shared<Client>(NetAddress{ip, port});
    client->Connect();
    return client;
}
