#include "TCP/ServerTCP.h"

#include <ranges>

GioNet::ServerTCP::ServerTCP(unsigned short port)
    : Server(std::make_shared<Socket>(NetAddress{"", port}, CommunicationProtocols::TCP))
{    
}

GioNet::ServerTCP::~ServerTCP()
{
    connectionThread.detach();

    for (auto& thread : receiveThreads | std::views::values)
    {
        thread.detach();
    }

    receiveThreads.clear();
}

void GioNet::ServerTCP::Start()
{
    Server::Start();
    printf("Starting TCP server...\n");
    Socket& socket = GetSocketChecked();
    if(socket.Listen())
    {
        connectionThread = std::thread{&ServerTCP::RunConnectionThread, this};
    }
}

void GioNet::ServerTCP::RunConnectionThread()
{
    std::shared_ptr<Socket> socket = GetSocket();
    
    while (socket && socket->IsValid())
    {
        printf("Server listening for next connection...\n");

        std::shared_ptr<Socket> client = socket->AcceptConnection();
        
        if(client->IsValid())
        {
            AddPeer({client->GetAddress(), client});
        }
    }

    printf("Connection loop finished because connection was lost\n");
}

void GioNet::ServerTCP::OnPostPeerAdded(const Peer& peer)
{
    Server::OnPostPeerAdded(peer);
    
    receiveThreads[peer.address] = std::thread{&ServerTCP::RunReceiveThreadForPeer, this, peer};
    const char* greeting = "Greetings from server!";
    peer.connection->Send(greeting, 1 + strlen(greeting));
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

void GioNet::ServerTCP::RunReceiveThreadForPeer(const Peer& peer)
{
    std::shared_ptr<Socket> socket = peer.connection;
    while (socket && socket->IsValid())
    {
        char buffer[GIONET_DEFAULT_BUFFER];
        int receivedBytes = socket->Receive(buffer, sizeof(buffer));

        if(receivedBytes > 0)
        {
            // Yay! Got Data!
            printf("Data received from peer: %s\n", buffer);
        }
        else if(receivedBytes == SOCKET_ERROR)  // TODO - Refactor socket receive so ServerTCP doesn't need to know about SOCKET_ERROR
        {
            RemovePeer(peer);
            break;
        }
    }
}
