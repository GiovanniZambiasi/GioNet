#include "Server.h"
#include <assert.h>
#include <string>
#include "Socket.h"

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

void GioNet::Server::SendToPeer(const std::shared_ptr<Socket>& peer, const char* buffer, int len)
{
    auto connection = receiveThreads.find(peer);
    if(connection == receiveThreads.end())
    {
        printf("[ERROR]: Tried to send data to peer %s, but it's not connected", peer->ToString().c_str());
        return;
    }

    connection->first->Send(buffer, len);
}

void GioNet::Server::AddPeer(const std::shared_ptr<Socket>& peer)
{
    std::unique_lock _{connectionMutex};
    receiveThreads[peer] = std::thread{&Server::ReceiveLoop, this, peer};
}

void GioNet::Server::RemovePeer(const std::shared_ptr<Socket>& peer)
{
    std::unique_lock _{connectionMutex};

    auto receiveThread = receiveThreads.find(peer);
    if(receiveThread != receiveThreads.end())
    {
        receiveThread->second.detach();
        receiveThreads.erase(peer);
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
        NetAddress source{};
        received = peer->Receive(&buffer[0], sizeof(buffer), &source);

        if(received > 0)
        {
            printf("Received data from peer: '%s'\n", &buffer[0]);

            const char* buffer = "Hello from server!";
            peer->Send(buffer, strlen(buffer) + 1, source);
        }
        else
        {
            RemovePeer(peer);
        }
    }
    while (received != SOCKET_ERROR);
}
