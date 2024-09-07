#include "UDP/ServerUDP.h"

#include "Buffer.h"


GioNet::ServerUDP::ServerUDP(unsigned short port)
    : Server(std::make_shared<Socket>(NetAddress{"", port}, CommunicationProtocols::UDP))
{
}

GioNet::ServerUDP::~ServerUDP()
{
    ServerUDP::Stop();
}

void GioNet::ServerUDP::Start()
{
    Server::Start();
    GIONET_LOG("Starting UDP server...\n");
    listenThread = std::jthread{&ServerUDP::RunListenThread, this};
}

void GioNet::ServerUDP::Stop()
{
    Server::Stop();

    if(listenThread.joinable())
    {
        listenThread.request_stop();
    }
}

void GioNet::ServerUDP::RunListenThread()
{
    std::shared_ptr<Socket> socket = GetSocket();
    while (socket && socket->IsValid() && !listenThread.get_stop_token().stop_requested())
    {
        NetAddress source{};
        std::optional<Buffer> received = socket->ReceiveFrom(&source);

        if(received)
        {
            if(const Peer* peer = TryGetPeer(source))
            {
                InvokeDataReceived(*peer, std::move(*received));
                received.reset();
            }
            else
            {
                Peer newPeer{source, socket};
                AddPeer(newPeer);
                InvokeDataReceived(newPeer, std::move(*received));
                received.reset();
            }
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
