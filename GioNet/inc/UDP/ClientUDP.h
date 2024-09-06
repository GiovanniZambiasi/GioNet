#pragma once
#include "Client.h"

namespace GioNet
{
    class ClientUDP : public Client
    {
    public:
        ClientUDP(const NetAddress& address);

    private:
        void Start() override;

        std::optional<Buffer> DoReceive() override;
    };    
}
