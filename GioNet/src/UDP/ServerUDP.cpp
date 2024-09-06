#include "UDP/ServerUDP.h"

#include "Buffer.h"


GioNet::ServerUDP::ServerUDP(unsigned short port)
    : Server(std::make_shared<Socket>(NetAddress{"", port}, CommunicationProtocols::UDP))
{
}

GioNet::ServerUDP::~ServerUDP()
{
    listenThread.detach();
}

void GioNet::ServerUDP::Start()
{
    Server::Start();
    printf("Starting UDP server...\n");
    listenThread = std::thread{&ServerUDP::RunListenThread, this};
}

void GioNet::ServerUDP::RunListenThread()
{
    std::shared_ptr<Socket> socket = GetSocket();
    while (socket && socket->IsValid())
    {
        NetAddress source{};
        std::optional<Buffer> received = socket->ReceiveFrom(&source);

        if(received)
        {
            if(!HasPeer(source))
            {
                AddPeer(Peer{source, nullptr});
            }
            
            printf("Received data: %s\n", received->Data());
        }
        else
        {
            // TODO - Fix recv failed after a broken send
        }
    }
}

std::optional<int> GioNet::ServerUDP::DoSend(const Buffer& buffer, const Peer& peer)
{
    return GetSocketChecked().SendTo(buffer, peer.address);
}
