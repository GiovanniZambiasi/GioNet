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
    for (std::thread& thread : threads)
    {
        thread.detach();
    }

    threads.clear();
}

void GioNet::Server::Listen()
{
    assert(listenSocket && listenSocket->IsValid());
    listenSocket->Listen();
    threads.emplace_back(&Server::ConnectionLoop, this);
}

void GioNet::Server::SendToPeer(const Peer& peer, const char* buffer, int len)
{
    listenSocket->SendTo(peer.windowsSocket, buffer, len);
}

void GioNet::Server::AddConnectedPeer(const Peer& peer)
{
    connectedPeers.push_back(peer);
    const char* buffer = "Hello from server!";
    SendToPeer(peer, buffer, strlen(buffer) + 1);
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
    int received;
    do
    {
        char buffer[GIONET_DEFAULT_BUFFER];
        received = listenSocket->ReceiveFrom(peer.windowsSocket, &buffer[0], sizeof(buffer));
        printf("Received data from peer: '%s'\n", &buffer[0]);
    }
    while (received != SOCKET_ERROR);
}

