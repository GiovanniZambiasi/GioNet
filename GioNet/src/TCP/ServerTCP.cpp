#include "TCP/ServerTCP.h"

#include <ranges>

#include "Buffer.h"

GioNet::ServerTCP::ServerTCP(unsigned short port)
    : Server(std::make_shared<Socket>(NetAddress{"", port}, CommunicationProtocols::TCP))
{    
}

GioNet::ServerTCP::~ServerTCP()
{
    ServerTCP::Stop();
}

void GioNet::ServerTCP::Start()
{
    Server::Start();
    GIONET_LOG("Starting TCP server...\n");
    Socket& socket = GetSocketChecked();
    if(socket.Listen())
    {
        connectionThread = std::jthread{&ServerTCP::RunConnectionThread, this};
    }
}

void GioNet::ServerTCP::Stop()
{
    Server::Stop();

    connectionThread.request_stop();
    receiveThreads.clear();
}

void GioNet::ServerTCP::RunConnectionThread()
{
    std::shared_ptr<Socket> socket = GetSocket();
    
    while (socket && socket->IsValid() && !connectionThread.get_stop_token().stop_requested())
    {
        GIONET_LOG("Server listening for next connection...\n");

        std::shared_ptr<Socket> client = socket->AcceptConnection();
        
        if(client && client->IsValid())
        {
            AddPeer({client->GetAddress(), client});
        }
    }

    GIONET_LOG("Connection loop finished because connection was lost\n");
}

void GioNet::ServerTCP::OnPostPeerAdded(const Peer& peer)
{
    Server::OnPostPeerAdded(peer);
    
    receiveThreads[peer.address] = std::jthread{[this, peer](std::stop_token stop)
    {
        RunReceiveThreadForPeer(peer, stop);
    }};
    peer.connection->Send({"Greetings from server!"});
}

void GioNet::ServerTCP::OnPrePeerRemoved(const Peer& peer)
{
    Server::OnPrePeerRemoved(peer);
    
    auto receiveThread = receiveThreads.find(peer.address);
    if(receiveThread != receiveThreads.end())
    {
        receiveThread->second.detach();
        receiveThreads.erase(peer.address);
    }
}

void GioNet::ServerTCP::RunReceiveThreadForPeer(const Peer& peer, std::stop_token stop)
{
    std::shared_ptr<Socket> socket = peer.connection;
    while (socket && socket->IsValid() && !stop.stop_requested())
    {
        std::optional<Buffer> received = socket->Receive();

        if(received)
        {
            GIONET_LOG("Data received from peer: %s\n", received->Data());
        }
        else
        {
            RemovePeer(peer);
            break;
        }
    }
}

std::optional<int> GioNet::ServerTCP::DoSend(const Buffer& buffer, const Peer& peer)
{
    if(peer.connection && peer.connection->IsValid())
        return peer.connection->Send(buffer);
    
    return {};
}
