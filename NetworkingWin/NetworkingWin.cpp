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
        std::shared_ptr<GioNet::Socket> socket = sys.OpenServerSocket();
        
        if(socket->Bind())
        {
        }
        else
        {
            return 2;
        }
    }
    else if(netMode == "client")
    {
        auto& sys = GioNet::NetSystem::Get();
        std::shared_ptr<GioNet::Socket> socket = sys.OpenClientSocket("localhost");
    }
    else
    {
        return 1;
    }
}

