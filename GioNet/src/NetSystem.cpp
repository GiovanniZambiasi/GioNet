#include "NetSystem.h"
#include <assert.h>
#include "Client.h"
#include "Server.h"

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
        GIONET_LOG("[ERROR]: Winsock initialization with result %i\n", errorCode);
    }
}

GioNet::NetSystem::~NetSystem()
{
    WINSOCK_CALL_AND_REPORT(WSACleanup())
}

std::shared_ptr<GioNet::Server> GioNet::NetSystem::CreateServer(unsigned short port)
{
    return std::make_shared<Server>(port);
}

std::shared_ptr<GioNet::Client> GioNet::NetSystem::CreateClient(const char* ip, unsigned short port)
{
    return std::make_shared<Client>(NetAddress{ip, port});
}
