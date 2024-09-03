#pragma once
#include <memory>
#include "Core.h"

namespace GioNet
{
    struct SocketParams;
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

        std::shared_ptr<Server> StartServer(const char* port = nullptr);

        std::shared_ptr<Client> StartClient(const char* ip, const char* port = nullptr);

        std::shared_ptr<Socket> CreateAndBindServerSocket(const char* port = nullptr);

        std::shared_ptr<Socket> CreateClientSocket(const char* ip, const char* port = nullptr);

    private:
        std::shared_ptr<Socket> CreateSocket(const char* ip, const char* port, const addrinfo& config);
    };    
}


