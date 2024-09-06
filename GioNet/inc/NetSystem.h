#pragma once
#include <memory>
#include "Core.h"

namespace GioNet
{
    class Socket;
    class Server;
    class Client;
    
    class NetSystem
    {
        static std::unique_ptr<NetSystem> instance;

        WSADATA wsaData{};
        
        NetSystem();

    public:
        static NetSystem& Get();
        
        ~NetSystem();

        GIONET_NOCOPY_NOMOVE(NetSystem)
        
        static void Reset() { instance.reset(); }

        std::shared_ptr<Server> CreateServer(unsigned short port, CommunicationProtocols protocol);

        std::shared_ptr<Client> CreateClient(const char* ip, unsigned short port, CommunicationProtocols protocol);
    };    
}


