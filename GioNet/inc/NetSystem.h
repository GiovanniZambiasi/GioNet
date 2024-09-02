#pragma once
#include <memory>
#include "Core.h"

namespace GioNet
{
}

namespace GioNet
{
    struct SocketCreationParams;
    class Socket;
    
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

        std::shared_ptr<Socket> OpenServerSocket(const char* port = nullptr);

        std::shared_ptr<Socket> OpenClientSocket(const char* ip, const char* port = nullptr);

    private:
        std::shared_ptr<Socket> OpenSocket(const SocketCreationParams& config);
    };    
}


