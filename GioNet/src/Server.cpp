#include "Server.h"
#include <assert.h>
#include <string>
#include "Socket.h"

std::string GioNet::Peer::ToString() const
{
    return connection ? connection->ToString() : address.ToString();
}

GioNet::Server::Server(unsigned short port, CommunicationProtocols protocol)
{
    listenSocket = std::make_shared<Socket>(NetAddress{"", port}, protocol);
    listenSocket->Bind();
}

GioNet::Server::~Server()
{
    for (auto& entry : receiveThreads)
    {
        entry.second.detach();
    }

    listenThread.detach();
}

void GioNet::Server::Start()
{
    assert(listenSocket && listenSocket->IsValid());

    switch (listenSocket->GetProtocol())
    {
    case CommunicationProtocols::TCP:
        if (listenSocket->Listen())
        {
            listenThread = std::thread{&Server::ConnectionLoop, this};
        }
        break;
    case CommunicationProtocols::UDP:
        printf("Server started receiving data...\n");
        listenThread = std::thread{&Server::ReceiveLoop, this, listenSocket};
        break;
    default:
        printf("[ERROR]: Unimplemented protocol!");
        break;
    }
}

void GioNet::Server::AddPeer(const Peer& peer)
{
    std::unique_lock _{connectionMutex};
    receiveThreads[peer.address] = std::thread{&Server::ReceiveLoop, this, peer.connection};
    peers.push_back(peer);
    printf("Successfully connected to peer %s\n", peer.ToString().c_str());
}

void GioNet::Server::RemovePeer(const Peer& peer)
{
    std::unique_lock _{connectionMutex};

    auto peerEntry = std::find(peers.begin(), peers.end(), peer); 
    if(peerEntry != peers.end())
    {
        peers.erase(peerEntry);
        auto receiveThread = receiveThreads.find(peer.address);
        if(receiveThread != receiveThreads.end())
        {
            receiveThread->second.detach();
            receiveThreads.erase(peer.address);
        }
    }
    
    printf("Disconnecting peer %s\n", peer.ToString().c_str());
}

void GioNet::Server::ConnectionLoop()
{
    while (true)
    {
        printf("Server listening for connections...\n");
        std::shared_ptr<Socket> client = listenSocket->Accept();
        
        if(client->IsValid())
        {
            AddPeer({client->GetAddress(), client});
        }
        else if(!listenSocket->IsValid())
        {
            break;
        }
    }

    printf("Connection loop finished because connection was lost\n");
}

// TODO - Separate UDP receive from TCP receive
void GioNet::Server::ReceiveLoop(const std::shared_ptr<Socket>& socket)
{
    int received;
    do
    {
        char buffer[GIONET_DEFAULT_BUFFER];
        NetAddress source{};
        received = socket->Receive(&buffer[0], sizeof(buffer), &source);

        if(received > 0)
        {
            printf("Received data from peer: '%s'\n", &buffer[0]);

            const char* buffer = "Hello from server!";
            socket->Send(buffer, strlen(buffer) + 1, source);
        }
        else
        {
            RemovePeer({source, socket});
        }
    }
    while (received != SOCKET_ERROR);
}
