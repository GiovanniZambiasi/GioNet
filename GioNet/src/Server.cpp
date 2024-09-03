#include "Server.h"
#include <assert.h>
#include <string>
#include "Socket.h"

GioNet::Server::Server(const std::shared_ptr<Socket>& listenSocket)
    : listenSocket(listenSocket)
{
}

void GioNet::Server::Listen()
{
    assert(listenSocket && listenSocket->IsValid());
    listenSocket->Listen();
    threads.emplace_back(&Server::ConnectionLoop, this);
}

void GioNet::Server::AddConnectedPeer(const Peer& peer)
{
    connectedPeers.push_back(peer);
    threads.emplace_back(&Server::ReceiveLoop, this, peer);
}

void GioNet::Server::ConnectionLoop()
{
    while (true)
    {
        printf("Server listening for connections...\n");
        Peer client = listenSocket->Accept();
        printf("Successfully connected to client %s\n", client.ToString().c_str());
        AddConnectedPeer(client);
    }
}

void GioNet::Server::ReceiveLoop(const Peer& peer)
{
    while (true)
    {
        char buffer[512];
        listenSocket->ReceiveFrom(peer.windowsSocket, &buffer[0], 512);
        printf("Received data from peer: '%s'\n", &buffer[0]);
    }
}

