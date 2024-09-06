#pragma once
#include "Client.h"

namespace GioNet
{
    class ClientTCP : public Client
    {
    public:
        ClientTCP(const NetAddress& address);

    private:
        void Start() override;

        std::optional<Buffer> DoReceive() override;
        
    };
}
