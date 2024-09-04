#include "Server.h"
#include <assert.h>
#include <string>
#include "Socket.h"

GioNet::Server::Server(unsigned short port)
{
    listenSocket = std::make_shared<Socket>(NetAddress{"", port}, CommunicationProtocols::TCP);
    listenSocket->Bind();
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

    if (listenSocket->Listen())
    {
        listenThread = std::thread{&Server::ConnectionLoop, this};
    }
}

void GioNet::Server::SendToPeer(const std::shared_ptr<Socket>& peer, const char* buffer, int len)
{
    auto connection = connectionThreads.find(peer);
    if(connection == connectionThreads.end())
    {
        printf("[ERROR]: Tried to send data to peer %s, but it's not connected", peer->ToString().c_str());
        return;
    }

    connection->first->Send(buffer, len);
}

void GioNet::Server::AddPeer(const std::shared_ptr<Socket>& peer)
{
    std::unique_lock _{connectionMutex};
    connectionThreads[peer] = std::thread{&Server::ReceiveLoop, this, peer};
    
    const char* buffer = "Hello from server!";
    SendToPeer(peer, buffer, strlen(buffer) + 1);
}

void GioNet::Server::RemovePeer(const std::shared_ptr<Socket>& peer)
{
    std::unique_lock _{connectionMutex};

    auto receiveThread = connectionThreads.find(peer);
    if(receiveThread != connectionThreads.end())
    {
        receiveThread->second.detach();
        connectionThreads.erase(peer);
    }
    
    printf("Disconnecting peer %s\n", peer->ToString().c_str());
}

void GioNet::Server::ConnectionLoop()
{
    while (true)
    {
        printf("Server listening for connections...\n");
        std::shared_ptr<Socket> client = listenSocket->Accept();
        
        if(client->IsValid())
        {
            printf("Successfully connected to client\n");
            AddPeer(client);
        }
        else if(!listenSocket->IsValid())
        {
            break;
        }
    }

    printf("Connection loop finished because connection was lost\n");
}

void GioNet::Server::ReceiveLoop(const std::shared_ptr<Socket>& peer)
{
    int received;
    do
    {
        char buffer[GIONET_DEFAULT_BUFFER];
        received = peer->Receive(&buffer[0], sizeof(buffer));

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
