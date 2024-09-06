#include "NetSystem.h"
#include <assert.h>
#include <cstdio>
#include "Client.h"
#include "Server.h"
#include "TCP/ClientTCP.h"
#include "TCP/ServerTCP.h"
#include "UDP/ClientUDP.h"
#include "UDP/ServerUDP.h"

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

std::shared_ptr<GioNet::Server> GioNet::NetSystem::CreateServer(unsigned short port, CommunicationProtocols protocol)
{
    switch (protocol)
    {
    case CommunicationProtocols::TCP:
        return std::make_shared<ServerTCP>(port);
    case CommunicationProtocols::UDP:
        return std::make_shared<ServerUDP>(port);
    default:
        assert(false);
        return {};
    }
}

std::shared_ptr<GioNet::Client> GioNet::NetSystem::CreateClient(const char* ip, unsigned short port, CommunicationProtocols protocol)
{
    switch (protocol)
    {
    case CommunicationProtocols::TCP:
        return std::make_shared<ClientTCP>(NetAddress{ip, port});
    case CommunicationProtocols::UDP:
        return std::make_shared<ClientUDP>(NetAddress{ip, port});
    default:
        assert(false);
        return {};
    }
}
