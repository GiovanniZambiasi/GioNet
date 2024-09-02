#include <assert.h>
#include <iostream>
#include "GioNet.h"

int main(int argC, char* argV[])
{
    assert(argC > 1);
    std::string netMode = argV[1];
    
    if(netMode == "server")
    {
        auto& sys = GioNet::NetSystem::Get();
        GioNet::Server server = sys.StartServer();
        server.Listen();
    }
    else if(netMode == "client")
    {
        auto& sys = GioNet::NetSystem::Get();
        std::shared_ptr<GioNet::Socket> socket = sys.OpenClientSocket("localhost");
        socket->Connect();
    }
    else
    {
        return 1;
    }
}

