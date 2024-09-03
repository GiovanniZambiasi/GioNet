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
    printf("Server listening for connections...\n");
    while (true)
    {
        Peer client = listenSocket->Accept();
        printf("Successfully connected to client %s\n", client.ToString().c_str());
        AddConnectedPeer(client);
    }
}

void GioNet::Server::AddConnectedPeer(const Peer& peer)
{
    connectedPeers.push_back(peer);
}
