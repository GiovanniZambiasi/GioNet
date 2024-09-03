#include "Server.h"
#include <assert.h>
#include <string>
#include "Socket.h"

GioNet::Server::Server(const std::shared_ptr<Socket>& listenSocket)
    : listenSocket(listenSocket)
{
}

GioNet::Server::~Server()
{
    for (auto& entry : connectionThreads)
    {
        entry.second.detach();
    }

    listenThread.detach();
}

void GioNet::Server::Listen()
{
    assert(listenSocket && listenSocket->IsValid());
    listenSocket->Listen();
    listenThread = std::thread{&Server::ConnectionLoop, this};
}

void GioNet::Server::SendToPeer(const Peer& peer, const char* buffer, int len)
{
    listenSocket->SendTo(peer.windowsSocket, buffer, len);
}

void GioNet::Server::AddPeer(const Peer& peer)
{
    std::unique_lock _{connectionMutex};
    const char* buffer = "Hello from server!";
    SendToPeer(peer, buffer, strlen(buffer) + 1);
    connectionThreads[peer] = std::thread{&Server::ReceiveLoop, this, peer};
}

void GioNet::Server::RemovePeer(const Peer& peer)
{
    std::unique_lock _{connectionMutex};

    auto receiveThread = connectionThreads.find(peer);
    if(receiveThread != connectionThreads.end())
    {
        receiveThread->second.detach();
        connectionThreads.erase(peer);
    }
    
    printf("Disconnecting peer %s\n", peer.ToString().c_str());
}

void GioNet::Server::ConnectionLoop()
{
    while (true)
    {
        printf("Server listening for connections...\n");
        Peer client = listenSocket->Accept();
        
        if(client.IsValid())
        {
            printf("Successfully connected to client %s\n", client.ToString().c_str());
            AddPeer(client);
        }
        else if(!listenSocket->IsValid())
        {
            break;
        }
    }

    printf("Connection loop finished because connection was lost\n");
}

void GioNet::Server::ReceiveLoop(const Peer& peer)
{
    int received;
    do
    {
        char buffer[GIONET_DEFAULT_BUFFER];
        received = listenSocket->ReceiveFrom(peer.windowsSocket, &buffer[0], sizeof(buffer));

        if(received > 0)
        {
            printf("Received data from peer: '%s'\n", &buffer[0]);
        }
        else
        {
            RemovePeer(peer);
        }
    }
    while (received != SOCKET_ERROR);
}
